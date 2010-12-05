/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements class MMSubgraphPlanarizer.
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

#include <ogdf/planarity/MMSubgraphPlanarizer.h>
#include <ogdf/planarity/FastPlanarSubgraph.h>
#include <ogdf/planarity/MMFixedEmbeddingInserter.h>


namespace ogdf {


MMSubgraphPlanarizer::MMSubgraphPlanarizer()
{
	FastPlanarSubgraph *s = new FastPlanarSubgraph();
	s->runs(100);
	m_subgraph.set(s);
	
	MMFixedEmbeddingInserter *pInserter = new MMFixedEmbeddingInserter();
	pInserter->removeReinsert(MMEdgeInsertionModule::rrAll);
	m_inserter.set(pInserter);
	
	m_permutations = 1;
}


Module::ReturnType MMSubgraphPlanarizer::doCall(PlanRepExpansion &PG,
	int cc,
	const EdgeArray<bool> *forbid, 
	int& crossingNumber,
	int& numNS,
	int& numSN)
{
	OGDF_ASSERT(m_permutations >= 1);
  
	List<edge> deletedEdges;
	PG.initCC(cc);

	ReturnType retValue ;
	
	if(forbid != 0) {
		List<edge> preferedEdges;
		edge e;
		forall_edges(e, PG) {
			edge eOrig = PG.originalEdge(e);
			if(eOrig && (*forbid)[eOrig])
				preferedEdges.pushBack(e);
		}

		retValue = m_subgraph.get().call(PG, preferedEdges, deletedEdges, true);

	} else {
		retValue = m_subgraph.get().call(PG, deletedEdges);
	}

	if(isSolution(retValue) == false)
		return retValue;

	for(ListIterator<edge> it = deletedEdges.begin(); it.valid(); ++it)
		*it = PG.originalEdge(*it);

	bool foundSolution = false;
	int bestcr = -1;

	for(int i = 1; i <= m_permutations; ++i)
	{
		for(ListConstIterator<edge> it = deletedEdges.begin(); it.valid(); ++it)
			PG.delCopy(PG.copy(*it));

		deletedEdges.permute();

		if(forbid != 0)
			m_inserter.get().call(PG, deletedEdges, *forbid);
		else
			m_inserter.get().call(PG, deletedEdges);
	
		crossingNumber = PG.computeNumberOfCrossings();
		
		if(i == 1 || crossingNumber < bestcr) {
			foundSolution = true;
			bestcr = crossingNumber;
			numNS = PG.numberOfNodeSplits();
			numSN = PG.numberOfSplittedNodes();
		}
		
		PG.initCC(cc);
	}
	
	crossingNumber = bestcr;
	
	return retFeasible;
}


} // namspace ogdf
