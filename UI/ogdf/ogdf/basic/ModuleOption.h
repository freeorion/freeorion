/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of parameterized class for module options
 * 
 * \author Carsten Gutwenger
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

#ifndef OGDF_MODULE_OPTION_H
#define OGDF_MODULE_OPTION_H


#include <ogdf/basic/basic.h>


namespace ogdf {


/**
 * \brief The parameterized base class for module options.
 *
 * M is type (base class) of corresponding module. Notice that module
 * instances passed to set() must be allocated with new and will be
 * freed by ModuleOption.
 */
template<class M> class ModuleOption {

	M *m_pModule; //!< Pointer to the module.

public:
	//! Initializes a module option; the initial module is just a 0-pointer.
	ModuleOption() : m_pModule(0) { }

	// destruction
	~ModuleOption() { delete m_pModule; }

	//! Sets the module to \a pM.
	/**
	 * This function will also free the module currently stored by the option.
	 */
	void set(M *pM) {
		delete m_pModule;
		m_pModule = pM;
	}

	//! Returns true iff the option currently stores a module.
	bool valid() const { return m_pModule != 0; }

	//! Returns a reference to the stored module.
	/**
	 * It is required that the option currently stores a module, i.e.,
	 * valid() is true.
	 */
	M &get() { return *m_pModule; }
};


} // end namespace ogdf


#endif
