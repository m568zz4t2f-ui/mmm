#include "TideDSP.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
int main(){volatile float guard=0;for(int scenario=0;scenario<8;++scenario){std::vector<double> results;for(int run=0;run<3;++run){tide::Parameters p;p.delayType=scenario==6?5:scenario==5?4:1;p.reverbType=scenario<4?scenario:2;p.shimmer=scenario==4||scenario==6?.45f:0;p.delayOn=scenario!=7;p.reverbOn=scenario!=7;p.freeze=false;tide::Engine e;e.prepare(48000,p);constexpr int frames=48000*3;auto start=std::chrono::steady_clock::now();for(int n=0;n<frames;++n){if(n%256==0)e.set(p);float x=scenario==7?0:.1f*std::sin(float(n)*.029f);auto y=e.tick({x,x*.73f});guard+=y.l*.00001f;}auto end=std::chrono::steady_clock::now();results.push_back(std::chrono::duration<double>(end-start).count());}std::sort(results.begin(),results.end());std::cout<<scenario<<","<<results[1]<<"\n";}std::cerr<<guard<<"\n";}
