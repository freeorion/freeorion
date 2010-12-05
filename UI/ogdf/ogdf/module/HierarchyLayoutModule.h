/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of interface hierarchy layout algorithms
 *        (3. phase of Sugiyama).
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

#ifndef OGDF_HIER_LAYOUT_MODULE_H
#define OGDF_HIER_LAYOUT_MODULE_H



#include <ogdf/layered/Hierarchy.h>
#include <ogdf/basic/GraphCopyAttributes.h>


namespace ogdf {


/**
 * \brief Interface of hierarchy layout algorithms.
 *
 * \see SugiyamaLayout
 */
class OGDF_EXPORT HierarchyLayoutModule {
public:
	//! Initializes a hierarchy layout module.
	HierarchyLayoutModule() { }

	virtual ~HierarchyLayoutModule() { }

	/**
	 * \brief Computes a hierarchy layout of \a H in \a AG.
	 * @param H is the input hierarchy.
	 * @param AG is assigned the hierarchy layout.
	 */
	void call(const Hierarchy& H, GraphAttributes &AG) {
		GraphCopyAttributes AGC(H,AG);
		doCall(H,AGC);		
		AGC.transform();
	}

	/**
	 * \brief Computes a hierarchy layout of \a H in \a AG.
	 * @param H is the input hierarchy.
	 * @param AG is assigned the hierarchy layout.
	 */
	void call(Hierarchy& H, GraphAttributes &AG) {
		GraphCopyAttributes AGC(H,AG);
		doCall(H,AGC);
		HierarchyLayoutModule::dynLayerDistance(AGC, H);
		HierarchyLayoutModule::addBends(AGC, H);
		AGC.transform();
	}


	/**
	 * \brief Computes a hierarchy layout of \a H in \a AG.
	 * @param H is the input hierarchy.
	 * @param AG is assigned the hierarchy layout.
	 * @param AGC is GraphCopyAttribute init. with H and AG
	 */
	//void call(const Hierarchy& H, GraphAttributes &, GraphCopyAttributes &AGC) {		
	//	doCall(H,AGC);
	//}


	static void addBends(GraphCopyAttributes &AGC, Hierarchy &H);

	static void dynLayerDistance(GraphCopyAttributes &AGC, Hierarchy &H);

private:
	
	//! after calling, ci (cj) contains the number of nodes of level i (j=i-1) which overlap the edge (s,t)
	static void overlap(GraphCopyAttributes &AGC, Hierarchy &H, node s, node t, int i, int &ci, int &cj);
	
protected:
	/**
	 * \brief Implements the actual algorithm call.
	 *
	 * Must be implemented by derived classes.
	 *
	 * @param H is the input hierarchy.
	 * @param AGC has to be assigned the hierarchy layout.
	 */
	virtual void doCall(const Hierarchy& H, GraphCopyAttributes &AGC) = 0;

	OGDF_MALLOC_NEW_DELETE

};


} // end namespace ogdf


#endif
