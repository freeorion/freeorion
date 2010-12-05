/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief MLG is the main Datastructure for ModularMultilevelMixerLayout
 *
 * \author Gereon Bartel
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

#ifndef OGDF_MULTILEVEL_GRAPH_H
#define OGDF_MULTILEVEL_GRAPH_H

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <vector>
#include <map>

namespace ogdf {

//Stores info on merging for a refinement level
struct NodeMerge {
	// Node/Edge IDs instead of pointers as the nodes themselves may be nonexistent.
	std::vector<int> m_deletedEdges;
	std::vector<int> m_changedEdges;
	std::map<int, float> m_doubleWeight; // for changed and deleted edges
	std::map<int, int> m_source;
	std::map<int, int> m_target;

	int m_mergedNode;
	std::vector< std::pair<int, float> > m_position; // optional information <target, distance>. mergedNode will be placed at average of relative distances to target.

	std::vector<int> m_changedNodes; // there may be placement strategies that use more than one reference-node.
	std::map<int, float> m_radius; // for changed nodes and the merged node
	
	int m_level;


	NodeMerge(int level);
	~NodeMerge();
};


class MultilevelGraph
{
private:
	bool m_createdGraph;
	Graph * m_G;
	std::vector<NodeMerge *> m_changes;
	NodeArray<float> m_x;
	NodeArray<float> m_y;
	NodeArray<float> m_radius;

	EdgeArray<float> m_weight;

	// Associations to index only as the node/edge may be nonexistent
	NodeArray<int> m_nodeAssociations;
	EdgeArray<int> m_edgeAssociations;

	std::vector<node> m_reverseNodeIndex;
	std::vector<int> m_reverseNodeMergeWeight;//<! Keeps number of vertices represented by vertex with given index  
	std::vector<edge> m_reverseEdgeIndex;

	MultilevelGraph * removeOneCC(std::vector<node> &componentSubArray);
	void copyFromGraph(const Graph &G, NodeArray<int> &nodeAssociations, EdgeArray<int> &edgeAssociations);
	void prepareGraphAttributes(GraphAttributes &GA) const;

	void initReverseIndizes();

public:
	~MultilevelGraph();
	MultilevelGraph();
	MultilevelGraph(Graph &G);
	MultilevelGraph(GraphAttributes &GA);
	// if the Graph is available without const, no copy needs to be created.
	MultilevelGraph(GraphAttributes &GA, Graph &G);

	// creates MultilevelGraph directly from GML file.
	MultilevelGraph(istream &is);
	MultilevelGraph(const String &filename);

	NodeArray<float> & getXArray();
	NodeArray<float> & getYArray();
	NodeArray<float> & getRArray();
	EdgeArray<float> & getWArray();

	edge getEdge(unsigned int index);
	node getNode(unsigned int index);
	float radius(node v);
	void radius(node v, float r);
	float x(node v);
	float y(node v);
	void x(node v, float x);
	void y(node v, float y);

	void weight(edge e, float weight);
	float weight(edge e);
	//returns the merge weight, i.e. the number of nodes represented by v on the current level
	int mergeWeight(node v) {return m_reverseNodeMergeWeight[v->index()];}

	void moveToZero();

	int getLevel();
	Graph & getGraph();
	void exportAttributes(GraphAttributes &GA) const;
	void exportAttributesSimple(GraphAttributes &GA) const;
	void importAttributes(const GraphAttributes &GA);
	void importAttributesSimple(const GraphAttributes &GA);
	void reInsertGraph(MultilevelGraph &MLG);
	void reInsertAll(std::vector<MultilevelGraph *> components);
	void copyNodeTo(node v, MultilevelGraph &MLG, std::map<node, node> &tempNodeAssociations, bool associate, int index = -1);
	void copyEdgeTo(edge e, MultilevelGraph &MLG, std::map<node, node> &tempNodeAssociations, bool associate, int index = -1);
	void writeGML(ostream &os);
	void writeGML(const String &fileName);

	// the original graph will be cleared to save Memory
	std::vector<MultilevelGraph *> splitIntoComponents();

	bool postMerge(NodeMerge * NM, node merged);
	//\a merged is the node now represented by \a theNode
	bool changeNode(NodeMerge * NM, node theNode, float newRadius, node merged);
	bool changeEdge(NodeMerge * NM, edge theEdge, float newWeight, node newSource, node newTarget);
	bool deleteEdge(NodeMerge * NM, edge theEdge);
	std::vector<edge> moveEdgesToParent(NodeMerge * NM, node theNode, node parent, bool deleteDoubleEndges, int adjustEdgeLengths);
	NodeMerge * getLastMerge();
	node undoLastMerge();

	void updateReverseIndizes();
	//sets the merge weights back to initial values
	void updateMergeWeights();
};

} // namespace ogdf

#endif
