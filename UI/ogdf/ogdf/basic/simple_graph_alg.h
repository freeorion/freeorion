/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of simple graph algorithms.
 * 
 * \author Carsten Gutwenger and Sebastian Leipert
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

#ifndef OGDF_SIMPLE_GRAPH_ALG_H
#define OGDF_SIMPLE_GRAPH_ALG_H


#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/SList.h>

namespace ogdf {


//---------------------------------------------------------
// Methods for loops
//---------------------------------------------------------

//! Returns true iff \a G contains no self-loop.
OGDF_EXPORT bool isLoopFree(const Graph &G);

//! Removes all self-loops from \a G and returns all nodes with self-loops in \a L.
template<class NODELIST>
void makeLoopFree(Graph &G, NODELIST &L)
{
	L.clear();

	edge e, eNext;
	for (e = G.firstEdge(); e; e = eNext) {
		eNext = e->succ();
		if (e->isSelfLoop()) {
			L.pushBack(e->source());
			G.delEdge(e);
		}
	}
}

//! Removes all self-loops from \a G.
OGDF_EXPORT void makeLoopFree(Graph &G);


//---------------------------------------------------------
// Methods for parallel edges
//---------------------------------------------------------

OGDF_EXPORT void parallelFreeSort(const Graph &G, SListPure<edge> &edges);

//! Returns true iff \a G contains no multi-edges.
OGDF_EXPORT bool isParallelFree(const Graph &G);

//! Returns the number of multi-edges in \a G.
OGDF_EXPORT int numParallelEdges(const Graph &G);


//! Removes all but one of each bundle of multi-edges.
/**
 * @param G is the input graph.
 * @param parallelEdges is assigned the list of remaining edges in \a G that were
 *        part of a bundle of mullti-edges in the input graph.
 */
template <class EDGELIST>
void makeParallelFree(Graph &G, EDGELIST &parallelEdges)
{
	parallelEdges.clear();
	if (G.numberOfEdges() <= 1) return;

	SListPure<edge> edges;
	parallelFreeSort(G,edges);

	SListConstIterator<edge> it = edges.begin();
	edge ePrev = *it++, e;
	bool bAppend = true;
	while(it.valid()) {
		e = *it++;
		if (ePrev->source() == e->source() && ePrev->target() == e->target()) {
			G.delEdge(e);
			if (bAppend) { parallelEdges.pushBack(ePrev); bAppend = false; }
		} else {
			ePrev = e; bAppend = true;
		}
	}
}


//! Removes all but one edge of each bundle of multi-edges in \a G.
inline void makeParallelFree(Graph &G) {
	List<edge> parallelEdges;
	makeParallelFree(G,parallelEdges);
}



OGDF_EXPORT void parallelFreeSortUndirected(const Graph &G, SListPure<edge> &edges,
	EdgeArray<int> &minIndex, EdgeArray<int> &maxIndex);

//! Returns true iff \a G contains neither multi-edges nor reversal edges.
OGDF_EXPORT bool isParallelFreeUndirected(const Graph &G);

//! return the number of multi- and reversal edges.
OGDF_EXPORT int numParallelEdgesUndirected(const Graph &G);


//! Removes all but one of each bundle of undirected multi-edges.
/**
 * Undirected means that edges (v,w) and (w,v) are considered as mutli-edges.
 * @param G is the input graph.
 * @param parallelEdges is assigned the list of remaining edges that were
 * part of a bundle of multi-edges in the input graph.
 */
template <class EDGELIST>
void makeParallelFreeUndirected(Graph &G, EDGELIST &parallelEdges)
{
	parallelEdges.clear();
	if (G.numberOfEdges() <= 1) return;

	SListPure<edge> edges;
	EdgeArray<int> minIndex(G), maxIndex(G);
	parallelFreeSortUndirected(G,edges,minIndex,maxIndex);

	SListConstIterator<edge> it = edges.begin();
	edge ePrev = *it++, e;
	bool bAppend = true;
	while(it.valid()) {
		e = *it++;
		if (minIndex[ePrev] == minIndex[e] && maxIndex[ePrev] == maxIndex[e]) {
			G.delEdge(e);
			if (bAppend) { parallelEdges.pushBack(ePrev); bAppend = false; }
		} else {
			ePrev = e; bAppend = true;
		}
	}
}


//! Removes all but one of each bundle of undirected multi-edges.
/**
 * Undirected means that edges (v,w) and (w,v) are considered as mutli-edges.
 */
inline void makeParallelFreeUndirected(Graph &G) {
	List<edge> parallelEdges;
	makeParallelFreeUndirected(G,parallelEdges);
}


// removes all but one of each bundle of undirected parallel edges
// ((v,w) and (w,v) are considered as the same edge); returns the
// list of remaining edges with parallel edges in the original graph
// including the number of the removed edges that have been parallel 
// to an edge in parallelEdges.

//! Removes all but one of each bundle of undirected multi-edges.
/**
 * Undirected means that edges (v,w) and (w,v) are considered as mutli-edges.
 * @param G is the input graph.
 * @param parallelEdges is assigned the list of remaining edges that were
 *        part of a bundle of multi-edges in the input graph.
 * @param cardPositive contains for each edge the number of removed multi-edges 
 *        pointing in the same direction.
 * @param cardNegative contains for each edge the number of removed multi-edges 
 *        pointing in the opposite direction.
 */
template <class EDGELIST>
void makeParallelFreeUndirected(Graph &G, 
								EDGELIST &parallelEdges, 
								EdgeArray<int> &cardPositive,
								EdgeArray<int> &cardNegative)
{
	parallelEdges.clear();
	cardPositive.fill(0);
	cardNegative.fill(0);
	if (G.numberOfEdges() <= 1) return;

	SListPure<edge> edges;
	EdgeArray<int> minIndex(G), maxIndex(G);
	parallelFreeSortUndirected(G,edges,minIndex,maxIndex);

	SListConstIterator<edge> it = edges.begin();
	edge ePrev = *it++, e;
	bool bAppend = true;
	int  counter = 0;
	while(it.valid()) 
	{
		e = *it++;
		if (minIndex[ePrev] == minIndex[e] && maxIndex[ePrev] == maxIndex[e]) 
		{
			if (ePrev->source() == e->source() && ePrev->target() == e->target()) 
				cardPositive[ePrev]++;
			else if (ePrev->source() == e->target() && ePrev->target() == e->source()) 
				cardNegative[ePrev]++;
			G.delEdge(e);
			if (bAppend) 
			{ 
				parallelEdges.pushBack(ePrev); 
				bAppend = false; 
			}
		} 
		else 
		{
			ePrev = e; bAppend = true;
		}
	}
}

//! Computes for each bundle of multi-edges the list of undirected multi-edges.
/**
 * Stores for one (arbitrarily chosen) reference edge all its undirected mutli-edges
 * of each bundle of undirected multi-edges ((v,w) and (w,v) are 
 * considered as the same edge); no edge is removed from the graph. 
 */
template <class EDGELIST>
void getParallelFreeUndirected(const Graph &G, EdgeArray<EDGELIST> &parallelEdges)
{
	if (G.numberOfEdges() <= 1) return;

	SListPure<edge> edges;
	EdgeArray<int> minIndex(G), maxIndex(G);
	parallelFreeSortUndirected(G,edges,minIndex,maxIndex);

	SListConstIterator<edge> it = edges.begin();
	edge ePrev = *it++, e;
	while(it.valid()) 
	{
		e = *it++;
		if (minIndex[ePrev] == minIndex[e] && maxIndex[ePrev] == maxIndex[e]) 
			parallelEdges[ePrev].pushBack(e);
		else 
			ePrev = e;
	}
}

//---------------------------------------------------------
// Methods for simple graphs
//---------------------------------------------------------


//! Returns true iff \a G contains neither self-loops nor multi-edges.
inline bool isSimple(const Graph &G) {
	return isLoopFree(G) && isParallelFree(G);
}

//! Removes all self-loops and all but one edge of each bundle of multi-edges.
inline void makeSimple(Graph &G) {
	makeLoopFree(G);
	makeParallelFree(G);
}



//! Returns true iff \a G contains neither self-loops nor undirected multi-edges.
inline bool isSimpleUndirected(const Graph &G) {
	return isLoopFree(G) && isParallelFreeUndirected(G);
}

//! Removes all self-loops and all but one edge of each bundle of undirected multi-edges.
inline void makeSimpleUndirected(Graph &G) {
	makeLoopFree(G);
	makeParallelFreeUndirected(G);
}



//---------------------------------------------------------
// Methods for connectivity
//---------------------------------------------------------

//! Returns true iff \a G is connected.
OGDF_EXPORT bool isConnected(const Graph &G);

//! Makes \a G connected by adding a minimum number of edges.
/**
 * @param G is the input graph.
 * @param added is assigned the added edges.
 */
OGDF_EXPORT void makeConnected(Graph &G, List<edge> &added);

//! makes \a G connected by adding a minimum number of edges.
inline void makeConnected(Graph &G) {
	List<edge> added;
	makeConnected(G,added);
}

//! Computes the connected components of \a G.
/**
 * Assign component numbers 0, 1, ... The component number of each
 * node is stored in \a component.
 * @param G is the input graph.
 * @param component is assigned a mapping from nodes to component numbers.
 * @return the number of connected components.
 */
OGDF_EXPORT int connectedComponents(const Graph &G, NodeArray<int> &component);

//the same as connnectedComponents, but returns the isolated nodes 
//! Computes the connected components of \a G.
/**
 * Assign component numbers 0, 1, ... The component number of each
 * node is stored in \a component.
 * @param G is the input graph.
 * @param isolated is assigned the list of isolated nodes. An isolated
 * node is a node without adjacent edges.
 * @param component is assigned a mapping from nodes to component numbers.
 * @return the number of connected components.
 */
OGDF_EXPORT int connectedIsolatedComponents(const Graph &G, List<node> &isolated, 
								NodeArray<int> &component);

//! Returns true if \a G is biconnected.
/**
 * If false is returned, then \a cutVertex is assigned either 0 if G is not connected,
 * or a cut vertex in \a G.
 */
OGDF_EXPORT bool isBiconnected(const Graph &G, node &cutVertex);

//! Returns true iff \a G is biconnected.
inline bool isBiconnected(const Graph &G) {
	node cutVertex;
	return isBiconnected(G,cutVertex);
}

//! Makes \a G biconnected by adding edges.
/**
 * @param G is the input graph.
 * @param added is assigned the list of inserted edges.
 */
OGDF_EXPORT void makeBiconnected(Graph &G, List<edge> &added);

//! Makes \a G biconnected by adding edges.
inline void makeBiconnected(Graph &G) {
	List<edge> added;
	makeBiconnected(G,added);
}

//! Computes the biconnected components of \a G.
/**
 * Assign component numbers 0, 1, ... The component number of each edge
 * is stored in \a component.
 * \return the number of biconnected components (including isolated nodes).
 */
OGDF_EXPORT int biconnectedComponents(const Graph &G, EdgeArray<int> &component);


//! Returns true iff \a G is triconnected.
/**
 * If true is returned, then either
 *   - \a s1 and \a s2 are either both 0 if \a G is not connected; or
 *   - \a s1 is a cut vertex and \a s2 = 0 if \a G is not biconnected; or
 *   - \a s1 and \a s2 are a separation pair otherwise.
 */
OGDF_EXPORT bool isTriconnected(const Graph &G, node &s1, node &s2);

//! Returns true iff \a G is triconnected.
inline bool isTriconnected(const Graph &G) {
	node s1, s2;
	return isTriconnected(G,s1,s2);
}


//! Returns true iff \a G is triconnected.
/**
 * If true is returned, then either
 *   - \a s1 and \a s2 are either both 0 if \a G is not connected; or
 *   - \a s1 is a cut vertex and \a s2 = 0 if \a G is not biconnected; or
 *   - \a s1 and \a s2 are a separation pair otherwise.
 *
 * \warning This method has quadratic running time. An efficient linear time
 *          version is provided by isTriconnected().
 */
OGDF_EXPORT bool isTriconnectedPrimitive(const Graph &G, node &s1, node &s2);

//! Returns true iff \a G is triconnected.
/**
 * \warning This method has quadratic running time. An efficient linear time
 *          version is provided by isTriconnected().
 */
inline bool isTriconnectedPrimitive(const Graph &G) {
	node s1, s2;
	return isTriconnectedPrimitive(G,s1,s2);
}


//---------------------------------------------------------
// Methods for directed graphs
//---------------------------------------------------------

//! Returns true if \a G is acyclic.
/**
 * @param G is the input graph
 * @param backedges is assigned the backedges of a DFS-tree.
 */
OGDF_EXPORT bool isAcyclic(const Graph &G, List<edge> &backedges);

//! Returns true iff \a G is acyclic.
inline bool isAcyclic(const Graph &G) {
	List<edge> backedges;
	return isAcyclic(G,backedges);
}

//! Returns true if \a G is acyclic (undirected version).
/**
 * @param G is the input graph
 * @param backedges is assigned the backedges of a DFS-tree.
 */
OGDF_EXPORT bool isAcyclicUndirected(const Graph &G, List<edge> &backedges);

//! Returns true iff \a G is acyclic (undirected version).
inline bool isAcyclicUndirected(const Graph &G) {
	List<edge> backedges;
	return isAcyclicUndirected(G,backedges);
}

//! Makes \a G acyclic by removing edges.
/**
 * The implementation removes all backedges of a DFS tree.
 */
OGDF_EXPORT void makeAcyclic(Graph &G);

//! Makes G acyclic by reversing edges.
/**
 * \remark The implementation ignores self-loops and reverses
 * the backedges of a DFS-tree.
 */
OGDF_EXPORT void makeAcyclicByReverse(Graph &G);


//! Returns true iff \a G contains exactly one source node (or is empty).
/**
 * @param G is the input graph.
 * @param source is assigned the single source if true is returned, or 0 otherwise.
 */
OGDF_EXPORT bool hasSingleSource(const Graph &G, node &source);

//! Returns true iff \a G contains exactly one source node (or is empty).
inline bool hasSingleSource(const Graph &G) {
	node source;
	return hasSingleSource(G,source);
}

//! Returns true iff \a G contains exactly one sink node (or is empty).
/**
 * @param G is the input graph.
 * @param sink is assigned the single sink if true is returned, or 0 otherwise.
 */
OGDF_EXPORT bool hasSingleSink(const Graph &G, node &sink);

// Returns true iff \a G contains exactly one sink node (or is empty).
inline bool hasSingleSink(const Graph &G) {
	node sink;
	return hasSingleSink(G,sink);
}


//! Returns true if \a G is an st-graph.
/**
 * A directed graph is an st-graph if it is acyclic, contains exactly one source s
 * and one sink t, and the edge (s,t).
 * @param G is the input graph.
 * @param s is assigned the single source (if true is returned).
 * @param t is assigned the single sink (if true is returned).
 * @param st is assigned the edge (s,t) (if true is returned).
 */
OGDF_EXPORT bool isStGraph(const Graph &G, node &s, node &t, edge &st);

//! Returns true if \a G is an st-graph.
inline bool isStGraph(const Graph &G) {
	node s, t;
	edge st;
	return isStGraph(G,s,t,st);
}

//! Computes a topological numbering of an acyclic graph \a G.
/**
 * \pre \a G is an acyclic directed graph.
 * @param G is the input graph.
 * @param num is assigned the topological numbering.
 */
OGDF_EXPORT void topologicalNumbering(const Graph &G, NodeArray<int> &num);

//---------------------------------------------------------
// Methods for trees and forests
//---------------------------------------------------------

//! Returns true iff \a G is a free forest, i.e., contains no undirect cycle.
OGDF_EXPORT bool isFreeForest(const Graph &G);


//---------------------------------------------------------
// Methods for trees and forests
//---------------------------------------------------------

//! Returns true iff \a G represents a forest, i.e., a collection of rooted trees.
/**
 * @param G is the input graph.
 * @param roots is assigned the list of root nodes of the trees in the forest.
 */
OGDF_EXPORT bool isForest(const Graph& G, List<node> &roots);

//! Returns true iff \a G represents a forest, i.e., a collection of rooted trees.
inline bool isForest(const Graph &G)
{
	List<node> roots;
	return isForest(G,roots);
}

//! Returns true iff \a G represents a tree
/**
 * @param G is the input graph.
 * @param root is assigned the root node (if true is returned).
 */
OGDF_EXPORT bool isTree (const Graph& G, node &root);

//! Returns true iff \a G represents a tree
inline bool isTree(const Graph &G) {
	node root;
	return isTree(G,root);
}


} // end namespace ogdf

#endif
