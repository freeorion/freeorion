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

#ifndef _OGREBULLETCOLLISIONS_TriangleShape_H
#define _OGREBULLETCOLLISIONS_TriangleShape_H

#include "../OgreBulletCollisionsPreRequisites.h"

#include "../OgreBulletCollisionsShape.h"

namespace OgreBulletCollisions
{
    // -------------------------------------------------------------------------
    // basic TriangleShape
    class TriangleCollisionShape : public CollisionShape
    {
    public:
        TriangleCollisionShape(const Ogre::Vector3 &p1, const Ogre::Vector3 &p2, const Ogre::Vector3 &p3);	
        TriangleCollisionShape( 
            Ogre::Real p1X, Ogre::Real p1Y, Ogre::Real p1Z, 
            Ogre::Real p2X, Ogre::Real p2Y, Ogre::Real p2Z,  
            Ogre::Real p3X, Ogre::Real p3Y, Ogre::Real p3Z);

	    virtual ~TriangleCollisionShape();

    };
}
#endif //_OGREBULLETCOLLISIONS_TriangleShape_H

