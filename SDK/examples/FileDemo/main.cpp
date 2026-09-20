#include <davis/davis.hpp>
extern "C" __attribute__((section(".text.entry"))) int DavisMain(){
 using namespace DavisSDK; const char path[]="/apps/demo.txt"; const char msg[]="Hello from a Davis DXE application.\n";
 auto f=FileOpen(path,OpenWrite|OpenCreate|OpenTruncate); if(!f)return (int)f.error;
 auto w=FileWrite((Handle)f.value,msg,sizeof(msg)-1); FileClose((Handle)f.value); if(!w)return (int)w.error; return 0;
}
