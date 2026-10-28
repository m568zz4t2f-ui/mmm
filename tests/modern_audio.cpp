// SPDX-License-Identifier: AGPL-3.0-or-later
#include <juce_audio_formats/juce_audio_formats.h>
#include "ModernDSP.h"
#include <iostream>
int main(int argc,char** argv){
 if(argc<2){std::cerr<<"Supply reference audio paths. Input audio is analysed, never redistributed.\n";return 2;}
 juce::AudioFormatManager formats;formats.registerBasicFormats();
 std::cout<<"fixture,rate,family,model,input_energy,wet_energy,wet_peak,stereo_correlation\n";
 for(int file=1;file<argc;++file){const auto path=juce::File(argv[file]);std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(path));if(!reader){std::cerr<<"Cannot decode "<<path.getFileName()<<'\n';return 1;}
  const float sr=float(reader->sampleRate);if(!std::isfinite(sr)||sr<8000||sr>192000||reader->lengthInSamples<=0||reader->numChannels==0){std::cerr<<"Unsupported fixture rate\n";return 1;}
  const int frames=int(std::min(reader->lengthInSamples,juce::int64(sr*4)));juce::AudioBuffer<float> input(2,frames);input.clear();if(!reader->read(&input,0,frames,0,true,true))return 1;
  if(reader->numChannels==1)input.copyFrom(1,0,input,0,0,frames);
  for(int family=0;family<2;++family)for(int model=0;model<(family?13:12);++model){tide::v13::DelayMachine delay;tide::v13::ReverbMachine reverb;
   if(family){reverb.prepare(sr);tide::v13::ReverbSettings p;p.model=model;p.decay=2;p.shimmer=(model==6||model==7)?.25f:0;reverb.set(p);reverb.clear();}
   else{delay.prepare(sr);tide::v13::DelaySettings p;p.model=model;p.time=p.rightTime=125;delay.set(p);delay.clear();}
   double dry=0,energy=0,ll=0,rr=0,lr=0;float peak=0;
   for(int n=0;n<frames+int(sr*2);++n){tide::Stereo x{n<frames?input.getSample(0,n):0,n<frames?input.getSample(1,n):0};const auto y=family?reverb.tick(x):delay.tick(x);
    if(!std::isfinite(y.l)||!std::isfinite(y.r)){std::cerr<<"Nonfinite fixture output\n";return 1;}
    dry+=double(x.l)*x.l+double(x.r)*x.r;energy+=double(y.l)*y.l+double(y.r)*y.r;peak=std::max(peak,std::max(std::abs(y.l),std::abs(y.r)));ll+=double(y.l)*y.l;rr+=double(y.r)*y.r;lr+=double(y.l)*y.r;}
   if(dry<=1e-12||energy<=1e-12){std::cerr<<"Empty fixture or wet response\n";return 1;}
   std::cout<<path.getFileName().toStdString()<<','<<sr<<','<<(family?"reverb":"delay")<<','<<model<<','<<dry<<','<<energy<<','<<peak<<','<<lr/std::sqrt(std::max(1e-30,ll*rr))<<'\n';
  }
 }
}
