// SPDX-License-Identifier: AGPL-3.0-or-later
#include "ModernDSP.h"
#include <cstdio>
#include <chrono>
using namespace tide::v13;
template<class Machine,class Settings>static void render(const char* kind,int model){Machine m;m.prepare(48000);Settings p;p.model=model;if constexpr(std::is_same<Settings,ReverbSettings>::value)p.shimmer=(model==6||model==7)?.25f:0;m.set(p);m.clear();double energy=0,cross=0,left=0,right=0,late=0;float maximum=0;double worst=0;auto begin=std::chrono::steady_clock::now();
 for(int n=0;n<144000;n+=64){auto start=std::chrono::steady_clock::now();if(n%256==0)m.set(p);for(int j=0;j<64;++j){int k=n+j;float x=k<4800?.08f*(std::sin(k*.057f)+.23f*std::sin(k*.417f)):0;auto y=m.tick({x,x*.37f});energy+=y.l*y.l+y.r*y.r;left+=y.l*y.l;right+=y.r*y.r;cross+=y.l*y.r;maximum=std::max(maximum,peak(y));if(k>=48000)late+=y.l*y.l+y.r*y.r;}worst=std::max(worst,std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count());}
 double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count();std::printf("%s,%d,%.8g,%.8g,%.8g,%.6f,%.3f,%.3f\n",kind,model,maximum,energy,late,cross/std::sqrt(std::max(1e-30,left*right)),ms,worst);
}
int main(){std::puts("kind,model,peak,energy,late_energy,stereo_correlation,render_ms,worst_block_us");for(int i=0;i<12;++i)render<DelayMachine,DelaySettings>("delay",i);for(int i=0;i<13;++i)render<ReverbMachine,ReverbSettings>("reverb",i);}
