/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Places Nodes at the Positio of the merge-partner
 *
 * \author Gereon Bartel
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

#include <ogdf/energybased/multilevelmixer/MedianPlacer.h>
#include <algorithm>
#include <vector>

namespace ogdf {

void MedianPlacer::placeOneLevel(MultilevelGraph &MLG)
{
	int level = MLG.getLevel();
	while (MLG.getLevel() == level && MLG.getLastMerge() != 0)
	{
		placeOneNode(MLG);
	}
}


void MedianPlacer::placeOneNode(MultilevelGraph &MLG)
{
	node merged = MLG.undoLastMerge();
	int i = 0;
	std::vector<float> xVector;
	std::vector<float> yVector;
	adjEntry adj;
	forall_adj(adj, merged) {
		i++;
		xVector.push_back(MLG.x(adj->twinNode()));
		yVector.push_back(MLG.y(adj->twinNode()));
	}
	std::nth_element(xVector.begin(), xVector.begin()+(i/2), xVector.end());
	std::nth_element(yVector.begin(), yVector.begin()+(i/2), yVector.end());
	float x = xVector[i/2];
	float y = yVector[i/2];
	if (i % 2 == 0) {
		std::nth_element(xVector.begin(), xVector.begin()+(i/2)-1, xVector.end());
		std::nth_element(yVector.begin(), yVector.begin()+(i/2)-1, yVector.end());
		x += xVector[i/2 - 1];
		y += yVector[i/2 - 1];
		x /= 2.0;
		y /= 2.0;
	}
	MLG.x(merged, x + ((m_randomOffset)?(float)randomDouble(-1.0, 1.0):0.f));
	MLG.y(merged, y + ((m_randomOffset)?(float)randomDouble(-1.0, 1.0):0.f));
}

} // namespace ogdf
