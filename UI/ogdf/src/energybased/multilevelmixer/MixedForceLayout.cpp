/*
 * $Revision: 2027 $
 *
 * last checkin:
 *   $Author: gutwenger $
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $
 ***************************************************************/

/** \file
 * \brief Uses Fruchtermann Rheingold and Fast Multipole Embedder for faster and better FR results.
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

#include <ogdf/energybased/multilevelmixer/MixedForceLayout.h>
#include <ogdf/energybased/SpringEmbedderFR.h>
#include <ogdf/energybased/FastMultipoleEmbedder.h>

namespace ogdf {

MixedForceLayout::MixedForceLayout()
{
	SpringEmbedderFR * FR = new SpringEmbedderFR();
	FR->scaling(SpringEmbedderFR::scInput);
	m_FR = FR;

	FastMultipoleEmbedder * FME = new FastMultipoleEmbedder();
	FME->setNumIterations(1000);
	FME->setRandomize(false);
	FME->setNumberOfThreads(2);

	m_FME = FME;
}


void MixedForceLayout::call(GraphAttributes &GA)
{
	m_FME->call(GA);
	m_FR->call(GA);
}


void MixedForceLayout::call(MultilevelGraph &MLG)
{
	m_FME->call(MLG);
	m_FR->call(MLG);
}

} // namespace ogdf
