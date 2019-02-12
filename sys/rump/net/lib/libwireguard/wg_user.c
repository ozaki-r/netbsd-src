/*	$NetBSD$	*/

/*
 * Copyright (C) Ryota Ozaki <ozaki.ryota@gmail.com>
 * All rights reserved.
 *
 * Based on wg_user.c by Antti Kantee.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#ifndef _KERNEL
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/param.h>

#include <net/if.h>
#include <net/if_tun.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rump/rumpuser_component.h>

#include "wg_user.h"

struct wg_user {
	struct wg_softc *wgu_sc;
	int wgu_devnum;
	char wgu_tun_name[IFNAMSIZ];

	int wgu_fd;
	int wgu_pipe[2];
	pthread_t wgu_rcvthr;

	int wgu_dying;

	char wgu_rcvbuf[9018]; /* jumbo frame max len */
	struct sockaddr_storage wgu_rcvsa;
};

static int
open_tun(const char *tun_name)
{
	char tun_path[MAXPATHLEN];
	int n, fd, error;

	n = snprintf(tun_path, sizeof(tun_path), "/dev/%s", tun_name);
	if (n == MAXPATHLEN)
		return E2BIG;

	fd = open(tun_path, O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "%s: can't open %s: %s\n",
		    __func__, tun_name, strerror(errno));
	}

	int i = 1;
	error = ioctl(fd, TUNSLMODE, &i);
	if (error == -1) {
		close(fd);
		fd = -1;
	}

	return fd;
}

static void
close_tun(struct wg_user *wgu)
{

	close(wgu->wgu_fd);
}

static void *
wg_user_rcvthread(void *aaargh)
{
	struct wg_user *wgu = aaargh;
	struct pollfd pfd[2];
	struct iovec iov[2];
	ssize_t nn = 0;
	int prv;

	rumpuser_component_kthread();

	pfd[0].fd = wgu->wgu_fd;
	pfd[0].events = POLLIN;
	pfd[1].fd = wgu->wgu_pipe[0];
	pfd[1].events = POLLIN;

	while (!wgu->wgu_dying) {
		prv = poll(pfd, 2, -1);
		if (prv == 0)
			continue;
		if (prv == -1) {
			/* XXX */
			fprintf(stderr, "%s: poll error: %d\n",
			    wgu->wgu_tun_name, errno);
			sleep(1);
			continue;
		}
		if (pfd[1].revents & POLLIN)
			continue;

		iov[0].iov_base = &wgu->wgu_rcvsa;
		iov[0].iov_len = sizeof(wgu->wgu_rcvsa);
		iov[1].iov_base = wgu->wgu_rcvbuf;
		iov[1].iov_len = sizeof(wgu->wgu_rcvbuf);

		nn = readv(wgu->wgu_fd, iov, 2);
		if (nn == -1 && errno == EAGAIN)
			continue;

		if (nn < 1) {
			/* XXX */
			fprintf(stderr, "%s: receive failed\n",
			    wgu->wgu_tun_name);
			sleep(1);
			continue;
		}

		rumpuser_component_schedule(NULL);
		wg_user_recv(wgu->wgu_sc, iov, 2);
		rumpuser_component_unschedule();
	}

	assert(wgu->wgu_dying);

	rumpuser_component_kthread_release();
	return NULL;
}

int
wg_user_create(const char *tun_name, struct wg_softc *wg,
    struct wg_user **wgup)
{
	struct wg_user *wgu = NULL;
	void *cookie;
	int rv;

	cookie = rumpuser_component_unschedule();

	wgu = malloc(sizeof(*wgu));
	if (wgu == NULL) {
		rv = errno;
		goto oerr1;
	}

	wgu->wgu_fd = open_tun(tun_name);
	if (wgu->wgu_fd == -1) {
		rv = errno;
		goto oerr2;
	}
	strcpy(wgu->wgu_tun_name, tun_name);
	wgu->wgu_sc = wg;

	if (pipe(wgu->wgu_pipe) == -1) {
		rv = errno;
		goto oerr3;
	}

	rv = pthread_create(&wgu->wgu_rcvthr, NULL, wg_user_rcvthread, wgu);
	if (rv != 0)
		goto oerr4;

	rumpuser_component_schedule(cookie);
	*wgup = wgu;
	return 0;

 oerr4:
	close(wgu->wgu_pipe[0]);
	close(wgu->wgu_pipe[1]);
 oerr3:
	close_tun(wgu);
 oerr2:
	free(wgu);
 oerr1:
	rumpuser_component_schedule(cookie);
	return rumpuser_component_errtrans(rv);
}

void
wg_user_send(struct wg_user *wgu, struct iovec *iov, size_t iovlen)
{
	void *cookie = rumpuser_component_unschedule();
	ssize_t idontcare __attribute__((__unused__));

	/*
	 * no need to check for return value; packets may be dropped
	 *
	 * ... sorry, I spoke too soon.  We need to check it because
	 * apparently gcc reinvented const poisoning and it's very
	 * hard to say "thanks, I know I'm not using the result,
	 * but please STFU and let's get on with something useful".
	 * So let's trick gcc into letting us share the compiler
	 * experience.
	 */
	idontcare = writev(wgu->wgu_fd, iov, iovlen);

	rumpuser_component_schedule(cookie);
}

int wg_user_dying(struct wg_user *);
int
wg_user_dying(struct wg_user *wgu)
{
	void *cookie = rumpuser_component_unschedule();

	wgu->wgu_dying = 1;
	if (write(wgu->wgu_pipe[1],
	    &wgu->wgu_dying, sizeof(wgu->wgu_dying)) == -1) {
		/*
		 * this is here mostly to avoid a compiler warning
		 * about ignoring the return value of write()
		 */
		fprintf(stderr, "%s: failed to signal thread\n",
		    wgu->wgu_tun_name);
	}

	rumpuser_component_schedule(cookie);

	return 0;
}

void
wg_user_destroy(struct wg_user *wgu)
{
	void *cookie = rumpuser_component_unschedule();

	pthread_join(wgu->wgu_rcvthr, NULL);
	close_tun(wgu);
	close(wgu->wgu_pipe[0]);
	close(wgu->wgu_pipe[1]);
	free(wgu);

	rumpuser_component_schedule(cookie);
}
#endif
