// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline diagnostic; printed measurements are not a release pass verdict.
#include "TideDSP.h"
#include <iostream>
#include <cmath>
int main(){std::cout<<"model,interval_index,peak,first_second_RMS,last_second_RMS\n";for(int model:{3,4})for(int interval=0;interval<6;++interval){tide::Parameters p;p.delayOn=false;p.reverbModel=model;p.reverbInterval=interval;p.shimmer=1;p.regeneration=.7f;p.decay=20;p.size=1.7f;p.reverbMotion=.8f;p.reverbTone=16000;p.reverbDuck=0;p.predelay=0;p.reverbAmount=.8f;p.mix=1;tide::Engine e;e.prepare(16000,p);double first=0,last=0;float peak=0;for(int n=0;n<16000*30;++n){float x=n<16000?.03f*(std::sin(float(n)*.0864f)+.3f*std::sin(float(n)*.267f)):0;auto y=e.tick({x,.73f*x});if(!std::isfinite(y.l)||!std::isfinite(y.r))return 2;peak=std::max(peak,std::max(std::abs(y.l),std::abs(y.r)));if(n<16000)first+=double(y.l)*y.l+double(y.r)*y.r;if(n>=16000*29)last+=double(y.l)*y.l+double(y.r)*y.r;}std::cout<<model<<","<<interval<<","<<peak<<","<<std::sqrt(first/32000)<<","<<std::sqrt(last/32000)<<"\n";}}
