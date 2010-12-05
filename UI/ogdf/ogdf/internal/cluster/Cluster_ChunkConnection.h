/*
 * $Revision: 2037 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2010-09-29 14:13:26 +0200 (Wed, 29 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief implementation of initial cut-constraint class for the Branch&Cut algorithm
 * for the Maximum C-Planar SubGraph problem.
 * 
 * A feasible ILP solution has to imply a completely connected, planar Sub-Clustergraph.
 * For each cluster that is not connected, additional connection edges have to be inserted
 * between the chunks of the cluster, to obtain c-connectivity.
 * Thus, initial constraints are added that guarantee initial c-connectivity, if the number of chunks
 * is at most 3. If some cluster consists of more than 3 chunks, additional constraints
 * have to be separated during the optimization.
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

#ifndef OGDF_CLUSTER_CHUNK_CONNECTION_H
#define OGDF_CLUSTER_CHUNK_CONNECTION_H

#include <ogdf/basic/Array.h>
#include <ogdf/basic/ArrayBuffer.h>
#include <ogdf/cluster/ClusterGraph.h>
#include <ogdf/internal/cluster/Cluster_EdgeVar.h>
#include <ogdf/internal/cluster/basics.h>

#include <abacus/constraint.h>

namespace ogdf {


class ChunkConnection : public BaseConstraint {
#ifdef OGDF_DEBUG
	//Mainly for debugging output purposes
	friend class Master;
	friend class Sub;
	friend class CPlanarMaster;
	friend class CPlanarSub;
#endif
public:
	
	ChunkConnection(ABA_MASTER *master, const ArrayBuffer<node>& chunk, const ArrayBuffer<node>& cochunk);
		
	virtual ~ChunkConnection();
		
	// Computes and returns the coefficient for the given variable
	virtual double coeff(ABA_VARIABLE *v) { 
		EdgeVar *ev = (EdgeVar *)v;
		//Save for both clustered planarity testing and maximum c-planar subgraph
		return (ev->theEdgeType() != EdgeVar::CONNECT) ? 0.0 : (double)coeff(ev->sourceNode(), ev->targetNode());
	}
	inline int coeff(const nodePair& n) { return coeff(n.v1,n.v2); }
	int coeff(node v1, node v2);
		
	void printMe(ostream& out) const {
		out << "[ChunkCon: (";
		int j;
		forall_arrayindices(j,m_chunk) {
			Logger::slout() << m_chunk[j] << ",";
		}
		out << "|";
		forall_arrayindices(j,m_cochunk) {
			Logger::slout() << m_cochunk[j] << ",";
		} 
		out << ")]";
	}
	
private:

	// The nodePairs corresponding to the constraint
	Array<node> m_chunk;
	Array<node> m_cochunk;
};

}

#endif
