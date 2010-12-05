/*
 * $Revision: 2013 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-08-27 14:56:33 +0200 (Fri, 27 Aug 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of Feasible Upward Planar Subgraph (FUPS) Module, an interface for subgraph computation.
 * 
 * \author Hoi-Ming Wong
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2008
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

#ifndef OGDF_FUPS_MODULE_H
#define OGDF_FUPS_MODULE_H

#include <ogdf/basic/Module.h>
#include <ogdf/upward/UpwardPlanRep.h>

namespace ogdf {

/**
 * \brief Interface for feasible upward planar subgraph algorithms.
 *
 */
class OGDF_EXPORT FUPSModule : public Module
{

public:

	//! Initializes a feasible upward planar subgraph module.
	FUPSModule() { }

	// destruction
	virtual ~FUPSModule() { }

	/**
	 * \brief Computes a feasible upward planar subgraph of the input graph.
	 *
	 * @param UPR represents the feasible upward planar subgraph after the call. \a UPR has to be initialzed as a
	 *		  UpwardPlanRep of the input graph \a G and is modified.
	 *		  The subgraph is represented as an upward planar representation. 		
	 * @param delEdges is the list of deleted edges which are deleted from the input graph in order to obtain the subgraph. 
			  The edges are edges of the original graph of UPR.
	 * \return the status of the result.
	 */
	ReturnType call(UpwardPlanRep &UPR,		
		List<edge> &delEdges)
	{
		return doCall(UPR, delEdges);		
	}

	//! Computes a upward planarized representation of the input graph (shorthand for call)
	ReturnType operator()(UpwardPlanRep &UPR,		
			List<edge> &delEdges)
	{
		return call(UPR, delEdges);
	}
	

protected:
	/**
	 * \brief Computes a feasible upward planar subgraph of the input graph.
	 *
	 * @param UPR represents the feasible upward planar subgraph after the call. \a UPR has to be initialzed as a
	 *        UpwardPlanRep of the input graph \a G and is modified.
	 *		  The subgraph is represented as an upward planar representation. 		
	 * @param delEdges is the list of deleted edges which are deleted from the input graph in order to obtain the subgraph.
			  The edges are edges of the original graph G.
	 * \return the status of the result.
	 */
	virtual ReturnType doCall(UpwardPlanRep &UPR,		
		List<edge> &delEdges) = 0;
		
	OGDF_MALLOC_NEW_DELETE

};

} // end namespace ogdf

#endif
