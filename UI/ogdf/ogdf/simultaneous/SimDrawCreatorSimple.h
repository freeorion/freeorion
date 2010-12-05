/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Offers simple SimDraw creations.
 * 
 * \author Michael Schulz and Daniel Lueckerath
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

#ifndef OGDF_SIMDRAW_CREATOR_SIMPLE_H
#define OGDF_SIMDRAW_CREATOR_SIMPLE_H

#include <ogdf/simultaneous/SimDrawCreator.h>

namespace ogdf 
{
  //! Offers predefined SimDraw creations.
  /**
   * This class offers some predefined SimDraw creations, such as 
   * an instance of two outerplanar graphs from Brass et al. (WADS'03)
   * or an instance of a path and a planar graph from Erten and Kobourov
   * (GD'04).
   */

  class OGDF_EXPORT SimDrawCreatorSimple : public SimDrawCreator
    {

    public:
      //! constructor
      SimDrawCreatorSimple(SimDraw &SD) : SimDrawCreator(SD) {}

      //! creates pair-of-tree instance from Geyer, Kaufmann, Vrto (GD'05)
      void createTrees_GKV05(int n);

      //! creates instance of a path and a planar graph from Erten and Kobourov (GD'04)
      void createPathPlanar_EK04();
      
      //! creates K5 instance from Erten and Kobourov (GD'04)
      void createK5_EK04();

      //!creates K5 instance from Gassner et al. (WG'06)
      void createK5_GJPSS06();

      //!creates instance of two outerplanar graphs from Brass et al. (WADS'03)
      void createOuterplanar_BCDEEIKLM03();

      //!creates instance from Kratochvil (GD'98)
      void createKrat98(int N, int nodeNumber);

      //! creates instance with numberofBasic*2 outer, 
      //! numberOfParallels*numberOfBasic inner Nodes and one Root.
      void createWheel(int numberOfParallels, int numberOfbasic);

      //! creates simultaneously planar simultaneous graph with n+1 basic graphs.
      void createExpo(int n);
    };

}

#endif
