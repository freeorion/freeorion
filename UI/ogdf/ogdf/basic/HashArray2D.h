/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class HashArray2D.
 *
 * This is a class implementing a 2-dimensional Hash array.
 * It uses templates for the keys and the data of the objects
 * stored in it.
 * 
 * \author René Weiskircher
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

#include <ogdf/basic/HashArray.h>
#include <ogdf/basic/tuples.h>
#include <ogdf/basic/HashIterator2D.h>

#ifndef OGDF_HASHARRAY2D_H
#define OGDF_HASHARRAY2D_H


namespace ogdf {


/**
 * \brief Indexed 2-dimensional arrays using hashing for element access.
 *
 * A 2D-hash array can be used like a usual 2-dimensional array but with a general
 * index type. The class uses five template parameters:
 *   - \a I1_ is the first index type.
 *   - \a I2_ is the second index type.
 *   - \a E_ is the element type.
 *   - \a Hash1_ is the hash function type for the first index type.
 *   - \a Hash2_ is the hash function type for the second index type.
 * The hash function type arguments are optional; their defaults use the class
 * DefHashFunc.
 */
template< class I1_, class I2_, class E_,
	class Hash1_ = DefHashFunc<I1_>,
	class Hash2_ = DefHashFunc<I2_> >
class HashArray2D : private Hashing<Tuple2<I1_,I2_>, E_,
	HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >
{
public:
	//! The type of const-iterators for 2D-hash arrays.
	typedef HashConstIterator2D<I1_,I2_,E_,Hash1_,Hash2_> const_iterator;

	//! Creates a 2D-hash array.
	HashArray2D() /*: m_size(0)*/ {}

	//! Creates a 2D-hash array and sets the default value to \a x.
	HashArray2D(const E_ &defaultValue,
		const Hash1_ &hashFunc1 = Hash1_(),
		const Hash2_ &hashFunc2 = Hash2_())
		: Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >(256,
			HashFuncTuple<I1_,I2_,Hash1_,Hash2_>(hashFunc1,hashFunc2)),
		m_defaultValue(defaultValue) {}

	//! Copy constructor.
	HashArray2D(const HashArray2D<I1_,I2_,E_,Hash1_,Hash2_> &A) :
		Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >(A),
			m_defaultValue(A.m_defaultValue) { }

	//! Assignment operator.
	HashArray2D &operator=(const HashArray2D<I1_,I2_,E_,Hash1_,Hash2_> &A) {
		m_defaultValue = A.m_defaultValue;
		Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::operator=(A);

		return *this;
	}

	~HashArray2D(){};

	//! Returns a const reference to entry (\a i,\a j).
	const E_ &operator()(const I1_ &i, const I2_ &j) const {
		HashElement<Tuple2<I1_,I2_>,E_> *pElement =
			Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::lookup(Tuple2<I1_,I2_>(i,j));
		if (pElement) return pElement->info();
		else return m_defaultValue;
	}

	//! Returns a reference to entry (\a i,\a j).
	E_ &operator()(const I1_ &i, const I2_ &j) {
		Tuple2<I1_,I2_> t(i,j);
		HashElement<Tuple2<I1_,I2_>,E_> *pElement =
			Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::lookup(t);
		if (!pElement)
			pElement = Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::fastInsert(t,m_defaultValue);
		return pElement->info();
	}

	//! Returns true iff entry (\a i,\a j) is defined.
	bool isDefined(const I1_ &i, const I2_ &j) const {
		return Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::member(Tuple2<I1_,I2_>(i,j));
	}

	//! Undefines the entry at index (\a i,\a j).
	void undefine(const I1_ &i, const I2_ &j) {
		return Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::del(Tuple2<I1_,I2_>(i,j));
	}

	//! Returns an iterator pointing to the first element.
	HashConstIterator2D<I1_,I2_,E_,Hash1_,Hash2_> begin() const {
		return HashConstIterator2D<I1_,I2_,E_>(
			Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::begin());
	}

	//! Returns the number of defined elements in the table.
	int size() const {
		return Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::size();
	}
	
	//! Returns if any indices are defined
	int empty() const { 
		return Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::empty();
	}


	//! Undefines all indices.
	void clear() {
		Hashing<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::clear();
	}

private:
	E_ m_defaultValue; //!< The default value of the array.
};

}

#endif
