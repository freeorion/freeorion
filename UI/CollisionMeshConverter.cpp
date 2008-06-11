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
-----------------------------------------------------------------------------*/

#include "CollisionMeshConverter.h"

#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRenderable.h>
#include <OgreSceneNode.h>
#include <OgreSubEntity.h>
#include <OgreSubMesh.h>

#include <btBulletCollisionCommon.h>

#include <boost/cast.hpp>


//------------------------------------------------------------------------------------------------
CollisionMeshConverter::CollisionMeshConverter(Ogre::Entity* entity, const Ogre::Matrix4& transform) :
    mBoundRadius(-1),
    mBounds(Ogre::Vector3(-1, -1, -1)),
    mEntity(0),
    mNode(0),
    mVertexBuffer(),
    mIndexBuffer()
{
    AddEntity(entity, transform);
}

//------------------------------------------------------------------------------------------------
CollisionMeshConverter::CollisionMeshConverter(Ogre::Renderable* rend, const Ogre::Matrix4& transform) :
    mBoundRadius(-1),
    mBounds(Ogre::Vector3(-1, -1, -1)),
    mEntity(0),
    mNode(0),
    mTransform(transform),
    mVertexBuffer(),
    mIndexBuffer()
{
    Ogre::RenderOperation op;
    rend->getRenderOperation(op);
    AddVertexData(op.vertexData);
    if (op.useIndexes)
        AddIndexData(op.indexData);
}

//------------------------------------------------------------------------------------------------
Ogre::Real CollisionMeshConverter::GetRadius() const
{
    if (mBoundRadius == -1) {
        GetSize();
        mBoundRadius = std::max(mBounds.x, std::max(mBounds.y, mBounds.z)) * 0.5;
    }
    return mBoundRadius;
}

//------------------------------------------------------------------------------------------------
Ogre::Vector3 CollisionMeshConverter::GetSize() const
{
    const std::size_t vCount = GetVertexCount();
    if (mBounds == Ogre::Vector3(-1, -1, -1) && vCount) {
        const Ogre::Vector3 * const v = GetVertices();

        Ogre::Vector3 vmin(v[0]);
        Ogre::Vector3 vmax(v[0]);

        for (std::size_t j = 1; j < vCount; ++j) {
            vmin.x = std::min(vmin.x, v[j].x);
            vmin.y = std::min(vmin.y, v[j].y);
            vmin.z = std::min(vmin.z, v[j].z);

            vmax.x = std::max(vmax.x, v[j].x);
            vmax.y = std::max(vmax.y, v[j].y);
            vmax.z = std::max(vmax.z, v[j].z);
        }

        mBounds.x = vmax.x - vmin.x;
        mBounds.y = vmax.y - vmin.y;
        mBounds.z = vmax.z - vmin.z;
    }

    return mBounds;
}

//------------------------------------------------------------------------------------------------
const Ogre::Vector3* CollisionMeshConverter::GetVertices() const
{ return &mVertexBuffer[0]; }

//------------------------------------------------------------------------------------------------
std::size_t CollisionMeshConverter::GetVertexCount() const
{ return mVertexBuffer.size(); }

//------------------------------------------------------------------------------------------------
const unsigned int* CollisionMeshConverter::GetIndices() const
{ return &mIndexBuffer[0]; }

//------------------------------------------------------------------------------------------------
std::size_t CollisionMeshConverter::GetIndexCount() const
{ return mIndexBuffer.size(); }

//------------------------------------------------------------------------------------------------
std::pair<btTriangleMesh*, btBvhTriangleMeshShape*> CollisionMeshConverter::CollisionShape() const
{
    std::pair<btTriangleMesh*, btBvhTriangleMeshShape*> retval;
    assert(mVertexBuffer.size() && mIndexBuffer.size() >= 6 &&
           ("Mesh must have some vertices and at least 6 indices (2 triangles)"));

    btTriangleMesh* triMesh = new btTriangleMesh;

    std::vector<unsigned int>::const_iterator index_it = mIndexBuffer.begin();

    std::size_t numFaces = mIndexBuffer.size() / 3;
    btVector3 vertexPos[3];
    for (std::size_t n = 0; n < numFaces; ++n) {
        for (unsigned int i = 0; i < 3; ++i) {
            const Ogre::Vector3& vec = mVertexBuffer[*index_it++];
            vertexPos[i][0] = vec.x;
            vertexPos[i][1] = vec.y;
            vertexPos[i][2] = vec.z;
        }

        triMesh->addTriangle(vertexPos[0], vertexPos[1], vertexPos[2]);
    }

    const bool useQuantizedAABB = true;
    btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triMesh, useQuantizedAABB);

    retval.first = triMesh;
    retval.second = shape;

    mVertexBuffer.clear();
    mIndexBuffer.clear();

    return retval;
}

//------------------------------------------------------------------------------------------------
void CollisionMeshConverter::AddVertexData(const Ogre::VertexData* vertex_data,
                                           const Ogre::VertexData* blended_data,
                                           const Ogre::Mesh::IndexMap* indexMap)
{
    if (!vertex_data)
        return;

    const Ogre::VertexData* data = blended_data ? blended_data : vertex_data;

    const std::size_t prev_size = mVertexBuffer.size();
    mVertexBuffer.resize(mVertexBuffer.size() + data->vertexCount);

    // Get the positional buffer element
    const Ogre::VertexElement* posElem = data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);			
    Ogre::HardwareVertexBufferSharedPtr vbuf = data->vertexBufferBinding->getBuffer(posElem->getSource());
    const std::size_t vSize = vbuf->getVertexSize();

    unsigned char* vertex = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
    float* pReal;
    Ogre::Vector3*  curVertices = &mVertexBuffer[prev_size];
    const std::size_t vertexCount = data->vertexCount;
    for (std::size_t j = 0; j < vertexCount; ++j) {
        posElem->baseVertexPointerToElement(vertex, &pReal);
        vertex += vSize;

        curVertices->x = *pReal++;
        curVertices->y = *pReal++;
        curVertices->z = *pReal++;

        *curVertices = mTransform * *curVertices;

        ++curVertices;
    }
    vbuf->unlock();
}

//------------------------------------------------------------------------------------------------
void CollisionMeshConverter::AddIndexData(Ogre::IndexData* data, const std::size_t offset)
{
    const std::size_t prev_size = mIndexBuffer.size();
    mIndexBuffer.resize(mIndexBuffer.size() + data->indexCount);

    const std::size_t numTris = data->indexCount / 3;
    Ogre::HardwareIndexBufferSharedPtr ibuf = data->indexBuffer;
    const bool use32bitindexes = ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT;
    std::size_t index_offset = prev_size;

    if (use32bitindexes) {
        const unsigned int* pInt = static_cast<unsigned int*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        for (std::size_t k = 0; k < numTris; ++k) {
            mIndexBuffer[index_offset++] = offset + *pInt++;
            mIndexBuffer[index_offset++] = offset + *pInt++;
            mIndexBuffer[index_offset++] = offset + *pInt++;
        }
        ibuf->unlock();
    } else {
        const unsigned short* pShort = static_cast<unsigned short*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        for (std::size_t k = 0; k < numTris; ++k) {
            mIndexBuffer[index_offset++] = offset + static_cast<unsigned int>(*pShort++);
            mIndexBuffer[index_offset++] = offset + static_cast<unsigned int>(*pShort++);
            mIndexBuffer[index_offset++] = offset + static_cast<unsigned int>(*pShort++);
        }
        ibuf->unlock();
    }
}

//------------------------------------------------------------------------------------------------
void CollisionMeshConverter::AddEntity(Ogre::Entity* entity, const Ogre::Matrix4& transform)
{
    // Each entity added need to reset size and radius
    // next time getRadius and getSize are asked, they're computed.
    mBounds = Ogre::Vector3(-1, -1, -1);
    mBoundRadius = -1;

    mEntity = entity;
    mNode = boost::polymorphic_downcast<Ogre::SceneNode*>(mEntity->getParentNode());
    mTransform = transform;

    if (mEntity->getMesh()->sharedVertexData)
        AddVertexData(mEntity->getMesh()->sharedVertexData);

    for (std::size_t i = 0; i < mEntity->getNumSubEntities(); ++i) {
        Ogre::SubMesh* sub_mesh = mEntity->getSubEntity(i)->getSubMesh();

        if (!sub_mesh->useSharedVertices) {
            AddIndexData(sub_mesh->indexData, mVertexBuffer.size());
            AddVertexData(sub_mesh->vertexData);
        } else {
            AddIndexData(sub_mesh->indexData);
        }
    }
}

//------------------------------------------------------------------------------------------------
void CollisionMeshConverter::AddMesh(const Ogre::MeshPtr& mesh, const Ogre::Matrix4& transform)
{
    // Each entity added need to reset size and radius
    // next time GetRadius and GetSize are asked, they're computed.
    mBounds  = Ogre::Vector3(-1, -1, -1);
    mBoundRadius = -1;

    mTransform = transform;

    if (mesh->hasSkeleton()) {
        Ogre::LogManager::getSingleton().logMessage(
            "CollisionMeshConverter::AddMesh : Mesh " + mesh->getName() +
            " as skeleton but added to trimesh non animated");
    }

    if (mesh->sharedVertexData)
        AddVertexData(mesh->sharedVertexData);

    for (std::size_t i = 0; i < mesh->getNumSubMeshes(); ++i) {
        Ogre::SubMesh* sub_mesh = mesh->getSubMesh(i);

        if (!sub_mesh->useSharedVertices) {
            AddIndexData(sub_mesh->indexData, mVertexBuffer.size());
            AddVertexData(sub_mesh->vertexData);
        } else {
            AddIndexData(sub_mesh->indexData);
        }
    }
}
