/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */ 
/*		--------------------------------------		      */ 
/*								      */
/*   file:	   drawlib.c					      */
/*   version:	   1.00.00					      */
/*   creation:	   14.4.93					      */
/*   author:	   I. Lemke  (...-Version 0.99.99)		      */ 
/*		   G. Sander (Version 1.00.00-...)		      */ 
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */ 
/*   description:  Library of drawing routines			      */ 
/*		   of edges					      */
/*   status:	   in work					      */
/*								      */
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
 * Revision 3.13  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.12  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 * infobox behaviour improved.
 * First version of fisheye (carthesian).
 * Options Noedge and nonode.
 *
 * Revision 3.11  1994/11/23  14:50:47  sander
 * Drawing speed improved. Nodes that are too small are now drawn
 * as a small box without content.
 *
 * Revision 3.10  1994/08/09  10:44:03  sander
 * Spline drawing visibility check added.
 *
 */

/************************************************************************
 *  This file is a collection of auxiliary functions that implement the
 *  primitivas to draw objects. 
 *  Note: we are independent from the device here. The only device 
 *  dependent functions we need here are 
 *    gs_line(x1,y1,x2,y2,c)  which draws a line of color c 
 *			      from (x1,y1) to (x2,y2)
 *    gs_rectangle(x,y,w,h,c) which draws a filled rectangle of
 *			      color c at (x,y) with size w and h 
 *  They can be found in the corresponding device module, e.g. in
 *  sunvdev.c or in X11dev.c.
 *
 *  This file here provides the following functions:
 *    gs_calctextboxsize(v)   calculate size of a minimal box around node v 
 *			      relatively to the actal scaling.
 *			      After gs_calctextboxsize, the result is in
 *				    - int gs_boxw	 (the width)
 *				    - int gs_boxh	 (the height)
 *    gs_textbox(v)	      draw node v at the actual position with the
 *			      actual scaling.
 *    gs_reverttextbox(v)     draw node v at the actual position with the
 *			      actual scaling. The color is reverted.
 *    gs_halfreverttextbox(v) draw node v at the actual position with the
 *			      actual scaling. The color is half reverted.
 *    gs_calcrhombsize(v)     calculate size of a minimal rhomb around a 
 *			      node v relatively to the actal scaling.
 *			      See gs_calctextboxsize.
 *    gs_rhomb(v)	      draw a rhomb node at the actual position with
 *			      the actual scaling.
 *    gs_revertrhomb(v)	      draw a rhomb node at the actual position with
 *			      the actual scaling. The color is reverted.
 *    gs_halfrevertrhomb(v)   draw a rhomb node at the actual position with
 *			      the actual scaling. The color is half reverted.
 *    gs_calctrianglesize(v)  calculate size of a minimal triangle around a 
 *			      node v relatively to the actal scaling.
 *			      See gs_calctextboxsize.
 *    gs_triangle(v)	      draw a triangle node at the actual position with
 *			      the actual scaling.
 *    gs_reverttriangle(v)    draw a triangle node at the actual position with
 *			      the actual scaling. The color is reverted.
 *    gs_halfreverttriangle(v)draw a triangle node at the actual position with
 *			      the actual scaling. The color is half reverted.
 *    gs_calcellipsesize(v)   calculate size of a minimal ellipse around a 
 *			      node v relatively to the actal scaling.
 *			      See gs_calctextboxsize.
 *    gs_ellipse(v)	      draw a ellipse node at the actual position with
 *			      the actual scaling.
 *    gs_revertellipse(v)     draw a ellipse node at the actual position with
 *			      the actual scaling. The color is reverted.
 *    gs_halfrevertellipse(v) draw a ellipse node at the actual position with
 *			      the actual scaling. The color is half reverted.
 *    gs_stringbox(v)	      draw node v at the actual position with the
 *			      actual scaling, but without border.
 *    gs_solidarrow   	      draw solid  edge e with global scaling.
 *    gs_dashedarrow	      draw dashed edge e with global scaling.
 *    gs_dottedarrow	      draw dotted edge e with global scaling.
 *
 * One important remark: string and texbox output is done with the
 * scaling factor we have set by gs_setshrink. The global values 
 * G_stretch and G_shrink do not influence that behaviour.
 * Arrow drawing however is done with respect to G_stretch and G_shrink,
 * but not with respect to the value set by gs_setshrink. The reason
 * for this is that boxes can be individually scaled while arrows cannot. 
 ************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "globals.h"
#include "main.h"
#include "options.h"
#include "alloc.h"
#include "grprint.h"
#include "fisheye.h"
#include "drawstr.h"
#include "drawlib.h"

#ifdef INCLUDE_DRAW
#undef FAST_X11_DRAWING
#endif

/*  Prototypes
 *  ---------- 
 */

/* The following two functions are device dependent and are implemented
 * in sunvdv.c or X11dv.c.
 */

#ifndef INCLUDE_DRAW
extern void gs_line	 _PP((int x1,int y1,int x2,int y2,int c));
extern void gs_rectangle _PP((long x,long y,int w,int h,int c));
#endif

void gs_wait_message    _PP((int c));

/* Static functions */

static int	check_visible	_PP((int a, int b, int c, int d));
static void	gs_border	_PP((int x,int y,int w,int h,int t,int c));
static void	gs_rhombborder	_PP((int x,int y,int w,int h,int t,int c));
static void	gs_triborder	_PP((int x,int y,int w,int h,int t,int c));
static void	gs_ellipsborder	_PP((int x,int y,int w,int h,int t,int c));
static void	gs_filledellips _PP((int x,int y,int w,int h,int c1,int c2));
static void 	gs_labelbox	_PP((GEDGE e));

static int	gs_arroworientation _PP((int x1, int y1, int x2, int y2));
static void	draw_arrowhead	    _PP((GEDGE e, int flag));

static void 	draw_solidanchors   _PP(( GEDGE e));
static void 	draw_dashedanchors  _PP(( GEDGE e));
static void 	draw_dottedanchors  _PP(( GEDGE e));
static void 	draw_splineanchors  _PP(( GEDGE e));

static void 	gs_mysolidline   _PP((int x1,int y1,int x2,int y2,int t,int c));
static void 	gs_mydashedline  _PP((int x1,int y1,int x2,int y2,int t,int c));
static void 	gs_mydottedline	 _PP((int x1,int y1,int x2,int y2,int t,int c));

static void	gs_sosolidline	_PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_wesolidline	_PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_sesolidline	_PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_swsolidline	_PP((int x1,int y1,int x2,int y2,int t,int c));

static void	gs_sodashedline _PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_wedashedline _PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_sedashedline _PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_swdashedline _PP((int x1,int y1,int x2,int y2,int t,int c));

static void	gs_sodottedline _PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_wedottedline _PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_sedottedline _PP((int x1,int y1,int x2,int y2,int t,int c));
static void	gs_swdottedline _PP((int x1,int y1,int x2,int y2,int t,int c));

static void 	gs_freearrowpoint
   _PP((int x1, int y1, int x2, int y2, int s, int c,int m,int ls));
static void	gs_n_arrowpoint    _PP((int x,int y,int s,int c,int m, int ls));
static void	gs_w_arrowpoint    _PP((int x,int y,int s,int c,int m, int ls));
static void	gs_e_arrowpoint    _PP((int x,int y,int s,int c,int m, int ls));
static void	gs_s_arrowpoint    _PP((int x,int y,int s,int c,int m, int ls));
static void	gs_ne_arrowpoint   _PP((int x,int y,int s,int c,int m, int ls));
static void	gs_nw_arrowpoint   _PP((int x,int y,int s,int c,int m, int ls));
static void	gs_se_arrowpoint   _PP((int x,int y,int s,int c,int m, int ls));
static void	gs_sw_arrowpoint   _PP((int x,int y,int s,int c,int m, int ls));

static void draw_spline           _PP((GEDGE e,int f));
static GEDGE dummy_continue_edge  _PP(( GNODE v, GEDGE e));

static void draw_start_part  _PP((int x0,int y0,int x1,int y1,int c,int t,int m));
static void draw_final_part  _PP((int x0,int y0,int x1,int y1,int c,int t,int m));
static void draw_spline_part _PP((int x0,int y0,int x1,int y1,int x2,int y2,int c,int t,int m));

static void check_special_dummy   _PP((GNODE node, GNODE sn, int x1, int y1));
static void check_border_points   _PP((GNODE v,int x1,int y1));
static void check_spline_point    _PP((int kx,int ky,int x1,int y1));


/* Global variables 
 * ----------------
 * Instead of calculating arcustangens, we use precalculated tables.
 * Given a fifty-scaled degree d = 50 * (y2-y1) / (x2-x1), then 
 * xoffset[d] and yoffset[d] are the offsets of an rectangle whose
 * hypothenusis is about 15 point. Valid for 0< (y2-y1) / (x2-x1) < 1.
 *			.
 *		     .	|			    .
 *	 15 point .	| yoffset[d]		 .
 *	       .	|		      .     arctan((y2-y1)/(x2-x1))
 *	    ____________|		   _____________
 *	     xoffset[d]
 *
 * With other words: For any angle alpha (0 degree < alpha < 45 degree), 
 * there is a rectangular triangle with hypothenusis ca. 15 point and
 * katheses ca. xoffset[arctan(alpha)] and yoffset[arctan(alpha)].  
 */

static int	xoffset[51] =
{
	15,15,15,15,15,15,15,15,15,15,
	15,15,15,15,14,14,14,14,14,14,
	14,14,14,14,14,13,13,13,13,13,
	13,13,13,13,12,12,12,12,12,12,
	12,12,11,11,11,11,11,11,11,11,
	11
};

static int	yoffset[51] =
{
	0,0,1,1,1,2,2,2,2,3,
	3,3,4,4,4,4,5,5,5,5,
	6,6,6,6,7,7,7,7,7,8,
	8,8,8,8,8,9,9,9,9,9,
	9,10,10,10,10,10,10,10,10,10,
	11
};



/*--------------------------------------------------------------------*/
/*   Testbox drawing						      */
/*--------------------------------------------------------------------*/

/* Color of the background for textboxdrawing */
/* This value is used for the fast character drawing routine of X11 */

int gs_actbackground;


/* Calculate the size of a text box 
 * --------------------------------
 * The width of the box is returned in gs_boxw, the height is
 * returned in gs_boxh. Both values are scaled relatively to
 * mystretch/myshrink.
 * We let a minimal border of 3 scaled pixels arround the text.
 * Note: We do not calculate the size of the visible part of the box, 
 * but the size of the entire box.
 */

int	gs_boxw;
int	gs_boxh;


#ifdef ANSI_C
void gs_calctextboxsize(GNODE v)
#else
void gs_calctextboxsize(v)
GNODE	v;
#endif
{
	int	border;
	
	assert((v));
	gs_calcstringsize(NLABEL(v));	
	if (NBORDERW(v)==0) border = 0;
	else  border = ((6+2*NBORDERW(v))*mystretch)/myshrink;
	gs_boxw = gs_stringw+border;
	gs_boxh = gs_stringh+border;
}


/*  Draw a text box for node v
 *  --------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The box is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the box is centered, left 
 *  or right justified accordung to the flag in v.
 */

#ifdef ANSI_C
void gs_textbox(GNODE v)
#else
void gs_textbox(v)
GNODE	v;
#endif
{
	int	t;
	int	border, height, width;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = height  = NHEIGHT(v)*G_stretch/G_shrink;
	w = width   = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	/* check parts to draw because of visibility */
	if (x	< V_xmin)    { w = x+w-V_xmin; x = V_xmin; }
	if (x+w >=V_xmax)    w = V_xmax-x-1;
	if (y	< V_ymin)    { h = y+h-V_ymin; y = V_ymin; }
	if (y+h >=V_ymax)    h = V_ymax-y-1;

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((3+NBORDERW(v))*mystretch)/myshrink;
	}

	/* clear background with color of node and draw border */
	if (colored) {
		gs_actbackground = NCOLOR(v);
		gs_rectangle(x,y,w,h,gs_actbackground);
		if (t) gs_border(myxpos,myypos,width,height,t,NBCOLOR(v));
	}
	else {	
		gs_actbackground = WHITE;
		gs_rectangle(x,y,w,h,WHITE);
		if (t) gs_border(myxpos,myypos,width,height,t,BLACK);
	}

	/* calc string position */

	if (myshrink==0)		return;
        if (gs_stlimit*myshrink>gs_shlimit*mystretch)   return;
	gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + border;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos - border + width-gs_stringw;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (width-gs_stringw)/2;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	}

	/* print string */
	if (colored) gs_printstr(NLABEL(v),NTCOLOR(v));
	else         gs_printstr(NLABEL(v),BLACK);
}


/*  Draw a reverted text box for node v
 *  -----------------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The box is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the box is centered, left 
 *  or right justified accordung to the flag in v.
 *  The color is reverted.
 */

#ifdef ANSI_C
void gs_reverttextbox(GNODE v)
#else
void gs_reverttextbox(v)
GNODE	v;
#endif
{
	int	t;
	int	border, height, width;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = height  = NHEIGHT(v)*G_stretch/G_shrink;
	w = width   = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:	
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	/* check parts to draw because of visibility */
	if (x	< V_xmin)    { w = x+w-V_xmin; x = V_xmin; }
	if (x+w >=V_xmax)    w = V_xmax-x-1;
	if (y	< V_ymin)    { h = y+h-V_ymin; y = V_ymin; }
	if (y+h >=V_ymax)    h = V_ymax-y-1;

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((3+NBORDERW(v))*mystretch)/myshrink;
	}

	/* clear background with color of border and draw border 
         * with color of node 
	 */
	if (colored) {
		gs_actbackground = NBCOLOR(v);
		gs_rectangle(x,y,w,h,gs_actbackground);
		if (t) gs_border(myxpos,myypos,width,height,t,NCOLOR(v));
	}
	else {	
		gs_actbackground = BLACK;
		gs_rectangle(x,y,w,h,BLACK);
		if (t) gs_border(myxpos,myypos,width,height,t,WHITE);
	}

	/* calc string position */

	if (myshrink==0)		return;
        if (gs_stlimit*myshrink>gs_shlimit*mystretch)   return;
	gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + border;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos - border + width-gs_stringw;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (width-gs_stringw)/2;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	}

	/* print string with color of node */
	if (colored) gs_printstr(NLABEL(v),NCOLOR(v));
	else         gs_printstr(NLABEL(v),WHITE);
}


/*  Draw a halfreverted text box for node v
 *  ---------------------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The box is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the box is centered, left 
 *  or right justified accordung to the flag in v.
 *  The color is reverted.
 */

#ifdef ANSI_C
void gs_halfreverttextbox(GNODE	v)
#else
void gs_halfreverttextbox(v)
GNODE	v;
#endif
{
	int	t;
	int	border, height, width;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = height  = NHEIGHT(v)*G_stretch/G_shrink;
	w = width   = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}


	/* check parts to draw because of visibility */
	if (x	< V_xmin)    { w = x+w-V_xmin; x = V_xmin; }
	if (x+w >=V_xmax)    w = V_xmax-x-1;
	if (y	< V_ymin)    { h = y+h-V_ymin; y = V_ymin; }
	if (y+h >=V_ymax)    h = V_ymax-y-1;

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((3+NBORDERW(v))*mystretch)/myshrink;
	}

	/* clear background with half/half color and draw border */
	if (colored) {
		gs_actbackground = NBCOLOR(v);
		gs_rectangle(x,y,w/2,h,gs_actbackground);
		gs_rectangle(x+w/2+1,y,w/2,h,NCOLOR(v));
		if (t) gs_border(myxpos,myypos,width,height,t,NBCOLOR(v));
	}
	else {	
		gs_actbackground = BLACK;
		gs_rectangle(x,y,w/2,h,gs_actbackground);
		gs_rectangle(x+w/2+1,y,w/2,h,WHITE);
		if (t) gs_border(myxpos,myypos,width,height,t,BLACK);
	}

	/* calc string position */

	if (myshrink==0)		return;
        if (gs_stlimit*myshrink>gs_shlimit*mystretch)   return;
	gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + border;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos - border + width-gs_stringw;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (width-gs_stringw)/2;
		    myypos = myypos + (height-gs_stringh)/2;
		    break;
	}

	/* print string with color of node */
	if (colored) gs_printstr(NLABEL(v),NCOLOR(v));
	else         gs_printstr(NLABEL(v),WHITE);
}


/*  Draw a string box for node v
 *  ----------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The box is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the box is centered.
 *  No border is drawn. This is used for labels.
 */

#ifdef ANSI_C
void gs_stringbox(GNODE	v)
#else
void gs_stringbox(v)
GNODE	v;
#endif
{
	int	height, width;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = height  = NHEIGHT(v)*G_stretch/G_shrink;
	w = width   = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
			else         gs_rectangle(x,y,w,h,WHITE);
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
			else         gs_rectangle(x,y,w,h,WHITE);
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
			else         gs_rectangle(x,y,w,h,WHITE);
			return;
		}
		break;
	}

	/* check parts to draw because of visibility */
	if (x	< V_xmin)    { w = x+w-V_xmin; x = V_xmin; }
	if (x+w >=V_xmax)    w = V_xmax-x-1;
	if (y	< V_ymin)    { h = y+h-V_ymin; y = V_ymin; }
	if (y+h >=V_ymax)    h = V_ymax-y-1;

	/* clear background with color of node and draw border */
	if (colored) {
		gs_actbackground = NCOLOR(v);
		gs_rectangle(x,y,w,h,gs_actbackground);
	}
	else {	
		gs_actbackground = WHITE;
		gs_rectangle(x,y,w,h,WHITE);
	}


	/* calc string position */

	if (myshrink==0)		return;
        if (gs_stlimit*myshrink>gs_shlimit*mystretch)   return;
	gs_calcstringsize(NLABEL(v));

	myxpos = myxpos + (width-gs_stringw)/2;
	myypos = myypos + (height-gs_stringh)/2;

	/* print string */
	if (colored) gs_printstr(NLABEL(v),NTCOLOR(v));
	else         gs_printstr(NLABEL(v),BLACK);
}



/*  Draw a string box for a label of an edge e 
 *  ------------------------------------------
 *  The box is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the box is centered.
 *  No border is drawn. This is used for labels.
 */

#ifdef ANSI_C
static void gs_labelbox(GEDGE e)
#else
static void gs_labelbox(e)
GEDGE	e;
#endif
{
	int	height, width;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	if (!ELABEL(e)) return;
	gs_setshrink(G_stretch, G_shrink);

	if (myshrink==0)		return;
        if (gs_stlimit*myshrink>gs_shlimit*mystretch)   return;

	gs_calcstringsize(ELABEL(e));
	x = (ETBENDX(e)+EBBENDX(e))/2;
	y = (ETBENDY(e)+EBBENDY(e))/2;
	gs_stringw += (ETHICKNESS(e)/2+1);
       	gs_stringh += (ETHICKNESS(e)/2+1);
        x = x - gs_stringw/2;
      	y = y - gs_stringh/2;
        gs_setto(x * G_stretch / G_shrink,
                 y * G_stretch / G_shrink );

	x = myxpos;
	y = myypos;
	h = height  = gs_stringh;
	w = width   = gs_stringw;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small labels: no text */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (colored) gs_rectangle(x,y,w,h,G_color);
			else         gs_rectangle(x,y,w,h,WHITE);
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (colored) gs_rectangle(x,y,w,h,G_color);
			else         gs_rectangle(x,y,w,h,WHITE);
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (colored) gs_rectangle(x,y,w,h,G_color);
			else         gs_rectangle(x,y,w,h,WHITE);
			return;
		}
		break;
	}

	/* check parts to draw because of visibility */
	if (x	< V_xmin)    { w = x+w-V_xmin; x = V_xmin; }
	if (x+w >=V_xmax)    w = V_xmax-x-1;
	if (y	< V_ymin)    { h = y+h-V_ymin; y = V_ymin; }
	if (y+h >=V_ymax)    h = V_ymax-y-1;

	gs_calcstringsize(ELABEL(e));

	/* clear background with color of node and draw border */
	if (colored) {
		gs_actbackground = G_color;
		gs_rectangle(x,y,w,h,gs_actbackground);
	}
	else {	
		gs_actbackground = WHITE;
		gs_rectangle(x,y,w,h,WHITE);
	}

	/* calc string position */
	myxpos = myxpos + (width-gs_stringw)/2;
	myypos = myypos + (height-gs_stringh)/2;

	/* print string */
	if (colored) gs_printstr(ELABEL(e),ELABELCOL(e));
	else         gs_printstr(ELABEL(e),BLACK);
}

 
/*  Draw a border 
 *  -------------
 *  draw a border line of thickness t around a box at (x,y).
 *  The box has width w and height h. The drawing color is c.
 *  NO SCALING !!!
 */

#ifdef ANSI_C
static void	gs_border(int x,int y,int w,int h,int t,int c)
#else
static void	gs_border(x, y, w, h, t, c)
int x, y, w, h, t, c;
#endif
{
	int mx,my,mw,mh;

	mx = x;
	my = y;
	mw = w;
	mh = h;
	/* Check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;
	if (x	< V_xmin)    { mx = V_xmin; mw = x+w-V_xmin; }
	if (y	< V_ymin)    { my = V_ymin; mh = y+h-V_ymin; }
	if (mx+mw >=V_xmax)  mw = V_xmax-mx-1;
	if (my+mh >=V_ymax)  mh = V_ymax-my-1;

	/* We draw the visible part only */ 
	if ((y>=V_ymin)&&(y<V_ymax)) 
		gs_rectangle(mx, y, mw, t, c);
	if ((x>=V_xmin)&&(x<V_xmax)) 
		gs_rectangle(x, my, t, mh, c);
	if ((y+h-t>=V_ymin)&&(y+h-t<V_ymax)) 
		gs_rectangle(mx, y + h - t, mw, t, c);
	if ((x+w-t>=V_xmin)&&(x+w-t<V_xmax)) 
		gs_rectangle(x + w - t, my, t, mh, c);
}


/*--------------------------------------------------------------------*/
/*   Rhomb drawing						      */
/*--------------------------------------------------------------------*/

/* A rhomb can be described by its inner box and its outer box:
 *
 *               ------
 *              |  /\  |   For NWIDTH and NHEIGHT, the outer box is
 *              | /--\ |   relevant, for the boxtext however, the inner
 *              |/|  |\|   box should be considered.
 *              |\|  |/|
 *              | \--/ |
 *              |  \/  |
 *               ------
 */


/* Calculate the size of a rhomb box 
 * ---------------------------------
 * The width of the outer box is returned in gs_boxw, the height is
 * returned in gs_boxh. Both values are scaled relatively to
 * mystretch/myshrink.
 * Note: We do not calculate the size of the visible part of the box, 
 * but the size of the entire box.
 */


#ifdef ANSI_C
void gs_calcrhombsize(GNODE v)
#else
void gs_calcrhombsize(v)
GNODE	v;
#endif
{
	int	border;
	
	assert((v));
	gs_calcstringsize(NLABEL(v));	
	if (NBORDERW(v)==0) border = 0;
	else  border = ((2*NBORDERW(v)-2)*mystretch)/myshrink;
	gs_boxw = 2*(gs_stringw+border);
	gs_boxh = 2*(gs_stringh+border);
}


/*  Draw a rhomb box for node v
 *  ---------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The box is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the box is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the rhomb.
 */

#ifdef ANSI_C
void gs_rhomb(GNODE v)
#else
void gs_rhomb(v)
GNODE	v;
#endif
{
	int	t, i;
	int	border;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
		fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}


	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of node */
	if (colored) gs_actbackground = NCOLOR(v);
	else	     gs_actbackground = WHITE;
#ifdef POSTSCRIPT_DEVICE
	ps_fillrhomb(x,y,w,h,gs_actbackground);
#else
#ifdef FAST_X11_DRAWING
	X11_fast_triangle(x, y+h/2, x+w/2, y,   x+w, y+h/2, gs_actbackground);
	X11_fast_triangle(x, y+h/2, x+w/2, y+h, x+w, y+h/2, gs_actbackground);
#else
	if (w>h) {
		for (i=0; i<=(w+1)/2; i++) 
			gs_mysolidline(x+i, y+h/2-i*h/w, x+i, y+h/2+i*h/w,1,
				gs_actbackground);
		for (i=0; i<=(w+1)/2; i++) 
			gs_mysolidline(x+w-i, y+h/2-i*h/w, x+w-i, y+h/2+i*h/w,1,
				gs_actbackground);
	}
	else {
		for (i=0; i<=(h+1)/2; i++) 
			gs_mysolidline(x+w/2-i*w/h, y+i, x+w/2+i*w/h, y+i,1,
				gs_actbackground);
		for (i=0; i<=(h+1)/2; i++) 
			gs_mysolidline(x+w/2-i*w/h, y+h-i, x+w/2+i*w/h, y+h-i,1,
				gs_actbackground);
	}
#endif
#endif

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + (w+3)/4 + border;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + (3*w+3)/4 - border -gs_stringw;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (w+3)/4 + ((w+1)/2-gs_stringw)/2;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	}

	/* print string */
	if (colored) gs_printstr(NLABEL(v),NTCOLOR(v));
	else         gs_printstr(NLABEL(v),BLACK);

	/* draw border */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_rhombborder(x,y,w,h,t,NBCOLOR(v));
		else         gs_rhombborder(x,y,w,h,t,BLACK);
	}
}


/*  Draw a reverted rhomb box for node v
 *  ------------------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The box is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the box is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the rhomb.
 *  The color is reverted.
 */

#ifdef ANSI_C
void gs_revertrhomb(GNODE v)
#else
void gs_revertrhomb(v)
GNODE	v;
#endif
{
	int	t, i;
	int	border;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of border */
	if (colored) gs_actbackground = NBCOLOR(v);
	else	     gs_actbackground = BLACK;
#ifdef POSTSCRIPT_DEVICE
	ps_fillrhomb(x,y,w,h,gs_actbackground);
#else
#ifdef FAST_X11_DRAWING
	X11_fast_triangle(x, y+h/2, x+w/2, y,   x+w, y+h/2, gs_actbackground);
	X11_fast_triangle(x, y+h/2, x+w/2, y+h, x+w, y+h/2, gs_actbackground);
#else
	if (w>h) {
		for (i=0; i<=(w+1)/2; i++) 
			gs_mysolidline(x+i, y+h/2-i*h/w, x+i, y+h/2+i*h/w,1,
				gs_actbackground);
		for (i=0; i<=(w+1)/2; i++) 
			gs_mysolidline(x+w-i, y+h/2-i*h/w, x+w-i, y+h/2+i*h/w,1,
				gs_actbackground);
	}
	else {
		for (i=0; i<=(h+1)/2; i++) 
			gs_mysolidline(x+w/2-i*w/h, y+i, x+w/2+i*w/h, y+i,1,
				gs_actbackground);
		for (i=0; i<=(h+1)/2; i++) 
			gs_mysolidline(x+w/2-i*w/h, y+h-i, x+w/2+i*w/h, y+h-i,1,
				gs_actbackground);
	}
#endif
#endif

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + (w+3)/4 + border;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + (3*w+3)/4 - border -gs_stringw;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (w+3)/4 + ((w+1)/2-gs_stringw)/2;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	}

	/* print string with color of node */
	if (colored) gs_printstr(NLABEL(v),NCOLOR(v));
	else         gs_printstr(NLABEL(v),WHITE);

	/* draw border with color of node */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_rhombborder(x,y,w,h,t,NCOLOR(v));
		else         gs_rhombborder(x,y,w,h,t,WHITE);
	}
}


/*  Draw a halfreverted rhomb box for node v
 *  ----------------------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The box is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the box is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the rhomb.
 *  The color is half reverted.
 */

#ifdef ANSI_C
void gs_halfrevertrhomb(GNODE v)
#else
void gs_halfrevertrhomb(v)
GNODE	v;
#endif
{
	int	t, i;
	int	border;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of border */
	if (colored) gs_actbackground = NBCOLOR(v);
	else	     gs_actbackground = BLACK;
#ifdef FAST_X11_DRAWING
	X11_fast_triangle(x, y+h/2, x+w/2, y,   x+w/2, y+h, gs_actbackground);
	if (colored)
		X11_fast_triangle(x+w/2, y, x+w/2, y+h, x+w, y+h/2, NCOLOR(v));
	else 	X11_fast_triangle(x+w/2, y, x+w/2, y+h, x+w, y+h/2, WHITE);
#else
	if (w>h) {
		for (i=0; i<=(w+1)/2; i++) 
			gs_mysolidline(x+i, y+h/2-i*h/w, x+i, y+h/2+i*h/w,1,
				gs_actbackground);
		if (colored)
			for (i=0; i<=(w+1)/2; i++) 
				gs_mysolidline(x+w-i, y+h/2-i*h/w, 
					x+w-i, y+h/2+i*h/w,1, NCOLOR(v));
		else 	for (i=0; i<=(w+1)/2; i++) 
				gs_mysolidline(x+w-i, y+h/2-i*h/w, 
					x+w-i, y+h/2+i*h/w,1, WHITE);
	}
	else {
		for (i=0; i<=(h+1)/2; i++) 
			gs_mysolidline(x+w/2-i*w/h, y+i, x+w/2+i*w/h, y+i,1,
				gs_actbackground);
		if (colored)
			for (i=0; i<=(h+1)/2; i++) 
				gs_mysolidline(x+w/2-i*w/h, y+h-i, 
					x+w/2+i*w/h, y+h-i,1, NCOLOR(v));
		else 	for (i=0; i<=(h+1)/2; i++) 
				gs_mysolidline(x+w/2-i*w/h, y+h-i, 
					x+w/2+i*w/h, y+h-i,1, WHITE);
	}
#endif

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + (w+3)/4 + border;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + (3*w+3)/4 - border -gs_stringw;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (w+3)/4 + ((w+1)/2-gs_stringw)/2;
		    myypos = myypos + (h+3)/4 + ((h+1)/2-gs_stringh)/2;
		    break;
	}

	/* print string with color of node */
	if (colored) gs_printstr(NLABEL(v),NCOLOR(v));
	else         gs_printstr(NLABEL(v),WHITE);

	/* draw border */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_rhombborder(x,y,w,h,t,NBCOLOR(v));
		else         gs_rhombborder(x,y,w,h,t,BLACK);
	}
}


/*  Draw a rhomb border 
 *  -------------------
 *  draw a border line of thickness t around a rhomb at (x,y).
 *  The box has width w and height h. The drawing color is c.
 *  NO SCALING !!!
 */

#ifdef ANSI_C
static void	gs_rhombborder(int x,int y,int w,int h,int t,int c)
#else
static void	gs_rhombborder(x, y, w, h, t, c)
int x, y, w, h, t, c;
#endif
{
	gs_mysolidline(x,    y+h/2,x+w/2,y,    t,c);
	gs_mysolidline(x,    y+h/2,x+w/2,y+h,  t,c);
	gs_mysolidline(x+w/2,y,    x+w,  y+h/2,t,c);
	gs_mysolidline(x+w/2,y+h,  x+w,  y+h/2,t,c);
}



/*--------------------------------------------------------------------*/
/*   Triangle drawing						      */
/*--------------------------------------------------------------------*/

/* A triangle can be described by its inner box and its outer box:
 *
 *             --------
 *            |   /\   |   For NWIDTH and NHEIGHT, the outer box is
 *            |  /--\  |   relevant, for the boxtext however, the inner
 *            | /|  |\ |   box should be considered.
 *            |/ |  | \|
 *             --------
 */


/* Calculate the size of a triangle box 
 * ------------------------------------
 * The width of the outer box is returned in gs_boxw, the height is
 * returned in gs_boxh. Both values are scaled relatively to
 * mystretch/myshrink.
 * Note: We do not calculate the size of the visible part of the box, 
 * but the size of the entire box.
 */


#ifdef ANSI_C
void gs_calctrianglesize(GNODE v)
#else
void gs_calctrianglesize(v)
GNODE	v;
#endif
{
	int	border;
	
	assert((v));
	gs_calcstringsize(NLABEL(v));	
	if (NBORDERW(v)==0) border = 0;
	else  border = ((2*NBORDERW(v)-2)*mystretch)/myshrink;
	gs_boxw = 2*(gs_stringw+border);
	gs_boxh = 2*(gs_stringh+border);
}


/*  Draw a triangle for node v
 *  --------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The triangle is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the triangle is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the triangle.
 */


#ifdef ANSI_C
void gs_triangle(GNODE v)
#else
void gs_triangle(v)
GNODE	v;
#endif
{
	int	t, i;
	int	border;
	int	x,y,w,h;
	int	xoffs,yoffs;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of node */
	if (colored) gs_actbackground = NCOLOR(v);
	else	     gs_actbackground = WHITE;
#ifdef POSTSCRIPT_DEVICE
	ps_filltriangle(x,y,w,h,gs_actbackground);
#else
#ifdef FAST_X11_DRAWING
	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		X11_fast_triangle(x, y+h/2, x+w, y, x+w, y+h, gs_actbackground);
		break;
	case RIGHT_TO_LEFT:
		X11_fast_triangle(x+w, y+h/2, x, y, x, y+h, gs_actbackground);
		break;
	case BOTTOM_TO_TOP:
		X11_fast_triangle(x+w/2, y+h, x,y,  x+w,y,  gs_actbackground);
		break;
	case TOP_TO_BOTTOM:
		X11_fast_triangle(x+w/2, y, x,y+h,  x+w,y+h, gs_actbackground);
		break;
	}
#else
	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		for (i=0; i<w; i++) 
			gs_mysolidline(x+i, y+h/2-i*h/w/2, x+i, y+h/2+i*h/w/2,
				1, gs_actbackground);
		break;
	case RIGHT_TO_LEFT:
		for (i=0; i<w; i++) 
			gs_mysolidline(x+w-i,y+h/2-i*h/w/2,x+w-i,y+h/2+i*h/w/2,
				1, gs_actbackground);
		break;
	case BOTTOM_TO_TOP:
		for (i=0; i<h; i++) 
			gs_mysolidline(x+w/2-i*w/h/2,y+h-i,x+w/2+i*w/h/2,y+h-i,
				1, gs_actbackground);
		break;
	case TOP_TO_BOTTOM:
		for (i=0; i<h; i++) 
			gs_mysolidline(x+w/2-i*w/h/2, y+i, x+w/2+i*w/h/2, y+i,
				1, gs_actbackground);
		break;
	}
#endif
#endif

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		xoffs = (w+1)/2;
		yoffs = (h+3)/4;
		break;
	case RIGHT_TO_LEFT:
		xoffs = 0;
		yoffs = (h+3)/4;
		break;
	case BOTTOM_TO_TOP:
		xoffs = (w+2)/4;
		yoffs = 0;
		break;
	case TOP_TO_BOTTOM:
		xoffs = (w+2)/4;
		yoffs = (h+1)/2;
		break;
	}

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + xoffs + border;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + xoffs + (w+1)/2 - border -gs_stringw;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + xoffs + ((w+1)/2-gs_stringw)/2;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	}

	/* print string */
	if (colored) gs_printstr(NLABEL(v),NTCOLOR(v));
	else         gs_printstr(NLABEL(v),BLACK);

	/* draw border */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_triborder(x,y,w,h,t,NBCOLOR(v));
		else         gs_triborder(x,y,w,h,t,BLACK);
	}
}


/*  Draw a reverted triangle for node v
 *  -----------------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The triangle is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the triangle is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the triangle.
 *  The color is reverted.
 */

#ifdef ANSI_C
void gs_reverttriangle(GNODE v)
#else
void gs_reverttriangle(v)
GNODE	v;
#endif
{
	int	t, i;
	int	border;
	int	x,y,w,h;
	int	xoffs,yoffs;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of border */
	if (colored) gs_actbackground = NBCOLOR(v);
	else	     gs_actbackground = BLACK;
#ifdef POSTSCRIPT_DEVICE
	ps_filltriangle(x,y,w,h,gs_actbackground);
#else
#ifdef FAST_X11_DRAWING
	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		X11_fast_triangle(x, y+h/2, x+w, y, x+w, y+h, gs_actbackground);
		break;
	case RIGHT_TO_LEFT:
		X11_fast_triangle(x+w, y+h/2, x, y, x, y+h, gs_actbackground);
		break;
	case BOTTOM_TO_TOP:
		X11_fast_triangle(x+w/2, y+h, x,y,  x+w,y,  gs_actbackground);
		break;
	case TOP_TO_BOTTOM:
		X11_fast_triangle(x+w/2, y, x,y+h,  x+w,y+h, gs_actbackground);
		break;
	}
#else
	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		for (i=0; i<w; i++) 
			gs_mysolidline(x+i, y+h/2-i*h/w/2, x+i, y+h/2+i*h/w/2,
				1, gs_actbackground);
		break;
	case RIGHT_TO_LEFT:
		for (i=0; i<w; i++) 
			gs_mysolidline(x+w-i,y+h/2-i*h/w/2,x+w-i,y+h/2+i*h/w/2,
				1, gs_actbackground);
		break;
	case BOTTOM_TO_TOP:
		for (i=0; i<h; i++) 
			gs_mysolidline(x+w/2-i*w/h/2,y+h-i,x+w/2+i*w/h/2,y+h-i,
				1, gs_actbackground);
		break;
	case TOP_TO_BOTTOM:
		for (i=0; i<h; i++) 
			gs_mysolidline(x+w/2-i*w/h/2, y+i, x+w/2+i*w/h/2, y+i,
				1, gs_actbackground);
		break;
	}
#endif
#endif

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		xoffs = (w+1)/2;
		yoffs = (h+3)/4;
		break;
	case RIGHT_TO_LEFT:
		xoffs = 0;
		yoffs = (h+3)/4;
		break;
	case BOTTOM_TO_TOP:
		xoffs = (w+2)/4;
		yoffs = 0;
		break;
	case TOP_TO_BOTTOM:
		xoffs = (w+2)/4;
		yoffs = (h+1)/2;
		break;
	}

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + xoffs + border;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + xoffs + (w+1)/2 - border -gs_stringw;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + xoffs + ((w+1)/2-gs_stringw)/2;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	}

	/* print string with color of node */
	if (colored) gs_printstr(NLABEL(v),NCOLOR(v));
	else         gs_printstr(NLABEL(v),WHITE);

	/* draw border with color of node */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_triborder(x,y,w,h,t,NCOLOR(v));
		else         gs_triborder(x,y,w,h,t,WHITE);
	}
}


/*  Draw a halfreverted triangle for node v
 *  ---------------------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The triangle is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the triangle is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the triangle.
 *  The color is half reverted.
 */

#ifdef ANSI_C
void gs_halfreverttriangle(GNODE v)
#else
void gs_halfreverttriangle(v)
GNODE	v;
#endif
{
	int	t, i, mcol;
	int	border;
	int	x,y,w,h;
	int	xoffs,yoffs;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of border */
	if (colored) { gs_actbackground = NBCOLOR(v); mcol = NCOLOR(v); }
	else	     { gs_actbackground = BLACK;      mcol = WHITE;     }

#ifdef FAST_X11_DRAWING
	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		X11_fast_triangle(x,y+h/2,x+w,y+h/2,x+w, y+h, gs_actbackground);
		X11_fast_triangle(x,y+h/2,x+w,y,x+w,y+h/2, mcol);
		break;
	case RIGHT_TO_LEFT:
		X11_fast_triangle(x+w,y+h/2,x,y+h/2,x,y+h,   gs_actbackground);
		X11_fast_triangle(x+w,y+h/2,x, y,   x,y+h/2, mcol);
		break;
	case BOTTOM_TO_TOP:
		X11_fast_triangle(x+w/2, y+h, x+w/2,y, x+w,y, gs_actbackground);
		X11_fast_triangle(x+w/2, y+h, x,y,  x+w/2,y,  mcol);
		break;
	case TOP_TO_BOTTOM:
		X11_fast_triangle(x+w/2,y,x+w/2,y+h,x+w,y+h, gs_actbackground);
		X11_fast_triangle(x+w/2,y,x,y+h, x+w/2,y+h, mcol);
		break;
	}
#else
	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		for (i=0; i<w; i++) { 
			gs_mysolidline(x+i, y+h/2-i*h/w/2, x+i, y+h/2,
				1, gs_actbackground);
			gs_mysolidline(x+i, y+h/2        , x+i, y+h/2+i*h/w/2,
				1, mcol);
		}
		break;
	case RIGHT_TO_LEFT:
		for (i=0; i<w; i++) { 
			gs_mysolidline(x+w-i,y+h/2-i*h/w/2,x+w-i,y+h/2,
				1, gs_actbackground);
			gs_mysolidline(x+w-i,y+h/2        ,x+w-i,y+h/2+i*h/w/2,
				1, mcol);
		}
		break;
	case BOTTOM_TO_TOP:
		for (i=0; i<h; i++) { 
			gs_mysolidline(x+w/2-i*w/h/2,y+h-i,x+w/2,y+h-i,
				1, gs_actbackground);
			gs_mysolidline(x+w/2        ,y+h-i,x+w/2+i*w/h/2,y+h-i,
				1, mcol);
		}
		break;
	case TOP_TO_BOTTOM:
		for (i=0; i<h; i++) { 
			gs_mysolidline(x+w/2-i*w/h/2, y+i, x+w/2, y+i,
				1, gs_actbackground);
			gs_mysolidline(x+w/2        , y+i, x+w/2+i*w/h/2, y+i,
				1, mcol);
		}
		break;
	}
#endif

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		xoffs = (w+1)/2;
		yoffs = (h+3)/4;
		break;
	case RIGHT_TO_LEFT:
		xoffs = 0;
		yoffs = (h+3)/4;
		break;
	case BOTTOM_TO_TOP:
		xoffs = (w+2)/4;
		yoffs = 0;
		break;
	case TOP_TO_BOTTOM:
		xoffs = (w+2)/4;
		yoffs = (h+1)/2;
		break;
	}

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + xoffs + border;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + xoffs + (w+1)/2 - border -gs_stringw;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + xoffs + ((w+1)/2-gs_stringw)/2;
		    myypos = myypos + yoffs + ((h+1)/2-gs_stringh)/2;
		    break;
	}

	/* print string with color of node */
	if (colored) gs_printstr(NLABEL(v),NCOLOR(v));
	else         gs_printstr(NLABEL(v),WHITE);

	/* draw border */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_triborder(x,y,w,h,t,NBCOLOR(v));
		else         gs_triborder(x,y,w,h,t,BLACK);
	}
}


/*  Draw a triangle border 
 *  ----------------------
 *  draw a border line of thickness t around a triangle at (x,y).
 *  The box has width w and height h. The drawing color is c.
 *  NO SCALING !!!
 */

#ifdef ANSI_C
static void	gs_triborder(int x,int y,int w,int h,int t,int c)
#else
static void	gs_triborder(x, y, w, h, t, c)
int x, y, w, h, t, c;
#endif
{
	switch (G_orientation) {
	case LEFT_TO_RIGHT:
		gs_mysolidline(x+w,  y,    x+w,  y+h,  t,c);
		gs_mysolidline(x+w,  y+h,  x,    y+h/2,t,c);
		gs_mysolidline(x,    y+h/2,x+w,  y,    t,c);
		return;
	case RIGHT_TO_LEFT:
		gs_mysolidline(x,    y,    x+w,  y+h/2,t,c);
		gs_mysolidline(x,    y,    x,    y+h  ,t,c);
		gs_mysolidline(x,    y+h  ,x+w,  y+h/2,t,c);
		return;
	case BOTTOM_TO_TOP:
		gs_mysolidline(x,    y,   x+w,  y,    t,c);
		gs_mysolidline(x,    y,   x+w/2,y+h,  t,c);
		gs_mysolidline(x+w,  y,   x+w/2,y+h,  t,c);
		return;
	case TOP_TO_BOTTOM:
		gs_mysolidline(x,    y+h, x+w,  y+h,  t,c);
		gs_mysolidline(x,    y+h, x+w/2,y,    t,c);
		gs_mysolidline(x+w/2,y,   x+w,  y+h,  t,c);
		return;
	}
}



/*--------------------------------------------------------------------*/
/*   Ellipse drawing						      */
/*--------------------------------------------------------------------*/

/* An ellipse can be described by its inner box and its outer box.
 * See triangle and rhomb as before.
 * For NWIDTH and NHEIGHT, the outer box is relevant, for the boxtext 
 * however, the inner box should be considered.
 *
 * Let w=NWIDTH and h=NHEIGHT. The ellipse formula is:
 *
 *       y*y = (h/2)*(h/2) - (x*h/w)*(x*h/w) 
 *
 * Note that the size of the inner box is w/sqrt(2)  and h/sqrt(2). 
 */


/* Calculate the size of a ellipse box 
 * -----------------------------------
 * The width of the outer box is returned in gs_boxw, the height is
 * returned in gs_boxh. Both values are scaled relatively to
 * mystretch/myshrink.
 * Note: We do not calculate the size of the visible part of the box, 
 * but the size of the entire box.
 */


#ifdef ANSI_C
void gs_calcellipsesize(GNODE v)
#else
void gs_calcellipsesize(v)
GNODE	v;
#endif
{
	int	border;
	
	assert((v));
	gs_calcstringsize(NLABEL(v));	
	if (NBORDERW(v)==0) border = 0;
	else  border = ((2*NBORDERW(v)-2)*mystretch)/myshrink;
	gs_boxw = 1414*(gs_stringw+border)/1000;
	gs_boxh = 1414*(gs_stringh+border)/1000;
}


/*  Draw a ellipse for node v
 *  -------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The ellipse is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the ellipse is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the ellipse.
 */

#ifdef ANSI_C
void gs_ellipse(GNODE v)
#else
void gs_ellipse(v)
GNODE	v;
#endif
{
	int	t;
	int	border;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of node */
	if (colored) gs_actbackground = NCOLOR(v);
	else	     gs_actbackground = WHITE;
#ifdef POSTSCRIPT_DEVICE
	ps_fillellipse(x,y,w,h,gs_actbackground);
#else
	gs_filledellips(x, y, w, h, gs_actbackground, gs_actbackground);
#endif

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + 146*w/1000 + border;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + w - 146*w/1000 - border -gs_stringw;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (w-gs_stringw)/2;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	}

	/* print string */
	if (colored) gs_printstr(NLABEL(v),NTCOLOR(v));
	else         gs_printstr(NLABEL(v),BLACK);

	/* draw border */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_ellipsborder(x,y,w,h,t,NBCOLOR(v));
		else         gs_ellipsborder(x,y,w,h,t,BLACK);
	}
}


/*  Draw a reverted ellipse for node v
 *  -----------------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The ellipse is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the ellipse is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the ellipse.
 *  The color is reverted.
 */

#ifdef ANSI_C
void gs_revertellipse(GNODE v)
#else
void gs_revertellipse(v)
GNODE	v;
#endif
{
	int	t;
	int	border;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
		fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of border */
	if (colored) gs_actbackground = NBCOLOR(v);
	else	     gs_actbackground = BLACK;
#ifdef POSTSCRIPT_DEVICE
	ps_fillellipse(x,y,w,h,gs_actbackground);
#else
	gs_filledellips(x, y, w, h, gs_actbackground, gs_actbackground);
#endif

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + 146*w/1000 + border;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + w - 146*w/1000 - border -gs_stringw;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (w-gs_stringw)/2;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	}

	/* print string with color of node */
	if (colored) gs_printstr(NLABEL(v),NCOLOR(v));
	else         gs_printstr(NLABEL(v),WHITE);

	/* draw border with color of node */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_ellipsborder(x,y,w,h,t,NCOLOR(v));
		else         gs_ellipsborder(x,y,w,h,t,WHITE);
	}
}


/*  Draw a halfreverted ellipse for node v
 *  ---------------------------------------
 *  We assume that NWIDTH and NHEIGHT are already set in an appropriate
 *  way and already scaled relatively.
 *  The ellipse is scaled according mystretch/myshrink and drawn at
 *  position (myxpos,myypos). The text of the ellipse is centered, left 
 *  or right justified accordung to the flag in v, however relatively
 *  to the inner box of the ellipse.
 *  The color is half reverted.
 */

#ifdef ANSI_C
void gs_halfrevertellipse(GNODE v)
#else
void gs_halfrevertellipse(v)
GNODE	v;
#endif
{
	int	t;
	int	border;
	int	x,y,w,h;
	int 	x1, x2, x3, x4, y1, y2, y3, y4;

	x = myxpos;
	y = myypos;
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;

	/* check visibility */
	if (x	>=V_xmax)    return;
	if (y	>=V_ymax)    return;
	if (x+w < V_xmin)    return;
	if (y+h < V_ymin)    return;

	/* optimizing routine for very small nodes */
	switch (fisheye_view) {
	case 0:
		if ((w<=3) || (h<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case CSCF_VIEW:
	case FCSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x+w, y+h, &x2, &y2);
		if ((x2-x1<=0) || (y2-y1<=0)) return; 
		if ((x2-x1<=3) || (y2-y1<=3)) {
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	case PSCF_VIEW:
	case FPSCF_VIEW:
	 	fe_g_to_s(x,   y,   &x1, &y1);
		fe_g_to_s(x,   y+h, &x2, &y2);
		fe_g_to_s(x+w, y+h, &x3, &y3);
		fe_g_to_s(x+w, y,   &x4, &y4);
		if (  ((y2-y1<=0)&&(y3-y4<=0)) 
		    ||((x4-x1<=0)&&(x3-x2<=0))) return; 
		if (  ((y2-y1<=3)&&(y3-y4<=3)) 
		    ||((x4-x1<=3)&&(x3-x2<=3))) { 
			if (NBORDERW(v)==0) {
				if (colored) gs_rectangle(x,y,w,h,NCOLOR(v));
				else         gs_rectangle(x,y,w,h,WHITE);
			}
			else {
				if (colored) gs_rectangle(x,y,w,h,NBCOLOR(v));
				else         gs_rectangle(x,y,w,h,BLACK);
			}
			return;
		}
		break;
	}

	if (NBORDERW(v)==0) { t = 0; border = 0; }
	else {	t = (NBORDERW(v)*mystretch)/myshrink;
		if (t<1)	t=1;
		border	= ((NBORDERW(v)-1)*mystretch)/myshrink;
	}

	/* clear background with color of border */
	if (colored) gs_actbackground = NBCOLOR(v);
	else	     gs_actbackground = BLACK;

	if (colored) 
		gs_filledellips(x, y, w, h, NCOLOR(v), gs_actbackground);
	else 
		gs_filledellips(x, y, w, h, WHITE,     gs_actbackground);

	/* calc string position */

        if ((myshrink!=0) && (gs_stlimit*myshrink<=gs_shlimit*mystretch)) 
		gs_calcstringsize(NLABEL(v));

	switch (NTEXTMODE(v)) {
	case LEFT:  myxpos = myxpos + 146*w/1000 + border;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	case RIGHT: myxpos = myxpos + w - 146*w/1000 - border -gs_stringw;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	default:    /* CENTER */
		    myxpos = myxpos + (w-gs_stringw)/2;
		    myypos = myypos + (h-gs_stringh)/2;
		    break;
	}

	/* print string with color of node */
	if (colored) gs_printstr(NLABEL(v),NCOLOR(v));
	else         gs_printstr(NLABEL(v),WHITE);

	/* draw border */
	if (t) {
#ifndef POSTSCRIPT_DEVICE
		if (t>1) t++;
#endif
		if (colored) gs_ellipsborder(x,y,w,h,t,NBCOLOR(v));
		else         gs_ellipsborder(x,y,w,h,t,BLACK);
	}
}


/*  Draw a ellipse border 
 *  ---------------------
 *  draw a border line of thickness t around an ellipse at (x,y).
 *  The box has width w and height h. The drawing color is c.
 *  NO SCALING !!!
 *  Note: here we could speedup for X11 here.
 */

#ifdef ANSI_C
static void	gs_ellipsborder(int x,int y,int w,int h,int t,int c)
#else
static void	gs_ellipsborder(x, y, w, h, t, c)
int x, y, w, h, t, c;
#endif
{
	int i,j, old_i,old_j;

#ifdef FAST_X11_DRAWING
	if (fisheye_view == 0) {
		X11_fast_ellipse(x,y,w,h,t,c);
		return;
	}
#endif
	old_i = 0;
	old_j = h/2;
	for (i=1; i<=(w+1)/2; i++) {
		j = h*h/4 - i*h/w * i*h/w;
		if (j>=0) j = gstoint(sqrt((double)j));
		else j = 0;

		gs_mysolidline(x+w/2+old_i, y+h/2+old_j, x+w/2+i, y+h/2+j, t,c);
		gs_mysolidline(x+w/2-old_i, y+h/2+old_j, x+w/2-i, y+h/2+j, t,c);
		gs_mysolidline(x+w/2+old_i, y+h/2-old_j, x+w/2+i, y+h/2-j, t,c);
		gs_mysolidline(x+w/2-old_i, y+h/2-old_j, x+w/2-i, y+h/2-j, t,c);
		old_i=i;
		old_j=j;
	}

	gs_mysolidline(x+w/2+old_i, y+h/2+old_j, x+w/2+old_i, y+h/2-old_j,t,c);
	gs_mysolidline(x+w/2-old_i, y+h/2+old_j, x+w/2-old_i, y+h/2-old_j,t,c);
}


/*  Draw a filled ellipse 
 *  ---------------------
 *  The box has width w and height h. The drawing colors are c1 and c2.
 *  NO SCALING !!!
 *  Note: here we could speedup for X11 here.
 */

#ifdef ANSI_C
static void	gs_filledellips(int x,int y,int w,int h,int c1,int c2)
#else
static void	gs_filledellips(x, y, w, h, c1, c2)
int x, y, w, h, c1, c2;
#endif
{
	int i,j;

#ifdef FAST_X11_DRAWING
	X11_fast_filledellipse(x,y,w,h,c1,c2);
#else
	for (i=0; i<=(w+1)/2; i++) {
		j = h*h/4 - i*h/w * i*h/w;
		if (j>=0) j = gstoint(sqrt((double)j));
		else j = 0;
		gs_mysolidline(x+w/2+i, y+h/2+j, x+w/2+i, y+h/2-j, 1,c1);
		gs_mysolidline(x+w/2-i, y+h/2+j, x+w/2-i, y+h/2-j, 1,c2);
	}
#endif
}


/*--------------------------------------------------------------------*/
/*   Arrow drawing auxiliaries					      */
/*--------------------------------------------------------------------*/

/* Calculate the orientation of a line
 * -----------------------------------
 * The union circle is split into 8 regions, each containing an angle
 * of ca. 45 degree. E.g. all edges (x1,y1) -> (x2,y2) going to the right
 * with a gradient between -22.5 degree and +22.5 degree have orientation
 * EAST, etc. Note that tan(22.5) is about 4/10, thus these are all edges 
 * with an absolute gradient less than 4/10, 
 * i.e. |delta_x|*10 < |delta_y|*4.
 * Similar for the other orientations.
 */

#ifdef ANSI_C
static int	gs_arroworientation(int x1,int y1,int x2,int y2)
#else
static int	gs_arroworientation(x1, y1, x2, y2)
int	x1, y1, x2, y2;
#endif
{
	int	delta_x, delta_y, abs_x, abs_y;

	delta_x = x2-x1;
	delta_y = y2-y1;
	abs_x = (delta_x < 0) ? -delta_x : delta_x;
	abs_y = (delta_y < 0) ? -delta_y : delta_y;
	if (10*abs_y <= 4*abs_x) {
		if (delta_x < 0)  return(ORI_WEST);	
		else		  return(ORI_EAST);	
	}
	else if (10*abs_x <= 4*abs_y) {
		if (delta_y < 0)  return(ORI_NORTH);	
		else		  return(ORI_SOUTH);	
	}
	else if (delta_y < 0) {
		if (delta_x > 0)	return(ORI_NORTHEAST);
		else if (delta_x < 0)	return(ORI_NORTHWEST);
	}
	else if (delta_y > 0) {
		if (delta_x > 0)	return(ORI_SOUTHEAST);
		else if (delta_x < 0)	return(ORI_SOUTHWEST);
	}

	/* Well, the following cases only occur if 10*abs_y etc.
	 * overflows. Normally, (in reasonable graphs) we never
	 * come to this point.
	 */
	if (!silent) {
        	FPRINTF(stderr,"Warning: the graph is much too large for VCG.\n");
        	FPRINTF(stderr,"The coordinates may exceed the range of integer values !\n");
        }
	if (abs_y/4 <= abs_x/10) {
		if (delta_x < 0)  return(ORI_WEST);	
		else		  return(ORI_EAST);	
	}
	else if (abs_x/4 <= abs_y/10) {
		if (delta_y < 0)  return(ORI_NORTH);	
		else		  return(ORI_SOUTH);	
	}

	/* If everything fails (then the graph is much too large
	 * for VCG, we return NORTH.
	 */
	return(ORI_NORTH);
}


/*  Draw the arrowheads of an edge
 *  ------------------------------
 *  We use global variables gbl_x1, etc. because we must change
 *  them inside the function.
 *  After drawing the arrowhead at (gbl_x1,gbl_y1) or (gbl_x2,gbl_y2) 
 *  we change gbl_x1, ... such that the line ends exactly where the
 *  arrowhead starts.
 *  s is the lenght of the katheses of the arrowhead. c is the color.
 *  All arrowheads are drawn: if the edge is reverted, then we draw 
 *  at (gbl_x1,gbl_y1), if the edge is doubled, we draw at both
 *  sides, otherwise we draw at (gbl_x2,gbl_y2).
 *
 *  Sketch: (gbl_x1,gbl_y1) =>	 *
 *				***
 *			       *****
 *				 | <= new (gbl_x1,gbl_y1) after change
 */


static int gbl_x1,gbl_x2,gbl_y1,gbl_y2;

#ifdef ANSI_C
static void draw_arrowhead(GEDGE e, int flag)
#else
static void draw_arrowhead(e, flag)
GEDGE e;
int flag;
#endif
{
	int c, s, okay, x2, y2, m, ls;

	s  = (EARROWSIZE(e)*G_stretch)/G_shrink;
	c  = EARROWCOL(e);
	m  = EARROWSTYLE(e);
	ls = (ETHICKNESS(e)*G_stretch)/G_shrink;
	if (!colored) c = BLACK;

	okay = 1;
	if (m==ASNONE) okay = 0;
	if (m==ASNONESPEC) okay = 0;
	if (flag) { if (!NANCHORNODE(EEND(e))) okay = 0; }
	else if (NANCHORNODE(EEND(e))) okay = 0;

	if (okay) {
		if ((!flag) && (G_arrowmode==AMFREE)) {
			x2 = EBBENDX(e) * G_stretch/G_shrink;
			y2 = EBBENDY(e) * G_stretch/G_shrink;
			if ((x2==gbl_x2) && (y2==gbl_y2)) {
				x2 = ETBENDX(e) * G_stretch/G_shrink;
				y2 = ETBENDY(e) * G_stretch/G_shrink;
			}
			gs_freearrowpoint(gbl_x2,gbl_y2, x2, y2, s,c,m,ls);
		}
		else switch (EORI(e)) {
		case ORI_NORTH:     gs_n_arrowpoint(gbl_x2,gbl_y2,s,c,m,ls);
				    gbl_y2 = gbl_y2 + 7 * s /10;
				    break;
		case ORI_SOUTH:     gs_s_arrowpoint(gbl_x2,gbl_y2,s,c,m,ls);
				    gbl_y2 = gbl_y2 - 7 * s /10;
				    break;
		case ORI_WEST:	    gs_w_arrowpoint(gbl_x2,gbl_y2,s,c,m,ls);
				    gbl_x2 = gbl_x2 + 7 * s /10;
				    break; 
		case ORI_EAST:	    gs_e_arrowpoint(gbl_x2,gbl_y2,s,c,m,ls);
				    gbl_x2 = gbl_x2 - 7 * s /10;
				    break; 
		case ORI_SOUTHEAST: gs_se_arrowpoint(gbl_x2,gbl_y2,s,c,m,ls);
				    gbl_x2 = gbl_x2 - s /2;
				    gbl_y2 = gbl_y2 - s /2;
				    break; 
		case ORI_NORTHWEST: gs_nw_arrowpoint(gbl_x2,gbl_y2,s,c,m,ls);
				    gbl_x2 = gbl_x2 + s /2;
				    gbl_y2 = gbl_y2 + s /2;
				    break; 
		case ORI_SOUTHWEST: gs_sw_arrowpoint(gbl_x2,gbl_y2,s,c,m,ls);
				    gbl_x2 = gbl_x2 + s /2;
				    gbl_y2 = gbl_y2 - s /2;
				    break; 
		case ORI_NORTHEAST: gs_ne_arrowpoint(gbl_x2,gbl_y2,s,c,m,ls); 
				    gbl_x2 = gbl_x2 - s /2;
				    gbl_y2 = gbl_y2 + s /2;
				    break; 
		}
	}

	s  = (EARROWBSIZE(e)*G_stretch)/G_shrink;
	c  = EARROWBCOL(e);
	m  = EARROWBSTYLE(e);
	if (!colored) c = BLACK;

	okay = 1;
	if (m==ASNONE) okay = 0;
	if (m==ASNONESPEC) okay = 0;
	if (flag) { if (!NANCHORNODE(ESTART(e))) okay = 0; }
	else if (NANCHORNODE(ESTART(e))) okay = 0;

	if (okay) {
		if ((!flag) && (G_arrowmode==AMFREE)) {
			x2 = ETBENDX(e) * G_stretch/G_shrink;
			y2 = ETBENDY(e) * G_stretch/G_shrink;
			if ((x2==gbl_x1) && (y2==gbl_y1)) {
				x2 = EBBENDX(e) * G_stretch/G_shrink;
				y2 = EBBENDY(e) * G_stretch/G_shrink;
			}
			gs_freearrowpoint(gbl_x1,gbl_y1, x2, y2, s,c,m,ls);
		}
		else switch (EORI2(e)) { 
		case ORI_NORTH:     gs_n_arrowpoint(gbl_x1,gbl_y1,s,c,m,ls);
				    gbl_y1 = gbl_y1 + 7 * s /10;
				    break;
		case ORI_SOUTH:     gs_s_arrowpoint(gbl_x1,gbl_y1,s,c,m,ls);
				    gbl_y1 = gbl_y1 - 7 * s /10;
				    break;
		case ORI_WEST:	    gs_w_arrowpoint(gbl_x1,gbl_y1,s,c,m,ls);
				    gbl_x1 = gbl_x1 + 7 * s /10;
				    break; 
		case ORI_EAST:	    gs_e_arrowpoint(gbl_x1,gbl_y1,s,c,m,ls);
				    gbl_x1 = gbl_x1 - 7 * s /10;
				    break; 
		case ORI_SOUTHEAST: gs_se_arrowpoint(gbl_x1,gbl_y1,s,c,m,ls);
				    gbl_x1 = gbl_x1 - s /2;
				    gbl_y1 = gbl_y1 - s /2;
				    break; 
		case ORI_NORTHWEST: gs_nw_arrowpoint(gbl_x1,gbl_y1,s,c,m,ls);
				    gbl_x1 = gbl_x1 + s /2;
				    gbl_y1 = gbl_y1 + s /2;
				    break; 
		case ORI_SOUTHWEST: gs_sw_arrowpoint(gbl_x1,gbl_y1,s,c,m,ls);
				    gbl_x1 = gbl_x1 + s /2;
				    gbl_y1 = gbl_y1 - s /2;
				    break; 
		case ORI_NORTHEAST: gs_ne_arrowpoint(gbl_x1,gbl_y1,s,c,m,ls); 
				    gbl_x1 = gbl_x1 - s /2;
				    gbl_y1 = gbl_y1 + s /2;
				    break; 
		}
	}
}


/*--------------------------------------------------------------------*/
/*   Solid arrow drawing onto the X11 window			      */
/*--------------------------------------------------------------------*/

/* This is a speedup of the follow edge feature
 * The functions x11_followedge_arrow and x11_followedge_anchors
 * are copies of gs_solidarrow and draw_solidanchors, but they
 * do not use the normal line functions.
 */

#ifndef INCLUDE_DRAW
#ifdef X11

#ifdef ANSI_C
void x11_followedge_arrow(GEDGE e)
#else
void x11_followedge_arrow(e)
GEDGE	e;
#endif
{
	int	x1, x2, y1, y2, x3, y3, x4, y4, c;
	int	t,topbend,botbend;

	gbl_x1 = x1 = ESTARTX(e) * G_stretch/G_shrink;
	gbl_y1 = y1 = ESTARTY(e) * G_stretch/G_shrink;
	gbl_x2 = x2 = EENDX(e) * G_stretch/G_shrink;
	gbl_y2 = y2 = EENDY(e) * G_stretch/G_shrink;
	x3 = ETBENDX(e) * G_stretch/G_shrink;
	y3 = ETBENDY(e) * G_stretch/G_shrink;
	x4 = EBBENDX(e) * G_stretch/G_shrink;
	y4 = EBBENDY(e) * G_stretch/G_shrink;

	t  = (ETHICKNESS(e)*G_stretch)/G_shrink + 3;
	if ( t==0 )	t = 1;
	c  = ECOLOR(e);
	if (!colored) c = BLACK;

	if (EANCHOR(e)==66) { x11_followedge_anchors(e); return; } 

	draw_arrowhead(e, 0);

	topbend = 0;
	if ((y3!=y1)||(x3!=x1)) { 
		topbend = 1; 
		if ((x1<x3) && (gbl_x1>x3)) topbend = 0;
		if ((x1>x3) && (gbl_x1<x3)) topbend = 0;
		if ((y1<y3) && (gbl_y1>y3)) topbend = 0;
		if ((y1>y3) && (gbl_y1<y3)) topbend = 0;
	}
	botbend = 0;
	if ((y4!=y2)||(x4!=x2)) { 
		botbend = 1; 
		if ((x2<x4) && (gbl_x2>x4)) botbend = 0;
		if ((x2>x4) && (gbl_x2<x4)) botbend = 0;
		if ((y2<y4) && (gbl_y2>y4)) botbend = 0;
		if ((y2>y4) && (gbl_y2<y4)) botbend = 0;
	}

	/* Take the co-ordinates corrected by draw_arrowhead */
		
	x1 = gbl_x1; y1 = gbl_y1; y2 = gbl_y2; x2 = gbl_x2;

	if (topbend) {
		x11_followedge_line(x1,y1,x3,y3,t,c);
		if (botbend) {
			x11_followedge_line(x3,y3,x4,y4,t,c);
			x11_followedge_line(x4,y4,x2,y2,t,c);
		} 
		else 	x11_followedge_line(x3,y3,x2,y2,t,c);
	}
	else {
		if (botbend) {
			x11_followedge_line(x1,y1,x4,y4,t,c);
			x11_followedge_line(x4,y4,x2,y2,t,c);
		}
		else 	x11_followedge_line(x1,y1,x2,y2,t,c);
	}
}


#ifdef ANSI_C
void x11_followedge_anchors(GEDGE e)
#else
void x11_followedge_anchors(e)
GEDGE e;
#endif
{
	GEDGE e1;
	ADJEDGE a;
	GNODE v;
	int   h, w, x1, x2, yb, y1, y2, xx, d;
	int   t, c;

	x1 = ESTARTX(e) * G_stretch/G_shrink;
	x2 = EENDX(e) * G_stretch/G_shrink;
	v = ESTART(e);
        gs_setshrink(G_stretch * NSTRETCH(v),
                     G_shrink  * NSHRINK(v)  );
        gs_setto(NX(v) * G_stretch / G_shrink,
                 NY(v) * G_stretch / G_shrink );
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;
	gs_calcstringsize(NLABEL(v));
	yb = myypos + (h-gs_stringh)/2 - (10*mystretch)/myshrink;

	if (CTARGET2(NCONNECT(EEND(e)))) {
		e1 = CEDGE2(NCONNECT(EEND(e)));
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink + 3;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
                x11_followedge_line(xx,y1,x2,y2,t,c);
	}

	a = NSUCC(EEND(e));
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink + 3;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb +(-EANCHOR(e1)*16*mystretch)/myshrink;  
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		x11_followedge_line(xx,y1,x2,y2,t,c);
		a = ANEXT(a);
	}
	a = NPRED(EEND(e));
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink + 3;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb +(-EANCHOR(e1)*16*mystretch)/myshrink;  
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		x11_followedge_line(xx,y1,x2,y2,t,c);
		a = ANEXT(a);
	}
}

#endif /* X11 */
#endif /* not INCLUDE_DRAW */

/*--------------------------------------------------------------------*/
/*   Solid arrow drawing					      */
/*--------------------------------------------------------------------*/

/*  Draw an solid arrow e 
 *  ---------------------
 *  This includes the drawing of the bend line and the drawing of all
 *  arrowheads, relatively to the global scaling factor 
 *  (G_stretch/G_shrink).
 */

#ifdef ANSI_C
void	gs_solidarrow(GEDGE e)
#else
void	gs_solidarrow(e)
GEDGE	e;
#endif
{
	int	x1, x2, y1, y2, x3, y3, x4, y4, c;
	int	t,topbend,botbend;

	if (G_spline) { draw_spline(e,1); return; }

	gbl_x1 = x1 = ESTARTX(e) * G_stretch/G_shrink;
	gbl_y1 = y1 = ESTARTY(e) * G_stretch/G_shrink;
	gbl_x2 = x2 = EENDX(e) * G_stretch/G_shrink;
	gbl_y2 = y2 = EENDY(e) * G_stretch/G_shrink;
	x3 = ETBENDX(e) * G_stretch/G_shrink;
	y3 = ETBENDY(e) * G_stretch/G_shrink;
	x4 = EBBENDX(e) * G_stretch/G_shrink;
	y4 = EBBENDY(e) * G_stretch/G_shrink;

	t  = (ETHICKNESS(e)*G_stretch)/G_shrink;
	if ( t==0 )	t = 1;
	c  = ECOLOR(e);
	if (!colored) c = BLACK;

	if (EANCHOR(e)==66) { draw_solidanchors(e); return; } 

	draw_arrowhead(e, 0); 

	topbend = 0;
	if ((y3!=y1)||(x3!=x1)) { 
		topbend = 1; 
		if ((x1<x3) && (gbl_x1>x3)) topbend = 0;
		if ((x1>x3) && (gbl_x1<x3)) topbend = 0;
		if ((y1<y3) && (gbl_y1>y3)) topbend = 0;
		if ((y1>y3) && (gbl_y1<y3)) topbend = 0;
	}
	botbend = 0;
	if ((y4!=y2)||(x4!=x2)) { 
		botbend = 1; 
		if ((x2<x4) && (gbl_x2>x4)) botbend = 0;
		if ((x2>x4) && (gbl_x2<x4)) botbend = 0;
		if ((y2<y4) && (gbl_y2>y4)) botbend = 0;
		if ((y2>y4) && (gbl_y2<y4)) botbend = 0;
	}

	/* Take the co-ordinates corrected by draw_arrowhead */
		
	x1 = gbl_x1; y1 = gbl_y1; y2 = gbl_y2; x2 = gbl_x2;

	if (topbend) {
		gs_mysolidline(x1,y1,x3,y3,t,c);
		if (botbend) {
			gs_mysolidline(x3,y3,x4,y4,t,c);
			gs_mysolidline(x4,y4,x2,y2,t,c);
		} 
		else 	gs_mysolidline(x3,y3,x2,y2,t,c);
	}
	else {
		if (botbend) {
			gs_mysolidline(x1,y1,x4,y4,t,c);
			gs_mysolidline(x4,y4,x2,y2,t,c);
		}
		else 	gs_mysolidline(x1,y1,x2,y2,t,c);
	}

	if ((G_dirtyel||locFlag) && G_displayel) { gs_labelbox(e); }
}


/* Draw the solid anchor lines instead an anchor edge
 * --------------------------------------------------
 */

#ifdef ANSI_C
static void draw_solidanchors(GEDGE e)
#else
static void draw_solidanchors(e)
GEDGE e;
#endif
{
	GEDGE e1;
	ADJEDGE a;
	GNODE v;
	int   h, w, x1, x2, yb, y1, y2, xx, d;
	int   t, c;

	x1 = ESTARTX(e) * G_stretch/G_shrink;
	x2 = EENDX(e) * G_stretch/G_shrink;
	v = ESTART(e);
        gs_setshrink(G_stretch * NSTRETCH(v),
                     G_shrink  * NSHRINK(v)  );
        gs_setto(NX(v) * G_stretch / G_shrink,
                 NY(v) * G_stretch / G_shrink );
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;
	gs_calcstringsize(NLABEL(v));
	yb = myypos + (h-gs_stringh)/2 - (10*mystretch)/myshrink;

	if (CTARGET2(NCONNECT(EEND(e)))) {
		e1 = CEDGE2(NCONNECT(EEND(e)));
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x1 = xx;
		gbl_y1 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x1;
		y1 = gbl_y1;
                gs_mysolidline(xx,y1,x2,y2,t,c);
	}

	a = NSUCC(EEND(e));
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb +(-EANCHOR(e1)*16*mystretch)/myshrink;  
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x1 = xx;
		gbl_y1 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x1;
		y1 = gbl_y1;
		gs_mysolidline(xx,y1,x2,y2,t,c);
		a = ANEXT(a);
	}
	a = NPRED(EEND(e));
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb +(-EANCHOR(e1)*16*mystretch)/myshrink;  
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x2 = xx;
		gbl_y2 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x2;
		y2 = gbl_y2;
		gs_mysolidline(xx,y1,x2,y2,t,c);
		a = ANEXT(a);
	}
}


/* Line drawing driver
 * -------------------
 * Draw a line from (x1,y1) to (x2,y3) with thickness t and color c
 */

#ifdef ANSI_C
static void gs_mysolidline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void gs_mysolidline(x1,y1,x2,y2,t,c)
int x1,y1,x2,y2,t,c;
#endif
{
	/* note: checkvisible changes gbl_x1, ..., gbl_y2 again
	 * such that after check_visible(a,b,c,d) holds:
	 *     gbl_x1 = replacement for a 
	 *     gbl_y1 = replacement for b 
	 *     gbl_x2 = replacement for c
	 *     gbl_y2 = replacement for d 
	 */
		
	switch (gs_arroworientation(x1,y1,x2,y2)) {
	case ORI_NORTH:     if (check_visible(x2,y2,x1,y1)) 
				gs_sosolidline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTH:     if (check_visible(x1,y1,x2,y2)) 
				gs_sosolidline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_WEST:	    if (check_visible(x1,y1,x2,y2)) 
				gs_wesolidline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_EAST:	    if (check_visible(x2,y2,x1,y1)) 
				gs_wesolidline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTHEAST: if (check_visible(x1,y1,x2,y2)) 
				gs_sesolidline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_NORTHWEST: if (check_visible(x2,y2,x1,y1)) 
				gs_sesolidline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTHWEST: if (check_visible(x1,y1,x2,y2)) 
				gs_swsolidline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_NORTHEAST: if (check_visible(x2,y2,x1,y1)) 
				gs_swsolidline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	}
}


/*  Basic line drawing:
 *  ===================
 *  We draw small rectangles at the end of the arrows. This ensures
 *  that the concatenation of lines is smooth. Note however that 
 *  the line thickness should not be larger than the border or the
 *  arrowheads we pick the lines on.
 */


/*  Solid line to the south
 *  -----------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_sosolidline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_sosolidline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i,e1,e2;

	assert((y2>=y1)); 

#ifdef FAST_X11_DRAWING
	X11_fast_line(x1,y1,x2,y2,t,c);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_thickline(x1,y1,x2,y2,t,c);
#else
	gs_line(x1,y1,x2,y2,c);
	if (t<=1) return;
	e1 = t/2;
	e2 = t/2;
	if (t%2==1) e2++;
	for (i=0; i<e1; i++) gs_line(x1-i,y1-e1+1+i,x2-i,y2+e1-1-i,c);
	for (i=0; i<e2; i++) gs_line(x1+i,y1-e2+1+i,x2+i,y2+e2-1-i,c);
#endif
#endif
}

/*  Solid line to the west 
 *  ----------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_wesolidline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_wesolidline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i,e1,e2;

	assert((x2<=x1)); 

#ifdef FAST_X11_DRAWING
	X11_fast_line(x1,y1,x2,y2,t,c);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_thickline(x1,y1,x2,y2,t,c);
#else
	gs_line(x1,y1,x2,y2,c);
	if (t<=1) return;
	e1 = t/2;
	e2 = t/2;
	if (t%2==1) e2++;
	for (i=0; i<e1; i++) gs_line(x1+e1-1-i,y1-i,x2-e1+1+i,y2-i,c);
	for (i=0; i<e2; i++) gs_line(x1+e2-1-i,y1+i,x2-e2+1+i,y2+i,c);
#endif
#endif
}

/*  Solid line to the southeast 
 *  ---------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_sesolidline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_sesolidline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i,w;

	assert((y2>=y1));
	assert((x2>=x1));
	
#ifdef FAST_X11_DRAWING
	X11_fast_line(x1,y1,x2,y2,t,c);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_thickline(x1,y1,x2,y2,t,c);
#else
	w = 7*t/10;
	gs_line(x1,y1,x2,y2,c);
	if (w<=1) return;
	if (w%1) {
		for (i=0; i<w; i++) gs_line(x1-w/2+i,y1-w/2,x2+w/2,y2+w/2-i,c);
		for (i=0; i<w; i++) gs_line(x1-w/2,y1-w/2+i,x2+w/2-i,y2+w/2,c);
		return;
	}
	for (i=0; i<w; i++) gs_line(x1-w/2+i,y1-w/2,x2+w/2-1,y2+w/2-i-1,c);
	for (i=0; i<w; i++) gs_line(x1-w/2,y1-w/2+i,x2+w/2-1-i,y2+w/2-1,c);
#endif
#endif
}

/*  Solid line to the southwest 
 *  ---------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_swsolidline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_swsolidline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i,w;

	assert((y2>=y1));
	assert((x2<=x1));
	
#ifdef FAST_X11_DRAWING
	X11_fast_line(x1,y1,x2,y2,t,c);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_thickline(x1,y1,x2,y2,t,c);
#else
	w = 7*t/10;
	gs_line(x1,y1,x2,y2,c);
	if (w<=1) return;
	if (w%1) {
		for (i=0; i<w; i++) gs_line(x1+w/2-i,y1-w/2,x2-w/2,y2+w/2-i,c);
		for (i=0; i<w; i++) gs_line(x1+w/2,y1-w/2+i,x2-w/2+i,y2+w/2,c);
		return;
	}
	for (i=0; i<w; i++) gs_line(x1+w/2-i,y1-w/2,x2-w/2+1,y2+w/2-1-i,c);
	for (i=0; i<w; i++) gs_line(x1+w/2,y1-w/2+i,x2-w/2+1+i,y2+w/2-1,c);
#endif
#endif
}


/*--------------------------------------------------------------------*/
/*   Dashed arrow drawing					      */
/*--------------------------------------------------------------------*/

/*  Draw an dashed arrow e 
 *  ----------------------
 *  This includes the drawing of the bend line and the drawing of all
 *  arrowheads, relatively to the global scaling factor 
 *  (G_stretch/G_shrink).
 */

#ifdef ANSI_C
void	gs_dashedarrow(GEDGE e)
#else
void	gs_dashedarrow(e)
GEDGE	e;
#endif
{
	int	x1, x2, y1, y2, x3, y3, x4, y4, c;
	int	t,topbend,botbend;

	if (G_spline) { draw_spline(e,1); return; }

	gbl_x1 = x1 = ESTARTX(e) * G_stretch/G_shrink;
	gbl_y1 = y1 = ESTARTY(e) * G_stretch/G_shrink;
	gbl_x2 = x2 = EENDX(e) * G_stretch/G_shrink;
	gbl_y2 = y2 = EENDY(e) * G_stretch/G_shrink;
	x3 = ETBENDX(e) * G_stretch/G_shrink;
	y3 = ETBENDY(e) * G_stretch/G_shrink;
	x4 = EBBENDX(e) * G_stretch/G_shrink;
	y4 = EBBENDY(e) * G_stretch/G_shrink;

	t  = (ETHICKNESS(e)*G_stretch)/G_shrink;
	if ( t==0 )	t = 1;
	c  = ECOLOR(e);
	if (!colored) c = BLACK;

	if (EANCHOR(e)==66) { draw_dashedanchors(e); return; } 

	draw_arrowhead(e, 0);

	topbend = 0;
	if ((y3!=y1)||(x3!=x1)) { 
		topbend = 1; 
		if ((x1<x3) && (gbl_x1>x3)) topbend = 0;
		if ((x1>x3) && (gbl_x1<x3)) topbend = 0;
		if ((y1<y3) && (gbl_y1>y3)) topbend = 0;
		if ((y1>y3) && (gbl_y1<y3)) topbend = 0;
	}
	botbend = 0;
	if ((y4!=y2)||(x4!=x2)) { 
		botbend = 1; 
		if ((x2<x4) && (gbl_x2>x4)) botbend = 0;
		if ((x2>x4) && (gbl_x2<x4)) botbend = 0;
		if ((y2<y4) && (gbl_y2>y4)) botbend = 0;
		if ((y2>y4) && (gbl_y2<y4)) botbend = 0;
	}

	/* Take the co-ordinates corrected by draw_arrowhead */
		
	x1 = gbl_x1; y1 = gbl_y1; y2 = gbl_y2; x2 = gbl_x2;

	if (topbend) {
		gs_mydashedline(x1,y1,x3,y3,t,c);
		if (botbend) {
			gs_mydashedline(x3,y3,x4,y4,t,c);
			gs_mydashedline(x4,y4,x2,y2,t,c);
		} 
		else 	gs_mydashedline(x3,y3,x2,y2,t,c);
	}
	else {
		if (botbend) {
			gs_mydashedline(x1,y1,x4,y4,t,c);
			gs_mydashedline(x4,y4,x2,y2,t,c);
		}
		else 	gs_mydashedline(x1,y1,x2,y2,t,c);
	}

	if ((G_dirtyel||locFlag) && G_displayel) { gs_labelbox(e); }
}


/* Draw the dashed anchor lines instead an anchor edge
 * --------------------------------------------------
 */

#ifdef ANSI_C
static void draw_dashedanchors(GEDGE e)
#else
static void draw_dashedanchors(e)
GEDGE e;
#endif
{
	GEDGE e1;
	ADJEDGE a;
	GNODE v;
	int   h, w, x1, x2, yb, y1, y2, xx, d;
	int   t, c;

	x1 = ESTARTX(e) * G_stretch/G_shrink;
	x2 = EENDX(e) * G_stretch/G_shrink;
	v = ESTART(e);
        gs_setshrink(G_stretch * NSTRETCH(v),
                     G_shrink  * NSHRINK(v)  );
        gs_setto(NX(v) * G_stretch / G_shrink,
                 NY(v) * G_stretch / G_shrink );
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;
	gs_calcstringsize(NLABEL(v));
	yb = myypos + (h-gs_stringh)/2 - (10*mystretch)/myshrink;

	if (CTARGET2(NCONNECT(EEND(e)))) {
		e1 = CEDGE2(NCONNECT(EEND(e)));
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x1 = xx;
		gbl_y1 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x1;
		y1 = gbl_y1;
                gs_mydashedline(xx,y1,x2,y2,t,c);
	}

	a = NSUCC(EEND(e));
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x1 = xx;
		gbl_y1 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x1;
		y1 = gbl_y1;
		gs_mydashedline(xx,y1,x2,y2,t,c);
		a = ANEXT(a);
	}
	a = NPRED(EEND(e));
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x2 = xx;
		gbl_y2 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x2;
		y1 = gbl_y2;
		gs_mydashedline(xx,y1,x2,y2,t,c);
		a = ANEXT(a);
	}
}

/* Line drawing driver
 * -------------------
 * Draw a dashed line from (x1,y1) to (x2,y3) with thickness t and color c
 */

#ifdef ANSI_C
static void gs_mydashedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void gs_mydashedline(x1,y1,x2,y2,t,c)
int x1,y1,x2,y2,t,c;
#endif
{
	/* note: checkvisible changes gbl_x1, ..., gbl_y2 again
	 * such that after check_visible(a,b,c,d) holds:
	 *     gbl_x1 = replacement for a 
	 *     gbl_y1 = replacement for b 
	 *     gbl_x2 = replacement for c
	 *     gbl_y2 = replacement for d 
	 */
		
	switch (gs_arroworientation(x1,y1,x2,y2)) {
	case ORI_NORTH:     if (check_visible(x2,y2,x1,y1)) 
				gs_sodashedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTH:     if (check_visible(x1,y1,x2,y2)) 
				gs_sodashedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_WEST:	    if (check_visible(x1,y1,x2,y2)) 
				gs_wedashedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_EAST:	    if (check_visible(x2,y2,x1,y1)) 
				gs_wedashedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTHEAST: if (check_visible(x1,y1,x2,y2)) 
				gs_sedashedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_NORTHWEST: if (check_visible(x2,y2,x1,y1)) 
				gs_sedashedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTHWEST: if (check_visible(x1,y1,x2,y2)) 
				gs_swdashedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_NORTHEAST: if (check_visible(x2,y2,x1,y1)) 
				gs_swdashedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	}
}

/*  Basic dashed line drawing:
 *  ==========================
 *  We draw small rectangles at the end of the arrows. This ensures
 *  that the concatenation of lines is smooth. Note however that 
 *  the line thickness should not be larger than the border or the
 *  arrowheads we pick the lines on.
 */


/*  Dashed line to the south
 *  ------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_sodashedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_sodashedline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i, h, d;
	int xx1, xx2, yy1, yy2;
	int xof, yof;	/* xoffset and yoffset of the dash part */
	int flag;	/* indicates whether the edge is flat	*/
	int s1, s2;	/* gradient s1/s2 of edge		*/
	int abss1, abss2;   /* and its absolute 		*/

	if (y1==y2) return;
	assert((y2>y1));

#ifdef FAST_X11_DRAWING
	X11_fast_dashedline(x1,y1,x2,y2,t,c,15*G_stretch/G_shrink);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_dashedthickline(x1,y1,x2,y2,t,c,15*G_stretch/G_shrink);
#else

	abss1 = s1 = x2-x1;	/* s1/s2 is the gradient */
	abss2 = s2 = y2-y1;
	if (abss1<0) abss1 = -abss1;
	if (abss2<0) abss2 = -abss2;
	
	/* Because the xoffset and yoffset table contains only a 
	 * 45 degree part, we do this trick, to extend it to 90 degree.
	 */

	if ( abss1 >= abss2) {	    /* |gradient| <= 1 --> flat edge */ 
		h = (abss2*50)/abss1;
		xof  = xoffset[h];
		yof  = yoffset[h];
		flag = 0;
	}
	else {	/* abss1 < abss2 */ /* |gradient| > 1 --> steep edge */
		h = (abss1*50)/abss2;
		yof  = xoffset[h];
		xof  = yoffset[h];
		flag = 1;
	}
	xof = xof * G_stretch/G_shrink;
	yof = yof * G_stretch/G_shrink;
	if (!xof && !yof) xof = yof = 1;  /* for security */

	i = 0;
	xx1 = xx2 = x1;
	yy1 = yy2 = y1;
	if (flag || (xof==0) ) {  /* steep edge */
		/* s2 > 0, see assertion */
		d = yof;
		if (x1>x2)  
			/* y1 /> y2, x1 \> x2 */
			while( (xx2-xof>x2) || (yy2+yof<y2) ) {
				/* a bit to southwest: s1/s2<0	*/
				xx1 = x1 + (i*d*s1)/s2;
				if ( xx1<x2 )	xx1 = x2; 
				yy1 = y1 + i*d;
				if ( yy1>y2 )	yy1 = y2; 
				xx2 = xx1-xof;
				if ( xx2<x2 )	{ xx2 = x2; xx1 = x2+xof; } 
				yy2 = yy1+yof; 
				if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
				gs_sosolidline(xx1, yy1, xx2, yy2, t, c);
				i += 2;
			}
		else	/* y1 /> y2, x1 /> x2 */	
			while( (xx2+xof<x2) || (yy2+yof<y2) ) {
				/* a bit to southeast: s1/s2>0 */
				xx1 = x1 + (i*d*s1)/s2;
				if ( xx1>x2 )	xx1 = x2; 
				yy1 = y1 + i*d;
				if ( yy1>y2 )	yy1 = y2; 
				xx2 = xx1+xof;
				if ( xx2>x2 )	{ xx2 = x2; xx1 = x2-xof; } 
				yy2 = yy1+yof; 
				if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
				gs_sosolidline(xx1, yy1, xx2, yy2, t, c);
				i += 2;
			}
	}
	else /* (!flag && (xof!=0)) */ {  /* flat edge	*/
		/* |s1| >= |s2| and s2 > 0 */
		d = xof;
		if (x1>x2)
			/* y1 /> y2, x1 \> x2 */
			while ( (xx2-xof>x2) || (yy2+yof<y2) ) {
				/* a bit to southwest: s2/s1<0	*/
				xx1 = x1 - i*d;
				if ( xx1<x2 )	xx1 = x2;
				yy1 = y1 - (i*d*s2)/s1;
				if ( yy1>y2 )	yy1 = y2; 
				xx2 = xx1-xof;
				if ( xx2<x2 )	{ xx2 = x2; xx1 = x2+xof; } 
				yy2 = yy1+yof; 
				if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
				gs_sosolidline(xx1, yy1, xx2, yy2, t, c);
				i += 2;
			}
		else	/* y1 /> y2, x1 /> x2 */	
		    	while ( (xx2+xof<x2) || (yy2+yof<y2) ) {
				/* a bit to southeast: s2/s1>0 */
				xx1 = x1 + i*d;
				if ( xx1>x2 )	xx1 = x2;
				yy1 = y1 + (i*d*s2)/s1;
				if ( yy1>y2 )	yy1 = y2; 
				xx2 = xx1+xof;
				if ( xx2>x2 )	{ xx2 = x2; xx1 = x2-xof; } 
				yy2 = yy1+yof; 
				if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
				gs_sosolidline(xx1, yy1, xx2, yy2, t, c);
				i += 2;
			}
	}
#endif
#endif
} /* gs_sodashedline */



/*  Dashed line to the west 
 *  ------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_wedashedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_wedashedline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i, h, d;
	int xx1, xx2, yy1, yy2;
	int xof, yof;	/* xoffset and yoffset of the dash part */
	int flag;	/* indicates whether the edge is flat	*/
	int s1, s2;	/* gradient s1/s2 of edge		*/
	int abss1, abss2;   /* and its absolute 		*/

	if (x1==x2) return;
	assert((x2<x1)); 

#ifdef FAST_X11_DRAWING
	X11_fast_dashedline(x1,y1,x2,y2,t,c,15*G_stretch/G_shrink);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_dashedthickline(x1,y1,x2,y2,t,c,15*G_stretch/G_shrink);
#else


	abss1 = s1 = x2-x1;	/* s1/s2 is the gradient */
	abss2 = s2 = y2-y1;
	if (abss1<0) abss1 = -abss1;
	if (abss2<0) abss2 = -abss2;
	
	/* Because the xoffset and yoffset table contains only a 
	 * 45 degree part, we do this trick, to extend it to 90 degree.
	 */

	if ( abss1 >= abss2) {	    /* |gradient| <= 1 --> flat edge */ 
		h = (abss2*50)/abss1;
		xof  = xoffset[h];
		yof  = yoffset[h];
		flag = 0;
	}
	else {	/* abss1 < abss2 */ /* |gradient| > 1 --> steep edge */
		h = (abss1*50)/abss2;
		yof  = xoffset[h];
		xof  = yoffset[h];
		flag = 1;
	}
	xof = xof * G_stretch/G_shrink;
	yof = yof * G_stretch/G_shrink;
	if (!xof && !yof) xof = yof = 1;  /* for security */

	i = 0;
	xx1 = xx2 = x1;
	yy1 = yy2 = y1;
	if (flag || (xof==0) ) {  /* steep edge */
		/* |s2| > |s1| and |s1| > 0 */
		d = yof;
		if (y1>y2)  
			/* x1 \> x2, y1 \> y2 */
			while( (xx2-xof>x2) || (yy2-yof>y2) ) {
				/* a bit to northwest: s1/s2>0 */
				xx1 = x1 - (i*d*s1)/s2;
				if ( xx1<x2 )	xx1 = x2; 
				yy1 = y1 - i*d;
				if ( yy1<y2 )	yy1 = y2; 
				xx2 = xx1-xof;
				if ( xx2<x2 )	{ xx2 = x2; xx1 = x2+xof; } 
				yy2 = yy1-yof; 
				if ( yy2<y2 )	{ yy2 = y2; yy1 = y2+yof; } 
				gs_wesolidline(xx1, yy1, xx2, yy2, t, c);
				i += 2;
			}
		else	/* x1 \> x2, y1 /> y2 */	
			while( (xx2-xof>x2) || (yy2+yof<y2) ) {
				/* a bit to southwest: s1/s2<0 */
				xx1 = x1 + (i*d*s1)/s2;
				if ( xx1<x2 )	xx1 = x2; 
				yy1 = y1 + i*d;
				if ( yy1>y2 )	yy1 = y2; 
				xx2 = xx1-xof;
				if ( xx2<x2 )	{ xx2 = x2; xx1 = x2+xof; } 
				yy2 = yy1+yof; 
				if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
				gs_wesolidline(xx1, yy1, xx2, yy2, t, c);
				i += 2;
			}
	}
	else /* (!flag && (xof!=0)) */ {  /* flat edge	*/
		/* |s1| > 0 */
		d = xof;
		if (y1>y2)
			/* x1 \> x2, y1 \> y2 */
			while ( (xx2-xof>x2) || (yy2-yof>y2) ) {
				/* a bit to northwest: s2/s1>0 */
				xx1 = x1 - i*d;
				if ( xx1<x2 )	xx1 = x2;
				yy1 = y1 - (i*d*s2)/s1;
				if ( yy1<y2 )	yy1 = y2; 
				xx2 = xx1-xof;
				if ( xx2<x2 )	{ xx2 = x2; xx1 = x2+xof; } 
				yy2 = yy1-yof; 
				if ( yy2<y2 )	{ yy2 = y2; yy1 = y2+yof; } 
				gs_wesolidline(xx1, yy1, xx2, yy2, t, c);
				i += 2;
			}
		else	/* x1 \> x2, y1 /> y2 */	
			while ( (xx2-xof>x2) || (yy2+yof<y2) ) {
				/* a bit to southwest: s2/s1<0 */
				xx1 = x1 - i*d;
				if ( xx1<x2 )	xx1 = x2;
				yy1 = y1 - (i*d*s2)/s1;
				if ( yy1>y2 )	yy1 = y2; 
				xx2 = xx1-xof;
				if ( xx2<x2 )	{ xx2 = x2; xx1 = x2+xof; } 
				yy2 = yy1+yof; 
				if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
				gs_wesolidline(xx1, yy1, xx2, yy2, t, c);
				i += 2;
			}
	}
#endif
#endif
} /* gs_wedashedline */


/*  Dashed line to the southeast 
 *  ----------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_sedashedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_sedashedline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i, h, d;
	int xx1, xx2, yy1, yy2;
	int xof, yof;	/* xoffset and yoffset of the dash part */
	int flag;	/* indicates whether the edge is flat	*/
	int s1, s2;	/* gradient s1/s2 of edge		*/
	int abss1, abss2;   /* and its absolute 		*/

	if ((x1==x2) || (y1==y2)) return;
	assert((y2>y1));
	assert((x2>x1));

#ifdef FAST_X11_DRAWING
	X11_fast_dashedline(x1,y1,x2,y2,t,c,15*G_stretch/G_shrink);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_dashedthickline(x1,y1,x2,y2,t,c,15*G_stretch/G_shrink);
#else


	abss1 = s1 = x2-x1;	/* s1/s2 is the gradient */
	abss2 = s2 = y2-y1;
	if (abss1<0) abss1 = -abss1;
	if (abss2<0) abss2 = -abss2;
	
	/* Because the xoffset and yoffset table contains only a 
	 * 45 degree part, we do this trick, to extend it to 90 degree.
	 */

	if ( abss1 >= abss2) {	    /* |gradient| <= 1 --> flat edge */ 
		h = (abss2*50)/abss1;
		xof  = xoffset[h];
		yof  = yoffset[h];
		flag = 0;
	}
	else {	/* abss1 < abss2 */ /* |gradient| > 1 --> steep edge */
		h = (abss1*50)/abss2;
		yof  = xoffset[h];
		xof  = yoffset[h];
		flag = 1;
	}
	xof = xof * G_stretch/G_shrink;
	yof = yof * G_stretch/G_shrink;
	if (!xof && !yof) xof = yof = 1;  /* for security */

	i = 0;
	xx1 = xx2 = x1;
	yy1 = yy2 = y1;
	if (flag || (xof==0) ) {  /* steep edge */
		/* |s2| > 0 and |s1| > 0 */
		d = yof;
		/* x1 /> x2, y1 /> y2 */
		while( (xx2+xof<x2) || (yy2+yof<y2) ) {
			/* to southeast: s1/s2>0 */
			xx1 = x1 + (i*d*s1)/s2;
			if ( xx1>x2 )	xx1 = x2; 
			yy1 = y1 + i*d;
			if ( yy1>y2 )	yy1 = y2; 
			xx2 = xx1+xof;
			if ( xx2>x2 )	{ xx2 = x2; xx1 = x2-xof; } 
			yy2 = yy1+yof; 
			if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
			gs_sesolidline(xx1, yy1, xx2, yy2, t, c);
			i += 2;
		}
	}
	else /* (!flag && (xof!=0)) */ {  /* flat edge	*/
		/* |s2| > 0 and |s1| > 0 */
		d = xof;
		/* x1 /> x2, y1 /> y2 */
		while ( (xx2+xof<x2) || (yy2+yof<y2) ) {
			/* to southeast: s2/s1>0 */
			xx1 = x1 + i*d;
			if ( xx1>x2 )	xx1 = x2;
			yy1 = y1 + (i*d*s2)/s1;
			if ( yy1>y2 )	yy1 = y2; 
			xx2 = xx1+xof;
			if ( xx2>x2 )	{ xx2 = x2; xx1 = x2-xof; } 
			yy2 = yy1+yof; 
			if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
			gs_sesolidline(xx1, yy1, xx2, yy2, t, c);
			i += 2;
		}
	}
#endif
#endif
} /* gs_sedashedline */


/*  Dashed line to the southwest 
 *  ----------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_swdashedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_swdashedline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i, h, d;
	int xx1, xx2, yy1, yy2;
	int xof, yof;	/* xoffset and yoffset of the dash part */
	int flag;	/* indicates whether the edge is flat	*/
	int s1, s2;	/* gradient s1/s2 of edge		*/
	int abss1, abss2;   /* and its absolute 		*/

	if ((x1==x2) || (y1==y2)) return;
	assert((y2>y1));
	assert((x2<x1));

#ifdef FAST_X11_DRAWING
	X11_fast_dashedline(x1,y1,x2,y2,t,c,15*G_stretch/G_shrink);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_dashedthickline(x1,y1,x2,y2,t,c,15*G_stretch/G_shrink);
#else


	abss1 = s1 = x2-x1;	/* s1/s2 is the gradient */
	abss2 = s2 = y2-y1;
	if (abss1<0) abss1 = -abss1;
	if (abss2<0) abss2 = -abss2;
	
	/* Because the xoffset and yoffset table contains only a 
	 * 45 degree part, we do this trick, to extend it to 90 degree.
	 */

	if ( abss1 >= abss2) {	    /* |gradient| <= 1 --> flat edge */ 
		h = (abss2*50)/abss1;
		xof  = xoffset[h];
		yof  = yoffset[h];
		flag = 0;
	}
	else {	/* abss1 < abss2 */ /* |gradient| > 1 --> steep edge */
		h = (abss1*50)/abss2;
		yof  = xoffset[h];
		xof  = yoffset[h];
		flag = 1;
	}
	xof = xof * G_stretch/G_shrink;
	yof = yof * G_stretch/G_shrink;
	if (!xof && !yof) xof = yof = 1;  /* for security */

	i = 0;
	xx1 = xx2 = x1;
	yy1 = yy2 = y1;
	if (flag || (xof==0) ) {  /* steep edge */
		/* |s2| > 0 and |s1| > 0 */
		d = yof;
		/* x1 \> x2, y1 /> y2 */
		while( (xx2-xof>x2) || (yy2+yof<y2) ) {
			/* to southwest: s1/s2<0 */
			xx1 = x1 + (i*d*s1)/s2;
			if ( xx1<x2 )	xx1 = x2; 
			yy1 = y1 + i*d;
			if ( yy1>y2 )	yy1 = y2; 
			xx2 = xx1-xof;
			if ( xx2<x2 )	{ xx2 = x2; xx1 = x2+xof; } 
			yy2 = yy1+yof; 
			if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
			gs_swsolidline(xx1, yy1, xx2, yy2, t, c);
			i += 2;
		}
	}
	else /* (!flag && (xof!=0)) */ {  /* flat edge	*/
		/* |s2| > 0 and |s1| > 0 */
		d = xof;
		/* x1 \> x2, y1 /> y2 */
		while ( (xx2-xof>x2) || (yy2+yof<y2) ) {
			/* to southwest: s2/s1<0 */
			xx1 = x1 - i*d;
			if ( xx1<x2 )	xx1 = x2;
			yy1 = y1 - (i*d*s2)/s1;
			if ( yy1>y2 )	yy1 = y2; 
			xx2 = xx1-xof;
			if ( xx2<x2 )	{ xx2 = x2; xx1 = x2+xof; } 
			yy2 = yy1+yof; 
			if ( yy2>y2 )	{ yy2 = y2; yy1 = y2-yof; } 
			gs_swsolidline(xx1, yy1, xx2, yy2, t, c);
			i += 2;
		}
	}
#endif
#endif
} /* gs_swdashedline */


/*--------------------------------------------------------------------*/
/*   Dotted arrow drawing					      */
/*--------------------------------------------------------------------*/

/*  Draw an dotted arrow e 
 *  ----------------------
 *  This includes the drawing of the bend line and the drawing of all
 *  arrowheads, relatively to the global scaling factor 
 *  (G_stretch/G_shrink).
 */

#ifdef ANSI_C
void	gs_dottedarrow(GEDGE e)
#else
void	gs_dottedarrow(e)
GEDGE	e;
#endif
{
	int	x1, x2, y1, y2, x3, y3, x4, y4, c;
	int	t,topbend,botbend;

	if (G_spline) { draw_spline(e,1); return; }

	gbl_x1 = x1 = ESTARTX(e) * G_stretch/G_shrink;
	gbl_y1 = y1 = ESTARTY(e) * G_stretch/G_shrink;
	gbl_x2 = x2 = EENDX(e) * G_stretch/G_shrink;
	gbl_y2 = y2 = EENDY(e) * G_stretch/G_shrink;
	x3 = ETBENDX(e) * G_stretch/G_shrink;
	y3 = ETBENDY(e) * G_stretch/G_shrink;
	x4 = EBBENDX(e) * G_stretch/G_shrink;
	y4 = EBBENDY(e) * G_stretch/G_shrink;

	t  = (ETHICKNESS(e)*G_stretch)/G_shrink;
	if ( t==0 )	t = 1;
	c  = ECOLOR(e);
	if (!colored) c = BLACK;

	if (EANCHOR(e)==66) { draw_dottedanchors(e); return; } 

	draw_arrowhead(e, 0);

	topbend = 0;
	if ((y3!=y1)||(x3!=x1)) { 
		topbend = 1; 
		if ((x1<x3) && (gbl_x1>x3)) topbend = 0;
		if ((x1>x3) && (gbl_x1<x3)) topbend = 0;
		if ((y1<y3) && (gbl_y1>y3)) topbend = 0;
		if ((y1>y3) && (gbl_y1<y3)) topbend = 0;
	}
	botbend = 0;
	if ((y4!=y2)||(x4!=x2)) { 
		botbend = 1; 
		if ((x2<x4) && (gbl_x2>x4)) botbend = 0;
		if ((x2>x4) && (gbl_x2<x4)) botbend = 0;
		if ((y2<y4) && (gbl_y2>y4)) botbend = 0;
		if ((y2>y4) && (gbl_y2<y4)) botbend = 0;
	}

	/* Take the co-ordinates corrected by draw_arrowhead */
		
	x1 = gbl_x1; y1 = gbl_y1; y2 = gbl_y2; x2 = gbl_x2;

	if (topbend) {
		gs_mydottedline(x1,y1,x3,y3,t,c);
		if (botbend) {
			gs_mydottedline(x3,y3,x4,y4,t,c);
			gs_mydottedline(x4,y4,x2,y2,t,c);
		} 
		else 	gs_mydottedline(x3,y3,x2,y2,t,c);
	}
	else {
		if (botbend) {
			gs_mydottedline(x1,y1,x4,y4,t,c);
			gs_mydottedline(x4,y4,x2,y2,t,c);
		}
		else 	gs_mydottedline(x1,y1,x2,y2,t,c);
	}

	if ((G_dirtyel||locFlag) && G_displayel) { gs_labelbox(e); }
}


/* Draw the solid anchor lines instead an anchor edge
 * --------------------------------------------------
 */

#ifdef ANSI_C
static void draw_dottedanchors(GEDGE e)
#else
static void draw_dottedanchors(e)
GEDGE e;
#endif
{
	GEDGE e1;
	ADJEDGE a;
	GNODE v;
	int   h, w, x1, x2, yb, y1, y2, xx, d;
	int   t, c;

	x1 = ESTARTX(e) * G_stretch/G_shrink;
	x2 = EENDX(e) * G_stretch/G_shrink;
	v = ESTART(e);
        gs_setshrink(G_stretch * NSTRETCH(v),
                     G_shrink  * NSHRINK(v)  );
        gs_setto(NX(v) * G_stretch / G_shrink,
                 NY(v) * G_stretch / G_shrink );
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;
	gs_calcstringsize(NLABEL(v));
	yb = myypos + (h-gs_stringh)/2 - (10*mystretch)/myshrink;

	if (CTARGET2(NCONNECT(EEND(e)))) {
		e1 = CEDGE2(NCONNECT(EEND(e)));
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x1 = xx;
		gbl_y1 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x1;
		y1 = gbl_y1;
                gs_mydottedline(xx,y1,x2,y2,t,c);
	}


	a = NSUCC(EEND(e));
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x1 = xx;
		gbl_y1 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x1;
		y1 = gbl_y1;
		gs_mydottedline(xx,y1,x2,y2,t,c);
		a = ANEXT(a);
	}
	a = NPRED(EEND(e));
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x2 = xx;
		gbl_y2 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x2;
		y1 = gbl_y2;
		gs_mydottedline(xx,y1,x2,y2,t,c);
		a = ANEXT(a);
	}
}


/* Line drawing driver
 * -------------------
 * Draw a dotted line from (x1,y1) to (x2,y3) with thickness t and color c
 */

#ifdef ANSI_C
static void gs_mydottedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void gs_mydottedline(x1,y1,x2,y2,t,c)
int x1,y1,x2,y2,t,c;
#endif
{
	/* note: checkvisible changes gbl_x1, ..., gbl_y2 again
	 * such that after check_visible(a,b,c,d) holds:
	 *     gbl_x1 = replacement for a 
	 *     gbl_y1 = replacement for b 
	 *     gbl_x2 = replacement for c
	 *     gbl_y2 = replacement for d 
	 */
		
	switch (gs_arroworientation(x1,y1,x2,y2)) {
	case ORI_NORTH:     if (check_visible(x2,y2,x1,y1)) 
				gs_sodottedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTH:     if (check_visible(x1,y1,x2,y2)) 
				gs_sodottedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_WEST:	    if (check_visible(x1,y1,x2,y2)) 
				gs_wedottedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_EAST:	    if (check_visible(x2,y2,x1,y1)) 
				gs_wedottedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTHEAST: if (check_visible(x1,y1,x2,y2)) 
				gs_sedottedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_NORTHWEST: if (check_visible(x2,y2,x1,y1)) 
				gs_sedottedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_SOUTHWEST: if (check_visible(x1,y1,x2,y2)) 
				gs_swdottedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	case ORI_NORTHEAST: if (check_visible(x2,y2,x1,y1)) 
				gs_swdottedline(gbl_x1,gbl_y1,
					       gbl_x2,gbl_y2,t,c); 
			    break;
	}
}


/*  Basic dotted line drawing:
 *  ==========================
 *  We draw small rectangles at the end of the arrows. This ensures
 *  that the concatenation of lines is smooth. Note however that 
 *  the line thickness should not be larger than the border or the
 *  arrowheads we pick the lines on.
 *  The algorithm is the basically the same as for dashed lines.
 *  However, the dashes are now so small that they are dots.
 */


/*  Dotted line to the south
 *  ------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_sodottedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_sodottedline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i, h, d;
	int xx1, yy1;
	int xof, yof;	/* xoffset and yoffset of space between dots */
	int flag;	/* indicates whether the edge is flat	*/
	int s1, s2;	/* gradient s1/s2 of edge		*/
	int abss1, abss2;   /* and its absolute 		*/

	if (y1==y2) return;
	assert((y2>y1));

#ifdef FAST_X11_DRAWING
	X11_fast_dottedline(x1,y1,x2,y2,t,c,15*3*G_stretch/12/G_shrink);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_dottedthickline(x1,y1,x2,y2,t,c,15*3*G_stretch/12/G_shrink);
#else

	abss1 = s1 = x2-x1;	/* s1/s2 is the gradient */
	abss2 = s2 = y2-y1;
	if (abss1<0) abss1 = -abss1;
	if (abss2<0) abss2 = -abss2;
	
	/* Because the xoffset and yoffset table contains only a 
	 * 45 degree part, we do this trick, to extend it to 90 degree.
	 */

	if ( abss1 >= abss2) {	    /* |gradient| <= 1 --> flat edge */ 
		h = (abss2*50)/abss1;
		xof  = xoffset[h]*3/12;
		yof  = yoffset[h]*3/12;
		flag = 0;
	}
	else {	/* abss1 < abss2 */ /* |gradient| > 1 --> steep edge */
		h = (abss1*50)/abss2;
		yof  = xoffset[h]*3/12;
		xof  = yoffset[h]*3/12;
		flag = 1;
	}
	xof = xof * G_stretch/G_shrink;
	yof = yof * G_stretch/G_shrink;
	if (!xof && !yof) xof = yof = 1;  /* for security */

	i = 0;
	xx1 = x1;
	yy1 = y1;
	if (flag || (xof==0) ) {  /* steep edge */
		/* s2 > 0, see assertion */
		d = yof;
		if (x1>x2)  
			/* y1 /> y2, x1 \> x2 */ 
			while( (xx1-xof>x2) || (yy1+yof<y2) ) {
				/* a bit to southwest: s1/s2<0	*/
				xx1 = x1 + (i*d*s1)/s2;
				if ( xx1<x2 )	xx1 = x2; 
				yy1 = y1 + i*d;
				if ( yy1>y2 )	yy1 = y2; 
				gs_sosolidline(xx1, yy1, xx1, yy1, t, c);
				i += 2;
			}
		else	/* y1 /> y2, x1 /> x2 */ 
		    	while( (xx1+xof<x2) || (yy1+yof<y2) ) {
				/* a bit to southeast: s1/s2>0 */
				xx1 = x1 + (i*d*s1)/s2;
				if ( xx1>x2 )	xx1 = x2; 
				yy1 = y1 + i*d;
				if ( yy1>y2 )	yy1 = y2; 
				gs_sosolidline(xx1, yy1, xx1, yy1, t, c);
				i += 2;
			}
	}
	else /* (!flag && (xof!=0)) */ {  /* flat edge	*/
		/* |s1| >= |s2| and s2 > 0 */
		d = xof;
		if (x1>x2)
			/* y1 /> y2, x1 \> x2 */
			while ( (xx1-xof>x2) || (yy1+yof<y2) ) {
				/* a bit to southwest: s2/s1<0	*/
				xx1 = x1 - i*d;
				if ( xx1<x2 )	xx1 = x2;
				yy1 = y1 - (i*d*s2)/s1;
				if ( yy1>y2 )	yy1 = y2; 
				gs_sosolidline(xx1, yy1, xx1, yy1, t, c);
				i += 2;
			}
		else 	/* y1 /> y2, x1 /> x2 */	
			while ( (xx1+xof<x2) || (yy1+yof<y2) ) {
				/* a bit to southeast: s2/s1>0 */
				xx1 = x1 + i*d;
				if ( xx1>x2 )	xx1 = x2;
				yy1 = y1 + (i*d*s2)/s1;
				if ( yy1>y2 )	yy1 = y2; 
				gs_sosolidline(xx1, yy1, xx1, yy1, t, c);
				i += 2;
			}
	}

#endif
#endif

} /* gs_sodottedline */



/*  Dotted line to the west 
 *  ------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_wedottedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_wedottedline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i, h, d;
	int xx1, yy1;
	int xof, yof;	/* xoffset and yoffset of space between dots */
	int flag;	/* indicates whether the edge is flat	*/
	int s1, s2;	/* gradient s1/s2 of edge		*/
	int abss1, abss2;   /* and its absolute 		*/

	if (x1==x2) return;
	assert((x2<x1)); 

#ifdef FAST_X11_DRAWING
	X11_fast_dottedline(x1,y1,x2,y2,t,c,15*3*G_stretch/12/G_shrink);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_dottedthickline(x1,y1,x2,y2,t,c,15*3*G_stretch/12/G_shrink);
#else

	abss1 = s1 = x2-x1;	/* s1/s2 is the gradient */
	abss2 = s2 = y2-y1;
	if (abss1<0) abss1 = -abss1;
	if (abss2<0) abss2 = -abss2;
	
	/* Because the xoffset and yoffset table contains only a 
	 * 45 degree part, we do this trick, to extend it to 90 degree.
	 */

	if ( abss1 >= abss2) {	    /* |gradient| <= 1 --> flat edge */ 
		h = (abss2*50)/abss1;
		xof  = xoffset[h]*3/12;
		yof  = yoffset[h]*3/12;
		flag = 0;
	}
	else {	/* abss1 < abss2 */ /* |gradient| > 1 --> steep edge */
		h = (abss1*50)/abss2;
		yof  = xoffset[h]*3/12;
		xof  = yoffset[h]*3/12;
		flag = 1;
	}
	xof = xof * G_stretch/G_shrink;
	yof = yof * G_stretch/G_shrink;
	if (!xof && !yof) xof = yof = 1;  /* for security */

	i = 0;
	xx1 = x1;
	yy1 = y1;
	if (flag || (xof==0) ) {  /* steep edge */
		/* |s2| > |s1| and |s1| > 0 */
		d = yof;
		if (y1>y2) 
			/* x1 \> x2, y1 \> y2 */ 
			while( (xx1-xof>x2) || (yy1-yof>y2) ) {
				/* a bit to northwest: s1/s2>0 */
				xx1 = x1 - (i*d*s1)/s2;
				if ( xx1<x2 )	xx1 = x2; 
				yy1 = y1 - i*d;
				if ( yy1<y2 )	yy1 = y2; 
				gs_wesolidline(xx1, yy1, xx1, yy1, t, c);
				i += 2;
			}
		else 	/* x1 \> x2, y1 /> y2 */
			while( (xx1-xof>x2) || (yy1+yof<y2) ) {
				/* a bit to southwest: s1/s2<0 */
				xx1 = x1 + (i*d*s1)/s2;
				if ( xx1<x2 )	xx1 = x2; 
				yy1 = y1 + i*d;
				if ( yy1>y2 )	yy1 = y2; 
				gs_wesolidline(xx1, yy1, xx1, yy1, t, c);
				i += 2;
			}
	}
	else /* (!flag && (xof!=0)) */ {  /* flat edge	*/
		/* |s1| > 0 */
		d = xof;
		if (y1>y2)
			/* x1 \> x2, y1 \> y2 */ 
			while ( (xx1-xof>x2) || (yy1-yof>y2) ) {
				/* a bit to northwest: s2/s1>0 */
				xx1 = x1 - i*d;
				if ( xx1<x2 )	xx1 = x2;
				yy1 = y1 - (i*d*s2)/s1;
				if ( yy1<y2 )	yy1 = y2; 
				gs_wesolidline(xx1, yy1, xx1, yy1, t, c);
				i += 2;
			}
		else 	/* x1 \> x2, y1 /> y2 */
		    	while ( (xx1-xof>x2) || (yy1+yof<y2) ) {
				/* a bit to southwest: s2/s1<0 */
				xx1 = x1 - i*d;
				if ( xx1<x2 )	xx1 = x2;
				yy1 = y1 - (i*d*s2)/s1;
				if ( yy1>y2 )	yy1 = y2; 
				gs_wesolidline(xx1, yy1, xx1, yy1, t, c);
				i += 2;
			}
	}

#endif
#endif

} /* gs_wedottedline */


/*  Dotted line to the southeast 
 *  ----------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_sedottedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_sedottedline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i, h, d;
	int xx1, yy1;
	int xof, yof;	/* xoffset and yoffset of space between dots */
	int flag;	/* indicates whether the edge is flat	*/
	int s1, s2;	/* gradient s1/s2 of edge		*/
	int abss1, abss2;   /* and its absolute 		*/

	if ((x1==x2) || (y1==y2)) return;
	assert((y2>y1));
	assert((x2>x1));

#ifdef FAST_X11_DRAWING
	X11_fast_dottedline(x1,y1,x2,y2,t,c,15*3*G_stretch/12/G_shrink);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_dottedthickline(x1,y1,x2,y2,t,c,15*3*G_stretch/12/G_shrink);
#else

	abss1 = s1 = x2-x1;	/* s1/s2 is the gradient */
	abss2 = s2 = y2-y1;
	if (abss1<0) abss1 = -abss1;
	if (abss2<0) abss2 = -abss2;
	
	/* Because the xoffset and yoffset table contains only a 
	 * 45 degree part, we do this trick, to extend it to 90 degree.
	 */

	if ( abss1 >= abss2) {	    /* |gradient| <= 1 --> flat edge */ 
		h = (abss2*50)/abss1;
		xof  = xoffset[h]*3/12;
		yof  = yoffset[h]*3/12;
		flag = 0;
	}
	else {	/* abss1 < abss2 */ /* |gradient| > 1 --> steep edge */
		h = (abss1*50)/abss2;
		yof  = xoffset[h]*3/12;
		xof  = yoffset[h]*3/12;
		flag = 1;
	}
	xof = xof * G_stretch/G_shrink;
	yof = yof * G_stretch/G_shrink;
	if (!xof && !yof) xof = yof = 1;  /* for security */

	i = 0;
	xx1 = x1;
	yy1 = y1;
	if (flag || (xof==0) ) {  /* steep edge */
		/* |s2| > 0 and |s1| > 0 */
		d = yof;
		/* x1 /> x2, y1 /> y2 */
		while( (xx1+xof<x2) || (yy1+yof<y2) ) {
			/* to southeast: s1/s2>0 */
			xx1 = x1 + (i*d*s1)/s2;
			if ( xx1>x2 )	xx1 = x2; 
			yy1 = y1 + i*d;
			if ( yy1>y2 )	yy1 = y2; 
			gs_sesolidline(xx1, yy1, xx1, yy1, t, c);
			i += 2;
		}
	}
	else /* (!flag && (xof!=0)) */ {  /* flat edge	*/
		/* |s2| > 0 and |s1| > 0 */
		d = xof;
		/* x1 /> x2, y1 /> y2 */
		while ( (xx1+xof<x2) || (yy1+yof<y2) ) {
			/* to southeast: s2/s1>0 */
			xx1 = x1 + i*d;
			if ( xx1>x2 )	xx1 = x2;
			yy1 = y1 + (i*d*s2)/s1;
			if ( yy1>y2 )	yy1 = y2; 
			gs_sesolidline(xx1, yy1, xx1, yy1, t, c);
			i += 2;
		}
	}

#endif
#endif

} /* gs_sedottedline */


/*  Dotted line to the southwest 
 *  ----------------------------
 *  c is the color. t is the thickness of the line.
 */

#ifdef ANSI_C
static void	gs_swdottedline(int x1,int y1,int x2,int y2,int t,int c)
#else
static void	gs_swdottedline(x1, y1, x2, y2, t, c)
int	x1, y1, x2, y2, t, c;
#endif
{
	int i, h, d;
	int xx1, yy1;
	int xof, yof;	/* xoffset and yoffset of space between dots */
	int flag;	/* indicates whether the edge is flat	*/
	int s1, s2;	/* gradient s1/s2 of edge		*/
	int abss1, abss2;   /* and its absolute 		*/

	if ((x1==x2) || (y1==y2)) return;
	assert((y2>y1));
	assert((x2<x1));

#ifdef FAST_X11_DRAWING
	X11_fast_dottedline(x1,y1,x2,y2,t,c,15*3*G_stretch/12/G_shrink);
#else
#ifdef POSTSCRIPT_DEVICE
	ps_dottedthickline(x1,y1,x2,y2,t,c,15*3*G_stretch/12/G_shrink);
#else

	abss1 = s1 = x2-x1;	/* s1/s2 is the gradient */
	abss2 = s2 = y2-y1;
	if (abss1<0) abss1 = -abss1;
	if (abss2<0) abss2 = -abss2;
	
	/* Because the xoffset and yoffset table contains only a 
	 * 45 degree part, we do this trick, to extend it to 90 degree.
	 */

	if ( abss1 >= abss2) {	    /* |gradient| <= 1 --> flat edge */ 
		h = (abss2*50)/abss1;
		xof  = xoffset[h]*3/12;
		yof  = yoffset[h]*3/12;
		flag = 0;
	}
	else {	/* abss1 < abss2 */ /* |gradient| > 1 --> steep edge */
		h = (abss1*50)/abss2;
		yof  = xoffset[h]*3/12;
		xof  = yoffset[h]*3/12;
		flag = 1;
	}
	xof = xof * G_stretch/G_shrink;
	yof = yof * G_stretch/G_shrink;
	if (!xof && !yof) xof = yof = 1;  /* for security */

	i = 0;
	xx1 = x1;
	yy1 = y1;
	if (flag || (xof==0) ) {  /* steep edge */
		/* |s2| > 0 and |s1| > 0 */
		d = yof;
		/* x1 \> x2, y1 /> y2 */
		while( (xx1-xof>x2) || (yy1+yof<y2) ) {
			/* to southwest: s1/s2<0 */
			xx1 = x1 + (i*d*s1)/s2;
			if ( xx1<x2 )	xx1 = x2; 
			yy1 = y1 + i*d;
			if ( yy1>y2 )	yy1 = y2; 
			gs_swsolidline(xx1, yy1, xx1, yy1, t, c);
			i += 2;
		}
	}
	else /* (!flag && (xof!=0)) */ {  /* flat edge	*/
		/* |s2| > 0 and |s1| > 0 */
		d = xof;
		/* x1 \> x2, y1 /> y2 */
		while ( (xx1-xof>x2) || (yy1+yof<y2) ) {
			/* to southwest: s2/s1<0 */
			xx1 = x1 - i*d;
			if ( xx1<x2 )	xx1 = x2;
			yy1 = y1 - (i*d*s2)/s1;
			if ( yy1>y2 )	yy1 = y2; 
			gs_swsolidline(xx1, yy1, xx1, yy1, t, c);
			i += 2;
		}
	}

#endif
#endif

} /* gs_swdottedline */


/* Draw an anchor node
 * -------------------
 * An anchor node is an auxiliary construct that itself is invisible.
 * Instead, we draw lines from the left or right side to the lower
 * or upper side.
 * The anchor node is the connection between edges that have an anchor
 * point and their target.
 */

#ifdef ANSI_C
void    gs_anchornode(GNODE w)
#else
void    gs_anchornode(w)
GNODE w;
#endif
{
	GEDGE e,e1;
	GNODE v;
	ADJEDGE a, ac;
	int x1,x2,y1,y2;
	int c,t,h,yb;
	int conflict;

	if (G_spline) return;
	e = CEDGE(NCONNECT(w));
	t  = (ETHICKNESS(e)*G_stretch)/G_shrink;
	if ( t==0 )	t = 1;
	c  = ECOLOR(e);
	if (!colored) c = BLACK;

	x1 = EENDX(e) * G_stretch/G_shrink;
	v = ESTART(e);
        gs_setshrink(G_stretch * NSTRETCH(v),
                     G_shrink  * NSHRINK(v)  );
        gs_setto(NX(v) * G_stretch / G_shrink,
                 NY(v) * G_stretch / G_shrink );
	h = NHEIGHT(v)*G_stretch/G_shrink;
	gs_calcstringsize(NLABEL(v));
	yb = myypos + (h-gs_stringh)/2 - (10*mystretch)/myshrink;
	if (NSHAPE(v)==TRIANGLE) yb = yb + h/4;

#ifdef DEBUG
  gs_border(NX(w)* G_stretch / G_shrink,
	    NY(w)* G_stretch / G_shrink,
	    NWIDTH(w)*G_stretch/G_shrink,
	    NHEIGHT(w)*G_stretch/G_shrink,1,BLACK); 
#endif

	if (CTARGET2(NCONNECT(w))) {
		e1 = CEDGE2(NCONNECT(w));
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		x2 = ESTARTX(e1) * G_stretch/G_shrink;
		y2 = ESTARTY(e1) * G_stretch/G_shrink;
		if (manhatten_edges==1) {
			switch (ELSTYLE(e1)) {
                        case SOLID:  gs_mysolidline(x1,y1,x2,y1,t,c);
                                     gs_mysolidline(x2,y1,x2,y2,t,c);
                                     break;
                        case DASHED: gs_mydashedline(x1,y1,x2,y1,t,c);
                                     gs_mydashedline(x2,y1,x2,y2,t,c);
                                     break;
                        case DOTTED: gs_mydottedline(x1,y1,x2,y1,t,c);
                                     gs_mydottedline(x2,y1,x2,y2,t,c);
                                     break;
			case UNVISIBLE: break;
               		}
		}
		else {	
			switch (ELSTYLE(e1)) {
                        case SOLID:  gs_mysolidline(x1,y1,x2,y2,t,c);
                                     break;
                        case DASHED: gs_mydashedline(x1,y1,x2,y2,t,c);
                                     break;
                        case DOTTED: gs_mydottedline(x1,y1,x2,y2,t,c);
                                     break;
			case UNVISIBLE: break;
			}
                }
	}

	a = NSUCC(w);
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		x2 = ESTARTX(AKANTE(a)) * G_stretch/G_shrink;
		y2 = ESTARTY(AKANTE(a)) * G_stretch/G_shrink;
		if (manhatten_edges==1) {
			conflict = 0;
			ac = NPRED(w);
			while (ac) {
				if (  (ESTARTX(AKANTE(a)) == EENDX(AKANTE(ac)))
				    &&(EANCHOR(AKANTE(a))>EANCHOR(AKANTE(ac))))
					conflict = 1;
				ac = ANEXT(ac);
			}
			if (conflict) {
				switch (ELSTYLE(e1)) {
	                        case SOLID:  gs_mysolidline(x1,y1,x2,y2,t,c);
	                                     break;
	                        case DASHED: gs_mydashedline(x1,y1,x2,y2,t,c);
	                                     break;
	                        case DOTTED: gs_mydottedline(x1,y1,x2,y2,t,c);
	                                     break;
				case UNVISIBLE: break;
       	        		}
			}
			else {
				switch (ELSTYLE(e1)) {
                        	case SOLID:  gs_mysolidline(x1,y1,x2,y1,t,c);
                                	     gs_mysolidline(x2,y1,x2,y2,t,c);
	                                     break;
	                        case DASHED: gs_mydashedline(x1,y1,x2,y1,t,c);
	                                     gs_mydashedline(x2,y1,x2,y2,t,c);
	                                     break;
	                        case DOTTED: gs_mydottedline(x1,y1,x2,y1,t,c);
	                                     gs_mydottedline(x2,y1,x2,y2,t,c);
	                                     break;
				case UNVISIBLE: break;
       	        		}
			}
		}
		else {
			switch (ELSTYLE(e1)) {
                        case SOLID:  gs_mysolidline(x1,y1,x2,y2,t,c);
                                     break;
                        case DASHED: gs_mydashedline(x1,y1,x2,y2,t,c);
                                     break;
                        case DOTTED: gs_mydottedline(x1,y1,x2,y2,t,c);
                                     break;
			case UNVISIBLE: break;
               		}
		}
		a = ANEXT(a);
	}
	a = NPRED(w);
	while (a) {
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		y1 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		x2 = EENDX(AKANTE(a)) * G_stretch/G_shrink;
		y2 = EENDY(AKANTE(a)) * G_stretch/G_shrink;
		if (manhatten_edges==1) {
			conflict = 0;
			ac = NSUCC(w);
			while (ac) {
				if (  (EENDX(AKANTE(a)) == ESTARTX(AKANTE(ac)))
				    &&(EANCHOR(AKANTE(a))<EANCHOR(AKANTE(ac))))
					conflict = 1;
				ac = ANEXT(ac);
			}
			if (conflict) {
				switch (ELSTYLE(e1)) {
	                        case SOLID:  gs_mysolidline(x1,y1,x2,y2,t,c);
	                                     break;
	                        case DASHED: gs_mydashedline(x1,y1,x2,y2,t,c);
	                                     break;
	                        case DOTTED: gs_mydottedline(x1,y1,x2,y2,t,c);
	                                     break;
				case UNVISIBLE: break;
				}
			}
			else {
				switch (ELSTYLE(e1)) {
	                        case SOLID:  gs_mysolidline(x1,y1,x2,y1,t,c);
	                                     gs_mysolidline(x2,y1,x2,y2,t,c);
	                                     break;
	                        case DASHED: gs_mydashedline(x1,y1,x2,y1,t,c);
	                                     gs_mydashedline(x2,y1,x2,y2,t,c);
	                                     break;
	                        case DOTTED: gs_mydottedline(x1,y1,x2,y1,t,c);
	                                     gs_mydottedline(x2,y1,x2,y2,t,c);
	                                     break;
				case UNVISIBLE: break;
       	        		}
			}
		}
		else {	
			switch (ELSTYLE(e1)) {
                        case SOLID:  gs_mysolidline(x1,y1,x2,y2,t,c);
                                     break;
                        case DASHED: gs_mydashedline(x1,y1,x2,y2,t,c);
                                     break;
                        case DOTTED: gs_mydottedline(x1,y1,x2,y2,t,c);
                                     break;
			case UNVISIBLE: break;
			}
                }
		a = ANEXT(a);
	}
}


/*--------------------------------------------------------------------*/
/*   Bezier-spline drawing					      */
/*--------------------------------------------------------------------*/

/*  Cubic spline drawing
 *  --------------------
 *  from (x0,y0) to (x3,y3) with aux.points (x1,y1) and (x2,y2).
 *  The thickness is t, the color c, the mode m (SOLID, etc).
 *
 *  The incoming gradient is (y1-y0)/(x1-x0), the outgoing gradient
 *  is (y3-y2)/(x3-x2).
 *  This function is not optimized for speed. This seems to be 
 *  unnecessary, because splines are in any case extremly slow.
 *  The flat_factor indicates how flat the curve should be.
 *  flat_factor = 1   means: the curve is maximal flat.
 *  flat factor = 100 means: the curve is maximal bend.
 */ 

#ifdef ANSI_C
static void gs_bezierspline(
		int x0,int y0,int x1,int y1,int x2,int y2,int x3,int y3,
		int t,int c,int m)
#else
static void gs_bezierspline(x0,y0,x1,y1,x2,y2,x3,y3,t,c,m)
int	x0, y0, x1, y1, x2, y2, x3, y3, t, c,m;
#endif
{
	int sx1,sy1,sx2,sy2;
	int ax,bx,cx;
	int ay,by,cy;
	int i,fac;
	int actx, acty;
	int oldx, oldy;
	int dashx, dashy;
	int d1,d2,d3,dash;

	gs_wait_message('d');
	if (G_flat_factor<1)   G_flat_factor=1;
	if (G_flat_factor>100) G_flat_factor=100;
	sx1 = x0+(x1-x0) * G_flat_factor/100;
	sy1 = y0+(y1-y0) * G_flat_factor/100;
	sx2 = x3+(x2-x3) * G_flat_factor/100;
	sy2 = y3+(y2-y3) * G_flat_factor/100;


#ifdef POSTSCRIPT_DEVICE
	if (  (check_visible(x0, y0, sx1,sy1))
	    &&(check_visible(sx1,sy1,sx2,sy2))
	    &&(check_visible(sx2,sy2,x3, y3)))
		ps_bezierspline(x0,y0,sx1,sy1,sx2,sy2,x3,y3,t,c,m);

#else

	cx = 3 * (sx1-x0);
	bx = 3 * (sx2-sx1) - cx;
	ax = x3 - x0 - bx -cx;
	cy = 3 * (sy1-y0);
	by = 3 * (sy2-sy1) - cy;
	ay = y3 - y0 - by -cy;

	/*  The spline function is for i in [0..1]
	 *     x(i) = ax i^3 + bx i^2 + cx i + x0
	 *     y(i) = ay i^3 + by i^2 + cy i + y0
 	 * 
	 *  We split this into fac steps, e.g. at scaling 1 into 50 steps.
	 */

	fac = 50 * G_stretch/G_shrink;
	if (fac<2) fac=2;

	oldx = x0;
	oldy = y0;
	dashx = x0;
	dashy = y0;

	switch (m) {
		case DASHED: 
		     dash = 1;
		case DOTTED: 
		     gs_mysolidline(x0,y0,x0,y0,t,c);
		     gs_mysolidline(x3,y3,x3,y3,t,c);
		     break;
	}

	for (i=1; i<=fac; i++) {
		actx = ax*i*i*i/fac/fac/fac + bx*i*i/fac/fac + cx*i/fac + x0;
		acty = ay*i*i*i/fac/fac/fac + by*i*i/fac/fac + cy*i/fac + y0;

		d1 = actx-oldx;
		d2 = acty-oldy;

		if (i==fac) { actx = x3; acty = y3; }
		else {
			if ((m==SOLID) && (d1==0) && (d2*d2<=4))
				continue;
			if ((m==SOLID) && (d2==0) && (d1*d1<=4))
				continue;
		}

		switch (m) {
                        case SOLID:  gs_mysolidline( oldx,oldy,actx,acty,t,c);
                                     break;
                        case DASHED: 
				     d1 = actx-dashx;
				     d2 = acty-dashy;
				     d3 = 15*G_stretch/G_shrink;
				     if (d1*d1+d2*d2>d3*d3) {
					    dash=1-dash; 	 
					    dashx = oldx;
					    dashy = oldy;
				     }
				     if (dash) 
					gs_mysolidline(oldx,oldy,actx,acty,t,c);
                                     break;
                        case DOTTED: 
				     d3 = 15*3*G_stretch/12/G_shrink;
				     if (d1*d1+d2*d2<d3*d3) continue; 	 
				     gs_mysolidline(actx,acty,actx,acty,t,c);
                                     break;
			case UNVISIBLE: break;
                }
		oldx = actx;
		oldy = acty;
	}

#endif
}


/*--------------------------------------------------------------------*/
/*   Arrowhead drawing						      */
/*--------------------------------------------------------------------*/

#ifdef ANSI_C
static void gs_freearrowpoint(int x1, int y1, int x2, int y2, 
				int size, int color, int mode, int t)
#else
static void gs_freearrowpoint(x1, y1, x2, y2, size, color, mode, t)
int x1, y1, x2, y2;
int size, color, mode, t;
#endif
{
	int d, ax, ay, bx, by, s1, s2, rx1, rx2, ry1, ry2;

	if ((x1<V_xmin)||(x1>=V_xmax)) return;
	if ((y1<V_ymin)||(y1>=V_ymax)) return;
	if ((x1==x2)&&(y2==y1)) return; 

	s1 = s2 = 1;
	ax = x2 - x1;
	ay = y2 - y1;
	if (ax<0) { ax = -ax; s1 = -1; }
	if (ay<0) { ay = -ay; s2 = -1; }

	if (ax>ay) {
		d = (50 * ay) / ax;
		ax = xoffset[d] * size * 17;
		ay = yoffset[d] * size * 17;
		by = xoffset[d] * size * 10;
		bx = yoffset[d] * size * 10;

	}
	else {  d = (50 * ax) / ay;
		ay = xoffset[d] * size * 17;
		ax = yoffset[d] * size * 17;
		bx = xoffset[d] * size * 10;
		by = yoffset[d] * size * 10;

	}
	rx1 = x1 + s1 * (ax - bx)/ 300;
	ry1 = y1 + s2 * (ay + by)/ 300;
	rx2 = x1 + s1 * (ax + bx)/ 300;
	ry2 = y1 + s2 * (ay - by)/ 300;

	if (mode==ASLINE) {
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(rx1,ry1,x1,y1,t,color);
		gs_mysolidline(rx2,ry2,x1,y1,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}

#ifdef POSTSCRIPT_DEVICE
        ps_freearrowpoint(x1,y1,rx1,ry2,rx2,ry2,color);
#else
#ifdef FAST_X11_DRAWING 
	X11_fast_triangle(x1,y1,rx1,ry1,rx2,ry2,color);
#else
	if (x1<rx1) for (ax=x1; ax<=rx1; ax++) {
		ay = (ax-x1)* (ry1-y1)/(rx1-x1) + y1; 
		gs_line(ax,ay,rx2,ry2,color);
	}
	else if (x1>rx1) for (ax=x1; ax>=rx1; ax--) {
		ay = (ax-x1)* (ry1-y1)/(rx1-x1) + y1; 
		gs_line(ax,ay,rx2,ry2,color);
	}
	if (x1<rx2) for (ax=x1; ax<=rx2; ax++) {
		ay = (ax-x1)* (ry2-y1)/(rx2-x1) + y1; 
		gs_line(ax,ay,rx1,ry1,color);
	}
	else if (x1>rx2) for (ax=x1; ax>=rx2; ax--) {
		ay = (ax-x1)* (ry2-y1)/(rx2-x1) + y1; 
		gs_line(ax,ay,rx1,ry1,color);
	}
	if (rx1<rx2) for (ax=rx1; ax<=rx2; ax++) {
		ay = (ax-rx1)* (ry2-ry1)/(rx2-rx1) + ry1; 
		gs_line(ax,ay,x1,y1,color);
	}
	else if (rx1>rx2) for (ax=rx1; ax>=rx2; ax--) {
		ay = (ax-rx1)* (ry2-ry1)/(rx2-rx1) + ry1; 
		gs_line(ax,ay,x1,y1,color);
	}
	gs_line(x1,y1,rx1,ry1,color);
	gs_line(x1,y1,rx2,ry2,color);
	gs_line(rx1,ry1,rx2,ry2,color);
#endif
#endif
}


/* Arrowhead to the north
 * ----------------------
 * with `size' of katheses and color
 */

 
#ifdef ANSI_C
static void	gs_n_arrowpoint(int x,int y,int size,int color,int mode,int t)
#else
static void	gs_n_arrowpoint(x, y, size, color, mode, t)
int	x, y, size, color, mode, t;
#endif
{
	int	i, ax, ay, bx, by;

	if ((x<V_xmin)||(x>=V_xmax)) return;
	if ((y<V_ymin)||(y>=V_ymax)) return;

	if (mode==ASLINE) {
		i = 7*size/10 - 1;
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(x-i,y+i,x,y,t,color);
		gs_mysolidline(x+i,y+i,x,y,t,color);
		gs_mysolidline(x,y+i,x,y,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}
#ifdef POSTSCRIPT_DEVICE
        ps_n_arrowpoint(x, y, size, color);
#else
#ifdef FAST_X11_DRAWING
	i = 7*size/10 - 1;
	X11_fast_triangle(x-i,y+i,x,y,x+i,y+i,color);
#else
	for (i=0; i<7*size/10; i++)  gs_line(x-i,y+i,x+i,y+i,color);
#endif
#endif
}


/* Arrowhead to the northeast
 * --------------------------
 * with `size' of katheses and color
 */


#ifdef ANSI_C
static void	gs_ne_arrowpoint(int x,int y,int size,int color,int mode,int t)
#else
static void	gs_ne_arrowpoint(x, y, size, color, mode, t)
int	x, y, size, color, mode, t;
#endif
{
	int	i, ax, ay, bx, by;

	if ((x<V_xmin)||(x>=V_xmax)) return;
	if ((y<V_ymin)||(y>=V_ymax)) return;
	if (mode==ASLINE) {
		i = size - 1;
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(x-i,y,x,y,t,color);
		gs_mysolidline(x,y+i,x,y,t,color);
		gs_mysolidline(x-size/2,y+size/2,x,y,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}
#ifdef POSTSCRIPT_DEVICE
        ps_ne_arrowpoint(x, y, size, color);
#else
#ifdef FAST_X11_DRAWING
	X11_fast_triangle(x-size+1,y,x,y,x,y+size-1,color);
#else
	for (i=0; i<size; i++)	gs_line(x-size+1+i,y+i,x,y+i,color);
#endif
#endif
}


/* Arrowhead to the northwest
 * --------------------------
 * with `size' of katheses and color
 */


#ifdef ANSI_C
static void	gs_nw_arrowpoint(int x,int y,int size,int color,int mode,int t)
#else
static void	gs_nw_arrowpoint(x, y, size, color, mode, t)
int	x, y, size, color, mode, t;
#endif
{
	int	i, ax, ay, bx, by;

	if ((x<V_xmin)||(x>=V_xmax)) return;
	if ((y<V_ymin)||(y>=V_ymax)) return;
	if (mode==ASLINE) {
		i = size - 1;
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(x+i,y,x,y,t,color);
		gs_mysolidline(x,y+i,x,y,t,color);
		gs_mysolidline(x+size/2,y+size/2,x,y,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}
#ifdef POSTSCRIPT_DEVICE
        ps_nw_arrowpoint(x, y, size, color);
#else
#ifdef FAST_X11_DRAWING
	X11_fast_triangle(x+size-1,y,x,y,x,y+size-1,color);
#else
	for (i=0; i<size; i++)	gs_line(x,y+i,x+size-1-i,y+i,color);	
#endif
#endif
}


/* Arrowhead to the south 
 * ----------------------
 * with `size' of katheses and color
 */


#ifdef ANSI_C
static void	gs_s_arrowpoint(int x,int y,int size,int color,int mode,int t)
#else
static void	gs_s_arrowpoint(x, y, size, color, mode,t)
int	x, y, size, color, mode, t;
#endif
{
	int	i, ax, ay, bx, by;

	if ((x<V_xmin)||(x>=V_xmax)) return;
	if ((y<V_ymin)||(y>=V_ymax)) return;
	if (mode==ASLINE) {
		i = 7*size/10 - 1;
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(x-i,y-i,x,y,t,color);
		gs_mysolidline(x+i,y-i,x,y,t,color);
		gs_mysolidline(x,y-i,x,y,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}
#ifdef POSTSCRIPT_DEVICE
        ps_s_arrowpoint(x, y, size, color);
#else
#ifdef FAST_X11_DRAWING
	i = 7*size/10 - 1;
	X11_fast_triangle(x-i,y-i,x,y,x+i,y-i,color);
#else
	for (i=0; i<7*size/10; i++)  gs_line(x-i,y-i,x+i,y-i,color);
#endif
#endif
}


/* Arrowhead to the southeast 
 * --------------------------
 * with `size' of katheses and color
 */


#ifdef ANSI_C
static void	gs_se_arrowpoint(int x,int y,int size,int color,int mode,int t)
#else
static void	gs_se_arrowpoint(x, y, size, color, mode, t)
int	x, y, size, color, mode, t;
#endif
{
	int	i, ax, ay, bx, by;

	if ((x<V_xmin)||(x>=V_xmax)) return;
	if ((y<V_ymin)||(y>=V_ymax)) return;
	if (mode==ASLINE) {
		i = size - 1;
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(x-i,y,x,y,t,color);
		gs_mysolidline(x,y-i,x,y,t,color);
		gs_mysolidline(x-size/2,y-size/2,x,y,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}
#ifdef POSTSCRIPT_DEVICE
        ps_se_arrowpoint(x, y, size, color);
#else
#ifdef FAST_X11_DRAWING
	X11_fast_triangle(x-size+1,y,x,y,x,y-size+1,color);
#else
	for (i=0; i<size; i++)	gs_line(x-size+1+i,y-i,x,y-i,color);
#endif
#endif
}


/* Arrowhead to the southwest 
 * --------------------------
 * with `size' of katheses and color
 */


#ifdef ANSI_C
static void	gs_sw_arrowpoint(int x,int y,int size,int color,int mode,int t)
#else
static void	gs_sw_arrowpoint(x, y, size, color, mode, t)
int	x, y, size, color, mode, t;
#endif
{
	int	i, ax, ay, bx, by;

	if ((x<V_xmin)||(x>=V_xmax)) return;
	if ((y<V_ymin)||(y>=V_ymax)) return;
	if (mode==ASLINE) {
		i = size - 1;
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(x+i,y,x,y,t,color);
		gs_mysolidline(x,y-i,x,y,t,color);
		gs_mysolidline(x+size/2,y-size/2,x,y,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}
#ifdef POSTSCRIPT_DEVICE
        ps_sw_arrowpoint(x, y, size, color);
#else
#ifdef FAST_X11_DRAWING
	X11_fast_triangle(x+size-1,y,x,y,x,y-size+1,color);
#else
	for (i=0; i<size; i++)	gs_line(x,y-i,x+size-1-i,y-i,color);
#endif
#endif
}


/* Arrowhead to the east 
 * ---------------------
 * with `size' of katheses and color
 */


#ifdef ANSI_C
static void	gs_e_arrowpoint(int x,int y,int size,int color,int mode,int t)
#else
static void	gs_e_arrowpoint(x, y, size, color, mode,t)
int	x, y, size, color, mode, t;
#endif
{
	int	i, ax, ay, bx, by;

	if ((x<V_xmin)||(x>=V_xmax)) return;
	if ((y<V_ymin)||(y>=V_ymax)) return;
	if (mode==ASLINE) {
		i = 7*size/10 - 1;
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(x-i,y-i,x,y,t,color);
		gs_mysolidline(x-i,y+i,x,y,t,color);
		gs_mysolidline(x-i,y,x,y,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}
#ifdef POSTSCRIPT_DEVICE
        ps_e_arrowpoint(x, y, size, color);
#else
#ifdef FAST_X11_DRAWING
	i = 7*size/10 - 1;
	X11_fast_triangle(x-i,y-i,x,y,x-i,y+i,color);
#else
	for (i=0; i<7*size/10; i++)  gs_line(x-i,y-i,x-i,y+i,color);
#endif
#endif
}


/* Arrowhead to the west 
 * ---------------------
 * with `size' of katheses and color
 */


#ifdef ANSI_C
static void	gs_w_arrowpoint(int x,int y,int size,int color,int mode,int t)
#else
static void	gs_w_arrowpoint(x, y, size, color, mode, t)
int	x, y, size, color, mode, t;
#endif
{
	int	i, ax, ay, bx, by;

	if ((x<V_xmin)||(x>=V_xmax)) return;
	if ((y<V_ymin)||(y>=V_ymax)) return;
	if (mode==ASLINE) {
		i = 7*size/10 - 1;
		ax = gbl_x1; ay = gbl_y1; bx = gbl_x2; by = gbl_y2;
		gs_mysolidline(x+i,y-i,x,y,t,color);
		gs_mysolidline(x+i,y+i,x,y,t,color);
		gs_mysolidline(x+i,y,x,y,t,color);
		gbl_x1 = ax; gbl_y1 = ay; gbl_x2 = bx; gbl_y2 = by;
		return;
	}
#ifdef POSTSCRIPT_DEVICE
        ps_w_arrowpoint(x, y, size, color);
#else
#ifdef FAST_X11_DRAWING
	i = 7*size/10 - 1;
	X11_fast_triangle(x+i,y-i,x,y,x+i,y+i,color);
#else
	for (i=0; i<7*size/10; i++)  gs_line(x+i,y-i,x+i,y+i,color);
#endif
#endif
}


/*--------------------------------------------------------------------*/
/*   Check visibility of lines					      */
/*--------------------------------------------------------------------*/

/*  Abbreviations for intersection points.
 *  See comments in check_visible.
 */

/*  Calculate intersection with line f(x) = V_ymin.
 *  If intersection point is in quadrant 5, set gbl_x1, gbl_y1 to 
 *  replace a,b.
 */
 
#define cut_ab_yminline(a,b,c,d)  {				   \
	 intersectionp = ((V_ymin-d)*(a-c)+c*(b-d)) / (b-d);	   \
	 if ((V_xmin<=intersectionp) && (intersectionp<V_xmax)) {  \
		    gbl_x1 = intersectionp;			   \
		    gbl_y1 = V_ymin;				   \
		    res = 1;					   \
	 }							   \
	 else res = 0;						   \
}

/*  Calculate intersection with line f(x) = V_ymin.
 *  If intersection point is in quadrant 5, set gbl_x2, gbl_y2 to 
 *  replace c,d.
 */
 
#define cut_cd_yminline(a,b,c,d)  {				   \
	 intersectionp = ((V_ymin-d)*(a-c)+c*(b-d)) / (b-d);	   \
	 if ((V_xmin<=intersectionp) && (intersectionp<V_xmax)) {  \
		    gbl_x2 = intersectionp;			   \
		    gbl_y2 = V_ymin;				   \
		    res = 1;					   \
	 }							   \
	 else res = 0;						   \
}


/*  Calculate intersection with line f(x) = V_ymax-1.
 *  If intersection point is in quadrant 5, set gbl_x1, gbl_y1 to 
 *  replace a,b.
 */
 
#define cut_ab_ymaxline(a,b,c,d)  {				   \
	 intersectionp = ((V_ymax-d-1)*(a-c)+c*(b-d)) / (b-d);	   \
	 if ((V_xmin<=intersectionp) && (intersectionp<V_xmax))  { \
		    gbl_x1 = intersectionp;			   \
		    gbl_y1 = V_ymax;				   \
		    res = 1;					   \
	 }							   \
	 else res = 0;						   \
}

/*  Calculate intersection with line f(x) = V_ymax-1.
 *  If intersection point is in quadrant 5, set gbl_x2, gbl_y2 to 
 *  replace c,d.
 */
 
#define cut_cd_ymaxline(a,b,c,d)  {				   \
	 intersectionp = ((V_ymax-d-1)*(a-c)+c*(b-d)) / (b-d);	   \
	 if ((V_xmin<=intersectionp) && (intersectionp<V_xmax))  { \
		    gbl_x2 = intersectionp;			   \
		    gbl_y2 = V_ymax;				   \
		    res = 1;					   \
	 }							   \
	 else res = 0;						   \
}

/*  Calculate intersection with line f(y) = V_xmin.
 *  If intersection point is in quadrant 5, set gbl_x1, gbl_y1 to 
 *  replace a,b.
 */
 
#define cut_ab_xminline(a,b,c,d)  {				   \
	 intersectionp = ((V_xmin-c)*(b-d)+d*(a-c)) / (a-c);	   \
	 if ((V_ymin<=intersectionp) && (intersectionp<V_ymax)) {  \
		    gbl_x1 = V_xmin;				   \
		    gbl_y1 = intersectionp;			   \
		    res = 1;					   \
	 }							   \
	 else res = 0;						   \
}

/*  Calculate intersection with line f(y) = V_xmin.
 *  If intersection point is in quadrant 5, set gbl_x2, gbl_y2 to 
 *  replace c,d.
 */
 
#define cut_cd_xminline(a,b,c,d)  {				   \
	 intersectionp = ((V_xmin-c)*(b-d)+d*(a-c)) / (a-c);	   \
	 if ((V_ymin<=intersectionp) && (intersectionp<V_ymax)) {  \
		    gbl_x2 = V_xmin;				   \
		    gbl_y2 = intersectionp;			   \
		    res = 1;					   \
	 }							   \
	 else res = 0;						   \
}


/*  Calculate intersection with line f(y) = V_xmax-1.
 *  If intersection point is in quadrant 5, set gbl_x1, gbl_y1 to 
 *  replace a,b.
 */
 
#define cut_ab_xmaxline(a,b,c,d)  {				   \
	 intersectionp = ((V_xmax-c-1)*(b-d)+d*(a-c)) / (a-c);	   \
	 if ((V_ymin<=intersectionp) && (intersectionp<V_ymax)) {  \
		    gbl_x1 = V_xmax;				   \
		    gbl_y1 = intersectionp;			   \
		    res = 1;					   \
	 }							   \
	 else res = 0;						   \
}

/*  Calculate intersection with line f(y) = V_xmax-1.
 *  If intersection point is in quadrant 5, set gbl_x2, gbl_y2 to 
 *  replace c,d.
 */
 
#define cut_cd_xmaxline(a,b,c,d)  {				   \
	 intersectionp = ((V_xmax-c-1)*(b-d)+d*(a-c)) / (a-c);	   \
	 if ((V_ymin<=intersectionp) && (intersectionp<V_ymax)) {  \
		    gbl_x2 = V_xmax;				   \
		    gbl_y2 = intersectionp;			   \
		    res = 1;					   \
	 }							   \
	 else res = 0;						   \
}


/*  Check whether a line from (a,b) to (c,d) is visible
 *  ---------------------------------------------------
 *  Returns 1 if the line is visible, otherwise 0.
 *  The global value gbl_x1, gbl_y1, gbl_x2, gbl_y2 contain replacement
 *  values if only a part of the line is visible.
 *  The visible part is defined by the rectangular between 
 *  (V_xmin,V_ymin) and (V_xmax-1,V_ymax-1).
 */


#ifdef ANSI_C
static int     check_visible(int a,int b,int c,int d)
#else
static int     check_visible(a,b,c,d)
int	a, b, c, d;	
#endif
{
	int	qab, qcd;
	int	quadrantrelation;
	int	res;
	int	intersectionp;

   /*  1. Step: we calculate the number of the quadrant where the points
    *		(a,b) and (c,d) are placed in.
    *		The visible part partitioned the co-ordinate system
    *		into 9 quadrants: 
    *
    *
    *		  1	    |	      2 	 |	 3
    *			    |			 |
    *	      -------(V_xmin,V_ymin) -------(V_xmax,V_ymin) -----------
    *			    |			 |
    *		  4	    |	      5 	 |	 6
    *			    |			 |
    *	      -------(V_xmin,V_ymax) -------(V_xmax,V_ymax) -----------
    *			    |			 |
    *		  7	    |	      8 	 |	 9
    *
    *  Note: only points in quadrant 5 are visible.
    */

	gbl_x1 = a;
	gbl_y1 = b;
	gbl_x2 = c;
	gbl_y2 = d;

	if (fisheye_view!=0) return(1);
	qab = 1;
	if (b>=V_ymin) qab = 4;
	if (b>=V_ymax) qab += 3;
	if (a>=V_xmin)	qab++;
	if (a>=V_xmax)	qab++;
	qcd = 1;
	if (d>=V_ymin) qcd = 4;
	if (d>=V_ymax) qcd += 3;
	if (c>=V_xmin)	qcd++; 
	if (c>=V_xmax)	qcd++;
	if ((qcd==5)&&(qab==5)) return(1); /* both points are visible */
 
   /* Now: at least one endpoint is not visible.
    * We calculate in which relation the endpoints (a,b) and (c,d) are:
    */

	quadrantrelation = qab * 10 + qcd;  /* for simplification */

   /* The idea is to find a point of of the line inside quadrant 5.
    * Thus we use the straight line equation of the line (a,b)-(c,d)
    * and calculate the intersection points of this straight line
    * with the bordering straight lines V_xmin, V_ymin, V_xmax, V_ymax.
    *
    * The straight line equation through (a,b)-(c,d) is:
    *
    *	    y = f(x) = (b-d)/(a-c) * (x-c) + d;
    *
    * or    x = f(y) = (a-c)/(b-d) * (y-d) + c;   respectively
    *
    * If we find a intersection point, we check wether this intersection
    * point is in quadrant 5. If yes, then at least this intersection
    * point is visible. 
    */

	switch (quadrantrelation) { 
		case 25:
			cut_ab_yminline(a,b,c,d);
			return(res);
		case 52:
			cut_cd_yminline(a,b,c,d);
			return(res);
		case 85:
			cut_ab_ymaxline(a,b,c,d);
			return(res);
		case 58:
			cut_cd_ymaxline(a,b,c,d);
			return(res);
		case 45:
			cut_ab_xminline(a,b,c,d);
			return(res);
		case 54:
			cut_cd_xminline(a,b,c,d);
			return(res);
		case 65:
			cut_ab_xmaxline(a,b,c,d);
			return(res);
		case 56:
			cut_cd_xmaxline(a,b,c,d);
			return(res);
		case 15:
			cut_ab_xminline(a,b,c,d);
			if (!res) cut_ab_yminline(a,b,c,d);
			return(res);
		case 51:
			cut_cd_xminline(a,b,c,d);
			if (!res) cut_cd_yminline(a,b,c,d);
			return(res);
		case 35:
			cut_ab_xmaxline(a,b,c,d);
			if (!res) cut_ab_yminline(a,b,c,d);
			return(res);
		case 53:	
			cut_cd_xmaxline(a,b,c,d);
			if (!res) cut_cd_yminline(a,b,c,d);
			return(res);
		case 75:
			cut_ab_xminline(a,b,c,d);
			if (!res) cut_ab_ymaxline(a,b,c,d);
			return(res);
		case 57:
			cut_cd_xminline(a,b,c,d);
			if (!res) cut_cd_ymaxline(a,b,c,d);
			return(res);
		case 95:
			cut_ab_xmaxline(a,b,c,d);
			if (!res) cut_ab_ymaxline(a,b,c,d);
			return(res);
		case 59:
			cut_cd_xmaxline(a,b,c,d);
			if (!res) cut_cd_ymaxline(a,b,c,d);
			return(res);
		case 48:
			cut_ab_xminline(a,b,c,d);
			if (res) cut_cd_ymaxline(a,b,c,d);
			return(res);
		case 84:
			cut_cd_xminline(a,b,c,d);
			if (res) cut_ab_ymaxline(a,b,c,d);
			return(res);
		case 68:
			cut_ab_xmaxline(a,b,c,d);
			if (res) cut_cd_ymaxline(a,b,c,d);
			return(res);;
		case 86:
			cut_cd_xmaxline(a,b,c,d);
			if (res) cut_ab_ymaxline(a,b,c,d);
			return(res);
		case 24:
			cut_cd_xminline(a,b,c,d);
			if (res) cut_ab_yminline(a,b,c,d);
			return(res);
		case 42:
			cut_ab_xminline(a,b,c,d);
			if (res) cut_cd_yminline(a,b,c,d);
			return(res);
		case 26:
			cut_cd_xmaxline(a,b,c,d);
			if (res) cut_ab_yminline(a,b,c,d);
			return(res);
		case 62:
			cut_ab_xmaxline(a,b,c,d);
			if (res) cut_cd_yminline(a,b,c,d);
			return(res);
			
		case 18:
			cut_cd_ymaxline(a,b,c,d);
			if (res) {
				cut_ab_xminline(a,b,c,d);
				if (!res) cut_ab_yminline(a,b,c,d);
			}
			return(res);
		case 81:
			cut_ab_ymaxline(a,b,c,d);
			if (res) {
				cut_cd_xminline(a,b,c,d);
				if (!res) cut_cd_yminline(a,b,c,d);
			}
			return(res);
		case 38:
			cut_cd_ymaxline(a,b,c,d);
			if (res) {
				cut_ab_xmaxline(a,b,c,d);
				if (!res) cut_ab_yminline(a,b,c,d);
			}
			return(res);
		case 83:
			cut_ab_ymaxline(a,b,c,d);
			if (res) {
				cut_cd_xmaxline(a,b,c,d);
				if (!res) cut_cd_yminline(a,b,c,d);
			}
			return(res);
		case 27:
			cut_ab_yminline(a,b,c,d);
			if (res) {
				cut_cd_xminline(a,b,c,d);
				if (!res) cut_cd_ymaxline(a,b,c,d);
			}
			return(res);
		case 72:
			cut_cd_yminline(a,b,c,d);
			if (res) {
				cut_ab_xminline(a,b,c,d);
				if (!res) cut_ab_ymaxline(a,b,c,d);
			}
			return(res);
		case 29:
			cut_ab_yminline(a,b,c,d);
			if (res) {
				cut_cd_xmaxline(a,b,c,d);
				if (!res) cut_cd_ymaxline(a,b,c,d);
			}
			return(res);
		case 92:
			cut_cd_yminline(a,b,c,d);
			if (res) {
				cut_ab_xmaxline(a,b,c,d);
				if (!res) cut_ab_ymaxline(a,b,c,d);
			}
			return(res);

		case 43:
			cut_ab_xminline(a,b,c,d);
			if (res) {
				cut_cd_xmaxline(a,b,c,d);
				if (!res) cut_cd_yminline(a,b,c,d);
			}
			return(res);
		case 34:
			cut_cd_xminline(a,b,c,d);
			if (res) {
				cut_ab_xmaxline(a,b,c,d);
				if (!res) cut_ab_yminline(a,b,c,d);
			}
			return(res);
		case 49:
			cut_ab_xminline(a,b,c,d);
			if (res) {
				cut_cd_xmaxline(a,b,c,d);
				if (!res) cut_cd_ymaxline(a,b,c,d);
			}
			return(res);
		case 94:
			cut_cd_xminline(a,b,c,d);
			if (res) {
				cut_ab_xmaxline(a,b,c,d);
				if (!res) cut_ab_ymaxline(a,b,c,d);
			}
			return(res);
		case 61:
			cut_ab_xmaxline(a,b,c,d);
			if (res) {
				cut_cd_xminline(a,b,c,d);
				if (!res) cut_cd_yminline(a,b,c,d);
			}
			return(res);
		case 16:
			cut_cd_xmaxline(a,b,c,d);
			if (res) {
				cut_ab_xminline(a,b,c,d);
				if (!res) cut_ab_yminline(a,b,c,d);
			}
			return(res);
		case 67:
			cut_ab_xmaxline(a,b,c,d);
			if (res) {
				cut_cd_xminline(a,b,c,d);
				if (!res) cut_cd_ymaxline(a,b,c,d);
			}
			return(res);
		case 76:
			cut_cd_xmaxline(a,b,c,d);
			if (res) {
				cut_ab_xminline(a,b,c,d);
				if (!res) cut_ab_ymaxline(a,b,c,d);
			}
			return(res);

		case 19:
			cut_ab_xminline(a,b,c,d);
			if (res) {
				cut_cd_xmaxline(a,b,c,d);
				if (!res) cut_cd_ymaxline(a,b,c,d);
			}
			else {	cut_ab_yminline(a,b,c,d);
				if (res) { 
					cut_cd_xmaxline(a,b,c,d);
					if (!res) cut_cd_ymaxline(a,b,c,d);
				}
			}
			return(res);
		case 91:
			cut_cd_xminline(a,b,c,d);
			if (res) {
				cut_ab_xmaxline(a,b,c,d);
				if (!res) cut_ab_ymaxline(a,b,c,d);
			}
			else {	cut_cd_yminline(a,b,c,d);
				if (res) {
					cut_ab_xmaxline(a,b,c,d);
					if (!res) cut_ab_ymaxline(a,b,c,d);
				}
			}
			return(res);
		case 37:
			cut_ab_xmaxline(a,b,c,d);
			if (res) {
				cut_cd_xminline(a,b,c,d);
				if (!res) cut_cd_ymaxline(a,b,c,d);
			}
			else {	cut_ab_yminline(a,b,c,d);
				if (res) {
					cut_cd_xminline(a,b,c,d);
					if (!res) cut_cd_ymaxline(a,b,c,d);
				}
			}
			return(res);
		case 73:
			cut_cd_xmaxline(a,b,c,d);
			if (res) {
				cut_ab_xminline(a,b,c,d);
				if (!res) cut_ab_ymaxline(a,b,c,d);
			}
			else {	cut_cd_yminline(a,b,c,d);
				if (res) {
					cut_ab_xminline(a,b,c,d);
					if (!res) cut_ab_ymaxline(a,b,c,d);
				}
			}
			return(res);
		case 28:
			cut_ab_yminline(a,b,c,d);
			cut_cd_ymaxline(a,b,c,d);
			return(res);
		case 82:
			cut_cd_yminline(a,b,c,d);
			cut_ab_ymaxline(a,b,c,d);
			return(res);
		case 64:
			cut_cd_xminline(a,b,c,d);
			cut_ab_xmaxline(a,b,c,d);
			return(res);
		case 46:
			cut_ab_xminline(a,b,c,d);
			cut_cd_xmaxline(a,b,c,d);
			return(res);
		default:
			return(0);
	}
	/* NOTREACHED */
	return(0);
}	


/*--------------------------------------------------------------------*/
/*   Calculation and drawing of splines  */
/*--------------------------------------------------------------------*/



/*  Draw a spline line 
 *  ------------------
 *  from a start node through all dummy nodes to the end node, where
 *  start node and end node are no dummy nodes.
 *  If we call this with an edge whose start node is a dummy node,
 *  nothing happens.
 */


/* We store in the testnode the nodes that are actually on the path,
 * in order to avoid that they influence the splines.
 */

static GNODE my_testnode1, my_testnode2, my_testnode3;
static int ax[7];
static int ay[7];
static int nr_points;

#ifdef ANSI_C
static void draw_spline(GEDGE e, int first)
#else
static void draw_spline(e,first)
GEDGE e;
int first;
#endif
{
	GNODE sn,en;
        int     x1, x2, y1, y2, x3, y3, x4, y4, c;
        int     t, m,topbend,botbend;
	int 	i, okay;

	debugmessage("draw_spline","");

	/* Check whether the predecessor is a dummy node.
	 * If yes, we do not draw this part of the edge here,
	 * because we draw at when we draw the original edge.
	 */

	sn = ESTART(e);
	en = EEND(e);
	my_testnode2 = sn;

if (first) {
	if ((NWIDTH(sn)==0) && (NHEIGHT(sn)==0)) {
		if ((NWIDTH(en)==0) && (NHEIGHT(en)==0)) return; 
		if (EART(e)!='R') return;
	}
	else if ((NWIDTH(en)==0) && (NHEIGHT(en)==0)) {
		if (EART(e)=='R') return;
	} 
}
	if (EART(e)=='R') { 
		en = ESTART(e);
		sn = EEND(e);
		my_testnode2 = sn;
	}

	/* Okay: now this is an edge that starts at sn which is no
	 * dummy node. We follow this edge now along all dummy nodes
	 * and draw the appropriate splines.
	 */

	if (EANCHOR(e)==66) { draw_splineanchors(e); return; }

	if (first && (NANCHORNODE(sn))) return;
	if (first && (NANCHORNODE(en))) return;

	if (first) nr_points = 0;

	while (e) {

		my_testnode1 = my_testnode2;
		my_testnode2 = sn;
		my_testnode3 = en;
		gbl_x1 = x1 = ESTARTX(e) * G_stretch/G_shrink;
        	gbl_y1 = y1 = ESTARTY(e) * G_stretch/G_shrink;
      		gbl_x2 = x2 = EENDX(e) * G_stretch/G_shrink;
        	gbl_y2 = y2 = EENDY(e) * G_stretch/G_shrink;
        	x3 = ETBENDX(e) * G_stretch/G_shrink;
        	y3 = ETBENDY(e) * G_stretch/G_shrink;
        	x4 = EBBENDX(e) * G_stretch/G_shrink;
        	y4 = EBBENDY(e) * G_stretch/G_shrink;
        	t  = (ETHICKNESS(e)*G_stretch)/G_shrink;
		m = ELSTYLE(e);
        	if ( t==0 )     t = 1;
        	c  = ECOLOR(e);
        	if (!colored) c = BLACK;

		draw_arrowhead(e, 0);

		if (en==ESTART(e)) {

			/* it is a bottom up edge in self loops
			 * or cross edges.
			 * We exchange the coordinates, because the
			 * coordinates are still as in top-down direction.
			 */
			i = gbl_x1; gbl_x1 = gbl_x2; gbl_x2 = i;
			i = gbl_y1; gbl_y1 = gbl_y2; gbl_y2 = i;
			i = x1; x1 = x2; x2 = i;
			i = y1; y1 = y2; y2 = i;
			i = x3; x3 = x4; x4 = i;
			i = y3; y3 = y4; y4 = i;
		}

        	topbend = 0;
        	if ((y3!=y1)||(x3!=x1)) {
               		topbend = 1;
			if (((y3-y1)*(y3-y1)<10) && 
			    ((x3-x1)*(x3-x1)<10))   topbend = 0;
                	if ((x1<x3) && (gbl_x1>x3)) topbend = 0;
                	if ((x1>x3) && (gbl_x1<x3)) topbend = 0;
                	if ((y1<y3) && (gbl_y1>y3)) topbend = 0;
                	if ((y1>y3) && (gbl_y1<y3)) topbend = 0;
        	}
        	botbend = 0;
        	if ((y4!=y2)||(x4!=x2)) {
                	botbend = 1;
			if (((y2-y4)*(y2-y4)<10) && 
			    ((x2-x4)*(x2-x4)<10))   botbend = 0;
                	if ((x2<x4) && (gbl_x2>x4)) botbend = 0;
                	if ((x2>x4) && (gbl_x2<x4)) botbend = 0;
                	if ((y2<y4) && (gbl_y2>y4)) botbend = 0;
                	if ((y2>y4) && (gbl_y2<y4)) botbend = 0;
        	}

        	if (first) { ax[nr_points]= gbl_x1; ay[nr_points++]= gbl_y1; }
		if (topbend) { 
        		ax[nr_points] = x3; ay[nr_points] = y3; 
			if (nr_points>1) {
				i = nr_points-2;
				okay = 1;
				if ((ax[i]==ax[i+1]) && (ax[i+1]==ax[i+2]))
					okay = 0;
				if ((ay[i]==ay[i+1]) && (ay[i+1]==ay[i+2]))
					okay = 0;
				if (okay) nr_points++;
				else {
        				ax[nr_points-1] = x3; 
					ay[nr_points-1] = y3; 
				}
			}
			else nr_points++;
		}
		if (botbend) { 
        		ax[nr_points] = x4; ay[nr_points++] = y4; 
		}
        	ax[nr_points] = gbl_x2; ay[nr_points++] = gbl_y2; 


		/* Draw the splines until we have only 3 remaining points.
		 * The reason is: for a spline, we need at least 3 points.
		 * The last spline cannot be drawn, because we cannot be
		 * sure about the lenght of its last part. 
		 */

		if (first) draw_start_part(ax[0],ay[0],ax[1],ay[1], c,t,m);
		first = 0;

		while (nr_points>3) {
			draw_spline_part(ax[0],ay[0],ax[1],ay[1],ax[2],ay[2],
							c,t,m);
			for (i=0; i<nr_points-1; i++) {
				ax[i] = ax[i+1];
				ay[i] = ay[i+1];
			}
			nr_points--;
		}

		/* Check whether end of e is a dummy node */

		if ((NWIDTH(en)==0) && (NHEIGHT(en)==0)) {
			e =  dummy_continue_edge(en,e);
		}
		else e = NULL;
		if (e) {
			sn = en;
			en = EEND(e);
			if (en==sn) en=ESTART(e);
		}
	}


	/* at last: draw the final lines: this should be one spline
	 * and one line.
	 */
	while (nr_points>2) {
		draw_spline_part(ax[0],ay[0],ax[1],ay[1],ax[2],ay[2],
						c,t,m);
		for (i=0; i<nr_points-1; i++) {
			ax[i] = ax[i+1];
			ay[i] = ay[i+1];
		}
		nr_points--;
	}
	if (nr_points==2)
		draw_final_part(ax[0],ay[0],ax[1],ay[1],c,t,m);

}


/* Draw the spline anchor lines instead an anchor edge
 * ---------------------------------------------------
 */

#ifdef ANSI_C
static void draw_splineanchors(GEDGE e)
#else
static void draw_splineanchors(e)
GEDGE e;
#endif
{
	GEDGE e1;
	ADJEDGE a;
	GNODE v;
	int   h, w, x1, x2, yb, y1, y2, xx, d;
	int   t, c, m;

	debugmessage("draw_splineanchors","");

	x1 = ESTARTX(e) * G_stretch/G_shrink;
	x2 = EENDX(e) * G_stretch/G_shrink;
	v = ESTART(e);
        gs_setshrink(G_stretch * NSTRETCH(v),
                     G_shrink  * NSHRINK(v)  );
        gs_setto(NX(v) * G_stretch / G_shrink,
                 NY(v) * G_stretch / G_shrink );
	h = NHEIGHT(v)*G_stretch/G_shrink;
	w = NWIDTH(v) *G_stretch/G_shrink;
	gs_calcstringsize(NLABEL(v));
	yb = myypos + (h-gs_stringh)/2 - (10*mystretch)/myshrink;

	nr_points = 0;
	if (CTARGET2(NCONNECT(EEND(e)))) {
		e1 = CEDGE2(NCONNECT(EEND(e)));
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		m = ELSTYLE(e);
		y1 = y2 = yb + (-EANCHOR(e1)*16*mystretch)/myshrink;
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x1 = xx;
		gbl_y1 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x1;
		y1 = gbl_y1;
        	ax[nr_points]= xx; ay[nr_points++]= y1; 
        	ax[nr_points]= x2; ay[nr_points++]= y2; 
        	ax[nr_points]= ESTARTX(e1) * G_stretch/G_shrink; 
		ay[nr_points++]= ESTARTY(e1) * G_stretch/G_shrink; 
		draw_start_part(ax[0],ay[0],ax[1],ay[1], c,t,m);
		draw_spline(e1,0);
	}

	a = NSUCC(EEND(e));
	while (a) {
		nr_points = 0;
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		m = ELSTYLE(e1);
		y1 = y2 = yb +(-EANCHOR(e1)*16*mystretch)/myshrink;  
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x1 = xx;
		gbl_y1 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x1;
		y1 = gbl_y1;
        	ax[nr_points]= xx; ay[nr_points++]= y1; 
        	ax[nr_points]= x2; ay[nr_points++]= y2; 
        	ax[nr_points]= ESTARTX(e1) * G_stretch/G_shrink; 
		ay[nr_points++]= ESTARTY(e1) * G_stretch/G_shrink; 
		draw_start_part(ax[0],ay[0],ax[1],ay[1], c,t,m);
		draw_spline(e1,0);
		a = ANEXT(a);
	}
	a = NPRED(EEND(e));
	while (a) {
		nr_points = 0;
		e1 = AKANTE(a);
		t  = (ETHICKNESS(e1)*G_stretch)/G_shrink;
		if ( t==0 )	t = 1;
		c  = ECOLOR(e1);
		if (!colored) c = BLACK;
		m = ELSTYLE(e1);
		y1 = y2 = yb +(-EANCHOR(e1)*16*mystretch)/myshrink;  
		switch (NSHAPE(v)) {
		case RHOMB:
			if (y1-myypos<h/2) d = ((h/2-y1+myypos)*w)/h;
			else		   d = ((y1-myypos-h/2)*w)/h; 
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		case TRIANGLE:
			y1 = y2 = y1 + h/4;
			d = ((h-y1+myypos)*w/2)/h;
			if (x1<x2) xx = x1-d+w/4;
			else	   xx = x1+d-w/4;
			break;
		case ELLIPSE:
			d = (w+1)/2 - gstoint( sqrt( 
				(double)(w*w)/4.0-(double)(w*w)/(double)(h*h)*
			  	((double)h/2.0-y1+myypos) 
				*((double)h/2.0-y1+myypos)));
			if (x1<x2) xx = x1-d;
			else	   xx = x1+d;
			break;
		default: xx = x1;
		}
		gbl_x2 = xx;
		gbl_y2 = y1;
		draw_arrowhead(e1,1);
		xx = gbl_x2;
		y1 = gbl_y2;
        	ax[nr_points]= xx; ay[nr_points++]= y1; 
        	ax[nr_points]= x2; ay[nr_points++]= y2; 
        	ax[nr_points]= EENDX(e1) * G_stretch/G_shrink; 
		ay[nr_points++]= EENDY(e1) * G_stretch/G_shrink; 
		draw_start_part(ax[0],ay[0],ax[1],ay[1], c,t,m);
		draw_spline(e1,0);
		a = ANEXT(a);
	}
}



/*  Look for the continue edge of a dummy node
 *  ------------------------------------------
 *  Assume that v is a dummy node. Every dummy node has only two edges.
 *  The one of these edges is e. We a looking for the other edge.
 */

#ifdef ANSI_C
static GEDGE dummy_continue_edge(GNODE v, GEDGE e)
#else
static GEDGE dummy_continue_edge(v,e)
GNODE v;
GEDGE e;
#endif
{
	GEDGE e2;
	CONNECT c;

	debugmessage("dummy_continue_edge","");

	e2 = NULL;
	c = NCONNECT(v);
	if (c && (CEDGE(c))) {
		if (!e2) e2 = CEDGE(c);
		if (e2==e) e2=NULL; 
	}
	if (c && (CEDGE2(c))) {
		if (!e2) e2 = CEDGE2(c);
		if (e2==e) e2=NULL; 
	}
	/* We have maximal 2 succs at dummy nodes */
	if (NSUCC(v)) {
		if (!e2) e2 = AKANTE(NSUCC(v));
		if (e2==e) e2=NULL; 
	}
	if (NSUCC(v) && (ANEXT(NSUCC(v)))) {
		if (!e2) e2 = AKANTE(ANEXT(NSUCC(v)));
		if (e2==e) e2=NULL; 
	}
	/* We have maximal 2 preds at dummy nodes */
	if (NPRED(v)) {
		if (!e2) e2 = AKANTE(NPRED(v));
		if (e2==e) e2=NULL; 
	}
	if (NPRED(v) && (ANEXT(NPRED(v)))) {
		if (!e2) e2 = AKANTE(ANEXT(NPRED(v)));
		if (e2==e) e2=NULL; 
	}
	return(e2);
}


/*  Draw the straight line from x0,y0 to the half of x1,y1
 *  ------------------------------------------------------
 */

#ifdef ANSI_C
static void draw_start_part(int x0,int y0,int x1,int y1,int c,int t,int m)
#else
static void draw_start_part(x0,y0,x1,y1,c,t,m)
int x0,y0,x1,y1,c,t,m;
#endif
{
	debugmessage("draw_start_part","");

	x1 = (x0+x1)/2;
	y1 = (y0+y1)/2;
	switch (m) {
	case SOLID:  gs_mysolidline( x0,y0,x1,y1,t,c);
		     break;
	case DASHED: gs_mydashedline(x0,y0,x1,y1,t,c);
		     break;
	case DOTTED: gs_mydottedline(x0,y0,x1,y1,t,c);
		     break;
	case UNVISIBLE: break;
	}
}


/*  Draw the straight line from the half of x0,y0 to x1,y1
 *  ------------------------------------------------------
 */

#ifdef ANSI_C
static void draw_final_part(int x0,int y0,int x1,int y1,int c,int t,int m)
#else
static void draw_final_part(x0,y0,x1,y1,c,t,m)
int x0,y0,x1,y1,c,t,m;
#endif
{
	debugmessage("draw_start_part","");

	x0 = (x0+x1)/2;
	y0 = (y0+y1)/2;

	switch (m) {
	case SOLID:  gs_mysolidline( x0,y0,x1,y1,t,c);
		     break;
	case DASHED: gs_mydashedline(x0,y0,x1,y1,t,c);
		     break;
	case DOTTED: gs_mydottedline(x0,y0,x1,y1,t,c);
		     break;
	case UNVISIBLE: break;
	}
}


/*  Draw the spline line from the half of x0,y0 to the half of x2,y2
 *  ----------------------------------------------------------------
 *  The idea is to find a triangle (spl_x0,spl_y0)-(spl_x2,spl_y2)-(x1,y1) 
 *  such that no point of a node etc. is drawn inside this triangle. 
 *  Then we can draw the spline completely inside the triangle.
 */

static int spl_x0, spl_y0, spl_x2, spl_y2;

#ifdef ANSI_C
static void draw_spline_part(
		int x0,int y0,int x1,int y1,int x2,int y2,int c,int t,int m)
#else
static void draw_spline_part(x0,y0,x1,y1,x2,y2,c,t,m)
int x0,y0,x1,y1,x2,y2,c,t,m;
#endif
{
	GNODE v,w;
	int k,r;
	double kk,rr;

	debugmessage("draw_spline_part","");


	spl_x0 = x0 = (x0+x1)/2;
	spl_y0 = y0 = (y0+y1)/2;
	spl_x2 = x2 = (x2+x1)/2;
	spl_y2 = y2 = (y2+y1)/2;

	kk = (double)(spl_x0-x1)* (double)(spl_x0-x1) +
	     (double)(spl_y0-y1)* (double)(spl_y0-y1);
	k = gstoint(sqrt(kk));
	rr = (double)(spl_x2-x1)* (double)(spl_x2-x1) +
	     (double)(spl_y2-y1)* (double)(spl_y2-y1);
	r = gstoint(sqrt(rr));

	if (2*k>3*r) {
		spl_x0 = x1 + (spl_x0-x1)*r/k;
		spl_y0 = y1 + (spl_y0-y1)*r/k;
	}
	else if (2*r>3*k) {
		spl_x2 = x1 + (spl_x2-x1)*k/r;
		spl_y2 = y1 + (spl_y2-y1)*k/r;
	}

#ifdef SPLINEDEBUG0
		gs_mysolidline( spl_x2,spl_y2,spl_x0,spl_y0,1,BLACK);
#endif

	/* Now, check all nodes and change spl_x0,...spl_y2
	 * such that no node is in the triangle.
	 */

	v = nodelist;
	while (v) {
		if ( (  (NX(v)*G_stretch/G_shrink!=x1)
		      ||(NY(v)*G_stretch/G_shrink!=y1))
		    &&(my_testnode1!=v)
		    &&(my_testnode2!=v)
		    &&(my_testnode3!=v))
			check_border_points(v,x1,y1);
		v = NNEXT(v);
	}
	v = labellist;
	while (v) {
		if ( (  (NX(v)*G_stretch/G_shrink!=x1)
		      ||(NY(v)*G_stretch/G_shrink!=y1))
		    &&(my_testnode1!=v)
		    &&(my_testnode2!=v)
		    &&(my_testnode3!=v))
			check_border_points(v,x1,y1);
		v = NNEXT(v);
	}
	v = dummylist;
	while (v) {
		if ( (  (NX(v)*G_stretch/G_shrink!=x1)
		      ||(NY(v)*G_stretch/G_shrink!=y1))
		    &&(my_testnode1!=v)
		    &&(my_testnode2!=v)
		    &&(my_testnode3!=v)) {
			w = NULL;	
			if (NTIEFE(v)==NTIEFE(my_testnode1)) w = my_testnode1; 
			if (NTIEFE(v)==NTIEFE(my_testnode2)) w = my_testnode2; 
			if (NTIEFE(v)==NTIEFE(my_testnode3)) w = my_testnode3; 
			if (w) check_special_dummy(v,w,x1,y1); 
			else check_border_points(v,x1,y1);
		}
		v = NNEXT(v);
	}

	if (((spl_x0!=x1)||(spl_y0!=x1))&&((spl_x2!=x1)||(spl_y2!=x1))) {
		gs_bezierspline(spl_x0,spl_y0,x1,y1,x1,y1,spl_x2,spl_y2,t,c,m); 

#ifdef SPLINEDEBUG1
		gs_mysolidline( spl_x2,spl_y2,spl_x0,spl_y0,1,BLACK);
#endif
	}
	if ((spl_x0!=x0)||(spl_y0!=y0)) {
		switch (m) {
		case SOLID:  gs_mysolidline( x0,y0,spl_x0,spl_y0,t,c);
			     break;
		case DASHED: gs_mydashedline(x0,y0,spl_x0,spl_y0,t,c);
			     break;
		case DOTTED: gs_mydottedline(x0,y0,spl_x0,spl_y0,t,c);
			     break;
		case UNVISIBLE: break;
		}
	}

	if ((spl_x2!=x2)||(spl_y2!=y2)) {
		switch (m) {
		case SOLID:  gs_mysolidline( spl_x2,spl_y2,x2,y2,t,c);
			     break;
		case DASHED: gs_mydashedline(spl_x2,spl_y2,x2,y2,t,c);
			     break;
		case DOTTED: gs_mydottedline(spl_x2,spl_y2,x2,y2,t,c);
			     break;
		case UNVISIBLE: break;
		}
	}

}


/*  Check whether a border point of node is inside the spline triangle 
 *  ------------------------------------------------------------------
 *  This is the same as check_border_points. However, node and the
 *  spline point sn are at the same level, and node is a dummy node.
 *  Thus we check whether the line through sn crosses the line
 *  through sn once. In this case, node does not hinder the spline
 *  through sn, because the crossing only shifts nearer to sn.
 * 
 *  Example:   |      |   can become    .       . 
 *             |      |    a spline      .     .
 *             sn    node                   .
 *               \  /                     .   .
 *                \/                     /     \
 *                /\                    /       \
 */

#ifdef ANSI_C
static void check_special_dummy(GNODE node,GNODE sn,int x1,int y1)
#else
static void check_special_dummy(node,sn,x1,y1)
GNODE node;
GNODE sn;
int x1,y1;
#endif
{
	int cross1, cross2;
	GEDGE e1,e2,e3,e4;
	GNODE tnode,snode,tsn,ssn,h;

	debugmessage("check_special_dummy","");

	/* assert((NTIEFE(node)==NTIEFE(sn))); */

	if (node==sn) return;
	e1 = dummy_continue_edge(sn,NULL);
	e2 = dummy_continue_edge(sn,e1);
	e3 = dummy_continue_edge(node,NULL);
	e4 = dummy_continue_edge(node,e3);

	if (!e1 || !e2 || !e3 || !e4) {
		check_border_points(node,x1,y1); 
		return; 
	}

	tsn = ESTART(e1);
	if (tsn==sn) tsn = EEND(e1);
	ssn = ESTART(e2);
	if (ssn==sn) ssn = EEND(e2);
	if (NTIEFE(ssn)>=NTIEFE(sn)) {
		h   = ssn;
		ssn = tsn;
		tsn = h;	
	}
	tnode = ESTART(e3);
	if (tnode==node) tnode = EEND(e3);
	snode = ESTART(e4);
	if (snode==node) snode = EEND(e4);
	if (NTIEFE(snode)>=NTIEFE(sn)) {
		h     = snode;
		snode = tnode;
		tnode = h;	
	}

	cross1 = 0;
	if ((NPOS(ssn)<NPOS(snode)) && (NPOS(sn)>NPOS(node))) cross1 = 1;
	if ((NPOS(ssn)>NPOS(snode)) && (NPOS(sn)<NPOS(node))) cross1 = 1;
	cross2 = 0;
	if ((NPOS(tsn)<NPOS(tnode)) && (NPOS(sn)>NPOS(node))) cross2 = 1;
	if ((NPOS(tsn)>NPOS(tnode)) && (NPOS(sn)<NPOS(node))) cross2 = 1;

	if (cross1+cross2 == 1) { return; }
	/* Okay, we have no crossing or two. Then we must be careful */

	check_border_points(node,x1,y1); 
}



/*  Check whether a border point of node is inside the spline triangle 
 *  ------------------------------------------------------------------
 *  The spline triangle is (spl_x0,spl_y0)-(spl_x2,spl_y2)-(x1,y1) 
 *  We change spl_x0, spl_y0, spl_x2, spl_y2, if a border point
 *  of v is inside the triangle.
 */

#ifdef ANSI_C
static void check_border_points(GNODE node,int x1,int y1)
#else
static void check_border_points(node,x1,y1)
GNODE node;
int x1,y1;
#endif
{
	int kx,ky;
	ADJEDGE a;
	GEDGE e;

	debugmessage("check_border_points","");
	if (!NANCHORNODE(node)) {
		switch(NSHAPE(node)) {
		case BOX:
			if ((NWIDTH(node)==0) && (NHEIGHT(node)==0)) {
				kx = NX(node);			
				ky = NY(node);
				check_spline_point(kx,ky,x1,y1);
				break;
			}
		case ELLIPSE:
			kx = NX(node) - 1;			
			ky = NY(node) - 1;
			check_spline_point(kx,ky,x1,y1);
			kx = NX(node) - 1;			
			ky = NY(node) + NHEIGHT(node) + 1;
			check_spline_point(kx,ky,x1,y1);
			kx = NX(node) + NWIDTH(node) + 1;			
			ky = NY(node) - 1;
			check_spline_point(kx,ky,x1,y1);
			kx = NX(node) + NWIDTH(node) + 1;			
			ky = NY(node) + NHEIGHT(node) + 1;
			check_spline_point(kx,ky,x1,y1);
			break;
		case RHOMB: 
			kx = NX(node) - 1;			
			ky = NY(node) + NHEIGHT(node)/2;
			check_spline_point(kx,ky,x1,y1);
			kx = NX(node) + NWIDTH(node)/2;			
			ky = NY(node) - 1;
			check_spline_point(kx,ky,x1,y1);
			kx = NX(node) + NWIDTH(node) + 1;			
			ky = NY(node) + NHEIGHT(node)/2;
			check_spline_point(kx,ky,x1,y1);
			kx = NX(node) + NWIDTH(node)/2;			
			ky = NY(node) + NHEIGHT(node) + 1;
			check_spline_point(kx,ky,x1,y1);
			break;
		case TRIANGLE: 
			switch (G_orientation) {
			case LEFT_TO_RIGHT:
				kx = NX(node) + NWIDTH(node) + 1;
				ky = NY(node) - 1;
				check_spline_point(kx,ky,x1,y1);
				kx = NX(node) + NWIDTH(node) + 1;
				ky = NY(node) + NHEIGHT(node) + 1;
				check_spline_point(kx,ky,x1,y1);
				kx = NX(node) - 1;			
				ky = NY(node) + NHEIGHT(node)/2;
				check_spline_point(kx,ky,x1,y1);
				break;
			case RIGHT_TO_LEFT:
				kx = NX(node) - 1;			
				ky = NY(node) - 1;
				check_spline_point(kx,ky,x1,y1);
				kx = NX(node) - 1;			
				ky = NY(node) + NHEIGHT(node) + 1;
				check_spline_point(kx,ky,x1,y1);
				kx = NX(node) + NWIDTH(node) + 1;
				ky = NY(node) + NHEIGHT(node)/2;
				check_spline_point(kx,ky,x1,y1);
				break;
			case BOTTOM_TO_TOP:
				kx = NX(node) - 1;			
				ky = NY(node) - 1;
				check_spline_point(kx,ky,x1,y1);
				kx = NX(node) + NWIDTH(node) + 1;
				ky = NY(node) - 1;
				check_spline_point(kx,ky,x1,y1);
				kx = NX(node) + NWIDTH(node)/2;
				ky = NY(node) + NHEIGHT(node) + 1;
				check_spline_point(kx,ky,x1,y1);
				break;
			case TOP_TO_BOTTOM:
				kx = NX(node) - 1;			
				ky = NY(node) + NHEIGHT(node) + 1;
				check_spline_point(kx,ky,x1,y1);
				kx = NX(node) + NWIDTH(node) + 1;
				ky = NY(node) + NHEIGHT(node) + 1;
				check_spline_point(kx,ky,x1,y1);
				kx = NX(node) + NWIDTH(node)/2;
				ky = NY(node) - 1;
				check_spline_point(kx,ky,x1,y1);
				break;
			}
		}
		
	}

	a = NSUCC(node);
	while (a) {
		e = AKANTE(a);
		if ((e)&&(ETBENDY(e)!=ESTARTY(e))) {	
			kx = ETBENDX(e);
			ky = ETBENDY(e);
			check_spline_point(kx,ky,x1,y1);
		}
		a = ANEXT(a);
	}
	a = NPRED(node);
	while (a) {
		e = AKANTE(a);
		if ((e)&&(EBBENDY(e)!=EENDY(e))) {	
			kx = EBBENDX(e);
			ky = EBBENDY(e);
			check_spline_point(kx,ky,x1,y1);
		}
		a = ANEXT(a);
	}

}



/*  Check whether a border point of (kx,ky) is inside the spline triangle 
 *  ---------------------------------------------------------------------
 *  The spline triangle is (spl_x0,spl_y0)-(spl_x2,spl_y2)-(x1,y1) 
 *  We change spl_x0, spl_y0, spl_x2, spl_y2, if a border point
 *  of v is inside the triangle.
 */

#ifdef ANSI_C
static void check_spline_point(int kx,int ky,int x1,int y1)
#else
static void check_spline_point(kx,ky,x1,y1)
int kx,ky,x1,y1;
#endif
{
	int ngx0,ngx2,ngy0,ngy2;
	int gradAZ, gradAN, gradBZ, gradBN;

	debugmessage("check_spline_point","");

	kx = kx*G_stretch/G_shrink;
	ky = ky*G_stretch/G_shrink;

	if ((kx==x1)&&(ky==y1))         return;
	if ((kx==spl_x0)&&(ky==spl_y0)) return;
	if ((kx==spl_x2)&&(ky==spl_y2)) return;

	/* First check whether it is a valid triangle */
	if ((spl_x0==spl_x2) && (spl_x2==x1)) 	  return;
	if ((spl_y0==spl_y2) && (spl_y2==y1)) 	  return;
	if ((spl_x0==spl_x2) && (spl_y0==spl_y2)) return;
	if ((spl_x0==x1)     && (spl_y0==y1)) 	  return;
	if ((spl_x2==x1)     && (spl_y2==y1)) 	  return;

	/* Now check whether kx,ky are near the triangle */
	if ((kx<=spl_x0)&&(kx<=spl_x2)&&(kx<=x1)) return;
	if ((kx>=spl_x0)&&(kx>=spl_x2)&&(kx>=x1)) return;
	if ((ky<=spl_y0)&&(ky<=spl_y2)&&(ky<=y1)) return;
	if ((ky>=spl_y0)&&(ky>=spl_y2)&&(ky>=y1)) return;


	/* Now check exactly */
	if (spl_x0!=spl_x2) {
		gradAZ = spl_y0-spl_y2;
		gradAN = spl_x0-spl_x2;
		if (spl_x0!=x1) {
			gradBZ = spl_y0-y1;
			gradBN = spl_x0-x1; 
			if (gradAZ*gradBN==gradBZ*gradAN) return;
			ngx0 = ( y1*gradAN*gradBN
				-ky*gradAN*gradBN
				+kx*gradAZ*gradBN
				-x1*gradBZ*gradAN)/
				(gradAZ*gradBN-gradBZ*gradAN);
			ngy0 = gradAZ*(ngx0-kx)/gradAN+ky;
		}
		else {
			ngx0 = x1;
			ngy0 = gradAZ*(ngx0-kx)/gradAN+ky;
		}
		if (spl_x2!=x1) {
			gradBZ = spl_y2-y1;
			gradBN = spl_x2-x1; 
			if (gradAZ*gradBN==gradBZ*gradAN) return;
			ngx2 = ( y1*gradAN*gradBN
				-ky*gradAN*gradBN
				+kx*gradAZ*gradBN
				-x1*gradBZ*gradAN)/
				(gradAZ*gradBN-gradBZ*gradAN);
			ngy2 = gradAZ*(ngx2-kx)/gradAN+ky;
		}
		else {
			ngx2 = x1;
			ngy2 = gradAZ*(ngx2-kx)/gradAN+ky;
		}
	}
	else { /* we know x1 != spl_x0 and x1 != spl_x2 */
			gradBZ = spl_y0-y1;
			gradBN = spl_x0-x1; 
			ngx0 = kx;
			ngy0 = gradBZ*(ngx0-x1)/gradBN+y1;
			gradBZ = spl_y2-y1;
			gradBN = spl_x2-x1; 
			ngx2 = kx;
			ngy2 = gradBZ*(ngx2-x1)/gradBN+y1;
	}

	if ((ngx0-x1>0) && (spl_x0-x1<0)) return;
	if ((ngy0-y1>0) && (spl_y0-y1<0)) return;
	if ((ngx2-x1>0) && (spl_x2-x1<0)) return;
	if ((ngy2-y1>0) && (spl_y2-y1<0)) return;

	if ((spl_x0>x1) && (ngx0>spl_x0)) return;
	if ((spl_x0<x1) && (ngx0<spl_x0)) return;
	if ((spl_y0>y1) && (ngy0>spl_y0)) return;
	if ((spl_y0<y1) && (ngy0<spl_y0)) return;
	if ((spl_x2>x1) && (ngx2>spl_x2)) return;
	if ((spl_x2<x1) && (ngx2<spl_x2)) return;
	if ((spl_y2>y1) && (ngy2>spl_y2)) return;
	if ((spl_y2<y1) && (ngy2<spl_y2)) return;

	spl_x0 = ngx0;
	spl_y0 = ngy0;
	spl_x2 = ngx2;
	spl_y2 = ngy2;


#ifdef SPLINEDEBUG2
	gs_mysolidline( spl_x2,spl_y2,kx,ky,1,BLACK);
#endif

}


