//This main is compiled only for Mac OS X builds

#import <Cocoa/Cocoa.h>

#include "chmain.h"
#include <iostream>

int main(int argc, char *argv[])
{
    if (mainConfigOptionsSetup(argc, argv) != 0) {
        std::cerr << "main() failed config." << std::endl;
        return 1;
    }
        
    return NSApplicationMain(argc,  (const char **) argv);
}
