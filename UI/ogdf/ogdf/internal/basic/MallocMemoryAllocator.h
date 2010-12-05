/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of memory manager for allocating small
 *        pieces of memory
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

#ifndef OGDF_MALLOC_MEMORY_ALLOCATOR_H
#define OGDF_MALLOC_MEMORY_ALLOCATOR_H


namespace ogdf {

//! Implements a simple memory manager using \c malloc() and \c free().
class OGDF_EXPORT MallocMemoryAllocator {
public:

	MallocMemoryAllocator() { }
	~MallocMemoryAllocator() { }


	static void init() { }
	static void initThread() { }
	static void cleanup() { }

	static bool checkSize(size_t /* nBytes */) { return true; }

	//! Allocates memory of size \a nBytes.
	static void *allocate(size_t nBytes, const char *, int) { return allocate(nBytes); }

	//! Allocates memory of size \a nBytes.
	static void *allocate(size_t nBytes)
	{
		void *p = malloc(nBytes);
		if (OGDF_UNLIKELY(p == 0)) OGDF_THROW(ogdf::InsufficientMemoryException);
		return p;
	}


	//! Deallocates memory at address \a p which is of size \a nBytes.
	static void deallocate(size_t /* nBytes */, void *p) { free(p); }

	//! Deallocate a complete list starting at \a pHead and ending at \a pTail.
	/**
	 * The elements are assumed to be chained using the first word of each element and
	 * elements are of size \a nBytes.
	 */
	static void deallocateList(size_t /* nBytes */, void *pHead, void *pTail)
	{
		MemElemPtr q, pStop = MemElemPtr(pTail)->m_next;
		while (pHead != pStop) {
			q = MemElemPtr(pHead)->m_next;
			free(pHead);
			pHead = q;
		}
	}

	static void flushPool() { }
	static void flushPool(__uint16 /* nBytes */) { }

	//! Always returns 0, since no blocks are allocated.
	static size_t memoryAllocatedInBlocks() { return 0; }

	//! Always returns 0, since no blocks are allocated.
	static size_t memoryInFreelist() { return 0; }
};

} // namespace ogdf

#endif
