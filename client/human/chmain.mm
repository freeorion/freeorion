//This main is compiled only for Mac OS X builds

#import <Cocoa/Cocoa.h>

#include "../../util/OptionsDB.h"

#include "chmain.h"
#include <iostream>

int main(int argc, char *argv[])
{
    // copy command line arguments to vector
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i)
        args.push_back(argv[i]);
    
    // set options from command line or config.xml, or generate config.xml
    if (mainConfigOptionsSetup(args) != 0) {
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
