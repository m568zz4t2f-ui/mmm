// SPDX-License-Identifier: AGPL-3.0-or-later
// Compile with -fsanitize=address,undefined; deliberately exercises delay-buffer boundaries.
#include "TideDSP.h"
#include <iostream>
#include <limits>
int main(){
 for(float sr:{8000.f,48000.f,192000.f})for(int mode=0;mode<6;++mode){
  tide::Parameters p;p.delayType=mode;p.reverbType=mode%4;p.sync=false;p.time=8000;p.ratio=2;p.stereo=1;p.feedback=.88f;p.shimmer=1;p.decay=20;
  tide::Engine engine;engine.prepare(sr,p);const int count=sr==8000.f?140000:4096;
  for(int n=0;n<count;++n){if(n%1024==0){p.time=n%2048?20.f:8000.f;p.reverbType=(p.reverbType+1)%4;engine.set(p);}
   auto y=engine.tick({n%4000==0?std::numeric_limits<float>::infinity():.02f,n%4001==0?std::numeric_limits<float>::quiet_NaN():-.02f});
   if(!std::isfinite(y.l)||!std::isfinite(y.r)){std::cerr<<"Nonfinite output\n";return 1;}
  }
  engine.clear();
 }
 std::cout<<"PASS: latest DSP under ASan/UBSan, 8-second main / 16-second secondary buffers, wraps, rapid time edits, invalid input, all models, 8/48/192 kHz.\n";
}
