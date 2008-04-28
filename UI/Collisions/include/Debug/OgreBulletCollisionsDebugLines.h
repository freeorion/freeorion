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
#ifndef _OgreBulletCollisions_DEBUGLines_H_
#define _OgreBulletCollisions_DEBUGLines_H_

#include "../OgreBulletCollisionsPreRequisites.h"

namespace OgreBulletCollisions
{
    //------------------------------------------------------------------------------------------------
    class  DebugLines:public Ogre::SimpleRenderable
    {
    public:
        DebugLines(void);
        ~DebugLines(void);

        void addLine (const Ogre::Vector3 &start,const Ogre::Vector3 &end)
        {
            clear ();

            _points.push_back (start);
            _points.push_back (end);
        }

        void addLine(Ogre::Real start_x, Ogre::Real start_y, Ogre::Real start_z, 
            Ogre::Real end_x, Ogre::Real end_y, Ogre::Real end_z)
        {
            addLine (Ogre::Vector3(start_x,start_y,start_z),
                Ogre::Vector3(end_x,end_y,end_z));
        }

        void addPoint (const Ogre::Vector3 &pt)
        {
            clear();

            _points.push_back(pt);
        }

        void addPoint (Ogre::Real x, Ogre::Real y, Ogre::Real z)
        {
            addPoint (Ogre::Vector3(x, y, z));
        }

        void draw ();
        void clear ();

        Ogre::Real getSquaredViewDepth (const Ogre::Camera *cam) const;
        Ogre::Real getBoundingRadius (void) const;

    protected:

        Vector3Vector _points;
        bool _drawn;

        static bool _materials_created;
    };
}
#endif //_OgreBulletCollisions_DEBUGLines_H_

