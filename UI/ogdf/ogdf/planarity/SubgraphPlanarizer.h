/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class SubgraphPlanarizer.
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


#ifndef OGDF_SUBGRAPH_PLANARIZER_H
#define OGDF_SUBGRAPH_PLANARIZER_H

#include <ogdf/module/CrossingMinimizationModule.h>
#include <ogdf/module/PlanarSubgraphModule.h>
#include <ogdf/module/EdgeInsertionModule.h>
#include <ogdf/basic/ModuleOption.h>
#include <ogdf/basic/Logger.h>


namespace ogdf
{

//! The planarization approach for crossing minimization.
/**
 * This crossing minimization module represents a customizable implementation
 * of the planarization approach. This approach consists of two phases.
 * In the first phase, a planar subgraph is computed, and in the second
 * phase, the remaining edges are re-inserted one-by-one, each time with
 * as few crossings as possible; the crossings are then replaced by dummy
 * nodes of degree four, resulting in a <i>planarized representation</i> of the
 * graph.
 *
 * Both steps, the computation of the planar subgraph and the re-insertion
 * of a single edge, are implemented using module options. Additionaly,
 * the second phase can be repeated several times, each time with a randomly
 * permuted order of the edges to be re-inserted, and taking the solution
 * with the least crossings. This can improve the quality of the solution
 * significantly. More details on the planarization approach can be found in
 *
 * C. Gutwenger, P. Mutzel: <i>An Experimental Study of Crossing
 * Minimization Heuristics</i>. 11th International Symposium on %Graph
 * Drawing 2003, Perugia (GD '03), LNCS 2912, pp. 13-24, 2004.
 *
 * <H3>Optional parameters</H3>
 *
 * <table>
 *   <tr>
 *     <th><i>Option</i><th><i>Type</i><th><i>Default</i><th><i>Description</i>
 *   </tr><tr>
 *     <td><i>permutations</i><td>int<td>1
 *     <td>The number of permutations the (complete) edge insertion phase is repeated.
 *   </tr><tr>
 *     <td><i>setTimeout</i><td>bool<td>true
 *     <td>If set to true, the time limit is also passed to submodules; otherwise,
 *     a timeout might be checked late when a submodule requires a lot of runtime.
 *   </tr>
 * </table>
 *
 * <H3>%Module options</H3>
 * The various phases of the algorithm can be exchanged by setting
 * module options allowing flexible customization. The algorithm provides
 * the following module options:
 *
 * <table>
 *   <tr>
 *     <th><i>Option</i><th><i>Type</i><th><i>Default</i><th><i>Description</i>
 *   </tr><tr>
 *     <td><i>subgraph</i><td>PlanarSubgraphModule<td>FastPlanarSubgraph
 *     <td>The module for the computation of the planar subgraph.
 *   </tr><tr>
 *     <td><i>inserter</i><td>EdgeInsertionModule<td>VariableEmbeddingInserter
 *     <td>The module used for edge insertion. The edges not contained in the planar
 *     subgraph are re-inserted one-by-one, each with as few crossings as possible. 
 *   </tr>
 * </table>
*/
class OGDF_EXPORT SubgraphPlanarizer : public CrossingMinimizationModule, public Logger
{
	class CrossingStructure
	{
	public:
		CrossingStructure() : m_numCrossings(0) { }
		void init(PlanRep &PG, int weightedCrossingNumber);
		void restore(PlanRep &PG, int cc);
		
		int numberOfCrossings() const { return m_numCrossings; }
		int weightedCrossingNumber() const { return m_weightedCrossingNumber; }
		const SListPure<int> &crossings(edge e) const { return m_crossings[e]; }
		
	private:
		int m_numCrossings;
		int m_weightedCrossingNumber;
		EdgeArray<SListPure<int> > m_crossings;
	};

protected:
	//! Implements the algorithm call.
	virtual ReturnType doCall(PlanRep &PG,
		int cc,
		const EdgeArray<int>  &cost,
		const EdgeArray<bool> &forbid,
		const EdgeArray<unsigned int>  &subgraphs,
		int& crossingNumber);

public:
	//! Creates an instance of subgraph planarizer.
	SubgraphPlanarizer();

	//! Sets the module option for the computation of the planar subgraph.
	void setSubgraph(PlanarSubgraphModule *pSubgraph) {
		m_subgraph.set(pSubgraph);
	}

	//! Sets the module option for the edge insertion module.
	void setInserter(EdgeInsertionModule *pInserter) {
		m_inserter.set(pInserter);
	}

	//! Returns the number of permutations.
	int permutations() { return m_permutations; }

	//! Sets the number of permutations to \a p.
	void permutations(int p) { m_permutations = p; }

	//! Returns the current setting of options <i>setTimeout</i>.
	bool setTimeout() { return m_setTimeout; }

	//! Sets the option <i>setTimeout</i> to \a b.
	void setTimeout(bool b) { m_setTimeout = b; }
	
private:
	ModuleOption<PlanarSubgraphModule>  m_subgraph; //!< The planar subgraph algorithm.
	ModuleOption<EdgeInsertionModule>   m_inserter; //!< The edge insertion module.

	int m_permutations;	//!< The number of permutations.
	bool m_setTimeout;	//!< The option for setting timeouts in submodules.
};

}

#endif // OGDF_SUBGRAPH_PLANARIZER_H
