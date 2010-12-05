/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of Mixed-Model layout algorithm.
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


#include <ogdf/planarlayout/MixedModelLayout.h>
#include <ogdf/augmentation/PlanarAugmentation.h>
#include <ogdf/augmentation/PlanarAugmentationFix.h>
#include <ogdf/planarlayout/BiconnectedShellingOrder.h>
#include <ogdf/module/MixedModelCrossingsBeautifierModule.h>
#include <ogdf/planarity/SimpleEmbedder.h>

#include "MixedModelBase.h"


namespace ogdf {


void MMOrder::init(
	PlanRep &PG,
	ShellingOrderModule &compOrder,
	adjEntry adjExternal)
{
	compOrder.callLeftmost(PG, m_lmc, adjExternal);
	m_left .init(1,m_lmc.length());
	m_right.init(1,m_lmc.length());
}


MixedModelLayout::MixedModelLayout()
{
	m_augmenter.set(new PlanarAugmentation);
	m_compOrder.set(new BiconnectedShellingOrder);
	m_crossingsBeautifier.set(new MMDummyCrossingsBeautifier);
	m_embedder.set(new SimpleEmbedder);
}


void MixedModelLayout::doCall(
	PlanRep &PG,
	adjEntry adjExternal,
	GridLayout &gridLayout,
	IPoint &boundingBox,
	bool fixEmbedding)
{
	// handle graphs with less than 3 nodes
	node v1, v2;
	switch (PG.numberOfNodes()) {
	case 0:
		boundingBox = IPoint(0,0);
		return;

	case 1:
		v1 = PG.firstNode();
		gridLayout.x(v1) = gridLayout.y(v1) = 0;
		boundingBox = IPoint(0,0);
		return;

	case 2:
		v1 = PG.firstNode();
		v2 = v1->succ();
		gridLayout.x(v1) = gridLayout.y(v1) = gridLayout.y(v2) = 0;
		gridLayout.x(v2) = 1;
		boundingBox = IPoint(1,0);
		return;
	}

	MixedModelBase mm(PG,gridLayout);

	if(fixEmbedding) {
		OGDF_ASSERT(PG.representsCombEmbedding());
		PlanarAugmentationFix fixAugmenter;
		mm.computeOrder(fixAugmenter, 0, adjExternal, m_compOrder.get());
	} else
		mm.computeOrder(m_augmenter.get(),&m_embedder.get(),0,m_compOrder.get());

	mm.assignIopCoords();
	mm.placeNodes();
	mm.postprocessing1();
	mm.setBends();
	mm.postprocessing2();

	m_crossingsBeautifier.get().call(PG,gridLayout);

	int xmin, ymin;
	gridLayout.computeBoundingBox(xmin,boundingBox.m_x,ymin,boundingBox.m_y);
}


} // end namespace ogdf
