/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of an interface for c-planar subgraph algorithms.
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

#ifndef OGDF_CPLANAR_SUBGRAPH_MODULE_H
#define OGDF_CPLANAR_SUBGRAPH_MODULE_H

#include <ogdf/basic/Module.h>
#include <ogdf/basic/Timeouter.h>

#include <ogdf/cluster/ClusterGraph.h>
//#include <ogdf/internal/cluster/MaxCPlanar_Master.h>

namespace ogdf {

//--------------------------------------------------------------------------
//CPlanarSubgraphModule
//base class of algorithms for the computation of c-planar subgraphs
//--------------------------------------------------------------------------
/**
 * 
 * \brief Interface of algorithms for the computation of c-planar subgraphs.
 */
class CPlanarSubgraphModule : public Module, public Timeouter
{

public:
    //! Constructs a cplanar subgraph module
    CPlanarSubgraphModule() {}
    //! Destruction
    virtual ~CPlanarSubgraphModule() {}
    
    /**
     *  \brief  Computes set of edges delEdges, which have to be deleted  
     *  in order to get a c-planar subgraph.
     * 
     * Must be implemented by derived classes.
     * @param G is the clustergraph.
     * @param delEdges holds the edges not in the subgraph on return.
     *
    */
    ReturnType call(const ClusterGraph &G, List<edge> &delEdges) {
        return doCall(G, delEdges);
    }
   
    
protected:
    
    /** 
     * \brief Computes a maximum c-planar subgraph. 
     * 
     * If delEdges is empty on return, the clustered graph G is c-planar-
     * The actual algorithm call that must be implemented by derived classes!
     *  
     * @param delEdges holds the set of edges that have to be deleted.
     */
    virtual ReturnType doCall(const ClusterGraph &G,
            List<edge> &delEdges) = 0;

    OGDF_MALLOC_NEW_DELETE
};

} //end namespace ogdf


#endif // OGDF_CPLANAR_SUBGRAPH_MODULE_H
