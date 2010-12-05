/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares CliqueFinder class.
 * CliqueFinder searches for complete (dense) subgraphs.
 * 
 * \author Karsten Klein
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

#ifndef OGDF_CLIQUEFINDER_H
#define OGDF_CLIQUEFINDER_H


#include <ogdf/basic/NodeArray.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/GraphCopy.h>

namespace ogdf{


//! Finds cliques and dense subgraphs.
/**
 * The class CliqueFinder can be called on a graph 
 * to retrieve (disjoint) cliques or dense subgraphs
 * respectively. Uses SPQR trees to find 3-connected
 * components.
 * 
 * In the following, clique always stands for a subgraph
 * with properties that can be defined by the user to change
 * the standard definition of a complete subgraph, e.g. a
 * minimum size/degree etc.
 * We search for cliques in graph G by first dividing G into
 * its triconnnected components and then using a greedy 
 * heuristics within each component
 */
class OGDF_EXPORT CliqueFinder {

public:
	//constructor
	CliqueFinder(const Graph &G);
	~CliqueFinder();
	
	//Calls
	//We first make G biconnected, this keeps the triconnected 
	//components. then the triconnected components are computed
	//within these components, we search for cliques
	
	//!Searches for cliques and returns the clique index number for each node
	/**
	 * Each clique will be assigned a different number, each node gets the 
	 * number of the clique it is contained in, -1 if not a clique member
	 */
	void call(NodeArray<int> &cliqueNumber);
	//!Searches for cliques and returns the list of cliques
	/**
	 * Each clique on return is represented by a list of member nodes
	 * in the list of cliques cliqueLists
	 */
	void call(List< List<node> > &cliqueLists);

	//! the minimum degree of the nodes within the clique/subgraph
	void setMinSize(int i) { m_minDegree = max(2, i-1);}
	enum postProcess {ppNone, ppSimple};

	//! set the abstract measure of density needed for subgraphs to be detected
	/**
	 * does not have effect for graphs with #n<4
	 */
	void setDensity(int density)
	{
		if (density < 0) m_density = 0;
		else if (density > 100) m_density = 100;
		else m_density = density;
	}

protected:
	/**
	 * doing the real work, find subgraphs in graph G, skip
	 * all nodes with degree < minDegree
	 * value 2: all triangles are cliques
	 */
	void doCall(int minDegree = 2);

	//------------------------------------------------------
	//sets the results of doCall depending on call signature

	//clique nodes get numbers from >=0, all other nodes -1
	void setResults(NodeArray<int> &cliqueNumber);
	void setResults(List< List<node> > &cliqueLists);
	void setResults(List< List<node>* > &cliqueLists);

	//work on the result of the first phase (heuristic), e.g.
	//by dropping, splitting or joining some of the found subgraphs
	void postProcessCliques(List< List<node>* > &cliqueList,
		EdgeArray<bool> &usableEdge);

	//check if node v is adjacent to all nodes in node list
	bool allAdjacent(node v, List<node>* vList);
	void writeGraph(Graph &G, NodeArray<int> &cliqueNumber,
		const String &fileName);

	//does a heuristic evaluation of node v (in m_pCopy)
	//concerning its qualification as a cluster start node
	//the higher the return value, the better the node
	//uses only edges with usableEdge == true
	int evaluate(node v, EdgeArray<bool> &usableEdge);
	void checkCliques(List< List<node>* > &cliqueList, bool sizeCheck = true);
	bool cliqueOK(List<node> *clique);
	void findClique(node v, List<node> &neighbours,
						int numRandom = 0);

private:
	const Graph* m_pGraph;
	GraphCopy* m_pCopy;
	NodeArray<int> m_copyCliqueNumber;
	NodeArray<bool> m_usedNode; //node is assigned to clique
	//List< List<node>* > m_cliqueList;
	int m_minDegree;
	int m_numberOfCliques; //stores the number of found cliques
	postProcess m_postProcess;
	bool m_callByList; //stores information on type of call for result setting
	List< List<node> > *m_pList; //stores pointer on list given as call parameter

	int m_density; //an abstract value from 0..100 definin how dense the
	               //subgraphs need to be, is not directly related to any
				   //measure (degree, ...) but translated into a constraint
			       //based on the heuristical search of the subgraphs
};//CliqueFinder

}//end namespace ogdf

#endif
