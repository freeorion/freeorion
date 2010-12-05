/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Declaration and implementation of a HyperGraph
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

#ifndef OGDF_HYPER_GRAPH_H
#define OGDF_HYPER_GRAPH_H

#include <ogdf/basic/EList.h>
#include <ogdf/basic/EFreeList.h>
#include <iostream>

namespace ogdf {

class HyperGraph
{
public:
	//! forward declaration to make AdjElement pointers available
	class AdjElement;

	//! Class for a node element
	class NodeElement
	{
		friend class HyperGraph;
	public:
		//! Returns the index of this node.
		inline int index() const { return m_index; };

		//! Returns the number of incident edges.
		inline int degree() const { return m_numAdj; };

	private:
		NodeElement* m_pPrev;
		NodeElement* m_pNext;
		AdjElement* m_pFirstAdj;
		AdjElement* m_pLastAdj;
		int m_numAdj;
		int m_index;
	}; // class NodeElement

	//! Class for a hyper edge.
	class EdgeElement
	{
		friend class HyperGraph;
	public:
		//! Returns the index of this element used when accessing arrays.
		inline int index() const { return m_index; }

		//! Returns the number of incident nodes.
		inline int cardinality() const { return m_numAdj; };

	private:
		EdgeElement* m_pPrev;
		EdgeElement* m_pNext;
		AdjElement* m_pFirstAdj;
		AdjElement* m_pLastAdj;
		int m_numAdj;
		int m_index;
	}; // class EdgeElement

	//! class for an adjacency element
	class AdjElement
	{
		friend class HyperGraph;
	public:
		inline AdjElement() : m_pNode(0),
							  m_pEdge(0) {};

		//! Returns the node this instance connects to an edge
		inline NodeElement* theNode() { return m_pNode; };

		//! Returns the edge this instance connects to a node
		inline EdgeElement* theEdge() { return m_pEdge; };

		//! Returns the index of this element used when accessing arrays
		inline int index() const { return m_index; }
	private:

		inline AdjElement(NodeElement* pNode,
						  EdgeElement* pEdge) :
							m_pNode(pNode),
							m_pEdge(pEdge),
							m_pPrev_nodeAdj(0),
							m_pNext_nodeAdj(0),
							m_pPrev_edgeAdj(0),
							m_pNext_edgeAdj(0)
		{};

		NodeElement* m_pNode;
		EdgeElement* m_pEdge;

		AdjElement* m_pPrev_nodeAdj;
		AdjElement* m_pNext_nodeAdj;

		AdjElement* m_pPrev_edgeAdj;
		AdjElement* m_pNext_edgeAdj;
		int m_index;
	};

	typedef NodeElement* node;
	typedef EdgeElement* edge;

protected:
	NodeElement* m_pFirstNode;
	NodeElement* m_pLastNode;
	int m_numNodes;

	EdgeElement* m_pFirstEdge;
	EdgeElement* m_pLastEdge;
	int m_numEdges;

public:
	typedef EList<
				HyperGraph, NodeElement,
				&HyperGraph::m_numNodes,
				&HyperGraph::m_pFirstNode,
				&HyperGraph::m_pLastNode,
				&NodeElement::m_pPrev,
				&NodeElement::m_pNext
			> NodeList;

	//! typedef for the embedded list of edges
	typedef EList<
				HyperGraph, EdgeElement,
				&HyperGraph::m_numEdges,
				&HyperGraph::m_pFirstEdge,
				&HyperGraph::m_pLastEdge,
				&EdgeElement::m_pPrev,
				&EdgeElement::m_pNext
			> EdgeList;

	//! typedef for the embedded list of AdjElement at a node
	typedef EList<
				NodeElement, AdjElement,
				&NodeElement::m_numAdj,
				&NodeElement::m_pFirstAdj,
				&NodeElement::m_pLastAdj,
				&AdjElement::m_pPrev_nodeAdj,
				&AdjElement::m_pNext_nodeAdj
			> NodeAdjList;

	//! typedef for the embedded list of AdjElement at an edge
	typedef EList<
				EdgeElement, AdjElement,
				&EdgeElement::m_numAdj,
				&EdgeElement::m_pFirstAdj,
				&EdgeElement::m_pLastAdj,
				&AdjElement::m_pPrev_edgeAdj,
				&AdjElement::m_pNext_edgeAdj
			> EdgeAdjList;
protected:
	EFreeListIndexPool<
		NodeElement,
		&NodeElement::m_pNext,
		&NodeElement::m_index
	> m_nodeAllocator;

	EFreeListIndexPool<
		EdgeElement,
		&EdgeElement::m_pNext,
		&EdgeElement::m_index
	> m_edgeAllocator;

	EFreeListIndexPool<
		AdjElement,
		&AdjElement::m_pNext_nodeAdj, // note: we just need one next pointer, nobody cares which one.
		&AdjElement::m_index
	> m_adjAllocator;

	template<typename E>
	class GraphArrayBase
	{
		friend class HyperGraph;
	public:
		//! Interface method for resizing a graph array.
		virtual void setSize(int numElements) = 0;

		//! the embedded next and  prev pointer.
		GraphArrayBase<E>* m_prev;
		GraphArrayBase<E>* m_next;
	};

	template<typename T, typename E>
	class GraphArray : protected Array<T>, GraphArrayBase<E>
	{
		friend class HyperGraph;
	public:
		// Creates an empty graph array not attached to a graph.
		GraphArray() : m_pGraph(0), Array<T>() {};

		//! Creates a graph array attached to \a pGraph.
		GraphArray(HyperGraph* pGraph) : m_pGraph(pGraph), m_initialValue(), Array<T>()
		{
			if (m_pGraph)
				m_pGraph->registerArray(this);
		}

		//! Creates a graph array attached to \a pGraph where each entry
		GraphArray(HyperGraph* pGraph, const T& initialValue) : m_pGraph(pGraph), m_initialValue(initialValue), Array<T>()
		{
			if (m_pGraph)
				m_pGraph->registerArray(this);
		}

		//! If the array is attached to a graph, the array will be detached before it is deallocated.
		virtual ~GraphArray()
		{
			if (m_pGraph)
				m_pGraph->unregisterArray(this);
		}

		//! Returns a const reference to the entry for an indexed element.
		inline const T& operator[](const E* e) const
		{
			return Array<T>::operator [](e->index());
		}

		//! Returns a reference to the entry for an indexed element.
		inline T& operator[](const E* e)
		{
			return Array<T>::operator [](e->index());
		}

		//! Returns the graph the array is attached to.
		inline HyperGraph* graph() const
		{
			return m_pGraph;
		}

	protected:

		//! Implementation of \a GraphArrayBase::setSize(int).
		virtual void setSize(int numElements)
		{
			if (numElements > Array<T>::size())
				Array<T>::grow(numElements-Array<T>::size(),m_initialValue);
		}

		//! The graph the array is attached to.
		HyperGraph* m_pGraph;

		//! The initial value for unset entries.
		T m_initialValue;

	}; // end of class GraphArray

	//! The \a ArrayController manages the GraphArrays of an Element.
	template<typename ElementType>
	class ArrayController
	{
	public:
		//! Creates a new ArrayController where the actual table size is zero
		ArrayController() : m_tableSize(0)
		{
			ArrayListType::init(this);
		}

		//! Frees the ArrayController and unregisters all remaining arrays
		~ArrayController()
		{
			// Unregister all arrays, but do not delete them.
			while (!ArrayListType::empty(this))
				unregisterArray(ArrayListType::front(this));
		}

		//! Registers a new GraphArray and sets the size to \a m_tableSize.
		inline void registerArray(GraphArrayBase<ElementType>* pArray)
		{
			ArrayListType::pushBack(this, pArray);
			pArray->setSize(m_tableSize);
		}

		//! Unregisters an array by removing it from the list
		inline void unregisterArray(GraphArrayBase<ElementType>* pArray)
		{
			ArrayListType::remove(this, pArray);
			pArray->setSize(0);
		}

		//! function to calculate the new tableSize. returns min(2^k | 2^k >= minSize)
		inline int newTableSize(int minSize)
		{
			int res = 1;
			while (res < minSize)
				res = res << 1;
			return res;
		}

		//! called by the hyper graph when the amount of used entries has changed.
		inline void numUsedIndicesChanged(int numUsedIndices)
		{
			// check if we have to grow
			if (numUsedIndices > m_tableSize)
			{
				// we have to resize
				// calculate new table size
				m_tableSize = newTableSize(numUsedIndices);
				// iterate over all arrays
				for (typename ArrayListType::iterator it = ArrayListType::begin(this);
					 it.valid(); it++)
				{
					// and resize them
					(*it)->setSize(m_tableSize);
				};
			};
		}

	protected:
		//! First array in the chain of attached arrays
		GraphArrayBase<ElementType>* m_first;

		//! Last array in the chain of attached arrays
		GraphArrayBase<ElementType>* m_last;

		//! Number of attached arrays
		int m_numArrays;

		//! Current table size which corresponds to the length of all attached arrays.
		int m_tableSize;

		//! Typedef for the embedded list of arrays
		typedef EList< ArrayController<ElementType>, GraphArrayBase<ElementType>,
					   &ArrayController<ElementType>::m_numArrays,
					   &ArrayController<ElementType>::m_first,
					   &ArrayController<ElementType>::m_last,
					   &GraphArrayBase<ElementType>::m_prev,
					   &GraphArrayBase<ElementType>::m_next> ArrayListType;
	};

	ArrayController<NodeElement>  m_nodeArrayController; 	//!< controller for node arrays
	ArrayController<EdgeElement>  m_edgeArrayController; 	//!< controller for edge arrays
	ArrayController<AdjElement>  m_adjArrayController; 		//!< controller for adj arrays

	void registerArray(GraphArrayBase<NodeElement>* pArray) { m_nodeArrayController.registerArray(pArray); }
	void registerArray(GraphArrayBase<EdgeElement>* pArray) { m_edgeArrayController.registerArray(pArray); };
	void registerArray(GraphArrayBase<AdjElement>* pArray) { m_adjArrayController.registerArray(pArray); };


	void unregisterArray(GraphArrayBase<NodeElement>* pArray) { m_nodeArrayController.unregisterArray(pArray); }
	void unregisterArray(GraphArrayBase<EdgeElement>* pArray) { m_edgeArrayController.unregisterArray(pArray); };
	void unregisterArray(GraphArrayBase<AdjElement>* pArray) { m_adjArrayController.unregisterArray(pArray); };

	//! Allocates a new NodeElement using the EFreeListIndexPool and lets the controller check if arrays have to be resized.
	inline NodeElement* allocateNodeElement()
	{
		NodeElement* pResult = m_nodeAllocator.alloc();
		m_nodeArrayController.numUsedIndicesChanged(m_nodeAllocator.numUsedIndices());
		return pResult;
	}

	//! Allocates a new EdgeElement using the EFreeListIndexPool and lets the controller check if arrays have to be resized.
	inline EdgeElement* allocateEdgeElement()
	{
		EdgeElement* pResult = m_edgeAllocator.alloc();
		m_edgeArrayController.numUsedIndicesChanged(m_edgeAllocator.numUsedIndices());
		return pResult;
	}

	//! Allocates a new AdjElement using the EFreeListIndexPool and lets the controller check if arrays have to be resized.
	inline AdjElement* allocateAdjElement()
	{
		AdjElement* pResult = m_adjAllocator.alloc();
		m_adjArrayController.numUsedIndicesChanged(m_adjAllocator.numUsedIndices());
		return pResult;
	}

	//! Frees a used NodeElement
	inline void freeNodeElement(NodeElement* pNode)
	{
		m_nodeAllocator.free(pNode);
	}

	//! Frees a used EdgeElement
	inline void freeEdgeElement(EdgeElement* pEdge)
	{
		m_edgeAllocator.free(pEdge);
	}

	//! Frees a used AdjElement
	inline void freeAdjElement(AdjElement* pAdj)
	{
		m_adjAllocator.free(pAdj);
	}

public:

	//! NodeArray implementation.
	template<typename T>
	class NodeArray : public GraphArray<T, NodeElement>
	{
	public:
		NodeArray(HyperGraph* pGraph) : GraphArray<T, NodeElement>(pGraph) {}
		NodeArray(HyperGraph* pGraph, const T& initValue) : GraphArray<T, NodeElement>(pGraph, initValue) {}
	};

	//! EdgeArray implementation.
	template<typename T>
	class EdgeArray : public GraphArray<T, EdgeElement>
	{
	public:
		EdgeArray(HyperGraph* pGraph) : GraphArray<T, EdgeElement>(pGraph) {}
		EdgeArray(HyperGraph* pGraph, const T& initValue) : GraphArray<T, EdgeElement>(pGraph, initValue) {}
	};

	//! AdjArray implementation.
	template<typename T>
	class AdjArray : public GraphArray<T, AdjElement>
	{
	public:
		AdjArray(HyperGraph* pGraph) : GraphArray<T, AdjElement>(pGraph) {}
		AdjArray(HyperGraph* pGraph, const T& initValue) : GraphArray<T, AdjElement>(pGraph, initValue) {}
	};


	//! Constructor for an empty hyper graph.
	HyperGraph()
	{
		NodeList::init( this );
		EdgeList::init( this );
	}

	//! Creates a new node.
	inline NodeElement* newNode()
	{
		NodeElement* pNode = allocateNodeElement();
		NodeList::pushBack( this, pNode );
		NodeAdjList::init( pNode );
		return pNode;
	}

	//! Creates a new edge which is not incident to any nodes.
	inline EdgeElement* newEdge()
	{
		EdgeElement* pEdge = allocateEdgeElement();
		EdgeList::pushBack( this, pEdge );
		EdgeAdjList::init( pEdge );
		return pEdge;
	}

	//! Creates a new edge which is incident to the two nodes \a pNode1, \a pNode2.
	inline EdgeElement* newEdge(NodeElement* pNode1, NodeElement* pNode2)
	{
		EdgeElement* pEdge = newEdge();
		newAdjElement( pNode1, pEdge );
		newAdjElement( pNode2, pEdge );
		return pEdge;
	}

	//! Creates a new \a AdjElement which makes \a pNode and \a pEdge incident.
	/**
	 * Note: This function does not check if \a pNode and \a pEdge are already incident.
	 * The HyperGraph can deal with duplicate AdjElements. However, for reasons of clarity
	 * it is not a good idea to make use of it.
	 * @param pNode is the node.
	 * @param pEdge is the hyper edge.
	 */
	AdjElement* newAdjElement(NodeElement* pNode, EdgeElement* pEdge)
	{
		AdjElement* pAdj = allocateAdjElement();
		pAdj->m_pNode = pNode;
		pAdj->m_pEdge = pEdge;

		NodeAdjList::pushBack( pNode, pAdj );
		EdgeAdjList::pushBack( pEdge, pAdj );
		return pAdj;
	}

	//! Deletes \a pAdj
	void delAdjElement(AdjElement* pAdj)
	{
		EdgeAdjList::remove( pAdj->theEdge(), pAdj );
		NodeAdjList::remove( pAdj->theNode(), pAdj );
		freeAdjElement( pAdj );
	}

	//! Deletes \a pAdj
	void delAdjElement(NodeElement* pNode, EdgeElement* pEdge)
	{
		AdjElement* pAdj = findAdjElement(pNode, pEdge);
		if (pAdj) delAdjElement(pAdj);
	}

	//! Makes \a pNode and \a pEdge incident and the corresponding \a AdjElement is returned.
	inline AdjElement* addNode(NodeElement* pNode, EdgeElement* pEdge, bool checkIfAlreadyExists = false)
	{
		if (!checkIfAlreadyExists)
			return newAdjElement( pNode, pEdge );

		AdjElement* pAdj = findAdjElement(pNode, pEdge);
		if (!pAdj)
			pAdj = newAdjElement( pNode, pEdge );
		return pAdj;
	}

	//! If \a pNode and \a pEdge are incident, the corresponding AdjElement is removed.
	/**
	 * Note: The function requires time O(min(degree(pNode), cardinality(pEdge))).
	 * @param pNode is the node.
	 * @param pEdge is the hyper edge.
	 * @param removeDuplicates is a flag, if it is set the function will remove any existing duplicates too.
	*/
	inline void removeNode(NodeElement* pNode, EdgeElement* pEdge, bool removeDuplicates = false)
	{
		AdjElement* pAdj = findAdjElement(pNode, pEdge);
		if (!removeDuplicates && pAdj)
		{
			delAdjElement(pAdj);
		} else
		{
			while (pAdj)
			{
				delAdjElement(pAdj);
				pAdj = findAdjElement(pNode, pEdge);
			};
		}
	}

	//! Returns the \a AdjElement for pNode and pEdge.
	/**
	 *  In case \a pNode and \a pEdge are not incident the function returns 0.
	 *  Note: The function requires time O(min(degree(pNode), cardinality(pEdge))).
	 * @param pNode is the node.
	 * @param pEdge is the hyper edge.
	 */
	inline AdjElement* findAdjElement(NodeElement* pNode, EdgeElement* pEdge) const
	{
		if (EdgeAdjList::size(pEdge) < NodeAdjList::size(pNode))
		{
			for (EdgeAdjList::iterator it = EdgeAdjList::begin(pEdge);
					it.valid(); it++)
			{
				if ((*it)->theNode() == pNode) return (*it);
			};
		} else
		{
			for (NodeAdjList::iterator it = NodeAdjList::begin(pNode);
					it.valid(); it++)
			{
				if ((*it)->theEdge() == pEdge) return (*it);
			};
		}
		return 0;
	}

	//! Deletes an edge.
	void delEdge(EdgeElement* pEdge)
	{
		for (EdgeAdjList::iterator it = EdgeAdjList::begin(pEdge); it.valid();)
		{
			AdjElement* pAdj = *it;
			NodeAdjList::remove( pAdj->theNode(), pAdj );
			it = EdgeAdjList::remove( pEdge, it );
			freeAdjElement( pAdj );
		};
		EdgeList::remove( this, pEdge );
		freeEdgeElement( pEdge );
	}

	//! Deletes a node.
	void delNode(NodeElement* pNode)
	{
		for (NodeAdjList::iterator it = NodeAdjList::begin(pNode); it.valid();)
		{
			AdjElement* pAdj = *it;
			EdgeAdjList::remove( pAdj->theEdge(), pAdj );
			it = NodeAdjList::remove( pNode, it );
			freeAdjElement( pAdj );
		};
		NodeList::remove( this, pNode );
		freeNodeElement( pNode );
	}

	inline int numberOfNodes() const {  return NodeList::size(this);  };
	inline int numberOfEdges() const {  return EdgeList::size(this);  };

	//! Clears the graph.
	void clear()
	{
		while (numberOfNodes())
		{
			delNode( NodeList::front(this) );
		};

		while (numberOfEdges())
		{
			delEdge( EdgeList::front(this) );
		};
	};

	void toCliqueGraph(Graph* pG,
					   ogdf::NodeArray<NodeElement*>* pNodeMap = 0,
					   ogdf::EdgeArray<EdgeElement*>* pEdgeMap = 0)
	{
		HyperGraph::NodeArray<ogdf::node> hgNode_to_gNode(this);
		for (NodeList::iterator it = HyperGraph::NodeList::begin(this); it.valid(); it++)
		{
			NodeElement* hgNode = *it;
			ogdf::node gNode  = pG->newNode();
			hgNode_to_gNode[ hgNode ] = gNode;
			if (pNodeMap)
				(*pNodeMap)[gNode] = hgNode;
		};

		for (HyperGraph::EdgeList::iterator it = HyperGraph::EdgeList::begin(this); it.valid(); it++)
		{
			EdgeElement* hgEdge = *it;
			for (HyperGraph::EdgeAdjList::iterator adj_it = HyperGraph::EdgeAdjList::begin(hgEdge); adj_it.valid(); adj_it++)
			{
				for (HyperGraph::EdgeAdjList::iterator adj_it2 = adj_it.succ(); adj_it2.valid(); adj_it2++)
				{
					NodeElement* hgNode = (*adj_it)->theNode();
					NodeElement* hgNode2 = (*adj_it2)->theNode();
					ogdf::node gNode  = hgNode_to_gNode[ hgNode ];
					ogdf::node gNode2  = hgNode_to_gNode[ hgNode2 ];

					ogdf::edge gEdge = pG->newEdge(gNode, gNode2);
					if (pEdgeMap)
						(*pEdgeMap)[gEdge] = hgEdge;
				};
			};
		};
	};

	void toStarGraph(Graph* pG,
					 ogdf::NodeArray<NodeElement*>* pNodeMap = 0,
					 ogdf::EdgeArray<EdgeElement*>* pEdgeMap = 0)
	{
		HyperGraph::NodeArray<ogdf::node> hgNode_to_gNode(this);
		for (NodeList::iterator it = HyperGraph::NodeList::begin(this); it.valid(); it++)
		{
			NodeElement* hgNode = *it;
			ogdf::node gNode  = pG->newNode();
			hgNode_to_gNode[ hgNode ] = gNode;
			if (pNodeMap)
				(*pNodeMap)[gNode] = hgNode;
		};

		for (HyperGraph::EdgeList::iterator it = HyperGraph::EdgeList::begin(this); it.valid(); it++)
		{
			EdgeElement* hgEdge = *it;
			ogdf::node gNodeDummy  = pG->newNode();
			if (pNodeMap)
				(*pNodeMap)[gNodeDummy] = 0;

			for (HyperGraph::EdgeAdjList::iterator adj_it = HyperGraph::EdgeAdjList::begin(hgEdge); adj_it.valid(); adj_it++)
			{
				NodeElement* hgNode = (*adj_it)->theNode();
				ogdf::node gNode  = hgNode_to_gNode[ hgNode ];
				ogdf::edge gEdge = pG->newEdge(gNodeDummy, gNode);
				if (pEdgeMap)
					(*pEdgeMap)[gEdge] = hgEdge;
			};
		};
	};
}; // end of class HyperGraph

template<typename ListType, typename ArrayType>
static void writeToStream(std::ostream& outStream, ArrayType& array)
{
	for (typename ListType::iterator it = ListType::begin(array.graph());it.valid(); it++)
	{
		outStream << array[*it] << std::endl;
	};
};

template<typename ListType, typename ArrayType>
static void readFromStream(std::istream& inStream, ArrayType& array)
{
	for (typename ListType::iterator it = ListType::begin(array.graph());it.valid(); it++)
	{
		inStream >> array[(*it)];
	};
};

template<typename T>
static std::ostream& operator<<(std::ostream& outStream, HyperGraph::NodeArray<T>& array)
{
	writeToStream<HyperGraph::NodeList, HyperGraph::NodeArray<T> >(outStream, array);
	return outStream;
};

template<typename T>
static std::ostream& operator<<(std::ostream& outStream, HyperGraph::EdgeArray<T>& array)
{
	writeToStream<HyperGraph::EdgeList, HyperGraph::EdgeArray<T> >(outStream, array);
	return outStream;
};

template<typename T>
static std::ostream& operator<<(std::ostream& outStream, HyperGraph::AdjArray<T>& array)
{
	for (HyperGraph::EdgeList::iterator it = HyperGraph::EdgeList::begin(array.graph());it.valid(); it++)
	{
		HyperGraph::edge e = *it;
		for (HyperGraph::EdgeAdjList::iterator adj_it = HyperGraph::EdgeAdjList::begin(e); adj_it.valid(); adj_it++)
		{
			outStream << array[(*adj_it)] << std::endl;
		};
	};
	return outStream;
};

template<typename T>
static std::istream& operator>>(std::istream& inStream, HyperGraph::NodeArray<T>& array)
{
	readFromStream<HyperGraph::NodeList, HyperGraph::NodeArray<T> >(inStream, array);
	return inStream;
};

template<typename T>
static std::istream& operator>>(std::istream& inStream, HyperGraph::EdgeArray<T>& array)
{
	readFromStream<HyperGraph::EdgeList, HyperGraph::EdgeArray<T> >(inStream, array);
	return inStream;
};

template<typename T>
static std::ostream& operator>>(std::istream& inStream, HyperGraph::AdjArray<T>& array)
{
	for (HyperGraph::EdgeList::iterator it = HyperGraph::EdgeList::begin(array.graph());it.valid(); it++)
	{
		HyperGraph::edge e = *it;
		for (HyperGraph::EdgeAdjList::iterator adj_it = HyperGraph::EdgeAdjList::begin(e); adj_it.valid(); adj_it++)
		{
			inStream >> array[(*adj_it)];
		};
	};
	return inStream;
};

static std::ostream& operator<<(std::ostream& outStream, HyperGraph& graph)
{
	outStream << graph.numberOfNodes() << std::endl;
	outStream << graph.numberOfEdges() << std::endl;

	HyperGraph::NodeArray<int> packed_index(&graph);

	int i=0;
	for (HyperGraph::NodeList::iterator it = HyperGraph::NodeList::begin(&graph);it.valid(); it++)
	{
		packed_index[*it] = i++;
	};

	for (HyperGraph::EdgeList::iterator it = HyperGraph::EdgeList::begin(&graph);it.valid(); it++)
	{
		HyperGraph::edge e = *it;
		outStream << e->cardinality() << std::endl;
		for (HyperGraph::EdgeAdjList::iterator adj_it = HyperGraph::EdgeAdjList::begin(e); adj_it.valid(); adj_it++)
		{
			outStream << packed_index[(*adj_it)->theNode()] << std::endl;
		};
	};
	return outStream;
};

static std::istream& operator>>(std::istream& inStream, HyperGraph& graph)
{
	int numNodes;
	int numEdges;
	int numAdjElements;
	inStream >> numNodes;
	inStream >> numEdges;

	HyperGraph::node* pNodes = new HyperGraph::node[numNodes];
	for (int i=0; i<numNodes;i++)
		pNodes[i] = graph.newNode();

	for (int i=0; i<numEdges;i++)
	{
		HyperGraph::edge e = graph.newEdge();
		int numAdj;
		inStream >> numAdj;
		for (int j=0; j<numAdj; j++)
		{
			int packedNodeIndex;
			inStream >> packedNodeIndex;
			graph.addNode(pNodes[packedNodeIndex], e);
		};
	};
	return inStream;
};

} // end of namespace ogdf

#endif /* HYPERGRAPH_H_ */
