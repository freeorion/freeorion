/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief useable example of the Modular Multilevel Mixer
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

#include <ogdf/energybased/multilevelmixer/MMMExampleNoTwistLayout.h>
#include <ogdf/basic/PreprocessorLayout.h>
#include <ogdf/packing/ComponentSplitterLayout.h>
#include <ogdf/energybased/multilevelmixer/ModularMultilevelMixerLayout.h>
#include <ogdf/energybased/multilevelmixer/ScalingLayout.h>
#include <ogdf/energybased/FastMultipoleEmbedder.h>

#include <ogdf/energybased/multilevelmixer/LocalBiconnectedMerger.h>
#include <ogdf/energybased/multilevelmixer/BarycenterPlacer.h>

namespace ogdf {

MMMExampleNoTwistLayout::MMMExampleNoTwistLayout()
{
}


void MMMExampleNoTwistLayout::call(GraphAttributes &GA)
{
	MultilevelGraph MLG(GA);
	call(MLG);
	MLG.exportAttributes(GA);
}


void MMMExampleNoTwistLayout::call(MultilevelGraph &MLG)
{
	// Fast Multipole Embedder
	ogdf::FastMultipoleEmbedder * FME = new ogdf::FastMultipoleEmbedder();
	FME->setNumIterations(1000);
	FME->setRandomize(false);

	// Local Biconnected Merger
	ogdf::LocalBiconnectedMerger * LBCM = new ogdf::LocalBiconnectedMerger();
	LBCM->setFactor(2.0);
	LBCM->setEdgeLengthAdjustment(0); // BEFORE (but arg is int!): LBCM->setEdgeLengthAdjustment(0.1);

	// Barycenter Placer with weighted Positions
	ogdf::BarycenterPlacer * BP = new ogdf::BarycenterPlacer();
	BP->weightedPositionPritority(true);

	// No Scaling
	ogdf::ScalingLayout * SL = new ogdf::ScalingLayout();
	SL->setExtraScalingSteps(1);
	SL->setScaling(5.0, 10.0);
	SL->setScalingType(ogdf::ScalingLayout::st_relativeToDesiredLength);
	SL->setSecondaryLayout(FME);
	SL->setLayoutRepeats(1);

	ogdf::ModularMultilevelMixer MMM;
	MMM.setLayoutRepeats(1);
//	MMM.setAllEdgeLenghts(5.0);
//	MMM.setAllNodeSizes(1.0);
	MMM.setLevelLayoutModule(SL);
	MMM.setInitialPlacer(BP);
	MMM.setMultilevelBuilder(LBCM);

	// set Postprocessing Options
	MMM.setPostLayoutModule(0);
	MMM.setPostTimeFactor(0);
	MMM.setPostIterations(0);
	MMM.setPostProcessingAfterEveryStep(false);

	ogdf::TileToRowsCCPacker TTRCCP;
	ogdf::ComponentSplitterLayout CS(TTRCCP);
	CS.setLayoutModule(MMM);
	ogdf::PreprocessorLayout PPL;
	PPL.setLayoutModule(CS);
	PPL.setRandomizePositions(true);

	PPL.call(MLG);
}

} // namespace ogdf

