#include "Uefi.hpp"
#include "../Kernel/BootInfo.hpp"
#include "../Base/Memory.hpp"
using namespace Davis;
static void Say(Uefi::SystemTable*st,Uefi::Char16*m){if(st&&st->conOut&&st->conOut->outputString)st->conOut->outputString(st->conOut,m);}
static void SayAscii(Uefi::SystemTable*st,const char*s){static Uefi::Char16 b[192];usize n=0;while(s&&s[n]&&n<190){b[n]=(Uefi::Char16)(u8)s[n];n++;}b[n]=0;Say(st,b);}
static void SayHex(Uefi::SystemTable*st,u64 v){static const char*h="0123456789ABCDEF";char b[19];b[0]='0';b[1]='x';for(int i=0;i<16;i++)b[2+i]=h[(v>>((15-i)*4))&15];b[18]=0;SayAscii(st,b);}
static void StatusLine(Uefi::SystemTable*st,const char*name,Uefi::Status v){SayAscii(st,name);SayAscii(st," = ");SayHex(st,v);SayAscii(st,v==Uefi::Success?"  OK\r\n":"  FAIL\r\n");}
static bool EqName(const Uefi::Char16*a,const char*b){usize i=0;for(;b[i];i++)if(a[i]!=(Uefi::Char16)(u8)b[i])return false;return a[i]==0;}
static bool HasDxeSuffix(const Uefi::Char16*s){usize n=0;while(s[n]&&n<240)n++;if(n<4)return false;auto lo=[](Uefi::Char16 c){return(c>='A'&&c<='Z')?Uefi::Char16(c+32):c;};return lo(s[n-4])=='.'&&lo(s[n-3])=='d'&&lo(s[n-2])=='x'&&lo(s[n-1])=='e';}
static u32 KindFor(const Uefi::Char16*n){
 if(EqName(n,"HelloDavis.dxe"))return AppHello;if(EqName(n,"PreemptionA.dxe"))return AppPreemptionA;if(EqName(n,"PreemptionB.dxe"))return AppPreemptionB;
 if(EqName(n,"AudioDriver.dxe"))return AppDriverAudio;if(EqName(n,"NetworkDriver.dxe"))return AppDriverNetwork;if(EqName(n,"StorageDriver.dxe"))return AppDriverStorage;
 if(EqName(n,"UsbDriver.dxe"))return AppDriverUsb;if(EqName(n,"InputDriver.dxe"))return AppDriverInput;if(EqName(n,"GraphicsDriver.dxe"))return AppDriverGraphics;if(EqName(n,"WifiDriver.dxe"))return AppDriverWifi;
 if(EqName(n,"Writer.dxe"))return AppWriter;if(EqName(n,"Calculator.dxe"))return AppCalculator;if(EqName(n,"Files.dxe"))return AppFiles;if(EqName(n,"Terminal.dxe"))return AppTerminal;if(EqName(n,"Settings.dxe"))return AppSettings;if(EqName(n,"Browser.dxe"))return AppBrowser;if(EqName(n,"Media.dxe"))return AppMedia;if(EqName(n,"Photos.dxe"))return AppPhotos;if(EqName(n,"SkyDefender.dxe"))return AppSkyDefender;if(EqName(n,"Solitaire.dxe"))return AppSolitaire;if(EqName(n,"Chess.dxe"))return AppChess;if(EqName(n,"Checkers.dxe"))return AppCheckers;if(EqName(n,"Causeway.dxe"))return AppCauseway;if(EqName(n,"Pebble.dxe"))return AppPebble;if(EqName(n,"Morrow.dxe"))return AppMorrow;if(EqName(n,"Forge.dxe"))return AppForge;if(EqName(n,"ForgeRunner.dxe"))return AppForgeRunner;return AppGeneric;}
#pragma pack(push,1)
struct EfiFileInfo { u64 size,fileSize,physicalSize; u64 createTime[2],lastAccessTime[2],modificationTime[2]; u64 attribute; Uefi::Char16 fileName[1]; };
#pragma pack(pop)

static Uefi::Char16 Lower16(Uefi::Char16 c){return(c>='A'&&c<='Z')?(Uefi::Char16)(c+32):c;}
static bool HasExt(const Uefi::Char16*s,const char*e){usize n=0,m=0;while(s[n]&&n<240)n++;while(e[m])m++;if(n<m)return false;for(usize i=0;i<m;i++)if(Lower16(s[n-m+i])!=(Uefi::Char16)(u8)e[i])return false;return true;}
static u32 MediaKindFor(const Uefi::Char16*n){if(HasExt(n,".wav")||HasExt(n,".mp3")||HasExt(n,".ogg")||HasExt(n,".mid")||HasExt(n,".midi")||HasExt(n,".flac"))return MediaAudio;if(HasExt(n,".bmp")||HasExt(n,".png")||HasExt(n,".gif")||HasExt(n,".jpg")||HasExt(n,".jpeg"))return MediaImage;return MediaOther;}
static void CopyMediaPath(char*out,const char*prefix,const Uefi::Char16*name){usize z=0;for(;prefix&&prefix[z]&&z<126;z++)out[z]=prefix[z];if(z&&out[z-1]!='/')out[z++]='/';for(usize i=0;name[i]&&z<127;i++){Uefi::Char16 c=name[i];out[z++]=(char)(c<128?c:'?');}out[z]=0;}
static void ScanMediaDir(Uefi::BootServices*bs,Uefi::FileProtocol*dir,const char*prefix,BootInfo&bi,u32 depth){if(!dir||bi.mediaCount>=MaxBootMedia)return;alignas(8) u8 infoBuf[1024];while(bi.mediaCount<MaxBootMedia){usize got=sizeof(infoBuf);auto rs=dir->read(dir,&got,infoBuf);if(rs!=Uefi::Success||got==0)break;auto*fi=(EfiFileInfo*)infoBuf;if(!fi->fileName[0]||EqName(fi->fileName,".")||EqName(fi->fileName,".."))continue;if(fi->attribute&0x10){if(depth<2&&!EqName(fi->fileName,"EFI")){Uefi::FileProtocol*sub=nullptr;if(dir->open(dir,&sub,fi->fileName,1,0)==Uefi::Success&&sub){char np[128];CopyMediaPath(np,prefix,fi->fileName);ScanMediaDir(bs,sub,np,bi,depth+1);sub->close(sub);}}continue;}u32 kind=MediaKindFor(fi->fileName);if(kind==MediaOther||!fi->fileSize||fi->fileSize>1024*1024)continue;Uefi::FileProtocol*f=nullptr;if(dir->open(dir,&f,fi->fileName,1,0)!=Uefi::Success||!f)continue;usize pages=(usize)((fi->fileSize+4095)/4096);u64 a=0;if(bs->allocatePages(Uefi::AllocateAnyPages,Uefi::LoaderData,pages,&a)==Uefi::Success){usize n=(usize)fi->fileSize;if(f->read(f,&n,(void*)(uptr)a)==Uefi::Success&&n){auto&slot=bi.media[bi.mediaCount++];slot.base=(uptr)a;slot.bytes=n;slot.kind=kind;CopyMediaPath(slot.path,prefix,fi->fileName);}}f->close(f);}}


struct KnownApp { const char* ascii; u32 kind; };
static const KnownApp KnownApps[] = {
 // v410: deterministic user-application-first staging. A failing optional driver must
 // never prevent the desktop applications from reaching the kernel boot manifest.
 {"Writer.dxe",AppWriter},{"Calculator.dxe",AppCalculator},{"Files.dxe",AppFiles},{"Terminal.dxe",AppTerminal},{"Settings.dxe",AppSettings},{"Browser.dxe",AppBrowser},{"Media.dxe",AppMedia},{"Photos.dxe",AppPhotos},{"SkyDefender.dxe",AppSkyDefender},{"Solitaire.dxe",AppSolitaire},{"Chess.dxe",AppChess},{"Checkers.dxe",AppCheckers},{"Causeway.dxe",AppCauseway},{"Pebble.dxe",AppPebble},{"Morrow.dxe",AppMorrow},{"Forge.dxe",AppForge},{"ForgeRunner.dxe",AppForgeRunner},
 {"HelloDavis.dxe",AppHello},{"PreemptionA.dxe",AppPreemptionA},{"PreemptionB.dxe",AppPreemptionB},
 {"AudioDriver.dxe",AppDriverAudio},{"NetworkDriver.dxe",AppDriverNetwork},{"StorageDriver.dxe",AppDriverStorage},{"UsbDriver.dxe",AppDriverUsb},{"InputDriver.dxe",AppDriverInput},{"GraphicsDriver.dxe",AppDriverGraphics},{"WifiDriver.dxe",AppDriverWifi}
};
static bool AlreadyLoaded(const BootInfo&bi,u32 kind){for(usize i=0;i<bi.appCount;i++)if(bi.apps[i].kind==kind)return true;return false;}
static bool OpenKnownApp(Uefi::BootServices*bs,Uefi::FileProtocol*dir,const KnownApp&ka,BootInfo&bi){
 if(AlreadyLoaded(bi,ka.kind)||bi.appCount>=MaxBootApps)return true;Uefi::Char16 name[64];for(usize q=0;q<64;q++)name[q]=0;usize z=0;while(ka.ascii[z]&&z<63){name[z]=(Uefi::Char16)(u8)ka.ascii[z];z++;}
 Uefi::FileProtocol*app=nullptr;if(dir->open(dir,&app,name,1,0)!=Uefi::Success||!app)return false;constexpr usize AppMax=256*1024;u64 a=0;bool ok=false;
 if(bs->allocatePages(Uefi::AllocateAnyPages,Uefi::LoaderData,AppMax/4096,&a)==Uefi::Success){usize n=AppMax;if(app->read(app,&n,(void*)(uptr)a)==Uefi::Success&&n){auto&slot=bi.apps[bi.appCount++];slot.base=(uptr)a;slot.bytes=n;slot.kind=ka.kind;if(ka.kind==AppHello){bi.appBase=slot.base;bi.appBytes=slot.bytes;}ok=true;}}
 app->close(app);return ok;
}


#pragma pack(push,1)
struct CandidateMarker { u32 magic,stageDigest; };
struct CandidateState { u32 magic,stageDigest,state,checkpoint; };
#pragma pack(pop)
static constexpr u32 CandidateMarkerMagic=0x47424344; // DCBG
static constexpr u32 CandidateStateMagic=0x54424344; // DCBT
static Uefi::Guid CandidateGuid={0x6f8d7a31,0x9b5e,0x4c31,{0xa4,0x73,0x44,0x41,0x56,0x49,0x53,0x23}};
static Uefi::Char16 CandidateVar[]={'D','a','v','i','s','C','a','n','d','i','d','a','t','e','B','o','o','t',0};
static bool ReadCandidateMarker(Uefi::FileProtocol*root,CandidateMarker&out){
 static Uefi::Char16 p[]={'\\','D','A','V','I','S','\\','C','A','N','D','I','D','A','T','E','\\','C','A','N','D','I','D','A','T','E','.','S','T','A','G','E',0};
 Uefi::FileProtocol*f=nullptr;if(root->open(root,&f,p,1,0)!=Uefi::Success||!f)return false;usize n=sizeof(out);auto r=f->read(f,&n,&out);f->close(f);return r==Uefi::Success&&n==sizeof(out)&&out.magic==CandidateMarkerMagic&&out.stageDigest;
}
static bool CandidateShouldBoot(Uefi::SystemTable*st,const CandidateMarker&m){
 if(!st||!st->runtimeServices||!st->runtimeServices->getVariable||!st->runtimeServices->setVariable)return false;
 CandidateState q{};usize n=sizeof(q);u32 attrs=0;auto r=st->runtimeServices->getVariable(CandidateVar,&CandidateGuid,&attrs,&n,&q);
 if(r==Uefi::Success&&n==sizeof(q)&&q.magic==CandidateStateMagic&&q.stageDigest==m.stageDigest&&q.state!=0)return false;
 CandidateState a{CandidateStateMagic,m.stageDigest,1,0};constexpr u32 va=1|2|4;
 return st->runtimeServices->setVariable(CandidateVar,&CandidateGuid,va,sizeof(a),&a)==Uefi::Success;
}


static constexpr Uefi::Status NotReady=0x8000000000000006ULL;
enum class RecoveryChoice:u32{KnownGood=0,Recovery=1,Candidate=2};
static const char* CheckpointName(u32 c){switch(c){case 1:return "KERNEL_ENTRY";case 2:return "MEMORY_READY";case 3:return "RUNTIME_READY";case 4:return "HARDWARE_READY";case 5:return "INPUT_READY";case 6:return "DESKTOP_READY";case 7:return "RING3_READY";case 8:return "HEALTHY";default:return "NONE";}}
static bool CandidateAvailable(Uefi::SystemTable*st,const CandidateMarker&m){
 if(!st||!st->runtimeServices||!st->runtimeServices->getVariable)return false; CandidateState q{};usize n=sizeof(q);u32 attrs=0;auto r=st->runtimeServices->getVariable(CandidateVar,&CandidateGuid,&attrs,&n,&q); return !(r==Uefi::Success&&n==sizeof(q)&&q.magic==CandidateStateMagic&&q.stageDigest==m.stageDigest&&q.state!=0);
}
static RecoveryChoice RecoveryCenter(Uefi::SystemTable*st,bool candidateAvailable){
 SayAscii(st,"\r\n=== DAVIS RECOVERY CENTER v26 ===\r\n");
 if(st&&st->runtimeServices&&st->runtimeServices->getVariable){CandidateState q{};usize n=sizeof(q);u32 attrs=0;auto r=st->runtimeServices->getVariable(CandidateVar,&CandidateGuid,&attrs,&n,&q);if(r==Uefi::Success&&n==sizeof(q)&&q.magic==CandidateStateMagic){SayAscii(st,"Last candidate checkpoint: ");SayAscii(st,CheckpointName(q.checkpoint));SayAscii(st,q.state==2?" [HEALTHY]\r\n":" [INCOMPLETE]\r\n");}}
 SayAscii(st,"K = boot current known-good\r\nR = boot protected recovery generation\r\n");if(candidateAvailable)SayAscii(st,"C = one-shot Forge candidate test\r\n");
 SayAscii(st,"Automatic choice in ~2 seconds: known-good\r\n");
 if(!st||!st->conIn||!st->conIn->readKeyStroke)return RecoveryChoice::KnownGood;
 auto stall=(Uefi::Status(*)(usize))st->bootServices->stall;
 for(u32 i=0;i<200;i++){Uefi::InputKey k{};auto r=st->conIn->readKeyStroke(st->conIn,&k);if(r==Uefi::Success){auto c=k.unicodeChar;if(c=='r'||c=='R'){SayAscii(st,"Recovery generation selected.\r\n");return RecoveryChoice::Recovery;}if(c=='c'||c=='C'){if(candidateAvailable){SayAscii(st,"Candidate test selected.\r\n");return RecoveryChoice::Candidate;}}if(c=='k'||c=='K'||c=='\r'){SayAscii(st,"Known-good selected.\r\n");return RecoveryChoice::KnownGood;}}if(stall)stall(10000);}
 return RecoveryChoice::KnownGood;
}
extern "C" Uefi::Status efi_main(Uefi::Handle image,Uefi::SystemTable*st){
 static Uefi::Char16 start[]={'D','a','v','i','s','B','o','o','t',' ','D','e','s','k','t','o','p',' ','v','1','.','5','3','\r','\n',0};
 static Uefi::Char16 path[]={'\\','D','A','V','I','S','\\','D','A','V','I','S','K','R','N','.','B','I','N',0};
 static Uefi::Char16 candidatePath[]={'\\','D','A','V','I','S','\\','C','A','N','D','I','D','A','T','E','\\','D','A','V','I','S','K','R','N','.','B','I','N',0};
 static Uefi::Char16 recoveryPath[]={'\\','D','A','V','I','S','\\','R','E','C','O','V','E','R','Y','\\','K','N','O','W','N','_','G','O','O','D','_','v','2','5','\\','S','Y','S','T','E','M','\\','D','A','V','I','S','\\','D','A','V','I','S','K','R','N','.','B','I','N',0};
 static Uefi::Char16 recoveryAppsPath[]={'\\','D','A','V','I','S','\\','R','E','C','O','V','E','R','Y','\\','K','N','O','W','N','_','G','O','O','D','_','v','2','5','\\','S','Y','S','T','E','M','\\','D','A','V','I','S','\\','A','P','P','S',0};
 static Uefi::Char16 appsPath[]={'\\','D','A','V','I','S','\\','A','P','P','S',0};
 static Uefi::Char16 livePath[]={'\\','D','A','V','I','S','\\','E','N','A','B','L','E','_','L','I','V','E','_','P','R','O','O','F','.','T','X','T',0};
 static Uefi::Char16 diagnosticPath[]={'\\','D','A','V','I','S','\\','D','I','A','G','N','O','S','T','I','C','_','B','O','O','T','.','T','X','T',0};
 static Uefi::Char16 safePath[]={'\\','D','A','V','I','S','\\','S','A','F','E','_','B','O','O','T','.','T','X','T',0};
 static Uefi::Char16 probePath[]={'\\','D','A','V','I','S','\\','H','A','R','D','W','A','R','E','_','P','R','O','B','E','.','T','X','T',0};
 static Uefi::Char16 allowWritesPath[]={'\\','D','A','V','I','S','\\','A','L','L','O','W','_','P','H','Y','S','I','C','A','L','_','W','R','I','T','E','S','.','T','X','T',0};
 static Uefi::Char16 dumpEnablePath[]={'\\','D','A','V','I','S','\\','E','X','T','E','N','S','I','V','E','_','D','U','M','P','.','T','X','T',0};
 static Uefi::Char16 dumpFilePath[]={'\\','D','A','V','I','S','\\','L','A','S','T','D','U','M','P','.','T','X','T',0};
 static Uefi::Char16 writeTestPath[]={'\\','D','A','V','I','S','\\','W','R','I','T','E','_','T','E','S','T','.','T','X','T',0};
 static Uefi::Char16 dumpVarName[]={'D','a','v','i','s','D','i','a','g','D','u','m','p',0};
 static Uefi::Char16 disablePath[]={'\\','D','A','V','I','S','\\','D','I','S','A','B','L','E','_','D','X','E','.','T','X','T',0};
 static Uefi::Char16 fail[]={'D','a','v','i','s','B','o','o','t',' ','F','A','I','L','\r','\n',0};
 Say(st,start);if(!st||!st->bootServices)return 1;auto*bs=st->bootServices;SayAscii(st,"[01] BOOT SERVICES OK\r\n");
 SayAscii(st,"[02] LOADED IMAGE...\r\n"); Uefi::LoadedImageProtocol*li=nullptr;if(bs->handleProtocol(image,&Uefi::LoadedImageGuid,(void**)&li)!=Uefi::Success||!li){Say(st,fail);return 2;}
 SayAscii(st,"[03] FILESYSTEM...\r\n"); Uefi::SimpleFileSystemProtocol*fs=nullptr;if(bs->handleProtocol(li->deviceHandle,&Uefi::SimpleFsGuid,(void**)&fs)!=Uefi::Success||!fs){Say(st,fail);return 3;}
 SayAscii(st,"[04] OPEN VOLUME...\r\n"); Uefi::FileProtocol*root=nullptr,*file=nullptr;if(fs->openVolume(fs,&root)!=Uefi::Success||!root){Say(st,fail);return 4;}
 // v1.47: obsolete synchronous UEFI write/NVRAM diagnostics removed from normal boot.
 CandidateMarker cm{};bool candidateMarker=ReadCandidateMarker(root,cm);bool candidateAvailable=candidateMarker&&CandidateAvailable(st,cm);RecoveryChoice recoveryChoice=RecoveryCenter(st,candidateAvailable);bool recovery=(recoveryChoice==RecoveryChoice::Recovery);bool candidate=(recoveryChoice==RecoveryChoice::Candidate)&&candidateAvailable&&CandidateShouldBoot(st,cm); SayAscii(st,recovery?"[05] OPEN PROTECTED RECOVERY KERNEL...\r\n":(candidate?"[05] OPEN CANDIDATE KERNEL...\r\n":"[05] OPEN KNOWN-GOOD KERNEL...\r\n")); Uefi::Char16*selectedPath=recovery?recoveryPath:(candidate?candidatePath:path); if(root->open(root,&file,selectedPath,1,0)!=Uefi::Success||!file){ if(recovery){SayAscii(st,"RECOVERY IMAGE UNAVAILABLE; FALLING BACK TO KNOWN-GOOD.\r\n");recovery=false;} if(candidate){candidate=false;} if(root->open(root,&file,path,1,0)!=Uefi::Success||!file){Say(st,fail);return 5;} }
 SayAscii(st,"[06] KERNEL OPEN OK\r\n"); constexpr u64 KernelBase=0x02000000ULL;constexpr usize KernelMax=64*1024*1024;u64 addr=KernelBase;if(bs->allocatePages(Uefi::AllocateAddress,Uefi::LoaderData,KernelMax/4096,&addr)!=Uefi::Success){Say(st,fail);return 6;}
 SayAscii(st,"[07] KERNEL MEMORY OK\r\n"); usize bytes=KernelMax;if(file->read(file,&bytes,(void*)KernelBase)!=Uefi::Success||!bytes){Say(st,fail);return 7;} /* Raw kernel BIN omits .bss. The linked image currently reserves tens of MiB of BSS (notably compositor surfaces), so explicitly zero the remainder of the reserved kernel aperture before ExitBootServices. */ for(usize z=bytes;z<KernelMax;z++)((volatile u8*)(uptr)KernelBase)[z]=0; file->close(file);SayAscii(st,"[08] KERNEL LOAD+BSS ZERO OK\r\n");static BootInfo bi{};bi.candidateBoot=candidate;bi.candidateStageDigest=candidate?cm.stageDigest:0;
 // v1.51 recovery: recursive media preload is disabled during normal boot.
 bi.mediaCount=0;
 Uefi::FileProtocol*disable=nullptr;if(root->open(root,&disable,disablePath,1,0)==Uefi::Success&&disable){bi.appLaunchDisabled=true;disable->close(disable);}Uefi::FileProtocol*live=nullptr;if(root->open(root,&live,livePath,1,0)==Uefi::Success&&live){bi.liveProofRequested=true;live->close(live);}Uefi::FileProtocol*diag=nullptr;if(root->open(root,&diag,diagnosticPath,1,0)==Uefi::Success&&diag){bi.diagnosticBootRequested=true;diag->close(diag);}Uefi::FileProtocol*safe=nullptr;if(root->open(root,&safe,safePath,1,0)==Uefi::Success&&safe){bi.safeBootRequested=true;safe->close(safe);}Uefi::FileProtocol*probe=nullptr;if(root->open(root,&probe,probePath,1,0)==Uefi::Success&&probe){bi.hardwareProbeRequested=true;probe->close(probe);}bi.physicalStorageReadOnly=true;Uefi::FileProtocol*writes=nullptr;if(root->open(root,&writes,allowWritesPath,1,0)==Uefi::Success&&writes){bi.physicalStorageReadOnly=false;writes->close(writes);}Uefi::FileProtocol*dumpEnable=nullptr;if(root->open(root,&dumpEnable,dumpEnablePath,1,0)==Uefi::Success&&dumpEnable){bi.extensiveDumpRequested=true;dumpEnable->close(dumpEnable);}bi.runtimeServices=(uptr)st->runtimeServices;
 if(!bi.appLaunchDisabled){
  SayAscii(st,"[09] DETERMINISTIC DXE MANIFEST...\r\n"); Uefi::FileProtocol*dir=nullptr;Uefi::Char16*selectedAppsPath=recovery?recoveryAppsPath:appsPath;
  if(root->open(root,&dir,selectedAppsPath,1,0)==Uefi::Success&&dir){
   for(usize k=0;k<sizeof(KnownApps)/sizeof(KnownApps[0])&&bi.appCount<MaxBootApps;k++)OpenKnownApp(bs,dir,KnownApps[k],bi);
   dir->close(dir);
  }
  SayAscii(st,"     staged apps = ");SayHex(st,bi.appCount);SayAscii(st,"\r\n");
 }SayAscii(st,"[10] OPTIONAL FLAGS OK\r\n");root->close(root);
 SayAscii(st,"[11] GOP...\r\n"); Uefi::GraphicsOutputProtocol*gop=nullptr;if(bs->locateProtocol(&Uefi::GopGuid,nullptr,(void**)&gop)!=Uefi::Success||!gop||!gop->mode||!gop->mode->info){Say(st,fail);return 8;}
 SayAscii(st,"[12] GOP OK\r\n"); bi.framebuffer.base=(uptr)gop->mode->frameBufferBase;bi.framebuffer.width=gop->mode->info->horizontalResolution;bi.framebuffer.height=gop->mode->info->verticalResolution;bi.framebuffer.pixelsPerScanLine=gop->mode->info->pixelsPerScanLine;bi.framebuffer.pixelFormat=gop->mode->info->pixelFormat;
 for(usize i=0;i<st->numberOfTableEntries;i++){auto&c=st->configurationTable[i];if(Uefi::Eq(c.vendorGuid,Uefi::Acpi20Guid)||Uefi::Eq(c.vendorGuid,Uefi::Acpi10Guid)){bi.rsdp=(uptr)c.vendorTable;break;}}
 SayAscii(st,"[13] ACPI TABLE PASS OK\r\n"); SayAscii(st,"[14] FINAL MEMORY MAP...\r\n"); usize mapBytes=0,key=0,desc=0;u32 ver=0;bs->getMemoryMap(&mapBytes,nullptr,&key,&desc,&ver);mapBytes+=desc*8;void*map=nullptr;if(bs->allocatePool(Uefi::LoaderData,mapBytes,&map)!=Uefi::Success){Say(st,fail);return 9;}SayAscii(st,"[15] EXIT BOOT SERVICES...\r\n"); Uefi::Status status=1;for(int attempt=0;attempt<4;attempt++){usize n=mapBytes;if(bs->getMemoryMap(&n,map,&key,&desc,&ver)!=Uefi::Success)continue;bi.memoryMap=(uptr)map;bi.memoryMapBytes=n;bi.descriptorSize=desc;bi.descriptorVersion=ver;status=bs->exitBootServices(image,key);if(status==Uefi::Success)break;}if(status!=Uefi::Success)return 10; /* Post-EBS: firmware console is invalid. Mark framebuffer directly. */ if(bi.framebuffer.base&&bi.framebuffer.width&&bi.framebuffer.height){u32*fb=(u32*)(uptr)bi.framebuffer.base;u32 w=bi.framebuffer.width<320?bi.framebuffer.width:320;u32 h=bi.framebuffer.height<10?bi.framebuffer.height:10;for(u32 y=0;y<h;y++)for(u32 x=0;x<w;x++)fb[(usize)y*bi.framebuffer.pixelsPerScanLine+x]=0x0000FF00;} using KernelEntry=void(__attribute__((ms_abi)) *)(BootInfo*);((KernelEntry)KernelBase)(&bi);for(;;){}
}
extern "C" void* memcpy(void*d,const void*s,unsigned long n){auto*a=(unsigned char*)d;auto*b=(const unsigned char*)s;while(n--)*a++=*b++;return d;}
extern "C" void* memset(void*d,int v,unsigned long n){auto*a=(unsigned char*)d;while(n--)*a++=(unsigned char)v;return d;}
