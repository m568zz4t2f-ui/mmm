// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline diagnostic; printed measurements are not a release pass verdict.
#include "TideDSP.h"
#include <iostream>
#include <cmath>
#include <new>
int main(){for(float sr:{48000.f,192000.f})for(int model:{3,5})for(int control=0;control<2;++control){tide::BoutiqueTank a,b;tide::Parameters p;p.shimmer=.4f;p.reverbMotion=0;p.size=.5f;auto q=p;if(control==0)q.reverbMotion=1;else q.size=1.7f;a.prepare(sr,model);b.prepare(sr,model);a.set(p);b.set(q);double diff=0,energy=0;for(int n=0;n<int(sr);++n){float x=n<int(sr*.1f)?.03f*std::sin(float(n)*.0538f):0;auto l=a.tick({x,x*.73f}),r=b.tick({x,x*.73f});diff+=std::pow(double(l.l)-r.l,2)+std::pow(double(l.r)-r.r,2);energy+=double(l.l)*l.l+double(l.r)*l.r;}std::cout<<sr<<","<<model<<","<<(control==0?"Motion":"Size")<<","<<std::sqrt(diff/std::max(energy,1e-30))<<"\n";}}
