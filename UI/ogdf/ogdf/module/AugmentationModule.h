/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of interface for graph augmentation algorithms
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

#ifndef OGDF_AUGMENTATION_MODULE_H
#define OGDF_AUGMENTATION_MODULE_H



#include <ogdf/basic/Graph.h>

namespace ogdf {

/**
 * \brief The base class for graph augmentation algorithms.
 *
 * The class \a AugmentationModule is the base class for augmentation modules.
 * An augmentation module transforms an input graph \a G into an output
 * graph \a G' by adding edges, such that \a G' has a certain
 * property, e.g., biconnected.
 *
 * <H3>Implementation of Augmentation Algorithms</H3>
 * An implementation of an augmentation module must override
 * the protected method doCall(G,L), which gets as
 * input a graph reference \a G. It then adds the augmented edges
 * to \a G and returns the list of added edges in \a L.
 */

class OGDF_EXPORT AugmentationModule {
public:
	//! Initializes an augmentation module.
	AugmentationModule() { }
	// destruction
	virtual ~AugmentationModule() { }

	//! Calls the augmentation module for graph \a G.
	void call(Graph& G) {
		List<edge> L;
		call(G,L);
	}

	//! Calls the augmentation module for graph \a G.
	void operator()(Graph& G) { call(G); }

	/**
	 * \brief Calls the augmentation module for graph \a G.
	 *
	 * Returns the list of added edges in \a L.
	 */
	void call(Graph& G, List<edge> &L) {
		doCall(G,L);
		m_nAddedEdges = L.size();
	}
	
	/**
	 * \brief Calls the augmentation module for graph \a G.
	 *
	 * Returns the list of added edges in \a L.
	 */
	void operator()(Graph& G, List<edge> &L) { call(G,L); }
	
	//! Returns the number of added edges.
	int numberOfAddedEdges() const {
		return m_nAddedEdges;
	}

protected:
	/**
	 * \brief Implements the augmentation algorithm for graph \a G.
	 *
	 * Returns the list of added edges in \a L.
	 */
	virtual void doCall(Graph& G, List<edge> &L) = 0;

private:
	int m_nAddedEdges;

	OGDF_MALLOC_NEW_DELETE
};

} // end namespace ogdf

#endif
