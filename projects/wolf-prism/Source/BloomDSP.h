// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>
#include <cstdint>

namespace wolf {
constexpr float pi = 3.14159265358979323846f;
struct Settings {
    float decay=6.0f, predelay=32.0f, tone=6500.0f, motion=0.28f;
    float width=1.0f, mix=0.28f, lowcut=180.0f, output=0.0f;
    bool freeze=false;
};
inline float safe(float v) noexcept { return std::isfinite(v) ? v : 0.0f; }
inline Settings sanitise(Settings s) noexcept {
    s.decay=std::clamp(safe(s.decay),0.25f,20.0f);
    s.predelay=std::clamp(safe(s.predelay),0.0f,200.0f);
    s.tone=std::clamp(safe(s.tone),800.0f,16000.0f);
    s.motion=std::clamp(safe(s.motion),0.0f,1.0f);
    s.width=std::clamp(safe(s.width),0.0f,1.0f);
    s.mix=std::clamp(safe(s.mix),0.0f,1.0f);
    s.lowcut=std::clamp(safe(s.lowcut),20.0f,800.0f);
    s.output=std::clamp(safe(s.output),-18.0f,6.0f); return s;
}
class Delay {
    std::vector<float> data;
    // Logical clearing bounds audio-thread work, even for a 16-second buffer.
    std::vector<std::uint64_t> stamps;
    std::uint64_t generation=1;
    std::size_t pos=0;
    float at(std::size_t i) const noexcept { return stamps[i/64]==generation?data[i]:0.f; }
public:
    void prepare(std::size_t n) { data.assign(std::max<std::size_t>(n,4),0);stamps.assign((data.size()+63)/64,0);generation=1;pos=0; }
    void clear() noexcept { ++generation;pos=0; }
    void push(float x) noexcept {
        const auto block=pos/64;
        if(stamps[block]!=generation){const auto first=block*64;std::fill(data.begin()+std::ptrdiff_t(first),data.begin()+std::ptrdiff_t(std::min(first+64,data.size())),0.f);stamps[block]=generation;}
        data[pos]=x;if(++pos==data.size())pos=0;
    }
    float readInt(std::size_t n) const noexcept {
        n=std::min(n,data.size()-2);const auto back=n+1;
        return at(pos>=back?pos-back:pos+data.size()-back);
    }
    // Relative to the most recently written sample; zero reads that sample.
    float read(float n) const noexcept {
        n=std::clamp(n,0.0f,float(data.size()-2));
        double p=double(pos)-1.0-double(n); if(p<0) p+=double(data.size());
        if(p>=double(data.size())) p-=double(data.size());
        auto a=std::size_t(p); auto b=a+1; if(b==data.size()) b=0;
        const float x=at(a);return x+float(p-double(a))*(at(b)-x);
    }
    float readCubic(float n) const noexcept {
        n=std::clamp(n,1.f,float(data.size()-3));
        double p=double(pos)-1-double(n);if(p<0)p+=double(data.size());
        const auto b=std::size_t(p),a=b==0?data.size()-1:b-1,c=b+1==data.size()?0:b+1,d=c+1==data.size()?0:c+1;
        const float t=float(p-double(b)),x0=at(a),x1=at(b),x2=at(c),x3=at(d);
        return x1+.5f*t*(x2-x0+t*(2*x0-5*x1+4*x2-x3+t*(3*(x1-x2)+x3-x0)));
    }
};
struct Allpass {
    Delay line; float length=1;
    void prepare(float sr,float ms) { length=std::max(1.0f,std::round(sr*ms*0.001f)); line.prepare(std::size_t(length)+3); }
    float tick(float x) noexcept {
        const auto d=line.readInt(std::size_t(length-1)), y=d-0.65f*x;
        line.push(x+0.65f*y); return y;
    }
};
struct OnePole { float z=0; float tick(float x,float a) noexcept { z=(1-a)*x+a*z; return z; } };
class BloomDSP {
    float sr=48000, smooth=0, delayFade=1, oldDelay=0, newDelay=0, wantedDelay=0;
    std::array<Delay,8> tank;
    std::array<Delay,2> pre;
    std::array<std::array<Allpass,4>,2> diffusion;
    std::array<float,8> lengths{},phase{},feedback{},feedbackTarget{},oscSin{},oscCos{},rotSin{},rotCos{};
    std::array<OnePole,8> damping;
    std::array<OnePole,2> inputBass,outputBass;
    Settings current{},target{};
    float dampA=0,dampTarget=0,bassA=0,bassTarget=0,outputGain=1,gainTarget=1;
    unsigned controlClock=0;
    void coefficients() noexcept {
        dampTarget=target.freeze?0.f:std::exp(-2*pi*std::min(target.tone,sr*0.45f)/sr);
        bassTarget=std::exp(-2*pi*target.lowcut/sr);
        gainTarget=std::pow(10.0f,target.output/20.0f);
        for(std::size_t i=0;i<8;++i) feedbackTarget[i]=target.freeze?1.f:std::pow(10.0f,-3.0f*(lengths[i]/sr)/target.decay);
    }
public:
    static std::array<float,8> scatter(std::array<float,8> v) noexcept {
        for(std::size_t step=1;step<8;step*=2)
            for(std::size_t base=0;base<8;base+=step*2)
                for(std::size_t j=0;j<step;++j) { float a=v[base+j],b=v[base+j+step]; v[base+j]=a+b;v[base+j+step]=a-b; }
        for(auto& x:v)x*=0.3535533905932738f;
        return v;
    }
    void prepare(double sampleRate,Settings initial={},bool expansive=false) {
        sr=float(std::clamp(sampleRate,8000.0,384000.0));
        current=target=sanitise(initial); smooth=1-std::exp(-1.0f/(0.04f*sr));
        constexpr float ms[8]={31.1f,37.7f,43.3f,53.1f,61.7f,71.3f,79.7f,89.3f};
        constexpr float diff[4]={4.3f,7.9f,12.7f,19.3f};
        for(std::size_t i=0;i<8;++i) {
            lengths[i]=std::round(ms[i]*(expansive?(1.45f+float(i%3)*.13f):1.f)*0.001f*sr);
            tank[i].prepare(std::size_t(lengths[i]+sr*0.003f)+8);
            phase[i]=float(i)*0.733f; oscSin[i]=std::sin(phase[i]);oscCos[i]=std::cos(phase[i]);
            const float angle=2*pi*(0.09f+float(i)*0.019f)/sr;
            rotSin[i]=std::sin(angle);rotCos[i]=std::cos(angle); damping[i].z=0;
        }
        for(std::size_t c=0;c<2;++c) { pre[c].prepare(std::size_t(sr*0.205f)+8);inputBass[c].z=outputBass[c].z=0;
            for(std::size_t j=0;j<4;++j)diffusion[c][j].prepare(sr,diff[j]*(c==0?1.0f:1.113f)*(expansive?1.7f:1.f)); }
        oldDelay=newDelay=wantedDelay=current.predelay*0.001f*sr;delayFade=1; controlClock=0;
        coefficients();feedback=feedbackTarget;dampA=dampTarget;bassA=bassTarget;outputGain=gainTarget;
    }
    void reset() noexcept {
        for(auto& d:tank)d.clear();
        for(auto& d:pre)d.clear();
        for(auto& channel:diffusion)for(auto& a:channel)a.line.clear();
        for(auto& f:damping)f.z=0;
        for(auto& f:inputBass)f.z=0;
        for(auto& f:outputBass)f.z=0;
    }
    void set(Settings s) noexcept { target=sanitise(s); wantedDelay=target.predelay*0.001f*sr; coefficients(); }
    void process(float* left,float* right,int count,bool embedded=false) noexcept {
        constexpr float injectL[8]={1,1,-1,1,-1,-1,1,-1};
        constexpr float injectR[8]={1,-1,1,1,-1,1,-1,-1};
        constexpr float tapL[8]={1,1,1,-1,1,-1,-1,-1};
        constexpr float tapR[8]={1,-1,1,-1,-1,1,-1,1};
        for(int n=0;n<count;++n) {
            const float dryL=safe(left[n]),dryR=safe(right[n]);
            auto follow=[this](float& v,float t){v+=smooth*(t-v);};
            follow(current.motion,target.motion);follow(dampA,dampTarget);
            if(!embedded){follow(current.mix,target.mix);follow(current.width,target.width);follow(bassA,bassTarget);follow(outputGain,gainTarget);}
            if(delayFade>=1 && std::abs(wantedDelay-newDelay)>0.25f) { oldDelay=newDelay;newDelay=wantedDelay;delayFade=0; }
            delayFade=std::min(1.0f,delayFade+1.0f/(0.03f*sr));
            float input[2]={dryL,dryR};
            for(std::size_t c=0;c<2;++c) {
                if(!embedded){input[c]-=inputBass[c].tick(input[c],bassA);pre[c].push(input[c]);input[c]=(1-delayFade)*pre[c].read(oldDelay)+delayFade*pre[c].read(newDelay);}
                for(auto& ap:diffusion[c])input[c]=ap.tick(input[c]);
            }
            std::array<float,8> delayed{}; float wetL=0,wetR=0;
            for(std::size_t i=0;i<8;++i) {
                const float sinNext=oscSin[i]*rotCos[i]+oscCos[i]*rotSin[i];
                oscCos[i]=oscCos[i]*rotCos[i]-oscSin[i]*rotSin[i];oscSin[i]=sinNext;
                float delay=lengths[i]-1+current.motion*sr*0.0007f*oscSin[i];
                delayed[i]=damping[i].tick(tank[i].read(delay),dampA);
                wetL+=tapL[i]*delayed[i]*0.35355339f;wetR+=tapR[i]*delayed[i]*0.35355339f;
            }
            auto mixed=scatter(delayed);
            for(std::size_t i=0;i<8;++i) {
                follow(feedback[i],feedbackTarget[i]);
                tank[i].push(feedback[i]*mixed[i]+0.25f*(injectL[i]*input[0]+injectR[i]*input[1]));
            }
            if(embedded){left[n]=safe(wetL);right[n]=safe(wetR);}
            else{
            wetL-=outputBass[0].tick(wetL,bassA);wetR-=outputBass[1].tick(wetR,bassA);
            const float mid=0.5f*(wetL+wetR), side=0.5f*(wetL-wetR)*current.width;
            // Linear crossfade preserves dry level and makes mix=1 a true send return.
            left[n]=safe(((1-current.mix)*dryL+current.mix*(mid+side))*outputGain);
            right[n]=safe(((1-current.mix)*dryR+current.mix*(mid-side))*outputGain);
            }
            // Bound oscillator round-off over multi-hour sessions, without trig in the sample loop.
            if((++controlClock&4095u)==0) for(std::size_t i=0;i<8;++i) {
                float norm=1/std::sqrt(oscSin[i]*oscSin[i]+oscCos[i]*oscCos[i]);oscSin[i]*=norm;oscCos[i]*=norm;
            }
        }
    }
};
} // namespace wolf