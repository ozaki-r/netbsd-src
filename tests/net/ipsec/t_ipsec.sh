#	$NetBSD$
#
# Copyright (c) 2015 The NetBSD Foundation, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

inetserver="rump_server -lrumpnet -lrumpnet_net -lrumpnet_netinet -lrumpnet_netinet6"
inetserver="$inetserver -lrumpvfs -lrumpdev -lrumpdev_opencrypto"
inetserver="$inetserver -lrumpkern_z -lrumpkern_crypto -lrumpnet_netipsec"
inetserver="$inetserver -lrumpnet_netipsec"

SOCK1=unix://commsock1
atf_test_case basic cleanup

basic_head()
{
	atf_set "descr" "Does simple ipsec tests"
	atf_set "require.progs" "rump_server"
}

cleanup()
{
	env RUMP_SERVER=$SOCK1 rump.halt
}

basic_body()
{
	atf_check -s exit:0 ${inetserver} $SOCK1

	export RUMP_SERVER=$SOCK1
	atf_check -s exit:0 -o match:ipsec rump.sysctl -a
	atf_check -s exit:0 -o ignore rump.sysctl -w kern.cryptodevallowsoft=-1

	export LD_PRELOAD=/usr/lib/librumphijack.so

	# from https://www.netbsd.org/docs/network/ipsec/
	atf_check -s exit:0 -o ignore setkey -c <<-EOF
	add 10.1.1.1 20.1.1.1 esp 9876 -E 3des-cbc "hogehogehogehogehogehoge";
	add 20.1.1.1 10.1.1.1 esp 10000 -E 3des-cbc 0xdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef;
	spdadd 10.1.1.1 20.1.1.1 any -P out ipsec esp/transport//use;
	EOF

	unset LD_PRELOAD
}

basic_cleanup()
{
	cleanup
}

atf_init_test_cases()
{
	atf_add_test_case basic
}
