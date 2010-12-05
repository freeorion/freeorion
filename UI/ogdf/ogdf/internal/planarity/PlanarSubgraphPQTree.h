/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class PlanarSubgraphPQTree.
 *
 * Datastructure used by the planarization module FastPlanarSubgraph.
 * Derived class of MaxSequencePQTree. Implements an Interface 
 * of MaxSequencePQTree for the planarization module FastPlanarSubgraph.
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


#ifndef OGDF_PLANAR_SUBGRAPH_PQTREE_H
#define OGDF_PLANAR_SUBGRAPH_PQTREE_H



#include <ogdf/internal/planarity/PQTree.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/SList.h>
#include <ogdf/internal/planarity/PlanarLeafKey.h>
#include <ogdf/internal/planarity/MaxSequencePQTree.h>

namespace ogdf {

// Overriding the function doDestruction (see basic.h)
// Allows deallocation of lists of PlanarLeafKey<whaInfo*>
// and PQLeafKey<edge,indInfo*,bool> in constant time using OREAS
// memory management.


typedef PQLeafKey<edge,whaInfo*,bool> *PtrPQLeafKeyEWB;

template<>
inline bool doDestruction<PtrPQLeafKeyEWB>(const PtrPQLeafKeyEWB*) { return false; }

typedef PlanarLeafKey<whaInfo*> *PtrPlanarLeafKeyW;

template<>
inline bool doDestruction<PtrPlanarLeafKeyW>(const PtrPlanarLeafKeyW*) { return false; }



class PlanarSubgraphPQTree: public MaxSequencePQTree<edge,bool> {

public:

	PlanarSubgraphPQTree() : MaxSequencePQTree<edge,bool>() {};

	virtual ~PlanarSubgraphPQTree() {};

	// Initializes a new PQ-tree with a set of leaves.
	virtual int Initialize(SListPure<PlanarLeafKey<whaInfo*>*> &leafKeys);
		
	// Replaces the pertinent subtree by a set of new leaves.
	void ReplaceRoot(SListPure<PlanarLeafKey<whaInfo*>*> &leafKeys);

	// Reduces a set of leaves.
    virtual bool Reduction(SListPure<PlanarLeafKey<whaInfo*>*> &leafKeys,
						   SList<PQLeafKey<edge,whaInfo*,bool>*> &eliminatedKeys);
private:

	// Replaces a pertinet subtree by a set of new leaves if the root 
	// is full.
	void ReplaceFullRoot(SListPure<PlanarLeafKey<whaInfo*>*> &leafKeys);

	// Replaces a pertinet subtree by a set of new leaves if the root 
	// is partial.
	void ReplacePartialRoot(SListPure<PlanarLeafKey<whaInfo*>*> &leafKeys);

	// Removes the leaves that have been marked for elimination from the PQ-tree.
	void removeEliminatedLeaves(SList<PQLeafKey<edge,whaInfo*,bool>*> &eliminatedKeys);

};	

}

#endif
