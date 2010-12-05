/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares EdgeComparer class.
 *
 * The EdgeComparer compares adjacency entries on base of
 * the position of the nodes given by an Attributed Graph's
 * layout information.
 * 
 * \author Karsten Klein
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


#ifndef OGDF_EDGECOMPARER_H
#define OGDF_EDGECOMPARER_H



#include <ogdf/planarity/PlanRep.h>
#include <ogdf/basic/GraphAttributes.h>

namespace ogdf {

//! The EdgeComparer compares adjacency entries on base of the position of the nodes given by an Attributed Graph's layout information
/**
 * helper function for ordering / sorting
 * assumes that PG is a planrep on original AG and that PG is
 * not normalized, i.e., if there are bends in AG, they do not
 * have a counterpart in PG (all nodes in PG have an original)
 * temporary: we assume that we have two adjentries
 * at a common point, so we leave the check for now
 * if they meet and at which point
 * 
 * \todo check if sorting order fits adjacency list
 */
class OGDF_EXPORT EdgeComparer : public VComparer<adjEntry>
{
public:
	//order: clockwise
	
	EdgeComparer(const GraphAttributes& AG, const PlanRep& PR) : 
	  m_AG(&AG), m_PR(&PR)
	{}

	//! compare the edges directly in AG
	EdgeComparer(const GraphAttributes &AG) : m_AG(&AG), m_PR(0) {}

	int compare(const adjEntry &e1, const adjEntry &e2) const;

	//! check if vector u->v lies within 180degree halfcircle before vector u->w in clockwise order (i.e. twelve o'clock lies before 1)
	bool before(const DPoint u, const DPoint v, const DPoint w) const;
	
private:
	
	//! returns a value > 0, if vector uv lies "before" vector uw
	int orientation(const DPoint u, 
				    const DPoint v, 
				    const DPoint w) const;

    //! compares by angle relative to x-axis
	int compareVectors(const double& x1, 
						const double& y1, 
						const double& x2, 
						const double& y2) const;
	//! computes angle between vectors p->q, p->r
	double angle(DPoint p, DPoint q, DPoint r) const;

	inline int signOf(const double& x) const
	{ 
		if ( x == 0 ) return 0;
		else if (x > 0 ) return 1;
			 else return -1;
	}


	const GraphAttributes *m_AG;
	const PlanRep *m_PR;
};//EdgeComparer


}//namespace ogdf

#endif
