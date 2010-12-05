/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares ClusterGraphCopyAttributes, which manages access 
 *  on copy of an attributed clustered graph.
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

#ifndef OGDF_A_CLUSTER_GRAPH_COPY_H
#define OGDF_A_CLUSTER_GRAPH_COPY_H



#include <ogdf/layered/ExtendedNestingGraph.h>
#include <ogdf/cluster/ClusterGraphAttributes.h>


namespace ogdf {

/** 
 * \brief Manages access on copy of an attributed clustered graph
 */
class OGDF_EXPORT ClusterGraphCopyAttributes {

	const ExtendedNestingGraph *m_pH;
	ClusterGraphAttributes     *m_pACG;
	NodeArray<double> m_x, m_y;

public:
	//! Initializes instance of class ClusterGraphCopyAttributes.
	ClusterGraphCopyAttributes(
		const ExtendedNestingGraph &H,
		ClusterGraphAttributes &ACG) :
		m_pH(&H), m_pACG(&ACG), m_x(H,0), m_y(H,0) { }

	~ClusterGraphCopyAttributes() { }

	//! Returns corresponding ClusterGraphAttributes.
	const ClusterGraphAttributes &getClusterGraphAttributes() const { return *m_pACG; }

	//! Returns width of node v.
	double getWidth(node v) const {
		node vOrig = m_pH->origNode(v);
		return (vOrig == 0) ? 0.0 : m_pACG->width(vOrig);
	}

	//! Returns height of node v.
	double getHeight(node v) const {
		node vOrig = m_pH->origNode(v);
		return (vOrig == 0) ? 0.0 : m_pACG->height(vOrig);
	}

	//! Returns reference to x-coord. of node v.
	const double &x(node v) const {
		return m_x[v];
	}

	//! Returns reference to x-coord. of node v.
	double &x(node v) {
		return m_x[v];
	}

	//! Returns reference to y-coord. of node v.
	const double &y(node v) const {
		return m_y[v];
	}

	//! Returns reference to y-coord. of node v.
	double &y(node v) {
		return m_y[v];
	}

	//! Returns coordinate of upper cluster boundary of original cluster \a cOrig.
	double top(cluster cOrig) const {
		return m_pACG->clusterYPos(cOrig);
	}
	//! Returns coordinate of lower cluster boundary of original cluster \a cOrig.
	double bottom(cluster cOrig) const {
		return m_pACG->clusterYPos(cOrig) + m_pACG->clusterHeight(cOrig);
	}

	//! Sets the position of the cluster rectangle for original cluster \a cOrig.
	void setClusterRect(
		cluster cOrig,
		double left,
		double right,
		double top,
		double bottom)
	{
		m_pACG->clusterXPos  (cOrig) = left;
		m_pACG->clusterYPos  (cOrig) = top;
		m_pACG->clusterWidth (cOrig) = right-left;
		m_pACG->clusterHeight(cOrig) = bottom-top;
	}

	void setClusterLeftRight(
		cluster cOrig,
		double left,
		double right)
	{
		m_pACG->clusterXPos  (cOrig) = left;
		m_pACG->clusterWidth (cOrig) = right-left;
	}

	void setClusterTopBottom(
		cluster cOrig,
		double top,
		double bottom)
	{
		m_pACG->clusterYPos  (cOrig) = top;
		m_pACG->clusterHeight(cOrig) = bottom-top;
	}

	//! Sets attributes for the original graph in attributed graph.
	void transform();
};


} // end namespace ogdf


#endif
