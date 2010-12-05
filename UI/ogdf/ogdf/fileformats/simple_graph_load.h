/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of simple graph loaders.
 * 
 * \author Markus Chimani, Carsten Gutwenger
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

#ifndef OGDF_SIMPLE_GRAPH_LOAD_H
#define OGDF_SIMPLE_GRAPH_LOAD_H

#include <ogdf/basic/Graph.h>

namespace ogdf { 

//! Loads a graph in the Rome-Graph-format from the given stream.
/**
 * Warning: this is a very simple implementation only usable for very properly formatted 
 * files!
 * 
 * The format contains (in this order) n "node-lines", 1 "separator-line", m "edge-lines". 
 * These lines are as follows (whereby all IDs are integer numbers):
 * "node-line": <i>NodeId</i> 0
 * "separator-line": starts with a #-sign
 * "edge-line": <i>EdgeId</i> 0 <i>SourceNodeId</i> <i>TargetNodeId</i> 
 */
OGDF_EXPORT bool loadRomeGraphStream(Graph &G, std::istream& fileStream);

//! Loads a graph in the Rome-Graph-format from the specified file.
/**
 * Uses loadRomeGraphStream internally; see there for details.
 */
OGDF_EXPORT bool loadRomeGraph(Graph &G, const char *fileName);

//! Loads a graph in a simple format

/**
 * Simple format has a leading line stating the name of the graph
 * and a following line stating the size of the graph
 * *BEGIN unknown_name.numN.numE
 * *GRAPH numN numE UNDIRECTED UNWEIGHTED
 * */
OGDF_EXPORT bool loadSimpleGraphStream(Graph &G, std::istream& fileStream);
//! Loads a graph in the simple format from the specified file. This
// format is used e.g. for the graphs from Petra Mutzels Ph.D. Thesis.
/**
 * Uses loadSimpleGraphStream internally; see there for details.
 */
OGDF_EXPORT bool loadSimpleGraph(Graph &G, const char *fileName);

//! Loads a graph in the Y-graph-format from the given stream (one line). 
/**
 * This format is e.g. produced by NAUTY (http://www.cs.sunysb.edu/~algorith/implement/nauty/implement.shtml)
 * 
 * Details  on the format, as given in NAUTYs graph generator (see above link):
 * "[A] graph occupies one line with a terminating newline.
 * Except for the newline, each byte has the format  01xxxxxx, where
 * each "x" represents one bit of data. 
 * 
 * First byte:  xxxxxx is the number of vertices n
 * 
 * Other ceiling(n(n-1)/12) bytes:  These contain the upper triangle of
 * the adjacency matrix in column major order.  That is, the entries
 * appear in the order (0,1),(0,2),(1,2),(0,3),(1,3),(2,3),(0,4),... .
 * The bits are used in left to right order within each byte.
 * Any unused bits on the end are set to zero. 
 */
OGDF_EXPORT bool loadYGraph(Graph &G, FILE *lineStream);

//! Loads a hypergraph in the BENCH-format from the given stream.
/**
 * A hypergraph in OGDF is represented by its point-based expansion, i.e., for each
 * hyperedge <i>h</i> we have a corresponding hypernode <i>n</i>. All nodes originally 
 * incident to <i>h</i> are incident to <i>n</i>, i.e., have regular edges to <i>n</i>.
 *   
 * \a G ... the graph into which we load the data
 * \a hypernodes ... the list of nodes which have to be interpreted as hypernodes
 * \a shell ... if null only the BENCH-hypergraph is loaded. Otherwise we extend the loaded graph
 * by a simple edge <i>e=(i,o)</i> and two hyperedges: one hyperedges groups all input nodes and 
 * <i>i</i> together, the other hyperedge groups all output edges and <i>o</i>.
 * These additional edges are then also collocated in shell. 
 * 
 * Warning: this is a very simple implementation only usable for very properly formatted 
 * files!
 */
OGDF_EXPORT bool loadBenchHypergraphStream(Graph &G, List<node>& hypernodes, List<edge>* shell, std::istream& fileStream);

//! Loads a hypergraph in the BENCH-format from the specified file.
/**
 * Uses loadBenchHypergraphStream internally; see there for details.
 */
OGDF_EXPORT bool loadBenchHypergraph(Graph &G, List<node>& hypernodes, List<edge>* shell, const char *fileName);

//! Loads a hypergraph in the PLA-format from the given stream.
/**
 * A hypergraph in OGDF is represented by its point-based expansion, i.e., for each
 * hyperedge <i>h</i> we have a corresponding hypernode <i>n</i>. All nodes originally 
 * incident to <i>h</i> are incident to <i>n</i>, i.e., have regular edges to <i>n</i>.
 *   
 * \a G ... the graph into which we load the data
 * \a hypernodes ... the list of nodes which have to be interpreted as hypernodes
 * \a shell ... if null only the BENCH-hypergraph is loaded. Otherwise we extend the loaded graph
 * by a simple edge <i>e=(i,o)</i> and two hyperedges: one hyperedges groups all input nodes and 
 * <i>i</i> together, the other hyperedge groups all output edges and <i>o</i>.
 * These additional edges are then also collocated in shell. 
 * 
 * Warning: this is a very simple implementation only usable for very properly formatted 
 * files!
 */
OGDF_EXPORT bool loadPlaHypergraphStream(Graph &G, List<node>& hypernodes, List<edge>* shell, std::istream& fileStream);

//! Loads a hypergraph in the PLA-format from the specified file.
/**
 * Uses loadPlaHypergraphStream internally; see there for details.
 */
OGDF_EXPORT bool loadPlaHypergraph(Graph &G, List<node>& hypernodes, List<edge>* shell, const char *fileName);



}

#endif //OGDF_SIMPLE_GRAPH_LOAD_H
