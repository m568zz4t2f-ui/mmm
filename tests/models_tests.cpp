// SPDX-License-Identifier: AGPL-3.0-or-later
#include "Parameters.h"
#include <iostream>
#include <cstdlib>
static void check(bool v,const char* msg){if(!v){std::cerr<<"FAIL: "<<msg<<'\n';std::exit(1);}}
int main(){
 check(tide::numPresets==64,"16 compatibility and 48 revised/new combined recipes");
 check(tide::presets[7].pitchMode==1,"Air Throw preserves Fixed Harmony");
 for(int type=1;type<=7;++type)for(float sr:{8000.f,48000.f,96000.f}){tide::Parameters p;p.soundVersion=1;p.delayOn=false;p.reverbModel=type;p.mix=1;p.decay=2;p.predelay=0;p.shimmer=(type==3||type==4)?.4f:0;tide::Engine e;e.prepare(sr,p);double energy=0;float max=0;for(int n=0;n<int(sr*2);++n){auto y=e.tick({n==0?.25f:0,n==0?.12f:0});check(std::isfinite(y.l)&&std::isfinite(y.r),"new reverb finite");energy+=double(y.l)*y.l+double(y.r)*y.r;max=std::max(max,std::max(std::abs(y.l),std::abs(y.r)));}check(energy>1e-7,"each reverb has a wet impulse response");check(max<2,"reverb impulse gain bounded");e.clear();for(int n=0;n<2000;++n){auto y=e.tick({});check(y.l==0&&y.r==0,"new tanks clear completely");}}
 for(int type=1;type<=3;++type){tide::Parameters p;p.delayModel=type;p.soundVersion=1;p.sync=false;p.time=80;p.reverbOn=false;p.mix=1;p.delayMotion=0;p.stereo=0;tide::Engine e;e.prepare(8000,p);double energy=0;for(int n=0;n<16000;++n){auto y=e.tick({n<800?.05f*std::sin(float(n)*.13f):0,0});check(std::isfinite(y.l)&&y.r==0,"new delays finite with independent channel state");energy+=double(y.l)*y.l;}check(energy>1e-6,"each delay returns audio");}
 {tide::Parameters p;p.sync=false;p.time=100;p.bTime=300;p.dualMode=1;p.delayMotion=0;p.feedback=p.bFeedback=0;p.delayLowcut=20;p.delayTone=16000;tide::Echo e;e.prepare(8000,p);e.set(p);int left=-1,right=-1;for(int n=0;n<3000;++n){auto y=e.tick({n==0?.5f:0,n==0?.5f:0},p);if(left<0&&std::abs(y.l)>.001f)left=n;if(right<0&&std::abs(y.r)>.001f)right=n;}check(left==800&&right==2400,"independent A/B timing");}
 {tide::Parameters p;p.delayOn=false;p.reverbModel=1;tide::Engine e;e.prepare(8000,p);for(int model=1;model<=7;++model){p.reverbModel=model;e.set(p);for(int n=0;n<2000;++n){auto y=e.tick({.01f,.007f});check(std::isfinite(y.l)&&std::isfinite(y.r),"rapid model changes stay finite");}}for(int n=0;n<24000;++n)e.tick({});check(e.activeReverbs()<=3,"outgoing modern tails are bounded");}
 std::cout<<"PASS: recipe inventory, dedicated reverb IRs at 3 rates, finite gain, clear, extension delays, independent Dual.\n";
}
