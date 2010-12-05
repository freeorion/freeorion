/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class PlanarityGrid which implements an
 *        energy function where the energy of a layout depends
 *        on the number of crossings.
 * 
 * Uses the UniformGris Class to compute the number of crossings.
 *
 * \author Rene Weiskircher
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

#ifndef OGDF_PLANARITYGRID_H
#define OGDF_PLANARITYGRID_H


#include <ogdf/internal/energybased/EnergyFunction.h>
#include <ogdf/internal/energybased/UniformGrid.h>


namespace ogdf {


class PlanarityGrid: public EnergyFunction {
public: 
	//initializes data structures to speed up later computations
	PlanarityGrid(GraphAttributes &AG);
	~PlanarityGrid();
	// computes energy of initial layout and stores it in m_energy
	void computeEnergy();
private:
	// computes energy of candidate
	void compCandEnergy();
	// changes internal data if candidate is taken
	void internalCandidateTaken();
#ifdef OGDF_DEBUG
		virtual void printInternalData() const;
#endif
	const GraphAttributes &m_layout; //The current layout
	UniformGrid *m_currentGrid; //stores grid for current layout
	UniformGrid *m_candidateGrid; //stores grid for candidate layout
}; // class Planarity


}// namespace ogdf

#endif
