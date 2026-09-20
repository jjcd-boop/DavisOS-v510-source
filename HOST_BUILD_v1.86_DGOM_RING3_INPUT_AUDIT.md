# Davis OS v1.86 — DGOM + Ring-3 input integration + audit follow-through

- Replaces the 24 x 640 x 480 static window pixel arrays with Davis Graphics Object Manager (DGOM) metadata plus page-backed objects allocated on demand.
- DGOM v1: 64 object metadata slots, 16 MiB per-process graphics quota, 64 MiB global quota, exact page allocation, resize/reallocation with preserved overlap, generation counter, ownership, rights metadata, and automatic process teardown reclamation.
- PresentWindow copies validated user pixels into the owning DGOM object. This is intentionally not zero-copy/shared mapping yet.
- Maximum DXE surface raised to 1280x720 without multiplying kernel BSS by the maximum dimensions.
- Ring-3 InputDriver forwards validated boot HID reports to the DesktopCompositor service over bounded IPC.
- DesktopService accepts injection only from the registered InputDriver service owner, applies mouse movement/buttons to the desktop input state, and translates newly-pressed boot keyboard usages into focused DXE key events.
- Protected legacy Input.cpp and Ps2Diagnostics.cpp are not edited.
- v1.85 boot loader 64 MiB aperture/BSS-zeroing recovery remains unchanged.

Audit status: desktop/compositor remains Ring 0 compatibility code; DGOM backing is kernel-owned system RAM; no shared user/compositor mappings, GPU objects, fences, VRAM placement, or IOMMU yet. USB xHCI Ring-3 path remains single-device/root-port oriented and hardware validation is still required. Wi-Fi/audio/storage/network/graphics physical migration remains incomplete.
