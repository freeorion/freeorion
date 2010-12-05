/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of interface for two-layer crossing
 *        minimization algorithms.
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

#ifndef OGDF_TWO_LAYER_CROSS_MIN_H
#define OGDF_TWO_LAYER_CROSS_MIN_H



#include <ogdf/layered/Hierarchy.h>


namespace ogdf {


/**
 * \brief Interface of two-layer crossing minimization algorithms.
 *
 * The interface of a two-layer crossing minimization algorithm consists of
 * three methods:
 *   -# init(const Hierarchy & H) must be called first. This initializes the module
 *      for operating on hierarchy \a H.
 *   -# call(Level &L) (or operator()(Level &L)) performs two-layer crossing minimization,
 *      where \a L is the permutable level and the neighbor level of \a L (fixed
 *      level) is determined by the hierarchy (see documentation of class Hierarchy).
 *      Any number of call's may be performed once init() has been executed.
 *   -# cleanup() has to be called last and performs some final clean-up work.
 */
class OGDF_EXPORT TwoLayerCrossMin {
public:
	//! Initializes a two-layer crossing minimization module.
	TwoLayerCrossMin() { }

	virtual ~TwoLayerCrossMin() { }

	/**
	 * \brief Initializes the crossing minimization module for hierarchy \a H.
	 *
	 * @param H is the hierarchy on which the module shall operate.
	 */
	virtual void init(const Hierarchy & /*H*/) { }

	/**
	 * \brief Performs crossing minimization for level \a L.
	 *
	 * @param L is the level in the hierarchy on which nodes are permuted; the
	 *        neighbor level (fixed level) is determined by the hierarchy.
	 */
	virtual void call(Level &L) = 0;

	/**
	 * \brief Performs crossing minimization for level \a L.
	 *
	 * @param L is the level in the hierarchy on which nodes are permuted; the
	 *        neighbor level (fixed level) is determined by the hierarchy.
	 */
	void operator()(Level &level) {
		call(level);
	}

	//! Performs clean-up.
	virtual void cleanup() { }

	OGDF_MALLOC_NEW_DELETE
};


} // end namespace ogdf


#endif
