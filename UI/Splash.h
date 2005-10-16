// -*- C++ -*-
#ifndef _Splash_h_
#define _Splash_h_

#include <string>
#include <vector>

namespace GG {
    class StaticGraphic;
}

/** Loads a set of StaticGraphics that display the splash screen in chunks no larger than 512x512, to be friendly on
    minimal GL implementations. */
void LoadSplashGraphics(std::vector<std::vector<GG::StaticGraphic*> >& graphics);

inline std::pair<std::string, std::string> SplashRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _Splash_h_
