// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "PrismDSP.h"
class PrismProcessor final:public juce::AudioProcessor {
public:
 PrismProcessor();
 const juce::String getName()const override{return "Wolf Prism";}
 void prepareToPlay(double,int)override;void releaseResources()override{};void reset()override{}
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 using AudioProcessor::processBlock;using AudioProcessor::processBlockBypassed;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 juce::AudioProcessorEditor* createEditor()override;bool hasEditor()const override{return true;}
 bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}
 double getTailLengthSeconds()const override{return 240;}
 int getNumPrograms()override{return 96;}int getCurrentProgram()override{return program.load();}
 void setCurrentProgram(int)override;const juce::String getProgramName(int)override;void changeProgramName(int,const juce::String&)override;
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 float value(const char*)const;void setValue(const char*,float);bool queue(prism::Action);void tap();void saveSlot(int);void loadSlot(int);
 juce::Result saveFile(const juce::File&);juce::Result loadFile(const juce::File&);bool restore(const void*,int);
 juce::AudioProcessorValueTreeState state;prism::Engine engine;
 std::atomic<float> tempo{120},loopSeconds{0},meter{0};std::atomic<int> loopStatus{0};std::atomic<bool> visible{false};
 static constexpr const char* types[]={"characterType","movementType","diffusionType","textureType"};
 static constexpr const char* enabled[]={"characterOn","movementOn","diffusionOn","textureOn"};
private:
 struct Slot{juce::ValueTree params;prism::Gesture gesture;juce::String name;};
 std::array<std::unique_ptr<Slot>,80> slots;
 std::array<std::atomic<float>*,prism::FloatCount> fp{};std::array<std::atomic<float>*,4> tp{},ep{};std::array<std::atomic<float>*,13> aux{};
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();prism::Settings settings()const;
 juce::AbstractFifo fifo{128};std::array<prism::Action,128> actions{};std::atomic<int> program{0};double lastTap=0;
 prism::LoopImage pendingLoop;bool pending=false,prepared=false;double preparedRate=0;
 void render(juce::AudioBuffer<float>&,juce::MidiBuffer&,bool);void applyMidi(const juce::MidiMessage&);
 void factory(int);bool validTree(const juce::ValueTree&)const;void merge(const juce::ValueTree&);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PrismProcessor)
};
