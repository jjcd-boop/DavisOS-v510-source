# DavisOS v70.00 — Major Network Routing Qualification

This milestone adds a deterministic IPv4 route-selection primitive to the kernel network core. Local-subnet destinations resolve directly; off-subnet destinations resolve through the configured default gateway; unconfigured networking, zero destinations, and missing gateways fail closed.

A native host regression test exercises configured/unconfigured behavior, subnet routing, gateway routing, zero-address rejection, missing-gateway rejection, and 9,999 fail-closed route decisions. The complete v70 host qualification gate passes 45/45 checks.

The canonical kernel is rebuilt by the qualification gate. VM boot remains NOT_RUN_NO_QEMU_AVAILABLE and physical-hardware qualification remains required.
