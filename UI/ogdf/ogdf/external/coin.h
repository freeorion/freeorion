/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Handles connection to the COIN library, by offering 
 * helper classes.
 * 
 * If you use Coin, you need to include this file. Please follow
 * the example of CoinOptimalCrossingMinimizer for the correct use
 * of the USE_COIN precompiler flag.
 * 
 * \todo Currently, there is only a single implementation of the
 * CoinCallback-class declared herein (necc. for userdefined cuts).
 * This implementation is CPLEX specific.
 * 
 * \author Markus Chimani
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

#ifndef OGDF_COINY_H
#define OGDF_COINY_H

#include <ogdf/basic/basic.h>


#ifndef USE_COIN

	#define THROW_NO_COIN_EXCEPTION OGDF_THROW_PARAM(LibraryNotSupportedException, lnscCoin)
	
	namespace ogdf {		
		class CoinCallbacks {};
	}

#else // USE_COIN

	#define OGDF_THROW_NO_CALLBACK_EXCEPTION 

	#include <OsiSolverInterface.hpp>
	#include <CoinPackedVector.hpp>
	
	namespace ogdf {
		
		class OGDF_EXPORT CoinCallbacks {
			friend class OGDF_EXPORT CoinManager;
		public:
			enum CallbackType { CT_Cut = 1, CT_Heuristic = 2, CT_Incumbent = 4, CT_Branch  = 8 };
			enum CutReturn { CR_Error, CR_SolutionValid, CR_AddCuts, CR_DontAddCuts, CR_NoCutsFound };
			enum HeuristicReturn { HR_Error, HR_Ignore, HR_Update };
			enum IncumbentReturn { IR_Error, IR_Ignore, IR_Update };
//			enum BranchReturn { BR_Error, ... };
			virtual CutReturn cutCallback(const double /* objValue */, const double* /* fracSolution */, OsiCuts* /* addThese */) { OGDF_THROW_NO_CALLBACK_EXCEPTION; return CR_Error; };
			virtual HeuristicReturn heuristicCallback(double& /* objValue */, double* /* solution */) { OGDF_THROW_NO_CALLBACK_EXCEPTION; return HR_Error; };
			virtual IncumbentReturn incumbentCallback(const double /* objValue */, const double* /* solution */) { OGDF_THROW_NO_CALLBACK_EXCEPTION; return IR_Error; };
//			virtual BranchReturn branchCallback() { OGDF_THROW_NO_CALLBACK_EXCEPTION; return BR_Error; };
		private:
			bool registerCallbacks(OsiSolverInterface* _posi, int callbackTypes);
		};
		
		class OGDF_EXPORT CoinManager {
		public:
			static OsiSolverInterface* createCorrectOsiSolverInterface();
			static OsiSolverInterface* createCorrectOsiSolverInterface(CoinCallbacks* ccc, int callbackTypes) {
				OsiSolverInterface* posi = createCorrectOsiSolverInterface();
				if(ccc->registerCallbacks(posi, callbackTypes))
					return posi;
				delete posi;
				return NULL;
			}
			static void logging(OsiSolverInterface* osi, bool logMe);
		};
		
	}
	

#endif // USE_COIN

#endif // OGDF_COINY_H

