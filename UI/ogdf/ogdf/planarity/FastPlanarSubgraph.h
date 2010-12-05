/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of the FastPlanarSubgraph.
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


#ifndef OGDF_FAST_PLANAR_SUBGRAPH_H
#define OGDF_FAST_PLANAR_SUBGRAPH_H



#include <ogdf/module/PlanarSubgraphModule.h>


namespace ogdf {

/**
 * \brief Computation of a planar subgraph using PQ-trees.
 *
 * Literature: Jayakumar, Thulasiraman, Swamy 1989
 *
 * <h3>Optional Parameters</h3>
 * 
 * <table>
 *   <tr>
 *     <th>Option</th><th>Type</th><th>Default</th><th>Description</th>
 *   </tr><tr>
 *     <td><i>runs</i></td><td>int</td><td>0</td>
 *     <td>the number of randomized runs performed by the algorithm; the best
 *         solution is picked among all the runs. If runs is 0, one
 *         deterministic run is performed.</td>
 *   </tr>
 * </table>
 * 
 * Observe that this algorithm by theory does not compute a maximal
 * planar subgraph. It is however the fastest known good heuristic. 
 */
class OGDF_EXPORT FastPlanarSubgraph : public PlanarSubgraphModule{

public:
	//! Creates an instance of the fast planar subgraph algorithm.
	FastPlanarSubgraph() : PlanarSubgraphModule() {
		m_nRuns = 0;
	};

	// destructor
	~FastPlanarSubgraph() {};


	// options

	//! Sets the number of randomized runs to \a nRuns.
	void runs (int nRuns) {
		m_nRuns = nRuns;
	}

	//! Returns the current number of randomized runs.
	int runs() const {
		return m_nRuns;
	}


protected:
	//! Returns true, if G is planar, false otherwise.
	/**
	 * \todo Add timeout support (limit number of runs when timeout is reached).
	 */
	ReturnType doCall(const Graph &G,
		const List<edge> &preferedEdges,
		List<edge> &delEdges,
		const EdgeArray<int>  *pCost,
		bool preferedImplyPlanar);


private:
	int m_nRuns;  //!< The number of runs for randomization.


	//! Computes the list of edges to be deleted in \a G.
	/** Also performs randomization of the planarization algorithm.
	 */
	void computeDelEdges(const Graph &G,
		const EdgeArray<int> *pCost,
		const EdgeArray<edge> *backTableEdges,
		List<edge> &delEdges);

	//! Performs a planarization on a biconnected component pf \a G.
	/** The numbering contains an st-numbering of the component.
	 */
	void planarize(const Graph &G,
				   NodeArray<int> &numbering,
				   List<edge> &delEdges);
};

}
#endif
