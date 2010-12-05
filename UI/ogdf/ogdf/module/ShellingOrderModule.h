/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares the base class ShellingOrderModule for modules
 *        that compute a shelling order of a graph.
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

#ifndef OGDF_SHELLING_ORDER_MODULE_H
#define OGDF_SHELLING_ORDER_MODULE_H


#include <ogdf/planarlayout/ShellingOrder.h>


namespace ogdf {


/**
 * \brief Base class for modules that compute a shelling order of a graph.
 *
 */
class OGDF_EXPORT ShellingOrderModule
{
public:
	//! Computes a shelling order of an embedded graph G such that \a adj lies on the external face.
	/**
	 * @param G is the input graph; \a G must represent a combinatorial embedding.
	 * @param order is assigned the shelling order.
	 * @param adj is an adjacency entry on the external face; if \a adj is 0, a suitable
	 *        external face is chosen.
	 */
	void call(const Graph &G, ShellingOrder &order, adjEntry adj = 0);

	//! Computes a lefmost shelling order of an embedded graph G such that \a adj lies on the external face.
	/**
	 * @param G is the input graph; \a G must represent a combinatorial embedding.
	 * @param order is assigned the shelling order.
	 * @param adj is an adjacency entry on the external face; if \a adj is 0, a suitable
	 *        external face is chosen.
	 */
	void callLeftmost(const Graph &G, ShellingOrder &order, adjEntry adj = 0);
	
	//! Sets the option <i>base ratio</i> to \a x.
	void baseRatio(double x) {m_baseRatio = x;}

	//! Returns the current setting of the option <b>base ratio</b>.
	double baseRatio() const {return m_baseRatio;}
	
	virtual ~ShellingOrderModule() { }

protected:
	//! This pure virtual function does the actual computation.
	/**
	 * A derived class must implement this method. It is called with the embedded graph
	 * and an adjacency entry describing the external face, and must return the
	 * computed order in \a partition.
	 * @param G is the embedded input graph.
	 * @param adj is an adjacency entry on the external face.
	 * @param partition returns the coputed shelling order.
	 */
	virtual void doCall(const Graph &G,
		adjEntry adj,
		List<ShellingOrderSet> &partition) = 0;

	double m_baseRatio; //! The option <i>base ratio</i>.

};


} // end namespace ogdf


#endif
