// SPDX-License-Identifier: AGPL-3.0-or-later
#include "VersionedDSP.h"
#include <chrono>
#include <cstdio>
volatile float tideBenchmarkSink=0;
int main(){using namespace tide::v13;using Clock=std::chrono::steady_clock;std::puts("rate,configuration,model,seconds,audio_seconds,realtime_fraction");
 for(float rate:{44100.f,48000.f,96000.f,192000.f}){
  for(int family=0;family<2;++family)for(int model=0;model<(family?13:12);++model){DelayMachine delay;ReverbMachine reverb;DelaySettings d;ReverbSettings r;d.model=r.model=model;d.time=d.rightTime=125;r.shimmer=(model==6||model==7)?.25f:0;if(family){reverb.prepare(rate);reverb.set(r);}else{delay.prepare(rate);delay.set(d);}
   auto start=Clock::now();float checksum=0;for(int n=0;n<int(rate);++n){tide::Stereo input{.03f*std::sin(float(n)*.071f),.01f*std::sin(float(n)*.037f)};auto y=family?reverb.tick(input):delay.tick(input);checksum+=y.l;}double seconds=std::chrono::duration<double>(Clock::now()-start).count();tideBenchmarkSink=checksum;std::printf("%g,%s,%d,%.6f,1,%.6f\n",double(rate),family?"reverb":"delay",model,seconds,seconds);
  }
  for(int route=0;route<3;++route){Settings p;p.routing=route;p.crossFeedback=.3f;p.delays[0].model=7;p.delays[1].model=5;p.reverbs[0].model=5;p.reverbs[1].model=10;Engine engine;engine.prepare(rate,p);auto start=Clock::now();float checksum=0;for(int n=0;n<int(rate);++n){if(n%256==0)engine.set(p);auto y=engine.tick({.02f*std::sin(float(n)*.071f),.01f});checksum+=y.l;}double seconds=std::chrono::duration<double>(Clock::now()-start).count();tideBenchmarkSink=checksum;std::printf("%g,four_slots,%d,%.6f,1,%.6f\n",double(rate),route,seconds,seconds);}
 }
}
