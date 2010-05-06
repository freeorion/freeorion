/*-------------------------------------------------------------------------------------
Copyright (c) 2006 John Judnich
Modified 2008 by Erik Hjortsberg (erik.hjortsberg@iteam.se)

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------------*/

// RenderedTexture is a modified version of the ImpostorTexture class from the
// PagedGeometry code.

#include "EntityRenderer.h"

#include "../UI/ClientUI.h"
#include "../util/Directories.h"

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreQuaternion.h>
#include <OgreRenderTexture.h>
#include <OgreRoot.h>
#include <OgreSubEntity.h>
#include <OgreTextureManager.h>
#include <OgreTimer.h>
#include <OgreVector3.h>

#include <GG/Texture.h>


namespace {
    enum ImpostorBlendMode {
	ALPHA_REJECT_IMPOSTOR,
	ALPHA_BLEND_IMPOSTOR
    };

    const ImpostorBlendMode blendMode = ALPHA_BLEND_IMPOSTOR;
    const Ogre::uint32 textureSize = 128;

    std::string generateEntityKey(Ogre::Entity *entity)
    {
	Ogre::StringUtil::StrStreamType entityKey;
	entityKey << entity->getMesh()->getName();
	for (Ogre::uint32 i = 0; i < entity->getNumSubEntities(); ++i){
            entityKey << "-" << entity->getSubEntity(i)->getMaterialName();
	}
	return entityKey.str();
    }

    std::string removeInvalidCharacters(std::string s)
    {
        Ogre::StringUtil::StrStreamType s2;

        for (std::size_t i = 0; i < s.size(); ++i) {
            char c = s[i];
            if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' ||
                c == '\"' || c == '<' || c == '>' || c == '|') {
                s2 << '-';
            } else {
                s2 << c;
            }
        }

        return s2.str();
    }
}

////////////////////////////////////////////////////////////
// RenderedTexture
////////////////////////////////////////////////////////////
class RenderedTexture
{
public:
    ~RenderedTexture();

    boost::shared_ptr<GG::Texture> GGTexture() const;

    /** Returns a pointer to an RenderedTexture for the specified entity. If
	one does not already exist, one will automatically be created.
    */
    static RenderedTexture*
    getTexture(Ogre::Entity* entity, Ogre::SceneManager* sceneMgr, Ogre::uint8 renderQueueGroup);

    /** Remove created texture, note that all of the ImposterTextures must be
	deleted at once, because there is no track if a texture is still being
	used by something else
    */
    static void removeTexture(RenderedTexture* Texture);

private:
    RenderedTexture(Ogre::Entity* entity, Ogre::SceneManager* sceneMgr, Ogre::uint8 renderQueueGroup);

    void renderTextures();

    Ogre::SceneManager* sceneMgr;

    Ogre::Entity* entity;
    std::string entityKey;

    Ogre::MaterialPtr material;
    Ogre::TexturePtr texture;

    boost::shared_ptr<GG::Texture> ggTexture;

    Ogre::Real entityRadius;
    Ogre::Vector3 entityCenter;

    Ogre::uint8 renderQueueGroup;

    static std::string getUniqueID(const std::string& prefix)
        { return prefix + Ogre::StringConverter::toString(++s_guid); }

    static unsigned int s_guid;
    static std::map<std::string, RenderedTexture*> selfList;
};

unsigned int RenderedTexture::s_guid = 0;
std::map<std::string, RenderedTexture*> RenderedTexture::selfList;

RenderedTexture::RenderedTexture(Ogre::Entity* entity,
                                 Ogre::SceneManager* sceneMgr,
                                 Ogre::uint8 renderQueueGroup) :
    sceneMgr(sceneMgr),
    entity(entity),
    entityKey(generateEntityKey(entity)),
    renderQueueGroup(renderQueueGroup)
{
    //Add self to list of RenderedTexture's
    typedef std::pair<std::string, RenderedTexture*> ListItem;
    selfList.insert(ListItem(entityKey, this));

    // TODO: use bounding sphere
    //Note - this radius calculation assumes the object is somewhat rounded (like trees/rocks/etc.)
    Ogre::Real tmp;
    Ogre::AxisAlignedBox boundingBox = entity->getBoundingBox();
    entityRadius = boundingBox.getMaximum().x - boundingBox.getCenter().x;
    tmp = boundingBox.getMaximum().y - boundingBox.getCenter().y;
    if (tmp > entityRadius)
        entityRadius = tmp;
    tmp = boundingBox.getMaximum().z - boundingBox.getCenter().z;
    if (tmp > entityRadius)
        entityRadius = tmp;

    entityCenter = boundingBox.getCenter();

    //Render impostor textures
    renderTextures();

    //Set up material
    material =
        Ogre::MaterialManager::getSingleton().create(
            getUniqueID("RenderedEntityMaterial"), "EntityRenderer");

    Ogre::Material* m = material.getPointer();
    Ogre::Pass* p = m->getTechnique(0)->getPass(0);

    p->createTextureUnitState(texture->getName());

    p->setLightingEnabled(false);
    m->setReceiveShadows(false);

    if (blendMode == ALPHA_REJECT_IMPOSTOR){
        p->setAlphaRejectSettings(Ogre::CMPF_GREATER_EQUAL, 128);
        //p->setAlphaRejectSettings(CMPF_GREATER_EQUAL, 64);
    } else if (blendMode == ALPHA_BLEND_IMPOSTOR){
        p->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
        p->setDepthWriteEnabled(false);  
    }
}

RenderedTexture::~RenderedTexture()
{
    //Delete texture
    assert(!texture.isNull());
    std::string texName(texture->getName());

    texture.setNull();
    if (Ogre::TextureManager::getSingletonPtr())
        Ogre::TextureManager::getSingleton().remove(texName);

    //Delete material
    assert (!material.isNull());
    std::string matName(material->getName());

    material.setNull();
    if (Ogre::MaterialManager::getSingletonPtr())
        Ogre::MaterialManager::getSingleton().remove(matName);

    //Remove self from list of RenderedTextures
    selfList.erase(entityKey);
}

boost::shared_ptr<GG::Texture> RenderedTexture::GGTexture() const
{ return ggTexture; }

void RenderedTexture::renderTextures()
{
    //Set up RTT texture
    Ogre::TexturePtr renderTexture;
    if (renderTexture.isNull()) {
        renderTexture = Ogre::TextureManager::getSingleton().createManual(
            getUniqueID("RenderedEntityMaterial"), "EntityRenderer",
            Ogre::TEX_TYPE_2D, textureSize, textureSize, 0,
            Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET, 0);
    }
    renderTexture->setNumMipmaps(0);

    //Set up render target
    Ogre::RenderTexture* renderTarget = renderTexture->getBuffer()->getRenderTarget(); 
    renderTarget->setAutoUpdated(false);

    //Set up camera
    Ogre::SceneNode* camNode = sceneMgr->getSceneNode("EntityRenderer::cameraNode");
    Ogre::Camera* renderCamera = sceneMgr->createCamera(getUniqueID("EntityRendererCam"));
    camNode->attachObject(renderCamera);
    renderCamera->setLodBias(1000.0f);

    Ogre::Viewport* renderViewport = renderTarget->addViewport(renderCamera);
    renderViewport->setOverlaysEnabled(false);
    renderViewport->setClearEveryFrame(true);
    renderViewport->setShadowsEnabled(false);
    renderViewport->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f, 0.0f));

    //Set up scene node
    Ogre::SceneNode* node = sceneMgr->getSceneNode("EntityRenderer::renderNode");

    Ogre::SceneNode* oldSceneNode = entity->getParentSceneNode();
    if (oldSceneNode)
        oldSceneNode->detachObject(entity);
    node->attachObject(entity);
    node->setPosition(-entityCenter);

    //Set up camera FOV
    const Ogre::Real objDist = entityRadius * 100;
    const Ogre::Real nearDist = objDist - (entityRadius + 1); 
    const Ogre::Real farDist = objDist + (entityRadius + 1);

    renderCamera->setAspectRatio(1.0f);
    renderCamera->setFOVy(Ogre::Math::ATan(2.0 * entityRadius / objDist));
    renderCamera->setNearClipDistance(nearDist);
    renderCamera->setFarClipDistance(farDist);

    //Disable mipmapping (without this, masked textures look bad)
    Ogre::MaterialManager* mm = Ogre::MaterialManager::getSingletonPtr();
    Ogre::FilterOptions oldMinFilter = mm->getDefaultTextureFiltering(Ogre::FT_MIN);
    Ogre::FilterOptions oldMagFilter = mm->getDefaultTextureFiltering(Ogre::FT_MAG);
    Ogre::FilterOptions oldMipFilter = mm->getDefaultTextureFiltering(Ogre::FT_MIP);
    mm->setDefaultTextureFiltering(Ogre::FO_POINT, Ogre::FO_LINEAR,Ogre:: FO_NONE);

    //Disable fog
    Ogre::FogMode oldFogMode = sceneMgr->getFogMode();
    Ogre::ColourValue oldFogColor = sceneMgr->getFogColour();
    Ogre::Real oldFogDensity = sceneMgr->getFogDensity();
    Ogre::Real oldFogStart = sceneMgr->getFogStart();
    Ogre::Real oldFogEnd = sceneMgr->getFogEnd();
    sceneMgr->setFog(Ogre::FOG_NONE);

    // Get current status of the queue mode
    Ogre::SceneManager::SpecialCaseRenderQueueMode OldSpecialCaseRenderQueueMode =
        sceneMgr->getSpecialCaseRenderQueueMode();
    //Only render the entity
    sceneMgr->setSpecialCaseRenderQueueMode(Ogre::SceneManager::SCRQM_INCLUDE); 
    sceneMgr->addSpecialCaseRenderQueue(renderQueueGroup);

    Ogre::uint8 oldRenderQueueGroup = entity->getRenderQueueGroup();
    entity->setRenderQueueGroup(renderQueueGroup);
    bool oldVisible = entity->getVisible();
    entity->setVisible(true);
    float oldMaxDistance = entity->getRenderingDistance();
    entity->setRenderingDistance(0);

    //Calculate the filename hash used to uniquely identity this render
    std::string strKey = entityKey;
    char key[32] = {0};
    Ogre::uint32 i = 0;
    for (std::string::const_iterator it = entityKey.begin(); it != entityKey.end(); ++it) {
        key[i] ^= *it;
        i = (i+1) % sizeof(key);
    }
    for (i = 0; i < sizeof(key); ++i)
        key[i] = (key[i] % 26) + 'A';

    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
        GetUserDir().string(), "FileSystem", "BinFolder");
    std::string fileNamePNG =
        "Rendered." + std::string(key, sizeof(key)) + '.' +
        Ogre::StringConverter::toString(textureSize) + ".png";

    //Attempt to load the pre-render file if allowed
    bool needsRegen = false;
    if (!needsRegen) {
        try{
            texture = Ogre::TextureManager::getSingleton().load(
                fileNamePNG, "BinFolder", Ogre::TEX_TYPE_2D, 0);
        } catch (...) {
            needsRegen = true;
        }
    }

    if (needsRegen) {
        //If this has not been pre-rendered, do so now

        //Position camera
        camNode->setPosition(0, 0, 0);
        // TODO camNode->setOrientation(Quaternion(yaw, Vector3::UNIT_Y) * Quaternion(-pitch, Vector3::UNIT_X));
        camNode->translate(Ogre::Vector3(0, 0, objDist), Ogre::Node::TS_LOCAL);
						
        renderTarget->update();

        //Save RTT to file
        renderTarget->writeContentsToFile((GetUserDir() / fileNamePNG).string());

        //Load the render into the appropriate texture view
        texture = Ogre::TextureManager::getSingleton().load(fileNamePNG, "BinFolder", Ogre::TEX_TYPE_2D, 0);

        ggTexture = ClientUI::GetTexture(GetUserDir() / fileNamePNG);
    }

    entity->setVisible(oldVisible);
    entity->setRenderQueueGroup(oldRenderQueueGroup);
    entity->setRenderingDistance(oldMaxDistance);
    sceneMgr->removeSpecialCaseRenderQueue(renderQueueGroup);
    // Restore original state
    sceneMgr->setSpecialCaseRenderQueueMode(OldSpecialCaseRenderQueueMode); 

    //Re-enable mipmapping
    mm->setDefaultTextureFiltering(oldMinFilter, oldMagFilter, oldMipFilter);

    //Re-enable fog
    sceneMgr->setFog(oldFogMode, oldFogColor, oldFogDensity, oldFogStart, oldFogEnd);

    //Delete camera
    renderTarget->removeViewport(0);
    renderCamera->getSceneManager()->destroyCamera(renderCamera);

    //Delete scene node
    node->detachAllObjects();
    if (oldSceneNode)
        oldSceneNode->attachObject(entity);

    //Delete RTT texture
    assert(!renderTexture.isNull());
    std::string texName2(renderTexture->getName());

    renderTexture.setNull();
    if (Ogre::TextureManager::getSingletonPtr())
        Ogre::TextureManager::getSingleton().remove(texName2);
}

void RenderedTexture::removeTexture(RenderedTexture* texture)
{
    //Search for an existing impostor texture, in case it was already deleted
    for (std::map<std::string, RenderedTexture*>::iterator it = selfList.begin();
         it != selfList.end();
         ++it) {
        if (it->second == texture) {
            delete texture;
            break;
        }
    }
}

RenderedTexture* RenderedTexture::getTexture(Ogre::Entity* entity_,
                                             Ogre::SceneManager* sceneMgr,
                                             Ogre::uint8 renderQueueGroup)
{
    //Search for an existing impostor texture for the given entity
    std::string entityKey = generateEntityKey(entity_);
    std::map<std::string, RenderedTexture*>::iterator iter;
    iter = selfList.find(entityKey);

    if (iter != selfList.end())
        return iter->second;
    else
        return new RenderedTexture(entity_, sceneMgr, renderQueueGroup);
}


////////////////////////////////////////////////////////////
// EntityRenderer::Impl
////////////////////////////////////////////////////////////
struct EntityRenderer::Impl
{
    Impl(Ogre::SceneManager* scene_manager) :
        m_scene_manager(scene_manager)
        {
            m_scene_manager->getRootSceneNode()->createChildSceneNode("EntityRenderer::renderNode");
            m_scene_manager->getRootSceneNode()->createChildSceneNode("EntityRenderer::cameraNode");
            Ogre::ResourceGroupManager::getSingleton().createResourceGroup("EntityRenderer");

            ++s_instances;
        }

    ~Impl()
        {
            m_scene_manager->destroySceneNode("EntityRenderer::renderNode");
            m_scene_manager->destroySceneNode("EntityRenderer::cameraNode");
            Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup("EntityRenderer");

            --s_instances;
        }

    Ogre::SceneManager* m_scene_manager;

    static unsigned int s_instances;
};
unsigned int EntityRenderer::Impl::s_instances = 0;


////////////////////////////////////////////////////////////
// EntityRenderer
////////////////////////////////////////////////////////////
EntityRenderer* EntityRenderer::s_instance = 0;

EntityRenderer::EntityRenderer(Ogre::SceneManager* scene_manager) :
    m_impl(new Impl(scene_manager))
{ s_instance = this; }

EntityRenderer::~EntityRenderer()
{ delete m_impl; }

boost::shared_ptr<GG::Texture> EntityRenderer::GetTexture(Ogre::Entity* entity, Ogre::uint8 render_queue_group)
{
    RenderedTexture* rt = RenderedTexture::getTexture(entity, m_impl->m_scene_manager, render_queue_group);
    assert(rt);
    return rt->GGTexture();
}

void EntityRenderer::FreeTexture(const std::string& name)
{
    // TODO
}

EntityRenderer& EntityRenderer::Instance()
{
    assert(Impl::s_instances == 1u);
    return *s_instance;
}
