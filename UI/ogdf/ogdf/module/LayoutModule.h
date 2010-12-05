/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Declaration of interface for layout algorithms (class
 *        LayoutModule)
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

#ifndef OGDF_LAYOUT_MODULE_H
#define OGDF_LAYOUT_MODULE_H



#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/internal/energybased/MultilevelGraph.h>

namespace ogdf {


/**
 * \brief Interface of general layout algorithms.
 *
 */
class OGDF_EXPORT LayoutModule {
public:
	//! Initializes a layout module.
	LayoutModule() { }

	virtual ~LayoutModule() { }

	/**
	 * \brief Computes a layout of graph \a GA.
	 *
	 * This method is the actual algorithm call and must be implemented by
	 * derived classes.
	 * @param GA is the input graph and will also be assigned the layout information.
	 */
	virtual void call(GraphAttributes &GA) = 0;

	/**
	 * \brief Computes a layout of graph \a GA.
	 *
	 * @param GA is the input graph and will also be assigned the layout information.
	 */
	void operator()(GraphAttributes &GA) { call(GA); }

	/**
	 * \brief Computes a layout of graph \a MLG.
	 *
	 * This method can be implemented optionally to allow a LayoutModule to modify the Graph.
	 * This allows some Layout Algorithms to save Memory, compared to a normal call(GA)
	 * DO NOT implement this if you are not sure whether this would save you Memory!
	 * This Method only helps if the Graph is already in the MultiLevelGraph Format
	 *   (or can be converted without creating a copy) AND the Layout would need a copy otherwise.
	 * All Incremental Layouts (especially energy based) CAN be called by ModularMultilevelMixer.
	 * The standard implementation converts the MLG to GA and uses call(GA).
	 *
	 * If implemented, the following Implementation of call(GA) is advised
	 *   to ensure consistent behaviour of the two call Methods:
	 * void YourLayout::call(GraphAttributes &GA)
	 * {
	 *     MultilevelGraph MLG(GA);
	 *     call(MLG);
	 *     MLG.exportAttributes(GA);
	 * }
	 *
	 * @param MLG is the input graph and will also be assigned the layout information.
	 */
	virtual void call(MultilevelGraph &MLG) {
		GraphAttributes GA(MLG.getGraph());
		MLG.exportAttributesSimple(GA);
		call(GA);
		MLG.importAttributesSimple(GA);
	};

	OGDF_MALLOC_NEW_DELETE
};


} // end namespace ogdf


#endif
