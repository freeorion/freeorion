/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of PlanarModule which implements a planarity
 *        test and planar embedding algorithm.
 * 
 * \author Sebastian Leipert
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

#ifndef OGDF_PLANAR_MODULE_H
#define OGDF_PLANAR_MODULE_H

//=========================================================
// Main functions:
//
// planarityTest(Graph &G)  Tests a graph for planarity.
//
// planarEmbed(Graph &G)  Tests a graph for planarity and returns
//                        a planar embedding if G is planar.
//
//=========================================================

#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/NodeArray.h>
#include <ogdf/basic/SList.h>

namespace ogdf {


class OGDF_EXPORT PlanarModule{

public:

	PlanarModule() {};
	~PlanarModule() {};

	// Returns true, if G is planar, false otherwise.
	bool planarityTest(Graph &G);
	bool planarityTest(const Graph &G);

	// Returns true, if G is planar, false otherwise.
	// If true, G contains a planar embedding.
	bool planarEmbed(Graph &G){return preparation(G,true);}

private:

	// Prepares the planarity test and the planar embedding
	bool preparation(Graph &G,bool embed);

	// Performs a planarity test on a biconnected component
	// of G. numbering contains an st-numbering of the component.
	bool doTest(Graph &G,NodeArray<int> &numbering);

	// Performs a planarity test on a biconnected component
	// of G and embedds it planar. 
	// numbering contains an st-numbering of the component.
	bool doEmbed(Graph &G,
				 NodeArray<int>  &numbering,
				 EdgeArray<edge> &backTableEdges,
				 EdgeArray<edge> &forwardTableEdges);

	// Used by doEmbed. Computes an entire embedding from an
	// upward embedding.
	void entireEmbed(Graph &G,
					 NodeArray<SListPure<adjEntry> > &entireEmbedding,
					 NodeArray<SListIterator<adjEntry> > &adjMarker,
					 NodeArray<bool> &mark,
					 node v);

	void prepareParallelEdges(Graph &G);


	//private Members for handling parallel edges
	EdgeArray<ListPure<edge> > m_parallelEdges;
	EdgeArray<bool> m_isParallel;
	int	m_parallelCount;



};

}
#endif
