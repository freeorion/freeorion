// -*- C++ -*-
/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */ 
/*		--------------------------------------		      */ 
/*								      */
/*   file:	   grprint.h					      */
/*   version:	   1.00.00					      */
/*   creation:	   30.11.93				      */
/*   author:	   G. Sander (Version 1.00.00-...)		      */ 
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */ 
/*   description:  Print layout into a file   			      */ 
/*   status:	   in work					      */
/*								      */
/*--------------------------------------------------------------------*/


/* $Id$ */

/*
 *   Copyright (C) 1993--1995 by Georg Sander, Iris Lemke, and
 *                               the Compare Consortium 
 *
 *  This program and documentation is free software; you can redistribute 
 *  it under the terms of the  GNU General Public License as published by
 *  the  Free Software Foundation;  either version 2  of the License,  or
 *  (at your option) any later version.
 *
 *  This  program  is  distributed  in  the hope that it will be useful,
 *  but  WITHOUT ANY WARRANTY;  without  even  the  implied  warranty of
 *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
 *  GNU General Public License for more details.
 *
 *  You  should  have  received a copy of the GNU General Public License
 *  along  with  this  program;  if  not,  write  to  the  Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  The software is available per anonymous ftp at ftp.cs.uni-sb.de.
 *  Contact  sander@cs.uni-sb.de  for additional information.
 */


/* 
 * $Log$
 * Revision 1.1  2004/12/30 02:21:46  tzlaine
 * Initial add.
 *
 * Revision 3.4  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.3  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
 *
 * Revision 3.2  1994/04/27  16:05:19  sander
 * Revert of page and margins added. Messages while waiting
 * added. Adaption to the new print-dialog box.
 *
 * Revision 3.1  1994/03/01  10:59:55  sander
 * Copyright and Gnu Licence message added.
 * Problem with "nearedges: no" and "selfloops" solved.
 *
 * Revision 2.2  1994/01/21  19:33:46  sander
 * VCG Version tested on Silicon Graphics IRIX, IBM R6000 AIX and Sun 3/60.
 * Option handling improved. Option -grabinputfocus installed.
 * X11 Font selection scheme implemented. The user can now select a font
 * during installation.
 * Sun K&R C (a nonansi compiler) tested. Some portabitility problems solved.
 *
 * Revision 2.1  1993/12/08  21:21:34  sander
 * Reasonable fast and stable version
 *
 */

#ifndef GRPRINT_H
#define GRPRINT_H

/*--------------------------------------------------------------------*/

/* See grprint.c and grprin2.c for explanation
 * -------------------------------------------
 */

#ifdef __cplusplus
#include <map>
#include <string>
#include <vector>

typedef std::vector<std::pair<int, int> > TechGraphEdge;
typedef std::map<std::string, std::pair<int, int> > TechVertexMap;
typedef std::map<std::pair<std::string, std::string>, TechGraphEdge> TechEdgeMap;

void print_graph(TechVertexMap& vertices, TechEdgeMap& edges);
extern "C" {
#endif
char *color_text	_PP(( int c));
char *textmode_text	_PP(( int t));
char *shape_text	_PP(( int t));
char *linestyle_text	_PP(( int t));

int  print_pbm_or_ppm
  _PP((char *fname,int a,int b,int c,int d));
int  print_pbm	
  _PP((char *fn,int a,int b,int c,int d,int l,int t,int r,int st,int sh));
int  print_ppm	
  _PP((char *fn,int a,int b,int c,int d,int l,int t,int r,int st,int sh));
#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------*/

#endif /* GRPRINT_H  */


