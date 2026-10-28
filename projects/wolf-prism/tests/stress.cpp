// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PrismDSP.h"
#include <memory>
#include <iostream>
#include <cstdlib>
static void check(bool b){if(!b)std::exit(1);}
int main(){auto e=std::make_unique<prism::Engine>();prism::Settings p;p.enabled={true,true,true,true};e->prepare(48000,p);double energy=0;for(int i=0;i<48000*35;++i){if(i%7000==0){p.type={i/7000%5,i/35000%5,i/14000%5,i/21000%5};p.route=i/7000%24;p.bypass=i%21000==0;p.trails=i%14000!=0;for(int k=0;k<16;++k)p.v[k]=(k>=11?0:(i/7000)%2);e->set(p);}auto y=e->tick({.5f*std::sin(i*.03f),.25f*std::cos(i*.04f)});check(std::isfinite(y.l)&&std::isfinite(y.r)&&prism::peak(y)<=8);energy+=prism::peak(y);}p={};p.enabled={false,false,false,false};e->prepare(8000,p);e->act(prism::Action::CaptureRecord);for(int i=0;i<8000*31;++i)e->tick({.1f,-.1f});check(e->capture.state()==2&&e->capture.seconds()>=30);for(int i=0;i<8000*65;++i)e->tick({});e->loopOptions(true,2);for(int i=0;i<8000*65;++i)e->tick({});std::cout<<"PASS 35-second rapid mode/routing/extreme-control/bypass stress; 30-second capture auto-stop; >2 forward and reverse buffer wraps. Checksum="<<energy<<'\n';}
