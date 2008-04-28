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

#include "../include/OgreBulletCollisionsWorld.h"
#include "../include/Utils/OgreBulletConverter.h"

#include "../include/OgreBulletCollisionsObject.h"
#include "../include/Debug/OgreBulletCollisionsDebugShape.h"
#include "../include/OgreBulletCollisionsRay.h"


using namespace Ogre;
using namespace OgreBulletCollisions;

namespace OgreBulletCollisions
{
    // -------------------------------------------------------------------------
    CollisionsWorld::CollisionsWorld(SceneManager *scn, const AxisAlignedBox &bounds, bool init):
        mScnMgr(scn),
        mBounds(bounds),
        mShowDebugShapes(false),
        mShowDebugContactPoints(false),
        mDebugContactPoints(0)
	{
        mDispatcher = new btCollisionDispatcher(&mDefaultCollisionConfiguration);
        mBroadphase = new btAxisSweep3(
            OgreBtConverter::to(bounds.getMinimum()), 
            OgreBtConverter::to(bounds.getMaximum()));

        // if not called by a inherited class
        if (init)
		{

            mWorld = new btCollisionWorld(mDispatcher, mBroadphase, &mDefaultCollisionConfiguration);
		}
    }
    // -------------------------------------------------------------------------
    CollisionsWorld::~CollisionsWorld()
    {
        delete mWorld;
        delete mBroadphase;
        delete mDispatcher;
    }
    // -------------------------------------------------------------------------
    void CollisionsWorld::setShowDebugContactPoints(bool show)
    {
        if (show && !mShowDebugContactPoints)
        {
            assert (mDebugContactPoints == 0);

            mShowDebugContactPoints = true;
            return;
        }
        if (!show && mShowDebugContactPoints)
        {
            assert (mDebugContactPoints != 0);

            mShowDebugContactPoints = false;
            return;
        }
    }
    // -------------------------------------------------------------------------
    void CollisionsWorld::setShowDebugShapes(bool show)
    {
        if (show && !mShowDebugShapes)
        {
            //assert (mDebugShapes == 0);
            std::deque<Object*>::iterator it = mObjects.begin();
            while (it != mObjects.end())
            {
                (*it)->showDebugShape(show);
                ++it;
            }
            mShowDebugShapes = true;
            return;
        }
        if (!show && mShowDebugShapes)
        {
            //assert (mDebugShapes != 0);
            std::deque<Object*>::iterator it = mObjects.begin();
            while (it != mObjects.end())
            {
                (*it)->showDebugShape(show);
                ++it;
            }
            mShowDebugShapes = false;
            return;
        }
    }
    // -------------------------------------------------------------------------
    void CollisionsWorld::addObject(Object *obj)
    {
        mObjects.push_back (obj);
        mWorld->addCollisionObject(obj->getBulletObject());
    }
    //------------------------------------------------------------------------- 
	bool CollisionsWorld::removeObject(Object *obj)
    {
       std::deque<Object*>::iterator it = find(mObjects.begin(),mObjects.end(), obj);
       if (it == mObjects.end())
          return false;
       mObjects.erase(it);
       return true;
    }
    // -------------------------------------------------------------------------
    bool CollisionsWorld::isObjectregistered(Object *obj) const
    {
        std::deque<Object *>::const_iterator itRes = std::find(mObjects.begin(), mObjects.end(), obj);
        if (itRes != mObjects.end())
            return true;
        return false;
    }
    // -------------------------------------------------------------------------
    Object *CollisionsWorld::findObject(btCollisionObject *object) const
    {
        std::deque<Object *>::const_iterator it = mObjects.begin();
        while (it != mObjects.end())
        {
            if ((*it)->getBulletObject() == object)
                return (*it);
            ++it;
        }
        return 0;
    }
    // -------------------------------------------------------------------------
    Object *CollisionsWorld::findObject(SceneNode *node) const
    {
        std::deque<Object *>::const_iterator it = mObjects.begin();
        while (it != mObjects.end())
        {
            //if ((*it)->getParentNode() == node)
			if((*it)->getRootNode() == node)
                return (*it);
            ++it;
        }
        return 0;
    }
    // -------------------------------------------------------------------------
    void CollisionsWorld::discreteCollide()
    {
        mWorld->performDiscreteCollisionDetection();


        ///one way to draw all the contact points is iterating over contact manifolds / points:
        const unsigned int  numManifolds = mWorld->getDispatcher()->getNumManifolds();
        for (unsigned int i=0;i < numManifolds; i++)
        {
            btPersistentManifold* contactManifold = mWorld->getDispatcher()->getManifoldByIndexInternal(i);

            btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
            btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());

            contactManifold->refreshContactPoints(obA->getWorldTransform(),obB->getWorldTransform());

            const unsigned int numContacts = contactManifold->getNumContacts();
            for (unsigned int j = 0;j < numContacts; j++)
            {
                btManifoldPoint& pt = contactManifold->getContactPoint(j);

                if (mShowDebugContactPoints)
                {
                    btVector3 ptA = pt.getPositionWorldOnA();
                    btVector3 ptB = pt.getPositionWorldOnB();

                    mDebugContactPoints->addLine(ptA.x(),ptA.y(),ptA.z(),
                                                     ptB.x(),ptB.y(),ptB.z());
                }
            }
            //you can un-comment out this line, and then all points are removed
            //contactManifold->clearManifold();	
        }

        /*
        if (mShowDebugShapes)
        {
            std::deque<Object*>::iterator it = mObjects.begin();
            while (it != mObjects.end())
            {

                //(*it)->getBulletObject()->getWorldTransform().getOpenGLMatrix( m );
                mShowDebugDrawShapes->
                //GL_ShapeDrawer::drawOpenGL(m,objects[i].getCollisionShape(),btVector3(1,1,1),getDebugMode());

                ++it;
            }
        }
        */
    }

    // -------------------------------------------------------------------------
    void CollisionsWorld::launchRay(CollisionRayResultCallback &rayresult, short int collisionFilterMask)
    {
        mWorld->rayTest(
            OgreBtConverter::to(rayresult.getRayStartPoint()), 
            OgreBtConverter::to(rayresult.getRayEndPoint()), 
            *rayresult.getBulletRay (),
			collisionFilterMask);
    }
}

