#pragma once
#include "Pci.hpp"
namespace Davis::PlatformProfile {
enum class CpuFamily : unsigned char { Unknown, Intel, Amd };
struct State {
 CpuFamily cpuFamily; bool intelSerialIo; bool i2cCandidate; bool usbXhci; bool nvme; bool ahci;
 bool integratedDisplay; bool nvidiaDisplay; bool amdDisplay; bool hda; bool network;
 bool legacyInputFallback; bool preferI2cHid; bool hybridGraphics; unsigned int i2cCandidates;
};
void Detect(const Pci::State&, State&);
}
