# In-kernel implementation of WireGuard for NetBSD

## How to build

1. clone the repository and checkout `wireguard` branch,
2. add the following options to your kernel config, and
3. build your kenrel, `wgconfig` and `wg-keygen`.

### kernel options

    options         LIBSODIUM
    options         BLAKE2S
    pseudo-device   wg

## How to use

1. create and setup a WireGuard interface,
2. generate a pair of private and pubkey keys by using `wg-keygen`,
3. configure the interface by using `wgconfig`, and
4. configure a peer on the interface by using `wgconfig`.

The following intsructions describe a basic usage of the implementation.

    # Create and setup an interface
    ifconfig wg0 create
    ifconfig wg0 inet 10.0.0.1/24
    route add -net 10.0.0.0/24 -link wg0 -iface
    
    # Generate a pair of private and pubkey keys
    wg-keygen > ./privkey
    cat ./privkey | wg-keygen --pub > ./pubkey
    
    # Configure the interface
    wgconfig wg0 set private-key ./privkey
    wgconfig wg0 set listen-port 52428
    
    # Add a peer
    wgconfig wg0 add peer peer0 "2iWFzywbDvYu2gQW5Q7/z/g5/Cv4bDDd6L3OKXLOwxs=" --endpoint=192.168,0.2:52428 --allowed-ips=10.0.0.0/24
    
    # Try to send packets to the peer
    ping 10.0.0.2
    
    # Delete the peer
    wgconfig wg0 delete peer peer0

Note that you can omit `--endpoint=` option if the instance doesn't know
peer's endpoints in advance, i.e., it acts as a server.  In that case,
the instance can establish a secure session only if a peer, which acts
a client, starts handshake of a session to the instance.

## CAVEATS

- The implementation is heavily under development and should not be used it in production systems
- The `wireguard` branch will be rebased sometimes to track the tip of the NetBSD repository

## For developers

### Debug options

There are four debug options:
- `WG_DEBUG_LOG` outputs debug logs
- `WG_DEBUG_TRACE` outputs trace logs
- `WG_DEBUG_DUMP` outputs hash values, etc.
- `WG_DEBUG_PARAMS` makes some internal parameters configurable for testing and debugging

You can enable all options at once by `WG_DEBUG`.

### ATF tests

There is a bunch of ATF tests.  Build and install rump libraries and tests, then run:

    cd /usr/tests; atf-run net/wireguard | atf-report

If set `ATF_WIREGUARD_INTEROPERABILITY=yes`, `t_interoperability` tests
with a peer outside of rump kernels.  See `tests/net/wireguard/t_interoperability.sh`
to know how to set up the peer.

## Links

- [NetBSD](http://www.netbsd.org/)
- [WireGuard](https://www.wireguard.com)

## Author

- Ryota Ozaki
