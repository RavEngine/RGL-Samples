#include "AppleFns.hpp"
#import <Foundation/NSAutoreleasePool.h>
#if __APPLE__
thread_local NSAutoreleasePool *pool;

void AppleAutoreleasePoolInit(){
    pool = [[NSAutoreleasePool alloc] init];
}

void AppleAutoreleasePoolDrain(){
    [pool drain];
}

#endif
