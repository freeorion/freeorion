/*
 * $Revision: 2010 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-08-27 12:25:58 +0200 (Fri, 27 Aug 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of some auxiliary mathematical functions.
 * 
 * \author Stefan Hachul
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2006 oreas GmbH
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#ifndef OGDF_MATH_EXTENSION_H
#define OGDF_MATH_EXTENSION_H

#include <math.h>
#include <iostream>
#include <ogdf/basic/geometry.h>

namespace ogdf {

class mathExtension  
{
	public:
      mathExtension() { }                  //constructor
      ~mathExtension() { }                //destructor
 

      //Returns log2 x ( if x > 0 ).  
      double Log2 (double x)
        {
	  if( x < 0) 
	  {
	    cout<<" error: log2 of a negative number is not defined "<<endl;
	    return -1;
	  }
	  else //x > 0
	    return log(x)/log(2.0); 
	} 
      
      //Returns  log4 x (if x > 0).
      double Log4 (double x)
        {
	  if( x < 0) 
	  {
	    cout<<" error: log4 of a negative number is not defined "<<endl;
	    return -1;
	  }
	  else //x > 0
	    return log(x)/log(4.0); 
	} 

      //Returns min {x,y}. 
      double min (double x, double y)
        {
	  if(x<y) return x;
	  else return y;
	}

      //Returns max {x,y}.
      double max (double x,double y)
        {
	  if (x>y) return x;
          else return y;
	}

      //Returns n over k.
      long  binco(int n, int k)
        {  
	  long h;
	  h = fac(n)/(fac(n-k)*fac(k));
	  return h;
	}
      
      //Returns n!
      long fac(int n)
        {
	  int i;
	  long p = 1;
	  if(n < 0)
	    {
	      cout<<"mathExtension:: fac() value smaller 0"<<endl;
	      return 1;
	    }
	  else if(n == 0) 
	    return 1;
	  else
	    {
	      for(i = 1;i<=n;i++)
		p *=i;
	      return p;
	    }
	}            
      
      //Returns the norm of v.           
      double norm(DPoint v)
        { 
	  double x = v.m_x;
	  double y = v.m_y;
	  return sqrt(x*x + y*y);
	}
 
}; 
}//namespace ogdf
#endif
