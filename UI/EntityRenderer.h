// -*- C++ -*-
#ifndef _EntityRenderer_h_
#define _EntityRenderer_h_

#include <OgrePrerequisites.h>

#include <boost/shared_ptr.hpp>

#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

// include OpenGL headers
#if defined(__APPLE__) && defined(__MACH__)
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
#endif


namespace Ogre {
    class Entity;
    class SceneManager;
}

namespace GG {
    class Texture;
}

class EntityRenderer {
public:
    EntityRenderer(Ogre::SceneManager* scene_manager);
    ~EntityRenderer();

    boost::shared_ptr<GG::Texture> GetTexture(Ogre::Entity* entity, Ogre::uint8 render_queue_group);
    void FreeTexture(const std::string& name);

    static EntityRenderer& Instance();

private:
    struct Impl;
    Impl* m_impl;

    static EntityRenderer* s_instance;
};

#endif
