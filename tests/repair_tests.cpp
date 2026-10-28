// SPDX-License-Identifier: AGPL-3.0-or-later
#include "TideDSP.h"
#include <cstdlib>
#include <iostream>
static void check(bool ok,const char* message){if(!ok){std::cerr<<"FAIL: "<<message<<'\n';std::exit(1);}}
static double difference(tide::Parameters p,tide::Parameters q,int samples=24000){
 tide::Echo a,b;a.prepare(8000,p);b.prepare(8000,q);a.set(p);b.set(q);double diff=0;
 for(int n=0;n<samples;++n){const float x=n<8000?.07f*std::sin(float(n)*.19f):0;const auto l=a.tick({x,x*.7f},p),r=b.tick({x,x*.7f},q);diff+=std::pow(l.l-r.l,2)+std::pow(l.r-r.r,2);}
 return diff;
}
int main(){
 for(int model:{0,1,2,3,5,6,7,8}){
  tide::Parameters p;p.sync=false;p.time=100;p.delayMotion=0;p.stereo=2;p.feedback=.5f;p.delayLowcut=20;p.delayTone=16000;
  p.delayType=model<6?model:0;p.delayModel=model>=6?model-5:0;
  if(model==3)p.soundVersion=0;
  tide::Echo e;e.prepare(8000,p);e.set(p);double first[2]{},second[2]{};
  for(int n=0;n<2300;++n){auto y=e.tick({n==0?.5f:0,n==0?.5f:0},p);if(n>=800&&n<820){first[0]+=y.l*y.l;first[1]+=y.r*y.r;}if(n>=1600&&n<1620){second[0]+=y.l*y.l;second[1]+=y.r*y.r;}}
  // Pitched/granular and multitap arrivals have different timing; simple echoes must alternate.
  if(model<3||model==8)check(first[0]>1e-6&&first[1]<first[0]*.01&&second[1]>second[0]*10,"mono ping-pong alternates feedback lanes");
 }
 for(int model=0;model<9;++model){
  tide::Parameters p;p.sync=false;p.time=100;p.stereo=2;p.feedback=.5f;p.delayMotion=0;p.delayType=model<6?model:0;p.delayModel=model>=6?model-5:0;p.soundVersion=1;
  tide::Echo e;e.prepare(8000,p);e.set(p);int left=-1,right=-1;
  for(int n=0;n<16000;++n){auto y=e.tick({n<100?.1f*std::sin(n*.11f):0,n<100?.1f*std::sin(n*.11f):0},p);if(left<0&&std::abs(y.l)>1e-5f)left=n;if(right<0&&std::abs(y.r)>1e-5f)right=n;}
  check(left>=0&&right>left,"all nine models launch mono left before returning crossed feedback right");
 }
 for(int model:{3,4}){
  tide::Parameters p;p.sync=false;p.time=100;p.bTime=200;p.dualMode=1;p.delayType=model;p.soundVersion=1;p.delayMotion=0;p.feedback=p.bFeedback=0;
  auto q=p;q.bTime=400;check(difference(p,q)>1e-3,"Pattern/Reverse independent B time responds");
  p.sync=true;p.rhythm=2;p.bRhythm=2;q=p;q.bRhythm=4;check(difference(p,q)>1e-3,"Pattern/Reverse B subdivision responds");
 }
 for(int model:{1,2,3}){
  tide::Parameters p;p.sync=false;p.delayModel=model;p.delayType=0;auto q=p;q.delayType=2;
  check(difference(p,q)==0,"extension tone is independent of hidden legacy selector");
 }
 {tide::Parameters p;p.sync=false;p.delayModel=1;auto q=p;q.diffusion=0;check(difference(p,q)==0,"reverb diffusion does not alter delay");q=p;q.delayDiffusion=0;check(difference(p,q)>1e-4,"dedicated delay diffusion responds");}
 for(float sr:{8000.f,48000.f,192000.f})for(int model:{3,5})for(int control=0;control<2;++control){
  tide::Parameters p;p.reverbMotion=0;p.size=.5f;auto q=p;if(control==0)q.reverbMotion=1;else q.size=1.7f;
  tide::BoutiqueTank a,b;a.prepare(sr,model);b.prepare(sr,model);a.set(p);b.set(q);double diff=0;
  for(int n=0;n<int(sr*.6f);++n){float x=n<int(sr*.1f)?.04f*std::sin(float(n)*.054f):0;auto l=a.tick({x,x*.7f}),r=b.tick({x,x*.7f});diff+=std::pow(l.l-r.l,2)+std::pow(l.r-r.r,2);}
  check(diff>1e-5,"boutique plate Size and Motion affect active tank at all tested rates");
 }
 for(int model:{4,7}){
  tide::Parameters p;p.attack=.18f;p.shimmer=0;p.reverbMotion=0;tide::BoutiqueTank a,b;a.prepare(48000,model);b.prepare(48000,model);a.set(p);b.set(p);double base=0,diff=0;
  for(int n=0;n<97000;++n){const float x=n<2400?.1f*std::sin(n*.07f):0;auto l=a.tick({x,x}),r=b.tick({n==96000?.2f:x,n==96000?.2f:x});if(n>=96000&&n<96096){base+=l.l*l.l+l.r*l.r;diff+=std::pow(l.l-r.l,2)+std::pow(l.r-r.r,2);}}
  check(base>1e-10&&diff<base*.01,"new note preserves existing bloom tail");
 }
 {tide::Parameters p;p.sync=false;p.time=20;p.feedback=0;p.delayMotion=0;p.swell=1;p.stereo=0;tide::Echo e;e.prepare(8000,p);e.set(p);double early=0,late=0;
  for(int n=0;n<8000;++n){auto y=e.tick({.1f*std::sin(n*.19f),0},p);if(n>400&&n<1200)early+=y.l*y.l;if(n>5600&&n<6400)late+=y.l*y.l;}
  check(late>early*8,"swell is a rising attack on sustained input");
 }
 for(int model:{3,4}){
  tide::Parameters p;p.delayType=model;p.soundVersion=1;p.sync=false;p.time=120;p.bTime=170;p.dualMode=1;p.delayMotion=0;
  tide::Echo a,b;a.prepare(8000,p);b.prepare(8000,p);a.set(p);b.set(p);
  for(int n=0;n<12000;++n){const float x=.1f*std::sin(n*.21f);a.tick({x,x*.7f},p);b.tick({x,x*.7f},p);}
  auto q=p;q.time=370;q.bTime=430;b.set(q);auto old=a.tick({.03f,.02f},p),changed=b.tick({.03f,.02f},q);
  check(std::abs(old.l-changed.l)<.001f&&std::abs(old.r-changed.r)<.001f,"complete Pattern/Reverse structures begin time changes with a crossfade");
 }
 for(float sr:{8000.f,48000.f,192000.f})for(int interval=0;interval<6;++interval){
  tide::Parameters p;p.interval=interval;p.delayMotion=.8f;p.delayRate=2;tide::Granular grains;grains.prepare(sr);double energy=0;
  for(int n=0;n<int(sr*.4f);++n){auto y=grains.tick({.03f*std::sin(n*.03f),0},p);check(std::isfinite(y.l)&&y.r==0&&std::abs(y.l)<.2f,"bounded stereo grains at all intervals and tested rates");energy+=y.l*y.l;}
  check(energy>1e-5,"grain scheduler returns retained audio");grains.clear();for(int n=0;n<1000;++n){auto y=grains.tick({},p);check(y.l==0&&y.r==0,"grain reset hides all source history");}
 }
 std::cout<<"PASS: ping-pong, independent timing, model isolation, diffusion ownership, plate controls, tail continuity and swell.\n";
}
