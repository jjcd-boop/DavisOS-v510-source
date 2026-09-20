extern "C" __attribute__((section(".text.entry"))) int DavisMain(){
#if defined(__x86_64__)
 for(int i=0;i<8;i++) asm volatile("mov $2, %%rax; int $0x80" ::: "rax","memory");
 asm volatile("ud2"); // deliberate Ring-3 #UD for process-isolation proof
#endif
 return 0;
}
