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

#ifndef OGDF_POOL_MEMORY_ALLOCATOR_H
#define OGDF_POOL_MEMORY_ALLOCATOR_H

#ifndef OGDF_MEMORY_POOL_NTS
#include <ogdf/basic/CriticalSection.h>
#else
#include <ogdf/basic/System.h>
#endif


namespace ogdf {


//! The class \a PoolAllocator represents ogdf's pool memory allocator.
/**
 * <H3>Usage:</H3>
 *
 * Adding the macro \c #OGDF_NEW_DELETE in a class declaration overloads
 * new and delete operators of that class such that they use this
 * memory allocator. This is useful if the size of a class is less than
 * \c PoolAllocator::eTableSize bytes.
 *
 * Another benefit from the OGDF memory-manager is that it throws an
 * InsufficientMemoryException if no more memory is available. Hence
 * it is legal to omit checking if new returned 0 as long as stack-
 * unwinding frees all memory allocated so far.
 * It is also possible to make the usual \c new operator behave the same
 * way (throwing an InsufficientMemoryException) by defining the
 * macro \c #OGDF_MALLOC_NEW_DELETE in a class declaration.
 */

class PoolMemoryAllocator
{
	struct MemElem {
		MemElem *m_next;
	};
	struct MemElemEx {
		MemElemEx *m_next;
		MemElemEx *m_down;
	};

	typedef MemElem   *MemElemPtr;
	typedef MemElemEx *MemElemExPtr;

	struct PoolVector;
	struct PoolElement;
	struct BlockChain;
	typedef BlockChain *BlockChainPtr;

public:
	enum {
		eMinBytes = sizeof(MemElemPtr),
		eTableSize = 256,
		eBlockSize = 8192,
		ePoolVectorLength = 15
	};

	PoolMemoryAllocator() { }
	~PoolMemoryAllocator() { }

	//! Initializes the memory manager.
	static OGDF_EXPORT void init();

	static OGDF_EXPORT void initThread() {
#if !defined(OGDF_MEMORY_POOL_NTS) && defined(OGDF_NO_COMPILER_TLS)
		pthread_setspecific(s_tpKey,calloc(eTableSize,sizeof(MemElemPtr)));
#endif
	}

	//! Frees all memory blocks allocated by the memory manager.
	static OGDF_EXPORT void cleanup();

	static OGDF_EXPORT bool checkSize(size_t nBytes) {
		return nBytes < eTableSize;
	}

	//! Allocates memory of size \a nBytes.
	static OGDF_EXPORT void *allocate(size_t nBytes) {
#if !defined(OGDF_MEMORY_POOL_NTS) && defined(OGDF_NO_COMPILER_TLS)
		MemElemPtr *pFreeBytes = ((MemElemPtr*)pthread_getspecific(s_tpKey))+nBytes;
#else
		MemElemPtr *pFreeBytes = s_tp+nBytes;
#endif
		if (OGDF_LIKELY(*pFreeBytes != 0)) {
			MemElemPtr p = *pFreeBytes;
			*pFreeBytes = p->m_next;
			return p;
		} else {
			return fillPool(*pFreeBytes,__uint16(nBytes));
		}
	}

	//! Deallocates memory at address \a p which is of size \a nBytes.
	static OGDF_EXPORT void deallocate(size_t nBytes, void *p) {
#if !defined(OGDF_MEMORY_POOL_NTS) && defined(OGDF_NO_COMPILER_TLS)
		MemElemPtr *pFreeBytes = ((MemElemPtr*)pthread_getspecific(s_tpKey))+nBytes;
#else
		MemElemPtr *pFreeBytes = s_tp+nBytes;
#endif
		MemElemPtr(p)->m_next = *pFreeBytes;
		*pFreeBytes = MemElemPtr(p);
	}

	//! Deallocate a complete list starting at \a pHead and ending at \a pTail.
	/**
	 * The elements are assumed to be chained using the first word of each element and
	 * elements are of size \a nBytes. This is much more efficient the deallocating
	 * each element separately, since the whole chain can be concatenated with the
	 * free list, requiring only constant effort.
	 */
	static OGDF_EXPORT void deallocateList(size_t nBytes, void *pHead, void *pTail) {
#if !defined(OGDF_MEMORY_POOL_NTS) && defined(OGDF_NO_COMPILER_TLS)
		MemElemPtr *pFreeBytes = ((MemElemPtr*)pthread_getspecific(s_tpKey))+nBytes;
#else
		MemElemPtr *pFreeBytes = s_tp+nBytes;
#endif
		MemElemPtr(pTail)->m_next = *pFreeBytes;
		*pFreeBytes = MemElemPtr(pHead);
	}

	static OGDF_EXPORT void flushPool();
	static OGDF_EXPORT void flushPool(__uint16 nBytes);

	//! Returns the total amount of memory (in bytes) allocated from the system.
	static OGDF_EXPORT size_t memoryAllocatedInBlocks();

	//! Returns the total amount of memory (in bytes) available in the global free lists.
	static OGDF_EXPORT size_t memoryInGlobalFreeList();

	//! Returns the total amount of memory (in bytes) available in the thread's free lists.
	static OGDF_EXPORT size_t memoryInThreadFreeList();

private:
	static int slicesPerBlock(__uint16 nBytes) {
		int nWords;
		return slicesPerBlock(nBytes,nWords);
	}

	static int slicesPerBlock(__uint16 nBytes, int &nWords) {
		nWords = (nBytes+sizeof(MemElemPtr)-1)/sizeof(MemElemPtr);
		return (eBlockSize-sizeof(MemElemPtr))/(nWords*sizeof(MemElemPtr));
	}

	static void incVectorSlot(PoolElement &pe);

	static void flushPoolSmall(__uint16 nBytes);
	static MemElemExPtr collectGroups(
		__uint16 nBytes,
		MemElemPtr &pRestHead,
		MemElemPtr &pRestTail,
		int &nRest);

	static void *fillPool(MemElemPtr &pFreeBytes, __uint16 nBytes);

	static MemElemPtr allocateBlock(__uint16 nBytes);

	static PoolElement s_pool[eTableSize];
	static MemElemPtr s_freeVectors;
	static BlockChainPtr s_blocks;

#ifdef OGDF_MEMORY_POOL_NTS
	static MemElemPtr s_tp[eTableSize];
#elif defined(OGDF_NO_COMPILER_TLS)
	static CriticalSection *s_criticalSection;
	static pthread_key_t s_tpKey;
#else
	static CriticalSection *s_criticalSection;
	static OGDF_DECL_THREAD MemElemPtr s_tp[eTableSize];
#endif
};


}

#endif
