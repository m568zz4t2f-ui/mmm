// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/PluginProcessor.h"
#include <iostream>
#include <cstdlib>
static void check(bool b,const char* m){if(!b){std::cerr<<"FAIL: "<<m<<'\n';std::exit(1);}}
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI init;TideProcessor p;
 check(p.getParameters().size()==44,"all 44 parameters exposed");
 juce::StringArray names;for(auto* parameter:p.getParameters()){auto name=parameter->getName(100);check(!names.contains(name),"host parameter names are unique");names.add(name);}
 for(int i=0;i<tide::numPresets;++i){p.setCurrentProgram(i);check(p.factoryMatches(),"all factory presets match complete parameter state");}p.setCurrentProgram(0);
 for(int mode=0;mode<3;++mode){juce::AudioProcessor::BusesLayout layout;
  layout.inputBuses.add(mode==2?juce::AudioChannelSet::stereo():juce::AudioChannelSet::mono());layout.outputBuses.add(mode==0?juce::AudioChannelSet::mono():juce::AudioChannelSet::stereo());
  check(p.setBusesLayout(layout),"supported channel layout");p.prepareToPlay(48000,512);juce::AudioBuffer<float> b(p.getTotalNumOutputChannels(),512);juce::MidiBuffer midi;
  b.clear();b.setSample(0,0,.5f);p.processBlock(b,midi);p.reset();double energy=0;
  for(int n=0;n<120;++n){b.clear();p.processBlock(b,midi);for(int c=0;c<b.getNumChannels();++c)for(int i=0;i<512;++i){const float v=b.getSample(c,i);check(std::isfinite(v),"finite host audio");energy+=double(v)*v;}}
  check(energy>1e-8,"host reset preserves tails");p.prepareToPlay(48000,512);b.clear();p.processBlock(b,midi);check(b.getMagnitude(0,512)<1e-10,"reinitialization clears");
 }
 p.setCurrentProgram(4);p.setValue("rDecay",2.713f);p.lockMix.store(true);p.setValue("send",1);
 juce::MemoryBlock saved;p.getStateInformation(saved);p.setCurrentProgram(0);p.setValue("send",0);p.setStateInformation(saved.getData(),int(saved.getSize()));
 check(std::abs(p.value("rDecay")-2.71f)<.02f&&p.value("send")>.5f&&p.lockMix.load(),"session recall includes edits and locks");
 p.lockDelay.store(true);p.setValue("dFeedback",.713f);p.setCurrentProgram(10);check(std::abs(p.value("dFeedback")-.713f)<.002f,"side lock respected");
 p.lockDelay.store(false);p.setValue("dAmount",.123f);p.setValue("rAmount",.456f);p.setCurrentProgram(3);check(std::abs(p.value("dAmount")-.123f)<.002f&&std::abs(p.value("rAmount")-.456f)<.002f,"mix lock protects both amounts");p.lockMix.store(false);p.setValue("send",1);p.setCurrentProgram(2);check(p.value("send")==1,"send survives factory changes");
 const auto file=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("wolf-tide-test",".wolftide");
 check(p.savePreset(file,1).wasOk(),"save delay preset");const auto old=p.value("dFeedback");p.setValue("dFeedback",.01f);p.setValue("rDecay",3.14f);
 check(p.loadPreset(file,1).wasOk(),"load delay preset");check(std::abs(p.value("dFeedback")-old)<.002f&&std::abs(p.value("rDecay")-3.14f)<.02f,"side preset scope");
 check(p.loadPreset(file,2).failed(),"wrong preset scope rejected");file.replaceWithText("<WolfTidePreset version=\"1\" scope=\"1\"><p id=\"dFeedback\" value=\"nan\"/></WolfTidePreset>");check(p.loadPreset(file,1).failed(),"nonfinite preset rejected");file.deleteFile();
 p.visualMode.store(2);{juce::MemoryBlock uiState;p.getStateInformation(uiState);p.visualMode.store(0);p.setStateInformation(uiState.getData(),int(uiState.getSize()));check(p.visualMode.load()==2,"visual preference recalls without adding an audio parameter");}
 p.setValue("pitchMode",1);check(p.savePreset(file,1).wasOk(),"save pitch routing");p.setValue("pitchMode",0);check(p.loadPreset(file,1).wasOk()&&p.value("pitchMode")==1,"pitch routing belongs to delay presets");file.deleteFile();
 file.replaceWithText("<WolfTidePreset version=\"1\" scope=\"1\"><p id=\"interval\" value=\"3\"/></WolfTidePreset>");check(p.loadPreset(file,1).wasOk()&&p.value("pitchMode")==0,"1.0 presets retain climbing pitch behavior");file.deleteFile();
 {TideProcessor a,b;a.prepareToPlay(48000,512);b.prepareToPlay(48000,512);a.visualMode.store(0);b.visualMode.store(2);b.editorVisible.store(true);juce::MidiBuffer midi;juce::AudioBuffer<float> x(2,512),y(2,512);
  for(int block=0;block<110;++block){x.clear();if(block<6)x.setSample(0,block,.2f);y.makeCopyOf(x);a.processBlock(x,midi);b.processBlock(y,midi);for(int c=0;c<2;++c)for(int n=0;n<512;++n)check(x.getSample(c,n)==y.getSample(c,n),"visual mode and metering do not change audio");}}
 p.setCurrentProgram(-5);check(p.getCurrentProgram()>=0,"invalid factory index ignored");
 p.setValue("send",0);p.setCurrentProgram(0);
 check(p.factoryMatches(),"factory detector matches complete initial state");p.setValue("dWidth",.33f);check(!p.factoryMatches(),"advanced edit marks preset modified");p.setCurrentProgram(0);
 {std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());check(editor!=nullptr,"native editor");editor->setSize(1080,720);
  for(auto* child:editor->getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child))if(slider->getName()=="Time"){auto text=slider->getTextFromValue(slider->getValue());check(text.contains("500.0")&&text.contains("ms"),"synced time display shows actual milliseconds");}
  auto snapshot=[&](const juce::String& filename){auto picture=editor->createComponentSnapshot(editor->getLocalBounds());auto stream=juce::File::getCurrentWorkingDirectory().getChildFile(filename).createOutputStream();check(bool(stream),"snapshot file");stream->setPosition(0);stream->truncate();juce::PNGImageFormat png;check(png.writeImageToStream(picture,*stream),"snapshot image");};
  if(argc>1){snapshot(argv[1]);
   for(const juce::String name:{"EDIT DELAY","EDIT REVERB","ROUTING"}){
    bool found=false;for(auto* child:editor->getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()==name){button->onClick();found=true;break;}
    check(found&&editor->getHeight()==930,"deep editor opens");snapshot(juce::String(argv[1]).upToLastOccurrenceOf(".",false,false)+"-"+name.toLowerCase().replace(" ","-")+".png");
   }
  }
  editor->setSize(900,600);editor->setSize(1620,1395);
  if(argc>2){juce::File factoryDir=juce::File::getCurrentWorkingDirectory().getChildFile(argv[2]);factoryDir.createDirectory();
   for(int i=0;i<tide::numPresets;++i){p.setCurrentProgram(i);auto name=p.getProgramName(i).replace(" / "," - ");check(p.savePreset(factoryDir.getChildFile(name+".wolftide"),0).wasOk(),"export factory files");}
  }

 }
 std::cout<<"PASS: processor channels, host reset, state, locks, send, scoped files, input rejection, editor.\n";
}
