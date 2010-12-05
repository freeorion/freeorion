/*
 * $Revision: 2047 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2010-10-13 17:12:21 +0200 (Wed, 13 Oct 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class FMEThread.
 * 
 * \author Martin Gronemann
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2009
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

#include "FMEThread.h"

namespace ogdf {

#if defined(OGDF_SYSTEM_UNIX) && defined(OGDF_FME_THREAD_AFFINITY)
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <sched.h>

//! helper function for setting the affinity on the test machine.
void CPU_SET_ordered_dual_quad(int cpu, cpu_set_t* set)
{
	int cpuOrdered = cpu;
	switch ( cpu )
	{
         case 0 : cpuOrdered = 0; break;
         case 1 : cpuOrdered = 2; break;
         case 2 : cpuOrdered = 4; break;
         case 3 : cpuOrdered = 6; break;

         case 4 : cpuOrdered = 1; break;
         case 5 : cpuOrdered = 3; break;
         case 6 : cpuOrdered = 5; break;
         case 7 : cpuOrdered = 7; break;
	  default: cpuOrdered = 0; break;
	};
	CPU_SET(cpuOrdered, set);
};
void FMEThread::unixSetAffinity()
{
	cpu_set_t mask;
	CPU_ZERO( &mask ); 	
    CPU_SET_ordered_dual_quad(m_threadNr*(System::numberOfProcessors()/m_numThreads), &mask);
    sched_setaffinity( 0, sizeof(mask), &mask );
};
#endif


FMEThread::FMEThread(FMEThreadPool* pThreadPool, __uint32 threadNr) : m_threadNr(threadNr), m_pThreadPool(pThreadPool)
{
	m_numThreads = m_pThreadPool->numThreads();
};


void FMEThread::sync()
{
	if (m_numThreads>1)
		m_pThreadPool->syncBarrier()->threadSync();
};



FMEThreadPool::FMEThreadPool(__uint32 numThreads) : m_numThreads(numThreads)
{
	allocate();
};

FMEThreadPool::~FMEThreadPool()
{
	deallocate();
};

//! runs one iteration. This call blocks the main thread
void FMEThreadPool::runThreads()
{
	for (__uint32 i=1; i < numThreads(); i++)
	{
		thread(i)->start();
	};

	thread(0)->doWork();

	for (__uint32 i=1; i < numThreads(); i++)
	{
		thread(i)->join();
	};
};


void FMEThreadPool::allocate()
{
	typedef FMEThread* FMEThreadPtr;
	
	m_pSyncBarrier = new Barrier(m_numThreads);
	m_pThreads = new FMEThreadPtr[m_numThreads];
	for (__uint32 i=0; i < numThreads(); i++)
	{
		m_pThreads[i] = new FMEThread(this, i);
#ifdef OGDF_SYSTEM_WINDOWS
		m_pThreads[i]->priority(Thread::tpCritical);
		m_pThreads[i]->cpuAffinity(1 << i);
#endif
	};
};

void FMEThreadPool::deallocate()
{
	for (__uint32 i=0; i < numThreads(); i++)
	{
		delete m_pThreads[i];
	};
	delete[] m_pThreads;
	delete m_pSyncBarrier;
};

} // end of namespace ogdf

