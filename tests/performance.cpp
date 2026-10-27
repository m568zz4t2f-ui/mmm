// SPDX-License-Identifier: AGPL-3.0-or-later
#include "TideDSP.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
int main(){volatile float guard=0;for(int scenario=0;scenario<6;++scenario){tide::Parameters p;p.delayType=scenario==2?1:scenario==5?5:0;p.delayMotion=0;p.drive=0;p.reverbOn=scenario>=3;p.reverbType=2;p.delayOn=scenario!=0;p.mix=.4f;
#ifdef TIDE_NEW
 p.soundVersion=1;if(scenario==4){p.reverbModel=2;p.delayOn=false;}
#endif
 tide::Engine engine;engine.prepare(48000,p);std::vector<double> blocks;blocks.reserve(1500);for(int block=0;block<1600;++block){if(block%32==0)p.output=block%64?0:-.1f;auto begin=std::chrono::steady_clock::now();engine.set(p);for(int n=0;n<128;++n){float x=scenario==0?0:.06f*std::sin(float(block*128+n)*.029f);auto y=engine.tick({x,.73f*x});guard+=y.l*.000001f;}auto end=std::chrono::steady_clock::now();if(block>=100)blocks.push_back(std::chrono::duration<double,std::micro>(end-begin).count());}std::sort(blocks.begin(),blocks.end());std::cout<<scenario<<","<<blocks[750]<<","<<blocks[1485]<<","<<blocks.back()<<"\n";}std::cerr<<guard<<"\n";}
