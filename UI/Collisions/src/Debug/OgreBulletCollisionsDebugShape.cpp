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

#include "../../include/OgreBulletCollisionsShape.h"
#include "../../include/Debug/OgreBulletCollisionsDebugShape.h"
#include "../../include/Utils/OgreBulletConverter.h"

using namespace OgreBulletCollisions;
using namespace Ogre;

//------------------------------------------------------------------------------------------------
DebugCollisionShape::DebugCollisionShape(CollisionShape *shape, DebugCollisionShape::Mode mode) 
{
    setStatemode(mode);
    // try to draw debug wire frame of the shape
    mIsVisual = shape->drawWireFrame (this);

    // if no success (not possible or not implemented
    // does not draw, hence saving a segfault
    if (mIsVisual)
        DebugLines::draw ();
}
//------------------------------------------------------------------------------------------------
void DebugCollisionShape::setStatemode(DebugCollisionShape::Mode mode)
{
	if (mode != mStatemode)
	{
		mStatemode = mode;
		switch(mStatemode)
		{
			case DebugCollisionShape::Mode_Enabled:
				setMaterial("OgreBulletCollisionsDebugLines/Enabled");
			break;

			case DebugCollisionShape::Mode_Disabled:
				setMaterial("OgreBulletCollisionsDebugLines/Disabled");

			break;

			case DebugCollisionShape::Mode_Static:
				setMaterial("OgreBulletCollisionsDebugLines/Static");
			break;
		}
	}
}
//------------------------------------------------------------------------------------------------
DebugCollisionShape::~DebugCollisionShape() 
{
}
//------------------------------------------------------------------------------------------------
bool DebugCollisionShape::getIsVisual() const
{
    return mIsVisual;
}
//------------------------------------------------------------------------------------------------
void DebugCollisionShape::setIsVisual( bool val )
{
    mIsVisual = val;
}
//------------------------------------------------------------------------------------------------
RayDebugShape::RayDebugShape(const Ogre::Vector3& start, 
							   const Ogre::Vector3& direction, 
							   const Ogre::Real length)
{
	const Ogre::Vector3 end (start + (direction.normalisedCopy() * length));
	addLine(start, end);

	draw();
}
//------------------------------------------------------------------------------------------------
void RayDebugShape::setDefinition(const Ogre::Vector3& start,
							const Ogre::Vector3& direction, 
							const Ogre::Real length)
{
	clear();

	const Ogre::Vector3 end (start + (direction.normalisedCopy() * length));
	addLine(start, end);

	draw();
}
//------------------------------------------------------------------------------------------------
RayDebugShape::~RayDebugShape()
{
}
