/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of interface for mixed-model crossings
 * beautifier algorithms
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

#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_MIXED_MODEL_CROSSINGS_BEAUTIFIER_MODULE_H
#define OGDF_MIXED_MODEL_CROSSINGS_BEAUTIFIER_MODULE_H



#include <ogdf/planarity/PlanRep.h>
#include <ogdf/basic/GridLayout.h>


namespace ogdf {

/**
 * \brief The base class for Mixed-Model crossings beautifier algorithms.
 *
 * The class MixedModelCrossingsBeautifierModule is the base class for
 * mixed model bend crossing modules. Such a module transforms an input
 * graph \a G into an output graph \a G' such that crossings of edges don't
 * look weird.
 *
 * <H3>Implementation of Mixed-Model Crossings Beautifier Algorithms</H3>
 *
 * An implementation of a Mixed-Model crossings beautifier module must override
 * the protected method doCall(). 
 */

class OGDF_EXPORT MixedModelCrossingsBeautifierModule {
public:
	//! Initializes the Mixed-Model crossings beautifier module.
	MixedModelCrossingsBeautifierModule() { }

	// destruction
	virtual ~MixedModelCrossingsBeautifierModule() { }


	/*
	 * \brief Calls the Mixed-Model crossings beautifier module for graph \a PG and grid layout \a gl.
	 *
 	 * @param PG is the input graph.
	 * @param gl is the grid layout of \a PG.
	 */
	void call(const PlanRep &PG, GridLayout &gl);

	//! Returns the number of processed crossings.
	int numberOfCrossings() const {
		return m_nCrossings;
	}


protected:
	/**
	 * \brief Implements the crossings beautifier module.
	 *
	 * @param PG is the input graph.
	 * @param gl is the grid layout of \a PG.
	 * @param L is the list of crossing nodes.
	 */
	virtual void doCall(const PlanRep &PG, GridLayout &gl, const List<node> &L) = 0;

private:
	int m_nCrossings; //!< the number of processed crossings.

	OGDF_MALLOC_NEW_DELETE
};


//! Dummy implementation of Mixed-Model crossings beautifier.
/**
 * This implementation does no beautification at all and can thus be used
 * for obtaining the original Mixed-Model layout.
 */
class MMDummyCrossingsBeautifier : public MixedModelCrossingsBeautifierModule
{
protected:
	//!< Dummy implementation.
	void doCall(const PlanRep &, GridLayout &, const List<node> &) { }
};


} // end namespace ogdf

#endif
