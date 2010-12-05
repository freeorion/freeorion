/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class IncNodeInserter.
 *
 * This class represents the base class for strategies 
 * for the incremental drawing approach to insert nodes 
 * (having no layout fixation) into the fixed part of 
 * a PlanRep.
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


#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_INCNODEINSERTER_H
#define OGDF_INCNODEINSERTER_H


#include <ogdf/planarity/PlanRepInc.h>
#include <ogdf/basic/UMLGraph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/GraphObserver.h>

namespace ogdf {


//===============================================
//main function(s):
//
// insertcopyNode insert a node into a face
//
//===============================================


class OGDF_EXPORT IncNodeInserter
{
public:
	//creates inserter on PG
	IncNodeInserter(PlanRepInc &PG) : m_planRep(&PG ){};

	//insert copy in m_planRep for original node v
	virtual void insertCopyNode(node v, CombinatorialEmbedding &E,
		Graph::NodeType vTyp) = 0;

protected:
	//returns a face to insert a copy of v and a list of 
	//adjacency entries corresponding to the insertion adjEntries
	//for the adjacent edges 
	virtual face getInsertionFace(node v, CombinatorialEmbedding &E) = 0;

	PlanRepInc* m_planRep; //the PlanRep that is changed
}; //incnodeinserter

} //end namespace ogdf

#endif
