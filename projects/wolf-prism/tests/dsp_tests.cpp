// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PrismDSP.h"
#include <iostream>
#include <memory>
#include <chrono>
#include <cstdlib>
#include <new>
static bool watch=false;static std::size_t allocations=0;
void* operator new(std::size_t n){if(watch)++allocations;if(auto* p=std::malloc(n))return p;throw std::bad_alloc();}void operator delete(void* p)noexcept{std::free(p);}void operator delete(void* p,std::size_t)noexcept{std::free(p);}
static void check(bool b,const char* s){if(!b){std::cerr<<"FAIL "<<s<<'\n';std::exit(1);}}
int main(int argc,char**){auto engine=std::make_unique<prism::Engine>();double energy=0;for(int sr:{44100,48000,96000,192000}){prism::Settings p;p.enabled={false,false,false,false};engine->prepare(sr,p);for(int n=0;n<2048;++n){float x=.25f*std::sin(n*.1f);auto y=engine->tick({x,-x});check(y.l==x&&y.r==-x,"all-disabled dry identity");}for(int module=0;module<4;++module)for(int effect=0;effect<5;++effect){p.enabled={false,false,false,false};p.enabled[module]=true;p.type[module]=effect;p.v[prism::CharacterAmount+module]=.85f;engine->set(p);watch=true;for(int n=0;n<sr/3;++n){float x=n<sr/5?.2f*std::sin(float(n)*2*prism::pi*220/sr):0;auto y=engine->tick({x,x*.61f});check(std::isfinite(y.l)&&std::isfinite(y.r)&&prism::peak(y)<=8,"all modes finite bounded");energy+=prism::peak(y);}watch=false;}std::cout<<"PASS 20 modes at "<<sr<<" Hz\n";}
 prism::Settings p;p.v[prism::Mix]=0;engine->prepare(48000,p);for(int n=0;n<10000;++n){auto y=engine->tick({.25f,-.5f});check(y.l==.25f&&y.r==-.5f,"mix zero bit-exact dry");}p.v[prism::Mix]=1;p.v[prism::DiffusionAmount]=1;p.enabled={false,false,true,false};engine->prepare(48000,p);auto y=engine->tick({1,0});check(prism::peak(y)<1e-9f,"space full wet removes direct input");
 for(int n=0;n<48000;++n)engine->tick({});p.bypass=true;p.trails=true;engine->set(p);double tails=0;for(int n=0;n<48000;++n)tails+=prism::peak(engine->tick({}));check(tails>1e-5,"bypass trails remain");
 engine->prepare(48000,{});engine->act(prism::Action::CaptureRecord);for(int i=0;i<4800;++i)engine->tick({.2f,.1f});engine->act(prism::Action::CapturePlay);check(engine->capture.seconds()>.09,"capture duration");auto snap=engine->capture.snapshot();engine->act(prism::Action::CaptureDub);watch=true;for(int i=0;i<10000;++i)engine->tick({.1f,.1f});engine->act(prism::Action::CaptureUndo);watch=false;check(allocations==0,"zero core callback heap allocations");engine->capture.restore(snap);check(engine->capture.state()==4,"restored capture stopped");
 p={};p.gestureRecord=true;engine->set(p);for(int i=0;i<48000;++i)engine->tick({});p.gestureRecord=false;engine->set(p);engine->tick({});check(engine->gesture.available&&engine->gesture.count>=99,"gesture recording");
 for(int i=0;i<24;++i){auto o=prism::order(i);auto sorted=o;std::sort(sorted.begin(),sorted.end());check(sorted==std::array<int,4>{0,1,2,3},"valid permutations");p.route=i;engine->set(p);for(int n=0;n<2000;++n)check(prism::peak(engine->tick({.1f,-.1f}))<=8,"routing finite");}
 p.v[0]=std::numeric_limits<float>::quiet_NaN();engine->set(p);for(int n=0;n<4000;++n)check(prism::peak(engine->tick({std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}))<=8,"invalid audio sanitised");
 if(argc>1){p={};p.type={3,2,2,2};p.enabled={true,true,true,true};engine->prepare(48000,p);auto begin=std::chrono::steady_clock::now();for(int n=0;n<48000*5;++n)energy+=engine->tick({.2f*std::sin(n*.03f),.1f*std::cos(n*.04f)}).l;auto ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();std::cout<<"BENCH 5 seconds DSP in "<<ms<<" ms; real-time CPU ratio "<<ms/50<<" percent; checksum "<<energy<<'\n';}
 std::cout<<"PASS dry identity, 20 modes, sample rates, true wet Space, trails, capture, undo, Gesture, 24 routes, invalid input, no core heap allocations.\n";
}
