#	$NetBSD: Makefile,v 1.2 2019/01/27 02:08:45 pgoyette Exp $

.include "../Makefile.inc"

.PATH:	${S}/compat/common

KMOD=	compat_40

CPPFLAGS+=	-DCOMPAT_40 -DCOMPAT_50 -DCOMPAT_60 -DCOMPAT_70 -DCOMPAT_80

SRCS+=	compat_40_mod.c
SRCS+=	vfs_syscalls_40.c uipc_syscalls_40.c

.PATH:	${S}/dev/sysmon

SRCS+=	sysmon_power_40.c

.include <bsd.kmodule.mk>
