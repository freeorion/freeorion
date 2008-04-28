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

#ifndef _OGREBULLETCOLLISIONS_CollisionWorld_H
#define _OGREBULLETCOLLISIONS_CollisionWorld_H

#include "OgreBulletCollisionsPreRequisites.h"


namespace OgreBulletCollisions
{
    // -------------------------------------------------------------------------
    // basic CollisionWorld
    class CollisionsWorld 
    {
    public:
        CollisionsWorld(Ogre::SceneManager *scn, const Ogre::AxisAlignedBox &bounds, bool init = true);
	    virtual ~CollisionsWorld();

        void addObject(Object *obj);

		/// Returns false if obj was not found.
		bool removeObject(Object *obj);

        void discreteCollide();

        bool isObjectregistered(Object *) const;
        Object *findObject(Ogre::SceneNode *node) const;
        Object *findObject(btCollisionObject *object) const;


        // mShowDebugContactPoints getter
        bool getShowDebugContactPoints() const { return mShowDebugContactPoints; }
        // mShowDebugContactPoints setter
        void setShowDebugContactPoints(bool show);
        // mShowDebugShapes getter
        bool getShowDebugShapes() const { return mShowDebugShapes; }
        // mShowDebugShapes setter
        void setShowDebugShapes(bool val);

        Ogre::SceneManager *getSceneManager() const {return mScnMgr;}
        btCollisionWorld *getBulletCollisionWorld() const {return mWorld;}

        void launchRay (CollisionRayResultCallback &ray, short int collisionFilterMask = -1);

    protected:
        btCollisionWorld*          mWorld;
        btCollisionDispatcher*     mDispatcher;
        btAxisSweep3*	           mBroadphase;
        Ogre::AxisAlignedBox       mBounds;

		btDefaultCollisionConfiguration	mDefaultCollisionConfiguration;

        std::deque<Object *>        mObjects;

        bool                        mShowDebugShapes;
        bool                        mShowDebugContactPoints;
        DebugLines *                mDebugContactPoints;

        Ogre::SceneManager *        mScnMgr;
    };
}
#endif //_OGREBULLETCOLLISIONS_CollisionWorld_H

