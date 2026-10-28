// SPDX-License-Identifier: AGPL-3.0-or-later
#include <cstdlib>
#include <new>
#include <cstdio>
static bool track=false;static std::size_t calls=0,bytes=0;
void* operator new(std::size_t n){if(track){++calls;bytes+=n;}if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p)noexcept{std::free(p);}void operator delete(void* p,std::size_t)noexcept{std::free(p);}
#include "ModernDSP.h"
#include <chrono>
#include <limits>
static void check(bool value,const char* message){if(!value){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
int main(){using namespace tide::v13;
 for(float rate:{44100.f,48000.f,96000.f,192000.f}){
  Settings p;p.delays[0].feedback=p.delays[0].rightFeedback=p.delays[1].feedback=p.delays[1].rightFeedback=.92f;p.reverbs[0].decay=p.reverbs[1].decay=20;Engine engine;bytes=calls=0;track=true;engine.prepare(rate,p);track=false;std::printf("prepared,%g,%zu,%zu\n",double(rate),bytes,calls);
  calls=bytes=0;track=true;
  for(int n=0;n<int(rate*1.5f);++n){
   if(n%512==0){p.routing=(n/512)%3;p.crossFeedback=.3f;p.delays[0].model=(n/512)%12;p.delays[1].model=(n/1024)%12;p.reverbs[0].model=(n/512)%13;p.reverbs[1].model=(n/1024)%13;engine.set(p);}
   const float x=n%1000<100?.02f*std::sin(float(n)*.031f):0;auto y=engine.tick({x,-x*.3f});check(std::isfinite(y.l)&&std::isfinite(y.r)&&peak(y)<=1,"automated routing/model transitions bounded");
  }
  track=false;check(calls==0,"no tracked allocations in processing or parameter updates");std::printf("processing,%g,%zu,%zu\n",double(rate),bytes,calls);
 }
 {Settings p;for(auto& d:p.delays)d.on=false;for(auto& r:p.reverbs)r.on=false;Engine e;e.prepare(8000,p);for(int n=0;n<8000;++n){auto x=tide::Stereo{.1f*std::sin(n*.1f),-.03f};auto y=e.tick(x);check(x.l==y.l&&x.r==y.r,"all-disabled identity");}check(e.activeSlots()==0,"unused slots stay asleep");}
 {Settings p;p.mix=0;p.delays[1].on=false;for(auto& r:p.reverbs)r.on=false;Engine e;e.prepare(8000,p);for(int n=0;n<1000;++n){auto x=tide::Stereo{.13f,.07f};auto y=e.tick(x);check(x.l==y.l&&x.r==y.r,"mix-zero identity");}check(e.activeSlots()==1,"single-engine patch runs one slot");}
 {DelayMachine d;d.prepare(8000);DelaySettings p;p.time=p.texture=p.interval=std::numeric_limits<float>::quiet_NaN();d.set(p);for(int n=0;n<8000;++n){auto y=d.tick({.01f,0});check(std::isfinite(y.l)&&std::isfinite(y.r),"invalid delay control handling");}}
 std::puts("PASS: allocation instrumentation, bounded automation fixtures, slot sleep, dry identity and invalid-control checks. Not realtime certification.");
}
