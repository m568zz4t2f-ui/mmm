// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline diagnostic; printed measurements are not a release pass verdict.
#include "TideDSP.h"
#include <iostream>
#include <cmath>
int main(){for(int mode=0;mode<9;++mode){tide::Parameters p;p.sync=false;p.time=100;p.feedback=.5f;p.delayMotion=0;p.soundVersion=1;p.stereo=2;p.delayType=mode<6?mode:0;p.delayModel=mode<6?0:mode-5;tide::Echo e;e.prepare(48000,p);e.set(p);double stereo=0,energy=0;for(int n=0;n<48000;++n){const float x=n==0?.5f:0;auto y=e.tick({x,x},p);stereo+=std::pow(double(y.l)-y.r,2);energy+=double(y.l)*y.l+double(y.r)*y.r;}std::cout<<mode<<","<<std::sqrt(stereo/std::max(energy,1e-30))<<"\n";}}
