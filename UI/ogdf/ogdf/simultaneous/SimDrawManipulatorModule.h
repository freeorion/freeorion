/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Module for simdraw manipulator classes
 * 
 * \author Michael Schulz
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

#ifndef OGDF_SIMDRAW_MANIPULATOR_MODULE_H
#define OGDF_SIMDRAW_MANIPULATOR_MODULE_H

#include<ogdf/simultaneous/SimDraw.h>

namespace ogdf 
{
  //! Interface for simdraw manipulators
  /** 
   *  To avoid class SimDraw to become too large, several functions
   *  have been outsourced. These are systematically
   *  grouped in creation methods (SimDrawCreator), algorithm calls
   *  (SimDrawCaller) and coloring methods (SimDrawColorizer).
   *
   *  A manipulator instance always needs a SimDraw instance (base instance) 
   *  to work on. The base instance is linked by pointers,
   *  thus a change within the base instance after initializing does
   *  not cause trouble:
   *  \code
   *  SimDraw SD;
   *  SimDrawCreatorSimple SDCr(SD);
   *  SimDrawColorizer SDCo(SD);
   *  SDCr.createTrees_GKV05(4);
   *  SimDrawCaller SDCa(SD);
   *  SDCa.callUMLPlanarizationLayout();
   *  SDCo.addColor();
   *  \endcode
   */
  class OGDF_EXPORT SimDrawManipulatorModule
    {

    protected:
      //! pointer to current simdraw instance
      SimDraw *m_SD;

      //! pointer to current graph
      Graph *m_G;

      //! pointer to current graphattributes
      GraphAttributes *m_GA;

    public:
      //! default constructor
      /** creates its own simdraw instance
       */
      SimDrawManipulatorModule();
      
      //! constructor
      SimDrawManipulatorModule(SimDraw &SD) { init(SD); }

      //! initializing base instance
      void init(SimDraw &SD);

      //! returns base instance
      const SimDraw &constSimDraw() const { return *m_SD; }
    };

} // end namespace ogdf

#endif
