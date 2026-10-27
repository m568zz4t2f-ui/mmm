// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline diagnostic; printed measurements are not a release pass verdict.
#include "TideDSP.h"
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
using V=std::vector<tide::Stereo>;
V render(tide::Parameters p){tide::Echo e;e.prepare(16000,p);e.set(p);V v;v.reserve(48000);for(int n=0;n<48000;++n){float x=n%9600<3600?.035f*(std::sin(float(n)*.0538f)+.4f*std::sin(float(n)*.392f)):0;v.push_back(e.tick({x,.73f*x},p));}return v;}
double diff(const V& a,const V& b){double e=0,d=0;for(int n=0;n<int(a.size());++n){e+=double(a[n].l)*a[n].l+double(a[n].r)*a[n].r;d+=std::pow(double(a[n].l)-b[n].l,2)+std::pow(double(a[n].r)-b[n].r,2);}return std::sqrt(d/std::max(e,1e-30));}
int main(){using Setter=std::function<void(tide::Parameters&,bool)>;std::vector<Setter> s={[](auto& p,bool h){p.time=h?280:80;},[](auto& p,bool h){p.feedback=h?.7f:.1f;},[](auto& p,bool h){p.delayTone=h?12000:1500;},[](auto& p,bool h){p.delayMotion=h?.8f:0;},[](auto& p,bool h){p.drive=h?1:0;},[](auto& p,bool h){p.ratio=h?1.5f:.5f;},[](auto& p,bool h){p.dualMode=1;p.bTime=h?400:200;},[](auto& p,bool h){p.diffusion=h?1:0;},[](auto& p,bool h){p.interval=h?5:2;},[](auto& p,bool h){p.swell=h?1:0;}};
const char* names[]={"Digital","Tape","Analog","Pattern","Reverse","Pitch","Diffusion","Granular","LoFi"};std::cout<<"model,time,feedback,tone,motion,drive,ratio,B_time,diffusion,interval,swell\n";for(int i=0;i<9;++i){std::cout<<names[i];for(const auto& set:s){tide::Parameters a;a.soundVersion=1;a.sync=false;a.time=180;a.stereo=1;a.delayModel=i<6?0:i-5;a.delayType=i<6?i:0;a.delayMotion=0;a.pitchMode=1;auto b=a;set(a,false);set(b,true);std::cout<<","<<diff(render(a),render(b));}std::cout<<"\n";}
}
