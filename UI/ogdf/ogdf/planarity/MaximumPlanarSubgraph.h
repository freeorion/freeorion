/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class MaximumPlanarSubgraph.
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

#ifndef OGDF_MAXIMUM_PLANAR_SUBGRAPH_H
#define OGDF_MAXIMUM_PLANAR_SUBGRAPH_H

#include <ogdf/basic/Module.h>
#include <ogdf/basic/Timeouter.h>

#include <ogdf/module/PlanarSubgraphModule.h>
#include <ogdf/cluster/ClusterGraph.h>

#include <ogdf/external/abacus.h>

namespace ogdf {

//--------------------------------------------------------------------------
//MaximumPlanarSubgraph
//Exact computation of a maximum planar subgraph
//--------------------------------------------------------------------------
class OGDF_EXPORT MaximumPlanarSubgraph : public PlanarSubgraphModule
{

#ifndef USE_ABACUS
protected:
    virtual ReturnType doCall(const Graph &G,
            const List<edge> &preferedEdges,
            List<edge> &delEdges,
            const EdgeArray<int>  *pCost,
            bool preferedImplyPlanar)
    { THROW_NO_ABACUS_EXCEPTION; return retError; };
};
#else // Use_ABACUS

public:
    //construction
    MaximumPlanarSubgraph() {}
    //destruction
    virtual ~MaximumPlanarSubgraph() {}
    
protected:
    //implements the Planar Subgraph interface
    //for the given graph, a clustered graph with only
    //a single root cluster is generated
    // computes set of edges delEdges, which have to be deleted
    // in order to get a planar subgraph; edges in preferedEdges
    // should be contained in planar subgraph
    // Status: pCost and preferedEdges are ignored so far
    virtual ReturnType doCall(const Graph &G,
            const List<edge> &preferedEdges,
            List<edge> &delEdges,
            const EdgeArray<int>  *pCost,
            bool preferedImplyPlanar);
};

#endif // USE_ABACUS

} //end namespace ogdf


#endif // OGDF_MAXIMUM_PLANAR_SUBGRAPH_H
