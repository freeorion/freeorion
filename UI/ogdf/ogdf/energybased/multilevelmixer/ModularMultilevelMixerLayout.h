/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief MMM is a Multilevel Graph drawing Algorithm that can use different modules.
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

#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_MODULAR_MULTILEVEL_MIXER_H
#define OGDF_MODULAR_MULTILEVEL_MIXER_H

#include <ogdf/module/LayoutModule.h>
#include <ogdf/internal/energybased/MultilevelGraph.h>
#include <ogdf/energybased/multilevelmixer/MultilevelBuilder.h>
#include <ogdf/energybased/multilevelmixer/InitialPlacer.h>

namespace ogdf {

class OGDF_EXPORT ModularMultilevelMixer : public ogdf::LayoutModule
{
private:

	// m_oneLevelLayoutModule should not completely discard the initial Layout
	//   but do incremental beautification.
	// Usually a simple force-directed / energy-based Layout should be chosen.
	LayoutModule * m_oneLevelLayoutModule;

	// m_postLayoutModule is only used to beautify the final drawing.
	// m_postLayoutModule should not completly discard the initial Layout
	//   but do incremental beautification.
	// Usually a simple force-directed / energy-based Layout should be chosen.
	LayoutModule * m_postLayoutModule;
	
	// Used for the last layout (i.e. on the largest graph in the hierarchy).
	// Can be used to speed up the computation if the level layout is relatively
	// slow. If not set, m_oneLevelLayoutModule is used instead.
	LayoutModule *m_finalLayoutModule;

	// All Levels of the Graph are stored here.
	MultilevelGraph * m_multilevelGraph;

	// Calculates the Multilevel graph from an input-graph
	MultilevelBuilder * m_multilevelBuilder;

	// Calculates initial Positions for Nodes that are inserted into the
	// previous Level.
	InitialPlacer * m_initialPlacement;

	//! the secondary layout will be called \a m_times to improve quality
	int m_times;

	// if set to a positive value all edge weights will be set to this value
	float m_fixedEdgeLength;

	// if set to a positive value all node sizes will be set to this value
	float m_fixedNodeSize;

	// if set to a positive value the postprocessing will be called at most that many times.
	long m_postIterations;

	// if set to a positive value the postprocessing will be called until factor * runtime of algorithm has passed.
	float m_postTimeFactor;

	// if set to true, postprocessing will not only be called at the end but after every multilevel step
	bool m_postAfterEveryStep;

	void callPost(MultilevelGraph &MLG, clock_t time);
	
	float m_coarseningRatio; //<! Ratio between sizes of previous (p) and current (c) level graphs c/p 
	
	bool m_levelBound; //<! Determines if computation is stopped when number of levels too high
	bool m_randomize; //<! Determines if initial random layout is computed

public:

	//error codes for calls: no error, level bound exceeded by merger step
	enum erc {ercNone, ercLevelBound};
	ModularMultilevelMixer();

	void setLevelLayoutModule(LayoutModule * levelLayout);
	//! Sets finalLayout to be the method used in the final layout step instead
	//! of the level layout method
	void setFinalLayoutModule(LayoutModule *finalLayout); 
	void setPostLayoutModule(LayoutModule * postLayout);
	void setMultilevelBuilder(MultilevelBuilder * levelBuilder);
	void setInitialPlacer(InitialPlacer * placement);
	void setLayoutRepeats(int times = 1);
	void setAllEdgeLengths(float len);
	void setAllNodeSizes(float size);
	void setPostIterations(long iter);
	void setPostTimeFactor(float timeFactor);
	void setPostProcessingAfterEveryStep(bool on);
	//! If set to true, initial random layout is computed 
	void setRandomize(bool b) {m_randomize = b;}
	//could set it directly
	void setLevelBound(bool b) {m_levelBound = b;}

	void call(GraphAttributes &GA);
	void call(MultilevelGraph &MLG);
	
	erc errorCode() {return m_errorCode;}
	
	//! Returns ratio between sizes of previous (p) and current (c) level graphs c/p
	float coarseningRatio() {return m_coarseningRatio;}

private:
	
	erc m_errorCode;
};

} // namespace ogdf

#endif
