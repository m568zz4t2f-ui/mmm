// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PluginProcessor.h"
#include "PluginEditor.h"
namespace {
constexpr const char* choiceIds[]={"dType","rhythm","stereo","rType","routing","interval","dOn","rOn","sync","send","freeze","rInterval","pitchMode"};
// Side preset scope deliberately excludes shared routing, pitch interval and global balance.
bool inScope(const juce::String& id,int scope){
    if(scope==0)return true;
    if(scope==1)return (id.startsWith("d")&&id!="direct") || id=="drive"||id=="ratio"||id=="rhythm"||id=="stereo"||id=="sync"||id=="interval"||id=="pitchMode";
    return (id.startsWith("r")&&id!="routing")||id=="shimmer"||id=="bass";
}
}
juce::AudioProcessorValueTreeState::ParameterLayout TideProcessor::layout(){
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    for(const auto& s:tide::specs){const juce::String id(s.id);
        const juce::String hostName=(id.startsWith("d")&&id!="direct"?"Delay ":id.startsWith("r")?"Reverb ":id=="drive"||id=="ratio"?"Delay ":id=="shimmer"||id=="bass"?"Reverb ":"")+juce::String(s.name);
        auto attr=juce::AudioParameterFloatAttributes().withLabel(s.unit);
        if(juce::String(s.unit)=="%")attr=attr.withStringFromValueFunction([](float v,int){return juce::String(v*100,1);}).withValueFromStringFunction([](const juce::String& x){return x.getFloatValue()/100;});
        result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{s.id,1},hostName,juce::NormalisableRange<float>{s.lo,s.hi,s.step,s.skew},s.initial,attr));}
    auto choice=[&](const char* id,const char* name,juce::StringArray names,int initial){result.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},name,names,initial));};
    choice("dType","Delay engine",{"Digital","Tape","Bucket","Multitap","Reverse","Pitch"},0);
    choice("rhythm","Delay rhythm",{"1/16","1/8 triplet","1/8","1/8 dotted","1/4","1/4 dotted","1/2","1/2 dotted","1 bar"},3);
    choice("stereo","Delay stereo",{"Stereo","Dual","Ping-pong"},1);
    choice("rType","Reverb engine",{"Room","Plate","Hall","Bloom"},2);
    choice("routing","Routing",{"Delay > Reverb","Parallel"},0);
    choice("rInterval","Shimmer interval",{"-12 st","-7 st","+7 st","+12 st","+19 st","+24 st"},3);
    choice("interval","Delay pitch interval",{"-12 st","-7 st","+7 st","+12 st","+19 st","+24 st"},3);
    auto flag=[&](const char* id,const char* name,bool initial){result.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},name,initial));};
    flag("dOn","Delay active",true);flag("rOn","Reverb active",true);flag("sync","Tempo sync",true);flag("send","Send mode",false);flag("freeze","Hold reverb",false);
    choice("pitchMode","Delay pitch routing",{"Climbing repeats","Fixed harmony"},0);
    return result;
}
TideProcessor::TideProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"WolfTideState",layout()){
    for(int i=0;i<tide::numFloats;++i)floats[std::size_t(i)]=state.getRawParameterValue(tide::specs[i].id);
    for(int i=0;i<13;++i)choices[std::size_t(i)]=state.getRawParameterValue(choiceIds[i]);
    applyFactory(0,false);
}
float TideProcessor::value(const juce::String& id)const{if(auto* p=state.getRawParameterValue(id))return p->load();return 0;}
void TideProcessor::setValue(const juce::String& id,float v){if(auto* p=state.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}}
tide::Parameters TideProcessor::read()const{
    tide::Parameters p;auto f=[&](int i){return floats[std::size_t(i)]->load(std::memory_order_relaxed);};
    p.time=f(0);p.feedback=f(1);p.delayTone=f(2);p.delayAmount=f(3);p.delayMotion=f(4);p.delayWidth=f(5);p.drive=f(6);p.delayLowcut=f(7);p.delayDuck=f(8);p.delayRate=f(9);p.ratio=f(10);
    p.decay=f(11);p.predelay=f(12);p.reverbTone=f(13);p.reverbAmount=f(14);p.reverbMotion=f(15);p.reverbWidth=f(16);p.reverbLowcut=f(17);p.reverbDuck=f(18);p.shimmer=f(19);p.bassDecay=f(20);
    p.feed=f(21);p.direct=f(22);p.mix=f(23);p.output=f(24);p.bloom=f(25);p.bpm=f(26);p.delaySense=f(27);p.delayRelease=f(28);p.reverbSense=f(29);p.reverbRelease=f(30);
    auto c=[&](int i){return int(choices[std::size_t(i)]->load());};
    p.delayType=c(0);p.rhythm=c(1);p.stereo=c(2);p.reverbType=c(3);p.routing=c(4);p.interval=c(5);
    p.delayOn=c(6)!=0;p.reverbOn=c(7)!=0;p.sync=c(8)!=0;p.send=c(9)!=0;p.freeze=c(10)!=0;p.reverbInterval=c(11);p.pitchMode=c(12);return p;
}
void TideProcessor::prepareToPlay(double sr,int){engine.prepare(sr,read());clearFade=1;clearing=false;setLatencySamples(0);}
bool TideProcessor::isBusesLayoutSupported(const BusesLayout& b)const{
    const auto in=b.getMainInputChannelSet(),out=b.getMainOutputChannelSet();
    return (in==juce::AudioChannelSet::mono()&&(out==in||out==juce::AudioChannelSet::stereo()))||(in==juce::AudioChannelSet::stereo()&&out==in);
}
void TideProcessor::render(juce::AudioBuffer<float>& buffer,bool bypass){
    juce::ScopedNoDenormals noDenormals;auto settings=read();bool hostTempo=false;
    const bool collectMeters=editorVisible.load(std::memory_order_relaxed);
    if(auto* head=getPlayHead())if(auto pos=head->getPosition())if(auto bpm=pos->getBpm())if(std::isfinite(*bpm)&&*bpm>=20&&*bpm<=400){settings.bpm=float(*bpm);hostTempo=true;}
    tempo.store(settings.bpm);tempoFromHost.store(hostTempo);engine.set(settings);
    if(clearRequested.exchange(false))clearing=true;
    float d=0,r=0,o=0;const int ins=getTotalNumInputChannels(),outs=getTotalNumOutputChannels();
    for(int i=0;i<buffer.getNumSamples();++i){
        tide::Stereo input{buffer.getSample(0,i),buffer.getSample(ins>1?1:0,i)};
        auto y=engine.tick(input,bypass);
        if(clearing){clearFade=std::max(0.f,clearFade-1.f/float(getSampleRate()*.015));
            if(clearFade==0){engine.clear();clearing=false;}}
        else clearFade=std::min(1.f,clearFade+1.f/float(getSampleRate()*.015));
        y.l*=clearFade;y.r*=clearFade;
        if(outs==1)buffer.setSample(0,i,.5f*(y.l+y.r));else{buffer.setSample(0,i,y.l);buffer.setSample(1,i,y.r);}
        if(collectMeters){d=std::max(d,engine.delayPeak);r=std::max(r,engine.reverbPeak);o=std::max(o,std::max(std::abs(y.l),std::abs(y.r)));}
    }
    if(collectMeters){auto publish=[](std::atomic<float>& meter,float peak){auto old=meter.load(std::memory_order_relaxed);while(old<peak&&!meter.compare_exchange_weak(old,peak,std::memory_order_relaxed)){};};publish(delayMeter,d);publish(reverbMeter,r);publish(outputMeter,o);delayRunning.store(engine.activeEcho());reverbRunning.store(engine.activeReverbs()>0);shimmerRunning.store(engine.activeShimmer());}
}
void TideProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){render(b,false);}
void TideProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&){render(b,true);}
const juce::String TideProcessor::getProgramName(int i){return i>=0&&i<tide::numPresets?tide::presets[i].name:"";}
void TideProcessor::applyFactory(int i,bool respect){
    if(i<0||i>=tide::numPresets)return;
    const auto& p=tide::presets[i];
    const bool d=respect&&lockDelay.load(),r=respect&&lockReverb.load(),m=respect&&lockMix.load();
    const float oldDelayAmount=value("dAmount"),oldReverbAmount=value("rAmount");
    if(!d){setValue("interval",3);setValue("pitchMode",i==7?1.f:0.f);for(const auto& s:tide::specs)if(inScope(s.id,1))setValue(s.id,s.initial);
        setValue("dType",float(p.delay));setValue("rhythm",float(p.rhythm));setValue("stereo",float(p.stereo));setValue("dFeedback",p.feedback);setValue("dAmount",p.delayAmount);setValue("dMotion",p.motion);setValue("dDuck",p.duck);
        setValue("dOn",i==15?0.f:1.f);setValue("sync",i==5?0.f:1.f);if(i==5)setValue("dTime",105);delayPreset.store(p.delay);}
    if(!r){setValue("rInterval",3);for(const auto& s:tide::specs)if(inScope(s.id,2))setValue(s.id,s.initial);
        setValue("rType",float(p.reverb));setValue("rDecay",p.decay);setValue("rPredelay",p.pre);setValue("rTone",p.tone);setValue("rAmount",p.reverbAmount);setValue("rDuck",p.duck);setValue("shimmer",p.shimmer);setValue("rMotion",p.motion);
        setValue("rOn",i==14?0.f:1.f);reverbPreset.store(p.reverb);}
    setValue("routing",0);setValue("feed",p.feed);setValue("direct",1);setValue("bloom",0);setValue("freeze",0);
    // Send mode, fallback tempo and output are session controls: factory browsing never overwrites them.
    if(!m)setValue("mix",p.mix);else{setValue("dAmount",oldDelayAmount);setValue("rAmount",oldReverbAmount);}
    program.store(i);
}
void TideProcessor::setCurrentProgram(int i){applyFactory(i,true);}
void TideProcessor::applyDelayPreset(int i){if(i<0||i>5)return;
    const float amt=value("dAmount");
    for(const auto& s:tide::specs)if(inScope(s.id,1))setValue(s.id,s.initial);
    setValue("interval",3);setValue("pitchMode",0);setValue("dType",float(i));setValue("dOn",1);setValue("sync",1);setValue("rhythm",4);setValue("stereo",i==0?0.f:1.f);
    if(i==1){setValue("dTone",5200);setValue("dMotion",.25f);}if(i==2){setValue("dTone",3800);setValue("dMotion",.35f);}
    if(i==3){setValue("dFeedback",.4f);setValue("stereo",2);}if(i==4)setValue("dFeedback",.25f);
    if(lockMix.load())setValue("dAmount",amt);
    delayPreset.store(i);program.store(-1);
}
void TideProcessor::applyReverbPreset(int i){if(i<0||i>3)return;
    const float amt=value("rAmount");
    for(const auto& s:tide::specs)if(inScope(s.id,2))setValue(s.id,s.initial);
    setValue("rInterval",3);setValue("rType",float(i));setValue("rOn",1);
    constexpr float decay[]={1.2f,2.4f,4.5f,10};setValue("rDecay",decay[i]);
    if(i==3){setValue("rTone",4800);setValue("rMotion",.5f);setValue("shimmer",.15f);}
    if(lockMix.load())setValue("rAmount",amt);
    reverbPreset.store(i);program.store(-1);
}
bool TideProcessor::factoryMatches()const{
    const int i=program.load();if(i<0||i>=tide::numPresets||lockDelay.load()||lockReverb.load())return false;
    const auto& p=tide::presets[i];
    for(int n=0;n<tide::numFloats;++n){const auto& spec=tide::specs[n];const juce::String id(spec.id);
        if(id=="send"||id=="output"||id=="bpm")continue;
        float expected=spec.initial;
        if(id=="mix")expected=p.mix;else if(id=="dFeedback")expected=p.feedback;else if(id=="dAmount")expected=p.delayAmount;
        else if(id=="dMotion"||id=="rMotion")expected=p.motion;else if(id=="dDuck"||id=="rDuck")expected=p.duck;
        else if(id=="rDecay")expected=p.decay;else if(id=="rPredelay")expected=p.pre;else if(id=="rTone")expected=p.tone;
        else if(id=="rAmount")expected=p.reverbAmount;else if(id=="shimmer")expected=p.shimmer;
        else if(id=="feed")expected=p.feed;else if(id=="dTime"&&i==5)expected=105;
        if(std::abs(floats[std::size_t(n)]->load()-expected)>std::max(.002f,spec.step*.6f))return false;
    }
    return int(value("dType"))==p.delay&&int(value("rType"))==p.reverb&&int(value("rhythm"))==p.rhythm&&int(value("stereo"))==p.stereo
        &&int(value("dOn"))==int(i!=15)&&int(value("rOn"))==int(i!=14)&&int(value("sync"))==int(i!=5)&&int(value("routing"))==0&&int(value("freeze"))==0&&int(value("interval"))==3&&int(value("rInterval"))==3&&int(value("pitchMode"))==int(i==7);
}
void TideProcessor::getStateInformation(juce::MemoryBlock& out){
    auto tree=state.copyState();tree.setProperty("version",1,nullptr);tree.setProperty("program",program.load(),nullptr);
    tree.setProperty("lockDelay",lockDelay.load(),nullptr);tree.setProperty("lockReverb",lockReverb.load(),nullptr);tree.setProperty("lockMix",lockMix.load(),nullptr);tree.setProperty("reducedMotion",reducedMotion.load(),nullptr);tree.setProperty("visualMode",visualMode.load(),nullptr);
    if(auto xml=tree.createXml())copyXmlToBinary(*xml,out);
}
void TideProcessor::setStateInformation(const void* data,int size){
    if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName("WolfTideState")){
        auto tree=juce::ValueTree::fromXml(*xml);if(!tree.isValid())return;
        if(!tree.getChildWithProperty("id","pitchMode").isValid())setValue("pitchMode",0); // 1.0 sessions used climbing feedback.
        // Merge only known, finite parameters. Never replace the tree with malformed/missing values.
        for(const auto& child:tree){auto id=child.getProperty("id").toString();if(auto* param=state.getParameter(id)){
            const float v=float(child.getProperty("value",param->convertFrom0to1(param->getDefaultValue())));
            if(std::isfinite(v))param->setValueNotifyingHost(param->convertTo0to1(v));}}
        program.store(juce::jlimit(-1,tide::numPresets-1,int(tree.getProperty("program",-1))));
        lockDelay.store(bool(tree.getProperty("lockDelay",false)));lockReverb.store(bool(tree.getProperty("lockReverb",false)));lockMix.store(bool(tree.getProperty("lockMix",false)));reducedMotion.store(bool(tree.getProperty("reducedMotion",false)));
        visualMode.store(juce::jlimit(0,2,int(tree.getProperty("visualMode",reducedMotion.load()?0:1))));
    }
}
juce::Result TideProcessor::savePreset(const juce::File& file,int scope){
    juce::XmlElement root("WolfTidePreset");root.setAttribute("version",1);root.setAttribute("scope",scope);
    for(auto* p:getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(p))if(inScope(param->paramID,scope)){
        auto* child=root.createNewChildElement("p");child->setAttribute("id",param->paramID);child->setAttribute("value",double(param->convertFrom0to1(param->getValue())));}
    return file.replaceWithText(root.toString())?juce::Result::ok():juce::Result::fail("Could not write preset. Choose a writable folder.");
}
juce::Result TideProcessor::loadPreset(const juce::File& file,int scope){
    if(file.getSize()>1024*1024)return juce::Result::fail("Preset exceeds 1 MB limit.");
    auto xml=juce::XmlDocument::parse(file);if(!xml||!xml->hasTagName("WolfTidePreset")||xml->getIntAttribute("version")!=1||xml->getIntAttribute("scope",-1)!=scope)return juce::Result::fail("Choose a Wolf Tide preset saved for this section.");
    bool hasPitchMode=false;int valid=0;for(auto* child:xml->getChildIterator()){
        auto id=child->getStringAttribute("id");auto* param=state.getParameter(id);const float v=float(child->getDoubleAttribute("value",std::numeric_limits<double>::quiet_NaN()));
        if(!param||!inScope(id,scope)||!std::isfinite(v))return juce::Result::fail("Invalid preset parameter.");
        if(id=="pitchMode")hasPitchMode=true;
        ++valid;}
    if(valid==0)return juce::Result::fail("Preset contains no parameters.");
    if(scope!=2&&!hasPitchMode&&!(scope==0&&lockDelay.load()))setValue("pitchMode",0);
    for(auto* child:xml->getChildIterator()){auto id=child->getStringAttribute("id");if(scope==0&&((lockDelay.load()&&inScope(id,1))||(lockReverb.load()&&inScope(id,2))))continue;
        if(lockMix.load()&&(id=="mix"||id=="dAmount"||id=="rAmount"))continue;
        setValue(id,float(child->getDoubleAttribute("value")));}
    program.store(-1);return juce::Result::ok();
}
juce::AudioProcessorEditor* TideProcessor::createEditor(){return new TideEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new TideProcessor();}
