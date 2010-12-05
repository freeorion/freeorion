/*
 * $Revision: 2047 $
 *
 * last checkin:
 *   $Author: klein $
 *   $Date: 2010-10-13 17:12:21 +0200 (Wed, 13 Oct 2010) $
 ***************************************************************/

/** \file
 * \brief Places nodes on a circle around the barycenter of its neighbors.
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

#include <ogdf/energybased/multilevelmixer/CirclePlacer.h>
#include <ogdf/energybased/multilevelmixer/BarycenterPlacer.h>
#include <vector>

namespace ogdf {

CirclePlacer::CirclePlacer()
:m_circleSize(0.0f), m_fixedRadius(false), m_nodeSelection(nsNew)
{
}


void CirclePlacer::setRadiusFixed(bool fixed)
{
	m_fixedRadius = fixed;
}


void CirclePlacer::setCircleSize(float sizeIncrease)
{
	m_circleSize = sizeIncrease;
}


void CirclePlacer::setNodeSelection(NodeSelection nodeSel)
{
	m_nodeSelection = nodeSel;
}


void CirclePlacer::placeOneLevel(MultilevelGraph &MLG)
{
	int level = MLG.getLevel();
	DPoint center(0.0, 0.0);
	float radius = 0.0;

	std::map<node, bool> oldNodes;
	Graph &G = MLG.getGraph();
	double n = G.numberOfNodes();
	if (n > 0) {
		node v;
		forall_nodes(v, G) {
			oldNodes[v] = true;
			center = center + DPoint( MLG.x(v), MLG.y(v) );
		}
		center = DPoint(center.m_x / n, center.m_y / n);
		forall_nodes(v, G) {
			float r = sqrt( MLG.x(v) * MLG.x(v) + MLG.y(v) * MLG.y(v) );
			if (r > radius) radius = r;
		}
		radius += m_circleSize;
	} else {
		radius = 0.0f + m_circleSize;
	}

	BarycenterPlacer BP;
	BP.placeOneLevel(MLG);

	node v;
	forall_nodes(v, G) {
		if (!m_fixedRadius) {
			radius = (float)center.distance(DPoint(MLG.x(v), MLG.y(v))) + m_circleSize;
		}
		if (m_nodeSelection == nsAll
			|| (m_nodeSelection == nsNew && oldNodes[v])
			|| (m_nodeSelection == nsOld && !oldNodes[v]))
		{
			float angle = (float)(atan2( MLG.x(v) - center.m_x, -MLG.y(v) + center.m_y) - 0.5 * pi);
			MLG.x(v, cos(angle) * radius + ((m_randomOffset)?(float)randomDouble(-1.0, 1.0):0.f));
			MLG.y(v, sin(angle) * radius + ((m_randomOffset)?(float)randomDouble(-1.0, 1.0):0.f));
		}
	}
}

} // namespace ogdf
