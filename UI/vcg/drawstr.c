/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */ 
/*		--------------------------------------		      */ 
/*								      */
/*   file:	   drawstr.c					      */
/*   version:	   1.00.00					      */
/*   creation:	   31.1.95					      */
/*   author:	   I. Lemke  (...-Version 0.99.99)		      */ 
/*		   G. Sander (Version 1.00.00-...)		      */ 
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */ 
/*   description:  Library of String Drawing Routines                 */ 
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
 * Revision 1.1  1995/02/08  11:11:14  sander
 * Initial revision
 *
 */

/************************************************************************
 *  This file is a collection of auxiliary functions that implement the
 *  primitivas to draw a string. It includes the aux-code-module drawchr.h.
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
 *    gs_calcstringsize       calculate width and height of a string
 *			      depending on scaling factor. After 
 *			      gs_calcstringsize, the result is in the
 *			      global variables
 *				   - int gs_stringw	  (the width)
 *				   - int gs_stringh	  (the height)
 *    gs_printstr	      print a string at the actual position into
 *			      the drawing device.
 *    gs_setto(x,y)	      set position (x,y) to start printing a string
 *    gs_setshrink(a,b)       set scaling factor to a/b for boxes and strings
 *
 * One important remark: string output is done with the
 * scaling factor we have set by gs_setshrink. The global values 
 * G_stretch and G_shrink do not influence that behaviour.
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
#include "drawlib.h"
#include "drawstr.h"

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

/* For X11 only some speedup functions */

#ifdef FAST_X11_DRAWING
extern void X11_fast_line _PP((int x1,int y1,int x2,int y3,int t,int c));
#endif


/* Static functions */

static void	mysetto 	_PP((int x,int y));
static void	mymoveto	_PP((int x,int y,int c));
static void	myoutchar	_PP((int c, int col,int underline));


#ifndef INCLUDE_DRAW
#ifdef X11

/* For X11, we can use font buffering to speed up */

int     set_fontbuffer          _PP((int stretch, int shrink));
void    draw_fast_char          _PP((int a,int c,int x,int y));
void    finish_fast_chars       _PP((void));
#endif
#endif



/*--------------------------------------------------------------------*/
/*   Simple Turtle graphics					      */
/*--------------------------------------------------------------------*/

/*   Turtle graphics is used to implement the string drawing routine  */


/*   Scaling factor
 *   --------------
 *   Scaling factor is mystretch/myshrink. All positioning is done
 *   relative to the scaling factor.
 */

int	mystretch = 1;
int	myshrink  = 1;


/*    Scaling limit for character output:
 *
 *            stlimit    mystretch
 *    If      -------  > ---------  then characters are too small for output.
 *            shlimit    myshrink
 *
 *    We avoid the output of unreadable small characters in order to save
 *    time. 
 */

int 	gs_stlimit = 1;
int	gs_shlimit = 3;


/*   Thickness of the text
 *   ---------------------
 */ 

static int 	mythick = 1;



/*  Turtle graphics cursor positions
 *  --------------------------------
 *  The absoulte origin of the string is (mystartxpos, mystartypos).
 *  All co-ordinates of the turtle graphics are scaled offsets to this
 *  origin.
 *  The actual position of the turtle is (myaktxpos, myaktypos). 
 */

int myxpos, myypos;
static int myaktxpos, myaktypos;


/*  Turtle graphics primitiva 
 *  -------------------------
 */

/* 
 *  Set turtle to (x,y) relatively to the origin (myxpos,myypos).
 */

#ifdef ANSI_C
static void mysetto(int x,int y)
#else
static void mysetto(x,y)
int x,y;
#endif
{
	int s=mystretch;
	int t=myshrink;
	myaktxpos = myxpos+(s*x)/t;
	myaktypos = myypos+(s*y)/t;
}


/* 
 *  Move the turtle from its actual position to (x,y) relatively to 
 *  the origin (myxpos,myypos). Draw this movement with color c.
 */

#ifdef ANSI_C
static void mymoveto(int x,int y,int c)
#else
static void mymoveto(x,y,c)
int x,y,c;
#endif
{
	int s=mystretch;
	int t=myshrink;
	int a = myxpos+(s*x)/t;
	int b = myypos+(s*y)/t;
	int tt= (mythick*s)/t;

	if (tt>1) {
#ifdef FAST_X11_DRAWING
		X11_fast_line(myaktxpos,myaktypos,a,b,tt,c);
#else
#ifdef POSTSCRIPT_DEVICE
		ps_thickline(myaktxpos,myaktypos,a,b,tt,c);
#else
		int e1, e2, i;

		gs_line(myaktxpos,myaktypos,a,b,c);
		e1 = tt/2;
		e2 = tt/2;
		if (tt%2==1) e2++;
		for (i=0; i<e1; i++) 
			gs_line(myaktxpos-i,myaktypos-e1+1+i,a-i,b+e1-1-i,c);
		for (i=0; i<e2; i++) 
			gs_line(myaktxpos+i,myaktypos-e2+1+i,a+i,b+e2-1-i,c);
#endif
#endif
	}
	else gs_line(myaktxpos,myaktypos,a,b,c);
	myaktxpos = a;
	myaktypos = b;
}


/*--------------------------------------------------------------------*/
/*   Character drawing routine					      */
/*--------------------------------------------------------------------*/


/*  The module drawchr.h contains the definition of the characters.
 *  The advantage of including it here is, that the C compiler will be
 *  able to inline the turtle graphic routines.
 */


#include "drawchr.h"




/*   Draw character c at color col
 *   -----------------------------
 *   We have implemented a 16x8 bit scalable font here.
 *   We draw the character at (myxpos,myypos) and then
 *   move myxpos one character size forward.
 */

#ifndef INCLUDE_DRAW
#ifdef X11
static int fast_char_possible = 0;
#endif
#endif

#ifdef ANSI_C
static void myoutchar(int c, int col, int underline)
#else
static void myoutchar(c,col,underline)
int	c;
int	col;
int 	underline;
#endif
{
#ifndef INCLUDE_DRAW
#ifdef X11
	if (fast_char_possible && (mythick==1)) {
		draw_fast_char(col,c,myxpos,myypos);
		if (underline) {
			mysetto(0,14); 
			mymoveto(8,14,col); 
		}
		myxpos += ((8*mystretch)/myshrink);
		return;
	}
#endif
#endif

	if (c<128) myasciichar(c,   col);
	else	   myisochar(c-128, col);

	if (underline) {
		int oldthick;

		oldthick = mythick;
		mythick = 1;
		mysetto(0,14); 
		mymoveto(8,14,col); 
		mythick = oldthick;
	}
}



/*--------------------------------------------------------------------*/
/*   String drawing						      */
/*--------------------------------------------------------------------*/

/*   Draw a string s in color c
 *   --------------------------
 *   (not more than MAXCHARS characters)
 *   We start drawing at (myxpos,myypos).
 */

#define MAXCHARS 20000


#ifdef ANSI_C
void gs_printstr(char *s, int c)
#else
void gs_printstr(s,c)
char	*s;
int	c;
#endif
{
	int	mx,my;		/* origin of the whole string */
	int 	xcnt, ycnt;	/* counter for the positions  */
	int 	i;		/* counter for the characters */
	int 	actcolor,underline;
	int 	cc;

	if (myshrink==0)		return;
	if (gs_stlimit*myshrink>gs_shlimit*mystretch)	return;

	i = xcnt = ycnt = 0;
	mx = myxpos;   /* store the origin */
	my = myypos;

	actcolor = c;
	underline = 0;
	mythick   = 1;
#ifndef INCLUDE_DRAW
#ifdef X11
	fast_char_possible = set_fontbuffer(mystretch,myshrink);
#endif
#endif
	while (*s) { 

		myxpos = mx + (xcnt *  8 * mystretch) / myshrink;
		myypos = my + (ycnt * 16 * mystretch) / myshrink;

		/* check visibility */
		if ( my + ((ycnt+1) * 16 * mystretch) / myshrink >= V_ymax) 
			return;

		/* analyse */
		switch (*s) {
		case '\n': /* Next line */
			xcnt = 0;
			ycnt++;
			break;
		case '\t': /* Tabbing: 8 steps forward */
			xcnt += 8;
			break;
		case '\r': /* Carriage return: ignore */
		case '\v': /* Vertical tabbing: ignore */
			break;
#ifdef ANSI_C
		case '\a': /* Beep in ANSI C */
			PRINTF("\a");FFLUSH(stdout);
			break;
#endif
		case '\b': /* Backspace */
			xcnt--;
			break;
		case '\f': /* Form feed is misused for colors and other
			    * controlling styles.
			    */
			s++;
			if (!*s) return;
			if   (*s=='u') underline = 1;
			else if (*s=='n') { underline = 0; mythick = 1; }
#ifdef POSTSCRIPT_DEVICE
			else if (*s=='b') { mythick = 2; }
			else if (*s=='B') { mythick = 3; }
#else
			else if (*s=='b') { mythick = 3; }
			else if (*s=='B') { mythick = 4; }
#endif
			else if (*s=='i') { 
				s++;
				if (!*s) return;
				cc = *s-'0';
				s++;
				if (!*s) return;
				cc = cc*10+(*s-'0');
				s++;
				if (!*s) return;
				cc = cc*10+(*s-'0');
				if (   (myypos >=V_ymin) && (myxpos >=V_xmin) 
				    && (mx+((xcnt+1)*8*mystretch)/myshrink 
						< V_xmax))
					 myoutchar(cc,actcolor,underline);
				xcnt++;
			}
			else {  actcolor = *s-'0';
				s++;
				if (!*s) return;
				actcolor = actcolor*10 + (*s-'0');
			}
			if (actcolor<0)          actcolor = c;
			if (actcolor>=cmap_size) actcolor = c;
			if (!colored)	         actcolor = c;
			break;
		default: /* check visibility again, and draw */
			if (   (myypos >=V_ymin) && (myxpos >=V_xmin) 
			    && (mx+((xcnt+1)*8*mystretch)/myshrink < V_xmax))
				 myoutchar(*s,actcolor,underline);
			xcnt++;
		}
		i++;
		if (i>MAXCHARS) return;
		s++;
	}

	myxpos = mx + (xcnt *  8 * mystretch) / myshrink;
	myypos = my + (ycnt * 16 * mystretch) / myshrink;
#ifndef INCLUDE_DRAW
#ifdef X11
	if (fast_char_possible) finish_fast_chars();
#endif
#endif
}


/* Calculate the size of a string s
 * --------------------------------
 * The width of the string is returned in gs_stringw, the height is
 * returned in gs_stringh. Both values are scaled relatively to
 * mystretch/myshrink.
 * Note: We do not calculate the size of the visible part of the string, 
 * but the size of the entire string.
 */

int gs_stringw; 
int gs_stringh; 


#if 0
#ifdef ANSI_C
void gs_calcstringsize(char *s)
#else
void gs_calcstringsize(s)
char *s;
#endif
{
#if 0
	int	a,b,c;
	char	*ss;   

	b = 1;
	a = c = 0;
	ss = s;  
	while(*ss) {
		switch (*ss) {
		case '\n': /* Next line */
			b++;
			if (c>a) a=c;
			c=0;
			break;
		case '\t': /* Tabbing: 8 steps forward */
			c += 8;
			break;
		case '\r': /* Carriage return: ignore */
		case '\v': /* Vertical tabbing: ignore */
#ifdef ANSI_C
		case '\a': /* Beep in ANSI C */
#endif
			break;
		case '\b': /* Backspace */
			c--;
			break;
		case '\f': /* Form feed is misused for colors and other
			    * controlling styles.
			    */
			ss++;
			if (!*ss) {
				if (c>a) a=c;
				gs_stringw = (a*8*mystretch)/myshrink; 
				gs_stringh = (b*16*mystretch)/myshrink; 
				return;
			} 
			if   (*ss=='u') break;
			else if (*ss=='n') break;
			else if (*ss=='b')  break; 
			else if (*ss=='B')  break;
			else if (*ss=='i') { 
				ss++;
				if (!*ss) {
					if (c>a) a=c;
					gs_stringw =(a*8*mystretch)/myshrink; 
					gs_stringh =(b*16*mystretch)/myshrink; 
					return;
				} 
				ss++;
				if (!*ss) {
					if (c>a) a=c;
					gs_stringw =(a*8*mystretch)/myshrink; 
					gs_stringh =(b*16*mystretch)/myshrink; 
					return;
				} 
				ss++;
				if (!*ss) {
					if (c>a) a=c;
					gs_stringw =(a*8*mystretch)/myshrink; 
					gs_stringh =(b*16*mystretch)/myshrink; 
					return;
				}
				c++; 
			}
			else {  ss++;
				if (!*ss) {
					if (c>a) a=c;
					gs_stringw = (a*8*mystretch)/myshrink; 
					gs_stringh = (b*16*mystretch)/myshrink; 
					return;
				}
				break;
			}
			break;
		default: c++; 
		}
		ss++;  
	}
	if (c>a) a=c;
	gs_stringw = (a*8*mystretch)/myshrink; 
	gs_stringh = (b*16*mystretch)/myshrink; 
#else
    // TODO: put code here to determine the actual size of a techs; for now, we'll just use a constant size for all techs
    gs_stringw = (20*8*mystretch)/myshrink;
	gs_stringh = (1*16*mystretch)/myshrink;
#endif
}
#endif
 

/*  Set start position of a string
 *  ------------------------------
 */

#ifdef ANSI_C
void gs_setto(int x,int y)
#else
void gs_setto(x,y)
int x,y;
#endif
{
	myxpos = x;
	myypos = y;
}


/*  Set scale factor of a string
 *  ----------------------------
 */

#ifdef ANSI_C
void gs_setshrink(int a,int b)
#else
void gs_setshrink(a,b)
int a,b;
#endif
{
	mystretch = a;
	myshrink  = b;
}


