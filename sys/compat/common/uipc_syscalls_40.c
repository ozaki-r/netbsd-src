/*	$NetBSD: uipc_syscalls_40.c,v 1.8 2014/11/26 09:53:53 ozaki-r Exp $	*/

/* written by Pavel Cahyna, 2006. Public domain. */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: uipc_syscalls_40.c,v 1.8 2014/11/26 09:53:53 ozaki-r Exp $");

/*
 * System call interface to the socket abstraction.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/msg.h>
#include <sys/sysctl.h>
#include <sys/syscallargs.h>
#include <sys/errno.h>

#include <net/if.h>

#include <compat/sys/socket.h>
#include <compat/sys/sockio.h>

#ifdef COMPAT_OIFREQ
/*
 * Return interface configuration
 * of system.  List may be used
 * in later ioctl's (above) to get
 * other information.
 *
 * If the user buffer pointer is NULL, this routine copies no data and
 * returns the amount of space that would be needed.
 */
/*ARGSUSED*/
int
compat_ifconf(u_long cmd, void *data)
{
	struct oifconf *ifc = data;
	struct ifnet *ifp;
	struct ifaddr *ifa;
	struct oifreq ifr, *ifrp = NULL;
	int space = 0, error = 0, buflen = 0;
	const int sz = (int)sizeof(ifr);
	const bool docopy = ifc->ifc_req != NULL;
	char *buf = NULL;
	int s;

	if (docopy) {
		buflen = space = ifc->ifc_len;
		buf = kmem_zalloc(buflen, KM_SLEEP);
		KASSERT(buf != NULL);
		ifrp = (struct oifreq*)buf;
	}

	IFNET_RENTER(s);
	IFNET_FOREACH(ifp) {
		(void)strncpy(ifr.ifr_name, ifp->if_xname,
		    sizeof(ifr.ifr_name));
		if (ifr.ifr_name[sizeof(ifr.ifr_name) - 1] != '\0') {
			error = ENAMETOOLONG;
			goto out;
		}
		if (IFADDR_EMPTY(ifp)) {
			memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
			if (space >= sz) {
				memcpy(ifrp, &ifr, sz);
				ifrp++;
			}
			space -= sz;
			continue;
		}

		IFADDR_FOREACH(ifa, ifp) {
			struct sockaddr *sa = ifa->ifa_addr;
#ifdef COMPAT_OSOCK
			if (cmd == OOSIOCGIFCONF) {
				struct osockaddr *osa =
				    (struct osockaddr *)&ifr.ifr_addr;
				/*
				 * If it does not fit, we don't bother with it
				 */
				if (sa->sa_len > sizeof(*osa))
					continue;
				memcpy(&ifr.ifr_addr, sa, sa->sa_len);
				osa->sa_family = sa->sa_family;
				if (space >= sz) {
					memcpy(ifrp, &ifr, sz);
					ifrp++;
				}
			} else
#endif
			if (sa->sa_len <= sizeof(*sa)) {
				memcpy(&ifr.ifr_addr, sa, sa->sa_len);
				if (space >= sz) {
					memcpy(ifrp, &ifr, sz);
					ifrp++;
				}
			} else {
				space -= sa->sa_len - sizeof(*sa);
				if (space >= sz) {
					memcpy(ifrp, &ifr,
					    sizeof(ifr.ifr_name));
					memcpy(&ifrp->ifr_addr, sa,
					    sa->sa_len);
					ifrp = (struct oifreq *)
						(sa->sa_len +
						 (char *)&ifrp->ifr_addr);
				}
			}
			space -= sz;
		}
	}
out:
	IFNET_REXIT(s);

	if (error == 0) {
		if (docopy) {
			ifc->ifc_len -= space;
			KASSERT(ifc->ifc_len <= buflen);
			error = copyout(buf, ifc->ifc_req, ifc->ifc_len);
		} else
			ifc->ifc_len = -space;
	}

	if (docopy)
		kmem_free(buf, buflen);

	return error;
}
#endif
