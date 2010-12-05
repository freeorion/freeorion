/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class PlanarityGrid
 * 
 * The PlanarityGrid energy function counts the number of
 * crossings. It contains two UniformGrids: One for the
 * current layout and one for the candidate layout.
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


#include <ogdf/internal/energybased/PlanarityGrid.h>

namespace ogdf {

	PlanarityGrid::~PlanarityGrid()
	{
		delete m_currentGrid;
		if(m_candidateGrid != NULL)
			delete m_candidateGrid;
	}
	
	// intialize m_currentLayout and m_candidateLayout
	PlanarityGrid::PlanarityGrid(GraphAttributes &AG):
	EnergyFunction("PlanarityGrid",AG), m_layout(AG)
	{
		m_currentGrid = new UniformGrid(AG);
		m_candidateGrid = NULL;
	}

	// computes energy of layout, stores it and sets the crossingMatrix
	void PlanarityGrid::computeEnergy()
	{
		m_energy = m_currentGrid->numberOfCrossings();
	}


	// computes the energy if the node returned by testNode() is moved
	// to position testPos().
	void PlanarityGrid::compCandEnergy()
	{
		if(m_candidateGrid != NULL)
			delete m_candidateGrid;
		node v = testNode();
		const DPoint& newPos = testPos();
		if(m_currentGrid->newGridNecessary(v,newPos))
			m_candidateGrid = new UniformGrid(m_layout,v,newPos);
		else
			m_candidateGrid = new UniformGrid(*m_currentGrid,v,newPos);
		m_candidateEnergy = m_candidateGrid->numberOfCrossings();
	}

	// this functions sets the currentGrid to the candidateGrid
	void PlanarityGrid::internalCandidateTaken() {
		delete m_currentGrid;
		m_currentGrid = m_candidateGrid;
		m_candidateGrid = NULL;
	}

#ifdef OGDF_DEBUG
void PlanarityGrid::printInternalData() const {
	cout << "\nCurrent grid: " << *m_currentGrid;
	cout << "\nCandidate grid: ";
	if(m_candidateGrid != NULL)
		cout << *m_candidateGrid;
	else cout << "empty.";
}
#endif
}
