/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */
/*		--------------------------------------		      */
/*								      */
/*   file:	   prepare.c					      */
/*   version:	   1.00.00					      */
/*   creation:	   14.4.93					      */
/*   author:	   I. Lemke  (...-Version 0.99.99)		      */
/*		   G. Sander (Version 1.00.00-...)		      */  
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */
/*   description:  Preparation on fixed layout               	      */
/*		   of nodes					      */
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
 * Revision 3.6  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.5  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 *
 * Revision 3.4  1994/08/02  15:36:12  sander
 * Serious bug solved: maxindeg and maxoutdeg were accidently exchanged.
 * This yields to memory allocation mistaces. Solved now.
 *
 * Revision 3.3  1994/05/17  16:39:10  sander
 * attribute node_align added to allow nodes to be centered in the levels.
 *
 * Revision 3.2  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
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
 *
 */ 


/************************************************************************
 * The situation here is the following:
 * -----------------------------------
 * We are not in the layout phase, because all nodes have locations.
 * However, not all nodes have correct width and height, and the edges
 * have no position nor orientation.
 * This is calculated here.
 *
 * We have:
 *  1) We have proper adjacency lists.
 *  2) All visible nodes are in the nodelist or in the labellist.
 *     More exactly: all visible nodes originated directly by the
 *     specification are in the nodelist, and all visible edge label nodes 
 *     are in the labellist.
 *  3) All nodes from the nodelist that are invisible because of
 *     edge class hiding are in the list invis_nodes.
 *  4) All potentially visible edges are in the lists edgelist or tmpedgelist.
 *     Visible edges can be detected by the EINVISIBLE flag (==0) in these 
 *     lists. Note: invisible edges may also be in edgelist or tmpedgelist.
 *     Except the INVISIBLE flag, the edgelist IS NEVER CHANGED !!!
 *  5) An edge is visible iff it is used in the adjacency lists. For some
 *     edges, we create substeds; then, the substed is visible but the original
 *     edge is not visible.
 *  6) The locFlag is 1, if all visible nodes have positions (x,y).
 *
 * We calculate NINDEG and NOUTDEG of every node, reset width and height
 * of nodes, sort outgoing edges according to the gradient, calculate
 * ports and anchorpoints, and assign co-ordinates to the edges.
 *
 * After that, the following invariants hold:
 *    1)  NINDEG, NOUTDEG, NWIDTH and NHEIGHT are proper filled for nodes. 
 *        maxindeg and maxoutdeg are upper estimations of NINDEG and
 *	  NOUTDEG of nodes.
 *    2)  The adjacency lists NPRED(v) and the connection fields
 *        NCONNECT(v) contain all visible nodes.
 *    3)  Reverted edges are marked with EART(e)='R'.
 *	  Edges to the left are marked with EART(e)='l', and to
 * 	  the right with EART(e)='r'.
 *        Self loops or double edges don't anymore exist.
 *    4)  All nodes have filled NX, NY, NWIDTH and NHEIGHT.
 *        NX and NY are absolutely.
 *        NWIDTH and NHEIGHT are stretched or shrinked according to the
 *        local factors at the nodes.
 *    5)  For all visible edges, ESTARTX(e), ESTARTY(e), EENDX(e),
 *        EENDY(e), ETBENDX(e), ETBENDY(e), EBBENDX(e), EBBENDY(e)are filled.
 *    6)  EORI(e) is filled. If the edge has two arrows, EORI2(e)
 *        is filled, too.
 *
 * This file provides the following functions:
 * ------------------------------------------
 * prepare_nodes	Preparation of all nodes
 *
 ************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "alloc.h"
#include "main.h"
#include "options.h"
#include "folding.h"
#include "drawlib.h"
#include "steps.h"
#include "timing.h"

/* Prototypes
 * ----------
 */

static void 	calc_node_degree	_PP((GNODE v));
static void 	calc_node_anchor	_PP((GNODE v));
static void 	sort_adjacencies	_PP((GNODE v));
static int 	compare_ppos		_PP((const GEDGE *a,const GEDGE *b));
static int 	compare_spos		_PP((const GEDGE *a,const GEDGE *b));


/* Global variables
 * ----------------
 */

/* arrays where we can temporary sort the adjacency lists.
 */

static GEDGE    *adjarray2 = NULL;
static int      size_of_adjarray = 0;

/*--------------------------------------------------------------------*/
/*  Preparation of nodes         			      */
/*--------------------------------------------------------------------*/

#ifdef ANSI_C
void	prepare_nodes(void)
#else
void	prepare_nodes()
#endif
{
	int i,h,hh, xp, na;
	GNODE v,w;
	ADJEDGE a;

	start_time();
	debugmessage("prepare_nodes","");

	/* First, calculate width and height of each node */
	maxindeg = maxoutdeg = 0;
	v = nodelist;
	while (v) { calc_node_size(v); v = NNEXT(v); }
	v = labellist;
	while (v) { calc_node_size(v); v = NNEXT(v); }

        /* prepare back edges, i.e. revert all back edges initially
         */

        prepare_back_edges();

	/*  insert anchor edges: we blindly set the anchor node 
	 *  G_xspace pixels right or left of the anchored node.
	 */

	insert_anchor_edges();
	v = dummylist;
	while (v) { 
		if (NANCHORNODE(v)) {
			assert((NCONNECT(v)));
			w = CTARGET(NCONNECT(v));
			NY(v) = NY(w);
			NHEIGHT(v) = NHEIGHT(w);
			xp = 0;
			na = 0;
			a = NPRED(v);
			h = 0;
			while (a) { 
				h++;
				na++;
				xp += NX(SOURCE(a));
				a=ANEXT(a); 
			}
			a = NSUCC(v);
			hh = 0;
			while (a) { 
				hh++; 
				na++;
				xp += NX(TARGET(a));
				a=ANEXT(a); 
			}
			if (hh>h) h = hh;
			NWIDTH(v) =  (h+1)*G_xspace;	
			if (xp/na >= NX(w)+NWIDTH(w))
				NX(v) = NX(w)+NWIDTH(w)+G_xspace;
			else
				NX(v) = NX(w)-NWIDTH(v)-G_xspace;
		}
		v = NNEXT(v); 
	}

	/* Now, check the anchor points */
	maxindeg = maxoutdeg = 0;
	v = nodelist;
	while (v) { calc_node_anchor(v); v = NNEXT(v); }
	v = labellist;
	while (v) { calc_node_anchor(v); v = NNEXT(v); }
	v = dummylist;
	while (v) { calc_node_anchor(v); v = NNEXT(v); }

	/* Then, calculate the node in/outdegree */
	v = nodelist;
	while (v) { calc_node_degree(v); v = NNEXT(v); }
	v = labellist;
	while (v) { calc_node_degree(v); v = NNEXT(v); }
	v = dummylist;
	while (v) { calc_node_degree(v); v = NNEXT(v); }

        i = (maxindeg > maxoutdeg ? maxindeg : maxoutdeg);
        if (i+2 > size_of_adjarray) {
                if (adjarray2) free(adjarray2);
                adjarray2 = (GEDGE *)malloc((i+2)*sizeof(GEDGE));
                if (!adjarray2) Fatal_error("memory exhausted","");
                size_of_adjarray = i+2;
#ifdef DEBUG
                PRINTF("Sizeof table `adjarray2': %ld Bytes\n",
                        (i+2)*sizeof(GEDGE));
#endif
        }


	/* Now, sort the adjacency lists */
	v = nodelist;
	while (v) { sort_adjacencies(v); v = NNEXT(v); }
	v = labellist;
	while (v) { sort_adjacencies(v); v = NNEXT(v); }
	v = dummylist;
	while (v) { sort_adjacencies(v); v = NNEXT(v); }

	/* Now, calculate the node ports */
	v = nodelist;
	while (v) { calc_node_ports(v,1); v = NNEXT(v); }
	v = labellist;
	while (v) { calc_node_ports(v,1); v = NNEXT(v); }
	v = dummylist;
	while (v) { calc_node_ports(v,1); v = NNEXT(v); }

	/* Now, calculate the node ports */
	v = nodelist;
	while (v) { calc_edge_xy(v); v = NNEXT(v); }
	v = labellist;
	while (v) { calc_edge_xy(v); v = NNEXT(v); }
	v = dummylist;
	while (v) { calc_edge_xy(v); v = NNEXT(v); }

	/* Now, calculate the arrow orientation */
	v = nodelist;
	while (v) { calc_edgearrow(v); v = NNEXT(v); }
	v = dummylist;
	while (v) { calc_edgearrow(v); v = NNEXT(v); }
	/* Labels and dummy nodes have no arrows */

	/* calculate maximal x-y position
	 */
	calc_max_xy_pos();

	stop_time("prepare_nodes");
	debugmessage("end of prepare_nodes","");
}



/* Set NINDEG and NOUTDEG of the node
 * ----------------------------------
 */

#ifdef ANSI_C
static void calc_node_degree(GNODE v)
#else
static void calc_node_degree(v)
GNODE v;
#endif
{
	int k;
	ADJEDGE a;

	debugmessage("calc_node_degree","");
	a = NPRED(v);
	k = 0;
	while (a) { k++; a = ANEXT(a); }
	NINDEG(v) = k;
	if (k>maxoutdeg) maxoutdeg = k;
	a = NSUCC(v);
	k = 0;
	while (a) { k++; a = ANEXT(a); }
	NOUTDEG(v) = k;
	if (k>maxindeg) maxindeg = k;
}


/* Set NWIDTH and NHEIGHT of the node
 * ----------------------------------
 * If NWIDTH and NHEIGHT are not already set, they are derived from
 * the size of the label. 
 */

#ifdef ANSI_C
void calc_node_size(GNODE v)
#else
void calc_node_size(v)
GNODE v;
#endif
{
	debugmessage("calc_node_size","");

	if ((NWIDTH(v) == -1)||(NHEIGHT(v) == -1)) {
		gs_setshrink(NSTRETCH(v),NSHRINK(v));
		switch (NSHAPE(v)) {
		case RHOMB:    gs_calcrhombsize(v);    break;
		case TRIANGLE: gs_calctrianglesize(v); break;
		case ELLIPSE:  gs_calcellipsesize(v);  break;
		default:    gs_calctextboxsize(v);
		}
	}
	if (NWIDTH(v)  == -1) NWIDTH(v)   = gs_boxw;
	if (NHEIGHT(v) == -1) NHEIGHT(v)  = gs_boxh;
}


/* Calculate anchor point            
 * ----------------------
 * Further are edges reverted, if they are upwards, and the gradient
 * is calculated.
 * EWEIGHTP(e) is used to contain the gradient of incoming edges,
 * and EWEIGHTS(e) is used to contain the gradient of outgoing
 * edges. Note that edges with anchor left-on have gradient MININT, and 
 * edges with anchor right-on have gradient MAXINT.
 */

#ifdef ANSI_C
static void calc_node_anchor(GNODE v)
#else
static void calc_node_anchor(v)
GNODE v;
#endif
{
	ADJEDGE a,b;
	int 	x1,y1,x2,y2;

	debugmessage("calc_node_anchor","");
	a = NSUCC(v);
	while (a) {
		b = ANEXT(a);
		x1 = NX(SOURCE(a))+NWIDTH( SOURCE(a))/2;
		y1 = NY(SOURCE(a))+NHEIGHT(SOURCE(a))/2;
		x2 = NX(TARGET(a))+NWIDTH( TARGET(a))/2;
		y2 = NY(TARGET(a))+NHEIGHT(TARGET(a))/2;
		if (NY(SOURCE(a))>NY(TARGET(a))+NHEIGHT(TARGET(a))) {
			(void)revert_edge(AKANTE(a));
			if (y1==y2) EWEIGHTS(AKANTE(a)) = MININT;
			else        EWEIGHTS(AKANTE(a)) = 1000*(x1-x2)/(y1-y2); 
			if (y1==y2) EWEIGHTP(AKANTE(a)) = MAXINT;
			else        EWEIGHTP(AKANTE(a)) = -1000*(x1-x2)/(y1-y2); 
		}
		else if (NY(SOURCE(a))+NHEIGHT(SOURCE(a))<NY(TARGET(a))) {
			if (y1==y2) EWEIGHTS(AKANTE(a)) = MININT;
			else        EWEIGHTS(AKANTE(a)) = 1000*(x1-x2)/(y1-y2);
			if (y1==y2) EWEIGHTP(AKANTE(a)) = MAXINT;
			else        EWEIGHTP(AKANTE(a)) = -1000*(x1-x2)/(y1-y2); 
		}
		else if (NX(SOURCE(a))>NX(TARGET(a))+NWIDTH(TARGET(a))) {
			EWEIGHTS(AKANTE(a)) = MININT;
			EWEIGHTP(AKANTE(a)) = MAXINT;
			EART(AKANTE(a)) = 'l';
		}
		else if (NX(SOURCE(a))+NWIDTH(SOURCE(a))<NX(TARGET(a))) {
			EWEIGHTS(AKANTE(a)) = MAXINT;
			EWEIGHTP(AKANTE(a)) = MININT;
			EART(AKANTE(a)) = 'r';
		}
		else delete_adjedge(AKANTE(a)); /* Edge is not drawable */
		a = b;
	}
}

/* Sort the adjacency lists of node v
 * ----------------------------------
 * This gives the layout the final touch.
 */

#ifdef ANSI_C
static void sort_adjacencies(GNODE v)
#else
static void sort_adjacencies(v)
GNODE   v;
#endif
{
        int i;
        ADJEDGE a;
 
        debugmessage("sort_adjacencies","");
        assert((v));
        i = 0;
        a = NPRED(v);
        while (a) {
                adjarray2[i++] = AKANTE(a);
                a = ANEXT(a);
        }

#ifdef ANSI_C
        qsort(adjarray2,NINDEG(v),sizeof(GEDGE),
		(int (*) (const void *, const void *))compare_ppos);
#else
        qsort(adjarray2,NINDEG(v),sizeof(GEDGE), compare_ppos);
#endif
        i = 0;
        a = NPRED(v);
        while (a) {
                AKANTE(a) = adjarray2[i++];
                a = ANEXT(a);
        }
        if (i) { /* at least one predecessor */
                NPREDL(v) = adjarray2[0];
                NPREDR(v) = adjarray2[i-1];
        }
	i = 0;
	a = NSUCC(v);
	while (a) {
		adjarray2[i++] = AKANTE(a);
		a = ANEXT(a);
	}

#ifdef ANSI_C
        qsort(adjarray2,NOUTDEG(v),sizeof(GEDGE),
		(int (*) (const void *, const void *))compare_spos);
#else
        qsort(adjarray2,NOUTDEG(v),sizeof(GEDGE), compare_spos);
#endif
        i = 0;
        a = NSUCC(v);
        while (a) {
                AKANTE(a) = adjarray2[i++];
                a = ANEXT(a);
        }
        if (i) { /* at least one successor */
                NSUCCL(v) = adjarray2[0];
                NSUCCR(v) = adjarray2[i-1];
        }
}


/*  Compare functions for sort_adjedges
 *  ------------------------------------
 *  returns 1 if EWEIGHT(*a) > EWEIGHT(*b), 0 if equal, -1 otherwise
 */

#ifdef ANSI_C
static int compare_ppos(const GEDGE *a, const GEDGE *b)
#else
static int compare_ppos(a,b)
GEDGE   *a;
GEDGE   *b;
#endif
{
        if (EWEIGHTP(*a) > EWEIGHTP(*b))        return(1);
        if (EWEIGHTP(*a) < EWEIGHTP(*b))        return(-1);
        return(0);
}
 
#ifdef ANSI_C
static int compare_spos(const GEDGE *a,const GEDGE *b)
#else
static int compare_spos(a,b)
GEDGE   *a;
GEDGE   *b;
#endif
{
        if (EWEIGHTS(*a) > EWEIGHTS(*b))        return(1);
        if (EWEIGHTS(*a) < EWEIGHTS(*b))        return(-1);
        return(0);
}
 
 
