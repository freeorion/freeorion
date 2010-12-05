/*
 * $Revision: 2014 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-08-27 15:49:59 +0200 (Fri, 27 Aug 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class WSPD (well-separated pair decomposition).
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

#include "WSPD.h"
#include "FastUtils.h"

namespace ogdf {

WSPD::WSPD(__uint32 maxNumNodes) : m_maxNumNodes(maxNumNodes)
{
	m_maxNumPairs = maxNumNodes*2;
	m_numPairs = 0;
	allocate();
	clear();
}


WSPD::~WSPD(void)
{
	deallocate();
}

unsigned long WSPD::sizeInBytes() const
{
	return m_maxNumNodes*sizeof(WSPDNodeInfo) +
		   m_maxNumPairs*sizeof(WSPDPairInfo);
}

void WSPD::allocate()
{
	m_nodeInfo = (WSPDNodeInfo*)MALLOC_16(m_maxNumNodes*sizeof(WSPDNodeInfo));
	m_pairs = (WSPDPairInfo*)MALLOC_16(m_maxNumPairs*sizeof(WSPDPairInfo));
}

void WSPD::deallocate()
{
	FREE_16(m_nodeInfo);
	FREE_16(m_pairs);
}

void WSPD::clear()
{
	for (__uint32 i = 0; i < m_maxNumNodes; i++)
	{
		m_nodeInfo[i].numWSNodes = 0;
	};	
	m_numPairs = 0;
}

} // end of namespace ogdf
