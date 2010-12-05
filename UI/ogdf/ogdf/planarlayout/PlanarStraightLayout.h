/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class PlanarStraightLayout which represents
 *        a planar straight-line drawing algorithm.
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


#ifndef OGDF_PLANAR_STRAIGHT_LAYOUT_H
#define OGDF_PLANAR_STRAIGHT_LAYOUT_H


#include <ogdf/module/GridLayoutModule.h>
#include <ogdf/basic/ModuleOption.h>
#include <ogdf/module/AugmentationModule.h>
#include <ogdf/module/ShellingOrderModule.h>


namespace ogdf {

	class OGDF_EXPORT ShellingOrder;

/**
 * \brief Implementation of the Planar-Straight layout algorithm.
 *
 * The class PlanarStraightLayout implements a straight-line drawing algorithm
 * for planar graphs. It draws a planar graph on a grid of size at
 * most (2<i>n</i>-4) * (<i>n</i>-2) without edge crossings.
 *
 * The algorithm runs in several phases. In the first phase,
 * the graph is augmented by adding edges to achieve a certain connectivity
 * (e.g., biconnected or triconnected). Then, a shelling order of the
 * the resulting graph is computed. Both phases are implemented by using
 * module options, so you can replace them with different implementations.
 * However, you have to make sure, that the connectivity achieved by the
 * augmentation module is sufficient for the shelling order module (or the
 * input graph already has the required connectivity).
 *
 * The implementation used in PlanarStraightLayout is an improved version of
 * an algorithm presented in:
 *
 * Goos Kant: <i>Drawing Planar Graphs Using the Canonical Ordering</i>.
 * Algorithmica 16(1), pp. 4-32, 1996.
 *
 * <H3>Precondition</H3>
 * The input graph needs to be planar and simple (i.e., no self-loops and no
 * multiple edges).
 *
 * <H3>Optional parameters</H3>
 *
 * <table>
 *   <tr>
 *     <th><i>Option</i><th><i>Type</i><th><i>Default</i><th><i>Description</i>
 *   </tr><tr>
 *     <td><i>sizeOptimization</i><td>bool<td>true
 *     <td>If this option is set to true, the algorithm tries to reduce the
 *     size of the resulting grid layout.
 *   </tr><tr>
 *     <td><i>baseRatio</i><td>double<td>0.33
 *     <td>This option specifies the maximal number of nodes placed on the base line.
 *     The algorithm tries to place as many nodes as possible on the base
 *     line, but takes at most max(2, \a baseRatio * size of external face) many.
 *   </tr>
 * </table>
 *
 * <H3>%Module options</H3>
 * Instances of type PlanarDrawLayout provide the following module options:
 *
 * <table>
 *   <tr>
 *     <th><i>Option</i><th><i>Type</i><th><i>Default</i><th><i>Description</i>
 *   </tr><tr>
 *     <td><i>augmenter</i><td>AugmentationModule<td>PlanarAugmentation
 *     <td>Augments the graph by adding edges to obtain a planar graph with
 *     a certain connectivity (e.g., biconnected or triconnected).
 *   </tr><tr>
 *     <td><i>shellingOrder</i><td>ShellingOrderModule<td>BiconnectedShellingOrder
 *     <td>The algorithm to compute a shelling order. The connectivity assured
 *     by the planar augmentation module has to be sufficient for the shelling
 *     order module!
 *   </tr>
 * </table>
 *
 * <H3>Running Time</H3>
 * The computation of the layout takes time O(<I>n</I>), where <I>n</I> is
 * the number of nodes of the input graph.
 */
class OGDF_EXPORT PlanarStraightLayout : public PlanarGridLayoutModule
{
public:
	//! Constructs an instance of the Planar-Straight layout algorithm.
	PlanarStraightLayout();

	~PlanarStraightLayout() { }


	/**
	 *  @name Optional parameters
	 *  @{
	 */

	/**
	 * \brief Returns the current setting of option sizeOptimization.
	 * 
	 * If this option is set to true, the algorithm tries to reduce the
	 * size of the resulting grid layout.
	 */
	bool sizeOptimization() const { return m_sizeOptimization; }

	//! Sets the option sizeOptimization to \a opt.
	void sizeOptimization(bool opt) { m_sizeOptimization = opt; }

	/**
	 * \brief Returns the current setting of option baseRatio.
	 * 
	 * This option specifies the maximal number of nodes placed on the base line.
	 * The algorithm tries to place as many nodes as possible on the base
	 * line, but takes at most max(2, \a baseRatio * size of external face) many.
	 */
	double baseRatio() const { return m_baseRatio; }

	//! Sets the option baseRatio to \a ratio.
	void baseRatio(double ratio) { m_baseRatio = ratio; }


	/** @}
	 *  @name Module options
	 *  @{
	 */

	/**
	 * \brief Sets the augmentation module.
	 *
	 * The augmentation module needs to make sure that the graph gets the
	 * connectivity required for calling the shelling order module.
	 */
	void setAugmenter(AugmentationModule *pAugmenter) {
		m_augmenter.set(pAugmenter);
	}
	
	//! Sets the shelling order module.
	void setShellingOrder(ShellingOrderModule *pOrder){
		m_computeOrder.set(pOrder);
	}

	//! @}

private:
	bool   m_sizeOptimization; //!< The option for size optimization.
	double m_baseRatio; //!< The option for specifying the base ratio.

	ModuleOption<AugmentationModule> m_augmenter;		//!< The augmentation module.
	ModuleOption<ShellingOrderModule> m_computeOrder;	//!< The shelling order module.

	void doCall(
		const Graph &G,
		adjEntry adjExternal,
		GridLayout &gridLayout,
		IPoint &boundingBox,
		bool fixEmbedding);

	void computeCoordinates(const Graph &G,
		ShellingOrder &lmc,
		NodeArray<int> &x,
		NodeArray<int> &y);

};


} // end namespace ogdf


#endif
