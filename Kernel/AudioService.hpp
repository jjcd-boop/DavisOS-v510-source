#pragma once
#include "../Base/Types.hpp"
namespace Davis::AudioService {struct State{u64 pumps;bool ready,hdaPresent,hdaReady;u32 codecs;};extern State state;void Init();void PumpOnce();}
