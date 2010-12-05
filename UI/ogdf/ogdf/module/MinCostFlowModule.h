/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of base class of min-cost-flow algorithms
 *
 * Includes some useful functions dealing with min-cost flow
 * (generater, checker).
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


#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_MIN_COST_FLOW_MODULE_H
#define OGDF_MIN_COST_FLOW_MODULE_H


#include <ogdf/basic/Graph.h>


namespace ogdf {


/**
 * \brief Interface for min-cost flow algorithms.
 */
class OGDF_EXPORT MinCostFlowModule
{
public:
	//! Initializes a min-cost flow module.
	MinCostFlowModule() { }

	// destruction
	virtual ~MinCostFlowModule() { }

	/**
	 * \brief Computes a min-cost flow in the directed graph \a G.
	 *
	 * \pre \a G must be connected, \a lowerBound[\a e] \f$\leq\f$ \a upperBound[\a e]
	 *      for all edges \a e, and the sum over all supplies must be zero.
	 *
	 * @param G is the directed input graph.
	 * @param lowerBound gives the lower bound for the flow on each edge.
	 * @param upperBound gives the upper bound for the flow on each edge.
	 * @param cost gives the costs for each edge.
	 * @param supply gives the supply (or demand if negative) of each node.
	 * @param flow is assigned the computed flow on each edge.
	 * @param dual is assigned the computed dual variables.
	 * \return true iff a feasible min-cost flow exists.
	 */
	virtual bool call(
		const Graph &G,                   // directed graph
		const EdgeArray<int> &lowerBound, // lower bound for flow
		const EdgeArray<int> &upperBound, // upper bound for flow
		const EdgeArray<int> &cost,       // cost of an edge
		const NodeArray<int> &supply,     // supply (if neg. demand) of a node
		EdgeArray<int> &flow,			  // computed flow
		NodeArray<int> &dual            // computed dual variables
		) = 0;


	//
	// static functions
	//

	/**
	 * \brief Generates an instance of a min-cost flow problem with \a n nodes and
	 *        \a m+\a n edges.
	 */
	static void generateProblem(
		Graph &G,
		int n,
		int m,
		EdgeArray<int> &lowerBound,
		EdgeArray<int> &upperBound,
		EdgeArray<int> &cost,
		NodeArray<int> &supply);


	/**
	 * \brief Checks if a given min-cost flow problem instance satisfies
	 *        the preconditions.
	 * The following preconditions are checked:
	 *   - \a lowerBound[\a e] \f$\leq\f$ \a upperBound[\a e] for all edges \a e
	 *   - sum over all \a supply[\a v] = 0
	 *
	 * @param G is the input graph.
	 * @param lowerBound gives the lower bound for the flow on each edge.
	 * @param upperBound gives the upper bound for the flow on each edge.
	 * @param supply gives the supply (or demand if negative) of each node.
	 * \return true iff the problem satisfies the preconditions.
	 */
	static bool checkProblem(
		const Graph &G,
		const EdgeArray<int> &lowerBound,
		const EdgeArray<int> &upperBound,
		const NodeArray<int> &supply);



	/**
	 * \brief checks if a computed flow is a feasible solution to the given problem
	 *        instance.
	 *
	 * Checks in particular if:
	 *   - \a lowerBound[\a e] \f$\leq\f$ \a flow[\a e] \f$\leq\f$ \a upperBound[\a e]
	 *   - sum \a flow[\a e], \a e is outgoing edge of \a v minus
	 *     sum \a flow[\a e], \a e is incoming edge of \a v equals \a supply[\a v]
	 *     for each node \a v
	 *
	 * @param G is the input graph.
	 * @param lowerBound gives the lower bound for the flow on each edge.
	 * @param upperBound gives the upper bound for the flow on each edge.
	 * @param cost gives the costs for each edge.
	 * @param supply gives the supply (or demand if negative) of each node.
	 * @param flow is the flow on each edge.
	 * @param value is assigned the value of the flow.
	 * \return true iff the solution is feasible.
	 */
	static bool checkComputedFlow(
		const Graph &G,
		EdgeArray<int> &lowerBound,
		EdgeArray<int> &upperBound,
		EdgeArray<int> &cost,
		NodeArray<int> &supply,
		EdgeArray<int> &flow,
		int &value);

	/**
	 * \brief checks if a computed flow is a feasible solution to the given problem
	 *        instance.
	 *
	 * Checks in particular if:
	 *   - \a lowerBound[\a e] \f$\leq\f$ \a flow[\a e] \f$\leq\f$ \a upperBound[\a e]
	 *   - sum \a flow[\a e], \a e is outgoing edge of \a v minus
	 *     sum \a flow[\a e], \a e is incoming edge of \a v equals \a supply[\a v]
	 *     for each node \a v
	 *
	 * @param G is the input graph.
	 * @param lowerBound gives the lower bound for the flow on each edge.
	 * @param upperBound gives the upper bound for the flow on each edge.
	 * @param cost gives the costs for each edge.
	 * @param supply gives the supply (or demand if negative) of each node.
	 * @param flow is the flow on each edge.
	 * \return true iff the solution is feasible.
	 */
	static bool checkComputedFlow(
		const Graph &G,
		EdgeArray<int> &lowerBound,
		EdgeArray<int> &upperBound,
		EdgeArray<int> &cost,
		NodeArray<int> &supply,
		EdgeArray<int> &flow)
	{
		int value;
		return checkComputedFlow(
			G,lowerBound,upperBound,cost,supply,flow,value);
	}
};


} // end namespace ogdf


#endif
