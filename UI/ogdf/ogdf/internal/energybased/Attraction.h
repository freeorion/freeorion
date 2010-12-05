/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares class Attraction.
 * 
 * \author Rene Weiskircher
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

#ifndef OGDF_ATTRACTION_H
#define OGDF_ATTRACTION_H


//! Average length and height of nodes is multiplied by this factor to get preferred edge length
#define MULTIPLIER 2.0

#include <ogdf/internal/energybased/NodePairEnergy.h>

namespace ogdf {


//! Energy function for attraction between two adjacent vertices.
/**
 * Implements an energy function that simulates 
 * attraction between two adjacent vertices. There is an optimum 
 * distance where the energy is zero. The energy grows quadratic 
 * with the difference to the optimum distance. The optimum 
 * distance between two adjacent vertices depends on the size of 
 * the two vertices.
 */
class Attraction: public NodePairEnergy {
public:
		//Initializes data structures to speed up later computations
		Attraction(GraphAttributes &AG);
		~Attraction() {}
		//! set the preferred edge length to the absolute value l
		void setPreferredEdgelength(double l) {m_preferredEdgeLength = l;}
		//! set multiplier for the edge length with repspect to node size to multi
		void reinitializeEdgeLength(double multi);
#ifdef OGDF_DEBUG
		void printInternalData() const;
#endif
private:
	//! the length that that all edges should ideally have
	double m_preferredEdgeLength; 
	//! computes the energy contributed by the two nodes if they are placed at the two given positions
	double computeCoordEnergy(node,node, const DPoint&, const DPoint &) const;
};

}// namespace ogdf

#endif
