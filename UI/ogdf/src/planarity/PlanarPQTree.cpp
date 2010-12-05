/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of the class PlanarPQTree.
 * 
 * Implements a PQTree with added features for the planarity test.
 * Used by PlanarModule.
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


#include <ogdf/internal/planarity/PlanarPQTree.h>

namespace ogdf{

// Overriding the function doDestruction (see basic.h)
// Allows deallocation of lists of PlanarLeafKey<indInfo*>
// and PQLeafKey<edge,indInfo*,bool> in constant time using OREAS
// memory management.
	
typedef PlanarLeafKey<indInfo*> *PtrPlanarLeafKeyI;

template<>
inline bool doDestruction<PtrPlanarLeafKeyI>(const PtrPlanarLeafKeyI*) { return false; }


typedef PQLeafKey<edge,indInfo*,bool> *PtrPQLeafKeyEIB;

template<>
inline bool doDestruction<PtrPQLeafKeyEIB>(const PtrPQLeafKeyEIB*) { return false; }



// Replaces the pertinent subtree by a P-node with leaves as children
// corresponding to the incoming edges of the node v. These edges
// are to be specified by their keys stored in leafKeys.
void PlanarPQTree::ReplaceRoot(SListPure<PlanarLeafKey<indInfo*>*> &leafKeys)
{
   if (m_pertinentRoot->status() == FULL)
      ReplaceFullRoot(leafKeys);
   else
      ReplacePartialRoot(leafKeys);
}



// The function [[emptyAllPertinentNodes]] has to be called after a reduction
// has been processed. This overloaded function first destroys all full nodes
// by marking them as TO_BE_DELETED and then calling the base class function
// [[emptyAllPertinentNodes]].
void PlanarPQTree::emptyAllPertinentNodes()
{
	ListIterator<PQNode<edge,indInfo*,bool>*> it;
	for (it = m_pertinentNodes->begin(); it.valid(); it++)
	{
		PQNode<edge,indInfo*,bool>* nodePtr = (*it);
		if (nodePtr->status() == FULL)
			destroyNode(nodePtr);
	}
	if (m_pertinentRoot)
		m_pertinentRoot->status(FULL);

	PQTree<edge,indInfo*,bool>::emptyAllPertinentNodes();
}


// Initializes a PQTree by a set of leaves that will korrespond to
// the set of Keys stored in leafKeys.
int PlanarPQTree::Initialize(SListPure<PlanarLeafKey<indInfo*>*> &leafKeys)
{
	SListIterator<PlanarLeafKey<indInfo*>* >  it;
	SListPure<PQLeafKey<edge,indInfo*,bool>*> castLeafKeys;
	for (it = leafKeys.begin(); it.valid(); ++it)
		castLeafKeys.pushBack((PQLeafKey<edge,indInfo*,bool>*) *it);

	return PQTree<edge,indInfo*,bool>::Initialize(castLeafKeys);
}


// Reduction reduced a set of leaves determined by their keys stored 
// in leafKeys. Integer redNumber is for debugging only.
bool PlanarPQTree::Reduction(SListPure<PlanarLeafKey<indInfo*>*> &leafKeys)
{
	SListIterator<PlanarLeafKey<indInfo*>* >  it;
	SListPure<PQLeafKey<edge,indInfo*,bool>*> castLeafKeys;
	for (it = leafKeys.begin(); it.valid(); ++it)
		castLeafKeys.pushBack((PQLeafKey<edge,indInfo*,bool>*) *it);

	return PQTree<edge,indInfo*,bool>::Reduction(castLeafKeys);
}


// Function ReplaceFullRoot either replaces the full root 
// or one full child of a partial root of a pertinent subtree
// by a single P-node  with leaves corresponding the keys stored in leafKeys.
void PlanarPQTree::ReplaceFullRoot(SListPure<PlanarLeafKey<indInfo*>*> &leafKeys)
{
	if (!leafKeys.empty() && leafKeys.front() == leafKeys.back())
	{
		//ReplaceFullRoot: replace pertinent root by a single leaf
		PQLeaf<edge,indInfo*,bool> *leafPtr =
			OGDF_NEW PQLeaf<edge,indInfo*,bool>(m_identificationNumber++,
            EMPTY,(PQLeafKey<edge,indInfo*,bool>*)leafKeys.front());

		exchangeNodes(m_pertinentRoot,(PQNode<edge,indInfo*,bool>*) leafPtr);
		if (m_pertinentRoot == m_root)
			m_root = (PQNode<edge,indInfo*,bool>*) leafPtr;      
		m_pertinentRoot = 0;  // check for this emptyAllPertinentNodes
	}

	else if (!leafKeys.empty()) // at least two leaves
	{
		PQInternalNode<edge,indInfo*,bool> *nodePtr = 0; // dummy
		//replace pertinent root by a $P$-node
		if ((m_pertinentRoot->type() == PQNodeRoot::PNode) || 
			(m_pertinentRoot->type() == PQNodeRoot::QNode))
		{
			nodePtr = (PQInternalNode<edge,indInfo*,bool>*)m_pertinentRoot;
			nodePtr->type(PQNodeRoot::PNode);
			nodePtr->childCount(0);
			while (!fullChildren(m_pertinentRoot)->empty())
				removeChildFromSiblings(fullChildren(m_pertinentRoot)->popFrontRet());
		}      
		else if (m_pertinentRoot->type() == PQNodeRoot::leaf)
		{
			nodePtr = OGDF_NEW PQInternalNode<edge,indInfo*,bool>(m_identificationNumber++,
														 PQNodeRoot::PNode,EMPTY);
			exchangeNodes(m_pertinentRoot,nodePtr);
			m_pertinentRoot = 0;  // check for this emptyAllPertinentNodes
		}
		
		SListPure<PQLeafKey<edge,indInfo*,bool>*> castLeafKeys;
		SListIterator<PlanarLeafKey<indInfo*>* >  it;
		for (it = leafKeys.begin(); it.valid(); ++it)
			castLeafKeys.pushBack((PQLeafKey<edge,indInfo*,bool>*) *it);
		addNewLeavesToTree(nodePtr,castLeafKeys);
	}
}


// Function ReplacePartialRoot replaces all full nodes by a single P-node 
// with leaves corresponding the keys stored in leafKeys.
void PlanarPQTree::ReplacePartialRoot(SListPure<PlanarLeafKey<indInfo*>*> &leafKeys)
{
   m_pertinentRoot->childCount(m_pertinentRoot->childCount() + 1 -
							   fullChildren(m_pertinentRoot)->size());

   while (fullChildren(m_pertinentRoot)->size() > 1)
      removeChildFromSiblings(fullChildren(m_pertinentRoot)->popFrontRet());

   PQNode<edge,indInfo*,bool> *currentNode = fullChildren(m_pertinentRoot)->popFrontRet();

   currentNode->parent(m_pertinentRoot);
   m_pertinentRoot = currentNode;
   ReplaceFullRoot(leafKeys);
}


}
