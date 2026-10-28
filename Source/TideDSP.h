// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "BloomDSP.h"
#include <tuple>
namespace tide {
using wolf::Delay; using wolf::Allpass; using wolf::OnePole;
constexpr float pi=wolf::pi;
inline float finite(float x) noexcept {return std::isfinite(x)?std::clamp(x,-16.f,16.f):0.f;}
inline float pole(float hz,float sr) noexcept {return std::exp(-2*pi*std::clamp(hz,5.f,sr*.45f)/sr);}
struct Stereo {float l=0,r=0;};
inline int delayModel(int original,int extension) noexcept {return extension?std::clamp(extension,1,3)+5:std::clamp(original,0,5);}
inline int reverbModel(int original,int extension) noexcept {return extension?std::clamp(extension,1,7)+3:std::clamp(original,0,3);}
struct Parameters {
    int delayType=0, rhythm=3, stereo=1, reverbType=2, routing=0, interval=3, reverbInterval=3,pitchMode=0;
    int delayModel=0,reverbModel=0,soundVersion=0,reverbVersion=0,pattern=1,dualMode=0,bRhythm=4; bool manualTempo=false;
    float delayDiffusion=.7f;
    float bTime=500,bFeedback=.32f,bLevel=1,swell=0,diffusion=.7f,size=1,attack=.18f,regeneration=.3f;
    bool delayOn=true,reverbOn=true,sync=true,send=false,freeze=false;
    float time=375,feedback=.32f,delayTone=6500,delayMotion=.15f,delayWidth=1,delayAmount=.24f;
    float drive=.18f,delayLowcut=150,delayDuck=.15f,delayRate=.4f,ratio=.75f;
    float decay=4,predelay=28,reverbTone=6500,reverbMotion=.22f,reverbWidth=1,reverbAmount=.24f;
    float reverbLowcut=180,reverbDuck=.15f,shimmer=0,bassDecay=.8f;
    float delaySense=1,reverbSense=1,delayRelease=280,reverbRelease=280;
    float feed=.65f,direct=1,mix=.38f,output=0,bloom=0,bpm=120;
    auto delayValues()const noexcept{return std::tie(delayType,delayModel,soundVersion,rhythm,stereo,interval,pitchMode,time,bTime,bRhythm,bpm,sync,delayTone,delayLowcut,delayRate);}
    auto reverbValues()const noexcept{return std::tie(reverbModel,decay,reverbTone,reverbMotion,reverbLowcut,freeze,bloom,reverbInterval,reverbVersion,diffusion,size,attack,regeneration,shimmer);}
    auto values() const noexcept {return std::tie(delayDiffusion,delayModel,reverbModel,soundVersion,reverbVersion,pattern,dualMode,bRhythm,manualTempo,bTime,bFeedback,bLevel,swell,diffusion,size,attack,regeneration,delayType,rhythm,stereo,reverbType,routing,interval,reverbInterval,pitchMode,delayOn,reverbOn,sync,send,freeze,time,feedback,delayTone,delayMotion,delayWidth,delayAmount,drive,delayLowcut,delayDuck,delayRate,ratio,decay,predelay,reverbTone,reverbMotion,reverbWidth,reverbAmount,reverbLowcut,reverbDuck,shimmer,bassDecay,delaySense,reverbSense,delayRelease,reverbRelease,feed,direct,mix,output,bloom,bpm);}
};
inline constexpr float beats[]={.25f,1.f/3,.5f,.75f,1,1.5f,2,3,4};
inline constexpr float semitones[]={-12,-7,7,12,19,24};
inline const std::array<float,1025>& hannTable(){static const auto table=[](){std::array<float,1025> t{};for(std::size_t i=0;i<t.size();++i)t[i]=.5f-.5f*std::cos(2*pi*float(i)/1024);return t;}();return table;}
inline float hannWeight(const std::array<float,1025>& table,float phase) noexcept {const float p=std::clamp(phase,0.f,.99999994f)*1024;const auto i=std::size_t(p);return table[i]+(p-float(i))*(table[i+1]-table[i]);}
struct Lowpass {
    float b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
    void set(float hz,float sr,float q) noexcept {const float w=2*pi*hz/sr,c=std::cos(w),a=std::sin(w)/(2*q),norm=1/(1+a);b0=(1-c)*.5f*norm;b1=2*b0;b2=b0;a1=-2*c*norm;a2=(1-a)*norm;}
    float tick(float x) noexcept {const float y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y;}
    void clear() noexcept {z1=z2=0;}
};
// Linked stereo overlap-add, cubic reads and a pitch-dependent anti-alias filter.
class Pitch {
    std::array<Delay,2> line;std::array<std::array<Lowpass,2>,2> antiAlias;
    const std::array<float,1025>* hann=nullptr;
    bool modern=false,filterDirty=true;float cutoffCurrent=1000,cutoffTarget=1000;unsigned filterClock=0;
    float phase=0,sr=48000,ratio=2,targetRatio=2,window=3360,targetWindow=3360,sm=0,lastSemi=999;
public:
    void prepare(float rate){hann=&hannTable();sr=rate;for(auto& d:line)d.prepare(std::size_t(sr*.16f)+40);sm=1-std::exp(-1.f/(sr*.025f));lastSemi=999;set(12);ratio=targetRatio;window=targetWindow;clear();}
    void clear(){for(auto& d:line)d.clear();for(auto& row:antiAlias)for(auto& f:row)f.clear();phase=0;ratio=targetRatio;window=targetWindow;}
    void setModern(bool enabled) noexcept {if(modern!=enabled){modern=enabled;lastSemi=999;}}
    void set(float semi) noexcept {if(semi==lastSemi)return;lastSemi=semi;targetRatio=std::pow(2.f,semi/12.f);targetWindow=modern?sr*.105f:sr*.07f*(1+.5f*std::clamp(semi/12-1.f,0.f,1.f));
        const float cutoff=sr*.44f/std::max(1.f,targetRatio);cutoffTarget=cutoff;filterDirty=true;
        if(modern)return;
        cutoffCurrent=cutoff;for(auto& row:antiAlias){row[0].set(cutoff,sr,.5411961f);row[1].set(cutoff,sr,1.306563f);}}
    Stereo tick(Stereo in) noexcept {
        if(modern&&filterDirty&&(filterClock++&31u)==0){cutoffCurrent+=.05f*(cutoffTarget-cutoffCurrent);if(std::abs(cutoffCurrent-cutoffTarget)<std::max(.01f,cutoffTarget*2e-6f)){cutoffCurrent=cutoffTarget;filterDirty=false;}for(auto& row:antiAlias){row[0].set(cutoffCurrent,sr,.5411961f);row[1].set(cutoffCurrent,sr,1.306563f);}}
        const float input[]={in.l,in.r};for(std::size_t c=0;c<2;++c)line[c].push(antiAlias[c][1].tick(antiAlias[c][0].tick(input[c])));
        ratio+=sm*(targetRatio-ratio);window+=sm*(targetWindow-window);
        phase+=(1-ratio)/window;phase-=std::floor(phase);
        const float p2=phase<.5f?phase+.5f:phase-.5f;
        const float w=hannWeight(*hann,phase);
        auto read=[&](std::size_t c){return w*line[c].readCubic(32+phase*window)+(1-w)*line[c].readCubic(32+p2*window);};
        return {read(0),read(1)};
    }
};
// A bounded stereo grain scheduler. Each voice retains its read trajectory;
// scheduling starts only after the entire trajectory fits recorded history.
class Granular {
    struct Grain {float age=0,length=0,start=0,rate=1;bool active=false;};
    std::array<Grain,8> grains{};
    std::array<Delay,2> source;
    std::array<std::array<Lowpass,2>,2> antiAlias;
    float sr=48000,recorded=0,countdown=0,lastSemi=999,rate=2,companion=1,gain=.5f,smooth=0;
    unsigned sequence=0;
public:
    void prepare(float sampleRate){sr=sampleRate;smooth=1-std::exp(-1/(sr*.03f));for(auto& d:source)d.prepare(std::size_t(sr*.7f)+64);lastSemi=999;clear();}
    void clear(){for(auto& d:source)d.clear();for(auto& row:antiAlias)for(auto& f:row)f.clear();grains={};recorded=countdown=0;sequence=0;}
    Stereo tick(Stereo in,const Parameters& p){
        const float semi=semitones[std::clamp(p.interval,0,5)];
        if(semi!=lastSemi){lastSemi=semi;rate=std::pow(2.f,semi/12);companion=std::pow(2.f,(semi>17?semi-12:semi+7)/12);
            const float cutoff=sr*.43f/std::max({1.f,rate,companion});
            for(auto& row:antiAlias){row[0].set(cutoff,sr,.5411961f);row[1].set(cutoff,sr,1.306563f);}}
        source[0].push(antiAlias[0][1].tick(antiAlias[0][0].tick(in.l)));source[1].push(antiAlias[1][1].tick(antiAlias[1][0].tick(in.r)));
        recorded=std::min(recorded+1,sr*.65f);
        const float overlap=2+4*std::clamp(p.delayMotion,0.f,1.f);
        const float duration=sr*std::clamp(.12f/std::max(.05f,p.delayRate),.035f,.16f);
        if(--countdown<=0){
            for(auto& g:grains)if(!g.active){
                const float speed=(sequence++%3==2)?companion:rate;
                const float length=std::min(duration,(recorded-48)/std::max(1.f,speed));
                if(length>=sr*.008f){const float spray=float((sequence*37u)%101u)/100*sr*.012f*std::clamp(p.delayMotion,0.f,1.f);
                    const float start=32+std::max(0.f,(speed-1)*length)+std::min(spray,std::max(0.f,recorded-40-std::max(0.f,(speed-1)*length)));
                    g={0,length,start,speed,true};countdown=length/overlap;
                }else countdown=sr*.005f;
                break;
            }
        }
        gain+=smooth*(2/overlap-gain);
        Stereo out{};
        for(auto& g:grains)if(g.active){const float weight=hannWeight(hannTable(),g.age/g.length)*gain;
            const float delay=g.start+g.age*(1-g.rate);
            out.l+=source[0].readCubic(delay)*weight;out.r+=source[1].readCubic(delay)*weight;
            if(++g.age>=g.length)g.active=false;}
        return out;
    }
};

class Echo {
    std::array<Delay,2> line,reverseLine;
    std::array<OnePole,2> lp,hp;
    Pitch pitch;Granular grains;
    float sr=48000,oldTime=18000,newTime=18000,fade=1,wanted=18000;
    float bOld=24000,bNew=24000,bFade=1,bWanted=24000;
    float bFb=.32f,bVolume=1,diffusionDepth=.7f;
    float fb=.3f,amount=.24f,motion=.15f,width=1,on=1,lpA=0,hpA=0;
    int currentType=0,pendingType=0,currentPitchMode=0,pendingPitchMode=0; bool switching=false;float modeGain=1;
    float sm=0,lpTarget=0,hpTarget=0;
    float flutter=0,flutterSin=0,flutterCos=1,rotSin=0,rotCos=1;unsigned flutterClock=0;
    float wowSin=0,wowCos=1,wowRotSin=0,wowRotCos=1;
    const std::array<float,1025>* hann=nullptr;
    std::array<float,2> reverseOldPhase{},reverseNewPhase{};
    bool swellOpen=false;float swellQuiet=0;
    std::array<OnePole,2> bucketPole;std::array<std::array<Allpass,3>,2> smear;float swellGain=1,holdCount=0;Stereo held{};
public:
    Stereo feedOut{};
    void prepare(float rate,const Parameters& p){
        hann=&hannTable();sr=rate;sm=1-std::exp(-1.f/(sr*.025f));for(auto& d:line)d.prepare(std::size_t(sr*16.05f)+16);
        for(auto& d:reverseLine)d.prepare(std::size_t(sr*16.05f)+16);
        pitch.prepare(sr);grains.prepare(sr);for(std::size_t c=0;c<2;++c)for(std::size_t i=0;i<3;++i)smear[c][i].prepare(sr,(3.7f+float(i)*5.83f)*(c?1.13f:1.f));clear();swellGain=p.swell>0?0.f:1.f;bFb=p.bFeedback;bVolume=p.bLevel;diffusionDepth=p.delayDiffusion;fb=p.feedback;amount=p.delayAmount;width=p.delayWidth;motion=p.delayMotion;
        on=p.delayOn?1.f:0.f;oldTime=newTime=wanted=delaySamples(p);fade=1;bOld=bNew=bWanted=std::clamp((p.sync?60000.f/std::clamp(p.bpm,20.f,400.f)*beats[std::clamp(p.bRhythm,0,8)]:p.bTime)*sr*.001f,32.f,8*sr);bFade=1;currentType=pendingType=delayModel(p.delayType,p.delayModel);switching=false;modeGain=1;
        currentPitchMode=pendingPitchMode=p.pitchMode;
        lpA=pole(p.delayTone,sr);hpA=pole(p.delayLowcut,sr);
    }
    void clear(){for(auto& d:line)d.clear();for(auto& d:reverseLine)d.clear();pitch.clear();grains.clear();lp={};hp={};bucketPole={};for(auto& row:smear)for(auto& a:row)a.line.clear();swellGain=1;swellOpen=false;swellQuiet=0;holdCount=0;held={};reverseOldPhase={};reverseNewPhase={};flutterSin=wowSin=0;flutterCos=wowCos=1;flutterClock=0;feedOut={};}
    float delaySamples(const Parameters& p)const noexcept {
        const float ms=p.sync?60000.f/std::clamp(p.bpm,20.f,400.f)*beats[std::clamp(p.rhythm,0,8)]:p.time;
        return std::clamp(ms*.001f*sr,32.f,8.f*sr);
    }
    void set(const Parameters& p) noexcept {
        wanted=delaySamples(p);bWanted=std::clamp((p.sync?60000.f/std::clamp(p.bpm,20.f,400.f)*beats[std::clamp(p.bRhythm,0,8)]:p.bTime)*sr*.001f,32.f,8*sr);
        const int effectiveType=delayModel(p.delayType,p.delayModel);
        float cutoff=p.delayTone;
        if(effectiveType==1)cutoff*=.8f;
        if(effectiveType==2)cutoff=std::min(cutoff,1200.f+3500.f*std::min(1.f,sr*.15f/wanted));
        lpTarget=pole(cutoff,sr);hpTarget=pole(p.delayLowcut,sr);
        pitch.setModern(p.soundVersion!=0);pitch.set(semitones[std::clamp(p.interval,0,5)]);
        const float rotation=2*pi*6.73f/sr;rotSin=std::sin(rotation);rotCos=std::cos(rotation);
        const float wowRotation=2*pi*p.delayRate/sr;wowRotSin=std::sin(wowRotation);wowRotCos=std::cos(wowRotation);
        const int type=delayModel(p.delayType,p.delayModel);
        if(pendingType!=type){pendingType=type;switching=true;}
        if(pendingPitchMode!=p.pitchMode){pendingPitchMode=p.pitchMode;if(currentType==5||pendingType==5||currentType==7||pendingType==7)switching=true;else currentPitchMode=pendingPitchMode;}
    }
    Stereo tick(Stereo in,const Parameters& p,bool inject=true) noexcept {
        if(switching){modeGain=std::max(0.f,modeGain-1.f/(sr*.012f));if(modeGain<=0){if(currentType==4||pendingType==4){for(auto& d:line)d.clear();for(auto& d:reverseLine)d.clear();reverseOldPhase={};reverseNewPhase={};}if(pendingType==5||pendingType==7){pitch.clear();grains.clear();}currentType=pendingType;currentPitchMode=pendingPitchMode;switching=false;}}
        else modeGain=std::min(1.f,modeGain+1.f/(sr*.012f));
        auto follow=[&](float& x,float t){x+=sm*(t-x);};
        follow(bFb,std::clamp(p.bFeedback,0.f,.88f));follow(bVolume,p.bLevel);follow(diffusionDepth,p.delayDiffusion);
        follow(fb,std::min(.92f,p.feedback+p.bloom*.07f));follow(amount,p.delayAmount);follow(motion,p.delayMotion);follow(width,p.delayWidth);
        follow(on,p.delayOn&&inject?1.f:0.f);
        follow(lpA,lpTarget);follow(hpA,hpTarget);
        if(motion>1e-7f&&currentType!=4){const float s=wowSin*wowRotCos+wowCos*wowRotSin;wowCos=wowCos*wowRotCos-wowSin*wowRotSin;wowSin=s;
            if((++flutterClock&4095u)==0){const float norm=1/std::sqrt(wowSin*wowSin+wowCos*wowCos);wowSin*=norm;wowCos*=norm;const float f=1/std::sqrt(flutterSin*flutterSin+flutterCos*flutterCos);flutterSin*=f;flutterCos*=f;}}
        if(currentType==1&&motion>1e-7f){const float s=flutterSin*rotCos+flutterCos*rotSin;flutterCos=flutterCos*rotCos-flutterSin*rotSin;flutterSin=s;flutter=flutterSin;}
        const float depth=(currentType==1?3.f:currentType==2?1.5f:.4f)*sr*.001f*motion;
        // Coalesce automation while a fade is active; service the latest target even
        // when Engine::set correctly skips the next identical parameter block.
        if(currentType!=1&&fade>=1&&std::abs(wanted-newTime)>1){oldTime=newTime;newTime=wanted;reverseOldPhase[0]=reverseNewPhase[0];fade=0;}
        if(currentType==1){newTime+=sm*(wanted-newTime);oldTime=newTime;fade=1;}
        fade=std::min(1.f,fade+1.f/(sr*.045f));
        const float ratio=p.stereo==1?p.ratio:1.f;
        const float bTarget=bWanted;
        if(p.dualMode==1){if(bFade>=1&&std::abs(bTarget-bNew)>1){bOld=bNew;bNew=bTarget;reverseOldPhase[1]=reverseNewPhase[1];bFade=0;}bFade=std::min(1.f,bFade+1.f/(sr*.045f));}
        float a[2]{};
        for(std::size_t c=0;c<2;++c){
            const bool independent=c==1&&p.dualMode==1;
            const float scale=c==1?ratio:1.f;
            const float duration=independent?bNew:newTime*scale;
            const float previous=independent?bOld:oldTime*scale;
            const float blend=independent?bFade:fade;
            const float wobble=motion>1e-7f?depth*((c==0?wowSin:wowSin*.45359612f+wowCos*.89120736f)+(currentType==1?.12f*flutter:0.f)):0.f;
            auto taps=[&](float time){
                if(currentType!=3)return line[c].read(std::max(1.f,time-1+wobble));
                if(p.soundVersion==0)return .48f*line[c].read(std::max(1.f,time-1+wobble))+.24f*line[c].read(time*.5f-1)+.16f*line[c].read(time*.75f-1)+.12f*line[c].read(time*(c==0?.333333f:.666667f)-1);
                const int count=p.pattern==0?2:p.pattern==1?4:8;float sum=0;
                for(int i=0;i<count;++i)sum+=line[c].read(std::max(1.f,time*float(i+1)/float(count)-1+wobble))*(1.f-float(i)*.06f);
                return sum/(float(count)*(1.f-float(count-1)*.03f));
            };
            if(currentType==4){
                auto reverse=[&](float time,float& phase){phase+=1.f/std::max(32.f,time);phase-=std::floor(phase);const float other=phase<.5f?phase+.5f:phase-.5f;const float weight=hannWeight(*hann,phase);return weight*reverseLine[c].read(16+2*phase*time)+(1-weight)*reverseLine[c].read(16+2*other*time);};
                const float next=reverse(duration,reverseNewPhase[c]);
                a[c]=blend>=1?next:(1-blend)*reverse(previous,reverseOldPhase[c])+blend*next;
                if(blend>=1)reverseOldPhase[c]=reverseNewPhase[c];
            }else a[c]=blend>=1?taps(duration):(1-blend)*taps(previous)+blend*taps(duration);
        }
        if(currentType==6)for(std::size_t c=0;c<2;++c){float diff=a[c];for(auto& ap:smear[c])diff=ap.tick(diff);a[c]+=(diff-a[c])*diffusionDepth;}
        if(currentType==7){const Stereo source{a[0],a[1]};auto grain=grains.tick(source,p);a[0]=grain.l;a[1]=grain.r;}
        if(currentType==8){if(holdCount<=0){held={std::round(a[0]*2048)/2048,std::round(a[1]*2048)/2048};holdCount=4;}holdCount-=1;a[0]=held.l;a[1]=held.r;}
        if(currentType==5&&currentPitchMode==0){auto s=pitch.tick({a[0],a[1]});a[0]=s.l;a[1]=s.r;}
        for(std::size_t c=0;c<2;++c){a[c]=lp[c].tick(a[c],lpA);a[c]-=hp[c].tick(a[c],hpA);
            if(currentType==2)a[c]=bucketPole[c].tick(a[c],lpA);
            if((currentType==1||currentType==2)&&p.drive>1e-7f){const float drive=1+p.drive*3;a[c]=std::tanh(a[c]*drive)/drive;}}
        if(p.swell>1e-7f){
            const float level=std::max(std::abs(in.l),std::abs(in.r));
            if(level>1e-4f){if(!swellOpen){swellOpen=true;swellGain=0;}swellQuiet=0;}
            else if(level<5e-5f){swellQuiet+=1/sr;if(swellQuiet>.08f)swellOpen=false;}
            const float step=1/(sr*(.01f+p.swell*1.5f));
            swellGain+=std::clamp((swellOpen?1.f:0.f)-swellGain,-1/(sr*.05f),step);
        }else swellGain+=sm*(1-swellGain);
        // Launch mono into the left feedback lane; retain stereo side information.
        const float mid=.5f*(in.l+in.r),side=.5f*(in.l-in.r);
        const float input[2]={(p.stereo==2?mid+side:in.l)*on*swellGain,(p.stereo==2?-side:in.r)*on*swellGain};
        for(std::size_t c=0;c<2;++c){const float f=p.stereo==2?a[1-c]:a[c];
            const float channelFeedback=c==1&&p.dualMode==1?bFb:fb;
            const float x=finite(input[c]+channelFeedback*f);if(currentType==4)reverseLine[c].push(x);else line[c].push(x);}
        if(currentType==5&&currentPitchMode==1){auto shifted=pitch.tick({a[0],a[1]});a[0]=shifted.l;a[1]=shifted.r;}
        if(p.dualMode==1)a[1]*=bVolume;
        const float wetMid=.5f*(a[0]+a[1]),wetSide=.5f*(a[0]-a[1])*width;
        feedOut={(wetMid+wetSide)*modeGain,(wetMid-wetSide)*modeGain};return {feedOut.l*amount,feedOut.r*amount};
    }
};
// Four parallel damped combs per channel, followed by serial allpasses.
class Room {
    std::array<std::array<Delay,4>,2> comb;
    std::array<std::array<float,4>,2> length{},gain{},gainTarget{};
    std::array<std::array<OnePole,4>,2> lp;
    std::array<std::array<Allpass,2>,2> ap;
    std::array<Delay,2> early;
    float sr=48000,damp=0,dampTarget=0,sm=0;
public:
    void prepare(float rate){sr=rate;sm=1-std::exp(-1.f/(sr*.025f));gain={};damp=0;constexpr float ms[]={19.1f,27.7f,34.3f,41.9f};
        for(std::size_t c=0;c<2;++c){for(std::size_t i=0;i<4;++i){length[c][i]=std::round(sr*.001f*(ms[i]+float(c)*1.7f));comb[c][i].prepare(std::size_t(length[c][i])+3);lp[c][i].z=0;}
            early[c].prepare(std::size_t(sr*.065f)+4);ap[c][0].prepare(sr,3.7f+float(c)*.3f);ap[c][1].prepare(sr,1.3f+float(c)*.2f);}}
    void clear(){for(auto& d:early)d.clear();for(auto& row:comb)for(auto& d:row)d.clear();for(auto& row:ap)for(auto& a:row)a.line.clear();lp={};}
    void set(float decay,float tone,bool frozen=false){dampTarget=frozen?0.f:pole(tone,sr);for(std::size_t c=0;c<2;++c)for(std::size_t i=0;i<4;++i)gainTarget[c][i]=frozen?1.f:std::pow(10.f,-3*length[c][i]/sr/decay);}
    Stereo tick(Stereo x){damp+=sm*(dampTarget-damp);float out[2]{};const float in[]={x.l,x.r};for(std::size_t c=0;c<2;++c){early[c].push(in[c]*.8f+in[1-c]*.2f);for(std::size_t i=0;i<4;++i){gain[c][i]+=sm*(gainTarget[c][i]-gain[c][i]);float d=lp[c][i].tick(comb[c][i].readInt(std::size_t(length[c][i]-1)),damp);comb[c][i].push(in[c]*.2f+in[1-c]*.05f+d*gain[c][i]);out[c]+=d*.42f;}for(auto& a:ap[c])out[c]=a.tick(out[c]);
        constexpr float reflectionMs[]={7.1f,13.7f,23.3f,37.9f};for(std::size_t t=0;t<4;++t)out[c]+=early[c].readInt(std::size_t(sr*.001f*(reflectionMs[t]+float(c)*1.3f)))*(.18f-float(t)*.025f);}
        return {out[0],out[1]};}
};
// Original coupled stereo allpass tank. A plate-like topology, not a hardware clone.
class Plate {
    std::array<Delay,2> tank; std::array<std::array<Allpass,4>,2> ap;
    float size=1,sizeTarget=1,movement=0,movementTarget=0,phase=0;bool modern=false;
    std::array<OnePole,2> lp;float sr=48000,length[2]{},loopLength[2]{},gain[2]{},gainTarget[2]{},damp=0,dampTarget=0,sm=0;
public:
    void prepare(float rate){sr=rate;sm=1-std::exp(-1.f/(sr*.025f));gain[0]=gain[1]=damp=0;for(std::size_t c=0;c<2;++c){length[c]=std::round(sr*(c==0?.0673f:.0839f));tank[c].prepare(std::size_t(length[c]*1.8f+sr*.002f)+4);lp[c].z=0;
        loopLength[c]=length[c];for(std::size_t i=0;i<4;++i){ap[c][i].prepare(sr,(3.1f+float(i)*4.73f)*(c==0?1.f:1.17f));loopLength[c]+=ap[c][i].length;}}}
    void clear(){for(auto& d:tank)d.clear();for(auto& row:ap)for(auto& a:row)a.line.clear();lp={};}
    void setGeometry(float scale,float motion){modern=true;sizeTarget=std::clamp(scale,.5f,1.7f);movementTarget=std::clamp(motion,0.f,1.f);}
    void set(float decay,float tone,bool frozen=false){dampTarget=frozen?0.f:pole(tone,sr);for(std::size_t c=0;c<2;++c)gainTarget[c]=frozen?1.f:std::pow(10.f,-3*(loopLength[c]+length[c]*(sizeTarget-1))/sr/decay);}
    Stereo tick(Stereo x){size+=sm*(sizeTarget-size);movement+=sm*(movementTarget-movement);phase+=2*pi*.19f/sr;if(phase>2*pi)phase-=2*pi;
        const float wobble[]={modern?movement*sr*.0006f*std::sin(phase):0.f,modern?movement*sr*.0006f*std::sin(phase+1.7f):0.f};
        auto read=[&](std::size_t c,float fraction){return modern?tank[c].read(std::max(1.f,length[c]*size*fraction-1+wobble[c])):tank[c].readInt(std::size_t(length[c]*fraction-1));};
        damp+=sm*(dampTarget-damp);for(std::size_t c=0;c<2;++c)gain[c]+=sm*(gainTarget[c]-gain[c]);float d[2]={lp[0].tick(read(0,1),damp),lp[1].tick(read(1,1),damp)};
        float v[2]={x.l*.4f+(d[0]+d[1])*.70710678f*gain[0],x.r*.4f+(d[0]-d[1])*.70710678f*gain[1]};
        for(std::size_t c=0;c<2;++c){for(auto& a:ap[c])v[c]=a.tick(v[c]);tank[c].push(v[c]);}
        return {.55f*d[0]+.25f*(modern?read(0,.31f):tank[0].readInt(std::size_t(length[0]*.31f)))-.20f*(modern?read(1,.67f):tank[1].readInt(std::size_t(length[1]*.67f))),.55f*d[1]+.25f*(modern?read(1,.43f):tank[1].readInt(std::size_t(length[1]*.43f)))-.20f*(modern?read(0,.73f):tank[0].readInt(std::size_t(length[0]*.73f)))};}
};

// New models use orthogonal scattering, early reflections and model-specific
// diffusion. Halo/Studio Plate use a coupled allpass tank instead of the FDN.
class BoutiqueTank {
    std::array<Delay,8> lines;std::array<float,8> lengths{},wantedLengths{},feedback{},target{},oscS{},oscC{},rotS{},rotC{};
    std::array<std::array<Allpass,6>,2> diffuser;std::array<OnePole,8> damping,bass;
    std::array<std::array<Delay,4>,2> buildup;
    std::array<Delay,2> reflections;Plate plate;Pitch pitch;
    std::array<OnePole,2> pitchLow,pitchHigh;Stereo pitchReturn{};
    float sr=48000,sm=0,damp=0,dampTarget=0,bassPole=0,shimmerPole=0,attack=1,modulation=0,depth=0,regen=0,pitchBlend=0;
    float attackTarget=1,modulationTarget=0,depthTarget=0,regenTarget=0,pitchTarget=0;
    int model=1;bool frozen=false,configured=false;unsigned clock=0;
public:
    void prepare(float rate,int kind){sr=rate;model=kind;configured=false;lengths={};sm=1-std::exp(-1.f/(sr*.04f));
        for(std::size_t i=0;i<8;++i){if(model!=3&&model!=5)lines[i].prepare(std::size_t(sr*.31f)+16);oscS[i]=std::sin(float(i)*.83f);oscC[i]=std::cos(float(i)*.83f);const float angle=2*pi*(.07f+float(i)*.023f)/sr;rotS[i]=std::sin(angle);rotC[i]=std::cos(angle);}
        for(std::size_t c=0;c<2;++c){reflections[c].prepare(std::size_t(sr*.09f)+8);for(std::size_t i=0;i<6;++i)diffuser[c][i].prepare(sr,(2.9f+float(i)*3.73f)*(c?1.137f:1.f));}
        if(model==4||model==7)for(auto& row:buildup)for(auto& d:row)d.prepare(std::size_t(sr*.65f)+8);
        if(model==3||model==5)plate.prepare(sr);
        if(model==3||model==4){pitch.prepare(sr);pitch.setModern(true);}
        bassPole=pole(180,sr);shimmerPole=pole(6500,sr);clear();}
    void clear(){for(auto& row:buildup)for(auto& d:row)d.clear();for(auto& d:lines)d.clear();for(auto& d:reflections)d.clear();for(auto& row:diffuser)for(auto& a:row)a.line.clear();plate.clear();pitch.clear();damping={};bass={};pitchLow={};pitchHigh={};pitchReturn={};}
    void set(const Parameters& p){frozen=p.freeze;attackTarget=std::clamp(p.attack,.01f,2.f);depthTarget=std::clamp(p.diffusion,0.f,1.f);modulationTarget=p.reverbMotion;
        dampTarget=frozen?0:pole(p.reverbTone*(model==2?.85f:1.f),sr);regenTarget=std::clamp(p.regeneration,0.f,.7f);pitchTarget=p.shimmer;
        if(!configured){attack=attackTarget;depth=depthTarget;modulation=modulationTarget;regen=regenTarget;pitchBlend=pitchTarget;configured=true;}
        constexpr float ms[]={29.3f,37.1f,43.7f,53.3f,61.1f,73.9f,83.3f,97.7f};
        const float scale=(model==1?.55f:model==2?1.65f:1.f)*std::clamp(p.size,.5f,1.7f);
        for(std::size_t i=0;i<8;++i){wantedLengths[i]=sr*.001f*ms[i]*scale;if(lengths[i]==0)lengths[i]=wantedLengths[i];target[i]=frozen?1.f:std::pow(10.f,-3*wantedLengths[i]/sr/std::clamp(p.decay,.25f,20.f));}
        plate.setGeometry(p.size,p.reverbMotion);plate.set(p.decay,p.reverbTone,frozen);pitch.set(semitones[std::clamp(p.reverbInterval,0,5)]);}
    Stereo tick(Stereo in){damp+=sm*(dampTarget-damp);
        attack+=sm*(attackTarget-attack);depth+=sm*(depthTarget-depth);modulation+=sm*(modulationTarget-modulation);regen+=sm*(regenTarget-regen);pitchBlend+=sm*(pitchTarget-pitchBlend);
        float x[]={in.l*(frozen?0.f:1.f),in.r*(frozen?0.f:1.f)};
        const bool pitched=(model==3||model==4)&&pitchBlend>1e-6f;
        for(std::size_t c=0;c<2;++c){reflections[c].push(x[c]);x[c]+=(c?pitchReturn.r:pitchReturn.l)*regen*(pitched&&!frozen?1.f:0.f);
            const float source=x[c];const int count=model==1?2:model==2?6:4;for(int i=0;i<count;++i)x[c]=diffuser[c][std::size_t(i)].tick(x[c]);x[c]=source+(x[c]-source)*depth;}
        // Allpass input diffusion builds new energy without gating the stored tail.
        if(model==4||model==7)for(std::size_t c=0;c<2;++c)for(std::size_t i=0;i<4;++i){
            const float time=sr*attack*(.11f+float(i)*.047f)*(c?1.07f:1.f);
            const float g=model==4?.55f:.7f;
            const float delayed=buildup[c][i].read(std::max(1.f,time-1));
            const float y=delayed-g*x[c];buildup[c][i].push(x[c]+g*y);x[c]=y;
        }
        Stereo wet{};
        if(model==3||model==5){wet=plate.tick({x[0],x[1]});}
        else{
            std::array<float,8> delayed{};
            constexpr float il[]={1,1,-1,1,-1,-1,1,-1},ir[]={1,-1,1,1,-1,1,-1,-1};
            constexpr float tl[]={1,1,1,-1,1,-1,-1,-1},tr[]={1,-1,1,-1,-1,1,-1,1};
            for(std::size_t i=0;i<8;++i){const float next=oscS[i]*rotC[i]+oscC[i]*rotS[i];oscC[i]=oscC[i]*rotC[i]-oscS[i]*rotS[i];oscS[i]=next;
                lengths[i]+=sm*(wantedLengths[i]-lengths[i]);
                const float wobble=modulation*sr*(model==2?.0012f:.00035f)*oscS[i];delayed[i]=damping[i].tick(lines[i].read(lengths[i]-1+wobble),damp);
                // Frequency-dependent low decay without changing the full-band matrix norm.
                delayed[i]-=bass[i].tick(delayed[i],bassPole)*.025f*(frozen?0.f:1.f);
                wet.l+=delayed[i]*tl[i]*.35355339f;wet.r+=delayed[i]*tr[i]*.35355339f;}
            const auto mixed=wolf::BloomDSP::scatter(delayed);
            for(std::size_t i=0;i<8;++i){feedback[i]+=sm*(target[i]-feedback[i]);lines[i].push(finite(mixed[i]*feedback[i]+.22f*(il[i]*x[0]+ir[i]*x[1])));}
        }
        if(model==1||model==6){constexpr float ms[]={7.3f,13.1f,23.9f,41.3f};for(std::size_t c=0;c<2;++c){float early=0;for(int i=0;i<4;++i)early+=reflections[c].read(sr*.001f*(ms[i]+float(c)*1.7f))*(.18f-.025f*float(i));if(c)wet.r+=early;else wet.l+=early;}}
        if(pitched){auto up=pitch.tick(wet);float v[]={up.l,up.r};for(std::size_t c=0;c<2;++c){v[c]-=pitchHigh[c].tick(v[c],bassPole);v[c]=pitchLow[c].tick(v[c],shimmerPole);v[c]=std::tanh(v[c]*.65f);}
            pitchReturn={v[0],v[1]};wet.l+=pitchReturn.l*pitchBlend*.5f;wet.r+=pitchReturn.r*pitchBlend*.5f;}
        else pitchReturn={};
        if((++clock&4095u)==0)for(std::size_t i=0;i<8;++i){const float norm=1/std::sqrt(oscS[i]*oscS[i]+oscC[i]*oscC[i]);oscS[i]*=norm;oscC[i]*=norm;}
        return {finite(wet.l),finite(wet.r)};
    }
};

class Engine {
    std::array<BoutiqueTank,7> boutique;
    Echo echo;Room room;Plate plate;wolf::BloomDSP hall,bloomTank;
    std::array<Delay,2> pre;std::array<std::array<Allpass,3>,2> cloud;
    std::array<Delay,2> hallEarly;
    std::array<OnePole,2> low,shimmerBass,shimmerTreble;Pitch shimmer;
    Stereo pitchFeedback{}; Parameters p{};
    float sr=48000,env=0,delayEnv=0,duck=1,dd=1,mix=.38f,gain=1,delayActive=1,reverbActive=1,bloom=0;
    float feed=.65f,direct=1,verbAmount=.24f,verbWidth=1,shimmerAmount=0,freeze=0,bypass=1;
    float predelayOld=0,predelayNew=0,predelayFade=1,lowA=0,duckAttack=0,duckRelease=0,delayRelease=0;
    float smooth=0,gainTarget=1,pitchHP=0,pitchLP=0;std::array<float,11> weights{};std::array<float,11> tailGain{};
    int reverbCount=0,tankLimit=4;std::array<bool,11> live{};std::array<unsigned,11> quiet{};bool echoLive=false,shimmerLive=false,settingsReady=false;unsigned echoQuiet=0,echoUnused=0,shimmerQuiet=0,reverbUnused=0,idleFrames=0;
    static float peak(Stereo x) noexcept {return std::max(std::abs(x.l),std::abs(x.r));}
    void clearTank(int i) noexcept {if(i>=4)boutique[std::size_t(i-4)].clear();else if(i==0)room.clear();else if(i==1)plate.clear();else if(i==2){hall.reset();for(auto& early:hallEarly)early.clear();}else{bloomTank.reset();for(auto& row:cloud)for(auto& a:row)a.line.clear();}}
public:
    float delayPeak=0,reverbPeak=0,outputPeak=0;
    int activeReverbs() const noexcept {return reverbCount;}
    bool activeEcho() const noexcept {return echoLive;}
    bool activeShimmer() const noexcept {return shimmerLive;}
    void prepare(double rate,const Parameters& initial={}){
        sr=float(std::clamp(rate,8000.,192000.));tankLimit=4;p=initial;for(std::size_t i=0;i<boutique.size();++i)boutique[i].prepare(sr,int(i)+1);echo.prepare(sr,p);room.prepare(sr);plate.prepare(sr);
        wolf::Settings hs;hs.mix=1;hs.decay=p.decay;hs.predelay=0;hall.prepare(sr,hs);hs.motion=.6f;bloomTank.prepare(sr,hs,true);
        for(std::size_t c=0;c<2;++c){pre[c].prepare(std::size_t(sr*.21f)+8);low[c].z=0;
            hallEarly[c].prepare(std::size_t(sr*.065f)+8);
            for(std::size_t i=0;i<3;++i)cloud[c][i].prepare(sr,(11.f+float(i)*17.f)*(c==0?1.f:1.13f));}
        shimmer.prepare(sr);shimmerBass={};shimmerTreble={};pitchHP=pole(300,sr);pitchLP=pole(6500,sr);pitchFeedback={};env=delayEnv=0;duck=dd=1;mix=p.send?1:p.mix;gain=std::pow(10.f,p.output/20.f);
        delayActive=p.delayOn?1.f:0.f;reverbActive=p.reverbOn?1.f:0.f;feed=p.routing==1?0:p.feed;direct=p.direct;
        verbAmount=p.reverbAmount;verbWidth=p.reverbWidth;shimmerAmount=p.shimmer;freeze=0;bloom=p.bloom;bypass=1;
        predelayOld=predelayNew=p.predelay*.001f*sr;predelayFade=1;weights={};weights[std::size_t(reverbModel(p.reverbType,p.reverbModel))]=1;tailGain.fill(1);
        smooth=1-std::exp(-1.f/(sr*.03f));duckAttack=pole(1.f/(2*pi*.008f),sr);duckRelease=pole(1.f/(2*pi*.28f),sr);
        live={};reverbCount=0;quiet={};echoLive=shimmerLive=false;echoQuiet=echoUnused=shimmerQuiet=reverbUnused=0;settingsReady=false;set(p);
    }
    void clear(){for(auto& model:boutique)model.clear();echo.clear();room.clear();plate.clear();hall.reset();bloomTank.reset();for(auto& d:pre)d.clear();for(auto& row:cloud)for(auto& a:row)a.line.clear();shimmer.clear();pitchFeedback={};low={};shimmerBass={};shimmerTreble={};env=delayEnv=0;for(auto& early:hallEarly)early.clear();idleFrames=0;live={};reverbCount=0;quiet={};echoLive=shimmerLive=false;echoQuiet=echoUnused=shimmerQuiet=reverbUnused=0;delayPeak=reverbPeak=outputPeak=0;}
    void set(const Parameters& next){
        if(settingsReady&&p.values()==next.values())return;
        const bool updateDelay=!settingsReady||p.delayValues()!=next.delayValues();const bool updateReverb=!settingsReady||p.reverbValues()!=next.reverbValues();
        settingsReady=true;idleFrames=0;
        p=next;tankLimit=std::max(tankLimit,p.reverbModel?std::clamp(p.reverbModel,1,7)+4:4);duckRelease=std::exp(-1.f/(sr*p.reverbRelease*.001f));delayRelease=std::exp(-1.f/(sr*p.delayRelease*.001f));p.feedback=std::clamp(p.feedback,0.f,.88f);p.decay=std::clamp(p.decay,.25f,20.f);p.bpm=std::clamp(p.bpm,20.f,400.f);
        gainTarget=std::pow(10.f,p.output/20.f);if(updateReverb){shimmer.setModern(p.reverbVersion!=0);shimmer.set(semitones[std::clamp(p.reverbInterval,0,5)]);
        }if(updateDelay)echo.set(p);if(updateReverb){const float decay=std::min(24.f,p.decay*(1+p.bloom*.6f));
        room.set(std::min(8.f,decay),p.reverbTone,p.freeze);plate.set(decay,p.reverbTone,p.freeze);
        wolf::Settings hs;hs.freeze=p.freeze;hs.mix=1;hs.decay=decay;hs.predelay=0;hs.tone=p.reverbTone;hs.motion=p.reverbMotion;hs.lowcut=p.reverbLowcut;hs.width=1;hall.set(hs);
        hs.decay=std::min(20.f,decay*1.25f);hs.motion=std::max(.25f,p.reverbMotion);hs.tone=p.reverbTone*.8f;bloomTank.set(hs);
        for(std::size_t i=0;i<boutique.size();++i)if(p.reverbModel==int(i)+1||live[i+4])boutique[i].set(p);
        lowA=pole(p.reverbLowcut,sr);}
        const float nextDelay=p.predelay*.001f*sr;
        if(predelayFade>=1&&std::abs(nextDelay-predelayNew)>.5f){predelayOld=predelayNew;predelayNew=nextDelay;predelayFade=0;}
    }
    // No allocation or file operations in processing. Internal bypass stops excitation and retains tails.
    Stereo tick(Stereo dry,bool hostBypass=false) noexcept {
        dry={finite(dry.l),finite(dry.r)};
        const bool excitationPossible=!hostBypass&&((p.delayOn&&(p.delayAmount>1e-7f||(p.reverbOn&&p.feed>1e-7f&&p.reverbAmount>1e-7f)))||(p.reverbOn&&p.reverbAmount>1e-7f&&p.direct>1e-7f));
        if(!echoLive&&!shimmerLive&&activeReverbs()==0&&!(excitationPossible&&peak(dry)>0.f)&&idleFrames>unsigned(sr)){
            const float dryLevel=p.send?0.f:(1-(hostBypass?0.f:float(p.delayOn||p.reverbOn))*(p.send?1.f:p.mix))*gainTarget;
            delayPeak=reverbPeak=0;Stereo result{finite(dry.l*dryLevel),finite(dry.r*dryLevel)};outputPeak=peak(result);return result;
        }
        auto follow=[&](float& x,float t){x+=smooth*(t-x);};
        follow(bypass,hostBypass?0.f:1.f);follow(reverbActive,p.reverbOn?1.f:0.f);follow(delayActive,p.delayOn?1.f:0.f);
        follow(feed,p.routing==1?0.f:p.feed);follow(direct,p.direct);follow(verbAmount,p.reverbAmount);follow(verbWidth,p.reverbWidth);
        follow(shimmerAmount,p.shimmer);follow(freeze,p.freeze?1.f:0.f);follow(bloom,p.bloom);
        const float amp=std::max(std::abs(dry.l),std::abs(dry.r));env=amp+(amp>env?duckAttack:duckRelease)*(env-amp);
        delayEnv=amp+(amp>delayEnv?duckAttack:delayRelease)*(delayEnv-amp);
        follow(duck,1.f/(1+env*p.reverbSense*p.reverbDuck*12));follow(dd,1.f/(1+delayEnv*p.delaySense*p.delayDuck*12));
        
        const bool echoNeeded=p.delayAmount>1e-7f||(p.reverbOn&&p.reverbAmount>1e-7f&&feed>1e-7f);
        const bool echoExcited=p.delayOn&&!hostBypass&&echoNeeded;
        Stereo echoInput{dry.l*bypass,dry.r*bypass};
        if(echoExcited&&peak(echoInput)>1e-9f)echoLive=true;
        Stereo d{};
        if(echoLive){d=echo.tick(echoInput,p,echoExcited);echoUnused=echoNeeded?0:echoUnused+1;
            echoQuiet=(echoExcited&&peak(echoInput)>1e-9f)||peak(echo.feedOut)>1e-9f?0:echoQuiet+1;
            // A full 16-second gap is possible in Dual/Reverse; output silence alone is insufficient.
            if(echoQuiet>unsigned(sr*16.25f)||echoUnused>unsigned(sr)){echo.clear();echoLive=false;echoQuiet=echoUnused=0;}}
        d.l*=dd;d.r*=dd;
        delayPeak=std::max(std::abs(d.l),std::abs(d.r));
        Stereo input{(dry.l*direct+echo.feedOut.l*dd*feed)*reverbActive*bypass,(dry.r*direct+echo.feedOut.r*dd*feed)*reverbActive*bypass};
        reverbUnused=(p.reverbAmount>1e-7f||p.freeze)?0:std::min(reverbUnused+1,unsigned(sr)+2);
        if(reverbUnused>unsigned(sr)){input={};pitchFeedback={};if(reverbUnused==unsigned(sr)+1){for(std::size_t i=0;i<live.size();++i)if(live[i]){clearTank(int(i));live[i]=false;--reverbCount;quiet[i]=0;weights[i]=0;}for(auto& preLine:pre)preLine.clear();low={};}}
        // Freeze holds tank feedback while removing fresh excitation; modulation may slowly dissipate energy.
        input.l=input.l*(1-freeze)+pitchFeedback.l*shimmerAmount*.025f*(1-freeze);
        input.r=input.r*(1-freeze)+pitchFeedback.r*shimmerAmount*.025f*(1-freeze);
        float raw[2]={input.l,input.r};
        const float wantedPre=p.predelay*.001f*sr;
        if(predelayFade>=1&&std::abs(wantedPre-predelayNew)>.5f){predelayOld=predelayNew;predelayNew=wantedPre;predelayFade=0;}
        predelayFade=std::min(1.f,predelayFade+1.f/(sr*.04f));
        for(std::size_t c=0;c<2;++c){const float bass=low[c].tick(raw[c],lowA);raw[c]-=bass*(1-std::clamp(p.bassDecay,0.f,1.f));
            pre[c].push(raw[c]);raw[c]=(1-predelayFade)*pre[c].read(predelayOld)+predelayFade*pre[c].read(predelayNew);}
        const int selected=reverbModel(p.reverbType,p.reverbModel);
        for(std::size_t i=0;i<std::size_t(tankLimit);++i)if(live[i]||selected==int(i)||weights[i]>0)follow(weights[i],selected==int(i)?1.f:0.f);
        // Allocation is retained; DSP runs only for excitation or an existing tail.
        Stereo wet{};
        for(std::size_t i=0;i<std::size_t(tankLimit);++i){if(!live[i]&&weights[i]==0)continue;Stereo excitation{raw[0]*weights[i],raw[1]*weights[i]};
            if(peak(excitation)>1e-9f){if(!live[i]){tailGain[i]=1;++reverbCount;}live[i]=true;}
            if(!live[i])continue;
            Stereo tail{};
            if(i>=4)tail=boutique[i-4].tick(excitation);else if(i==0)tail=room.tick(excitation);else if(i==1)tail=plate.tick(excitation);else if(i==2){tail=excitation;hall.process(&tail.l,&tail.r,1,true);
                const float source[]={excitation.l,excitation.r};float reflected[2]{};for(std::size_t c=0;c<2;++c){hallEarly[c].push(source[c]*.75f+source[1-c]*.25f);constexpr float times[]={11.7f,21.3f,34.1f,51.7f};for(std::size_t t=0;t<4;++t)reflected[c]+=hallEarly[c].readInt(std::size_t(sr*.001f*(times[t]+float(c)*2.1f)))*(.13f-float(t)*.02f);}tail.l+=reflected[0];tail.r+=reflected[1];
            }else{tail=excitation;for(auto& a:cloud[0])tail.l=a.tick(tail.l);for(auto& a:cloud[1])tail.r=a.tick(tail.r);bloomTank.process(&tail.l,&tail.r,1,true);}
            // Bound outgoing modern tails; accelerate retirement under rapid browsing.
            if(i>=4&&int(i)!=selected)tailGain[i]=std::max(0.f,tailGain[i]-1.f/(sr*(activeReverbs()>3?.04f:2.f)));
            wet.l+=tail.l*tailGain[i];wet.r+=tail.r*tailGain[i];
            quiet[i]=peak(excitation)>1e-9f||peak(tail)>1e-9f?0:quiet[i]+1;
            if(quiet[i]>unsigned(sr*.75f)||tailGain[i]<=0){clearTank(int(i));live[i]=false;--reverbCount;quiet[i]=0;weights[i]=0;}
        }
        while(tankLimit>4&&tankLimit>selected+1&&!live[std::size_t(tankLimit-1)]&&weights[std::size_t(tankLimit-1)]==0)--tankLimit;
        wet={finite(wet.l),finite(wet.r)};
        if(p.reverbModel!=3&&p.reverbModel!=4&&shimmerAmount>1e-7f&&(peak(wet)>1e-9f||shimmerLive)){shimmerLive=true;auto shifted=shimmer.tick(wet);float shiftedChannels[]={shifted.l,shifted.r};
            for(std::size_t c=0;c<2;++c){shiftedChannels[c]-=shimmerBass[c].tick(shiftedChannels[c],pitchHP);shiftedChannels[c]=shimmerTreble[c].tick(shiftedChannels[c],pitchLP);}
            pitchFeedback={std::tanh(shiftedChannels[0]*.7f),std::tanh(shiftedChannels[1]*.7f)};
            wet.l+=shifted.l*shimmerAmount*.6f;wet.r+=shifted.r*shimmerAmount*.6f;
            shimmerQuiet=peak(wet)>1e-9f||peak(shifted)>1e-9f?0:shimmerQuiet+1;
            if(shimmerQuiet>unsigned(sr*.3f)){shimmer.clear();shimmerBass={};shimmerTreble={};pitchFeedback={};shimmerLive=false;shimmerQuiet=0;}
        }else if(shimmerLive){shimmer.clear();shimmerBass={};shimmerTreble={};pitchFeedback={};shimmerLive=false;shimmerQuiet=0;}
        const float mid=(wet.l+wet.r)*.5f,side=(wet.l-wet.r)*.5f*verbWidth;
        wet={(mid+side)*verbAmount*duck,(mid-side)*verbAmount*duck};
        reverbPeak=std::max(std::abs(wet.l),std::abs(wet.r));
        follow(mix,p.send?1.f:p.mix);follow(gain,gainTarget);
        // Wet amounts are calibrated around .30; overall mix remains a true dry/wet control.
        const float active=std::max(delayActive,reverbActive)*bypass;
        const float dryGain=p.send?0.f:1-mix*active;
        Stereo out{(dryGain*dry.l+mix*(d.l+wet.l)*2.5f)*gain,(dryGain*dry.r+mix*(d.r+wet.r)*2.5f)*gain};
        outputPeak=std::max(std::abs(out.l),std::abs(out.r));
        idleFrames=echoLive||shimmerLive||activeReverbs()>0||(excitationPossible&&peak(dry)>0.f)?0:std::min(idleFrames+1,unsigned(sr)+1);
        return {finite(out.l),finite(out.r)};
    }
};
} // namespace tide
