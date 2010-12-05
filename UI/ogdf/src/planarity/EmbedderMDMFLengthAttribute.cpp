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

#include <ogdf/internal/planarity/EmbedderMDMFLengthAttribute.h>

namespace ogdf {

mdmf_la mdmf_la::operator=(const mdmf_la& x)
{
	this->d = x.d;
	this->l = x.l;
	return *this;
}

mdmf_la mdmf_la::operator=(const int& x)
{
	this->d = x;
	this->l = 0;
	return *this;
}

bool mdmf_la::operator==(const mdmf_la& x)
{
	return (this->d == x.d && this->l == x.l);
}

bool mdmf_la::operator!=(const mdmf_la& x)
{
	return !(*this == x);
}

bool mdmf_la::operator>(const mdmf_la& x)
{
	return (this->d > x.d || (this->d == x.d && this->l > x.l));
}

bool mdmf_la::operator<(const mdmf_la& x)
{
	return !(*this >= x);
}

bool mdmf_la::operator>=(const mdmf_la& x)
{
	return (*this == x) || (*this > x);
}

bool mdmf_la::operator<=(const mdmf_la& x)
{
	return (*this == x) || (*this < x);
}

mdmf_la mdmf_la::operator+(const mdmf_la& x)
{
	return mdmf_la(this->d + x.d, this->l + x.l);
}

mdmf_la mdmf_la::operator-(const mdmf_la& x)
{
	return mdmf_la(this->d - x.d, this->l - x.l);
}

mdmf_la mdmf_la::operator+=(const mdmf_la& x)
{
	this->d += x.d;
	this->l += x.l;
	return *this;
}

mdmf_la mdmf_la::operator-=(const mdmf_la& x)
{
	this->d -= x.d;
	this->l -= x.l;
	return *this;
}

bool operator==(const mdmf_la& x, const mdmf_la& y)
{
	return (x.d == y.d && x.l == y.l);
}

bool operator!=(const mdmf_la& x, const mdmf_la& y)
{
	return !(x == y);
}

bool operator>(const mdmf_la& x, const mdmf_la& y)
{
	return (x.d > y.d || (x.d == y.d && x.l > y.l));
}

bool operator<(const mdmf_la& x, const mdmf_la& y)
{
	return y > x;
}

bool operator>=(const mdmf_la& x, const mdmf_la& y)
{
	return (x == y) || (x > y);
}

bool operator<=(const mdmf_la& x, const mdmf_la& y)
{
	return (x == y) || (x < y);
}

mdmf_la operator+(const mdmf_la& x, const mdmf_la& y)
{
	return mdmf_la(x.d + y.d, x.l + y.l);
}

mdmf_la operator-(const mdmf_la& x, const mdmf_la& y)
{
	return mdmf_la(x.d - y.d, x.l - y.l);
}

mdmf_la operator+=(const mdmf_la& x, const mdmf_la& y)
{
	return mdmf_la(x.d + y.d, x.l + y.l);
}

mdmf_la operator-=(const mdmf_la& x, const mdmf_la& y)
{
	return mdmf_la(x.d - y.d, x.l - y.l);
}

ostream& operator<<(ostream& s, const mdmf_la& x)
{
	s << x.d << ", " << x.l;
	return s;
}

} // end namespace ogdf
