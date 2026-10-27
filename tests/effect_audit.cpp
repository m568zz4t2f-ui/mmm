// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline diagnostic; printed measurements are not a release pass verdict.
#include "TideDSP.h"
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include <iomanip>
using V=std::vector<tide::Stereo>;
constexpr int rate=8000,frames=24000;
float stimulus(int n){if(n%4800>=1800)return 0;float t=float(n)/rate;return .04f*(std::sin(2*tide::pi*137*t)+.4f*std::sin(2*tide::pi*311*t)+.2f*std::sin(2*tide::pi*997*t));}
V render(tide::Parameters p){tide::Engine e;e.prepare(rate,p);V out;out.reserve(frames);for(int n=0;n<frames;++n){float x=stimulus(n);out.push_back(e.tick({x,x*.73f}));}return out;}
double difference(const V& a,const V& b){double d=0,e=0;for(std::size_t n=0;n<a.size();++n){d+=std::pow(double(a[n].l)-b[n].l,2)+std::pow(double(a[n].r)-b[n].r,2);e+=double(a[n].l)*a[n].l+double(a[n].r)*a[n].r;}return std::sqrt(d/std::max(e,1e-30));}
V echo(tide::Parameters p){tide::Echo e;e.prepare(rate,p);e.set(p);V out;for(int n=0;n<frames;++n){float x=stimulus(n);out.push_back(e.tick({x,x*.73f},p));}return out;}
int main(){std::cout<<std::setprecision(9);std::cout<<"REVERB_CONTROL_MATRIX relative RMS difference (0 = identical rendered audio)\nmodel,decay,tone,motion,size,diffusion,attack,regeneration,shimmer\n";
 using Setter=std::function<void(tide::Parameters&,bool)>;std::vector<Setter> setters={[](auto& p,bool h){p.decay=h?6:.7f;},[](auto& p,bool h){p.reverbTone=h?12000:1500;},[](auto& p,bool h){p.reverbMotion=h?.8f:0;},[](auto& p,bool h){p.size=h?1.5f:.5f;},[](auto& p,bool h){p.diffusion=h?1:0;},[](auto& p,bool h){p.attack=h?1.f:.01f;},[](auto& p,bool h){p.regeneration=h?.6f:0;},[](auto& p,bool h){p.shimmer=h?.6f:0;}};
 const char* names[]={"Room","Plate","Hall","Bloom","Chamber","Cloud","Halo","Glint","StudioPlate","StudioHall","StudioBloom"};
 for(int model=0;model<11;++model){std::cout<<names[model];for(const auto& set:setters){tide::Parameters a;a.delayOn=false;a.reverbType=model<4?model:2;a.reverbModel=model<4?0:model-3;a.shimmer=.3f;a.mix=1;a.reverbVersion=1;a.predelay=0;auto b=a;set(a,false);set(b,true);std::cout<<","<<difference(render(a),render(b));}std::cout<<"\n";}
 std::cout<<"\nSPECIAL_CASES\n";
 for(int type:{0,3,4}){tide::Parameters a;a.sync=false;a.time=100;a.dualMode=1;a.bTime=200;a.delayMotion=0;a.soundVersion=1;a.delayType=type;auto b=a;b.bTime=400;std::cout<<"independent_B_time_delay_type_"<<type<<","<<difference(echo(a),echo(b))<<"\n";}
 for(int model=1;model<=3;++model){tide::Parameters a;a.sync=false;a.time=800;a.delayTone=6000;a.delayModel=model;a.soundVersion=1;a.delayType=0;a.delayMotion=0;auto b=a;b.delayType=2;std::cout<<"hidden_legacy_type_extension_"<<model<<","<<difference(echo(a),echo(b))<<"\n";}
 {tide::Parameters a;a.sync=false;a.time=100;a.delayModel=1;a.soundVersion=1;a.delayMotion=0;a.diffusion=0;auto b=a;b.diffusion=1;std::cout<<"reverb_diffusion_changes_delay,"<<difference(echo(a),echo(b))<<"\n";}
 {tide::Parameters a;a.sync=false;a.time=100;a.delayType=5;a.pitchMode=0;a.soundVersion=1;a.delayMotion=0;a.interval=5;auto b=a;b.delayModel=2;std::cout<<"granular_vs_climbing_plus24,"<<difference(echo(a),echo(b))<<"\n";}
 {tide::Parameters a;a.delayOn=false;a.reverbType=1;a.mix=1;a.predelay=0;a.diffusion=0;a.shimmer=0;auto b=a;b.reverbModel=5;std::cout<<"studio_plate_vs_classic_zero_diffusion,"<<difference(render(a),render(b))<<"\n";}
 {tide::Parameters a;a.delayOn=false;a.reverbModel=4;a.mix=1;a.predelay=0;a.shimmer=0;auto b=a;b.reverbModel=7;std::cout<<"glint_vs_studio_bloom_no_shimmer,"<<difference(render(a),render(b))<<"\n";}
 for(int model:{4,7}){tide::Parameters p;p.shimmer=.3f;p.attack=.7f;tide::BoutiqueTank base,note;base.prepare(rate,model);note.prepare(rate,model);base.set(p);note.set(p);double e=0,d=0;for(int n=0;n<5000;++n){const float x=n<800?stimulus(n):0;auto a=base.tick({x,x*.73f});auto b=note.tick({n==4000?.1f:x,x*.73f});if(n>=4000&&n<4016){e+=double(a.l)*a.l+double(a.r)*a.r;d+=double(b.l)*b.l+double(b.r)*b.r;}}std::cout<<"bloom_tail_ratio_on_new_note_model_"<<model<<","<<std::sqrt(d/std::max(e,1e-30))<<"\n";}
 for(int amount:{0,1}){tide::Parameters p;p.sync=false;p.time=20;p.stereo=0;p.feedback=0;p.delayMotion=0;p.swell=float(amount);p.delayLowcut=20;p.delayTone=16000;tide::Echo e;e.prepare(rate,p);e.set(p);double early=0,late=0;for(int n=0;n<12000;++n){auto y=e.tick({.1f*std::sin(2*tide::pi*220*float(n)/rate),0},p);if(n>=160&&n<320)early+=double(y.l)*y.l;if(n>=8000&&n<8160)late+=double(y.l)*y.l;}std::cout<<"swell_"<<amount<<"_early_late_RMS,"<<std::sqrt(early/160)<<","<<std::sqrt(late/160)<<"\n";}
}
