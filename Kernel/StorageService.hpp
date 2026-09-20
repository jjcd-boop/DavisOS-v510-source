#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
#include "Nvme.hpp"
#include "NvmeStorageRuntime.hpp"
namespace Davis::StorageService {
struct State {u64 pumps,filesChecked,opensChecked,repairs;bool ready;Pci::State pci;Nvme::State nvme;bool nvmeProbeAttempted,nvmeIdentifyAttempted,nvmeIoAttempted;NvmeStorageRuntime::State nvmeStorage;};
extern State state;
void Init();void PumpOnce();
}
