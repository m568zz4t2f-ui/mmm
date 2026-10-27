// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PluginEditor.h"
#include "PresetBrowser.h"
namespace {
const juce::Colour ink(0xffeee6d7),muted(0xffb4b4ab),silver(0xffcec9bd),blue(0xff8cd1e5),bg(0xff141e24);
using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
using CA=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
using BA=juce::AudioProcessorValueTreeState::ButtonAttachment;
constexpr int mainIndices[]={0,1,2,3,11,12,13,14};
constexpr const char* tips[]={
 "Delay time in milliseconds. Turn SYNC off to edit freely.","Regeneration per pass. Kept below runaway feedback.","Repeat high-frequency cutoff; tape and bucket modes add their own coloration.","Audible delay return, independent of its reverb send.",
 "Delay modulation depth. Adds pitch movement.","Stereo width of repeats. 100% preserves their stereo field.","Soft saturation in Tape and Bucket modes.","Bass removed from repeating echoes.","Ducks echoes while the input is loud; they recover in the gaps.","Rate of delay modulation.","Second delay time as a ratio of the first, in Dual mode.",
 "Nominal decay. Room caps at 8 s; Hall/Bloom at 20 s. Damping shortens higher frequencies.","Time before the reverb begins.","Treble damping inside the reverb.","Audible reverb return.","Slow modulation in Hall and Bloom.","Stereo width of the reverb return. Check wide settings in mono.","Frequency used by the reverb bass-weight filter.","Ducks the reverb while you play or sing.","Pitch-shifted atmosphere and bounded feedback. Uses its own shimmer interval selector.","Amount of bass retained below the low-cut frequency.",
 "Stereo delay feed into reverb, independent of Delay Amount. Inactive in Parallel routing.","Original performance feed into reverb.","Global dry/wet blend. Send mode keeps the dry path silent.","Final gain trim. Amber meter means output exceeds 0 dBFS.","Performance macro: increases delay feedback and reverb decay within bounded ranges.","Tempo used when the host does not supply BPM.","Detector sensitivity for delay ducking. Raise for quieter recordings.","Time for echoes to recover after the input quiets.","Detector sensitivity for reverb ducking. Raise for quieter recordings.","Time for reverb to recover after the input quiets."
};
void label(juce::Graphics& g,const juce::String& text,float x,float y,float w,float h,float size,juce::Colour colour=ink){g.setColour(colour);g.setFont(juce::FontOptions(size));g.drawText(text,juce::Rectangle<float>(x,y,w,h),juce::Justification::centredLeft);}
}
TideLook::TideLook(){
 setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff182831));setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff53616a));setColour(juce::ComboBox::textColourId,ink);
 setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff182831));setColour(juce::PopupMenu::textColourId,ink);setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff365f71));
 setColour(juce::Slider::textBoxTextColourId,ink);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
 setColour(juce::Slider::trackColourId,blue);setColour(juce::Slider::thumbColourId,blue);setColour(juce::Label::textColourId,muted);
 setColour(juce::TextButton::textColourOffId,muted);setColour(juce::TextButton::textColourOnId,ink);
}
void TideLook::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down){
 auto r=b.getLocalBounds().toFloat().reduced(.8f);auto c=b.getToggleState()?juce::Colour(0xff376578):juce::Colour(0xff21313b);
 g.setColour(c.brighter(hover?.15f:0).darker(down?.1f:0));g.fillRoundedRectangle(r,6);g.setColour(juce::Colour(0xff40566b));g.drawRoundedRectangle(r,6,.7f);
}
void TideLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider& slider){
 const auto r=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(10);const float radius=std::min(r.getWidth(),r.getHeight())*.5f;
 const float cx=r.getCentreX(),cy=r.getCentreY();auto accent=slider.findColour(juce::Slider::rotarySliderFillColourId);
 juce::Path track,arc;track.addCentredArc(cx,cy,radius,radius,0,start,end,true);arc.addCentredArc(cx,cy,radius,radius,0,start,start+pos*(end-start),true);
 g.setColour(juce::Colour(0xff3a4b5b));g.strokePath(track,juce::PathStrokeType(3));g.setColour(accent);g.strokePath(arc,juce::PathStrokeType(3,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 juce::ColourGradient gradient(juce::Colour(0xff626b70),cx,cy-radius,juce::Colour(0xff080c10),cx,cy+radius,false);g.setGradientFill(gradient);g.fillEllipse(cx-radius+6,cy-radius+6,2*(radius-6),2*(radius-6));
 g.setColour(juce::Colours::black.withAlpha(.35f));g.drawEllipse(cx-radius+6,cy-radius+7,2*(radius-6),2*(radius-6),2);
 g.setColour(juce::Colour(0xffabb3b3).withAlpha(.25f));g.drawEllipse(cx-radius+10,cy-radius+10,2*(radius-10),2*(radius-10),.7f);
 const float angle=start+pos*(end-start);g.setColour(ink);g.drawLine(cx+std::sin(angle)*(radius*.5f),cy-std::cos(angle)*(radius*.5f),cx+std::sin(angle)*(radius*.77f),cy-std::cos(angle)*(radius*.77f),2);
}
TideEditor::TideEditor(TideProcessor& p):AudioProcessorEditor(&p),processor(p){
 setLookAndFeel(&look);setResizable(true,true);setResizeLimits(900,600,1620,1695);
 auto show=[&](juce::Component& c){addAndMakeVisible(c);};
 for(int i=0;i<tide::numFloats;++i){auto& k=knobs[std::size_t(i)];auto& l=labels[std::size_t(i)];const auto& spec=tide::specs[i];
  show(k);show(l);k.setName(spec.name);k.setTooltip(i<31?::tips[i]:spec.name);k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k.setTextBoxStyle(juce::Slider::TextBoxBelow,false,104,23);
  k.setColour(juce::Slider::rotarySliderFillColourId,i<11||(i>=31&&i<=34)?silver:blue);k.setDoubleClickReturnValue(true,spec.initial);k.setPopupMenuEnabled(true);
  l.setText(juce::String(spec.name).toUpperCase(),juce::dontSendNotification);l.setJustificationType(juce::Justification::centred);l.setFont(juce::FontOptions(11));
  sliders[std::size_t(i)]=std::make_unique<SA>(p.state,spec.id,k);
  const juce::String unit(spec.unit);k.textFromValueFunction=[this,unit,i](double x){if(i==0&&processor.value("sync")>.5f){const float time=std::min(8000.f,60000.f/std::max(20.f,processor.tempo.load())*tide::beats[std::clamp(int(processor.value("rhythm")),0,8)]);return juce::String(time,1)+" ms";}return juce::String(unit=="%"?x*100:x,unit=="Hz"&&x>10?0:unit=="s"?2:1)+" "+unit;};
  k.valueFromTextFunction=[unit](const juce::String& text){return text.getDoubleValue()/(unit=="%"?100.:1.);};k.updateText();
 }
 auto setupCombo=[&](juce::ComboBox& c,const char* id,int index,juce::StringArray items){show(c);c.addItemList(items,1);combos[std::size_t(index)]=std::make_unique<CA>(p.state,id,c);};
 show(delayType);delayType.addItemList({"Digital","Tape","Analog / Bucket","Pattern / Multitap","Reverse","Pitch","Diffusion","Granular Pitch","Lo-Fi"},1);
 delayType.onChange=[this]{int n=delayType.getSelectedId()-1;if(n<0)return;processor.setValue("dVersion",1);processor.setValue("dModel",n>=6?float(n-5):0.f);if(n<6)processor.setValue("dType",float(n));};
 show(reverbType);reverbType.addItemList({"Room / Classic","Plate / Classic","Hall / Classic","Bloom / Classic","Chamber","Cloud Ensemble","Halo Shimmer","Glint","Studio Plate","Studio Hall","Studio Bloom"},1);
 reverbType.onChange=[this]{int n=reverbType.getSelectedId()-1;if(n<0)return;processor.setValue("rVersion",1);processor.setValue("rModel",n>=4?float(n-3):0.f);if(n<4)processor.setValue("rType",float(n));};
 setupCombo(rhythm,"rhythm",2,{"1/16","1/8 triplet","1/8","1/8 dotted","1/4","1/4 dotted","1/2","1/2 dotted","1 bar"});
 setupCombo(stereo,"stereo",3,{"Stereo","Dual","Ping-pong"});
 setupCombo(routing,"routing",4,{"Delay > Reverb","Parallel"});
 setupCombo(interval,"interval",5,{"-12 st","-7 st","+7 st","+12 st","+19 st","+24 st"});
 setupCombo(reverbInterval,"rInterval",6,{"-12 st","-7 st","+7 st","+12 st","+19 st","+24 st"});
 setupCombo(pitchMode,"pitchMode",7,{"Climbing repeats","Fixed harmony"});pitchMode.setTooltip("Climbing: each feedback pass shifts again. Fixed harmony: feedback stays at its original pitch, then the return is shifted. The wet pitch window adds latency; dry audio stays immediate.");
 setupCombo(pattern,"dPattern",8,{"2 taps","4 taps","8 taps"});setupCombo(dualMode,"dDual",9,{"Linked ratio","Independent A / B"});setupCombo(bRhythm,"dBRhythm",10,{"B: 1/16","B: 1/8 triplet","B: 1/8","B: dotted eighth","B: quarter","B: dotted quarter","B: half","B: dotted half","B: bar"});setupCombo(manualTempo,"manualTempo",11,{"Host tempo","Manual tempo"});
 delayType.setTooltip("Choose the delay processing model; this keeps your other controls.");reverbType.setTooltip("Choose reverb topology. Existing tank tails continue.");
 rhythm.setTooltip("Tempo division, used when SYNC is enabled.");stereo.setTooltip("Stereo preserves channels. Dual uses B/A timing ratio. Ping-pong crosses feedback.");routing.setTooltip("Serial feeds the stereo delay into reverb. Parallel feeds each independently.");interval.setTooltip("Pitch delay interval.");reverbInterval.setTooltip("Shimmer interval, independent of Pitch delay.");
 juce::TextButton* toggles[]={&delayOn,&reverbOn,&sync,&send,&hold};const char* ids[]={"dOn","rOn","sync","send","freeze"};
 for(int i=0;i<5;++i){show(*toggles[i]);toggles[i]->setClickingTogglesState(true);buttons[std::size_t(i)]=std::make_unique<BA>(p.state,ids[i],*toggles[i]);}
 delayOn.setTooltip("Stop new delay input; let existing repeats decay.");reverbOn.setTooltip("Stop new reverb input, including delay repeats; let the existing tail decay.");send.setTooltip("Use on a DAW return/aux: removes the dry signal. This setting survives factory preset changes.");hold.setTooltip("Hold the reverb tank and stop new excitation. Modulation can slowly soften the held sound.");
 for(auto* c:{&presetDelay,&presetReverb,&presetCombined})show(*c);
 for(int i=0;i<9;++i)for(int j=0;j<2;++j)presetDelay.addItem(juce::String(i<6?tide::delayNames[i]:tide::newDelayNames[i-6])+(j?" / Wide dotted":" / Clear quarter"),i*2+j+1);
 for(int i=0;i<11;++i)for(int j=0;j<2;++j)presetReverb.addItem(juce::String(i<4?tide::reverbNames[i]:tide::newReverbNames[i-4])+(j?" / Spacious":" / Intimate"),i*2+j+1);
 for(int pass=0;pass<2;++pass)for(int i=pass==0?16:0;i<(pass==0?tide::numPresets:16);++i)presetCombined.addItem(p.getProgramName(i)+(i<16?" [Legacy]":""),i+1);
 presetDelay.setText("Delay presets",juce::dontSendNotification);presetReverb.setText("Reverb presets",juce::dontSendNotification);
 presetCombined.setSelectedId(std::max(0,p.selectedProgram()+1),juce::dontSendNotification);
 presetDelay.onChange=[this]{if(presetDelay.getSelectedId()>0)processor.applyDelayPreset((presetDelay.getSelectedId()-1)/2,(presetDelay.getSelectedId()-1)%2!=0);};
 presetReverb.onChange=[this]{if(presetReverb.getSelectedId()>0)processor.applyReverbPreset((presetReverb.getSelectedId()-1)/2,(presetReverb.getSelectedId()-1)%2!=0);};
 presetCombined.onChange=[this]{if(presetCombined.getSelectedId()>0)processor.setCurrentProgram(presetCombined.getSelectedId()-1);};
 for(auto* b:{&editDelay,&editReverb,&editRoute,&clear,&saveDelay,&saveReverb,&saveCombined,&lockDelay,&lockReverb,&lockMix,&motion,&undo,&undoLoad,&tap,&lockRoute,&help})show(*b);
 editDelay.onClick=[this]{showEdit(editing==1?0:1);};editReverb.onClick=[this]{showEdit(editing==2?0:2);};editRoute.onClick=[this]{showEdit(editing==3?0:3);};
 clear.onClick=[this]{processor.clearRequested.store(true);};clear.setTooltip("Fade out, empty stored echoes and tails, then fade in. Does not change settings.");
 saveDelay.onClick=[this]{showBrowser(1);};saveReverb.onClick=[this]{showBrowser(2);};saveCombined.onClick=[this]{showBrowser(0);};saveCombined.setButtonText("BROWSE");saveDelay.setTooltip("Search delay presets, favorites and user sounds.");saveReverb.setTooltip("Search reverb presets, favorites and user sounds.");
 undoLoad.onClick=[this]{processor.undoLastLoad();};lockRoute.setClickingTogglesState(true);lockRoute.onClick=[this]{processor.lockRouting.store(lockRoute.getToggleState());};tap.onClick=[this]{const double now=juce::Time::getMillisecondCounterHiRes();if(lastTap>0&&now-lastTap>=250&&now-lastTap<=2000){processor.setValue("bpm",float(60000/(now-lastTap)));processor.setValue("manualTempo",1);processor.setValue("sync",1);}lastTap=now;};
 for(auto* b:{&lockDelay,&lockReverb,&lockMix,&motion})b->setClickingTogglesState(true);
 lockDelay.onClick=[this]{processor.lockDelay.store(lockDelay.getToggleState());};lockReverb.onClick=[this]{processor.lockReverb.store(lockReverb.getToggleState());};lockMix.onClick=[this]{processor.lockMix.store(lockMix.getToggleState());};
 lockDelay.setTooltip("Keep delay parameters when loading combined presets.");lockReverb.setTooltip("Keep reverb parameters when loading combined presets.");lockMix.setTooltip("Keep overall Mix and both Amount controls while browsing presets.");
 motion.onClick=[this]{const int mode=(processor.visualMode.load()+1)%3;processor.visualMode.store(mode);processor.reducedMotion.store(mode==0);timerCallback();};motion.setTooltip("Cycle STATIC / ACTIVITY / MOTION. Activity animates only while sound is present. Visuals never change the audio.");
 undo.setTooltip("First click stores A. Later clicks swap the current sound with the stored version.");
 undo.onClick=[this]{juce::MemoryBlock current;processor.getStateInformation(current);if(alternateReady)processor.setStateInformation(alternate.getData(),int(alternate.getSize()));alternate=current;alternateReady=true;undo.setButtonText("SWAP A / B");};
 help.onClick=[]{juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,"Wolf Tide / Quick guide","Grey = stereo delay. Blue = stereo reverb. Each side has its own presets and on/off button.\n\nBottom presets recall a complete sound. LOCK protects a side; LOCK MIX protects all three blend controls. SEND removes dry audio for return tracks.\n\nEDIT opens deeper controls. ROUTING adjusts what enters the reverb. Double-click resets a knob; click its value to type precisely.\n\nHOLD sustains the reverb; CLEAR TAILS deliberately empties both effects. DAW Stop can preserve tails only while the host keeps processing.\n\nFactory sounds are designed starting points, not measured copies of other products. Version 1.2.0.");};
 show(status);show(tempoLabel);status.setFont(juce::FontOptions(11));tempoLabel.setFont(juce::FontOptions(11));tempoLabel.setJustificationType(juce::Justification::centredRight);
 setSize(1080,720);resized();timerCallback();if(isShowing())startTimerHz(12);
}
TideEditor::~TideEditor(){processor.editorVisible.store(false);stopTimer();setLookAndFeel(nullptr);}
void TideEditor::visibilityChanged(){const bool showing=isShowing();processor.editorVisible.store(showing);if(showing){timerCallback();startTimerHz(12);}else stopTimer();}
void TideEditor::showEdit(int which){editing=which;const int h=which?1130:720;setSize(getWidth(),juce::roundToInt(float(getWidth())*float(h)/1080));resized();repaint();}
void TideEditor::fileMenu(int scope){juce::PopupMenu menu;menu.addItem(1,"Save "+juce::String(scope==0?"combined":scope==1?"delay":"reverb")+" preset...");menu.addItem(2,"Load preset...");
 menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(scope==0?&saveCombined:scope==1?&saveDelay:&saveReverb),[safe=juce::Component::SafePointer<TideEditor>(this),scope](int id){if(safe&&id>0)safe->chooseFile(scope,id==1);});}
void TideEditor::chooseFile(int scope,bool save){
 const auto folder=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Wolf Audio Presets");folder.createDirectory();
 chooser=std::make_unique<juce::FileChooser>(save?"Save Wolf Tide preset":"Load Wolf Tide preset",folder.getChildFile(scope==0?"My Tide.wolftide":scope==1?"My Delay.wolftide":"My Reverb.wolftide"),"*.wolftide");
 chooser->launchAsync((save?juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::warnAboutOverwriting:juce::FileBrowserComponent::openMode)|juce::FileBrowserComponent::canSelectFiles,
  [safe=juce::Component::SafePointer<TideEditor>(this),scope,save](const juce::FileChooser& fc){if(!safe)return;auto file=fc.getResult();if(file==juce::File{})return;
   auto result=save?safe->processor.savePreset(file.withFileExtension("wolftide"),scope):safe->processor.loadPreset(file,scope);
   if(result.failed())juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Preset",result.getErrorMessage());});
}
void TideEditor::timerCallback(){
 const float oldD=dLevel,oldR=rLevel,oldO=oLevel;
 dLevel=std::max(processor.delayMeter.exchange(0),dLevel*.80f);rLevel=std::max(processor.reverbMeter.exchange(0),rLevel*.86f);oLevel=std::max(processor.outputMeter.exchange(0),oLevel*.84f);
 const int visualMode=processor.visualMode.load();const bool animate=visualMode==2||(visualMode==1&&std::max(dLevel,rLevel)>.0001f);
 if(animate)phase+=.025f;
 if(isShowing()&&getTimerInterval()!=(animate?1000/24:1000/12))startTimerHz(animate?24:12);
 delayOn.setButtonText(processor.value("dOn")>.5f?(processor.delayRunning.load()?"ON":"READY"):dLevel>.0001f?"TAIL":"OFF");reverbOn.setButtonText(processor.value("freeze")>.5f?"HOLD":processor.value("rOn")>.5f?(processor.reverbRunning.load()?"ON":"READY"):rLevel>.0001f?"TAIL":"OFF");
 lockDelay.setToggleState(processor.lockDelay.load(),juce::dontSendNotification);lockReverb.setToggleState(processor.lockReverb.load(),juce::dontSendNotification);lockMix.setToggleState(processor.lockMix.load(),juce::dontSendNotification);
 motion.setButtonText(visualMode==0?"STATIC":visualMode==1?"ACTIVITY":"MOTION");motion.setToggleState(visualMode!=0,juce::dontSendNotification);
 const bool synced=processor.value("sync")>.5f;knobs[0].setEnabled(!synced);if(synced&&tempoLabel.getText()!=juce::String(processor.tempo.load(),1)+" BPM / "+(processor.tempoFromHost.load()?"HOST":"LOCAL"))knobs[0].updateText();rhythm.setEnabled(synced);knobs[21].setEnabled(processor.value("routing")<.5f);
 delayType.setSelectedId(processor.value("dModel")>0?int(processor.value("dModel"))+6:int(processor.value("dType"))+1,juce::dontSendNotification);
 reverbType.setSelectedId(processor.value("rModel")>0?int(processor.value("rModel"))+4:int(processor.value("rType"))+1,juce::dontSendNotification);
 lockRoute.setToggleState(processor.lockRouting.load(),juce::dontSendNotification);undoLoad.setEnabled(processor.undoReady);
 const int mode=processor.value("dModel")>0?int(processor.value("dModel"))+5:int(processor.value("dType"));knobs[6].setEnabled(mode==1||mode==2);knobs[10].setEnabled(processor.value("stereo")>.5f&&processor.value("stereo")<1.5f);
 pitchMode.setEnabled(mode==5);interval.setEnabled(mode==5||mode==7);
 knobs[15].setEnabled(processor.value("rType")>=1.5f);knobs[23].setEnabled(processor.value("send")<.5f);
 tempoLabel.setText(juce::String(processor.tempo.load(),1)+" BPM / "+(processor.tempoFromHost.load()?"HOST":"LOCAL"),juce::dontSendNotification);
 const int program=processor.selectedProgram();const auto title=program>=0?processor.getProgramName(program):processor.loadedScope==0&&processor.loadedName.isNotEmpty()?processor.loadedName:juce::String("Custom combination");
 if(presetCombined.getText()!=title)presetCombined.setText(title,juce::dontSendNotification);
 status.setText(oLevel>=1?"OUTPUT ABOVE 0 dBFS / LOWER OUTPUT IN ROUTING":"STEREO IN / STEREO OUT     |     "+juce::String(processor.factoryMatches()?"FACTORY PRESET":(processor.lockDelay.load()||processor.lockReverb.load()||processor.lockMix.load()||processor.lockRouting.load())?"PARTIAL / LOCKS ACTIVE":"EDITED / CUSTOM SOUND"),juce::dontSendNotification);
 status.setColour(juce::Label::textColourId,oLevel>=1?juce::Colour(0xffffbc7a):muted);
 const std::array<float,8> next{processor.value("dFeedback"),processor.value("dMotion"),processor.value("rMotion"),processor.value("shimmer"),processor.value("feed"),processor.value("dOn"),processor.value("rOn"),float(visualMode)};
 const bool changed=next!=visualValues;visualValues=next;
 const float sx=float(getWidth())/1080,sy=float(getHeight())/(editing?1130.f:720.f);auto region=[&](int x,int y,int w,int h){repaint(juce::Rectangle<int>(int(float(x)*sx),int(float(y)*sy),int(float(w)*sx)+2,int(float(h)*sy)+2));};
 if(changed||(visualMode!=0&&((visualMode==2||dLevel>.0001f)||std::abs(oldD-dLevel)>.0001f)))region(42,235,474,145);
 if(changed||(visualMode!=0&&((visualMode==2||rLevel>.0001f)||std::abs(oldR-rLevel)>.0001f)))region(565,235,474,145);
 if(changed)region(530,290,20,23);
 if(std::abs(oldO-oLevel)>.001f)region(960,698,80,8);
}
void TideEditor::drawEchoes(juce::Graphics& g){
 const float feedback=processor.value("dFeedback"),activity=processor.visualMode.load()==0?0.f:std::min(1.f,dLevel*5);
 g.saveState();g.reduceClipRegion(juce::Rectangle<int>(42,235,472,145));
 for(int row=0;row<2;++row){const float cy=273.f+float(row)*62;g.setColour(silver.withAlpha(.12f));g.drawHorizontalLine(int(cy),54,502);
  for(int i=0;i<8;++i){const float strength=std::pow(std::max(.1f,feedback),float(i)*.45f);const float x=67+float(i)*58;
   const float radius=2+strength*(7+activity*5);const float y=cy+std::sin(phase+float(i)*.8f+float(row))*processor.value("dMotion")*13;
   g.setColour(silver.withAlpha(.04f+activity*.10f));g.fillEllipse(x-radius*2,y-radius*2,radius*4,radius*4);
   g.setColour(silver.withAlpha(.15f+strength*.6f));g.fillEllipse(x-radius,y-radius,radius*2,radius*2);
  }}
 label(g,"L",46,250,20,20,10,muted);label(g,"R",46,312,20,20,10,muted);g.restoreState();
}
void TideEditor::drawOcean(juce::Graphics& g){
 g.saveState();g.reduceClipRegion(juce::Rectangle<int>(565,235,472,145));
 const float activity=processor.visualMode.load()==0?0.f:std::min(1.f,rLevel*6),movement=processor.value("rMotion"),shimmer=processor.value("shimmer");
 for(int j=0;j<9;++j){juce::Path wave;for(int i=0;i<=100;++i){const float x=566+float(i)*4.7f,t=float(i)*.06f;
   const float envelope=std::sin(float(i)/100*tide::pi);const float y=256+float(j)*12+envelope*(std::sin(t*1.8f+phase*(.4f+movement)+float(j)*.4f)*12+std::sin(t*3-phase*.4f)*6)*(0.4f+activity);
   if(i==0)wave.startNewSubPath(x,y);else wave.lineTo(x,y);}
  g.setColour(blue.withAlpha(.12f+float(j)*.035f+activity*.12f));g.strokePath(wave,juce::PathStrokeType(1.2f));}
 if(shimmer>.01f){g.setColour(juce::Colour(0xffd5b7ff).withAlpha(shimmer*.8f));for(int i=0;i<12;++i){const float x=590+float(i)*37,y=248+std::sin(float(i)*2.3f+phase*.3f)*11;g.fillEllipse(x,y,2.5f,2.5f);}}
 g.restoreState();
}
void TideEditor::buildBackdrop(){
 const int h=editing?1130:720;const float scale=std::clamp(2.f*float(getWidth())/1080,1.f,3.f);backdrop=juce::Image(juce::Image::RGB,juce::roundToInt(1080*scale),juce::roundToInt(float(h)*scale),true);juce::Graphics g(backdrop);g.addTransform(juce::AffineTransform::scale(scale));g.fillAll(bg);
 // Printed artwork is generated once per layout, never per animation frame.
 juce::Random grain(0x574f4c46);for(int i=0;i<18000;++i){g.setColour(ink.withAlpha(.018f));g.fillRect(grain.nextInt(1080),grain.nextInt(h),1,1);}
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff40464a),24,105,juce::Colour(0xff252c31),534,570,false));g.fillRoundedRectangle(24,105,510,465,12);
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff167485),546,105,juce::Colour(0xff103b4b),1056,570,false));g.fillRoundedRectangle(546,105,510,465,12);
 for(int side=0;side<2;++side){g.saveState();g.reduceClipRegion(juce::Rectangle<int>(side?547:25,106,508,463));for(int j=0;j<13;++j){juce::Path contour;for(int i=0;i<90;++i){float x=float((side?550:24)+i*6),y=123.f+float(j)*37+12*std::sin(float(i)*.12f+float(j)*.43f);if(i==0)contour.startNewSubPath(x,y);else contour.lineTo(x,y);}g.setColour(ink.withAlpha(.025f));g.strokePath(contour,juce::PathStrokeType(.7f));}g.restoreState();}
 // Recessed displays and a restrained printed faceplate are cached with the artwork.
 g.setColour(juce::Colour(0xff0a171d));g.fillRoundedRectangle(42,233,474,145,8);g.fillRoundedRectangle(564,233,474,145,8);
 g.setColour(ink.withAlpha(.18f));g.drawRoundedRectangle(42,233,474,145,8,1);g.drawRoundedRectangle(564,233,474,145,8,1);
 label(g,"T A P E   /   T I M E   /   T E X T U R E",57,244,390,16,9,muted);label(g,"S P A C E   /   D E P T H   /   L I G H T",580,244,400,16,9,blue);
 g.setColour(juce::Colour(0xffbf926c).withAlpha(.45f));g.drawRoundedRectangle(24,105,510,465,16,.8f);g.drawRoundedRectangle(546,105,510,465,16,.8f);
 label(g,"W O L F   A U D I O",28,17,220,20,11,muted);g.setColour(ink);g.setFont(juce::FontOptions("Georgia",51,juce::Font::italic));g.drawText("Tide",25,34,160,54,juce::Justification::centredLeft);
 label(g,"E C H O   /   A T M O S P H E R E",165,55,370,20,10,muted);
 g.setColour(juce::Colour(0xffbf926c));g.drawEllipse(570,38,35,35,1.1f);g.drawLine(588,32,588,79,.7f);g.drawLine(566,56,609,56,.7f);
 label(g,"01  /  DELAY",43,121,220,24,17,silver);label(g,"02  /  REVERB",565,121,230,24,17,blue);
 label(g,"CHARACTER",45,189,90,18,10,muted);label(g,"CHARACTER",567,189,90,18,10,muted);
 g.setColour(juce::Colour(0xff293c47));g.fillRoundedRectangle(24,585,1032,105,12);g.setColour(juce::Colour(0xffbf926c).withAlpha(.4f));g.drawRoundedRectangle(24,585,1032,105,12,.7f);
 label(g,"COMBINED PRESET",43,596,180,20,10,muted);label(g,"MIX",846,595,70,20,10,muted);label(g,"BLOOM",953,595,80,20,10,blue);
 if(editing){g.setColour(juce::Colour(0xff21323b));g.fillRoundedRectangle(24,704,1032,397,12);label(g,editing==1?"DELAY / DEEP EDIT":editing==2?"REVERB / DEEP EDIT":"ROUTING / MIX & OUTPUT",43,713,500,24,12,editing==1?silver:blue);}
}
void TideEditor::paint(juce::Graphics& g){
 const float baseH=editing?1130.f:720.f;g.addTransform(juce::AffineTransform::scale(float(getWidth())/1080,float(getHeight())/baseH));
 if(backdrop.isValid())g.drawImage(backdrop,0,0,1080,int(baseH),0,0,backdrop.getWidth(),backdrop.getHeight());
 if(g.clipRegionIntersects(juce::Rectangle<int>(42,235,474,145)))drawEchoes(g);
 if(g.clipRegionIntersects(juce::Rectangle<int>(565,235,474,145)))drawOcean(g);
 g.setColour(juce::Colour(0xffbf926c));g.drawLine(531,301,548,301,2);g.setColour(blue.withAlpha(.3f+processor.value("feed")*.7f));g.fillEllipse(536,297,8,8);
 g.setColour(juce::Colour(0xff354758));g.fillRoundedRectangle(961,699,77,4,2);g.setColour(oLevel>=1?juce::Colour(0xffffbc7a):blue);g.fillRoundedRectangle(961,699,77*std::min(1.f,oLevel),4,2);
}
void TideEditor::resized(){
 buildBackdrop();
 const float sx=float(getWidth())/1080,sy=float(getHeight())/(editing?1130.f:720.f);
 auto rect=[&](int x,int y,int w,int h){return juce::Rectangle<int>(juce::roundToInt(float(x)*sx),juce::roundToInt(float(y)*sy),juce::roundToInt(float(w)*sx),juce::roundToInt(float(h)*sy));};
 for(int i=0;i<tide::numFloats;++i){knobs[std::size_t(i)].setVisible(false);labels[std::size_t(i)].setVisible(false);}
 auto knob=[&](int index,int x,int y,int w,int h,bool linear=false){auto& k=knobs[std::size_t(index)];auto& l=labels[std::size_t(index)];k.setVisible(true);l.setVisible(true);k.setSliderStyle(linear?juce::Slider::LinearHorizontal:juce::Slider::RotaryHorizontalVerticalDrag);l.setBounds(rect(x,y,w,19));k.setBounds(rect(x,y+20,w,h-20));};
 for(int i=0;i<8;++i){const bool right=i>=4;const int x=(right?565:43)+(i%4)*119;knob(mainIndices[i],x,390,112,140);}
 delayOn.setBounds(rect(438,118,76,28));reverbOn.setBounds(rect(960,118,76,28));
 presetDelay.setBounds(rect(43,157,365,28));saveDelay.setBounds(rect(414,157,42,28));lockDelay.setBounds(rect(462,157,52,28));
 presetReverb.setBounds(rect(565,157,365,28));saveReverb.setBounds(rect(936,157,42,28));lockReverb.setBounds(rect(984,157,52,28));
 delayType.setBounds(rect(137,188,180,29));reverbType.setBounds(rect(659,188,180,29));
 sync.setBounds(rect(324,188,64,29));rhythm.setBounds(rect(394,188,120,29));hold.setBounds(rect(960,188,76,29));
 editDelay.setBounds(rect(43,531,155,26));editReverb.setBounds(rect(565,531,155,26));
 presetCombined.setBounds(rect(43,620,376,34));saveCombined.setBounds(rect(428,620,108,34));
 editRoute.setBounds(rect(546,620,94,34));send.setBounds(rect(649,620,66,34));lockMix.setBounds(rect(725,620,92,34));
 knob(23,828,613,105,73,true);knob(25,938,613,101,73,true);labels[23].setVisible(false);labels[25].setVisible(false);
 clear.setBounds(rect(678,42,115,29));undo.setBounds(rect(804,42,105,29));undoLoad.setBounds(rect(678,79,115,24));motion.setBounds(rect(918,42,87,29));help.setBounds(rect(1014,42,40,29));
 status.setBounds(rect(31,696,780,20));tempoLabel.setBounds(rect(771,79,274,19));
 stereo.setVisible(editing==1);interval.setVisible(editing==1);pitchMode.setVisible(editing==1);reverbInterval.setVisible(editing==2);routing.setVisible(editing==3);pattern.setVisible(editing==1);dualMode.setVisible(editing==1);bRhythm.setVisible(editing==1);manualTempo.setVisible(editing==3);tap.setVisible(editing==3);lockRoute.setVisible(editing==3);
 if(editing==1){pattern.setBounds(rect(43,907,150,27));dualMode.setBounds(rect(209,907,190,27));bRhythm.setBounds(rect(416,907,190,27));constexpr int extended[]={31,32,33,34};for(int i=0;i<4;++i)knob(extended[i],43+i*245,951,180,132);pitchMode.setBounds(rect(500,713,240,27));stereo.setBounds(rect(755,713,135,27));interval.setBounds(rect(900,713,135,27));constexpr int ids[]={4,5,6,7,8,9,10,27,28};for(int i=0;i<9;++i)knob(ids[i],43+i*110,752,101,132);}
 if(editing==2){constexpr int extended[]={35,36,37,38};for(int i=0;i<4;++i)knob(extended[i],43+i*245,951,180,132);reverbInterval.setBounds(rect(900,713,135,27));constexpr int ids[]={15,16,17,18,19,20,29,30};for(int i=0;i<8;++i)knob(ids[i],43+i*124,752,115,132);}
 if(editing==3){manualTempo.setBounds(rect(43,914,190,30));tap.setBounds(rect(245,914,80,30));lockRoute.setBounds(rect(340,914,120,30));routing.setBounds(rect(774,713,261,27));constexpr int ids[]={21,22,24,26};for(int i=0;i<4;++i)knob(ids[i],80+i*245,752,180,132);}
}

void TideEditor::showBrowser(int scope){auto safe=juce::Component::SafePointer<TideEditor>(this);auto browser=std::make_unique<TidePresetBrowser>(processor,scope,[safe,scope]{if(safe)safe->fileMenu(scope);});juce::CallOutBox::launchAsynchronously(std::move(browser),(scope==0?saveCombined:scope==1?saveDelay:saveReverb).getScreenBounds(),nullptr);}
