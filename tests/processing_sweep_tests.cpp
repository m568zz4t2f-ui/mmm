// SPDX-License-Identifier: AGPL-3.0-or-later
#include "ModernDSP.h"
#include <cstdio>
#include <cstdlib>
using namespace tide::v13;
static void check(bool ok,const char* message){if(!ok){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
int main(){
    // Independent tonal contracts catch the original cancellation/sideband defect.
    for(float semi:{-12.f,-7.f,7.f,12.f,19.f,24.f})for(float texture:{0.f,.5f,1.f})for(double hz:{82.4069,110.,440.}){
        Pitch p;p.prepare(48000);p.set(semi,texture);p.clear();double e=0,re=0,im=0;constexpr int count=96000;
        const double target=hz*std::pow(2.,semi/12.);
        for(int n=0;n<count;++n){float x=.1f*std::sin(2*3.141592653589793*hz*n/48000);auto y=p.tick({x,-x*.6f});check(std::isfinite(y.l)&&std::isfinite(y.r),"pitch finite");if(n>=48000){e+=y.l*y.l;re+=y.l*std::cos(2*3.141592653589793*target*n/48000);im+=y.l*std::sin(2*3.141592653589793*target*n/48000);check(std::abs(y.r+.6f*y.l)<2e-5f,"linked stereo trajectories");}}
        double rms=std::sqrt(2*e/48000)/.1,component=2*std::hypot(re,im)/48000/.1;
        if(rms<.8||rms>1.15||component<.75)std::fprintf(stderr,"pitch semi=%g texture=%g hz=%g rms=%g target=%g\n",semi,texture,hz,rms,component);
        check(rms>.8&&rms<1.15,"pitch note level within measured acceptance band");check(component>.75,"requested pitch component retained");
    }
    {DelayMachine a,b;a.prepare(48000);b.prepare(48000);DelaySettings p;p.model=0;p.time=p.rightTime=20;p.feedback=p.rightFeedback=.8f;p.motion=0;a.set(p);b.set(p);a.clear();b.clear();
     double error=0;for(int n=0;n<24000;++n){float x=n<400?.01f*std::sin(n*.17f):0;auto lo=a.tick({x,0}),hi=b.tick({x*10,0});error+=std::abs(hi.l-10*lo.l);}check(error<.002,"Digital feedback retains small-signal linearity");}
    {ReverbMachine a,b;a.prepare(48000);b.prepare(48000);ReverbSettings p;p.model=6;p.predelay=0;p.motion=0;p.decay=8;p.regeneration=.5f;p.shimmer=0;a.set(p);a.clear();p.shimmer=.001f;b.set(p);b.clear();double ea=0,eb=0;
     for(int n=0;n<144000;++n){tide::Stereo in{n==0?.1f:0,0};auto x=a.tick(in),y=b.tick(in);if(n>=96000){ea+=x.l*x.l+x.r*x.r;eb+=y.l*y.l+y.r*y.r;}}double ratio=eb/ea;std::printf("shimmer late-energy ratio %.6f\n",ratio);check(ratio>.8&&ratio<1.2,"near-zero shimmer preserves base-tail continuity");}
    {ReverbMachine r;r.prepare(48000);ReverbSettings p;p.model=11;p.predelay=0;p.attack=.1f;p.decay=4;p.shape=.5f;r.set(p);r.clear();double early=0,late=0;
     for(int n=0;n<48000;++n){float x=.1f*std::sin(n*.057f);auto y=r.tick({x,x});if(n>2400&&n<6000)early+=y.l*y.l+y.r*y.r;if(n>33600)late+=y.l*y.l+y.r*y.r;}check(early>1e-8&&late<early*.001,"Nonlinear gate closes on sustained input after its onset window");}
    {auto repeatRatio=[](float tone){DelayMachine d;d.prepare(48000);DelaySettings p;p.model=3;p.time=p.rightTime=40;p.feedback=p.rightFeedback=.7f;p.tone=tone;p.lowcut=5;for(auto& t:p.taps){t.level=0;t.feedback=0;}p.taps[3].level=1;p.taps[3].feedback=1;d.set(p);d.clear();double first=0,second=0;for(int n=0;n<6000;++n){float x=n<240?.1f*std::sin(2*3.141592653589793*4000*n/48000):0;auto y=d.tick({x,0});if(n>=1920&&n<2880)first+=y.l*y.l;if(n>=3840&&n<4800)second+=y.l*y.l;}return second/first;};check(repeatRatio(700)<repeatRatio(16000)*.25,"Multitap tone filters successive feedback passes");}
    {DelayMachine fresh,reused;DelaySettings p;p.model=10;p.motion=0;reused.prepare(48000);reused.set(p);reused.clear();for(int n=0;n<1000;++n)reused.tick({.1f,0});reused.prepare(8000);fresh.prepare(8000);fresh.set(p);fresh.clear();double difference=0;for(int n=0;n<8000;++n){float x=.03f*std::sin(n*.31f);auto a=fresh.tick({x,0}),b=reused.tick({x,0});difference+=std::abs(a.l-b.l);}check(difference<1e-4,"Spectral coefficients refresh after a sample-rate change");}
    // Sample-rate edge: out-of-band partials must not collapse into duplicate combs.
    {ReverbMachine r;r.prepare(8000);ReverbSettings p;p.model=12;p.tune=1000;p.size=.5f;p.predelay=0;p.decay=.5f;p.tone=3500;r.set(p);r.clear();double energy=0;for(int n=0;n<8000;++n){auto y=r.tick({n==0?.1f:0,0});check(std::isfinite(y.l)&&std::isfinite(y.r),"modal resonator finite at rate edge");energy+=y.l*y.l+y.r*y.r;}check(energy>1e-10,"in-band resonator modes survive high tuning");}
    std::puts("PASS: 54 pitch fixtures, stereo linkage, clean feedback, Multitap filtering, Spectral rate change, shimmer continuity, gated sustain and resonator rate edge.");
}
