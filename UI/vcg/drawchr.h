
/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */ 
/*		--------------------------------------		      */ 
/*								      */
/*   file:	   drawchr.h					      */
/*   version:	   1.00.00					      */
/*   creation:	   31.1.95					      */
/*   author:	   I. Lemke  (...-Version 0.99.99)		      */ 
/*		   G. Sander (Version 1.00.00-...)		      */ 
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */ 
/*   description:  Code module to draw characters                     */ 
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
 * Revision 1.1  1995/02/08  11:11:14  sander
 * Initial revision
 *
 */

/************************************************************************
 *  This file contains the definition of the character output primitive
 *  function.
 *  This module is a little bit ugly: it is a h-module even if it only
 *  contains some static functions. The reason is, that it is included
 *  in some c-modules (device drivers)  with different interface functions 
 *  that are defined differently in these c-modules. For instance, see
 *  drawstr.c. It requires the definition of a simple turtle graphics with 
 *  interface functions (look into the device drivers):
 *
 *     mysetto    - set the turtle to a certain point
 *     mymoveto   - move the turtle to the next point and draw with color
 *
 *  This module provides the following functions:
 *
 *     myasciichar  - draw a ASCII character with color (code set 0-127).
 *     myisochar    - draw a ISO   character with color (code set 128-255).
 *
 ************************************************************************/
 

#ifndef DRAWCHR_H
#define DRAWCHR_H

/*--------------------------------------------------------------------*/

/*   Draw ASCII character c at color col
 *   -----------------------------------
 *   c is in 0 ... 127.
 *   We have implemented a 16x8 bit scalable font here.
 */

#ifdef ANSI_C
static void myasciichar(int c, int col)
#else
static void myasciichar(c,col)
int	c;
int	col;
#endif
{
	switch(c) {
	case ' ':
		break; 
	case '!': 
		mysetto(3,2); 
		mymoveto(3,8,col);
		mysetto(3,11); 
		mymoveto(3,11,col);
		break;
	case '"': 
		mysetto(2,2); 
		mymoveto(2,4,col);
		mysetto(5,2); 
		mymoveto(5,4,col);
		break;
	case '#': 
		mysetto(1,11); 
		mymoveto(3,2,col);
		mysetto(1,5); 
		mymoveto(7,5,col);
		mysetto(0,9); 
		mymoveto(6,9,col);
		mysetto(4,11); 
		mymoveto(6,2,col);
		break;
	case '$': 
		mysetto(5,4); 
		mymoveto(5,3,col);
		mymoveto(4,2,col); 
		mymoveto(2,2,col); 
		mymoveto(1,3,col);
		mymoveto(1,4,col); 
		mymoveto(3,6,col); 
		mymoveto(4,6,col);
		mymoveto(5,8,col); 
		mymoveto(5,10,col); 
		mymoveto(4,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,9,col);
		mysetto(3,1); 
		mymoveto(3,12,col);
		break;
	case '%': 
		mysetto(0,3); 
		mymoveto(1,2,col); 
		mymoveto(2,2,col);
		mymoveto(3,3,col); 
		mymoveto(3,4,col); 
		mymoveto(2,5,col);
		mymoveto(1,5,col); 
		mymoveto(0,4,col); 
		mymoveto(0,3,col);
		mysetto(3,9); 
		mymoveto(4,8,col); 
		mymoveto(5,8,col);
		mymoveto(6,9,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(4,11,col); 
		mymoveto(3,10,col); 
		mymoveto(3,9,col);
		mysetto(6,2); 
		mymoveto(1,11,col);
		break;
	case '&': 
		mysetto(6,7); 
		mymoveto(3,11,col);
		mymoveto(1,11,col); 
		mymoveto(0,10,col);
		mymoveto(0,7,col); 
		mymoveto(4,4,col);
		mymoveto(4,3,col); 
		mymoveto(3,2,col);
		mymoveto(2,2,col); 
		mymoveto(1,3,col);
		mymoveto(1,4,col); 
		mymoveto(2,5,col);
		mymoveto(4,9,col); 
		mymoveto(4,10,col);
		mymoveto(6,11,col);
		break;
	case '\'': 
		mysetto(4,2); 
		mymoveto(4,3,col); 
		mymoveto(2,5,col);
		break;
	case '(': 
		mysetto(5,1); 
		mymoveto(2,4,col);
		mymoveto(2,9,col); 
		mymoveto(5,12,col); 
		break;
	case ')': 
		mysetto(2,1); 
		mymoveto(5,4,col);
		mymoveto(5,9,col); 
		mymoveto(2,12,col); 
		break;
	case '*': 
		mysetto(1,5); 
		mymoveto(2,6,col);
		mymoveto(4,6,col); 
		mymoveto(5,7,col);
		mysetto(3,4); 
		mymoveto(3,8,col);
		mysetto(1,7); 
		mymoveto(2,6,col);
		mymoveto(4,6,col); 
		mymoveto(5,5,col);
		break;
	case '+': 
		mysetto(0,7); 
		mymoveto(6,7,col);
		mysetto(3,10); 
		mymoveto(3,4,col);
		break;
	case ',': 
		mysetto(4,10); 
		mymoveto(5,10,col);
		mymoveto(5,11,col); 
		mymoveto(4,11,col); 
		mymoveto(4,10,col);
		mysetto(5,11); 
		mymoveto(5,12,col); 
		mymoveto(3,14,col);
		break;
	case '-': 
		mysetto(1,7); 
		mymoveto(6,7,col);
		break;
	case '.': 
		mysetto(4,10); 
		mymoveto(5,10,col);
		mymoveto(5,11,col); 
		mymoveto(4,11,col); 
		mymoveto(4,10,col);
		break;
	case '/': 
		mysetto(0,12); 
		mymoveto(6,1,col);
		break;
	case '0': 
		mysetto(2,10); 
		mymoveto(2,3,col);
		mymoveto(3,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(3,11,col); 
		mymoveto(2,10,col);
		break;
	case '1': 
		mysetto(2,11); 
		mymoveto(6,11,col);
		mysetto(4,11); 
		mymoveto(4,2,col); 
		mymoveto(2,4,col);
		break;
	case '2': 
		mysetto(1,4); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,4,col); 
		mymoveto(1,11,col);
		mymoveto(6,11,col);
		break;
	case '3': 
		mysetto(1,4); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,5,col); 
		mymoveto(5,6,col);
		mymoveto(3,6,col);
		mysetto(5,6); 
		mymoveto(6,7,col); 
		mymoveto(6,10,col);
		mymoveto(5,11,col); 
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mymoveto(1,9,col);
		break;
	case '4': 
		mysetto(5,11); 
		mymoveto(5,2,col);
		mymoveto(0,8,col); 
		mymoveto(6,8,col);
		break;
	case '5': 
		mysetto(6,2); 
		mymoveto(1,2,col); 
		mymoveto(1,6,col);
		mymoveto(5,6,col); 
		mymoveto(6,7,col); 
		mymoveto(6,10,col);
		mymoveto(5,11,col); 
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mymoveto(1,9,col);
		break;
	case '6': 
		mysetto(6,3); 
		mymoveto(5,2,col); 
		mymoveto(2,2,col);
		mymoveto(1,3,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mymoveto(6,7,col);
		mymoveto(5,6,col); 
		mymoveto(1,6,col);
		break;
	case '7': 
		mysetto(1,3); 
		mymoveto(1,2,col); 
		mymoveto(6,2,col);
		mymoveto(4,10,col); 
		mymoveto(4,11,col);
		break;
	case '8': 
		mysetto(1,5); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,5,col); 
		mymoveto(5,6,col);
		mymoveto(2,6,col);  
		mymoveto(1,5,col);
		mysetto(5,6); 
		mymoveto(6,7,col); 
		mymoveto(6,10,col);
		mymoveto(5,11,col); 
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mymoveto(1,7,col); 
		mymoveto(2,6,col);
		break;
	case '9': 
		mysetto(1,10); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		mymoveto(6,3,col); 
		mymoveto(5,2,col); 
		mymoveto(2,2,col);
		mymoveto(1,3,col);  
		mymoveto(1,6,col);  
		mymoveto(2,7,col);
		mymoveto(6,7,col);
		break;
	case ':': 
		mysetto(4,5); 
		mymoveto(5,5,col);
		mymoveto(5,6,col); 
		mymoveto(4,6,col); 
		mymoveto(4,5,col);
		mysetto(4,10); 
		mymoveto(5,10,col);
		mymoveto(5,11,col); 
		mymoveto(4,11,col); 
		mymoveto(4,10,col);
		break;
	case ';': 
		mysetto(4,5); 
		mymoveto(5,5,col);
		mymoveto(5,6,col); 
		mymoveto(4,6,col); 
		mymoveto(4,5,col);
		mysetto(4,10); 
		mymoveto(5,10,col);
		mymoveto(5,11,col); 
		mymoveto(4,11,col); 
		mymoveto(4,10,col);
		mysetto(5,11); 
		mymoveto(5,12,col); 
		mymoveto(3,14,col);
		break;
	case '<': 
		mysetto(6,1); 
		mymoveto(1,6,col);
		mymoveto(6,11,col); 
		break;
	case '=': 
		mysetto(1,5); 
		mymoveto(6,5,col);
		mysetto(1,9); 
		mymoveto(6,9,col);
		break;
	case '>': 
		mysetto(1,1); 
		mymoveto(6,6,col);
		mymoveto(1,11,col); 
		break;
	case '?': 
		mysetto(1,4); 
		mymoveto(1,3,col); 
		mymoveto(2,2,col);
		mymoveto(5,2,col); 
		mymoveto(6,3,col); 
		mymoveto(6,4,col);
		mymoveto(4,7,col); 
		mymoveto(4,8,col); 
		mysetto(4,11);
		mymoveto(4,11,col);
		break;
	case '@': 
		mysetto(6,10); 
		mymoveto(5,11,col);
		mymoveto(3,11,col); 
		mymoveto(1,9,col); 
		mymoveto(1,4,col);
		mymoveto(3,2,col); 
		mymoveto(4,2,col); 
		mymoveto(6,4,col);
		mymoveto(6,7,col); 
		mymoveto(5,8,col); 
		mymoveto(4,8,col);
		mymoveto(3,7,col); 
		mymoveto(3,6,col); 
		mymoveto(4,5,col);
		mymoveto(5,5,col); 
		mymoveto(6,6,col);
		break;
	case 'A': 
		mysetto(1,11); 
		mymoveto(1,8,col); 
		mymoveto(3,2,col);
		mymoveto(4,2,col); 
		mymoveto(6,8,col); 
		mymoveto(6,11,col);
		mysetto(1,8); 
		mymoveto(6,8,col);
		break;
	case 'B': 
		mysetto(1,11); 
		mymoveto(1,2,col); 
		mymoveto(4,2,col);
		mymoveto(5,3,col); 
		mymoveto(5,5,col); 
		mymoveto(4,6,col);
		mymoveto(1,6,col);
		mysetto(4,6); 
		mymoveto(6,8,col); 
		mymoveto(6,10,col);
		mymoveto(5,11,col); 
		mymoveto(1,11,col);
		break;
	case 'C': 
		mysetto(6,4); 
		mymoveto(6,3,col); 
		mymoveto(5,2,col);
		mymoveto(2,2,col);
		mymoveto(1,3,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mymoveto(6,9,col);
		break;
	case 'D': 
		mysetto(1,2); 
		mymoveto(1,11,col); 
		mymoveto(4,11,col);
		mymoveto(6,9,col); 
		mymoveto(6,4,col); 
		mymoveto(4,2,col);
		mymoveto(1,2,col);
		break;
	case 'E': 
		mysetto(1,2); 
		mymoveto(1,11,col); 
		mymoveto(6,11,col);
		mysetto(1,2); 
		mymoveto(6,2,col);
		mysetto(1,6); 
		mymoveto(5,6,col);
		break;
	case 'F': 
		mysetto(1,2); 
		mymoveto(1,11,col);
		mysetto(1,2); 
		mymoveto(6,2,col);
		mysetto(1,6); 
		mymoveto(5,6,col);
		break;
	case 'G': 
		mysetto(6,4); 
		mymoveto(6,3,col); 
		mymoveto(5,2,col);
		mymoveto(2,2,col);
		mymoveto(1,3,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(4,11,col); 
		mymoveto(6,10,col);
		mysetto(6,11); 
		mymoveto(6,7,col);
		mymoveto(4,7,col);
		break;
	case 'H': 
		mysetto(1,2); 
		mymoveto(1,11,col);
		mysetto(6,2); 
		mymoveto(6,11,col);
		mysetto(1,6); 
		mymoveto(6,6,col);
		break;
	case 'I': 
		mysetto(1,2); 
		mymoveto(5,2,col);
		mysetto(3,2); 
		mymoveto(3,11,col);
		mysetto(1,11); 
		mymoveto(5,11,col);
		break;
	case 'J': 
		mysetto(3,2); 
		mymoveto(6,2,col);
		mysetto(5,2); 
		mymoveto(5,10,col);
		mymoveto(4,11,col); 
		mymoveto(2,11,col);
		mymoveto(1,10,col); 
		mymoveto(1,9,col);
		break;
	case 'K': 
		mysetto(1,2); 
		mymoveto(1,11,col);
		mysetto(1,7); 
		mymoveto(6,2,col);
		mysetto(1,7); 
		mymoveto(6,11,col);
		break;
	case 'L': 
		mysetto(1,2); 
		mymoveto(1,11,col); 
		mymoveto(6,11,col);
		break;
	case 'M': 
		mysetto(0,11); 
		mymoveto(0,2,col); 
		mymoveto(3,9,col);
		mymoveto(6,2,col); 
		mymoveto(6,11,col);
		break;
	case 'N': 
		mysetto(1,11); 
		mymoveto(1,2,col); 
		mymoveto(6,11,col);
		mymoveto(6,2,col);
		break;
	case 'O': 
		mysetto(1,10); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		break;
	case 'P': 
		mysetto(1,2); 
		mymoveto(1,11,col);
		mysetto(1,2); 
		mymoveto(5,2,col); 
		mymoveto(6,3,col);
		mymoveto(6,5,col); 
		mymoveto(5,6,col); 
		mymoveto(1,6,col);
		break;
	case 'Q': 
		mysetto(1,10); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mysetto(2,11); 
		mymoveto(5,14,col); 
		mymoveto(6,14,col);
		break;
	case 'R': 
		mysetto(1,2); 
		mymoveto(1,11,col);
		mysetto(1,2); 
		mymoveto(5,2,col); 
		mymoveto(6,3,col);
		mymoveto(6,5,col); 
		mymoveto(5,6,col); 
		mymoveto(1,6,col);
		mysetto(3,6); 
		mymoveto(6,11,col);
		break;
	case 'S': 
		mysetto(6,4); 
		mymoveto(6,3,col);
		mymoveto(5,2,col); 
		mymoveto(2,2,col); 
		mymoveto(1,3,col);
		mymoveto(1,4,col); 
		mymoveto(3,6,col); 
		mymoveto(4,6,col);
		mymoveto(6,8,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,9,col);
		break;
	case 'T': 
		mysetto(0,2); 
		mymoveto(6,2,col);
		mysetto(3,2); 
		mymoveto(3,11,col);
		break;
	case 'U': 
		mysetto(1,2); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mymoveto(6,2,col);
		break;
	case 'V': 
		mysetto(1,2); 
		mymoveto(3,11,col); 
		mymoveto(4,11,col);
		mymoveto(6,2,col);
		break;
	case 'W': 
		mysetto(0,2); 
		mymoveto(2,11,col);
		mymoveto(3,5,col); 
		mymoveto(4,11,col); 
		mymoveto(6,2,col);
		break;
	case 'X': 
		mysetto(1,2); 
		mymoveto(1,3,col);
		mymoveto(6,10,col); 
		mymoveto(6,11,col);
		mysetto(1,11); 
		mymoveto(1,10,col);
		mymoveto(6,3,col); 
		mymoveto(6,2,col);
		break;
	case 'Y': 
		mysetto(0,2); 
		mymoveto(3,7,col); 
		mymoveto(6,2,col);
		mysetto(3,7); 
		mymoveto(3,11,col);
		break;
	case 'Z': 
		mysetto(1,2); 
		mymoveto(6,2,col);
		mymoveto(6,4,col); 
		mymoveto(1,9,col); 
		mymoveto(1,11,col);
		mymoveto(6,11,col);
		break;
	case '[': 
		mysetto(5,1); 
		mymoveto(2,1,col);
		mymoveto(2,13,col); 
		mymoveto(5,13,col);
		break;
	case '\\': 
		mysetto(0,1); 
		mymoveto(6,12,col);
		break;
	case ']': 
		mysetto(2,1); 
		mymoveto(5,1,col);
		mymoveto(5,13,col); 
		mymoveto(2,13,col);
		break;
	case '^': 
		mysetto(1,5); 
		mymoveto(3,3,col);
		mymoveto(4,3,col); 
		mymoveto(6,5,col);
		break;
	case '_': 
		mysetto(0,12); 
		mymoveto(6,12,col); 
		break;
	case '`': 
		mysetto(2,2); 
		mymoveto(2,3,col); 
		mymoveto(4,5,col);
		break;
	case 'a': 
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(4,5,col);
		mymoveto(5,6,col); 
		mymoveto(5,10,col); 
		mymoveto(6,11,col);
		mysetto(5,10); 
		mymoveto(4,11,col); 
		mymoveto(2,11,col);
		mymoveto(1,10,col); 
		mymoveto(1,9,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col);
		break;
	case 'b': 
		mysetto(1,2); 
		mymoveto(1,11,col);
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		break;
	case 'c': 
		mysetto(6,6); 
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		break;
	case 'd': 
		mysetto(6,2); 
		mymoveto(6,11,col);
		mysetto(6,6); 
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		break;
	case 'e': 
		mysetto(1,8); 
		mymoveto(6,8,col); 
		mymoveto(6,6,col);
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		break;
	case 'f': 
		mysetto(1,6); 
		mymoveto(5,6,col);
		mysetto(3,11); 
		mymoveto(3,3,col); 
		mymoveto(4,2,col);
		mymoveto(5,2,col); 
		mymoveto(6,3,col);
		break;
	case 'g': 
		mysetto(6,6); 
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		mysetto(6,5); 
		mymoveto(6,13,col); 
		mymoveto(5,14,col);
		mymoveto(2,14,col); 
		mymoveto(1,13,col);
		break;
	case 'h': 
		mysetto(1,2); 
		mymoveto(1,11,col);
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,11,col);
		break;
	case 'i': 
		mysetto(4,2); 
		mymoveto(4,1,col);
		mysetto(2,5); 
		mymoveto(4,5,col); 
		mymoveto(4,11,col);
		break;
	case 'j': 
		mysetto(5,2); 
		mymoveto(5,1,col);
		mysetto(3,5); 
		mymoveto(5,5,col); 
		mymoveto(5,13,col);
		mymoveto(4,14,col); 
		mymoveto(2,14,col);
		mymoveto(1,13,col);
		break;
	case 'k': 
		mysetto(1,2); 
		mymoveto(1,11,col);
		mysetto(1,9); 
		mymoveto(2,9,col); 
		mymoveto(5,6,col);
		mysetto(2,9); 
		mymoveto(6,11,col);
		break;
	case 'l': 
		mysetto(2,2); 
		mymoveto(4,2,col); 
		mymoveto(4,11,col);
		break;
	case 'm': 
		mysetto(0,11); 
		mymoveto(0,5,col);
		mymoveto(2,5,col); 
		mymoveto(3,6,col); 
		mymoveto(3,11,col);
		mysetto(3,6); 
		mymoveto(4,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,11,col);
		break;
	case 'n': 
		mysetto(1,11); 
		mymoveto(1,5,col);
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,11,col);
		break;
	case 'o': 
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		break;
	case 'p': 
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,5,col);
		mymoveto(1,15,col);
		break;
	case 'q': 
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		mysetto(6,5); 
		mymoveto(6,15,col);
		break;
	case 'r': 
		mysetto(1,11); 
		mymoveto(1,5,col);
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col);
		break;
	case 's': 
		mysetto(6,6); 
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,7,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col); 
		mymoveto(6,9,col); 
		mymoveto(6,10,col);
		mymoveto(5,11,col); 
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		break;
	case 't': 
		mysetto(1,5); 
		mymoveto(4,5,col);
		mysetto(3,3); 
		mymoveto(3,10,col); 
		mymoveto(4,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		break;
	case 'u': 
		mysetto(6,5); 
		mymoveto(6,11,col);
		mysetto(6,10); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,5,col);
		break;
	case 'v': 
		mysetto(1,5);
		mymoveto(4,11,col); 
		mymoveto(6,5,col);
		break;
	case 'w': 
		mysetto(0,5); 
		mymoveto(0,10,col); 
		mymoveto(1,11,col);
		mymoveto(2,11,col); 
		mymoveto(3,10,col); 
		mymoveto(3,7,col);
		mysetto(3,10); 
		mymoveto(4,11,col); 
		mymoveto(5,11,col);
		mymoveto(6,10,col); 
		mymoveto(6,5,col);
		break;
	case 'x': 
		mysetto(1,5); 
		mymoveto(1,6,col); 
		mymoveto(6,10,col);
		mymoveto(6,11,col);
		mysetto(1,11); 
		mymoveto(1,10,col); 
		mymoveto(6,6,col);
		mymoveto(6,5,col);
		break;
	case 'y': 
		mysetto(1,5); 
		mymoveto(4,12,col);
		mysetto(6,5); 
		mymoveto(4,12,col); 
		mymoveto(2,15,col);
		mymoveto(1,14,col);
		break;
	case 'z': 
		mysetto(1,5); 
		mymoveto(6,5,col); 
		mymoveto(1,11,col);
		mymoveto(6,11,col);
		break;
	case '{': 
		mysetto(6,1); 
		mymoveto(4,1,col); 
		mymoveto(3,2,col);
		mymoveto(3,6,col); 
		mymoveto(2,7,col); 
		mymoveto(1,7,col);
		mymoveto(2,7,col); 
		mymoveto(3,8,col); 
		mymoveto(3,12,col);
		mymoveto(4,13,col); 
		mymoveto(6,13,col);
		break;
	case '|': 
		mysetto(3,1); 
		mymoveto(3,12,col);
		break;
	case '}': 
		mysetto(1,1); 
		mymoveto(3,1,col); 
		mymoveto(4,2,col);
		mymoveto(4,6,col); 
		mymoveto(5,7,col); 
		mymoveto(6,7,col);
		mymoveto(5,7,col); 
		mymoveto(4,8,col); 
		mymoveto(4,12,col);
		mymoveto(3,13,col); 
		mymoveto(1,13,col);
		break;
	case '~': 
		mysetto(1,4); 
		mymoveto(2,3,col); 
		mymoveto(3,3,col);
		mymoveto(4,4,col); 
		mymoveto(5,4,col); 
		mymoveto(6,3,col);
		break;
	}
}

/*--------------------------------------------------------------------*/

/*   Draw ISO character c at color col
 *   ---------------------------------
 *   This character set is compatible to the ISO Latin 1 PostScript
 *   character encoding vector. 
 *   c is in 0 ... 127. The corresponding ISO character is c+128.
 *   We have implemented a 16x8 bit scalable font here.
 */

#ifdef ANSI_C
static void myisochar(int c, int col)
#else
static void myisochar(c,col)
int	c;
int	col;
#endif
{
	switch(c) {
	case '\020':   /* dotless i */
		mysetto(2,5); 
		mymoveto(4,5,col); 
		mymoveto(4,11,col);
		break;
	case '\021':   /* accent grave */
		mysetto(2,1); 
		mymoveto(4,2,col); 
		break;
	case '\022':   /* accent accute */
	case '\064':   /* accent accute */
		mysetto(4,1); 
		mymoveto(2,2,col); 
		break;
	case '\023':   /* accent circumflex */
		mysetto(2,2); 
		mymoveto(3,1,col); 
		mymoveto(4,2,col); 
		break;
	case '\024':   /* accent tilde */
		mysetto(1,2); 
		mymoveto(2,1,col); 
		mymoveto(4,2,col); 
		mymoveto(5,1,col); 
		break;
	case '\025':   /* accent macron */
	case '\057':   /* accent macron */
		mysetto(1,1); 
		mymoveto(5,1,col); 
		break;
	case '\026':   /* accent breve */
		mysetto(1,1); 
		mymoveto(2,2,col); 
		mymoveto(4,2,col); 
		mymoveto(5,1,col); 
		break;
	case '\027':   /* accent dot */
		mysetto(4,1); 
		mymoveto(4,2,col); 
		break;
	case '\030':   /* accent double dot */
	case '\050':   /* dieresis */
		mysetto(2,1); 
		mymoveto(2,2,col); 
		mysetto(4,1); 
		mymoveto(4,2,col); 
		break;
	case '\032':   /* ring */
	case '\060':   /* degree */
		mysetto(2,1); 
		mymoveto(3,1,col); 
		mymoveto(4,2,col); 
		mymoveto(4,3,col); 
		mymoveto(3,4,col); 
		mymoveto(2,4,col); 
		mymoveto(1,3,col); 
		mymoveto(1,2,col); 
		mymoveto(2,1,col); 
		break;
	case '\033':   /* cedilla */
	case '\070':   /* cedilla */
		mysetto(3,11); 
		mymoveto(3,13,col); 
		mymoveto(4,13,col); 
		mymoveto(5,14,col); 
		mymoveto(4,15,col); 
		mymoveto(3,15,col); 
		break;
	case '\035':   /* hungarumlaut */
		mysetto(3,1); 
		mymoveto(1,2,col); 
		mysetto(5,1); 
		mymoveto(3,2,col); 
		break;
	case '\036':   /* ogonek */
		mysetto(4,11); 
		mymoveto(3,13,col); 
		mymoveto(3,14,col); 
		mymoveto(5,14,col); 
		break;
	case '\037':   /* accent caron */
		mysetto(2,1); 
		mymoveto(3,2,col); 
		mymoveto(4,1,col); 
		break;
	case '\040':   /* space */
		break;
	case '\041':   /* exclam down */
		mysetto(3,14); 
		mymoveto(3,8,col);
		mysetto(3,5); 
		mymoveto(3,5,col);
		break;
	case '\042':   /* cent */
		mysetto(6,6); 
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		mysetto(4,4); 
		mymoveto(2,12,col);
		break;
	case '\043':   /* pound */
		mysetto(6,4); 
		mymoveto(6,3,col); 
		mymoveto(5,2,col);
		mymoveto(3,2,col);
		mymoveto(2,3,col); 
		mymoveto(3,7,col); 
		mymoveto(1,11,col); 
		mymoveto(2,10,col); 
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mysetto(1,7); 
		mymoveto(5,7,col); 
		break;
	case '\044':   /* currency */
		mysetto(1,5); 
		mymoveto(2,4,col); 
		mymoveto(4,4,col);
		mymoveto(5,5,col); 
		mymoveto(5,7,col); 
		mymoveto(4,8,col);
		mymoveto(2,8,col); 
		mymoveto(1,7,col); 
		mymoveto(1,5,col);
		mysetto(0,3); 
		mymoveto(1,4,col);
		mysetto(0,9); 
		mymoveto(1,8,col);
		mysetto(6,3); 
		mymoveto(5,4,col);
		mysetto(6,9); 
		mymoveto(5,8,col);
		break;
	case '\045':   /* yen */
		mysetto(0,2); 
		mymoveto(3,7,col); 
		mymoveto(6,2,col);
		mysetto(3,7); 
		mymoveto(3,11,col);
		mysetto(0,7); 
		mymoveto(6,7,col);
		mysetto(0,9); 
		mymoveto(6,9,col);
		break;
	case '\046':   /* brokenbar */
		mysetto(3,1); 
		mymoveto(3,5,col);
		mysetto(3,7); 
		mymoveto(3,11,col);
		break;
	case '\047':   /* section */
		mysetto(6,3); 
		mymoveto(5,2,col);
		mymoveto(3,2,col);
		mymoveto(2,3,col);
		mymoveto(2,4,col);
		mymoveto(6,8,col);
		mymoveto(6,9,col);
		mymoveto(5,10,col);
		mymoveto(3,10,col);
		mysetto(4,6); 
		mymoveto(2,6,col);
		mymoveto(1,7,col);
		mymoveto(1,8,col);
		mymoveto(5,12,col);
		mymoveto(5,13,col);
		mymoveto(4,14,col);
		mymoveto(2,14,col);
		mymoveto(1,13,col);
		break;
	case '\051':   /* copyright */
		mysetto(0,4); 
		mymoveto(1,3,col);
		mymoveto(5,3,col);
		mymoveto(6,4,col);
		mymoveto(6,10,col);
		mymoveto(5,11,col);
		mymoveto(1,11,col);
		mymoveto(0,10,col);
		mymoveto(0,4,col);
		mysetto(4,5); 
		mymoveto(3,5,col);
		mymoveto(2,6,col);
		mymoveto(2,8,col);
		mymoveto(3,9,col);
		mymoveto(4,9,col);
		break;
	case '\052':   /* ordfeminine (small underlined a) */
		mysetto(2,2); 
		mymoveto(3,2,col);
		mymoveto(4,3,col); 
		mymoveto(4,6,col); 
		mymoveto(5,7,col);
		mysetto(4,6); 
		mymoveto(3,7,col); 
		mymoveto(2,7,col);
		mymoveto(1,6,col); 
		mymoveto(1,5,col); 
		mymoveto(2,4,col);
		mymoveto(4,4,col);
		mysetto(1,9); 
		mymoveto(5,9,col);
		break;
	case '\053':   /* guillemot left: << */
		mysetto(4,5); 
		mymoveto(1,8,col);
		mymoveto(4,11,col); 
		mysetto(6,5); 
		mymoveto(3,8,col);
		mymoveto(6,11,col); 
		break;
	case '\054':   /* logical not */
		mysetto(1,5); 
		mymoveto(6,5,col);
		mymoveto(6,9,col); 
		break;
	case '\055':   /* hyphen */
		mysetto(1,8); 
		mymoveto(6,8,col);
		break;
	case '\056':   /* registered */
		mysetto(0,4); 
		mymoveto(1,3,col);
		mymoveto(5,3,col);
		mymoveto(6,4,col);
		mymoveto(6,10,col);
		mymoveto(5,11,col);
		mymoveto(1,11,col);
		mymoveto(0,10,col);
		mymoveto(0,4,col);
		mysetto(2,9); 
		mymoveto(2,5,col);
		mymoveto(3,5,col);
		mymoveto(4,6,col);
		mymoveto(3,7,col);
		mymoveto(2,7,col);
		mymoveto(4,9,col);
		break;
	case '\061':   /* +- */
		mysetto(0,6); 
		mymoveto(6,6,col);
		mysetto(3,9); 
		mymoveto(3,3,col);
		mysetto(0,11); 
		mymoveto(6,11,col);
		break;
	case '\062':   /* 2 superior */
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(4,2,col);
		mymoveto(5,3,col); 
		mymoveto(5,4,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col);
		break;
	case '\063':   /* 3 superior */
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(4,2,col);
		mymoveto(5,3,col); 
		mymoveto(5,4,col); 
		mymoveto(4,5,col); 
		mymoveto(3,5,col); 
		mysetto(4,5); 
		mymoveto(5,6,col);
		mymoveto(5,7,col);
		mymoveto(4,8,col);
		mymoveto(3,8,col);
		mymoveto(2,7,col);
		break;
	case '\065':   /* greek mu */
		mysetto(6,5); 
		mymoveto(6,11,col);
		mysetto(6,10); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,5,col);
		mysetto(1,10); 
		mymoveto(1,14,col);
		break;
	case '\066':   /* paragraph: reverted P */
		mysetto(6,11); 
		mymoveto(6,2,col);
		mysetto(4,11); 
		mymoveto(4,2,col);
		mymoveto(2,2,col);
		mymoveto(1,3,col);
		mymoveto(1,4,col);
		mymoveto(2,5,col);
		mymoveto(4,5,col);
		break;
	case '\067':   /* period centered */
		mysetto(4,6); 
		mymoveto(5,6,col);
		mymoveto(5,7,col); 
		mymoveto(4,7,col); 
		mymoveto(4,6,col);
		break;
	case '\071':   /* 1 superior */
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(3,8,col);
		mysetto(2,8); 
		mymoveto(4,8,col);
		break;
	case '\072':   /* ordmasculine (small underlined o) */
		mysetto(2,2); 
		mymoveto(4,2,col);
		mymoveto(5,3,col); 
		mymoveto(5,6,col); 
		mymoveto(4,7,col);
		mymoveto(2,7,col);
		mymoveto(1,6,col);
		mymoveto(1,3,col);
		mymoveto(2,2,col);
		mysetto(1,9); 
		mymoveto(5,9,col);
		break;
	case '\073':   /* guillemot right: >> */
		mysetto(1,5); 
		mymoveto(4,8,col);
		mymoveto(1,11,col); 
		mysetto(3,5); 
		mymoveto(6,8,col);
		mymoveto(3,11,col); 
		break;
	case '\074':   /* 1/4 */
		mysetto(0,3); 
		mymoveto(1,2,col); 
		mymoveto(1,8,col);
		mysetto(0,8); 
		mymoveto(2,8,col);
		mysetto(0,11); 
		mymoveto(6,2,col);
		mysetto(5,13); 
		mymoveto(5,7,col);
		mymoveto(2,11,col);
		mymoveto(6,11,col);
		break;
	case '\075':   /* 1/2 */
		mysetto(0,3); 
		mymoveto(1,2,col); 
		mymoveto(1,8,col);
		mysetto(0,8); 
		mymoveto(2,8,col);
		mysetto(0,11); 
		mymoveto(6,2,col);
		mysetto(3,8); 
		mymoveto(4,7,col); 
		mymoveto(5,7,col);
		mymoveto(6,8,col); 
		mymoveto(6,9,col); 
		mymoveto(3,13,col);
		mymoveto(6,13,col);
		break;
	case '\076':   /* 3/4 */
		mysetto(0,2); 
		mymoveto(1,1,col); 
		mymoveto(2,1,col);
		mymoveto(3,2,col); 
		mymoveto(3,3,col); 
		mymoveto(2,4,col); 
		mymoveto(0,4,col); 
		mymoveto(2,4,col); 
		mymoveto(3,5,col);
		mymoveto(3,6,col);
		mymoveto(2,7,col);
		mymoveto(1,7,col);
		mymoveto(0,6,col);
		mysetto(0,11); 
		mymoveto(6,2,col);
		mysetto(5,13); 
		mymoveto(5,7,col);
		mymoveto(2,11,col);
		mymoveto(6,11,col);
		break;
	case '\077':   /* question down */
		mysetto(6,12); 
		mymoveto(6,13,col); 
		mymoveto(5,14,col);
		mymoveto(2,14,col); 
		mymoveto(1,13,col); 
		mymoveto(1,12,col);
		mymoveto(3,9,col); 
		mymoveto(3,8,col); 
		mysetto(3,4);
		mymoveto(3,4,col);
		break;
	case '\100':   /* A grave */
		mysetto(1,11); 
		mymoveto(1,8,col); 
		mymoveto(3,2,col);
		mymoveto(4,2,col); 
		mymoveto(6,8,col); 
		mymoveto(6,11,col);
		mysetto(1,8); 
		mymoveto(6,8,col);
		mysetto(2,0); 
		mymoveto(4,1,col); 
		break;
	case '\101':   /* A accute */
		mysetto(1,11); 
		mymoveto(1,8,col); 
		mymoveto(3,2,col);
		mymoveto(4,2,col); 
		mymoveto(6,8,col); 
		mymoveto(6,11,col);
		mysetto(1,8); 
		mymoveto(6,8,col);
		mysetto(5,0); 
		mymoveto(3,1,col); 
		break;
	case '\102':   /* A circimflex */
		mysetto(1,11); 
		mymoveto(1,8,col); 
		mymoveto(3,2,col);
		mymoveto(4,2,col); 
		mymoveto(6,8,col); 
		mymoveto(6,11,col);
		mysetto(1,8); 
		mymoveto(6,8,col);
		mysetto(2,1); 
		mymoveto(3,0,col); 
		mymoveto(4,1,col); 
		break;
	case '\103':   /* A tilde */
		mysetto(1,11); 
		mymoveto(1,8,col); 
		mymoveto(3,2,col);
		mymoveto(4,2,col); 
		mymoveto(6,8,col); 
		mymoveto(6,11,col);
		mysetto(1,8); 
		mymoveto(6,8,col);
		mysetto(1,1); 
		mymoveto(2,0,col); 
		mymoveto(4,1,col); 
		mymoveto(5,0,col); 
		break;
	case '\104':   /* A dieresis, german "A */
		mysetto(1,11); 
		mymoveto(1,8,col); 
		mymoveto(3,2,col);
		mymoveto(4,2,col); 
		mymoveto(6,8,col); 
		mymoveto(6,11,col);
		mysetto(1,8); 
		mymoveto(6,8,col);
		mysetto(2,0); 
		mymoveto(2,1,col); 
		mysetto(5,0); 
		mymoveto(5,1,col); 
		break;
	case '\105':   /* A ring */
		mysetto(1,11); 
		mymoveto(1,8,col); 
		mymoveto(3,2,col);
		mymoveto(4,2,col); 
		mymoveto(6,8,col); 
		mymoveto(6,11,col);
		mysetto(1,8); 
		mymoveto(6,8,col);
		mysetto(3,0); 
		mymoveto(4,0,col);
		mymoveto(4,1,col);
		mymoveto(3,1,col);
		mymoveto(3,0,col);
		break;
	case '\106':   /* AE */
		mysetto(1,11); 
		mymoveto(1,8,col); 
		mymoveto(3,2,col);
		mymoveto(4,2,col); 
		mysetto(1,8); 
		mymoveto(4,8,col);
		mysetto(4,2); 
		mymoveto(4,11,col); 
		mymoveto(6,11,col);
		mysetto(4,2); 
		mymoveto(6,2,col);
		mysetto(4,6); 
		mymoveto(5,6,col);
		break;
	case '\107':   /* C cedilla */
		mysetto(6,4); 
		mymoveto(6,3,col); 
		mymoveto(5,2,col);
		mymoveto(2,2,col);
		mymoveto(1,3,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mymoveto(6,9,col);
		mysetto(3,11); 
		mymoveto(3,13,col); 
		mymoveto(4,13,col); 
		mymoveto(5,14,col); 
		mymoveto(4,15,col); 
		mymoveto(3,15,col); 
		break;
	case '\110':   /* E grave */
		mysetto(1,2); 
		mymoveto(1,11,col); 
		mymoveto(6,11,col);
		mysetto(1,2); 
		mymoveto(6,2,col);
		mysetto(1,6); 
		mymoveto(5,6,col);
		mysetto(2,0); 
		mymoveto(4,1,col); 
		break;
	case '\111':   /* E accute */
		mysetto(1,2); 
		mymoveto(1,11,col); 
		mymoveto(6,11,col);
		mysetto(1,2); 
		mymoveto(6,2,col);
		mysetto(1,6); 
		mymoveto(5,6,col);
		mysetto(3,1); 
		mymoveto(5,0,col); 
		break;
	case '\112':   /* E circumflex */
		mysetto(1,2); 
		mymoveto(1,11,col); 
		mymoveto(6,11,col);
		mysetto(1,2); 
		mymoveto(6,2,col);
		mysetto(1,6); 
		mymoveto(5,6,col);
		mysetto(2,1); 
		mymoveto(3,0,col); 
		mymoveto(4,1,col); 
		break;
	case '\113':   /* E dieresis, "E */
		mysetto(1,2); 
		mymoveto(1,11,col); 
		mymoveto(6,11,col);
		mysetto(1,2); 
		mymoveto(6,2,col);
		mysetto(1,6); 
		mymoveto(5,6,col);
		mysetto(2,0); 
		mymoveto(2,0,col); 
		mysetto(5,0); 
		mymoveto(5,0,col); 
		break;
	case '\114':   /* I grave */
		mysetto(1,2); 
		mymoveto(5,2,col);
		mysetto(3,2); 
		mymoveto(3,11,col);
		mysetto(1,11); 
		mymoveto(5,11,col);
		mysetto(2,0); 
		mymoveto(4,1,col); 
		break;
	case '\115':   /* I accute */
		mysetto(1,2); 
		mymoveto(5,2,col);
		mysetto(3,2); 
		mymoveto(3,11,col);
		mysetto(1,11); 
		mymoveto(5,11,col);
		mysetto(2,1); 
		mymoveto(4,0,col); 
		break;
	case '\116':   /* I circumflex */
		mysetto(1,2); 
		mymoveto(5,2,col);
		mysetto(3,2); 
		mymoveto(3,11,col);
		mysetto(1,11); 
		mymoveto(5,11,col);
		mysetto(2,1); 
		mymoveto(3,0,col); 
		mymoveto(4,1,col); 
		break;
	case '\117':   /* I dieresis, "I */
		mysetto(1,2); 
		mymoveto(5,2,col);
		mysetto(3,2); 
		mymoveto(3,11,col);
		mysetto(1,11); 
		mymoveto(5,11,col);
		mysetto(2,0); 
		mymoveto(2,0,col); 
		mysetto(4,0); 
		mymoveto(4,0,col); 
		break;
	case '\120':   /* D dashed, Eth */
		mysetto(1,2); 
		mymoveto(1,11,col); 
		mymoveto(4,11,col);
		mymoveto(6,9,col); 
		mymoveto(6,4,col); 
		mymoveto(4,2,col);
		mymoveto(1,2,col);
		mysetto(0,6); 
		mymoveto(2,6,col);
		break;
	case '\121':   /* N tilde */
		mysetto(1,11); 
		mymoveto(1,2,col); 
		mymoveto(6,11,col);
		mymoveto(6,2,col);
		mysetto(1,1); 
		mymoveto(2,0,col); 
		mymoveto(4,1,col); 
		mymoveto(5,0,col); 
		break;
	case '\122':   /* O grave */
		mysetto(1,10); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mysetto(2,0); 
		mymoveto(4,1,col); 
		break;
	case '\123':   /* O accute */
		mysetto(1,10); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mysetto(3,1); 
		mymoveto(5,0,col); 
		break;
	case '\124':   /* O circumflex */
		mysetto(1,10); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mysetto(2,1); 
		mymoveto(3,0,col); 
		mymoveto(4,1,col); 
		break;
	case '\125':   /* O tilde */
		mysetto(1,10); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mysetto(1,1); 
		mymoveto(2,0,col); 
		mymoveto(4,1,col); 
		mymoveto(5,0,col); 
		break;
	case '\126':   /* O dieresis, german "O */
		mysetto(1,10); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mysetto(2,0); 
		mymoveto(2,1,col); 
		mysetto(5,0); 
		mymoveto(5,1,col); 
		break;
	case '\127':   /* times, multiply */
		mysetto(1,4); 
		mymoveto(6,10,col);
		mysetto(6,4); 
		mymoveto(1,10,col);
		break;
	case '\130':   /* O slash, average  */
		mysetto(1,10); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(5,2,col);
		mymoveto(6,3,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col);
		mysetto(0,12); 
		mymoveto(6,1,col); 
		break;
	case '\131':   /* U grave */
		mysetto(1,2); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mymoveto(6,2,col);
		mysetto(2,0); 
		mymoveto(4,1,col); 
		break;
	case '\132':   /* U accute */
		mysetto(1,2); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mymoveto(6,2,col);
		mysetto(3,1); 
		mymoveto(5,0,col); 
		break;
	case '\133':   /* U circumflex */
		mysetto(1,2); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mymoveto(6,2,col);
		mysetto(2,1); 
		mymoveto(3,0,col); 
		mymoveto(4,1,col); 
		break;
	case '\134':   /* U dieresis, german "U */
		mysetto(1,2); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col); 
		mymoveto(6,2,col);
		mysetto(2,0); 
		mymoveto(2,1,col); 
		mysetto(5,0); 
		mymoveto(5,1,col); 
		break;
	case '\135':   /* Y accute */
		mysetto(0,2); 
		mymoveto(3,7,col); 
		mymoveto(6,2,col);
		mysetto(3,7); 
		mymoveto(3,11,col);
		mysetto(2,1); 
		mymoveto(4,0,col); 
		break;
	case '\136':   /* Thorn */
		mysetto(1,7); 
		mymoveto(5,7,col);
		mymoveto(6,8,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(1,11,col); 
		mymoveto(1,5,col);
		mymoveto(1,15,col);
		break;
	case '\137':   /* german dbls, sharp sz */
		mysetto(1,12); 
		mymoveto(1,3,col);
		mymoveto(2,2,col); 
		mymoveto(4,2,col);
		mymoveto(5,3,col); 
		mymoveto(5,4,col); 
		mymoveto(3,6,col);
		mymoveto(2,6,col);
		mymoveto(6,7,col); 
		mymoveto(6,10,col);
		mymoveto(5,11,col); 
		mymoveto(2,11,col); 
		break;
	case '\140':   /* a grave */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(4,5,col);
		mymoveto(5,6,col); 
		mymoveto(5,10,col); 
		mymoveto(6,11,col);
		mysetto(5,10); 
		mymoveto(4,11,col); 
		mymoveto(2,11,col);
		mymoveto(1,10,col); 
		mymoveto(1,9,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col);
		mysetto(2,2); 
		mymoveto(4,3,col); 
		break;
	case '\141':   /* a accute */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(4,5,col);
		mymoveto(5,6,col); 
		mymoveto(5,10,col); 
		mymoveto(6,11,col);
		mysetto(5,10); 
		mymoveto(4,11,col); 
		mymoveto(2,11,col);
		mymoveto(1,10,col); 
		mymoveto(1,9,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col);
		mysetto(3,3); 
		mymoveto(5,2,col); 
		break;
	case '\142':   /* a circumflex */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(4,5,col);
		mymoveto(5,6,col); 
		mymoveto(5,10,col); 
		mymoveto(6,11,col);
		mysetto(5,10); 
		mymoveto(4,11,col); 
		mymoveto(2,11,col);
		mymoveto(1,10,col); 
		mymoveto(1,9,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col);
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(4,3,col); 
		break;
	case '\143':   /* a tilde */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(4,5,col);
		mymoveto(5,6,col); 
		mymoveto(5,10,col); 
		mymoveto(6,11,col);
		mysetto(5,10); 
		mymoveto(4,11,col); 
		mymoveto(2,11,col);
		mymoveto(1,10,col); 
		mymoveto(1,9,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col);
		mysetto(1,3); 
		mymoveto(2,2,col); 
		mymoveto(4,3,col); 
		mymoveto(5,2,col); 
		break;
	case '\144':   /* a dieresis, german "a */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(4,5,col);
		mymoveto(5,6,col); 
		mymoveto(5,10,col); 
		mymoveto(6,11,col);
		mysetto(5,10); 
		mymoveto(4,11,col); 
		mymoveto(2,11,col);
		mymoveto(1,10,col); 
		mymoveto(1,9,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col);
		mysetto(2,2); 
		mymoveto(2,3,col); 
		mysetto(5,2); 
		mymoveto(5,3,col); 
		break;
	case '\145':   /* a ring */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(4,5,col);
		mymoveto(5,6,col); 
		mymoveto(5,10,col); 
		mymoveto(6,11,col);
		mysetto(5,10); 
		mymoveto(4,11,col); 
		mymoveto(2,11,col);
		mymoveto(1,10,col); 
		mymoveto(1,9,col); 
		mymoveto(2,8,col);
		mymoveto(5,8,col);
		mysetto(2,1); 
		mymoveto(3,1,col); 
		mymoveto(4,2,col); 
		mymoveto(4,3,col); 
		mymoveto(3,4,col); 
		mymoveto(2,4,col); 
		mymoveto(1,3,col); 
		mymoveto(1,2,col); 
		mymoveto(2,1,col); 
		break;
	case '\146':   /* ae */
		mysetto(0,6); 
		mymoveto(1,5,col); 
		mymoveto(2,5,col);
		mymoveto(3,6,col); 
		mymoveto(3,10,col); 
		mymoveto(2,11,col); 
		mymoveto(1,11,col);
		mymoveto(0,10,col); 
		mymoveto(0,9,col); 
		mymoveto(1,8,col);
		mymoveto(3,8,col);
		mysetto(3,8); 
		mymoveto(6,8,col); 
		mymoveto(6,6,col);
		mymoveto(5,5,col); 
		mymoveto(4,5,col);
		mymoveto(3,6,col); 
		mymoveto(3,10,col); 
		mymoveto(4,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		break;
	case '\147':   /* c, cedilla */
		mysetto(6,6); 
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		mysetto(3,11); 
		mymoveto(3,13,col); 
		mymoveto(4,13,col); 
		mymoveto(5,14,col); 
		mymoveto(4,15,col); 
		mymoveto(3,15,col); 
		break;
	case '\150':   /* e grave */
		mysetto(1,8); 
		mymoveto(6,8,col); 
		mymoveto(6,6,col);
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		mysetto(2,2); 
		mymoveto(4,3,col); 
		break;
	case '\151':   /* e accute */
		mysetto(1,8); 
		mymoveto(6,8,col); 
		mymoveto(6,6,col);
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		mysetto(3,3); 
		mymoveto(5,2,col); 
		break;
	case '\152':   /* e circumflex */
		mysetto(1,8); 
		mymoveto(6,8,col); 
		mymoveto(6,6,col);
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(4,3,col); 
		break;
	case '\153':   /* e dieresis, "e */
		mysetto(1,8); 
		mymoveto(6,8,col); 
		mymoveto(6,6,col);
		mymoveto(5,5,col); 
		mymoveto(2,5,col);
		mymoveto(1,6,col); 
		mymoveto(1,10,col); 
		mymoveto(2,11,col);
		mymoveto(5,11,col); 
		mymoveto(6,10,col);
		mysetto(2,2); 
		mymoveto(2,3,col); 
		mysetto(5,2); 
		mymoveto(5,3,col); 
		break;
	case '\154':   /* i grave */
		mysetto(2,5); 
		mymoveto(4,5,col); 
		mymoveto(4,11,col);
		mysetto(3,2); 
		mymoveto(5,3,col); 
		break;
	case '\155':   /* i accute */
		mysetto(2,5); 
		mymoveto(4,5,col); 
		mymoveto(4,11,col);
		mysetto(3,3); 
		mymoveto(5,2,col); 
		break;
	case '\156':   /* i circumflex */
		mysetto(2,5); 
		mymoveto(4,5,col); 
		mymoveto(4,11,col);
		mysetto(3,3); 
		mymoveto(4,2,col); 
		mymoveto(5,3,col); 
		break;
	case '\157':   /* i dieresis, "i */
		mysetto(2,5); 
		mymoveto(4,5,col); 
		mymoveto(4,11,col);
		mysetto(3,2); 
		mymoveto(3,3,col); 
		mysetto(5,2); 
		mymoveto(5,3,col); 
		break;
	case '\160':   /* eth */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		mysetto(5,5);
		mymoveto(1,2,col);
		mysetto(1,4);
		mymoveto(3,2,col);
		break;
	case '\161':   /* n tilde */
		mysetto(1,11); 
		mymoveto(1,5,col);
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,11,col);
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(5,3,col); 
		mymoveto(6,2,col); 
		break;
	case '\162':   /* o grave */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		mysetto(2,2); 
		mymoveto(4,3,col); 
		break;
	case '\163':   /* o accute */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		mysetto(3,3); 
		mymoveto(5,2,col); 
		break;
	case '\164':   /* o circumflex */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(4,3,col); 
		break;
	case '\165':   /* o tilde */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(5,3,col); 
		mymoveto(6,2,col); 
		break;
	case '\166':   /* o dieresis, german "o */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		mysetto(2,2); 
		mymoveto(2,3,col); 
		mysetto(5,2); 
		mymoveto(5,3,col); 
		break;
	case '\167':   /* divide */
		mysetto(1,7); 
		mymoveto(5,7,col);
		mysetto(3,5); 
		mymoveto(3,5,col);
		mysetto(3,9); 
		mymoveto(3,9,col);
		break;
	case '\170':   /* o slash */
		mysetto(1,6); 
		mymoveto(2,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,6,col);
		mysetto(0,12); 
		mymoveto(6,4,col); 
		break;
	case '\171':   /* u grave */
		mysetto(6,5); 
		mymoveto(6,11,col);
		mysetto(6,10); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,5,col);
		mysetto(2,2); 
		mymoveto(4,3,col); 
		break;
	case '\172':   /* u accute */
		mysetto(6,5); 
		mymoveto(6,11,col);
		mysetto(6,10); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,5,col);
		mysetto(3,3); 
		mymoveto(5,2,col); 
		break;
	case '\173':   /* u circumflex */
		mysetto(6,5); 
		mymoveto(6,11,col);
		mysetto(6,10); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,5,col);
		mysetto(2,3); 
		mymoveto(3,2,col); 
		mymoveto(4,3,col); 
		break;
	case '\174':   /* u dieresis, german "u */
		mysetto(6,5); 
		mymoveto(6,11,col);
		mysetto(6,10); 
		mymoveto(5,11,col);
		mymoveto(2,11,col); 
		mymoveto(1,10,col); 
		mymoveto(1,5,col);
		mysetto(2,2); 
		mymoveto(2,3,col); 
		mysetto(5,2); 
		mymoveto(5,3,col); 
		break;
	case '\175':   /* y accute */
		mysetto(1,5); 
		mymoveto(4,12,col);
		mysetto(6,5); 
		mymoveto(4,12,col); 
		mymoveto(2,15,col);
		mymoveto(1,14,col);
		mysetto(2,3); 
		mymoveto(4,2,col); 
		break;
	case '\176':   /* thorn */
		mysetto(1,2); 
		mymoveto(1,5,col); 
		mymoveto(5,5,col);
		mymoveto(6,6,col); 
		mymoveto(6,10,col); 
		mymoveto(5,11,col);
		mymoveto(1,11,col); 
		mymoveto(1,5,col);
		mymoveto(1,15,col);
		break;
	case '\177':   /* y dieresis, "y */
		mysetto(1,5); 
		mymoveto(4,12,col);
		mysetto(6,5); 
		mymoveto(4,12,col); 
		mymoveto(2,15,col);
		mymoveto(1,14,col);
		mysetto(2,2); 
		mymoveto(2,3,col); 
		mysetto(5,2); 
		mymoveto(5,3,col); 
		break;
	}
}



/*--------------------------------------------------------------------*/

#endif /* DRAWCHR_H */

