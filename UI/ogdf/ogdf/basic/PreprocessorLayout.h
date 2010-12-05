/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Preprocessor Layout simplifies Graphs for use in other Algorithms
 *
 * \author Gereon Bartel
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

#include <ogdf/module/LayoutModule.h>

#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_PREPROCESSOR_LAYOUT_H
#define OGDF_PREPROCESSOR_LAYOUT_H

namespace ogdf {

/** \brief Deleted Edges are stored in EdgeData
 *
 * EdgeData stores the deleted edges to allow restauration of the original
 * graph after the layout has been computed.
 */
struct EdgeData
{
	EdgeData(unsigned int edgeIndex, unsigned int sourceIndex, unsigned int targetIndex, double weight)
		:edgeIndex(edgeIndex), sourceIndex(sourceIndex), targetIndex(targetIndex), weight(weight)
	{};

	unsigned int edgeIndex;
	unsigned int sourceIndex;
	unsigned int targetIndex;
	double weight;
};

/** \brief The PreprocessorLayout removes duplicate edges and selfloops
 *
 * To draw a graph using the Modular Multilevel Micer or other layouts the
 * graph must fit the following criteria: No edge may be a duplicate of another
 * and no edge should end at the same node it starts.
 * Edges that conflict with these rules are deleted in the PreprocessorLayout.
 * A secondary layout is then called that can work on the graph in correct form.
 * After the layout has been computed, the edges are inserted back into the
 * graph, as they may have been relevant for the user.
 */
class OGDF_EXPORT PreprocessorLayout : public LayoutModule
{
private:
	LayoutModule * m_secondaryLayout;
	std::vector<EdgeData> m_deletedEdges;
	bool m_randomize;

	void call(Graph &G, MultilevelGraph &MLG);

public:

	//! Constructor
	PreprocessorLayout();

	//! Destructor
	~PreprocessorLayout();


	//! calculates a drawing for the Graph MLG
	void call(MultilevelGraph &MLG);

	//! calculates a drawing for the Graph GA
	void call(GraphAttributes &GA);

	//! sets the secondary layout
	void setLayoutModule(LayoutModule &layout);

	//! defines whether the positions of the node are randomized before the secondary layout call.
	void setRandomizePositions(bool on);
	
};

} // namespace ogdf

#endif
