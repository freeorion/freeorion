/* SCCS-info %W% %E% */
 
/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         timelim.h                                          */
/*   version:      1.00.00                                            */
/*   creation:     14.4.1993                                          */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Time Limit Management                              */
/*   status:       in work                                            */
/*                                                                    */
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
 * Revision 1.2  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 1.1  1994/12/23  18:12:45  sander
 * Initial revision
 *
 */

#ifndef TIMELIM_H 
#define TIMELIM_H 

/*--------------------------------------------------------------------*/

/* Prototypes
 * ----------
 * See timelim.c for more information.
 */

void    init_timelimit    _PP((int x));
void    free_timelimit    _PP((void));
int     test_timelimit    _PP((int perc));


/*--------------------------------------------------------------------*/

#endif /* TIMELIM_H  */

