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

#include "../../include/Debug/OgreBulletCollisionsDebugLines.h"
#include "../../include/Utils/OgreBulletConverter.h"

using namespace OgreBulletCollisions;
using namespace Ogre;

//------------------------------------------------------------------------------------------------
bool DebugLines::_materials_created = false;
//------------------------------------------------------------------------------------------------
DebugLines::DebugLines() : SimpleRenderable()
{
    mRenderOp.vertexData = new Ogre::VertexData();
    _drawn = false;

    if (!_materials_created)
    {
        MaterialPtr red = MaterialManager::getSingleton().create("OgreBulletCollisionsDebugLines/Disabled","OgreBulletCollisions");
        MaterialPtr green = MaterialManager::getSingleton().create("OgreBulletCollisionsDebugLines/Enabled","OgreBulletCollisions");
        MaterialPtr blue = MaterialManager::getSingleton().create("OgreBulletCollisionsDebugLines/Static","OgreBulletCollisions");

        red->setReceiveShadows(false);
        red->getTechnique(0)->setLightingEnabled(true);
        red->getTechnique(0)->getPass(0)->setSelfIllumination(1,0,0);

        green->setReceiveShadows(false);
        green->getTechnique(0)->setLightingEnabled(true);
        green->getTechnique(0)->getPass(0)->setSelfIllumination(0,1,0);

        blue->setReceiveShadows(false);
        blue->getTechnique(0)->setLightingEnabled(true);
        blue->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,1);

        _materials_created = true;
    }
    setCastShadows (false);
    this->setMaterial("OgreBulletCollisionsDebugLines/Enabled");
}


//------------------------------------------------------------------------------------------------
void DebugLines::clear()
{
    if (_drawn)
    {
        _drawn = false;
        _points.clear();
        delete mRenderOp.vertexData;

        mRenderOp.vertexData = new Ogre::VertexData();
    }
}
//------------------------------------------------------------------------------------------------
DebugLines::~DebugLines(void)
{
    clear();

    delete mRenderOp.vertexData;
}
//------------------------------------------------------------------------------------------------
void DebugLines::draw()
{
    if (_drawn || _points.empty()) 
        return;
    else 
        _drawn = true;

    // Initialization stuff
    mRenderOp.indexData = 0;
    mRenderOp.vertexData->vertexCount = _points.size();
    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.operationType = RenderOperation::OT_LINE_LIST;
    mRenderOp.useIndexes = false;

    Ogre::VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
    Ogre::VertexBufferBinding *bind = mRenderOp.vertexData->vertexBufferBinding;

    decl->addElement(0, 0, VET_FLOAT3, VES_POSITION);

    HardwareVertexBufferSharedPtr vbuf =
        HardwareBufferManager::getSingleton().createVertexBuffer(
        decl->getVertexSize(0),
        mRenderOp.vertexData->vertexCount,
        HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    bind->setBinding(0, vbuf);

    // Drawing stuff
    unsigned int size = (unsigned int)_points.size();
    Ogre::Vector3 vaabMin = _points[0];
    Ogre::Vector3 vaabMax = _points[0];

    float *prPos = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

    for(unsigned int i = 0; i < size; i++)
    {
        *prPos++ = _points[i].x;
        *prPos++ = _points[i].y;
        *prPos++ = _points[i].z;

        if (_points[i].x < vaabMin.x)
            vaabMin.x = _points[i].x;
        if (_points[i].y < vaabMin.y)
            vaabMin.y = _points[i].y;
        if (_points[i].z < vaabMin.z)
            vaabMin.z = _points[i].z;

        if (_points[i].x > vaabMax.x)
            vaabMax.x = _points[i].x;
        if (_points[i].y > vaabMax.y)
            vaabMax.y = _points[i].y;
        if (_points[i].z > vaabMax.z)
            vaabMax.z = _points[i].z;
    }

    vbuf->unlock();

    mBox.setExtents(vaabMin, vaabMax);
}
//------------------------------------------------------------------------------------------------
Real DebugLines::getSquaredViewDepth(const Camera *cam) const
{
    Vector3 vMin, vMax, vMid, vDist;
    vMin = mBox.getMinimum();
    vMax = mBox.getMaximum();
    vMid = ((vMin - vMax) * 0.5) + vMin;
    vDist = cam->getDerivedPosition() - vMid;

    return vDist.squaredLength();
}
//------------------------------------------------------------------------------------------------
Real DebugLines::getBoundingRadius() const
{
    return Math::Sqrt(std::max(mBox.getMaximum().squaredLength(), mBox.getMinimum().squaredLength()));
}
