/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class HashIterator2D.
 *
 * This class implements an iterator for the HashArray2D.
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


#ifndef OGDF_HASHITERATOR2D_H
#define OGDF_HASHITERATOR2D_H


namespace ogdf {


/**
 * \brief Const-iterator for 2D-hash arrays.
 *
 */
template< class I1_, class I2_, class E_,
	class Hash1_ = DefHashFunc<I1_>,
	class Hash2_ = DefHashFunc<I2_> >
class HashConstIterator2D :
	private HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >
{
public:
	//! Creates an (invalid) iterator.
	HashConstIterator2D() {};

	//! Copy constructor.
	HashConstIterator2D(const HashConstIterator2D<I1_,I2_,E_,Hash1_,Hash2_> &it)
		: HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >(it) { }

	//! Copy constructor (from HashConstIterator).
	HashConstIterator2D(const HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> > &it)
		: HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >(it) { }

	//! Assignemnt operator.
	HashConstIterator2D<I1_,I2_,E_,Hash1_,Hash2_> &
		operator=(const HashConstIterator2D<I1_,I2_,E_,Hash1_,Hash2_> &it)
	{
		HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::operator=(it);
		return *this;
	}

	//! Returns true iff the iterator points to an element.
	bool valid() const {
		return HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::valid();
	}

	//! Returns the first key of the hash element pointed to.
	const I1_ &key1() const {
		return HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::key().x1();
	}

	//! Returns the second key of the hash element pointed to.
	const I2_ &key2() const {
		return HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::key().x2();
	}

	//! Returns the information of the element pointed to.
	const E_ &info() const {
		return HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::info();
	}

	//! Sets the iterator to the next element in the 2D-hash array.
	HashConstIterator2D<I1_,I2_,E_,Hash1_,Hash2_> &operator++() {
		HashConstIterator<Tuple2<I1_,I2_>,E_,HashFuncTuple<I1_,I2_,Hash1_,Hash2_> >::operator++();
		return *this;
	}
};

}



#endif

