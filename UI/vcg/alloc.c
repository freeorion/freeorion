/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         alloc.c                                            */
/*   version:      1.00.00                                            */
/*   creation:     14.4.1993                                          */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Memory Management                                  */
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
 * Revision 3.9  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.8  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 *
 * Revision 3.7  1994/08/03  13:58:44  sander
 * Horizontal order mechanism changed.
 * Attribute horizontal_order for edges added.
 *
 * Revision 3.6  1994/05/17  16:37:18  sander
 * attribute node_align added to allow nodes to be centered in the levels.
 *
 * Revision 3.5  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
 *
 * Revision 3.4  1994/05/05  08:20:30  sander
 * dllist_free_all added for the local optimization of crossings.
 *
 * Revision 3.3  1994/04/27  16:05:19  sander
 * Some general changes for the PostScript driver.
 * Horizontal order added. Bug fixes of the folding phases:
 * Folding of nested graphs works now.
 *
 * Revision 3.2  1994/03/04  19:11:24  sander
 * Specification of levels per node added.
 * X11 geometry behaviour (option -geometry) changed such
 * that the window is now opened automatically.
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


/****************************************************************************
 * This file is a collection of auxiliary functions that implement the
 * memory management. It provides the following functions:
 *
 * myalloc		allocates memory from the internal memory management.
 * free_memory		gives the complete memory free.
 *
 * nodealloc		allocates a GNODE object (node), adds it to nodelist
 * graphalloc		allocates a GNODE object (graph), adds it to graphlist
 * tmpnodealloc		allocates a temporary GNODE object
 * free_node		deallocate a nontemporary GNODE object
 *
 * nodelist_alloc	allocates a stable node list element (i.e. a cons 
 *			cell whose head is a GNODE object)
 * tmpnodelist_alloc	allocates a temporary node list element (i.e. a cons 
 *			cell whose head is a GNODE object)
 * free_regionnodelist	deallocate a list of node list elements. 
 *		 	These were used as storage of regions.
 *
 * edgealloc		allocates a GEDGE object (edge), adds it to edgelist
 * tmpedgealloc		allocates a temporary GEDGE object
 *
 * near_edge_insert     insert a near edge into near_edge_list. A near edge
 *			is a special edge that must always be placed near an
 *			other edge. See the nearedge-specification-feature.
 *			We use a special adjacency list to notify all 
 *			near edges.
 * bentnear_edge_insert insert a bent near edge into bent_near_edge_list. 
 *			A bent near edge is a special edge consisting of a
 *			near edge, a dummy node and a normal edge.
 *			See the nearedge-specification-feature.
 *			We use a special adjacency list to notify all 
 *			bent near edges.
 * back_edge_insert     insert a back edge into back_edge_list. A back edge
 *			is an edge that preferably is reverted.
 *			We use a special adjacency list to notify all 
 *			back edges.
 * prededgealloc	allocates an edge list element (i.e. a cons cell whose
 *                      head is a GEDGE object), used as adjacency list of
 *                      predecessors of a node.
 * succedgealloc	allocates an edge list element (i.e. a cons cell whose
 *                      head is a GEDGE object), used as adjacency list of
 *                      succecessors of a node.
 *
 * connectalloc		allocates a CONNECT element of a node
 *
 * dllist_alloc		allocates a DLLIST-cons cell. These are double linked
 *			lists of nodes.
 * dllist_free	 	gives one DLLIST-cons cell free.
 *
 * free_all_lists	gives all temporary memory free.
 * reinit_all_lists     reinitialize all memory lists. This is done if the
 *			memory is given free: all lists are set to NULL.
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "grammar.h"
#include "main.h"
#include "options.h"
#include "alloc.h"
#include "folding.h"

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

static GNODE internal_nodealloc	_PP((void));
static void free_nodelists	_PP((void));
static GEDGE internal_edgealloc	_PP((void));
static void free_tmpedges	_PP((void));
static ADJEDGE  edgelist_alloc	_PP((void));
static void free_edgelists	_PP((void));
static void free_connect	_PP((void));

/*--------------------------------------------------------------------*/
/* Memory allocation                                                  */
/*--------------------------------------------------------------------*/

/*  Core Memory Management 
 *  ======================
 */

#ifdef DEBUG
static long act_alloc_size  = 0;
static long act_alloc_edges = 0;
static long act_alloc_nodes = 0;
#endif

static long node_refnum = 0L;	/* reference counter for REFNUM	of nodes */


/*   Core Memory allocation
 *   ----------------------  
 *   allocate x bytes. We use the memory mechanism from the generated 
 *   parser. 
 */

#ifdef ANSI_C
char *myalloc(int x)
#else
char *myalloc(x)
int 	x;
#endif
{
	debugmessage("myalloc. Nr. of Bytes:",my_itoa(x));
#ifdef DEBUG
	act_alloc_size += x;
	PRINTF("Alloc Summary: %ld Bytes allocated\n",act_alloc_size);
#endif
	return(ParseMalloc(x));
}


/*   Core Memory deallocation
 *   ------------------------  
 *   deallocate the complete memory.
 */

#ifdef ANSI_C
void 	free_memory(void)
#else
void 	free_memory()
#endif
{
	debugmessage("free_memory","");
#ifdef DEBUG
	act_alloc_size  = 0;
	act_alloc_nodes = 0;
	act_alloc_edges = 0;
#endif
	FreeHash();
	ParseFree();
	node_refnum = 0L;	
	reinit_all_lists();
}

/*--------------------------------------------------------------------*/

/*  Memory Management for Nodes 
 *  ===========================
 */

/*  We distinguish between
 *     1) GNODE objects as nodes from the specification
 *     2) GNODE objects as representation of subgraphs
 *     3) temporary GNODE objects, used to calculate a layout.
 *        They can be deleted on each change of layout.
 */

/*  Nodes from the specification come into the nodelist.
 *  Subgraphs come into the subgraphlist.
 *  Temporary nodes come into the tmpnodelist.
 *  Free GNODE objects are collected into the node_freelist. 
 */

int nodeanz   = 0;    	       /* Number of nodes in nodelist         */
int dummyanz  = 0;             /* Number of dummy nodes (not labels)  */ 
 
 
GNODE nodelist     = NULL;     /* List of all real nodes as specified */
GNODE nodelistend  = NULL;     /* End of this list                    */
GNODE graphlist    = NULL;     /* List of all subgraphs as specified  */
GNODE graphlistend = NULL;     /* End of this list                    */

GNODE invis_nodes  = NULL;     /* List of all invisible nodes         */
GNODE labellist    = NULL;     /* List of dummy nodes that contain    */
                               /* the text of an edge label           */
GNODE labellistend = NULL;     /* End of this list                    */
GNODE dummylist    = NULL;     /* List of other dummy nodes           */

GNODE tmpnodelist   = NULL;     /* list of allocated temoprary nodes */
static GNODE node_freelist = NULL;     /* list of free GNODE objects */



/*  Allocate a GNODE object
 *  -----------------------
 *  First, we look in the free list, if we have a free node. Otherwise,
 *  we allocate a node from the core memory.
 *  We also set some default values.
 */

#ifdef ANSI_C
static GNODE internal_nodealloc(void)
#else
static GNODE internal_nodealloc()
#endif
{
	GNODE   h;

	debugmessage("internal_nodealloc","");
	if (node_freelist) {
		h = node_freelist;
		node_freelist = NINTERN(node_freelist);
	}
	else {
	 	h = (GNODE) myalloc(sizeof(struct gnode));
#ifdef DEBUG
		act_alloc_nodes++;
		PRINTF("Alloc Summary: %ld GNODEs allocated\n",act_alloc_nodes);
#endif
	}

	NREFNUM(h)	= node_refnum++;
	NTITLE(h) 	= NULL;
	NLABEL(h) 	= NULL;
	NINFO1(h)	= NULL;
	NINFO2(h)	= NULL;
	NINFO3(h)	= NULL;
	NLEVEL(h) 	= -1;
	NSHAPE(h) 	= 0;
	NHORDER(h) 	= -1;
	NSX(h)        	= 0L;	
	NSY(h)        	= 0L;
	NX(h)        	= 0L;	
	NY(h)        	= 0L;
	NSGRAPH(h)	= NULL;
	NROOT(h)	= NULL;
	NREGREPL(h)	= NULL;
	NREGION(h)	= NULL;
	NREGROOT(h)	= NULL;
	NINLIST(h)	= 1;
	NINVISIBLE(h)	= 0;
	NTIEFE(h)    	= -1;
	NPOS(h)      	= -1;   
	NWEIGHTS(h)     = 0L;
	NWEIGHTP(h)	= 0L;
	NMARK(h)     	= 0; 
	NREVERT(h)     	= 0; 
	NANCHORNODE(h) 	= 0; 
	h->bary 	= -1;
	NDFS(h)      	= -1L; 
	NINDEG(h)    	= 0;
	NOUTDEG(h)	= 0;
	NVPTR(h)	= NULL;
	NPRED(h)      	= NULL;
	NSUCC(h)     	= NULL; 
	NSVPRED(h)     	= NULL;
	NSVSUCC(h)     	= NULL; 
	NPREDL(h)       = NULL;
        NPREDR(h)       = NULL; 
        NSUCCL(h)       = NULL; 
        NSUCCR(h)       = NULL;
	NCONNECT(h)  	= NULL;
	NNEXT(h) 	= NULL;
	NINTERN(h)	= NULL;
	return(h);
}



/*  Allocate a real node 
 *  --------------------
 *  and update the nodelist. Real nodes are such nodes that are specified
 *  as node: { .... }.
 *  We inheret the attributes from the refnode.
 */

#ifdef ANSI_C
GNODE nodealloc(GNODE refnode)
#else
GNODE nodealloc(refnode)
GNODE refnode;
#endif
{
	GNODE h;

	debugmessage("nodealloc","");
	h = internal_nodealloc();
	copy_nodeattributes(refnode, h);
	NBEFORE(h)      = nodelistend;
	if (nodelistend) NNEXT(nodelistend) = h;
	nodelistend	= h;
	nodeanz++;
	if (nodelist == NULL) nodelist = h;
	return(h);

}

/*  Initialize a node with the node defaults
 *  ----------------------------------------
 */
 
#ifdef ANSI_C
void nodedefaults(GNODE node)
#else
void nodedefaults(node)
GNODE node;
#endif
{
	debugmessage("nodedefaults","");

	NTITLE(node)	= G_title;
	NLABEL(node)	= NULL;
	NLEVEL(node)	= -1;
	NSHAPE(node)	= BOX;
	NHORDER(node)	= -1;
	NINFO1(node)	= "";
	NINFO2(node)	= "";
	NINFO3(node)	= "";
	NSX(node)	= 0L;
	NSY(node)	= 0L;
	NTEXTMODE(node)	= CENTER;
	NSTATE(node)    = 0;
	NWIDTH(node)    = -1;
	NHEIGHT(node)   = -1;
	NBORDERW(node)	= 2;
	NFOLDING(node)	= -1;
	NCOLOR(node)	= G_color;
	NTCOLOR(node)	= BLACK;
	NBCOLOR(node)	= BLACK;
	NSHRINK(node)	= 1;
	NSTRETCH(node)	= 1;
}


/*  Initialize a node with the foldnode defaults
 *  --------------------------------------------
 */

#ifdef ANSI_C
void foldnodedefaults(GNODE node)
#else
void foldnodedefaults(node)
GNODE node;
#endif
{
	debugmessage("foldnodedefaults","");

	NTITLE(node)	= NULL;
	NLABEL(node)	= NULL;
	NLEVEL(node)	= -1;
	NSHAPE(node)	= -1;
	NHORDER(node)	= -1;
	NINFO1(node)	= NULL;
	NINFO2(node)	= NULL;
	NINFO3(node)	= NULL;
	NSX(node)	= 0L;
	NSY(node)	= 0L;
	NTEXTMODE(node)	= -1;
	NSTATE(node)    = -1;
	NWIDTH(node)    = -1;
	NHEIGHT(node)   = -1;
	NBORDERW(node)	= 4;
	NFOLDING(node)	= -1;
	NCOLOR(node)	= -1;
	NTCOLOR(node)	= -1;
	NBCOLOR(node)	= -1;
	NSHRINK(node)	= -1;
	NSTRETCH(node)	= -1;
}


/*  Copy the foldnode attributes from fn to y
 *  -----------------------------------------
 */

#ifdef ANSI_C
void inherit_foldnode_attributes(GNODE fn, GNODE y)
#else
void inherit_foldnode_attributes(fn, y)
GNODE fn, y;
#endif
{
	debugmessage("inherit_foldnode_attributes","");

	/* NTITLE not needed */
	/* NFOLDING not needed */
	if (NLABEL(fn))	 	NLABEL(y)	= NLABEL(fn);
	if (NLEVEL(fn)!= -1) 	NLEVEL(y)	= NLEVEL(fn);
	if (NSHAPE(fn)!= -1) 	NSHAPE(y)	= NSHAPE(fn);
	if (NHORDER(fn)!= -1) 	NHORDER(y)	= NHORDER(fn);
	if (NINFO1(fn))	 	NINFO1(y)	= NINFO1(fn);
	if (NINFO2(fn))	 	NINFO2(y)	= NINFO2(fn);
	if (NINFO3(fn))	 	NINFO3(y)	= NINFO3(fn);
	if (NSX(fn)!= -1L)	NSX(y)		= NSX(fn);
	if (NSY(fn)!= -1L) 	NSY(y)	 	= NSY(fn);
	if (NTEXTMODE(fn)!= -1)	NTEXTMODE(y)	= NTEXTMODE(fn);
	if (NSTATE(fn)!= -1)  	NSTATE(y)     	= NSTATE(fn);
	if (NWIDTH(fn)!= -1)  	NWIDTH(y)	= NWIDTH(fn);
	if (NHEIGHT(fn)!= -1) 	NHEIGHT(y)    	= NHEIGHT(fn);
	if (NBORDERW(fn)!= -1)	NBORDERW(y)	= NBORDERW(fn);
	if (NCOLOR(fn)!= -1) 	NCOLOR(y)	= NCOLOR(fn);
	if (NTCOLOR(fn)!= -1)	NTCOLOR(y)	= NTCOLOR(fn);
	if (NBCOLOR(fn)!= -1) 	NBCOLOR(y)	= NBCOLOR(fn);
	if (NSHRINK(fn)!= -1) 	NSHRINK(y)	= NSHRINK(fn);
	if (NSTRETCH(fn)!= -1) 	NSTRETCH(y)	= NSTRETCH(fn);
}


/*  Copy a GNODE object x into a second GNODE object y
 *  --------------------------------------------------
 */

#ifdef ANSI_C
void copy_nodeattributes(GNODE x, GNODE y)
#else
void copy_nodeattributes(x, y)
GNODE x, y;
#endif
{
	NTITLE(y)	= NTITLE(x);
	NLABEL(y)	= NLABEL(x);
	NLEVEL(y)	= NLEVEL(x);
	NSHAPE(y)	= NSHAPE(x);
	NHORDER(y)	= NHORDER(x);
	NINFO1(y)	= NINFO1(x);
	NINFO2(y)	= NINFO2(x);
	NINFO3(y)	= NINFO3(x);
	NSX(y)		= NSX(x);
	NSY(y)		= NSY(x);
	NTEXTMODE(y)	= NTEXTMODE(x);
	NSTATE(y)    	= NSTATE(x);
	NWIDTH(y)    	= NWIDTH(x);
	NHEIGHT(y)   	= NHEIGHT(x);
	NBORDERW(y)	= NBORDERW(x);
	NFOLDING(y)	= NFOLDING(x);
	NCOLOR(y)	= NCOLOR(x);
	NTCOLOR(y)	= NTCOLOR(x);
	NBCOLOR(y)	= NBCOLOR(x);
	NSHRINK(y)	= NSHRINK(x);
	NSTRETCH(y)	= NSTRETCH(x);
}



/*  Allocate a graph object 
 *  -----------------------
 *  and update the graphlist. Such objects are summary nodes whose graph
 *  is specified as graph: { .... }. 
 *  We inheret the attributes from the refnode.
 */

#ifdef ANSI_C
GNODE graphalloc(GNODE refnode)
#else
GNODE graphalloc(refnode)
GNODE refnode;
#endif
{
	GNODE   h;

	debugmessage("graphalloc","");
	h = internal_nodealloc();
	copy_nodeattributes(refnode, h);
	NFOLDING(h)	= -1;
	NINLIST(h)	= 0;
	NINVISIBLE(h) 	= 1;
	NDFS(h)		= 0L;
	NBEFORE(h)	= graphlistend;

	if (graphlistend) NNEXT(graphlistend) = h;
	graphlistend	= h;
	if (graphlist == NULL) graphlist = h;
	return(h);
}


/*  Allocate a temporary GNODE object 
 *  ---------------------------------
 *  and update the tmpnodelist. These are node for dummy's, label's etc.
 *  These nodes are only needed for the layouting.
 */

#ifdef ANSI_C
GNODE	tmpnodealloc(
	int     textm,
	int	width,
	int	height,
	int	borderw,
	int	fold,
	int	color,
	int	textc,
	int	borderc,
	int     shrink,
	int	stretch,
	int	horder)
#else
GNODE	tmpnodealloc(textm,width,height,borderw,fold,color,textc,borderc,
                 	    shrink,stretch,horder)
int     textm,width,height,borderw,fold,color,textc,borderc;
int     shrink,stretch,horder;
#endif
{
	GNODE	h;

	debugmessage("tmpnodealloc","");
	h = internal_nodealloc();

	NHORDER(h)	= horder;
        NTEXTMODE(h)    = textm;
	NSTATE(h)    	= 0;   
        NWIDTH(h)       = width;
        NHEIGHT(h)      = height;
        NBORDERW(h)     = borderw;
        NFOLDING(h)     = fold;
        NCOLOR(h)       = color;
        NTCOLOR(h)      = textc;
        NBCOLOR(h)      = borderc;
        NSHRINK(h)      = shrink;
        NSTRETCH(h)     = stretch;
        NINLIST(h)      = 0;
        NINVISIBLE(h)   = 1;
        NDFS(h)         = 0L;
        NBEFORE(h)      = NULL;
	
	NINTERN(h)   = tmpnodelist;
	tmpnodelist  = h;
	return(h);
}


/*  Deallocate all temporary GNODE objects
 *  --------------------------------------
 */

#ifdef ANSI_C
void free_tmpnodes(void)
#else
void free_tmpnodes()
#endif
{
	GNODE	h;

	debugmessage("free_tmpnodes","");
	h = tmpnodelist;
	if (h) {
		while (NINTERN(h)) h = NINTERN(h);
		NINTERN(h) = node_freelist;
		node_freelist = tmpnodelist;
		tmpnodelist = NULL;
	}
	labellist    = NULL;  
	labellistend = NULL;  
	dummylist = NULL;
	/* Labels and dummys are temporary nodes thus they 
	 * are given free, too 
 	 */
}


/*  Deallocate one GNODE objects
 *  ----------------------------
 *  This object should not be temporary !!!
 *  In fact, it is a region summary substitution node. See folding.c
 *  If the object would be allocated by tmpnodealloc,
 *  it could be given free twice, which is wrong.
 */

#ifdef ANSI_C
void free_node(GNODE h)
#else
void free_node(h)
GNODE h;
#endif
{
	debugmessage("free_node","");
	NINTERN(h) = node_freelist;
	node_freelist = h;
}


/*  Give a position, search a node in the node list
 *  -----------------------------------------------
 *  This is used in the menues after selecting a node. 
 *  At this time point, all visible nodes are in the node list.
 */

#ifdef ANSI_C
GNODE	search_xy_node(long x,long y)
#else
GNODE	search_xy_node(x,y)
long	x,y;
#endif
{
	GNODE	v;
	int	width, height;
	long	xpos, ypos;

	v = nodelist;
	while (v) {
		xpos = (NX(v)*G_stretch)/G_shrink - V_xmin;
		ypos = (NY(v)*G_stretch)/G_shrink - V_ymin;
		width = (NWIDTH(v)*G_stretch)/G_shrink;
		height = (NHEIGHT(v)*G_stretch)/G_shrink;
		if ( (xpos <= x) && (x <= xpos+width) && 
	     	     (ypos <= y) && (y <= ypos+height) )
		    	return(v);      /* node found */
		v = NNEXT(v);
	}
	return(NULL);		/* no node found */
} 


/*  Check the graph consistency
 *  ---------------------------
 *  A serious problem is that subgraphs may not have any nodes.
 *  This leads to confusion if such a subgraph is folded.
 *  Thus, for such subgraphs, we add auxiliary nodes.
 */

#ifdef ANSI_C
void check_graph_consistency(void)
#else
void check_graph_consistency()
#endif
{
	GNODE v,w;

	v = graphlist;

	while (v) {
		if (NSGRAPH(v)==NULL) {
			if (!silent) {
				FPRINTF(stderr,"\nWarning: Graph %s", 
					(NTITLE(v)?NTITLE(v):""));	
				FPRINTF(stderr," has no nodes.");
				FPRINTF(stderr," I add a node ! \n");
			}
			
			w = nodealloc( v );
			NTITLE(w) = "artificial node";
			NROOT(w) = v;
			NSGRAPH(v) = nodelist_alloc(w);
		}
		v = NNEXT(v);
	}
}

/*--------------------------------------------------------------------*/


/*  Memory Management lists of GNODE objects 
 *  ========================================
 */

/*  Lists of GNODE objects, if they are not connected via internal
 *  GNODE pointers, use special cons-cells, i.e. GNLIST objects, whose 
 *  heads are GNODE objects. Because some cons-cells are temporary, 
 *  we use a similar memory management as for temporary GNODE objects.
 */


static GNLIST tmpnconslist   = NULL;  /* list of allocated cons cells */
static GNLIST foldnconslist   = NULL; /* list of all. fold cons cells */
static GNLIST ncons_freelist = NULL;  /* list of free cons cells      */


/*  Allocate a GNLIST object
 *  ------------------------
 *  First, we look in the free list, if we have a free node. Otherwise,
 *  we allocate a node from the core memory.
 *  We also set some default values.
 *  These node lists are part of the stable graph representation, i.e.
 *  need not to be freed unless a reload of the graph. Thus we don't
 *  store them in the tmpnconslist.
 */

#ifdef ANSI_C
GNLIST  nodelist_alloc(GNODE v)
#else
GNLIST  nodelist_alloc(v)
GNODE v;
#endif
{
	GNLIST	h;

	debugmessage("nodelist_alloc","");
	h = (GNLIST)myalloc(sizeof(struct gnlist));
	GNINTERN(h) = NULL;
	GNNODE(h)   = v;
	GNNEXT(h)   = NULL;
	return(h);
}

/*  Allocate a temporary GNLIST object
 *  ----------------------------------
 *  First, we look in the free list, if we have a free node. Otherwise,
 *  we allocate a node from the core memory.
 *  We also set some default values.
 *  These node lists are temporary, thus we store them in the
 *  tmpnconslist, to give them free later.
 */

#ifdef ANSI_C
GNLIST  tmpnodelist_alloc(void)
#else
GNLIST  tmpnodelist_alloc()
#endif
{
	GNLIST	h;

	debugmessage("tmpnodelist_alloc","");
	if (ncons_freelist) {
		h = ncons_freelist;
		ncons_freelist = GNINTERN(ncons_freelist);
	}
	else    h = (GNLIST)myalloc(sizeof(struct gnlist));
	GNINTERN(h) = tmpnconslist;
	GNNODE(h)   = NULL;
	GNNEXT(h)   = NULL;
	tmpnconslist = h;
	return(h);
}


/*  Allocate a foldlist GNLIST object
 *  ---------------------------------
 *  First, we look in the free list, if we have a free node. Otherwise,
 *  we allocate a node from the core memory.
 *  We also set some default values.
 *  These node lists are used for the folding action keepers in
 *  folding.c. They live longer than temporary nodes, but are
 *  also temporary, because they are deallocated after folding.
 */

#ifdef ANSI_C
GNLIST  foldnodelist_alloc(void)
#else
GNLIST  foldnodelist_alloc()
#endif
{
	GNLIST	h;

	debugmessage("foldnodelist_alloc","");
	if (ncons_freelist) {
		h = ncons_freelist;
		ncons_freelist = GNINTERN(ncons_freelist);
	}
	else    h = (GNLIST)myalloc(sizeof(struct gnlist));
	GNINTERN(h) = foldnconslist;
	GNNODE(h)   = NULL;
	GNNEXT(h)   = NULL;
	foldnconslist = h;
	return(h);
}



/*  Deallocate all temporary GNLIST objects
 *  --------------------------------------
 */

#ifdef ANSI_C
static void free_nodelists(void)
#else
static void free_nodelists()
#endif
{
	GNLIST	h;

	debugmessage("free_nodelists","");
	h = tmpnconslist;
	if (h) {
		while(GNINTERN(h)) h = GNINTERN(h);
		GNINTERN(h) = ncons_freelist;
		ncons_freelist = tmpnconslist;
		tmpnconslist = NULL;
	}
}


/*  Deallocate all fold GNLIST objects
 *  ----------------------------------
 */

#ifdef ANSI_C
void free_foldnodelists(void)
#else
void free_foldnodelists()
#endif
{
	GNLIST	h;

	debugmessage("free_foldnodelists","");
	h = foldnconslist;
	if (h) {
		while(GNINTERN(h)) h = GNINTERN(h);
		GNINTERN(h) = ncons_freelist;
		ncons_freelist = foldnconslist;
		foldnconslist = NULL;
	}
}


/*  Deallocate GNLIST objects of regions
 *  ------------------------------------
 *  These GNLIST objects should be allocated by nodelist_alloc,
 *  i.e. should not be temporary.
 */

#ifdef ANSI_C
void free_regionnodelist(GNLIST	r)
#else
void free_regionnodelist(r)
GNLIST	r;
#endif
{
	GNLIST	h;

	debugmessage("free_regionnodelists","");
	h = r;
	if (h) {
		while(GNINTERN(h)) h = GNINTERN(h);
		GNINTERN(h) = ncons_freelist;
		ncons_freelist = r;
	}
}


/*--------------------------------------------------------------------*/


/*  Memory Management for Edges 
 *  ===========================
 */

/*  We distinguish between
 *     1) GEDGE objects as edges from the specification
 *     2) GEDGE objects from the specification that are not visualized
 *        directly. Neverthelesss, we need the attributes of these
 *        edges, thus we create a auxiliary GEDGE object for them. 
 *     3) temporary GEDGE objects, used to calculate a layout.
 *        They can be deleted on each change of layout.
 */

/*  Edge from the specification come into the edgelist.
 *  Temporary edges come into the tmpedgelist.
 *  Free GEDGE objects are collected into the node_freelist. 
 */

int edgeanz = 0;      	       /* Number of edges in edgelist         */

GEDGE edgelist     = NULL;     /* List of all real edges as specified */
GEDGE edgelistend  = NULL;     /* End of this list                    */

GEDGE tmpedgelist   = NULL;     /* list of allocated temporary edges */
static GEDGE edge_freelist = NULL;     /* list of free GEDGE objects        */


/*  Allocate a GEDGE object
 *  -----------------------
 *  First, we look in the free list, if we have a free edge. Otherwise,
 *  we allocate an edge from the core memory.
 *  We also set some default values.
 */

#ifdef ANSI_C
static GEDGE internal_edgealloc(void)
#else
static GEDGE internal_edgealloc()
#endif
{
	GEDGE   h;

	debugmessage("internal_edgealloc","");
	if (edge_freelist) {
		h = edge_freelist;
		edge_freelist = EINTERN(edge_freelist);
	}
	else {
	 	h = (GEDGE) myalloc(sizeof(struct gedge));
#ifdef DEBUG
		act_alloc_edges++;
		PRINTF("Alloc Summary: %ld GEDGEs allocated\n",act_alloc_edges);
#endif
	}

	ESTART(h)     	= NULL;
	EEND(h)       	= NULL;
	ESTARTX(h)    	= 0;
	ESTARTY(h)    	= 0;
	ETBENDX(h)    	= 0;
	ETBENDY(h)    	= 0;
	EBBENDX(h)    	= 0;
	EBBENDY(h)    	= 0;
	EENDX(h)      	= 0;
	EENDY(h)      	= 0;
	EORI(h)		= NO_ORI;
	EORI2(h)	= NO_ORI;
	ELABEL(h)  	= NULL;
	EART(h)       	= 'U';
	ELNODE(h)      	= NULL;
	EANCHOR(h)	= 0;
	EINVISIBLE(h)	= 0;
	EWEIGHTS(h)   	= 0;
	EWEIGHTP(h)   	= 0;
	ENEXT(h)	= NULL;
	EINTERN(h)	= NULL;
	ELABELCOL(h)	= BLACK;
	return(h);
}


/*  Allocate a real edge 
 *  --------------------
 *  and update the edgelist. These edges are specified, e.g.
 *  as edge: { ... }
 *  We inheret the attributes from the refedge.
 */

#ifdef ANSI_C
GEDGE edgealloc(GEDGE refedge)
#else
GEDGE edgealloc(refedge)
GEDGE refedge;
#endif
{
	GEDGE   h;
 
	debugmessage("edgealloc","");
	h = internal_edgealloc();
	copy_edgeattributes(refedge, h);
	EBEFORE(h)	= edgelistend;
	if (edgelistend) ENEXT(edgelistend) = h;
	edgelistend     = h;
	edgeanz++;
	if (edgelist == NULL) edgelist = h;
	return(h);
}


/*  Initialize an edge with the edge defaults
 *  -----------------------------------------
 */
 
#ifdef ANSI_C
void edgedefaults(GEDGE edge)
#else
void edgedefaults(edge)
GEDGE edge;
#endif
{
	debugmessage("edgedefaults","");

	ELABEL(edge)		= NULL;
	ELSTYLE(edge)    	= SOLID;
	ETHICKNESS(edge)	= 2;
	ECLASS(edge)		= 1;
	EPRIO(edge)		= 1;
	EHORDER(edge)		= -1;
	ECOLOR(edge)		= BLACK;
	ELABELCOL(edge)		= BLACK;
	EARROWSIZE(edge)	= 10;
	EARROWSTYLE(edge)	= ASSOLID;
	EARROWCOL(edge)		= BLACK;
	EARROWBSIZE(edge)	= 0;
	EARROWBSTYLE(edge)	= ASNONESPEC;
	EARROWBCOL(edge)	= BLACK;
}


/*  Initialize an edge with the foldedge defaults
 *  ---------------------------------------------
 */

#ifdef ANSI_C
void foldedgedefaults(GEDGE edge)
#else
void foldedgedefaults(edge)
GEDGE edge;
#endif
{
	debugmessage("foldedgedefaults","");

	ELABEL(edge)		= "...";
	ELSTYLE(edge)    	= -1;
	ETHICKNESS(edge)	= 4;
	ECLASS(edge)		= -1;
	EPRIO(edge)		= -1;
	EHORDER(edge)		= -1;
	ECOLOR(edge)		= -1;
	ELABELCOL(edge)		= -1;
	EARROWSIZE(edge)	= -1;
	EARROWSTYLE(edge)	= -1;
	EARROWCOL(edge)		= -1;
	EARROWBSIZE(edge)	= -1;
	EARROWBSTYLE(edge)	= -1;
	EARROWBCOL(edge)	= -1;
}


/*  Copy the foldedge attributes from fn to y
 *  -----------------------------------------
 */

#ifdef ANSI_C
void inherit_foldedge_attributes(GEDGE fn, GEDGE y)
#else
void inherit_foldedge_attributes(fn, y)
GEDGE fn, y;
#endif
{
	debugmessage("inherit_foldedge_attributes","");

	if (ELABEL(fn)) 	   ELABEL(y)       = ELABEL(fn);
	if (ELSTYLE(fn)     != -1) ELSTYLE(y)      = ELSTYLE(fn);
	if (ETHICKNESS(fn)  != -1) ETHICKNESS(y)   = ETHICKNESS(fn);
	if (ECLASS(fn)      != -1) ECLASS(y)       = ECLASS(fn);
	if (EPRIO(fn)       != -1) EPRIO(y)        = EPRIO(fn);
	if (EHORDER(fn)     != -1) EHORDER(y)      = EHORDER(fn);
	if (ECOLOR(fn)      != -1) ECOLOR(y)       = ECOLOR(fn);
	if (ELABELCOL(fn)   != -1) ELABELCOL(y)    = ELABELCOL(fn);
	if (EARROWSIZE(fn)  != -1) EARROWSIZE(y)   = EARROWSIZE(fn);
	if (EARROWSTYLE(fn) != -1) EARROWSTYLE(y)  = EARROWSTYLE(fn);
	if (EARROWCOL(fn)   != -1) EARROWCOL(y)    = EARROWCOL(fn);
	if (EARROWBSIZE(fn) != -1) EARROWBSIZE(y)  = EARROWBSIZE(fn);
	if (EARROWBSTYLE(fn)!= -1) EARROWBSTYLE(y) = EARROWBSTYLE(fn);
	if (EARROWBCOL(fn)  != -1) EARROWBCOL(y)   = EARROWBCOL(fn);
}


/*  Copy a GEDGE object x into a second GEDGE object y
 *  --------------------------------------------------
 */

#ifdef ANSI_C
void copy_edgeattributes(GEDGE x, GEDGE y)
#else
void copy_edgeattributes(x, y)
GEDGE x, y;
#endif
{
	ELABEL(y)	= ELABEL(x);
	ELSTYLE(y)    	= ELSTYLE(x);
	ETHICKNESS(y)	= ETHICKNESS(x);
	ECLASS(y)	= ECLASS(x);
	EPRIO(y)	= EPRIO(x);
	EHORDER(y)	= EHORDER(x);
	ECOLOR(y)	= ECOLOR(x);
	ELABELCOL(y)	= ELABELCOL(x);
	EARROWSIZE(y)	= EARROWSIZE(x);
	EARROWSTYLE(y)	= EARROWSTYLE(x);
	EARROWCOL(y)	= EARROWCOL(x);
	EARROWBSIZE(y)	= EARROWBSIZE(x);
	EARROWBSTYLE(y)	= EARROWBSTYLE(x);
	EARROWBCOL(y)	= EARROWBCOL(x);
}



/*  Allocate a temporary GEDGE object 
 *  ---------------------------------
 *  and update the tmpedgelist. These are edges to dummy nodes or
 *  to labels.
 */

#ifdef ANSI_C
GEDGE	tmpedgealloc(
	int	lstyle,
	int	thick,
	int	xclass,
	int	prio,
	int	ecolor,
	int	elcol,
	int	arrows,
	int	barrows,
	int	arrowsty,
	int	barrowsty,
	int	arrowc,
	int	barrowc,
	int	horder)
#else
GEDGE	tmpedgealloc(lstyle,thick,xclass,prio,ecolor,elcol,arrows,
		barrows,arrowsty,barrowsty,arrowc,barrowc,horder)
int	lstyle,thick,xclass,prio,ecolor,elcol,arrows,
	barrows,arrowsty,barrowsty,arrowc,barrowc,horder;
#endif
{
	GEDGE	h;

	debugmessage("tmpedgealloc","");
	h = internal_edgealloc();

	ELSTYLE(h)    	= lstyle;
	ETHICKNESS(h)	= thick;
	ECLASS(h)	= xclass;
	EPRIO(h)	= prio;
	EHORDER(h)	= horder;
	ECOLOR(h)	= ecolor;
	ELABELCOL(h)	= elcol;
	EBEFORE(h)	= NULL;
	EARROWSTYLE(h)	= ASSOLID;
	EARROWCOL(h)	= ecolor;
	EARROWSIZE(h)	= arrows;
	EARROWSTYLE(h)	= arrowsty;
	EARROWCOL(h)	= arrowc;
	EARROWBSIZE(h)	= barrows;
	EARROWBSTYLE(h)	= barrowsty;
	EARROWBCOL(h)	= barrowc;

	EINTERN(h) = tmpedgelist;
	tmpedgelist = h;
	return(h);
}


/*  Deallocate all temporary GEDGE objects
 *  --------------------------------------
 */

#ifdef ANSI_C
static void free_tmpedges(void)
#else
static void free_tmpedges()
#endif
{
	GEDGE	h;

	debugmessage("free_tmpedges","");
	h = tmpedgelist;
	if (h) {
		while (EINTERN(h)) h = EINTERN(h);
		EINTERN(h) = edge_freelist;
		edge_freelist = tmpedgelist;
		tmpedgelist = NULL;
	}
}


/*--------------------------------------------------------------------*/

/*  Memory Management lists of GEDGE objects 
 *  ========================================
 */

/*  Lists of GEDGE objects are used in adjacency lists.
 *  We use special cons-cells, i.e. ADJEDGE objects, whose 
 *  heads are GEDGE objects. Because these cons-cells are temporary, 
 *  we use a similar memory management as for temporary GNODE objects.
 *
 *  Further, we have one nontemporary list of edges that contains the
 *  default connections as specified by `near_edge'. This is the
 *  near_edge_list.
 *  Dito, we have one nontemporary list of edges that contains the
 *  edges specified by `back_edge'. This is the back_edge_list.
 */


/* for stable default connections: */

ADJEDGE near_edge_list = NULL;	    /* list of default connections */
ADJEDGE bent_near_edge_list = NULL; /* list of bent near edges     */
ADJEDGE back_edge_list = NULL;	    /* list of back edges          */

/* for temporaries: */

static ADJEDGE tmpeconslist   = NULL;  /* list of allocated cons cells */
static ADJEDGE econs_freelist = NULL;  /* list of free cons cells      */


/*  Insert a near edge into near_edge_list
 *  --------------------------------------
 *  First, we look in the free list, if we have a free cell. Otherwise,
 *  we allocate a cell from the core memory.
 */

#ifdef ANSI_C
void near_edge_insert(GEDGE e)
#else
void near_edge_insert(e)
GEDGE e;
#endif
{
	ADJEDGE	h;

	debugmessage("near_edge_insert","");
	if (econs_freelist) {
		h = econs_freelist;
		econs_freelist = AINTERN(econs_freelist);
	}
	else    h = (ADJEDGE)myalloc(sizeof(struct adjedge));
	AKANTE(h) = e;
	ANEXT(h) = AINTERN(h) = near_edge_list;
	near_edge_list = h;
}


/*  Insert a bent near edge into bent_near_edge_list
 *  ------------------------------------------------
 *  First, we look in the free list, if we have a free cell. Otherwise,
 *  we allocate a cell from the core memory.
 */

#ifdef ANSI_C
void bentnear_edge_insert(GEDGE e)
#else
void bentnear_edge_insert(e)
GEDGE e;
#endif
{
	ADJEDGE	h;

	debugmessage("bentnear_edge_insert","");
	if (econs_freelist) {
		h = econs_freelist;
		econs_freelist = AINTERN(econs_freelist);
	}
	else    h = (ADJEDGE)myalloc(sizeof(struct adjedge));
	AKANTE(h) = e;
	ANEXT(h) = AINTERN(h) = bent_near_edge_list;
	bent_near_edge_list = h;
}



/*  Insert a back edge into back_edge_list
 *  --------------------------------------
 *  First, we look in the free list, if we have a free cell. Otherwise,
 *  we allocate a cell from the core memory.
 */

#ifdef ANSI_C
void back_edge_insert(GEDGE e)
#else
void back_edge_insert(e)
GEDGE e;
#endif
{
	ADJEDGE	h;

	debugmessage("back_edge_insert","");
	if (econs_freelist) {
		h = econs_freelist;
		econs_freelist = AINTERN(econs_freelist);
	}
	else    h = (ADJEDGE)myalloc(sizeof(struct adjedge));
	AKANTE(h) = e;
	ANEXT(h) = AINTERN(h) = back_edge_list;
	back_edge_list = h;
}



/*  Allocate a ADJEDGE object
 *  -------------------------
 *  First, we look in the free list, if we have a free cell. Otherwise,
 *  we allocate a cell from the core memory.
 */

#ifdef ANSI_C
static ADJEDGE  edgelist_alloc(void)
#else
static ADJEDGE  edgelist_alloc()
#endif
{
	ADJEDGE	h;

	debugmessage("edgelist_alloc","");
	if (econs_freelist) {
		h = econs_freelist;
		econs_freelist = AINTERN(econs_freelist);
	}
	else    h = (ADJEDGE)myalloc(sizeof(struct adjedge));
	AINTERN(h) = tmpeconslist;
	tmpeconslist = h;
	return(h);
}


/*  Add a new edge to the predecessors of a node
 *  --------------------------------------------
 */

#ifdef ANSI_C
ADJEDGE prededgealloc(GNODE node, GEDGE edge)
#else
ADJEDGE prededgealloc(node,edge)
GNODE	node;	
GEDGE   edge;
#endif
{
	ADJEDGE e;
        
	/* assert((EEND(edge)==node)); */
	e = edgelist_alloc();
	AKANTE(e)	= edge;
	ANEXT(e)  	= NPRED(node);
	NPRED(node) 	= e;
	return(e);
}


/*  Add a new cons cell to the successors of a node
 *  -----------------------------------------------
 */

#ifdef ANSI_C
ADJEDGE succedgealloc(GNODE node, GEDGE edge)
#else
ADJEDGE succedgealloc(node,edge)
GNODE   node;        
GEDGE   edge;
#endif
{
	ADJEDGE e;   
        
	/* assert((ESTART(edge)==node)); */
	e = edgelist_alloc();
	AKANTE(e)	= edge;
	ANEXT(e) 	= NSUCC(node);
	NSUCC(node)	= e;
	return(e); 
}


/*  Deallocate all temporary ADJEDGE objects
 *  ----------------------------------------
 */

#ifdef ANSI_C
static void free_edgelists(void)
#else
static void free_edgelists()
#endif
{
	ADJEDGE	h;

	debugmessage("free_edgelists","");
	h = tmpeconslist;
	if (h) {
		while(AINTERN(h)) h = AINTERN(h);
		AINTERN(h) = econs_freelist;
		econs_freelist = tmpeconslist;
		tmpeconslist = NULL;
	}
}



/*--------------------------------------------------------------------*/

/*  Memory Management for CONNECT objects 
 *  =====================================
 */

/*  CONNECT objects are annotations of GNODE objects.
 *  They indicate that two nodes must be directly neigboured during
 *  the layout. This occurs if nodes are at the same level connected.
 *  E.g.     
 *          /    |    \      this situation is layouted as if we had only
 *         A<----B---->C     one node B. The connections of B are A and C.
 */

static CONNECT 	connectlist      = NULL;   /* list of alloc. connect cells */
static CONNECT 	connect_freelist = NULL;   /* list of free   connect cells */


/*  Allocate a CONNECT object
 *  -------------------------
 *  First, we look in the free list, if we have a free cell. Otherwise,
 *  we allocate a cell from the core memory.
 *  The new connect node is inserted into the connection field of the 
 *  GNODE node.
 */

#ifdef ANSI_C
CONNECT	connectalloc(GNODE node)
#else
CONNECT	connectalloc(node)
GNODE	node;
#endif
{
	CONNECT	h;

	debugmessage("connectalloc","");
	if (connect_freelist) {
		h = connect_freelist;
		connect_freelist = CINTERN(connect_freelist);
	}
	else 	h = (CONNECT)myalloc(sizeof(struct connect));
	CTARGET(h) 	= NULL;
	CEDGE(h)	= NULL;
	CTARGET2(h)	= NULL;
	CEDGE2(h)	= NULL;
	CINTERN(h) 	= connectlist;
	connectlist	= h;
	NCONNECT(node) 	= h;
	return(h);
}


/*  Deallocate all temporary CONNECT objects
 *  ----------------------------------------
 */

#ifdef ANSI_C
static void free_connect(void)
#else
static void free_connect()
#endif
{
	CONNECT	h;
		
	debugmessage("free_connect","");
	h = connectlist;
	if (h) {
		while(CINTERN(h)) h = CINTERN(h);
		CINTERN(h) = connect_freelist;
		connect_freelist = connectlist;
		connectlist = NULL;
	}
}


/*--------------------------------------------------------------------*/

/*  Memory Management for DLLIST objects 
 *  ====================================
 */

/*  To have a good layout, we calculate the number of crossings of edges
 *  and try to minimize them. For the calculation of crossings, we need
 *  double linked lists of nodes (see step2.c) representing the upper
 *  list of touched nodes and the lower list of touched nodes. Because
 *  nodes may have multible occurences in these lists, we use the special
 *  data structure DLLIST. 
 *  We reuse the DSUCC field to keep the list of free dllist nodes.
 *  But THIS dllist_freelist is NOT double linked.
 */

static DLLIST	dllist_freelist  = NULL;     /* list of free dllist nodes */


/*  Allocate a DLLIST object
 *  ------------------------
 *  First, we look in the free list, if we have a free cell. Otherwise,
 *  we allocate a cell from the core memory.
 *  The successor is always NULL, because we append at the end of the
 *  list. pred is the predecessor.
 */


#ifdef ANSI_C
DLLIST 	dllist_alloc(GNODE node, DLLIST pred)
#else
DLLIST 	dllist_alloc(node,pred)
GNODE  node;
DLLIST pred;
#endif
{
	DLLIST	h;

	debugmessage("dllist_alloc","");
	if (dllist_freelist) {
		h = dllist_freelist;
		dllist_freelist = DSUCC(dllist_freelist);
	}
	else    h = (DLLIST)myalloc(sizeof(struct dllist));
	DNODE(h) = node;
	DPRED(h) = pred;
	DSUCC(h) = NULL;
	return(h);
}


/*  Deallocate one DLLIST objects
 *  -----------------------------
 *  The crossing algorithm is stable enough such that after calculation
 *  of crossings all DLLIST elements are given free by this function.
 *  Thus we don't need any additional memory management.
 */

#ifdef ANSI_C
void	dllist_free(DLLIST x)
#else
void	dllist_free(x)
DLLIST x;
#endif
{
	debugmessage("dllist_free","");
       	DSUCC(x) = dllist_freelist;
       	dllist_freelist = x;
}


/*  Deallocate a list of DLLIST objects
 *  -----------------------------------
 */

#ifdef ANSI_C
void	dllist_free_all(DLLIST x)
#else
void	dllist_free_all(x)
DLLIST x;
#endif
{
	DLLIST h;

	debugmessage("dllist_free","");
	if (x) {
		h = x;
		while (DSUCC(h)) h = DSUCC(h);
        	DSUCC(h) = dllist_freelist;
        	dllist_freelist = x;
	}
}



/*--------------------------------------------------------------------*/


/*  Deallocation of all temporary lists 
 *  ===================================
 */


#ifdef ANSI_C
void	free_all_lists(void)
#else
void	free_all_lists()
#endif
{
	free_tmpnodes();
	free_tmpedges();
        free_nodelists();
	free_edgelists();
	free_connect();
}


/*  Reinitialization of all struct keeping lists 
 *  --------------------------------------------
 */

#ifdef ANSI_C
void    reinit_all_lists(void)
#else
void    reinit_all_lists()
#endif
{
	ufoldstart  = NULL;
        foldstart   = NULL;
        foldstops   = NULL;
        f_subgraphs = NULL;
        uf_subgraphs= NULL;
	invis_nodes	 = NULL;
	labellist	 = NULL;
	labellistend 	 = NULL;
	dummylist	 = NULL;

	nodeanz 	 = 0; 
	dummyanz  	 = 0;
	nodelist	 = NULL;
	nodelistend	 = NULL;
	graphlist        = NULL;
	graphlistend     = NULL;
	tmpnodelist	 = NULL;
	node_freelist    = NULL;

	tmpnconslist     = NULL;
	ncons_freelist   = NULL;

	edgeanz 	 = 0;
	edgelist     	 = NULL;
	edgelistend  	 = NULL;
	tmpedgelist   	 = NULL;
	edge_freelist    = NULL;

	near_edge_list      = NULL;
	back_edge_list      = NULL;
	bent_near_edge_list = NULL;
	tmpeconslist        = NULL;
	econs_freelist      = NULL;

	connectlist	 = NULL;
	connect_freelist = NULL;

	dllist_freelist	 = NULL;
}


