/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */
/*		--------------------------------------		      */
/*								      */
/*   file:	   draw.h					      */
/*   version:	   1.00.00					      */
/*   creation:	   4.4.93					      */
/*   author:	   I. Lemke  (...-Version 0.99.99)		      */
/*		   G. Sander (Version 1.00.00-...)		      */
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */
/*   description:  Draw nodes and edges 			      */
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
 * Revision 3.3  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.2  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
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

#ifndef DRAW_H
#define DRAW_H

/*--------------------------------------------------------------------*/

/* See draw.c for explanation
 * --------------------------
 */

/* Prototypes
 * ----------
 */

void	draw_main	_PP((void));
void 	draw_one_node	_PP((GNODE v));


/*--------------------------------------------------------------------*/
 
#endif /* DRAW_H  */

