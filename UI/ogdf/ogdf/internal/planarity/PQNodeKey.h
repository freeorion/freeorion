/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration and implementation of the class PQNodeKey.
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


#ifndef OGDF_PQ_NODE_KEY_H
#define OGDF_PQ_NODE_KEY_H



#include <stdlib.h>
#include <ogdf/internal/planarity/PQBasicKey.h>

namespace ogdf {


template<class T,class X,class Y> class PQNode;

/**
 * The class template PQNodeKey is a derived class of class template
 * PQBasicKey. PQNodeKey is a concrete class. 
 * It is constructed to store any kind of information of nodes of the
 * PQ-tree. It may be used for both internal nodes as well as leaves.
 * 
 * The information is stored in \a m_userStructInfo and
 * is assigned to a unique node in the PQ-tree. This
 * unique node can be identified with the \a m_nodePointer of the
 * astract base class PQBasicKey. The
 * maintainance of this pointer is left to the user. By keeping the
 * responsibillity by the user, nodes with certain informations can
 * be identified and  accessed by her in constant time. This makes
 * the adaption of algorithms fast and easy. 
 */

template<class T,class X,class Y>
class PQNodeKey : public PQBasicKey<T,X,Y>
{  
public:

	//! Stores the information. Has to be overloaded by the client.
    X m_userStructInfo;

	// Constructor
    PQNodeKey(X info):PQBasicKey<T,X,Y>() { m_userStructInfo = info; }
    
	// Destructor
	virtual ~PQNodeKey() { }

	//! Returns 0.
    virtual T userStructKey() { return 0; }
    
	//! Returns \a m_userStructInfo.
	virtual X userStructInfo() { return m_userStructInfo; }

	//! Returns 0.
    virtual Y userStructInternal() { return 0; }
};

}

#endif
