// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "PluginProcessor.h"
class PrismLook:public juce::LookAndFeel_V4 {
public:PrismLook();void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;
};
class PrismEditor final:public juce::AudioProcessorEditor,private juce::Timer {
 PrismProcessor& p;PrismLook look;juce::TooltipWindow tooltip{this,500};
 std::array<juce::Slider,8> knobs;std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,8> attachments;
 std::array<juce::TextButton,20> effect;std::array<juce::TextButton,4> on;
 juce::ComboBox presets,user,routing,filter,speed;juce::TextButton secondary{"SECONDARY"},setup{"FX SETUP"},gesture{"GESTURE REC"},gesturePlay{"GESTURE PLAY"},gestureClear{"CLEAR GESTURE"},saveUser{"SAVE USER"},loadUser{"LOAD USER"},save{"EXPORT BANK"},load{"IMPORT BANK"},capture{"RECORD"},play{"PLAY"},stop{"STOP"},erase{"ERASE"},dub{"DUB"},undo{"UNDO"},tap{"TAP"},bypass{"ENGAGED"};
 juce::ToggleButton sync{"Tempo Sync"},hostSync{"Host Tempo"},trails{"Bypass Trails"},pre{"Capture Pre-FX"},reverse{"Capture Reverse"},dual{"Dual Bypass"};
 std::array<juce::ToggleButton,4> masks;std::array<juce::Slider,3> extras;std::array<juce::Label,3> extraLabels;std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,3> extraAttach;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> combos;
 juce::Label status,message;std::unique_ptr<juce::FileChooser> chooser;bool shifted=false,expanded=false,updating=false;float meter=0;
 void attach();void timerCallback()override;void file(bool);void toggle(const char*);
public:explicit PrismEditor(PrismProcessor&);~PrismEditor()override;void paint(juce::Graphics&)override;void resized()override;
};
