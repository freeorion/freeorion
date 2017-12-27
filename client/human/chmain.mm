//This main is compiled only for Mac OS X builds

// workaround by Apple to avoid conflicting macro names, fixes compile error
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0

#import <Cocoa/Cocoa.h>

#include "../../util/OptionsDB.h"
#include "../../util/Version.h"

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
    auto help_arg = GetOptionsDB().Get<std::string>("help");
    if (help_arg != "NOOP") {
        GetOptionsDB().GetUsage(std::cerr, help_arg, true);
        return 0;   // quit without actually starting game
    }

    // did the player request the version output?
    if (GetOptionsDB().Get<bool>("version")) {
        std::cout << "FreeOrionCH " << FreeOrionVersionString() << std::endl;
        return 0;   // quit without actually starting game
    }

    return NSApplicationMain(argc,  (const char **) argv);
}
