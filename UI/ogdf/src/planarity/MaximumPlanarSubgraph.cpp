 /*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class MaximumPlanarSubgraph
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
 
 #ifdef USE_ABACUS
 
 #include <ogdf/planarity/MaximumPlanarSubgraph.h>
 #include <ogdf/cluster/MaximumCPlanarSubgraph.h>
 #include <ogdf/planarity/PlanarModule.h>
 #include <ogdf/basic/simple_graph_alg.h>
 
 namespace ogdf {
 
 Module::ReturnType MaximumPlanarSubgraph::doCall(const Graph &G,
            const List<edge> &preferedEdges,
            List<edge> &delEdges,
            const EdgeArray<int>  *pCost,
            bool preferedImplyPlanar)
{

	if (G.numberOfEdges() < 9)
		return retOptimal;

	//if the graph is planar, we don't have to do anything
    PlanarModule pm;
    if (pm.planarityTest(G))
    	return retOptimal;

	//---------
    //Exact ILP
    MaximumCPlanarSubgraph mc;
    List<nodePair> addEdges;
        
	delEdges.clear();    
	
	node v;
	NodeArray<node> tableNodes(G,0);
	EdgeArray<edge> tableEdges(G,0);
	NodeArray<bool> mark(G,0);

	EdgeArray<int> componentID(G);

	// Determine Biconnected Components
	int bcCount = biconnectedComponents(G,componentID);

	// Determine edges per biconnected component
	Array<SList<edge> > blockEdges(0,bcCount-1);
	edge e;
	forall_edges(e,G)
	{
		if (!e->isSelfLoop())
			blockEdges[componentID[e]].pushFront(e);
	} 

	// Determine nodes per biconnected component.
	Array<SList<node> > blockNodes(0,bcCount-1);
	int i;
	for (i = 0; i < bcCount; i++)
	{
		SListIterator<edge> it;
		for (it = blockEdges[i].begin(); it.valid(); ++it)
		{
			e = *it;
			if (!mark[e->source()])
			{
				blockNodes[i].pushBack(e->source());
				mark[e->source()] = true;
			}
			if (!mark[e->target()])
			{
				blockNodes[i].pushBack(e->target());
				mark[e->target()] = true;
			}
		}
		SListIterator<node> itn;
		for (itn = blockNodes[i].begin(); itn.valid(); ++itn)
			mark[*itn] = false;
	}
	

	// Perform computation for every biconnected component
	ReturnType mr;
	
	if (bcCount == 1) {
		ClusterGraph CG(G);
		mr = mc.call(CG, delEdges, addEdges);
		
		return mr;
	} 
	else 
	{
		for (i = 0; i < bcCount; i++)
		{
			Graph C;
		
			SListIterator<node> itn;
			for (itn = blockNodes[i].begin(); itn.valid(); ++ itn)
			{
				v = *itn;
				node w = C.newNode();
				tableNodes[v] = w;
			}


			SListIterator<edge> it;
			for (it = blockEdges[i].begin(); it.valid(); ++it)
			{
				e = *it;
				edge f = C.newEdge(tableNodes[e->source()],tableNodes[e->target()]);
				tableEdges[e] = f;
			}

			// Construct a translation table for the edges.
			// Necessary, since edges are deleted in a new graph.
			// that represents the biconnectedcomponent of the original graph.
			EdgeArray<edge> backTableEdges(C,0);
			for (it = blockEdges[i].begin(); it.valid(); ++it)
				backTableEdges[tableEdges[*it]] = *it; 

			// The deleted edges of the biconnected component
			List<edge> delEdgesOfBC;				

			ClusterGraph CG(C);
			mr = mc.call(CG, delEdgesOfBC, addEdges);
			//abort if not optimal solution found, i.e., feasible is also not allowed
			if (mr != retOptimal)
				return mr;
		
			// get the original edges that are deleted and 
			// put them on the list delEdges.
			while (!delEdgesOfBC.empty())
				delEdges.pushBack(backTableEdges[delEdgesOfBC.popFrontRet()]);

		}
	}
	return mr;
}//docall for graph

} //end namespace ogdf
 
#endif //USE_ABACUS
