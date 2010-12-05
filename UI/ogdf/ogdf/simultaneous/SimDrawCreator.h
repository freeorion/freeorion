/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Offers variety of possible SimDraw creations.
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

#ifndef OGDF_SIMDRAW_CREATOR_H
#define OGDF_SIMDRAW_CREATOR_H

#include <ogdf/simultaneous/SimDrawManipulatorModule.h>

namespace ogdf 
{
  //! Creates variety of possible SimDraw creations
  /**
   * This class is used for creating simdraw instances.
   * Possible features include reading a graph, randomly modifying 
   * or clearing the edgeSubgraph value and changing the subGraphBits.
   */

  class OGDF_EXPORT SimDrawCreator : public SimDrawManipulatorModule
    {

    public:
      //! constructor
      SimDrawCreator(SimDraw &SD) : SimDrawManipulatorModule(SD) {}

      //! returns SubGraphBits from edge e
      unsigned int &SubGraphBits(edge e) { return m_GA->subGraphBits(e); }

      //! returns SubGraphBits from edge e
      const unsigned int &SubGraphBits(edge e) const { return m_GA->subGraphBits(e); }

      //! reads a Graph
      void readGraph(const Graph &G) { *m_G = G; }
      
      //! randomly chose edgeSubGraph value for two graphs
      /**
       * Assigns random edgeSubGraph values to all edges to create
       * a SimDraw instance consisting of two basic graphs.
       * Each edge in m_G has a chance of \a doubleESGProbability (in Percent) 
       * to belong to two SubGraphs.
       * Otherwise it has equal chance to belong to either basic graph.
       */
      void randomESG2(int doubleESGProbability = 50);
      
      //! randomly chose edgeSubGraph value for three graphs
      /**
       * Assigns random edgeSubGraph values to all edges to create
       * a SimDraw instance consisting of three basic graphs.
       * Each edge in m_G has a chance of \a doubleESGProbabilit (in Percent) 
       * to belong to two basic graphs and a chance of \a tripleESGProbability
       * (in Percent) to belong to three basic graphs.
       */
      void randomESG3(int doubleESGProbability = 50, int tripleESGProbability = 25);

      //! randomly chose edgeSubGraph value for graphNumber graphs
      /**
       * Assigns random edgeSubGraph values to all edges to create
       * a SimDraw instance consisting of \a graphNumber basic graphs.
       * Each edge has an equal chance for each SubGraphBit - value.
       */
      void randomESG(int graphNumber);

      //! clears edgeSubGraph value
      /**
       * This method clears all SubGraph values from m_G.
       * After this function all edges belong to no basic graph.
       * CAUTION: All edges need to be reset their edgeSubGraph value
       * for maintaining consistency.
       */
      void clearESG();

      //! randomly creates a simdraw instance
      /**
       * This method creates a random graph with \a numberOfNodes nodes,
       * \a numberOfEdges edges. It is transfered into a simdraw instance with 
       * \a numberOfBasicGraphs basic graphs.
       *
       * randomSimpleGraph from graph_generators.h is used to
       * create a random graph. Furthermore randomESG is used on this graph
       * to generate \a numberOfBasicGraphs basic graphs.
       */
      void createRandom(int numberOfNodes, int numberOfEdges, int numberOfBasicGraphs);

    };

}

#endif
