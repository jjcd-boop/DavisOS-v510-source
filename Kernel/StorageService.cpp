#include "StorageService.hpp"
#include "DxeServices.hpp"
#include "BlockDevice.hpp"
#include "KernelMemory.hpp"
#include "NvmeBlockBackend.hpp"
namespace Davis::StorageService {
State state{};
void Init(){state={};BlockDevice::Init();Pci::Scan(state.pci);Nvme::Init(state.nvme);Nvme::Discover(state.nvme,state.pci);state.ready=true;}
void PumpOnce(){
 if(!state.ready)return;BlockDevice::PumpOnce();
 // Capability probing is deliberately deferred to the scheduled storage context.
 // v1.24 reads controller registers only; it does not enable, reset, or submit NVMe commands.
 if(state.nvme.discovered&&state.nvme.barValid&&!state.nvmeProbeAttempted){state.nvmeProbeAttempted=true;if(Nvme::ReadCapabilities(state.nvme)&&KernelMemory::ready)Nvme::BeginAdminController(state.nvme,KernelMemory::allocator);}
 if(state.nvme.adminReady&&!state.nvmeIdentifyAttempted&&KernelMemory::ready){state.nvmeIdentifyAttempted=true;Nvme::IdentifyStorage(state.nvme,KernelMemory::allocator);}
 if(state.nvme.identifyComplete&&!state.nvmeIoAttempted&&KernelMemory::ready){state.nvmeIoAttempted=true;if(NvmeStorageRuntime::BringUp(state.nvmeStorage,state.nvme,KernelMemory::allocator))NvmeBlockBackend::BindRuntime(state.nvme,state.nvmeStorage,KernelMemory::allocator);}
 u64 files=0,opens=0,repairs=0;DxeServices::StorageMaintenance(files,opens,repairs);state.pumps++;state.filesChecked+=files;state.opensChecked+=opens;state.repairs+=repairs;
}
}
