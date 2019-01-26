# $NetBSD$
#

.include <bsd.own.mk>

TESTSDIR=	${TESTSBASE}/net/wireguard

.for name in basic interoperability misc tunnel
TESTS_SH+=		t_${name}
TESTS_SH_SRC_t_${name}=	../net_common.sh ./common.sh t_${name}.sh
.endfor

.include <bsd.test.mk>
