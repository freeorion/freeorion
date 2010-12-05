/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of linear time layout algorithm for free
 *        trees (class RadialTreeLayout).
 *
 * Based on chapter 3.1.1 Radial Drawings of Graph Drawing by
 * Di Battista, Eades, Tamassia, Tollis.
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

#ifndef OGDF_RADIAL_TREE_LAYOUT_H
#define OGDF_RADIAL_TREE_LAYOUT_H

#include <ogdf/module/LayoutModule.h>
#include <ogdf/basic/SList.h>


namespace ogdf {


//! The radial tree layout algorithm.
/**
 * <H3>Optional parameters</H3>
 * Radial tree layout provides the following optional parameters.
 *
 * <table>
 *   <tr>
 *     <th><i>Option</i><th><i>Type</i><th><i>Default</i><th><i>Description</i>
 *   </tr><tr>
 *     <td><i>levelDistance</i><td>double<td>50.0
 *     <td>The minimal vertical distance required between levels.
 *   </tr><tr>
 *     <td><i>connectedComponentDistance</i><td>double<td>50.0
 *     <td>The minimal horizontal distance required between trees in the forest.
 *   </tr><tr>
 *     <td><i>rootSelection</i><td> #RootSelectionType <td> #rootIsCenter
 *     <td>Specifies how to select the root of the tree.
 *   </tr>
 * </table>
*/
class OGDF_EXPORT RadialTreeLayout : public LayoutModule
{
public:
	//! Selection strategies for root of the tree.
	enum RootSelectionType {
		rootIsSource, //!< Select a source in the graph.
		rootIsSink,   //!< Select a sink in the graph.
		rootIsCenter  //!< Select the center of the tree.
	};

private:
    double m_levelDistance;          //!< The minimal distance between levels.
	double m_connectedComponentDistance; //!< The minimal distance between trees.

	RootSelectionType m_selectRoot;  //!< Specifies how to determine the root.

	node            m_root;          //!< The root of the tree.

	int             m_numLevels;     //!< The number of levels (root is on level 0).
	NodeArray<int>  m_level;         //!< The level of a node.
	NodeArray<node> m_parent;        //!< The parent of a node (0 if root).
	NodeArray<double> m_leaves;      //!< The weighted number of leaves in subtree.
	Array<SListPure<node> > m_nodes; //!< The nodes at a level.

	NodeArray<double> m_angle;       //!< The angle of node center (for placement).
	NodeArray<double> m_wedge;       //!< The wedge reserved for subtree.

	NodeArray<double> m_diameter;    //!< The diameter of a circle bounding a node.
	Array<double>     m_width;       //!< The width of a circle.

	Array<double>   m_radius;        //!< The width of a level.
	double          m_outerRadius;   //!< The radius of circle bounding the drawing.
	
	struct Group
	{
		RadialTreeLayout *m_data;

		bool            m_leafGroup;
		SListPure<node> m_nodes;
		double          m_sumD;
		double          m_sumW;
		double          m_leftAdd;
		double          m_rightAdd;

		Group(RadialTreeLayout *data, node v) {
			m_data = data;
			m_leafGroup = (v->degree() == 1);
			m_nodes.pushBack(v);
			m_sumD = m_data->diameter()[v] + m_data->levelDistance();
			m_sumW = m_data->leaves()[v];
			m_leftAdd = m_rightAdd = 0.0;
		}

		bool isSameType(node v) const {
			return (m_leafGroup == (v->degree() == 1));
		}

		void append(node v) {
			m_nodes.pushBack(v);
			m_sumD += m_data->diameter()[v] + m_data->levelDistance();
			m_sumW += m_data->leaves()[v];
		}

		double add() const { return m_leftAdd + m_rightAdd; }
		node leftVertex () const { return m_nodes.front(); }
		node rightVertex() const { return m_nodes.back (); }
	};

	class Grouping : public List<Group>
	{
	public:
		void computeAdd(double &D, double &W);
	};

	NodeArray<Grouping> m_grouping;

public:
	//! Creates an instance of radial tree layout and sets options to default values.
	RadialTreeLayout();

	//! Copy constructor.
	RadialTreeLayout(const RadialTreeLayout &tl);

	// destructor
	~RadialTreeLayout();

	//! Assignment operator.
	RadialTreeLayout &operator=(const RadialTreeLayout &tl);

	//! Calls the algorithm for graph attributes \a GA.
	/**
	 * The algorithm preserve the order of children which is given by
	 * the adjacency lists.
	 *
	 * \pre The graph is a tree.
	 * @param GA represents the input graph and is assigned the computed layout.
	 */
	void call(GraphAttributes &GA);


	// option that determines the minimal vertical distance 
	// required between levels

	//! Returns the option <i>levelDistance</i>.
	double levelDistance() const { return m_levelDistance; }

	//! Sets the option <i>levelDistance</i> to \a x.
	void levelDistance(double x) { m_levelDistance = x; }

	// option that determines the minimal horizontal distance 
	// required between trees in the forest

	//! Returns the option <i>connectedComponentDistance</i>.
	double connectedComponentDistance() const { return m_connectedComponentDistance; }

	//! Sets the option <i>connectedComponentDistance</i> to \a x.
	void connectedComponentDistance(double x) { m_connectedComponentDistance = x; }

	// option that determines if the root is on the top or on the bottom

	//! Returns the option <i>rootSelection</i>.
	RootSelectionType rootSelection() const { return m_selectRoot; }

	//! Sets the option <i>rootSelection</i> to \a sel.
	void rootSelection(RootSelectionType sel) { m_selectRoot = sel; }

	const NodeArray<double> &diameter() const { return m_diameter; }
	const NodeArray<double> &leaves()   const { return m_leaves;   }

private:
	void FindRoot(const Graph &G);
	void ComputeLevels(const Graph &G);
	void ComputeDiameters(GraphAttributes &AG);
	void ComputeAngles(const Graph &G);
	void ComputeCoordinates(GraphAttributes &AG);
	void ComputeGrouping(int i);

	OGDF_NEW_DELETE
};

} // end namespace ogdf

#endif

