/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */
/*		--------------------------------------		      */
/*								      */
/*   file:	   folding.c					      */
/*   version:	   1.00.00					      */
/*   creation:	   17.9.1993					      */
/*   author:	   I. Lemke  (...-Version 0.99.99)		      */
/*		   G. Sander (Version 1.00.00-...)		      */  
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */
/*   description:  Folding and Unfolding of the graph		      */
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


/* $Log$
 * Revision 1.1  2004/12/30 02:21:46  tzlaine
 * Initial add.
 *
 * Revision 3.11  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.10  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 *
 * Revision 3.9  1994/11/23  14:50:47  sander
 * Bug in folding operations corrected.
 *
 * Revision 3.8  1994/08/05  12:13:25  sander
 * Treelayout added. Attributes "treefactor" and "spreadlevel" added.
 * Scaling as abbreviation of "stretch/shrink" added.
 *
 * Revision 3.7  1994/08/03  13:58:44  sander
 * Horizontal order mechanism changed.
 * Attribute horizontal_order for edges added.
 *
 * Revision 3.6  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
 *
 * Revision 3.5  1994/05/05  08:20:30  sander
 * Algorithm late labels added: If labels are inserted
 * after partitioning, this may yield a better layout.
 *
 * Revision 3.4  1994/04/27  16:05:19  sander
 * Some general changes for the PostScript driver.
 * Horizontal order added. Bug fixes of the folding phases:
 * Folding of nested graphs works now.
 *
 * Revision 3.3  1994/03/04  19:11:24  sander
 * Specification of levels per node added.
 * X11 geometry behaviour (option -geometry) changed such
 * that the window is now opened automatically.
 *
 * Revision 3.2  1994/03/02  11:48:54  sander
 * Layoutalgoritms mindepthslow, maxdepthslow, minindegree, ... mandegree
 * added.
 * Anchors and nearedges are not anymore allowed to be intermixed.
 * Escapes in strings are now allowed.
 *
 * Revision 3.1  1994/03/01  10:59:55  sander
 * Copyright and Gnu Licence message added.
 * Problem with "nearedges: no" and "selfloops" solved.
 *
 * Revision 2.4  1994/02/08  09:55:18  sander
 * Hide edge algorithm changed: now all edges of a node must be hidden,
 * not only the forward edges, to hide the node itself.
 *
 * Revision 2.3  1994/01/21  19:33:46  sander
 * VCG Version tested on Silicon Graphics IRIX, IBM R6000 AIX and Sun 3/60.
 * Option handling improved. Option -grabinputfocus installed.
 * X11 Font selection scheme implemented. The user can now select a font
 * during installation.
 * Sun K&R C (a nonansi compiler) tested. Some portabitility problems solved.
 *
 */


/************************************************************************
 * The situation here is the following:
 * -----------------------------------
 * All exisiting nodes and edges come directly from the specification.
 * graphlist contains the list of all invisible subgraph summary nodes.
 * nodelist  contains the list of all visible nodes.
 * edgelist  contains the list of all edges, visible or not.
 * temporary nodes and edges are given free, 
 * i.e. tmpnodelist = tmpedgelist = NULL.
 * invis_nodes contains all nodes that are invisible because of a previous
 * edge class hiding.
 * Graphs and regions may be folded, but it is irrelevant which edges are
 * hidden, because the adjacency lists still not exist (e.g. they are removed
 * just before folding).
 *
 * Note again (see also alloc.h): 
 *   Folded graphs have a summary node in nodelist and all nodes of this
 *   graphs outside nodelist.
 *   Unfolded graphs have the nodes in nodelist and the summary node in
 *   graphlist.
 *	NSGRAPH(r) is the list of nodes of subgraph r
 *	NROOT(n) = r if n is node of subgraph r
 *   Folded regions have a summary node in nodelist, and all nodes of
 *   this region outside nodelist.
 *	NREGREPL(r) is the start node of the region which has the summary 
 *		    node r.
 *	NREGION(r)  is the list of nodes of region r (except the start node)
 *	NREGROOT(n) = r if n is invisible and node of region r
 *
 * The task of the driver function `folding' is now to create the correct
 * adjacency lists by respecting new graph/region foldings or unfoldings
 * and hidden edge classes.
 *
 * The `folding keepers' are storages to remark which nodes must be folded
 * or unfolded. The function folding folds with respect to the folding 
 * keepers. After that, it reinitializes the folding keepers.
 * The folding keepers DO NOT indicate which subgraphs or regions ARE
 * ACTUALLY FOLDED ! They indicate which actions must be done next.
 * The management of the array hide_class is different. It is initialized
 * once before reading the specification, and changed incrementally by 
 * actions. At every time point, it indicates which edge classes ARE hidden.
 *
 * After folding, the following invariants hold:
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
 *
 * This file provides the following functions:
 * ------------------------------------------
 * add_sgfoldstart	add a start point to the subgraph folding keepers
 * add_foldstart	add a start point to the region folding keepers
 * add_foldstop 	add a stop point to the region folding keepers
 * clear_hide_class	reinitialize hide_class[] (the array indicating which
 *			edge classes are hidden)
 * folding		do all foldings and create the adjacency lists
 * create_adjedge	insert an edge into the adjacency lists of its source
 *			and target node.
 * delete_adjedge	delete an adjacency edge from source and target,
 *			i.e. make the edge invisible.
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "grammar.h"
#include "alloc.h"
#include "main.h"
#include "options.h"
#include "timelim.h"
#include "drawlib.h"
#include "folding.h"
#include "steps.h"


/* Prototypes
 * ----------
 */

static void 	revert_subgraph  _PP((GNODE v));  
static int	foldstop_reached _PP((GNODE v));

static void	delete_node	_PP((GNODE v,int k));
static void	insert_node	_PP((GNODE v,int k));

static void	delete_sgnodes	_PP((GNODE u));
static void	fold_sg 	_PP((GNODE u));
static void	unfold_sg	_PP((GNODE u));
 
static void	refresh_all_nodes	_PP((GNODE v));
static void	refresh 		_PP((void));
static void 	sort_all_nodes		_PP((void));
static int      compare_ndfs		_PP((const GNODE *a,const GNODE *b));

static long	no_dfs			_PP((GNODE n));
static long	no_indeg		_PP((GNODE n));
static long	no_outdeg		_PP((GNODE n));
static long	no_degree		_PP((GNODE n));

static void	create_adjacencies	_PP((void));
static void	create_lab_adjacencies	_PP((void));
static void	adapt_labelpos		_PP((GNODE v,GEDGE e));
static GNODE	search_visible		_PP((GNODE v));
static GEDGE	substed_edge		_PP((GEDGE e));

static void	unfold_region	_PP((GNODE n));
static void	fold_region	_PP((GNODE n, int k));
static void	recursive_fold	_PP((GNODE v, GNODE n, int k));

static void	hide_edge_classes _PP((void));

static void	summarize_edges    _PP((void));
static void 	split_double_edges _PP((void));

#ifdef DEBUG
static void db_print_somenode_list(GNODE w,GNODE wend);
#endif

/*--------------------------------------------------------------------*/
/*  For Debugging only           				      */
/*--------------------------------------------------------------------*/

#ifdef NEVER 

GNODE mydebugnode;

#ifdef ANSI_C
void init_mydebugnode(void)
#else
void init_mydebugnode()
#endif
{
	GNODE v;

	mydebugnode = NULL;
	v = nodelist;
	while (v) {
		if ((NTITLE(v)) && (strcmp(NTITLE(v),"")==0)) {
			PRINTF("found in nodelist\n");
			mydebugnode = v;
		}
		v = NNEXT(v);
	}
	v = graphlist;
	while (v) {
		if ((NTITLE(v)) && (strcmp(NTITLE(v),"")==0)) {
			PRINTF("found in graphlist\n");
			mydebugnode = v;
		}
		v = NNEXT(v);
	}
	if (!mydebugnode) PRINTF("not found\n");
}

#ifdef ANSI_C
int check_mydebugnode(void)
#else
int check_mydebugnode()
#endif
{
	ADJEDGE a;

	if (!mydebugnode) return(1);

	a = NSUCC(mydebugnode);

	while (a) {
		if (SOURCE(a)!=mydebugnode) {
			printf("%d\n",SOURCE(a));
			return(0);
		}
		a   = ANEXT(a);
	}
	a = NPRED(mydebugnode);
	while (a) {
		if (TARGET(a)!=mydebugnode) {
			printf("%d\n",TARGET(a));
			return(0);
		}
		a   = ANEXT(a);
	}
	return(1);
}


#ifdef ANSI_C
static void print_all_nodes(char *x, GNODE v)
#else
static void print_all_nodes(x,v)
char *x;
GNODE	v;
#endif
{
	printf("List %s: ",x);
	while (v) {
		PRINTF("%s  ",(NTITLE(v)?NTITLE(v):"(null)"));
		v = NNEXT(v);
	}
	printf("\n");
}

#endif



/*--------------------------------------------------------------------*/
/*  Management of folding keepers				      */
/*--------------------------------------------------------------------*/

/* Global variables
 * ----------------
 */

GNLIST	f_subgraphs; /* List of subgraph nodes to folding		   */
GNLIST  uf_subgraphs;/* Node where subgraph unfolding starts		   */
GNLIST	foldstops;   /* List of nodes where a fold region operation stops  */
GNLIST	foldstart;   /* List of nodes where a fold region operation starts */
GNLIST  ufoldstart;  /* Node where region unfolding starts		   */


/*   Fold starters and stoppers
 *   ==========================
 *   Region fold starter are halfreverted, region fold stopper or
 *   subgraph stopper etc. are reverted.
 */

/* Clear all fold starter or stopper lists.
 * ---------------------------------------
 */

#ifdef ANSI_C
void clear_folding_keepers(void)
#else
void clear_folding_keepers()
#endif
{
	GNLIST l;

	/* Set folding keeper to NOREVERT */
	l = f_subgraphs;
	while (l) { NREVERT(GNNODE(l))= NOREVERT; 
		    revert_subgraph(GNNODE(l));
	      	    l = GNNEXT(l); 
		  }
	l = uf_subgraphs;
	while (l) { NREVERT(GNNODE(l))= NOREVERT; l = GNNEXT(l); }
	l = foldstops;
	while (l) { NREVERT(GNNODE(l))= NOREVERT; l = GNNEXT(l); }
	l = foldstart;
	while (l) { NREVERT(GNNODE(l))= NOREVERT; l = GNNEXT(l); }
	l = ufoldstart;
	while (l) { NREVERT(GNNODE(l))= NOREVERT; l = GNNEXT(l); }
	free_foldnodelists();
	ufoldstart  = NULL;
	foldstart   = NULL;
	foldstops   = NULL;
	f_subgraphs = NULL;
	uf_subgraphs= NULL;
}


/*   Add fold subgraph starter
 *   -------------------------
 *   Add a new fold subgraph starter v to the list foldstart.
 *   Note: v is the root node of the graph.
 */

#ifdef ANSI_C
void add_sgfoldstart(GNODE v)
#else
void add_sgfoldstart(v)
GNODE v;
#endif
{
	GNLIST	l,*lp;

	if (!v) return;
	debugmessage("add_sgfoldstart",(NTITLE(v)?NTITLE(v):"(null)"));
	if (NREVERT(v)==AREVERT) { /* delete it from sgfoldstart */
		NREVERT(v) = NOREVERT;
		l  = f_subgraphs;
		lp = &f_subgraphs;
		while (l) { 
			if (GNNODE(l)==v) { *lp = GNNEXT(l); break; }
			lp = &GNNEXT(l);
			l  = GNNEXT(l);
		}
		revert_subgraph(v);
		return;
	}
	NREVERT(v) = AREVERT;
	l = foldnodelist_alloc();
	GNNODE(l) = v;
	GNNEXT(l) = f_subgraphs;
	f_subgraphs = l;
	revert_subgraph(v);
}

/* Revert subgraph
 * ---------------
 * If a subgraph is selected for folding, all its nodes
 * are reverted, to easy the menu selection.
 * The nodes get the same revert-flag as the root node.
 */
 
#ifdef ANSI_C
static void revert_subgraph(GNODE v)  
#else
static void revert_subgraph(v)
GNODE v;  
#endif
{
        GNODE w; 
	GNLIST l;
        int rev; 
 
	debugmessage("revert_subgraph",(NTITLE(v)?NTITLE(v):"(null)"));
        rev = NREVERT(v); 
        l = NSGRAPH(v); 
 
        while (l) { 
                w = GNNODE(l); 
                NREVERT(w) = rev;  
		if (NSGRAPH(w)) {
			revert_subgraph(w);
		}
                l = GNNEXT(l); 
        } 
}
 

/*   Add unfold subgraph starter
 *   ---------------------------
 *   Add a new unfold subgraph starter v to the list foldstart.
 */

#ifdef ANSI_C
void add_sgunfoldstart(GNODE v)
#else
void add_sgunfoldstart(v)
GNODE v;
#endif
{
	GNLIST	l,*lp;

	if (!v) return;
	debugmessage("add_sgunfoldstart",(NTITLE(v)?NTITLE(v):"(null)"));
	if (NREVERT(v)==AREVERT) { /* delete it from uf_subgraphs */
		NREVERT(v) = NOREVERT;
		l  = uf_subgraphs;
		lp = &uf_subgraphs;
		while (l) { 
			if (GNNODE(l)==v) { *lp = GNNEXT(l); break; }
			lp = &GNNEXT(l);
			l  = GNNEXT(l);
		}
		return;
	}
	NREVERT(v) = AREVERT;
	l = foldnodelist_alloc();
	GNNODE(l) = v;
	GNNEXT(l) = uf_subgraphs;
	uf_subgraphs = l;
}


/*   Add fold region starter
 *   -----------------------
 *   Add a new fold region starter v to the list foldstarters.
 */

#ifdef ANSI_C
void add_foldstart(GNODE v)
#else
void add_foldstart(v)
GNODE v;
#endif
{
	GNLIST	l,*lp;

	if (!v) return;
	debugmessage("add_foldstart",(NTITLE(v)?NTITLE(v):"(null)"));

	if (NREVERT(v)==BREVERT) { /* delete it from foldstart */
		NREVERT(v) = NOREVERT;
		l  = foldstart;
		lp = &foldstart;
		while (l) { 
			if (GNNODE(l)==v) { *lp = GNNEXT(l); break; }
			lp = &GNNEXT(l);
			l  = GNNEXT(l);
		}
		return;
	}
	if (NREVERT(v)==AREVERT) return; 
	NREVERT(v) = BREVERT;
	l = foldnodelist_alloc();
	GNNODE(l) = v;
	GNNEXT(l) = foldstart;
	foldstart = l;
}

/*   Add unfold region starter
 *   -------------------------
 *   Add a new unfold region starter v to the list foldstarters.
 */

#ifdef ANSI_C
void add_unfoldstart(GNODE v)
#else
void add_unfoldstart(v)
GNODE v;
#endif
{
	GNLIST	l,*lp;

	if (!v) return;
	debugmessage("add_unfoldstart",(NTITLE(v)?NTITLE(v):"(null)"));
	if (NREVERT(v)==AREVERT) { /* delete it from ufoldstart */
		NREVERT(v) = NOREVERT;
		l  = ufoldstart;
		lp = &ufoldstart;
		while (l) { 
			if (GNNODE(l)==v) { *lp = GNNEXT(l); break; }
			lp = &GNNEXT(l);
			l  = GNNEXT(l);
		}
		return;
	}
	NREVERT(v) = AREVERT;
	l = foldnodelist_alloc();
	GNNODE(l) = v;
	GNNEXT(l) = ufoldstart;
	ufoldstart = l;
}


/*   Add fold region stopper
 *   -----------------------
 *   Add a new fold region stopper v to the list foldstops.
 */

#ifdef ANSI_C
void add_foldstop(GNODE v)
#else
void add_foldstop(v)
GNODE v;
#endif
{
	GNLIST	l,*lp;

	if (!v) return;
	debugmessage("add_foldstop",(NTITLE(v)?NTITLE(v):"(null)"));
	if (NREVERT(v)==AREVERT) { /* delete it from foldstops */
		NREVERT(v) = NOREVERT;
		l  = foldstops;
		lp = &foldstops;
		while (l) { 
			if (GNNODE(l)==v) { *lp = GNNEXT(l); break; }
			lp = &GNNEXT(l);
			l  = GNNEXT(l);
		}
		return;
	}
	NREVERT(v) = AREVERT;
	l = foldnodelist_alloc();
	GNNODE(l) = v;
	GNNEXT(l) = foldstops;
	foldstops = l;
}


/*   Check fold stopper
 *   ------------------
 *   Returns 1 if v is in the list foldstops, i.e. if v is a fold stopper.
 */
 
#ifdef ANSI_C
static int foldstop_reached(GNODE v)
#else
static int foldstop_reached(v)
GNODE	v;
#endif
{
	GNLIST	l;

	if (!v) return(1);
	debugmessage("foldstop_reached",(NTITLE(v)?NTITLE(v):"(null)"));
	l = foldstops;
	while (l) {
		if (GNNODE(l) == v) return(1);
		l = GNNEXT(l);
	}
	return(0);
}


/*--------------------------------------------------------------------*/
/*  Folding and creation of adjacency lists			      */
/*--------------------------------------------------------------------*/

/*  Defines
 *  -------
 *  To have the possibility to revert foldings, or to find
 *  appropritate substitution node, we must notate why a node
 *  is invisible. Thus we set the INVISIBLE flag:
 *	0   ->	 the node is visible
 *	1   ->	 the node is a unfolded subgraph
 *	2   ->	 the node is part of a folded subgraph
 *	3   ->	 the node is part of a folded region
 *	4   ->	 the node is invisible because it is only reacheable 
 *		 by hidden edge classes
 */

#define UNFOLDED_SGRAPH 1
#define FOLDED_SGNODE	2
#define FOLDED_RGNODE	3
#define HIDDEN_CNODE	4


/*   Folding main driver
 *   ===================
 *   We try to do as much as possible BEFORE the adjacency lists are
 *   created, because given all visible nodes, it is relatively simple
 *   to create the adjacency lists of those edges that are visible.
 *   We do subgraph folding, subgraph unfolding and region unfolding
 *   before the adjacency lists are created.
 *   However, to fold subgraphs or to hide edges, we need the adjacency
 *   lists, thus this is done later.
 *   Note that there is no operation `unhide edge class' because
 *   adjacency lists are always created new which automatically respects
 *   only those edges that are unhidden. 
 *   At last, we check whether all visible nodes have (x,y) locations,
 *   and set the locFlag according.
 *
 *   The fold stoppers are set by the specification
 *   or by menu interaction.
 */

#ifdef ANSI_C
void	folding(void)
#else
void	folding()
#endif
{
	GNODE	v,vn;
	GNLIST	l;
	int	rclass;

	debugmessage("folding","");

	assert((labellist == NULL));
	assert((tmpnodelist == NULL));
	assert((tmpedgelist == NULL));

	if (G_displayel==NO) edge_label_phase=0;
	if (G_dirtyel==YES)  edge_label_phase=0;

	/* 1) make all node that are invisible because of edge classes 
	 *    visible 
	 */

	gs_wait_message('f');
	v = invis_nodes;
	while (v) {
		vn = NNEXT(v);	
		insert_node(v,HIDDEN_CNODE);
		v = vn; 	/* because insert_node changes NNEXT(v) */
	}
	invis_nodes = NULL;


	/* 2) subgraph folding */

	l = f_subgraphs;
	while (l) {
		fold_sg(GNNODE(l));
		l = GNNEXT(l);
	}

	/* 3) subgraph unfolding */

	l = uf_subgraphs;
	while (l) {
		unfold_sg(GNNODE(l));
		l = GNNEXT(l);
	}

	/* 4) Unfold region (independent of the class) */

	l = ufoldstart;
	while (l) {
		unfold_region(GNNODE(l));
		l = GNNEXT(l);
	}

	/* 5) refresh the situation: initialize indegree of nodes,
	 *    visibility flag of edges etc. Clear adjacency lists.
	 */

	gs_wait_message('f');
	refresh();

	/* 6) Construct adjacency lists first time */

	create_adjacencies();
	hide_edge_classes();

	/* 7) Fold region: fold first the lower then the higher classes */


	for (rclass=0; rclass<17; rclass++) {
		l = foldstart;
		while (l) {
			if (GNNODE(l) && (NFOLDING(GNNODE(l))==rclass))
				fold_region(GNNODE(l),rclass);
			l = GNNEXT(l);
		}
	}

	/* 8) If labels necessary, create labels */

	refresh();
	if ((G_displayel==YES) && (G_dirtyel==NO) && (edge_label_phase==0)) 
		create_lab_adjacencies();
	else 	create_adjacencies();

	/* 9) Hide edge classes */

	gs_wait_message('f');
	hide_edge_classes();


	/* For stable layout: sort nodelist */
	sort_all_nodes(); 

	/* 10) check whether all nodes have already locations 
	 *     and transfer the specified locations.
	 */

	locFlag = 1;
	v = nodelist;
	while (v && locFlag) {
		if ((NSX(v)==0L) && (NSY(v)==0L)) locFlag=0;
		NX(v) = NSX(v);
		NY(v) = NSY(v);
		v = NNEXT(v);
	}

	/* 11) If labels, we calculate edge positions without labels.
 	 */

	if (locFlag && (G_displayel==YES)) {
		free_tmpnodes();
		refresh();
		create_adjacencies();
		gs_wait_message('f');
		hide_edge_classes();
		v = nodelist;
		while (v) {
			NX(v) = NSX(v);
			NY(v) = NSY(v);
			v = NNEXT(v);
		}
	}

	/* 12) If necessary, remove double edges from the adjacency
	 *     lists.
	 */
	
	gs_wait_message('f');
	if (summarize_double_edges) summarize_edges();


	/* 13) Otherwise split edges which are doubled, such that they
	 *     can be recognized better.
	 */

	if ((!locFlag) && (!summarize_double_edges))
		split_double_edges();

	/* 14) Reinit the folding keepers to get a clean situation
	 *     for the next folding.
	 */

	clear_folding_keepers();
}


/*--------------------------------------------------------------------*/
/*  Folding primitives						      */
/*--------------------------------------------------------------------*/

/*   Insertion and deletion at the node list
 *   =======================================
 *   As long as the adjacency lists are not created, folding and is
 *   simply done by insertion and deletion of nodes at the nodelist.
 *   With this method, `fold subgraph' and `unfold subgraph' and
 *   `unfold region' can be implemented.
 *   To have the possibility to revert such foldings, or to find
 *   appropritate substitution node (see substed_edge), we must notate 
 *   why a node is invisible. Thus we set the INVISIBLE flag:
 *	0   ->	 the node is visible
 *	1   ->	 the node is a unfolded subgraph
 *	2   ->	 the node is part of a folded subgraph
 *	3   ->	 the node is part of a folded region
 *	4   ->	 the node is invisible because it is only reacheable 
 *		 by hidden edge classes
 *   See above the defines of 
 *	UNFOLDED_SGRAPH, FOLDED_SGNODE, FOLDED_RGNODE, HIDDEN_CNODE 
 */

/* Abbreviation to delete and insert nodes in double linked lists */

#define del_node_from_dl_list(v,l,le) { \
	if (NBEFORE(v)) NNEXT(NBEFORE(v)) = NNEXT(v); \
	else		l		  = NNEXT(v); \
	if (NNEXT(v)) NBEFORE(NNEXT(v)) = NBEFORE(v); \
	else	      le		= NBEFORE(v); \
}

#define ins_node_in_dl_list(v,l,le) { \
	NBEFORE(v)   = le;     \
	if (le) NNEXT(le) = v; \
	le = v; 	       \
	if (!l) l = v;	       \
}


/*   Delete a node v from the nodelist
 *   ---------------------------------
 *   The node becomes invisible.
 *   The invisibly flag is set to k, which is the reason why the node
 *   is invisible.
 */

#ifdef ANSI_C
static void	delete_node(GNODE v,int k)
#else
static void	delete_node(v,k)
GNODE	v;
int k;
#endif
{
	assert((v));
	assert((k!=0));
	debugmessage("delete_node",(NTITLE(v)?NTITLE(v):"(null)"));
	if (!NINLIST(v)) return;
	NINLIST(v) = 0;
	NINVISIBLE(v) = k;
	del_node_from_dl_list(v,nodelist,nodelistend);
	NNEXT(v)   = NULL;
	NBEFORE(v) = NULL;
	if (NSGRAPH(v)) { /* a subgraph comes back into the graph list */
		if (k==UNFOLDED_SGRAPH) {
			ins_node_in_dl_list(v,graphlist,graphlistend);
		}
	}
	nodeanz--;

}


/*   Inserts a node v into the nodelist
 *   ----------------------------------
 *   The node becomes visible.
 *   The reason of becoming visible is, that invisibility reason k
 *   is not anymore valid. If k was not the invisibility reason,
 *   then the insertion cannot be done.
 */

#ifdef ANSI_C
static void	insert_node(GNODE v,int k)
#else
static void	insert_node(v,k)
GNODE	v;
int k;
#endif
{
	assert((v));
	assert((k!=0));
	debugmessage("insert_node",(NTITLE(v)?NTITLE(v):"(null)"));
	if (NINLIST(v)) return;
	if (NINVISIBLE(v)!=k) return;
	NINLIST(v) = 1;
	NINVISIBLE(v) = 0;
	if (NSGRAPH(v)) { /* a subgraph must be removed from the graph list */
		if (k==UNFOLDED_SGRAPH) {
			del_node_from_dl_list(v,graphlist,graphlistend);
		}
	}
	NNEXT(v)   = NULL;
	NBEFORE(v) = NULL;
	ins_node_in_dl_list(v,nodelist,nodelistend);
	nodeanz++;

}


/*--------------------------------------------------------------------*/
/*  Folding of subgraphs					      */
/*--------------------------------------------------------------------*/

/*  Delete all nodes of a subgraph from the nodelist
 *  ------------------------------------------------
 *  i.e. it hides the subgraph u (and all its subgraphs too).
 */

#ifdef ANSI_C
static void	delete_sgnodes(GNODE u)
#else
static void	delete_sgnodes(u)
GNODE	u;
#endif
{
	GNODE	v;
	GNLIST	s;

	if (!u) return;
	debugmessage("delete_sgnodes",(NTITLE(u)?NTITLE(u):"(null)"));
	s = NSGRAPH(u);
	while (s) {
		v = GNNODE(s);
		delete_node(v,FOLDED_SGNODE);
		if (NSGRAPH(v)) delete_sgnodes(v);
		s = GNNEXT(s);
	}
}


/*   Fold a subgraph
 *   ---------------
 *   u is the root node of the subgraph
 *   Folding means: delete all nodes of this subgraph from the nodelist
 *   and insert the summary node.
 */

#ifdef ANSI_C
static void	fold_sg(GNODE u)
#else
static void	fold_sg(u)
GNODE	u;
#endif
{
	if (!u) return;
	debugmessage("fold_sg",(NTITLE(u)?NTITLE(u):"(null)"));

	delete_sgnodes(u);
	insert_node(u,UNFOLDED_SGRAPH);
}


/*   Unfold a subgraph
 *   -----------------
 *   u is the root node of the subgraph
 *   Unfolding means: delete this subgraph summary node from the nodelist
 *   and insert all its nodes.
 */


#ifdef ANSI_C
static void	unfold_sg(GNODE	u)
#else
static void	unfold_sg(u)
GNODE	u;
#endif
{
	GNODE	v;
	GNLIST	s;

	if (!u) return;
	debugmessage("unfold_sg",(NTITLE(u)?NTITLE(u):"(null)"));

	s = NSGRAPH(u);
	while (s) {
		v = GNNODE(s);
		insert_node(v,FOLDED_SGNODE);

		/* If v was an unfolded subgraph before the folding,
		 * it was before not visible, thus it is not inserted,
		 * since the insert reason FOLDED_SGNODE is not the
		 * delete reason UNFOLDED_SG. Then we have to unfold its
		 * subnodes, too.
		 */
		if (NSGRAPH(v) && (NINVISIBLE(v)==UNFOLDED_SGRAPH)) 
			unfold_sg(v);
		s = GNNEXT(s);
	}
	delete_node(u,UNFOLDED_SGRAPH);
}


/*--------------------------------------------------------------------*/
/*  Folding of regions						      */
/*--------------------------------------------------------------------*/

/*   Note that region operations are not inverse implemented, even
 *   if the effect is inverse.
 *   The reason is, that unfolding is done BEFORE the adjacency lists
 *   exist, but folding can only be done AFTER the adjacency lists exist.
 *   Firther we use a dirty trick here: instead of creating a summary
 *   node of the region, we use the start node of the region as summary
 *   node. This has the advantage that the summary node is in the correct
 *   subgraph, and that we have no problems with the hashtable to
 *   search node title. An auxiliary node is created as REGREPL, to contain
 *   the information of the originally start node. This must be done
 *   because the originally start node could be a summary node of a
 *   previous region folding.
 */


/*   Fold Region
 *   -----------
 *   Starting from a node n, all nodes and edges reacheable by 
 *   edges of class <= k are folded.
 *   Before, the adjacency lists were created.
 */

#ifdef ANSI_C
static void	fold_region(GNODE n,int	k)
#else
static void	fold_region(n,k)
GNODE	n;
int	k;
#endif
{
	ADJEDGE a;
	GEDGE	e;
	GNODE	h;


	assert((n));
	debugmessage("fold_region",(NTITLE(n)?NTITLE(n):"(null)"));

	/* First, we create a stable replacement node, that stores 
	 * all information of the node. Then we use the start node
	 * as summary node, because the start node is part of the 
	 * right graph.
	 */

	h = nodealloc(n);
	NTITLE(h)   = NTITLE(n);
	NROOT(h)    = NROOT(n);
	NREGREPL(h) = NREGREPL(n);
	NREGION(h)  = NREGION(n);
	NREGROOT(h) = NREGROOT(n);

	NSX(n) = NSY(n) = 0L;   /* Locations are not inherited ! */ 

	NWIDTH(n) = NHEIGHT(n) = -1; /* Sizes are not inherited */

	delete_node(h,FOLDED_RGNODE);	      /* h is invisible */ 

	/* Now: h has the purpose of n, and n is the summary node 
	 * Now we change the properties of the summary nodes according
	 * to the defaults.
	 */

	inherit_foldnode_attributes(&foldnode, n);

	NREGREPL(n) = h;
	NREGION(n)  = NULL;   /* here we collect the nodes of this region */

	/* Fold the nodes recursively */
	a = NSUCC(n);
	while (a) {
		e = AKANTE(a);
		if ( ECLASS(e)<=k ) {
			if ( !foldstop_reached(EEND(e)) ) {
				EINVISIBLE(e) = 1;
				recursive_fold(EEND(e),n,k);
			}
		}
		a = ANEXT(a);
	}
}

/*  Recursive fold region
 *  ---------------------
 *  This is an auxiliary function of fold_region. 
 *  It folds the region of class <=k starting at v. The summary node
 *  of the region is n. It makes the nodes of the region invisible,
 *  and calculates substitutions for edges.
 *  The function is a little bit inefficient if we have cross edges
 *  inside the folded region, because these might be substituted
 *  and the substed will be later removed. 
 */

#ifdef ANSI_C
static void	recursive_fold(GNODE v,GNODE n,int k)
#else
static void	recursive_fold(v,n,k)
GNODE	v;
GNODE	n;
int	k;
#endif
{
	ADJEDGE a;
	GEDGE	e,ee;
	GNLIST	l;

	assert((v));
	assert((n));
	debugmessage("recursive_fold",(NTITLE(v)?NTITLE(v):"(null)"));

	/* Check of cycle: both checks means the same */
	if ( !NINLIST(v) ) return;
	if ( NREGROOT(v) == n )  return;

	/* Check of cycle: this is the node we started folding */
	if ( v == n )            return;

	/* Add v to the region list of n */
	NREGROOT(v) = n;
	l = nodelist_alloc(v);
	if ( !NREGION(n) ) {
		GNNEXT(l)  = NULL;
		NREGION(n) = l;
	}
	else {	GNNEXT(l)  = NREGION(n);
		NREGION(n) = l;
	}
	delete_node(v,FOLDED_RGNODE);

	/* Go into the recursion */
	a = NSUCC(v);
	while (a) {
		e = AKANTE(a); 
		if ( ECLASS(e) <= k ) {
			if ( !foldstop_reached(EEND(e)) ) {
				EINVISIBLE(e) = 1;
				recursive_fold(EEND(e),n,k);
			}
		}
		a = ANEXT(a);
	}

	/* Substitute the predecessor edges of v */
	a = NPRED(v);
	while (a) {
		e = AKANTE(a); 
		ee = substed_edge(e);
		if (ee!=e) {
			/* Edge e invisible or substituted: 
			 * Remove edge e from adjacency list of the source. 
			 */
			delete_adjedge(e);
			/* and insert new edge, but avoid self loops at
			 * the region node. 
			 */
			assert((!ee)||(EEND(ee)==n));
			if (ee && (ESTART(ee)!=n)) create_adjedge(ee);
		}
		a = ANEXT(a);
	}
	
	/* Substitute the successor edges of v */
	a = NSUCC(v);
	while (a) {
		e = AKANTE(a); 
		ee = substed_edge(e);
		if (ee!=e) {
			/* Edge e invisible or substituted: 
			 * Remove edge e from adjacency list of the source. 
			 */
			delete_adjedge(e);
			/* and insert new edge, but avoid self loops at
			 * the region node.
			 */
			assert((!ee)||(ESTART(ee)==n));
			if (ee && (EEND(ee)!=n)) create_adjedge(ee);
		}
		a = ANEXT(a);
	}
	NPRED(v) = NSUCC(v) = NULL;  /* because v is invisible */
}


/*   Unfold Region
 *   -------------
 *   Different than "fold region", this is done BEFORE the adjacency
 *   lists exist. Unfold the region that has summary node n.
 */

#ifdef ANSI_C
static void	unfold_region(GNODE n)
#else
static void	unfold_region(n)
GNODE	n;
#endif
{
	GNLIST	l, startl;
	GNODE	h;

	assert((n));
	debugmessage("unfold_region",(NTITLE(n)?NTITLE(n):"(null)"));

	if (NREGREPL(n)==NULL) return; /* it was no region */

	h = NREGREPL(n);
	startl = l = NREGION(n);

	/* Make n again a normal node, i.e. restore the attributes from h. */

	NLABEL(n)    = NLABEL(h);
	NTEXTMODE(n) = NTEXTMODE(h); 
	NWIDTH(n)    = NWIDTH(h);
	NHEIGHT(n)   = NHEIGHT(h); 
	NBORDERW(n)  = NBORDERW(h);
	NSX(n)	     = NSX(h);		
	NSY(n)	     = NSY(h);	       
	NFOLDING(n)  = NFOLDING(h); 
	NCOLOR(n)    = NCOLOR(h); 
	NTCOLOR(n)   = NTCOLOR(h); 
	NBCOLOR(n)   = NBCOLOR(h);  
	NSHRINK(n)   = NSHRINK(h);  
	NSTRETCH(n)  = NSTRETCH(h);
	NINFO1(n)    = NINFO1(h);
	NINFO2(n)    = NINFO2(h);
	NINFO3(n)    = NINFO3(h);
	NLEVEL(n)    = NLEVEL(h);
	NSHAPE(n)    = NSHAPE(h);
	NHORDER(n)   = NHORDER(h);
	NROOT(n)     = NROOT(h);
	NREGREPL(n)  = NREGREPL(h);
	NREGION(n)   = NREGION(h);
	NREGROOT(n)  = NREGROOT(h);

	free_node(h);	      /* give h free */ 

	while (l) {
		h = GNNODE(l);
		NREGROOT(h) = NULL;
		insert_node(h,FOLDED_RGNODE);
		l = GNNEXT(l);
	}

	free_regionnodelist(startl); /* give the region cons cells free */
}

/*--------------------------------------------------------------------*/
/*  Hiding of edges						      */
/*--------------------------------------------------------------------*/

/*   Management of Edge Classes
 *   ==========================
 *   The hidden edge classes i have hide_class[i]=1.
 *   The nodes from the nodelist that are hidden are pushed into
 *   the list invis_nodes.
 *   At the beginning of the layout process, these nodes are 
 *   repushed into the nodelist.
 */

int 	*hide_class=NULL; 	/* table which edge classes are hidden*/


/*  Initialization
 *  -------------- 
 *  All classes are visible, no class is hidden.
 */

#ifdef ANSI_C
void	clear_hide_class(void)
#else
void	clear_hide_class()
#endif
{
	int	i;

	debugmessage("clear_hide_class","");
	if (!hide_class) return;
	for (i=0;i<max_nr_classes;i++) hide_class[i] = 0;
}


/*  Insert node into the list invis_nodes.
 *  -------------------------------------
 *  If the node is in the node list, remove it from the node list
 *  and push it into the list invis_nodes.
 *  We collect the temporary hidden nodes that come not to invis_nodes
 *  into the list tmpinvis_nodes. This list is only internally used
 *  in hide_edge_classes. 
 */

static GNODE tmpinvis_nodes;

#ifdef ANSI_C
static void	hide_node(GNODE v)
#else
static void	hide_node(v)
GNODE v;
#endif
{
	debugmessage("hide_node","");

	/* remove node from the node list */
	if (!NINLIST(v)) {
		del_node_from_dl_list(v,labellist,labellistend); 
		NBEFORE(v) = NULL;
		NNEXT(v) = tmpinvis_nodes;
		tmpinvis_nodes = v;
	} 
	else {
		delete_node(v,HIDDEN_CNODE);
		NNEXT(v) = invis_nodes;
		invis_nodes = v;
	}
}


/*  Hide edges and node
 *  -------------------
 */
  
#ifdef ANSI_C
static void	hide_edge_classes(void)
#else
static void	hide_edge_classes()
#endif
{
	GEDGE h;
	GNODE v,w;
	ADJEDGE a,b;
	int	allhidden;

	debugmessage("hide_edge_classes","");
	tmpinvis_nodes = NULL;
	v = nodelist;
	while (v) {
		w = NNEXT(v);
		if ((NPRED(v)!=NULL)||(NSUCC(v)!=NULL)) {
			allhidden = 1;
			a = NPRED(v);
			while (a && allhidden) {
				h = AKANTE(a);
				assert(ECLASS(h)>0);
				if (!hide_class[ECLASS(h)-1]) allhidden=0; 
				a = ANEXT(a);
			}
			a = NSUCC(v);
			while (a && allhidden) {
				h = AKANTE(a);
				assert(ECLASS(h)>0);
				if (!hide_class[ECLASS(h)-1]) allhidden=0; 
				a = ANEXT(a);
			}
			if (allhidden) hide_node(v);
		}
		else { if (hide_single_nodes) hide_node(v); }
		v = w;
	}
	v = labellist;
	while (v) {
		w = NNEXT(v);
		if ((NPRED(v)!=NULL)||(NSUCC(v)!=NULL)) {
			allhidden = 1;
			a = NPRED(v);
			while (a && allhidden) {
				h = AKANTE(a);
				assert(ECLASS(h)>0);
				if (!hide_class[ECLASS(h)-1]) allhidden=0; 
				a = ANEXT(a);
			}
			a = NSUCC(v);
			while (a && allhidden) {
				h = AKANTE(a);
				assert(ECLASS(h)>0);
				if (!hide_class[ECLASS(h)-1]) allhidden=0; 
				a = ANEXT(a);
			}
			if (allhidden) hide_node(v);
		}
		else { if (hide_single_nodes) hide_node(v); }
		v = w;
	}
	/* I assume that the following is not anymore necessary: */
	v = invis_nodes;
	while (v) {
		/* delete all outgoing edges */
		a = NSUCC(v);
		while (a) {
			b = ANEXT(a);	
			h = AKANTE(a);
			assert(h);
			if (!EINVISIBLE(h)) delete_adjedge(h);
			EINVISIBLE(h) = 1;
			a = b;	
		}
		v = NNEXT(v);
	}		
	/* I assume that the following is not anymore necessary:
	 * It may be that the edge of a labeled node is not hidden
	 * because of the class, but because it is the successor of a 
	 * hidden node. Then the label is useless because its edge
	 * is deleted. We delete now the label, too.
	 */
	v = labellist;
	while (v) {
		w = NNEXT(v);
		a = NPRED(v);
		if (!a) hide_node(v); 
		v = w;
	}
	/* I assume that the following is not anymore necessary: */
	v = tmpinvis_nodes;
	while (v) {
		/* delete all outgoing edges */
		a = NSUCC(v);
		while (a) {
			b = ANEXT(a);	
			h = AKANTE(a);
			assert(h);
			if (!EINVISIBLE(h)) delete_adjedge(h);
			EINVISIBLE(h) = 1;
			a = b;	
		}
		v = NNEXT(v);
	}		
		
	h = edgelist;
	while (h) {
		assert(ECLASS(h)>0);
		if (hide_class[ECLASS(h)-1]) {
			if (!EINVISIBLE(h)) delete_adjedge(h);
			EINVISIBLE(h) = 1;
		}
		h = ENEXT(h);
	}
	h = tmpedgelist;
	while (h) {
		assert(ECLASS(h)>0);
		if (hide_class[ECLASS(h)-1]) {
			if (!EINVISIBLE(h)) delete_adjedge(h);
			EINVISIBLE(h) = 1;
		}
		h = EINTERN(h);
	}
}


/*--------------------------------------------------------------------*/
/*  Refresh the situation					      */
/*--------------------------------------------------------------------*/

/*   Refresh the whole graph
 *   =======================
 *   i.e. refresh all nodes and edges existing before the layouting.
 *   All layout relevent attributes must become undefined and the
 *   adjacency list must be cleared.
 *   Edges must be reverted and must made visible.
 *   Note: this should be done when maximal many nodes are in the node
 *   list, i.e. no unfolding is done anymore, but before the creation
 *   of adjacency lists.
 */

#ifdef ANSI_C
static void	refresh(void)
#else
static void	refresh()
#endif
{
	GNODE	v;
	GEDGE	e;
	char 	hh;

	debugmessage("refresh","");

	assert((labellist    == NULL));
	assert((labellistend == NULL));
	assert((tmpnodelist == NULL));

	/* Clear layout attributes of all nodes */

	refresh_all_nodes(nodelist);
	refresh_all_nodes(graphlist);
	refresh_all_nodes(invis_nodes);

	dummyanz  = 0;	/* no dummy nodes available  */

	/* Revert reverted edges and make them visible */

	e = edgelist;
	while (e) {
		if (EART(e) == 'R') {
			v	  = ESTART(e);
			ESTART(e) = EEND(e);
			EEND(e)   = v;
        		hh = EARROWSIZE(e);
   		        EARROWSIZE(e)  = EARROWBSIZE(e);
        		EARROWBSIZE(e) = hh;
        		hh = EARROWCOL(e);
        		EARROWCOL(e)  = EARROWBCOL(e);
        		EARROWBCOL(e) = hh;
        		hh = EARROWSTYLE(e);
        		EARROWSTYLE(e)  = EARROWBSTYLE(e);
        		EARROWBSTYLE(e) = hh;
		}
		EART(e) 	= 'U';
		ELNODE(e) 	= NULL;
		EINVISIBLE(e)	= 0;
		EORI(e) 	= NO_ORI;
		EORI2(e)	= NO_ORI;
		e = ENEXT(e);
	}
}


/*  Refresh all nodes of a list
 *  ---------------------------
 *  Given a nodelist v, connected via NNEXT, all nodes become
 *  undefined deepth, position, mark, indegree, outdegree.
 *  Old adjacency list are cleared.
 */

#ifdef ANSI_C
static void refresh_all_nodes(GNODE v)
#else
static void refresh_all_nodes(v)
GNODE	v;
#endif
{
	debugmessage("refresh_all_nodes","");
	while (v) {
		NTIEFE(v)	= -1;
		NPOS(v) 	= -1;
		NMARK(v)	= 0;
		NREVERT(v)	= NOREVERT;
		NINDEG(v)	= 0;
		NOUTDEG(v)	= 0;
		NDFS(v) 	= 0L;
		NX(v) 		= 0L;
		NY(v) 		= 0L;
		NCONNECT(v)	= NULL;
		NSUCC(v)	= NPRED(v)	= NULL;
		NSUCCL(v)	= NSUCCR(v)	= NULL;
		NPREDL(v)	= NPREDR(v)	= NULL;
		v = NNEXT(v);
	}
}

/*  Sort all nodes
 *  --------------
 *  We sort the nodes according to the a criterium. Because the ordering 
 *  influences the layout, this increases the stability of the layout,
 *  i.e. after folding of a small subpart, the layout of the large
 *  graph part does not change arbitrary.
 *
 *  The criterium is the following:
 *  
 *  layout_flag = 0, 1, 2, 3   -> we use the NREFNUM of the nodes.
 *  layout_flag = 4            -> we use the reverse dfs-depth starting 
 *				  at that node
 *  layout_flag = 5            -> we use the dfs-depth starting 
 *				  at that node
 *  layout_flag = 6            -> we use the indegree of that node.
 *  layout_flag = 7            -> we use the reverse indegree of that node.
 *  layout_flag = 8            -> we use the outdegree of that node.
 *  layout_flag = 9            -> we use the reverse outdegree of that node.
 *  layout_flag = 10           -> we use the degree of that node.
 *  layout_flag = 11           -> we use the reverse degree of that node.
 */ 

static GNODE *node_sort_array = NULL;
static int    noso_size = 0;

#ifdef ANSI_C
static void sort_all_nodes(void)
#else
static void sort_all_nodes()
#endif
{
	GNODE v, w;
	int   i,max;


	/* In order to reach stability, we first sort the nodes
	 * according to their refnum.
	 */

	v = nodelist;
	i = 0;
	while (v) { i++; NDFS(v) = NREFNUM(v); v = NNEXT(v); }
	
	max = i;
	if (max < 2) return;
        if (max+2 > noso_size) {
                if (node_sort_array) free(node_sort_array);
                node_sort_array = (GNODE *)malloc((max+2)*sizeof(GNODE));
                if (!node_sort_array) Fatal_error("memory exhausted","");
                noso_size = max+2;
#ifdef DEBUG
                PRINTF("Sizeof table `node_sort_array': %ld Bytes\n",
                        (max+2)*sizeof(GNODE));
#endif
        }
	v = nodelist;
	i = 0;
	while (v) {
		node_sort_array[i++] = v;			
		v = NNEXT(v);
	}

#ifdef ANSI_C
	qsort(node_sort_array,max,sizeof(GNODE),
		(int (*) (const void *, const void *))compare_ndfs);
#else
	qsort(node_sort_array,max,sizeof(GNODE), compare_ndfs);
#endif

	/* Now check whether the 10 % time limit is exceeded */

	if (G_timelimit>0) { 
		if (test_timelimit(10)) {
			layout_flag = 1;
			gs_wait_message('t');
		}
	}
	

	/* Now, we sort according to a second criterium, e.g. degree, etc.
	 */

	v = nodelist;
	switch (layout_flag) {
	case 4: while (v) { 
			if (G_timelimit>0) { 
				if (test_timelimit(15)) {
					gs_wait_message('t');
					layout_flag = 1;
					break; 
				}
			}
			w = nodelist;
			while (w) { NMARK(w) = 0; w = NNEXT(w); }
			NDFS(v) =  - no_dfs(v); 
			v = NNEXT(v); 
		}
		break;
	case 5: while (v) { 
			if (G_timelimit>0) { 
				if (test_timelimit(15)) {
					gs_wait_message('t');
					layout_flag = 1;
					break; 
				}
			}
			w = nodelist;
			while (w) { NMARK(w) = 0; w = NNEXT(w); }
			NDFS(v) =  no_dfs(v); 
			v = NNEXT(v); 
		}
		break;
	case 6: while (v) { 
			NDFS(v) =  no_indeg(v); 
			v = NNEXT(v); 
		}
		break;
	case 7: while (v) { 
			NDFS(v) =  - no_indeg(v); 
			v = NNEXT(v); 
		}
		break;
	case 8: while (v) { 
			NDFS(v) =  no_outdeg(v); 
			v = NNEXT(v); 
		}
		break;
	case 9: while (v) { 
			NDFS(v) =  - no_outdeg(v); 
			v = NNEXT(v); 
		}
		break;
	case 10:
		while (v) { 
			NDFS(v) =  no_degree(v); 
			v = NNEXT(v); 
		}
		break;
	case 11:
		while (v) { 
			NDFS(v) =  - no_degree(v); 
			v = NNEXT(v); 
		}
		break;
	}
	
	switch (layout_flag) {
	case 4: case 5: case 6: case 7: case 8:
	case 9: case 10: case 11:
#ifdef ANSI_C
		qsort(node_sort_array,max,sizeof(GNODE),
			(int (*) (const void *, const void *))compare_ndfs);
#else
		qsort(node_sort_array,max,sizeof(GNODE), compare_ndfs);
#endif
	}

	for (i=1; i<max-1; i++) {
		NNEXT(node_sort_array[i]) = node_sort_array[i+1];
		NBEFORE(node_sort_array[i]) = node_sort_array[i-1];
	}
	NNEXT(node_sort_array[0])       = node_sort_array[1];
	NBEFORE(node_sort_array[0])     = NULL;
	NNEXT(node_sort_array[max-1])   = NULL;
	NBEFORE(node_sort_array[max-1]) = node_sort_array[max-2];
	nodelist    = node_sort_array[0];
	nodelistend = node_sort_array[max-1];

	v = nodelist;
	while (v) { NDFS(v) = 0L; NMARK(v) = 0; v = NNEXT(v); }
}


/*  Compare function for sorting according dfs values
 *  -------------------------------------------------
 *  returns 1 if NDFS(*a) > NDFS(*b), 0 if equal, -1 otherwise.
 */
 
#ifdef ANSI_C
static int      compare_ndfs(const GNODE *a,const GNODE *b)
#else
static int      compare_ndfs(a,b)
GNODE   *a;
GNODE   *b;
#endif
{
        if (NDFS(*a) > NDFS(*b))                return(1);
        if (NDFS(*a) < NDFS(*b))                return(-1);
        return(0);
}


/*  Set the dfs value of a node
 *  ---------------------------
 *  The idea is that we look for the maximal depth of the dfs spanning 
 *  tree starting at that node. 
 *  If we later sort the nodes according to the negative dfs value,
 *  The first nodes are the nodes that create a maximal depth layout,
 *  because their dfs value is maximal.
 *  Conversly, using positive dfs values, we get a minimal depth
 *  layout.
 */

#ifdef ANSI_C
static long	no_dfs(GNODE n)
#else
static long	no_dfs(n)
GNODE n;
#endif
{
	ADJEDGE edge;
	long res, z;

	debugmessage("no_dfs","");

	if (NMARK(n)) return(0L);
	NMARK(n) = 1;
	edge = NSUCC(n);
	res = 0L;
	while (edge) {
		z =  no_dfs(TARGET(edge))+1L;
		if (z>res)  res = z;
		edge = ANEXT(edge);
	}
	return(res);
}


/* Calculate the indegree of a node
 * --------------------------------
 */

#ifdef ANSI_C
static long	no_indeg(GNODE n)
#else
static long	no_indeg(n)
GNODE n;
#endif
{
	ADJEDGE edge;
	long res;

	debugmessage("no_indeg","");

	edge = NPRED(n);
	res = 0L;
	while (edge) { res++; edge = ANEXT(edge); }
	return(res);
}


/* Calculate the outdegree of a node
 * --------------------------------
 */

#ifdef ANSI_C
static long	no_outdeg(GNODE n)
#else
static long	no_outdeg(n)
GNODE n;
#endif
{
	ADJEDGE edge;
	long res;

	debugmessage("no_outdeg","");

	edge = NSUCC(n);
	res = 0L;
	while (edge) { res++; edge = ANEXT(edge); }
	return(res);
}


/* Calculate the degree of a node
 * --------------------------------
 */

#ifdef ANSI_C
static long	no_degree(GNODE n)
#else
static long	no_degree(n)
GNODE n;
#endif
{
	ADJEDGE edge;
	long res;

	debugmessage("no_degree","");

	res = 0L;
	edge = NPRED(n);
	while (edge) { res++; edge = ANEXT(edge); }
	edge = NSUCC(n);
	while (edge) { res++; edge = ANEXT(edge); }
	return(res);
}


/*--------------------------------------------------------------------*/
/*  Creation and management of adjacency lists			      */
/*--------------------------------------------------------------------*/

/*  Primitives
 *  ==========
 */


/*   Create an adjacency
 *   -------------------
 *   i.e. insert an edge into the adjacency lists of its source and
 *   target node.
 */

#ifdef ANSI_C
void	create_adjedge(GEDGE edge)
#else
void	create_adjedge(edge)
GEDGE	edge;
#endif
{
	ADJEDGE a;
	debugmessage("create_adjedge","");
	assert((edge));
	assert((ESTART(edge)));
	assert((EEND(edge)));
	a = NSUCC(ESTART(edge));
	while (a) {
		if (AKANTE(a)==edge) { return; }
		a   = ANEXT(a);
	}
	a = NPRED(EEND(edge));
	while (a) {
		if (AKANTE(a)==edge) { return; }
		a   = ANEXT(a);
	}

	prededgealloc(EEND(edge),edge);
	succedgealloc(ESTART(edge),edge);
	EINVISIBLE(edge) = 0;
}

/*   Delete an adjacency
 *   -------------------
 *   i.e. delete an edge e from the adjacency lists of its source
 *   and target node.
 */


#ifdef ANSI_C
void	delete_adjedge(GEDGE edge)
#else
void	delete_adjedge(edge)
GEDGE	edge;
#endif
{
	ADJEDGE a,b,*ap,*abp;

	debugmessage("delete_adjedge","");
	assert((edge));
	assert((ESTART(edge)));
	assert((EEND(edge)));
	a = NSUCC(ESTART(edge));
	ap = &(NSUCC(ESTART(edge)));
	while (a) {
		abp = &ANEXT(a);
		b   = ANEXT(a);
		if (AKANTE(a)==edge) *ap = ANEXT(a); 
		a = b;
		ap = abp;
	}
	a = NPRED(EEND(edge));
	ap = &(NPRED(EEND(edge)));
	while (a) {
		abp = &(ANEXT(a));
		b = ANEXT(a);
		if (AKANTE(a)==edge) *ap = ANEXT(a);
		a = b;
		ap = abp;
	}
	EINVISIBLE(edge) = 1;
}


 
/*   Create an label node
 *   --------------------
 *   For adjacencies with labels, we create label nodes and auxiliary
 *   edges between the label:	s -> label -> t.
 *   This simplifies the layout, because edge labels can be dealed
 *   as nodes.
 *   The new temporary node is inserted into the label list.
 */

#ifdef ANSI_C
GNODE	create_labelnode(GEDGE e)
#else
GNODE	create_labelnode(e)
GEDGE	e;
#endif
{
	GNODE	v;

	debugmessage("create_labelnode","");
	v = tmpnodealloc(CENTER,-1,-1,0,-1,
			G_color,ELABELCOL(e),ELABELCOL(e),1,1,-1);
	NSX(v) = (NSX(ESTART(e))+NSX(EEND(e)))/2L; 
	NSY(v) = (NSY(ESTART(e))+NSY(EEND(e)))/2L; 
	NTITLE(v)   = "";
	NLABEL(v)   = ELABEL(e);
	NSTRETCH(v) = 1;
	NSHRINK(v)  = 1;
	adapt_labelpos(v,e);
	NNEXT(v)   = NULL;
	NBEFORE(v) = NULL;
	ins_node_in_dl_list(v,labellist,labellistend); 
	EINVISIBLE(e)=1;
	ELNODE(e) = v;
	return(v);
}

/*   Calculate the string size of an label
 *   -------------------------------------
 *   and adapt the coordinates of v. The coordinates of labels derived
 *   from nodes that have no coordinates may be wrong; but in this
 *   case, all positions are recalculated anyway. 
 */

#ifdef ANSI_C
static void adapt_labelpos(GNODE v,GEDGE e)
#else
static void adapt_labelpos(v,e)
GNODE v;
GEDGE e;
#endif
{
	int	a,b,c;
	char	*ss;
 
	assert((v));
	b = 1;
	a = c = 0;
	ss = NLABEL(v);
	if (!ss) return;
	if (NSHRINK(v)==0) NSHRINK(v) = 1;
	gs_setshrink(NSTRETCH(v),NSHRINK(v));
	gs_calcstringsize(ss);
	gs_stringw += (ETHICKNESS(e)/2+1);
	gs_stringh += (ETHICKNESS(e)/2+1);
	NWIDTH(v)  = gs_stringw; 
	NHEIGHT(v) = gs_stringh; 
	NSX(v) = NSX(v) - (long)(gs_stringw/2);
	NSY(v) = NSY(v) - (long)(gs_stringh/2);
}


/*   Search visible node 
 *   -------------------
 *   given a node v of a subgraph or region nest, look for the 
 *   innerst subgraph summary node or region node that is visible.
 *   Return NULL if no such node exists.
 *   The Search is driven by the reason why v is unvisible.
 */

#ifdef ANSI_C
static GNODE	search_visible(GNODE v)
#else
static GNODE	search_visible(v)
GNODE	v;
#endif
{
	GNODE w;

	debugmessage("search_visible","");

	/* Dangerous: I'm not sure that this terminates always */

	if (!v) return(NULL);
	if (NINLIST(v)) return(v);
	switch (NINVISIBLE(v)) {
		case UNFOLDED_SGRAPH: 
			w = v;	
			while (NINVISIBLE(w)==UNFOLDED_SGRAPH) {
				w = GNNODE(NSGRAPH(v));
			}
			if (NINVISIBLE(w)==FOLDED_SGNODE)
				return(search_visible(NROOT(v)));
			else	return(search_visible(GNNODE(NSGRAPH(v))));
		case FOLDED_SGNODE:
			return(search_visible(NROOT(v)));
		case FOLDED_RGNODE:
			return(search_visible(NREGROOT(v)));
	}
	return(NULL);
}	


/*   Check substitution of edges
 *   ---------------------------
 *   If a subgraph is folded, all edges to nodes of this subgraph must
 *   be replaced by edges to the summary node.
 *   As side effect, the invisibility flag of the edge is set, if the
 *   edge is invisible.
 *   This function returns e,			if e must be drawn, 
 *			   a substitution edge, if e is from a visible node
 *						to an invisible aubgraph node
 *			   NULL,		if e is invisible.
 *
 *   Note the situation we expect: edgelist contains all stable edge,
 *   independent of visible or not. 
 *   tmpedgelist contains the temporary edges that may be additionally
 *   visible.
 *   Subgraph folding/unfolding was done before, and region folding
 *   and edge hiding is done after that !
 */
	
#ifdef ANSI_C
static GEDGE	substed_edge(GEDGE e)
#else
static GEDGE	substed_edge(e)
GEDGE	e;
#endif
{
	GNODE	s,t,ss,tt;
	GEDGE	h;

	assert((e));
	debugmessage("substed_edge","");

	/* We assume: e is in edgelist !!! */

	if (EINVISIBLE(e)) return(NULL);
	s = ESTART(e);
	t = EEND(e);
	if ( NINLIST(s) && NINLIST(t) ) return(e);  /* edge must be drawn */
	EINVISIBLE(e) = 1;

	ss = search_visible(s);
	tt = search_visible(t);

	/* We never want to have an edge from a folded region/subgraph to 
	 * the region/subgraph summary node. This would create unnecessary
 	 * self loops.
	 */
	if (((ss!=s)||(tt!=t))&&(ss==tt)) return(NULL);

	/*  It may really happen that s or t are ZERO, e.g. if there are 
	 *  invisible subgraphs that are not folded but don't contain nodes.
	 */
	if ( !ss || !tt ) return(NULL);

	/*  Look whether we have already a substitution between s and t.
	 *  Before the first call, we assume that tmpedgelist is NULL.
	 */

	h = tmpedgelist;
	while (h) {
		if ((ESTART(h)==ss) && (EEND(h)==tt)) return(h);
		h = EINTERN(h);
	}	

	/*  HERE IS THE POSSIBLE ENTRY POINT TO GIVE THE EDGES TO FOLDED
	 *  SUBGRAPHS A SPECIAL ATTRIBUTE
	 */ 

	h = tmpedgealloc(
		ELSTYLE(e),
		ETHICKNESS(e),
		ECLASS(e),
		EPRIO(e),
		ECOLOR(e),
		ELABELCOL(e),
		EARROWSIZE(e),
		EARROWBSIZE(e),
		EARROWSTYLE(e),
		EARROWBSTYLE(e),
		EARROWCOL(e),
		EARROWBCOL(e),
		EHORDER(e));
	if (s==ss) EANCHOR(h) = EANCHOR(e);

	inherit_foldedge_attributes(&foldedge, h);
	
	ESTART(h)	= ss;
	EEND(h) 	= tt;
	return(h);
}



/*   Create adjacency lists
 *   ======================
 *   This is the first version: we don't need labels.
 *   Adjacency lists of all nodes in the nodelist, i.e. of all visible nodes.
 */


#ifdef ANSI_C
static void create_adjacencies(void)
#else
static void create_adjacencies()
#endif
{
	GEDGE	edge, e;

	debugmessage("create_adjacencies","");
	edge = edgelist;
	while (edge) {
		e = substed_edge(edge);
		if (e) create_adjedge(e);
		edge = ENEXT(edge);
	}
}



/*   Create adjacency lists
 *   ======================
 *   This is the second version: we need labels.
 *   Adjacency lists of all nodes in the nodelist, i.e. of all visible nodes,
 *   and of all label nodes.
 *   Warning: This function is a little bit inefficient: If substed_edge
 *   creates a labled substed edge, this edge is again substituted by
 *   two edges. However this occurs only for summary nodes, thus we hope
 *   it is quite seldom.
 */


#ifdef ANSI_C
static void create_lab_adjacencies(void)
#else
static void create_lab_adjacencies()
#endif
{
	GEDGE	edge, e, e1, e2;
	GNODE	v;

	debugmessage("create_lab_adjacencies","");
	edge = edgelist;
	while (edge) {
		e = substed_edge(edge);
		if (e) {
			if ( ELABEL(e) ) {
				v = create_labelnode(e);
				e1 = tmpedgealloc(
						ELSTYLE(e),
						ETHICKNESS(e),
						ECLASS(e),
						EPRIO(e),
						ECOLOR(e),
						ELABELCOL(e),
						0,
						EARROWBSIZE(e),
						ASNONE,
						EARROWBSTYLE(e),
						EARROWCOL(e),
						EARROWBCOL(e),
						EHORDER(e));
				EANCHOR(e1) = EANCHOR(e);
				ESTART(e1)	= ESTART(e);
				EEND(e1)	= v;
				ELABEL(e1)	= NULL;
				create_adjedge(e1);
				e2 = tmpedgealloc(
						ELSTYLE(e),
						ETHICKNESS(e),
						ECLASS(e),
						EPRIO(e),
						ECOLOR(e),
						ELABELCOL(e),
						EARROWSIZE(e),
						0,
						EARROWSTYLE(e),
						ASNONE,
						EARROWCOL(e),
						EARROWBCOL(e),
						EHORDER(e));
				ESTART(e2)	= v;
				EEND(e2)	= EEND(e);
				ELABEL(e2)	= NULL;
				create_adjedge(e2);
			}
			else create_adjedge(e);
		}
		edge = ENEXT(edge);
	}
}

/*--------------------------------------------------------------------*/

/* Summarize doublicated edges
 * ---------------------------
 * It is sometimes very ugly to have many identical edges from one node
 * to the same other node. To avoid to layout such situations,
 * we delete all duplicates. This happens only for edges without
 * labels.
 * But note that this function is very slow and may be cubic in 
 * complexity. Thus it is optional.
 */

#ifdef ANSI_C
static void summarize_edges(void)
#else
static void summarize_edges()
#endif
{
	GNODE v;
	GEDGE e1, e2;
	ADJEDGE a,b,c;
	int found, ide;

	v = nodelist;
	while (v) {
		a = NSUCC(v);
		while (a) {
			b = ANEXT(a);
			c = NSUCC(v);
			found = 0;
			while (c) {
				if (c!=a) ide = 1;
				else ide = 0;
				e1 = AKANTE(a);				
				e2 = AKANTE(c);				
				if (ESTART(e1)      != ESTART(e2))       ide=0;
				if (EEND(e1)        != EEND(e2))         ide=0;
				if (ELSTYLE(e1)     != ELSTYLE(e2))      ide=0;
				if (ETHICKNESS(e1)  >  ETHICKNESS(e2))   ide=0;
				if (EPRIO(e1)       >  EPRIO(e2))        ide=0;
				if (EHORDER(e1)     != EHORDER(e2))      ide=0;
				if (ECLASS(e1)      != ECLASS(e2))       ide=0;
				if (ECOLOR(e1)      != ECOLOR(e2))       ide=0;
				if (EARROWSIZE(e1)  != EARROWSIZE(e2))   ide=0;
				if (EARROWBSIZE(e1) != EARROWBSIZE(e2))  ide=0;
				if (EARROWSTYLE(e1) != EARROWSTYLE(e2))  ide=0;
				if (EARROWBSTYLE(e1)!= EARROWBSTYLE(e2)) ide=0;
				if (EARROWCOL(e1)   != EARROWCOL(e2))    ide=0;
				if (EARROWBCOL(e1)  != EARROWBCOL(e2))   ide=0;
				if (EANCHOR(e1)     != EANCHOR(e2))      ide=0;

				if (ide) {
					found++; break;
				}
				c = ANEXT(c);
			}
			if (found) delete_adjedge(AKANTE(a));	
			a = b;
		}	
		v = NNEXT(v);
	}
#ifdef CHECK_ASSERTIONS
	v = labellist;
	while (v) {
		a = NSUCC(v);
		assert((ANEXT(a)==NULL));  /* only one edge !!! */
		v = NNEXT(v);
	}
#endif
}



/*--------------------------------------------------------------------*/

/* Split doublicated edges
 * -----------------------
 * This is something like the converse of "summarize edges":
 * If we have several edges between two nodes a and b, these edges
 * may be drawn at the same place such that we cannot distinguish
 * them. Thus, we add dummy label nodes to them, which are layouted
 * at different places such that we we can see them.
 *
 * Example:   A ===> B   is drawn as   A--->B
 *                                      \   ^
 *                                       \_/
 *
 * Note: self loops are not split since we have a special treatment 
 * of self loops.
 */

#ifdef ANSI_C
static void split_double_edges(void)
#else
static void split_double_edges()
#endif
{
	GNODE v,w;
	GEDGE e, e1, e2;
	ADJEDGE a,b,c;
	int found;

	v = nodelist;
	while (v) {
		a = NSUCC(v);
		while (a) {
			b = ANEXT(a);
			if (TARGET(a)==SOURCE(a)) {
				a = b;	/* selfloop */
				continue;
			}
			c = NSUCC(v);
			found = 0;
			while (c) {
				if ((c!=a)&&(TARGET(c)==TARGET(a))) {
					found++; break;
				}
				c = ANEXT(c);
			}
			if (found) { 
				e = AKANTE(a);
				w = create_labelnode(e);
				NLABEL(w) = "";
				NWIDTH(w) = NHEIGHT(w) = 0;
				e1 = tmpedgealloc(
						ELSTYLE(e),
						ETHICKNESS(e),
						ECLASS(e),
						EPRIO(e),
						ECOLOR(e),
						ELABELCOL(e),
						0,
						EARROWBSIZE(e),
						ASNONE,
						EARROWBSTYLE(e),
						EARROWCOL(e),
						EARROWBCOL(e),
						EHORDER(e));
				EANCHOR(e1) = EANCHOR(e);
				ESTART(e1)	= v;
				EEND(e1)	= w;
				ELABEL(e1)	= ELABEL(e);
				create_adjedge(e1);
				e2 = tmpedgealloc(
						ELSTYLE(e),
						ETHICKNESS(e),
						ECLASS(e),
						EPRIO(e),
						ECOLOR(e),
						ELABELCOL(e),
						EARROWSIZE(e),
						0,
						EARROWSTYLE(e),
						ASNONE,
						EARROWCOL(e),
						EARROWBCOL(e),
						EHORDER(e));
				ESTART(e2)	= w;
				EEND(e2)	= EEND(e);
				ELABEL(e2)	= NULL;
				create_adjedge(e2);
				delete_adjedge(e);	
			}
			a = b;
		}	
		v = NNEXT(v);
	}
}



/*--------------------------------------------------------------------*/

#ifdef DEBUG

/* Debugging function: print a double linked nodelist 
 * --------------------------------------------------
 * w is startnode, wend is endnode. We print maximal
 * DB_MAXNODES nodes to avoid to come into an infinite
 * loop, if the data structure is incorrect.
 */

#define DB_MAXNODES 25

#ifdef ANSI_C
static void db_print_somenode_list(GNODE w,GNODE wend)
#else
static void db_print_somenode_list(w,wend)
GNODE w;
GNODE wend;
#endif
{
	GNODE v; int i;

	v = w;
	i = 0;
	PRINTF("Addresses Startnode %ld Endnode %ld\n",w,wend);
	while (v) {
		i++; if (i>DB_MAXNODES) break;
		PRINTF("Address %ld:%s [%d]    (Address next: %ld)\n",
			v,(NTITLE(v)?NTITLE(v):"(null)"),NINVISIBLE(v),
			NNEXT(v));
		v = NNEXT(v);
	}
}
#endif 

