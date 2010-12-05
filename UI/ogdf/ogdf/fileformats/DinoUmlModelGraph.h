/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Contains the class DinoUmlModelGraph which represents the
 * complete UML Model in a graph like data structure.
 * 
 * \author Dino Ahr
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

#ifndef OGDF_DINO_UML_MODEL_GRAPH_H
#define OGDF_DINO_UML_MODEL_GRAPH_H

#include <ogdf/basic/NodeArray.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/String.h>

namespace ogdf {

	//---------------------------------------------------------
	// D i n o U m l M o d e l G r a p h
	//---------------------------------------------------------
	/** This class represents the complete UML Model in a graph-
	 *  like data structure.
	 */
	class OGDF_EXPORT DinoUmlModelGraph : public Graph {

	private:

		/** The name of the model. */
		String m_modelName;

		/** The label of the contained nodes. */
		NodeArray<String> m_nodeLabel;

		/** The types of the contained edges. 
		 *  Types are association or generalization. 
		 */
		EdgeArray<Graph::EdgeType> m_eType; 
		
		/** The types of the contained nodes. 
		 *  Types are vertex, dummy, generalizationMerger 
		 */
		NodeArray<Graph::NodeType> m_vType; 

	public:

		/** Constructor. */
		DinoUmlModelGraph();

		/** Destructor. */
		~DinoUmlModelGraph();

		/** Sets the name of the model. */
		void setModelName(const String &name) { m_modelName = name; }

		/** Returns a const reference to the label of the given node. */
		const String &getNodeLabel(node v) const { return m_nodeLabel[v]; }

		/** Returns a reference to the label of the given node. */
		String &labelNode(node v) { return m_nodeLabel[v]; }

		/** Returns a const reference to the type of the given edge. */
		const Graph::EdgeType &type(edge e) const { return m_eType[e]; }
	
		/** Returns a reference to the type of the given edge. */
		Graph::EdgeType &type(edge e) { return m_eType[e]; }

		/** Returns a const reference to the type of the given node. */
		const Graph::NodeType &type(node v) const { return m_vType[v]; }
	
		/** Returns a reference to the type of the given node. */
		Graph::NodeType &type(node v) { return m_vType[v]; }

	}; // class DinoUmlModelGraph

	/** Output operator for DinoUmlModelGraph. */
	ostream &operator<<(ostream &os, const DinoUmlModelGraph &modelGraph);

} // end namespace ogdf

#endif
