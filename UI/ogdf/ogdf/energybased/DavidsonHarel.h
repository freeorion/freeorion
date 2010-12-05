/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares class DavidsonHarel which implements the
 * Davidson-Harel approach for drawing graphs.
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

#ifndef OGDF_DAVIDSON_HAREL_H
#define OGDF_DAVIDSON_HAREL_H


#include <ogdf/internal/energybased/EnergyFunction.h>


namespace ogdf {


//! The Davidson-Harel approach for drawing graphs.
class OGDF_EXPORT DavidsonHarel
{
public:

	//! Creates an instance of Davidsen-Harel base class.
	DavidsonHarel();

	~DavidsonHarel() {};

	//! Sets the start temperature to \a startTemp.
	void setStartTemperature(int startTemp);

    //! Sets the number of iterations for each temperature step to \a steps.
	void setNumberOfIterations(int steps);

	//! Adds an energy function \a F with a certain weight.
	void addEnergyFunction(EnergyFunction *F, double weight);

	//! Returns a list of the names of the energy functions.
	List<String> returnEnergyFunctionNames();

	//! Returns a list of the weights of the energy functions.
	List<double> returnEnergyFunctionWeights();

	//! Calls the Davidson-Harel method for graph \a GA.
	void call(GraphAttributes &GA);

private:
	//! The default starting temperature.
	const static int m_defaultTemp;
	//! the constant PI
	const static double m_PI;       
	//! The default starting radius.
	const static double m_defaultRadius;
	//! Per default, the number of iterations per temperature are set as a constant multiple of the number of vertices.
	const static int m_iterationMultiplier;
	//! The fraction by which the temperature is lowered after a temperature step is finished.
	const static double m_coolingFactor;
	//! the constant by which the radius of the circle around each vertex is shrunk when the temperature is lowered
	const static double m_shrinkFactor;

	int m_temperature;          //!< The temperature during the annealing process.
	double m_shrinkingFactor;   //!< The factor for radius.
	double m_diskRadius;        //!< The radius of the disk around the old position of a vertex where the new position will be.
	double m_energy;            //!< The current energy of the system.
	int m_numberOfIterations;   //!< The number of iterations per temperature step.

	List<EnergyFunction*> m_energyFunctions; //!< The list of the energy functions.
	List<double> m_weightsOfEnergyFunctions; //!< The list of the weights for the energy functions.

	List<node> m_nonIsolatedNodes; //!< The list of nodes with degree greater 0.

	//! Resets the parameters for subsequent runs.
	void initParameters(); 

	//! Randomly computes a node and a new position for that node.
	node computeCandidateLayout(const GraphAttributes &, DPoint &) const;

	//! Tests if new energy value satisfies annealing property (only better if m_fineTune).
	bool testEnergyValue(double newVal);

	//! Computes a random number between zero and one
	double randNum() const;

	//! Computes the first disk radius as the half the diamter of the enclosing rectangle.
	void computeFirstRadius(const GraphAttributes &AG);

	//! Computes the energy of the initial layout and stores it in \a m_energy.
	void computeInitialEnergy();

	//! Computes positions for the vertices of degree zero.
	void placeIsolatedNodes(GraphAttributes &AG) const;

	//! Fake assignment operator (dummy to avoid copying)
	DavidsonHarel& operator=(const DavidsonHarel &dh);
	//! Fake copy constructor (dummy to avoid copying)
	DavidsonHarel(const DavidsonHarel &) {};
};

} //end namespace
#endif
