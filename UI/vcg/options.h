/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         options.h                                          */
/*   version:      1.00.00                                            */
/*   creation:     1.4.1993                                           */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Option handling                                    */
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


#ifndef OPTIONS_H
#define OPTIONS_H


/*--------------------------------------------------------------------*/

/* See options.c for explanation
 * -----------------------------
 */

/*--------------------------------------------------------------------*/

/* Global Variables
 * ----------------
 */

extern int  gblargi;
extern int  gblargc;
extern char **gblargv;

extern int grabinputfocus;

extern char *Xmydiplayname;
extern char *Xmygeometry;
extern int  Xmyborderwidth;
extern char *Xmybackingstore;

extern int multiple_files;
extern int silent;
extern int nr_max_errors;
extern int fastflag;
extern int supress_nodes;
extern int supress_edges;
extern int summarize_double_edges;
extern int hide_single_nodes;
extern int layout_flag;

#define TREE_LAYOUT 20

extern int crossing_heuristics;
extern int local_unwind;
extern int near_edge_layout;
extern int fine_tune_layout;
extern int edge_label_phase;
extern int min_baryiterations;
extern int max_baryiterations;
extern int skip_baryphase2;
extern int max_mediumshifts;
extern int min_mediumshifts;
extern int nwdumping_phase;
extern int max_centershifts;
extern int min_centershifts;
extern int prio_phase;
extern int straight_phase;
extern int max_straighttune;
extern int max_edgebendings;
extern int manhatten_edges;
extern int one_line_manhatten;
extern int layout_downfactor; 
extern int layout_upfactor; 
extern int layout_nearfactor; 

extern int G_spline;
extern int G_flat_factor;

extern int touch_file;

extern int    exfile;
extern char   exfilename[800];
extern int    extype;
extern int    expaper;
extern int    excolor;
extern int    exori;
extern int    expapernum;
extern int    exxdpi;
extern int    exydpi;
extern float  exscaling;
extern float  exwidth;
extern float  exheight;
extern float  exleftm;
extern float  exrightm;
extern float  extopm;
extern float  exbottomm;
extern int    exbbox;

extern int      cmap_size;
extern int      cmap_changed;
extern unsigned char origredmap[];
extern unsigned char origgreenmap[];
extern unsigned char origbluemap[];
extern unsigned char redmap[];
extern unsigned char greenmap[];
extern unsigned char bluemap[];

#ifdef VMS
extern int   RootWinMaxDepth;
#else
extern int maxDepth;
#endif
extern int ScreenWidth;
extern int ScreenHeight;
extern int colored;

extern long V_xmin;
extern long V_xmax;
extern long V_ymin;
extern long V_ymax;
extern long V_xmin_initial;
extern long V_ymin_initial;



extern char Dataname[];
extern char filename[];

#define MAXCLASS  16

extern int G_timelimit;
extern int G_xymax_final;
extern char *G_title;
extern long G_x, G_y;
extern int G_width, G_height;
extern int G_width_set, G_height_set;
extern int G_xmax, G_ymax;
extern int G_xbase, G_ybase;
extern int G_xspace, G_yspace;
extern int G_orientation;
extern int G_folding;
extern int G_invisible[];
extern int G_color;
extern int G_displayel;
extern int G_dirtyel;
extern int G_shrink, G_stretch;
extern int G_yalign;
extern int G_portsharing;
extern int G_arrowmode;
extern int G_dspace;

extern int G_xraster;
extern int G_yraster;
extern int G_dxraster;


extern char    *fold_elabel;
extern int     fold_lstyle;
extern int     fold_thick;
extern int     fold_ecolor;
extern int     fold_elcolor;
extern int     fold_arrows;
extern int     fold_barrows;
extern int     fold_arrowc;
extern int     fold_barrowc;
extern int     fold_arrsty;
extern int     fold_barrsty;

extern int 	max_eprio;

extern int  info_name_available;
extern char *info_names[3];
extern int  max_nr_classes;
extern int  class_name_available;
extern char **class_names;

extern int grabinputfocus;
extern int locFlag;



/*  Prototypes
 *  ----------
 */

int  scanOptions		_PP((int argc, char *argv[]));
void print_version_copyright	_PP((void));
void print_basic_help		_PP((void));
void print_help			_PP((void));


/*--------------------------------------------------------------------*/
 
#endif /* OPTIONS_H */


