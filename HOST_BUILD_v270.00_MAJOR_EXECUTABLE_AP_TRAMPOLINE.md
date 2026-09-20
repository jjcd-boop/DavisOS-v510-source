# Davis OS v270.00 — Executable AP Trampoline

This milestone replaces the v260 fail-stop AP image prefix with an x86 bootstrap assembled as real machine code.

The trampoline begins in 16-bit real mode after SIPI, derives its low-memory physical base from CS, patches and loads a local GDT, enters 32-bit protected mode, loads the handoff CR3, enables PAE and IA32_EFER.LME, enables paging, performs a far transfer into the 64-bit code segment, installs the handoff stack, and jumps to the kernel AP entry with the handoff pointer in RDI.

The image builder copies the linked trampoline into the SIPI page and places the authenticated handoff at offset 512. Validation checks the complete trampoline bytes and handoff before startup. The blob is deliberately bounded below the handoff offset.

Host qualification compiles the same assembly that is linked into the kernel and performs 100,000 image-build/validation cycles, including deliberate code corruption rejection.

## Hardware boundary
This is executable x86 transition code, but host qualification does not prove a physical AP successfully executed it. Real-machine qualification still requires LAPIC delivery, identity mapping of the low trampoline, valid AP-visible CR3 mappings, architectural rendezvous, and observation of secondary CPUs entering the Davis scheduler.
