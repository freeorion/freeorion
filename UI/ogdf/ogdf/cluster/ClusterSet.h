/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration and implementation of class ClusterSetSimple,
 * ClusterSetPure and ClusterSet
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


#include <ogdf/cluster/ClusterArray.h>
#include <ogdf/basic/List.h>



namespace ogdf {


//---------------------------------------------------------
// ClusterSetSimple
// maintains a subset S of the clusters contained in an associated
// cluster graph G (only insertion of elements and clear operation)
//---------------------------------------------------------
class OGDF_EXPORT ClusterSetSimple {
public:
	// creates a new empty cluster set associated with cluster graph CG
	ClusterSetSimple(const ClusterGraph &CG) : m_isContained(CG,false) { }

	// destructor
	~ClusterSetSimple() { }

	// inserts cluster c into set S
	// running time: O(1)
	// Precond.: c is a cluster in the associated graph
	void insert(cluster c) {
		OGDF_ASSERT(c->graphOf() == m_isContained.graphOf());
		bool &isContained = m_isContained[c];
		if (isContained == false) {
			isContained = true;
			m_clusters.pushFront(c);
		}
	}


	// removes all clusters from set S
	// running time: O(|S|)
	void clear() {
		SListIterator<cluster> it;
		for(it = m_clusters.begin(); it.valid(); ++it) {
			m_isContained[*it] = false;
		}
		m_clusters.clear();
	}


	// returns true iff cluster c is contained in S
	// running time: O(1)
	// Precond.: c is a cluster in the asociated graph
	bool isMember(cluster c) const {
		OGDF_ASSERT(c->graphOf() == m_isContained.graphOf());
		return m_isContained[c];
	}

	// returns the list of clusters contained in S
	const SListPure<cluster> &clusters() const {
		return m_clusters;
	}

private:
	// m_isContained[c] is true <=> c is contained in S
	ClusterArray<bool> m_isContained;
	// list of clusters contained in S
	SListPure<cluster> m_clusters;
};



//---------------------------------------------------------
// ClusterSetPure
// maintains a subset S of the clusters contained in an associated
// graph G (no efficient access to size of S)
//---------------------------------------------------------
class OGDF_EXPORT ClusterSetPure {
public:
	// creates a new empty cluster set associated with graph G
	ClusterSetPure(const ClusterGraph &G) : m_it(G,ListIterator<cluster>()) { }

	// destructor
	~ClusterSetPure() { }

	// inserts cluster c into set S
	// running time: O(1)
	// Precond.: c is a cluster in the associated graph
	void insert(cluster c) {
		OGDF_ASSERT(c->graphOf() == m_it.graphOf());
		ListIterator<cluster> &itV = m_it[c];
		if (!itV.valid())
			itV = m_clusters.pushBack(c);
	}

	// removes cluster c from set S
	// running time: O(1)
	// Precond.: c is a cluster in the asociated graph
	void remove(cluster c) {
		OGDF_ASSERT(c->graphOf() == m_it.graphOf());
		ListIterator<cluster> &itV = m_it[c];
		if (itV.valid()) {
			m_clusters.del(itV);
			itV = ListIterator<cluster>();
		}
	}


	// removes all clusters from set S
	// running time: O(|S|)
	void clear() {
		ListIterator<cluster> it;
		for(it = m_clusters.begin(); it.valid(); ++it) {
			m_it[*it] = ListIterator<cluster>();
		}
		m_clusters.clear();
	}


	// returns true iff cluster c is contained in S
	// running time: O(1)
	// Precond.: c is a cluster in the asociated graph
	bool isMember(cluster c) const {
		OGDF_ASSERT(c->graphOf() == m_it.graphOf());
		return m_it[c].valid();
	}

	// returns the list of clusters contained in S
	const ListPure<cluster> &clusters() const {
		return m_clusters;
	}

private:
	// m_it[c] contains list iterator pointing to c if c is contained in S,
	// an invalid list iterator otherwise
	ClusterArray<ListIterator<cluster> > m_it;
	// list of clusters contained in S
	ListPure<cluster> m_clusters;
};



//---------------------------------------------------------
// ClusterSet
// maintains a subset S of the clusters contained in an associated
// graph G
//---------------------------------------------------------
class OGDF_EXPORT ClusterSet {
public:
	// creates a new empty cluster set associated with graph G
	ClusterSet(const ClusterGraph &G) : m_it(G,ListIterator<cluster>()) { }

	// destructor
	~ClusterSet() { }

	// inserts cluster c into set S
	// running time: O(1)
	// Precond.: c is a cluster in the associated graph
	void insert(cluster c) {
		OGDF_ASSERT(c->graphOf() == m_it.graphOf());
		ListIterator<cluster> &itV = m_it[c];
		if (!itV.valid())
			itV = m_clusters.pushBack(c);
	}

	// removes cluster c from set S
	// running time: O(1)
	// Precond.: c is a cluster in the asociated graph
	void remove(cluster c) {
		OGDF_ASSERT(c->graphOf() == m_it.graphOf());
		ListIterator<cluster> &itV = m_it[c];
		if (itV.valid()) {
			m_clusters.del(itV);
			itV = ListIterator<cluster>();
		}
	}


	// removes all clusterss from set S
	// running time: O(|S|)
	void clear() {
		ListIterator<cluster> it;
		for(it = m_clusters.begin(); it.valid(); ++it) {
			m_it[*it] = ListIterator<cluster>();
		}
		m_clusters.clear();
	}


	// returns true iff cluster c is contained in S
	// running time: O(1)
	// Precond.: c is a cluster in the asociated graph
	bool isMember(cluster c) const {
		OGDF_ASSERT(c->graphOf() == m_it.graphOf());
		return m_it[c].valid();
	}

	// returns the size of set S
	// running time: O(1)
	int size() const {
		return m_clusters.size();
	}

	// returns the list of clusters contained in S
	const List<cluster> &clusters() const {
		return m_clusters;
	}

private:
	// m_it[c] contains list iterator pointing to c if c is contained in S,
	// an invalid list iterator otherwise
	ClusterArray<ListIterator<cluster> > m_it;
	// list of clusters contained in S
	List<cluster> m_clusters;
};


} // end namespace ogdf


#endif
