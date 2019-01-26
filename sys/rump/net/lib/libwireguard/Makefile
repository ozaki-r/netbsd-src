#	$NetBSD: Makefile,v 1.1 2015/11/04 23:57:06 knakahara Exp $
#

.PATH:	${.CURDIR}/../../../../net ${.CURDIR}/../../../../netinet	\
	${.CURDIR}/../../../../netinet6

LIB=	rumpnet_wireguard
COMMENT= WireGuard

IOCONF=	WG.ioconf
SRCS=	if_wg.c

# For sysctl knobs
CPPFLAGS.if_wg.c+=	-DWG_DEBUG

SRCS+=	wg_component.c

.include <bsd.lib.mk>
.include <bsd.klinks.mk>
