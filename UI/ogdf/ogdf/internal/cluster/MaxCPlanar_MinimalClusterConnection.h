/*
 * $Revision: 2036 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2010-09-29 12:22:43 +0200 (Wed, 29 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of an initial constraint class for the Branch&Cut algorithm
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
 * 
 * \author Mathias Jansen
 * 
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

#ifndef OGDF_MAX_CPLANAR_MINIMAL_CLUSTER_CONNECTION_H
#define OGDF_MAX_CPLANAR_MINIMAL_CLUSTER_CONNECTION_H

#include <ogdf/internal/cluster/Cluster_EdgeVar.h>
#include <ogdf/internal/cluster/MaxCPlanar_Master.h>

#include <abacus/constraint.h>

namespace ogdf {


class MinimalClusterConnection : public ABA_CONSTRAINT {
	
public:

	MinimalClusterConnection(ABA_MASTER *master, List<nodePair> &edges);

	virtual ~MinimalClusterConnection();
	
	// Computes and returns the coefficient for the given variable
	virtual double coeff(ABA_VARIABLE *v);
	
private:

	// The node pairs corresponding to the constraint
	List<nodePair> m_edges;
		
};

}

#endif
