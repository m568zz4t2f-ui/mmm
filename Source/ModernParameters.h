// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "Parameters.h"
#include "ModernDSP.h"
#include <string_view>
namespace tide::v13 {
struct ModernSpec {tide::Spec spec;const char* models;};
inline constexpr ModernSpec modernSpecs[]={
 {{"dAge","Delay A Age",0,1,0.001f,1,0.25f,"%"},"1,2,9,11"},
 {{"dTexture","Delay A Texture",0,1,0.001f,1,0.5f,"%"},"5,6,7,10"},
 {{"dDensity","Delay A Density",0,1,0.001f,1,0.5f,"%"},"6,7"},
 {{"dPosition","Delay A Position",0,1,0.001f,1,0.2f,"%"},"7"},
 {{"dBits","Delay A Bit depth",4,24,1,1,12,"bits"},"8"},
 {{"dReduction","Delay A Rate reduction",0,1,0.001f,1,0.2f,"%"},"8"},
 {{"dNoise","Delay A Noise",0,1,0.001f,1,0,"%"},"8"},
 {{"dReverseAttack","Delay A Reverse attack",0,1,0.001f,1,0.25f,"%"},"4"},
 {{"dSpread","Delay A Grain harmony",0,1,0.001f,1,0.5f,"%"},"7"},
 {{"dTap1Time","Delay A tap 1 time ratio",0.125f,2,0.001f,1,0.25f,"x"},"3"},
 {{"dTap1Level","Delay A tap 1 level",0,1,0.001f,1,0.25f,"%"},"3,11"},
 {{"dTap1Pan","Delay A tap 1 pan",-1,1,0.001f,1,-0.7f,""},"3,11"},
 {{"dTap1Feedback","Delay A tap 1 feedback weight",0,1,0.001f,1,0.5f,"%"},"3,11"},
 {{"dTap2Time","Delay A tap 2 time ratio",0.125f,2,0.001f,1,0.5f,"x"},"3"},
 {{"dTap2Level","Delay A tap 2 level",0,1,0.001f,1,0.25f,"%"},"3,11"},
 {{"dTap2Pan","Delay A tap 2 pan",-1,1,0.001f,1,0.7f,""},"3,11"},
 {{"dTap2Feedback","Delay A tap 2 feedback weight",0,1,0.001f,1,0.5f,"%"},"3,11"},
 {{"dTap3Time","Delay A tap 3 time ratio",0.125f,2,0.001f,1,0.75f,"x"},"3"},
 {{"dTap3Level","Delay A tap 3 level",0,1,0.001f,1,0.25f,"%"},"3,11"},
 {{"dTap3Pan","Delay A tap 3 pan",-1,1,0.001f,1,-0.3f,""},"3,11"},
 {{"dTap3Feedback","Delay A tap 3 feedback weight",0,1,0.001f,1,0.5f,"%"},"3,11"},
 {{"dTap4Time","Delay A tap 4 time ratio",0.125f,2,0.001f,1,1.0f,"x"},"3"},
 {{"dTap4Level","Delay A tap 4 level",0,1,0.001f,1,0.25f,"%"},"3,11"},
 {{"dTap4Pan","Delay A tap 4 pan",-1,1,0.001f,1,0.3f,""},"3,11"},
 {{"dTap4Feedback","Delay A tap 4 feedback weight",0,1,0.001f,1,0.5f,"%"},"3,11"},
 {{"d2Time","Delay B Time",20,8000,0.1f,0.45f,500,"ms"},""},
 {{"d2Feedback","Delay B Feedback",0,0.88f,0.001f,1,0.27f,"%"},""},
 {{"d2Tone","Delay B Tone",800,16000,1,0.45f,6500,"Hz"},""},
 {{"d2Amount","Delay B Amount",0,0.8f,0.001f,1,0.24f,"%"},""},
 {{"d2Motion","Delay B Motion",0,1,0.001f,1,0.15f,"%"},""},
 {{"d2Width","Delay B Width",0,1.5f,0.001f,1,1,"%"},""},
 {{"d2Drive","Delay B Drive",0,1,0.001f,1,0.18f,"%"},""},
 {{"d2Lowcut","Delay B Low cut",20,1000,1,0.5f,150,"Hz"},""},
 {{"d2Rate","Delay B Motion rate",0.05f,5,0.01f,0.5f,0.4f,"Hz"},""},
 {{"d2Age","Delay B Age",0,1,0.001f,1,0.25f,"%"},"1,2,9,11"},
 {{"d2Texture","Delay B Texture",0,1,0.001f,1,0.5f,"%"},"5,6,7,10"},
 {{"d2Density","Delay B Density",0,1,0.001f,1,0.5f,"%"},"6,7"},
 {{"d2Position","Delay B Position",0,1,0.001f,1,0.2f,"%"},"7"},
 {{"d2Bits","Delay B Bit depth",4,24,1,1,12,"bits"},"8"},
 {{"d2Reduction","Delay B Rate reduction",0,1,0.001f,1,0.2f,"%"},"8"},
 {{"d2Noise","Delay B Noise",0,1,0.001f,1,0,"%"},"8"},
 {{"d2ReverseAttack","Delay B Reverse attack",0,1,0.001f,1,0.25f,"%"},"4"},
 {{"d2Spread","Delay B Grain harmony",0,1,0.001f,1,0.5f,"%"},"7"},
 {{"d2Tap1Time","Delay B tap 1 time ratio",0.125f,2,0.001f,1,0.25f,"x"},"3"},
 {{"d2Tap1Level","Delay B tap 1 level",0,1,0.001f,1,0.25f,"%"},"3,11"},
 {{"d2Tap1Pan","Delay B tap 1 pan",-1,1,0.001f,1,-0.7f,""},"3,11"},
 {{"d2Tap1Feedback","Delay B tap 1 feedback weight",0,1,0.001f,1,0.5f,"%"},"3,11"},
 {{"d2Tap2Time","Delay B tap 2 time ratio",0.125f,2,0.001f,1,0.5f,"x"},"3"},
 {{"d2Tap2Level","Delay B tap 2 level",0,1,0.001f,1,0.25f,"%"},"3,11"},
 {{"d2Tap2Pan","Delay B tap 2 pan",-1,1,0.001f,1,0.7f,""},"3,11"},
 {{"d2Tap2Feedback","Delay B tap 2 feedback weight",0,1,0.001f,1,0.5f,"%"},"3,11"},
 {{"d2Tap3Time","Delay B tap 3 time ratio",0.125f,2,0.001f,1,0.75f,"x"},"3"},
 {{"d2Tap3Level","Delay B tap 3 level",0,1,0.001f,1,0.25f,"%"},"3,11"},
 {{"d2Tap3Pan","Delay B tap 3 pan",-1,1,0.001f,1,-0.3f,""},"3,11"},
 {{"d2Tap3Feedback","Delay B tap 3 feedback weight",0,1,0.001f,1,0.5f,"%"},"3,11"},
 {{"d2Tap4Time","Delay B tap 4 time ratio",0.125f,2,0.001f,1,1.0f,"x"},"3"},
 {{"d2Tap4Level","Delay B tap 4 level",0,1,0.001f,1,0.25f,"%"},"3,11"},
 {{"d2Tap4Pan","Delay B tap 4 pan",-1,1,0.001f,1,0.3f,""},"3,11"},
 {{"d2Tap4Feedback","Delay B tap 4 feedback weight",0,1,0.001f,1,0.5f,"%"},"3,11"},
 {{"rEarly","Reverb A Early reflections",0,1,0.001f,1,0.35f,"%"},"0,2,4,9"},
 {{"rDamping","Reverb A Bass damping",0,1,0.001f,1,0.5f,"%"},"2,3,5,7,9,10,11"},
 {{"rTexture","Reverb A Pitch texture",0,1,0.001f,1,0.5f,"%"},"6,7"},
 {{"rTune","Reverb A Resonator tuning",30,1000,0.1f,0.5f,110,"Hz"},"12"},
 {{"rShape","Reverb A Gate / swell shape",0,1,0.001f,1,0.5f,"%"},"11"},
 {{"r2Decay","Reverb B Decay",0.25f,20,0.01f,0.45f,3,"s"},""},
 {{"r2Predelay","Reverb B Pre-delay",0,200,0.1f,1,18,"ms"},""},
 {{"r2Tone","Reverb B Tone",1000,16000,1,0.45f,6500,"Hz"},""},
 {{"r2Amount","Reverb B Amount",0,0.8f,0.001f,1,0.24f,"%"},""},
 {{"r2Motion","Reverb B Motion",0,1,0.001f,1,0.18f,"%"},""},
 {{"r2Width","Reverb B Width",0,1.5f,0.001f,1,1,"%"},""},
 {{"r2Lowcut","Reverb B Low cut",20,1000,1,0.5f,180,"Hz"},""},
 {{"r2Diffusion","Reverb B Diffusion",0,1,0.001f,1,0.7f,"%"},""},
 {{"r2Size","Reverb B Size",0.5f,1.7f,0.001f,1,1,"x"},""},
 {{"r2Attack","Reverb B Attack / gate",0.01f,2,0.001f,0.5f,0.18f,"s"},""},
 {{"r2Regeneration","Reverb B Pitch regeneration",0,0.6f,0.001f,1,0.3f,"%"},""},
 {{"r2Shimmer","Reverb B Shimmer",0,1,0.001f,1,0,"%"},""},
 {{"r2Bass","Reverb B Bass weight",0,1,0.001f,1,0.8f,"%"},""},
 {{"r2Early","Reverb B Early reflections",0,1,0.001f,1,0.35f,"%"},"0,2,4,9"},
 {{"r2Damping","Reverb B Bass damping",0,1,0.001f,1,0.5f,"%"},"2,3,5,7,9,10,11"},
 {{"r2Texture","Reverb B Pitch texture",0,1,0.001f,1,0.5f,"%"},"6,7"},
 {{"r2Tune","Reverb B Resonator tuning",30,1000,0.1f,0.5f,110,"Hz"},"12"},
 {{"r2Shape","Reverb B Gate / swell shape",0,1,0.001f,1,0.5f,"%"},"11"},
 {{"d2Ratio","Delay B stereo time ratio",0.25f,2,0.001f,1,0.75f,"x"},""},
 {{"d2Diffusion","Delay B diffusion",0,1,0.001f,1,0.7f,"%"},"6"},
 {{"splitHz","Split frequency",30,12000,1,0.4f,500,"Hz"},""},
 {{"crossFeedback","Cross feedback",0,0.3f,0.001f,1,0,"%"},""},
};
inline constexpr int numModernFloats=int(sizeof(modernSpecs)/sizeof(ModernSpec));
struct ChoiceSpec {const char* id;const char* name;const char* items;int initial;};
inline constexpr ChoiceSpec modernChoices[]={
 {"dRevision","Delay processing","Legacy|Tide 1.3",0},{"rRevision","Reverb processing","Legacy|Tide 1.3",0},
 {"d13Model","Delay A machine","Digital|Tape|Bucket|Multitap|Reverse|Pitch|Diffusion|Granular Pitch|Lo-Fi|Oil Can|Spectral|Magnetic Drum",0},
 {"r13Model","Reverb A machine","Room|Plate|Hall|Bloom|Chamber|Cloud Ensemble|Halo Shimmer|Glint|Studio Plate|Studio Hall|Studio Bloom|Nonlinear|Resonator",2},
 {"d2Model","Delay B machine","Digital|Tape|Bucket|Multitap|Reverse|Pitch|Diffusion|Granular Pitch|Lo-Fi|Oil Can|Spectral|Magnetic Drum",1},
 {"r2Model","Reverb B machine","Room|Plate|Hall|Bloom|Chamber|Cloud Ensemble|Halo Shimmer|Glint|Studio Plate|Studio Hall|Studio Bloom|Nonlinear|Resonator",1},
 {"routing13","Four-slot routing","Series|Parallel|Frequency split",0},
 {"d2Stereo","Delay B stereo","Stereo|Dual|Ping-pong",1},{"d2Interval","Delay B interval","-12 st|-7 st|+7 st|+12 st|+19 st|+24 st",3},
 {"r2Interval","Reverb B interval","-12 st|-7 st|+7 st|+12 st|+19 st|+24 st",3},
 {"d2Rhythm","Delay B rhythm","1/16|1/8 triplet|1/8|1/8 dotted|1/4|1/4 dotted|1/2|1/2 dotted|1 bar",3},
 {"d2PitchMode","Delay B pitch routing","Climbing repeats|Fixed harmony",1}
};
inline constexpr const char* modernFlags[]={"d2On","r2On","d2Sync","dFreeze","d2Freeze","r2Freeze"};
inline constexpr const char* modernFlagNames[]={"Delay B active","Reverb B active","Delay B tempo sync","Delay A grain freeze","Delay B grain freeze","Reverb B hold"};
inline bool isModernId(std::string_view id){for(const auto& x:modernSpecs)if(id==x.spec.id)return true;for(const auto& x:modernChoices)if(id==x.id)return true;for(auto* x:modernFlags)if(id==x)return true;return false;}
inline int modelFor(int oldType,int oldExtension,bool delay){return oldExtension?oldExtension+(delay?5:3):oldType;}
inline bool applicable(std::string_view id,int model,bool delay){
 for(const auto& row:modernSpecs)if(id==row.spec.id){if(!*row.models)break;const char* p=row.models;while(*p){int n=0;while(*p>='0'&&*p<='9')n=n*10+(*p++-'0');if(n==model)return true;if(*p)++p;}return false;}
 if(delay){if(id=="drive"||id=="d2Drive")return model==1||model==2||model==9||model==11;if(id=="dMotion"||id=="dRate"||id=="d2Motion"||id=="d2Rate")return model!=4;if(id=="interval"||id=="d2Interval")return model==5||model==7;}
 else{if(id=="shimmer"||id=="rRegeneration"||id=="r2Shimmer"||id=="r2Regeneration"||id=="rInterval"||id=="r2Interval")return model==6||model==7;if(id=="rAttack"||id=="r2Attack")return model==3||model==10||model==11;if(id=="rMotion"||id=="r2Motion")return model!=0&&model!=4&&model!=12;if(id=="rDiffusion"||id=="r2Diffusion")return model!=12;}
 return true;
}
// The getter reads cached atomic parameter pointers: no String construction in processing.
template<class Getter> Settings settingsFrom(const tide::Parameters& old,Getter f){
 Settings p;p.legacyDelay=f("dRevision")<.5f;p.legacyReverb=f("rRevision")<.5f;
 p.routing=int(f("routing13"));p.splitFrequency=f("splitHz");p.crossFeedback=f("crossFeedback");p.feed=old.feed;p.direct=old.direct;p.mix=old.mix;p.output=old.output;p.send=old.send;
 p.delayDuck=old.delayDuck;p.reverbDuck=old.reverbDuck;p.delaySense=old.delaySense;p.reverbSense=old.reverbSense;p.delayRelease=old.delayRelease;p.reverbRelease=old.reverbRelease;
 auto& a=p.delays[0];a.model=int(f("d13Model"));a.on=old.delayOn;a.time=old.sync?60000/std::clamp(old.bpm,20.f,400.f)*tide::beats[std::clamp(old.rhythm,0,8)]:old.time;a.stereo=old.stereo;
 a.rightTime=old.stereo==1?(old.dualMode?(old.sync?60000/std::clamp(old.bpm,20.f,400.f)*tide::beats[std::clamp(old.bRhythm,0,8)]:old.bTime):a.time*old.ratio):a.time;
 a.feedback=std::min(.88f,old.feedback*(1+old.bloom*.25f));a.rightFeedback=old.dualMode&&old.stereo==1?old.bFeedback:a.feedback;a.rightLevel=old.dualMode&&old.stereo==1?old.bLevel:1;a.tone=old.delayTone;a.lowcut=old.delayLowcut;a.motion=old.delayMotion;a.rate=old.delayRate;a.width=old.delayWidth;a.drive=old.drive;a.interval=tide::semitones[std::clamp(old.interval,0,5)];a.pitchMode=old.pitchMode;a.diffusion=old.delayDiffusion;
 auto& b=p.delays[1];b.model=int(f("d2Model"));b.on=f("d2On")>.5f&&!p.legacyDelay;b.time=f("d2Sync")>.5f?60000/std::clamp(old.bpm,20.f,400.f)*tide::beats[std::clamp(int(f("d2Rhythm")),0,8)]:f("d2Time");b.stereo=int(f("d2Stereo"));b.rightTime=b.stereo==1?b.time*f("d2Ratio"):b.time;b.feedback=b.rightFeedback=f("d2Feedback");b.tone=f("d2Tone");b.lowcut=f("d2Lowcut");b.motion=f("d2Motion");b.rate=f("d2Rate");b.width=f("d2Width");b.drive=f("d2Drive");b.interval=tide::semitones[std::clamp(int(f("d2Interval")),0,5)];b.pitchMode=int(f("d2PitchMode"));b.diffusion=f("d2Diffusion");
 a.age=f("dAge");
 a.texture=f("dTexture");
 a.density=f("dDensity");
 a.position=f("dPosition");
 a.bits=f("dBits");
 a.reduction=f("dReduction");
 a.noise=f("dNoise");
 a.attack=f("dReverseAttack");
 a.spread=f("dSpread");
 a.freeze=f("dFreeze")>.5f;
 a.taps[0].time=f("dTap1Time");
 a.taps[0].level=f("dTap1Level");
 a.taps[0].pan=f("dTap1Pan");
 a.taps[0].feedback=f("dTap1Feedback");
 a.taps[1].time=f("dTap2Time");
 a.taps[1].level=f("dTap2Level");
 a.taps[1].pan=f("dTap2Pan");
 a.taps[1].feedback=f("dTap2Feedback");
 a.taps[2].time=f("dTap3Time");
 a.taps[2].level=f("dTap3Level");
 a.taps[2].pan=f("dTap3Pan");
 a.taps[2].feedback=f("dTap3Feedback");
 a.taps[3].time=f("dTap4Time");
 a.taps[3].level=f("dTap4Level");
 a.taps[3].pan=f("dTap4Pan");
 a.taps[3].feedback=f("dTap4Feedback");
 b.age=f("d2Age");
 b.texture=f("d2Texture");
 b.density=f("d2Density");
 b.position=f("d2Position");
 b.bits=f("d2Bits");
 b.reduction=f("d2Reduction");
 b.noise=f("d2Noise");
 b.attack=f("d2ReverseAttack");
 b.spread=f("d2Spread");
 b.freeze=f("d2Freeze")>.5f;
 b.taps[0].time=f("d2Tap1Time");
 b.taps[0].level=f("d2Tap1Level");
 b.taps[0].pan=f("d2Tap1Pan");
 b.taps[0].feedback=f("d2Tap1Feedback");
 b.taps[1].time=f("d2Tap2Time");
 b.taps[1].level=f("d2Tap2Level");
 b.taps[1].pan=f("d2Tap2Pan");
 b.taps[1].feedback=f("d2Tap2Feedback");
 b.taps[2].time=f("d2Tap3Time");
 b.taps[2].level=f("d2Tap3Level");
 b.taps[2].pan=f("d2Tap3Pan");
 b.taps[2].feedback=f("d2Tap3Feedback");
 b.taps[3].time=f("d2Tap4Time");
 b.taps[3].level=f("d2Tap4Level");
 b.taps[3].pan=f("d2Tap4Pan");
 b.taps[3].feedback=f("d2Tap4Feedback");
 auto& r=p.reverbs[0];r.model=int(f("r13Model"));r.on=old.reverbOn;r.freeze=old.freeze;r.decay=std::min(20.f,old.decay*(1+old.bloom*.6f));r.predelay=old.predelay;r.tone=old.reverbTone;r.lowcut=old.reverbLowcut;r.motion=old.reverbMotion;r.width=old.reverbWidth;r.size=old.size;r.diffusion=old.diffusion;r.attack=old.attack;r.regeneration=old.regeneration;r.shimmer=old.shimmer;r.interval=tide::semitones[std::clamp(old.reverbInterval,0,5)];r.bass=old.bassDecay;
 auto& t=p.reverbs[1];t.model=int(f("r2Model"));t.on=f("r2On")>.5f&&!p.legacyReverb;t.freeze=f("r2Freeze")>.5f;
 t.decay=f("r2Decay");
 t.predelay=f("r2Predelay");
 t.tone=f("r2Tone");
 t.lowcut=f("r2Lowcut");
 t.motion=f("r2Motion");
 t.width=f("r2Width");
 t.size=f("r2Size");
 t.diffusion=f("r2Diffusion");
 t.attack=f("r2Attack");
 t.regeneration=f("r2Regeneration");
 t.shimmer=f("r2Shimmer");
 t.bass=f("r2Bass");
 t.interval=tide::semitones[std::clamp(int(f("r2Interval")),0,5)];
 r.early=f("rEarly");
 r.damping=f("rDamping");
 r.texture=f("rTexture");
 r.tune=f("rTune");
 r.shape=f("rShape");
 t.early=f("r2Early");
 t.damping=f("r2Damping");
 t.texture=f("r2Texture");
 t.tune=f("r2Tune");
 t.shape=f("r2Shape");
 p.delayLevel={old.delayAmount,f("d2Amount")};p.reverbLevel={old.reverbAmount,f("r2Amount")};return p;
}
} // namespace tide::v13
