// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PrismDSP.h"
#include <chrono>
#include <memory>
#include <iostream>
#include <vector>
int main(){auto e=std::make_unique<prism::Engine>();constexpr int block=256;constexpr double sr=48000;std::vector<double> us(2000);volatile float sink=0;for(int mode=0;mode<3;++mode){prism::Settings p;p.enabled={true,true,true,true};if(mode==1)p.type={3,2,2,2};if(mode==2)p.type={2,4,4,4};e->prepare(sr,p);int sample=0;for(int n=0;n<48000;++n)sink+=e->tick({.1f,.07f}).l;auto start=std::chrono::steady_clock::now();for(auto& cost:us){auto t=std::chrono::steady_clock::now();for(int i=0;i<block;++i){float x=.2f*std::sin(float(sample++)*.031f);sink+=e->tick({x,x*.73f}).l;}cost=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-t).count();}double elapsed=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();double avg=std::accumulate(us.begin(),us.end(),0.)/us.size();std::sort(us.begin(),us.end());std::cout<<"case="<<mode<<" avg_us="<<avg<<" p99_us="<<us[1980]<<" max_us="<<us.back()<<" deadline_us="<<block/sr*1e6<<" realtime_percent="<<elapsed/(us.size()*block/sr*1e6)*100<<'\n';}std::cout<<"Core only, includes synthetic input generation/timing, 48kHz stereo/256 samples, all 4 modules enabled. No GUI/DAW overhead; not a DAW CPU meter. Checksum="<<sink<<'\n';}
