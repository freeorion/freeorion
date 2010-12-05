/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/
 
/** \file
 * \brief planar biconnected augmentation approximation algorithm
 * 
 * \author Bernd Zey
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

#ifndef OGDF_PLANAR_AUGMENTATION_H
#define OGDF_PLANAR_AUGMENTATION_H

#include <ogdf/module/AugmentationModule.h>
#include <ogdf/basic/List.h>
#include <ogdf/decomposition/DynamicBCTree.h>

namespace ogdf {


enum paStopCause {paPlanarity, paCDegree, paBDegree, paRoot};


class PlanarAugmentation;
class labelStruct;


typedef labelStruct* label;

/**
 * \brief auxiliary class for the planar augmentation algorithm
 * 
 *   A label contains several pendants, a parent- and a head- node.
 *   The head node is a cutvertex in the correspondign BC-Tree.
 *   The pendants can be connected by edges so planarity is maintained.
 */
class labelStruct {

friend class OGDF_EXPORT PlanarAugmentation;
friend class OGDF_EXPORT PlanarAugmentationFix;

private:

	/**
	 * \brief the "parent" of the pendants in the BC-Tree, m_parent is a b-vertex or a c-vertex
	 * if it is a b-vertex m_parent != 0
	 * otherwise m_parent == 0 and the parent is the head node
	 * m_head is always != 0
	 */
	node m_parent;
	/**
	 * \brief the cutvertex and perhaps (see above) the parent node
	 */	
	node m_head;
	/**
	 * \brief list with all pendants of the label
	 */		
	List<node> m_pendants;
	/**
	 * \brief the stop cause that occurs when traversing from the pendants to the bc-tree-root
	 * 		  computed in labelMatcher::followPath()
	 */	
	paStopCause m_stopCause;

public:
	labelStruct(node parent, node cutvertex, paStopCause sc = paBDegree) {
		m_parent = parent;
		m_head = cutvertex;
		m_stopCause = sc;
	};

	bool isBLabel() {
		return (m_parent != 0);
	};

	bool isCLabel() {
		return (m_parent == 0);
	};

	//! return pendant with number nr, starts counting at 0
	node getPendant(int nr) {
		return (nr < m_pendants.size()) ? (*(m_pendants.get(nr))) : 0;
	};

	node getFirstPendant() {
		return (m_pendants.size() > 0) ? m_pendants.front() : 0;
	};

	node getLastPendant() {
		return (m_pendants.size() > 0) ? m_pendants.back() : 0;
	};
	
    //! return number of pendants
	int size() {
		return m_pendants.size();
	};
	
	void removePendant(node pendant);

	void removePendant(ListIterator<node> it){
		m_pendants.del(it);
	};

	void removeFirstPendant() {
		if (m_pendants.size() > 0){
			m_pendants.popFront();
		}
	};
	
	void addPendant(node pendant) {
		m_pendants.pushBack(pendant);
	};

	void deleteAllPendants() {
		m_pendants.clear();
	};

	//! return the parent node. If the label is a c-label it returns m_head
	node parent() {
		return (m_parent != 0) ? m_parent : m_head;
	};

	//! returns the head node
	node head() {
		return m_head;
	};
	
	void setParent(node newParent){
		m_parent = newParent;
	}
	
	void setHead(node newHead){
		m_head = newHead;
	}
	
	paStopCause stopCause(){
		return m_stopCause;	
	}
	
	void stopCause(paStopCause sc){
		m_stopCause = sc;	
	}

	OGDF_NEW_DELETE
}; // class labelStruct



/**
 * \brief The algorithm for planar biconnectivity augmentation (Mutzel, Fialko).
 * 
 * The class \a PlanarAugmentation implements an augmentation algorithm
 * that augments a graph to a biconnected graph. In addition, if the graph was
 * planar before augmentation, the resulting graph will be biconnected and
 * planar.
 * The algorithm uses (dynamic) BC-trees and achieves biconnectivity by
 * inserting edges between nodes of pendants (that are leaves in the bc-tree).
 * The guaranteed approximation-quality is 5/3.
 *
 * The implementation is based on the following publication:
 *
 * Sergej Fialko, Petra Mutzel: <i>A New Approximation Algorithm for the Planar
 * Augmentation Problem</i>. Proc. SODA 1998, pp. 260-269.
 */
class OGDF_EXPORT PlanarAugmentation : public AugmentationModule {

public:
	//! Creates an instance of the planar augmentation algorithm.
	PlanarAugmentation() { }

	~PlanarAugmentation() { }

protected:
	/**
	 * \brief The implementation of the algorithm call.
	 * 
	 * \param G is the working graph.
	 * \param L is the list of all new edges.
	 */
	void doCall(Graph& G, List<edge>& L);

	
private:
	/**
	 * \brief Counts the number of planarity tests.
	 */
	int m_nPlanarityTests;
	
	/**
	 * \brief The working graph.
	 */
	Graph* m_pGraph;
	/**
	 * \brief The corresponding BC-Tree.
	 */
	DynamicBCTree* m_pBCTree;
	
	/**
	 * \brief The inserted edges by the algorithm.
	 */
	List<edge>* m_pResult;
	
	/**
	 * \brief The list of all labels, sorted by size (decreasing).
	 */
	List<label> m_labels;
	/**
	 * \brief The list of all pendants (leaves in the BC-Tree).
	 */
	List<node> m_pendants;

	/**
	 * \brief The list of pendants that has to be deleted after each reduceChain.
	 */
	List<node> m_pendantsToDel;
	
	/**
	 * \brief The label a BC-Node belongs to.
	 */
	NodeArray<label> m_belongsTo;
	/**
	 * \brief The list iterator in m_labels if the node in the BC-Tree is a label.
	 */
	NodeArray< ListIterator<label> > m_isLabel;

	/**
	 * \brief Stores for each node of the bc-tree the children that have an adjacent bc-node 
	 * 		  that doesn't belong to the same parent-node.
	 * 
	 * This is necessary because the bc-tree uses an union-find-data-structure to store 
	 * dependancies between bc-nodes. The adjacencies in the bc-tree won't be updated.
	 */
	NodeArray< SList<adjEntry> > m_adjNonChildren; 	


private:

	/**
	 * \brief The main function for planar augmentation.
	 */
	void augment();
	
	/**
	 * \brief Makes the graph connected by new edges between pendants of
	 *  	  the connected components
	 */
	void makeConnectedByPendants();

	/**
	 * \brief Is called for every pendant-node. It traverses to the 
	 * 		  root and creates a label or updates one.
	 * 
	 * \param p is a pendant in the BC-Tree.
	 * \param labelOld is the old label of \a p.
	 */
	void reduceChain(node p, label labelOld = 0);

	/**
	 * \brief Is called in reduceChain. It traverses to the root and checks 
	 * 		  several stop conditions.
	 * 
	 * \param v is a node of the BC-Tree.
	 * \param last is the last found C-vertex in the BC-Tree, is modified by
	 * 		  the method.
	 * \return the stop-cause.
	 */	
	paStopCause followPath(node v, node& last);

	/**
	 * \brief Checks planarity for a new edge (v1,v2) in the original graph.
	 * 
	 * \param v1, \ v2 are nodes of the original graph.
	 * \return true iff the graph (including the new edge) is planar.
	 */	
	bool planarityCheck(node v1, node v2);
	
	/**
	 * \brief Returns a node that belongs to bc-node v and is adjacent to the cutvertex.
	 * 
	 * \param v is a node in the BC-Tree.
	 * \param cutvertex is the last cutvertex found.
	 * \return a node of the original graph.
	 */	
	node adjToCutvertex(node v, node cutvertex = 0);
	
	/**
	 * \brief Traverses from pendant to ancestor and returns the
	 *  	  last node before ancestor on the path.
	 */
	node findLastBefore(node pendant, node ancestor);

	/**
	 * \brief Deletes the pendant p, removes it from the corresponding label
	 * 		  and updates the label-order.
	 */	
	void deletePendant(node p, bool removeFromLabel = true);
	/**
	 * \brief Adds a pendant p to the label l and updates the label-order.
	 */	
	void addPendant(node p, label& l);
	
	/**
	 * \brief Connects two pendants.
	 * 
	 * \return the new edge in the original graph.
	 */	 
	edge connectPendants(node pendant1, node pendant2);
	/**
	 * \brief Removes all pendants of a label.
	 */	
	void removeAllPendants(label& l);

	/**
	 * \brief Connects all pendants of label \a l with new edges.
	 */
	void joinPendants(label& l);

	/**
	 * \brief Connects the only pendant of l with a computed ancestor.
	 */	
	void connectInsideLabel(label& l);

	/**
	 * \brief Inserts label l into m_labels by decreasing order.
	 * 
	 * \return the corresponding list iterator.
	 */
	ListIterator<label> insertLabel(label l);

	/**
	 * \brief deletes label \a l.
	 */
	void deleteLabel(label& l, bool removePendants = true);

	/**
	 * \brief Inserts edges between pendants of label first and second.
	 * 		  first.size() is gerater than second.size() or equal.
	 */
	void connectLabels(label first, label second);

	/**
	 * \brief Creates a new label and inserts it into m_labels.
	 */
	label newLabel(node cutvertex, node p, paStopCause whyStop);

	/**
	 * \brief Finds two matching labels, so all pendants can be connected 
	 * 		  without losing planarity.
	 *
	 * \param first is the label with maximum size, modified by the function.
	 * \param second is the matching label, modified by the function:
	 * 		  0 if no matching is found.
	 * \return true iff a matching label is found.
	 */
	bool findMatching(label& first, label& second);
	
	/**
	 * \brief Checks if the pendants of label a and label b can be connected
	 * 		  without creating a new pendant.
	 */	
	bool connectCondition(label a, label b);

	/**
	 * \brief Updates the adjNonChildren-data.
	 * 
	 * \param newBlock is a new created block of the BC-Tree.
	 * \param path is the path in the BC-Tree between the two connected nodes.
	 */
	void updateAdjNonChildren(node newBlock, SList<node>& path);

	/**
	 * \brief Modifies the root of the BC-Tree that newRoot replaces oldRoot.
	 */
	void modifyBCRoot(node oldRoot, node newRoot);

	/**
	 * \brief Major updates caused by the new edges.
	 * 
	 * \param newEdges is a list of all new edges.
	 */	
	void updateNewEdges(const SList<edge> &newEdges);

	/**
	 * \brief Cleanup.
	 */	
	void terminate();
	
};	// class PlanarAugmentation


} // namespace ogdf

#endif
