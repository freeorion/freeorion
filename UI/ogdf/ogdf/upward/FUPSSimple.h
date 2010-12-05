/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of the FastPlanarSubgraph.
 * 
 * \author Hoi-Ming Wong
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


#ifndef OGDF_FUPS_SIMPLE_H
#define OGDF_FUPS_SIMPLE_H


#include <ogdf/module/FUPSModule.h>


namespace ogdf {


class OGDF_EXPORT FUPSSimple : public FUPSModule{

public:
	//! Creates an instance of feasible subgraph algorithm.
	FUPSSimple() : m_nRuns(0) {};

	// destructor
	~FUPSSimple() {};


	// options

	//! Sets the number of randomized runs to \a nRuns.
	void runs (int nRuns) {
		m_nRuns = nRuns;
	}

	//! Returns the current number of randomized runs.
	int runs() const {
		return m_nRuns;
	}


	//! return a adjEntry of node v which right face is f. Be Carefully! The adjEntry is not always unique.
	adjEntry getAdjEntry(const CombinatorialEmbedding &Gamma, node v, face f) 
	{		
 		adjEntry adj = 0;
 		forall_adj(adj, v) {			
			if (Gamma.rightFace(adj) == f)
				break;
		}

		OGDF_ASSERT(Gamma.rightFace(adj) == f);

		return adj;
	}

protected:	

	/**
	 * \brief Computes a feasible upward planar subgraph of the input graph.
	 *
	 * @param UPR represents the feasible upward planar subgraph after the call. \a UPR has to be initialzed as a
	 *        UpwardPlanRep of the input connected graph G and is modified to obtain the upward planar subgraph.
	 *		  The subgraph is represented as an upward planar representation. 		
	 * @param delEdges is the deleted edges in order to obtain the subgraph. The edges are edges of the original graph G.
	 * \return the status of the result.
	 */	
	virtual Module::ReturnType doCall(UpwardPlanRep &UPR,		
		List<edge> &delEdges);	


private:
	
	int m_nRuns;  //!< The number of runs for randomization.

	void computeFUPS(UpwardPlanRep &UPR,		
					List<edge> &delEdges);

	//! Compute a (random) span tree of the input sT-Graph.
	/*
	 * @param GC The Copy of the input graph G.
	 * @param &delEdges The deleted edges (edges of G).
	 * @param random compute a random span tree
	 * @multisource true, if the original graph got multisources. In this case, the incident edges of 
	 *  the source are allways included in the span tree
	 */
	void getSpanTree(GraphCopy &GC, List<edge> &delEdges, bool random);

	/*
	 * Function use by geSpannTree to compute the spannig tree.
	 */
	void dfs_visit(const Graph &G, edge e, NodeArray<bool> &visited, EdgeArray<bool> &treeEdges, bool random);	

	// construct a merge graph with repsect to gamma and its test acyclicity
	bool constructMergeGraph(GraphCopy &M, // copy of the original graph, muss be embedded
							adjEntry adj_orig, // the adjEntry of the original graph, which right face is the ext. Face and adj->theNode() is the source
							const List<edge> &del_orig); // deleted edges
};

}
#endif
