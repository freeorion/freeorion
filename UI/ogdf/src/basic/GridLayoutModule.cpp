/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements grid mapping mechanism of class GridLayoutModule
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


#include <ogdf/module/GridLayoutModule.h>


namespace ogdf {


void GridLayoutModule::call(GraphAttributes &AG)
{
	const Graph &G = AG.constGraph();

	// compute grid layout
	GridLayout gridLayout(G);
	doCall(G,gridLayout,m_gridBoundingBox);

	// transform grid layout to real layout
	mapGridLayout(G,gridLayout,AG);
}


void GridLayoutModule::callGrid(const Graph &G, GridLayout &gridLayout)
{
	gridLayout.init(G);
	doCall(G,gridLayout,m_gridBoundingBox);
}


void GridLayoutModule::mapGridLayout(const Graph &G,
	GridLayout &gridLayout,
	GraphAttributes &AG)
{
	double maxWidth = 0; // maximum width of columns and rows;
	double yMax = 0;

	node v;
	forall_nodes(v,G) {
		if (AG.width (v) > maxWidth) maxWidth = AG.width (v);
		if (AG.height(v) > maxWidth) maxWidth = AG.height(v);
		if (gridLayout.y(v) > yMax) yMax = gridLayout.y(v);
	}

	maxWidth += m_separation;

	// set position of nodes
	forall_nodes(v,G) {
		AG.x(v) = gridLayout.x(v) * maxWidth;
		AG.y(v) = (yMax - gridLayout.y(v)) * maxWidth;
	}

	// transform bend points of edges
	edge e;
	forall_edges(e,G) {
		DPolyline &dpl = AG.bends(e);
		dpl.clear();

		IPolyline ipl = gridLayout.polyline(e);
		ListConstIterator<IPoint> it;
		for(it = ipl.begin(); it.valid(); ++it) {
			const IPoint &ip = *it;
			dpl.pushBack(DPoint(ip.m_x*maxWidth, (yMax-ip.m_y)*maxWidth));
		}
	}
}


void PlanarGridLayoutModule::callFixEmbed(GraphAttributes &AG, adjEntry adjExternal)
{
	const Graph &G = AG.constGraph();

	// compute grid layout
	GridLayout gridLayout(G);
	doCall(G,adjExternal,gridLayout,m_gridBoundingBox,true);

	// transform grid layout to real layout
	mapGridLayout(G,gridLayout,AG);
}


void PlanarGridLayoutModule::callGridFixEmbed(
	const Graph &G,
	GridLayout &gridLayout,
	adjEntry adjExternal)
{
	gridLayout.init(G);
	doCall(G,adjExternal,gridLayout,m_gridBoundingBox,true);
}


void GridLayoutPlanRepModule::callGrid(PlanRep &PG, GridLayout &gridLayout)
{
	gridLayout.init(PG);
	doCall(PG,0,gridLayout,m_gridBoundingBox,false);
}

void GridLayoutPlanRepModule::callGridFixEmbed(
	PlanRep &PG,
	GridLayout &gridLayout,
	adjEntry adjExternal)
{
	gridLayout.init(PG);
	doCall(PG,adjExternal,gridLayout,m_gridBoundingBox,true);
}

void GridLayoutPlanRepModule::doCall(
	const Graph &G, 
	adjEntry adjExternal,
	GridLayout &gridLayout, 
	IPoint &boundingBox, 
	bool fixEmbedding)
{
	// create temporary graph copy and grid layout
	PlanRep PG(G);
	PG.initCC(0); // currently only for a single component!
	GridLayout glPG(PG);

	// determine adjacency entry on external face of PG (if required)
	if(adjExternal != 0) {
		edge eG  = adjExternal->theEdge();
		edge ePG = PG.copy(eG);
		adjExternal = (adjExternal == eG->adjSource()) ? ePG->adjSource() : ePG->adjTarget();
	}

	// call algorithm for copy
	doCall(PG,adjExternal,glPG,boundingBox,fixEmbedding);

	// extract layout for original graph
	node v;
	forall_nodes(v,G) {
		node vPG = PG.copy(v);
		gridLayout.x(v) = glPG.x(vPG);
		gridLayout.y(v) = glPG.y(vPG);
	}

	edge e;
	forall_edges(e,G) {
		IPolyline &ipl = gridLayout.bends(e);
		ipl.clear();

		ListConstIterator<edge> it;
		for(it = PG.chain(e).begin(); it.valid(); ++it)
			ipl.conc(glPG.bends(*it));
	}
}


} // end namespace ogdf

