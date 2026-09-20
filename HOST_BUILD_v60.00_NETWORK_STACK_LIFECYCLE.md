# DavisOS v60.00 — Major Autonomous Network Stack & Lifecycle Pass

This milestone hardens TCP/IPv4 receive validation, DNS/DHCP trust boundaries, TCP socket lifecycle, browser socket reclamation, Forge network tuning, and release qualification.

## Runtime changes
- TCP now rejects corrupt IPv4 headers and fragmented IPv4 TCP datagrams until reassembly exists.
- TCP validates that inbound traffic is addressed to the configured local IPv4 address.
- TCP header-length validation is bounded to legal IPv4/TCP header sizes.
- DNS replies must originate from the configured resolver, have the response bit set, and carry a successful response code before acceptance.
- DHCP replies must carry the DHCP magic cookie and match the local client hardware address and transaction identity.
- TCP sockets now have an explicit Release lifecycle and can be safely reused.
- Browser session/resource paths reclaim TCP sockets instead of exhausting the fixed socket table over repeated navigation/resource fetches.

## Forge
- Forge System Builder version advanced to 60.00.
- Added bounded `kernel.tcp_max_sockets` transformation (4..64).
- Unsafe socket-capacity requests fail closed.

## Qualification
- Added network protocol hardening regression.
- Added 5,000-cycle TCP socket-table lifecycle stress (40,000 opens).
- Full host release gate: 43/43 PASS.
- VM boot remains NOT_RUN_NO_QEMU_AVAILABLE.
- Physical hardware qualification remains required.
