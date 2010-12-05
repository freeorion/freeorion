/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of heuristics for two-layer crossing
 * minimization (BarycenterHeuristic, MedianHeuristic)
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


#include <ogdf/layered/BarycenterHeuristic.h>
#include <ogdf/layered/MedianHeuristic.h>


namespace ogdf {


//---------------------------------------------------------
// BarycenterHeuristic
// implements the barycenter heuristic for 2-layer 
// crossing minimization
//---------------------------------------------------------

void BarycenterHeuristic::call (Level &L)
{
	const Hierarchy& H = L.hierarchy();

	for (int i = 0; i <= L.high(); ++i) {
		node v = L[i];
		long sumpos = 0L;
    
		const Array<node> &adjNodes = L.adjNodes(v);
		for(int j = 0; j <= adjNodes.high(); ++j)
			sumpos += H.pos(adjNodes[j]);

		m_weight[v] = (adjNodes.high() < 0) ? 0.0 :
			double(sumpos) / double(adjNodes.size());
	}
	
	L.sort(m_weight);
}


//---------------------------------------------------------
// MedianHeuristic
// implements the median heuristic for 2-layer 
// crossing minimization
//---------------------------------------------------------

void MedianHeuristic::call (Level &L)
{
	const Hierarchy& H = L.hierarchy();

	for (int i = 0; i <= L.high(); ++i) {
		node v = L[i];
    
		const Array<node> &adjNodes = L.adjNodes(v);
		const int high = adjNodes.high();

		if (high < 0) m_weight[v] = 0;
		else if (high & 1)
			m_weight[v] = H.pos(adjNodes[high/2]) + H.pos(adjNodes[1+high/2]);
		else
			m_weight[v] = 2*H.pos(adjNodes[high/2]);
	}

	L.sort(m_weight,0,2*H.adjLevel(L.index()).high());
}


} // namespace ogdf
