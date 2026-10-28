// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "ModernDSP.h"
namespace tide::v13 {
// Both revisions are prepared outside processing. Old sessions use the original
// complete signal path; mixed sections use explicit legacy stems in the new router.
class VersionedEngine {
    tide::Engine legacy;Engine modern;
    float sr=48000,mix=0;int revision=0;
    bool legacyDelay=true,legacyReverb=true;
public:
    float delayPeak=0,reverbPeak=0;
    void prepare(float rate,const tide::Parameters& oldSettings,const Settings& newSettings,int selected=0){sr=rate;legacy.prepare(rate,oldSettings);modern.prepare(rate,newSettings);mix=selected?1.f:0.f;set(oldSettings,newSettings,selected);}
    void set(const tide::Parameters& oldSettings,const Settings& newSettings,int selected){auto old=oldSettings;revision=selected?1:0;legacyDelay=!revision||newSettings.legacyDelay;legacyReverb=!revision||newSettings.legacyReverb;if(revision)old.routing=newSettings.routing==1?1:0;legacy.set(old);modern.set(newSettings);}
    void clear(){legacy.clear();modern.clear();}
    bool activeEcho()const{return legacyDelay?legacy.activeEcho():modern.delayActive();}
    int activeReverbs()const{return legacyReverb?legacy.activeReverbs():int(modern.reverbActive());}
    bool activeShimmer()const{return legacyReverb?legacy.activeShimmer():modern.reverbActive();}
    Stereo tick(Stereo input,bool bypass=false)noexcept{
        Stereo result;
        if(revision==0&&mix==0){result=legacy.tick(input,bypass);delayPeak=legacy.delayPeak;reverbPeak=legacy.reverbPeak;return result;}
        const bool needLegacy=legacyDelay||legacyReverb||modern.needsLegacy()||mix<1||revision==0;
        const auto modernOut=modern.tick(input,bypass,needLegacy?&legacy:nullptr);
        // The hybrid path advances legacy exactly once per sample, including transitions.
        mix+=std::clamp(float(revision)-mix,-1/(sr*.075f),1/(sr*.075f));
        // The legacy output is reconstructed from the same stems during the short fade.
        // On a revision change, the new router keeps its outgoing machine tails alive.
        result=mix>=1?modernOut:blend(modern.legacyOutput,modernOut,mix);delayPeak=modern.delayPeak;reverbPeak=modern.reverbPeak;return result;
    }
};
}
