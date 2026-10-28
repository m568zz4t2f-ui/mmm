// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "BloomDSP.h"
#include <limits>
#include <numeric>
namespace prism {
constexpr float pi=wolf::pi;
struct Stereo {float l=0,r=0;};
inline Stereo operator+(Stereo a,Stereo b){return {a.l+b.l,a.r+b.r};}
inline Stereo operator-(Stereo a,Stereo b){return {a.l-b.l,a.r-b.r};}
inline Stereo operator*(Stereo a,float b){return {a.l*b,a.r*b};}
inline float clean(float x){return std::isfinite(x)?std::clamp(x,-8.f,8.f):0.f;}
inline Stereo clean(Stereo x){return {clean(x.l),clean(x.r)};}
inline float peak(Stereo x){return std::max(std::abs(x.l),std::abs(x.r));}
inline float blend(float a,float b,float v){return a+(b-a)*v;}
inline Stereo blend(Stereo a,Stereo b,float v){return a+(b-a)*v;}
inline constexpr const char* effects[4][5]={{"Drive","Sweeten","Fuzz","Howl","Swell"},{"Doubler","Vibrato","Phaser","Tremolo","Pitch"},{"Cascade","Reels","Space","Collage","Reverse"},{"Filter","Squash","Cassette","Broken","Interference"}};
inline constexpr const char* groups[]={"CHARACTER","MOVEMENT","DIFFUSION","TEXTURE"};
enum FloatID {Tilt,Rate,Time,Mix,CharacterAmount,MovementAmount,DiffusionAmount,TextureAmount,Sensitivity,MovementDrift,DiffusionDrift,Output,CharacterVolume,MovementVolume,DiffusionVolume,TextureVolume,BPM,CaptureLevel,FloatCount};
inline constexpr const char* floatIDs[]={"tilt","rate","time","mix","characterAmount","movementAmount","diffusionAmount","textureAmount","sensitivity","movementDrift","diffusionDrift","output","characterVolume","movementVolume","diffusionVolume","textureVolume","bpm","captureLevel"};
inline constexpr const char* floatNames[]={"Character Tilt","Movement Rate","Diffusion Time","Global Mix","Character Amount","Movement Amount","Diffusion Amount","Texture Amount","Character Sensitivity","Movement Drift","Diffusion Drift","Output Level","Character Volume","Movement Volume","Diffusion Wet Volume","Texture Volume","Manual Tempo","Capture Level"};
inline constexpr std::array<float,FloatCount> initial={.5f,.35f,.4f,1.f,.25f,.25f,.3f,.5f,.5f,.1f,.1f,0.f,0.f,0.f,0.f,0.f,120.f,.8f};
struct Settings {
 std::array<float,FloatCount> v=initial;
 std::array<int,4> type={1,0,2,0};std::array<bool,4> enabled={true,true,true,false};
 int route=0,filter=0,dualMask=15;bool bypass=false,trails=true,sync=false,capturePre=false,dualBypass=false,gestureRecord=false,gesturePlay=true;
};
inline std::array<int,4> order(int route){std::array<int,4> p={0,1,2,3};for(int i=0;i<std::clamp(route,0,23);++i)std::next_permutation(p.begin(),p.end());return p;}
inline Settings sanitise(Settings p){for(int i=0;i<FloatCount;++i){float lo=0,hi=1;if(i==BPM){lo=20;hi=300;}else if(i==Output||(i>=CharacterVolume&&i<=TextureVolume)){lo=-18;hi=12;}p.v[i]=std::clamp(std::isfinite(p.v[i])?p.v[i]:initial[i],lo,hi);}for(auto& t:p.type)t=std::clamp(t,0,4);p.route=std::clamp(p.route,0,23);p.filter=std::clamp(p.filter,0,2);p.dualMask=std::clamp(p.dualMask,0,15);return p;}
struct Filter {float z=0;float lp(float x,float c){z+=c*(x-z);return z;}float hp(float x,float c){return x-lp(x,c);}};
struct DC {float prev=0,z=0;float tick(float x){float y=x-prev+.995f*z;prev=x;z=std::isfinite(y)?y:0;return z;}};
struct ADAA {float old=0;static double integral(double x){double a=std::abs(x);return a+std::log1p(std::exp(-2*a))-.69314718056f;}float tick(float x){x=std::clamp(x,-30.f,30.f);float d=x-old;float y=std::abs(d)<.0001f?std::tanh(.5f*(x+old)):float((integral(x)-integral(old))/d);old=x;return y;}};
struct SVF {float s1=0,s2=0;float tick(float x,float g,float k){float a=1/(1+g*(g+k));float v1=a*(s1+g*(x-s2));float v2=s2+g*v1;s1=2*v1-s1;s2=2*v2-s2;return v2;}};
struct Osc {float s=0,c=1,rs=0,rc=1;unsigned n=0;void set(float hz,float sr){float a=2*pi*hz/sr;rs=std::sin(a);rc=std::cos(a);}float tick(){float t=s*rc+c*rs;c=c*rc-s*rs;s=t;if((++n&4095)==0){float g=1/std::sqrt(s*s+c*c);s*=g;c*=g;}return s;}};
class Movement {
 std::array<wolf::Delay,2> line;std::array<std::array<float,12>,2> ap{};Osc osc,slow;float sr=48000,pitchPhase=0,rateHz=.5f,drift=0,amount=0;int mode=0,stages=4;float pitchRatio=1,aa=.6f;std::array<std::array<Filter,4>,2> anti;
public:
 void prepare(float s){sr=s;for(auto& l:line)l.prepare(std::size_t(sr*.2f)+16);ap={};osc={};slow={};slow.set(.137f,sr);}
 void set(int m,float rate,float amt,float d,bool sync,float bpm){mode=m;amount=amt;drift=d;rateHz=sync?bpm/60*std::pow(2.f,std::round(rate*6)-3):.07f*std::pow(130.f,rate);osc.set(rateHz,sr);stages=2+2*std::min(5,int(amt*6));pitchRatio=std::pow(2.f,rate*2-1);aa=1-std::exp(-2*pi*std::min(sr*.42f,sr*.42f/std::max(1.f,pitchRatio))/sr);}
 Stereo tick(Stereo x){float lfo=osc.tick(),warble=slow.tick();float v[2]={x.l,x.r};for(int c=0;c<2;++c){float mod=lfo*(1-drift*.4f)+warble*drift*.6f+(c?osc.c*drift*.35f:0);float y=v[c];
  if(mode==0||mode==1){line[c].push(v[c]);float ms=mode==0?12+rateHz*.9f+float(c)*3.5f:9;float depth=mode==0?1.5f+drift*5:amount*6;float z=line[c].readCubic(std::max(2.f,sr*.001f*(ms+depth*mod)));y=mode==0?blend(v[c],z,amount*.65f):z;}
  else if(mode==2){float f=std::clamp(600+1500*(mod+1),40.f,sr*.4f);float g=std::tan(pi*f/sr),a=(1-g)/(1+g);for(int i=0;i<stages;++i){float z=-a*y+ap[c][i];ap[c][i]=y+a*z;y=z;}y=blend(v[c],y,amount*.8f);}
  else if(mode==3){float wave=amount>.65f?std::tanh(mod*(1+(amount-.65f)*25)):mod;y*=1-amount*.5f*(wave+1);}
  else {float in=v[c];for(auto& a:anti[c])in=a.lp(in,aa);line[c].push(in);if(std::abs(pitchRatio-1)<.0001f)y=v[c];else{float p2=pitchPhase+.5f;if(p2>=1)p2-=1;float w=.5f-.5f*std::cos(2*pi*pitchPhase);float z1=line[c].readCubic(8+pitchPhase*sr*.06f),z2=line[c].readCubic(8+p2*sr*.06f);float shifted=w*z1+(1-w)*z2;if(drift>.001f){float steps=std::pow(2.f,14-drift*8);shifted=std::round(shifted*steps)/steps;}y=blend(v[c],shifted,amount);}}
  v[c]=clean(y);
 }if(mode==4){pitchPhase+=(1-pitchRatio)/(sr*.06f);pitchPhase-=std::floor(pitchPhase);}return {v[0],v[1]};}
};
class Character {
 std::array<Filter,2> tiltLP;std::array<DC,2> dc;std::array<ADAA,2> sat;std::array<SVF,2> svf;float sr=48000,env=0,swell=0,attack=.001f,release=.001f,tone=.1f,tilt=.5f,amount=.2f,gain=1,sens=1,howlG=.1f;int mode=0;
public:
 void prepare(float s){sr=s;tiltLP={};dc={};sat={};svf={};env=swell=0;}
 void set(int m,float t,float a,float sensitivity){mode=m;tilt=t;amount=a;sens=std::pow(10.f,(sensitivity-.5f)*1.8f);gain=1+a*(m==2?55.f:m==0?14.f:3.f);tone=1-std::exp(-2*pi*900/sr);attack=1-std::exp(-1/(sr*(.008f+a*a*1.5f)));release=1-std::exp(-1/(sr*(.05f+a*.4f)));howlG=std::tan(pi*std::min(sr*.4f,120+t*t*4500)/sr);}
 Stereo tick(Stereo x){float e=peak(x)*sens;env+=(e>env?.02f:.0003f)*(e-env);float v[2]={x.l,x.r};for(int c=0;c<2;++c){float low=tiltLP[c].lp(v[c],tone),hi=v[c]-low;float t=(tilt-.5f)*1.2f;float z=low*(1-t)+hi*(1+t);
  if(mode==4){float target=env>.015f?1.f:0.f;if(c==0)swell+=(target>swell?attack:release)*(target-swell);z*=swell;}
  else if(mode==3){z=svf[c].tick(z,howlG,.12f+(1-amount)*.8f);z=sat[c].tick(z*(1+amount*20)*sens)/(1+amount*2);}
  else {if(mode==1)z/=1+env*amount*3;float bias=mode==2?(tilt-.5f)*.7f:0;z=(sat[c].tick(z*gain*sens+bias)-std::tanh(bias))/(std::sqrt(gain)*sens);if(mode==2)z*=1.7f;}
  v[c]=dc[c].tick(z);
 }return clean({v[0],v[1]});}
};
class Texture {
 std::array<wolf::Delay,2> line;std::array<Filter,2> low,tiltLP;std::array<DC,2> dc;std::array<ADAA,2> sat;Osc wow,flutter,drop,carrier;float sr=48000,amt=.5f,env=0,cut=.5f,tiltCoef=.1f;int mode=0,style=0;std::uint32_t rng=0x5350494e;float noise=0;
public:
 void prepare(float s){sr=s;for(auto& l:line)l.prepare(std::size_t(sr*.08f)+16);wow.set(.43f,sr);flutter.set(5.7f,sr);drop.set(.71f,sr);carrier.set(370,sr);low={};tiltLP={};dc={};sat={};env=0;}
 void set(int m,float a,int f){mode=m;amt=a;style=f;float hz=mode==0?(style==1?60*std::pow(320.f,a):style==2?25*std::pow(220.f,a):1000):18000*std::pow(.13f,a);cut=1-std::exp(-2*pi*std::min(hz,sr*.45f)/sr);tiltCoef=1-std::exp(-2*pi*1000/sr);}
 Stereo tick(Stereo x){float w=wow.tick(),fl=flutter.tick(),dr=drop.tick(),car=carrier.tick();env+=(peak(x)>env?.04f:.0002f)*(peak(x)-env);rng=rng*1664525u+1013904223u;noise+=.01f*(float(rng>>8)/8388608.f-1-noise);float v[2]={x.l,x.r};for(int c=0;c<2;++c){float z=v[c];
  if(mode==0){if(style==0){float lp=tiltLP[c].lp(z,tiltCoef);float t=(amt-.5f)*1.7f;z=amt==.5f?z:lp*(1-t)+(z-lp)*(1+t);}else if(style==1){if(amt<.9999f)z=low[c].lp(z,cut);}else if(amt>.0001f)z=low[c].hp(z,cut);}
  else if(mode==1){z/=1+env*amt*12;z=sat[c].tick(z*(1+amt*5))/(1+amt);}
  else if(mode==2){line[c].push(z);z=line[c].readCubic(sr*(.009f+amt*(w*.002f+fl*.00035f)));z=low[c].lp(z,cut);z=sat[c].tick(z*(1+amt*2))/(1+amt);}
  else if(mode==3){line[c].push(z);float d=std::max(0.f,dr-.2f);z=line[c].readCubic(4+sr*.018f*amt*d);z*=1-amt*std::max(0.f,w)*.75f;z*=1+car*amt*.25f;}
  else{float glitches=std::max(0.f,w-.4f)*amt;z=z*(1-glitches)+z*car*amt*.4f+noise*amt*env*.12f;float steps=std::pow(2.f,14-amt*11);float quant=std::round(z*steps)/steps;z=blend(z,quant,amt);}
  v[c]=mode==0?z:dc[c].tick(z);
 }return clean({v[0],v[1]});}
};
class Diffusion {
 std::array<wolf::Delay,2> line;std::array<Filter,2> low,bass;std::array<ADAA,2> sat;std::array<std::array<Filter,4>,2> anti;wolf::BloomDSP reverb;Osc wow;float sr=48000,time=.4f,amt=.3f,drift=.1f,delay=12000,targetDelay=12000,damping=.3f,feedback=.4f,phase=0,grain=6000,speed=1,window=12000;int mode=2;float feedbackGain=0;bool sleeping=false;std::uint64_t silent=0;Stereo last{};float bassCoef=.01f;
public:
 void prepare(float s){sr=s;for(auto& l:line)l.prepare(std::size_t(sr*4.1f)+32);wolf::Settings init;init.mix=1;reverb.prepare(s,init);wow={};wow.set(.31f,sr);low={};bass={};sat={};anti={};last={};silent=0;sleeping=false;delay=targetDelay=sr*.25f;}
 void clear(){for(auto& l:line)l.clear();reverb.reset();low={};bass={};last={};phase=0;silent=0;}
 void set(int m,float t,float a,float d,bool sync,float bpm){mode=m;time=t;amt=a;drift=d;constexpr float div[]={.125f,.25f,.5f,.75f,1,1.5f,2,4};targetDelay=sr*(sync?60/bpm*div[std::min(7,int(t*8))]:.025f+1.475f*t*t);targetDelay=std::clamp(targetDelay,16.f,sr*3.9f);feedback=std::min(a*.985f,std::pow(10.f,-3.f*(targetDelay/sr)/80.f));damping=1-std::exp(-2*pi*(m==0?2200.f:6500.f)/sr);bassCoef=1-std::exp(-2*pi*90/sr);speed=std::pow(2.f,2*t-1);window=sync?std::clamp(sr*60/bpm,128.f,sr*1.5f):sr*.45f;grain=std::clamp(targetDelay,128.f,sr*1.8f);wolf::Settings p;p.decay=.35f+19*t*t;p.predelay=4+20*t;p.tone=4000+6000*(1-t);p.motion=d*.7f;p.lowcut=130;p.mix=1;reverb.set(p);}
 Stereo tick(Stereo x,bool feed=true){if(!feed)x={};if(sleeping&&peak(x)<1e-9f)return {};sleeping=false;float mod=wow.tick();delay+=.00015f*(targetDelay-delay);Stereo wet{};
  if(mode==2){float l=x.l,r=x.r;reverb.process(&l,&r,1,false);wet={l,r};}
  else {float in[2]={x.l,x.r},out[2]{};for(int c=0;c<2;++c){float source=in[c]+feedback*(c?last.r:last.l);source=bass[c].hp(source,bassCoef);if(mode==0||mode==1){source=low[c].lp(source,damping);source=sat[c].tick(source*(mode==0?1.4f:1.8f))/(mode==0?1.4f:1.8f);line[c].push(source);out[c]=line[c].readCubic(std::max(2.f,delay+(c?1:-1)*drift*sr*.0012f*mod));}
   else {for(auto& f:anti[c])source=f.lp(source,.62f);line[c].push(source);float p2=phase+.5f;if(p2>=1)p2-=1;float w=.5f-.5f*std::cos(2*pi*phase);if(mode==4){out[c]=w*line[c].readCubic(16+phase*window*(1+speed))+(1-w)*line[c].readCubic(16+p2*window*(1+speed));}else{float r=drift>.5f&&mod>0?2.f:.5f+time*1.5f;out[c]=w*line[c].readCubic(16+phase*grain*r)+(1-w)*line[c].readCubic(16+p2*grain*r);}}
  }wet={out[0],out[1]};phase+=1/(mode==4?window:grain);if(phase>=1)phase-=1;last=clean(wet);}
  if(peak(x)<1e-9f&&peak(wet)<1e-7f){if(++silent>std::uint64_t(sr*24)){sleeping=true;}}else silent=0;
  return clean(wet);
 }
 Stereo apply(Stereo x,float amount,float vol,bool feed=true){auto wet=tick(x,feed)*vol;if(mode==0||mode==1||mode==3)return x+wet*.65f;return blend(x,wet,amount);}
};
struct LoopImage {double sampleRate=48000;std::vector<Stereo> audio;};
enum class Action {CaptureRecord,CapturePlay,CaptureStop,CaptureErase,CaptureDub,CaptureUndo,GestureClear,Clear};
class Capture {
 std::vector<Stereo> audio,undo;std::vector<unsigned> stamps;unsigned generation=1;std::size_t length=0,write=0;double pos=0,sr=48000;int status=0;bool hasUndo=false;float playGain=0;Stereo old{};
 Stereo dubAt(std::size_t i)const{return stamps[i/64]==generation?undo[i]:Stereo{};}
 void writeDub(std::size_t i,Stereo x){auto page=i/64;if(stamps[page]!=generation){auto first=page*64;std::fill(undo.begin()+first,undo.begin()+std::min(first+64,undo.size()),Stereo{});stamps[page]=generation;}undo[i]=x;}
public:
 void prepare(double s){sr=s;audio.assign(std::size_t(sr*30)+4,{});undo.resize(audio.size());stamps.assign((audio.size()+63)/64,0);generation=1;length=write=0;pos=0;status=0;hasUndo=false;playGain=0;}
 void act(Action a){if(a==Action::CaptureRecord){length=write=0;pos=0;status=1;hasUndo=false;++generation;}else if(a==Action::CapturePlay){if(status==1){length=write;if(length<16){status=0;return;}pos=0;}if(length)status=2;}else if(a==Action::CaptureStop)status=length?4:0;else if(a==Action::CaptureErase){length=write=0;status=0;pos=0;hasUndo=false;}else if(a==Action::CaptureDub&&length){status=status==3?2:3;hasUndo=true;}else if(a==Action::CaptureUndo&&hasUndo){++generation;hasUndo=false;status=2;}}
 Stereo tick(Stereo x,float level,bool reverse=false,float speed=1){Stereo out{};if(status==1){audio[write++]=x;if(write>=audio.size()-4){length=write;pos=0;status=2;}}else if((status==2||status==3)&&length){std::size_t i=std::size_t(pos)%length,j=(i+1)%length;out=blend(audio[i]+dubAt(i),audio[j]+dubAt(j),float(pos-std::floor(pos)));std::size_t fade=std::min<std::size_t>(length/8,std::size_t(sr*.008));if(fade&&i>=length-fade){float t=float(i-(length-fade))/float(fade);auto ahead=audio[i-(length-fade)]+dubAt(i-(length-fade));out=blend(out,ahead,t);}if(status==3){writeDub(i,clean(dubAt(i)*.98f+x*.7f));}pos+=reverse?-speed:speed;double period=double(length-fade);while(pos>=length)pos-=period;while(pos<fade)pos+=period;}
  float target=(status==2||status==3)?1.f:0.f;playGain+=.005f*(target-playGain);if(target)old=out;return x+old*(level*playGain);
 }
 int state()const{return status;}double seconds()const{return double(length)/sr;}
 LoopImage snapshot()const{LoopImage im;im.sampleRate=sr;auto n=status==1?write:length;im.audio.resize(n);for(std::size_t i=0;i<n;++i)im.audio[i]=audio[i]+dubAt(i);return im;}
 void restore(const LoopImage& im){length=std::min(audio.size()-4,std::size_t(im.audio.size()*sr/im.sampleRate));for(std::size_t i=0;i<length;++i){double p=double(i)*im.sampleRate/sr;auto a=std::min(im.audio.size()-1,std::size_t(p)),b=std::min(im.audio.size()-1,a+1);audio[i]=blend(im.audio[a],im.audio[b],float(p-a));}status=length?4:0;pos=0;playGain=0;hasUndo=false;++generation;}
};
struct Gesture {
 static constexpr int capacity=3000;std::array<std::array<float,8>,capacity> frames{};int count=0;float bpm=120;bool available=false;
};
class Engine {
 float sr=48000,follow=.002f,bypassFade=1,transition=1;Settings target{},current{};std::array<int,4> route={0,1,2,3};std::array<float,4> enables={1,1,1,0},gain={1,1,1,1};Character character;Movement movement;Texture texture;std::array<Diffusion,2> diff;int selected=0,tail=-1;float tailFade=0;std::uint64_t tailAge=0;unsigned control=0,gestureTick=0;double gesturePos=0;bool recording=false;Stereo previous{};std::array<int,4> previousTypes={1,0,2,0};float outGain=1;bool reversing=false;float loopSpeed=1;
 void coefficients(){character.set(target.type[0],current.v[Tilt],current.v[CharacterAmount],current.v[Sensitivity]);movement.set(target.type[1],current.v[Rate],current.v[MovementAmount],current.v[MovementDrift],target.sync,current.v[BPM]);texture.set(target.type[3],current.v[TextureAmount],target.filter);diff[selected].set(target.type[2],current.v[Time],current.v[DiffusionAmount],current.v[DiffusionDrift],target.sync,current.v[BPM]);for(int i=0;i<4;++i)gain[i]=std::pow(10.f,current.v[CharacterVolume+i]/20);outGain=std::pow(10.f,current.v[Output]/20);}
public:
 Capture capture;Gesture gesture;
 void prepare(double s,Settings p={}){sr=float(std::clamp(s,8000.,192000.));target=current=sanitise(p);follow=1-std::exp(-1/(sr*.02f));character.prepare(sr);movement.prepare(sr);texture.prepare(sr);for(auto& d:diff)d.prepare(sr);capture.prepare(sr);route=order(target.route);previousTypes=target.type;selected=0;tail=-1;control=0;transition=1;bypassFade=target.bypass?0:1;for(int i=0;i<4;++i)enables[i]=target.enabled[i]?1:0;coefficients();}
 void set(Settings p){p=sanitise(p);if(p.type!=previousTypes||p.route!=target.route){transition=0;previousTypes=p.type;route=order(p.route);}if(p.type[2]!=target.type[2]){tail=selected;tailFade=1;tailAge=0;selected=1-selected;diff[selected].clear();}target=p;}
 void loopOptions(bool reverse,float speed){reversing=reverse;loopSpeed=speed;}
 void act(Action a){if(a==Action::GestureClear){gesture.count=0;gesture.available=false;gesturePos=0;}else if(a==Action::Clear){capture.act(Action::CaptureErase);for(auto& d:diff)d.clear();}else capture.act(a);}
 Stereo tick(Stereo input,bool hostBypass=false){input=clean(input);if(target.gestureRecord&&!recording){gesture.count=0;gesture.available=false;gesture.bpm=target.v[BPM];gestureTick=0;recording=true;}else if(!target.gestureRecord&&recording){recording=false;gesture.available=gesture.count>1;gesturePos=0;}
  std::array<float,8> primary{};std::copy_n(target.v.begin(),8,primary.begin());if(recording){if(++gestureTick>=unsigned(sr/100)){gestureTick=0;if(gesture.count<Gesture::capacity)gesture.frames[gesture.count++]=primary;}}
  else if(target.gesturePlay&&gesture.available&&gesture.count>1){int i=int(gesturePos)%gesture.count,j=(i+1)%gesture.count;float f=float(gesturePos-std::floor(gesturePos));for(int k=0;k<8;++k)primary[k]=blend(gesture.frames[i][k],gesture.frames[j][k],f);gesturePos+=100./sr*target.v[BPM]/gesture.bpm;if(gesturePos>=gesture.count)gesturePos-=gesture.count;}
  for(int i=0;i<FloatCount;++i)current.v[i]+=follow*((i<8?primary[i]:target.v[i])-current.v[i]);if((control++&63u)==0)coefficients();
  const bool bypass=target.bypass||hostBypass;const float active=bypass?0.f:1.f;bypassFade+=follow*(active-bypassFade);Stereo x=input;if(target.capturePre)x=capture.tick(x,current.v[CaptureLevel],reversing,loopSpeed);
  for(int id:route){bool enabled=target.enabled[id];if(target.dualBypass&&(target.dualMask&(1<<id)))enabled=false;float goal=enabled?1.f:0.f;if(bypass)goal=0;enables[id]+=follow*(goal-enables[id]);if(id==2){bool feed=!bypass&&enabled;Stereo d=diff[selected].apply(feed?x:Stereo{},current.v[DiffusionAmount],gain[2],feed);Stereo wet=feed?(d-x):d; // bypass preserves only diffusion output, never duplicates input
    if(tail>=0){auto t=diff[tail].tick({},false);wet=wet+t*(tailFade*.4f);if(++tailAge>std::uint64_t(sr*8)){tail=-1;}tailFade=std::max(0.f,tailFade-1/(sr*8));}
    if(feed)x=x+wet*enables[2];else if(target.trails)x=x+wet;
   }else if(enables[id]>.00001f){auto y=id==0?character.tick(x):id==1?movement.tick(x):texture.tick(x);x=blend(x,y*gain[id],enables[id]);}}
  if(!target.capturePre)x=capture.tick(x,current.v[CaptureLevel],reversing,loopSpeed);
  Stereo wet=x;if(!target.trails&&bypass)wet=input;Stereo out=blend(input,wet,current.v[Mix])*outGain;
  if(bypass&&!target.trails)out=blend(input,out,bypassFade);else if(bypass&&target.trails)out=input+(wet-input)*current.v[Mix];
  if(transition<1){transition=std::min(1.f,transition+1/(sr*.012f));out=blend(previous,out,transition);}previous=out;return clean(out);
 }
};
} // namespace prism
