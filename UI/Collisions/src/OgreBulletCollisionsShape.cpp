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

#include "../include/OgreBulletCollisionsShape.h"
#include "../include/Utils/OgreBulletConverter.h"
#include "../include/Debug/OgreBulletCollisionsDebugLines.h"

using namespace Ogre;
using namespace OgreBulletCollisions;

namespace OgreBulletCollisions
{
    // -------------------------------------------------------------------------
    CollisionShape::CollisionShape()
        :	
        mShape(0)
    {
    }
    // -------------------------------------------------------------------------
    CollisionShape::~CollisionShape()
    {
       delete mShape;
    }
    // -------------------------------------------------------------------------
    bool CollisionShape::drawWireFrame(DebugLines *wire, const Vector3 &pos, const Quaternion &quat) const
    {   
        if (mShape->isConvex ())
            return drawConvexWireFrame (wire, pos, quat);

        return false;
    }
    // -------------------------------------------------------------------------
    bool CollisionShape::drawConvexWireFrame(DebugLines *wire, const Vector3 &pos, const Quaternion &quat) const
    {   
        assert (mShape->isConvex ());
        
        const btConvexShape * const s = static_cast <btConvexShape *> (mShape);

        Vector3 lastVec;
        bool sideBeginning;

        #define getVertex(X,Y,Z) BtOgreConverter::to(s->localGetSupportingVertex (btVector3(X,Y,Z)))

        const bool hasVecTransform = (pos != Vector3::ZERO);
        const bool hasQuatTransform = (quat != Quaternion::IDENTITY);
        const bool hasTransform = (hasVecTransform) || (hasQuatTransform);

        Vector3 curVec;
        size_t i = 0;
        const int subDivisionCount = 1;
        const float subDivide = 1.0f / subDivisionCount;
        for (int x = -subDivisionCount; x <= subDivisionCount; x++)
        {
            for (int y = -subDivisionCount; y <= subDivisionCount; y++)
            {
                sideBeginning = true;
                for (int z = -subDivisionCount; z <= subDivisionCount; z++)
                {
                    curVec = getVertex(x*subDivide, y*subDivide, z*subDivide);
                    if (hasTransform)
                    {
                        // apply transformation
                        if (hasQuatTransform) 
                            curVec = quat * curVec;
                        if (hasVecTransform) 
                            curVec += pos;
//                        if (hasVecTransform) 
//                            curVec -= pos;
//                        if (hasQuatTransform) 
//                            curVec = quat * curVec;
                    }

                    if (sideBeginning)
                        sideBeginning = false;
                    else
                        wire->addLine (lastVec, curVec);
                    lastVec = curVec;

                    i++;
                }
            }
        }


        for (int x = -subDivisionCount; x <= subDivisionCount; x++)
        {
            for (int z = -subDivisionCount; z <= subDivisionCount; z++)
            {
                sideBeginning = true;
                for (int y = -subDivisionCount; y <= subDivisionCount; y++)
                {
                    curVec = getVertex(x*subDivide, y*subDivide, z*subDivide);
                    if (hasTransform)
                    {
                        // apply transformation
                        if (hasVecTransform) 
                            curVec -= pos;
                        if (hasQuatTransform) 
                            curVec = quat * curVec;
                    }

                    if (sideBeginning)
                        sideBeginning = false;
                    else
                        wire->addLine (lastVec, curVec);
                    lastVec = curVec;

                    i++;
                }
            }
        }



        for (int z = -subDivisionCount; z <= subDivisionCount; z++)
        {
            for (int y = -subDivisionCount; y <= subDivisionCount; y++)
            {
                sideBeginning = true;
                for (int x = -subDivisionCount; x <= subDivisionCount; x++)
                {
                    curVec = getVertex(x*subDivide, y*subDivide, z*subDivide);
                    if (hasTransform)
                    {
                        // apply transformation
                        if (hasVecTransform) 
                            curVec -= pos;
                        if (hasQuatTransform) 
                            curVec = quat * curVec;
                    }

                    if (sideBeginning)
                        sideBeginning = false;
                    else
                        wire->addLine (lastVec, curVec);
                    lastVec = curVec;

                    i++;
                }
            }
        }
#undef getVertex

        return true;
    }
}

