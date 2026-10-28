// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "PluginProcessor.h"
class TideLook final:public juce::LookAndFeel_V4 {
public:
 TideLook();
 void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;
 void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool)override;
};
class TideEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit TideEditor(TideProcessor&);
 ~TideEditor()override;
 void paint(juce::Graphics&)override;
 void resized()override;
 void visibilityChanged()override;
 void refreshSelectors();
private:
 void timerCallback()override;
 void showEdit(int);
 void fileMenu(int);
 void chooseFile(int,bool);
 void drawEchoes(juce::Graphics&);
 void drawOcean(juce::Graphics&);
 void buildBackdrop();void showBrowser(int);
 TideProcessor& processor;TideLook look;
 std::array<juce::Slider,tide::numFloats> knobs;
 std::array<juce::Label,tide::numFloats> labels;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,tide::numFloats> sliders;
 juce::ComboBox delayType,reverbType,rhythm,stereo,routing,interval,reverbInterval,pitchMode,pattern,dualMode,bRhythm,manualTempo,presetDelay,presetReverb,presetCombined;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>,12> combos;
 juce::TextButton delayOn{"ON"},reverbOn{"ON"},sync{"SYNC"},send{"SEND"},hold{"HOLD"};
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>,5> buttons;
 juce::TextButton editDelay{"EDIT DELAY"},editReverb{"EDIT REVERB"},editRoute{"ROUTING"},clear{"CLEAR TAILS"};
 juce::TextButton saveDelay{"..."},saveReverb{"..."},saveCombined{"SAVE / LOAD"};
 juce::TextButton lockDelay{"LOCK"},lockReverb{"LOCK"},lockMix{"LOCK MIX"},motion{"MOTION"};
 juce::TextButton undo{"A / B"},undoLoad{"UNDO LOAD"},tap{"TAP"},lockRoute{"LOCK ROUTE"},help{"?"};
 double lastTap=0;
 juce::Label status,tempoLabel,buildLabel;
 juce::TooltipWindow tips{this,650};
 std::unique_ptr<juce::FileChooser> chooser;juce::MemoryBlock alternate;
 int editing=0;float phase=0,dLevel=0,rLevel=0,oLevel=0;bool alternateReady=false;
 juce::Image backdrop;std::array<float,8> visualValues{};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TideEditor)
};
