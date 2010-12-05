/*
 * $Revision: 2045 $
 *
 * last checkin:
 *   $Author: tschaefer $
 *   $Date: 2010-10-06 23:47:55 +0200 (Wed, 06 Oct 2010) $
 ***************************************************************/

/** \file
 * \brief ScalingLayout scales and calls a secondary layout
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

#ifndef OGDF_SCALING_LAYOUT_H
#define OGDF_SCALING_LAYOUT_H

#include <ogdf/module/LayoutModule.h>
#include <ogdf/energybased/multilevelmixer/ModularMultilevelMixerLayout.h>
#include <ogdf/internal/energybased/MultilevelGraph.h>

namespace ogdf {

/*!\class ScalingLayout ScalingLayout.h "ogdf/energybased/multilevelmixer/ScalingLayout.h"
 * \brief Scales a Graph relative to the ScalingType.
 *
 * For use with ModularMultilevelMixer.
 */
class OGDF_EXPORT ScalingLayout : public ogdf::LayoutModule
{
public:
	/*!
	 * \brief To define the relative scale used for a Graph, the ScalingType is applied.
	 */
	enum ScalingType {
		//! Scales by a factor relative to the drawing.
		st_relativeToDrawing,      
		/*! 
		 * Scales by a factor relative to the avg edge weights
		 * to be used in combination with the fixed edge length
		 * setting in ModularMultilevelMixerLayout.
		 */
		st_relativeToAvgLength, 
		//! Scales by a factor relative to the desired Edgelength m_desEdgeLength.
		st_relativeToDesiredLength, 
		//! Absolute factor, can be used to scale relative to level size change.
		st_absolute
	};

	ScalingLayout();

	/**
	 * \brief Computes a layout of graph \a GA.
	 *
	 * @param GA is the input graph and will also be assigned the layout information.
	 */
	void call(GraphAttributes &GA);
	
	/**
	 * \brief Computes a layout of graph \a MLG.
	 *
	 * @param MLG is the input graph and will also be assigned the layout information.
	 */
	void call(MultilevelGraph &MLG);

	/*!
	 * \brief Sets the minimum and the maximum scaling factor.
	 *
	 * @param min sets the minimum
	 * @param max sets the maximum
	 */
	void setScaling(float min, float max);
	
	/*!
	 * \brief Sets how often the scaling should be repeated.
	 *
	 * @param steps is the number of repeats
	 */
	void setExtraScalingSteps(unsigned int steps);
	
	/*!
	 * \brief Sets a LayoutModule that should be applied after scaling.
	 *
	 * @param layout is the secondary LayoutModule
	 */
	void setSecondaryLayout(LayoutModule* layout);
	
	/*!
	 * \brief Is used top compute the scaling realtively to the level size change when ScalingType st_absolute is used.
	 *
	 * @param mmm is the ModularMultilevelMixer
	 */
	void setMMM(ModularMultilevelMixer* mmm);
	
	/*!
	 * \brief Sets a ScalingType wich sets the relative scale for the Graph
	 *
	 * @param type is the ScalingType
	 */
	void setScalingType(ScalingType type);
	
	/*!
	 * \brief Sets how often the LayoutModule should be applied.
	 *
	 * @param repeats is the number of repeats
	 */
	void setLayoutRepeats(unsigned int repeats);
	//TODO: only a workaround, this should be retrieved from the layout module
	//when we have a interface class on top of Layoutmodule that allows this
	void setDesiredEdgeLength(double eLength);

private:

	// Usually a simple force-directed / energy-based Layout should be chosen.
	LayoutModule * m_secondaryLayoutModule;

	float m_minScaling;
	float m_maxScaling; 
	ModularMultilevelMixer* m_mmm;//!< Used to derive level size ratio if st_absolute 
	double m_desEdgeLength;

	// 0 = scale to maxScaling only
	unsigned int m_extraScalingSteps;

	unsigned int m_layoutRepeats;

	ScalingType m_scalingType;
};

} // namespace ogdf

#endif
