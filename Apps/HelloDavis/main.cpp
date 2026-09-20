extern "C" __attribute__((section(".text.entry"))) int DavisMain(){
#if defined(__x86_64__)
  asm volatile("mov $2, %%rax; int $0x80" ::: "rax","memory");
  asm volatile("mov $0, %%rax; mov $42, %%rdi; int $0x80" ::: "rax","rdi","memory");
#endif
  return 42;
}
