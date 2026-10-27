// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline diagnostic; printed measurements are not a release pass verdict.
#include <cstdlib>
#include <new>
#include <cstdio>
static bool recording=false;static size_t bytes=0,calls=0;
void* operator new(size_t n){if(recording){bytes+=n;calls++;}if(void* p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void* p)noexcept{std::free(p);}void operator delete(void* p,size_t)noexcept{std::free(p);}
#include "TideDSP.h"
int main(){for(double rate:{48000.,192000.}){tide::Parameters p;p.reverbOn=false;p.delayType=0;tide::Engine e;bytes=calls=0;recording=true;e.prepare(rate,p);recording=false;std::printf("%.0f,%.3f,%zu\n",rate,double(bytes)/(1024*1024),calls);bytes=calls=0;recording=true;for(int n=0;n<48000;++n){if(n%128==0)e.set(p);e.tick({.01f,.007f});}recording=false;std::printf("processing_allocations,%zu,%zu\n",calls,bytes);}}
