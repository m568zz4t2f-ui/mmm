// SPDX-License-Identifier: AGPL-3.0-or-later
#include "Parameters.h"
#include <fstream>
#include <iostream>
#include <cstdint>
void u16(std::ostream& f,unsigned n){f.put(char(n));f.put(char(n>>8));}void u32(std::ostream& f,unsigned n){u16(f,n&65535);u16(f,n>>16);}
int main(int argc,char** argv){if(argc<2)return 1;const int choice=argc>2?std::clamp(std::atoi(argv[2]),0,tide::numPresets-1):0;
 const auto& x=tide::presets[choice];tide::Parameters p;p.delayType=x.delay;p.reverbType=x.reverb;p.rhythm=x.rhythm;p.stereo=x.stereo;p.feedback=x.feedback;p.delayAmount=x.delayAmount;p.decay=x.decay;p.predelay=x.pre;p.reverbTone=x.tone;p.reverbAmount=x.reverbAmount;p.feed=x.feed;p.reverbDuck=p.delayDuck=x.duck;p.shimmer=x.shimmer;p.delayMotion=p.reverbMotion=x.motion;p.mix=x.mix;p.bpm=100;p.pitchMode=choice==7?1:0;
 if(choice==5){p.sync=false;p.time=105;}if(choice==14)p.reverbOn=false;if(choice==15)p.delayOn=false;
 constexpr int sr=48000,seconds=18,n=sr*seconds;std::ofstream f(argv[1],std::ios::binary);f.write("RIFF",4);u32(f,36+n*4);f.write("WAVEfmt ",8);u32(f,16);u16(f,1);u16(f,2);u32(f,sr);u32(f,sr*4);u16(f,4);u16(f,16);f.write("data",4);u32(f,n*4);
 tide::Engine e;e.prepare(sr,p);float peak=0;const float notes[]={196,246.94165f,293.6648f,369.9944f,440,369.9944f,293.6648f,246.94165f};
 for(int i=0;i<n;++i){const float time=float(i)/sr;float input=0;if(time<8){const float local=std::fmod(time,.6f);const int note=int(time/.6f)%8;const float attack=std::min(1.f,local/.003f);const float decay=std::exp(-local*7);const float a=2*tide::pi*notes[note]*local;input=.19f*attack*decay*(std::sin(a)+.3f*std::sin(2*a)+.12f*std::sin(3*a));}
 auto y=e.tick({input,input*.92f});const float fade=std::min(1.f,float(n-i)/float(sr));y.l*=fade;y.r*=fade;peak=std::max(peak,std::max(std::abs(y.l),std::abs(y.r)));u16(f,unsigned(std::int16_t(std::clamp(y.l,-1.f,1.f)*32767)));u16(f,unsigned(std::int16_t(std::clamp(y.r,-1.f,1.f)*32767)));}
 std::cout<<x.name<<" / synthetic plucks, not a hardware comparison / peak "<<peak<<'\n';return !f;
}
