// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "PluginProcessor.h"
#include "TideLook.h"
// Native, scrollable advanced editing. Attachments share the same APVTS as host automation.
class TideModernPanel final:public juce::Component,private juce::Timer {
 TideProcessor& processor;TideLook theme;juce::ComboBox page;juce::Label heading;juce::Viewport viewport;juce::Component content;
 std::vector<std::unique_ptr<juce::Component>> controls;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> combos;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
 int selected=-1,model=-1;bool queued=false;
 void queueRebuild(){if(queued)return;queued=true;juce::MessageManager::callAsync([safe=juce::Component::SafePointer<TideModernPanel>(this)]{if(safe){safe->queued=false;safe->rebuild();}});}
 void timerCallback()override{const int tab=page.getSelectedId()-1;const int next=tab<4?int(processor.value(tab==0?"d13Model":tab==1?"d2Model":tab==2?"r13Model":"r2Model")):-1;if(next!=model||selected!=tab)queueRebuild();else refreshEnabled();}
 void refreshEnabled(){
  const bool familyReady=selected>=4||processor.value(selected<2?"dRevision":"rRevision")>.5f;
  for(const auto& control:controls){const auto id=control->getName();if(id.isEmpty())continue;bool enabled=familyReady||id=="dRevision"||id=="rRevision";
   const bool sync=processor.value(selected==1?"d2Sync":"sync")>.5f;const bool independent=processor.value("dDual")>.5f,dual=processor.value("stereo")>.5f&&processor.value("stereo")<1.5f;
   if(id=="dTime"||id=="d2Time")enabled&=!sync;if(id=="rhythm"||id=="d2Rhythm")enabled&=sync;
   if(id=="dDual")enabled&=dual;if(id=="ratio")enabled&=dual&&!independent;if(id=="dBTime")enabled&=dual&&independent&&!sync;if(id=="dBRhythm")enabled&=dual&&independent&&sync;if(id=="dBFeedback"||id=="dBLevel")enabled&=dual&&independent;if(id=="d2Ratio")enabled&=int(processor.value("d2Stereo"))==1;
   if(id=="splitHz")enabled&=processor.value("routing13")>1.5f;if(id=="feed")enabled&=int(processor.value("routing13"))!=1;
   if(id=="crossFeedback")enabled&=(processor.value("dRevision")>.5f&&processor.value("dOn")>.5f&&processor.value("d2On")>.5f)||(processor.value("rRevision")>.5f&&processor.value("rOn")>.5f&&processor.value("r2On")>.5f);
   control->setEnabled(enabled);
  }
 }
 void rebuild(){
  sliders.clear();combos.clear();buttons.clear();controls.clear();selected=page.getSelectedId()-1;const bool delay=selected<2;
  model=selected<4?int(processor.value(selected==0?"d13Model":selected==1?"d2Model":selected==2?"r13Model":"r2Model")):-1;
  std::vector<juce::String> ids;
  if(selected==0)ids={"dRevision","d13Model","dOn","sync","rhythm","dTime","dFeedback","dAmount","dTone","dLowcut","stereo","dWidth","dMotion","dRate","drive","interval","pitchMode","dDual","ratio","dBTime","dBRhythm","dBFeedback","dBLevel","dDuck","dSense","dRelease","dDiffusion","dFreeze"};
  if(selected==1)ids={"dRevision","d2Model","d2On","d2Sync","d2Rhythm","d2Time","d2Feedback","d2Amount","d2Tone","d2Lowcut","d2Stereo","d2Ratio","d2Width","d2Motion","d2Rate","d2Drive","d2Interval","d2PitchMode","d2Diffusion","d2Freeze"};
  if(selected==2)ids={"rRevision","r13Model","rOn","freeze","rDecay","rAmount","rPredelay","rTone","rLowcut","bass","rWidth","rMotion","rDiffusion","rSize","rAttack","shimmer","rInterval","rRegeneration","rDuck","rSense","rRelease"};
  if(selected==3)ids={"rRevision","r2Model","r2On","r2Freeze","r2Decay","r2Amount","r2Predelay","r2Tone","r2Lowcut","r2Bass","r2Width","r2Motion","r2Diffusion","r2Size","r2Attack","r2Shimmer","r2Interval","r2Regeneration"};
  if(selected==4)ids={"routing13","splitHz","crossFeedback","feed","direct","mix","output","bloom","send","manualTempo","bpm"};
  for(const auto& row:tide::v13::modernSpecs){const juce::String id(row.spec.id);const bool second=selected==1||selected==3;const juce::String prefix=delay?(second?"d2":"d"):(second?"r2":"r");if(selected<4&&id.startsWith(prefix)&&!(second==false&&(id.startsWith("d2")||id.startsWith("r2")))&&std::find(ids.begin(),ids.end(),id)==ids.end())ids.push_back(id);}
  int row=0;
  auto add=[&](std::unique_ptr<juce::Component> control,int x,int w){control->setBounds(x,12+row*48,w,34);content.addAndMakeVisible(*control);controls.push_back(std::move(control));};
  for(const auto& id:ids){auto* parameter=processor.state.getParameter(id);if(!parameter)continue;
   if(selected<4&&!tide::v13::applicable(id.toStdString(),model,delay))continue;
   if(selected==0&&id=="dDiffusion"&&model!=6)continue;if(selected==1&&id=="d2Diffusion"&&model!=6)continue;
   if((id=="dFreeze"||id=="d2Freeze")&&model!=7)continue;
   if(id=="pitchMode"&&model!=5)continue;if(id=="d2PitchMode"&&model!=5)continue;
   auto text=std::make_unique<juce::Label>();text->setText(parameter->getName(64)+(parameter->getLabel().isNotEmpty()?" ("+parameter->getLabel()+")":""),juce::dontSendNotification);text->setColour(juce::Label::textColourId,tide::ui::ink);text->setFont(tide::ui::font(13));add(std::move(text),10,300);
   const bool isModel=id=="d13Model"||id=="r13Model"||id=="d2Model"||id=="r2Model";
   if(auto* choice=dynamic_cast<juce::AudioParameterChoice*>(parameter)){auto box=std::make_unique<juce::ComboBox>();auto* raw=box.get();raw->setName(id);raw->addItemList(choice->choices,1);add(std::move(box),320,480);combos.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.state,id,*raw));if(isModel)raw->onChange=[this]{queueRebuild();};}
   else if(dynamic_cast<juce::AudioParameterBool*>(parameter)){auto button=std::make_unique<juce::ToggleButton>("Enabled");auto* raw=button.get();raw->setName(id);raw->setClickingTogglesState(true);add(std::move(button),320,180);buttons.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state,id,*raw));}
   else{auto slider=std::make_unique<juce::Slider>();auto* raw=slider.get();raw->setName(id);raw->setSliderStyle(juce::Slider::LinearHorizontal);raw->setTextBoxStyle(juce::Slider::TextBoxRight,false,110,30);raw->setTextValueSuffix(" "+parameter->getLabel());raw->setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));add(std::move(slider),320,480);sliders.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,*raw));}
   ++row;
  }
  content.setSize(820,std::max(480,row*48+24));viewport.setViewPosition(0,0);refreshEnabled();
 }
public:
 TideModernPanel(TideProcessor& p,int tab):processor(p){
  setLookAndFeel(&theme);addAndMakeVisible(page);addAndMakeVisible(heading);addAndMakeVisible(viewport);viewport.setViewedComponent(&content,false);heading.setText("TIDE 1.3 / INDEPENDENT MACHINES",juce::dontSendNotification);heading.setColour(juce::Label::textColourId,tide::ui::ink);heading.setFont(tide::ui::font(17,juce::Font::bold));page.addItemList({"Delay A","Delay B","Reverb A","Reverb B","Routing / global"},1);page.setSelectedId(tab+1,juce::dontSendNotification);page.onChange=[this]{queueRebuild();};setSize(850,600);rebuild();if(isShowing())startTimerHz(8);}
 ~TideModernPanel()override{setLookAndFeel(nullptr);stopTimer();viewport.setViewedComponent(nullptr,false);}
 void visibilityChanged()override{if(isShowing()){refreshEnabled();startTimerHz(8);}else stopTimer();}
 void paint(juce::Graphics& g)override{tide::ui::paintSkin(g,getLocalBounds().toFloat(),tide::ui::Skin::body);}
 void resized()override{heading.setBounds(18,12,800,28);page.setBounds(18,48,800,30);viewport.setBounds(8,90,getWidth()-16,getHeight()-98);}
};
