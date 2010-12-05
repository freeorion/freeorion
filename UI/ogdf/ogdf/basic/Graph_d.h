/*
 * $Revision: 2047 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2010-10-13 17:12:21 +0200 (Wed, 13 Oct 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Pure declaration header, find template implementation in 
 *        Graph.h
 *
 * Declaration of NodeElement, EdgeElement, and Graph classes.
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

#ifndef OGDF_GRAPH_D_H
#define OGDF_GRAPH_D_H


#include <ogdf/basic/List.h>


namespace ogdf {

//
// in embedded graphs, adjacency lists are given in clockwise order.
//


class OGDF_EXPORT Graph;
class OGDF_EXPORT NodeElement;
class OGDF_EXPORT EdgeElement;
class OGDF_EXPORT AdjElement;
class OGDF_EXPORT FaceElement;
class OGDF_EXPORT GraphListBase;
class OGDF_EXPORT ClusterElement;


//! The base class for objects used by graphs like nodes, edges, etc.
/**
 * Such graph objects are maintained in list (see GraphList<T>),
 * and \a GraphElement basically provides a next and previous pointer
 * for these objects.
 */
class OGDF_EXPORT GraphElement {
	friend class Graph;
	friend class GraphListBase;

protected:
	GraphElement *m_next; //!< The successor in the list.
	GraphElement *m_prev; //!< The predecessor in the list.

	OGDF_NEW_DELETE
}; // class GraphElement


//! Base class for GraphElement lists.
class OGDF_EXPORT GraphListBase {
protected:
	GraphElement *m_head; //!< Pointer to the first element in the list.
	GraphElement *m_tail; //!< Pointer to the last element in the list.

public:
	//! Constructs an empty list.
	GraphListBase() { m_head = m_tail = 0; }
	// destruction
	~GraphListBase() { }

	//! Adds element \a pX at the end of the list.
	void pushBack(GraphElement *pX) {
		pX->m_next = 0;
		pX->m_prev = m_tail;
		if (m_head)
			m_tail = m_tail->m_next = pX;
		else
			m_tail = m_head = pX;
	}

	//! Inserts element \a pX after element \a pY.
	void insertAfter(GraphElement *pX, GraphElement *pY) {
		pX->m_prev = pY;
		GraphElement *pYnext = pX->m_next = pY->m_next;
		pY->m_next = pX;
		if (pYnext) pYnext->m_prev = pX;
		else m_tail = pX;
	}

	//! Inserts element \a pX before element \a pY.
	void insertBefore(GraphElement *pX, GraphElement *pY) {
		pX->m_next = pY;
		GraphElement *pYprev = pX->m_prev = pY->m_prev;
		pY->m_prev = pX;
		if (pYprev) pYprev->m_next = pX;
		else m_head = pX;
	}

	//! Removes element \a pX from the list.
	void del(GraphElement *pX) {
		GraphElement *pxPrev = pX->m_prev, *pxNext = pX->m_next;

		if (pxPrev)
			pxPrev->m_next = pxNext;
		else
			m_head = pxNext;
		if (pxNext)
			pxNext->m_prev = pxPrev;
		else
			m_tail = pxPrev;
	}

	//! Sorts the list according to \a newOrder.
	template<class LIST>
	void sort(const LIST &newOrder) {
		GraphElement *pPred = 0;
		typename LIST::const_iterator it = newOrder.begin();
		if (!it.valid()) return;

		m_head = *it;
		for(; it.valid(); ++it) {
			GraphElement *p = *it;
			if ((p->m_prev = pPred) != 0) pPred->m_next = p;
			pPred = p;
		}

		(m_tail = pPred)->m_next = 0;
	}

	//! Reverses the order of the list elements.
	void reverse() {
		GraphElement *pX = m_head;
		m_head = m_tail;
		m_tail = pX;
		while(pX) {
			GraphElement *pY = pX->m_next;
			pX->m_next = pX->m_prev;
			pX = pX->m_prev = pY;
		}
	}

	//! Exchanges the positions of \a pX and \a pY in the list.
	void swap(GraphElement *pX, GraphElement *pY) {
		if (pX->m_next == pY) {
			pX->m_next = pY->m_next;
			pY->m_prev = pX->m_prev;
			pY->m_next = pX;
			pX->m_prev = pY;

		} else if(pY->m_next == pX) {
			pY->m_next = pX->m_next;
			pX->m_prev = pY->m_prev;
			pX->m_next = pY;
			pY->m_prev = pX;

		} else {
			swap(pX->m_next,pY->m_next);
			swap(pX->m_prev,pY->m_prev);
		}

		if(pX->m_prev)
			pX->m_prev->m_next = pX;
		else
			m_head = pX;
		if(pX->m_next)
			pX->m_next->m_prev = pX;
		else
			m_tail = pX;

		if(pY->m_prev)
			pY->m_prev->m_next = pY;
		else
			m_head = pY;
		if(pY->m_next)
			pY->m_next->m_prev = pY;
		else
			m_tail = pY;

		OGDF_ASSERT(consistencyCheck());
	}


	//! Checks consistency of graph list.
	bool consistencyCheck() {
		if (m_head == 0) {
			return (m_tail == 0);

		} else if (m_tail == 0) {
			return false;

		} else {
			if (m_head->m_prev != 0)
				return false;
			if (m_tail->m_next != 0)
				return false;

			GraphElement *pX = m_head;
			for(; pX; pX = pX->m_next) {
				if (pX->m_prev) {
					if (pX->m_prev->m_next != pX)
						return false;
				} else if(pX != m_head)
					return false;

				if (pX->m_next) {
					if (pX->m_next->m_prev != pX)
						return false;
				} else if (pX != m_tail)
					return false;
			}
		}

		return true;
	}

	OGDF_NEW_DELETE
}; // class GraphListBase


//! Lists of graph objects (like nodes, edges, etc.).
/**
 * The template type \a T must be a class derived from GraphElement.
 */
template<class T> class GraphList : protected GraphListBase {
public:
	//! Constructs an empty list.
	GraphList() { }
	// destruction (deletes all elements)
	~GraphList() {
		if (m_head)
			OGDF_ALLOCATOR::deallocateList(sizeof(T), m_head,m_tail);
	}

	//! Returns the first element in the list.
	T *begin () const { return (T *)m_head; }
	//! Returns the last element in the list.
	T *rbegin() const { return (T *)m_tail; }

	//! Returns true iff the list is empty.
	bool empty() { return m_head; }

	//! Adds element \a pX at the end of the list.
	void pushBack(T *pX) {
		GraphListBase::pushBack(pX);
	}

	//! Inserts element \a pX after element \a pY.
	void insertAfter(T *pX, T *pY) {
		GraphListBase::insertAfter(pX,pY);
	}

	//! Inserts element \a pX before element \a pY.
	void insertBefore(T *pX, T *pY) {
		GraphListBase::insertBefore(pX,pY);
	}

	//! Moves element \a pX to list \a L and inserts it before or after \a pY.
	void move(T *pX, GraphList<T> &L, T *pY, Direction dir) {
		GraphListBase::del(pX);
		if (dir == after)
			L.insertAfter(pX,pY);
		else
			L.insertBefore(pX,pY);
	}

	//! Moves element \a pX to list \a L and inserts it at the end.
	void move(T *pX, GraphList<T> &L) {
		GraphListBase::del(pX);
		L.pushBack(pX);
	}

	//! Moves element \a pX from its current position to a position after \a pY.
	void moveAfter(T *pX, T *pY){
		GraphListBase::del(pX);
		insertAfter(pX,pY);
	}

	//! Moves element \a pX from its current position to a position before \a pY.
	void moveBefore(T *pX, T *pY){
		GraphListBase::del(pX);
		insertBefore(pX,pY);
	}

	//! Removes element \a pX from the list and deletes it.
	void del(T *pX) {
		GraphListBase::del(pX);
		delete pX;
	}

	//! Only removes element \a pX from the list; does not delete it.
	void delPure(T *pX) {
		GraphListBase::del(pX);
	}

	//! Removes all elements from the list and deletes them.
	void clear() {
		if (m_head) {
			OGDF_ALLOCATOR::deallocateList(sizeof(T),m_head,m_tail);
			m_head = m_tail = 0;
		}
	}

	//! Sorts all elements according to \a newOrder.
	template<class T_LIST>
	void sort(const T_LIST &newOrder) {
		GraphListBase::sort(newOrder);
	}


	//! Reverses the order of the list elements.
	void reverse() {
		GraphListBase::reverse();
	}

	//! Exchanges the positions of \a pX and \a pY in the list.
	void swap(T *pX, T *pY) {
		GraphListBase::swap(pX,pY);
	}


	//! Checks consistency of graph list; returns true if ok.
	bool consistencyCheck() {
		return GraphListBase::consistencyCheck();
	}
	
	
	OGDF_NEW_DELETE
}; // class GraphList<T>


typedef NodeElement *node; //!< The type of nodes.
typedef EdgeElement *edge; //!< The type of edges.
typedef AdjElement *adjEntry; //!< The type of adjacency entries.



//! Class for adjacency list elements.
/**
 * Adjacency list elements represent the occurrence of an edges in
 * the adjacency list of a node.
 */
class OGDF_EXPORT AdjElement : private GraphElement {
	friend class Graph;
	friend class GraphListBase;
	friend class GraphList<AdjElement>;

	AdjElement *m_twin; //!< The corresponding adjacency entry (same edge)
	edge m_edge; //!< The associated edge.
	node m_node; //!< The node whose adjacency list contains this entry.
	int m_id;    //!< The (unique) index of the adjacency entry.

	//! Constructs an adjacency element for a given node.
	AdjElement(node v) : m_node(v) { }
	//! Constructs an adjacency entry for a given edge and index.
	AdjElement(edge e, int id) : m_edge(e), m_id(id) { }

public:
	//! Returns the associated edge.
	edge theEdge() const { return m_edge; }
	//! Conversion to edge.
	operator edge() const { return m_edge; }
	//! Returns the node whose adjacency list contains this element.
	node theNode() const { return m_node; }

	//! Returns the corresponding adjacency element associated with the same edge.
	adjEntry twin() const { return m_twin; }

	//! Returns the associated node of the corresponding adjacency entry (shorthand for twin()->theNode()).
	node twinNode() const { return m_twin->m_node; }

	//! Returns the index of this adjacency element.
	int index() const { return m_id; }

	// traversing faces in clockwise (resp. counter-clockwise) order
	// (if face is an interior face)
	
	//! Returns the clockwise successor in face. Use faceCycleSucc instead!
	adjEntry clockwiseFaceSucc() const { return m_twin->cyclicPred(); }
	//! Returns the clockwise predecessor in face.  Use faceCycleSucc instead!
	adjEntry clockwiseFacePred() const { return cyclicSucc()->m_twin; }
	//! Returns the counter-clockwise successor in face.  	
	adjEntry counterClockwiseFaceSucc() const { return m_twin->cyclicSucc(); }
	//! Returns the counter-clockwise predecessor in face.	
	adjEntry counterClockwiseFacePred() const { return cyclicPred()->m_twin; }

	// default is traversing faces in clockwise order
	//! Returns the cyclic successor in face.
	adjEntry faceCycleSucc() const { return clockwiseFaceSucc(); }
	//! Returns the cyclic predecessor in face.
	adjEntry faceCyclePred() const { return clockwiseFacePred(); }


	//! Returns the successor in the adjacency list.
	adjEntry succ() const { return (adjEntry)m_next; }
	//! Returns the predecessor in the adjacency list.
	adjEntry pred() const { return (adjEntry)m_prev; }

	//! Returns the cyclic successor in the adjacency list.
	adjEntry cyclicSucc() const;
	//! Returns the cyclic predecessor in the adjacency list.
	adjEntry cyclicPred() const;

#ifdef OGDF_DEBUG
	const Graph *graphOf() const;
#endif

	OGDF_NEW_DELETE
}; // class AdjElement


//! Class for the representation of nodes.
class OGDF_EXPORT NodeElement : private GraphElement {
	friend class Graph;
	friend class GraphList<NodeElement>;

	GraphList<AdjElement> m_adjEdges; //!< The adjacency list of the node.
	int m_indeg;  //!< The indegree of the node.
	int m_outdeg; //!< The outdegree of the node.
	int m_id;     //!< The (unique) index of the node.

#ifdef OGDF_DEBUG
	// we store the graph containing this node for debugging purposes
	const Graph *m_pGraph; //!< The graph containg this node (debug only).
#endif


	// construction
#ifdef OGDF_DEBUG
	//! Constructs a node element with index \a id.
	/**
	 * \remarks The parameter \a pGraph is only passed in a debug build.
	 * It is used, e.g., by NodeArray for checking if a node belongs to
	 * the correct graph.
	 */
	NodeElement(const Graph *pGraph, int id) :
		m_indeg(0), m_outdeg(0), m_id(id), m_pGraph(pGraph) { }
#else
	NodeElement(int id) : m_indeg(0), m_outdeg(0), m_id(id) { }
#endif


public:
	//! Returns the (unique) node index.
	int index() const { return m_id; }

	//! Returns the indegree of the node.
	int indeg() const { return m_indeg; }
	//! Returns the outdegree of the node.
	int outdeg() const { return m_outdeg; }
	//! Returns the degree of the node (indegree + outdegree).
	int degree() const { return m_indeg + m_outdeg; }

	//! Returns the first entry in the adjaceny list.
	adjEntry firstAdj() const { return m_adjEdges.begin();  }
	//! Returns the last entry in the adjacency list.
	adjEntry lastAdj () const { return m_adjEdges.rbegin(); }

	//! Returns the successor in the list of all nodes.
	node succ() const { return (node)m_next; }
	//! Returns the predecessor in the list of all nodes.
	node pred() const { return (node)m_prev; }

#ifdef OGDF_DEBUG
	//! Returns the graph containing this node (debug only).
	const Graph *graphOf() const { return m_pGraph; }
#endif

	OGDF_NEW_DELETE
}; // class NodeElement


inline adjEntry AdjElement::cyclicSucc() const
{
	return (m_next) ? (adjEntry)m_next : m_node->firstAdj();
}

inline adjEntry AdjElement::cyclicPred() const
{
	return (m_prev) ? (adjEntry)m_prev : m_node->lastAdj();
}

inline bool test_forall_adj_edges(adjEntry &adj, edge &e)
{
	if (adj) { e = adj->theEdge(); return true; }
	else return false;
}



//! Class for the representation of edges.
class OGDF_EXPORT EdgeElement : private GraphElement {
	friend class Graph;
	friend class GraphList<EdgeElement>;

	node m_src; //!< The source node of the edge.
	node m_tgt; //!< The target node of the edge.
	AdjElement *m_adjSrc; //!< Corresponding adjacancy entry at source node.
	AdjElement *m_adjTgt; //!< Corresponding adjacancy entry at target node.
	int m_id; // The (unique) index of the node.

	//! Constructs an edge element (\a src,\a tgt).
	/**
	 * @param src is the source node of the edge.
	 * @param tgt is the target node of the edge.
	 * @param adjSrc is the corresponding adjacency entry at source node.
	 * @param adjTgt is the corresponding adjacency entry at target node.
	 * @param id is the index of the edge.
	 */
	EdgeElement(node src, node tgt, AdjElement *adjSrc, AdjElement *adjTgt, int id) :
		m_src(src), m_tgt(tgt), m_adjSrc(adjSrc), m_adjTgt(adjTgt), m_id(id) { }
		
	//! Constructs an edge element (\a src,\a tgt).
	/**
	 * @param src is the source node of the edge.
	 * @param tgt is the target node of the edge.
	 * @param id is the index of the edge.
	 */
	EdgeElement(node src, node tgt, int id) :
		m_src(src), m_tgt(tgt), m_id(id) { }

public:
	//! Returns the index of the edge.
	int index() const { return m_id; }
	//! Returns the source node of the edge.
	node source() const { return m_src; }
	//! Returns the target node of the edge.
	node target() const { return m_tgt; }

	//! Returns the corresponding adjacancy entry at source node.
	adjEntry adjSource() const { return m_adjSrc; }
	//! Returns the corresponding adjacancy entry at target node.
	adjEntry adjTarget() const { return m_adjTgt; }

	//! Returns the adjacent node different from \a v.
	node opposite(node v) const { return (v == m_src) ? m_tgt : m_src; }
	// Returns true iff the edge is a self-loop (source node = target node).
	bool isSelfLoop() const { return m_src == m_tgt; }

	//! Returns the successor in the list of all edges.
	edge succ() const { return (edge)m_next; }
	//! Returns the predecessor in the list of all edges.
	edge pred() const { return (edge)m_prev; }

#ifdef OGDF_DEBUG
	//! Returns the graph containing this node (debug only).
	const Graph *graphOf() const { return m_src->graphOf(); }
#endif

	//! Returns true iff \a v is incident to the edge.
	bool isIncident(node v) const { return v == m_src || v == m_tgt; }
	
	//! Returns the common node of the edge and \a e. Returns NULL if the two edges are not adjacent.
	node commonNode(edge e) const { return (m_src==e->m_src || m_src==e->m_tgt) ? m_src : ((m_tgt==e->m_src || m_tgt==e->m_tgt) ? m_tgt: 0); }

	OGDF_NEW_DELETE
}; // class EdgeElement


#ifdef OGDF_DEBUG
inline const Graph *AdjElement::graphOf() const {
	return m_node->graphOf();
}
#endif


template<>inline bool doDestruction<node>(const node *) { return false; }
template<>inline bool doDestruction<edge>(const edge *) { return false; }
template<>inline bool doDestruction<adjEntry>(const adjEntry *) { return false; }

class NodeArrayBase;
class EdgeArrayBase;
class AdjEntryArrayBase;
template<class T> class NodeArray;
template<class T> class EdgeArray;
template<class T> class AdjEntryArray;
class OGDF_EXPORT GraphObserver;


//---------------------------------------------------------
// iteration macros
//---------------------------------------------------------

//! Iteration over all nodes \a v of graph \a G.
#define forall_nodes(v,G) for((v)=(G).firstNode(); (v); (v)=(v)->succ())
//! Iteration over all nodes \a v of graph \a G in reverse order.
#define forall_rev_nodes(v,G) for((v)=(G).lastNode(); (v); (v)=(v)->pred())

//! Iteration over all edges \a e of graph \a G.
#define forall_edges(e,G) for((e)=(G).firstEdge(); (e); (e)=(e)->succ())
//! Iteration over all edges \a e of graph \a G in reverse order.
#define forall_rev_edges(e,G) for((e)=(G).lastEdge(); (e); (e)=(e)->pred())

//! Iteration over all adjacency list entries \a adj of node \a v.
#define forall_adj(adj,v) for((adj)=(v)->firstAdj(); (adj); (adj)=(adj)->succ())
//! Iteration over all adjacency list entries \a adj of node \a v in reverse order.
#define forall_rev_adj(adj,v) for((adj)=(v)->lastAdj(); (adj); (adj)=(adj)->pred())

//! Iteration over all adjacent edges \a e of node \a v.
#define forall_adj_edges(e,v)\
for(ogdf::adjEntry ogdf_loop_var=(v)->firstAdj();\
	ogdf::test_forall_adj_edges(ogdf_loop_var,(e));\
	ogdf_loop_var=ogdf_loop_var->succ())


//! Data type for general directed graphs (adjacency list representation).
/**
 * <H3>Iteration</H3>
 * Besides the usage of iteration macros defined in Graph_d.h, the following
 * code is recommended for further iteration tasks.
 * <ul>
 *   <li> Iteration over all outgoing edges \a e of node \a v:
 *     \code
 *  forall_adj_edges(e,v)
 *    if(e->source() != v) continue;
 *     \endcode
 *
 *   <li> Iteration over all ingoing edges \a e of node \a v:
 *     \code
 *  forall_adj_edges(e,v)
 *	  if(e->target() != v) continue;
 *     \endcode
 * 
 *   <li> Iteration over all nodes \a x reachable by an outgoing edge \a e
 *        of node \a v (without self-loops):
 *     \code
 *  forall_adj_edges(e,v)
 *    if ((x = e->target()) == v) continue;
 *     \endcode
 *
 *   <li> Iteration over all nodes \a x reachable by an outgoing edge \a e
 *        of node \a v (with self-loops):
 *     \code
 *  forall_adj_edges(e,v) {
 *    if (e->source() != v) continue;
 *    x = e->target();
 *  }
 *     \endcode
 *
 *  <li> Iteration over all nodes \a x reachable by an ingoing edge \a e
 *       of node \a v (without self-loops):
 *     \code
 *  forall_adj_edges(e,v)
 *    if ((x = e->source()) == v) continue;
 *     \endcode
 *
 * <li> Iteration over all nodes \a x reachable by an ingoing edge \a e
 *      of node \a v (with self-loops):
 *     \code
 *  forall_adj_edges(e,v) {
 *    if (e->target() != v) continue;
 *    x = e->source();
 *  }
 *     \endcode
 * </ul>
 */

class OGDF_EXPORT Graph
{
	GraphList<NodeElement> m_nodes; //!< The list of all nodes.
	GraphList<EdgeElement> m_edges; //!< The list of all edges.
	int m_nNodes; //!< The number of nodes in the graph.
	int m_nEdges; //!< The number of edges in the graph.

	int m_nodeIdCount; //!< The Index that will be assigned to the next created node.
	int m_edgeIdCount; //!< The Index that will be assigned to the next created edge.

	int m_nodeArrayTableSize; //!< The current table size of node arrays associated with this graph.
	int m_edgeArrayTableSize; //!< The current table size of edge arrays associated with this graph.

	mutable ListPure<NodeArrayBase*> m_regNodeArrays; //!< The registered node arrays.
	mutable ListPure<EdgeArrayBase*> m_regEdgeArrays; //!< The registered edge arrays.
	mutable ListPure<AdjEntryArrayBase*> m_regAdjArrays;  //!< The registered adjEntry arrays.
	mutable ListPure<GraphObserver*> m_regStructures; //!< The registered graph structures.

	GraphList<EdgeElement> m_hiddenEdges; //!< The list of hidden edges.

public:
	//
	// enumerations
	//

	//! The type of edges (only used in derived classes).
	enum EdgeType {
		association = 0,
		generalization = 1,
		dependency = 2
	}; // should be more flexible, standard, dissect, expand
	
	//! The type of nodes.
	enum NodeType {
		vertex,
		dummy,
		generalizationMerger,
		generalizationExpander,
		highDegreeExpander,
		lowDegreeExpander,
		associationClass
	};


	//! Constructs an empty graph.
	Graph();
	
	//! Constructs a graph that is a copy of \a G.
	/**
	 * The constructor assures that the adjacency lists of nodes in the
	 * constructed graph are in the same order as the adjacency lists in \a G.
	 * This is in particular important when dealing with embedded graphs.
	 */
	Graph(const Graph &G);
	
	// destruction
	virtual ~Graph();


	/**
	 * @name Access methods
	 */
	//@{

	//! Returns true iff the graph is empty, i.e., contains no nodes.
	bool empty() const { return m_nNodes == 0; }

	//! Returns the number of nodes in the graph.
	int numberOfNodes() const { return m_nNodes; }

	//! Returns the number of edges in the graph.
	int numberOfEdges() const { return m_nEdges; }

	//! Returns the largest used node index.
	int maxNodeIndex() const { return m_nodeIdCount-1; }
	//! Returns the largest used edge index.
	int maxEdgeIndex() const { return m_edgeIdCount-1; }
	//! Returns the largest used adjEntry index.
	int maxAdjEntryIndex() const { return (m_edgeIdCount<<1)-1; }

	//! Returns the table size of node arrays associated with this graph.
	int nodeArrayTableSize() const { return m_nodeArrayTableSize; }
	//! Returns the table size of edge arrays associated with this graph.
	int edgeArrayTableSize() const { return m_edgeArrayTableSize; }
	//! Returns the table size of adjEntry arrays associated with this graph.
	int adjEntryArrayTableSize() const { return m_edgeArrayTableSize << 1; }

	//! Returns the first node in the list of all nodes.
	node firstNode() const { return m_nodes.begin (); }
	//! Returns the last node in the list of all nodes.
	node lastNode () const { return m_nodes.rbegin(); }
	
	//! Returns the first edge in the list of all edges.
	edge firstEdge() const { return m_edges.begin (); }
	//! Returns the last edge in the list of all edges.
	edge lastEdge () const { return m_edges.rbegin(); }

	//! Returns a randomly chosen node.
	node chooseNode() const;
	//! Returns a randomly chosen edge.
	edge chooseEdge() const;

	//! Returns a list with all nodes of the graph.
	template<class NODELIST>
	void allNodes(NODELIST &nodes) const {
		nodes.clear();
		for (node v = m_nodes.begin(); v; v = v->succ())
			nodes.pushBack(v);
	}

	//! Returns a list with all edges of the graph.
	template<class EDGELIST>
	void allEdges(EDGELIST &edges) const {
		edges.clear();
		for (edge e = m_edges.begin(); e; e = e->succ())
			edges.pushBack(e);
	}

	//! Returns a list with all edges adjacent to node \a v.
	template<class EDGELIST>
	void adjEdges(node v, EDGELIST &edges) const {
		edges.clear();
		edge e;
		forall_adj_edges(e,v)
			edges.pushBack(e);
	}

	//! Returns a list with all entries in the adjacency list of node \a v.
	template<class ADJLIST>
	void adjEntries(node v, ADJLIST &entries) const {
		entries.clear();
		adjEntry adj;
		forall_adj(adj,v)
			entries.pushBack(adj);
	}

	//! Returns a list with all incoming edges of node \a v.
	template<class EDGELIST>
	void inEdges(node v, EDGELIST &edges) const {
		edges.clear();
		edge e;
		forall_adj_edges(e,v)
			if (e->target() == v) edges.pushBack(e);
	}

	//! Returns a list with all outgoing edges of node \a v.
	template<class EDGELIST>
	void outEdges(node v, EDGELIST &edges) const {
		edges.clear();
		edge e;
		forall_adj_edges(e,v)
			if (e->source() == v) edges.pushBack(e);
	}


	//@}
	/**
	 * @name Creation of new nodes and edges
	 */
	//@{

	//! Creates a new node and returns it.
	node newNode();

	//! Creates a new node with predefined index and returns it.
	/**
	 * \pre \a index is currently not the index of any other
	 *   node in the graph.
	 * \attention Passing a node index that is already in use results
	 * in an inconsistent data structure. Only use this method if you
	 * know what you're doing!
	 */
	node newNode(int index);

	//! Creates a new edge (\a v,\a w) and returns it.
	edge newEdge(node v, node w);

	//! Creates a new edge (\a v,\a w) with predefined index and returns it.
	/**
	 * \pre \a index is currently not the index of any other
	 *   edge in the graph.
	 * \attention  Passing an edge index that is already in use results
	 * in an inconsistent data structure. Only use this method if you
	 * know what you're doing!
	 */
	edge newEdge(node v, node w, int index);

	//! Creates a new edge at predefined positions in the adjacency lists.
	/**
	 * Let \a v be the node whose adjacency list contains \a adjSrc,
	 * and \a w the node whose adjacency list contains \a adjTgt. Then,
	 * the created edge is (\a v,\a w).
	 * @param adjSrc is the adjacency entry after which the new edge is inserted
	 *        in the adjacency list of \a v.
	 * @param adjTgt is the adjacency entry after which the new edge is inserted
	 *        in the adjacency list of \a w.
	 * @param dir specifies if the edge is inserted before or after the given
	 *        adjacency entries.
	 * \return the created edge.
	 */
	edge newEdge(adjEntry adjSrc, adjEntry adjTgt, Direction dir = ogdf::after);
	
	//! Creates a new edge at predefined positions in the adjacency lists.
	/**
	 * Let \a w be the node whose adjacency list contains \a adjTgt. Then,
	 * the created edge is (\a v,\a w).
	 * @param v is the source node of the new edge; the edge is added at the end
	 *        of the adjacency list of \a v.
	 * @param adjTgt is the adjacency entry after which the new edge is inserted
	 *        in the adjacency list of \a w.
	 */
	edge newEdge(node v, adjEntry adjTgt);

	//! Creates a new edge at predefined positions in the adjacency lists.
	/**
	 * Let \a v be the node whose adjacency list contains \a adjSrc. Then,
	 * the created edge is (\a v,\a w).
	 * @param adjSrc is the adjacency entry after which the new edge is inserted
	 *        in the adjacency list of \a v.
	 * @param w is the source node of the new edge; the edge is added at the end
	 *        of the adjacency list of \a w.
	 */
	edge newEdge(adjEntry adjSrc, node w);


	//@}
	/**
	 * @name Removing nodes and edges
	 */
	//@{

	//! Removes node \a v and all incident edges from the graph.
	void delNode(node v);

	//! Removes edge \a e from the graph.
	void delEdge(edge e);

	//! Removes all nodes and all edges from the graph.
	void clear();

	
	//@}
	/**
	 * @name Hiding edges
	 * These methods are used for temporarily hiding edges. Edges are removed from the
	 * list of all edges and their corresponding adfjacency entries from the repsective
	 * adjacency lists, but the edge objects themselves are not destroyed; hiddenedges
	 * can later be reactivated with restoreEdge().
	 */
	//@{

	//! Hides the edge \a e.
	/**
	 * The edge \a e is removed from the list of all edges and adjacency lists of nodes, but
	 * not deleted; \a e can be restored by calling restoreEdge(e).
	 * \attention If an edge is hidden, its source and target node may not be deleted!
	 */
	void hideEdge(edge e);

	//! Restores a hidden edge \a e.
	/**
	 * \pre \a e is currently hidden and the source and target not have not been removed!
	 */
	void restoreEdge(edge e);

	//! Restores all hidden edges.
	void restoreAllEdges();

	/**
	 * @name Advanced modification methods
	 */
	//@{

	
	//! Splits edge \a e into two edges introducing a new node.
	/**
	 * Let \a e=(\a v,\a w). Then, the resulting two edges are \a e=(\a v,\a u)
	 * and \a e'=(\a u,\a w), where \a u is a new node.
	 * \return The edge \a e'.
	 * \note The edge \a e is modified by this operation.
	 */
	virtual edge split(edge e);

	//! Undoes a split operation.
	/**
	 * Removes node \a u by joining the two edges adjacent to \a u. The
	 * outgoing edge of \a u is removed and the incoming edge \a e is reused
	 * \return The edge \a e.
	 * \pre \a u has exactly one incoming and one outgoing edge, and
	 *    none of them is a self-loop.
	 */
	void unsplit(node u);

	//! Undoes a split operation.
	/**
	 * For two edges \a eIn = (\a x,\a u) and \a eOut = (\a u,\a y), removes
	 * node \a u by joining \a eIn and \a eOut. Edge \a eOut is removed and
	 * \a eIn is reused.
	 *
	 * \pre \a eIn and \a eOut are the only edges incident with \a u and
	 *      none of them is a self-loop.
	 */
	virtual void unsplit(edge eIn, edge eOut);

	//! Splits a node while preserving the order of adjacency entries.
	/**
	 * This method splits a node \a v into two nodes \a vl and \a vr. Node
	 * \a vl receives all adjacent edges of \a v from \a adjStartLeft until
	 * the edge preceding \a adjStartRight, and \a vr the remaining nodes
	 * (thus \a adjStartRight is the first edge that goes to \a vr). The
	 * order of adjacency entries is preserved. Additionally, a new edge
	 * (\a vl,\a vr) is created, such that this edge is inserted before
	 * \a adjStartLeft and \a adjStartRight in the the adjacency lists of
	 * \a vl and \a vr.
	 *
	 * Node \a v is modified to become node \a vl, and node \a vr is returned.
	 * This method is useful when modifying combinatorial embeddings.
	 *
	 * @param adjStartLeft is the first entry that goes to the left node.
	 * @param adjStartRight is the first entry that goes to the right node.
	 * \return the newly created node.
	 */
	node splitNode(adjEntry adjStartLeft, adjEntry adjStartRight);

	//! Contracts edge \a e while preserving the order of adjacency entries.
	node contract(edge e);

	// moves edge e to adjacency list of owner(adjSrc) inserting it before
	// or after adjSrc, and to owner(adjTgt) inserting it before or after
	// adjTgt; e is afterwards an edge from owner(adjSrc) to owner(adjTgt)
	void move(edge e, adjEntry adjSrc, Direction dirSrc,
		adjEntry adjTgt, Direction dirTgt);

	//! Moves the target node of edge \a e to node \a w.
	/**
	 * If \a e=(\a v,\a u) before, then \a e=(\a v,\a w) afterwards-
	 */
	void moveTarget(edge e, node w);

	//! Moves the target node of edge \a e to node \a w.
	/**
	 * If \a e=(\a v,\a u) before, then \a e=(\a v,\a w) afterwards.
	 * Inserts the adjacency entry before or after \a adjTgt according to \a dir.
	 */
	void moveTarget(edge e, adjEntry adjTgt, Direction dir);

	//! Moves the source node of edge \a e to node \a w.
	/**
	 * If \a e=(\a v,\a u) before, then \a e=(\a w,\a u) afterwards.
	 */
	void moveSource(edge e, node w);

	//! Moves the source node of edge \a e to node \a w.
	/**
	 * If \a e=(\a v,\a u) before, then \a e=(\a w,\a u) afterwards.
	 * Inserts the adjacency entry before or after \a adjSrc according to \a dir.
	 */
	void moveSource(edge e, adjEntry adjSrc, Direction dir);
	
	//! Searches and returns the edge between node \a v and node \a w.
	//  If no such edge exists, 0 is returned.
	edge searchEdge (node v, node w) const;

	//! Reverses the edge \a e, i.e., exchanges source and target node.
	void reverseEdge(edge e);

	//! Reverses all edges in the graph.
	void reverseAllEdges();

	//! Collapses all nodes in the list \a nodes to the first node in the list. 
	/**
	 *  Parallel edges are removed.
	 */
	template<class NODELIST>
	void collaps(NODELIST &nodes){
		node v = nodes.popFrontRet();
		while (!nodes.empty())
		{
			node w = nodes.popFrontRet();
			adjEntry adj = w->firstAdj();
			while (adj !=0)
			{
				adjEntry succ = adj->succ();
				edge e = adj->theEdge();
				if (e->source() == v || e->target() == v)
					delEdge(e);
				else if (e->source() == w)
					moveSource(e,v);
				else
					moveTarget(e,v);
				adj = succ;
			}
			delNode(w);
		}
	}

	//! Sorts the adjacency list of node \a v according to \a newOrder.
	/**
	 * \pre \a newOrder contains exactly the adjacency entries of \a v!
	 */
	template<class ADJ_ENTRY_LIST>
	void sort(node v, const ADJ_ENTRY_LIST &newOrder) {
#ifdef OGDF_DEBUG
		typename ADJ_ENTRY_LIST::const_iterator it;
		for(it = newOrder.begin(); it.valid() ; ++it) {
			OGDF_ASSERT((*it)->theNode() == v);
		}
#endif
		v->m_adjEdges.sort(newOrder);
	}

	//! Reverses the adjacency list of \a v.
	void reverseAdjEdges(node v) {
		v->m_adjEdges.reverse();
	}

	//! Moves the adjacency entry \a adjMove before or after \a adjPos.
	/**
	 * @param adjMove is an entry in the adjacency list of a node in this graph.
	 * @param adjPos is an entry in the same adjacency list as \a adjMove.
	 * @param dir specifies if \a adjMove is moved before or after \a adjPos.
	 * \pre \a adjMove and adjAfter are distinct entries in the same adjacency list.
	 */
	void moveAdj(adjEntry adjMove, Direction dir, adjEntry adjPos) {
		OGDF_ASSERT(adjMove->graphOf() == this && adjPos->graphOf() == this);
		OGDF_ASSERT(adjMove != 0 && adjPos != 0);
		GraphList<AdjElement> &adjList = adjMove->m_node->m_adjEdges;
		adjList.move(adjMove, adjList, adjPos, dir);
	}

	//! Moves the adjacency entry \a adjMove after \a adjAfter.
	/**
	 * @param adjMove is an entry in the adjacency list of a node in this graph.
	 * @param adjAfter is an entry in the same adjacency list as \a adjMove.
	 * \pre \a adjMove and \a adjAfter are distinct entries in the same adjacency list.
	 */
	void moveAdjAfter(adjEntry adjMove, adjEntry adjAfter) {
		OGDF_ASSERT(adjMove->graphOf() == this && adjAfter->graphOf() == this);
		OGDF_ASSERT(adjMove != 0 && adjAfter != 0);
		adjMove->m_node->m_adjEdges.moveAfter(adjMove,adjAfter);
	}

	//! Moves the adjacency entry \a adjMove before \a adjBefore.
	/**
	 * @param adjMove is an entry in the adjacency list of a node in this graph.
	 * @param adjBefore is an entry in the same adjacency list as \a adjMove.
	 * \pre \a adjMove and \a adjBefore are distinct entries in the same adjacency list.
	 */
	void moveAdjBefore(adjEntry adjMove, adjEntry adjBefore) {
		OGDF_ASSERT(adjMove->graphOf() == this && adjBefore->graphOf() == this);
		OGDF_ASSERT(adjMove != 0 && adjBefore != 0);
		adjMove->m_node->m_adjEdges.moveBefore(adjMove,adjBefore);
	}

	//! Reverses all adjacency lists.
	void reverseAdjEdges();

	//! Exchanges two entries in an adjacency list.
	/**
	 * \pre \a adj1 and \a adj2 must be in a common adjacency list.
	 */
	void swapAdjEdges(adjEntry adj1, adjEntry adj2) {
		OGDF_ASSERT(adj1->theNode() == adj2->theNode());
		OGDF_ASSERT(adj1->graphOf() == this);

		adj1->theNode()->m_adjEdges.swap(adj1,adj2);
	}


	//@}
	/**
	 * @name Input and output
	 */
	//@{

	//! Reads a graph in GML format from file \a fileName.
	bool readGML(const char *fileName);

	//! Reads a graph in GML format from input stream \a is.
	bool readGML(istream &is);

	//! Writes the graph in GML format to file \a fileName.
	void writeGML(const char *fileName) const;

	//! Writes the graph in GML format to output stream \a os.
	void writeGML(ostream &os) const;

	//! Reads a graph in LEDA format from file \a fileName.
	bool readLEDAGraph(const char *fileName);

	//! Read a graph in LEDA format from input stream \a is.
	bool readLEDAGraph(istream &is);


	//@}
	/**
	 * @name Miscellaneous 
	 */
	//@{

	//! Returns the genus of the graph.
	/**
	 * The genus of a graph is defined as follows. Let \f$G\f$ be a graph
	 * with \f$m\f$ edges, \f$n\f$ nodes, \f$c\f$ connected components, \f$nz\f$
	 * isolated vertices, and \f$fc\f$ face cycles. Then,
	 * \f[
	 *   genus(G) = (m/2 + 2c - n -nz -fc)/2
	 * \f]
	 */
	int genus() const;

	//! Returns true iff the graph represents a combinatorial embedding.
	bool representsCombEmbedding() const {
		return (genus() == 0);
	}

	//! Checks the consistency of the data structure.
	/**
	 * \remark This method is meant for debugging purposes.
	 */
	bool consistencyCheck() const;


	//@}
	/**
	 * @name Registering of arrays
	 * These methods are used by various graph array types like NodeArray or EdgeArray.
	 * There should be no need to use them directly in user code.
	 */
	//@{

	//! Registers a node array.
	ListIterator<NodeArrayBase*> registerArray(NodeArrayBase *pNodeArray) const;
	//! Registers an edge array.
	ListIterator<EdgeArrayBase*> registerArray(EdgeArrayBase *pEdgeArray) const;
	//! Registers an adjEntry array.
	ListIterator<AdjEntryArrayBase*> registerArray(AdjEntryArrayBase *pAdjArray) const;
	//! Registers a GraphObserver (e.g. a ClusterGraph).
	ListIterator<GraphObserver*> registerStructure(GraphObserver *pStructure) const;

	//! Unregisters a node array.
	void unregisterArray(ListIterator<NodeArrayBase*> it) const;
	//! Unregisters an edge array.
	void unregisterArray(ListIterator<EdgeArrayBase*> it) const;
	//! unregisters an adjEntry array.
	void unregisterArray(ListIterator<AdjEntryArrayBase*> it) const;
	//! Unregisters a GraphObserver.
	void unregisterStructure(ListIterator<GraphObserver*> it) const;


	//! Resets the edge id count to \a maxId.
	/**
	 * The next edge will get edge id \a maxId+1. Use this function with caution!
	 * It is provided as an efficient way to reduce the edge id count. The Graph class
	 * increments the edge id count whenever an edge is created; free edge ids resulting
	 * from removing edges are not reused (there is not something like a freelist).
	 *
	 * This function is , e.g., useful, when a lot of edges has been added and
	 * <em>all</em> these edges are removed again (without creating other new edges
	 * meanwile). Then, it is safe to reduce the edge id count to the value it had
	 * before, cf. the following code snippet:
	 * \code
	 *   int oldIdCount = G.maxEdgeIndex();
	 *   Create some edges
	 *   ...
	 *   Remove all these edges again
	 *   G.resetEdgeIdCount(oldIdCount);
	 * \endcode
	 *
	 * Reducing the edge id count will reduce the memory consumption of edge arrays
	 * associated with the graph.
	 *
	 * @param maxId is an upper bound of the edge ids in the graph.
	 * \pre -1 \f$\leq\f$ \a maxId \f$\leq\f$ maximal edge id in the graph.
	 */
	void resetEdgeIdCount(int maxId);


	//@}
	/**
	 * @name Operators
	 */
	//@{
	//! Assignment operator.
	/**
	 * The assignment operature assures that the adjacency lists of nodes in the
	 * constructed graph are in the same order as the adjacency lists in \a G.
	 * This is in particular important when dealing with embedded graphs.
	 */
	Graph &operator=(const Graph &G);

	OGDF_MALLOC_NEW_DELETE

	//@}

public:

	//! Returns the smallest power of 2 which is >= 2^\a start and > \a idCount.
	static int nextPower2(int start, int idCount);


protected:
	void construct(const Graph &G, NodeArray<node> &mapNode,
		EdgeArray<edge> &mapEdge);

	void assign(const Graph &G, NodeArray<node> &mapNode,
		EdgeArray<edge> &mapEdge);

	/**
	 * \brief Constructs a copy of the subgraph of \a G induced by \a nodes.
	 * 
	 * This method preserves the order in the adjacency lists, i.e., if
	 * \a G is embedded, its embedding induces the embedding of the copy.
	 */
	void constructInitByNodes(
		const Graph &G,
		const List<node> &nodes,
		NodeArray<node> &mapNode,
		EdgeArray<edge> &mapEdge);

	void constructInitByActiveNodes(
		const List<node> &nodes,
		const NodeArray<bool> &activeNodes,
		NodeArray<node> &mapNode,
		EdgeArray<edge> &mapEdge);

private:
	void copy(const Graph &G, NodeArray<node> &mapNode,
		EdgeArray<edge> &mapEdge);
	void copy(const Graph &G);

	edge createEdgeElement(node v, node w, adjEntry adjSrc, adjEntry adjTgt);
	node pureNewNode();

	// moves adjacency entry to node w
	void moveAdj(adjEntry adj, node w);

	void reinitArrays();
	void reinitStructures();
	void resetAdjEntryIndex(int newIndex, int oldIndex);

	bool readToEndOfLine(istream &is);
}; // class Graph



//! Bucket function using the index of an edge's source node as bucket.
class OGDF_EXPORT BucketSourceIndex : public BucketFunc<edge> {
public:
	//! Returns source index of \a e.
	int getBucket(const edge &e) { return e->source()->index(); }
};

//! Bucket function using the index of an edge's target node as bucket.
class OGDF_EXPORT BucketTargetIndex : public BucketFunc<edge> {
public:
	//! Returns target index of \a e.
	int getBucket(const edge &e) { return e->target()->index(); }
};


} //namespace 

#endif

