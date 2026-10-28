// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "ModernDSP.h"
namespace tide::v13 {
// Development bridge only: host state/parameter integration is not implemented yet.
// ponytail: both revisions are prepared to avoid allocation on a revision change;
// a background-prepared pool is required before claiming a low-memory release.
class VersionedEngine {
    tide::Engine legacy;Engine modern;
    float sr=48000,mix=0;int revision=0;
public:
    void prepare(float rate,const tide::Parameters& oldSettings,const Settings& newSettings,int selected=0){sr=rate;legacy.prepare(rate,oldSettings);modern.prepare(rate,newSettings);revision=selected?1:0;mix=float(revision);}
    void set(const tide::Parameters& oldSettings,const Settings& newSettings,int selected){legacy.set(oldSettings);modern.set(newSettings);revision=selected?1:0;}
    void clear(){legacy.clear();modern.clear();}
    Stereo tick(Stereo input,bool bypass=false)noexcept{
        if(revision==0&&mix==0)return legacy.tick(input,bypass);
        if(revision==1&&mix==1)return modern.tick(input,bypass);
        mix+=std::clamp(float(revision)-mix,-1/(sr*.075f),1/(sr*.075f));
        return blend(legacy.tick(input,bypass),modern.tick(input,bypass),mix);
    }
};
}
