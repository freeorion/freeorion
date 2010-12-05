/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Length attribute used in EmbedderMinDepthMaxFace.
 * It contains two components (d, l) and a linear order is defined by:
 * (d, l) > (d', l') iff d > d' or (d = d' and l > l')
 * 
 * \author Thorsten Kerkhof
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

#ifndef OGDF_EMBEDDER_MDMF_LENGTH_ATTRIBUTE_H
#define OGDF_EMBEDDER_MDMF_LENGTH_ATTRIBUTE_H

#include <ogdf/basic/basic.h>

namespace ogdf {

class mdmf_la
{
public:
	//constructors and destructor
	mdmf_la() { d = l = 0; }
	mdmf_la(const int& d, const int& l) : d(d), l(l) { }
	mdmf_la(const int& d) : d(d), l(0) { }
	mdmf_la(const mdmf_la& x) : d(x.d), l(x.l) { }
	~mdmf_la() { }

	mdmf_la operator=(const mdmf_la& x);
	mdmf_la operator=(const int& x);
	bool operator==(const mdmf_la& x);
	bool operator!=(const mdmf_la& x);
	bool operator>(const mdmf_la& x);
	bool operator<(const mdmf_la& x);
	bool operator>=(const mdmf_la& x);
	bool operator<=(const mdmf_la& x);
	mdmf_la operator+(const mdmf_la& x);
	mdmf_la operator-(const mdmf_la& x);
	mdmf_la operator+=(const mdmf_la& x);
	mdmf_la operator-=(const mdmf_la& x);

public:
	//the two components:
	int d;
	int l;
};

bool operator==(const mdmf_la& x, const mdmf_la& y);
bool operator!=(const mdmf_la& x, const mdmf_la& y);
bool operator>(const mdmf_la& x, const mdmf_la& y);
bool operator<(const mdmf_la& x, const mdmf_la& y);
bool operator>=(const mdmf_la& x, const mdmf_la& y);
bool operator<=(const mdmf_la& x, const mdmf_la& y);
mdmf_la operator+(const mdmf_la& x, const mdmf_la& y);
mdmf_la operator-(const mdmf_la& x, const mdmf_la& y);
mdmf_la operator+=(const mdmf_la& x, const mdmf_la& y);
mdmf_la operator-=(const mdmf_la& x, const mdmf_la& y);
ostream& operator<<(ostream& s, const mdmf_la& x);

} // end namespace ogdf

#endif
