// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TideDSP.h"
#include <cstdint>
#include <type_traits>

// Original 1.3 development engine. The 1.2.2 engine remains in TideDSP.h.
namespace tide::v13 {
inline Stereo operator+(Stereo a,Stereo b) noexcept{return {a.l+b.l,a.r+b.r};}
inline Stereo operator*(Stereo a,float b) noexcept{return {a.l*b,a.r*b};}
inline float peak(Stereo x) noexcept{return std::max(std::abs(x.l),std::abs(x.r));}
inline float bounded(float x,float lo,float hi,float fallback) noexcept{return std::isfinite(x)?std::clamp(x,lo,hi):fallback;}
inline float lerp(float a,float b,float t) noexcept{return a+(b-a)*t;}
inline Stereo blend(Stereo a,Stereo b,float t) noexcept{return a*(1-t)+b*t;}
inline float clean(float x) noexcept{return std::isfinite(x)?std::clamp(x,-8.f,8.f):0.f;}
inline Stereo clean(Stereo x) noexcept{return {clean(x.l),clean(x.r)};}
inline float protect(float x) noexcept{const float a=std::abs(x);return a<=.98f?x:std::copysign(.98f+.02f*std::tanh((a-.98f)/.02f),x);}
inline constexpr const char* delayNames[]={"Digital","Tape","Bucket","Multitap","Reverse","Pitch","Diffusion","Granular Pitch","Lo-Fi","Oil Can","Spectral","Magnetic Drum"};
inline constexpr const char* reverbNames[]={"Room","Plate","Hall","Bloom","Chamber","Cloud Ensemble","Halo Shimmer","Glint","Studio Plate","Studio Hall","Studio Bloom","Nonlinear","Resonator"};
struct Noise {
    std::uint32_t state=0x9e3779b9;
    float tick() noexcept{state=state*1664525u+1013904223u;return float(state>>8)*(2.f/16777216.f)-1;}
};
struct Oscillator {
    float s=0,c=1,rs=0,rc=1;unsigned count=0;
    void set(float hz,float sr){const float angle=2*pi*hz/sr;rs=std::sin(angle);rc=std::cos(angle);}
    float tick() noexcept{const float next=s*rc+c*rs;c=c*rc-s*rs;s=next;if((++count&4095u)==0){const float norm=1/std::max(1e-9f,std::sqrt(s*s+c*c));s*=norm;c*=norm;}return s;}
};
struct Diffuser {
    wolf::Delay line;float length=1;
    void prepare(float sr,float maxMs){line.prepare(std::size_t(sr*maxMs*.001f)+8);}
    void clear(){line.clear();}
    float tick(float x,float samples,float feedback=.65f) noexcept{const float d=line.read(std::max(1.f,samples-1)),y=d-feedback*x;line.push(clean(x+feedback*y));return y;}
};
struct Bandpass {
    float b0=0,b2=0,a1=0,a2=0,z1=0,z2=0;
    void set(float hz,float q,float sr){const float w=2*pi*std::clamp(hz,20.f,sr*.42f)/sr,alpha=std::sin(w)/(2*q),norm=1/(1+alpha);b0=alpha*norm;b2=-b0;a1=-2*std::cos(w)*norm;a2=(1-alpha)*norm;}
    float tick(float x) noexcept{const float y=b0*x+z1;z1=-a1*y+z2;z2=b2*x-a2*y;return y;}
    void clear(){z1=z2=0;}
};
// Linked stereo variable-speed reader with waveform-matched, short splices.
// Search is bounded independently of sample rate; no allocation on a splice.
class Pitch {
    std::array<wolf::Delay,2> source,halfSource,quarterSource;
    std::array<std::array<tide::Lowpass,2>,2> filter,halfFilter,quarterFilter;
    float sr=48000,ratio=1,target=1,window=4560,wantedWindow=4560;
    float readDelay=2400,oldDelay=2400,fade=1,smooth=0;
    float recorded=0;
    float sample(std::size_t c,float delay)const noexcept {
        // Continuously maintained bandwidth histories prevent old broadband samples
        // from bypassing anti-alias filtering after an upward interval change.
        if(ratio<=1)return source[c].readCubic(delay);
        if(ratio<1.1f)return lerp(source[c].readCubic(delay),halfSource[c].readCubic(delay),(ratio-1)*10);
        if(ratio<=2)return halfSource[c].readCubic(delay);
        if(ratio<2.2f)return lerp(halfSource[c].readCubic(delay),quarterSource[c].readCubic(delay),(ratio-2)*5);
        return quarterSource[c].readCubic(delay);
    }
    float splice(float from,float proposed) noexcept {
        constexpr int points=24;std::array<Stereo,points> reference{};
        const float stride=std::max(1.f,sr/4000.f);
        double energy=0;
        for(int j=0;j<points;++j){float d=from+float(j)*stride;reference[j]={sample(0,d),sample(1,d)};energy+=reference[j].l*reference[j].l+reference[j].r*reference[j].r;}
        if(energy<1e-14)return proposed;
        const float radius=std::min(sr*.014f,window*.4f),step=std::max(1.f,sr/6000.f);
        float best=proposed;double bestCost=1e30;
        auto cost=[&](float d){double e=0;for(int j=0;j<points;++j){float offset=d+float(j)*stride;float l=sample(0,offset)-reference[j].l,r=sample(1,offset)-reference[j].r;e+=l*l+r*r;}return e/energy+1e-5*std::abs(d-proposed)/std::max(1.f,radius);};
        const float lo=std::max(32.f,proposed-radius),hi=std::min(sr*.19f,proposed+radius);
        for(float d=lo;d<=hi;d+=step){double c=cost(d);if(c<bestCost){bestCost=c;best=d;}}
        const float centre=best;
        for(float d=std::max(lo,centre-step);d<=std::min(hi,centre+step);d+=std::max(.25f,sr/192000.f)){double c=cost(d);if(c<bestCost){bestCost=c;best=d;}}
        return best;
    }
public:
    void prepare(float rate){sr=rate;smooth=1-std::exp(-1/(sr*.025f));for(auto* bank:{&source,&halfSource,&quarterSource})for(auto& d:*bank)d.prepare(std::size_t(sr*.24f)+64);for(std::size_t c=0;c<2;++c){filter[c][0].set(sr*.43f,sr,.5411961f);filter[c][1].set(sr*.43f,sr,1.306563f);halfFilter[c][0].set(sr*.215f,sr,.5411961f);halfFilter[c][1].set(sr*.215f,sr,1.306563f);quarterFilter[c][0].set(sr*.1075f,sr,.5411961f);quarterFilter[c][1].set(sr*.1075f,sr,1.306563f);}set(0,.5f);clear();}
    void clear(){for(auto* bank:{&source,&halfSource,&quarterSource})for(auto& d:*bank)d.clear();for(auto* bank:{&filter,&halfFilter,&quarterFilter})for(auto& row:*bank)for(auto& f:row)f.clear();ratio=target;window=wantedWindow;readDelay=oldDelay=32+window*.5f;fade=1;recorded=0;}
    void set(float semi,float texture){wantedWindow=sr*(.035f+.12f*bounded(texture,0,1,.5f));target=std::pow(2.f,bounded(semi,-24,24,0)/12);}
    Stereo tick(Stereo in) noexcept {
        ratio+=smooth*(target-ratio);
        const float input[]={in.l,in.r};for(std::size_t c=0;c<2;++c){source[c].push(filter[c][1].tick(filter[c][0].tick(input[c])));halfSource[c].push(halfFilter[c][1].tick(halfFilter[c][0].tick(input[c])));quarterSource[c].push(quarterFilter[c][1].tick(quarterFilter[c][0].tick(input[c])));}recorded=std::min(sr*.23f,recorded+1);
        const float speed=1-ratio,fadeSamples=sr*.004f,margin=32+std::abs(speed)*fadeSamples;
        if(fade>=1&&((speed<-.00001f&&readDelay<margin)||(speed>.00001f&&readDelay>32+window-margin))){
            oldDelay=readDelay;window=wantedWindow;
            float proposed=std::clamp(readDelay+(speed<0?window:-window),margin,32+window);
            readDelay=recorded>sr*.2f?splice(oldDelay,proposed):proposed;fade=0;
        }
        auto read=[&](float d){return Stereo{sample(0,d),sample(1,d)};};
        Stereo out=read(readDelay);
        if(fade<1){const float t=fade*fade*(3-2*fade);out=blend(read(oldDelay),out,t);fade=std::min(1.f,fade+1/fadeSamples);oldDelay+=speed;}
        readDelay+=speed;return clean(out);
    }
};
struct Tap {float time=1,level=.25f,pan=0,feedback=.5f;bool operator==(const Tap& o)const{return std::tie(time,level,pan,feedback)==std::tie(o.time,o.level,o.pan,o.feedback);}};
struct DelaySettings {
    int model=0,stereo=0,pitchMode=1;bool on=true,freeze=false;
    float time=375,rightTime=375,feedback=.32f,rightFeedback=.32f,rightLevel=1,tone=11000,lowcut=35,motion=0,rate=.4f,width=1,drive=.15f;
    float diffusion=.7f,age=.25f,texture=.5f,density=.5f,position=.2f,bits=12,reduction=.2f,noise=0,attack=.25f,interval=12,spread=.5f;
    auto values()const{return std::tie(model,stereo,pitchMode,on,freeze,time,rightTime,feedback,rightFeedback,rightLevel,tone,lowcut,motion,rate,width,drive,diffusion,age,texture,density,position,bits,reduction,noise,attack,interval,spread,taps);}
    bool operator==(const DelaySettings& o)const{return values()==o.values();}
    std::array<Tap,4> taps{{{.25f,.25f,-.7f,.5f},{.5f,.25f,.7f,.5f},{.75f,.25f,-.3f,.5f},{1,.25f,.3f,.5f}}};
};
struct ReverbSettings {
    int model=2;bool on=true,freeze=false;
    float bass=0,decay=4,predelay=28,tone=6500,lowcut=100,motion=.2f,width=1,size=1,diffusion=.7f,attack=.2f,regeneration=.2f,shimmer=0,interval=12;
    float early=.35f,damping=.5f,texture=.5f,tune=110,shape=.5f;
    auto values()const{return std::tie(model,on,freeze,bass,decay,predelay,tone,lowcut,motion,width,size,diffusion,attack,regeneration,shimmer,interval,early,damping,texture,tune,shape);}
    bool operator==(const ReverbSettings& o)const{return values()==o.values();}
};
// Owned capture history; freeze stops the writer, not merely the live input.
class CaptureGrains {
    struct Voice {float offset=0,age=0,length=0,rate=1;bool active=false;};
    std::array<Voice,12> voices{};std::array<wolf::Delay,2> source;
    std::array<std::array<tide::Lowpass,2>,2> filters;
    float sr=48000,recorded=0,countdown=0,lastInterval=999,ratio=1,gain=.5f,smooth=0,lastSpread=-1,harmony=1;
    unsigned sequence=0;
public:
    void prepare(float rate){sr=rate;hannTable();smooth=1-std::exp(-1/(sr*.03f));for(auto& d:source)d.prepare(std::size_t(sr*2.1f)+32);lastInterval=999;clear();}
    void clear(){for(auto& d:source)d.clear();for(auto& row:filters)for(auto& f:row)f.clear();voices={};recorded=countdown=0;sequence=0;}
    Stereo tick(Stereo input,const DelaySettings& p) noexcept{
        if(lastInterval!=p.interval){lastInterval=p.interval;ratio=std::pow(2.f,std::clamp(p.interval,-24.f,24.f)/12);const float cutoff=sr*.43f/std::max(1.f,ratio*1.5f);for(auto& row:filters){row[0].set(cutoff,sr,.5411961f);row[1].set(cutoff,sr,1.306563f);}}
        if(!p.freeze){source[0].push(filters[0][1].tick(filters[0][0].tick(input.l)));source[1].push(filters[1][1].tick(filters[1][0].tick(input.r)));recorded=std::min(recorded+1,sr*2.f);}
        if(lastSpread!=p.spread){lastSpread=p.spread;harmony=std::pow(1.5f,p.spread);}
        const float overlap=2+std::clamp(p.density,0.f,1.f)*6;
        if(--countdown<=0){for(auto& v:voices)if(!v.active){const float speed=ratio*((sequence++%3==2)?harmony:1.f);const float duration=std::min(sr*(.025f+.16f*p.texture),(recorded-64)/std::max(1.f,speed));
            if(duration>=sr*.008f){const float needed=32+(p.freeze?speed:std::max(0.f,speed-1))*duration;const float room=std::max(0.f,recorded-needed-8);v={needed+room*std::clamp(p.position,0.f,1.f)*.7f,0,duration,speed,true};countdown=duration/overlap;}else countdown=sr*.005f;break;}}
        gain+=smooth*(2/overlap-gain);Stereo wet{};float weightSum=0;
        for(auto& v:voices)if(v.active){const float w=hannWeight(hannTable(),v.age/v.length)*gain;weightSum+=w;wet.l+=source[0].readCubic(v.offset)*w;wet.r+=source[1].readCubic(v.offset)*w;v.offset+=(p.freeze?0.f:1.f)-v.rate;if(++v.age>=v.length||v.offset<1||v.offset>recorded-4)v.active=false;}
        return wet*(1/std::max(1.f,weightSum));
    }
};
class DelayMachine {
    std::array<wolf::Delay,2> line;
    std::array<std::array<Diffuser,6>,2> diffusion;
    std::array<wolf::OnePole,2> tone,bass,clockFilter,expansion,returnTone,returnBass,reconstruction;
    std::array<std::array<wolf::OnePole,4>,2> headLoss;std::array<float,4> headPole{};
    std::array<std::array<Bandpass,6>,2> spectral;
    Pitch pitch; CaptureGrains granular;
    Noise random;Oscillator wow,flutter;wolf::OnePole irregular;
    DelaySettings p{};bool configured=false;
    float sr=48000,smooth=0,time=18000,right=18000,oldTime=18000,oldRight=18000,targetTime=18000,targetRight=18000,fade=1;
    float feedback=.32f,rightFeedback=.32f,width=1,tonePole=0,bassPole=0,clockPole=0,level=1,modulation=0,drive=.15f,clock=0,heldL=0,heldR=0,recorded=0;
    std::array<float,2> reversePhase{},reverseOldPhase{};float clockRate=1,heldNoise=0,noiseLevel=0,tapePole=0,lofiPeriod=1,lofiSteps=2048,lastSpectral=-1,pitchBlend=1;
    float toneCurrent=0,bassCurrent=0,oilPole=0,tapeCurrent=0,oilCurrent=0,clockCurrent=0,clockRateCurrent=1;
public:
    void prepare(float rate){sr=rate;smooth=1-std::exp(-1/(sr*.03f));for(auto& d:line)d.prepare(std::size_t(sr*16.1f)+16);for(auto& row:diffusion)for(auto& d:row)d.prepare(sr,70);pitch.prepare(sr);granular.prepare(sr);lastSpectral=-1;configured=false;set(p);clear();}
    void clear(){for(auto& d:line)d.clear();for(auto& row:diffusion)for(auto& d:row)d.clear();tone={};bass={};clockFilter={};expansion={};returnTone={};returnBass={};reconstruction={};headLoss={};for(auto& row:spectral)for(auto& f:row)f.clear();pitch.clear();granular.clear();recorded=clock=0;heldL=heldR=0;reversePhase={};reverseOldPhase={};time=oldTime=targetTime;right=oldRight=targetRight;feedback=p.feedback;rightFeedback=p.rightFeedback;width=p.width;drive=p.drive;modulation=p.motion;fade=1;toneCurrent=tonePole;bassCurrent=bassPole;pitchBlend=float(p.pitchMode);tapeCurrent=tapePole;oilCurrent=oilPole;clockCurrent=clockPole;clockRateCurrent=clockRate;}
    void set(const DelaySettings& next){if(configured&&next==p)return;p=next;configured=true;
        p.time=bounded(p.time,20,8000,375);p.rightTime=bounded(p.rightTime,20,8000,375);p.feedback=bounded(p.feedback,0,.92f,.32f);p.rightFeedback=bounded(p.rightFeedback,0,.92f,.32f);
        p.tone=bounded(p.tone,20,sr*.45f,6500);p.lowcut=bounded(p.lowcut,5,1000,35);p.motion=bounded(p.motion,0,1,0);p.rate=bounded(p.rate,.03f,5,.4f);p.width=bounded(p.width,0,1.5f,1);p.rightLevel=bounded(p.rightLevel,0,1,1);
        p.drive=bounded(p.drive,0,1,.15f);p.diffusion=bounded(p.diffusion,0,1,.7f);p.age=bounded(p.age,0,1,.25f);p.texture=bounded(p.texture,0,1,.5f);p.density=bounded(p.density,0,1,.5f);p.position=bounded(p.position,0,1,.2f);p.bits=bounded(p.bits,4,24,12);p.reduction=bounded(p.reduction,0,1,.2f);p.noise=bounded(p.noise,0,1,0);p.attack=bounded(p.attack,0,1,.25f);p.interval=bounded(p.interval,-24,24,12);p.spread=bounded(p.spread,0,1,.5f);
        for(auto& tap:p.taps){tap.time=bounded(tap.time,.125f,2,1);tap.level=bounded(tap.level,0,1,.25f);tap.pan=bounded(tap.pan,-1,1,0);tap.feedback=bounded(tap.feedback,0,1,.5f);}
        p.model=std::clamp(p.model,0,11);targetTime=std::clamp(p.time*.001f*sr,32.f,8*sr);targetRight=std::clamp(p.rightTime*.001f*sr,32.f,8*sr);
        tonePole=pole(p.tone,sr);bassPole=pole(p.lowcut,sr);wow.set(std::clamp(p.rate,.03f,5.f),sr);flutter.set(6.7f+2.3f*p.age,sr);pitch.set(p.interval,p.texture);
        const float clockHz=std::clamp(2048*sr/std::max(targetTime,32.f),1000.f,sr*.9f);clockRate=clockHz/sr;clockPole=pole(std::min(p.tone,clockHz*(.18f-.1f*p.age)),sr);
        if(p.model==10&&lastSpectral!=p.texture){lastSpectral=p.texture;for(std::size_t c=0;c<2;++c)for(std::size_t band=0;band<6;++band)spectral[c][band].set(90.f*std::pow(std::min(14000.f,sr*.38f)/90.f,float(band)/5),.8f+p.texture*3,sr);}
        for(std::size_t i=0;i<4;++i){headPole[i]=pole(std::min(p.tone,14000.f/(1+float(i)*(.15f+p.age))),sr);}
        noiseLevel=std::clamp(p.noise,0.f,1.f)*.006f;oilPole=pole(7000/(1+3*p.age),sr);tapePole=std::max(tonePole,pole(14000*(1-.85f*p.age),sr));lofiPeriod=std::pow(2.f,p.reduction*7);lofiSteps=std::pow(2.f,p.bits-1);}
    Stereo tick(Stereo input,float cross=0) noexcept {
        const bool moving=p.model==1||p.model==2||p.model==9||p.model==11;
        if(moving){time+=smooth*(targetTime-time);right+=smooth*(targetRight-right);oldTime=time;oldRight=right;fade=1;}
        else {if(fade>=1&&(std::abs(targetTime-time)>1||std::abs(targetRight-right)>1)){oldTime=time;oldRight=right;reverseOldPhase=reversePhase;time=targetTime;right=targetRight;fade=0;}fade=std::min(1.f,fade+1/(sr*.05f));}
        const float residue=p.model==9?.08f+.14f*p.age:0.f;feedback+=smooth*((residue+(1-residue)*p.feedback)*(1-cross)-feedback);rightFeedback+=smooth*((residue+(1-residue)*p.rightFeedback)*(1-cross)-rightFeedback);width+=smooth*(p.width-width);modulation+=smooth*(p.motion-modulation);drive+=smooth*(p.drive-drive);level+=smooth*(p.rightLevel-level);
        const float wobble=wow.tick(),flutterValue=flutter.tick(),drift=irregular.tick(random.tick(),.9995f);recorded=std::min(recorded+1,8*sr);
        tapeCurrent+=smooth*(tapePole-tapeCurrent);oilCurrent+=smooth*(oilPole-oilCurrent);clockCurrent+=smooth*(clockPole-clockCurrent);clockRateCurrent+=smooth*(clockRate-clockRateCurrent);toneCurrent+=smooth*(tonePole-toneCurrent);bassCurrent+=smooth*(bassPole-bassCurrent);pitchBlend+=smooth*(float(p.pitchMode)-pitchBlend);
        Stereo out{};std::array<float,2> tapFeedback{};float* channels[]={&out.l,&out.r};
        for(std::size_t c=0;c<2;++c){
            const float length=c?right:time,previous=c?oldRight:oldTime;
            const float modulationSamples=modulation*sr*.001f*(p.model==9?7.f:p.model==1?2.5f:1.f)*(wobble+(p.model==1?.18f*flutterValue+.9f*drift:0.f));
            auto read=[&](float duration){return line[c].readCubic(std::max(1.f,duration-1+modulationSamples));};
            float wet=fade>=1?read(length):lerp(read(previous),read(length),fade);
            if(p.model==3||p.model==11){auto taps=[&](float duration){float sum=0;for(std::size_t i=0;i<4;++i){const auto& tap=p.taps[i];const float offset=p.model==11?std::array<float,4>{.333333f,.5f,.666667f,1}[i]:tap.time;const float panGain=std::sqrt(std::clamp(1+(c?tap.pan:-tap.pan),0.f,2.f));float head=read(std::min(16*sr,duration*offset));if(p.model==11)head=headLoss[c][i].tick(head,headPole[i]);sum+=head*tap.level*panGain;}float norm=0;for(const auto& tap:p.taps)norm+=tap.level;return sum/std::max(1.f,norm);};wet=fade>=1?taps(length):lerp(taps(previous),taps(length),fade);
                auto returns=[&](float duration){float sum=0,norm=0;for(std::size_t i=0;i<4;++i){const auto& tap=p.taps[i];const float offset=p.model==11?std::array<float,4>{.333333f,.5f,.666667f,1}[i]:tap.time;const float weight=std::clamp(tap.feedback,0.f,1.f);sum+=read(std::min(16*sr,duration*offset))*weight;norm+=weight;}return sum/std::max(1.f,norm);};
                tapFeedback[c]=fade>=1?returns(length):lerp(returns(previous),returns(length),fade); }
            if(p.model==4){auto reverse=[&](float duration,float& phase){phase+=1/std::max(32.f,duration);phase-=std::floor(phase);const float other=phase<.5f?phase+.5f:phase-.5f;const float exponent=1+p.attack*3,w=std::pow(hannWeight(hannTable(),phase),exponent),v=std::pow(hannWeight(hannTable(),other),exponent);return (w*line[c].readCubic(16+2*phase*duration)+v*line[c].readCubic(16+2*other*duration))/std::max(1e-6f,w+v);};const float next=reverse(length,reversePhase[c]);wet=fade>=1?next:lerp(reverse(previous,reverseOldPhase[c]),next,fade);}
            if(p.model==2){wet=std::copysign(wet*wet,wet);wet=expansion[c].tick(wet,clockCurrent);}
            if(p.model==9)wet=.72f*wet+.28f*read(length*.73f);
            if(p.model==6||p.model==9){const int count=p.model==9?2:6;for(int stage=0;stage<count;++stage){float d=diffusion[c][std::size_t(stage)].tick(wet,sr*.001f*(4.1f+float(stage)*8.39f)*(c?1.13f:1.f)*(p.model==9?1+(.05f+.15f*p.age)*(wobble+.3f*drift):.5f+p.texture),p.model==9?.45f+.2f*p.age:.2f+.5f*p.diffusion);wet=lerp(wet,d,p.model==9?1.f:std::clamp(2+p.density*4-float(stage),0.f,1.f));}}
            if(p.model==10){wet=0;for(std::size_t band=0;band<6;++band){const float ratio=.25f+float(band)*(.16f+.2f*p.texture);wet+=spectral[c][band].tick(read(std::min(16*sr,length*ratio)))*.16666667f;}}
            const float cutoff=p.model==1||p.model==11?tapeCurrent:p.model==9?std::max(toneCurrent,oilCurrent):toneCurrent;wet=tone[c].tick(wet,cutoff);wet-=bass[c].tick(wet,bassCurrent);
            if(p.model==1||p.model==2||p.model==9||p.model==11){const float k=1+drive*3;wet=std::tanh(wet*k)/k;}
            *channels[c]=wet;
        }
        Stereo pitchReturn{};if(p.model==5)pitchReturn=pitch.tick(out);
        if(p.model==7){out=granular.tick(out,p);}
        if(p.model==8){const float period=lofiPeriod;if(clock<=0){const float steps=lofiSteps;heldL=std::round((out.l+random.tick()*noiseLevel)*steps)/steps;heldR=std::round((out.r+random.tick()*noiseLevel)*steps)/steps;clock=period;}clock-=1;out={heldL,heldR};out.l=reconstruction[0].tick(out.l,toneCurrent);out.r=reconstruction[1].tick(out.r,toneCurrent);}
        Stereo excitation=input;
        if(p.stereo==2){const float side=.5f*(input.l-input.r);excitation={input.l,-side};}
        Stereo returns=p.model==3||p.model==11?Stereo{tapFeedback[0],tapFeedback[1]}:out;
        if(p.model==3||p.model==11){returns.l=returnTone[0].tick(returns.l,toneCurrent);returns.r=returnTone[1].tick(returns.r,toneCurrent);returns.l-=returnBass[0].tick(returns.l,bassCurrent);returns.r-=returnBass[1].tick(returns.r,bassCurrent);}
        if(p.model==5)returns=blend(pitchReturn,out,pitchBlend);
        const float f[]={p.stereo==2?returns.r:returns.l,p.stereo==2?returns.l:returns.r};const float in[]={excitation.l,excitation.r};
        if(p.model==2)clock+=clockRateCurrent;
        for(std::size_t c=0;c<2;++c){float x=(p.freeze?0.f:in[c])+(c?rightFeedback:feedback)*((p.model==1||p.model==2||p.model==9||p.model==11)?std::tanh(f[c]):std::clamp(f[c],-4.f,4.f));
            if(p.model==2){x=clockFilter[c].tick(x,clockCurrent);if(clock>=1){if(c)heldR=std::copysign(std::sqrt(std::abs(x)),x);else heldL=std::copysign(std::sqrt(std::abs(x)),x);}x=c?heldR:heldL;}
            line[c].push(clean(x));}
        if(p.model==2&&clock>=1)clock-=std::floor(clock);
        if(p.model==5)out=pitchReturn;
        out.r*=level;const float mid=.5f*(out.l+out.r),side=.5f*(out.l-out.r)*width;
        return clean(Stereo{mid+side,mid-side})*(1/std::max(1.f,width));
    }
};

// Separate topologies share storage primitives, not a single configured tank.
class ReverbMachine {
    std::array<wolf::Delay,16> lines;
    std::array<std::array<Diffuser,8>,2> diffusion;
    std::array<wolf::Delay,2> early,pre;
    std::array<wolf::OnePole,16> damping,bass;std::array<wolf::OnePole,2> lowcut;float lowPole=0;
    std::array<Oscillator,16> movement;
    std::array<float,16> length{},wantedLength{},gain{},wantedGain{},lineDamp{},wantedDamp{};
    std::array<float,8> modalLevel{};
    Pitch pitch;Stereo shifted{};float shimmer=0,regeneration=0,onsetClock=0,detect=0,detectRate=0;
    ReverbSettings p{};bool configured=false;int activeLineCount=8;
    float sr=48000,smooth=0,damp=0,dampTarget=0,bassA=0,age=0,gate=0,width=1;
    float predelay=0,wantedPre=0,oldPre=0,preFade=1,build=0;
    static constexpr float times[16]={29.3f,37.1f,43.7f,53.3f,61.1f,73.9f,83.3f,97.7f,109.1f,127.7f,139.3f,151.1f,167.3f,181.1f,193.7f,211.3f};
    float diffuse(float x,std::size_t c,int count,float scale,float feedback=.65f,int start=0) noexcept{
        for(int i=0;i<count;++i)x=diffusion[c][std::size_t(i+start)].tick(x,sr*.001f*(2.9f+float(i)*4.13f)*scale*(c?1.117f:1.f),feedback*(.45f+.5f*p.diffusion));
        return x;
    }
    Stereo reflections(Stereo in,bool chamber) noexcept {
        early[0].push(in.l);early[1].push(in.r);Stereo out{};
        constexpr float roomTimes[]={5.1f,11.3f,17.9f,29.7f,43.1f,59.3f};
        constexpr float chamberTimes[]={3.7f,7.3f,13.1f,19.9f,31.7f,47.3f};
        for(std::size_t i=0;i<6;++i){const float t=(chamber?chamberTimes[i]:roomTimes[i])*sr*.001f*p.size;
            out.l+=early[0].read(t)*(.21f-.025f*float(i))+early[1].read(t*1.071f)*.035f;
            out.r+=early[1].read(t*1.113f)*(.21f-.025f*float(i))+early[0].read(t*1.037f)*.035f;}
        return out;
    }
    Stereo combs(Stereo in,int count,bool chamber,float cross) noexcept {
        Stereo wet{};std::array<float,16> snapshot{};if(chamber)for(int i=0;i<count;++i)snapshot[std::size_t(i)]=lines[std::size_t(i)].read(length[std::size_t(i)]-1);
        for(int i=0;i<count;++i){const auto n=std::size_t(i);length[n]+=smooth*(wantedLength[n]-length[n]);gain[n]+=smooth*(wantedGain[n]*(1-cross)-gain[n]);
            const float d=damping[n].tick(lines[n].read(length[n]-1),lineDamp[n]);
            const float injection=(i&1?in.r:in.l)*.22f+(i&1?in.l:in.r)*.06f;
            const float coupling=chamber?.12f*snapshot[(n+1)%std::size_t(count)]:0;
            lines[n].push(clean(injection+d*gain[n]*(chamber?.88f:1.f)+coupling*gain[n]));
            if(i&1)wet.r+=d;else wet.l+=d;
        }
        return {diffuse(wet.l/std::sqrt(float(count)),0,chamber?3:2,1),diffuse(wet.r/std::sqrt(float(count)),1,chamber?3:2,1)};
    }
    Stereo plate(Stereo in,bool studio,float cross) noexcept {
        // Coupled, dispersive figure-eight branches; Studio adds a second coupled pair.
        const int branches=studio?4:2;std::array<float,4> delayed{};
        for(int i=0;i<branches;++i){const auto n=std::size_t(i);length[n]+=smooth*(wantedLength[n]-length[n]);gain[n]+=smooth*(wantedGain[n]*(1-cross)-gain[n]);
            delayed[n]=damping[n].tick(lines[n].read(length[n]-1+p.motion*sr*.0005f*movement[n].tick()),lineDamp[n]);}
        Stereo wet{};
        for(int i=0;i<branches;++i){const auto n=std::size_t(i);const float partner=delayed[(n+1)%std::size_t(branches)];
            const float scatter=studio?(.5f*(delayed[0]+(i&1?-delayed[1]:delayed[1])+(i&2?-delayed[2]:delayed[2])+((i==1||i==2)?-delayed[3]:delayed[3]))):(delayed[n]+(i&1?-partner:partner))*.70710678f;
            float x=(i&1?in.r:in.l)*.28f+scatter*gain[n];
            x=diffuse(x,std::size_t(i&1),studio?4:3,studio?.83f:1.17f,.65f,studio?(i/2)*4:0);
            lines[n].push(clean(x));
            const float taps=.55f*delayed[n]+.25f*lines[n].read(length[n]*.31f)-.2f*lines[(n+1)%std::size_t(branches)].read(length[(n+1)%std::size_t(branches)]*.67f);
            if(i&1)wet.r+=taps;else wet.l+=taps;
        }
        return wet*(studio?.70710678f:1.f);
    }
    Stereo fdn(Stereo in,int count,bool ensemble,bool bloom,float cross) noexcept {
        std::array<float,16> d{};Stereo wet{};float sum=0;
        for(int i=0;i<count;++i){const auto n=std::size_t(i);length[n]+=smooth*(wantedLength[n]-length[n]);gain[n]+=smooth*(wantedGain[n]*(1-cross)-gain[n]);
            const float offset=p.motion*sr*(ensemble?.0014f:.00035f)*movement[n].tick();
            d[n]=damping[n].tick(lines[n].read(length[n]-1+offset),lineDamp[n]);
            d[n]-=bass[n].tick(d[n],bassA)*.025f*(p.freeze?0.f:p.damping);
            sum+=d[n];wet.l+=d[n]*(i&1?-1.f:1.f);wet.r+=d[n]*(i%3?-1.f:1.f);
        }
        std::array<float,16> mixed=d;
        if(p.model==9){for(int span=1;span<count;span*=2)for(int base=0;base<count;base+=2*span)for(int j=0;j<span;++j){float a=mixed[std::size_t(base+j)],b=mixed[std::size_t(base+j+span)];mixed[std::size_t(base+j)]=(a+b)*.70710678f;mixed[std::size_t(base+j+span)]=(a-b)*.70710678f;}}
        else if(p.model==3||p.model==10||p.model==6){for(int i=0;i<count;i+=2){float a=d[std::size_t(i)],b=d[std::size_t(i+1)];mixed[std::size_t(i)]=(a+b)*.70710678f;mixed[std::size_t(i+1)]=(b-a)*.70710678f;}}
        else for(int i=0;i<count;++i)mixed[std::size_t(i)]=d[std::size_t(i)]-2*sum/float(count);
        for(int i=0;i<count;++i){const auto n=std::size_t(i);const int index=p.model==3?(i+1)%count:p.model==10?(i+5)%count:p.model==6?(i+7)%count:p.model==7?(i+5)%count:i;const float scattered=mixed[std::size_t(index)];
            const float injection=(i&1?in.r:in.l)*.18f+(i%3?in.l:in.r)*.035f;
            lines[n].push(clean(scattered*gain[n]+injection));}
        wet=wet*(.7f/std::sqrt(float(count)));
        if(ensemble){wet.l=diffuse(wet.l,0,6,1.31f,.65f,2);wet.r=diffuse(wet.r,1,6,1.31f,.65f,2);}
        if(bloom){wet.l=diffuse(wet.l,0,3,1+p.attack*4,.72f,5);wet.r=diffuse(wet.r,1,3,1+p.attack*4,.72f,5);}
        return wet;
    }
public:
    void prepare(float rate){sr=rate;bassA=pole(180,sr);detectRate=1-std::exp(-1/(sr*.01f));smooth=1-std::exp(-1/(sr*.04f));for(auto& d:lines)d.prepare(std::size_t(sr*.65f)+16);
        for(std::size_t c=0;c<2;++c){early[c].prepare(std::size_t(sr*.18f)+8);pre[c].prepare(std::size_t(sr*.21f)+8);for(auto& d:diffusion[c])d.prepare(sr,350);}
        pitch.prepare(sr);configured=false;set(p);clear();}
    void clear(){for(auto& d:lines)d.clear();for(auto& d:early)d.clear();for(auto& d:pre)d.clear();for(auto& row:diffusion)for(auto& d:row)d.clear();damping={};bass={};lowcut={};pitch.clear();shifted={};shimmer=p.shimmer;regeneration=p.regeneration;onsetClock=1e6f;detect=0;lineDamp=wantedDamp;length=wantedLength;gain=wantedGain;damp=dampTarget;predelay=oldPre=wantedPre;preFade=1;age=gate=build=0;}
    void set(const ReverbSettings& next){if(configured&&next==p)return;p=next;configured=true;
        p.decay=bounded(p.decay,.25f,20,4);p.predelay=bounded(p.predelay,0,200,28);p.tone=bounded(p.tone,20,sr*.45f,6500);p.lowcut=bounded(p.lowcut,5,1000,100);p.motion=bounded(p.motion,0,1,.2f);p.width=bounded(p.width,0,1.5f,1);p.size=bounded(p.size,.5f,1.7f,1);p.diffusion=bounded(p.diffusion,0,1,.7f);p.attack=bounded(p.attack,.01f,2,.2f);p.regeneration=bounded(p.regeneration,0,.6f,.2f);p.shimmer=bounded(p.shimmer,0,1,0);p.interval=bounded(p.interval,-24,24,12);p.early=bounded(p.early,0,1,.35f);p.damping=bounded(p.damping,0,1,.5f);p.texture=bounded(p.texture,0,1,.5f);p.tune=bounded(p.tune,30,1000,110);p.shape=bounded(p.shape,0,1,.5f);
        lowPole=pole(p.lowcut,sr);p.bass=bounded(p.bass,0,1,0);p.model=std::clamp(p.model,0,12);activeLineCount=p.model==0?4:p.model==1?2:p.model==4?6:p.model==8?4:(p.model==5||p.model==9)?16:(p.model==6||p.model==7||p.model==10)?12:8;p.size=std::clamp(p.size,.5f,1.7f);p.decay=std::clamp(p.decay,.25f,20.f);wantedPre=std::clamp(p.predelay,0.f,200.f)*sr*.001f;dampTarget=p.freeze?0:pole(p.tone,sr);
        for(std::size_t i=0;i<16;++i){float ms=times[i];
            if(p.model==0){constexpr float room[]={13.7f,19.9f,28.3f,37.1f};ms=room[i%4];}
            if(p.model==1){constexpr float plate[]={67.3f,91.7f};ms=plate[i%2];}if(p.model==6)ms=times[i]*1.37f;
            if(p.model==4){constexpr float chamber[]={11.3f,17.9f,23.3f,31.7f,41.9f,47.3f};ms=chamber[i%6];}
            if(p.model==5)ms*=1.51f;
            if(p.model==7)ms=times[i]*.43f;
            if(p.model==8){constexpr float plate[]={41.7f,59.3f,73.9f,89.3f};ms=plate[i%4];}
            if(p.model==9)ms*=1.19f;
            if(p.model==12){constexpr float harmonics[]={1,1.5f,2,2.5f,3,4,5,6};ms=1000/(std::clamp(p.tune,30.f,1000.f)*harmonics[i%8]);}
            wantedLength[i]=std::clamp(ms*sr*.001f*p.size,8.f,sr*.6f);
            const float reference=(1-dampTarget)/(1+dampTarget),loss=std::pow(std::max(1e-6f,reference),wantedLength[i]/(sr*.06f));wantedDamp[i]=(1-loss)/(1+loss);
            wantedGain[i]=p.freeze?1.f:std::pow(10.f,-3*wantedLength[i]/sr/p.decay);
            movement[i].set(.071f+float(i)*.0173f,sr);
        }
        if(p.model==12){constexpr float harmonics[]={1,1.5f,2,2.5f,3,4,5,6};for(std::size_t i=0;i<8;++i){const float hz=p.tune*harmonics[i]/p.size;modalLevel[i]=hz<sr*.45f?1.f:0.f;const float w=2*pi*std::min(hz,sr*.45f)/sr;const float period=sr/std::min(hz,sr*.45f),phaseDelay=std::atan2(dampTarget*std::sin(w),1-dampTarget*std::cos(w))/w;wantedLength[i]=std::max(1.1f,period-phaseDelay);wantedDamp[i]=dampTarget;wantedGain[i]=p.freeze?1.f:std::pow(10.f,-3*period/sr/p.decay);}}
        pitch.set(p.interval,p.texture);}
    Stereo tick(Stereo input,float cross=0) noexcept {
        damp+=smooth*(dampTarget-damp);for(int i=0;i<activeLineCount;++i)lineDamp[std::size_t(i)]+=smooth*(wantedDamp[std::size_t(i)]-lineDamp[std::size_t(i)]);width+=smooth*(p.width-width);shimmer+=smooth*(p.shimmer-shimmer);regeneration+=smooth*(p.regeneration-regeneration);
        if(preFade>=1&&std::abs(wantedPre-predelay)>.5f){oldPre=predelay;predelay=wantedPre;preFade=0;}preFade=std::min(1.f,preFade+1/(sr*.04f));
        input.l-=lowcut[0].tick(input.l,lowPole)*(1-p.bass);input.r-=lowcut[1].tick(input.r,lowPole)*(1-p.bass);
        pre[0].push(p.freeze?0:input.l);pre[1].push(p.freeze?0:input.r);
        Stereo in{lerp(pre[0].read(oldPre),pre[0].read(predelay),preFade),lerp(pre[1].read(oldPre),pre[1].read(predelay),preFade)};
        if(peak(in)>1e-6f){if(age>.1f)build=0;age=0;}else age+=1/sr;
        const float amplitude=peak(in);onsetClock+=1/sr;if(amplitude>std::max(1e-5f,detect*2)&&onsetClock>.03f)onsetClock=0;detect+=detectRate*(amplitude-detect);
        const bool pitched=(p.model==6||p.model==7)&&shimmer>1e-7f;
        const float regen=pitched&&!p.freeze?regeneration*.3f*shimmer:0;
        in=in+shifted*regen;
        const auto earlyWet=(p.model==0||p.model==2||p.model==4||p.model==7||p.model==9)?reflections(in,p.model==4):Stereo{};
        if(p.model==2||p.model==5||p.model==9||p.model==11){in={lerp(in.l,diffuse(in.l,0,2,.67f),p.diffusion),lerp(in.r,diffuse(in.r,1,2,.67f),p.diffusion)};}
        Stereo wet{};
        switch(p.model){
            case 0:wet=combs(in,4,false,cross)+earlyWet*p.early;break;
            case 1:wet=plate(in,false,cross);break;
            case 2:wet=fdn(in,8,false,false,cross)+earlyWet*p.early;break;
            case 3:in={diffuse(in.l,0,3,1+p.attack*3,.7f),diffuse(in.r,1,3,1+p.attack*3,.7f)};wet=fdn(in,8,false,false,cross);break;
            case 4:wet=combs(in,6,true,cross)+earlyWet*p.early;break;
            case 5:wet=fdn(in,16,true,false,cross);break;
            case 6:wet=fdn({diffuse(in.l,0,4,.91f,.61f),diffuse(in.r,1,4,.91f,.61f)},12,false,false,std::min(.9f,cross+regen));break;
            case 7:wet=earlyWet*.45f+fdn({diffuse(in.l,0,3,.37f,.45f),diffuse(in.r,1,3,.37f,.45f)},12,false,false,std::min(.9f,cross+regen));break;
            case 8:wet=plate(in,true,cross);break;
            case 9:wet=fdn(in,16,false,false,cross)+earlyWet*(p.early*.7f);break;
            case 10:in={diffuse(in.l,0,5,1+p.attack*5,.72f),diffuse(in.r,1,5,1+p.attack*5,.72f)};wet=fdn(in,12,false,true,cross);break;
            case 11:{wet=fdn(in,8,false,false,cross);const float duration=.05f+p.attack*.8f;const float target=onsetClock<duration?1.f:0.f;gate+=std::clamp(target-gate,-1/(sr*(.003f+.15f*p.shape)),1/(sr*.003f));build=std::min(1.f,build+1/(sr*duration));const float envelope=lerp(1.f,build*build,std::max(0.f,p.shape*2-1));wet=wet*(gate*envelope);break;}
            case 12:{for(std::size_t i=0;i<8;++i){length[i]+=smooth*(wantedLength[i]-length[i]);gain[i]+=smooth*(wantedGain[i]*(1-cross)-gain[i]);const float d=damping[i].tick(lines[i].read(length[i]-1),lineDamp[i]);lines[i].push(clean((i&1?in.r:in.l)*.12f*modalLevel[i]+d*gain[i]));if(i&1)wet.r+=d*.25f*modalLevel[i];else wet.l+=d*.25f*modalLevel[i];}break;}
        }
        if(p.model==3||p.model==10){build=std::min(1.f,build+1/(sr*std::max(.01f,p.attack)));float envelope=p.model==3?build*build:build*build*(3-2*build);wet=wet*envelope;}
        if(pitched){shifted=pitch.tick(p.model==7?earlyWet:wet);shifted={std::tanh(shifted.l),std::tanh(shifted.r)};wet=wet+shifted*(shimmer*.4f);}else shifted={};
        const float mid=.5f*(wet.l+wet.r),side=.5f*(wet.l-wet.r)*width;
        return clean(Stereo{mid+side,mid-side})*(1/std::max(1.f,width));
    }
};

// Return calibration is outside each feedback loop. No signal-dependent gain matching.
inline constexpr float delayTrim[]={.81f,.89f,.92f,2.13f,1.10f,1.16f,.79f,1.38f,.81f,.89f,6.35f,1.46f};
inline constexpr float reverbTrim[]={.87f,2.23f,1.66f,1.91f,1.93f,4.21f,4.09f,1.98f,1.97f,2.85f,2.57f,1.92f,2.20f};
// Two prepared voices allow a current machine and an outgoing tail without allocating.
// ponytail: two-voice ceiling; a third selection retires the oldest voice over 40 ms.
template<class Machine,class Settings> class Slot {
    std::array<Machine,2> machines;std::array<Settings,2> settings{};
    std::array<bool,2> live{};std::array<unsigned,2> quiet{};std::array<float,2> gains{1,1},inputGain{1,1};
    int current=0;float sr=48000;Settings requested{};bool pending=false,retire=false;
    void activate(){const int old=current;current=1-current;settings[std::size_t(current)]=requested;machines[std::size_t(current)].set(requested);machines[std::size_t(current)].clear();gains[std::size_t(current)]=1;inputGain[std::size_t(current)]=live[std::size_t(old)]?0.f:1.f;quiet[std::size_t(current)]=0;live[std::size_t(current)]=false;pending=retire=false;(void)old;}
public:
    Stereo calibrated{};
    void prepare(float rate){sr=rate;for(auto& machine:machines)machine.prepare(rate);clear();}
    void clear(){for(auto& machine:machines)machine.clear();live={};quiet={};gains={1,1};inputGain={1,1};calibrated={};pending=retire=false;}
    void set(const Settings& next){requested=next;if(next.model!=settings[std::size_t(current)].model){pending=true;if(live[std::size_t(1-current)])retire=true;else activate();}else{settings[std::size_t(current)]=next;machines[std::size_t(current)].set(next);pending=false;}}
    bool active()const noexcept{return live[0]||live[1];}
    Stereo tick(Stereo input,float cross=0) noexcept{
        if(pending&&!live[std::size_t(1-current)])activate();
        Stereo sum{};calibrated={};
        for(std::size_t i=0;i<2;++i){const bool inject=int(i)==current&&requested.on;inputGain[i]+=std::clamp((inject?1.f:0.f)-inputGain[i],-1/(sr*.01f),1/(sr*.01f));const auto excitation=int(i)==current&&(requested.on||live[i])?input*inputGain[i]:Stereo{};if(peak(excitation)>1e-10f)live[i]=true;if(!live[i])continue;
            auto wet=machines[i].tick(excitation,cross);quiet[i]=peak(excitation)>1e-10f||peak(wet)>1e-10f?0:quiet[i]+1;
            if(retire&&int(i)!=current){gains[i]=std::max(0.f,gains[i]-1/(sr*.04f));}
            sum=sum+wet*gains[i];
            const auto index=std::size_t(settings[i].model);const float trim=std::is_same<Settings,DelaySettings>::value?delayTrim[std::min(index,std::size_t(11))]:reverbTrim[std::min(index,std::size_t(12))];calibrated=calibrated+wet*(gains[i]*trim);
            if(quiet[i]>unsigned(sr*(std::is_same<Settings,DelaySettings>::value?16.5f:1.f))||gains[i]<=0){machines[i].clear();live[i]=false;quiet[i]=0;}}
        return sum;
    }
};
struct Settings {
    std::array<DelaySettings,2> delays{};std::array<ReverbSettings,2> reverbs{};
    std::array<float,2> delayLevel{.3f,.3f},reverbLevel{.3f,.3f};
    int routing=0;float splitFrequency=500,crossFeedback=0,mix=.38f,output=0,feed=.65f,direct=1;
    float delayDuck=0,reverbDuck=0,delaySense=1,reverbSense=1,delayRelease=280,reverbRelease=280;
    bool send=false,legacyDelay=false,legacyReverb=false;
};
class Engine {
    std::array<Slot<DelayMachine,DelaySettings>,2> delays;
    std::array<Slot<ReverbMachine,ReverbSettings>,2> reverbs;
    Settings p{};float sr=48000,smooth=0,mix=.38f,gain=1,gainTarget=1,splitPole=0,cross=0,excitation=1,dryActive=1,legacyDelayMix=0,legacyReverbMix=0;
    std::array<float,3> routes{1,0,0};std::array<float,2> delayLevel{},reverbLevel{};
    float feed=.65f,direct=1,delayEnv=0,reverbEnv=0,duckAttack=0,delayReleasePole=0,reverbReleasePole=0;
    std::array<Stereo,2> previousDelay{},previousReverb{};std::array<wolf::OnePole,2> split;
public:
    Stereo legacyOutput{};float delayPeak=0,reverbPeak=0;bool delayActive()const{return delays[0].active()||delays[1].active();}bool reverbActive()const{return reverbs[0].active()||reverbs[1].active();}
    void prepare(float rate,const Settings& initial={}){sr=std::clamp(rate,8000.f,192000.f);smooth=1-std::exp(-1/(sr*.03f));duckAttack=pole(20,sr);for(auto& d:delays)d.prepare(sr);for(auto& r:reverbs)r.prepare(sr);set(initial);mix=p.send?1:p.mix;legacyDelayMix=p.legacyDelay?1.f:0.f;legacyReverbMix=p.legacyReverb?1.f:0.f;gain=gainTarget;delayLevel=p.delayLevel;reverbLevel=p.reverbLevel;feed=p.feed;direct=p.direct;routes={};routes[std::size_t(p.routing)]=1;clear();}
    void clear(){for(auto& d:delays)d.clear();for(auto& r:reverbs)r.clear();previousDelay={};previousReverb={};split={};delayEnv=reverbEnv=0;delayPeak=reverbPeak=0;}
    void set(const Settings& next){p=next;p.routing=std::clamp(p.routing,0,2);p.mix=bounded(p.mix,0,1,.38f);p.output=bounded(p.output,-24,6,0);p.splitFrequency=bounded(p.splitFrequency,30,sr*.4f,500);p.crossFeedback=bounded(p.crossFeedback,0,.3f,0);p.feed=bounded(p.feed,0,1,.65f);p.direct=bounded(p.direct,0,1,1);for(auto& level:p.delayLevel)level=bounded(level,0,.8f,.3f);for(auto& level:p.reverbLevel)level=bounded(level,0,.8f,.3f);gainTarget=std::pow(10.f,p.output/20);splitPole=pole(p.splitFrequency,sr);delayReleasePole=std::exp(-1/(sr*bounded(p.delayRelease,50,1500,280)*.001f));reverbReleasePole=std::exp(-1/(sr*bounded(p.reverbRelease,50,1500,280)*.001f));for(std::size_t i=0;i<2;++i){delays[i].set(p.delays[i]);reverbs[i].set(p.reverbs[i]);}}
    bool needsLegacy()const noexcept{return legacyDelayMix>0||legacyReverbMix>0||p.legacyDelay||p.legacyReverb;}
    Stereo tick(Stereo input,bool bypass=false,tide::Engine* legacy=nullptr) noexcept {
        input=clean(input);legacyDelayMix+=std::clamp((p.legacyDelay?1.f:0.f)-legacyDelayMix,-1/(sr*.075f),1/(sr*.075f));legacyReverbMix+=std::clamp((p.legacyReverb?1.f:0.f)-legacyReverbMix,-1/(sr*.075f),1/(sr*.075f));auto follow=[&](float& x,float target){x+=smooth*(target-x);};
        follow(mix,p.send?1:p.mix);follow(gain,gainTarget);follow(cross,p.crossFeedback);follow(excitation,bypass?0.f:1.f);follow(feed,p.feed);follow(direct,p.direct);
        for(int i=0;i<3;++i)follow(routes[std::size_t(i)],p.routing==i?1.f:0.f);
        for(std::size_t i=0;i<2;++i){follow(delayLevel[i],p.delayLevel[i]);follow(reverbLevel[i],p.reverbLevel[i]);}
        const float amp=peak(input);delayEnv=amp+(amp>delayEnv?duckAttack:delayReleasePole)*(delayEnv-amp);reverbEnv=amp+(amp>reverbEnv?duckAttack:reverbReleasePole)*(reverbEnv-amp);
        const float dd=1/(1+delayEnv*p.delaySense*p.delayDuck*12),rd=1/(1+reverbEnv*p.reverbSense*p.reverbDuck*12);
        const Stereo source=input*excitation,low{split[0].tick(source.l,splitPole),split[1].tick(source.r,splitPole)},high{source.l-low.l,source.r-low.r};
        std::array<Stereo,2> d{},r{};Stereo audibleDelay{},delayFeed{};
        if(legacyDelayMix<1){
            for(std::size_t i=0;i<2;++i){Stereo x=i?blend(source,p.delays[0].on?d[0]:source,routes[0]):source;x=blend(x,i?high:low,routes[2]);const Stereo feedback{std::tanh(previousDelay[1-i].l)*cross,std::tanh(previousDelay[1-i].r)*cross};
                d[i]=delays[i].tick(x+feedback,cross);audibleDelay=audibleDelay+delays[i].calibrated*delayLevel[i];delayFeed=delayFeed+d[i];}
            delayFeed=delayFeed*(1/std::max(1.f,float(delays[0].active())+float(delays[1].active())));
            // Bound correlated multi-slot sums using their requested gain budget.
            audibleDelay=audibleDelay*(1/std::max(1.f,delayLevel[0]+delayLevel[1]));
        }
        if(legacy)legacyOutput=legacy->tick(input,bypass,legacyDelayMix>0?nullptr:&audibleDelay,legacyDelayMix>0?nullptr:&delayFeed);
        audibleDelay=audibleDelay*dd;delayFeed=delayFeed*dd;
        if(legacy){audibleDelay=blend(audibleDelay,legacy->delayStem,legacyDelayMix);delayFeed=blend(delayFeed,legacy->delayFeed,legacyDelayMix);}
        Stereo audibleReverb{};
        if(legacyReverbMix<1){
            const auto reverbSource=source*direct+delayFeed*(feed*(1-routes[1]));
            for(std::size_t i=0;i<2;++i){Stereo x=i?blend(reverbSource,p.reverbs[0].on?r[0]:reverbSource,routes[0]):reverbSource;x=blend(x,(i?high:low)*direct+d[i]*(feed*dd),routes[2]);const Stereo feedback{std::tanh(previousReverb[1-i].l)*cross,std::tanh(previousReverb[1-i].r)*cross};r[i]=reverbs[i].tick(x+feedback,cross);audibleReverb=audibleReverb+reverbs[i].calibrated*reverbLevel[i];}
            audibleReverb=audibleReverb*(rd/std::max(1.f,reverbLevel[0]+reverbLevel[1]));
        }
        if(legacy)audibleReverb=blend(audibleReverb,legacy->reverbStem,legacyReverbMix);
        previousDelay=d;previousReverb=r;delayPeak=peak(audibleDelay);reverbPeak=peak(audibleReverb);
        const bool enabled=p.delays[0].on||p.delays[1].on||p.reverbs[0].on||p.reverbs[1].on;follow(dryActive,enabled&&!bypass?1.f:0.f);
        auto out=(input*(p.send?0.f:1-mix*dryActive)+(audibleDelay+audibleReverb)*(2.5f*mix))*gain;
        // Only protect the wet-active output; dry bypass and mix=0 remain transparent.
        if(mix<=1e-7f||(!delayActive()&&!reverbActive()&&!p.legacyDelay&&!p.legacyReverb))return input*(p.send?0.f:gain);
        return {protect(out.l),protect(out.r)};
    }
    int activeSlots()const noexcept{int count=0;for(const auto& d:delays)count+=d.active();for(const auto& r:reverbs)count+=r.active();return count;}
};
} // namespace tide::v13
