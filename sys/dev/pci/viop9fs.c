/*	$NetBSD$	*/

/*
 * Copyright (c) 2019 Internet Initiative Japan, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/condvar.h>
#include <sys/device.h>
#include <sys/mutex.h>
#include <sys/sysctl.h>
//#include <uvm/uvm_page.h>
#include <sys/module.h>
#include <sys/syslog.h>
#include <sys/select.h>
#include <sys/kmem.h>

#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/uio.h>

#include <dev/pci/virtioreg.h>
#include <dev/pci/virtiovar.h>

#include "ioconf.h"

//#define V9FS_DEBUG	1
//#define V9FS_DUMP	1
#ifdef V9FS_DEBUG
#define DLOG(level, fmt, args...) \
	do { log(level, "%s: " fmt, __func__, ##args); } while (0)
#else
#define DLOG(level, fmt, args...) __nothing
#endif

/* Configuration registers */
#define VIRTIO_P9FS_CONFIG_TAG_LEN	0 /* 16bit */
#define VIRTIO_P9FS_CONFIG_TAG		2

/* Feature bits */
#define VIRTIO_P9FS_F_MUST_TELL_HOST (1<<0)
#define VIRTIO_P9FS_F_STATS_VQ	(1<<1)

#define VIRTIO_P9FS_FLAG_BITS \
	VIRTIO_COMMON_FLAG_BITS \
	"\x02""STATS_VQ" \
	"\x01""MUST_TELL_HOST"

// #define P9P_DEFREQLEN (16*1024) from usr.sbin/puffs/mount_9p/ninepuffs.h
#define V9FS_MAX_REQLEN		(16 * 1024)
#define V9FS_SEGSIZE		PAGE_SIZE
#define V9FS_N_SEGMENTS		(V9FS_MAX_REQLEN / V9FS_SEGSIZE)

#define P9_MAX_TAG_LEN		16

CTASSERT((PAGE_SIZE) == (VIRTIO_PAGE_SIZE)); /* XXX */

struct viop9fs_softc {
	device_t		sc_dev;

	struct virtio_softc	*sc_virtio;
	struct virtqueue	sc_vq[1];

	uint16_t		sc_taglen;
	uint8_t			sc_tag[P9_MAX_TAG_LEN];

	int			sc_state;
#define VIOP9FS_S_INIT		0
#define VIOP9FS_S_REQUESTING	1
#define VIOP9FS_S_REPLIED	2
#define VIOP9FS_S_CONSUMING	3

	kcondvar_t		sc_wait;
	kmutex_t		sc_lock;

	int			sc_flags;
#define VIOP9FS_INUSE		__BIT(0)
	struct selinfo		sc_sel;

	bus_dmamap_t		sc_dmamap_tx;
	bus_dmamap_t		sc_dmamap_rx;
	char			*sc_buf_tx;
	char			*sc_buf_rx;
	size_t			sc_buf_rx_len;
	off_t			sc_buf_rx_offset;
};

static struct viop9fs_softc *viop9fs_sc;

static int	p9_initialized = 0; /* multiple p9 is not allowed */

static int	viop9fs_match(device_t, cfdata_t, void *);
static void	viop9fs_attach(device_t, device_t, void *);
static void	viop9fs_read_config(struct viop9fs_softc *);
static int	viop9fs_config_change(struct virtio_softc *);
static int	viop9fs_request_done(struct virtqueue *);

static int	viop9fs_read(struct file *, off_t *, struct uio *, kauth_cred_t,
		    int);
static int	viop9fs_write(struct file *, off_t *, struct uio *,
		    kauth_cred_t, int);
static int	viop9fs_ioctl(struct file *, u_long, void *);
static int	viop9fs_close(struct file *);
static int	viop9fs_kqfilter(struct file *, struct knote *);

static const struct fileops viop9fs_fileops = {
	.fo_name = "viop9fs",
	.fo_read = viop9fs_read,
	.fo_write = viop9fs_write,
	.fo_ioctl = viop9fs_ioctl,
	.fo_fcntl = fnullop_fcntl,
	.fo_poll = fnullop_poll,
	.fo_stat = fbadop_stat,
	.fo_close = viop9fs_close,
	.fo_kqfilter = viop9fs_kqfilter,
	.fo_restart = fnullop_restart,
};

static dev_type_open(viop9fs_dev_open);
static dev_type_close(viop9fs_dev_close);

const struct cdevsw viop9fs_cdevsw = {
	.d_open = viop9fs_dev_open,
	.d_close = viop9fs_dev_close,
	.d_read = noread,
	.d_write = nowrite,
	.d_ioctl = noioctl,
	.d_stop = nostop,
	.d_tty = notty,
	.d_poll = nopoll,
	.d_mmap = nommap,
	.d_kqfilter = nokqfilter,
	.d_discard = nodiscard,
	.d_flag = D_OTHER | D_MPSAFE
};

#ifdef _MODULE
devmajor_t viop9fs_bmajor = -1, viop9fs_cmajor = -1;
#endif

#if 0
void
viop9fsattach(int unused)
{

	/*
	 * Nothing to do here, initialization is handled by the
	 * module initialization code in tuninit() below).
	 */
}

static void
viop9fsinit(void)
{

#if 0
	mutex_init(&tun_softc_lock, MUTEX_DEFAULT, IPL_NET);
	LIST_INIT(&tun_softc_list);
	LIST_INIT(&tunz_softc_list);
#endif
#ifdef _MODULE
	devsw_attach("viop9fs", NULL, &viop9fs_bmajor, &viop9fs_cdevsw, &viop9fs_cmajor);
#endif
}
#endif

static int
viop9fs_dev_open(dev_t dev, int flag, int mode, struct lwp *l)
{
	struct viop9fs_softc *sc = viop9fs_sc;
	struct file *fp;
	int error, fd;

	KASSERT(sc != NULL);

#if 0
	if (ISSET(sc->sc_flags, VIOP9FS_INUSE))
		return EBUSY;
#endif

	/* falloc() will fill in the descriptor for us. */
	error = fd_allocfile(&fp, &fd);
	if (error != 0)
		return error;

	sc->sc_flags |= VIOP9FS_INUSE;

	return fd_clone(fp, fd, flag, &viop9fs_fileops, sc);
}

int
viop9fs_dev_close(dev_t dev, int flag, int mode, struct lwp *l)
{
	struct viop9fs_softc *sc = viop9fs_sc;

	KASSERT(ISSET(sc->sc_flags, VIOP9FS_INUSE));
	sc->sc_flags &= ~VIOP9FS_INUSE;

	return 0;
}

static int
viop9fs_ioctl(struct file *fp, u_long cmd, void *addr)
{
	int error = 0;

	switch (cmd) {
	case FIONBIO:
		break;
	default:
		error = EINVAL;
		break;
	}

	return error;
}

static int
viop9fs_read(struct file *fp, off_t *offp, struct uio *uio,
    kauth_cred_t cred, int flags)
{
	struct viop9fs_softc *sc = fp->f_data;
	struct virtio_softc *vsc = sc->sc_virtio;
	struct virtqueue *vq = &sc->sc_vq[0];
	int error, slot, len;

	DLOG(LOG_DEBUG, "%s: enter\n", __func__);

	if (sc->sc_state == VIOP9FS_S_INIT) {
		DLOG(LOG_DEBUG, "%s: not requested\n", device_xname(sc->sc_dev));
		return EAGAIN;
	}

	if (sc->sc_state == VIOP9FS_S_CONSUMING) {
		KASSERT(sc->sc_buf_rx_len > 0);
		/* We already have some remaining, consume it. */
		len = sc->sc_buf_rx_len - sc->sc_buf_rx_offset;
		goto consume;
	}

#if 0
	if (uio->uio_resid != V9FS_MAX_REQLEN)
		return EINVAL;
#else
	if (uio->uio_resid > V9FS_MAX_REQLEN)
		return EINVAL;
#endif

	error = 0;
	mutex_enter(&sc->sc_lock);
	while (sc->sc_state == VIOP9FS_S_REQUESTING) {
		error = cv_wait_sig(&sc->sc_wait, &sc->sc_lock);
		if (error != 0)
			break;
	}
	if (sc->sc_state == VIOP9FS_S_REPLIED)
		sc->sc_state = VIOP9FS_S_CONSUMING;
	mutex_exit(&sc->sc_lock);

	if (error != 0)
		return error;

	error = virtio_dequeue(vsc, vq, &slot, &len);
	if (error != 0) {
		log(LOG_ERR, "%s: virtio_dequeue failed: %d\n",
		       device_xname(sc->sc_dev), error);
		return error;
	}
	DLOG(LOG_DEBUG, "%s: len=%d\n", __func__, len);
	sc->sc_buf_rx_len = len;
	sc->sc_buf_rx_offset = 0;
	bus_dmamap_sync(virtio_dmat(vsc), sc->sc_dmamap_tx, 0, V9FS_MAX_REQLEN,
	    BUS_DMASYNC_POSTWRITE);
	bus_dmamap_sync(virtio_dmat(vsc), sc->sc_dmamap_rx, 0, V9FS_MAX_REQLEN,
	    BUS_DMASYNC_POSTREAD);
	virtio_dequeue_commit(vsc, vq, slot);
#ifdef V9FS_DUMP
	int i;
	log(LOG_DEBUG, "%s: buf: ", __func__);
	for (i = 0; i < len; i++) {
		log(LOG_DEBUG, "%c", (char)sc->sc_buf_rx[i]);
	}
	log(LOG_DEBUG, "\n");
#endif

consume:
	DLOG(LOG_DEBUG, "%s: uio_resid=%lu\n", __func__, uio->uio_resid);
	if (len < uio->uio_resid)
		return EINVAL;
	len = uio->uio_resid;
	error = uiomove(sc->sc_buf_rx + sc->sc_buf_rx_offset, len, uio);
	if (error != 0)
		return error;

	sc->sc_buf_rx_offset += len;
	if (sc->sc_buf_rx_offset == sc->sc_buf_rx_len) {
		sc->sc_buf_rx_len = 0;
		sc->sc_buf_rx_offset = 0;
		/* FIXME */
		mutex_enter(&sc->sc_lock);
		sc->sc_state = VIOP9FS_S_INIT;
		selnotify(&sc->sc_sel, 0, 1);
		mutex_exit(&sc->sc_lock);
	}

	return error;
}

static int
viop9fs_write(struct file *fp, off_t *offp, struct uio *uio,
    kauth_cred_t cred, int flags)
{
	struct viop9fs_softc *sc = fp->f_data;
	struct virtio_softc *vsc = sc->sc_virtio;
	struct virtqueue *vq = &sc->sc_vq[0];
	int error, slot;
	size_t len;

	DLOG(LOG_DEBUG, "%s: enter\n", __func__);

	if (sc->sc_state != VIOP9FS_S_INIT) {
		DLOG(LOG_DEBUG, "%s: already requesting\n", __func__);
		error = EAGAIN;
		goto out;
	}

	if (uio->uio_resid == 0) {
		error = 0;
		goto out;
	}

	if (uio->uio_resid > V9FS_MAX_REQLEN) {
		error = EINVAL;
		goto out;
	}

	len = uio->uio_resid;
	error = uiomove(sc->sc_buf_tx, len, uio);
	if (error != 0)
		goto out;

	DLOG(LOG_DEBUG, "%s: len=%lu\n", __func__, len);
#ifdef V9FS_DUMP
	int i;
	log(LOG_DEBUG, "%s: buf: ", __func__);
	for (i = 0; i < len; i++) {
		log(LOG_DEBUG, "%c", (char)sc->sc_buf_tx[i]);
	}
	log(LOG_DEBUG, "\n");
#endif

	error = virtio_enqueue_prep(vsc, vq, &slot);
	if (error != 0) {
		log(LOG_ERR, "%s: virtio_enqueue_prep failed\n",
		       device_xname(sc->sc_dev));
		goto out;
	}
	DLOG(LOG_DEBUG, "%s: slot=%d\n", __func__, slot);
	error = virtio_enqueue_reserve(vsc, vq, slot, V9FS_N_SEGMENTS * 2);
	if (error != 0) {
		log(LOG_ERR, "%s: virtio_enqueue_reserve failed\n",
		       device_xname(sc->sc_dev));
		goto out;
	}

	/* Tx */
	bus_dmamap_sync(virtio_dmat(vsc), sc->sc_dmamap_tx, 0,
	    len, BUS_DMASYNC_PREWRITE);
	virtio_enqueue(vsc, vq, slot, sc->sc_dmamap_tx, true);
	/* Rx */
	bus_dmamap_sync(virtio_dmat(vsc), sc->sc_dmamap_rx, 0,
	    V9FS_MAX_REQLEN, BUS_DMASYNC_PREREAD);
	virtio_enqueue(vsc, vq, slot, sc->sc_dmamap_rx, false);
	virtio_enqueue_commit(vsc, vq, slot, true);

	sc->sc_state = VIOP9FS_S_REQUESTING;
out:
	return error;
}

static int
viop9fs_close(struct file *fp)
{

	return 0;
}

static void
filt_viop9fs_detach(struct knote *kn)
{
	struct viop9fs_softc *sc = kn->kn_hook;

	/* FIXME lock */
	SLIST_REMOVE(&sc->sc_sel.sel_klist, kn, knote, kn_selnext);
}

static int
filt_viop9fs_read(struct knote *kn, long hint)
{
	struct viop9fs_softc *sc = kn->kn_hook;
	int rv;

	kn->kn_data = sc->sc_buf_rx_len;
	rv = (kn->kn_data > 0) || sc->sc_state != VIOP9FS_S_INIT;

	return rv;
}

static const struct filterops viop9fs_read_filtops = {
	.f_isfd = 1,
	.f_attach = NULL,
	.f_detach = filt_viop9fs_detach,
	.f_event = filt_viop9fs_read,
};

static int
filt_viop9fs_write(struct knote *kn, long hint)
{
	struct viop9fs_softc *sc = kn->kn_hook;

	return sc->sc_state == VIOP9FS_S_INIT;
}

static const struct filterops viop9fs_write_filtops = {
	.f_isfd = 1,
	.f_attach = NULL,
	.f_detach = filt_viop9fs_detach,
	.f_event = filt_viop9fs_write,
};

static int
viop9fs_kqfilter(struct file *fp, struct knote *kn)
{
	struct viop9fs_softc *sc = fp->f_data;
	struct klist *klist;

	/* FIXME lock */
	switch (kn->kn_filter) {
	case EVFILT_READ:
		klist = &sc->sc_sel.sel_klist;
		kn->kn_fop = &viop9fs_read_filtops;
		break;

	case EVFILT_WRITE:
		klist = &sc->sc_sel.sel_klist;
		kn->kn_fop = &viop9fs_write_filtops;
		break;

	default:
		log(LOG_ERR, "%s: kn_filter=%u\n", __func__, kn->kn_filter);
		return EINVAL;
	}

	kn->kn_hook = sc;

	SLIST_INSERT_HEAD(klist, kn, kn_selnext);

	return 0;
}

CFATTACH_DECL_NEW(viop9fs, sizeof(struct viop9fs_softc),
    viop9fs_match, viop9fs_attach, NULL, NULL);

static int
viop9fs_match(device_t parent, cfdata_t match, void *aux)
{
	struct virtio_attach_args *va = aux;

	if (va->sc_childdevid == PCI_PRODUCT_VIRTIO_9P)
		return 1;

	return 0;
}

static void
viop9fs_attach(device_t parent, device_t self, void *aux)
{
	struct viop9fs_softc *sc = device_private(self);
	struct virtio_softc *vsc = device_private(parent);
	int error;

	if (virtio_child(vsc) != NULL) {
		aprint_normal(": child already attached for %s; "
			      "something wrong...\n", device_xname(parent));
		return;
	}

	if (p9_initialized++) {
		aprint_normal(": p9 already exists; something wrong...\n");
		goto err_none;
	}

	sc->sc_dev = self;
	sc->sc_virtio = vsc;

	error = virtio_alloc_vq(vsc, &sc->sc_vq[0], 0, V9FS_MAX_REQLEN, 2,
	    "viop9fs");
	if (error != 0)
		goto err_none;

	sc->sc_vq[0].vq_done = viop9fs_request_done;

	viop9fs_read_config(sc);
	aprint_normal(": tag=%s", sc->sc_tag);

	sc->sc_buf_tx = kmem_alloc(V9FS_MAX_REQLEN, KM_SLEEP);
	sc->sc_buf_rx = kmem_alloc(V9FS_MAX_REQLEN, KM_SLEEP);

	error = bus_dmamap_create(virtio_dmat(vsc), V9FS_MAX_REQLEN,
	    V9FS_N_SEGMENTS, V9FS_SEGSIZE, 0, BUS_DMA_WAITOK, &sc->sc_dmamap_tx);
	if (error != 0) {
		aprint_error_dev(sc->sc_dev, "bus_dmamap_create failed: %d\n",
		    error);
		goto err_vq;
	}
	error = bus_dmamap_create(virtio_dmat(vsc), V9FS_MAX_REQLEN,
	    V9FS_N_SEGMENTS, V9FS_SEGSIZE, 0, BUS_DMA_WAITOK, &sc->sc_dmamap_rx);
	if (error != 0) {
		aprint_error_dev(sc->sc_dev, "bus_dmamap_create failed: %d\n",
		    error);
		goto err_vq;
	}

	error = bus_dmamap_load(virtio_dmat(vsc), sc->sc_dmamap_tx,
	    sc->sc_buf_tx, V9FS_MAX_REQLEN, NULL, BUS_DMA_WAITOK | BUS_DMA_WRITE);
	if (error != 0) {
		aprint_error_dev(sc->sc_dev, "bus_dmamap_load failed: %d\n",
		    error);
		goto err_dmamap;
	}
	error = bus_dmamap_load(virtio_dmat(vsc), sc->sc_dmamap_rx,
	    sc->sc_buf_rx, V9FS_MAX_REQLEN, NULL, BUS_DMA_WAITOK | BUS_DMA_READ);
	if (error != 0) {
		aprint_error_dev(sc->sc_dev, "bus_dmamap_load failed: %d\n",
		    error);
		goto err_dmamap;
	}

	sc->sc_state = VIOP9FS_S_INIT;
	mutex_init(&sc->sc_lock, MUTEX_DEFAULT, IPL_NONE);
	cv_init(&sc->sc_wait, "viop9fs");

	virtio_child_attach_start(vsc, self, IPL_VM, sc->sc_vq,
	    viop9fs_config_change, virtio_vq_intr,
	    VIRTIO_F_PCI_INTR_MPSAFE | VIRTIO_F_PCI_INTR_SOFTINT, 0,
	    VIRTIO_P9FS_FLAG_BITS);

	error = virtio_child_attach_finish(vsc);
	if (error != 0)
		goto err_mutex;

	viop9fs_sc = sc;
	return;

err_mutex:
	cv_destroy(&sc->sc_wait);
	mutex_destroy(&sc->sc_lock);
err_dmamap:
	bus_dmamap_destroy(virtio_dmat(vsc), sc->sc_dmamap_tx);
	bus_dmamap_destroy(virtio_dmat(vsc), sc->sc_dmamap_rx);
err_vq:
	virtio_free_vq(vsc, &sc->sc_vq[0]);
err_none:
	virtio_child_attach_failed(vsc);
	return;
}

static void
viop9fs_read_config(struct viop9fs_softc *sc)
{
	unsigned int reg;
	int i;

	/* these values are explicitly specified as little-endian */
	reg = virtio_read_device_config_2(sc->sc_virtio,
	    VIRTIO_P9FS_CONFIG_TAG_LEN);
	sc->sc_taglen = le16toh(reg);

	for (i = 0; i < sc->sc_taglen; i++) {
		reg = virtio_read_device_config_4(sc->sc_virtio,
		    VIRTIO_P9FS_CONFIG_TAG + i);
		sc->sc_tag[i] = (uint8_t)reg;
	}
	sc->sc_tag[i] = '\0';
}

/*
 * Config change callback: wakeup the kthread.
 */
static int
viop9fs_config_change(struct virtio_softc *vsc)
{
#if 0
	struct viop9fs_softc *sc = device_private(virtio_child(vsc));

	viop9fs_read_config(sc);
	mutex_enter(&sc->sc_lock);
	cv_signal(&sc->sc_wait);
	mutex_exit(&sc->sc_lock);
#endif

	return 1;
}

static int
viop9fs_request_done(struct virtqueue *vq)
{
	struct virtio_softc *vsc = vq->vq_owner;
	struct viop9fs_softc *sc = device_private(virtio_child(vsc));

	DLOG(LOG_DEBUG, "%s\n", __func__);

	mutex_enter(&sc->sc_lock);
	sc->sc_state = VIOP9FS_S_REPLIED;
	cv_broadcast(&sc->sc_wait);
	selnotify(&sc->sc_sel, 0, 1);
	mutex_exit(&sc->sc_lock);

	return 1;
}

MODULE(MODULE_CLASS_DRIVER, viop9fs, "virtio");
 
#ifdef _MODULE
#include "ioconf.c"
#endif
 
static int 
viop9fs_modcmd(modcmd_t cmd, void *opaque)
{
	int error = 0;
 
#ifdef _MODULE
	switch (cmd) {
	case MODULE_CMD_INIT:
		error = config_init_component(cfdriver_ioconf_viop9fs, 
		    cfattach_ioconf_viop9fs, cfdata_ioconf_viop9fs); 
		devsw_attach("viop9fs", NULL, &viop9fs_bmajor, &viop9fs_cdevsw, &viop9fs_cmajor);
		break;
	case MODULE_CMD_FINI:
		error = config_fini_component(cfdriver_ioconf_viop9fs,
		    cfattach_ioconf_viop9fs, cfdata_ioconf_viop9fs);
		break;
	default:
		error = ENOTTY;
		break; 
	}
#endif
   
	return error;
}
