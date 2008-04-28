/***************************************************************************

This source file is part of OGREBULLET
(Object-oriented Graphics Rendering Engine Bullet Wrapper)
For the latest info, see http://www.ogre3d.org/phpBB2addons/viewforum.php?f=10

Copyright (c) 2007 tuan.kuranes@gmail.com



This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include "../include/OgreBulletCollisions.h"

#include "../include/OgreBulletCollisionsObject.h"
#include "../include/Debug/OgreBulletCollisionsDebugShape.h"

#include "../include/OgreBulletCollisionsObjectState.h"
#include "../include/OgreBulletCollisionsShape.h"
#include "../include/OgreBulletCollisionsWorld.h"

using namespace Ogre;
using namespace OgreBulletCollisions;

namespace OgreBulletCollisions
{
    //-----------------------------------------------------------------------
    const Ogre::String Object::mMovableType = "OgreBullet::Object";
    // -------------------------------------------------------------------------
    Object::Object(const String &name, CollisionsWorld *world, bool init)
        :	
        MovableObject(name),
        UserDefinedObject(),
        mWorld(world),
        mShape(0),
        mState(0),
        mRootNode(0),
        mBounds(Vector3::ZERO, Vector3::ZERO),
        mDebugShape(0),
        mShapeNode(0),
        mDebugNode(0)
    {
        if (init)
        {
            mObject = new btCollisionObject();
            mState = new ObjectState(this);
        }
    }
    // -------------------------------------------------------------------------
    Object::~Object()
    {
        if (mRootNode)
        {
            showDebugShape(false);
            mShapeNode->detachObject (this);
            mRootNode->removeAndDestroyChild (mShapeNode->getName ());
            //mRootNode->getParentSceneNode ()->removeAndDestroyChild (mRootNode->getName ());
        }

        getBulletCollisionWorld()->removeCollisionObject( mObject );
		getCollisionWorld()->removeObject(this);

        delete mObject;        
        delete mShape;
        delete mState;
        delete mDebugShape;
    }
    //-----------------------------------------------------------------------
    void Object::showDebugShape(bool show)
    {
        if (show && mDebugShape == 0 && mShape)
        {
            mDebugShape = new DebugCollisionShape(mShape);
            if (mDebugShape->getIsVisual ())
            {
                assert (mDebugNode == 0);
                mDebugNode = mRootNode->createChildSceneNode(mName + "DebugShape");
                mDebugNode->attachObject (mDebugShape);
            }
        }
        else if (mDebugShape)
        {
            if (mDebugShape->getIsVisual ())
            {
                assert (mDebugNode);
                mDebugNode->detachObject (mDebugShape->getName());
                mRootNode->removeAndDestroyChild (mDebugNode->getName());
                mDebugNode = 0;
            }
            assert (mDebugNode == 0);
            delete mDebugShape;
            mDebugShape = 0;
        }
    }
    // -------------------------------------------------------------------------
    void Object::setTransform(const btVector3 &pos, const btQuaternion &quat)
    { 
        mRootNode->setPosition(pos[0], pos[1], pos[2]);
        mRootNode->setOrientation(quat.getW(),quat.getX(), quat.getY(), quat.getZ());
    }
    // -------------------------------------------------------------------------
    void Object::setPosition(const btVector3 &pos)
    {
        mRootNode->setPosition(pos[0], pos[1], pos[2]);
    }
    // -------------------------------------------------------------------------
    void Object::setOrientation(const btQuaternion &quat)
    {   
        mRootNode->setOrientation(quat.getW(),quat.getX(), quat.getY(), quat.getZ());
    }
    // -------------------------------------------------------------------------
    void Object::setTransform(const btTransform& worldTrans)
    { 
        mRootNode->setPosition(worldTrans.getOrigin()[0], worldTrans.getOrigin()[1],worldTrans.getOrigin()[2]);
        mRootNode->setOrientation(worldTrans.getRotation().getW(),worldTrans.getRotation().getX(), worldTrans.getRotation().getY(), worldTrans.getRotation().getZ());
    }
    //-----------------------------------------------------------------------
    void Object::setShape(CollisionShape *shape, 
        const Vector3 &pos, 
        const Quaternion &quat)
    {
        mShape = shape;

        mRootNode = mWorld->getSceneManager()->getRootSceneNode()->createChildSceneNode(mName, pos, quat);
        mShapeNode = mRootNode->createChildSceneNode(mName + "Shape");
        mShapeNode->attachObject(this);

        mObject->setCollisionShape(shape->getBulletShape());
        showDebugShape(mWorld->getShowDebugShapes());       
    }
    // -------------------------------------------------------------------------
    //-----------------------------------------------------------------------
    void Object::_notifyAttached(Node* parent, bool isTagPoint)
    {
        MovableObject::_notifyAttached(parent,isTagPoint);
        if (parent)
        {
            Object* other_object = mWorld->findObject(static_cast<SceneNode*>(parent));
            if ((other_object) && (other_object != this))
            {
                static_cast<SceneNode*>(parent)->detachObject(other_object);

            }
            setPosition(parent->getPosition());
            setOrientation(parent->getOrientation());
        } 
    }
#if (OGRE_VERSION >=  ((1 << 16) | (5 << 8) | 0)) // must have at least shoggoth (1.5.0)
    //-----------------------------------------------------------------------
	void Object::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		//visitor->visit(this, 0, false);
	}
#endif
    //-----------------------------------------------------------------------
    const Ogre::String& Object::getMovableType() const
    {
        return mMovableType;
    }

    //-----------------------------------------------------------------------
    void Object::_notifyCurrentCamera(Camera* camera)
    {
    }

    //-----------------------------------------------------------------------
    const AxisAlignedBox& Object::getBoundingBox(void) const
    {
        return mBounds;
    }
    //-----------------------------------------------------------------------
    Real Object::getBoundingRadius(void) const
    {
        return Ogre::Real(0.0);
    }

    //-----------------------------------------------------------------------
    void Object::_updateRenderQueue(RenderQueue* queue)
    {
    }
}

