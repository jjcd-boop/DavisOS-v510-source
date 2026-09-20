# Davis SDK v1.05
Freestanding C++ application-facing wrappers for the Davis DXE syscall ABI.

Include `include/davis/davis.hpp`. Applications no longer need inline assembly or hard-coded register conventions. The SDK currently wraps Exit, Yield, Log, Window Create/Present, and File Open/Read/Write/Close.

This is the first source SDK. DXE2 packaging/import generation remains a separate next step; capability declarations are still part of the DXE2 image metadata and the kernel remains authoritative.
