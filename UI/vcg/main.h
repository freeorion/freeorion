// -*- C++ -*-
/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         main.h                                             */
/*   version:      1.00.00                                            */
/*   creation:     1.4.1993                                           */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Top level program                                  */
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
 * Revision 3.14  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.13  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 * infobox behaviour improved.
 * First version of fisheye (carthesian).
 * Options Noedge and nonode.
 * Titles in the node title box are now sorted.
 * Timelimit functionality improved.
 *
 * Revision 3.12  1994/11/23  14:50:47  sander
 * Option -nocopt added.
 *
 * Revision 3.11  1994/08/08  16:01:47  sander
 * Attributes xraster, xlraster, yraster added.
 *
 * Revision 3.10  1994/08/05  12:13:25  sander
 * Treelayout added. Attributes "treefactor" and "spreadlevel" added.
 * Scaling as abbreviation of "stretch/shrink" added.
 *
 * Revision 3.9  1994/08/02  15:36:12  sander
 * CHECKNODE option added to allow tracing of properties
 * of one single node.
 *
 * Revision 3.8  1994/06/07  14:09:59  sander
 * Splines implemented.
 * HP-UX, Linux, AIX, Sun-Os, IRIX compatibility tested.
 * The tool is now ready to be distributed.
 *
 * Revision 3.7  1994/05/17  16:39:10  sander
 * attribute node_align added to allow nodes to be centered in the levels.
 *
 * Revision 3.6  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
 *
 * Revision 3.5  1994/05/05  08:20:30  sander
 * Algorithm late labels added: If labels are inserted
 * after partitioning, this may yield a better layout.
 *
 * Revision 3.4  1994/04/27  16:05:19  sander
 * Some general changes for the PostScript driver.
 * Horizontal order added. Bug fixes of the folding phases:
 * Folding of nested graphs works now.
 *
 * Revision 3.3  1994/03/04  19:11:24  sander
 * Specification of levels per node added.
 * X11 geometry behaviour (option -geometry) changed such
 * that the window is now opened automatically.
 *
 * Revision 3.2  1994/03/03  14:12:21  sander
 * median centering heuristics added to reduce crossings.
 *
 * Revision 3.1  1994/03/01  10:59:55  sander
 * Copyright and Gnu Licence message added.
 * Problem with "nearedges: no" and "selfloops" solved.
 *
 * Revision 2.4  1994/01/21  19:33:46  sander
 * VCG Version tested on Silicon Graphics IRIX, IBM R6000 AIX and Sun 3/60.
 * Option handling improved. Option -grabinputfocus installed.
 * X11 Font selection scheme implemented. The user can now select a font
 * during installation.
 * Sun K&R C (a nonansi compiler) tested. Some portabitility problems solved.
 *
 * Revision 2.3  1994/01/03  15:29:06  sander
 * First complete X11 version.
 *
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
#include <string>
void parse_part(const std::string& str);
extern "C" {
#endif

/*--------------------------------------------------------------------*/

/* See main.c for explanation
 * --------------------------
 */

/* from parser -------------------------------------------*/
extern FILE     *yyin;

void   line_directive _PP((char *text));
char  *my_itoa        _PP((int x));
void   syntaxerror    _PP((int line,int pos,char *mesge));
void   warning        _PP((int line,int pos,char *mesge));

#define SYERR(x,m) syntaxerror(xfirst_line(x),xfirst_column(x),m)

/* from device --------------------------------------------*/
void gs_wait_message	_PP((int c));

/*---------------------------------------------------------*/

/* Global Variables
 * ----------------
 */

extern char short_banner[];
extern char version_str[];
extern char date_str[];
extern char revision_str[];

/*  Prototypes
 *  ----------
 */

void Fatal_error	_PP((char *x, char *y));
void visualize_part	_PP((void));
void relayout		_PP((void));

/*--------------------------------------------------------------------*/
 
#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */


