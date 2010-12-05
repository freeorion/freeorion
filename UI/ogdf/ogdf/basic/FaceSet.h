/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief declaration and implementation of class FaceSetSimple, 
 * FaceSetPure and FaceSet
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

#ifndef OGDF_FACE_SET_H
#define OGDF_FACE_SET_H


#include <ogdf/basic/FaceArray.h>
#include <ogdf/basic/List.h>



namespace ogdf {


//! Maintains a subset S of the faces contained in an associated combinatorial embedding E 
/** (only insertion of elements and clear operation)
 */
class OGDF_EXPORT FaceSetSimple {
public:
	//! creates a new empty face set associated with combinatorial embedding E
	FaceSetSimple(const CombinatorialEmbedding &E) : m_isContained(E,false) { }

	//! destructor
	~FaceSetSimple() { }

	//! inserts face f into set S
	/** running time: O(1)
	 *  Precond.: f is a face in the associated combinatorial embedding
	 */
	void insert(face f) {
		OGDF_ASSERT(f->embeddingOf() == m_isContained.embeddingOf());
		bool &isContained = m_isContained[f];
		if (isContained == false) {
			isContained = true;
			m_faces.pushFront(f);
		}
	}


	//! removes all faces from set S
	/** running time: O(|S|)
	 */
	void clear() {
		SListIterator<face> it;
		for(it = m_faces.begin(); it.valid(); ++it) {
			m_isContained[*it] = false;
		}
		m_faces.clear();
	}


	//! returns true iff face f is contained in S
	/** running time: O(1)
	 * Precond.: f is a face in the asociated embedding
	 */
	bool isMember(face f) const {
		OGDF_ASSERT(f->embeddingOf() == m_isContained.embeddingOf());
		return m_isContained[f];
	}

	//! returns the list of faces contained in S
	const SListPure<face> &faces() const {
		return m_faces;
	}

private:
	//! m_isContained[f] is true <=> f is contained in S
	FaceArray<bool> m_isContained;
	//! list of faces contained in S
	SListPure<face> m_faces;
};



//! maintains a subset S of the faces contained in an associated combinatorial embedding E 
/** (no efficient access to size of S)
 */
class OGDF_EXPORT FaceSetPure {
public:
	//! creates a new empty face set associated with combinatorial embedding E
	FaceSetPure(const CombinatorialEmbedding &E) : m_it(E,ListIterator<face>()) { }

	//! destructor
	~FaceSetPure() { }

	//! inserts face f into set S
	/** running time: O(1)
	 * Precond.: f is a face in the associated combinatorial embedding
	 */
	void insert(face f) {
		OGDF_ASSERT(f->embeddingOf() == m_it.embeddingOf());
		ListIterator<face> &itF = m_it[f];
		if (!itF.valid())
			itF = m_faces.pushBack(f);
	}

	//! removes face f from set S
	/** running time: O(1)
	 * Precond.: f is a face in the asociated embedding
	 */
	void remove(face f) {
		OGDF_ASSERT(f->embeddingOf() == m_it.embeddingOf());
		ListIterator<face> &itF = m_it[f];
		if (itF.valid()) {
			m_faces.del(itF);
			itF = ListIterator<face>();
		}
	}


	//! removes all faces from set S
	/** running time: O(|S|)
	 */
	void clear() {
		ListIterator<face> it;
		for(it = m_faces.begin(); it.valid(); ++it) {
			m_it[*it] = ListIterator<face>();
		}
		m_faces.clear();
	}


	//! returns true iff face f is contained in S
	/** running time: O(1)
	 * Precond.: f is a face in the asociated embedding
	 */
	bool isMember(face f) const {
		OGDF_ASSERT(f->embeddingOf() == m_it.embeddingOf());
		return m_it[f].valid();
	}

	//! returns the list of faces contained in S
	const ListPure<face> &faces() const {
		return m_faces;
	}

private:
	//! m_it[f] contains list iterator pointing to f if f is contained in S, an invalid list iterator otherwise
	FaceArray<ListIterator<face> > m_it;
	//! list of faces contained in S
	ListPure<face> m_faces;
};



//! maintains a subset S of the faces contained in an associated combinatorial embedding E
class OGDF_EXPORT FaceSet {
public:
	//! creates a new empty face set associated with combinatorial embedding E
	FaceSet(const CombinatorialEmbedding &E) : m_it(E,ListIterator<face>()) { }

	//! destructor
	~FaceSet() { }

	//! inserts face f into set S
	/** running time: O(1)
	 * Precond.: f is a face in the associated combinatorial embedding
	 */
	void insert(face f) {
		OGDF_ASSERT(f->embeddingOf() == m_it.embeddingOf());
		ListIterator<face> &itF = m_it[f];
		if (!itF.valid())
			itF = m_faces.pushBack(f);
	}

	//! removes face f from set S
	/* running time: O(1)
	 * Precond.: f is a face in the asociated embedding
	 */
	void remove(face f) {
		OGDF_ASSERT(f->embeddingOf() == m_it.embeddingOf());
		ListIterator<face> &itF = m_it[f];
		if (itF.valid()) {
			m_faces.del(itF);
			itF = ListIterator<face>();
		}
	}


	//! removes all faces from set S
	/** running time: O(|S|)
	 */
	void clear() {
		ListIterator<face> it;
		for(it = m_faces.begin(); it.valid(); ++it) {
			m_it[*it] = ListIterator<face>();
		}
		m_faces.clear();
	}


	//! returns true iff face f is contained in S
	/** running time: O(1)
	 * Precond.: f is a face in the asociated embedding
	 */
	bool isMember(face f) const {
		OGDF_ASSERT(f->embeddingOf() == m_it.embeddingOf());
		return m_it[f].valid();
	}

	//! returns the size of set S
	/** running time: O(1)
	 */
	int size() const {
		return m_faces.size();
	}

	//! returns the list of faces contained in S
	const List<face> &faces() const {
		return m_faces;
	}

private:
	//! m_it[f] contains list iterator pointing to f if f is contained in S,an invalid list iterator otherwise
	FaceArray<ListIterator<face> > m_it;
	//! list of faces contained in S
	List<face> m_faces;
};


} // end namespace ogdf


#endif

