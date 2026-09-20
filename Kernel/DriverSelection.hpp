#pragma once
#include "../Base/Types.hpp"
#include "Pci.hpp"
#include "DriverCatalog.hpp"
namespace Davis::DriverSelection {
static constexpr u32 MaxBindings=128;
static constexpr u32 MaxCandidates=8;
struct Candidate { const DriverCatalog::Match* match; u16 score; bool failed; };
struct Binding { Pci::Device pci; DriverCatalog::Family family; Candidate candidates[MaxCandidates]; u32 candidateCount; u32 selected; bool active; };
struct State { Binding bindings[MaxBindings]; u32 count; bool ready; };
extern State state;
void Init();
void Discover();
const Candidate* Selected(const Binding&);
bool FailSelected(u32 bindingIndex);
}
