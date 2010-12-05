/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of some useful functions dealing with 
//   min-cost flow (generater, checker)
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


#include <ogdf/module/MinCostFlowModule.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/simple_graph_alg.h>


namespace ogdf {


void MinCostFlowModule::generateProblem(
	Graph &G,
	int n,
	int m,
	EdgeArray<int> &lowerBound,
	EdgeArray<int> &upperBound,
	EdgeArray<int> &cost,
	NodeArray<int> &supply)
{
	ogdf::randomGraph(G,n,m);

	node s = G.firstNode();
	node t = G.lastNode();

	node v;
	forall_nodes(v,G) {
		G.newEdge(s,v);
		G.newEdge(v,t);
	}

	edge e;
	forall_edges(e,G) {
		lowerBound[e] = 0;
		upperBound[e] = (e->source() != s) ? ogdf::randomNumber(1,10) : ogdf::randomNumber(2,13);
		cost[e] = ogdf::randomNumber(0,100);
	}


		
	node vl;
	for(v = G.firstNode(), vl = G.lastNode(); true; v = v->succ(), vl = vl->pred()) {
		if (v == vl) {
			supply[v] = 0;
			break;
		}

		supply[v] = -(supply[vl] = ogdf::randomNumber(-1,1));

		if (vl == v->succ())
			break;
	}

}

bool MinCostFlowModule::checkProblem(
	const Graph &G,
	const EdgeArray<int> &lowerBound,
	const EdgeArray<int> &upperBound,
	const NodeArray<int> &supply)
{
	if(isConnected(G) == false)
		return false;

	edge e;
	forall_edges(e,G) {
		if (lowerBound[e] > upperBound[e])
			return false;
	}

	int sum = 0;
	node v;
	forall_nodes(v,G) {
		sum += supply[v];
	}

	return (sum == 0);
}


bool MinCostFlowModule::checkComputedFlow(
	const Graph &G,
	EdgeArray<int> &lowerBound,
	EdgeArray<int> &upperBound,
	EdgeArray<int> &cost,
	NodeArray<int> &supply,
	EdgeArray<int> &flow,
	int &value)
{
	value = 0;

	edge e;
	forall_edges(e,G) {
		if (flow[e] < lowerBound[e] || upperBound[e] < flow[e]) {
			return false;
		}

		value += flow[e] * cost[e];
	}

	node v;
	forall_nodes(v,G) {
		int sum = 0;
		forall_adj_edges(e,v) {
			if(e->isSelfLoop())
				continue;

			if (e->source() == v)
				sum += flow[e];
			else
				sum -= flow[e];
		}
		if (sum != supply[v])
			return false;
	}

	return true;
}


} // end namespace ogdf
