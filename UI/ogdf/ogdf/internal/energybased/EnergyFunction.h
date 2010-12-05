/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares class EnergyFunction...
 * 
 * ...which specifies an interface for energy functions for 
 * the Davidson Harel graph drawing method. It is used in the
 * class DavidsonHarel.
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

#ifndef OGDF_ENERGY_FUNCTION_H
#define OGDF_ENERGY_FUNCTION_H


#include <ogdf/basic/GraphAttributes.h>


namespace ogdf {

//! The interface for energy functions for the Davidson Harel graph drawing method. 
/**
 * It is used in the class DavidsonHarel.
 */
class EnergyFunction {
public:
	//! Initializes data dtructures to speed up later computations
	EnergyFunction(const String &funcname, GraphAttributes &AG);
	virtual ~EnergyFunction() { }
	
	//! computes energy for the layout at the beginning of the optimization process
	virtual void computeEnergy()=0;
	//! sets m_testNode, m_testX and m_testY and computes the energy for the new configuration (vertex v moves to newPos)
	double computeCandidateEnergy(
		const node v,
		const DPoint &newPos);
	//! prints the name of the energy function
	String getName() const {return m_name;}
	//! Changes m_currentX and m_currentY by setting the position of m_testNode to m_testX and m_testY. Sets m_energy to m_candidateEnergy. Computes the energy of the layout stored in AG.
	void candidateTaken();
#ifdef OGDF_DEBUG
	//! prints status information for debugging
	void printStatus() const;
#endif
	double energy() const {return m_energy;}
protected:
	const Graph &m_G;//!< the graph that should be drawn
	const String m_name;//!< name of the energy function
	double m_candidateEnergy;//!< the energy of the layout if the candidate layout is chosen
	double m_energy;//!< energy of the current layout
	//! returns candidate position for the node to be moved
	DPoint testPos() {return m_testPos;}
	//! returns the current position of vertex v
	DPoint currentPos(const node v) const {return DPoint(m_AG.x(v),m_AG.y(v));}
	//! returns the vertex that is under consideration in the current step
	node testNode() const {return m_testNode;}
	//! changes the data of a specific energy function if the candidate was taken
	virtual void internalCandidateTaken() = 0;
	//! computes the energy if m_testNode changes position to m_testX and m_testY, sets the value of m_candidateEnergy.
	virtual void compCandEnergy()=0;
#ifdef OGDF_DEBUG
	virtual void printInternalData() const = 0;
#endif
private:
	//! the copy constructor is fake and can not be used.
	EnergyFunction(const EnergyFunction &e):m_G(e.m_G),m_name(e.m_name),m_AG(e.m_AG) {};
	//! the assignment operator is fake and can not be used.
	EnergyFunction& operator=(const EnergyFunction &e);
	GraphAttributes& m_AG;	//!< This stores the graph with its graphical attributes and the current positions for the vertices
	node m_testNode;//!< The node that changed position in the candidate
	DPoint m_testPos;//!< New candidate positions for m_testNode
};

}// end namespace
#endif
