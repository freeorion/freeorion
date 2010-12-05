/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Computes an embedding of a graph with minimum depth.
 * See paper "Graph Embedding with Minimum Depth and Maximum External
 * Face" by C. Gutwenger and P. Mutzel (2004) for details.
 * 
 * \author Thorsten Kerkhof
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

#ifndef OGDF_EMBEDDER_MIN_DEPTH_H
#define OGDF_EMBEDDER_MIN_DEPTH_H

#include <ogdf/module/EmbedderModule.h>
#include <ogdf/decomposition/BCTree.h>
#include <ogdf/decomposition/StaticSPQRTree.h>

namespace ogdf {

//! Planar graph embedding with minimum block-nesting depth.
class OGDF_EXPORT EmbedderMinDepth : public EmbedderModule
{
public:
	//constructor
	EmbedderMinDepth() { }

	/**
	 * \brief Computes an embedding of \a G with minimum depth.
	 *
	 * \param PG is the original graph.
	 * \param return adjExternal is an adjacency entry in the external face.
	 */
	void call(PlanRep& PG, adjEntry& adjExternal);

private:
	/**
	 * \brief Computes recursively the block graph for every block.
	 *
	 * \param bT is a block node in the BC-tree.
	 * \param cH is a node of bT in the block graph.
	 */
	void computeBlockGraphs(const node& bT, const node& cH);

	/**
	 * \brief Bottom-up-traversal of bcTree computing the values \a m_{cT, bT}
	 * for all edges \a (cT, bT) in the BC-tree. The length of each vertex
	 * \a v \neq c in \a bT is set to 1 if \a v \in M_{bT} and to 0 otherwise.
	 *
	 * \param \a bT is a block vertex in the BC-tree.
	 * \param \a cH is a vertex in the original graph \a G.
	 * \return Minimum depth of an embedding of \a bT with \a cH on the external
	 *    face.
	 */
	int bottomUpTraversal(const node& bT, const node& cH);

	/**
	 * \brief Top-down-traversal of BC-tree. The minimum depth of the BC-tree-node
	 * bT is calculated and before calling the function recursively for all
	 * children of bT in the BC-tree, the nodeLength of the cut-vertex which bT
	 * and the child have in common is computed. The length of each node is set to
	 * 1 if it is in M_B and 0 otherwise, except for |M_B| = 1, than it is set to
	 * 1 if it is in M2 with m2 = max_{v \in V_B, v != c} m_B(v) and
	 * M2 = {c \in V_B \ {v} | m_B(c) = m2}.
	 *
	 * \param \a bT is a block vertex in the BC-tree.
	 */
	void topDownTraversal(const node& bT);

	/**
   * \brief Computes the adjacency list for all nodes in a block and calls
   * recursively the function for all blocks incident to nodes in bT.
	 *
	 * \param bT is the tree node treated in this function call.
   */
  void embedBlock(const node& bT);

	/**
   * \brief Computes the adjacency list for all nodes in a block and calls
   * recursively the function for all blocks incident to nodes in bT.
	 *
	 * \param bT is the tree node treated in this function call.
	 * \param cT is the parent cut vertex node of bT in the BC-tree. cT is 0 if bT
	 *   is the root block.
	 * \param after is the adjacency entry of the cut vertex, after which bT has to
	 *   be inserted.
   */
  void embedBlock(const node& bT, const node& cT, ListIterator<adjEntry>& after);

private:
	/** BC-tree of the original graph */
	BCTree* pBCTree;

	/** an adjacency entry on the external face */
	adjEntry* pAdjExternal;

	/** all blocks */
	NodeArray<Graph> blockG;

	/** a mapping of nodes in the auxiliaryGraph of the BC-tree to blockG */
	NodeArray< NodeArray<node> > nH_to_nBlockEmbedding;

	/** a mapping of edges in the auxiliaryGraph of the BC-tree to blockG */
	NodeArray< EdgeArray<edge> > eH_to_eBlockEmbedding;

	/** a mapping of nodes in blockG to the auxiliaryGraph of the BC-tree */
	NodeArray< NodeArray<node> > nBlockEmbedding_to_nH;

	/** a mapping of edges in blockG to the auxiliaryGraph of the BC-tree */
	NodeArray< EdgeArray<edge> > eBlockEmbedding_to_eH;
	
	/** saving for each node in the block graphs its length */
	NodeArray< NodeArray<int> > nodeLength;

	/** an array containing the minimum depth of each block */
	NodeArray<int> minDepth;

	/** an array saving the length for each edge in the BC-tree */
	EdgeArray<int> m_cB;

	/** M_B = {cH \in B | m_B(cH) = m_B} with m_B = max_{c \in B} m_B(c)
	 *  and m_B(c) = max {0} \cup {m_{c, B'} | c \in B', B' \neq B}. */
	NodeArray< List<node> > M_B;

	/** M2 is empty, if |M_B| != 1, otherwise M_B = {cH}
	 *  M2 = {cH' \in V_B \ {v} | m_B(cH') = m2} with
	 *  m2 = max_{vH \in V_B, vH != cH} m_B(vH). */
	NodeArray< List<node> > M2;

	/** saves for every node of PG the new adjacency list */
	NodeArray< List<adjEntry> > newOrder;

	/** treeNodeTreated saves for all block nodes in the
	 *  BC-tree if it has already been treated or not. */
	NodeArray<bool> treeNodeTreated;

	/** The SPQR-trees of the blocks */
	NodeArray<StaticSPQRTree*> spqrTrees;
};

} // end namespace ogdf

#endif
