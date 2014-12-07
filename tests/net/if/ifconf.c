/*	$NetBSD$	*/
/*
 * Copyright (c) 2014 The NetBSD Foundation, Inc.
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
__RCSID("$NetBSD$");
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <unistd.h>

#include <rump/rump.h>
#include <rump/rump_syscalls.h>
#include <rump/rumpclient.h>

int
main(int argc, char *argv[])
{
	int nbuf, fd, r;
	struct ifconf ifc;
	struct ifreq *bufs;
	struct ifreq ifreq;

	if (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <n_ifreq>", getprogname());

	nbuf = atoi(argv[1]);
	bufs = malloc(sizeof(ifreq) * nbuf);
	if (bufs == NULL)
		err(EXIT_FAILURE, "malloc(sizeof(ifreq) * %d)", bufs);

	if (rumpclient_init() == -1)
		err(EXIT_FAILURE, "rumpclient_init");

	fd = rump_sys_socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		err(EXIT_FAILURE, "rump_sys_socket");

	ifc.ifc_len = sizeof(ifreq) * nbuf;
	ifc.ifc_req = bufs;

	r = rump_sys_ioctl(fd, SIOCGIFCONF, &ifc);
	if (r == -1)
		err(EXIT_FAILURE, "rump_sys_ioctl");

	/* TODO */

	return EXIT_SUCCESS;
}
