//OgreController is used for Mac OS X builds to integrate
// the Ogre setup for FreeOrion within a Cocoa environment

#import <Cocoa/Cocoa.h>
#import "Ogre/Ogre.h"

#include "chmain.h"
#include <iostream>

@interface OgreController : NSObject {
    
}

@end

@implementation OgreController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    if (mainSetupAndRunOgre() != 0)
        std::cerr << "OgreController failed to setup or run ogre." << std::endl;
}

@end
