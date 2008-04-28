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

#include "../../include/OgreBulletCollisions.h"

#include "../../include/Shapes/OgreBulletCollisionsTriangleShape.h"

#include "BulletCollision/CollisionShapes/btTriangleShape.h"
#include "../../include/Utils/OgreBulletConverter.h"
#include "../../include/Debug/OgreBulletCollisionsDebugLines.h"

using namespace Ogre;
using namespace OgreBulletCollisions;

namespace OgreBulletCollisions
{
    // -------------------------------------------------------------------------
    TriangleCollisionShape::TriangleCollisionShape(
        const Ogre::Vector3 &p1, 
        const Ogre::Vector3 &p2, 
        const Ogre::Vector3 &p3):	
        CollisionShape()
    {
            mShape = new btTriangleShape(OgreBtConverter::to(p1), 
                                         OgreBtConverter::to(p2), 
                                         OgreBtConverter::to(p3)); 
    }
    // -------------------------------------------------------------------------
    TriangleCollisionShape::TriangleCollisionShape( 
        Ogre::Real p1X, Ogre::Real p1Y, Ogre::Real p1Z, 
        Ogre::Real p2X, Ogre::Real p2Y, Ogre::Real p2Z,  
        Ogre::Real p3X, Ogre::Real p3Y, Ogre::Real p3Z):
        CollisionShape()
    {
        mShape = new btTriangleShape(btVector3(p1X, p1Z, p1Z), 
                                     btVector3(p2X, p2Y, p2Z), 
                                     btVector3(p3X, p3Y, p3Z)); 
    }
    // -------------------------------------------------------------------------
    TriangleCollisionShape::~TriangleCollisionShape()
    {
    }
}

