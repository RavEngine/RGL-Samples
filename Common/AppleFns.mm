#if __APPLE__
#include "AppleFns.hpp"
#import <Foundation/NSAutoreleasePool.h>
thread_local NSAutoreleasePool *pool;

void AppleAutoreleasePoolInit(){
    pool = [[NSAutoreleasePool alloc] init];
}

void AppleAutoreleasePoolDrain(){
    [pool drain];
}

#endif
