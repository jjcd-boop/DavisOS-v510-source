#include "DxeServices.hpp"
#include "ObjectHandles.hpp"
#include "GraphicsObjectManager.hpp"
namespace Davis::DxeServices {
State state{};
static u64 nextZ=1;
static bool eq(const char*a,const char*b){if(!a||!b)return false;while(*a&&*b){if(*a++!=*b++)return false;}return *a==*b;}
static void copyPath(char*d,const char*s){usize i=0;for(;i<MaxPath-1&&s&&s[i];i++)d[i]=s[i];d[i]=0;}
static bool validPath(const char*p){
 if(!p||p[0]!='/')return false; usize n=0; bool seg=false;
 for(;p[n];n++){if(n>=MaxPath-1)return false;char c=p[n];if((unsigned char)c<32||c=='\\')return false;if(c=='/'){seg=false;continue;}
  if(c=='.'&&(p[n+1]=='/'||p[n+1]==0))return false;
  if(c=='.'&&p[n+1]=='.'&&(p[n+2]=='/'||p[n+2]==0))return false; seg=true;}
 return n>1&&seg;
}
void Init(){GraphicsObjectManager::Init();for(auto&w:state.windows)w.active=false;for(auto&s:state.surfaces){s={};}for(auto&f:state.files)f.active=false;for(auto&o:state.openFiles)o.active=false;state.nextId=0x1000;nextZ=1;}
u64 CreateWindow(u64 owner,i64 x,i64 y,i64 w,i64 h){
 if(!owner||w<64||h<48||w>(i64)MaxSurfaceWidth||h>(i64)MaxSurfaceHeight)return 0;
 i32 ws=-1,ss=-1;for(usize i=0;i<MaxWindows;i++)if(!state.windows[i].active){ws=(i32)i;break;}for(usize i=0;i<MaxWindowSurfaces;i++)if(!state.surfaces[i].active){ss=(i32)i;break;}if(ws<0||ss<0)return 0;
 u64 id=++state.nextId;auto*go=GraphicsObjectManager::Create(owner,(u32)w,(u32)h,GraphicsObjectManager::Read|GraphicsObjectManager::Write|GraphicsObjectManager::Present|GraphicsObjectManager::Compose);if(!go)return 0;state.windows[ws]={id,owner,x,y,w,h,0,nextZ++,ss,true};auto&sf=state.surfaces[ss];sf={id,owner,go->id,(u32)w,(u32)h,true};return id;
}
const Window* FindWindow(u64 owner,u64 objectId){for(auto&e:state.windows)if(e.active&&e.owner==owner&&e.id==objectId)return &e;return nullptr;}
Window* MutableWindow(u64 owner,u64 objectId){for(auto&e:state.windows)if(e.active&&e.owner==owner&&e.id==objectId)return &e;return nullptr;}
bool MoveWindow(u64 owner,u64 objectId,i64 x,i64 y){auto*w=MutableWindow(owner,objectId);if(!w)return false;w->x=x;w->y=y;return true;}
bool ResizeWindow(u64 owner,u64 objectId,i64 w,i64 h){auto*win=MutableWindow(owner,objectId);auto*sf=MutableWindowSurface(owner,objectId);if(!win||!sf||w<64||h<48||w>(i64)MaxSurfaceWidth||h>(i64)MaxSurfaceHeight)return false;if(!GraphicsObjectManager::Resize(owner,sf->graphicsObjectId,(u32)w,(u32)h))return false;win->w=w;win->h=h;sf->width=(u32)w;sf->height=(u32)h;return true;}
bool RaiseWindow(u64 owner,u64 objectId){auto*w=MutableWindow(owner,objectId);if(!w)return false;w->z=nextZ++;return true;}
WindowSurface* MutableWindowSurface(u64 owner,u64 objectId){for(auto&s:state.surfaces)if(s.active&&s.owner==owner&&s.objectId==objectId)return &s;return nullptr;}
const WindowSurface* FindWindowSurface(u64 owner,u64 objectId){for(auto&s:state.surfaces)if(s.active&&s.owner==owner&&s.objectId==objectId)return &s;return nullptr;}
bool PresentWindow(u64 owner,u64 objectId){for(auto&e:state.windows)if(e.active&&e.owner==owner&&e.id==objectId){e.presents++;return true;}return false;}
void DestroyWindow(u64 owner,u64 objectId){for(auto&s:state.surfaces)if(s.active&&s.owner==owner&&s.objectId==objectId){GraphicsObjectManager::Release(owner,s.graphicsObjectId);s={};}for(auto&e:state.windows)if(e.active&&e.owner==owner&&e.id==objectId)e={};}

bool RaiseFirstWindowForOwner(u64 owner){for(auto&e:state.windows)if(e.active&&e.owner==owner){e.z=nextZ++;return true;}return false;}
bool OwnerHasPresented(u64 owner){for(auto&e:state.windows)if(e.active&&e.owner==owner&&e.presents)return true;return false;}
u64 OpenFileObject(u64 owner,const char*path,u32 flags,u32*rightsOut){
 if(!owner||!validPath(path))return 0; const bool rd=(flags&1)!=0, wr=(flags&2)!=0, create=(flags&4)!=0, trunc=(flags&8)!=0; if(!rd&&!wr)return 0;
 i32 fi=-1;for(usize i=0;i<MaxFiles;i++)if(state.files[i].active&&eq(state.files[i].path,path)){fi=(i32)i;break;}
 if(fi<0&&create){for(usize i=0;i<MaxFiles;i++)if(!state.files[i].active){fi=(i32)i;state.files[i]={};state.files[i].active=true;copyPath(state.files[i].path,path);break;}}
 if(fi<0)return 0;if(trunc&&wr)state.files[fi].size=0;
 u32 rights=ObjectHandles::RightClose|(rd?ObjectHandles::RightRead:0)|(wr?ObjectHandles::RightWrite:0);
 for(auto&o:state.openFiles)if(!o.active){o={++state.nextId,owner,(u32)fi,0,rights,true};if(rightsOut)*rightsOut=rights;return o.id;}return 0;
}
const OpenFile* FindOpenFile(u64 owner,u64 id){for(auto&o:state.openFiles)if(o.active&&o.owner==owner&&o.id==id)return &o;return nullptr;}
i64 ReadFileObject(u64 owner,u64 id,void*dst,u64 n){if(!dst&&n)return -1;for(auto&o:state.openFiles)if(o.active&&o.owner==owner&&o.id==id){if(!(o.rights&ObjectHandles::RightRead))return -1;auto&f=state.files[o.fileIndex];u64 avail=o.offset<f.size?f.size-o.offset:0;if(n>avail)n=avail;for(u64 i=0;i<n;i++)((u8*)dst)[i]=f.data[o.offset+i];o.offset+=n;return (i64)n;}return -1;}
i64 WriteFileObject(u64 owner,u64 id,const void*src,u64 n){if(!src&&n)return -1;for(auto&o:state.openFiles)if(o.active&&o.owner==owner&&o.id==id){if(!(o.rights&ObjectHandles::RightWrite))return -1;auto&f=state.files[o.fileIndex];u64 room=o.offset<MaxFileBytes?MaxFileBytes-o.offset:0;if(n>room)n=room;for(u64 i=0;i<n;i++)f.data[o.offset+i]=((const u8*)src)[i];o.offset+=n;if(o.offset>f.size)f.size=o.offset;return (i64)n;}return -1;}
bool CloseFileObject(u64 owner,u64 id){for(auto&o:state.openFiles)if(o.active&&o.owner==owner&&o.id==id){o={};return true;}return false;}
bool SeekFileObject(u64 owner,u64 id,u64 offset){for(auto&o:state.openFiles)if(o.active&&o.owner==owner&&o.id==id){auto&f=state.files[o.fileIndex];if(offset>f.size)return false;o.offset=offset;return true;}return false;}
bool DeleteFileObject(u64 owner,const char*path){if(!owner||!validPath(path))return false;i32 fi=-1;for(usize i=0;i<MaxFiles;i++)if(state.files[i].active&&eq(state.files[i].path,path)){fi=(i32)i;break;}if(fi<0)return false;for(auto&o:state.openFiles)if(o.active&&o.fileIndex==(u32)fi)return false;state.files[fi]={};return true;}
bool RenameFileObject(u64 owner,const char*oldPath,const char*newPath){if(!owner||!validPath(oldPath)||!validPath(newPath)||eq(oldPath,newPath))return false;i32 fi=-1;for(usize i=0;i<MaxFiles;i++){if(state.files[i].active&&eq(state.files[i].path,newPath))return false;if(state.files[i].active&&eq(state.files[i].path,oldPath))fi=(i32)i;}if(fi<0)return false;for(auto&o:state.openFiles)if(o.active&&o.fileIndex==(u32)fi)return false;copyPath(state.files[fi].path,newPath);return true;}
i64 ListDirectory(const char* path,void* dst,u64 bytes){
 if(!path||!dst||bytes<64)return -1; char* out=(char*)dst; u64 used=0;
 const char* roots[]={"Documents/","Downloads/","Pictures/","Music/","Videos/","System/"};
 if(eq(path,"/")||eq(path,"/Home")||eq(path,"/Home/")){for(auto n:roots){u64 l=0;while(n[l])l++;if(used+l+1>bytes)break;for(u64 j=0;j<l;j++)out[used++]=n[j];out[used++]='\n';}}
 const char* prefix=path; u64 plen=0;while(prefix[plen])plen++;
 for(auto&f:state.files)if(f.active){bool match=true;for(u64 j=0;j<plen;j++)if(f.path[j]!=prefix[j]){match=false;break;}if(!match)continue;const char* n=f.path+plen;while(*n=='/')n++;if(!*n)continue;u64 l=0;while(n[l]&&n[l]!='/')l++;if(!l||used+l+1>bytes)continue;for(u64 j=0;j<l;j++)out[used++]=n[j];out[used++]='\n';}
 if(used<bytes)out[used]=0;return (i64)used;
}
void RevokeProcess(u64 owner){GraphicsObjectManager::RevokeProcess(owner);for(auto&s:state.surfaces)if(s.active&&s.owner==owner){s={};}for(auto&e:state.windows)if(e.active&&e.owner==owner)e={};for(auto&o:state.openFiles)if(o.active&&o.owner==owner)o={};}
void StorageMaintenance(u64& filesChecked,u64& opensChecked,u64& repairs){
 filesChecked=opensChecked=repairs=0;
 for(auto&f:state.files)if(f.active){filesChecked++;if(f.size>MaxFileBytes){f.size=MaxFileBytes;repairs++;}f.path[MaxPath-1]=0;}
 for(auto&o:state.openFiles)if(o.active){opensChecked++;if(o.fileIndex>=MaxFiles||!state.files[o.fileIndex].active){o={};repairs++;continue;}auto&f=state.files[o.fileIndex];if(o.offset>f.size){o.offset=f.size;repairs++;}}
}
}
