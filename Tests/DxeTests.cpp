#include "../Kernel/Dxe.hpp"
#include <cstdio>
int main(){
 using namespace Davis; alignas(16) unsigned char img[256]={};
 auto*h=(Dxe::Header*)img; h->magic=Dxe::Magic;h->headerBytes=sizeof(Dxe::Header);
 h->abiMajor=1;h->abiMinor=0;h->flags=Dxe::UserMode|Dxe::PositionIndependent;
 h->imageBytes=sizeof(img);h->codeOffset=128;h->codeBytes=64;h->entryOffset=128;
 h->dataOffset=192;h->dataBytes=32;h->requiredStackBytes=65536;
 if(Dxe::Validate(img,sizeof(img))!=Dxe::ValidateResult::Ok)return 1;
 h->magic=0;if(Dxe::Validate(img,sizeof(img))!=Dxe::ValidateResult::BadMagic)return 2;
 std::puts("PASS DXE1 validation");return 0;
}
