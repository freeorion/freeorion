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

#ifndef OGDF_MEMORY_H
#define OGDF_MEMORY_H

#if defined(OGDF_SYSTEM_WINDOWS) || defined(__CYGWIN__)
#define WIN32_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>
#endif

#include <stdlib.h>
#include <new>

struct MemElem { MemElem *m_next; };
typedef MemElem *MemElemPtr;

//---------------------------------------------------------------------
// configuration of memory-manager (can also be set by compiler flag)
//
// the good old pool allocator (not thread-safe)
//#define OGDF_MEMORY_POOL_NTS
//
// just using malloc/free (thread-safe)
//#define OGDF_MEMORY_MALLOC_TS
//
// new buffered-pool allocator per thread pool (thread-safe)
//#define OGDF_MEMORY_POOL_TS
//
// default (nothing defined): depending on system / compiler
//---------------------------------------------------------------------

// By default, we use the non-thread safe variant on cygwin and g++ 3.x
// since thread-local storage is not working there, and a thread-safe
// pool allocator otherwise.
#if !defined(OGDF_MEMORY_POOL_NTS) && !defined(OGDF_MEMORY_MALLOC_TS) && !defined(OGDF_MEMORY_POOL_TS)
#define OGDF_MEMORY_POOL_TS
#endif

#include <ogdf/internal/basic/PoolMemoryAllocator.h>
#include <ogdf/internal/basic/MallocMemoryAllocator.h>


namespace ogdf {

#define OGDF_MM(Alloc) \
public: \
void *operator new(size_t nBytes) { \
	if(OGDF_LIKELY(Alloc::checkSize(nBytes))) \
		return Alloc::allocate(nBytes); \
	else \
	return MallocMemoryAllocator::allocate(nBytes); \
} \
void *operator new(size_t, void *p) { return p; } \
void operator delete(void *p, size_t nBytes) { \
	if(OGDF_LIKELY(p != 0)) { \
		if(OGDF_LIKELY(Alloc::checkSize(nBytes))) \
			Alloc::deallocate(nBytes, p); \
		else \
			MallocMemoryAllocator::deallocate(nBytes, p); \
	} \
}

#define OGDF_NEW new

#ifdef OGDF_MEMORY_MALLOC_TS
#define OGDF_ALLOCATOR ogdf::MallocMemoryAllocator
#else
#define OGDF_ALLOCATOR ogdf::PoolMemoryAllocator
#endif

//! Creates new and delete operators in a class using ogdf's memory allocator.
#define OGDF_NEW_DELETE OGDF_MM(OGDF_ALLOCATOR)

//! Creates new and delete operators in a class using the malloc memory allocator.
#define OGDF_MALLOC_NEW_DELETE OGDF_MM(MallocMemoryAllocator)

} // namespace ogdf


#endif
