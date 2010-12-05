/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares the class BiconnectedShellingOrder...
 * 
 * ...which computes a shelling order for a biconnected planar graph.
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

#ifndef OGDF_BICONNECTED_SHELLING_ORDER_H
#define OGDF_BICONNECTED_SHELLING_ORDER_H


#include <ogdf/module/ShellingOrderModule.h>


namespace ogdf {

/**
 * \brief Computation of the shelling order for biconnected graphs.
 *
 * \pre The input graph has to be simple (no multi-edges, no self-loops),
 * planar and biconnected.
 */
class OGDF_EXPORT BiconnectedShellingOrder : public ShellingOrderModule
{
public:
	//! Creates a biconnected shelling order module.
	BiconnectedShellingOrder() {
		m_baseRatio = 0.33;
	}

protected:
	//! The actual implementation of the module call.
	virtual void doCall(const Graph &G,
		adjEntry adj,
		List<ShellingOrderSet> &partition);
};


} // end namespace ogdf


#endif
