# elf2dxe (Davis OS v1.07)
Converts a linked x86-64 little-endian ELF application into a DXE2 image by reading PT_LOAD program headers. It preserves virtual addresses, entry point, R/W/X permissions, and BSS size while repacking file payloads into the DXE container. W+X loads are rejected.

Example:
`python elf2dxe.py FileDemo.elf -o FileDemo.dxe --app-id 0x1001 --cap file-read --cap file-write`

The input ELF should be linked with a Davis application linker script that page-aligns PT_LOAD virtual addresses. Relocations and shared-library dependencies remain unsupported.
