// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
class TideProcessor final: public juce::AudioProcessor {
public:
    TideProcessor();
    using AudioProcessor::processBlock;using AudioProcessor::processBlockBypassed;
    const juce::String getName()const override{return "Wolf Tide";}
    void prepareToPlay(double,int)override;
    void releaseResources()override{}
    // Keep tails through transport reset; explicit CLEAR is available in the editor.
    bool isBusesLayoutSupported(const BusesLayout&)const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
    juce::AudioProcessorEditor* createEditor()override;
    bool hasEditor()const override{return true;}
    bool acceptsMidi()const override{return false;}
    bool producesMidi()const override{return false;}
    bool isMidiEffect()const override{return false;}
    double getTailLengthSeconds()const override{return 120;}
    int getNumPrograms()override{return tide::numPresets;}
    int getCurrentProgram()override{return std::max(0,program.load());}
    int selectedProgram()const{return program.load();}
    void setCurrentProgram(int)override;
    const juce::String getProgramName(int)override;
    void changeProgramName(int,const juce::String&)override{}
    void getStateInformation(juce::MemoryBlock&)override;
    void setStateInformation(const void*,int)override;
    void applyDelayPreset(int);
    void applyReverbPreset(int);
    void setValue(const juce::String&,float);
    float value(const juce::String&)const;
    juce::Result savePreset(const juce::File&,int scope);
    juce::Result loadPreset(const juce::File&,int scope);
    bool factoryMatches()const;
    juce::AudioProcessorValueTreeState state;
    std::atomic<float> delayMeter{0},reverbMeter{0},outputMeter{0},tempo{120};
    std::atomic<bool> tempoFromHost{false},lockDelay{false},lockReverb{false},lockMix{false},reducedMotion{false};
    std::atomic<bool> clearRequested{false};
    std::atomic<bool> editorVisible{false},delayRunning{false},reverbRunning{false},shimmerRunning{false};
    std::atomic<int> visualMode{1}; // 0 static, 1 activity, 2 continuous
    std::atomic<int> delayPreset{-1},reverbPreset{-1};
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    tide::Parameters read()const;
    void render(juce::AudioBuffer<float>&,bool);
    void applyFactory(int,bool respectLocks);
    tide::Engine engine;
    std::array<std::atomic<float>*,tide::numFloats> floats{};
    std::array<std::atomic<float>*,13> choices{};
    std::atomic<int> program{0};
    float clearFade=1;bool clearing=false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TideProcessor)
};
