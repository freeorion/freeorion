/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration and implementation of GraphReduction class
 *        reduces by Leaves & Chains.
 * 
 * \author Markus Chimani
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


#ifndef OGDF_GRAPH_REDUCTION_H
#define OGDF_GRAPH_REDUCTION_H


#include <ogdf/basic/NodeArray.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/SList.h>
#include <ogdf/basic/CombinatorialEmbedding.h>


namespace ogdf {


//---------------------------------------------------------
// GraphReduction
// kick leaves & chains
// GraphReduction is read-only !!!
//---------------------------------------------------------
class OGDF_EXPORT GraphReduction : public Graph {
protected:

	const Graph *m_pGraph; // original graph
	NodeArray<node> m_vOrig; // corresponding node in original graph
	EdgeArray<List<edge> > m_eOrig; // corresponding edge in original graph

	NodeArray<node> m_vReduction; // corresponding node in graph copy
	EdgeArray<edge> m_eReduction; // corresponding chain of edges in graph copy
	
	GraphReduction() : m_vOrig(), m_eOrig(), m_vReduction(), m_eReduction() {}

public:
	// construction
	GraphReduction(const Graph& G);
	virtual ~GraphReduction() {};

	// returns original graph
	const Graph &original() const { return *m_pGraph; }

	// returns original node
	node original(node v) const { return m_vOrig[v]; }
	// returns original edges
	const List<edge> &original(edge e) const { return m_eOrig[e]; }

	// returns reduction of node v (0 if none)
	node reduction(node v) const { return m_vReduction[v]; }
	// returns reduction of edge e
	edge reduction(edge e) const { return m_eReduction[e]; }

}; // class GraphCopy


} // end namespace ogdf

#endif
