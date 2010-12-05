/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief implementation of class CCLayoutPackModule.
 * 
 * \author Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 *
 * Copyright (C). All rights reserved.
 * See README.txt in the root directory of the OGDF installation for details.
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/

#include <ogdf/module/CCLayoutPackModule.h>


namespace ogdf {

template<class POINT>
bool CCLayoutPackModule::checkOffsetsTP(
	const Array<POINT> &box,
	const Array<POINT> &offset)
{
	OGDF_ASSERT(box.size() == offset.size());
	const int n = box.size();

	for (int i = 0; i < n; ++i)
	{
		typename POINT::numberType xl = offset[i].m_x;
		typename POINT::numberType xr = xl + box[i].m_x;
		typename POINT::numberType yb = offset[i].m_y;
		typename POINT::numberType yt = yb + box[i].m_y;

		OGDF_ASSERT(xl <= xr && yb <= yt);

		for (int j = i+1; j < n; ++j)
		{
			typename POINT::numberType xl2 = offset[j].m_x;
			typename POINT::numberType xr2 = xl2 + box[j].m_x; 
			typename POINT::numberType yb2 = offset[j].m_y;
			typename POINT::numberType yt2 = yb2 + box[j].m_y;

			if (xr2 > xl && xl2 < xr && yt2 > yb && yb2 < yt)
				return false;
		}
	}

	return true;
}

bool CCLayoutPackModule::checkOffsets(const Array<DPoint> &box,
	const Array<DPoint> &offset)
{
	return checkOffsetsTP(box,offset);
}

bool CCLayoutPackModule::checkOffsets(const Array<IPoint> &box,
	const Array<IPoint> &offset)
{
	return checkOffsetsTP(box,offset);
}


} // end namespace ogdf
