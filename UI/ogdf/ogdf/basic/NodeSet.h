/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration and implementation of class NodeSetSimple,
 *        NodeSetPure and NodeSet
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

#ifndef OGDF_NODE_SET_H
#define OGDF_NODE_SET_H


#include <ogdf/basic/NodeArray.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/SList.h>



namespace ogdf {


//---------------------------------------------------------
// NodeSetSimple
// maintains a subset S of the nodes contained in an associated
// graph G (only insertion of elements and clear operation)
//---------------------------------------------------------
class OGDF_EXPORT NodeSetSimple {
public:
	// creates a new empty face set associated with combinatorial embedding E
	NodeSetSimple(const Graph &G) : m_isContained(G,false) { }

	// destructor
	~NodeSetSimple() { }

	// inserts node v into set S
	// running time: O(1)
	// Precond.: v is a node in the associated graph
	void insert(node v) {
		OGDF_ASSERT(v->graphOf() == m_isContained.graphOf());
		bool &isContained = m_isContained[v];
		if (isContained == false) {
			isContained = true;
			m_nodes.pushFront(v);
		}
	}


	// removes all nodes from set S
	// running time: O(|S|)
	void clear() {
		SListIterator<node> it;
		for(it = m_nodes.begin(); it.valid(); ++it) {
			m_isContained[*it] = false;
		}
		m_nodes.clear();
	}


	// returns true iff node v is contained in S
	// running time: O(1)
	// Precond.: v is a node in the asociated graph
	bool isMember(node v) const {
		OGDF_ASSERT(v->graphOf() == m_isContained.graphOf());
		return m_isContained[v];
	}

	// returns the list of nodes contained in S
	const SListPure<node> &nodes() const {
		return m_nodes;
	}

private:
	// m_isContained[v] is true <=> v is contained in S
	NodeArray<bool> m_isContained;
	// list of nodes contained in S
	SListPure<node> m_nodes;
};



//---------------------------------------------------------
// NodeSetPure
// maintains a subset S of the nodes contained in an associated
// graph G (no efficient access to size of S)
//---------------------------------------------------------
class OGDF_EXPORT NodeSetPure {
public:
	// creates a new empty node set associated with graph G
	NodeSetPure(const Graph &G) : m_it(G,ListIterator<node>()) { }

	// destructor
	~NodeSetPure() { }

	// inserts node v into set S
	// running time: O(1)
	// Precond.: v is a node in the associated graph
	void insert(node v) {
		OGDF_ASSERT(v->graphOf() == m_it.graphOf());
		ListIterator<node> &itV = m_it[v];
		if (!itV.valid())
			itV = m_nodes.pushBack(v);
	}

	// removes node v from set S
	// running time: O(1)
	// Precond.: v is a node in the asociated graph
	void remove(node v) {
		OGDF_ASSERT(v->graphOf() == m_it.graphOf());
		ListIterator<node> &itV = m_it[v];
		if (itV.valid()) {
			m_nodes.del(itV);
			itV = ListIterator<node>();
		}
	}


	// removes all nodes from set S
	// running time: O(|S|)
	void clear() {
		ListIterator<node> it;
		for(it = m_nodes.begin(); it.valid(); ++it) {
			m_it[*it] = ListIterator<node>();
		}
		m_nodes.clear();
	}


	// returns true iff node v is contained in S
	// running time: O(1)
	// Precond.: v is a node in the asociated graph
	bool isMember(node v) const {
		OGDF_ASSERT(v->graphOf() == m_it.graphOf());
		return m_it[v].valid();
	}

	// returns the list of nodes contained in S
	const ListPure<node> &nodes() const {
		return m_nodes;
	}

private:
	// m_it[v] contains list iterator pointing to v if v is contained in S,
	// an invalid list iterator otherwise
	NodeArray<ListIterator<node> > m_it;
	// list of nodes contained in S
	ListPure<node> m_nodes;
};



//---------------------------------------------------------
// NodeSet
// maintains a subset S of the nodes contained in an associated
// graph G
//---------------------------------------------------------
class OGDF_EXPORT NodeSet {
public:
	// creates a new empty node set associated with graph G
	NodeSet(const Graph &G) : m_it(G,ListIterator<node>()) { }

	// destructor
	~NodeSet() { }

	// inserts node v into set S
	// running time: O(1)
	// Precond.: v is a node in the associated graph
	void insert(node v) {
		OGDF_ASSERT(v->graphOf() == m_it.graphOf());
		ListIterator<node> &itV = m_it[v];
		if (!itV.valid())
			itV = m_nodes.pushBack(v);
	}

	// removes node v from set S
	// running time: O(1)
	// Precond.: v is a node in the asociated graph
	void remove(node v) {
		OGDF_ASSERT(v->graphOf() == m_it.graphOf());
		ListIterator<node> &itV = m_it[v];
		if (itV.valid()) {
			m_nodes.del(itV);
			itV = ListIterator<node>();
		}
	}


	// removes all nodess from set S
	// running time: O(|S|)
	void clear() {
		ListIterator<node> it;
		for(it = m_nodes.begin(); it.valid(); ++it) {
			m_it[*it] = ListIterator<node>();
		}
		m_nodes.clear();
	}


	// returns true iff node v is contained in S
	// running time: O(1)
	// Precond.: v is a node in the asociated graph
	bool isMember(node v) const {
		OGDF_ASSERT(v->graphOf() == m_it.graphOf());
		return m_it[v].valid();
	}

	// returns the size of set S
	// running time: O(1)
	int size() const {
		return m_nodes.size();
	}

	// returns the list of nodes contained in S
	const List<node> &nodes() const {
		return m_nodes;
	}

private:
	// m_it[v] contains list iterator pointing to v if v is contained in S,
	// an invalid list iterator otherwise
	NodeArray<ListIterator<node> > m_it;
	// list of nodes contained in S
	List<node> m_nodes;
};


} // end namespace ogdf


#endif
