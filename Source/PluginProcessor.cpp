// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdlib>
namespace {
bool numericValue(const juce::var& raw,float& value){
    const auto text=raw.toString();const char* begin=text.toRawUTF8();char* end=nullptr;
    value=float(std::strtod(begin,&end));return end!=begin&&*end==0&&std::isfinite(value);
}
constexpr const char* choiceIds[]={"dType","rhythm","stereo","rType","routing","interval","dOn","rOn","sync","send","freeze","rInterval","pitchMode","dModel","rModel","dVersion","dPattern","dDual","dBRhythm","manualTempo","rVersion"};
// Side preset scope deliberately excludes shared routing, pitch interval and global balance.
bool inScope(const juce::String& id,int scope){
    if(scope==0)return true;
    if(scope==1)return (id.startsWith("d")&&id!="direct") || id=="drive"||id=="ratio"||id=="rhythm"||id=="stereo"||id=="sync"||id=="interval"||id=="pitchMode"||id=="dVersion";
    return (id.startsWith("r")&&id!="routing"&&id!="routing13"&&id!="rhythm"&&id!="ratio")||id=="shimmer"||id=="bass";
}
}
juce::AudioProcessorValueTreeState::ParameterLayout TideProcessor::layout(){
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    for(int n=0;n<31;++n){const auto& s=tide::specs[n];const juce::String id(s.id);
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
    // Append extensions after every legacy parameter; keep old IDs, order and choice domains.
    for(int n=31;n<39;++n){const auto& spec=tide::specs[n];result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{spec.id,1},juce::String(spec.id[0]=='d'?"Delay ":"Reverb ")+spec.name,juce::NormalisableRange<float>{spec.lo,spec.hi,spec.step,spec.skew},spec.initial));}
    choice("dModel","Delay extension",{"Original","Diffusion","Granular Pitch","Lo-Fi"},0);
    choice("rModel","Reverb extension",{"Original","Chamber","Cloud Ensemble","Halo Shimmer","Glint","Studio Plate","Studio Hall","Studio Bloom"},0);
    choice("dVersion","Delay sound version",{"Legacy 1.1","Modern 1.2"},0);
    choice("dPattern","Pattern taps",{"2 taps","4 taps","8 taps"},1);
    choice("dDual","Dual configuration",{"Linked ratio","Independent A / B"},0);
    choice("dBRhythm","B subdivision",{"1/16","1/8 triplet","1/8","1/8 dotted","1/4","1/4 dotted","1/2","1/2 dotted","1 bar"},4);
    flag("manualTempo","Manual tempo",false);
    choice("rVersion","Reverb sound version",{"Legacy 1.1","Modern 1.2"},0);
    const auto& spec=tide::specs[39];
    result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{spec.id,1},"Delay Diffusion amount",juce::NormalisableRange<float>{spec.lo,spec.hi,spec.step,spec.skew},spec.initial));
    for(const auto& row:tide::v13::modernSpecs){const auto& s=row.spec;auto attributes=juce::AudioParameterFloatAttributes().withLabel(s.unit);
        if(juce::String(s.unit)=="%")attributes=attributes.withStringFromValueFunction([](float v,int){return juce::String(v*100,1);}).withValueFromStringFunction([](const juce::String& text){return text.getFloatValue()/100;});
        result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{s.id,1},s.name,juce::NormalisableRange<float>{s.lo,s.hi,s.step,s.skew},s.initial,attributes));}
    for(const auto& s:tide::v13::modernChoices)choice(s.id,s.name,juce::StringArray::fromTokens(s.items,"|",""),s.initial);
    for(int i=0;i<6;++i)flag(tide::v13::modernFlags[i],tide::v13::modernFlagNames[i],false);
    return result;
}
TideProcessor::TideProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"WolfTideState",layout()){
    for(int i=0;i<tide::numFloats;++i)floats[std::size_t(i)]=state.getRawParameterValue(tide::specs[i].id);
    for(int i=0;i<21;++i)choices[std::size_t(i)]=state.getRawParameterValue(choiceIds[i]);
    for(const auto& s:tide::v13::modernSpecs)modernValues.emplace(s.spec.id,state.getRawParameterValue(s.spec.id));
    for(const auto& s:tide::v13::modernChoices)modernValues.emplace(s.id,state.getRawParameterValue(s.id));
    for(auto* id:tide::v13::modernFlags)modernValues.emplace(id,state.getRawParameterValue(id));
    applyFactory(16,false);
}
TideProcessor::~TideProcessor()=default;
float TideProcessor::value(const juce::String& id)const{if(auto* p=state.getRawParameterValue(id))return p->load();return 0;}
void TideProcessor::setValue(const juce::String& id,float v){if(auto* p=state.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}}
tide::Parameters TideProcessor::read()const{
    tide::Parameters p;auto f=[&](int i){return floats[std::size_t(i)]->load(std::memory_order_relaxed);};
    p.time=f(0);p.feedback=f(1);p.delayTone=f(2);p.delayAmount=f(3);p.delayMotion=f(4);p.delayWidth=f(5);p.drive=f(6);p.delayLowcut=f(7);p.delayDuck=f(8);p.delayRate=f(9);p.ratio=f(10);
    p.decay=f(11);p.predelay=f(12);p.reverbTone=f(13);p.reverbAmount=f(14);p.reverbMotion=f(15);p.reverbWidth=f(16);p.reverbLowcut=f(17);p.reverbDuck=f(18);p.shimmer=f(19);p.bassDecay=f(20);
    p.feed=f(21);p.direct=f(22);p.mix=f(23);p.output=f(24);p.bloom=f(25);p.bpm=f(26);p.delaySense=f(27);p.delayRelease=f(28);p.reverbSense=f(29);p.reverbRelease=f(30);
    auto c=[&](int i){return int(choices[std::size_t(i)]->load());};
    p.delayType=c(0);p.rhythm=c(1);p.stereo=c(2);p.reverbType=c(3);p.routing=c(4);p.interval=c(5);
    p.delayOn=c(6)!=0;p.reverbOn=c(7)!=0;p.sync=c(8)!=0;p.send=c(9)!=0;p.freeze=c(10)!=0;p.reverbInterval=c(11);p.pitchMode=c(12);p.delayModel=c(13);p.reverbModel=c(14);p.soundVersion=c(15);p.pattern=c(16);p.dualMode=c(17);p.bRhythm=c(18);p.manualTempo=c(19)!=0;p.reverbVersion=c(20);
    p.bTime=f(31);p.bFeedback=f(32);p.bLevel=f(33);p.swell=f(34);p.diffusion=f(35);p.size=f(36);p.attack=f(37);p.regeneration=f(38);p.delayDiffusion=f(39);return p;
}
tide::v13::Settings TideProcessor::readModern(const tide::Parameters& old)const{return tide::v13::settingsFrom(old,[&](std::string_view id){auto found=modernValues.find(id);return found==modernValues.end()?0.f:found->second->load(std::memory_order_relaxed);});}
void TideProcessor::prepareToPlay(double sr,int){auto old=read();auto next=readModern(old);const int selected=!(next.legacyDelay&&next.legacyReverb);if(preparedRate!=sr){engine.prepare(float(sr),old,next,selected);preparedRate=sr;clearFade=1;clearing=false;}else engine.set(old,next,selected);setLatencySamples(0);}
bool TideProcessor::isBusesLayoutSupported(const BusesLayout& b)const{
    const auto in=b.getMainInputChannelSet(),out=b.getMainOutputChannelSet();
    return (in==juce::AudioChannelSet::mono()&&(out==in||out==juce::AudioChannelSet::stereo()))||(in==juce::AudioChannelSet::stereo()&&out==in);
}
void TideProcessor::render(juce::AudioBuffer<float>& buffer,bool bypass){
    juce::ScopedNoDenormals noDenormals;auto settings=read();bool hostTempo=false;
    const bool collectMeters=editorVisible.load(std::memory_order_relaxed);
    if(!settings.manualTempo)if(auto* head=getPlayHead())if(auto pos=head->getPosition())if(auto bpm=pos->getBpm())if(std::isfinite(*bpm)&&*bpm>=20&&*bpm<=400){settings.bpm=float(*bpm);hostTempo=true;}
    tempo.store(settings.bpm);tempoFromHost.store(hostTempo);const auto modern=readModern(settings);engine.set(settings,modern,!(modern.legacyDelay&&modern.legacyReverb));
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
namespace {
juce::NamedValueSet recipe(const tide::Preset& p){
    juce::NamedValueSet v;for(const auto& spec:tide::specs)v.set(spec.id,spec.initial);
    auto put=[&](const char* id,float x){v.set(id,x);};
    put("dType",float(p.delay));put("rType",float(p.reverb));put("rhythm",float(p.rhythm));put("stereo",float(p.stereo));put("pitchMode",float(p.pitchMode));put("sync",p.sync?1.f:0.f);put("dOn",p.delayOn?1.f:0.f);put("rOn",p.reverbOn?1.f:0.f);put("dTime",p.time);
    put("dFeedback",p.feedback);put("dAmount",p.delayAmount);put("dMotion",p.motion);put("dDuck",p.duck);put("rDecay",p.decay);put("rPredelay",p.pre);put("rTone",p.tone);put("rAmount",p.reverbAmount);put("rDuck",p.duck);put("shimmer",p.shimmer);put("rMotion",p.motion);put("mix",p.mix);put("feed",p.feed);
    put("dModel",float(p.dModel));put("rModel",float(p.rModel));put("dVersion",float(p.version));put("rVersion",float(p.version));put("drive",p.drive);put("dTone",p.delayTone);put("dPattern",float(p.pattern));
    put("dDual",0);put("dBRhythm",3);put("interval",3);put("rInterval",3);put("routing",0);put("freeze",0);
    if(p.rModel==7)put("rAttack",.7f);
    for(const auto& row:tide::v13::modernSpecs)put(row.spec.id,row.spec.initial);
    for(const auto& row:tide::v13::modernChoices)put(row.id,float(row.initial));
    for(auto* id:tide::v13::modernFlags)put(id,0);
    put("dRevision",p.modernDelay>=0?1.f:0.f);put("rRevision",p.modernReverb>=0?1.f:0.f);
    put("d13Model",float(p.modernDelay>=0?p.modernDelay:tide::v13::modelFor(p.delay,p.dModel,true)));
    put("r13Model",float(p.modernReverb>=0?p.modernReverb:tide::v13::modelFor(p.reverb,p.rModel,false)));
    put("d2On",p.delayB>=0?1.f:0.f);put("r2On",p.reverbB>=0?1.f:0.f);
    if(p.delayB>=0)put("d2Model",float(p.delayB));if(p.reverbB>=0)put("r2Model",float(p.reverbB));put("routing13",float(p.route13));
    put("dAge",p.modernDelay==1?.35f:.25f);put("dTexture",p.modernDelay==7?.65f:.5f);put("dDensity",p.modernDelay==7?.7f:.5f);
    put("rEarly",p.modernReverb==0||p.modernReverb==4?.5f:.3f);put("rTune",110);if(p.modernReverb==11)put("rAttack",.28f);
    return v;
}
bool routingControl(const juce::String& id){return id=="routing"||id=="feed"||id=="direct"||id=="routing13"||id=="splitHz"||id=="crossFeedback";}
}
namespace {
bool identityParameter(const juce::String& id,int scope){return inScope(id,scope)&&id!="output"&&id!="bpm"&&id!="send"&&id!="manualTempo";}
}
juce::String TideProcessor::sidePresetName(int scope,int index){
    const int model=index/2;
    const auto name=scope==1?tide::v13::delayNames[std::clamp(model,0,11)]:tide::v13::reverbNames[std::clamp(model,0,12)];
    return juce::String(name)+(index%2?" / Wide":" / Focused");
}
void TideProcessor::rememberPreset(int scope,const juce::String& name,bool factory,int index,const juce::NamedValueSet* expected){
    auto identity=std::make_shared<PresetIdentity>();identity->name=name.substring(0,256);identity->factory=factory;identity->index=index;
    for(auto* parameter:getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(identityParameter(param->paramID,scope)){
        const auto id=param->paramID;float v=value(id);
        if(expected&&expected->contains(id))v=param->getNormalisableRange().snapToLegalValue(float((*expected)[id]));
        identity->values.set(id,v);
    }
    std::atomic_store(&presetIdentities[std::size_t(scope)],std::shared_ptr<const PresetIdentity>(identity));
}
TideProcessor::PresetDisplay TideProcessor::presetDisplay(int scope)const{
    if(scope<0||scope>2)return {};
    auto identity=std::atomic_load(&presetIdentities[std::size_t(scope)]);
    if(!identity)return {scope==0?"Custom combination":scope==1?"Custom delay":"Custom reverb",false,false,-1};
    bool modified=false;
    for(int i=0;i<identity->values.size();++i){const float expected=float(identity->values.getValueAt(i));if(std::abs(value(identity->values.getName(i).toString())-expected)>std::max(1e-6f,std::abs(expected)*1e-7f)){modified=true;break;}}
    return {identity->name,identity->factory,modified,identity->index,!identity->factory};
}
void TideProcessor::restorePresetIdentities(const juce::ValueTree& tree){
    for(auto& identity:presetIdentities)std::atomic_store(&identity,std::shared_ptr<const PresetIdentity>{});
    const int selected=program.load();
    if(selected>=0&&selected<tide::numPresets){const auto expected=recipe(int(tree.getProperty("version",1))<4&&selected<52?tide::originalPresets[selected]:tide::presets[std::size_t(selected)]);for(int scope=0;scope<3;++scope)rememberPreset(scope,tide::presets[selected].name,true,scope==0?selected:-1,&expected);}
    else if(loadedName.isNotEmpty()&&loadedName!="Custom combination")rememberPreset(0,loadedName,false,-1);
    std::array<bool,3> restored{};
    for(const auto& child:tree)if(child.hasType("TidePresetIdentity")){
        const int scope=int(child.getProperty("scope",-1));if(scope<0||scope>2||restored[std::size_t(scope)])continue;
        auto identity=std::make_shared<PresetIdentity>();identity->name=child.getProperty("name").toString().substring(0,256);identity->factory=bool(child.getProperty("factory",false));identity->index=int(child.getProperty("index",-1));
        if(identity->name.isEmpty()||identity->index < -1||identity->index>=(scope==0?tide::numPresets:scope==1?24:26))continue;
        bool valid=true;
        for(const auto& item:child){const auto id=item.getProperty("id").toString();auto* param=state.getParameter(id);float v=0;
            if(!param||!identityParameter(id,scope)||identity->values.contains(id)||!numericValue(item.getProperty("value"),v)||v<param->getNormalisableRange().start||v>param->getNormalisableRange().end){valid=false;break;}
            identity->values.set(id,v);
        }
        for(auto* parameter:getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(identityParameter(param->paramID,scope)&&!identity->values.contains(param->paramID)){
            if(int(tree.getProperty("version",1))<4&&tide::v13::isModernId(param->paramID.toStdString()))identity->values.set(param->paramID,param->convertFrom0to1(param->getDefaultValue()));else valid=false;
        }
        if(valid){std::atomic_store(&presetIdentities[std::size_t(scope)],std::shared_ptr<const PresetIdentity>(identity));restored[std::size_t(scope)]=true;}
    }
    delayPreset.store(presetDisplay(1).index);reverbPreset.store(presetDisplay(2).index);
}
void TideProcessor::captureUndo(){getStateInformation(lastLoad);undoReady=true;}
void TideProcessor::undoLastLoad(){if(!undoReady)return;juce::MemoryBlock now;getStateInformation(now);setStateInformation(lastLoad.getData(),int(lastLoad.getSize()));lastLoad=now;}
void TideProcessor::applyFactory(int i,bool respect){
    if(i<0||i>=tide::numPresets)return;
    if(respect)captureUndo();const auto values=recipe(tide::presets[i]);
    for(int n=0;n<values.size();++n){const auto id=values.getName(n).toString();
        if(id=="output"||id=="bpm"||id=="send"||id=="manualTempo")continue;
        if(respect&&((lockDelay.load()&&inScope(id,1))||(lockReverb.load()&&inScope(id,2))||(lockMix.load()&&(id=="mix"||id=="dAmount"||id=="rAmount"||id=="d2Amount"||id=="r2Amount"))||(lockRouting.load()&&routingControl(id))))continue;
        setValue(id,float(values.getValueAt(n)));}
    program.store(i);loadedName=tide::presets[i].name;loadedScope=0;
    rememberPreset(0,loadedName,true,i,&values);
    if(!respect||!lockDelay.load()){rememberPreset(1,loadedName,true,-1,&values);delayPreset.store(-1);}
    if(!respect||!lockReverb.load()){rememberPreset(2,loadedName,true,-1,&values);reverbPreset.store(-1);}
}
void TideProcessor::setCurrentProgram(int i){applyFactory(i,true);}
void TideProcessor::applyDelayPreset(int i,bool alternate){if(i<0||i>11)return;captureUndo();
    const float amt=value("dAmount"),bAmount=value("d2Amount");for(const auto& spec:tide::specs)if(inScope(spec.id,1))setValue(spec.id,spec.initial);
    setValue("dModel",float(i>=6?i-5:0));setValue("dType",float(i<6?i:0));setValue("dVersion",1);setValue("dOn",1);setValue("sync",1);setValue("rhythm",alternate?3.f:4.f);setValue("stereo",alternate?2.f:0.f);setValue("interval",3);setValue("pitchMode",1);setValue("drive",i==1||i==2?.2f:0.f);setValue("dMotion",i==1?.2f:i==2?.25f:0.f);setValue("dTone",i==1?5200.f:i==2?3800.f:11000.f);setValue("dDual",0);setValue("dPattern",alternate?2.f:1.f);setValue("dBRhythm",3);
    for(const auto& row:tide::v13::modernSpecs)if(inScope(row.spec.id,1))setValue(row.spec.id,row.spec.initial);
    for(const auto& row:tide::v13::modernChoices)if(inScope(row.id,1))setValue(row.id,float(row.initial));
    setValue("dRevision",1);setValue("d13Model",float(i));setValue("d2On",0);setValue("dFreeze",0);setValue("d2Freeze",0);
    // Existing IDs retain their original domains; new machines use appended model IDs.
    if(i>8){setValue("dType",0);setValue("dModel",0);}setValue("dTexture",i==7?.65f:.5f);setValue("dDensity",i==7?.7f:.5f);
    if(lockMix.load()){setValue("dAmount",amt);setValue("d2Amount",bAmount);}loadedScope=1;delayPreset.store(i*2+int(alternate));program.store(-1);loadedName="Custom combination";
    juce::NamedValueSet expected;expected.set(loadedScope==1?"dAmount":"rAmount",loadedScope==1?tide::specs[3].initial:tide::specs[14].initial);
    expected.set(loadedScope==1?"d2Amount":"r2Amount",.24f);
    rememberPreset(loadedScope,sidePresetName(loadedScope,i*2+int(alternate)),true,i*2+int(alternate),&expected);
}
void TideProcessor::applyReverbPreset(int i,bool alternate){if(i<0||i>12)return;captureUndo();
    const float amt=value("rAmount"),bAmount=value("r2Amount");for(const auto& spec:tide::specs)if(inScope(spec.id,2))setValue(spec.id,spec.initial);
    setValue("rModel",float(i>=4?i-3:0));setValue("rType",float(i<4?i:2));setValue("rVersion",1);setValue("rInterval",3);setValue("rOn",1);setValue("rDecay",i==0?1.2f:i==1?2.4f:i==4?1.1f:alternate?8.f:4.5f);setValue("rPredelay",alternate?45.f:18.f);setValue("rMotion",alternate?.35f:.18f);
    if(i==6||i==7)setValue("shimmer",alternate?.45f:.25f);if(i==3||i==10)setValue("rAttack",alternate?.9f:.3f);
    for(const auto& row:tide::v13::modernSpecs)if(inScope(row.spec.id,2))setValue(row.spec.id,row.spec.initial);
    for(const auto& row:tide::v13::modernChoices)if(inScope(row.id,2))setValue(row.id,float(row.initial));
    setValue("rRevision",1);setValue("r13Model",float(i));setValue("r2On",0);setValue("r2Freeze",0);setValue("freeze",0);if(i>10){setValue("rType",2);setValue("rModel",0);}if(i==11)setValue("rAttack",alternate?.6f:.2f);
    if(lockMix.load()){setValue("rAmount",amt);setValue("r2Amount",bAmount);}loadedScope=2;reverbPreset.store(i*2+int(alternate));program.store(-1);loadedName="Custom combination";
    juce::NamedValueSet expected;expected.set(loadedScope==1?"dAmount":"rAmount",loadedScope==1?tide::specs[3].initial:tide::specs[14].initial);
    expected.set(loadedScope==1?"d2Amount":"r2Amount",.24f);
    rememberPreset(loadedScope,sidePresetName(loadedScope,i*2+int(alternate)),true,i*2+int(alternate),&expected);
}
bool TideProcessor::factoryMatches()const{
    const auto display=presetDisplay(0);
    return display.factory&&!display.modified&&display.index==program.load();
}
void TideProcessor::getStateInformation(juce::MemoryBlock& out){
    auto tree=state.copyState();tree.setProperty("version",4,nullptr);tree.setProperty("loadedName",loadedName,nullptr);tree.setProperty("lockRouting",lockRouting.load(),nullptr);tree.setProperty("program",program.load(),nullptr);
    tree.setProperty("lockDelay",lockDelay.load(),nullptr);tree.setProperty("lockReverb",lockReverb.load(),nullptr);tree.setProperty("lockMix",lockMix.load(),nullptr);tree.setProperty("reducedMotion",reducedMotion.load(),nullptr);tree.setProperty("visualMode",visualMode.load(),nullptr);
    tree.setProperty("loadedScope",loadedScope,nullptr);
    for(int scope=0;scope<3;++scope)if(auto identity=std::atomic_load(&presetIdentities[std::size_t(scope)])){
        juce::ValueTree node("TidePresetIdentity");node.setProperty("scope",scope,nullptr);node.setProperty("name",identity->name,nullptr);node.setProperty("factory",identity->factory,nullptr);node.setProperty("index",identity->index,nullptr);
        for(int i=0;i<identity->values.size();++i){juce::ValueTree item("PresetValue");item.setProperty("id",identity->values.getName(i).toString(),nullptr);item.setProperty("value",identity->values.getValueAt(i),nullptr);node.addChild(item,-1,nullptr);}
        tree.addChild(node,-1,nullptr);
    }
    if(auto xml=tree.createXml())copyXmlToBinary(*xml,out);
}
void TideProcessor::setStateInformation(const void* data,int size){
    if(size<=0||size>1024*1024)return;
    if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName("WolfTideState")){
        auto tree=juce::ValueTree::fromXml(*xml);if(!tree.isValid()||int(tree.getProperty("version",1))<1||int(tree.getProperty("version",1))>4)return;
        juce::StringArray seen;
        for(const auto& child:tree){const auto id=child.getProperty("id").toString();if(auto* param=state.getParameter(id)){
            if(!child.hasProperty("value")||seen.contains(id))return;
            float v=0;if(!numericValue(child.getProperty("value"),v))return;const auto& range=param->getNormalisableRange();
            if(!std::isfinite(v)||v<range.start||v>range.end)return;
            if(dynamic_cast<juce::AudioParameterChoice*>(param)||dynamic_cast<juce::AudioParameterBool*>(param))if(std::abs(v-std::round(v))>1e-5f)return;
            seen.add(id);
        }}
        const int schema=int(tree.getProperty("version",1));
        const int required=schema>=4?getParameters().size():schema>=3?61:schema==2?60:43;
        for(int n=0;n<required;++n){auto* param=dynamic_cast<juce::RangedAudioParameter*>(getParameters()[n]);if(param&&!seen.contains(param->paramID))return;}
        if(schema<4)for(auto* parameter:getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(tide::v13::isModernId(param->paramID.toStdString()))param->setValueNotifyingHost(param->getDefaultValue());
        // Extension defaults belong to old states, never to the last modern preset played.
        for(int i=31;i<tide::numFloats;++i)if(!tree.getChildWithProperty("id",tide::specs[i].id).isValid())setValue(tide::specs[i].id,tide::specs[i].initial);
        constexpr const char* added[]={"dModel","rModel","dVersion","dPattern","dDual","dBRhythm","manualTempo","rVersion"};
        for(const auto* id:added)if(!tree.getChildWithProperty("id",id).isValid())if(auto* param=state.getParameter(id))param->setValueNotifyingHost(param->getDefaultValue());
        if(!tree.getChildWithProperty("id","dDiffusion").isValid()){
            auto reverbDiffusion=tree.getChildWithProperty("id","rDiffusion");
            const float migrated=float(reverbDiffusion.getProperty("value",.7f));
            setValue("dDiffusion",std::isfinite(migrated)?migrated:.7f);
        }
        loadedName=tree.getProperty("loadedName","Custom combination").toString();lockRouting.store(bool(tree.getProperty("lockRouting",false)));
        if(!tree.getChildWithProperty("id","pitchMode").isValid())setValue("pitchMode",0); // 1.0 sessions used climbing feedback.
        // Merge only known, finite parameters. Never replace the tree with malformed/missing values.
        for(const auto& child:tree){auto id=child.getProperty("id").toString();if(auto* param=state.getParameter(id)){
            const float v=float(child.getProperty("value",param->convertFrom0to1(param->getDefaultValue())));
            if(std::isfinite(v))param->setValueNotifyingHost(param->convertTo0to1(v));}}
        program.store(juce::jlimit(-1,tide::numPresets-1,int(tree.getProperty("program",-1))));
        lockDelay.store(bool(tree.getProperty("lockDelay",false)));lockReverb.store(bool(tree.getProperty("lockReverb",false)));lockMix.store(bool(tree.getProperty("lockMix",false)));reducedMotion.store(bool(tree.getProperty("reducedMotion",false)));
        loadedScope=juce::jlimit(0,2,int(tree.getProperty("loadedScope",0)));restorePresetIdentities(tree);
        visualMode.store(juce::jlimit(0,2,int(tree.getProperty("visualMode",reducedMotion.load()?0:1))));
    }
}
juce::Result TideProcessor::savePreset(const juce::File& file,int scope){
    juce::XmlElement root("WolfTidePreset");root.setAttribute("version",4);root.setAttribute("scope",scope);root.setAttribute("name",file.getFileNameWithoutExtension());root.setAttribute("presetId",scope==0&&factoryMatches()?"factory.tide."+juce::String(program.load()):juce::Uuid().toString());root.setAttribute("delayModel",int(value("dModel")));root.setAttribute("reverbModel",int(value("rModel")));root.setAttribute("delayVersion",int(value("dVersion")));root.setAttribute("reverbVersion",int(value("rVersion")));
    for(auto* p:getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(p))if(inScope(param->paramID,scope)){
        auto* child=root.createNewChildElement("p");child->setAttribute("id",param->paramID);child->setAttribute("value",double(param->convertFrom0to1(param->getValue())));}
    if(!file.replaceWithText(root.toString()))return juce::Result::fail("Could not write preset. Choose a writable folder.");
    rememberPreset(scope,file.getFileNameWithoutExtension(),false,-1);
    return juce::Result::ok();
}
juce::Result TideProcessor::loadPreset(const juce::File& file,int scope){
    if(file.getSize()>1024*1024)return juce::Result::fail("Preset exceeds 1 MB limit.");
    auto xml=juce::XmlDocument::parse(file);if(!xml||!xml->hasTagName("WolfTidePreset")||(xml->getIntAttribute("version")<1||xml->getIntAttribute("version")>4)||xml->getIntAttribute("scope",-1)!=scope)return juce::Result::fail("Choose a Wolf Tide preset saved for this section.");
    bool hasPitchMode=false;int valid=0;juce::StringArray seen;for(auto* child:xml->getChildIterator()){
        auto id=child->getStringAttribute("id");auto* param=state.getParameter(id);float v=0;if(!numericValue(child->getStringAttribute("value"),v))return juce::Result::fail("Invalid preset number.");
        if(xml->getIntAttribute("version")==1&&scope==2&&(id=="rhythm"||id=="ratio"))continue;
        if(!param||!inScope(id,scope)||!std::isfinite(v)||seen.contains(id)||v<param->getNormalisableRange().start||v>param->getNormalisableRange().end)return juce::Result::fail("Invalid preset parameter.");
        if(dynamic_cast<juce::AudioParameterChoice*>(param)||dynamic_cast<juce::AudioParameterBool*>(param))if(std::abs(v-std::round(v))>1e-5f)return juce::Result::fail("Invalid preset choice.");
        seen.add(id);if(id=="pitchMode")hasPitchMode=true;
        ++valid;}
    if(valid==0)return juce::Result::fail("Preset contains no parameters.");
    if(xml->getIntAttribute("version")>=2)for(auto* parameter:getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(inScope(param->paramID,scope)&&(xml->getIntAttribute("version")>=4||!tide::v13::isModernId(param->paramID.toStdString()))&&(xml->getIntAttribute("version")>=3||param->paramID!="dDiffusion")&&!seen.contains(param->paramID))return juce::Result::fail("Preset is incomplete for this version.");
    captureUndo();
    auto protectedByLocks=[&](const juce::String& id){return (scope==0&&((lockDelay.load()&&inScope(id,1))||(lockReverb.load()&&inScope(id,2))))||(lockRouting.load()&&routingControl(id))||(lockMix.load()&&(id=="mix"||id=="dAmount"||id=="rAmount"||id=="d2Amount"||id=="r2Amount"));};
    if(xml->getIntAttribute("version")<4)for(auto* parameter:getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(tide::v13::isModernId(param->paramID.toStdString())&&inScope(param->paramID,scope)&&!protectedByLocks(param->paramID))param->setValueNotifyingHost(param->getDefaultValue());
    if(scope!=2&&!seen.contains("dDiffusion")&&!(scope==0&&lockDelay.load())){
        float migrated=.7f;
        for(auto* child:xml->getChildIterator())if(child->getStringAttribute("id")=="rDiffusion")migrated=float(child->getDoubleAttribute("value",.7));
        setValue("dDiffusion",migrated);
    }
    if(xml->getIntAttribute("version")==1){for(int n=31;n<tide::numFloats;++n)if(inScope(tide::specs[n].id,scope)&&!(scope==0&&((lockDelay.load()&&inScope(tide::specs[n].id,1))||(lockReverb.load()&&inScope(tide::specs[n].id,2)))))setValue(tide::specs[n].id,tide::specs[n].initial);
      for(const auto* id:{"dModel","rModel","dVersion","dPattern","dDual","dBRhythm","manualTempo","rVersion"})if(inScope(id,scope)&&!(scope==0&&((lockDelay.load()&&inScope(id,1))||(lockReverb.load()&&inScope(id,2)))))if(auto* param=state.getParameter(id))param->setValueNotifyingHost(param->getDefaultValue());}
    if(scope!=2&&!hasPitchMode&&!(scope==0&&lockDelay.load()))setValue("pitchMode",0);
    for(auto* child:xml->getChildIterator()){auto id=child->getStringAttribute("id");if(xml->getIntAttribute("version")==1&&scope==2&&(id=="rhythm"||id=="ratio"))continue;if(scope==0&&((lockDelay.load()&&inScope(id,1))||(lockReverb.load()&&inScope(id,2))))continue;
        if(id=="output"||id=="bpm"||id=="send"||id=="manualTempo")continue;
        if(lockRouting.load()&&routingControl(id))continue;
        if(lockMix.load()&&(id=="mix"||id=="dAmount"||id=="rAmount"||id=="d2Amount"||id=="r2Amount"))continue;
        setValue(id,float(child->getDoubleAttribute("value")));}
    program.store(-1);loadedName=xml->getStringAttribute("name",file.getFileNameWithoutExtension());loadedScope=scope;
    juce::NamedValueSet expected;for(auto* child:xml->getChildIterator())if(auto* param=state.getParameter(child->getStringAttribute("id")))expected.set(param->paramID,float(child->getDoubleAttribute("value")));
    if(xml->getIntAttribute("version")<4)for(auto* parameter:getParameters())if(auto* param=dynamic_cast<juce::RangedAudioParameter*>(parameter))if(tide::v13::isModernId(param->paramID.toStdString())&&!expected.contains(param->paramID))expected.set(param->paramID,param->convertFrom0to1(param->getDefaultValue()));
    rememberPreset(scope,loadedName,false,-1,&expected);
    if(scope==0){if(!lockDelay.load())rememberPreset(1,loadedName,false,-1,&expected);if(!lockReverb.load())rememberPreset(2,loadedName,false,-1,&expected);}
    else if(scope==1)delayPreset.store(-1);else reverbPreset.store(-1);
    return juce::Result::ok();
}
juce::AudioProcessorEditor* TideProcessor::createEditor(){return new TideEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new TideProcessor();}
