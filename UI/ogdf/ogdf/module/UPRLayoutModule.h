/*
 * $Revision: 2013 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-08-27 14:56:33 +0200 (Fri, 27 Aug 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of interface for layout algorithms for a UpwardPlanRep
 *      
 * 
 * \author Hoi-Ming Wong
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2008
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

#ifndef OGDF_UPR_LAYOUT_MODULE_H
#define OGDF_UPR_LAYOUT_MODULE_H



#include <ogdf/layered/Hierarchy.h>
#include <ogdf/basic/GraphCopyAttributes.h>


namespace ogdf {


/**
 * \brief Interface of hierarchy layout algorithms.
 *
 * \see SugiyamaLayout
 */
class OGDF_EXPORT UPRLayoutModule {
public:
	//! Initializes a upward planarized representation layout module.
	UPRLayoutModule() { }

	virtual ~UPRLayoutModule() { }

	/**
	 * \brief Computes a upward layout of \a UPR in \a AG.
	 * @param UPR is the upward planarized representation of the input graph. The original graph of UPR muss be the input graph.
	 * @param AG is assigned the hierarchy layout.
	 */
	void call(const UpwardPlanRep &UPR, GraphAttributes &AG) {	
		doCall(UPR, AG);		
	}	

	int numberOfLevels;

protected:
	/**
	 * \brief Implements the actual algorithm call.
	 *
	 * Must be implemented by derived classes.
	 *
	 * @param UPR is the upward planarized representation of the input graph. The original graph of UPR muss be the input graph.
	 * @param AG has to be assigned the hierarchy layout.
	 */
	virtual void doCall(const UpwardPlanRep &UPR, GraphAttributes &AG) = 0;

	OGDF_MALLOC_NEW_DELETE
};


} // end namespace ogdf


#endif
