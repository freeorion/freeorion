/*
 * $Revision: 2047 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2010-10-13 17:12:21 +0200 (Wed, 13 Oct 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class ArrayGraph.
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

#include "ArrayGraph.h"
#include "FastUtils.h"

namespace ogdf {

ArrayGraph::ArrayGraph():   m_numNodes(0), 
							m_numEdges(0),
							m_nodeXPos(0), 
							m_nodeYPos(0), 
							m_nodeSize(0),
							m_nodeMoveRadius(0),
							m_desiredEdgeLength(0),
							m_nodeAdj(0),
							m_edgeAdj(0)
{
};

ArrayGraph::ArrayGraph(__uint32 maxNumNodes, __uint32 maxNumEdges) : 
							m_numNodes(maxNumNodes),
							m_numEdges(maxNumEdges),
							m_nodeXPos(0), 
							m_nodeYPos(0), 
							m_nodeSize(0),
							m_nodeMoveRadius(0),
							m_desiredEdgeLength(0),
							m_nodeAdj(0),
							m_edgeAdj(0)
{
	allocate(maxNumNodes, maxNumEdges);
};

ArrayGraph::ArrayGraph(const GraphAttributes& GA, const EdgeArray<float>& edgeLength, const NodeArray<float>& nodeSize) : 
							m_numNodes(0),
							m_numEdges(0),
							m_nodeXPos(0), 
							m_nodeYPos(0), 
							m_nodeSize(0), 
							m_nodeMoveRadius(0),
							m_desiredEdgeLength(0),
							m_nodeAdj(0),
							m_edgeAdj(0)
{
	allocate(GA.constGraph().numberOfNodes(), GA.constGraph().numberOfEdges());
	readFrom(GA, edgeLength, nodeSize);
};

ArrayGraph::~ArrayGraph(void)
{
	if (m_nodeXPos)
		deallocate();
}

void ArrayGraph::allocate(__uint32 numNodes, __uint32 numEdges)
{
	m_nodeXPos = (float*)MALLOC_16(numNodes*sizeof(float));
	m_nodeYPos = (float*)MALLOC_16(numNodes*sizeof(float));
	m_nodeSize = (float*)MALLOC_16(numNodes*sizeof(float));
	m_nodeMoveRadius = (float*)MALLOC_16(numNodes*sizeof(float));
	m_nodeAdj = (NodeAdjInfo*)MALLOC_16(numNodes*sizeof(NodeAdjInfo));
	m_desiredEdgeLength = (float*)MALLOC_16(numEdges*sizeof(float));
	m_edgeAdj = (EdgeAdjInfo*)MALLOC_16(numEdges*sizeof(EdgeAdjInfo));

	for (__uint32 i=0; i < numNodes; i++)
		nodeInfo(i).degree = 0;
};

void ArrayGraph::deallocate()
{
	FREE_16(m_nodeXPos);
	FREE_16(m_nodeYPos);
	FREE_16(m_nodeSize);
	FREE_16(m_nodeMoveRadius);
	FREE_16(m_nodeAdj);
	FREE_16(m_desiredEdgeLength);
	FREE_16(m_edgeAdj);
};
void ArrayGraph::pushBackEdge(__uint32 a, __uint32 b, float desiredEdgeLength)
{
	// get the index of a free element
	__uint32 e_index = m_numEdges++;

	// get the pair entry
	EdgeAdjInfo& e = edgeInfo(e_index);

	// (a,b) is the pair we are adding
	e.a = a;
	e.b = b;
	
	m_desiredEdgeLength[e_index] = desiredEdgeLength;
	m_desiredAvgEdgeLength += (double)desiredEdgeLength;
	// get the node info
	NodeAdjInfo& aInfo = nodeInfo(a);
	NodeAdjInfo& bInfo = nodeInfo(b);

	// if a is part of at least one edge
	if (aInfo.degree)
	{
		// adjust the links
		EdgeAdjInfo& a_e = edgeInfo(aInfo.lastEntry);
		// check which one is a
		if (a==a_e.a)
			a_e.a_next = e_index;
		else
			a_e.b_next = e_index;
	} else
	{
		// this edge is the first for a => set the firstEntry link
		aInfo.firstEntry = e_index;
	};
	
	// same for b: if b is part of at least one edge
	if (bInfo.degree)
	{
		// adjust the links
		EdgeAdjInfo& b_e = edgeInfo(bInfo.lastEntry);
		// check which one is b
		if (b==b_e.a)
			b_e.a_next = e_index;
		else
			b_e.b_next = e_index;
	} else
	{
		// this edge is the first for b => set the firstEntry link
		bInfo.firstEntry = e_index;
	};
	// and the lastEntry link
	aInfo.lastEntry = e_index;
	bInfo.lastEntry = e_index;
	// one more edge for each node
	aInfo.degree++;
	bInfo.degree++;
};

void ArrayGraph::readFrom(const GraphAttributes& GA, const EdgeArray<float>& edgeLength, const NodeArray<float>& nodeSize)
{
	const Graph& G = GA.constGraph();
	NodeArray<__uint32> nodeIndex(G);
	
	node v;
	m_numNodes = 0;
	m_numEdges = 0;
	m_avgNodeSize = 0;
	m_desiredAvgEdgeLength = 0;
	forall_nodes(v, G)
	{
		m_nodeXPos[m_numNodes] = (float)GA.x(v);
		m_nodeYPos[m_numNodes] = (float)GA.y(v);
		m_nodeSize[m_numNodes] = nodeSize[v];
		nodeIndex[v] = m_numNodes;
		m_avgNodeSize += nodeSize[v];
		m_numNodes++;
	};
	m_avgNodeSize = m_avgNodeSize / (double)m_numNodes;

	edge e;
	forall_edges(e, G)
	{
		pushBackEdge(nodeIndex[e->source()], nodeIndex[e->target()], (float)edgeLength[e]);
	};
	m_desiredAvgEdgeLength = m_desiredAvgEdgeLength / (double)m_numEdges;
};

void ArrayGraph::writeTo(GraphAttributes& GA)
{
	const Graph& G = GA.constGraph();
	node v;
	__uint32 i = 0;
	forall_nodes(v, G)
	{
		GA.x(v) = m_nodeXPos[i];
		GA.y(v) = m_nodeYPos[i];
		i++;
	};
};

void ArrayGraph::transform(float translate, float scale)
{
	for (__uint32 i=0; i < m_numNodes; i++)
	{
		m_nodeXPos[i] = (m_nodeXPos[i] + translate)*scale; 
		m_nodeYPos[i] = (m_nodeYPos[i] + translate)*scale; 
	};
};

void ArrayGraph::centerGraph()
{
	double dx_sum = 0;
	double dy_sum = 0;

	for (__uint32 i=0; i < m_numNodes; i++)
	{
		dx_sum += m_nodeXPos[i]; 
		dy_sum += m_nodeYPos[i];
	};

	dx_sum /= (double)m_numNodes;
	dy_sum /= (double)m_numNodes;
	for (__uint32 i=0; i < m_numNodes; i++)
	{
		m_nodeXPos[i] -= (float)dx_sum; 
		m_nodeYPos[i] -= (float)dy_sum; 
	}
};

} // end of namespace ogdf

