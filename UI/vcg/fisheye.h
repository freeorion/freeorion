/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         fisheye.h                                          */
/*   version:      1.00.00                                            */
/*   creation:     14.4.1993                                          */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Fisheye co-ordinate transformations                */
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
 * Revision 1.2  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 1.1  1994/12/23  18:12:45  sander
 * Initial revision
 *
 *
 */

#ifndef FISHEYE_H
#define FISHEYE_H

/*--------------------------------------------------------------------*/

/* See fisheye.c for explanation
 * -----------------------------
 */

/*---------------------------------------------------------*/

/* Global Variables
 * ----------------
 */

extern long gfocus_x, gfocus_y;
extern long fe_scaling;
extern long gfishdist;
extern int fisheye_view;

#define CSNF_VIEW   1
#define CSCF_VIEW   2
#define PSCF_VIEW   3
#define FCSCF_VIEW  4
#define FPSCF_VIEW  5


/*  Prototypes
 *  ----------
 */

void init_fe _PP((int s1,int s2,int s3,int s4,int sfx,int sfy));
void exit_fe _PP((void));

void set_fe_scaling    _PP((int stretch, int shrink));
void change_fe_scaling _PP((int stretch, int shrink));
int normal_fe_focus    _PP((void));
int set_fe_xfocus      _PP((long fx));
int set_fe_yfocus      _PP((long fy));
int incr_fe_focus      _PP((long dfx, long dfy));
int  change_fe_winsize _PP((int sxmin, int sxmax, int symin, int symax));
int  set_gfishdist     _PP((long gd));
void fe_g_to_s 	       _PP((long x, long y, int  *resx, int  *resy));
void fe_s_to_g         _PP((int  x, int  y, long *resx, long *resy));

/*--------------------------------------------------------------------*/
 
#endif /* FISHEYE_H */


