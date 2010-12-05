/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Places Nodes at the Positio of the merge-partner
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

#include <ogdf/energybased/multilevelmixer/ZeroPlacer.h>

namespace ogdf {

ZeroPlacer::ZeroPlacer()
:m_randomRange(1.0)
{
}


void ZeroPlacer::setRandomRange(double range)
{
	m_randomRange = range;
}


void ZeroPlacer::placeOneLevel(MultilevelGraph &MLG)
{
	int level = MLG.getLevel();
	while (MLG.getLevel() == level && MLG.getLastMerge() != 0)
	{
		placeOneNode(MLG);
	}
}


void ZeroPlacer::placeOneNode(MultilevelGraph &MLG)
{
	NodeMerge * NM = MLG.getLastMerge();
	node parent = MLG.getNode(NM->m_changedNodes[0]);
	node merged = MLG.undoLastMerge();
	MLG.x(merged, MLG.x(parent) + ((m_randomOffset)?(float)randomDouble(-m_randomRange, m_randomRange):0.f));
	MLG.y(merged, MLG.y(parent) + ((m_randomOffset)?(float)randomDouble(-m_randomRange, m_randomRange):0.f));
}

} // namespace ogdf
