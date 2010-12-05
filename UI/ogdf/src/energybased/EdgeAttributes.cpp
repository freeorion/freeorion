/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class EdgeAttributes.
 * 
 * \author Stefan Hachul
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


#include <ogdf/internal/energybased/EdgeAttributes.h>


namespace ogdf {

ostream &operator<< (ostream & output, const EdgeAttributes & A)
{
    output <<"length: "<< A.length;
    output<<"  index of original edge ";
    if (A.e_original == NULL)
      output <<"NULL";
    else output<<A.e_original->index(); 
    output<<"  index of subgraph edge ";
    if (A.e_subgraph == NULL)
      output <<"NULL";
    if (A.moon_edge) 
      output<<" is moon edge ";
    else
      output <<" no moon edge ";
    if (A.extra_edge) 
      output<<" is extra edge ";
    else
      output <<" no extra edge ";
	return output;
}


istream &operator>> (istream & input,  EdgeAttributes & A)
{
    input >> A.length;
	return input;
}


EdgeAttributes :: EdgeAttributes()  
{
 length = 0;
 e_original = NULL;
 e_subgraph = NULL;
 moon_edge = false;
 extra_edge = false;
}


EdgeAttributes :: ~EdgeAttributes()
{
  ;
}

}//namespace ogdf
