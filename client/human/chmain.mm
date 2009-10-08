//This main is compiled only for Mac OS X builds

#import <Cocoa/Cocoa.h>

#include "../../util/OptionsDB.h"

#include "chmain.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if (mainConfigOptionsSetup(argc, argv) != 0) {
        std::cerr << "main() failed config." << std::endl;
        return 1;
    }

    // did the player request help output?
    if (GetOptionsDB().Get<bool>("help")) {
        GetOptionsDB().GetUsage(std::cerr);
        return 0;   // quit without actually starting game
    }

    return NSApplicationMain(argc,  (const char **) argv);
}
