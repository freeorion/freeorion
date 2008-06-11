// -*- C++ -*-
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
----------------------------------------------------------------------------- */
#ifndef _OgreBulletCollisionsMeshToShapeConverter_H_
#define _OgreBulletCollisionsMeshToShapeConverter_H_

#include <OgreMesh.h>


class btTriangleMesh;
class btBvhTriangleMeshShape;

class CollisionMeshConverter
{
public:
    CollisionMeshConverter(Ogre::Renderable* rend,
                           const Ogre::Matrix4& transform = Ogre::Matrix4::IDENTITY);
    CollisionMeshConverter(Ogre::Entity* entity,
                           const Ogre::Matrix4& transform = Ogre::Matrix4::IDENTITY);

    Ogre::Real              GetRadius() const;
    Ogre::Vector3           GetSize() const;
    const Ogre::Vector3*    GetVertices() const;
    std::size_t             GetVertexCount() const;
    const unsigned int*     GetIndices() const;
    std::size_t             GetIndexCount() const;

    std::pair<btTriangleMesh*, btBvhTriangleMeshShape*> CollisionShape() const;

private:
    void AddVertexData(const Ogre::VertexData* vertex_data, 
                       const Ogre::VertexData* blended_data = 0, 
                       const Ogre::Mesh::IndexMap* indexMap = 0);
    void AddIndexData(Ogre::IndexData* data, const std::size_t offset = 0);
    void AddEntity(Ogre::Entity* entity, const Ogre::Matrix4& transform);
    void AddMesh(const Ogre::MeshPtr& mesh, const Ogre::Matrix4& transform);

    mutable Ogre::Real    mBoundRadius;
    mutable Ogre::Vector3 mBounds;

    Ogre::Entity*                      mEntity;
    Ogre::SceneNode*                   mNode;
    Ogre::Matrix4                      mTransform;
    mutable std::vector<Ogre::Vector3> mVertexBuffer;
    mutable std::vector<unsigned int>  mIndexBuffer;
};

#endif
