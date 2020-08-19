/*	$NetBSD: hypervisor.h,v 1.1 2018/04/01 04:35:03 ryo Exp $	*/
/*-
 * Copyright (c) 2013, 2014 Andrew Turner
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: head/sys/arm64/include/hypervisor.h 281494 2015-04-13 14:43:10Z andrew $
 */

#ifndef _MACHINE_HYPERVISOR_H_
#define	_MACHINE_HYPERVISOR_H_

/*
 * These registers are only useful when in hypervisor context,
 * e.g. specific to EL2, or controlling the hypervisor.
 */

/*
 * Architecture feature trap register
 */
#define	CPTR_RES0	0x7fefc800
#define	CPTR_RES1	0x000033ff
#define	CPTR_TFP	0x00000400
#define	CPTR_TTA	0x00100000
#define	CPTR_TCPAC	0x80000000

/*
 * Hypervisor Config Register
 */

#define	HCR_VM		__BIT(0)
#define	HCR_SWIO	__BIT(1)
#define	HCR_PTW		__BIT(2)
#define	HCR_FMO		__BIT(3)
#define	HCR_IMO		__BIT(4)
#define	HCR_AMO		__BIT(5)
#define	HCR_VF		__BIT(6)
#define	HCR_VI		__BIT(7)
#define	HCR_VSE		__BIT(8)
#define	HCR_FB		__BIT(9)
#define	HCR_BSU_MASK	__BITS(11,10)
#define	HCR_DC		__BIT(12)
#define	HCR_TWI		__BIT(13)
#define	HCR_TWE		__BIT(14)
#define	HCR_TID0	__BIT(15)
#define	HCR_TID1	__BIT(16)
#define	HCR_TID2	__BIT(17)
#define	HCR_TID3	__BIT(18)
#define	HCR_TSC		__BIT(19)
#define	HCR_TIDCP	__BIT(20)
#define	HCR_TACR	__BIT(21)
#define	HCR_TSW		__BIT(22)
#define	HCR_TPC		__BIT(23)
#define	HCR_TPU		__BIT(24)
#define	HCR_TTLB	__BIT(25)
#define	HCR_TVM		__BIT(26)
#define	HCR_TGE		__BIT(27)
#define	HCR_TDZ		__BIT(28)
#define	HCR_HCD		__BIT(29)
#define	HCR_TRVM	__BIT(30)
#define	HCR_RW		__BIT(31)
#define	HCR_CD		__BIT(32)
#define	HCR_ID		__BIT(33)
#define	HCR_E2H		__BIT(34)
#define	HCR_TLOR	__BIT(35)
#define	HCR_TERR	__BIT(36)
#define	HCR_TEA		__BIT(37)
#define	HCR_MIOCNCE	__BIT(38)
/*     	RES0		__BIT(39) */
#define	HCR_APK		__BIT(40)
#define	HCR_API		__BIT(41)
#define	HCR_NV		__BIT(42)
#define	HCR_NV1		__BIT(43)
#define	HCR_AT		__BIT(44)
#define	HCR_NV2		__BIT(45)
#define	HCR_FWB		__BIT(46)
#define	HCR_FIEN	__BIT(47)
/*     	RES0		__BIT(48) */
#define	HCR_TID4	__BIT(49)
#define	HCR_TICAB	__BIT(50)
#define	HCR_AMVOFFEN	__BIT(51)
#define	HCR_TOCU	__BIT(52)
#define	HCR_EnSCXT	__BIT(53)
#define	HCR_TTLBIS	__BIT(54)
#define	HCR_TTLBOS	__BIT(55)
#define	HCR_ATA		__BIT(56)
#define	HCR_DCT		__BIT(57)
#define	HCR_TID5	__BIT(58)
#define	HCR_TWEDEn	__BIT(59)
#define	HCR_TWEDEL_MASK	__BITS(63,60)

#endif

