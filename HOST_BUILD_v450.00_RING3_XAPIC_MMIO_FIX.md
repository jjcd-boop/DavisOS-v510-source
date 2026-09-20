# DavisOS v450.00 — Ring-3 xAPIC supervisor MMIO fix

Physical v440 diagnostics showed every desktop app at stage 5 (InterruptContract), READY=NO, with no fault and no scheduler dispatch. Audit found the common regression introduced by Ring-3 address-space isolation: xAPIC physical MMIO 0xFEE00000 lies numerically inside the lower canonical user range, while DeviceMappings used Paging::MapKernel4K, whose policy rejects all lower-half user-range virtual addresses. Therefore every per-app InterruptAddressSpace install failed before UserContext/Ready.

v450 adds Paging::MapSupervisor4K for explicit kernel-owned mappings that may live in the lower canonical half. It never sets the x86 U/S user bit. DeviceMappings now uses this primitive for supervisor-only MMIO. The xAPIC page remains writable/NX/supervisor-only and CPL3 user mapping into the same hierarchy is rejected. Normal MapKernel4K retains its original lower-half exclusion.

Desktop launch diagnostics were also corrected: an app with a process object but App.ready=false now reports preparation failure with stage/error instead of the misleading "prepared, window not created yet" notice.

A new host regression proves 0xFEE00000 can be installed into an application CR3 as identity-mapped supervisor MMIO while U=0, W=1, NX=1, and a user mapping attempt is rejected. Qualification also fixes an existing bookkeeping bug where the aggregate pass/fail value was computed before the final v390 gates were appended.

Physical storage writes remain disabled. Physical validation is still required to prove applications proceed from stage 5 through UserContext/Ready, scheduler dispatch, CPL3 entry, syscall/IPC, and window creation.
