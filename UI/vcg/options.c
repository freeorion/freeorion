/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         options.c                                          */
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
 * Revision 1.3  1995/02/08  12:53:51  sander
 * Negative integers allowed for integer options
 *
 * Revision 1.2  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 1.1  1994/12/23  18:12:45  sander
 * Initial revision
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "main.h"
#include "alloc.h"
#include "fisheye.h"
#include "options.h"


/************************************************************************
 * This module provides function to scan the options and help messages
 *
 * This file provides the following functions:
 * ------------------------------------------
 * scanOptions              Scan the option array and return after the 
 *                          first file.
 * print_version_copyright  Print a version and a copyright message.
 * print_basic_help         Print a basic help message.
 * print_help               Print a full help message.
 ************************************************************************/


/* Prototypes
 * ==========
 */

static int simpleOption	_PP((char *optstr));
static int intOption    _PP((char *optstr, int *res));
static int stringOption	_PP((char *optstr, char *strw));
static int fnameOption  _PP((char *optstr, char *res));
static int unitOption	_PP((char *optstr, float *res));


/* Global variables
 * ================
 */


/* The global copy of the option argument buffer 
 */

int 	gblargi = 1;        /* the actual  index */
int 	gblargc;            /* the maximal index */
char	**gblargv;          /* the option buffer */


/* General flags regarding the option handling
 */

static int opt_error;        /* indicates errors during the option scan */
static int opt_give_help;    /* indicates the the user needs full help  */
static int opt_give_version; /* indicates the version is needed         */
static int opt_from_stdin;   /* indicates that the input is stdin       */


/*  Special flags for X11:
 *  ----------------------
 */


/*  Indicates that VCG should grab the input focus.
 */

int grabinputfocus;

char *Xmydiplayname   = NULL;
char *Xmygeometry     = NULL;
int  Xmyborderwidth   = 2;
char *Xmybackingstore = NULL;

#ifdef X11
static char *Xmyfontname = NULL;
extern char Xfontname[512];	/* from X11dev.c */
#endif


/*  Flag: indicates that we have more than one input file 
 */

int multiple_files = 0;


/*  Flag: indicates that we should be silent
 */

int silent = 0;

/*  Maximal number of parse errors
 */

int nr_max_errors = 16;

/*  Flag to suppress various tests
 */

int fastflag = 0;

/*  Flag to supress drawing of nodes and edges.
 */

int supress_nodes = 0;
int supress_edges = 0;

/*  Flag to summarize multiple edges without label between same nodes.
 */

int summarize_double_edges = 0;

/*  Flag to hide single nodes. Nodes that are unconnected with everything 
 *  else are sometimes very ugly.
 */

int hide_single_nodes = 0;


/*  Layout flag:  To change the layout may reduce the number of crossings, 
 *  but it takes. The standard layout is the fastest.
 *
 *    0   ->  normal SCC layout
 *    1   ->  maximize depth of the layout (tree traversal heuristic)
 *    2   ->  minimize depth of the layout (tree traversal heuristic)
 *    3   ->  topological order layout, minimize backward edges 
 *    4   ->  maximize depth of the layout (very slow) 
 *    5   ->  minimize depth of the layout (very slow) 
 *    6   ->  presort according increasing indegree  
 *    7   ->  presort according decreasing indegree  
 *    8   ->  presort according increasing outdegree 
 *    9   ->  presort according decreasing outdegree 
 *    10  ->  presort according increasing degree 
 *    11  ->  presort according decreasing degree 
 *    12  ->  dfs algorithm 
 *    TREE_LAYOUT -> use special layout for trees
 */

int layout_flag = 0;


/* Crossing heuristics:
 *
 *    0   ->  barycentering
 *    1   ->  mediancentering
 *    2   ->  bary   with mediancentering
 *    3   ->  median with barycentering
 */

int crossing_heuristics = 0;


/* Local crossing unwinding: 1 = yes, 0 = not
 */

int local_unwind = 1;


/*  Near edges allowed: 1 = yes, 0 = not 
 */

int near_edge_layout = 1;


/*  Fine tuning allowed: 1 = yes, 0 = no
 */

int fine_tune_layout = 1;


/*  When to add (nondirty) edge labels:
 *  Phase 0 = directly after folding
 *  Phase 1 = after partitioning
 */

int edge_label_phase = 0;

 
/*  Number of iteration used for barycentering layout
 *  Minimal and maximal.
 */

int min_baryiterations = 0;
int max_baryiterations = MAXINT;


/*  Flag, indicating whether the phase 2 of bary centering should be skipped
 */

int skip_baryphase2 = 0; 


/*  Number of iteration used for the initial medium shifting
 *  that calculates x positions. Minimal and maximal.
 */

int max_mediumshifts = 100;
int min_mediumshifts = 0;


/*  Flag, indicating that the last mediymshift phase is upwards
 *  instead downwards.
 */

int nwdumping_phase = 0;


/*  Number of iteration used for the calculation of x positions.
 *  Minimal and maximal.
 */

int max_centershifts = 100;
int min_centershifts = 0;

/*  Number of iteration used for the straight line phase.
 */

int max_straighttune = 100;


/*  Flag, indicates that the Priority layout phase should be used. 
 *  In this case, max_centershifts is 0, i.e. the center shift phase 
 *  is suppressed.
 *  The priority layout phase forces nodes with exactly one successor
 *  and one predecessor to be placed in a straight vertical line.
 *  This is useful for manhatten layout.
 */

int prio_phase = 0;

/*  Flag, indicates that the straigt line fine tuning should be
 *  switched on. This is useful for manhatten layout.
 */

int straight_phase = 0;


/*  Number of iterations during the calculation of bendings.
 */

int max_edgebendings = 100;

/*  Flag indicates that manhatten edges are required.
 *  Manhatten layout means that all edge segments are either horizontal
 *  or vertical, i.e. have a gradient of 0 degree or 90 degree.
 *  This implies prio_phase = 1.
 */

int manhatten_edges = 0;

/*  Flag indicates that horizontal segments of manhatten edges between
 *  layers are shared, thus there is only one line. This is helpful
 *  for trees.
 */

int one_line_manhatten = 0;

/* Layoutfactors that influence the centering of nodes in step3.
 * Normally,  (down,up,near) = (1,1,1)  is appropriate.
 * For trees, (down,up,near) = (10,1,5) gives better layouts.
 * (1,1,1) enforces nodes to be positioned central to all surrounding
 * nodes.
 * (10,1,5) enforces nodes to be positioned central to their successors.
 * (1,10,5) enforces nodes to be positioned central to their predecessors.
 */

int layout_downfactor = 1;
int layout_upfactor   = 1;
int layout_nearfactor = 1;


/*  Edges can be drawn as splines, if G_spline is 1.
 *  G_flat_factor is the factor how splines should be bend.
 */

int G_spline = 0;
int G_flat_factor = 70;


/*  Flag, indicates that after displaying, the specification file
 *  should be touched. For Animation only.
 */

int touch_file = 0;



/*  Interface for PostScript/Bitmap output without X11 interaction.
 *  The VCG tool acts as a kind of filter VCG -> PostScript.
 */

int    exfile    = 0;
char   exfilename[800];
int    extype    = 3;     /* PostScript */
int    expaper   = 1;     /* A4         */
int    excolor   = 3;     /* B&W        */
int    exori     = 1;     /* Portrait   */
int    expapernum= 1;
int    exxdpi    = 72;
int    exydpi    = 72;
float  exscaling = -1.0;   /* Maxspect */
float  exwidth   = -1.0;
float  exheight  = -1.0;
float  exleftm   = -1.0;  /* Left  Margin  */
float  exrightm  = -1.0;  /* Right  Margin */
float  extopm    = -1.0;  /* Top    Margin */
float  exbottomm = -1.0;  /* Bottom Margin */
int    exbbox    = 1;     /* with Bounding Box */


/*  Color maps
 *  ----------
 *  See alloc.h for color names
 */


int 	 cmap_size    = BASECMAPSIZE;
int 	 cmap_changed = 1;

unsigned char origredmap[BASECMAPSIZE] = {
	  255, 0, 255, 0, 255, 255, 0, 85, 0, 128, 0, 128, 128,
	  0, 255, 170, 128, 255, 128, 255, 255, 128, 238, 64, 127,
	  240, 160, 154, 255, 255, 218, 0 
};

unsigned char origgreenmap[BASECMAPSIZE] = {
	  255, 0, 0, 255, 255, 0, 255, 85, 0, 0, 128, 128, 0,
	  128, 215, 170, 128, 128, 255, 255, 128, 255, 130, 224,
	  255, 230, 32, 205, 192, 165, 112, 0 
};

unsigned char origbluemap[BASECMAPSIZE] = {
	  255, 255, 0, 0, 0, 255, 255, 85, 128, 0, 0, 0, 128,
	  128, 0, 170, 255, 128, 128, 128, 255, 255, 238, 208, 212,
	  140, 240, 50, 203, 0, 214, 0 
};

unsigned char redmap[CMAPSIZE];
unsigned char greenmap[CMAPSIZE];
unsigned char bluemap[CMAPSIZE];



/*  Screen Specifics
 *  ----------------
 */

#ifdef VMS
int 	RootWinMaxDepth;    /* Depth of the frame buffer         */
#else
int 	maxDepth;           /* Depth of the frame buffer         */
#endif
int	ScreenWidth;	    /* Size of the screen                */
int   	ScreenHeight;
int	colored= -1; 	    /* Color screen (1) or BW screen (0) */


/*  Layout Parameters
 *  -----------------
 */

long 	V_xmin;		    /* Minimal and maximal co-ordinates */
long	V_xmax;		    /* that are visible                 */
long	V_ymin;
long	V_ymax;

long 	V_xmin_initial = 0L;	    /* and their initial values */
long 	V_ymin_initial = 0L;


/*  File Specifics 
 *  --------------
 *  Because of C-stile line directives, the file name used by the
 *  parser for error messages is not always the same as the filename
 *  of the actal input file. 
 */

char	Dataname[ 801];     /* Filename that is actual input        */
char    filename[1024];     /* Filename from the view of the parser */ 


/*  Global Graph Attributes 
 *  -----------------------
 */

int 	G_timelimit = 0;	/* Limitation in the running time
				 * in seconds. 0 = no limit.
				 */

int	G_xymax_final = 0;

char    *G_title;		/* title of the global graph          */
long    G_x, G_y;		/* window location on the root screen */
int     G_width, G_height;	/* size of the open part of window    */ 
int     G_width_set;		/* indicates that the width  was set. */
int     G_height_set;		/* indicates that the height was set. */
int     G_xmax, G_ymax;		/* maximal size of window             */
int     G_xbase, G_ybase;	/* location of the origin (0,0)       */
int     G_xspace, G_yspace;	/* offset of drawing area             */
int     G_orientation;		/* top-to-bottom, or left-to-right    */
int     G_folding;		/* global graph folded (1) or not (0) */
int     G_color;		/* background color                   */
int     G_displayel;		/* edge labels drawn (1) or not (0)   */
int     G_dirtyel;		/* edge labels dirty (1) or not (0)   */
int     G_shrink, G_stretch;	/* global scaling factors             */
int     G_yalign;
int 	G_dspace;		/* space between dummy nodes	      */
int     G_portsharing;		/* edge sharing between the ports     */
int     G_arrowmode;		/* arrow drawing mode                 */

int 	G_xraster;		/* x-raster for real nodes            */
int 	G_yraster;		/* y-raster for real nodes	      */
int 	G_dxraster;		/* x-raster for dummy nodes	      */



/* Default values of folded edges. This initialization indicates that
 * folded edges are thick. 
 */
 
char    *fold_elabel= NULL;        /* folded edge label          */
int     fold_lstyle = -1;          /* folded edge line style     */
int     fold_thick  = 3;           /* folded edge thickness      */
int     fold_ecolor = -1;          /* folded edge color          */
int     fold_elcolor= -1;          /* folded edge color          */
int     fold_arrows = -1;          /* folded edge arrowsize      */
int     fold_barrows= -1;          /* folded edge back arrowsize */
int     fold_arrowc = -1;          /* folded edge arrowcolor     */
int     fold_barrowc= -1;          /* folded edge back arrowcolor*/
int     fold_arrsty = -1;          /* folded edge arrowstyle     */
int     fold_barrsty= -1;          /* folded edge back arrowstyle*/


/* 	The maximal edge priority 
 */

int max_eprio;


/*	The names of the info fields. If info_name_available, 
 *      info_names is set to a table of strings we use in the 
 *      interaction menu.
 */

int 	info_name_available;
char 	*info_names[3];

/*	The names of the edge classes. If class_name_available, 
 *      class_names is set to a table of strings we use in the 
 *      interaction menu.
 */


int     max_nr_classes;
int 	class_name_available;
char 	**class_names = NULL;


/* the following flag indicates whether all nodes to be visualized
 * have already locations, or not. If not, then we must layout the
 * graph.
 */

int     locFlag;



/*--------------------------------------------------------------------*/
/*  Special Routines to get certain types of options		      */
/*--------------------------------------------------------------------*/


/*  Get a simple option 
 *  -------------------
 *  e.g. like -a, -b, -c. We return true, if the option was recognized.
 *  We return false, if the option was not recognized.
 *  Before the actual argument index gblargi points to the actual
 *  argument.
 *  Afterwards, the actual argument index gblargi points to the
 *  next recognizable argument.
 */


#ifdef ANSI_C
static int simpleOption(char *optstr)
#else
static int simpleOption(optstr)
char *optstr;
#endif
{
	char *c, *d;

	if (gblargi >= gblargc) { opt_error = 1; return(0); }

	c = optstr;
	d = gblargv[gblargi];

	while ((*c) && (*d) && ((*c== *d) || (toupper(*c)== *d))) { c++; d++; }

	while (*c==' ') c++; 
	if (*c) return(0);
	while (*d==' ') d++; 
	if (*d) return(0);
	
	gblargi++; 
	return(1);
}



/*  Get an integer option (only positive integers)
 *  ----------------------------------------------
 *  e.g. on -a 4711, we set *res = 4711 and return true.
 *  We return false, if the option was not recognized.
 *  Before the actual argument index gblargi points to the actual
 *  argument.
 *  Afterwards, the actual argument index gblargi points to the
 *  next recognizable argument.
 */


#ifdef ANSI_C
static int intOption(char *optstr, int *res)
#else
static int intOption(optstr, res)
char *optstr;
int *res;
#endif
{
	char *c, *d;
	int oldi;

	oldi = gblargi;
	if (gblargi >= gblargc) { opt_error = 1; return(0); }

	c = optstr;
	d = gblargv[gblargi];

	while ((*c) && (*d) && ((*c== *d) || (toupper(*c)== *d))) { c++; d++; }

	while (*c==' ') c++; 
	if (*c) return(0);
	
	while (*d==' ') d++; 
	if (!*d) { 
		gblargi++; 
		if (gblargi >= gblargc) { 
			opt_error = 1; 
			FPRINTF(stderr,"Option error at `%s' : ", optstr); 
			FPRINTF(stderr,"Missing integer argument !\n");
			gblargi = oldi;
			return(0); 
		}
		d = gblargv[gblargi]; 
	}

	if (   (*d!='0') && (*d!='1') && (*d!='2') && (*d!='3') && (*d!='4') 
	    && (*d!='-') && (*d!='5') && (*d!='6') && (*d!='7') && (*d!='8') 
	    && (*d!='9')) {
			gblargi = oldi;
			return(0);
	}

	*res = atoi(d);

	gblargi++; 
	return(1);
}


/*  Get a string option
 *  -------------------
 *  i.e. we test on the string word strw.
 *  We return true, if the option + strw was recognized.
 *  We return false, if the option was not recognized.
 *  Before the actual argument index gblargi points to the actual
 *  argument.
 *  Afterwards, the actual argument index gblargi points to the
 *  next recognizable argument.
 */


#ifdef ANSI_C
static int stringOption(char *optstr, char *strw)
#else
static int stringOption(optstr, strw)
char *optstr;
char *strw;
#endif
{
	char *c, *d;
	int oldi;

	oldi = gblargi;
	if (gblargi >= gblargc) { opt_error = 1; return(0); }

	c = optstr;
	d = gblargv[gblargi];

	while ((*c) && (*d) && ((*c== *d) || (toupper(*c)== *d))) { c++; d++; }

	while (*c==' ') c++; 
	if (*c) return(0);
	
	while (*d==' ') d++; 
	if (!*d) { 
		gblargi++; 
		if (gblargi >= gblargc) { 
			opt_error = 1; 
			FPRINTF(stderr,"Option error at `%s' : ", optstr); 
			FPRINTF(stderr,"Missing string argument !\n");
			gblargi = oldi;
			return(0); 
		}
		d = gblargv[gblargi]; 
	}

	c = strw;
	while ((*c) && (*d) && ((*c== *d) || (toupper(*c)== *d))) { c++; d++; }

	while (*c==' ') c++; 
	if (*c) { gblargi = oldi; return(0); }
	while (*d==' ') d++; 
	if (*d) { gblargi = oldi; return(0); }

	gblargi++; 
	return(1);
}


/*  Get a filename option
 *  ---------------------
 *  If the option keyword is recognized, the next option is copied
 *  into the result, and true is returned. The result string buffer
 *  should have a capacity of at least 800 chars.
 *  We return false, if the option was not recognized.
 *  Before the actual argument index gblargi points to the actual
 *  argument.
 *  Afterwards, the actual argument index gblargi points to the
 *  next recognizable argument.
 */


#ifdef ANSI_C
static int fnameOption(char *optstr, char *res)
#else
static int fnameOption(optstr, res)
char *optstr;
char *res;
#endif
{
	char *c, *d;
	int oldi;

	oldi = gblargi;
	if (gblargi >= gblargc) { opt_error = 1; return(0); }

	c = optstr;
	d = gblargv[gblargi];

	while ((*c) && (*d) && ((*c== *d) || (toupper(*c)== *d))) { c++; d++; }

	while (*c==' ') c++; 
	if (*c) return(0);
	while (*d==' ') d++; 
	if (*d) return(0);
	
	gblargi++; 
	if (gblargi >= gblargc) { 
		opt_error = 1; 
		FPRINTF(stderr,"Option error at `%s' : ", optstr); 
		FPRINTF(stderr,"Missing file name !\n");
		gblargi = oldi;
		return(0); 
	}
	d = gblargv[gblargi]; 
	if (*d=='-') {
		opt_error = 1;
		FPRINTF(stderr,"Option error at `%s' : ", d);
		FPRINTF(stderr,"File names should not start with `-' !\n");
		gblargi = oldi;
		return(0); 
	}

	strncpy(res, d, 800);

	gblargi++; 
	return(1);
}



/*  Get a stringword option
 *  -----------------------
 *  If the option keyword is recognized, the next option is copied
 *  into the result, and true is returned. The result string buffer
 *  should have a capacity of at least 800 chars.
 *  We return false, if the option was not recognized.
 *  Before the actual argument index gblargi points to the actual
 *  argument.
 *  Afterwards, the actual argument index gblargi points to the
 *  next recognizable argument.
 */


/*  Get an unit option (float plus unit keyword)
 *  --------------------------------------------
 *  e.g. on -a 3.4 meter, we set *res = 3.4 * 100 and return true.
 *  We return false, if the option was not recognized.
 *  Before the actual argument index gblargi points to the actual
 *  argument.
 *  Afterwards, the actual argument index gblargi points to the
 *  next recognizable argument.
 */


#ifdef ANSI_C
static int unitOption(char *optstr, float *res)
#else
static int unitOption(optstr, res)
char *optstr;
float *res;
#endif
{
	char *c, *d;
	float help;
	int oldi;

	oldi = gblargi;
	if (gblargi >= gblargc) { opt_error = 1; return(0); }

	c = optstr;
	d = gblargv[gblargi];

	while ((*c) && (*d) && ((*c== *d) || (toupper(*c)== *d))) { c++; d++; }

	while (*c==' ') c++; 
	if (*c) return(0);
	
	while (*d==' ') d++; 
	if (!*d) { 
		gblargi++; 
		if (gblargi >= gblargc) { 
			opt_error = 1; 
			FPRINTF(stderr,"Option error at `%s' : ", optstr); 
			FPRINTF(stderr,"Missing float argument !\n");
			gblargi = oldi;
			return(0); 
		}
		d = gblargv[gblargi]; 
	}

	if (   (*d!='0') && (*d!='1') && (*d!='2') && (*d!='3') 
	    && (*d!='4') && (*d!='5') && (*d!='6') && (*d!='7') 
	    && (*d!='8') && (*d!='9') && (*d!='.') && (*d!=' ')) {
			gblargi = oldi;
			return(0);
	}

	help = atof(d);

	while (   (*d=='0') || (*d=='1') || (*d=='2') || (*d=='3') 
	       || (*d=='4') || (*d=='5') || (*d=='6') || (*d=='7') 
	       || (*d=='8') || (*d=='9') || (*d=='.') || (*d==' ')) d++;

	while (*d==' ') d++; 
	if (!*d) { 
		gblargi++; 
		if (gblargi >= gblargc) { 
			opt_error = 1; 
			FPRINTF(stderr,"Option error at `%s' : ", optstr); 
			FPRINTF(stderr,"Missing unit !\n");
			gblargi = oldi;
			return(0); 
		}
		d = gblargv[gblargi]; 
	}

	if (strcmp(d,"in") == 0)            help = help * 2.54;
	else if (strcmp(d,"inch")      ==0) help = help * 2.54;
	else if (strcmp(d,"ft")        ==0) help = help * 30.48;
	else if (strcmp(d,"foot")      ==0) help = help * 30.48;
	else if (strcmp(d,"feet")      ==0) help = help * 30.48;
	else if (strcmp(d,"foots")     ==0) help = help * 30.48;
	else if (strcmp(d,"mm")        ==0) help = help / 10.0;
	else if (strcmp(d,"millimeter")==0) help = help / 10.0;
	else if (strcmp(d,"cm")        ==0) help = help;
	else if (strcmp(d,"centimeter")==0) help = help;
	else if (strcmp(d,"decimeter") ==0) help = help * 10.0;
	else if (strcmp(d,"dezimeter") ==0) help = help * 10.0;
	else if (strcmp(d,"dm")        ==0) help = help * 10.0;
	else if (strcmp(d,"m")         ==0) help = help * 100.0;
	else if (strcmp(d,"meter")     ==0) help = help * 100.0;
	else {  opt_error = 1; 
		FPRINTF(stderr,"Option error at `%s' : ", optstr); 
		FPRINTF(stderr,"Missing unit !\n");
		gblargi = oldi;
		return(0); 
	}

	*res = help;
	gblargi++; 
	return(1);
}




/*--------------------------------------------------------------------*/
/*  Fetch option Routine					      */
/*--------------------------------------------------------------------*/


/* Scan the options
 * ----------------
 * If the first file name was found, we return 1, in order to allow
 * to treat this file.
 * The function can be reentered after a successful display of a graph,
 * in order to treat the remaining options. 
 */


#ifdef ANSI_C
int scanOptions(int argc, char *argv[])
#else
int scanOptions(argc, argv)
int 	argc;
char	*argv[];
#endif
{
	int help;

    G_xmax = G_ymax = -1;

#ifdef NOINPUTFOCUS
	grabinputfocus = 0;
#else
	grabinputfocus = 1;
#endif

	gblargc = argc;
	gblargv = argv;

	opt_error        = 0;
	opt_give_help    = 0;
	opt_give_version = 0;
	opt_from_stdin   = 0;
	Dataname[0] = 0;

	while (gblargi<gblargc) {

		/* General Options */
		/* ----------------*/

		if      (simpleOption("-h"))         opt_give_help = 1;
		else if (simpleOption("-?"))         opt_give_help = 1;
		else if (simpleOption("-help"))      opt_give_help = 1;
		else if (simpleOption("-v"))         opt_give_version = 1;
		else if (simpleOption("-version"))   opt_give_version = 1;
		else if (   intOption("-a",         &touch_file));
		else if (   intOption("-animation", &touch_file));
		else if (   intOption("-b",         &max_edgebendings));
		else if (   intOption("-bmax",      &max_edgebendings));
		else if (   intOption("-bending",   &max_edgebendings));
		else if (simpleOption("-bary"))      crossing_heuristics = 0;
		else if (simpleOption("-barymedian"))crossing_heuristics = 2;
		else if (   intOption("-c",         &max_baryiterations));
		else if (   intOption("-cmax",      &max_baryiterations));
		else if (   intOption("-crossing",  &max_baryiterations));
		else if (   intOption("-cmin",      &min_baryiterations));
		else if (   intOption("-e",         &nr_max_errors));
		else if (   intOption("-error",     &nr_max_errors));
		else if (simpleOption("-f"))         fastflag = 1;
		else if (simpleOption("-fast"))      fastflag = 1;
		else if (simpleOption("-hidesingles"))  hide_single_nodes = 1;
		else if (simpleOption("-ignoresingles"))hide_single_nodes = 1;
		else if (simpleOption("-l"))         edge_label_phase = 1;
		else if (simpleOption("-latelabels"))edge_label_phase = 1;
		else if (simpleOption("-m"))         multiple_files = 1;
		else if (simpleOption("-multi"))     multiple_files = 1;
		else if (simpleOption("-median"))    crossing_heuristics = 1;
		else if (simpleOption("-medianbary"))crossing_heuristics = 3;
		else if (simpleOption("-manhatten")) manhatten_edges = 1;
		else if (simpleOption("-manhattan")) manhatten_edges = 1;
		else if (simpleOption("-nomanhatten")) manhatten_edges = 2;
		else if (simpleOption("-nomanhattan")) manhatten_edges = 2;
		else if (simpleOption("-nocopt"))    skip_baryphase2 = 1;
		else if (simpleOption("-nocopt2"))   skip_baryphase2 = 1;
		else if (simpleOption("-nocopt1"))   local_unwind = 0;
		else if (simpleOption("-nocoptl"))   local_unwind = 0;
		else if (simpleOption("-nocoptloc")) local_unwind = 0;
		else if (simpleOption("-notune"))    fine_tune_layout = 0;
		else if (simpleOption("-nofinetune"))fine_tune_layout = 0;
		else if (simpleOption("-nonearedge"))near_edge_layout = 0;
		else if (simpleOption("-nonodes"))   supress_nodes = 1;
		else if (simpleOption("-noedges"))   supress_edges = 1;
		else if (simpleOption("-orthogonal"))manhatten_edges = 1;
		else if (simpleOption("-noorthogonal"))manhatten_edges = 2;
		else if (   intOption("-p",         &max_mediumshifts));
		else if (   intOption("-pmax",      &max_mediumshifts));
		else if (   intOption("-pendulum",  &max_mediumshifts));
		else if (   intOption("-pmin",      &min_mediumshifts));
		else if (simpleOption("-prio"))      prio_phase = 1;
		else if (simpleOption("-noprio"))    prio_phase = 2;
		else if (   intOption("-r",         &max_centershifts));
		else if (   intOption("-rmax",      &max_centershifts));
		else if (   intOption("-rubberband",&max_centershifts));
		else if (   intOption("-rmin",      &min_centershifts));
		else if (simpleOption("-smanhatten")) one_line_manhatten = 1;
		else if (simpleOption("-smanhattan")) one_line_manhatten = 1;
		else if (simpleOption("-nosmanhatten")) one_line_manhatten = 2;
		else if (simpleOption("-nosmanhattan")) one_line_manhatten = 2;
		else if (   intOption("-smax",      &max_straighttune));
		else if (simpleOption("-straight"))  straight_phase = 1;
		else if (simpleOption("-nostraight"))straight_phase = 2;
		else if (simpleOption("-s"))         summarize_double_edges=1;
		else if (simpleOption("-summarize")) summarize_double_edges=1;
		else if (simpleOption("-silent"))    silent = 1; 
		else if (simpleOption("-spline"))    G_spline = 1; 
		else if (   intOption("-t"         ,&G_timelimit)) {
			if (G_timelimit<=1) G_timelimit = 1;
		}
		else if (   intOption("-timelimit" ,&G_timelimit)) {
			if (G_timelimit<=1) G_timelimit = 1;
		}
		else if (   intOption("-xpos"      ,&help)) {
			if (help>0) V_xmin_initial = (long)help;
		}
		else if (   intOption("-ypos"      ,&help)) {
			if (help>0) V_ymin_initial = (long)help;
		}
		else if (simpleOption("-blackwhite")) {
				colored = 0;
				excolor = 3;
		} 
		else if (simpleOption("-nocolors")) {
				colored = 0;
				excolor = 3;
		} 
		else if (simpleOption("-nocolours")) {
				colored = 0;
				excolor = 3;
		} 

		/* Options for the Layout Algorithm */
		/* -------------------------------- */

		else if (stringOption("-d", "normal"))      layout_flag = 0;
		else if (stringOption("-d", "dfs"))         layout_flag = 12;
		else if (stringOption("-d", "0"))           layout_flag = 3;
		else if (stringOption("-d", "minbackward")) layout_flag = 3;
		else if (stringOption("-d", "+"))           layout_flag = 1;
		else if (stringOption("-d", "maxdepth"))    layout_flag = 1;
		else if (stringOption("-d", "++"))          layout_flag = 4;
		else if (stringOption("-d", "maxdepthslow"))layout_flag = 4;
		else if (stringOption("-d", "-"))           layout_flag = 2;
		else if (stringOption("-d", "mindepth"))    layout_flag = 2;
		else if (stringOption("-d", "--"))          layout_flag = 5;
		else if (stringOption("-d", "mindepthslow"))layout_flag = 5;
		else if (stringOption("-d", "minindeg"))    layout_flag = 6;
		else if (stringOption("-d", "minindegree")) layout_flag = 6;
		else if (stringOption("-d", "maxindeg"))    layout_flag = 7;
		else if (stringOption("-d", "maxindegree")) layout_flag = 7;
		else if (stringOption("-d", "minoutdeg"))   layout_flag = 8;
		else if (stringOption("-d", "minoutdegree"))layout_flag = 8;
		else if (stringOption("-d", "maxoutdeg"))   layout_flag = 9;
		else if (stringOption("-d", "maxoutdegree"))layout_flag = 9;
		else if (stringOption("-d", "mindeg"))      layout_flag = 10;
		else if (stringOption("-d", "mindegree"))   layout_flag = 10;
		else if (stringOption("-d", "maxdeg"))      layout_flag = 11;
		else if (stringOption("-d", "maxdegree"))   layout_flag = 11;
		else if (stringOption("-d", "tree")) layout_flag = TREE_LAYOUT;

		/* Options for the view */
		/* -------------------- */

		else if (stringOption("-view", "normal")) fisheye_view = 0;
		else if (stringOption("-view", "cfish"))  fisheye_view = CSCF_VIEW;
		else if (stringOption("-view", "pfish"))  fisheye_view = PSCF_VIEW;
		else if (stringOption("-view", "fcfish")) fisheye_view = FCSCF_VIEW;
		else if (stringOption("-view", "fpfish")) fisheye_view = FPSCF_VIEW;

		/* Options for the Printer driver */
		/* ------------------------------ */

		else if (fnameOption("-vcgoutput", exfilename)) {
			exfile = 1;
			extype = 0;
		}
		else if (fnameOption("-pbmoutput", exfilename)) {
			exfile = 1;
			extype = 1;
			excolor= 3;
		}
		else if (fnameOption("-ppmoutput", exfilename)) {
			exfile = 1;
			extype = 2;
			excolor= 1;
		}
		else if (fnameOption("-psoutput",  exfilename)) {
			exfile = 1;
			extype = 3;
		}
		else if (stringOption("-paper", "a4"))      expaper = 1;
		else if (stringOption("-paper", "b5"))      expaper = 2;
		else if (stringOption("-paper", "a5"))      expaper = 3;
		else if (stringOption("-paper", "11x17"))   expaper = 4;
		else if (stringOption("-paper", "tabloid")) expaper = 4;
		else if (stringOption("-paper", "8x11"))    expaper = 5;
		else if (stringOption("-paper", "letter"))  expaper = 5;
		else if (stringOption("-paper", "8x14"))    expaper = 6;
		else if (stringOption("-paper", "legal"))   expaper = 6;
		else if (simpleOption("-noboundingbox"))    exbbox  = 0; 
		else if (simpleOption("-color"))            excolor = 1; 
		else if (simpleOption("-colour"))           excolor = 1; 
		else if (simpleOption("-grey"))             excolor = 2; 
		else if (simpleOption("-gray"))             excolor = 2; 
		else if (simpleOption("-portrait"))         exori   = 1; 
		else if (simpleOption("-landscape"))        exori   = 2; 
		else if (   intOption("-split",&expapernum)) {
			if (expapernum>16)     expapernum = 5;
			else if (expapernum>9) expapernum = 4;
			else if (expapernum>4) expapernum = 3;
			else if (expapernum>1) expapernum = 2;
			else expapernum =  1;
		}
		else if (   intOption("-xdpi",   &exxdpi));
		else if (   intOption("-ydpi",   &exydpi));
		else if (   intOption("-scale",  &help))    
			exscaling = (float)help/100.0;
		else if (  unitOption("-width",  &exwidth));    
		else if (  unitOption("-height", &exheight));    
		else if (  unitOption("-lm",     &exleftm));    
		else if (  unitOption("-rm",     &exrightm));    
		else if (  unitOption("-tm",     &extopm));    
		else if (  unitOption("-bm",     &exbottomm));    
 

		/* Special X11 options  */
		/* -------------------- */
#ifdef X11
		else if (wordOption("-display",      &Xmydiplayname)); 
		else if (wordOption("-geometry",     &Xmygeometry)); 
		else if (wordOption("-bs",           &Xmybackingstore)); 
		else if (wordOption("-backingstore", &Xmybackingstore)); 
		else if (wordOption("-font",         &Xmyfontname)) {
			strncpy(Xfontname, Xmyfontname, 510);
			Xfontname[511] = 0;
		}
		else if ( intOption("-bw",           &Xmyborderwidth));
		else if ( intOption("-borderwidth",  &Xmyborderwidth));
		else if (simpleOption("-grabinputfocus")) 
				grabinputfocus = 1-grabinputfocus;
#endif

		/* Further general options  */
		/* ------------------------ */

		else if (simpleOption("-"))        opt_from_stdin = 1;
		else if (gblargv[gblargi][0]=='-') opt_error = 1;
		else {  strncpy(Dataname, gblargv[gblargi], 800 );
			gblargi++; 
			Dataname[800] = 0;
			if (opt_from_stdin) opt_error = 1;
			else return(1);
		}
		if (opt_error) return(0);
	}

	if (opt_from_stdin) strcpy(Dataname,"-");

    if (fastflag) {
		min_baryiterations = 0;
		max_baryiterations = 2;
		min_mediumshifts = 0;
		max_mediumshifts = 2;
		min_centershifts = 0;
		max_centershifts = 2;
		max_edgebendings = 2;
		max_straighttune = 2;
	}

	return(1);
}



/*--------------------------------------------------------------------*/
/*  Give Version, basic help or general help                          */
/*--------------------------------------------------------------------*/


/* Print the version and copyright notice
 * --------------------------------------
 */


#ifdef ANSI_C
void print_version_copyright(void)
#else
void print_version_copyright()
#endif
{
	if (!opt_give_version) return;

	PRINTF("VCG/XVCG - USAAR Visualization Tool %s\n",version_str);
	PRINTF("-----------------------------------------\n");
	PRINTF("%s %s\n",&(revision_str[1]),date_str);

	PRINTF("Copyright (C) 1993-1995 I.Lemke/G.Sander & the Compare Consortium.\n");
	PRINTF("This software is supported by the ESPRIT project 5399 Compare.\n");
	PRINTF("We thank the Compare Consortium for the permission to distribute\n");
	PRINTF("this software freely. VCG/XVCG comes with ABSOLUTELY NO WARRANTY.\n");
	PRINTF("You are welcome to redistribute XVCG under certain conditions.\n");
	PRINTF("See the file `COPYING' for details.\n");
	PRINTF("Contact  sander@cs.uni-sb.de  for additional information.\n\n");

	if (Dataname[0]) PRINTF("Input specification: %s\n\n", Dataname);
}


/* Print the basic help
 * --------------------
 */

#ifdef ANSI_C
void print_basic_help(void)
#else
void print_basic_help()
#endif
{
	PRINTF("USAAR Visualization Tool %s (C) 1993--1995 I.Lemke/G.Sander & Compare\n",
		version_str);
	PRINTF("%s %s\n",&(revision_str[1]),date_str);
	PRINTF("Usage:   %s [options] [filename]\n",gblargv[0]);	
	PRINTF("Enter %s -h  for help information.\n", gblargv[0]);
	exit(0);
}


/* Print the full help
 * -------------------
 */

#ifdef ANSI_C
void print_help(void)
#else
void print_help()
#endif
{
	if (!opt_give_help) return;

	PRINTF("USAAR Visualization Tool %s (C) 1993--1995 I.Lemke/G.Sander & Compare\n",
		version_str);

	PRINTF("%s %s\n\n",&(revision_str[1]),date_str);

	PRINTF("Usage:   %s [options] [filename]\n\n",gblargv[0]);	
	PRINTF("General options:\n");
	PRINTF("================\n");

	/* General Options */
	/* ----------------*/

	PRINTF("-                 The input is <stdin> instead of a file.\n");
	PRINTF("-h | -? | - help  Print help.\n");
	PRINTF("-v | -version     Print version and copyright.\n");
	PRINTF("-silent           Be silent during the layout.\n");
	PRINTF("-nocolors | -blackwhite\n");
	PRINTF("                  Don't use colors.\n");
	PRINTF("-e <num> | -error <num>\n");
	PRINTF("                  Maximal number of parse errors (default: 16).\n");
	PRINTF("-a <num> | -animation <num>\n");
	PRINTF("                  Animation handler. <num> > 0  means: touch input file after\n");
	PRINTF("                  successful visualization and then sleep for <num>  seconds.\n");
	PRINTF("                  <num> < 0  means: send signal USRSIG1  to the caller  after\n");
	PRINTF("                  successful visualization.\n");
	PRINTF("-m | -multi       Read multiple files one after another to display a sequence\n");
	PRINTF("                  of graphs.\n");

	PRINTF("-bary             Use bary centering as crossing reduction (default).\n");
	PRINTF("-median           Use median centerings as crossing reduction. On graphs with\n");
	PRINTF("                  large average degree of edges, bary centering is faster.\n");
	PRINTF("-barymedian       Use hybrid method as crossing reduction.  Bary centering is\n");
	PRINTF("                  used unless the bary values  are equal.  Only then,  median\n");
	PRINTF("                  centering is used.\n");
	PRINTF("-medianbary       Use hybrid method as crossing reduction.Median centering is\n");
	PRINTF("                  used unless the median values are equal.  Only  then,  bary\n");
	PRINTF("                  centering is used.\n");

	PRINTF("-notune | -nofinetune\n");
	PRINTF("                  Switch the fine tuning layout phase off.  This yield a less\n");
	PRINTF("                  compact distribution of nodes in the levels.\n");
	PRINTF("-manhattan | -orthogonal\n");
	PRINTF("                  Switch orthogonal manhattan layout on. This forces edges of\n");
	PRINTF("                  gradient 0 or 90 degree.\n");
	PRINTF("-nomanhattan | -noorthogonal\n");
	PRINTF("                  Switch orthogonal manhattan layout explicitly off.\n");
	PRINTF("-smanhattan       Use manhattan layout without separation  of horizontal line\n");
	PRINTF("                  segments.  This looks nice for trees but might be confusing\n");
	PRINTF("                  in general.\n");
	PRINTF("-nosmanhattan     Switch this phase explicitly off.\n");
	PRINTF("-prio             Switch priority layout on.  This forces straight long edges\n");
	PRINTF("                  with gradient 90 degree.\n");
	PRINTF("-noprio           Switch priority layout explicitly off.\n");
	PRINTF("-straight         Switch the straight line tuning phase on. This phase forces\n");
	PRINTF("                  straight long edges with gradient 90 degree. It can be used\n");
	PRINTF("                  to improve the priority layout.\n");
	PRINTF("-nostraight       Switch the straight line tuning phase explicitly off.\n");
	PRINTF("-nonearedge       Do not allow near edges in the layout.\n");
	PRINTF("-l | -latelabels  Create labels after partitioning of edges.  This may reduce\n");
	PRINTF("                  the number of crossings.\n");
	PRINTF("-hidesingles | -ignoresingles\n");
	PRINTF("                  Ignore single nodes (nodes without visible edges)  that are\n");
	PRINTF("                  not connected with the remaining graph.\n");
	PRINTF("-s | -summarize   Summarize duplicated edges.\n");

	PRINTF("\n");
	PRINTF("Options for the speed of the layout:\n");
	PRINTF("====================================\n");
	PRINTF("-t <num> | -timelimit <num>\n");
	PRINTF("                  Set the time limit to <num> seconds.   If the time limit is\n");
	PRINTF("                  exceeded,  the fastest possible layout mode is switched on.\n");
	PRINTF("                  This may yield a very ugly layout.\n");
	PRINTF("-f | -fast        Switch fast & dirty & ugly mode on.\n");
	PRINTF("                  This implies -bmax 2 -cmax 2 -pmax 2 -rmax 2 -smax 2.\n");
	PRINTF("-b <num> | -bmax <num> | -bending <num>\n");
	PRINTF("                  Maximal number of iterations used for the reduction of edge\n");
	PRINTF("                  bendings (default: 100).\n");
	PRINTF("-c <num> | -cmax <num> | -crossing <num>\n");
	PRINTF("                  Maximal number of iterations used for the reduction of edge\n");
	PRINTF("                  crossings (default: infinite).\n");
	PRINTF("-cmin <num>       Minimal number of iterations used for the reduction of edge\n");
	PRINTF("                  crossings (default: 0).\n");
	PRINTF("-p <num> | -pmax <num> | -pendulum <num>\n");
	PRINTF("                  Maximal number of iterations used for the balancing  by the\n");
	PRINTF("                  pendulum method (default: 100).\n");
	PRINTF("-pmin <num>       Minimal number of iterations used for the balancing  by the\n");
	PRINTF("                  pendulum method (default: 0).\n");
	PRINTF("-r <num> | -rmax <num> | -rubberband <num>\n");
	PRINTF("                  Maximal number of iterations used for the balancing  by the\n");
	PRINTF("                  rubberband method (default: 100).\n");
	PRINTF("-rmin <num>       Minimal number of iterations used for the balancing  by the\n");
	PRINTF("                  rubberband method (default: 0).\n");
	PRINTF("-smax <num>       Maximal number  of iterations used  for the  straight  line\n");
	PRINTF("                  tuning phase (default: 100).\n");
	PRINTF("-nocopt | -nocopt2\n");
	PRINTF("                  Skip optimization  phase 2  of crossing reduction.  This is\n");
	PRINTF("                  the most time consuming phase during crossing reduction.\n");
	PRINTF("-nocoptl | -nocoptloc\n");
	PRINTF("                  Switch local crossing optimization off. However, this phase\n");
	PRINTF("                  is normally not time critical.\n");


	PRINTF("\n");
	PRINTF("Options for the distribution of nodes in the layout:\n");
	PRINTF("====================================================\n");

	PRINTF("-d normal         Normal  distribution  of nodes  into the levels  (default).\n");
	PRINTF("                  This algorithm is based on  the calculation of the strongly\n");
	PRINTF("                  connected components.\n");
	PRINTF("-d dfs            Depth first search  distribution of nodes  into the levels.\n");
	PRINTF("                  This is the fastest method.\n");
	PRINTF("-d0 | -d minbackward\n");
	PRINTF("                  Reduce backward edges  heuristically.  This may  yield less\n");
	PRINTF("                  crossings and a uniform edge flow.\n");
	PRINTF("-d+ | -d maxdepth Maximize depth of the layout heuristically.\n");
	PRINTF("-d- | -d mindepth Minimize depth of the layout heuristically.\n");
	PRINTF("-d++ | -d maxdepthslow\n");
	PRINTF("                  Maximize depth of the layout (very slow).\n");
	PRINTF("-d-- | -d mindepthslow\n");
	PRINTF("                  Minimize depth of the layout (very slow).\n");
	PRINTF("-d minindeg | -d minindegree\n");
	PRINTF("                  Presort the nodes, i.e. lay out first the node with minimal\n");
	PRINTF("                  indegrees.\n");
	PRINTF("-d maxindeg | -d maxindegree\n");
	PRINTF("                  Presort the nodes, i.e. lay out first the node with maximal\n");
	PRINTF("                  indegrees.\n");
	PRINTF("-d minoutdeg | -d minoutdegree\n");
	PRINTF("                  Presort the nodes, i.e. lay out first the node with minimal\n");
	PRINTF("                  outdegrees.\n");
	PRINTF("-d maxoutdeg | -d maxoutdegree\n");
	PRINTF("                  Presort the nodes, i.e. lay out first the node with maximal\n");
	PRINTF("                  outdegrees.\n");
	PRINTF("-d mindeg | -d mindegree\n");
	PRINTF("                  Presort the nodes, i.e. lay out first the node with minimal\n");
	PRINTF("                  degrees.\n");
	PRINTF("-d maxdeg | -d maxdegree\n");
	PRINTF("                  Presort the nodes, i.e. lay out first the node with maximal\n");
	PRINTF("                  degrees.\n");
	PRINTF("-d tree           Use specialized layout  for trees.  This does not work with\n");
	PRINTF("                  non-trees.\n");

	PRINTF("\n");
	PRINTF("Options for the view onto the graph:\n");
	PRINTF("====================================\n");

	PRINTF("-view normal      Normal view on the graph. No distortion.\n");
	PRINTF("-view cfish       Cartesian fisheye view.  The graph is  distorted  such that\n");
	PRINTF("                  the whole graph is visible.  Horizontal  and vertical lines\n");
	PRINTF("                  don't change their direction.\n");
	PRINTF("-view fcfish      Cartesian fisheye view with fixed size focus.  The graph is\n");
	PRINTF("                  distorted, but not necessarily the whole graph is visible.\n");
	PRINTF("-view pfish       Polar fisheye view.  The graph is  distorted  such that the\n");
	PRINTF("                  whole graph is visible.  Even horizontal and vertical lines\n");
	PRINTF("                  might be distorted.\n");
	PRINTF("-view fpfish      Polar fisheye view  with  fixed size  focus.  The  graph is\n");
	PRINTF("                  distorted, but not necessarily the whole graph is visible.\n");
	PRINTF("-spline           Use splines instead of polygons to draw edges. WARNING: The\n");
	PRINTF("                  drawing of splines is very slow.\n");
	PRINTF("-nonodes          Suppress drawing of nodes.\n");
	PRINTF("-noedges          Suppress drawing of edges.\n");
	PRINTF("-xpos <num>       Set the x-coordinate of the initial point of the graph that\n");
	PRINTF("                  appears at the window origin or of the initial focus point.\n");
	PRINTF("-ypos <num>       Set the y-coordinate of the initial point of the graph that\n");
	PRINTF("                  appears at the window origin or of the initial focus point.\n");


	PRINTF("\n");
	PRINTF("Special printer driver options:\n");
	PRINTF("===============================\n");

	PRINTF("-vcgoutput <filename> \n");
	PRINTF("                  Produce a VCG file named <filename> that contains the graph\n");
	PRINTF("                  as laid out.  All nodes  of the graph are annotated  by the\n");
	PRINTF("                  calculated coordinates.In this case the interactive display\n");
	PRINTF("                  is suppressed.\n");
	PRINTF("-psoutput <filename> \n");
	PRINTF("                  Produce a PostScript file  named  <filename>  that contains\n");
	PRINTF("                  the  graph.   In  this  case  the  interactive  display  is\n");
	PRINTF("                  suppressed.\n");
	PRINTF("-pbmoutput <filename> \n");
	PRINTF("                  Produce a bitmap file named  <filename>  in PBM format that\n");
	PRINTF("                  contains the graph  in  black and white.  In this case  the\n");
	PRINTF("                  the interactive display is suppressed.\n");
	PRINTF("-ppmoutput <filename> \n");
	PRINTF("                  Produce a bitmap file named  <filename>  in PPM format that\n");
	PRINTF("                  contains the graph in colors.  In this case the interactive\n");
	PRINTF("                  display is suppressed.\n");
	PRINTF("-paper [ a4 | b5 | a5 | 11x17 | tabloid | 8x11 | letter | 8x14 | legal ]\n");
	PRINTF("                  Select the paper type. Default is din A4.\n");
	PRINTF("-noBoundingBox    Suppress the calculation  of a BoundingBox.  This is useful\n");
	PRINTF("                  only for PostScript output.\n");
	PRINTF("-color            Select a color file output (PostScript).\n");
	PRINTF("-grey | -gray     Select a gray-scaled file output (PostScript).\n");
	PRINTF("-blackwhite       Select a monochromatic file output (PostScript, default).\n");
	PRINTF("-portrait         Select the paper orientation `Portrait' (default).\n");
	PRINTF("-landscape        Select the paper orientation `Landscape'.\n");
	PRINTF("-split <num>      Split the graph  into  <num>  pages.  This  works  only for\n");
	PRINTF("                  PostScript output.\n");
	PRINTF("                  The number of pages must be one of 1,4,9,16,25.\n");
	PRINTF("-xdpi  <num>      Set the horizontal resolution for the bitmap output.\n");
	PRINTF("-ydpi  <num>      Set the vertical resolution for the bitmap output.\n");
	PRINTF("-scale <num>      Scale the graph to <num> percent  for the file output.  The\n");
	PRINTF("                  default scaling fits the graph with maximal aspect ratio to\n");
	PRINTF("                  the paper size.\n");
	PRINTF("-width <float> <units>\n");
	PRINTF("                  Fit the graph onto the paper such that its width is smaller\n");
	PRINTF("                  than <float> <units>. No scaling must be specified.\n");
	PRINTF("                  <units> are  inch | in | ft | foot | mm | cm | dm | m\n");
	PRINTF("-height <float> <units>\n");
	PRINTF("                  Fit the graph onto the paper  such that  height  is smaller\n");
	PRINTF("                  than <float> <units>. No scaling must be specified.\n");
	PRINTF("                  <units> are  inch | in | ft | foot | mm | cm | dm | m\n");
	PRINTF("-lm <float> <units>\n");
	PRINTF("                  Set the left  margin  of  the  output  to  <float> <units>.\n");
	PRINTF("                  No right margin must be specified. The default is centering\n");
	PRINTF("                  on the page.  If the graph  does not  fit  onto  the  page,\n");
	PRINTF("                  nonsense may happen.\n");
	PRINTF("                  <units> are  inch | in | ft | foot | mm | cm | dm | m\n");
	PRINTF("-rm <float> <units>\n");
	PRINTF("                  Set the right margin  of  the  output  to  <float> <units>.\n");
	PRINTF("                  No left margin must be specified.  The default is centering\n");
	PRINTF("                  on the page.  If the graph  does not  fit  onto  the  page,\n");
	PRINTF("                  nonsense may happen.\n");
	PRINTF("                  <units> are  inch | in | ft | foot | mm | cm | dm | m\n");
	PRINTF("-tm <float> <units>\n");
	PRINTF("                  Set the top   margin  of  the  output  to  <float> <units>.\n");
	PRINTF("                  No bottom margin must be specified.The default is centering\n");
	PRINTF("                  on the page.  If the graph  does not  fit  onto  the  page,\n");
	PRINTF("                  nonsense may happen.\n");
	PRINTF("                  <units> are  inch | in | ft | foot | mm | cm | dm | m\n");
	PRINTF("-bm <float> <units>\n");
	PRINTF("                  Set the bottom margin of  the  output  to  <float> <units>.\n");
	PRINTF("                  No  top  margin must be specified. The default is centering\n");
	PRINTF("                  on the page.  If the graph  does not  fit  onto  the  page,\n");
	PRINTF("                  nonsense may happen.\n");
	PRINTF("                  <units> are  inch | in | ft | foot | mm | cm | dm | m\n");

#ifdef X11
	PRINTF("\n");
	PRINTF("Special X11 options:\n");
	PRINTF("====================\n");

	PRINTF("-display <host:dpy>\n");
	PRINTF("                  Set the X11 server to contact.\n");
	PRINTF("-geometry <geom>  The size/location of the window.\n");
	PRINTF("-bw <num> | -borderwidth <num>\n");
	PRINTF("                  The border width of the window.\n");
	PRINTF("-font <xfont>     (default is `%s').\n",Xfontname);
	if (!grabinputfocus)
	PRINTF("-grabinputfocus   Grab the input focus actively.\n"); 
	else
	PRINTF("-grabinputfocus   Avoid to grab the input focus actively.\n"); 
#endif
	PRINTF("\n");
	exit(0);
}
