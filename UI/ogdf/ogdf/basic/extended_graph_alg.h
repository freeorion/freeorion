/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of extended graph algorithms
 * 
 * \author Sebastian Leipert, Karsten Klein
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

#ifndef OGDF_EXTENDED_GRAPH_ALG_H
#define OGDF_EXTENDED_GRAPH_ALG_H


#include <ogdf/cluster/ClusterGraph.h>

namespace ogdf {


//---------------------------------------------------------
// Methods for induced subgraphs
//---------------------------------------------------------

//! Computes the induced subgraph of nodes.
template<class LISTITERATOR>
void inducedSubGraph(const Graph &G, LISTITERATOR start, Graph &subGraph)
{
	NodeArray<node> nodeTableOrig2New;
	inducedSubGraph(G,start,subGraph,nodeTableOrig2New);
}

//! Computes the induced subgraph of nodes.
template<class LISTITERATOR>
void inducedSubGraph(const Graph &G,
					 LISTITERATOR start,
					 Graph &subGraph,
					 NodeArray<node> &nodeTableOrig2New)
{
	subGraph.clear();
	nodeTableOrig2New.init(G,0);

	EdgeArray<bool> mark(G,false);

	LISTITERATOR its;
	for (its = start; its.valid(); its++)
	{
		node w = (*its);
		OGDF_ASSERT(w != 0 && w->graphOf() == &G);
		nodeTableOrig2New[w] = subGraph.newNode();

		adjEntry adj = w->firstAdj();
		forall_adj(adj,w)
		{
			edge e = adj->theEdge();
			if (nodeTableOrig2New[e->source()] && nodeTableOrig2New[e->target()] && !mark[e])
			{
				subGraph.newEdge(nodeTableOrig2New[e->source()],nodeTableOrig2New[e->target()]);
				mark[e] = true;
			}
		}
	}
}
					 

//! Computes the induced subgraph of nodes.
template<class LISTITERATOR>
void inducedSubGraph(const Graph &G,
					 LISTITERATOR start,
					 Graph &subGraph,
					 NodeArray<node> &nodeTableOrig2New,
					 EdgeArray<edge> &edgeTableOrig2New)
{
	subGraph.clear();
	nodeTableOrig2New.init(G,0);
	edgeTableOrig2New.init(G,0);

	EdgeArray<bool> mark(G,false);

	LISTITERATOR its;
	for (its = start; its.valid(); its++)
	{
		node w = (*its);
		OGDF_ASSERT(w != 0 && w->graphOf() == &G);
		nodeTableOrig2New[w] = subGraph.newNode();

		adjEntry adj = w->firstAdj();
		forall_adj(adj,w)
		{
			edge e = adj->theEdge();
			if (nodeTableOrig2New[e->source()] && 
				nodeTableOrig2New[e->target()] && 
				!mark[e])
			{
				edgeTableOrig2New[e] = 
					subGraph.newEdge(
						nodeTableOrig2New[e->source()],
						nodeTableOrig2New[e->target()]);
				mark[e] = true;
			}
		}
	}
}


template<class NODELISTITERATOR,class EDGELIST>
void inducedSubgraph(Graph &G,NODELISTITERATOR &it,EDGELIST &E)
{
	NODELISTITERATOR itBegin = it;
	NodeArray<bool>  mark(G,false);

	for (;it.valid();it++)
		mark[(*it)] = true;
	it = itBegin;
	for (;it.valid();it++)
	{
		node v = (*it);
		adjEntry adj;
		forall_adj(adj,v)
		{
			edge e = adj->theEdge();
			if (mark[e->source()] && mark[e->target()])
				E.pushBack(e);
		}
	}
}


//---------------------------------------------------------
// Methods for clustered graphs
//---------------------------------------------------------


// true <=> C is C-connected
OGDF_EXPORT bool isCConnected(const ClusterGraph &C);

//connect clusters to achieve c-connectivity (probably loosing planarity)
//GG is underlying graph of C, returns inserted edges in addededges
//simple: only connect underlying cluster subgraph without crossing check
OGDF_EXPORT void makeCConnected(
	ClusterGraph& C,
	Graph& GG,
	List<edge>& addedEdges,
	bool simple = true);




//---------------------------------------------------------
// Methods for st-numbering
//---------------------------------------------------------


// Computes an st-Numbering.
// Precondition: G must be biconnected and simple.
// Exception: the Graph is allowed to have isolated nodes.
// The st-numbers are stored in NodeArray. Return value is 
// the number t. It is 0, if the computation was unsuccessful.
// The nodes s and t may be specified. In this case 
// s and t have to be adjacent.
// If s and t are set 0 and parameter randomized is set to true,
// the st edge is chosen to be a random edge in G.
OGDF_EXPORT int stNumber(const Graph &G,
	NodeArray<int> &numbering,
	node s = 0,
	node t = 0,
	bool randomized = false);

// Tests, whether a numbering is an st-numbering
// Precondition: G must be biconnected and simple.
// Exception: the Graph is allowed to have isolated nodes.
OGDF_EXPORT bool testSTnumber(const Graph &G, NodeArray<int> &st_no,int max);


//---------------------------------------------------------
// Methods for minimum spanning tree computation
//---------------------------------------------------------
//! Computes a minimum spanning tree using Prim's algorithm
/** Computes a minimum spanning tree MST of graph \a G with respect
 *  to edge weights in \a weight using Prim's algorithm.
 *  If an edge is in MST, the corresponding value in \a isInTree
 *  is set to true, otherwise to false.
 *  Returns the sum of the edge weights in the computed tree.
 * */ 
double computeMinST(const Graph &G, EdgeArray<double> &weight, EdgeArray<bool> &isInTree);

} // end namespace ogdf


#endif
