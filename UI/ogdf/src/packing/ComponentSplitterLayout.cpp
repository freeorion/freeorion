/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Splits and packs the components of a Graph
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

//#define USE_OGL_DRAWER

#ifdef USE_OGL_DRAWER
#include <../ModularMultilevelMixer/ModularMultilevelMixer/mmm/DrawOGLGraph.h>
#endif

#include <ogdf/packing/ComponentSplitterLayout.h>
#include <ogdf/basic/geometry.h>
#include <ogdf/graphalg/ConvexHull.h>

namespace ogdf {

ComponentSplitterLayout::ComponentSplitterLayout(CCLayoutPackModule &packer)
:m_packer(packer)
{
	m_secondaryLayout = 0;
	m_number_of_components = 0;
	m_targetRatio = 1.f;
	m_border = 30;
}


ComponentSplitterLayout::~ComponentSplitterLayout()
{
}


void ComponentSplitterLayout::call(GraphAttributes &GA)
{
	MultilevelGraph MLG(GA);
	call(MLG);
	MLG.exportAttributes(GA);
}


void ComponentSplitterLayout::call(MultilevelGraph &MLG)
{
	splitIntoComponents(MLG);

	if (m_secondaryLayout != 0)
	{
		// Calculating Graph Drawing for every Component
		for (std::vector<MultilevelGraph *>::iterator i = m_components.begin();
			i != m_components.end(); i++)
		{
//			(*i)->writeGML("last_component.gml");
			m_secondaryLayout->call((**i));
#ifdef USE_OGL_DRAWER
//			DrawOGLGraph::setGraph(*i);
//			DrawOGLGraph::drawUntilEsc();
#endif

		}
	}

	reassembleDrawings(MLG);
}


void ComponentSplitterLayout::splitIntoComponents(MultilevelGraph &MLG)
{
	m_components = MLG.splitIntoComponents();
}


double atan2ex(double y, double x)
{
	double angle = atan2(y, x);

	if (x == 0)
	{
		if (y >= 0) {
			angle = 0.5 * pi;
		} else {
			angle = 1.5 * pi;
		}
	}

	if (y == 0)
	{
		if (x >= 0)
		{
			angle = 0.0;
		} else {
			angle = pi;
		}
	}

	return angle;
}


void ComponentSplitterLayout::reassembleDrawings(MultilevelGraph &MLG)
{
	Array<IPoint> box;
	Array<IPoint> offset;
	Array<DPoint> oldOffset;
	Array<float> rotation;
	ConvexHull CH;

	// rotate components and create bounding rects
	for (std::vector<MultilevelGraph *>::iterator i = m_components.begin();
		i != m_components.end(); i++)
	{

		(*i)->moveToZero();
		// calculate convex hull
		DPolygon hull = CH.call(**i);

#ifdef USE_OGL_DRAWER
//		DrawOGLGraph::setGraph(*i);
//		DrawOGLGraph::setPoly(&hull);
//		DrawOGLGraph::drawUntilEsc();
//		DrawOGLGraph::setPoly(0);
#endif

		double best_area = DBL_MAX;
		DPoint best_normal;
		double best_width = 0.0;
		double best_height = 0.0;

		// find best rotation by using every face as rectangle border once.
		for (DPolygon::iterator j = hull.begin(); j != hull.end(); j++) {
			DPolygon::iterator k = hull.cyclicSucc(j);

			double dist = 0.0;
			DPoint norm = CH.calcNormal(*k, *j);
			for (DPolygon::iterator z = hull.begin(); z != hull.end(); z++) {
				double d = CH.leftOfLine(norm, *z, *k);
				if (d > dist) {
					dist = d;
				}
			}

			double left = 0.0;
			double right = 0.0;
			norm = CH.calcNormal(DPoint(0, 0), norm);
			for (DPolygon::iterator z = hull.begin(); z != hull.end(); z++) {
				double d = CH.leftOfLine(norm, *z, *k);
				if (d > left) {
					left = d;
				}
				else if (d < right) {
					right = d;
				}
			}
			double width = left - right;

			dist = max(dist, 1.0);
			width = max(width, 1.0);

			double area = dist * width;

			if (area <= best_area) {
				best_height = dist;
				best_width = width;
				best_area = area;
				best_normal = CH.calcNormal(*k, *j);
			}
		}

		if (hull.size() <= 1) {
			best_height = 1.0;
			best_width = 1.0;
			best_area = 1.0;
			best_normal = DPoint(1.0, 1.0);
		}

		float angle = (float)(-atan2(best_normal.m_y, best_normal.m_x) + 1.5 * pi);
		if (best_width < best_height) {
			angle += 0.5f * (float)pi;
			double temp = best_height;
			best_height = best_width;
			best_width = temp;
		}
		rotation.grow(1, angle);
		double left = hull.front().m_x;
		double top = hull.front().m_y;
		double bottom = hull.front().m_y;
		// apply rotation to hull and calc offset
		for (DPolygon::iterator j = hull.begin(); j != hull.end(); j++) {
			DPoint tempP = *j;
			double ang = atan2(tempP.m_y, tempP.m_x);
			double len = sqrt(tempP.m_x*tempP.m_x + tempP.m_y*tempP.m_y);
			ang += angle;
			tempP.m_x = cos(ang) * len;
			tempP.m_y = sin(ang) * len;

			if (tempP.m_x < left) {
				left = tempP.m_x;
			}
			if (tempP.m_y < top) {
				top = tempP.m_y;
			}
			if (tempP.m_y > bottom) {
				bottom = tempP.m_y;
			}
		}
		oldOffset.grow(1, DPoint(left + 0.5 * (double)m_border, -1.0 * best_height + 1.0 * bottom + 0.0 * top + 0.5 * (double)m_border));

		// save rect
		int w = static_cast<int>(best_width);
		int h = static_cast<int>(best_height);
		box.grow(1, IPoint(w + m_border, h + m_border));
	}

	offset.init(box.size());

	// call packer
	m_packer.call(box, offset, m_targetRatio);

	int index = 0;
	// Apply offset and rebuild Graph
	for (std::vector<MultilevelGraph *>::iterator i = m_components.begin();
		i != m_components.end(); i++, index++)
	{
		MultilevelGraph *temp = *i;

		if (temp != 0)
		{
			float angle = rotation[index];
			// apply rotation and offset to all nodes
			node v;
			forall_nodes(v, temp->getGraph()) {
				float x = temp->x(v);
				float y = temp->y(v);
				float ang = atan2(y, x);
				float len = sqrt(x*x + y*y);
				ang += angle;
				x = cos(ang) * len;
				y = sin(ang) * len;

				x += static_cast<float>(offset[index].m_x);
				y += static_cast<float>(offset[index].m_y);

				x -= (float)oldOffset[index].m_x;
				y -= (float)oldOffset[index].m_y;

				temp->x(v, x);
				temp->y(v, y);
			}

			MLG.reInsertGraph(*temp);
			delete temp;
			*i = 0;
		}
	}

	MLG.moveToZero();
}


void ComponentSplitterLayout::setLayoutModule(LayoutModule &layout)
{
	m_secondaryLayout = &layout;
}

} // namespace ogdf
