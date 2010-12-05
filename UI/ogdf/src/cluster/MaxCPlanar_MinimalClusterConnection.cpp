/*
 * $Revision: 2037 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2010-09-29 14:13:26 +0200 (Wed, 29 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief implementation of constraint class for the Branch&Cut algorithm
 * for the Maximum C-Planar SubGraph problem.
 * 
 * If some cluster has no connection to some other cluster,
 * the optimal solution might insert a new connection-edge between theese two clusters,
 * to obtain connectivity. Since the objective function minimizes the number
 * of new connection-edges, at most one new egde will be inserted between
 * two clusters that are not connected.
 * This behaviour of the LP-solution is guaranteed from the beginning, by creating
 * an initial constraint for each pair of non-connected clusters,
 * which is implemented by this constraint class.
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

#include <ogdf/internal/cluster/MaxCPlanar_MinimalClusterConnection.h>

using namespace ogdf;


MinimalClusterConnection::MinimalClusterConnection(ABA_MASTER *master, List<nodePair> &edges) :
	ABA_CONSTRAINT(master, 0, ABA_CSENSE::Less, 1.0, false, false, true)
{
	ListConstIterator<nodePair> it;
	for (it = edges.begin(); it.valid(); ++it) {
		m_edges.pushBack(*it);
	}
}


MinimalClusterConnection::~MinimalClusterConnection() {}


double MinimalClusterConnection::coeff(ABA_VARIABLE *v) {
	//TODO: speedup, we know between which nodepairs edges exist...
	EdgeVar *e = (EdgeVar *)v;
	ListConstIterator<nodePair> it;
	for (it = m_edges.begin(); it.valid(); ++it) {
		if ( ((*it).v1 == e->sourceNode() && (*it).v2 == e->targetNode()) ||
			 ((*it).v2 == e->sourceNode() && (*it).v1 == e->targetNode()) )
		{return 1.0;}
	}
	return 0.0;
}

#endif // USE_ABACUS
