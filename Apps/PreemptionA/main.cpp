extern "C" __attribute__((section(".text.entry"))) int DavisMain(){
#if defined(__x86_64__)
 for(;;) asm volatile("mov $2, %%rax; int $0x80" ::: "rax","memory");
#endif
 return 0;
}
