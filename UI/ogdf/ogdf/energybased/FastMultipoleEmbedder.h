/*
 * $Revision: 2014 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-08-27 15:49:59 +0200 (Fri, 27 Aug 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of Fast-Multipole-Embedder layout algorithm.
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


#ifdef _MSC_VER
#pragma once
#endif

#ifndef _FAST_MULTIPOLE_EMBEDDER_H_
#define _FAST_MULTIPOLE_EMBEDDER_H_

#include <ogdf/basic/Graph.h>
#include <ogdf/module/LayoutModule.h>
#include <ogdf/internal/energybased/MultilevelGraph.h>

namespace ogdf {

class ArrayGraph;
class LinearQuadtree;
class LinearQuadtreeExpansion;
class FMEThreadPool;
class FMEThread;
struct FMEGlobalOptions;
class GalaxyMultilevel;

class OGDF_EXPORT FastMultipoleEmbedder : public LayoutModule
{
public:
	//! constructor
	FastMultipoleEmbedder();
	
	//! destructor
	~FastMultipoleEmbedder();
	
	//! Calls the algorithm for graph \a MLG.
    void call(MultilevelGraph &MLG);

	//! Calls the algorithm for graph \a G with the given edgelength and returns the layout information in \a nodeXPosition, nodeYPosition.
    void call(const Graph& G, NodeArray<float>& nodeXPosition, NodeArray<float>& nodeYPosition, 
			  const EdgeArray<float>& edgeLength, const NodeArray<float>& nodeSize);

	//! Calls the algorithm for graph \a GA with the given edgelength and returns the layout information in \a GA.
    void call(GraphAttributes &GA, const EdgeArray<float>& edgeLength, const NodeArray<float>& nodeSize);

	//! Calls the algorithm for graph \a GA and returns the layout information in \a GA.
	void call(GraphAttributes &GA);

	//! sets the maximum number of iterations
	void setNumIterations(__uint32 numIterations) { m_numIterations = numIterations; };

	//! sets the number of coefficients for the expansions. default = 4
	void setMultipolePrec(__uint32 precision) { m_precisionParameter = precision; };
	
	//! if true, layout algorithm will randomize the layout in the beginning 
	void setRandomize(bool b) { m_randomize = b; };

	//!
	void setDefaultEdgeLength(float edgeLength) { m_defaultEdgeLength = edgeLength; };

	//!
	void setDefaultNodeSize(float nodeSize) { m_defaultNodeSize = nodeSize; };

	//!
	void setNumberOfThreads(__uint32 numThreads) { m_maxNumberOfThreads = numThreads; };

	//void setEnablePostProcessing(bool b) { m_doPostProcessing = b; };
private:
	void initOptions();

	void runMultipole();

	void runSingle();

	//! runs the simulation with the given number of iterations
	void run(__uint32 numIterations);

	//! allocates the memory
	void allocate(__uint32 numNodes, __uint32 numEdges);

	//! frees the memory
	void deallocate();
	
	__uint32 m_numIterations;

	ArrayGraph* m_pGraph;

	FMEThreadPool* m_threadPool;

	FMEGlobalOptions* m_pOptions;

	__uint32 m_precisionParameter;

	bool m_randomize;

	float m_defaultEdgeLength;

	float m_defaultNodeSize;

	__uint32 m_numberOfThreads;
	
	__uint32 m_maxNumberOfThreads;
};


class OGDF_EXPORT FastMultipoleMultilevelEmbedder : public LayoutModule
{
public:
	//! Constructor, just sets number of maximum threads
	FastMultipoleMultilevelEmbedder() : m_iMaxNumThreads(1) {}
	//! Calls the algorithm for graph \a GA and returns the layout information in \a GA.
	void call(GraphAttributes &GA);

	//! sets the bound for the number of nodes for multilevel step
	void multilevelUntilNumNodesAreLess(int nodesBound) { m_multiLevelNumNodesBound = nodesBound; };

	void maxNumThreads(int numThreads) { m_iMaxNumThreads = numThreads; }
private:
	//! internal function to compute a good edgelength
	void computeAutoEdgeLength(const GraphAttributes& GA, EdgeArray<float>& edgeLength, float factor = 1.0f);

	//! internal main function for the multilevel layout
	void run(GraphAttributes& GA, const EdgeArray<float>& edgeLength);

	//! creates all multilevels
	void createMultiLevelGraphs(Graph* pGraph, GraphAttributes& GA, const EdgeArray<float>& edgeLength);

	//! init the original graphs multilevel
	void initFinestLevel(GraphAttributes &GA, const EdgeArray<float>& edgeLength);

	//! calls the fast multipole embedder on current level
	void layoutCurrentLevel();

	//! assigns the nodes in the current level coords by coarser level
	void assignPositionsFromPrevLevel();

	//! writes the current level to graph attributes. used for output
	void writeCurrentToGraphAttributes(GraphAttributes& GA);
	
	//! refine
	void nextLevel();

	//! initialize datastructure by current level 
	void initCurrentLevel();

	//! clean up the multilevel graphs
	void deleteMultiLevelGraphs();

	//! for debugging only
	void dumpCurrentLevel(const String& filename);

	//! computes the maximum number of iterations by level nr
	__uint32 numberOfIterationsByLevelNr(__uint32 levelNr);

	int				  m_iMaxNumThreads;
	int				  m_iNumLevels;
	int				  m_multiLevelNumNodesBound;

	GalaxyMultilevel* m_pCurrentLevel;
	GalaxyMultilevel* m_pFinestLevel;
	GalaxyMultilevel* m_pCoarsestLevel;

	Graph*			  m_pCurrentGraph;
	NodeArray<float>* m_pCurrentNodeXPos;
	NodeArray<float>* m_pCurrentNodeYPos;
	EdgeArray<float>* m_pCurrentEdgeLength;
	NodeArray<float>* m_pCurrentNodeSize;
	NodeArray<float>  m_adjustedNodeSize;
	int				  m_iCurrentLevelNr;

	Graph*			  m_pLastGraph;
	NodeArray<float>* m_pLastNodeXPos;
	NodeArray<float>* m_pLastNodeYPos;
};
	
} // end of namespace ogdf

#endif

