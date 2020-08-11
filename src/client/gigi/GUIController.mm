// GUIController is used for Mac OS X builds to integrate
// the GUI setup for FreeOrion within a Cocoa environment

#import <Cocoa/Cocoa.h>

#include "chmain.h"
#include <iostream>

@interface GUIController : NSObject {
    
}

@end

@implementation GUIController

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    // set up rendering and run game
    if (mainSetupAndRun() != 0)
        std::cerr << "GUIController failed to setup or run the GUI." << std::endl;
    [NSApp terminate: self];
}

@end
