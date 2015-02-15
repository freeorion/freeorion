/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2007 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

#include <GG/Ogre/OgreGUI.h>

#include <GG/EventPump.h>

#include <OgreRoot.h>
#include <OgreRenderWindow.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include <GL/glew.h>
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <mach-o/dyld.h>
#else
#include <GL/glx.h>
#endif

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

using namespace GG;

namespace {
    struct CleanQuit {};

    class OgreModalEventPump : public ModalEventPump {
    public:
        OgreModalEventPump(const bool& done) : ModalEventPump(done) {}
        virtual void operator()() {
            GUI* gui = GUI::GetGUI();
            EventPumpState& state = State();
            Ogre::Root& root = Ogre::Root::getSingleton();
            while (!Done()) {
                Ogre::WindowEventUtilities::messagePump();
                LoopBody(gui, state, true, false);
                gui->HandleSystemEvents();
                if (!root.renderOneFrame())
                    break;
            }
        }
    };
}

OgreGUI::OgreGUI(Ogre::RenderWindow* window, Ogre::Root* root,
                 const boost::filesystem::path& config_file_path) :
    GUI(""),
    m_window(window),
    m_root(root),
    m_timer(),
    m_config_file_data()
{
    m_window->addListener(this);
    Ogre::WindowEventUtilities::addWindowEventListener(m_window, this);
    EnableMouseButtonDownRepeat(250, 15);
    EnableKeyPressRepeat(250, 15);

    if (boost::filesystem::exists(config_file_path)) {
        boost::filesystem::ifstream ifs(config_file_path);
        if (ifs) {
            Ogre::FileStreamDataStream file_stream(&ifs, false);
            m_config_file_data.bind(new Ogre::MemoryDataStream(file_stream));
        }
    }
}

OgreGUI::~OgreGUI() {
    Ogre::WindowEventUtilities::removeWindowEventListener(m_window, this);
    m_window->removeListener(this);
}

boost::shared_ptr<ModalEventPump> OgreGUI::CreateModalEventPump(bool& done)
{ return boost::shared_ptr<ModalEventPump>(new OgreModalEventPump(done)); }

unsigned int OgreGUI::Ticks() const
{ return m_timer.getMilliseconds(); }

X OgreGUI::AppWidth() const
{ return X(m_window->getWidth()); }

Y OgreGUI::AppHeight() const
{ return Y(m_window->getHeight()); }

const Ogre::SharedPtr<Ogre::DataStream>& OgreGUI::ConfigFileStream() const
{ return m_config_file_data; }

void OgreGUI::Exit(int code) {
    if (code == 0)
        throw CleanQuit();
    else
        std::exit(code);
}

OgreGUI* OgreGUI::GetGUI()
{ return dynamic_cast<OgreGUI*>(GUI::GetGUI()); }

void OgreGUI::RenderBegin() {}
void OgreGUI::RenderEnd() {}

void OgreGUI::Run() {
    Ogre::Root& root = Ogre::Root::getSingleton();
    Ogre::RenderSystem* active_renderer = root.getRenderSystem();
    assert(active_renderer);
    active_renderer->_initRenderTargets();
    root.clearEventTimes();
    try {
        bool done = false;
        OgreModalEventPump pump(done);
        pump();
    } catch (const CleanQuit&) {}
}

std::vector< std::string > OgreGUI::GetSupportedResolutions() const {
    std::vector<std::string> resolutions;
    Ogre::ConfigOptionMap& renderer_options = m_root->getRenderSystem()->getConfigOptions();

    for (Ogre::ConfigOptionMap::iterator it = renderer_options.begin(); it != renderer_options.end(); ++it) {
        // only concerned with video mode options
        if (it->first != "Video Mode")
            continue;

        for (unsigned int i = 0; i < it->second.possibleValues.size(); ++i) {
            resolutions.push_back(it->second.possibleValues[i]);
            if (resolutions.back().find_first_of("@") == std::string::npos)
                resolutions.back() += " @ 32";
        }
    }
    return resolutions;
}


void OgreGUI::HandleSystemEvents()
{ HandleSystemEventsSignal(); }

void OgreGUI::Enter2DMode() {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    using namespace Ogre;

    Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();

    // set-up matrices
    render_system->_setWorldMatrix(Matrix4::IDENTITY);
    render_system->_setViewMatrix(Matrix4::IDENTITY);
    render_system->_setProjectionMatrix(Matrix4::IDENTITY);

    glOrtho(0.0, Value(AppWidth()), Value(AppHeight()), 0.0, 0.0, Value(AppWidth()));

    // initialise render settings
    render_system->setLightingEnabled(false);
    render_system->_setDepthBufferParams(false, false);
    render_system->_setCullingMode(CULL_NONE);
    render_system->_setFog(FOG_NONE);
    render_system->_setColourBufferWriteEnabled(true, true, true, true);
    render_system->unbindGpuProgram(GPT_FRAGMENT_PROGRAM);
    render_system->unbindGpuProgram(GPT_VERTEX_PROGRAM);
    render_system->setShadingType(SO_GOURAUD);
    render_system->_setPolygonMode(PM_SOLID);

    Ogre::LayerBlendModeEx colour_blend_mode;
    colour_blend_mode.blendType = Ogre::LBT_COLOUR;
    colour_blend_mode.source1 = Ogre::LBS_TEXTURE;
    colour_blend_mode.source2 = Ogre::LBS_DIFFUSE;
    colour_blend_mode.operation = Ogre::LBX_MODULATE;
    Ogre::LayerBlendModeEx alpha_blend_mode;
    alpha_blend_mode.blendType = Ogre::LBT_ALPHA;
    alpha_blend_mode.source1 = Ogre::LBS_TEXTURE;
    alpha_blend_mode.source2 = Ogre::LBS_DIFFUSE;
    alpha_blend_mode.operation = Ogre::LBX_MODULATE;
    Ogre::TextureUnitState::UVWAddressingMode uvw_address_mode;
    uvw_address_mode.u = Ogre::TextureUnitState::TAM_CLAMP;
    uvw_address_mode.v = Ogre::TextureUnitState::TAM_CLAMP;
    uvw_address_mode.w = Ogre::TextureUnitState::TAM_CLAMP;

    // initialise texture settings
    render_system->_setTextureCoordCalculation(0, TEXCALC_NONE);
    render_system->_setTextureCoordSet(0, 0);
    render_system->_setTextureUnitFiltering(0, FO_LINEAR, FO_LINEAR, FO_POINT);
    render_system->_setTextureAddressingMode(0, uvw_address_mode);
    render_system->_setTextureMatrix(0, Matrix4::IDENTITY);
    render_system->_setAlphaRejectSettings(CMPF_ALWAYS_PASS, 0, false);
    render_system->_setTextureBlendMode(0, colour_blend_mode);
    render_system->_setTextureBlendMode(0, alpha_blend_mode);
    render_system->_disableTextureUnitsFrom(1);

    // enable alpha blending
    render_system->_setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);

    if (glBindBuffer) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    if (glUseProgram)
        glUseProgram(0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_FOG_COORDINATE_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
}

void OgreGUI::Exit2DMode() {
    glPopClientAttrib();
    glPopAttrib();
}

void OgreGUI::postRenderTargetUpdate(const Ogre::RenderTargetEvent& event) {
    RenderBegin();
    Render();
    RenderEnd();
}

void OgreGUI::windowMoved(Ogre::RenderWindow* window) {
    if (window != m_window)
        return;
    unsigned int width, height, depth;
    int left, top;
    window->getMetrics(width, height, depth, left, top);
    WindowMovedSignal(X(left), Y(top));
}

void OgreGUI::windowResized(Ogre::RenderWindow* window) {
    if (window != m_window)
        return;
    unsigned int width, height, depth;
    int left, top;
    window->getMetrics(width, height, depth, left, top);
    WindowResizedSignal(X(width), Y(height));
}

bool OgreGUI::windowClosing(Ogre::RenderWindow* window) {
    if (window != m_window)
        return true;
    WindowClosingSignal();
    return true;
}

void OgreGUI::windowClosed(Ogre::RenderWindow* window) {
    if (window != m_window)
        return;
    WindowClosedSignal();
    Exit(0);
}

void OgreGUI::windowFocusChange(Ogre::RenderWindow* window) {
    if (window != m_window)
        return;
    FocusChangedSignal();
    return;
}

