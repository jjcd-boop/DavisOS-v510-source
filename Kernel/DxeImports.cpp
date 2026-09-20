#include "DxeImports.hpp"
#include "Syscall.hpp"
namespace Davis::DxeImports {
Result ResolveSymbol(u32 s,u16 major,u16 minor,Resolution&o){o={};if(major!=1||minor>0)return Result::AbiMismatch;switch(s){
 case Exit:o.value=Syscall::Exit;break;case Log:o.value=Syscall::Log;break;case Yield:o.value=Syscall::Yield;break;
 case WindowCreate:o.value=Syscall::CreateWindow;o.requiredCapability=Dxe2::CapWindow;break;case WindowPresent:o.value=Syscall::PresentWindow;o.requiredCapability=Dxe2::CapWindow;break;
 case FileOpen:o.value=Syscall::OpenFile;o.requiredCapability=Dxe2::CapFileRead;break;case FileRead:o.value=Syscall::ReadFile;o.requiredCapability=Dxe2::CapFileRead;break;
 case FileWrite:o.value=Syscall::WriteFile;o.requiredCapability=Dxe2::CapFileWrite;break;case FileClose:o.value=Syscall::CloseFile;o.requiredCapability=Dxe2::CapFileRead;break;
 case ServiceLookup:o.value=Syscall::ServiceLookup;o.requiredCapability=Dxe2::CapIpc;break;case IpcCreate:o.value=Syscall::IpcCreate;o.requiredCapability=Dxe2::CapIpc;break;case IpcSend:o.value=Syscall::IpcSend;o.requiredCapability=Dxe2::CapIpc;break;case IpcReceive:o.value=Syscall::IpcReceive;o.requiredCapability=Dxe2::CapIpc;break;case IpcClose:o.value=Syscall::IpcClose;o.requiredCapability=Dxe2::CapIpc;break;
 default:return Result::UnknownSymbol;}return Result::Ok;}
Result Bind(const Dxe2::Header&h,const void*image,const Paging::Space&s){if(!h.importCount)return Result::Ok;const auto*im=(const Dxe2::Import*)((const u8*)image+h.importTableOffset);for(u32 i=0;i<h.importCount;i++){Resolution r{};auto rr=ResolveSymbol(im[i].symbol,im[i].abiMajor,im[i].abiMinor,r);if(rr!=Result::Ok)return rr;if(r.requiredCapability&&(h.capabilities&r.requiredCapability)!=r.requiredCapability)return Result::CapabilityDenied;u64 pa=0,pte=0;if(!Paging::Translate(s,im[i].patchVirtualAddress,&pa,&pte)||!(pte&Paging::U)||!(pte&Paging::W)||(pte&Paging::NX)==0)return Result::BadPatchAddress;*(volatile u64*)(uptr)pa=r.value;}return Result::Ok;}
}
