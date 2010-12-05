/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of node types and patterns for planar
 *        representations
 * 
 * \author Karsten Klein
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

//edge type patterns:
	//THREE TYPE LEVELS:
	//primary: holds information about structural/non-structural
    //		   nodes, this  influences the handling in algorithms
	//secondary: type of node, e.g. flow node, simple label node, ...
	//user edge types can be set locally

#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_NODE_TYPE_PATTERNS_H
#define OGDF_NODE_TYPE_PATTERNS_H

namespace ogdf {

	typedef long nodeType;

	enum UMLNodeTypePatterns {ntpPrimary = 0x0000000f, ntpSecondary = 0x000000f0,
		                       ntpTertiary = 0x00000f00, ntpFourth = 0x0000f000,
							   ntpUser = 0xff000000,
							   ntpAll = 0xffffffff
	}; //!!!attention sign, 7fffffff
	enum UMLNodeTypeConstants {
		//primary types (should be disjoint bits)
		ntPrimOriginal = 0x1, ntPrimCopy = 0x2,
		//secondary types: type of node (should be disjoint types, but not bits,
		//but may not completely cover others that are allowed to be set together)
		//preliminary: setsecondarytype deletes old type
		//defines the structure of the diagram, e.g. as flow transmitter
		ntSecStructural = 0x1, ntSecNonStructural = 0x2,
		//tertiary
		//crossing node, high/low degree expander
		ntTerCrossing = 0x1, ntTerExpander = 0x2, ntTerHDExpander = 0x6,
		ntTerLDExpander = 0xA,
		//fourth level types: special types
		//flow node, simple label node, type label node, expansion corner node
		ntFourFlow = 0x1, ntFourLabel = 0x2, ntFourType = 0x3, ntFourCorner = 0x4
        
		//user type hint: what you have done with the edge, e.g. brother edge
		//that is embedded crossing free and should be drawn bend free
	};
	enum UMLNodeTypeOffsets {
		ntoPrimary = 0, ntoSecondary = 4, ntoTertiary = 8, ntoFourth = 12, ntoFifth = 16,
		ntoUser = 24
	};

} //end namespace ogdf

#endif
