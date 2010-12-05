/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class NearestRectangleFinder
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


#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_NEAREST_RECTANGLE_FINDER_H
#define OGDF_NEAREST_RECTANGLE_FINDER_H


#include <ogdf/basic/Array.h>
#include <ogdf/basic/geometry.h>


namespace ogdf {


//---------------------------------------------------------
// NearestRectangleFinder
// finds in a given set of rectangles for each point in a given
// set of points the nearest rectangle
//---------------------------------------------------------
class OGDF_EXPORT NearestRectangleFinder
{
public:
	struct RectRegion;
	struct PairRectDist;
	struct PairCoordId;

	NearestRectangleFinder(double mad = 20, double td = 5) {
		m_maxAllowedDistance = mad;
		m_toleranceDistance = td;
	}

	// the maximal allowed distance between a rectangle and a point
	// rectangles with a greater distance are not considered
	void maxAllowedDistance(double mad) { m_maxAllowedDistance = mad; }
	double maxAllowedDistance() const { return m_maxAllowedDistance; }

	// the tolerance in which rectangles are considered to be ambigous, i.e.
	// if the rectangle with the minimum distance to point p has distance mindist
	// and there is another rectangle with distance dist such that
	// dist <= minDist + toleranceDistance, we say that the closest rectangle is not unique.
	void toleranceDistance(double td) { m_toleranceDistance = td; }
	double toleranceDistance() const { return m_toleranceDistance; }


	// finds the nearest rectangles for a given set of points
	// The nearest rectangles are passed in a list. If the list is empty, there
	// is no rectangle within the ,aximal allowed distance. If the list contains
	// more than one element, the nearest rectangle is not unique for the
	// given tolerance.
	void find(
		const Array<RectRegion> &region, // given rectangles
		const Array<DPoint> &point,      // given points
		Array<List<PairRectDist> > &nearest); // nearest rectangles

	// trivial implementation of find(). Can be used in order to check
	// correctness. Computes only rectangle with minimum distance without
	// considering maxAllowedDistance and toleranceDistance.
	void findSimple(
		const Array<RectRegion> &region,
		const Array<DPoint> &point,
		Array<List<PairRectDist> > &nearest);

private:
	class CoordComparer;
	class YCoordComparer;

	double m_maxAllowedDistance;
	double m_toleranceDistance;
};


//---------------------------------------------------------
// RectRegion
// represents a rectangle given by center point, width and height
//---------------------------------------------------------
struct NearestRectangleFinder::RectRegion
{
	friend ostream &operator<<(ostream &os, const RectRegion &rect) {
		os << "(" << rect.m_x << "," << rect.m_y << ":" <<
			rect.m_width << "," << rect.m_height << ")";
		return os;
	}

	double m_x, m_y, m_width, m_height;
};


//---------------------------------------------------------
// PairRectDist
// represents a rectangle (given by its index) and a
// distance value
//---------------------------------------------------------
struct OGDF_EXPORT NearestRectangleFinder::PairRectDist
{
	PairRectDist() { }

	PairRectDist(int index, double distance) {
		m_index = index;
		m_distance = distance;
	}

	friend ostream &operator<<(ostream &os, const PairRectDist &p) {
		os << "(" << p.m_index << "," << p.m_distance << ")";
		return os;
	}

	int m_index;
	double m_distance;
};



} // end namespace ogdf


#endif
