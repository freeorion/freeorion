/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Offers variety of possible algorithm calls for simultaneous
 * drawing.
 * 
 * \author Michael Schulz
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

#include<ogdf/simultaneous/SimDrawCaller.h>
#include<ogdf/layered/SugiyamaLayout.h>
#include<ogdf/planarity/PlanarizationLayout.h>
#include<ogdf/planarity/SubgraphPlanarizer.h>
#include<ogdf/planarity/VariableEmbeddingInserter.h>

#include <iostream>

namespace ogdf {

//*************************************************************
// refreshes m_esg
//
void SimDrawCaller::updateESG()
{
    edge e;
    forall_edges(e, *m_G)
		(*m_esg)[e] = m_GA->subGraphBits(e);

} // end updateESG


//*************************************************************
// Constructor
//
SimDrawCaller::SimDrawCaller(SimDraw &SD) : SimDrawManipulatorModule(SD)
{
    m_esg = OGDF_NEW EdgeArray<unsigned int>(*m_G);
    updateESG();

} // end constructor


//*************************************************************
// call for SugiyamaLayout
void SimDrawCaller::callSugiyamaLayout()
{    
    m_SD->addAttribute(GraphAttributes::nodeGraphics);	  
    m_SD->addAttribute(GraphAttributes::edgeGraphics);

    // nodes get default size
    node v;
    forall_nodes(v, *m_G)
		m_GA->height(v) = m_GA->width(v) = 5.0;
    
    // actual call of SugiyamaLayout
    updateESG();
    SugiyamaLayout SL;
    SL.setSubgraphs(m_esg); // needed to call SimDraw mode
    SL.call(*m_GA);

} // end callSugiyamaLayout


//*************************************************************
// call for PlanarizationLayout
void SimDrawCaller::callUMLPlanarizationLayout()
{
    // build corresponding UMLGraph
    UMLGraph UG(*m_G, GraphAttributes::edgeSubGraph);
    node v;
    forall_nodes(v,*m_G)
		UG.width(v) = UG.height(v) = 5.0;
    edge e;
    forall_edges(e,*m_G)
		UG.subGraphBits(e) = m_GA->subGraphBits(e);
    
    // actual call on UMLGraph
    PlanarizationLayout PL;
    PL.callSimDraw(UG);

    // transfer coordinats and edge bends to original
    m_SD->addAttribute(GraphAttributes::nodeGraphics);
    m_SD->addAttribute(GraphAttributes::edgeGraphics);
    forall_nodes(v,*m_G)
    {
		m_GA->x(v) = UG.x(v);
		m_GA->y(v) = UG.y(v);
    }
    forall_edges(e,*m_G)
		m_GA->bends(e) = UG.bends(e);

} // end callUMLPlanarizationLayout


//*************************************************************
// call for SubgraphPlanarizer
// returns crossing number
int SimDrawCaller::callSubgraphPlanarizer(int cc, int numberOfPermutations)
{   
    // transfer edge costs if existent
    EdgeArray<int> ec(*m_G, 1);
    if(m_GA->attributes() & GraphAttributes::edgeIntWeight)
    {
		edge e;
		forall_edges(e,*m_G)
			ec[e] = m_GA->intWeight(e);
    }

    // initialize
    updateESG();
    int crossNum = 0;
    PlanRep PR(*m_G);

    // actual call for connected component cc
    SubgraphPlanarizer SP;
    VariableEmbeddingInserter* vei = new VariableEmbeddingInserter();
    vei->removeReinsert(VariableEmbeddingInserter::rrIncremental);
    SP.setInserter(vei);
    SP.permutations(numberOfPermutations);
    SP.call(PR, cc, crossNum, &ec, 0, m_esg);

    // insert all dummy nodes into original graph *m_G
    NodeArray<node> newOrigNode(PR);
    node vPR;
    forall_nodes(vPR, PR)
    {
		if(PR.isDummy(vPR))
		{
			node vOrig = m_G->newNode();
			newOrigNode[vPR] = vOrig;
			m_SD->isDummy(vOrig) = true;
		}
		else
			newOrigNode[vPR] = PR.original(vPR); 
				//original nodes are saved
    }

    // insert all edges incident to dummy nodes into *m_G
    EdgeArray<bool> toBeDeleted(*m_G, false);
    EdgeArray<bool> visited(PR, false);
    forall_nodes(vPR, PR)
    {
		if(PR.isDummy(vPR))
		{
			node vNewOrig = newOrigNode[vPR]; //lebt in *m_G
			edge e;
			forall_adj_edges(e, vPR) //lebt in PR
			{
				if(!visited[e])
				{
					node w = e->opposite(vPR); //lebt in PR
					node wNewOrig = newOrigNode[w]; //lebt in *m_G
					edge eNewOrig = m_G->newEdge(vNewOrig,wNewOrig);
					m_GA->subGraphBits(eNewOrig) = m_GA->subGraphBits(PR.original(e));
					toBeDeleted[PR.original(e)] = true;
					visited[e] = true;
				}
			}
		}
    }

    // delete all old edges in *m_G that are replaced by dummy node edges
    List<edge> LE;
    m_G->allEdges(LE);
    forall_listiterators(edge, it, LE) 
    {
		if(toBeDeleted[ (*it) ])
			m_G->delEdge( (*it) );
    }
    
    return crossNum;
    
} // end callSubgraphPlanarizer

} // end namespace ogdf
