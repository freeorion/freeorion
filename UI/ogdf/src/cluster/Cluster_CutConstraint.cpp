/*
 * $Revision: 2037 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2010-09-29 14:13:26 +0200 (Wed, 29 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of a constraint class for the Branch&Cut algorithm
 * for the Maximum C-Planar SubGraph problem.
 * 
 * This class represents the cut-constraints belonging to the ILP formulation.
 * Cut-constraints are dynamically separated be means of cutting plane methods.
 * 
 * \author Mathias Jansen
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

#ifdef USE_ABACUS

#include <ogdf/internal/cluster/Cluster_CutConstraint.h>

using namespace ogdf;

CutConstraint::CutConstraint(ABA_MASTER *master, ABA_SUB *sub, List<nodePair> &edges) :
	BaseConstraint(master, sub, ABA_CSENSE::Greater, 1.0, true, true, true)
{
	ListConstIterator<nodePair> it;
	for (it = edges.begin(); it.valid(); ++it) {
		m_cutEdges.pushBack(*it);
	}
}


CutConstraint::~CutConstraint() {}


int CutConstraint::coeff(node n1, node n2) {
	ListConstIterator<nodePair> it;
	for (it = m_cutEdges.begin(); it.valid(); ++it) {
		if ( ((*it).v1 == n1 && (*it).v2 == n2) ||
			 ((*it).v2 == n1 && (*it).v1 == n2) )
		{return 1;}
	}
	return 0;
}

#endif // USE_ABACUS
