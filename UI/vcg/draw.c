/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */
/*		--------------------------------------		      */
/*								      */
/*   file:	   draw.c					      */
/*   version:	   1.00.00					      */
/*   creation:	   4.4.93					      */
/*   author:	   I. Lemke  (...-Version 0.99.99)		      */
/*		   G. Sander (Version 1.00.00-...)		      */
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */
/*   description:  Draw nodes and edges 			      */
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
 * Revision 3.8  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.7  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 *
 * Revision 3.6  1994/11/23  14:50:47  sander
 * Minor cosmetic change.
 *
 * Revision 3.5  1994/08/02  15:36:12  sander
 * Minor cosmetic change.
 *
 * Revision 3.4  1994/05/17  16:39:10  sander
 * attribute node_align added to allow nodes to be centered in the levels.
 *
 * Revision 3.3  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
 *
 * Revision 3.2  1994/03/03  15:35:39  sander
 * Edge line style `invisible' added.
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
 * Revision 2.1  1993/12/08  21:20:09  sander
 * Reasonable fast and stable version
 *
 */

/************************************************************************
 * The situation here is the following:
 * -----------------------------------
 * Now, after layouting, we draw all nodes and edges. This is used to 
 * refresh the canvas. It is device independent.
 *
 * We have:
 *    1)  All visible nodes are in nodelist, labellist and dummylist.
 *    2)  All pot. visible edges are in the lists edgelist or tmpedgelist,
 *	  Visible edges can be detected by the EINVISIBLE flag (==0) in these
 *	  lists. Note: invisible edges may also be in edgelist or tmpedgelist.
 *	  An edge is visible iff
 *		   a) it is used in the adjacency lists.
 *		or b) it is a direct neigbour edge in NCONNECT(v) for
 *		      some node v.
 *    3)  The adjacency lists NPRED(v) and the connection fields
 *	  NCONNECT(v) contain all visible nodes. 
 *    4)  Reverted edges are marked with EART(e)='R'
 *	  Self loops don't anymore exist.
 *    5)  All nodes have filled NX, NY, NWIDTH and NHEIGHT.
 *	  NX and NY are absolutely. 
 *	  NWIDTH and NHEIGHT are stretched or shrinked according to the 
 *	  local factors at the nodes.
 *    6)  For all visible edges, ESTARTX(e), ESTARTY(e), EENDX(e),
 *	  EENDY(e), ETBENDX(e), ETBENDY(e), EBBENDX(e), EBBENDY(e) are filled.
 *    7)  EORI(e) is filled. If the edge has two arrows, EORI2(e)
 *	  is filled, too.
 * 
 * The algorithm used here is rather simple: We visit all visible nodes
 * and edges and call the corresponding function from the drawlib to
 * draw them. See drawlib.c for details.
 *
 * This file provides the following functions:
 * ------------------------------------------
 * draw_main	       Main routine to draw nodes and edges
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "main.h"
#include "options.h"
#include "alloc.h"
#include "drawstr.h"
#include "drawlib.h"
#include "draw.h"
#include "timing.h"


/*  Prototypes
 *  ---------- 
 */

static	void	draw_nodes	_PP((void));
static	void	draw_edges	_PP((void));

/*--------------------------------------------------------------------*/
/*   Draw main routine						      */
/*--------------------------------------------------------------------*/

#ifdef ANSI_C
void	draw_main(void)
#else
void	draw_main()
#endif
{
	debugmessage("draw_main","");

	start_time();
	draw_nodes();
	stop_time("draw_main draw_nodes");
	start_time();
	draw_edges();
	stop_time("draw_main draw_edges");
}


/* Draw all nodes
 * ==============
 * We assume that nodes have already a filled NX, NY, NSTRETCH, NSHRINK,
 * NLABEL, NTEXTMODE, NBORDERW, NCOLOR, NTCOLOR, NBCOLOR, NHEIGHT, NWIDTH.
 */
 
#ifdef ANSI_C
static	void	draw_nodes(void)
#else
static	void	draw_nodes()
#endif
{
	GNODE v;

	debugmessage("draw_nodes","");
	
	if (supress_nodes) return;
	v = nodelist;
	while (v) {
		if (NWIDTH(v)==0) { v = NNEXT(v); continue; }
		gs_setshrink(G_stretch * NSTRETCH(v),
			     G_shrink  * NSHRINK(v)  );
		gs_setto(NX(v) * G_stretch / G_shrink,
			 NY(v) * G_stretch / G_shrink );

		draw_one_node(v);
		v = NNEXT(v);
	}
	
	v = labellist;
	while (v) {
		if (NWIDTH(v)==0) { v = NNEXT(v); continue; }
		gs_setshrink(G_stretch * NSTRETCH(v),
			     G_shrink  * NSHRINK(v)  );
		gs_setto(NX(v) * G_stretch / G_shrink,
			 NY(v) * G_stretch / G_shrink );
		gs_stringbox(v);
		v = NNEXT(v);
	}
#undef DEBUGDUMMY
#ifdef DEBUGDUMMY
	v = dummylist;
	while (v) {
		if ((NWIDTH(v)==0)&&(NHEIGHT(v)==0)) { 
			NWIDTH(v) = NHEIGHT(v) = 9; 
			NBORDERW(v) = 5; 
			NCOLOR(v) = BLACK; 
			gs_setshrink(G_stretch, G_shrink);
			gs_setto(NX(v) * G_stretch / G_shrink,
				 NY(v) * G_stretch / G_shrink );

			draw_one_node(v);
			NWIDTH(v) = NHEIGHT(v) = 0; 
			NBORDERW(v) = 0; 
		}
		v = NNEXT(v);
	}
	
#endif
	
	/*  Normal dummy nodes need not to be drawn, because they have no size. 
	 *  Anchor nodes are drawn.
	 */
	v = dummylist;
	while (v) {
		if (NANCHORNODE(v)) gs_anchornode(v);
		v = NNEXT(v);
	}
}

/* Draw one node
 * -------------
 * at actual postion with actual scaling.
 */

#ifdef ANSI_C
void draw_one_node(GNODE v)
#else
void draw_one_node(v)
GNODE v;
#endif
{
	switch (NSHAPE(v)) {
	case RHOMB:
		switch (NREVERT(v)) {		
		case NOREVERT: gs_rhomb(v);           break;
		case AREVERT:  gs_revertrhomb(v);     break;
		case BREVERT:  gs_halfrevertrhomb(v); break;
		}
		break;
	case TRIANGLE:
		switch (NREVERT(v)) {		
		case NOREVERT: gs_triangle(v);           break;
		case AREVERT:  gs_reverttriangle(v);     break;
		case BREVERT:  gs_halfreverttriangle(v); break;
		}
		break;
	case ELLIPSE:
		switch (NREVERT(v)) {		
		case NOREVERT: gs_ellipse(v);           break;
		case AREVERT:  gs_revertellipse(v);     break;
		case BREVERT:  gs_halfrevertellipse(v); break;
		}
		break;
	default:
		switch (NREVERT(v)) {		
		case NOREVERT: gs_textbox(v);           break;
		case AREVERT:  gs_reverttextbox(v);     break;
		case BREVERT:  gs_halfreverttextbox(v); break;
		}
	}
}


/*  Draw all edges
 *  ==============
 *  We assume that edges have already a filled ESTARTX, ESTARTY, ETBENDX
 *  ETBENDY, EBBENDX, EBBENDYm EENDX, EENDY, EORI, EORI2 (if EART='D'), 
 *  ELSTYLE, ETHICKNESS, 
 *  ECOLOR, EARROWSIZE and EART.
 *  Note that these edges should not have ELABEL entries, because 
 *  edge labels should be converted before into label nodes.
 *
 *  We traverse all nodes and draw all edges pointing to these nodes.
 *  This seems to be faster than to inspect edgelist and tmpedgelist.
 *  This allows to correct EARROWSIZE at dummy nodes and label nodes.
 */

#define backward_connection1(c) ((CEDGE(c))&& (EEND(CEDGE(c)) ==v))
#define backward_connection2(c) ((CEDGE2(c))&&(EEND(CEDGE2(c))==v))

#ifdef ANSI_C
static	void	draw_edges(void)
#else
static	void	draw_edges()
#endif
{
	GNODE	v;
	GEDGE	e;
	ADJEDGE li;
	CONNECT c;
	
	debugmessage("draw_edges","");

	if (supress_edges) return;
	v = nodelist;
	while (v) {
		c = NCONNECT(v);
		if (c) {
			if (backward_connection1(c)) {
				e = CEDGE(c);
				switch (ELSTYLE(e)) {
				case SOLID:  gs_solidarrow(e);
					     break;
				case DASHED: gs_dashedarrow(e);
					     break;
				case DOTTED: gs_dottedarrow(e);
					     break;
				case UNVISIBLE: break;
				}
			}
			if (backward_connection2(c)) {
				e = CEDGE2(c);
				switch (ELSTYLE(e)) {
				case SOLID:  gs_solidarrow(e);
					     break;
				case DASHED: gs_dashedarrow(e);
					     break;
				case DOTTED: gs_dottedarrow(e);
					     break;
				case UNVISIBLE: break;
				}
			}
		}
		li = NPRED(v);
		while (li) {
			e  = AKANTE(li);
			switch (ELSTYLE(e)) {
			case SOLID:  gs_solidarrow(e);
				     break;
			case DASHED: gs_dashedarrow(e);
				     break;
			case DOTTED: gs_dottedarrow(e);
				     break;
			case UNVISIBLE: break;
			}
			li = ANEXT(li);
		}
		v = NNEXT(v);
	}
	
	v = labellist;
	while (v) {
		c = NCONNECT(v);
		if (c) {
			if (backward_connection1(c)) {
				e = CEDGE(c);
				switch (ELSTYLE(e)) {
				case SOLID:  gs_solidarrow(e);
					     break;
				case DASHED: gs_dashedarrow(e);
					     break;
				case DOTTED: gs_dottedarrow(e);
					     break;
				case UNVISIBLE: break;
				}
			}
			if (backward_connection2(c)) {
				e = CEDGE2(c);
				switch (ELSTYLE(e)) {
				case SOLID:  gs_solidarrow(e);
					     break;
				case DASHED: gs_dashedarrow(e);
					     break;
				case DOTTED: gs_dottedarrow(e);
					     break;
				case UNVISIBLE: break;
				}
			}
		}
		li = NPRED(v);
		while (li) {
			e  = AKANTE(li);
			switch (ELSTYLE(e)) {
			case SOLID:  gs_solidarrow(e);
				     break;
			case DASHED: gs_dashedarrow(e);
				     break;
			case DOTTED: gs_dottedarrow(e);
				     break;
			case UNVISIBLE: break;
			}
			li = ANEXT(li);
		}
		v = NNEXT(v);
	}

	v = dummylist;
	while (v) {
		c = NCONNECT(v);
		if (c) {
			if (backward_connection1(c)) {
				e = CEDGE(c);
				switch (ELSTYLE(e)) {
				case SOLID:  gs_solidarrow(e);
					     break;
				case DASHED: gs_dashedarrow(e);
					     break;
				case DOTTED: gs_dottedarrow(e);
					     break;
				case UNVISIBLE: break;
				}
			}
			if (backward_connection2(c)) {
				e = CEDGE(c);
				switch (ELSTYLE(e)) {
				case SOLID:  gs_solidarrow(e);
					     break;
				case DASHED: gs_dashedarrow(e);
					     break;
				case DOTTED: gs_dottedarrow(e);
					     break;
				case UNVISIBLE: break;
				}
			}
		}
		li = NPRED(v);
		while (li) {
			e  = AKANTE(li);
			switch (ELSTYLE(e)) {
			case SOLID:  gs_solidarrow(e);
				     break;
			case DASHED: gs_dashedarrow(e);
				     break;
			case DOTTED: gs_dottedarrow(e);
				     break;
			case UNVISIBLE: break;
			}
			li = ANEXT(li);
		}
		v = NNEXT(v);
	}

}



