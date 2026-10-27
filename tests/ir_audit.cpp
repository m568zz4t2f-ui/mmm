// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline diagnostic; printed measurements are not a release pass verdict.
#include "TideDSP.h"
#include <cstdio>
int main(){for(int model=0;model<11;++model){tide::Parameters p;p.delayOn=false;p.reverbType=model<4?model:2;p.reverbModel=model<4?0:model-3;p.mix=1;p.reverbAmount=.3f;p.reverbDuck=0;p.predelay=0;p.decay=4;p.reverbTone=12000;p.bassDecay=1;p.shimmer=0;tide::Engine e;e.prepare(48000,p);char name[96];std::snprintf(name,sizeof(name),"/tmp/tide-audit-ir-%d.raw",model);auto* file=std::fopen(name,"wb");for(int n=0;n<48000*8;++n){auto y=e.tick({n==0?.1f:0,n==0?.1f:0});std::fwrite(&y,sizeof(y),1,file);}std::fclose(file);}}
