#include <cstdio>
#include "Kernel/Nvme.hpp"
using namespace Davis;
int main(){Nvme::State s{};s.identifyComplete=true;s.namespaceId=7;s.namespaceBlocks=1000000;s.logicalBlockBytes=512;Nvme::IoCommand c{};
 if(!Nvme::BuildIoCommand(s,Nvme::IoOpcode::Read,1,100,8,0x1000,0,c))return 1;if(c.opcode!=2||c.nsid!=7||c.cdw10!=100||c.cdw12!=7)return 2;
 if(Nvme::BuildIoCommand(s,Nvme::IoOpcode::Read,2,999999,2,0x1000,0,c))return 3;if(Nvme::BuildIoCommand(s,Nvme::IoOpcode::Write,3,0,16,0x1000,0,c))return 4;
 if(!Nvme::BuildIoCommand(s,Nvme::IoOpcode::Write,4,0,16,0x1000,0x2000,c))return 5;if(!Nvme::BuildIoCommand(s,Nvme::IoOpcode::Flush,5,0,1,0,0,c))return 6;
 if(!Nvme::CompletionSucceeded(1,9,9)||Nvme::CompletionSucceeded(3,9,9)||Nvme::CompletionSucceeded(1,9,8))return 7;
 unsigned long long n=0;for(unsigned i=0;i<100000;i++){u64 lba=(i*97ull)%999000ull;u16 blocks=(u16)(1+(i%8));if(!Nvme::BuildIoCommand(s,(i&1)?Nvme::IoOpcode::Read:Nvme::IoOpcode::Write,(u16)(1+(i%65534)),lba,blocks,0x1000,0x2000,c))return 8;n++;}
 std::printf("PASS nvme_io commands=%llu namespace_blocks=%llu block_bytes=%u\n",n,(unsigned long long)s.namespaceBlocks,s.logicalBlockBytes);return 0;}
