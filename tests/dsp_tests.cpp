// SPDX-License-Identifier: AGPL-3.0-or-later
#include "TideDSP.h"
#include <iostream>
#include <cstdlib>
#include <chrono>
static void check(bool b,const char* message){if(!b){std::cerr<<"FAIL: "<<message<<'\n';std::exit(1);}}
int main(){
 const auto started=std::chrono::steady_clock::now();double signature[6]{};
 // All six modes at three rates, hard-panned impulses and bounded high feedback.
 for(float sr:{44100.f,48000.f,96000.f})for(int mode=0;mode<6;++mode){
  tide::Parameters p;p.delayType=mode;p.reverbOn=false;p.delayAmount=.4f;p.mix=1;p.feedback=.72f;p.sync=false;p.time=180;p.stereo=0;p.delayMotion=0;p.delayDuck=0;
  tide::Engine e;e.prepare(sr,p);double energy=0,right=0;float peak=0;
  for(int n=0;n<int(sr*2);++n){auto y=e.tick({n==0?.6f:0,0});check(std::isfinite(y.l)&&std::isfinite(y.r),"finite delay output");peak=std::max(peak,std::abs(y.l));if(n>1000){energy+=double(y.l)*y.l;right+=double(y.r)*y.r;}}
  check(energy>1e-7,"delay returns nonzero sound");check(peak<4,"feedback bounded");check(right<1e-12,"stereo delay preserves hard-panned source");if(sr==48000)signature[mode]=energy;
 }
 check(std::abs(signature[0]-signature[1])>1e-5,"tape differs from digital");check(std::abs(signature[1]-signature[2])>1e-5,"bucket differs from tape");
 // Cross-feedback must produce echoes on both sides from a hard-panned input.
 {tide::Parameters p;p.reverbOn=false;p.sync=false;p.time=80;p.stereo=2;p.feedback=.6f;p.delayMotion=0;p.mix=1;tide::Engine e;e.prepare(48000,p);double right=0;
  for(int n=0;n<48000;++n){auto y=e.tick({n==0?.5f:0,0});right+=double(y.r)*y.r;}check(right>1e-6,"ping-pong crosses stereo channels");}
 // Four reverb models should produce distinct, finite stereo tails.
 double models[4]{};
 for(int mode=0;mode<4;++mode){tide::Parameters p;p.delayOn=false;p.reverbType=mode;p.mix=1;p.decay=2;p.reverbDuck=0;p.shimmer=.2f;
  tide::Engine e;e.prepare(48000,p);double l=0,r=0,mono=0;for(int n=0;n<144000;++n){auto y=e.tick({n==0?.5f:0,0});check(std::abs(y.l)<8&&std::abs(y.r)<8,"reverb/shimmer stability");if(n>12000){l+=double(y.l)*y.l;r+=double(y.r)*y.r;mono+=.25*double(y.l+y.r)*(y.l+y.r);}}
  check(mono>1e-9,"reverb survives mono fold-down");check(l>1e-9&&r>1e-9,"reverb creates stereo tail");models[mode]=l+r;
 }
 for(int i=0;i<4;++i)for(int j=i+1;j<4;++j)check(std::abs(models[i]-models[j])>1e-8,"different reverb models");
 // Exact clean delay timing with no modulation; unity timing at 120 BPM.
 {tide::Parameters p;p.time=250;p.sync=false;p.delayTone=16000;p.delayLowcut=20;p.delayMotion=0;p.delayAmount=.4f;p.stereo=0;p.feedback=0;
 tide::Echo e;e.prepare(48000,p);e.set(p);int first=-1;for(int n=0;n<13000;++n){auto y=e.tick({n==0?1.f:0,0},p);if(std::abs(y.l)>1e-5&&first<0)first=n;}check(first==12000,"fractional delay exact timing");}
 // Internal bypass retains tails; clear empties all tanks without allocating.
 {tide::Parameters p;p.delayType=0;p.sync=false;p.time=100;p.decay=4;p.mix=1;tide::Engine e;e.prepare(48000,p);
  for(int n=0;n<24000;++n)e.tick({n==0?.8f:0,n==0?.4f:0});p.delayOn=p.reverbOn=false;e.set(p);double energy=0;
  for(int n=0;n<24000;++n){auto y=e.tick({});energy+=double(y.l)*y.l+double(y.r)*y.r;}check(energy>1e-8,"bypass tails retained");e.clear();
  for(int n=0;n<4800;++n){auto y=e.tick({});check(std::abs(y.l)<1e-12&&std::abs(y.r)<1e-12,"clear silences tanks");}}
 // Host bypass must not excite new echoes, and returns dry unity after smoothing.
 {tide::Parameters p;tide::Engine e;e.prepare(48000,p);tide::Stereo y;for(int n=0;n<48000;++n)y=e.tick({.1f,-.1f},true);check(std::abs(y.l-.1f)<.002f,"host bypass dry unity");}
 // Return-track mode never leaks dry audio, including when both sides are off.
 {tide::Parameters p;p.send=true;p.delayOn=p.reverbOn=false;tide::Engine e;e.prepare(48000,p);for(int n=0;n<48000;++n){auto y=e.tick({.3f,-.2f});check(y.l==0&&y.r==0,"send mode dry suppression");}}
 // Delay amount must not change the stereo feed into reverb.
 {tide::Parameters p;p.sync=false;p.time=80;p.mix=1;p.direct=0;p.delayAmount=0;p.feed=1;p.reverbType=1;p.delayMotion=0;p.reverbMotion=0;
 tide::Engine a,b;a.prepare(48000,p);auto q=p;q.delayAmount=.7f;b.prepare(48000,q);double energy=0;
 for(int n=0;n<96000;++n){a.tick({n==0?.5f:0,0});b.tick({n==0?.5f:0,0});check(std::abs(a.reverbPeak-b.reverbPeak)<1e-7,"independent delay-to-reverb feed");energy+=a.reverbPeak;}check(energy>1e-5,"muted audible delay still feeds reverb");}
 // Rapid automation / mode changes / freeze at maximum musical settings.
 {tide::Parameters p;p.shimmer=1;p.feedback=.88f;p.decay=20;p.bloom=1;p.delayAmount=.8f;p.reverbAmount=.8f;p.mix=1;tide::Engine e;e.prepare(48000,p);
 for(int n=0;n<240000;++n){if(n%2048==0){p.delayType=(n/2048)%6;p.reverbType=(n/2048)%4;p.time=20+float((n/2048)%20)*90;p.sync=false;p.freeze=n>120000&&n<180000;e.set(p);}auto y=e.tick({.08f*std::sin(float(n)*.036f),.08f*std::sin(float(n)*.049f)});check(std::isfinite(y.l)&&std::abs(y.l)<16,"automation and hold remain bounded");}}
 // Regression: long maximum-shimmer tails must decay, including the high-gain room network.
 for(int mode=0;mode<4;++mode){tide::Parameters p;p.delayOn=false;p.reverbType=mode;p.decay=20;p.shimmer=1;p.mix=1;p.reverbAmount=.8f;p.reverbDuck=0;tide::Engine e;e.prepare(48000,p);float peak=0,late=0;
  for(int n=0;n<48000*30;++n){auto y=e.tick({n<48000?.1f*std::sin(float(n)*.038f):0,n<48000?.1f*std::sin(float(n)*.045f):0});peak=std::max(peak,std::max(std::abs(y.l),std::abs(y.r)));if(n>48000*28)late=std::max(late,std::max(std::abs(y.l),std::abs(y.r)));}
  check(peak<2&&late<.05f,"maximum shimmer decays without runaway");
 }
 const auto seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
 std::cout<<"PASS: timing, six delays, four reverbs, stereo, shimmer, bypass tails, independent feed, send mode, clear, automation. "<<seconds<<" s\n";
}
