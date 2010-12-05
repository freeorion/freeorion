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

#include <ogdf/energybased/multilevelmixer/RandomPlacer.h>

namespace ogdf {

RandomPlacer::RandomPlacer()
:m_circleSizeFactor(1.0)
{
}


void RandomPlacer::setCircleSize(double factor)
{
	m_circleSizeFactor = factor;
}


void RandomPlacer::placeOneLevel(MultilevelGraph &MLG)
{
	int level = MLG.getLevel();
	DPoint center(0.0, 0.0);
	double radius = 0.0;

	Graph &G = MLG.getGraph();
	double n = G.numberOfNodes();
	if (n > 0) {
		node v;
		forall_nodes(v, G) {
			center = center + DPoint( MLG.x(v), MLG.y(v) );
		}
		center = DPoint(center.m_x / n, center.m_y / n);
		forall_nodes(v, G) {
			double r = sqrt( MLG.x(v) * MLG.x(v) + MLG.y(v) * MLG.y(v) );
			if (r > radius) radius = r;
		}
		radius *= m_circleSizeFactor;
	} else {
		radius = 10.0 * m_circleSizeFactor;
	}

	while (MLG.getLevel() == level && MLG.getLastMerge() != 0)
	{
		placeOneNode(MLG, center, radius);
	}
}


void RandomPlacer::placeOneNode(MultilevelGraph &MLG, DPoint center, double radius)
{
	node merged = MLG.undoLastMerge();
	float angle = (float)randomDouble(0.0, 2 * pi);
	float randRadius = float(sqrt(randomDouble(0.0, radius * radius)));
	MLG.x(merged, cos(angle) * randRadius + ((m_randomOffset)?(float)randomDouble(-1.0, 1.0):0.f));
	MLG.y(merged, sin(angle) * randRadius + ((m_randomOffset)?(float)randomDouble(-1.0, 1.0):0.f));
}

} // namespace ogdf
