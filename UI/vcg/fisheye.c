/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         fisheye.c                                          */
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
 * Revision 1.3  1995/02/08  16:29:00  sander
 * Small bug with K&R C solved.
 *
 * Revision 1.2  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 1.1  1994/12/23  18:12:45  sander
 * Initial revision
 *
 */


/****************************************************************************
 * This file contains the different co-ordinate transformations used
 * to implement the different fisheye views, which are:
 *    1) a carthesian self adaptable continuous fisheye 
 *    2) a polar      self adaptable continuous fisheye 
 *    3) a carthesian fixed size continuous fisheye
 *    4) a polar      fixed size continuous fisheye
 *
 * Fisheye views are used to see a small region with a high scaling
 * while inspecting the remaining region with a low scaling.
 *
 * Note: the continuous fisheyes preserves bendings but do not
 * preserve crossings, i.e. the fisheye view may show more or less 
 * crossings than the normal view, due to speed requirements.
 * The noncontinous fisheyes preserve crossings but do not preserve
 * bendings, i.e. some originally straight edges may be shown with bendings.
 * The carthesian fisheyes do not preserve angles, while the polar
 * fisheyes preserves at least the angle between rays starting at the
 * focus point.
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "globals.h"
#include "grammar.h"
#include "main.h"
#include "options.h"
#include "alloc.h"
#include "steps.h"
#include "fisheye.h"

#undef DEBUG
#undef debugmessage
#ifdef DEBUG
#define debugmessage(a,b) {FPRINTF(stderr,"Debug: %s %s\n",a,b);}
#else
#define debugmessage(a,b) /**/
#endif


/* Prototypes
 * ----------
 */

static int  change_cscf_sfocus  _PP((void));
static int  change_cscf_gfocus  _PP((long gfx, long gfy));
static long cscf_sx_to_gx       _PP((int x));
static long cscf_sy_to_gy       _PP((int y));
static int  change_fcscf_gfocus _PP((long gfx, long gfy));
static void normalize_scaling   _PP((void));
static void normalize_fe_parameters _PP((void));
static int  change_pscf_sfocus  _PP((void));
static int  change_pscf_gfocus  _PP((long gfx, long gfy));
static void pscf_g_to_s _PP((long x, long y, int *resx, int *resy));


/* Global Variables
 * ----------------
 */

/* The fisheye view flag:
 *      0   =   normal flat view, no fisheye 
 *      1   =   CSNF = carthesian self adaptable noncontinuous (trapeze) fisheye
 *      2   =   CSCF = carthesian self adaptable continuous fisheye
 *      3   =   PSCF = polar      self adaptable continuous fisheye
 *      4   =   FCSCF= carthesian fixed continuous fisheye
 *      5   =   FPSCF= polar      fixed continuous fisheye
 */

int fisheye_view = 0;


/* The coordinate hash tables. They have a fixed size.
 * For the carthesian fisheye, they map the graph position to a screen
 * position.
 */

#define COORD_HASHSIZE 20000L

static short int *xcoord_hash = NULL;
static short int *ycoord_hash = NULL;

static long xhashmin, xhashmax;
static long yhashmin, yhashmax;


/* The minimal and maximal coordinates of the screen
 */

static int screen_xmin;
static int screen_xmax;
static int screen_ymin;
static int screen_ymax;

/* The coordinates of the focus point on the screen
 */

static int sfocus_x;
static int sfocus_y;

/* For the self adaptable fisheyes:
 * The minimal and maximal coordinates of the graph
 */

static long graph_xmin;
static long graph_xmax;
static long graph_ymin;
static long graph_ymax;

/* For the fisheye with fixed width:
 * the fisheye distance from the graph focus.
 */

long gfishdist = 500L;



/* The coordinates of the focus point in the graph
 */

long gfocus_x;
long gfocus_y;

/* The scaling factors in percent. fe_scaling = 100 means normal size.
 */

long fe_scaling;


/* Parameter that help a faster calculation of the coordinate
 * transformation.
 */

static double par_p;
static long par_xlp1;
static long par_xlp2;
static long par_xrp1;
static long par_xrp2;
static long par_yup1;
static long par_yup2;
static long par_ylp1;
static long par_ylp2;


/*--------------------------------------------------------------------*/
/* Carthesian self adaptable continuous fisheye                       */
/*--------------------------------------------------------------------*/

/* A cartesian self adaptable continuos fisheye transforms the
 * coordinates according to the following formula:
 *
 *              k * d           where d ist the distance to the focus in the
 *      f(d) = -----------      graph, k is the magnification, A is the
 *              A * d + 1       alignment and f(d) the resulting distance
 *                              to the focus on the screen.
 * Note:
 *                 k
 *     f'(d) = ------------- 
 *             (A * d + 1)^2
 *
 * Hence:
 *   f(0) = 0    i.e. the graph focus is transformed into the sceen focus
 *  f'(0) = k    i.e. the magnification at the focus is k
 *          
 * The alignment A is selected such that the whole graph is visible, i.e.
 *
 *                                      k            1
 *   f(d_max) = sd_max     i.e.   A = ------   -   -----
 *                                    sd_max       d_max
 *
 * where d_max is the maximal distance to the focus in the graph 
 * and  sd_max is the maximal distance to the focus on the sceen.     
 * The cartesion fisheye allows to have different sd_max, if we go from
 * the focus to the left, right, bottom, up.
 *
 * To avoid rounding errors, we use
 *
 *         k          1      par2       with  par2 = k * d_max - sd_max
 *   A = ------  -  -----  = ----
 *       sd_max     d_max    par1       and   par1 = d_max * sd_max
 *
 *
 *                        sd_max^2
 * Note that  f'(d_max) = -----------   is less than k, if k * d_max > sd_max
 *                        k * d_max^2   
 *
 * which means that in this case, the graph is shrinked at the borders of the
 * fisheye. The focus is visible at normal size, but the borders are 
 * distort such that the whole graph is visible.
 * 
 * The screen focus is self adaptable such that the relation of the position 
 * in the graph and the position of the screen are preserved,
 * i.e.
 *                         x - x_min
 *        sxf - s_min =  -------------  * (s_max - s_min)
 *                       x_max - x_min
 *
 * Implementation detail: we do not transfer straight lines into curves,
 * because this would need to much time. As result, the following properties
 * hold:
 *
 *    1) the fisheye preserve bendings 
 *    2) the fisheye does not preserve crossings
 *    3) the fisheye does not preserve angles, since it is a carthesian
 *       transformation.
 */



/*  Calculation of the auxiliary parameters
 *  ---------------------------------------
 *  A new calculation of the auxiliary parameters is necessary if
 *    a) the scaling changes.
 *    b) the position of the focus in the graph changes.
 *    c) the position of the focus in the window chnages.
 *    d) the size of the window changes.
 */

#ifdef ANSI_C
static void calc_cscf_parameters(void)
#else
static void calc_cscf_parameters()
#endif
{
	long s, t;
	register int i;
	short int k;

	debugmessage("calc_cscf_parameters","");

	par_xrp1 = (long)(screen_xmax-sfocus_x)*(graph_xmax-gfocus_x);
	par_xrp2 = fe_scaling*(graph_xmax-gfocus_x)
			- 100L*(long)(screen_xmax-sfocus_x);
	par_xlp1 = (long)(sfocus_x-screen_xmin)*(gfocus_x-graph_xmin);
	par_xlp2 = fe_scaling*(gfocus_x-graph_xmin)
			- 100L*(long)(sfocus_x-screen_xmin);
	par_ylp1 = (long)(screen_ymax-sfocus_y)*(graph_ymax-gfocus_y);
	par_ylp2 = fe_scaling*(graph_ymax-gfocus_y)
			- 100L*(long)(screen_ymax-sfocus_y);
	par_yup1 = (long)(sfocus_y-screen_ymin)*(gfocus_y-graph_ymin);
	par_yup2 = fe_scaling*(gfocus_y-graph_ymin)
			- 100L*(sfocus_y-screen_ymin);

	if (par_xrp1==0L) par_xrp1 = 1L;
	if (par_xlp1==0L) par_xlp1 = 1L;
	if (par_ylp1==0L) par_ylp1 = 1L;
	if (par_yup1==0L) par_yup1 = 1L;
	if (par_xrp2==0L) par_xrp2 = 1L;
	if (par_xlp2==0L) par_xlp2 = 1L;
	if (par_ylp2==0L) par_ylp2 = 1L;
	if (par_yup2==0L) par_yup2 = 1L;

	normalize_fe_parameters();

	if (xcoord_hash) {
		t = graph_xmax+1L;
		if (t>COORD_HASHSIZE) t = COORD_HASHSIZE;
		for (i=0; i<(int)t; i++) xcoord_hash[i]=(short)0;
		if (screen_xmin < screen_xmax)
			for (i=screen_xmin; i<=screen_xmax; i++) {
				s = cscf_sx_to_gx(i);
				if (s>=0 && s<COORD_HASHSIZE) 
					xcoord_hash[s] = (short)i;
			}
		k = xcoord_hash[0];
		for (i=0; i<(int)t; i++) {
			if (xcoord_hash[i]==(short)0) xcoord_hash[i] = k;
			else k = xcoord_hash[i];
		}
	}
	if (ycoord_hash) {
		t = graph_ymax+1;
		if (t>COORD_HASHSIZE) t = COORD_HASHSIZE;
		for (i=0; i<(int)t; i++) ycoord_hash[i]=(short)0;
		if (screen_ymin < screen_ymax)
			for (i=screen_ymin; i<=screen_ymax; i++) {
				s = cscf_sy_to_gy(i); 
				if (s>=0 && s<COORD_HASHSIZE) 
					ycoord_hash[s] = (short)i;
			}
		k = ycoord_hash[0];
		for (i=0; i<t; i++) {
			if (ycoord_hash[i]==(short)0) ycoord_hash[i] = k;
			else k = ycoord_hash[i];
		}
	}
	xhashmin = 0L;
	xhashmax = COORD_HASHSIZE-1L;
	yhashmin = 0L;
	yhashmax = COORD_HASHSIZE-1L;
}



/*  Initialization of the cscf_fisheye
 *  ----------------------------------
 */


#ifdef ANSI_C
static void init_cscf(int sxmin, int sxmax, int symin, int symax, 
		long gfx, long gfy)
#else
static void init_cscf(sxmin, sxmax, symin, symax, gfx, gfy)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
	long gfx;
	long gfy;
#endif
{
	debugmessage("init_cscf","");

	if ((sxmin!=0) || (sxmax!=0) || (symin!=0) || (symax!=0)) {
		screen_xmin = sxmin;
		screen_xmax = sxmax;
		screen_ymin = symin;
		screen_ymax = symax;
	}
	graph_xmin = 0L;
	graph_xmax = maximal_xpos + (long)G_xbase;
	graph_ymin = 0L;
	graph_ymax = maximal_ypos + (long)G_ybase;
	gfocus_x    = gfx;
	gfocus_y    = gfy;
	sfocus_x    = (int)((long)(screen_xmax-screen_xmin) 
			* (gfocus_x-graph_xmin) / (graph_xmax-graph_xmin));
	sfocus_y    = (int)((long)(screen_ymax-screen_ymin) 
			* (gfocus_y-graph_ymin) / (graph_ymax-graph_ymin));
	if (sfocus_x >= screen_xmax) sfocus_x = screen_xmax-5;
	if (sfocus_x <= screen_xmin) sfocus_x = screen_xmin+5;
	if (screen_xmax == screen_xmin+5) sfocus_x = screen_xmin+2;
	if (sfocus_y >= screen_ymax) sfocus_y = screen_ymax-5;
	if (sfocus_y <= screen_ymin) sfocus_y = screen_ymin+5;
	if (screen_ymax == screen_ymin+5) sfocus_y = screen_ymin+2;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	V_xmin = 0L;
	V_xmax = MAXLONG;
	V_ymin = 0L;
	V_ymax = MAXLONG;
	calc_cscf_parameters();
}


/*  Change scaling 
 *  --------------
 */

#ifdef ANSI_C
static void change_cscf_scaling(void)
#else
static void change_cscf_scaling()
#endif
{
	debugmessage("change_cscf_scaling","");

	calc_cscf_parameters();
}


/*  Change windowsize 
 *  -----------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_cscf_winsize(int sxmin, int sxmax, int symin, int symax)
#else
static int change_cscf_winsize(sxmin, sxmax, symin, symax)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
#endif
{
	int oldsxmin, oldsxmax, oldsymin, oldsymax;
	int ret;

	debugmessage("change_cscf_winsize","");

	ret = 0;
	oldsxmin = screen_xmin;
	oldsxmax = screen_xmax;
	oldsymin = screen_ymin;
	oldsymax = screen_ymax;
	screen_xmin = sxmin;
	screen_xmax = sxmax;
	screen_ymin = symin;
	screen_ymax = symax;
	ret += change_cscf_sfocus();
	if (oldsxmin != screen_xmin) return(1);
	if (oldsxmax != screen_xmax) return(1);
	if (oldsymin != screen_ymin) return(1);
	if (oldsymax != screen_ymax) return(1);
	return(ret);
}


/*  Change screen focus 
 *  -------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_cscf_sfocus(void)
#else
static int change_cscf_sfocus()
#endif
{
	int oldsfx, oldsfy;

	debugmessage("change_cscf_sfocus","");

	oldsfx = sfocus_x;
	oldsfy = sfocus_y;
	sfocus_x    = (int)((long)(screen_xmax-screen_xmin) 
			* (gfocus_x-graph_xmin) / (graph_xmax-graph_xmin));
	sfocus_y    = (int)((long)(screen_ymax-screen_ymin) 
			* (gfocus_y-graph_ymin) / (graph_ymax-graph_ymin));
	if (sfocus_x >= screen_xmax) sfocus_x = screen_xmax-5;
	if (sfocus_x <= screen_xmin) sfocus_x = screen_xmin+5;
	if (screen_xmax == screen_xmin+5) sfocus_x = screen_xmin+2;
	if (sfocus_y >= screen_ymax) sfocus_y = screen_ymax-5;
	if (sfocus_y <= screen_ymin) sfocus_y = screen_ymin+5;
	if (screen_ymax == screen_ymin+5) sfocus_y = screen_ymin+2;
	calc_cscf_parameters();
	if (oldsfx != sfocus_x) return(1);
	if (oldsfy != sfocus_y) return(1);
	return(0);
}


/*  Change graph focus 
 *  ------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_cscf_gfocus(long gfx, long gfy)
#else
static int change_cscf_gfocus(gfx, gfy)
	long gfx;
	long gfy;
#endif
{
	int ret;
	long oldgfx, oldgfy;

	debugmessage("change_cscf_gfocus","");

	ret = 0;
	oldgfx = gfocus_x;
	oldgfy = gfocus_y;
	gfocus_x    = gfx;
	gfocus_y    = gfy;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	ret += change_cscf_sfocus();
	if (oldgfx != gfocus_x) return(1);
	if (oldgfy != gfocus_y) return(1);
	return(ret);
}


/*  Increment graph focus 
 *  ---------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int incr_cscf_gfocus(long dfx, long dfy)
#else
static int incr_cscf_gfocus(dfx, dfy)
	long dfx;
	long dfy;
#endif
{
	debugmessage("incr_cscf_sfocus","");

	if ((dfx==0L) && (dfy==0L)) return(0);
	gfocus_x    += dfx;
	gfocus_y    += dfy;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	(void)change_cscf_sfocus();
	return(1);
}


/*  Translate a x,y-coord. of the graph into a x,y-coord. of the screen 
 *  -------------------------------------------------------------------
 */

#ifdef ANSI_C
static void cscf_g_to_s(long x, long y, int *resx, int *resy)
#else
static void cscf_g_to_s(x, y, resx, resy)
	long x, y;
	int  *resx, *resy;
#endif
{
	long dist, help;

	if ((xcoord_hash) && (x>=xhashmin) && (x<=xhashmax)) {
		/* NOTHING: this is handled in the entry point */;
	}
	else if (x > gfocus_x) {
		dist = x - gfocus_x;
		help = (par_xrp2 * dist * 10L) / par_xrp1 + 1000L; 
		if (help<=0L) help = MAXLONG;
		*resx = sfocus_x + 
			(int)((fe_scaling * dist * 10L) / help);
	}
	else {
		dist = gfocus_x - x;
		help = (par_xlp2 * dist * 10L) / par_xlp1 + 1000L; 
		if (help<=0L) help = MAXLONG;
		*resx = sfocus_x - 
			(int)((fe_scaling * dist * 10) / help);
	}

	if ((ycoord_hash) && (y>=yhashmin) && (y<=yhashmax)) {
		/* NOTHING: this is handled in the entry point */;
	}
	else if (y > gfocus_y) {
		dist = y - gfocus_y;
		help = (par_ylp2 * dist * 10L) / par_ylp1 + 1000L; 
		if (help<=0L) help = MAXLONG;
		*resy = sfocus_y + 
			(int)((fe_scaling * dist * 10) / help);
	}
	else {
		dist = gfocus_y - y;
		help = (par_yup2 * dist * 10L) / par_yup1 + 1000L; 
		if (help<=0L) help = MAXLONG;
		*resy = sfocus_y - 
			(int)((fe_scaling * dist * 10) / help);
	}
}



/*  Translate a x,y-coord. of the screen into a x,y-coord. of the graph 
 *  -------------------------------------------------------------------
 */

#ifdef ANSI_C
static void cscf_s_to_g(int x, int y, long *resx, long *resy)
#else
static void cscf_s_to_g(x, y, resx, resy)
	int x, y;
	long *resx, *resy;
#endif
{
	long dist, help;

	if (x > sfocus_x) {
		dist = (long)(x - sfocus_x);
		help = - par_xrp2 * dist + par_xrp1 * fe_scaling; 
		if (help!=0L)
			*resx = gfocus_x + (100L * dist * par_xrp1) / help;
		else    *resx = COORD_HASHSIZE;
	}
	else {
		dist = (long)(sfocus_x - x);
		help = - par_xlp2 * dist + par_xlp1 * fe_scaling; 
		if (help!=0L)
			*resx = gfocus_x - (100L * dist * par_xlp1) / help;
		else    *resx = COORD_HASHSIZE;
	}

	if (y > sfocus_y) {
		dist = (long)(y - sfocus_y);
		help = - par_ylp2 * dist + par_ylp1 * fe_scaling; 
		if (help!=0L)
			*resy = gfocus_y + (100L * dist * par_ylp1) / help;
		else	*resy = COORD_HASHSIZE;
	}
	else {
		dist = (long)(sfocus_y - y);
		help = - par_yup2 * dist + par_yup1 * fe_scaling; 
		if (help!=0L)
			*resy = gfocus_y - (100L * dist * par_yup1) / help;
		else 	*resy = COORD_HASHSIZE;
	}
}


/*  Translate a x-coordinate of the screen into a x-coordinate of the graph 
 *  -----------------------------------------------------------------------
 *  This is exactly as above, however only for the x component.
 */

#ifdef ANSI_C
static long cscf_sx_to_gx(int x)
#else
static long cscf_sx_to_gx(x)
	int x;
#endif
{
	long dist, help;

	if (x > sfocus_x) {
		dist = (long)(x - sfocus_x);
		help = - par_xrp2 * dist + par_xrp1 * fe_scaling; 
		if (help!=0L)
			return( gfocus_x + (100L * dist * par_xrp1) / help );
		return( COORD_HASHSIZE );
	}
	else {
		dist = (long)(sfocus_x - x);
		help = - par_xlp2 * dist + par_xlp1 * fe_scaling; 
		if (help!=0L)
			return( gfocus_x - (100L * dist * par_xlp1) / help );
		return( COORD_HASHSIZE );
	}
}


/*  Translate a y-coordinate of the screen into a y-coordinate of the graph 
 *  -----------------------------------------------------------------------
 *  This is exactly as above, however only for the y component.
 */

#ifdef ANSI_C
static long cscf_sy_to_gy(int y)
#else
static long cscf_sy_to_gy(y)
	int y;
#endif
{
	long dist, help;

	if (y > sfocus_y) {
		dist = (long)(y - sfocus_y);
		help = - par_ylp2 * dist + par_ylp1 * fe_scaling; 
		if (help!=0L)
			return( gfocus_y + (100L * dist * par_ylp1) / help );
		return( COORD_HASHSIZE );
	}
	else {
		dist = (long)(sfocus_y - y);
		help = - par_yup2 * dist + par_yup1 * fe_scaling; 
		if (help!=0L)
			return( gfocus_y - (100L * dist * par_yup1) / help );
		return( COORD_HASHSIZE );
	}
}


/*--------------------------------------------------------------------*/
/* Carthesian fixed width continuous fisheye                          */
/*--------------------------------------------------------------------*/

/* A carthesian continuos fisheye with a fixed radius of visible range.
 * For the formula, see the self adaptable continuous fisheye.
 * Now, we fisheye focus is always at the middle of the screen
 * and the maximal distance to the focus in the graph is gfishdist.
 * In the formula, d_max is now gfishdist.
 *
 *  with  par2 = k * gfishdist - sd_max
 *  and   par1 = gfishdist * sd_max
 *
 *
 * Implementation detail: we do not transfer straight lines into curves,
 * because this would need to much time. As result, the following properties
 * hold:
 *
 *    1) the fisheye preserve bendings 
 *    2) the fisheye does not preserve crossings
 *    3) the fisheye does not preserve angles, since it is a carthesian
 *       transformation.
 */



/*  Calculation of the auxiliary parameters
 *  ---------------------------------------
 *  A new calculation of the auxiliary parameters is necessary if
 *    a) the scaling changes.
 *    b) the position of the focus in the graph changes.
 *    c) the position of the focus in the window chnages.
 *    d) the size of the window changes.
 */


#ifdef ANSI_C
static void calc_fcscf_parameters(void)
#else
static void calc_fcscf_parameters()
#endif
{
	long s, t;
	register int i;
	short int k;

	debugmessage("calc_fcscf_parameters","");

	par_xrp1 = (long)(screen_xmax-sfocus_x) * gfishdist;
	par_xrp2 = fe_scaling * gfishdist
			- 100L*(long)(screen_xmax-sfocus_x);
	par_xlp1 = (long)(sfocus_x-screen_xmin) * gfishdist;
	par_xlp2 = fe_scaling * gfishdist
			- 100L*(long)(sfocus_x-screen_xmin);
	par_ylp1 = (long)(screen_ymax-sfocus_y) * gfishdist;
	par_ylp2 = fe_scaling * gfishdist
			- 100L*(long)(screen_ymax-sfocus_y);
	par_yup1 = (long)(sfocus_y-screen_ymin) * gfishdist;
	par_yup2 = fe_scaling * gfishdist
			- 100L*(sfocus_y-screen_ymin);

	if (par_xrp1==0L) par_xrp1 = 1L;
	if (par_xlp1==0L) par_xlp1 = 1L;
	if (par_ylp1==0L) par_ylp1 = 1L;
	if (par_yup1==0L) par_yup1 = 1L;
	if (par_xrp2<=0L) par_xrp2 = 1L;
	if (par_xlp2<=0L) par_xlp2 = 1L;
	if (par_ylp2<=0L) par_ylp2 = 1L;
	if (par_yup2<=0L) par_yup2 = 1L;

	normalize_fe_parameters();

	if (xcoord_hash) {
		t = graph_xmax+1L;
		if (t>COORD_HASHSIZE) t = COORD_HASHSIZE;
		for (i=0; i<(int)t; i++) xcoord_hash[i]=(short)0;
		for (i=0; i<=G_xmax; i++) {
			s = cscf_sx_to_gx(i);
			if (s>=0 && s<COORD_HASHSIZE) 
				xcoord_hash[s] = (short)i;
		}
		k = xcoord_hash[0];
		for (i=0; i<(int)t; i++) {
			if (xcoord_hash[i]==(short)0) xcoord_hash[i] = k;
			else k = xcoord_hash[i];
		}
		xhashmin = cscf_sx_to_gx(i);
		if (xhashmin < 0) xhashmin = 0;
		xhashmax = cscf_sx_to_gx(G_xmax);
		if (xhashmax>COORD_HASHSIZE-1)
			xhashmax = COORD_HASHSIZE-1;
	}
	if (ycoord_hash) {
		t = graph_ymax+1;
		if (t>COORD_HASHSIZE) t = COORD_HASHSIZE;
		for (i=0; i<(int)t; i++) ycoord_hash[i]=(short)0;
		i = 0;
		s = 0L;
		while ((s<COORD_HASHSIZE)&&(i<G_ymax)) {
			s = cscf_sy_to_gy(i); 
			if (s>=0 && s<COORD_HASHSIZE) 
				ycoord_hash[s] = (short)i;
			i++;
		}
		k = ycoord_hash[0];
		for (i=0; i<t; i++) {
			if (ycoord_hash[i]==(short)0) ycoord_hash[i] = k;
			else k = ycoord_hash[i];
		}
		yhashmin = cscf_sy_to_gy(i);
		if (yhashmin < 0) yhashmin = 0;
		yhashmax = cscf_sy_to_gy(G_ymax);
		if (yhashmax>COORD_HASHSIZE-1)
			yhashmax = COORD_HASHSIZE-1;
	}
}



/*  Initialization of the fcscf_fisheye
 *  -----------------------------------
 */


#ifdef ANSI_C
static void init_fcscf(int sxmin, int sxmax, int symin, int symax, 
		long gfx, long gfy)
#else
static void init_fcscf(sxmin, sxmax, symin, symax, gfx, gfy)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
	long gfx;
	long gfy;
#endif
{
	debugmessage("init_fcscf","");

	if ((sxmin!=0) || (sxmax!=0) || (symin!=0) || (symax!=0)) {
		screen_xmin = sxmin;
		screen_xmax = sxmax;
		screen_ymin = symin;
		screen_ymax = symax;
	}
	graph_xmin = 0L;
	graph_xmax = maximal_xpos + (long)G_xbase;
	graph_ymin = 0L;
	graph_ymax = maximal_ypos + (long)G_ybase;

	gfishdist = 2L * (long)(screen_xmax-screen_xmin);
	if (gfishdist<30L)  gfishdist = 30;

	sfocus_x    = (screen_xmax+screen_xmin)/2; 
	sfocus_y    = (screen_ymax+screen_ymin)/2;
	gfocus_x    = gfx;
	gfocus_y    = gfy;

	if (sfocus_x >= screen_xmax) sfocus_x = screen_xmax-5;
	if (sfocus_x <= screen_xmin) sfocus_x = screen_xmin+5;
	if (screen_xmax == screen_xmin+5) sfocus_x = screen_xmin+2;
	if (sfocus_y >= screen_ymax) sfocus_y = screen_ymax-5;
	if (sfocus_y <= screen_ymin) sfocus_y = screen_ymin+5;
	if (screen_ymax == screen_ymin+5) sfocus_y = screen_ymin+2;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	V_xmin = 0L;
	V_xmax = MAXLONG;
	V_ymin = 0L;
	V_ymax = MAXLONG;
	calc_fcscf_parameters();
}


/*  Change scaling 
 *  --------------
 */

#ifdef ANSI_C
static void change_fcscf_scaling(void)
#else
static void change_fcscf_scaling()
#endif
{
	debugmessage("change_fcscf_scaling","");

	calc_fcscf_parameters();
}


/*  Change windowsize 
 *  -----------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_fcscf_winsize(int sxmin, int sxmax, int symin, int symax)
#else
static int change_fcscf_winsize(sxmin, sxmax, symin, symax)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
#endif
{
	int oldsxmin, oldsxmax, oldsymin, oldsymax;

	debugmessage("change_fcscf_winsize","");

	oldsxmin = screen_xmin;
	oldsxmax = screen_xmax;
	oldsymin = screen_ymin;
	oldsymax = screen_ymax;
	screen_xmin = sxmin;
	screen_xmax = sxmax;
	screen_ymin = symin;
	screen_ymax = symax;
	sfocus_x    = (screen_xmax+screen_xmin)/2; 
	sfocus_y    = (screen_ymax+screen_ymin)/2;
	calc_fcscf_parameters();
	if (oldsxmin != screen_xmin) return(1);
	if (oldsxmax != screen_xmax) return(1);
	if (oldsymin != screen_ymin) return(1);
	if (oldsymax != screen_ymax) return(1);
	return(0);
}




/*  Change graph focus 
 *  ------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_fcscf_gfocus(long gfx, long gfy)
#else
static int change_fcscf_gfocus(gfx, gfy)
	long gfx;
	long gfy;
#endif
{
	long oldgfx, oldgfy;

	debugmessage("change_fcscf_gfocus","");

	oldgfx = gfocus_x;
	oldgfy = gfocus_y;
	gfocus_x    = gfx;
	gfocus_y    = gfy;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;

	/* The parameters don't change, but the hash tables may change */
	calc_fcscf_parameters();

	if (oldgfx != gfocus_x) return(1);
	if (oldgfy != gfocus_y) return(1);
	return(0);
}


/*  Increment graph focus 
 *  ---------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int incr_fcscf_gfocus(long dfx, long dfy)
#else
static int incr_fcscf_gfocus(dfx, dfy)
	long dfx;
	long dfy;
#endif
{
	debugmessage("incr_fcscf_sfocus","");

	if ((dfx==0L) && (dfy==0L)) return(0);
	gfocus_x    += dfx;
	gfocus_y    += dfy;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;

	/* The parameters don't change, but the hash tables may change */
	calc_fcscf_parameters();

	return(1);
}


/* Change the fisheye width
 * ------------------------
 * Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int set_fcscf_gfishdist(long gd)
#else
static int set_fcscf_gfishdist(gd)
long gd;
#endif
{
	int ret;

	debugmessage("set_fcscf_gfishdist","");

	if (gd<30L) return(0);

	ret = (gfishdist != gd);
	gfishdist = gd;
	calc_fcscf_parameters();
	return(ret);
}



/*--------------------------------------------------------------------*/
/* Polar self adaptable continuous fisheye                            */
/*--------------------------------------------------------------------*/

/* A polar self adaptable continuos fisheye preserves the angles of the
 * vectors starting at the focus point. It transforms the distance from
 * the focus point according to the following formula:
 *
 *              k * d           where d ist the distance to the focus in the
 *      f(d) = -----------      graph, k is the magnification, A is the
 *              A * d + 1       alignment and f(d) the resulting distance
 *                              to the focus on the screen.
 * Note:
 *                 k
 *     f'(d) = ------------- 
 *             (A * d + 1)^2
 *
 * Hence:
 *   f(0) = 0    i.e. the graph focus is transformed into the sceen focus
 *  f'(0) = k    i.e. the magnification at the focus is k
 *          
 * The alignment A is selected such that the whole graph is visible, i.e.
 *
 *                                      k            1
 *   f(d_max) = sd_max     i.e.   A = ------   -   -----
 *                                    sd_max       d_max
 *
 * where d_max is the maximal distance to the focus in the graph 
 * and  sd_max is the maximal distance to the focus on the sceen.     
 * The polar fisheye has only one d_max and sd_max. 
 * d_max is the minimal radius around the graph focus which encloses
 * the whole graph.
 * sdmax is the maximal radius around the screen focus such that this
 * circle is inside the screen. 
 *
 * To avoid rounding errors, we use
 *
 *         k          1      par2       with  par2 = k * d_max - sd_max
 *   A = ------  -  -----  = ----
 *       sd_max     d_max    par1       and   par1 = d_max * sd_max
 *
 *
 *                        sd_max^2
 * Note that  f'(d_max) = -----------   is less than k, if k * d_max > sd_max
 *                        k * d_max^2   
 *
 * which means that in this case, the graph is shrinked at the borders of the
 * fisheye. The focus is visible at normal size, but the borders are 
 * distort such that the whole graph is visible.
 * 
 * The screen focus is self adaptable such that whole graph is visible.
 * We cannot give a good formula for the problem of finding the screen
 * focus. The reason is: if we have a screen focus, we can calculate
 * par1 and par2 such that the left part is completely visible, or such
 * that the right part is completely visible. But if the right part is
 * completely visible, the left part may be clipped, i.e. not completely
 * visible, or the left part may have to much free space.
 * To avoid this, we simply try out the screen focus and select the 
 * position where the graph fits as best into the window.
 *
 * Implementation detail: we do not transfer straight lines into curves,
 * because this would need to much time. As result, the following properties
 * hold:
 *
 *    1) the fisheye preserve bendings 
 *    2) the fisheye does not preserve crossings
 *    3) the fisheye does not preserve angles. However, it preserves
 *       angles of rays starting at the focus point, since it is a polar 
 *       transformation.
 */



/*  Calculation of the auxiliary parameters
 *  ---------------------------------------
 *  A new calculation of the auxiliary parameters is necessary if
 *    a) the scaling changes.
 *    b) the position of the focus in the graph changes.
 *    c) the position of the focus in the window chnages.
 *    d) the size of the window changes.
 */

#ifdef ANSI_C
static void calc_pscf_parameters(void)
#else
static void calc_pscf_parameters()
#endif
{
	long s, t;
	int  x1, y1, diff, bestdiff, bestsf;
	double bestpar1, bestpar2;

	debugmessage("calc_pscf_parameters","");

	xhashmin = 0L;
	xhashmax = -1L;
	yhashmin = 0L;
	yhashmax = -1L;

	
	bestdiff = MAXINT;
	for (sfocus_x = screen_xmin+5; sfocus_x<screen_xmax-5; sfocus_x += 5) {
		diff = 0;
		s = (long)(screen_xmax-sfocus_x);
		t = graph_xmax - gfocus_x;

		par_xrp1 = s*t;
		par_xrp2 = fe_scaling*t - 100L*s;

		if (par_xrp1==0L) par_xrp1 = 1L;
		if (par_xrp2==0L) par_xrp2 = 1L;
		par_p = (double)par_xrp2 / (double)par_xrp1;
		pscf_g_to_s(graph_xmin, gfocus_y, &x1, &y1);
		if (x1<screen_xmin) diff += (screen_xmin-x1);
		else diff += (x1-screen_xmin);
		if (diff < bestdiff) {
			bestdiff = diff;
			bestpar1 = par_p;
			bestsf  = sfocus_x;
		}
	}
	sfocus_x = bestsf;

	bestdiff = MAXINT;
	for (sfocus_y = screen_ymin+5; sfocus_y<screen_ymax-5; sfocus_y += 5) {
		diff = 0;
		s = (long)(screen_ymax-sfocus_y);
		t = graph_ymax - gfocus_y;

		par_xrp1 = s*t;
		par_xrp2 = fe_scaling*t - 100L*s;

		if (par_xrp1==0L) par_xrp1 = 1L;
		if (par_xrp2==0L) par_xrp2 = 1L;
		par_p = (double)par_xrp2 / (double)par_xrp1;
		pscf_g_to_s(gfocus_x, graph_ymin, &x1, &y1);
		if (y1<screen_ymin) diff += (screen_ymin-y1);
		else diff += (y1-screen_ymin);
		if (diff < bestdiff) {
			bestdiff = diff;
			bestpar2 = par_p;
			bestsf   = sfocus_y;
		}
	}
	sfocus_y = bestsf;

	if (bestpar2>bestpar1) par_p = bestpar2;
	else par_p = bestpar1;
}



/*  Initialization of the pscf_fisheye
 *  ----------------------------------
 */


#ifdef ANSI_C
static void init_pscf(int sxmin, int sxmax, int symin, int symax, 
		long gfx, long gfy)
#else
static void init_pscf(sxmin, sxmax, symin, symax, gfx, gfy)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
	long gfx;
	long gfy;
#endif
{
	debugmessage("init_pscf","");

	if ((sxmin!=0) || (sxmax!=0) || (symin!=0) || (symax!=0)) {
		screen_xmin = sxmin;
		screen_xmax = sxmax;
		screen_ymin = symin;
		screen_ymax = symax;
	}
	graph_xmin = 0L;
	graph_xmax = maximal_xpos + (long)G_xbase;
	graph_ymin = 0L;
	graph_ymax = maximal_ypos + (long)G_ybase;
	gfocus_x    = gfx;
	gfocus_y    = gfy;
	sfocus_x    = (int)((long)(screen_xmax-screen_xmin) 
			* (gfocus_x-graph_xmin) / (graph_xmax-graph_xmin));
	sfocus_y    = (int)((long)(screen_ymax-screen_ymin) 
			* (gfocus_y-graph_ymin) / (graph_ymax-graph_ymin));
	if (sfocus_x >= screen_xmax) sfocus_x = screen_xmax-5;
	if (sfocus_x <= screen_xmin) sfocus_x = screen_xmin+5;
	if (screen_xmax == screen_xmin+5) sfocus_x = screen_xmin+2;
	if (sfocus_y >= screen_ymax) sfocus_y = screen_ymax-5;
	if (sfocus_y <= screen_ymin) sfocus_y = screen_ymin+5;
	if (screen_ymax == screen_ymin+5) sfocus_y = screen_ymin+2;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	V_xmin = 0L;
	V_xmax = MAXLONG;
	V_ymin = 0L;
	V_ymax = MAXLONG;
	calc_pscf_parameters();
}


/*  Change scaling 
 *  --------------
 */

#ifdef ANSI_C
static void change_pscf_scaling(void)
#else
static void change_pscf_scaling()
#endif
{
	debugmessage("change_pscf_scaling","");

	calc_pscf_parameters();
}


/*  Change windowsize 
 *  -----------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_pscf_winsize(int sxmin, int sxmax, int symin, int symax)
#else
static int change_pscf_winsize(sxmin, sxmax, symin, symax)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
#endif
{
	int oldsxmin, oldsxmax, oldsymin, oldsymax;
	int ret;

	debugmessage("change_pscf_winsize","");

	ret = 0;
	oldsxmin = screen_xmin;
	oldsxmax = screen_xmax;
	oldsymin = screen_ymin;
	oldsymax = screen_ymax;
	screen_xmin = sxmin;
	screen_xmax = sxmax;
	screen_ymin = symin;
	screen_ymax = symax;
	ret += change_pscf_sfocus();
	if (oldsxmin != screen_xmin) return(1);
	if (oldsxmax != screen_xmax) return(1);
	if (oldsymin != screen_ymin) return(1);
	if (oldsymax != screen_ymax) return(1);
	return(ret);
}


/*  Change screen focus 
 *  -------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_pscf_sfocus(void)
#else
static int change_pscf_sfocus()
#endif
{
	int oldsfx, oldsfy;

	debugmessage("change_pscf_sfocus","");

	oldsfx = sfocus_x;
	oldsfy = sfocus_y;
	sfocus_x    = (int)((long)(screen_xmax-screen_xmin) 
			* (gfocus_x-graph_xmin) / (graph_xmax-graph_xmin));
	sfocus_y    = (int)((long)(screen_ymax-screen_ymin) 
			* (gfocus_y-graph_ymin) / (graph_ymax-graph_ymin));
	if (sfocus_x >= screen_xmax) sfocus_x = screen_xmax-5;
	if (sfocus_x <= screen_xmin) sfocus_x = screen_xmin+5;
	if (screen_xmax == screen_xmin+5) sfocus_x = screen_xmin+2;
	if (sfocus_y >= screen_ymax) sfocus_y = screen_ymax-5;
	if (sfocus_y <= screen_ymin) sfocus_y = screen_ymin+5;
	if (screen_ymax == screen_ymin+5) sfocus_y = screen_ymin+2;
	calc_pscf_parameters();
	if (oldsfx != sfocus_x) return(1);
	if (oldsfy != sfocus_y) return(1);
	return(0);
}


/*  Change graph focus 
 *  ------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_pscf_gfocus(long gfx, long gfy)
#else
static int change_pscf_gfocus(gfx, gfy)
	long gfx;
	long gfy;
#endif
{
	int ret;
	long oldgfx, oldgfy;

	debugmessage("change_pscf_gfocus","");

	ret = 0;
	oldgfx = gfocus_x;
	oldgfy = gfocus_y;
	gfocus_x    = gfx;
	gfocus_y    = gfy;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	ret += change_pscf_sfocus();
	if (oldgfx != gfocus_x) return(1);
	if (oldgfy != gfocus_y) return(1);
	return(ret);
}


/*  Increment graph focus 
 *  ---------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int incr_pscf_gfocus(long dfx, long dfy)
#else
static int incr_pscf_gfocus(dfx, dfy)
	long dfx;
	long dfy;
#endif
{
	debugmessage("incr_pscf_sfocus","");

	if ((dfx==0L) && (dfy==0L)) return(0);
	gfocus_x    += dfx;
	gfocus_y    += dfy;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	(void)change_pscf_sfocus();
	return(1);
}


/*  Translate a x,y-coordinate of the graph into a x,y-coordinate of the screen 
 *  ---------------------------------------------------------------------------
 */

#ifdef ANSI_C
static void pscf_g_to_s(long x, long y, int *resx, int *resy)
#else
static void pscf_g_to_s(x, y, resx, resy)
	long x, y;
	int *resx, *resy;
#endif
{
	float xdiff, ydiff, dist1, dist2, help;

	xdiff = (float)(x - gfocus_x);
	ydiff = (float)(y - gfocus_y);
	dist1 = sqrt(xdiff*xdiff + ydiff*ydiff);

	help = par_p * dist1 + 100.0;
	if (help<=0.0) help = 0.1;
	dist2 = (double)fe_scaling * dist1 / help;

	*resx = sfocus_x + (int)(xdiff*dist2/dist1);
	*resy = sfocus_y + (int)(ydiff*dist2/dist1);
}



/*  Translate a x,y-coordinate of the screen into a x,y-coordinate of the graph 
 *  ---------------------------------------------------------------------------
 */

#ifdef ANSI_C
static void pscf_s_to_g(int x, int y, long *resx, long *resy)
#else
static void pscf_s_to_g(x, y, resx, resy)
	int x, y;
	long *resx, *resy;
#endif
{
	float xdiff, ydiff, dist1, dist2, help;

	xdiff = (float)(x - sfocus_x);
	ydiff = (float)(y - sfocus_y);
	dist2 = sqrt(xdiff*xdiff + ydiff*ydiff);

	help = - par_p * dist2 + fe_scaling;
	if (help<=0.0) help = 0.1;
	dist1 = 100.0 * dist2 / help;

	*resx = gfocus_x + (long)(xdiff*dist1/dist2);
	*resy = gfocus_y + (long)(ydiff*dist1/dist2);
}



/*--------------------------------------------------------------------*/
/* Polar fixed width continuous fisheye                               */
/*--------------------------------------------------------------------*/

/* A polar continuos fisheye with a fixed radius of visible range.
 * For the formula, see the self adaptable continuous fisheye.
 * Now, we fisheye focus is always at the middle of the screen
 * and the maximal distance to the focus in the graph is gfishdist.
 * In the formula, d_max is now gfishdist.
 *
 *  with  par2 = k * gfishdist - sd_max
 *  and   par1 = gfishdist * sd_max
 *
 * Implementation detail: we do not transfer straight lines into curves,
 * because this would need to much time. As result, the following properties
 * hold:
 *
 *    1) the fisheye preserve bendings 
 *    2) the fisheye does not preserve crossings
 *    3) the fisheye does not preserve angles. However, it preserves
 *       angles of rays starting at the focus point, since it is a polar 
 *       transformation.
 */



/*  Calculation of the auxiliary parameters
 *  ---------------------------------------
 *  A new calculation of the auxiliary parameters is necessary if
 *    a) the scaling changes.
 *    b) the position of the focus in the graph changes.
 *    c) the position of the focus in the window chnages.
 *    d) the size of the window changes.
 */

#ifdef ANSI_C
static void calc_fpscf_parameters(void)
#else
static void calc_fpscf_parameters()
#endif
{
	long s, t;

	debugmessage("calc_fpscf_parameters","");

	xhashmin = 0L;
	xhashmax = -1L;
	yhashmin = 0L;
	yhashmax = -1L;

	s = (long)(screen_xmax-sfocus_x);
	t = gfishdist;

	par_xrp1 = s*t;
	par_xrp2 = fe_scaling*t - 100L*s;

	if (par_xrp1==0L) par_xrp1 = 1L;
	if (par_xrp2<=0L) par_xrp2 = 1L;
	par_p = (double)par_xrp2 / (double)par_xrp1;

}



/*  Initialization of the fpscf_fisheye
 *  -----------------------------------
 */


#ifdef ANSI_C
static void init_fpscf(int sxmin, int sxmax, int symin, int symax, 
		long gfx, long gfy)
#else
static void init_fpscf(sxmin, sxmax, symin, symax, gfx, gfy)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
	long gfx;
	long gfy;
#endif
{
	debugmessage("init_fpscf","");

	if ((sxmin!=0) || (sxmax!=0) || (symin!=0) || (symax!=0)) {
		screen_xmin = sxmin;
		screen_xmax = sxmax;
		screen_ymin = symin;
		screen_ymax = symax;
	}
	graph_xmin = 0L;
	graph_xmax = maximal_xpos + (long)G_xbase;
	graph_ymin = 0L;
	graph_ymax = maximal_ypos + (long)G_ybase;

        gfishdist = 2L * (long)(screen_xmax-screen_xmin);
        if (gfishdist<30L)  gfishdist = 30;

	sfocus_x    = (screen_xmax+screen_xmin)/2;
        sfocus_y    = (screen_ymax+screen_ymin)/2;
	gfocus_x    = gfx;
	gfocus_y    = gfy;
	if (sfocus_x >= screen_xmax) sfocus_x = screen_xmax-5;
	if (sfocus_x <= screen_xmin) sfocus_x = screen_xmin+5;
	if (screen_xmax == screen_xmin+5) sfocus_x = screen_xmin+2;
	if (sfocus_y >= screen_ymax) sfocus_y = screen_ymax-5;
	if (sfocus_y <= screen_ymin) sfocus_y = screen_ymin+5;
	if (screen_ymax == screen_ymin+5) sfocus_y = screen_ymin+2;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	V_xmin = 0L;
	V_xmax = MAXLONG;
	V_ymin = 0L;
	V_ymax = MAXLONG;
	calc_fpscf_parameters();
}


/*  Change scaling 
 *  --------------
 */

#ifdef ANSI_C
static void change_fpscf_scaling(void)
#else
static void change_fpscf_scaling()
#endif
{
	debugmessage("change_fpscf_scaling","");

	calc_fpscf_parameters();
}


/*  Change windowsize 
 *  -----------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_fpscf_winsize(int sxmin, int sxmax, int symin, int symax)
#else
static int change_fpscf_winsize(sxmin, sxmax, symin, symax)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
#endif
{
	int oldsxmin, oldsxmax, oldsymin, oldsymax;

	debugmessage("change_fpscf_winsize","");

	oldsxmin = screen_xmin;
	oldsxmax = screen_xmax;
	oldsymin = screen_ymin;
	oldsymax = screen_ymax;
	screen_xmin = sxmin;
	screen_xmax = sxmax;
	screen_ymin = symin;
	screen_ymax = symax;
	calc_fpscf_parameters();
	if (oldsxmin != screen_xmin) return(1);
	if (oldsxmax != screen_xmax) return(1);
	if (oldsymin != screen_ymin) return(1);
	if (oldsymax != screen_ymax) return(1);
	return(0);
}


/*  Change graph focus 
 *  ------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int change_fpscf_gfocus(long gfx, long gfy)
#else
static int change_fpscf_gfocus(gfx, gfy)
	long gfx;
	long gfy;
#endif
{
	long oldgfx, oldgfy;

	debugmessage("change_fpscf_gfocus","");

	oldgfx = gfocus_x;
	oldgfy = gfocus_y;
	gfocus_x    = gfx;
	gfocus_y    = gfy;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	calc_fpscf_parameters();
	if (oldgfx != gfocus_x) return(1);
	if (oldgfy != gfocus_y) return(1);
	return(0);
}


/*  Increment graph focus 
 *  ---------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int incr_fpscf_gfocus(long dfx, long dfy)
#else
static int incr_fpscf_gfocus(dfx, dfy)
	long dfx;
	long dfy;
#endif
{
	debugmessage("incr_fpscf_sfocus","");

	if ((dfx==0L) && (dfy==0L)) return(0);
	gfocus_x    += dfx;
	gfocus_y    += dfy;
	if (gfocus_x >= graph_xmax) gfocus_x = graph_xmax-5L;
	if (gfocus_x <= graph_xmin) gfocus_x = graph_xmin+5L;
	if (graph_xmax == graph_xmin+5L) gfocus_x = graph_xmin+2L;
	if (gfocus_y >= graph_ymax) gfocus_y = graph_ymax-5L;
	if (gfocus_y <= graph_ymin) gfocus_y = graph_ymin+5L;
	if (graph_ymax == graph_ymin+5L) gfocus_y = graph_ymin+2L;
	calc_fpscf_parameters(); /* not necessary ? */
	return(1);
}


/* Change the fisheye width
 * ------------------------
 * Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
static int set_fpscf_gfishdist(long gd)
#else
static int set_fpscf_gfishdist(gd)
long gd;
#endif
{
	int ret;

	debugmessage("set_fpscf_gfishdist","");

	if (gd<30L) return(0);

	ret = (gfishdist != gd);
	gfishdist = gd;
	calc_fpscf_parameters();
	return(ret);
}

/*--------------------------------------------------------------------*/
/* Entry-Points for the different fisheyes                            */
/*--------------------------------------------------------------------*/

/*  Initialization of the fisheye
 *  -----------------------------
 */


#ifdef ANSI_C
void init_fe(int sxmin, int sxmax, int symin, int symax, 
		int sfx, int sfy)
#else
void init_fe(sxmin, sxmax, symin, symax, sfx, sfy)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
	int sfx;
	int sfy;
#endif
{
	long gfx, gfy;

	debugmessage("init_fe","");

	fe_scaling = (long)(G_stretch * 100 / G_shrink);
	if (fe_scaling<=0L)    fe_scaling = 1L;
	if (fe_scaling>=5000L) fe_scaling = 5000L;
	gfx = V_xmin + (long)(sxmax-sxmin)/2L;
	gfy = V_ymin + (long)(symax-symin)/2L;

	if (!xcoord_hash) 
		xcoord_hash = (short *)malloc(COORD_HASHSIZE*sizeof(short));
	if (!ycoord_hash) 
		ycoord_hash = (short *)malloc(COORD_HASHSIZE*sizeof(short));

	switch (fisheye_view) {
	case CSCF_VIEW: 
			init_cscf(sxmin,sxmax,symin,symax,gfx,gfy);
			G_stretch = G_shrink = 1;
			V_xmin = 0L;
			V_xmax = MAXLONG;
			return;
	case FCSCF_VIEW: 
			init_fcscf(sxmin,sxmax,symin,symax,gfx,gfy);
			G_stretch = G_shrink = 1;
			V_xmin = 0L;
			V_xmax = MAXLONG;
			return;
	case PSCF_VIEW: 
			init_pscf(sxmin,sxmax,symin,symax,gfx,gfy);
			G_stretch = G_shrink = 1;
			V_xmin = 0L;
			V_xmax = MAXLONG;
			return;
	case FPSCF_VIEW: 
			init_fpscf(sxmin,sxmax,symin,symax,gfx,gfy);
			G_stretch = G_shrink = 1;
			V_xmin = 0L;
			V_xmax = MAXLONG;
			return;
	} 
	fisheye_view = 0;

}

/* Exit a fisheye view 
 * -------------------
 */

#ifdef ANSI_C
void exit_fe(void)
#else
void exit_fe()
#endif
{
	debugmessage("exit_fe","");

	if (fisheye_view==0) return; 
	G_shrink = 100;
	G_stretch = fe_scaling;
	V_xmin = gfocus_x - (long)(screen_xmax-screen_xmin)/2; 
	V_ymin = gfocus_y - (long)(screen_ymax-screen_ymin)/2; 
	V_xmax = V_xmin + (long)G_xmax;
	V_ymax = V_ymin + (long)(G_ymax + COFFSET);
	fisheye_view = 0;
} 



/*  Translate a x,y-coord. of the graph into a x,y-coord. of the screen 
 *  -------------------------------------------------------------------
 */


#ifdef ANSI_C
void fe_g_to_s(long x, long y, int *resx, int *resy)
#else
void fe_g_to_s(x, y, resx, resy)
	long x, y;
	int *resx, *resy;
#endif
{
	int have_x = 0;

	if ((xcoord_hash) && (x>=xhashmin) && (x<=xhashmax)) {
			*resx = (int)xcoord_hash[x]; 
			have_x = 1;
	}
	if ((ycoord_hash) && (y>=yhashmin) && (y<=yhashmax)) {
			*resy = (int)ycoord_hash[y]; 
			if (have_x) return;
	}

	switch (fisheye_view) {
	case FCSCF_VIEW: 
	case CSCF_VIEW:  cscf_g_to_s(x,y,resx,resy); return;
	case FPSCF_VIEW: 
	case PSCF_VIEW:  pscf_g_to_s(x,y,resx,resy); return;
	} 
	*resx = (int)x;
	*resy = (int)y;
}


/*  Translate a x,y-coord. of the screen into a x,y-coord. of the graph 
 *  -------------------------------------------------------------------
 */

#ifdef ANSI_C
void fe_s_to_g(int x, int y, long *resx, long *resy)
#else
void fe_s_to_g(x, y, resx, resy)
	int x, y;
	long *resx, *resy;
#endif
{
	switch (fisheye_view) {
	case FCSCF_VIEW: 
	case CSCF_VIEW: cscf_s_to_g(x,y,resx,resy); return;
	case FPSCF_VIEW: 
	case PSCF_VIEW: pscf_s_to_g(x,y,resx,resy); return;
	} 
	*resx = (long)x;
	*resy = (long)y;
}


/*  Set scaling 
 *  -----------
 */

#ifdef ANSI_C
void set_fe_scaling(int stretch, int shrink)
#else
void set_fe_scaling(stretch, shrink)
	int stretch, shrink;
#endif
{
	long xx, yy;

	debugmessage("set_fe_scaling","");

	if (shrink==0) fe_scaling = 100L;
	else fe_scaling = (long)(stretch * 100 / shrink);
	if (fe_scaling<=0L)    fe_scaling = 1L;
	if (fe_scaling>=5000L) fe_scaling = 5000L;

	switch (fisheye_view) {
	case CSCF_VIEW:  change_cscf_scaling();  return;
	case FCSCF_VIEW: change_fcscf_scaling(); return;
	case PSCF_VIEW:  change_pscf_scaling();  return;
	case FPSCF_VIEW: change_fpscf_scaling();  return;
	} 
	xx = V_xmin * (long)G_shrink/(long)G_stretch;
        yy = V_ymin * (long)G_shrink/(long)G_stretch;
	G_stretch = stretch;
	G_shrink  = shrink;
	normalize_scaling();
        V_xmin = xx * (long)G_stretch/(long)G_shrink;
        V_ymin = yy * (long)G_stretch/(long)G_shrink;
        V_xmax = V_xmin+(long)G_xmax;
        V_ymax = V_ymin+(long)(G_ymax+COFFSET);
}


/*  Change scaling relatively
 *  -------------------------
 */

#ifdef ANSI_C
void change_fe_scaling(int stretch, int shrink)
#else
void change_fe_scaling(stretch, shrink)
	int stretch, shrink;
#endif
{
	long xx, yy;

	debugmessage("change_fe_scaling","");

	if (shrink==0) fe_scaling = 100L;
	else fe_scaling = fe_scaling * (long)stretch / (long)shrink;
	if (fe_scaling<=0L)    fe_scaling = 1L;
	if (fe_scaling>=5000L) fe_scaling = 5000L;
	switch (fisheye_view) {
	case CSCF_VIEW:  change_cscf_scaling();  return;
	case FCSCF_VIEW: change_fcscf_scaling(); return;
	case PSCF_VIEW:  change_pscf_scaling();  return;
	case FPSCF_VIEW: change_fpscf_scaling();  return;
	} 
	
	xx = V_xmin * (long)G_shrink/(long)G_stretch;
        yy = V_ymin * (long)G_shrink/(long)G_stretch;
	G_stretch = G_stretch * stretch;
	G_shrink  = G_shrink  * shrink;
	normalize_scaling();
        V_xmin = xx * (long)G_stretch/(long)G_shrink;
        V_ymin = yy * (long)G_stretch/(long)G_shrink;
        V_xmax = V_xmin+(long)G_xmax;
        V_ymax = V_ymin+(long)(G_ymax+COFFSET);
}



/*  Set graph focus to the normal position 
 *  --------------------------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
int normal_fe_focus(void)
#else
int normal_fe_focus()
#endif
{
	int ret;
	long dfx, dfy;

	debugmessage("normal_fe_focus","");

	dfx = (graph_xmin + graph_xmax)/2L;
	dfy = (graph_ymin + graph_ymax)/2L;
	switch (fisheye_view) {
	case CSCF_VIEW:  return(change_cscf_gfocus(dfx, dfy));
	case FCSCF_VIEW: return(change_fcscf_gfocus(dfx, dfy));
	case PSCF_VIEW:  return(change_pscf_gfocus(dfx, dfy));
	case FPSCF_VIEW: return(change_fpscf_gfocus(dfx, dfy));
	} 
	ret = 0;
	if (V_xmin!=0L) ret = 1;
	if (V_ymin!=0L) ret = 1;
        V_xmin = 0L;
        V_ymin = 0L;
        V_xmax = V_xmin+(long)G_xmax;
        V_ymax = V_ymin+(long)(G_ymax+COFFSET);
	return(ret);
}


/*  Set graph focus x-coordinate
 *  ----------------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
int set_fe_xfocus(long fx)
#else
int set_fe_xfocus(fx)
	long fx;
#endif
{
	int ret;

	debugmessage("set_fe_xfocus","");

	switch (fisheye_view) {
	case CSCF_VIEW:  return(change_cscf_gfocus(fx, gfocus_y));
	case FCSCF_VIEW: return(change_fcscf_gfocus(fx, gfocus_y));
	case PSCF_VIEW:  return(change_pscf_gfocus(fx, gfocus_y));
	case FPSCF_VIEW: return(change_fpscf_gfocus(fx, gfocus_y));
	} 
	ret = 0;
	if (V_xmin!=fx) ret = 1;
        V_xmin = fx;
        V_xmax = V_xmin+(long)G_xmax;
	return(ret);
}


/*  Set graph focus y-coordinate
 *  ----------------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
int set_fe_yfocus(long fy)
#else
int set_fe_yfocus(fy)
	long fy;
#endif
{
	int ret;

	debugmessage("set_fe_yfocus","");

	switch (fisheye_view) {
	case CSCF_VIEW:  return(change_cscf_gfocus(gfocus_x,fy));
	case FCSCF_VIEW: return(change_fcscf_gfocus(gfocus_x,fy));
	case PSCF_VIEW:  return(change_pscf_gfocus(gfocus_x,fy));
	case FPSCF_VIEW: return(change_fpscf_gfocus(gfocus_x,fy));
	} 
	ret = 0;
	if (V_ymin!=fy) ret = 1;
        V_ymin = fy;
        V_ymax = V_ymin+(long)(G_ymax+COFFSET);
	return(ret);
}



/*  Increment graph focus  
 *  ---------------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
int incr_fe_focus(long dfx, long dfy)
#else
int incr_fe_focus(dfx, dfy)
	long dfx, dfy;
#endif
{
	debugmessage("incr_fe_focus","");

	switch (fisheye_view) {
	case CSCF_VIEW:  return(incr_cscf_gfocus(dfx, dfy));
	case FCSCF_VIEW: return(incr_fcscf_gfocus(dfx, dfy));
	case PSCF_VIEW:  return(incr_pscf_gfocus(dfx, dfy));
	case FPSCF_VIEW: return(incr_fpscf_gfocus(dfx, dfy));
	} 
        V_xmin += dfx;
        V_ymin += dfy;
        V_xmax = V_xmin+(long)G_xmax;
        V_ymax = V_ymin+(long)(G_ymax+COFFSET);
	if ((dfx==0L) && (dfy==0L)) return(0);
	return(1);
}



/*  Change windowsize 
 *  -----------------
 *  Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
int change_fe_winsize(int sxmin, int sxmax, int symin, int symax)
#else
int change_fe_winsize(sxmin, sxmax, symin, symax)
	int sxmin;
	int sxmax;
	int symin;
	int symax;
#endif
{
	switch (fisheye_view) {
	case CSCF_VIEW:  return(change_cscf_winsize(sxmin,sxmax,symin,symax));
	case FCSCF_VIEW: return(change_fcscf_winsize(sxmin,sxmax,symin,symax));
	case PSCF_VIEW:  return(change_pscf_winsize(sxmin,sxmax,symin,symax));
	case FPSCF_VIEW: return(change_fpscf_winsize(sxmin,sxmax,symin,symax));
	} 
	return(0);
}


/* Change the fisheye width
 * ------------------------
 * Returns 1, if a redraw is necessary.
 */

#ifdef ANSI_C
int set_gfishdist(long gd)
#else
int set_gfishdist(gd)
long gd;
#endif
{
	switch (fisheye_view) {
	case FCSCF_VIEW: return(set_fcscf_gfishdist(gd));
	case FPSCF_VIEW: return(set_fpscf_gfishdist(gd)); 
	}
	return(0);
}


/*--------------------------------------------------------------------*/
/*  Normalize G_shrink and G_stretch                                  */
/*--------------------------------------------------------------------*/

/* This is done to minimize rounding errors if to often 
 * the scaling is changed.
 */

#ifdef ANSI_C
static void normalize_scaling(void)
#else
static void normalize_scaling()
#endif
{
	if (G_shrink<0) G_shrink   *= -1;
	if (G_stretch<0) G_stretch *= -1;
	if (G_shrink==G_stretch) G_shrink = G_stretch = 1;
	if (G_shrink ==0) G_shrink=1;
	if (G_stretch==0) G_stretch=1;
	if (G_stretch > G_shrink) {
		if (G_shrink * 5000 < G_stretch) {
			G_shrink = 1;
			G_stretch = 5000;
		}
		G_stretch = G_stretch * 60 / G_shrink;
		G_shrink  = 60;
	}
	if (G_stretch < G_shrink) {
		if (G_shrink > G_stretch * 5000) {
			G_shrink =  5000;
			G_stretch = 1;
		}
		G_shrink  = G_shrink * 60 / G_stretch;
		G_stretch = 60;
	}
	while ((G_shrink%2==0) && (G_stretch%2==0)) {
		G_shrink   = G_shrink/2;
		G_stretch  = G_stretch/2;
	}
	while ((G_shrink%3==0) && (G_stretch%3==0)) {
		G_shrink   = G_shrink/3;
		G_stretch  = G_stretch/3;
	}
	while ((G_shrink%5==0) && (G_stretch%5==0)) {
		G_shrink   = G_shrink/5;
		G_stretch  = G_stretch/5;
	}
	while ((G_shrink%7==0) && (G_stretch%7==0)) {
		G_shrink   = G_shrink/7;
		G_stretch  = G_stretch/7;
	}
	if (G_shrink ==0) G_shrink=1;
	if (G_stretch==0) G_stretch=1;
}


/*--------------------------------------------------------------------*/
/*  Normalize par_xrp1, ... par_yup2                                  */
/*--------------------------------------------------------------------*/

/* This is done to minimize rounding errors if the fisheye comes 
 * too extreme or to near to the normal view.
 */

#ifdef ANSI_C
static void normalize_fe_parameters(void)
#else
static void normalize_fe_parameters()
#endif
{
	double h;
	long s;

	assert((par_xrp1>0L));
	if (par_xrp2 < 0L) s = -1L; else s = 1L;	
	par_xrp2 = par_xrp2 * s;
	if (par_xrp1 > par_xrp2) {
		h = (double)par_xrp1/(double)par_xrp2;
		if (h<10.0) {
			par_xrp1 = (long)(h*100+0.5);
			par_xrp2 = s * 100L;
		}
		else if (h<100.0) {
			par_xrp1 = (long)(h*10+0.5);
			par_xrp2 = s * 10L;
		}
		else if (h>1000.0) {
			par_xrp1 = 1000L;
			par_xrp2 = s;
		}
		else {  par_xrp1 = (long)(h+0.5);
			par_xrp2 = s;
		} 
	}
	else {
		h = (double)par_xrp2/(double)par_xrp1;
		if (h<10.0) {
			par_xrp2 = s * (long)(h*100+0.5);
			par_xrp1 = 100L;
		}
		else if (h<100.0) {
			par_xrp2 = s * (long)(h*10+0.5);
			par_xrp1 = 10L;
		}
		else if (h>1000.0) {
			par_xrp2 = s*1000;
			par_xrp1 = 1L;
		}
		else {  par_xrp2 = s * (long)(h+0.5);
			par_xrp1 = 1L;
		} 
	}
	assert((par_xlp1>0L));
	if (par_xlp2 < 0L) s = -1L; else s = 1L;	
	par_xlp2 = par_xlp2 * s;
	if (par_xlp1 > par_xlp2) {
		h = (double)par_xlp1/(double)par_xlp2;
		if (h<10.0) {
			par_xlp1 = (long)(h*100+0.5);
			par_xlp2 = s * 100L;
		}
		else if (h<100.0) {
			par_xlp1 = (long)(h*10+0.5);
			par_xlp2 = s * 10L;
		}
		else if (h>1000.0) {
			par_xlp1 = 1000;
			par_xlp2 = s;
		}
		else {  par_xlp1 = (long)(h+0.5);
			par_xlp2 = s;
		} 
	}
	else {
		h = (double)par_xlp2/(double)par_xlp1;
		if (h<10.0) {
			par_xlp2 = s * (long)(h*100+0.5);
			par_xlp1 = 100L;
		}
		else if (h<100.0) {
			par_xlp2 = s * (long)(h*10+0.5);
			par_xlp1 = 10L;
		}
		else if (h>1000.0) {
			par_xlp2 = s * 1000;
			par_xlp1 = 1L;
		}
		else {  par_xlp2 = s * (long)(h+0.5);
			par_xlp1 = 1L;
		} 
	}
	assert((par_ylp1>0L));
	if (par_ylp2 < 0L) s = -1L; else s = 1L;	
	par_ylp2 = par_ylp2 * s;
	if (par_ylp1 > par_ylp2) {
		h = (double)par_ylp1/(double)par_ylp2;
		if (h<10.0) {
			par_ylp1 = (long)(h*100+0.5);
			par_ylp2 = s * 100L;
		}
		else if (h<100.0) {
			par_ylp1 = (long)(h*10+0.5);
			par_ylp2 = s * 10L;
		}
		else if (h>1000.0) {
			par_ylp1 = 1000;
			par_ylp2 = s;
		}
		else {  par_ylp1 = (long)(h+0.5);
			par_ylp2 = s;
		} 
	}
	else {
		h = (double)par_ylp2/(double)par_ylp1;
		if (h<10.0) {
			par_ylp2 = s * (long)(h*100+0.5);
			par_ylp1 = 100L;
		}
		else if (h<100.0) {
			par_ylp2 = s * (long)(h*10+0.5);
			par_ylp1 = 10L;
		}
		else if (h>1000.0) {
			par_ylp2 = s * 1000;
			par_ylp1 = 1L;
		}
		else {  par_ylp2 = s * (long)(h+0.5);
			par_ylp1 = 1L;
		} 
	}
	assert((par_yup1>0L));
	if (par_yup2 < 0L) s = -1L; else s = 1L;	
	par_yup2 = par_yup2 * s;
	if (par_yup1 > par_yup2) {
		h = (double)par_yup1/(double)par_yup2;
		if (h<10.0) {
			par_yup1 = (long)(h*100+0.5);
			par_yup2 = s * 100L;
		}
		else if (h<100.0) {
			par_yup1 = (long)(h*10+0.5);
			par_yup2 = s * 10L;
		}
		else if (h>1000.0) {
			par_yup1 = 1000;
			par_yup2 = s;
		}
		else {  par_yup1 = (long)(h+0.5);
			par_yup2 = s;
		} 
	}
	else {
		h = (double)par_yup2/(double)par_yup1;
		if (h<10.0) {
			par_yup2 = s * (long)(h*100+0.5);
			par_yup1 = 100L;
		}
		else if (h<100.0) {
			par_yup2 = s * (long)(h*10+0.5);
			par_yup1 = 10L;
		}
		else if (h>1000.0) {
			par_yup2 = s * 1000;
			par_yup1 = 1L;
		}
		else {  par_yup2 = s * (long)(h+0.5);
			par_yup1 = 1L;
		} 
	}
}

