/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of the class EmbedPQTree.
 * 
 * Implements a PQTree with added features for the planar 
 * embedding algorithm. Used by PlanarModule.
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


#include <ogdf/internal/planarity/EmbedPQTree.h>

namespace ogdf{	

// Overriding the function doDestruction (see basic.h)
// Allows deallocation of lists of PQLeafKey<edge,indInfo*,bool>
// in constant time using OGDF memory management.


typedef PQLeafKey<edge,indInfo*,bool> *PtrPQLeafKeyEIB;

template<>
inline bool doDestruction<PtrPQLeafKeyEIB>(const PtrPQLeafKeyEIB*) { return false;  }



// Replaces the pertinent subtree by a P-node with leaves as children
// corresponding to the incoming edges of the node v. These edges
// are to be specified by their keys stored in leafKeys.
// The function returns the frontier of the pertinent subtree and
// the direction indicators found within the pertinent leaves.
// The direction indicators are returned in two list: 
// opposed: containing the keys of indicators pointing into reverse 
// frontier scanning direction (thus their corsponding list has to be
// reversed.
// nonOpposed: containing the keys of indicators pointing into 
// frontier scanning direction (thus their corsponding list do not need 
// reversed in the first place)

void EmbedPQTree::ReplaceRoot(
	SListPure<PlanarLeafKey<indInfo*>*> &leafKeys,
	SListPure<edge> &frontier,
	SListPure<node> &opposed,
	SListPure<node> &nonOpposed,
	node v)
{
	SListPure<PQBasicKey<edge,indInfo*,bool>*> nodeFrontier;

	if (leafKeys.empty() && m_pertinentRoot == m_root)
	{
		front(m_pertinentRoot,nodeFrontier);
		m_pertinentRoot = 0;  // check for this emptyAllPertinentNodes

	} else {
		if (m_pertinentRoot->status() == FULL)
			ReplaceFullRoot(leafKeys,nodeFrontier,v);
		else
			ReplacePartialRoot(leafKeys,nodeFrontier,v);
	}

	// Check the frontier and get the direction indicators.
	while (!nodeFrontier.empty())
	{
		PQBasicKey<edge,indInfo*,bool>* entry = nodeFrontier.popFrontRet();
		if (entry->userStructKey()) // is a regular leaf
			frontier.pushBack(entry->userStructKey());

		else if (entry->userStructInfo()) {
			if (entry->userStructInfo()->changeDir)
				opposed.pushBack(entry->userStructInfo()->v);
			else
				nonOpposed.pushBack(entry->userStructInfo()->v);
		}
	}
}


// The function [[emptyAllPertinentNodes]] has to be called after a reduction
// has been processed. This overloaded function first destroys all full nodes
// by marking them as TO_BE_DELETED and then calling the base class function
// [[emptyAllPertinentNodes]].
void EmbedPQTree::emptyAllPertinentNodes()
{
	ListIterator<PQNode<edge,indInfo*,bool>*> it;

	for (it = m_pertinentNodes->begin(); it.valid(); it++)
	{
		PQNode<edge,indInfo*,bool>* nodePtr = (*it);
		if (nodePtr->status() == FULL)
			destroyNode(nodePtr);
	}
	if (m_pertinentRoot) // Node was kept in the tree. Do not free it.
		m_pertinentRoot->status(FULL);

	PQTree<edge,indInfo*,bool>::emptyAllPertinentNodes();
}



void EmbedPQTree::clientDefinedEmptyNode(PQNode<edge,indInfo*,bool>* nodePtr)
{
	if (nodePtr->status() == INDICATOR)
		delete nodePtr;
	else
		PQTree<edge,indInfo*,bool>::clientDefinedEmptyNode(nodePtr);
}



// Initializes a PQTree by a set of leaves that will korrespond to
// the set of Keys stored in leafKeys.
int EmbedPQTree::Initialize(SListPure<PlanarLeafKey<indInfo*>*> &leafKeys)
{
	SListIterator<PlanarLeafKey<indInfo*>* > it;

	SListPure<PQLeafKey<edge,indInfo*,bool>*> castLeafKeys;
	for (it = leafKeys.begin(); it.valid(); ++it)
		castLeafKeys.pushBack((PQLeafKey<edge,indInfo*,bool>*) *it);

	return PQTree<edge,indInfo*,bool>::Initialize(castLeafKeys);
}


// Reduction reduced a set of leaves determined by their keys stored 
// in leafKeys. Integer redNumber is for debugging only.
bool EmbedPQTree::Reduction(SListPure<PlanarLeafKey<indInfo*>*> &leafKeys)
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
// Furthermore it scans the frontier of the pertinent subtree, and returns it
// in frontier.
// If called by ReplacePartialRoot, the function ReplaceFullRoot handles
// the introduction of the direction  indicator. (This must be indicated 
// by addIndicator.
// Node v determines the node related to the pertinent leaves. It is needed 
// to assign the dirrection indicator to this sequence.

void EmbedPQTree::ReplaceFullRoot(
	SListPure<PlanarLeafKey<indInfo*>*> &leafKeys,
	SListPure<PQBasicKey<edge,indInfo*,bool>*> &frontier,
	node v,
	bool addIndicator,
	PQNode<edge,indInfo*,bool> *opposite)
{
	EmbedIndicator *newInd = 0; 

	front(m_pertinentRoot,frontier);
	if (addIndicator)
	{
		indInfo *newInfo = OGDF_NEW indInfo(v);
		embedKey *nodeInfoPtr = OGDF_NEW embedKey(newInfo);
		newInd = OGDF_NEW EmbedIndicator(m_identificationNumber++,
			(PQNodeKey<edge,indInfo*,bool>*)nodeInfoPtr);
		newInd->setNodeInfo(nodeInfoPtr);
		nodeInfoPtr->setNodePointer(newInd);
	}

	if (!leafKeys.empty() && leafKeys.front() == leafKeys.back())
	{
		//ReplaceFullRoot: replace pertinent root by a single leaf
		if (addIndicator)
		{
			opposite = m_pertinentRoot->getNextSib(opposite);
			if (!opposite) // m_pertinentRoot is endmost child
			{
				addNodeToNewParent(m_pertinentRoot->parent(),newInd,
								   m_pertinentRoot,opposite);
			}
			else 
				addNodeToNewParent(0,newInd,m_pertinentRoot,opposite);

			// Setting the sibling pointers into opposite direction of
			// scanning the front allows to track swaps of the indicator
			newInd->changeSiblings(m_pertinentRoot,0);
			newInd->changeSiblings(opposite,0);
			newInd->putSibling(m_pertinentRoot,LEFT);
			newInd->putSibling(opposite,RIGHT);
		}
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
		//replace pertinent root by a $P$-node
		if (addIndicator)
		{
			opposite = m_pertinentRoot->getNextSib(opposite);
			if (!opposite) // m_pertinentRoot is endmost child
			{
				addNodeToNewParent(m_pertinentRoot->parent(),newInd,
								   m_pertinentRoot,opposite);
			}
			else 
				addNodeToNewParent(0,newInd,m_pertinentRoot,opposite);

			// Setting the sibling pointers into opposite direction of
			// scanning the front allows to track swaps of the indicator
			newInd->changeSiblings(m_pertinentRoot,0);
			newInd->changeSiblings(opposite,0);
			newInd->putSibling(m_pertinentRoot,LEFT);
			newInd->putSibling(opposite,RIGHT);
		}

		PQInternalNode<edge,indInfo*,bool> *nodePtr = 0; // dummy
		if ((m_pertinentRoot->type() == PQNodeRoot::PNode) || 
			(m_pertinentRoot->type() == PQNodeRoot::QNode))
		{
			nodePtr = (PQInternalNode<edge,indInfo*,bool>*)m_pertinentRoot;
			nodePtr->type(PQNodeRoot::PNode);
			nodePtr->childCount(0);
			while (!fullChildren(m_pertinentRoot)->empty())
			{	
				PQNode<edge,indInfo*,bool> *currentNode =
					fullChildren(m_pertinentRoot)->popFrontRet();
				removeChildFromSiblings(currentNode);
			}
		}      
		else if (m_pertinentRoot->type() == PQNodeRoot::leaf)
		{
			nodePtr = OGDF_NEW PQInternalNode<edge,indInfo*,bool>(m_identificationNumber++,
														 PQNodeRoot::PNode,EMPTY);
			exchangeNodes(m_pertinentRoot,nodePtr);
			m_pertinentRoot = 0;  // check for this emptyAllPertinentNodes
		}
	
		SListPure<PQLeafKey<edge,indInfo*,bool>*> castLeafKeys;
		SListIterator<PlanarLeafKey<indInfo*>* > it;
		for (it = leafKeys.begin(); it.valid(); ++it)
			castLeafKeys.pushBack((PQLeafKey<edge,indInfo*,bool>*) *it);
		addNewLeavesToTree(nodePtr,castLeafKeys);
	}  
}


// Function ReplacePartialRoot replaces all full nodes by a single P-node 
// with leaves corresponding the keys stored in leafKeys.
// Furthermore it scans the frontier of the pertinent subtree, and returns it
// in frontier.
// node v determines the node related to the pertinent leaves. It is needed 
// to assign the dirrection indicator to this sequence.

void EmbedPQTree::ReplacePartialRoot(
	SListPure<PlanarLeafKey<indInfo*>*> &leafKeys,
	SListPure<PQBasicKey<edge,indInfo*,bool>*> &frontier,
	node v)
{
	m_pertinentRoot->childCount(m_pertinentRoot->childCount() + 1 -
								fullChildren(m_pertinentRoot)->size());

	PQNode<edge,indInfo*,bool> *predNode = 0; // dummy
	PQNode<edge,indInfo*,bool> *beginSequence = 0; // marks begin consecuitve seqeunce
	PQNode<edge,indInfo*,bool> *endSequence	= 0; // marks end consecutive sequence
	PQNode<edge,indInfo*,bool> *beginInd = 0; // initially, marks direct sibling indicator
											  // next to beginSequence not contained 
											  // in consectuive sequence

	// Get beginning and end of sequence.
	while (fullChildren(m_pertinentRoot)->size())
	{
		PQNode<edge,indInfo*,bool> *currentNode = fullChildren(m_pertinentRoot)->popFrontRet();
		if (!clientSibLeft(currentNode) ||
			clientSibLeft(currentNode)->status() == EMPTY)
		{
			if (!beginSequence)
			{
				beginSequence = currentNode;
				predNode = clientSibLeft(currentNode);
				beginInd =PQTree<edge,indInfo*,bool>::clientSibLeft(currentNode);
			}
			else 
				endSequence = currentNode;
		}
		else if (!clientSibRight(currentNode) ||
				 clientSibRight(currentNode)->status() == EMPTY )
		{
			if (!beginSequence)
			{
				beginSequence = currentNode;
				predNode = clientSibRight(currentNode);
				beginInd =PQTree<edge,indInfo*,bool>::clientSibRight(currentNode);
			}
			else 
				endSequence = currentNode;
		}
	}

	SListPure<PQBasicKey<edge,indInfo*,bool>*> partialFrontier;
	

	// Now scan the sequence of full nodes. Remove all of them but the last.
	// Call ReplaceFullRoot on the last one.
	// For every full node get its frontier. Scan intermediate indicators.

	PQNode<edge,indInfo*,bool> *currentNode = beginSequence;
	while (currentNode != endSequence)
	{		
		PQNode<edge,indInfo*,bool>* nextNode = 
			clientNextSib(currentNode,predNode);
		front(currentNode,partialFrontier);
		frontier.conc(partialFrontier);

		PQNode<edge,indInfo*,bool>* currentInd	= PQTree<edge,indInfo*,bool>::
			clientNextSib(currentNode,beginInd);

		// Scan for intermediate direction indicators.
		while (currentInd != nextNode)
		{
			PQNode<edge,indInfo*,bool> *nextInd = PQTree<edge,indInfo*,bool>::
				clientNextSib(currentInd,currentNode);
			if (currentNode == currentInd->getSib(RIGHT)) //Direction changed
				currentInd->getNodeInfo()->userStructInfo()->changeDir = true;
			frontier.pushBack((PQBasicKey<edge,indInfo*,bool>*) 
								currentInd->getNodeInfo());
			removeChildFromSiblings(currentInd);
			m_pertinentNodes->pushBack(currentInd);
			currentInd = nextInd;
		}

		removeChildFromSiblings(currentNode);
		currentNode = nextNode;
	}
		
	currentNode->parent(m_pertinentRoot);
	m_pertinentRoot = currentNode;
	ReplaceFullRoot(leafKeys,partialFrontier,v,true,beginInd);
	frontier.conc(partialFrontier);
}



// Overloads virtual function of base class PQTree
// Allows ignoring the virtual direction indicators during 
// the template matching algorithm.
PQNode<edge,indInfo*,bool>* EmbedPQTree::clientSibLeft(
	PQNode<edge,indInfo*,bool> *nodePtr) const 
{
	PQNode<edge,indInfo*,bool> *predNode = nodePtr;
	nodePtr = PQTree<edge,indInfo*,bool>::clientSibLeft(predNode);
	while (nodePtr && nodePtr->status() == INDICATOR) 
	{
		PQNode<edge,indInfo*,bool> *holdSib = predNode; 
		predNode = nodePtr;
		nodePtr  = predNode->getNextSib(holdSib);
	}

	return nodePtr;
}


// Overloads virtual function of base class PQTree
// Allows ignoring the virtual direction indicators during 
// the template matching algorithm.
PQNode<edge,indInfo*,bool>* EmbedPQTree::clientSibRight(
	PQNode<edge,indInfo*,bool> *nodePtr) const 
{
	PQNode<edge,indInfo*,bool> *predNode = nodePtr;
	nodePtr = PQTree<edge,indInfo*,bool>::clientSibRight(predNode);
	while (nodePtr && nodePtr->status() == INDICATOR) 
	{
		PQNode<edge,indInfo*,bool> *holdSib = predNode; 
		predNode = nodePtr;
		nodePtr = predNode->getNextSib(holdSib);
	}

	return nodePtr;
}


// Overloads virtual function of base class PQTree
// Allows ignoring the virtual direction indicators during 
// the template matching algorithm.
PQNode<edge,indInfo*,bool>* EmbedPQTree::clientLeftEndmost(
	PQNode<edge,indInfo*,bool> *nodePtr) const 
{
	PQNode<edge,indInfo*,bool> *left = PQTree<edge,indInfo*,bool>::clientLeftEndmost(nodePtr);
  
	if (!left || left->status() != INDICATOR) 
		return left;
	else 
		return clientNextSib(left,NULL);
}


// Overloads virtual function of base class PQTree
// Allows ignoring the virtual direction indicators during 
// the template matching algorithm.
PQNode<edge,indInfo*,bool>* EmbedPQTree::clientRightEndmost(
	PQNode<edge,indInfo*,bool> *nodePtr) const 
{
	PQNode<edge,indInfo*,bool> *right = PQTree<edge,indInfo*,bool>::clientRightEndmost(nodePtr);

	if (!right || right->status() != INDICATOR) 
		return right;
	else 
		return clientNextSib(right,NULL);
}


// Overloads virtual function of base class PQTree
// Allows ignoring the virtual direction indicators during 
// the template matching algorithm.
PQNode<edge,indInfo*,bool>* EmbedPQTree::clientNextSib(
	PQNode<edge,indInfo*,bool> *nodePtr, 
	PQNode<edge,indInfo*,bool> *other) const 
{
	PQNode<edge,indInfo*,bool> *left = clientSibLeft(nodePtr);
	if (left  != other) return left;

	PQNode<edge,indInfo*,bool> *right = clientSibRight(nodePtr);
	if (right != other) return right;
	
	return 0;
}


// Overloads virtual function of base class PQTree
// Allows to print debug information on the direction indicators
const char* EmbedPQTree::clientPrintStatus(PQNode<edge,indInfo*,bool> *nodePtr)
{
	if (nodePtr->status() == INDICATOR)
		return "INDICATOR";
	else
		return PQTree<edge,indInfo*,bool>::clientPrintStatus(nodePtr);
}


// The function front scans the frontier of nodePtr. It returns the keys 
// of the leaves found in the frontier of nodePtr in a SListPure. 
// These keys include keys of direction indicators detected in the frontier.
//
// CAREFUL: Funktion marks all full nodes for destruction. 
//			Only to be used in connection with replaceRoot. 
//
void EmbedPQTree::front(
	PQNode<edge,indInfo*,bool>* nodePtr,
	SListPure<PQBasicKey<edge,indInfo*,bool>*> &keys)
{   
	Stack<PQNode<edge,indInfo*,bool>*> S;
	S.push(nodePtr);
  
	while (!S.empty())
	{
		PQNode<edge,indInfo*,bool> *checkNode = S.pop();

		if (checkNode->type() == PQNodeRoot::leaf)
			keys.pushBack((PQBasicKey<edge,indInfo*,bool>*) checkNode->getKey());      
		else
		{
			PQNode<edge,indInfo*,bool>* firstSon = 0;
			if (checkNode->type() == PQNodeRoot::PNode)
			{  
				firstSon = checkNode->referenceChild();
			}
 			else if (checkNode->type() == PQNodeRoot::QNode)
			{
				firstSon = checkNode->getEndmost(RIGHT);
				// By this, we make sure that we start on the left side
				// since the left endmost child will be on top of the stack
			}

			if (firstSon->status() == INDICATOR)
			{
				keys.pushBack((PQBasicKey<edge,indInfo*,bool>*) firstSon->getNodeInfo());
				m_pertinentNodes->pushBack(firstSon);
				destroyNode(firstSon);
			}
			else
				S.push(firstSon);

			PQNode<edge,indInfo*,bool> *nextSon = firstSon->getNextSib(0);
			PQNode<edge,indInfo*,bool> *oldSib = firstSon;
			while (nextSon && nextSon != firstSon)
			{
				if (nextSon->status() == INDICATOR)
				{
					// Direction indicators point with their left sibling pointer
					// in the direction of their sequence. If an indicator is scanned
					// from the opposite direction, coming from its right sibling
					// the corresponding sequence must be reversed.
					if (oldSib == nextSon->getSib(LEFT)) //Direction changed
						nextSon->getNodeInfo()->userStructInfo()->changeDir = true;
					keys.pushBack((PQBasicKey<edge,indInfo*,bool>*) nextSon->getNodeInfo());
					m_pertinentNodes->pushBack(nextSon);
				}
				else
					S.push(nextSon);

				PQNode<edge,indInfo*,bool> *holdSib = nextSon->getNextSib(oldSib);
				oldSib = nextSon;
				nextSon = holdSib;
			}
		}
	}
}



// The function front scans the frontier of nodePtr. It returns the keys 
// of the leaves found in the frontier of nodePtr in a SListPure. 
// These keys include keys of direction indicators detected in the frontier.
//
// No direction is assigned to the direction indicators.
//
void EmbedPQTree::getFront(
	PQNode<edge,indInfo*,bool>* nodePtr,
	SListPure<PQBasicKey<edge,indInfo*,bool>*> &keys)
{   
	Stack<PQNode<edge,indInfo*,bool>*> S;
	S.push(nodePtr);
  
	while (!S.empty())
	{
		PQNode<edge,indInfo*,bool> *checkNode = S.pop();

		if (checkNode->type() == PQNodeRoot::leaf)
			keys.pushBack((PQBasicKey<edge,indInfo*,bool>*) checkNode->getKey());      
		else
		{
			PQNode<edge,indInfo*,bool>* firstSon  = 0;
			if (checkNode->type() == PQNodeRoot::PNode)
			{  
				firstSon = checkNode->referenceChild();
			}
 			else if (checkNode->type() == PQNodeRoot::QNode)
			{
				firstSon = checkNode->getEndmost(RIGHT);
				// By this, we make sure that we start on the left side
				// since the left endmost child will be on top of the stack
			}

			if (firstSon->status() == INDICATOR)
			{
				keys.pushBack((PQBasicKey<edge,indInfo*,bool>*) firstSon->getNodeInfo());
			}
			else
				S.push(firstSon);

			PQNode<edge,indInfo*,bool> *nextSon = firstSon->getNextSib(0);
			PQNode<edge,indInfo*,bool> *oldSib = firstSon;
			while (nextSon && nextSon != firstSon)
			{
				if (nextSon->status() == INDICATOR)
					keys.pushBack((PQBasicKey<edge,indInfo*,bool>*) nextSon->getNodeInfo());
				else
					S.push(nextSon);

				PQNode<edge,indInfo*,bool> *holdSib = nextSon->getNextSib(oldSib);
				oldSib = nextSon;
				nextSon = holdSib;
			}
		}
	}
}



}
