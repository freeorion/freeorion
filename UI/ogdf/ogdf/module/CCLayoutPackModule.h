/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of interface for algorithms that arrange/pack
 * layouts of connected components.
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

#ifndef OGDF_CC_LAYOUT_PACK_MODULE_H
#define OGDF_CC_LAYOUT_PACK_MODULE_H



#include <ogdf/basic/GraphAttributes.h>



namespace ogdf {


/**
 * \brief Base class of algorithms that arrange/pack layouts of connected
 *        components.
 *
 * \see PlanarizationLayout<BR>PlanarizationGridLayout
 */
class OGDF_EXPORT CCLayoutPackModule {
public:
	//! Initializes a layout packing module.
	CCLayoutPackModule() { }

	virtual ~CCLayoutPackModule() { }

	/**
	 * \brief Arranges the rectangles given by \a box.
	 *
	 * The algorithm call takes an input an array \a box of rectangles with
	 * real coordinates and computes in \a offset the offset to (0,0) of each
	 * rectangle in the layout.
	 *
	 * This method is the actual algorithm call and must be overridden by derived
	 * classes.
	 * @param box is the array of input rectangles.
	 * @param offset is assigned the offset of each rectangle to the origin (0,0).
	 *        The offset of a rectangle is its lower left point in the layout.
	 * @param pageRatio is the desired page ratio (width / height) of the
	 *        resulting layout.
	 */
	virtual void call(Array<DPoint> &box,
		Array<DPoint> &offset,
		double pageRatio = 1.0) = 0;

	/**
	 * \brief Arranges the rectangles given by \a box.
	 *
	 * The algorithm call takes an input an array \a box of rectangles with
	 * real coordinates and computes in \a offset the offset to (0,0) of each
	 * rectangle in the layout.
	 * @param box is the array of input rectangles.
	 * @param offset is assigned the offset of each rectangle to the origin (0,0).
	 *        The offset of a rectangle is its lower left point in the layout.
	 * @param pageRatio is the desired page ratio (width / height) of the
	 *        resulting layout.
	 */
	void operator()(Array<DPoint> &box,
		Array<DPoint> &offset,
		double pageRatio = 1.0)
	{
		call(box,offset,pageRatio);
	}

	/**
	 * \brief Arranges the rectangles given by \a box.
	 *
	 * The algorithm call takes an input an array \a box of rectangles with
	 * integer coordinates and computes in \a offset the offset to (0,0) of each
	 * rectangle in the layout.
	 *
	 * This method is the actual algorithm call and must be overridden by derived
	 * classes.
	 * @param box is the array of input rectangles.
	 * @param offset is assigned the offset of each rectangle to the origin (0,0).
	 *        The offset of a rectangle is its lower left point in the layout.
	 * @param pageRatio is the desired page ratio (width / height) of the
	 *        resulting layout.
	 */
	virtual void call(Array<IPoint> &box,
		Array<IPoint> &offset,
		double pageRatio = 1.0) = 0;

	/**
	 * \brief Arranges the rectangles given by \a box.
	 *
	 * The algorithm call takes an input an array \a box of rectangles with
	 * integer coordinates and computes in \a offset the offset to (0,0) of each
	 * rectangle in the layout.
	 * @param box is the array of input rectangles.
	 * @param offset is assigned the offset of each rectangle to the origin (0,0).
	 *        The offset of a rectangle is its lower left point in the layout.
	 * @param pageRatio is the desired page ratio (width / height) of the
	 *        resulting layout.
	 */
	void operator()(Array<IPoint> &box,
		Array<IPoint> &offset,
		double pageRatio = 1.0)
	{
		call(box,offset,pageRatio);
	}

	/**
	 * \brief Checks if the rectangles in \a box do not overlap for given offsets.
	 *
	 * This function serves for checking if the computed offsets are correct in
	 * the sense that the rectangles do not overlap in the resulting layout. 
	 * @param box is the array of rectangles.
	 * @param offset is the array of corresponding offsets.
	 */
	static bool checkOffsets(const Array<DPoint> &box,
		const Array<DPoint> &offset);

	/**
	 * \brief Checks if the rectangles in \a box do not overlap for given offsets.
	 *
	 * This function serves for checking if the computed offsets are correct in
	 * the sense that the rectangles do not overlap in the resulting layout. 
	 * @param box is the array of rectangles.
	 * @param offset is the array of corresponding offsets.
	 */
	static bool checkOffsets(const Array<IPoint> &box,
		const Array<IPoint> &offset);


	OGDF_MALLOC_NEW_DELETE

private:
	/**
	 * \brief Checks if the rectangles in \a box do not overlap for given offsets.
	 *
	 * This is a parameterized function for generic point types \a POINT.
	 * @param box is the array of rectangles.
	 * @param offset is the array of corresponding offsets.
	 */
	template<class POINT>
	static bool checkOffsetsTP(
		const Array<POINT> &box,
		const Array<POINT> &offset);
};


} // end namespace ogdf


#endif
