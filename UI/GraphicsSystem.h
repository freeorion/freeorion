#ifndef GRAPHICSSYSTEM_H
#define GRAPHICSSYSTEM_H

#include <GG/PtRect.h>

#include <vector>
#include <string>

// GrphicsSystem is a thin wrapper around the library used for creating the window and managind video modes, etc.
// The current implementation is based on SDL
class GraphicsSystem {
public:
    virtual std::vector<std::string> GetSupportedResolutions() const = 0;
    virtual GG::Pt CurrentResolution() const = 0;
    virtual ~GraphicsSystem(){}

    static GraphicsSystem* CreateGraphicsSystem();
protected:
    GraphicsSystem();
private:
    GraphicsSystem (const GraphicsSystem& other); //No copy
    GraphicsSystem& operator= (const GraphicsSystem& other); //No assign
};

#endif // GRAPHICSSYSTEM_H
