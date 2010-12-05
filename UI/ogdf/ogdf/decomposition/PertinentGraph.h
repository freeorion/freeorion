/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class PertinentGraph.
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


#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_PERTINENT_GRAPH_H
#define OGDF_PERTINENT_GRAPH_H


namespace ogdf {

	class OGDF_EXPORT SPQRTree;


//---------------------------------------------------------
// PertinentGraph
// pertinent graph of a node in an SPQR-tree
//---------------------------------------------------------
//! Pertinent graphs of nodes in an SPQR-tree.
/**
 * The class PertinentGraph represents the pertinent graph \a G(\a vT)
 * of a node \a vT in an SPQR-tree \a T with original graph \a G.
 *
 * The expansion graph E(\a e) of a virtual skeleton edge \a e is the skeleton graph
 * of the twin \a e' of \a e without \a e', where each virtual edge \a eV again is
 * replaced by its expansion graph E(\a eV). The pertinent graph \a G(\a vT) of a tree
 * node \a vT is obtained from the skeleton \a S of \a vT, where each edge except for
 * the reference edge of \a S is replaced by its expansion graph. Hence, if \a vT
 * is not the root node of \a T, all but one edge in \a G(\a vT) correspond to real
 * edges, otherwise all edges correspond to real edges.
 *
 * If \a P is the pertinent graph of a PlanarSPQRTree, the underlying graph
 * represents the combinatorial embedding which is implied by the embeddings
 * of the skeletons of \a T.
 */

class OGDF_EXPORT PertinentGraph
{
	friend class OGDF_EXPORT SPQRTree;

public:

	// constructor
	// Remark: Pertinent graphs are created by the pertinentGraph()
	//   function of SPQRTree.

	//! Creates an empty instance of type PertinentGraph.
	/**
	 * \remarks Pertinent graphs are created by the pertinentGraph()
	 *          function of SPQRTree.
	 */
	PertinentGraph() : m_vT(0) { }

	//! Initialization of a pertinent graph of tree node \a vT.
	void init(node vT) {
		m_P = Graph();
		m_vT = vT;
		m_vEdge = m_skRefEdge = 0;
		m_origV.init(m_P,0);
		m_origE.init(m_P,0);
	}


	//! Returns the tree node \a vT in \a T whose pertinent graph is this one.
	node treeNode() const {
		return m_vT;
	}

	//! Returns a reference to \a G(\a vT).
	const Graph &getGraph() const {
		return m_P;
	}

	//! Returns a reference to \a G(\a vT).
	Graph &getGraph() {
		return m_P;
	}

	//! Returns the edge in \a G(\a vT) corresponding to the reference edge in skeleton of \a vT.
	/**
	 * If \a vT is the root of \a T, then 0 is returned.
	 */
	edge referenceEdge() const {
		return m_vEdge;
	}

	//! Returns the reference edge in skeleton of \a vT.
	/**
	 * Notice that this edge may differ from the current reference edge in skeleton
	 * of \a vT if \a T has been rerooted after the construction of \a P.
	 */
	edge skeletonReferenceEdge() const {
		return m_skRefEdge;
	}

	//! Returns the vertex in \a G that corresponds to \a v.
	/**
	 * \pre \a v is a node in \a G(\a vT)
	 */
	node original(node v) const {
		return m_origV[v];
	}

	//! Returns the edge in \a G that corresponds to \a e.
	/**
	 * If \a e is the reference edge, then 0 is returned.
	 * \pre \a e is an edge in \a G(\a vT)
	 */
	edge original(edge e) const {
		return m_origE[e];
	}

protected:
	node  m_vT;  //!< corresponding tree node
	Graph m_P;   //!< actual graph
	edge  m_vEdge; //!< reference edge (in \a m_P)
	edge  m_skRefEdge; //!< reference edge (in skeleton(\a m_vT))

	NodeArray<node> m_origV; //!< corresp. original node
	EdgeArray<edge> m_origE; //!< corresp. original edge
};



} // end namespace ogdf


#endif
