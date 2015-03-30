#	$NetBSD$
#

.PATH:	${.CURDIR}/../../../../netipsec

CPPFLAGS+=	-DIPSEC -DINET -DINET6

SRCS+=		ipsec.c ipsec_netbsd.c key_debug.c xform_esp.c \
		ipsec_input.c ipsec_output.c keysock.c xform_ipcomp.c \
		ipsec_mbuf.c key.c xform_ah.c xform_ipip.c
# For TCP_SIGNATURE
#SRCS+=		xform_tcp.c

CPPFLAGS+=	-I${.CURDIR}/opt -I${.CURDIR}/../libnet/opt

# IPsec debugging
#CPPFLAGS+=	-DIPSEC_DEBUG
