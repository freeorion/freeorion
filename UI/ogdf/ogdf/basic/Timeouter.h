/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declares base class for modules with timeout functionality.
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

#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_TIMEOUTER_H
#define OGDF_TIMEOUTER_H


#include <ogdf/basic/basic.h>


namespace ogdf {

//! class for timeout funtionality
/** Holds a double value of the timeout time (in seconds).
 * Set the value to some negative value (e.g. -1) to turn the timeout
 * off. Note that 0 seconds is a perfectly feasible timeout value!
 */
class OGDF_EXPORT Timeouter
{
public:
	//! timeout is turned of by default
	Timeouter() : m_timeLimit(-1) { }
	
	//! timeout is set to the given value (seconds)
	Timeouter(double t) : m_timeLimit(t) { }
	
	//! timeout is turned off (false) or on (true) (with 0 second)
	Timeouter(bool t) : m_timeLimit(t ? 0 : -1) { }

	Timeouter(const Timeouter &t) : m_timeLimit(t.m_timeLimit) { } 
	
	~Timeouter() { }
	

	//! sets the time limit for the call (in seconds); <0 means no limit.
	void timeLimit(double t) {
		m_timeLimit = t;
	}
	
	//! shorthand to turn timelimit off or on (with 0 seconds)
	void timeLimit(bool t) {
		m_timeLimit = t ? 0 : -1;
	}
	
	//! returns the current time limit for the call
	double timeLimit() const {
		return m_timeLimit;
	}

	//! returns whether any time limit is set or not
	bool isTimeLimit() const {
		return m_timeLimit >= 0;
	}

protected:
	double m_timeLimit; //!< Time limit for module calls (< 0 means no limit).
};


} // end namespace ogdf


#endif
