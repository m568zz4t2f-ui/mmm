// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/PluginEditor.h"
#include "../Source/ModernPanel.h"
#include <cstdio>
#include <cstdlib>
static void check(bool value,const char* text){if(!value){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int main(int argc,char** argv){juce::ScopedJuceInitialiser_GUI init;TideProcessor p;juce::MidiBuffer midi;juce::AudioBuffer<float> audio(2,128);
 p.setCurrentProgram(16);check(p.value("dRevision")==1&&p.value("rRevision")==1,"new default uses 1.3");
 // Old session schema has no appended controls, irrespective of what was played last.
 auto state=[&]{juce::MemoryBlock data;p.getStateInformation(data);return data;};
 auto modern=state();auto xml=p.getXmlFromBinary(modern.getData(),int(modern.getSize()));auto old=juce::ValueTree::fromXml(*xml);old.setProperty("version",3,nullptr);
 for(auto* parameter:p.getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(tide::v13::isModernId(param->paramID.toStdString()))old.removeChild(old.getChildWithProperty("id",param->paramID),nullptr);
 auto oldXml=old.createXml();juce::MemoryBlock legacy;p.copyXmlToBinary(*oldXml,legacy);p.setCurrentProgram(57);p.setStateInformation(legacy.getData(),int(legacy.getSize()));
 check(p.value("dRevision")==0&&p.value("rRevision")==0&&p.value("d2On")==0&&p.value("r2On")==0,"1.2.2 state resets new engine and slots");
 p.applyDelayPreset(9);check(p.value("dRevision")==1&&p.value("rRevision")==0,"delay-only upgrade preserves legacy reverb revision");p.applyReverbPreset(12);check(p.value("d13Model")==9&&p.value("r13Model")==12,"new machine selectors independent");
 for(int mask=0;mask<16;++mask){p.lockDelay=false;p.lockReverb=false;p.lockMix=false;p.lockRouting=false;p.setCurrentProgram(57);auto d=p.value("d13Model"),r=p.value("r13Model"),dB=p.value("d2Model"),rB=p.value("r2Model"),route=p.value("routing13");p.lockDelay=mask&1;p.lockReverb=mask&2;p.lockMix=mask&4;p.lockRouting=mask&8;p.setCurrentProgram(54);
  if(mask&1)check(p.value("d13Model")==d&&p.value("d2Model")==dB,"lock delay protects both 1.3 slots");if(mask&2)check(p.value("r13Model")==r&&p.value("r2Model")==rB,"lock reverb protects both 1.3 slots");if(mask&8)check(p.value("routing13")==route,"routing lock protects new topology");}
 p.lockDelay=false;p.lockReverb=false;p.lockMix=false;p.lockRouting=false;
 p.setValue("d2Amount",.123f);p.setValue("r2Amount",.456f);p.lockMix=true;p.applyDelayPreset(10);p.applyReverbPreset(11);check(std::abs(p.value("d2Amount")-.123f)<.002f&&std::abs(p.value("r2Amount")-.456f)<.002f,"mix lock protects B returns during scoped starters");p.lockMix=false;
 const auto file=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("tide13-scope",".wolftide");
 for(int scope=0;scope<3;++scope){p.setCurrentProgram(57);check(p.savePreset(file,scope).wasOk(),"save schema 4");p.setCurrentProgram(54);check(p.loadPreset(file,scope).wasOk(),"load schema 4");if(scope<2)check(p.value("d2On")==1,"delay B state recalled");if(scope!=1)check(p.value("r2On")==1,"reverb B state recalled");}file.deleteFile();
 file.replaceWithText("<WolfTidePreset version=\"1\" scope=\"0\" name=\"Old\"><p id=\"dTime\" value=\"120\"/></WolfTidePreset>");p.setValue("routing13",2);p.setValue("d2Amount",.123f);p.lockRouting=true;p.lockMix=true;check(p.loadPreset(file,0).wasOk(),"legacy fixture loads with locks");check(p.value("routing13")==2&&std::abs(p.value("d2Amount")-.123f)<.002f,"old preset defaults preserve routing and B-return locks");check(p.presetDisplay(0).modified,"partial legacy load reports protected new parameters as modified");p.lockRouting=false;p.lockMix=false;file.deleteFile();
 // All 25 algorithms through the plugin's actual audio callback, in mixed and modern revisions.
 p.prepareToPlay(48000,128);
 for(int family=0;family<2;++family)for(int model=0;model<(family?13:12);++model){p.setCurrentProgram(16);p.setValue("sync",0);p.setValue("dTime",45);p.setValue("dDuck",0);p.setValue("rDuck",0);p.setValue("rPredelay",0);p.setValue("mix",1);p.setValue("send",1);p.setValue(family?"dOn":"rOn",0);p.setValue(family?"r13Model":"d13Model",float(model));p.clearRequested=true;double energy=0;
  for(int block=0;block<420;++block){audio.clear();if(block>=10&&block<100)for(int n=0;n<128;++n){const float x=.05f*std::sin(float(block*128+n)*.071f);audio.setSample(0,n,x);audio.setSample(1,n,x*.47f);}p.processBlock(audio,midi);for(int c=0;c<2;++c)for(int n=0;n<128;++n){auto v=audio.getSample(c,n);check(std::isfinite(v)&&std::abs(v)<=1,"host model finite and bounded");energy+=double(v)*v;}}
  check(energy>1e-7,"each host-integrated model returns wet audio");}
 for(int route=0;route<3;++route){p.setCurrentProgram(57);p.setValue("routing13",float(route));p.setValue("crossFeedback",.3f);for(int block=0;block<400;++block){audio.clear();if(block<40)audio.applyGain(0);for(int n=0;n<128;++n)audio.setSample(0,n,block<40?.02f*std::sin(float(n+block*128)*.031f):0);p.processBlock(audio,midi);check(std::isfinite(audio.getMagnitude(0,128)),"four-slot host routing and cross feedback finite");}}
 // Advanced controls are native attachments; rendering and automation rebuilds are exercised.
 TideEditor editor(p);
 for(int tab=0;tab<5;++tab){TideModernPanel panel(p,tab);
  auto image=panel.createComponentSnapshot(panel.getLocalBounds());check(image.isValid(),"advanced panel renders each slot");
  if(argc>1){auto destination=juce::File(argv[1]).getChildFile("advanced-"+juce::String(tab)+".png");auto stream=destination.createOutputStream();check(bool(stream),"snapshot destination");juce::PNGImageFormat format;check(format.writeImageToStream(image,*stream),"advanced snapshot saved");}}
 TideModernPanel automated(p,0);p.setValue("d13Model",8);
 juce::Timer::callAfterDelay(400,[&]{bool bits=false,age=false;for(auto* child:automated.getChildren())if(auto* viewport=dynamic_cast<juce::Viewport*>(child))if(auto* content=viewport->getViewedComponent())for(auto* field:content->getChildren()){bits|=field->getName()=="dBits";age|=field->getName()=="dAge";}
  check(bits&&!age,"host automation refreshes machine-specific advanced controls");for(auto* child:automated.getChildren())if(auto* page=dynamic_cast<juce::ComboBox*>(child))page->setSelectedId(2,juce::sendNotificationSync);});
 juce::Timer::callAfterDelay(800,[&]{bool bModel=false;for(auto* child:automated.getChildren())if(auto* viewport=dynamic_cast<juce::Viewport*>(child))if(auto* content=viewport->getViewedComponent())for(auto* field:content->getChildren())bModel|=field->getName()=="d2Model";check(bModel,"native page selector opens independent B controls");juce::MessageManager::getInstance()->stopDispatchLoop();});
 juce::MessageManager::getInstance()->runDispatchLoop();
 std::puts("PASS: schema-4 compatibility, per-section upgrades, locks, scoped four-slot recall, all 25 actual processor paths and native advanced panels.");}
