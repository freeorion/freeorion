/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of the class EmbedIndicator. 
 * 
 * Implements the direction Indicator. Used by class EmbedPQTree.
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



#ifndef OGDF_EMBED_INDICATOR_H
#define OGDF_EMBED_INDICATOR_H

 

#include <ogdf/internal/planarity/PQNode.h>
#include <ogdf/internal/planarity/PQNodeKey.h>
#include <ogdf/internal/planarity/PQInternalKey.h>
#include <ogdf/internal/planarity/EmbedKey.h>

namespace ogdf{


#define INDICATOR 6

class EmbedIndicator : public PQNode<edge,indInfo*,bool>
{
public:

	EmbedIndicator(int count,PQNodeKey<edge,indInfo*,bool>* infoPtr)
		: PQNode<edge,indInfo*,bool>(count,infoPtr) { }

	virtual ~EmbedIndicator() {
		delete getNodeInfo()->userStructInfo();
		delete getNodeInfo();
	}

	PQNodeType  type() const { return leaf; }

	void type(PQNodeType) { }

	int  status() const { return INDICATOR; }

	void status(int) { }

	int  mark() const { return UNMARKED; }

	void mark(int) { }

	PQLeafKey<edge,indInfo*,bool>* getKey() const { return 0; }

	bool setKey(PQLeafKey<edge,indInfo*,bool>* pointerToKey) {
		return (pointerToKey == 0);
	}

	PQInternalKey<edge,indInfo*,bool>* getInternal() const { return 0; }

	bool setInternal(PQInternalKey<edge,indInfo*,bool>* pointerToInternal) {
		return (pointerToInternal == 0);
	};
};


}

#endif

