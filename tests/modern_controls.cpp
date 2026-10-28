// SPDX-License-Identifier: AGPL-3.0-or-later
#include "ModernParameters.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
using namespace tide::v13;
static void check(bool ok,const char* text){if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int main(){
 std::map<std::string,float,std::less<>> values;for(const auto& row:modernSpecs)values[row.spec.id]=row.spec.initial;for(const auto& row:modernChoices)values[row.id]=float(row.initial);for(auto* id:modernFlags)values[id]=0;
 tide::Parameters old;old.sync=false;old.time=old.bTime=70;old.feedback=.45f;old.decay=1.5f;old.predelay=0;old.shimmer=.3f;
 int exercised=0;
 for(const auto& row:modernSpecs){const std::string id=row.spec.id;const bool delay=id[0]=='d';const bool reverb=id[0]=='r';if(!delay&&!reverb)continue;
  const bool second=id.size()>1&&id[1]=='2';const int slot=second?1:0;const std::string modelId=delay?(second?"d2Model":"d13Model"):(second?"r2Model":"r13Model");
  for(int model=0;model<(delay?12:13);++model){if(!applicable(id,model,delay))continue;
   values[modelId]=float(model);values["d2Time"]=70;values["r2Predelay"]=0;values["r2Shimmer"]=.3f;
   values[id]=row.spec.lo;auto lo=settingsFrom(old,[&](std::string_view key){return values.find(key)->second;});values[id]=row.spec.hi;auto hi=settingsFrom(old,[&](std::string_view key){return values.find(key)->second;});values[id]=row.spec.initial;
   DelayMachine da,db;ReverbMachine ra,rb;if(delay){da.prepare(8000);db.prepare(8000);da.set(lo.delays[std::size_t(slot)]);db.set(hi.delays[std::size_t(slot)]);da.clear();db.clear();}else{ra.prepare(8000);rb.prepare(8000);ra.set(lo.reverbs[std::size_t(slot)]);rb.set(hi.reverbs[std::size_t(slot)]);ra.clear();rb.clear();}
   double difference=0;for(int n=0;n<16000;++n){const float x=n<8000?.08f*(std::sin(n*.073f)+.3f*std::sin(n*.817f)):0;tide::Stereo in{x,x*.37f};auto a=delay?da.tick(in):ra.tick(in),b=delay?db.tick(in):rb.tick(in);check(std::isfinite(a.l)&&std::isfinite(b.l),"control extrema finite");difference+=std::pow(double(a.l)-b.l,2)+std::pow(double(a.r)-b.r,2);}
   // Amounts belong to the router, not the individual machine.
   if(id=="d2Amount"||id=="r2Amount")continue;
   if(difference<1e-12){std::fprintf(stderr,"No response: %s model %d (%.9g)\n",id.c_str(),model,difference);return 1;}++exercised;
  }
 }
 std::printf("PASS: %d exposed machine/control contracts respond at their extrema. Numerical response, not an audition score.\n",exercised);
}
