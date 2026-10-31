// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/PluginEditor.h"
#include "../Source/ModernPanel.h"
#include "../Source/PresetBrowser.h"
#include <thread>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
using Clock=std::chrono::steady_clock;
static double ms(Clock::duration elapsed){return std::chrono::duration<double,std::milli>(elapsed).count();}
static void check(bool ok,const char* text){if(!ok){std::cerr<<text<<'\n';std::exit(1);}}
struct Bench:juce::Timer {
 std::vector<std::unique_ptr<TideProcessor>> processors;std::vector<std::unique_ptr<TideEditor>> editors;
 std::vector<juce::Image> frames;std::vector<double> audioTimes,paintTimes,lags;std::atomic<bool> finished{false};std::thread audio;
 Clock::time_point start,last;std::clock_t cpu=0;int instances=1;bool open=false,forcePaint=true;
 Bench(int count,bool visible,bool force=true):instances(count),open(visible),forcePaint(force){
  for(int i=0;i<count;++i){auto p=std::make_unique<TideProcessor>();p->prepareToPlay(48000,256);p->visualMode=2;
   if(open){auto editor=std::make_unique<TideEditor>(*p);editor->addToDesktop(0);editor->setTopLeftPosition(20+i*12,20+i*12);editor->setVisible(true);frames.emplace_back(juce::Image::RGB,1080,720,true);editors.push_back(std::move(editor));}processors.push_back(std::move(p));}
  audioTimes.reserve(300);paintTimes.reserve(100);lags.reserve(100);last=start=Clock::now();cpu=std::clock();
  audio=std::thread([this]{juce::AudioBuffer<float> block(2,256);juce::MidiBuffer midi;auto next=Clock::now();for(int n=0;n<235;++n){const auto t=Clock::now();for(auto& p:processors){for(int i=0;i<256;++i){block.setSample(0,i,.025f*std::sin(float(n*256+i)*.071f));block.setSample(1,i,.017f*std::sin(float(n*256+i)*.053f));}p->processBlock(block,midi);}audioTimes.push_back(ms(Clock::now()-t));next+=std::chrono::microseconds(5333);std::this_thread::sleep_until(next);}finished=true;});startTimerHz(30);
 }
 void timerCallback()override{auto now=Clock::now();lags.push_back(std::max(0.,ms(now-last)-1000./30));last=now;if(open&&forcePaint){const auto paint=Clock::now();for(std::size_t i=0;i<editors.size();++i){juce::Graphics g(frames[i]);editors[i]->paintEntireComponent(g,true);}paintTimes.push_back(ms(Clock::now()-paint));}if(finished){stopTimer();audio.join();const double elapsed=ms(Clock::now()-start),cpuMs=1000.*double(std::clock()-cpu)/CLOCKS_PER_SEC;auto mean=[](const auto& values){double sum=0;for(auto x:values)sum+=x;return sum/std::max(std::size_t(1),values.size());};auto max=[](const auto& values){return values.empty()?0.:*std::max_element(values.begin(),values.end());};std::cout<<instances<<','<<(open?"open":"closed")<<','<<elapsed<<','<<cpuMs<<','<<mean(audioTimes)<<','<<max(audioTimes)<<','<<mean(paintTimes)<<','<<max(lags)<<'\n';juce::MessageManager::getInstance()->stopDispatchLoop();}}
 ~Bench()override{stopTimer();if(audio.joinable())audio.join();for(auto& e:editors)e->removeFromDesktop();}
};
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI init;if(argc>1&&(std::string(argv[1])=="--bench"||std::string(argv[1])=="--bench-normal")){Bench bench(std::stoi(argv[2]),std::stoi(argv[3])!=0,std::string(argv[1])=="--bench");juce::MessageManager::getInstance()->runDispatchLoop();return 0;}
 if(argc>2&&std::string(argv[1])=="--audio"){TideProcessor p;p.prepareToPlay(48000,256);TideEditor editor(p);std::ofstream out(argv[2],std::ios::binary);juce::AudioBuffer<float> block(2,256);juce::MidiBuffer midi;for(int program=0;program<tide::numPresets;++program){p.setCurrentProgram(program);p.clearRequested=true;for(int n=0;n<96;++n){for(int i=0;i<256;++i){const float x=n<48?.03f*std::sin(float(i+n*256)*.071f):0;block.setSample(0,i,x);block.setSample(1,i,x*.61f);}p.processBlock(block,midi);for(int c=0;c<2;++c)out.write(reinterpret_cast<const char*>(block.getReadPointer(c)),256*sizeof(float));}}check(bool(out),"audio fixture write");return 0;}
 if(argc>2&&std::string(argv[1])=="--screens"){juce::File folder(argv[2]);folder.createDirectory();TideProcessor p;TideEditor editor(p);auto save=[&](juce::Component& component,const juce::String& name,float scale){auto image=component.createComponentSnapshot(component.getLocalBounds(),true,scale);auto stream=folder.getChildFile(name+".png").createOutputStream();check(bool(stream),"screenshot write");stream->setPosition(0);stream->truncate();juce::PNGImageFormat png;check(png.writeImageToStream(image,*stream),"PNG write");};
  auto savePanel=[&](TideModernPanel& panel,const juce::String& name){save(panel,name,1);for(auto* child:panel.getChildren())if(auto* viewport=dynamic_cast<juce::Viewport*>(child))if(auto* content=viewport->getViewedComponent())save(*content,name+"-controls",1);};
  for(int width:{900,1080,1620}){editor.setSize(width,width*2/3);save(editor,"main-"+juce::String(width),1);if(width==1080)save(editor,"main-retina",2);}
  for(int tab=0;tab<5;++tab){TideModernPanel panel(p,tab);savePanel(panel,"advanced-"+juce::String(tab));}
  for(int scope=0;scope<3;++scope){TidePresetBrowser browser(p,scope,[]{});save(browser,scope==0?"presets":"presets-"+juce::String(scope),1);}
  for(int model=0;model<12;++model){p.setValue("d13Model",float(model));TideModernPanel panel(p,0);savePanel(panel,"delay-model-"+juce::String(model));}
  for(int model=0;model<13;++model){p.setValue("r13Model",float(model));TideModernPanel panel(p,2);savePanel(panel,"reverb-model-"+juce::String(model));}
  p.setValue("dRevision",0);p.setValue("rRevision",0);TideEditor legacy(p);
  for(const auto* title:{"EDIT DELAY","EDIT REVERB","ROUTING"}){for(auto* child:legacy.getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()==title){button->onClick();break;}save(legacy,juce::String("legacy-")+title,1);}
  return 0;}
 return 2;}
