# Davis dxe-pack v1.06
`dxe_pack.py` creates a strict DXE2 image from flat code and optional flat data payloads. It writes the DXE2 header and segment table, page-aligned virtual addresses, W^X permissions, stack contract, app identity/version and capability mask. Relocations and dynamic imports remain intentionally unsupported by this first packager; SDK wrappers are statically compiled into the application.

Example:
`python dxe_pack.py --code app.bin -o App.dxe --app-id 0x1001 --version 1.0 --cap file-read --cap file-write`
