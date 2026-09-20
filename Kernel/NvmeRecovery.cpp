#include "NvmeRecovery.hpp"
namespace Davis::NvmeRecovery {
bool Init(State&s,u32 m){s={};if(m<1||m>8)return false;s.stage=Stage::Healthy;s.maxAttempts=m;return true;}
bool Recover(State&s,const Hooks&h){if(!h.quiesce||!h.reset||!h.reinitialize||!s.maxAttempts||s.maxAttempts>8)return false;s.stage=Stage::Quiescing;if(!h.quiesce(h.context)){s.stage=Stage::Failed;s.failures++;return false;}for(u32 i=0;i<s.maxAttempts;i++){s.attempts++;s.stage=Stage::Resetting;if(!h.reset(h.context))continue;s.stage=Stage::Reinitializing;if(!h.reinitialize(h.context))continue;s.stage=Stage::Recovered;s.recoveries++;return true;}s.stage=Stage::Failed;s.failures++;return false;}
}
