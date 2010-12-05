/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Declaration and implementation of a simple freelist and an
 * index pool which generates unique indices for elements.
 *
 * \author Martin Gronemann
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

#ifndef OGDF_EFREELIST_H
#define OGDF_EFREELIST_H

#include <ogdf/basic/EList.h>

namespace ogdf {

template<typename E, E* E::*next> class EFreeList;
template<typename E, E* E::*next, int E::*index> class EFreeListIndexPool;

//! Simple implementation of a FreeList which buffers the memory allocation of an embedded list item.
template<typename E, E* E::*next>
class EFreeList
{
public:
	//! Constructs a new freelist
	inline EFreeList()  { FreeStack::init(this); }

	//! Destructor. Releases the mem used by the remaining elements on the stack.
	~EFreeList() { this->freeFreeList(); }

	//! Returns a new instance of E by either using an instance from the stack or creating a new one.
	inline E* alloc()
	{
		if (!FreeStack::empty(this))
			return FreeStack::popRet(this);
		else
			return new E();
	}

	//! Returns true if the stack is empty.
	inline bool empty() const { return FreeStack::empty(this); }

	//! Frees an item buy putting it onto the stack of free instances
	inline void free(E* ptr) { FreeStack::push(this, ptr); }

protected:
	//! deletes all instances in the list
	inline void freeFreeList()
	{
		while (!FreeStack::empty(this)) { delete FreeStack::popRet(this); };
	}

	//! Top of the stack
	E* m_pTop;

	//! Typedef for the embedded stack
	typedef EStack<EFreeList<E, next>, E, &EFreeList<E, next>::m_pTop, next> FreeStack;
};

//! More complex implementation of a FreeList, which is able to generate indeices for the elements.
template<typename E, E* E::*next, int E::*index>
class EFreeListIndexPool
{
public:
	//! Creates a new IndexPool and a FreeList.
	EFreeListIndexPool() : m_nextFreeIndex(0) { }

	//! Frees an element using the FreeList
	inline void free(E* ptr) { m_freeList.free(ptr); }

	//! The value indicates that all indices in 0..numUsedIndices-1 might be in use.
	inline int numUsedIndices() const { return m_nextFreeIndex; };

	//! Allocates a new Element by either using the free list or allocating a new one with a brand new index.
	inline E* alloc()
	{
		if (m_freeList.empty())
		{
			E* res = new E();
			res->*index = m_nextFreeIndex++;
			return res;
		} else
		{
			return m_freeList.alloc();
		};
	}

protected:
	//! The next brand new index.
	int m_nextFreeIndex;

	//! The free list for allocating the memory.
	EFreeList<E, next> m_freeList;
};

} // end of namespace ogdf

#endif /* ELISTPOOL_H_ */
