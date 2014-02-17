// -*- C++ -*-
#ifndef _EntityRenderer_h_
#define _EntityRenderer_h_

#include <boost/shared_ptr.hpp>

#include <OgreSceneManager.h>

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
