/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements class ExpansionGraph
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


#include <ogdf/upward/ExpansionGraph.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/NodeSet.h>


namespace ogdf {


// constructor
// computes biconnected componets of original graph
// does not create a copy graph
ExpansionGraph::ExpansionGraph(const Graph &G) :
	m_compNum(G), m_adjComponents(G),
	m_vCopy(G,0), m_vOrig(*this,0), m_vRep(*this,0), m_eOrig(*this,0)
{
	// compute biconnected components
	int numComp = biconnectedComponents(G,m_compNum);

	// for each component, build list of contained edges
	m_component.init(numComp);

	edge e;
	forall_edges(e,G)
		m_component[m_compNum[e]].pushBack(e);

	// for each vertex v, build list of components containing v
	NodeSetSimple contained(G);
	for(int i = 0; i < numComp; ++i)
	{
		SListConstIterator<edge> it;
		for(it = m_component[i].begin(); it.valid(); ++it)
		{
			e = *it;
			node v = e->source();
			if (contained.isMember(v) == false) {
				contained.insert(v);
				m_adjComponents[v].pushBack(i);
			}

			v = e->target();
			if (contained.isMember(v) == false) {
				contained.insert(v);
				m_adjComponents[v].pushBack(i);
			}
		}

		contained.clear();
	}
}


// builds expansion graph of i-th biconnected component of the original graph
void ExpansionGraph::init(int i)
{
	OGDF_ASSERT(0 <= i && i <= m_component.high());

	// remove previous component
	node v;
	forall_nodes(v,*this) {
		node vOrig = m_vOrig[v];
		if (vOrig)
			m_vCopy[vOrig] = 0;
	}
	clear();


	// create new component
	SListConstIterator<edge> it;
	for(it = m_component[i].begin(); it.valid(); ++it)
	{
		edge e = *it;

		edge eCopy = newEdge(getCopy(e->source()),getCopy(e->target()));
		m_eOrig[eCopy] = e;
	}

	// expand vertices
	forall_nodes(v,*this)
	{
		if (original(v) && v->indeg() >= 1 && v->outdeg() >= 1) {
			node vPrime = newNode();
			m_vRep[vPrime] = m_vOrig[v];

			SListPure<edge> edges;
			outEdges(v,edges);

			SListConstIterator<edge> it;
			for(it = edges.begin(); it.valid(); ++it)
				moveSource(*it,vPrime);

			newEdge(v,vPrime);
		}
	}
}


// builds expansion graph of graph G
// for debugging purposes only
void ExpansionGraph::init(const Graph &G)
{
	// remove previous component
	node v;
	forall_nodes(v,*this) {
		node vOrig = m_vOrig[v];
		if (vOrig)
			m_vCopy[vOrig] = 0;
	}
	clear();


	// create new component
	forall_nodes(v,G)
		getCopy(v);

	edge e;
	forall_edges(e,G)
	{
		edge eCopy = newEdge(getCopy(e->source()),getCopy(e->target()));
		m_eOrig[eCopy] = e;
	}

	// expand vertices
	forall_nodes(v,*this)
	{
		if (original(v) && v->indeg() >= 1 && v->outdeg() >= 1) {
			node vPrime = newNode();

			SListPure<edge> edges;
			outEdges(v,edges);

			SListConstIterator<edge> it;
			for(it = edges.begin(); it.valid(); ++it)
				moveSource(*it,vPrime);

			newEdge(v,vPrime);
		}
	}
}


} // end namespace ogdf

