// SPDX-License-Identifier: AGPL-3.0-or-later
#include "TideDSP.h"
#include <iostream>
#include <cstdlib>
#include <vector>
static void check(bool ok,const char* why){if(!ok){std::cerr<<"FAIL: "<<why<<'\n';std::exit(1);}}
static double pitchEnergy(float hz){tide::Pitch p;p.prepare(48000);p.set(24);p.clear();double sum=0;for(int n=0;n<48000;++n){auto y=p.tick({.25f*std::sin(2*tide::pi*hz*float(n)/48000),0});if(n>12000)sum+=double(y.l)*y.l;check(y.r==0,"linked stereo pitch does not leak channels");}return sum;}
int main(){
 // Generation clearing must hide old samples across blocks and wrap boundaries.
 {wolf::Delay d;d.prepare(139);for(int n=0;n<900;++n)d.push(float(n+1));d.clear();for(int n=0;n<137;++n)check(d.readInt(std::size_t(n))==0,"logical clear hides all history");d.push(.5f);check(d.readInt(0)==.5f,"first write after clear");for(int n=1;n<137;++n)check(d.readInt(std::size_t(n))==0,"first lazy block does not reveal stale samples");for(int n=0;n<500;++n)d.push(0);check(d.read(5.5f)==0&&d.readCubic(7.2f)==0,"interpolation after wrap and clear");}
 // An enabled but silent shimmer must not run. Only the selected tank runs on audio.
 for(int mode=0;mode<4;++mode){tide::Parameters p;p.delayOn=false;p.reverbType=mode;tide::Engine e;e.prepare(8000,p);for(int n=0;n<8000;++n)e.tick({.02f,.015f});check(e.activeReverbs()==1,"only the selected reverb runs");check(!e.activeEcho()&&!e.activeShimmer(),"unused delay and shimmer sleep");}
 {tide::Parameters p;p.shimmer=1;tide::Engine e;e.prepare(8000,p);for(int n=0;n<16000;++n)e.tick({});check(e.activeReverbs()==0&&!e.activeEcho()&&!e.activeShimmer(),"silent enabled engines sleep");auto y=e.tick({.2f,-.1f});check(std::isfinite(y.l)&&e.activeEcho(),"idle engine wakes on new input");}
 // Silence between echoes must not be mistaken for an exhausted delay buffer.
 {tide::Parameters p;p.reverbOn=false;p.sync=false;p.time=8000;p.ratio=2;p.stereo=1;p.delayMotion=p.delayDuck=0;p.feedback=0;p.mix=1;tide::Engine e;e.prepare(8000,p);float left=0,right=0;for(int n=0;n<128100;++n){auto y=e.tick({n==0?.5f:0,n==0?.5f:0});if(n>=64000&&n<64100)left=std::max(left,std::abs(y.l));if(n>=128000)right=std::max(right,std::abs(y.r));}check(left>.01f&&right>.01f,"8-second and 16-second repeats survive silent gaps");}
 // Muted returns need no processing once their short fades complete.
 {tide::Parameters p;p.delayAmount=p.reverbAmount=0;tide::Engine e;e.prepare(8000,p);for(int n=0;n<20000;++n)e.tick({.1f,-.1f});check(!e.activeEcho()&&e.activeReverbs()==0&&!e.activeShimmer(),"unrouted muted returns suspend under continuous input");}
 // Algorithm switching preserves the outgoing tail, and Clear never resurrects it.
 {tide::Parameters p;p.delayOn=false;p.mix=1;p.decay=3;p.reverbType=2;tide::Engine e;e.prepare(8000,p);for(int n=0;n<4000;++n)e.tick({n==0?.5f:0,0});p.reverbType=1;e.set(p);double tail=0;for(int n=0;n<8000;++n){auto y=e.tick({});tail+=double(y.l)*y.l+double(y.r)*y.r;}check(tail>1e-8,"outgoing algorithm tail remains audible");e.clear();for(int n=0;n<24000;++n){auto y=e.tick({});check(y.l==0&&y.r==0,"cleared audio never returns after sleep");}check(e.activeReverbs()==0,"cleared tanks sleep");}
 // Fixed harmony shifts the return, leaving the timed feedback loop unshifted.
 {tide::Parameters p;p.delayType=5;p.pitchMode=1;p.sync=false;p.time=200;p.stereo=0;p.feedback=.6f;p.delayMotion=0;p.delayLowcut=20;p.delayTone=16000;tide::Echo fixed;fixed.prepare(8000,p);fixed.set(p);double energy=0;for(int n=0;n<16000;++n){const auto y=fixed.tick({n<800?.1f*std::sin(2*tide::pi*220*float(n)/8000):0,0},p);energy+=double(y.l)*y.l;check(std::isfinite(y.l)&&y.r==0,"fixed pitch remains finite and stereo-linked");}check(energy>.001,"fixed harmony produces repeat audio");}
 // Upward transposition must reject content that would fold below Nyquist.
 const double pass=pitchEnergy(2000),reject=pitchEnergy(10000);check(pass>1&&reject/pass<.02,"+24-semitone anti-alias rejection");
 std::cout<<"PASS: logical clear, conditional DSP, wake-up, 16-second echoes, muted routing, tail transitions, +24 pitch alias rejection ("<<10*std::log10(reject/pass)<<" dB).\n";
}
