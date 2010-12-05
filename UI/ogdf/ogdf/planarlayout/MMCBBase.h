/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of base class for certain Mixed-Model
 * crossings beautifier.
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

#ifndef OGDF_MMCB_BASE_H
#define OGDF_MMCB_BASE_H



#include <ogdf/module/MixedModelCrossingsBeautifierModule.h>


namespace ogdf {

/**
 * \brief common base class for MMCBDoubleGrid and MMCBLocalStretch.
 *
 * MMCBBase contains the common functionality shared by the two Mixed-Model
 * crossings beautifier MMCBDoubleGrid and MMCBLocalStretch.
 */
class OGDF_EXPORT MMCBBase : public MixedModelCrossingsBeautifierModule
{
public:
	//! Constructor (does nothing).
	MMCBBase() { }

	~MMCBBase() { }

protected:
	static void insertBend(GridLayout &gl, edge e, node v, int x, int y);
	static void copyOn(int old_a[] , int new_a[]);
	static int workOn(GridLayout &gl, node v);
};


} // end namespace ogdf

#endif
