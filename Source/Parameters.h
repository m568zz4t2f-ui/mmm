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
 {"rRelease","Duck release",50,1500,1,.5f,280,"ms"}
};
constexpr int numFloats=int(sizeof(specs)/sizeof(Spec));
inline constexpr const char* delayNames[]={"Digital","Tape","Bucket","Multitap","Reverse","Pitch"};
inline constexpr const char* reverbNames[]={"Room","Plate","Hall","Bloom"};
inline constexpr const char* rhythmNames[]={"1/16","1/8 triplet","1/8","1/8 dotted","1/4","1/4 dotted","1/2","1/2 dotted","1 bar"};
inline constexpr const char* stereoNames[]={"Stereo","Dual","Ping-pong"};
inline constexpr const char* intervalNames[]={"-12 st","-7 st","+7 st","+12 st","+19 st","+24 st"};
struct Preset {const char* name;int delay,reverb,rhythm,stereo;float feedback,delayAmount,decay,pre,tone,reverbAmount,feed,duck,shimmer,motion,mix;};
inline constexpr Preset presets[]={
 {"Guitar / Rhythmic Sky",0,2,4,1,.34f,.30f,4.2f,32,6500,.23f,.45f,.15f,0,.18f,.38f},
 {"Guitar / Warm Lead",1,1,4,0,.29f,.27f,2.6f,24,5200,.23f,.55f,.12f,0,.25f,.34f},
 {"Guitar / Slow Horizon",1,3,5,1,.48f,.30f,10,65,4700,.38f,.8f,.18f,.20f,.5f,.46f},
 {"Guitar / Reverse Halo",4,3,4,1,.35f,.28f,7,38,6000,.30f,.7f,.1f,.24f,.3f,.43f},
 {"Vocal / Clear Lead",0,1,2,0,.22f,.17f,1.7f,42,6800,.24f,.35f,.65f,0,.08f,.30f},
 {"Vocal / Warm Slap",1,0,2,0,.12f,.24f,.75f,12,5800,.16f,.30f,.3f,0,.10f,.28f},
 {"Vocal / Chorus Lift",0,2,4,2,.30f,.24f,3.1f,55,6200,.26f,.45f,.6f,0,.15f,.36f},
 {"Vocal / Air Throw",5,2,4,2,.28f,.20f,4.8f,60,7400,.28f,.7f,.55f,.12f,.2f,.38f},
 {"Piano / Natural Room",0,0,2,0,.16f,.10f,1.4f,18,7400,.23f,.25f,.25f,0,.05f,.27f},
 {"Piano / Concert Horizon",0,2,4,1,.20f,.13f,3.2f,28,7100,.28f,.35f,.3f,0,.12f,.33f},
 {"Keys / Electric Velvet",2,1,2,1,.32f,.25f,2.5f,18,4900,.25f,.6f,.2f,0,.38f,.35f},
 {"Keys / Crystal Pad",5,3,5,1,.43f,.27f,11,70,5800,.34f,.8f,.12f,.32f,.45f,.48f},
 {"Ambient / Lunar Drift",3,3,5,2,.55f,.32f,14,85,4200,.38f,.85f,.1f,.28f,.6f,.5f},
 {"Ambient / Submerged",2,2,4,2,.52f,.33f,8.5f,45,2800,.32f,.8f,.15f,0,.55f,.45f},
 {"Utility / Delay Only",0,2,4,0,.32f,.36f,4,28,6500,.24f,.65f,0,0,.1f,.35f},
 {"Utility / Reverb Only",0,2,4,0,.32f,.24f,4,28,6500,.36f,.65f,0,0,.2f,.35f}
};
constexpr int numPresets=int(sizeof(presets)/sizeof(Preset));
} // namespace tide
