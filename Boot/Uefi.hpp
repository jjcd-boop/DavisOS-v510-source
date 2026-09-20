#pragma once
#include "../Base/Types.hpp"
namespace Davis::Uefi {
using Status=u64; using Handle=void*; using Char16=u16; using Event=void*;
constexpr Status Success=0; constexpr Status BufferTooSmall=0x8000000000000005ULL;
struct Guid{u32 a;u16 b,c;u8 d[8];};
struct TableHeader{u64 signature;u32 revision,headerSize,crc32,reserved;};
struct SimpleTextOutput; struct SimpleTextInput; struct BootServices; struct RuntimeServices;
struct ConfigurationTable{Guid vendorGuid;void*vendorTable;};
struct SystemTable{TableHeader hdr;Char16*firmwareVendor;u32 firmwareRevision;Handle consoleInHandle;SimpleTextInput*conIn;Handle consoleOutHandle;SimpleTextOutput*conOut;Handle standardErrorHandle;SimpleTextOutput*stdErr;RuntimeServices*runtimeServices;BootServices*bootServices;usize numberOfTableEntries;ConfigurationTable*configurationTable;};
struct InputKey{u16 scanCode;Char16 unicodeChar;};
struct SimpleTextInput{Status(*reset)(SimpleTextInput*,bool);Status(*readKeyStroke)(SimpleTextInput*,InputKey*);Event waitForKey;};
struct SimpleTextOutput{void*reset;Status(*outputString)(SimpleTextOutput*,Char16*);};
struct RuntimeServices{TableHeader hdr;void*getTime;void*setTime;void*getWakeupTime;void*setWakeupTime;void*setVirtualAddressMap;void*convertPointer;
 Status(*getVariable)(Char16*,Guid*,u32*,usize*,void*);void*getNextVariableName;Status(*setVariable)(Char16*,Guid*,u32,usize,void*);void*getNextHighMonotonicCount;void*resetSystem;void*updateCapsule;void*queryCapsuleCapabilities;void*queryVariableInfo;};
enum AllocateType:u32{AllocateAnyPages=0,AllocateMaxAddress=1,AllocateAddress=2};
enum MemoryType:u32{LoaderCode=1,LoaderData=2};
struct BootServices {TableHeader hdr;void*raiseTpl;void*restoreTpl;Status(*allocatePages)(AllocateType,MemoryType,usize,u64*);Status(*freePages)(u64,usize);Status(*getMemoryMap)(usize*,void*,usize*,usize*,u32*);Status(*allocatePool)(MemoryType,usize,void**);Status(*freePool)(void*);void*createEvent;void*setTimer;void*waitForEvent;void*signalEvent;void*closeEvent;void*checkEvent;void*installProtocolInterface;void*reinstallProtocolInterface;void*uninstallProtocolInterface;Status(*handleProtocol)(Handle,Guid*,void**);void*reserved;void*registerProtocolNotify;void*locateHandle;void*locateDevicePath;void*installConfigurationTable;void*loadImage;void*startImage;void*exit;void*unloadImage;Status(*exitBootServices)(Handle,usize);void*getNextMonotonicCount;void*stall;void*setWatchdogTimer;void*connectController;void*disconnectController;void*openProtocol;void*closeProtocol;void*openProtocolInformation;void*protocolsPerHandle;void*locateHandleBuffer;Status(*locateProtocol)(Guid*,void*,void**);};
struct LoadedImageProtocol{u32 revision;Handle parentHandle;SystemTable*systemTable;Handle deviceHandle;void*filePath;void*reserved;u32 loadOptionsSize;void*loadOptions;void*imageBase;u64 imageSize;u32 imageCodeType,imageDataType;Status(*unload)(Handle);};
struct FileProtocol;
struct SimpleFileSystemProtocol{u64 revision;Status(*openVolume)(SimpleFileSystemProtocol*,FileProtocol**);};
struct FileProtocol{u64 revision;Status(*open)(FileProtocol*,FileProtocol**,Char16*,u64,u64);Status(*close)(FileProtocol*);void*deleteFile;Status(*read)(FileProtocol*,usize*,void*);Status(*write)(FileProtocol*,usize*,void*);void*getPosition;void*setPosition;void*getInfo;void*setInfo;void*flush;};
struct GraphicsOutputModeInformation{u32 version,horizontalResolution,verticalResolution,pixelFormat;u32 pixelInformation[4];u32 pixelsPerScanLine;};
struct GraphicsOutputProtocolMode{u32 maxMode,mode;GraphicsOutputModeInformation*info;usize sizeOfInfo;u64 frameBufferBase;usize frameBufferSize;};
struct GraphicsOutputProtocol{void*queryMode;void*setMode;void*blt;GraphicsOutputProtocolMode*mode;};
inline bool Eq(const Guid&a,const Guid&b){const u8*x=(const u8*)&a,*y=(const u8*)&b;for(usize i=0;i<16;i++)if(x[i]!=y[i])return false;return true;}
static Guid LoadedImageGuid={0x5B1B31A1,0x9562,0x11d2,{0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B}};
static Guid SimpleFsGuid={0x964e5b22,0x6459,0x11d2,{0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}};
static Guid GopGuid={0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}};
static Guid Acpi20Guid={0x8868e871,0xe4f1,0x11d3,{0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}};
static Guid Acpi10Guid={0xeb9d2d30,0x2d88,0x11d3,{0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}};
}
