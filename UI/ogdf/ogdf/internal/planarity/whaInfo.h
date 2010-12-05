/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class whaInfo.
 * 
 * \author Sebastian Leipert
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


#ifndef OGDF_WHA_INFO_H
#define OGDF_WHA_INFO_H


#include <ogdf/internal/planarity/PQNodeRoot.h>


namespace ogdf{

/**
The definitions for W_TYPE, B_TYPE, H_TYPE and A_TYPE
describe the type of a node during the computation of the
maximal pertinent sequence. A pertinent node X in the PQ-tree will be
either of type B, W, A or H. Together
with some other information stored at every node the pertinent leaves
in the frontier of X that have to be deleted. For further
description of the types see Jayakumar, Thulasiraman and Swamy 1989.
*/

enum whaType {
	W_TYPE,	B_TYPE, H_TYPE, A_TYPE
};


/*

NOTICE:

A compiler bug occurs, when declaring a template class friend of a 
nontemplate class. This forces us to reject making MaxSequencePQTree
a friend of whaInfo. 

template<class T, class Y> class MaxSequencePQTree;
class whaKey;
*/

class whaInfo
{
/*
	friend class whaKey;
	template<class T, class Y> friend class MaxSequencePQTree;
*/

public:

	//The deleteType is set to type b (= keep all leaves in the frontier).
	whaInfo() {
		m_a = 0;
		m_h = 0;
		m_w = 0;
		m_deleteTyp       = B_TYPE;
		m_pertLeafCount   = 0;
		m_notVisitedCount = 0;
		m_aChild          = 0;
		m_hChild1         = 0;
		m_hChild2         = 0;
		m_hChild2Sib      = 0;
	}


	~whaInfo() {}


	void defaultValues() {
		m_a = 0;
		m_h = 0;
		m_w = 0;
		m_deleteTyp = B_TYPE;
		m_pertLeafCount = 0;
		m_notVisitedCount = 0;
	}


//private:

   
	// number of pertinent leaves in the frontier of the node respectively the
	// number of leaves that have to be deleted in the frontier of the node to 
	// make it an empty node.
	int m_h;

	// number of pertinent leaves in the frontier of the node that have to be
	// deleted in order to create a node of type h, that is a node, where a 
	// permutation of the leaves of the node exist such that the remaining 
	// pertinent leaves form a consecutive sequence on one end of the permutation.
	int m_w;

	// number of pertinent leaves in the frontier of the node that have to be
	// deleted in order to create a node of type $a$, that is a node, where a 
	// permutation of the leaves of the node exist such that the remaining 
	// pertinent leaves form a consecutive somewhere within the permutation.
	int m_a;

	// deleteType is type of the node being either 
	// W_TYPE, B_TYPE, H_TYPE or A_TYPE. 
	int m_deleteTyp;

	//the number of pertinent leaves in the frontier of a node.
	int m_pertLeafCount;

	//counts the number of pertinent children, that have not been
	// processed yet during the computation of the w,h,a-numbering.
	int m_notVisitedCount;

	// a pointer to the child of node that has to be of type a if the
	// node itself has been determined to be of type a.
	PQNodeRoot *m_aChild;

	// a pointer to the child of node that has to be of type h if the
	// node itself has been determined to be of type h.
	PQNodeRoot *m_hChild1;

	// a pointer to the child of node that has to be of type h if the
	// node itself has been determined to be of type a and m_aChild does 
	// contain the empty pointer.
	PQNodeRoot *m_hChild2;


	// m_hChild2Sib is a pointer to the pertinent sibling of m_hChild2. This
	// pointer is necessary if the sequence of pertinent children is not unique.
	PQNodeRoot *m_hChild2Sib;

	OGDF_NEW_DELETE
};


class whaKey : public PQNodeKey<edge,whaInfo*,bool>
{
public: 
	
	whaKey(whaInfo* i) : PQNodeKey<edge,whaInfo*,bool>(i) {}

	virtual ~whaKey() {}

	const char* print()
	{
		if (!m_printString) 
			m_printString = new char[128]; 
		ogdf::sprintf(m_printString,128,"w=%d, h=%d, a=%d",
			m_userStructInfo->m_w,m_userStructInfo->m_h,
			m_userStructInfo->m_a);
		return m_printString;
	}
};

}

#endif


