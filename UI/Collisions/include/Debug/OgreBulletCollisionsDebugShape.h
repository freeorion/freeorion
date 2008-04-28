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
#ifndef _OgreBulletCollisions_DEBUGOBJECT_H_
#define _OgreBulletCollisions_DEBUGOBJECT_H_

#include "../OgreBulletCollisionsPreRequisites.h"

#include "OgreBulletCollisionsDebugLines.h"

namespace OgreBulletCollisions
{
   
	//------------------------------------------------------------------------------------------------
	class  DebugCollisionShape : public DebugLines
	{
	public:
		enum Mode
		{
			Mode_Unknown,
			Mode_Enabled,
			Mode_Disabled,
			Mode_Static
        };

	public:
        /** DebugCollisionShape ctor
        */
        DebugCollisionShape(CollisionShape * shape, 
                            DebugCollisionShape::Mode mode = DebugCollisionShape::Mode_Enabled);
		/** ~DebugCollisionShape ctor
		*/
        virtual ~DebugCollisionShape();
        // mStatemode getter
        DebugCollisionShape::Mode getStatemode() const { return mStatemode; }
        // mStatemode setter
        void setStatemode(DebugCollisionShape::Mode val);
        // mIsVisual getter
        bool getIsVisual() const;
        // mIsVisual setter
        void setIsVisual(bool val);

	protected:
        DebugCollisionShape::Mode mStatemode;
        bool                      mIsVisual;


	};
	//------------------------------------------------------------------------------------------------
	class  RayDebugShape:public DebugLines
	{
	public:
		RayDebugShape(const Ogre::Vector3& start,const Ogre::Vector3& direction,const Ogre::Real length);
		void setDefinition(const Ogre::Vector3& start,const Ogre::Vector3& direction,const Ogre::Real length);
		virtual ~RayDebugShape();
    };
    //------------------------------------------------------------------------------------------------
}

#endif //_OgreBulletCollisions_DEBUGOBJECT_H_


