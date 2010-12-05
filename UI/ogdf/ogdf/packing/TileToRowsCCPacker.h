/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class TileToRowsCCPacker.
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

#ifndef OGDF_TILE_TO_ROWS_CC_PACKER_H
#define OGDF_TILE_TO_ROWS_CC_PACKER_H



#include <ogdf/module/CCLayoutPackModule.h>



namespace ogdf {


//! The tile-to-rows algorithm for packing drawings of connected components.
class OGDF_EXPORT TileToRowsCCPacker : public CCLayoutPackModule
{
	template<class POINT> struct RowInfo;

public:
	//! Creates an instance of tile-to-rows packer.
	TileToRowsCCPacker() { }

	virtual ~TileToRowsCCPacker() { }

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
	void call(Array<DPoint> &box,
		Array<DPoint> &offset,
		double pageRatio = 1.0);

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
	void call(Array<IPoint> &box,
		Array<IPoint> &offset,
		double pageRatio = 1.0);

private:
	template<class POINT>
	static void callGeneric(Array<POINT> &box,
		Array<POINT> &offset,
		double pageRatio);

	template<class POINT>
	static int findBestRow(Array<RowInfo<POINT> > &row,
		int nRows,
		double pageRatio,
		const POINT &rect);

};


} // end namespace ogdf


#endif
