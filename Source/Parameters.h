// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "TideDSP.h"
namespace tide {
struct Spec {const char* id;const char* name;float lo,hi,step,skew,initial;const char* unit;};
inline constexpr Spec specs[]={
 {"dTime","Time",20,8000,.1f,.45f,375,"ms"},
 {"dFeedback","Feedback",0,.88f,.001f,1,.32f,"%"},
 {"dTone","Tone",800,16000,1,.45f,6500,"Hz"},
 {"dAmount","Amount",0,.8f,.001f,1,.24f,"%"},
 {"dMotion","Motion",0,1,.001f,1,.15f,"%"},
 {"dWidth","Width",0,1.5f,.001f,1,1,"%"},
 {"drive","Drive",0,1,.001f,1,.18f,"%"},
 {"dLowcut","Low cut",20,1000,1,.5f,150,"Hz"},
 {"dDuck","Duck",0,1,.001f,1,.15f,"%"},
 {"dRate","Motion rate",.05f,5,.01f,.5f,.4f,"Hz"},
 {"ratio","B / A ratio",.25f,2,.001f,1,.75f,"x"},
 {"rDecay","Decay",.25f,20,.01f,.45f,4,"s"},
 {"rPredelay","Pre-delay",0,200,.1f,1,28,"ms"},
 {"rTone","Tone",1000,16000,1,.45f,6500,"Hz"},
 {"rAmount","Amount",0,.8f,.001f,1,.24f,"%"},
 {"rMotion","Motion",0,1,.001f,1,.22f,"%"},
 {"rWidth","Width",0,1.5f,.001f,1,1,"%"},
 {"rLowcut","Low cut",20,1000,1,.5f,180,"Hz"},
 {"rDuck","Duck",0,1,.001f,1,.15f,"%"},
 {"shimmer","Shimmer",0,1,.001f,1,0,"%"},
 {"bass","Bass weight",0,1,.001f,1,.8f,"%"},
 {"feed","Delay to reverb",0,1,.001f,1,.65f,"%"},
 {"direct","Direct to reverb",0,1,.001f,1,1,"%"},
 {"mix","Mix",0,1,.001f,1,.38f,"%"},
 {"output","Output",-24,6,.1f,1,0,"dB"},
 {"bloom","Bloom",0,1,.001f,1,0,"%"},
 {"bpm","Fallback tempo",30,240,.1f,1,120,"BPM"},
 {"dSense","Duck sensitivity",.25f,4,.01f,.7f,1,"x"},
 {"dRelease","Duck release",50,1500,1,.5f,280,"ms"},
 {"rSense","Duck sensitivity",.25f,4,.01f,.7f,1,"x"},
 {"rRelease","Duck release",50,1500,1,.5f,280,"ms"},
 {"dBTime","B time",20,8000,.1f,.45f,500,"ms"},
 {"dBFeedback","B repeats",0,.88f,.001f,1,.32f,"%"},
 {"dBLevel","B level",0,1,.001f,1,1,"%"},
 {"dSwell","Swell",0,1,.001f,1,0,"%"},
 {"rDiffusion","Diffusion",0,1,.001f,1,.7f,"%"},
 {"rSize","Size",.5f,1.7f,.001f,1,1,"x"},
 {"rAttack","Bloom attack",.01f,2,.001f,.5f,.18f,"s"},
 {"rRegeneration","Pitch regeneration",0,.7f,.001f,1,.3f,"%"},
 {"dDiffusion","Delay diffusion",0,1,.001f,1,.7f,"%"}
};
constexpr int numFloats=int(sizeof(specs)/sizeof(Spec));
inline constexpr const char* delayNames[]={"Digital","Tape","Bucket","Multitap","Reverse","Pitch"};
inline constexpr const char* newDelayNames[]={"Diffusion","Granular Pitch","Lo-Fi"};
inline constexpr const char* newReverbNames[]={"Chamber","Cloud Ensemble","Halo Shimmer","Glint","Studio Plate","Studio Hall","Studio Bloom"};
inline constexpr const char* reverbNames[]={"Room","Plate","Hall","Bloom"};
inline constexpr const char* rhythmNames[]={"1/16","1/8 triplet","1/8","1/8 dotted","1/4","1/4 dotted","1/2","1/2 dotted","1 bar"};
inline constexpr const char* stereoNames[]={"Stereo","Dual","Ping-pong"};
inline constexpr const char* intervalNames[]={"-12 st","-7 st","+7 st","+12 st","+19 st","+24 st"};
struct Preset {const char* name;int delay,reverb,rhythm,stereo;float feedback,delayAmount,decay,pre,tone,reverbAmount,feed,duck,shimmer,motion,mix;
 int pitchMode=0;bool sync=true,delayOn=true,reverbOn=true;float time=375;int dModel=0,rModel=0,version=0;float drive=.18f,delayTone=6500;int pattern=1;int modernDelay=-1,modernReverb=-1,delayB=-1,reverbB=-1,route13=0;};
inline constexpr Preset originalPresets[]={
 {"Guitar / Rhythmic Sky",0,2,4,1,.34f,.30f,4.2f,32,6500,.23f,.45f,.15f,0,.18f,.38f},
 {"Guitar / Warm Lead",1,1,4,0,.29f,.27f,2.6f,24,5200,.23f,.55f,.12f,0,.25f,.34f},
 {"Guitar / Slow Horizon",1,3,5,1,.48f,.30f,10,65,4700,.38f,.8f,.18f,.20f,.5f,.46f},
 {"Guitar / Reverse Halo",4,3,4,1,.35f,.28f,7,38,6000,.30f,.7f,.1f,.24f,.3f,.43f},
 {"Vocal / Clear Lead",0,1,2,0,.22f,.17f,1.7f,42,6800,.24f,.35f,.65f,0,.08f,.30f},
 {"Vocal / Warm Slap",1,0,2,0,.12f,.24f,.75f,12,5800,.16f,.30f,.3f,0,.10f,.28f,0,false,true,true,105},
 {"Vocal / Chorus Lift",0,2,4,2,.30f,.24f,3.1f,55,6200,.26f,.45f,.6f,0,.15f,.36f},
 {"Vocal / Air Throw",5,2,4,2,.28f,.20f,4.8f,60,7400,.28f,.7f,.55f,.12f,.2f,.38f,1},
 {"Piano / Natural Room",0,0,2,0,.16f,.10f,1.4f,18,7400,.23f,.25f,.25f,0,.05f,.27f},
 {"Piano / Concert Horizon",0,2,4,1,.20f,.13f,3.2f,28,7100,.28f,.35f,.3f,0,.12f,.33f},
 {"Keys / Electric Velvet",2,1,2,1,.32f,.25f,2.5f,18,4900,.25f,.6f,.2f,0,.38f,.35f},
 {"Keys / Crystal Pad",5,3,5,1,.43f,.27f,11,70,5800,.34f,.8f,.12f,.32f,.45f,.48f},
 {"Ambient / Lunar Drift",3,3,5,2,.55f,.32f,14,85,4200,.38f,.85f,.1f,.28f,.6f,.5f},
 {"Ambient / Submerged",2,2,4,2,.52f,.33f,8.5f,45,2800,.32f,.8f,.15f,0,.55f,.45f},
 {"Utility / Delay Only",0,2,4,0,.32f,.36f,4,28,6500,.24f,.65f,0,0,.1f,.35f,0,true,true,false},
 {"Utility / Reverb Only",0,2,4,0,.32f,.24f,4,28,6500,.36f,.65f,0,0,.2f,.35f,0,true,false,true},
 {"Guitar / Digital Quarter",0,2,4,0,0.32f,0.24f,8.f,28,6500,0.23f,.55f,0.f,0.f,0.f,.36f,1,true,true,true,375, 0,3,1,0.f,11000,1},
 {"Guitar / Dotted Eighth",0,2,3,2,0.38f,0.24f,8.f,28,6500,0.23f,.55f,0.f,0.f,0.f,.36f,1,true,true,true,375, 0,3,1,0.f,11000,1},
 {"Guitar / Warm Tape",1,2,4,0,0.3f,0.24f,2.6f,28,6500,0.23f,.55f,0.2f,0.f,0.18f,.36f,1,true,true,true,375, 0,5,1,0.2f,5000,1},
 {"Guitar / Analog Lead",2,2,4,0,0.34f,0.24f,2.6f,28,6500,0.23f,.55f,0.18f,0.f,0.25f,.36f,1,true,true,true,375, 0,5,1,0.2f,3800,1},
 {"Guitar / Stereo Dual",0,2,4,1,0.32f,0.24f,4.2f,28,6500,0.23f,.55f,0.f,0.f,0.1f,.36f,1,true,true,true,375, 0,6,1,0.f,11000,1},
 {"Guitar / Plate Solo",1,2,4,0,0.28f,0.24f,2.6f,28,6500,0.23f,.55f,0.15f,0.f,0.16f,.36f,1,true,true,true,375, 0,5,1,0.2f,5000,1},
 {"Guitar / Cloud Horizon",1,2,5,1,0.42f,0.24f,8.f,28,6500,0.23f,.55f,0.12f,0.f,0.25f,.36f,1,true,true,true,375, 0,2,1,0.2f,5000,1},
 {"Guitar / Halo Lift",0,2,4,1,0.32f,0.24f,8.f,28,6500,0.23f,.55f,0.15f,0.35f,0.18f,.36f,1,true,true,true,375, 0,3,1,0.f,11000,1},
 {"Vocal / Studio Slap",0,2,2,0,0.08f,0.24f,1.1f,28,6500,0.23f,.55f,0.4f,0.f,0.f,.36f,1,false,true,true,95, 0,1,1,0.f,11000,1},
 {"Vocal / Ducked Quarter",0,2,4,0,0.27f,0.24f,2.6f,28,6500,0.23f,.55f,0.65f,0.f,0.f,.36f,1,true,true,true,375, 0,5,1,0.f,11000,1},
 {"Vocal / Tape Double",1,2,2,0,0.13f,0.24f,1.1f,28,6500,0.23f,.55f,0.25f,0.f,0.08f,.36f,1,false,true,true,120, 0,1,1,0.2f,5000,1},
 {"Vocal / Dotted Throw",0,2,3,2,0.38f,0.24f,4.2f,28,6500,0.23f,.55f,0.6f,0.f,0.f,.36f,1,true,true,true,375, 0,6,1,0.f,11000,1},
 {"Vocal / Clean Plate",0,2,2,0,0.12f,0.24f,2.6f,28,6500,0.23f,.55f,0.55f,0.f,0.f,.36f,1,true,true,true,375, 0,5,1,0.f,11000,1},
 {"Vocal / Chamber Air",0,2,2,0,0.12f,0.24f,1.1f,28,6500,0.23f,.55f,0.4f,0.f,0.05f,.36f,1,true,true,true,375, 0,1,1,0.f,11000,1},
 {"Vocal / Glint Lift",0,2,4,0,0.22f,0.24f,8.f,28,6500,0.23f,.55f,0.6f,0.18f,0.14f,.36f,1,true,true,true,375, 0,4,1,0.f,11000,1},
 {"Vocal / Fixed Air",5,2,4,2,0.25f,0.24f,4.2f,28,6500,0.23f,.55f,0.55f,0.f,0.1f,.36f,1,true,true,true,375, 0,6,1,0.f,11000,1},
 {"Keys / Natural Chamber",0,2,2,0,0.12f,0.24f,1.1f,28,6500,0.23f,.55f,0.15f,0.f,0.f,.36f,1,true,true,true,375, 0,1,1,0.f,11000,1},
 {"Keys / Concert Hall",0,2,4,0,0.18f,0.24f,4.2f,28,6500,0.23f,.55f,0.2f,0.f,0.12f,.36f,1,true,true,true,375, 0,6,1,0.f,11000,1},
 {"Keys / Electric Tape",1,2,3,1,0.32f,0.24f,2.6f,28,6500,0.23f,.55f,0.15f,0.f,0.25f,.36f,1,true,true,true,375, 0,5,1,0.2f,5000,1},
 {"Keys / Analog Velvet",2,2,4,1,0.32f,0.24f,2.6f,28,6500,0.23f,.55f,0.15f,0.f,0.3f,.36f,1,true,true,true,375, 0,5,1,0.2f,3800,1},
 {"Keys / Stereo Pattern",3,2,4,2,0.32f,0.24f,4.2f,28,6500,0.23f,.55f,0.2f,0.f,0.12f,.36f,1,true,true,true,375, 0,6,1,0.f,11000,2},
 {"Keys / Cloud Ensemble",0,2,4,1,0.25f,0.24f,8.f,28,6500,0.23f,.55f,0.15f,0.f,0.35f,.36f,1,true,true,true,375, 0,2,1,0.f,11000,1},
 {"Keys / Halo Pad",5,2,5,1,0.3f,0.24f,8.f,28,6500,0.23f,.55f,0.12f,0.4f,0.25f,.36f,1,true,true,true,375, 0,3,1,0.f,11000,1},
 {"Keys / Glint Piano",0,2,4,0,0.18f,0.24f,8.f,28,6500,0.23f,.55f,0.3f,0.2f,0.18f,.36f,1,true,true,true,375, 0,4,1,0.f,11000,1},
 {"Ambient / Cloud Coast",1,2,5,1,0.5f,0.29f,8.f,28,6500,0.3f,.55f,0.12f,0.f,0.45f,.36f,1,true,true,true,375, 0,2,1,0.2f,5000,1},
 {"Ambient / Halo Ascent",5,2,5,1,0.38f,0.29f,8.f,28,6500,0.3f,.55f,0.1f,0.45f,0.35f,.36f,1,true,true,true,375, 0,3,1,0.f,11000,1},
 {"Ambient / Glint Bloom",0,2,5,1,0.45f,0.29f,8.f,28,6500,0.3f,.55f,0.1f,0.3f,0.35f,.36f,1,true,true,true,375, 0,4,1,0.f,11000,1},
 {"Ambient / Slow Bloom",3,2,5,2,0.48f,0.29f,8.f,28,6500,0.3f,.55f,0.15f,0.f,0.4f,.36f,1,true,true,true,375, 0,7,1,0.f,11000,1},
 {"Ambient / Diffused Echo",0,2,5,1,0.48f,0.29f,8.f,28,6500,0.3f,.55f,0.15f,0.f,0.25f,.36f,1,true,true,true,375, 1,2,1,0.f,11000,1},
 {"Ambient / Grain Lantern",0,2,4,1,0.35f,0.29f,8.f,28,6500,0.3f,.55f,0.15f,0.2f,0.15f,.36f,1,true,true,true,375, 2,4,1,0.f,11000,1},
 {"Ambient / Lo-Fi Shore",0,2,4,2,0.4f,0.29f,2.6f,28,6500,0.3f,.55f,0.15f,0.f,0.1f,.36f,1,true,true,true,375, 3,5,1,0.f,11000,1},
 {"Ambient / Reverse Tide",4,2,5,1,0.35f,0.29f,8.f,28,6500,0.3f,.55f,0.12f,0.f,0.3f,.36f,1,true,true,true,375, 0,2,1,0.f,11000,1},
 {"Utility / Modern Delay Only",0,2,4,0,0.32f,0.36f,4.2f,28,6500,0.3f,.55f,0.f,0.f,0.f,.36f,1,true,true,false,375, 0,6,1,0.f,11000,1},
 {"Utility / Modern Reverb Only",0,2,4,0,0.2f,0.29f,4.2f,28,6500,0.36f,.55f,0.f,0.f,0.2f,.36f,1,true,false,true,375, 0,6,1,0.f,11000,1},
 {"Utility / Vocal Slap Only",0,2,2,0,0.08f,0.36f,1.1f,28,6500,0.3f,.55f,0.25f,0.f,0.f,.36f,1,false,true,false,95, 0,1,1,0.f,11000,1},
 {"Utility / Cloud Only",0,2,4,1,0.2f,0.29f,8.f,28,6500,0.36f,.55f,0.1f,0.f,0.4f,.36f,1,true,false,true,375, 0,2,1,0.f,11000,1}
};
constexpr int numPresets=64;
inline const std::array<Preset,numPresets> presets=[] {
 std::array<Preset,numPresets> result{};std::copy(std::begin(originalPresets),std::end(originalPresets),result.begin());
 for(int i=16;i<52;++i){auto& p=result[std::size_t(i)];p.modernDelay=p.dModel?p.dModel+5:p.delay;p.modernReverb=p.rModel?p.rModel+3:p.reverb;
  p.delayAmount=p.delayOn?.28f:0;p.reverbAmount=p.reverbOn?.28f:0;p.decay=std::min(p.decay,p.modernReverb==0?2.5f:p.modernReverb==1||p.modernReverb==8?3.8f:12.f);p.drive=std::min(p.drive,.16f);p.mix=std::min(p.mix,.42f);}
 constexpr const char* names[]={"Guitar / Oil Can Current","Keys / Spectral Steps","Guitar / Magnetic Drum","Vocal / Gated Chamber","Keys / Tuned Resonance","Ambient / Four Tides","Guitar / Parallel Horizons","Ambient / Frequency Coast","Keys / Double Plate","Ambient / Grains and Halo","Vocal / Split Air","Utility / Four Slot Template"};
 constexpr int d[]={9,10,11,0,0,1,0,2,0,7,0,0},r[]={4,9,8,11,12,5,2,10,8,6,7,2};
 for(int n=0;n<12;++n){auto p=result[std::size_t(16+n%12)];p.name=names[n];p.modernDelay=d[n];p.modernReverb=r[n];p.delayAmount=.25f;p.reverbAmount=.25f;p.feedback=.28f;p.decay=n==3?1.2f:n==4?3.f:4.5f;p.shimmer=n==9?.3f:0;p.pitchMode=1;p.motion=.18f;p.mix=.36f;
  if(n>=5){p.delayB=n==8?-1:n==9?5:n%12;p.reverbB=n==6?-1:n==8?1:n%13;p.route13=n==7||n==10?2:1;}result[std::size_t(52+n)]=p;}
 return result;
}();
} // namespace tide
