/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class MaximalPlanarSubgraphSimple.
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


#include <ogdf/planarity/MaximalPlanarSubgraphSimple.h>
#include <ogdf/planarity/PlanarModule.h>


namespace ogdf {


Module::ReturnType MaximalPlanarSubgraphSimple::doCall(
	const Graph &G,
	const List<edge> &preferedEdges,
	List<edge> &delEdges,
	const EdgeArray<int>  * /* pCost */,
	bool preferedImplyPlanar)
{
	delEdges.clear();

	Graph H;
	NodeArray<node> mapToH(G);

	node v;
	forall_nodes(v,G)
		mapToH[v] = H.newNode();

	EdgeArray<bool> visited(G,false);
	PlanarModule pm;

	ListConstIterator<edge> it;
	for(it = preferedEdges.begin(); it.valid(); ++it)
	{
		edge eG = *it;
		visited[eG] = true;

		edge eH = H.newEdge(mapToH[eG->source()],mapToH[eG->target()]);

		if (preferedImplyPlanar == false && pm.planarityTest(H) == false) {
			H.delEdge(eH);
			delEdges.pushBack(eG);
		}
	}

	edge eG;
	forall_edges(eG,G)
	{
		if(visited[eG] == true)
			continue;

		edge eH = H.newEdge(mapToH[eG->source()],mapToH[eG->target()]);

		if (pm.planarityTest(H) == false) {
			H.delEdge(eH);
			delEdges.pushBack(eG);
		}
	}
	
	return retFeasible;
}


} // end namespace ogdf
