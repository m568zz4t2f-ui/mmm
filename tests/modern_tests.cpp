// SPDX-License-Identifier: AGPL-3.0-or-later
#include "VersionedDSP.h"
#include <iostream>
#include <cstdlib>
#include <vector>
static void check(bool value,const char* message){if(!value){std::cerr<<"FAIL: "<<message<<'\n';std::exit(1);}}
int main(){
 using namespace tide::v13;
 std::vector<std::vector<float>> responses;
 for(float sr:{8000.f,44100.f,48000.f,96000.f,192000.f}){
  for(int model=0;model<12;++model){DelayMachine machine;machine.prepare(sr);DelaySettings p;p.model=model;p.time=p.rightTime=70;p.feedback=p.rightFeedback=.25f;machine.set(p);machine.clear();double energy=0;std::vector<float> response;
   for(int n=0;n<int(sr*.8f);++n){const float x=n<int(sr*.025f)?.07f*std::sin(float(n)*.13f):0;auto y=machine.tick({x,x*.61f});check(std::isfinite(y.l)&&std::isfinite(y.r)&&peak(y)<2,"delay finite and bounded");energy+=double(y.l)*y.l+double(y.r)*y.r;if(sr==8000)response.push_back(y.l);}
   check(energy>1e-9,"each delay returns nonzero audio");if(sr==8000)responses.push_back(response);machine.clear();for(int n=0;n<1000;++n)check(peak(machine.tick({}))==0,"delay clear hides old history");
  }
  for(int model=0;model<13;++model){ReverbMachine machine;machine.prepare(sr);ReverbSettings p;p.model=model;p.predelay=0;p.decay=1.2f;p.shimmer=model==6||model==7?.25f:0;machine.set(p);machine.clear();double energy=0;std::vector<float> response;
   for(int n=0;n<int(sr);++n){auto y=machine.tick({n==0?.2f:0,n==0?.13f:0});check(std::isfinite(y.l)&&std::isfinite(y.r)&&peak(y)<2,"reverb finite and bounded");energy+=double(y.l)*y.l+double(y.r)*y.r;if(sr==8000)response.push_back(y.l);}
   check(energy>1e-9,"each reverb returns an impulse response");if(sr==8000)responses.push_back(response);machine.clear();for(int n=0;n<1000;++n)check(peak(machine.tick({}))==0,"reverb clear hides old history");
  }
 }
 for(std::size_t i=0;i<responses.size();++i)for(std::size_t j=0;j<i;++j){if(responses[i].size()!=responses[j].size())continue;double difference=0;for(std::size_t n=0;n<responses[i].size();++n)difference+=std::pow(double(responses[i][n])-responses[j][n],2);check(difference>1e-9,"machine impulse responses differ");}
 {CaptureGrains a,b;a.prepare(8000);b.prepare(8000);DelaySettings p;p.interval=12;
  for(int n=0;n<8000;++n){float x=.03f*std::sin(n*.13f);a.tick({x,x},p);b.tick({x,x},p);}p.freeze=true;double energy=0;
  for(int n=0;n<8000;++n){auto x=a.tick({},p),y=b.tick({.7f,-.3f},p);check(x.l==y.l&&x.r==y.r,"frozen capture ignores new input");energy+=x.l*x.l;}check(energy>1e-5,"frozen capture stays audible");}
 for(int routing=0;routing<3;++routing){Settings p;p.routing=routing;p.delays[0].time=p.delays[0].rightTime=70;p.delays[1].time=p.delays[1].rightTime=110;p.crossFeedback=.2f;Engine engine;engine.prepare(8000,p);
  for(int n=0;n<16000;++n){auto y=engine.tick({n<800?.03f*std::sin(n*.1f):0,0});check(std::isfinite(y.l)&&std::isfinite(y.r)&&peak(y)<=1,"four-slot routing bounded");}check(engine.activeSlots()>0,"routed tails remain active");engine.clear();for(int n=0;n<1000;++n)check(peak(engine.tick({}))==0,"routing clear hides all history");}
 {Slot<DelayMachine,DelaySettings> slot;slot.prepare(8000);DelaySettings p;p.model=3;p.time=p.rightTime=8000;p.feedback=p.rightFeedback=0;for(auto& tap:p.taps){tap.level=0;tap.feedback=0;}p.taps[3].time=2;p.taps[3].level=1;slot.set(p);float late=0;
  for(int n=0;n<128100;++n){auto y=slot.tick({n==0?.25f:0,0});if(n>=128000)late=std::max(late,std::abs(y.l));}check(late>.01f,"sixteen-second tap survives silent gaps and remains in recorded history");}
 {tide::Parameters p;p.soundVersion=1;p.reverbModel=2;p.sync=false;p.time=100;tide::Engine frozen;Settings modern;VersionedEngine bridge;frozen.prepare(48000,p);bridge.prepare(48000,p,modern,0);
  for(int n=0;n<48000;++n){if(n%1024==0){p.time=70+float(n%7)*30;frozen.set(p);bridge.set(p,modern,0);}tide::Stereo input{.04f*std::sin(n*.07f),.025f*std::sin(n*.113f)};auto a=frozen.tick(input),b=bridge.tick(input);check(a.l==b.l&&a.r==b.r,"legacy revision is sample-identical to frozen 1.2.2 engine");}
  bridge.set(p,modern,1);for(int n=0;n<8000;++n){auto y=bridge.tick({.01f,-.02f});check(std::isfinite(y.l)&&std::isfinite(y.r),"revision transition remains finite");}}
 std::cout<<"PASS: 25 experimental machines at five rates, distinct numerical responses, reset, capture freeze and four-slot routing. No musical-equivalence claim.\n";
}
