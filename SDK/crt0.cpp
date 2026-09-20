#include <davis/davis.hpp>
extern "C" int DavisMain();
extern "C" [[noreturn]] __attribute__((section(".text.entry"))) void _davis_start(){ DavisSDK::ProcessExit(DavisMain()); }
