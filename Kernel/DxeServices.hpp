#pragma once
#include "../Base/Types.hpp"
namespace Davis::DxeServices {
static constexpr usize MaxWindows=64, MaxWindowSurfaces=64, MaxSurfaceWidth=1280, MaxSurfaceHeight=720;
static constexpr usize MaxFiles=16, MaxOpenFiles=64, MaxPath=128, MaxFileBytes=16384;
struct Window { u64 id,owner; i64 x,y,w,h; u64 presents,z; i32 surfaceSlot; bool active; };
struct WindowSurface { u64 objectId,owner,graphicsObjectId; u32 width,height; bool active; };
struct File { char path[MaxPath]; u8 data[MaxFileBytes]; u64 size; bool active; };
struct OpenFile { u64 id,owner; u32 fileIndex; u64 offset; u32 rights; bool active; };
struct State { Window windows[MaxWindows]; WindowSurface surfaces[MaxWindowSurfaces]; File files[MaxFiles]; OpenFile openFiles[MaxOpenFiles]; u64 nextId; };
extern State state;
void Init();
u64 CreateWindow(u64 owner,i64 x,i64 y,i64 w,i64 h);
bool PresentWindow(u64 owner,u64 objectId);
WindowSurface* MutableWindowSurface(u64 owner,u64 objectId);
const WindowSurface* FindWindowSurface(u64 owner,u64 objectId);
void DestroyWindow(u64 owner,u64 objectId);
const Window* FindWindow(u64 owner,u64 objectId);
Window* MutableWindow(u64 owner,u64 objectId);
bool MoveWindow(u64 owner,u64 objectId,i64 x,i64 y);
bool ResizeWindow(u64 owner,u64 objectId,i64 w,i64 h);
bool RaiseWindow(u64 owner,u64 objectId);
bool RaiseFirstWindowForOwner(u64 owner);
bool OwnerHasPresented(u64 owner);
u64 OpenFileObject(u64 owner,const char*path,u32 openFlags,u32*rightsOut);
i64 ReadFileObject(u64 owner,u64 objectId,void*dst,u64 bytes);
i64 WriteFileObject(u64 owner,u64 objectId,const void*src,u64 bytes);
bool CloseFileObject(u64 owner,u64 objectId);
bool SeekFileObject(u64 owner,u64 objectId,u64 offset);
bool DeleteFileObject(u64 owner,const char* path);
bool RenameFileObject(u64 owner,const char* oldPath,const char* newPath);
i64 ListDirectory(const char* path,void* dst,u64 bytes);
const OpenFile* FindOpenFile(u64 owner,u64 objectId);
void RevokeProcess(u64 owner);
void StorageMaintenance(u64& filesChecked,u64& opensChecked,u64& repairs);
}
