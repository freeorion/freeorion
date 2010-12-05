/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Mathematical Helpers
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
 
#ifndef OGDF_MATH_H
#define OGDF_MATH_H

namespace ogdf {

#define DOUBLE_EPS 0.000001

class OGDF_EXPORT Math {
public:
	inline static int binomial(int n, int k) {
		if(k>n/2) k = n-k;
		int r = n;
		for(int i = 2; i<=k; ++i)
			r = (r * (n+1-i))/i;
		return r;
	}
	inline static double binomial(double n, double k) {
		if(k>n/2) k = n-k;
		double r = n;
		for(int i = 2; i<=k; ++i)
			r = (r * (n+1-i))/i;
		return r;
	}
	static int factorial(int n) {
		int r = 1;
		for(; n>1; --n) r *= n;
		return r;
	}

	static double factorial(double n) {
		double r = 1;
		for(; n>1; --n) r *= n;
		return r;
	}

	inline bool equald(double a, double b) {
		double d = a-b;
		return d < DOUBLE_EPS && d > -DOUBLE_EPS;
	}
};


}

#endif // OGDF_MATH_H
