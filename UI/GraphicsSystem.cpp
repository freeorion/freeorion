#include "GraphicsSystem.h"
#include "../../util/Logger.h"
#include <SDL2/SDL.h>

#include <boost/format.hpp>

#include <GG/PtRect.h>

class SDLGraphics: public GraphicsSystem {
    public:
        SDLGraphics():
        m_display_id(0){}

        virtual std::vector<std::string> GetSupportedResolutions() const {
            std::vector<std::string> mode_vec;

            unsigned valid_mode_count = SDL_GetNumDisplayModes(m_display_id);

            /* Check if our resolution is restricted */
            if ( valid_mode_count < 1 ) {
                Logger().errorStream() << "No valid resolutions found!?";
            } else {
                for (int i = 0; i < valid_mode_count; ++i) {
                    SDL_DisplayMode mode;
                    if (SDL_GetDisplayMode(m_display_id, i, &mode) != 0) {
                        SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
                    } else {
                        mode_vec.push_back(boost::io::str(boost::format("%1% x %2% @ %3%") % mode.w % mode.h % SDL_BITSPERPIXEL(mode.format)));
                    }
                }
            }

            return mode_vec;
        }

        virtual GG::Pt CurrentResolution() const {
            SDL_DisplayMode mode;
            SDL_GetCurrentDisplayMode(m_display_id, &mode);
            return GG::Pt(GG::X(mode.w), GG::Y(mode.h));
        }

    private:
        // TODO SDL: Support selecting used display
        int m_display_id;
};

GraphicsSystem::GraphicsSystem() {}

GraphicsSystem* GraphicsSystem::CreateGraphicsSystem() {
    return new SDLGraphics();
}
