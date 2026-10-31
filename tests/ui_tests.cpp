// SPDX-License-Identifier: AGPL-3.0-or-later
#include "../Source/PluginEditor.h"
#include "../Source/PresetBrowser.h"
#include <cstdlib>
#include <iostream>
static void check(bool ok,const char* message){if(!ok){std::cerr<<"FAIL: "<<message<<'\n';std::exit(1);}}
static juce::ComboBox& combo(TideEditor& editor,const char* name){for(auto* child:editor.getChildren())if(child->getName()==name)if(auto* box=dynamic_cast<juce::ComboBox*>(child))return *box;std::cerr<<"Missing combo "<<name;std::exit(1);}
static void refresh(TideProcessor& p,TideEditor& editor){
 editor.refreshSelectors();
 check(combo(editor,"delayModel").getSelectedId()==(p.value("dRevision")>.5f?int(p.value("d13Model"))+1:p.value("dModel")>0?int(p.value("dModel"))+6:int(p.value("dType"))+1),"delay selector follows active processing");
 check(combo(editor,"reverbModel").getSelectedId()==(p.value("rRevision")>.5f?int(p.value("r13Model"))+1:p.value("rModel")>0?int(p.value("rModel"))+4:int(p.value("rType"))+1),"reverb selector follows active processing");
 const char* names[]={"combinedPreset","delayPreset","reverbPreset"};
 for(int scope=0;scope<3;++scope){const auto display=p.presetDisplay(scope);auto& box=combo(editor,names[scope]);
  check(box.getText().contains(display.name),"scope label preserves selected identity");
  check(box.getText().startsWith(display.modified?"Modified: ":display.factory?"Factory: ":display.user?"User: ":""),"Factory/User/Modified state is visible");
  check(box.getSelectedId()==(display.factory&&!display.modified&&display.index>=0?display.index+1:10000),"menu item and display identity agree");
 }
 const auto identity=p.presetDisplay(0);const auto status=identity.modified?"MODIFIED":identity.factory?"FACTORY":identity.user?"USER":"CUSTOM";
 bool matching=false;for(auto* child:editor.getChildren())if(child->getName()=="presetStatus")if(auto* label=dynamic_cast<juce::Label*>(child))matching=label->getText().contains(status);
 check(matching,"status and preset selectors share the same identity state");

}
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI init;TideProcessor p;TideEditor editor(p);
 bool version=false;for(auto* child:editor.getChildren())if(child->getName()=="releaseBuild")if(auto* label=dynamic_cast<juce::Label*>(child))version=label->getText().contains("Wolf Tide v"+juce::String(JucePlugin_VersionString))&&label->getText().contains(TIDE_BUILD_ID);
 check(version,"prominent release label follows compiled version and source build identifier");
 p.setValue("sync",0);p.setValue("dTime",375);editor.refreshSelectors();p.setValue("sync",1);p.setValue("rhythm",4);editor.refreshSelectors();bool syncText=false;for(auto* child:editor.getChildren())if(auto* slider=dynamic_cast<juce::Slider*>(child))if(slider->getName()=="Time")for(auto* field:slider->getChildren())if(auto* value=dynamic_cast<juce::Label*>(field))syncText|=value->getText().contains("500.0");check(syncText,"sync toggle updates actual time even when BPM and stored milliseconds are unchanged");
 for(int i=0;i<tide::numPresets;++i){
  auto& box=combo(editor,"combinedPreset");box.setSelectedId(i+1,juce::sendNotificationSync);refresh(p,editor);
  check(p.presetDisplay(0).index==i&&!p.presetDisplay(0).modified,"all 52 combined dropdown paths have correct unmodified identities");
 }
 for(int scope=1;scope<=2;++scope)for(int i=0;i<(scope==1?24:26);++i){
  auto& box=combo(editor,scope==1?"delayPreset":"reverbPreset");box.setSelectedId(i+1,juce::sendNotificationSync);refresh(p,editor);
  check(p.presetDisplay(scope).index==i&&!p.presetDisplay(scope).modified,"all 40 side starter dropdown paths synchronize");
 }
 auto folder=juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("tide-ui","",false);folder.createDirectory();
 for(int scope=0;scope<3;++scope){auto file=folder.getChildFile("Scope-"+juce::String(scope)+".wolftide");check(p.savePreset(file,scope).wasOk(),"save user identity fixture");
  p.setCurrentProgram(16);check(p.loadPreset(file,scope).wasOk(),"load user identity fixture");refresh(p,editor);check(!p.presetDisplay(scope).factory&&p.presetDisplay(scope).name==file.getFileNameWithoutExtension(),"custom preset identity follows its own scope");
 }
 p.loadPreset(folder.getChildFile("Scope-1.wolftide"),1);const auto delayName=p.presetDisplay(1).name;
 p.loadPreset(folder.getChildFile("Scope-2.wolftide"),2);check(p.presetDisplay(1).name==delayName,"loading another scope preserves delay user identity");
 juce::MemoryBlock state;p.getStateInformation(state);p.setCurrentProgram(20);p.setStateInformation(state.getData(),int(state.getSize()));refresh(p,editor);
 check(p.presetDisplay(1).name==delayName&&p.presetDisplay(2).name=="Scope-2","project reopen restores independent identities");
 p.setCurrentProgram(16);p.setValue("d13Model",2);refresh(p,editor);check(p.presetDisplay(0).modified&&p.presetDisplay(1).modified,"manual/automated effect change marks factory identities modified");
 p.setValue("d13Model",0);refresh(p,editor);check(!p.presetDisplay(0).modified,"returning to saved parameters clears Modified");
 for(int mask=0;mask<16;++mask)for(int i=0;i<tide::numPresets;++i){
  p.lockDelay.store(false);p.lockReverb.store(false);p.lockMix.store(false);p.lockRouting.store(false);p.setCurrentProgram(16);
  const auto beforeDelay=p.presetDisplay(1).name;const auto beforeReverb=p.presetDisplay(2).name;
  const int delay=int(p.value("dModel"))*10+int(p.value("dType")),verb=int(p.value("rModel"))*10+int(p.value("rType"));
  p.lockDelay.store(mask&1);p.lockReverb.store(mask&2);p.lockMix.store(mask&4);p.lockRouting.store(mask&8);p.setCurrentProgram(i);refresh(p,editor);
  if(mask&1)check(int(p.value("dModel"))*10+int(p.value("dType"))==delay&&p.presetDisplay(1).name==beforeDelay,"locked delay retains type and identity");
  if(mask&2)check(int(p.value("rModel"))*10+int(p.value("rType"))==verb&&p.presetDisplay(2).name==beforeReverb,"locked reverb retains type and identity");
 }
 p.lockDelay.store(false);p.lockReverb.store(false);p.lockMix.store(false);p.lockRouting.store(false);
 const auto presets=juce::File(__FILE__).getParentDirectory().getParentDirectory().getChildFile("Factory-Presets");
 auto portable=presets.findChildFiles(juce::File::findFiles,true,"*.wolftide");check(portable.size()==206,"all preserved portable preset files found");
 for(const auto& file:portable){const auto parent=file.getParentDirectory().getFileName();const int scope=parent=="Delay"?1:parent=="Reverb"?2:0;
  check(p.loadPreset(file,scope).wasOk(),"every preserved portable preset migrates");refresh(p,editor);
 }
 auto legacy=folder.getChildFile("legacy.wolftide");legacy.replaceWithText("<WolfTidePreset version=\"1\" scope=\"1\" name=\"Legacy Delay\"><p id=\"interval\" value=\"3\"/></WolfTidePreset>");
 check(p.loadPreset(legacy,1).wasOk(),"legacy partial preset loads");refresh(p,editor);check(p.presetDisplay(1).name=="Legacy Delay","legacy preset identity is preserved");
 auto custom=folder.getChildFile("Custom Dream.wolftide");check(p.savePreset(custom,1).wasOk(),"save user name starting Custom");refresh(p,editor);check(combo(editor,"delayPreset").getText().startsWith("User: Custom Dream"),"user provenance is independent of naming");
 const auto prior=p.presetDisplay(2).name;p.applyReverbPreset(2,false);
 for(auto* child:editor.getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="UNDO LOAD")button->onClick();
 refresh(p,editor);check(p.presetDisplay(2).name==prior,"Undo Load restores selector identity");
 // Browser LOAD, previous/next and custom import callback share the editor refresh.
 bool refreshed=false;TidePresetBrowser browser(p,0,[]{},[&]{refreshed=true;editor.refreshSelectors();},folder.getChildFile("Browser"));
 juce::ListBox* list=nullptr;for(auto* child:browser.getChildren())if(auto* box=dynamic_cast<juce::ListBox*>(child))list=box;
 check(list!=nullptr,"browser list exists");list->selectRow(2);
 for(auto* child:browser.getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="LOAD")button->onClick();
 check(refreshed&&p.presetDisplay(0).index==18,"browser load immediately refreshes selectors");refresh(p,editor);
 auto press=[&](const char* title){for(auto* child:editor.getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()==title){button->onClick();return;}check(false,"performance button found");};
 p.setCurrentProgram(20);editor.refreshSelectors();press("A / B");p.setCurrentProgram(30);press("SWAP A / B");refresh(p,editor);check(p.presetDisplay(0).index==20,"A/B recall restores all selector identities");
 // Save and load a user preset through the actual browser controls, in an isolated test library.
 for(auto* child:browser.getChildren())if(child->getName()=="presetName")if(auto* field=dynamic_cast<juce::TextEditor*>(child))field->setText("Browser saved");
 for(auto* child:browser.getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="SAVE USER")button->onClick();
 refresh(p,editor);check(p.presetDisplay(0).name=="Browser saved"&&p.presetDisplay(0).user,"browser user save refreshes identity");
 p.setCurrentProgram(16);
 for(auto* child:browser.getChildren())if(child->getName()=="presetCollection")if(auto* box=dynamic_cast<juce::ComboBox*>(child))box->setSelectedId(2,juce::sendNotificationSync);
 list->selectRow(0);refreshed=false;
 for(auto* child:browser.getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()=="LOAD")button->onClick();
 check(refreshed&&p.presetDisplay(0).name=="Browser saved","browser user load refreshes identity immediately");refresh(p,editor);
 if(argc>1){editor.setSize(1080,720);auto picture=editor.createComponentSnapshot(editor.getLocalBounds());auto output=juce::File(argv[1]).createOutputStream();check(bool(output),"UI snapshot output");output->setPosition(0);output->truncate();juce::PNGImageFormat png;check(png.writeImageToStream(picture,*output),"UI snapshot rendered");}
 // Rendering must preserve transparent accessibility labels and visible tooltip text.
 {TideLook theme;juce::Label accessible;accessible.setBounds(0,0,220,80);accessible.setText("Accessible only",juce::dontSendNotification);accessible.setColour(juce::Label::textColourId,juce::Colours::transparentBlack);
  juce::Image image(juce::Image::RGB,220,80,true);juce::Graphics graphics(image);graphics.fillAll(juce::Colours::magenta);theme.drawLabel(graphics,accessible);
  bool untouched=true;for(int y=0;y<80;++y)for(int x=0;x<220;++x)untouched&=image.getPixelAt(x,y)==juce::Colours::magenta;check(untouched,"transparent accessibility label stays invisible");
  theme.drawTooltip(graphics,"Fine drag",220,80);bool legible=false;for(int y=10;y<68;++y)for(int x=12;x<208;++x){const auto colour=image.getPixelAt(x,y);legible|=colour.getRed()>230&&colour.getGreen()>220&&colour.getBlue()>210;}check(legible,"tooltip has visible native ivory text");}
 // Vary actual field sizes past the byte budget; shared compositions must remain bounded.
 {juce::Image image(juce::Image::RGB,1800,40,true);juce::Graphics g(image);for(int width=600;width<1600;width+=7)tide::ui::paintSkin(g,{0,0,float(width),30},tide::ui::Skin::field_normal);check(tide::ui::cachedSkinBytes()<=8*1024*1024,"shared skin cache stays within eight MiB");g.addTransform(juce::AffineTransform::scale(32));tide::ui::paintSkin(g,{0,0,1000,30},tide::ui::Skin::field_normal);check(tide::ui::cachedSkinBytes()<=8*1024*1024,"oversized physical scale bypasses the cache");}
 // A same-size page switch must invalidate the printed heading cache.
 {TideProcessor legacy;legacy.setValue("dRevision",0);legacy.setValue("rRevision",0);TideEditor view(legacy);
  auto press=[&](const char* title){bool found=false;for(auto* child:view.getChildren())if(auto* button=dynamic_cast<juce::TextButton*>(child))if(button->getButtonText()==title){button->onClick();found=true;break;}check(found,"legacy edit button exists");};
  press("EDIT DELAY");const auto first=view.createComponentSnapshot({43,713,450,24});const auto height=view.getHeight();
  press("EDIT REVERB");const auto second=view.createComponentSnapshot({43,713,450,24});check(view.getHeight()==height,"legacy page switch keeps geometry");
  bool changed=false;for(int y=0;y<first.getHeight();++y)for(int x=0;x<first.getWidth();++x)changed|=first.getPixelAt(x,y)!=second.getPixelAt(x,y);
  check(changed,"same-size expanded page switch refreshes printed heading");}
 folder.deleteRecursively();
 std::cout<<"PASS: 64 combined, 50 side starters, 206 portable files, independent user names, project recall, automation, 1024 lock combinations, browser refresh and compiled release label.\n";
}
