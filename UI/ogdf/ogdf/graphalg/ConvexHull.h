/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Declaration of doubly linked lists and iterators
 *
 * \author Gereon Bartel
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

#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_CONVEX_HULL_H
#define OGDF_CONVEX_HULL_H

#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/geometry.h>
#include <ogdf/internal/energybased/MultilevelGraph.h>
#include <vector>

namespace ogdf {

// all returned Polygons are clockwise (cw)
class OGDF_EXPORT ConvexHull {
private:
	bool sameDirection(const DPoint &start, const DPoint &end, const DPoint &s, const DPoint &e) const;

	// calculates a convex hull very quickly but only works with cross-free Polygons!
	DPolygon conv(const DPolygon &poly) const;

	// Calculates the Part of the convex hull that is left of line start-end
	// /a points should only contain points that really are left of the line.
	void leftHull(std::vector<DPoint> points, DPoint &start, DPoint &end, DPolygon &hullPoly) const;


public:
	ConvexHull();
	~ConvexHull();

	DPoint calcNormal(const DPoint &start, const DPoint &end) const;
	double leftOfLine(const DPoint &normal, const DPoint &point, const DPoint &pointOnLine) const;

	DPolygon call(std::vector<DPoint> points) const;
	DPolygon call(GraphAttributes &GA) const;
	DPolygon call(MultilevelGraph &MLG) const;

};

} // namespace ogdf

#endif
