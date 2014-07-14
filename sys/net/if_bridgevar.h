/*	$NetBSD: if_bridgevar.h,v 1.20 2014/07/14 02:34:36 ozaki-r Exp $	*/

/*
 * Copyright 2001 Wasabi Systems, Inc.
 * All rights reserved.
 *
 * Written by Jason R. Thorpe for Wasabi Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed for the NetBSD Project by
 *	Wasabi Systems, Inc.
 * 4. The name of Wasabi Systems, Inc. may not be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY WASABI SYSTEMS, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL WASABI SYSTEMS, INC
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1999, 2000 Jason L. Wright (jason@thought.net)
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Jason L. Wright
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * OpenBSD: if_bridge.h,v 1.14 2001/03/22 03:48:29 jason Exp
 */

/*
 * Data structure and control definitions for bridge interfaces.
 */

#ifndef _NET_IF_BRIDGEVAR_H_
#define _NET_IF_BRIDGEVAR_H_

#include <sys/callout.h>
#include <sys/queue.h>
#include <sys/mutex.h>
#include <sys/condvar.h>

/*
 * Commands used in the SIOCSDRVSPEC ioctl.  Note the lookup of the
 * bridge interface itself is keyed off the ifdrv structure.
 */
#define	BRDGADD			0	/* add bridge member (ifbreq) */
#define	BRDGDEL			1	/* delete bridge member (ifbreq) */
#define	BRDGGIFFLGS		2	/* get member if flags (ifbreq) */
#define	BRDGSIFFLGS		3	/* set member if flags (ifbreq) */
#define	BRDGSCACHE		4	/* set cache size (ifbrparam) */
#define	BRDGGCACHE		5	/* get cache size (ifbrparam) */
#define	BRDGGIFS		6	/* get member list (ifbifconf) */
#define	BRDGRTS			7	/* get address list (ifbaconf) */
#define	BRDGSADDR		8	/* set static address (ifbareq) */
#define	BRDGSTO			9	/* set cache timeout (ifbrparam) */
#define	BRDGGTO			10	/* get cache timeout (ifbrparam) */
#define	BRDGDADDR		11	/* delete address (ifbareq) */
#define	BRDGFLUSH		12	/* flush address cache (ifbreq) */

#define	BRDGGPRI		13	/* get priority (ifbrparam) */
#define	BRDGSPRI		14	/* set priority (ifbrparam) */
#define	BRDGGHT			15	/* get hello time (ifbrparam) */
#define	BRDGSHT			16	/* set hello time (ifbrparam) */
#define	BRDGGFD			17	/* get forward delay (ifbrparam) */
#define	BRDGSFD			18	/* set forward delay (ifbrparam) */
#define	BRDGGMA			19	/* get max age (ifbrparam) */
#define	BRDGSMA			20	/* set max age (ifbrparam) */
#define	BRDGSIFPRIO		21	/* set if priority (ifbreq) */
#define BRDGSIFCOST		22	/* set if path cost (ifbreq) */
#define BRDGGFILT	        23	/* get filter flags (ifbrparam) */
#define BRDGSFILT	        24	/* set filter flags (ifbrparam) */

/*
 * Generic bridge control request.
 */
struct ifbreq {
	char		ifbr_ifsname[IFNAMSIZ];	/* member if name */
	uint32_t	ifbr_ifsflags;		/* member if flags */
	uint8_t		ifbr_state;		/* member if STP state */
	uint8_t		ifbr_priority;		/* member if STP priority */
	uint8_t		ifbr_path_cost;		/* member if STP cost */
	uint8_t		ifbr_portno;		/* member if port number */
};


#ifdef __OpenBSD__
/* SIOCBRDGIFFLGS, SIOCBRDGIFFLGS */
#define	IFBIF_LEARNING		0x0001	/* ifs can learn */
#define	IFBIF_DISCOVER		0x0002	/* ifs sends packets w/unknown dest */
#define	IFBIF_BLOCKNONIP	0x0004	/* ifs blocks non-IP/ARP in/out */
#define	IFBIF_STP		0x0008	/* ifs participates in spanning tree */
#define IFBIF_BSTP_EDGE		0x0010	/* member stp edge port */
#define IFBIF_BSTP_AUTOEDGE	0x0020  /* member stp autoedge enabled */
#define IFBIF_BSTP_PTP		0x0040  /* member stp ptp */
#define IFBIF_BSTP_AUTOPTP	0x0080	/* member stp autoptp enabled */
#define	IFBIF_SPAN		0x0100	/* ifs is a span port (ro) */
#define	IFBIF_RO_MASK		0xff00	/* read only bits */
#endif

/* BRDGGIFFLAGS, BRDGSIFFLAGS */
#define	IFBIF_LEARNING		0x01	/* if can learn */
#define	IFBIF_DISCOVER		0x02	/* if sends packets w/ unknown dest. */
#define	IFBIF_STP		0x04	/* if participates in spanning tree */
#define	IFBIF_BLOCKNONIP	0x0008	/* ifs blocks non-IP/ARP in/out */
#define IFBIF_BSTP_EDGE		0x0010	/* member stp edge port */
#define IFBIF_BSTP_AUTOEDGE	0x0020  /* member stp autoedge enabled */
#define IFBIF_BSTP_PTP		0x0040  /* member stp ptp */
#define IFBIF_BSTP_AUTOPTP	0x0080	/* member stp autoptp enabled */
#define	IFBIF_SPAN		0x0100	/* ifs is a span port (ro) */
#define	IFBIF_RO_MASK		0xff00	/* read only bits */

#define	IFBIFBITS	"\020\1LEARNING\2DISCOVER\3STP"

/* BRDGFLUSH */
#define	IFBF_FLUSHDYN		0x00	/* flush learned addresses only */
#define	IFBF_FLUSHALL		0x01	/* flush all addresses */

/* BRDGSFILT */
#define IFBF_FILT_USEIPF	0x00000001 /* enable ipf on bridge */
#define IFBF_FILT_MASK		0x00000001 /* mask of valid values */

/* STP port states */
#define	BSTP_IFSTATE_DISABLED	0
#define	BSTP_IFSTATE_LISTENING	1
#define	BSTP_IFSTATE_LEARNING	2
#define	BSTP_IFSTATE_FORWARDING	3
#define	BSTP_IFSTATE_BLOCKING	4
#define	BSTP_IFSTATE_DISCARDING	5

/*
 * Interface list structure.
 */
struct ifbifconf {
	uint32_t	ifbic_len;	/* buffer size */
	union {
		void *	ifbicu_buf;
		struct ifbreq *ifbicu_req;
	} ifbic_ifbicu;
#define	ifbic_buf	ifbic_ifbicu.ifbicu_buf
#define	ifbic_req	ifbic_ifbicu.ifbicu_req
};

/*
 * Bridge address request.
 */
struct ifbareq {
	char		ifba_ifsname[IFNAMSIZ];	/* member if name */
	/*XXX: time_t */
	long		ifba_expire;		/* address expire time */
	uint8_t		ifba_flags;		/* address flags */
	uint8_t		ifba_dst[ETHER_ADDR_LEN];/* destination address */
};

#define	IFBAF_TYPEMASK	0x03	/* address type mask */
#define	IFBAF_DYNAMIC	0x00	/* dynamically learned address */
#define	IFBAF_STATIC	0x01	/* static address */

#define	IFBAFBITS	"\020\1STATIC"

/*
 * Address list structure.
 */
struct ifbaconf {
	uint32_t	ifbac_len;	/* buffer size */
	union {
		void *ifbacu_buf;
		struct ifbareq *ifbacu_req;
	} ifbac_ifbacu;
#define	ifbac_buf	ifbac_ifbacu.ifbacu_buf
#define	ifbac_req	ifbac_ifbacu.ifbacu_req
};

/*
 * Bridge parameter structure.
 */
struct ifbrparam {
	union {
		uint32_t ifbrpu_int32;
		uint16_t ifbrpu_int16;
		uint8_t ifbrpu_int8;
	} ifbrp_ifbrpu;
};
#define	ifbrp_csize	ifbrp_ifbrpu.ifbrpu_int32	/* cache size */
#define	ifbrp_ctime	ifbrp_ifbrpu.ifbrpu_int32	/* cache time (sec) */
#define	ifbrp_prio	ifbrp_ifbrpu.ifbrpu_int16	/* bridge priority */
#define	ifbrp_hellotime	ifbrp_ifbrpu.ifbrpu_int8	/* hello time (sec) */
#define	ifbrp_fwddelay	ifbrp_ifbrpu.ifbrpu_int8	/* fwd time (sec) */
#define	ifbrp_maxage	ifbrp_ifbrpu.ifbrpu_int8	/* max age (sec) */
#define	ifbrp_filter	ifbrp_ifbrpu.ifbrpu_int32	/* filtering flags */

#ifdef _KERNEL
#include <net/pktqueue.h>

/*
 * Timekeeping structure used in spanning tree code.
 */
struct bridge_timer {
	uint16_t	active;
	uint16_t	value;
};

#if 0
struct bstp_config_unit {
	uint64_t	cu_rootid;
	uint64_t	cu_bridge_id;
	uint32_t	cu_root_path_cost;
	uint16_t	cu_message_age;
	uint16_t	cu_max_age;
	uint16_t	cu_hello_time;
	uint16_t	cu_forward_delay;
	uint16_t	cu_port_id;
	uint8_t		cu_message_type;
	uint8_t		cu_topology_change_acknowledgment;
	uint8_t		cu_topology_change;
};

struct bstp_tcn_unit {
	uint8_t		tu_message_type;
};
#endif

struct bstp_timer {
	u_int16_t	active;
	u_int16_t	value;
	u_int32_t	latched;
};

struct bstp_pri_vector {
	u_int64_t	pv_root_id;
	u_int32_t	pv_cost;
	u_int64_t	pv_dbridge_id;
	u_int16_t	pv_dport_id;
	u_int16_t	pv_port_id;
};

struct bstp_config_unit {
	struct bstp_pri_vector	cu_pv;
	u_int16_t	cu_message_age;
	u_int16_t	cu_max_age;
	u_int16_t	cu_forward_delay;
	u_int16_t	cu_hello_time;
	u_int8_t	cu_message_type;
	u_int8_t	cu_topology_change_ack;
	u_int8_t	cu_topology_change;
	u_int8_t	cu_proposal;
	u_int8_t	cu_agree;
	u_int8_t	cu_learning;
	u_int8_t	cu_forwarding;
	u_int8_t	cu_role;
};

struct bstp_tcn_unit {
	u_int8_t	tu_message_type;
};

struct bstp_port {
	LIST_ENTRY(bstp_port)	bp_next;
	struct ifnet		*bp_ifp;	/* parent if */
	struct bstp_state	*bp_bs;
	void			*bp_lhcookie;	/* if linkstate hook */
	u_int8_t		bp_active;
	u_int8_t		bp_protover;
	u_int32_t		bp_flags;
	u_int32_t		bp_path_cost;
	u_int16_t		bp_port_msg_age;
	u_int16_t		bp_port_max_age;
	u_int16_t		bp_port_fdelay;
	u_int16_t		bp_port_htime;
	u_int16_t		bp_desg_msg_age;
	u_int16_t		bp_desg_max_age;
	u_int16_t		bp_desg_fdelay;
	u_int16_t		bp_desg_htime;
	struct bstp_timer	bp_edge_delay_timer;
	struct bstp_timer	bp_forward_delay_timer;
	struct bstp_timer	bp_hello_timer;
	struct bstp_timer	bp_message_age_timer;
	struct bstp_timer	bp_migrate_delay_timer;
	struct bstp_timer	bp_recent_backup_timer;
	struct bstp_timer	bp_recent_root_timer;
	struct bstp_timer	bp_tc_timer;
	struct bstp_config_unit bp_msg_cu;
	struct bstp_pri_vector	bp_desg_pv;
	struct bstp_pri_vector	bp_port_pv;
	u_int16_t		bp_port_id;
	u_int8_t		bp_state;
	u_int8_t		bp_tcstate;
	u_int8_t		bp_role;
	u_int8_t		bp_infois;
	u_int8_t		bp_tc_ack;
	u_int8_t		bp_tc_prop;
	u_int8_t		bp_fdbflush;
	u_int8_t		bp_priority;
	u_int8_t		bp_ptp_link;
	u_int8_t		bp_agree;
	u_int8_t		bp_agreed;
	u_int8_t		bp_sync;
	u_int8_t		bp_synced;
	u_int8_t		bp_proposing;
	u_int8_t		bp_proposed;
	u_int8_t		bp_operedge;
	u_int8_t		bp_reroot;
	u_int8_t		bp_rcvdtc;
	u_int8_t		bp_rcvdtca;
	u_int8_t		bp_rcvdtcn;
	u_int32_t		bp_forward_transitions;
	u_int8_t		bp_txcount;
};

/*
 * Software state for each bridge STP.
 */
struct bstp_state {
	struct ifnet		*bs_ifp;
	struct bstp_pri_vector	bs_bridge_pv;
	struct bstp_pri_vector	bs_root_pv;
	struct bstp_port	*bs_root_port;
	u_int8_t		bs_protover;
	u_int16_t		bs_migration_delay;
	u_int16_t		bs_edge_delay;
	u_int16_t		bs_bridge_max_age;
	u_int16_t		bs_bridge_fdelay;
	u_int16_t		bs_bridge_htime;
	u_int16_t		bs_root_msg_age;
	u_int16_t		bs_root_max_age;
	u_int16_t		bs_root_fdelay;
	u_int16_t		bs_root_htime;
	u_int16_t		bs_hold_time;
	u_int16_t		bs_bridge_priority;
	u_int8_t		bs_txholdcount;
	u_int8_t		bs_allsynced;
	callout_t		bs_bstpcallout;		/* stp callout */
	struct bstp_timer	bs_link_timer;
	struct timeval		bs_last_tc_time;
	LIST_HEAD(, bstp_port)	bs_bplist;
};
#define	bs_ifflags		bs_ifp->if_flags

/*
 * Bridge interface list entry.
 */
struct bridge_iflist {
	LIST_ENTRY(bridge_iflist) bif_next;
#if 0
	uint64_t		bif_designated_root;
	uint64_t		bif_designated_bridge;
	uint32_t		bif_path_cost;
	uint32_t		bif_designated_cost;
	struct bridge_timer	bif_hold_timer;
	struct bridge_timer	bif_message_age_timer;
	struct bridge_timer	bif_forward_delay_timer;
	uint16_t		bif_port_id;
	uint16_t		bif_designated_port;
	struct bstp_config_unit	bif_config_bpdu;
	uint8_t			bif_topology_change_acknowledge;
	uint8_t			bif_config_pending;
	uint8_t			bif_change_detection_enabled;
	uint8_t			bif_priority;
	uint8_t			bif_state;
#endif
	struct ifnet		*bif_ifp;	/* member if */
	struct bstp_port	*bif_stp;	/* STP port state */
	uint32_t		bif_flags;	/* member if flags */
	uint32_t		bif_refs;	/* reference count */
	bool			bif_waiting;	/* waiting for released  */
};
#define bif_state		bif_stp->bp_state

/*
 * Bridge route node.
 */
struct bridge_rtnode {
	LIST_ENTRY(bridge_rtnode) brt_hash;	/* hash table linkage */
	LIST_ENTRY(bridge_rtnode) brt_list;	/* list linkage */
	struct ifnet		*brt_ifp;	/* destination if */
	time_t			brt_expire;	/* expiration time */
	uint8_t			brt_flags;	/* address flags */
	uint8_t			brt_age;	/* age counter */
	uint8_t			brt_addr[ETHER_ADDR_LEN];
};

/*
 * Software state for each bridge.
 */
struct bridge_softc {
	struct ifnet		sc_if;
	LIST_ENTRY(bridge_softc) sc_list;
	uint64_t		sc_designated_root;
	uint64_t		sc_bridge_id;
	struct bridge_iflist	*sc_root_port;
	callout_t		sc_brcallout;	/* bridge callout */
	struct bstp_state	*sc_stp;	/* stp state */
#if 0
	uint32_t		sc_root_path_cost;
	uint16_t		sc_max_age;
	uint16_t		sc_hello_time;
	uint16_t		sc_forward_delay;
	uint16_t		sc_bridge_max_age;
	uint16_t		sc_bridge_hello_time;
	uint16_t		sc_bridge_forward_delay;
	uint16_t		sc_topology_change_time;
	uint16_t		sc_hold_time;
	uint16_t		sc_bridge_priority;
	uint8_t			sc_topology_change_detected;
	uint8_t			sc_topology_change;
#endif
	struct bridge_timer	sc_hello_timer;
	struct bridge_timer	sc_topology_change_timer;
	struct bridge_timer	sc_tcn_timer;
	uint32_t		sc_brtmax;	/* max # of addresses */
	uint32_t		sc_brtcnt;	/* cur. # of addresses */
	uint32_t		sc_brttimeout;	/* rt timeout in seconds */
	uint32_t		sc_hashkey;	/* hash key */
	LIST_HEAD(, bridge_iflist) sc_iflist;	/* member interface list */
	kmutex_t		*sc_iflist_lock;
	kcondvar_t		sc_iflist_cv;
	LIST_HEAD(, bridge_rtnode) *sc_rthash;	/* our forwarding table */
	LIST_HEAD(, bridge_rtnode) sc_rtlist;	/* list version of above */
	kmutex_t		*sc_rtlist_lock;
	uint32_t		sc_rthash_key;	/* key for hash */
	uint32_t		sc_filter_flags; /* ipf and flags */
	pktqueue_t *		sc_fwd_pktq;
};

/* Protocol versions */
#define	BSTP_PROTO_ID		0x00
#define	BSTP_PROTO_STP		0x00
#define	BSTP_PROTO_RSTP		0x02
#define	BSTP_PROTO_MAX		BSTP_PROTO_RSTP

/* STP port flags */
#define	BSTP_PORT_CANMIGRATE	0x0001
#define	BSTP_PORT_NEWINFO	0x0002
#define	BSTP_PORT_DISPUTED	0x0004
#define	BSTP_PORT_ADMCOST	0x0008
#define	BSTP_PORT_AUTOEDGE	0x0010
#define	BSTP_PORT_AUTOPTP	0x0020

extern const uint8_t bstp_etheraddr[];

void	bridge_ifdetach(struct ifnet *);

int	bridge_output(struct ifnet *, struct mbuf *, const struct sockaddr *,
	    struct rtentry *);

#if 0
void	bstp_initialization(struct bridge_softc *);
void	bstp_stop(struct bridge_softc *);
void	bstp_input(struct bridge_softc *, struct bridge_iflist *, struct mbuf *);
#endif
struct	bstp_state *bstp_create(struct ifnet *);
void	bstp_destroy(struct bstp_state *);
void	bstp_initialization(struct bstp_state *);
void	bstp_stop(struct bstp_state *);
int	bstp_ioctl(struct ifnet *, u_long, char *);
struct	bstp_port *bstp_add(struct bstp_state *, struct ifnet *);
void	bstp_delete(struct bstp_port *);
struct mbuf *bstp_input(struct bstp_state *, struct bstp_port *,
			struct ether_header *, struct mbuf *);
void	bstp_ifstate(void *);
uint8_t bstp_getstate(struct bstp_state *, struct bstp_port *);
void	bstp_ifsflags(struct bstp_port *, u_int);

void	bridge_enqueue(struct bridge_softc *, struct ifnet *, struct mbuf *,
	    int);

void	bridge_rtagenode(struct ifnet *, int);

#ifdef NET_MPSAFE
#define BRIDGE_MPSAFE	1
#endif

#define BRIDGE_LOCK(_sc)	if ((_sc)->sc_iflist_lock) \
					mutex_enter((_sc)->sc_iflist_lock)
#define BRIDGE_UNLOCK(_sc)	if ((_sc)->sc_iflist_lock) \
					mutex_exit((_sc)->sc_iflist_lock)
#define BRIDGE_LOCKED(_sc)	(!(_sc)->sc_iflist_lock || \
				 mutex_owned((_sc)->sc_iflist_lock))

#endif /* _KERNEL */
#endif /* !_NET_IF_BRIDGEVAR_H_ */
