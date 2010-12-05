/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Places Nodes with Solar System rules
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

#include <ogdf/energybased/multilevelmixer/SolarPlacer.h>

namespace ogdf {

void SolarPlacer::placeOneLevel(MultilevelGraph &MLG)
{
	int level = MLG.getLevel();
	while (MLG.getLevel() == level && MLG.getLastMerge() != 0)
	{
		placeOneNode(MLG);
	}
}


void SolarPlacer::placeOneNode(MultilevelGraph &MLG)
{
	NodeMerge * lastNM = MLG.getLastMerge();
	float x = 0.0;
	float y = 0.0;
	int i = 0;

	node sun = MLG.getNode(lastNM->m_changedNodes.front());
	std::vector< std::pair<int, float> > positions = lastNM->m_position;

	node merged = MLG.undoLastMerge();

	if (positions.size() > 0) {
		for (std::vector< std::pair<int, float> >::iterator j = positions.begin(); j != positions.end(); j++) {
			float factor = (*j).second;
			node other_sun = MLG.getNode((*j).first);
			i++;
			x += MLG.x(sun) * factor + MLG.x(other_sun) * (1.0f-factor);
			y += MLG.y(sun) * factor + MLG.y(other_sun) * (1.0f-factor);
		}
	} else {
		i++;
		x += MLG.x(sun);
		y += MLG.y(sun);
	}

	OGDF_ASSERT(i > 0);
	if (positions.size() == 0 || m_randomOffset) {
		x += (float)randomDouble(-1.0, 1.0);
		y += (float)randomDouble(-1.0, 1.0);
	}
	MLG.x(merged, (x / static_cast<float>(i)));
	MLG.y(merged, (y / static_cast<float>(i)));
}

} // namespace ogdf
