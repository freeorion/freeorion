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

#include "../../include/Shapes/OgreBulletCollisionsTrimeshShape.h"
#include "../../include/Debug/OgreBulletCollisionsDebugLines.h"
#include "../../include/Utils/OgreBulletConverter.h"

using namespace Ogre;
using namespace OgreBulletCollisions;

namespace OgreBulletCollisions
{
    // -------------------------------------------------------------------------
    TriangleMeshCollisionShape::TriangleMeshCollisionShape(
        Vector3        *vertices, 
        unsigned int vertexCount, 
        unsigned int *indices, 
        unsigned int indexCount) :	
        CollisionShape(),
        mTriMesh(0)
    {
        mTriMesh = new btTriangleMesh();

        unsigned int numFaces = indexCount / 3;

        btVector3    vertexPos[3];
        for (size_t n = 0; n < numFaces; ++n)
        {
            for (unsigned int i = 0; i < 3; ++i)
            {
                const Vector3 &vec = vertices[*indices];
                vertexPos[i][0] = vec.x;
                vertexPos[i][1] = vec.y;
                vertexPos[i][2] = vec.z;
                *indices++;
            }

            mTriMesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2]);
        }

		const bool useQuantizedAABB = true;
        mShape = new btBvhTriangleMeshShape(mTriMesh, useQuantizedAABB);

    }
    // -------------------------------------------------------------------------
    TriangleMeshCollisionShape::~TriangleMeshCollisionShape()
    {
    }
    // -------------------------------------------------------------------------
    bool TriangleMeshCollisionShape::drawWireFrame(DebugLines *wire) const
    {
        const int numTris = mTriMesh->getNumTriangles ();
        if (numTris > 0)
        {
			/*
            for (int currTriIdx = 0; numTris > currTriIdx; currTriIdx++)
            {
                btMyTriangle myTri = mTriMesh->getTriangle (currTriIdx);

                wire->addLine (BtOgreConverter::to(myTri.m_vert0), BtOgreConverter::to(myTri.m_vert1));
                wire->addLine (BtOgreConverter::to(myTri.m_vert1), BtOgreConverter::to(myTri.m_vert2));
                wire->addLine (BtOgreConverter::to(myTri.m_vert2), BtOgreConverter::to(myTri.m_vert0));

                
                //wire->addPoint (BtOgreConverter::to(myTri.m_vert0));
                //wire->addPoint (BtOgreConverter::to(myTri.m_vert1));
                //wire->addPoint (BtOgreConverter::to(myTri.m_vert2));
            }
            return true
			;*/
        }
        return false;
    }
}

