// SPDX-License-Identifier: AGPL-3.0-or-later
// Compile against old and current headers with the same toolchain; compare stdout bytes.
#include "TideDSP.h"
#include <cstdio>
int main(){for(float rate:{44100.f,48000.f,96000.f,192000.f})for(int d=0;d<9;++d)for(int r=0;r<11;++r){tide::Parameters p;p.sync=false;p.time=23;p.bTime=37;p.delayType=d<6?d:0;p.delayModel=d>=6?d-5:0;p.reverbType=r<4?r:2;p.reverbModel=r>=4?r-3:0;p.soundVersion=p.reverbVersion=1;p.decay=.6f;p.shimmer=r==6||r==7?.2f:0;tide::Engine engine;engine.prepare(rate,p);for(int n=0;n<int(rate*.13f);++n){if(n%512==0){p.time=23+float(n%3)*5;engine.set(p);}tide::Stereo input{.04f*std::sin(n*.071f),.02f*std::sin(n*.113f)};auto y=engine.tick(input,n>int(rate*.1f));std::fwrite(&y,sizeof(y),1,stdout);}}}