/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         folding.h                                          */
/*   version:      1.00.00                                            */
/*   creation:     17.9.1993                                          */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Folding and Unfolding of the graph                 */
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


/* $Log$
 * Revision 1.1  2004/12/30 02:21:46  tzlaine
 * Initial add.
 *
 * Revision 3.3  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.2  1994/05/05  08:20:30  sander
 * Algorithm late labels added.
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

#ifndef FOLDING_H
#define FOLDING_H

/*--------------------------------------------------------------------*/

/* See folding.c for explanation
 * -----------------------------
 */


/* Global variables
 * ----------------
 */

extern GNLIST  	f_subgraphs;
extern GNLIST  	uf_subgraphs;
extern GNLIST  	foldstops;
extern GNLIST  	foldstart; 
extern GNLIST  	ufoldstart;
extern int     	*hide_class;

/* Prototypes
 * ----------
 */

void	folding		 _PP((void));

void 	clear_folding_keepers _PP(());
void 	add_sgfoldstart	      _PP((GNODE v));
void 	add_sgunfoldstart     _PP((GNODE v));
void 	add_foldstart	      _PP((GNODE v));
void 	add_unfoldstart	      _PP((GNODE v));
void 	add_foldstop	      _PP((GNODE v));

void    create_adjedge	 _PP((GEDGE edge));
void    delete_adjedge	 _PP((GEDGE edge));
void	clear_hide_class _PP((void));
GNODE    create_labelnode     _PP((GEDGE e));


/*--------------------------------------------------------------------*/

#endif /* FOLDING_H */ 

