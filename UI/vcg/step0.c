/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         step0.c                                            */
/*   version:      1.00.00                                            */
/*   creation:     10.4.1993                                          */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  analysis phase of syntax trees                     */
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
 * Revision 3.18  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.17  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 *
 * Revision 3.16  1994/11/23  14:50:47  sander
 * Hash table of nodes changed. Dynamic adaption to the number of nodes.
 *
 * Revision 2.3  1994/01/10  09:22:52  sander
 * Distribution version 1.
 *
 */

/************************************************************************
 * The situation here is the following:
 * -----------------------------------
 * We have already parsed the specification and constructed a syntax tree.
 * The task of this module is to fill nodelist with the exisiting nodes, 
 * graphlist with the existing graph summary nodes and edgelist with the 
 * existing edges.
 * Nearedges, backedges and bendnearedges must be inserted additionally 
 * into corresponding lists of nearedges, backedges and bendedges.
 * We assume that nodelist, edgelist and graphlist are empty at this point.
 * For the connection structure between graph summary nodes and normal nodes,
 * see alloc.h.
 * We need NOT to create the adjacency lists of the nodes and edges in
 * this module, because this is done later, see folding.c.
 * However we have to prepare folding by recognizing the folding and 
 * invisible attributes. This is done by the following functions/structs: 
 *    - add_foldstart:   remark node as start node of a fold region operation 
 *    - add_foldstop:    remark node as stop  node of a fold region operation
 *    - add_sgfoldstart: remark node as summary node of folded subgraph
 *    - hide_class:      array. Component i is 1, if edge class i+1 is hidden.
 *    - clear_hide_class: initialization of this arry.
 * See folding.c for more information about these functions. We call these
 * functions and structs that are used to recognize which must be folded
 * the `folding keepers'. The real folding according to our settings of the
 * folding keepers is done there.
 * Note that the nodes and edges allocated here are stable, i.e. are not
 * reallocated if the layout of the graph changes. To have fast access to
 * these stable nodes, we use a hash table.
 *
 * The task is done by step0_main. After that, the syntax tree is not
 * anymore necessary, but note that the strings in the syntax tree may
 * be reused here.
 *
 * This file provides the following functions:
 * ------------------------------------------
 * step0_main		main routine of step0: walk through the syntax 
 *			tree, collect attributes and creates the internal
 *                      representation of the graph to be visualized.
 * foldnode		an auxiliary node that contains all attributes
 *			of folded nodes.
 * foldedge		an auxiliary edge that contains all attributes
 *			of folded edges. 
 *
 * A hashtable is used to store all existing nodes.
 * act_hash_size	is the size of the hashtable.
 * lookup_hashnode      returns a node of a given title. The node must
 *                      be derived directly from the specification, i.e. 
 *                      no dummy node. The node need not to be visible.
 * search_visible_node  returns a node of a given title, but only if it
 *			is visible. Similar to lookup_hashnode.
 * 
 * We have a curser that allows to browse through the hashtable.
 * init_hash_cursor     initializes the hash curser, i.e. sets it to the
 *			first position.
 * position_hash_cursor sets the hash cursor to a fixed position.
 * get_hash_cursor_succ returns the i.th node after the hash cursor.
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "grammar.h"
#include "alloc.h"
#include "main.h"
#include "options.h"
#include "folding.h"
#include "fisheye.h"
#include "steps.h"
#include "timing.h"


/*  Defines
 *  -------
 *  Faster access to the syntax tree.
 */

/* Decode a identifier or string node */

#define SDecode(x)  Decode(get_lnum(x))

/* Standard access to nums */
 
#define FETCHNUM()   get_lnum(son1(node2))
#define FETCHFLOAT() get_lrealnum(son1(node2))


/* Global Variables
 * ----------------
 */

struct gnode foldnode;
struct gedge foldedge;


/* Prototypes
 * ----------
 */

static void   node_analyse	_PP((yysyntaxtree no, GNODE ro, GNODE dfn));
static void   edge_analyse	_PP((yysyntaxtree no, GEDGE dfe));

static void   node_attributes	_PP((yysyntaxtree n, GNODE v,int r,int s));
static void   edge_attributes	_PP((yysyntaxtree n, GEDGE e));
static void   graph_attributes 	_PP((yysyntaxtree n, GNODE v,int r,int s));

static void   one_node_attribute 
	_PP((yysyntaxtree n2, GNODE v, int rh, int rs, int *bcs));
static void   one_edge_attribute
	_PP((yysyntaxtree node2, GEDGE e, int *es, int *as, int *bs));

static int    get_color		_PP((yysyntaxtree node));
static int    get_yesno         _PP((yysyntaxtree node));
static void   calc_nr_classes	_PP((yysyntaxtree x));
static GNODE  search_node	_PP((yysyntaxtree x,char *s));
static void   check_node	_PP((yysyntaxtree x,GNODE m));
static void   init_hashtable	_PP((void));
static int    hashval		_PP((char *s));
static void   insert_hashnode	_PP((GNODE x));


#ifdef CHECKNODE
static void debug_init_checknode _PP((void));
#else
#define debug_init_checknode()  /**/
#endif

/*--------------------------------------------------------------------*/
/* Analysis of syntax trees                                           */
/*--------------------------------------------------------------------*/

/* Local flags 
 * -----------
 */

static int subg_bit;	/* Nesting level of subgraphs: 0 means: top level */

/* Flags, indicate whether a foldattribute is set. The are 1 if ... */

static int fold_elcol_set;    /* ... edge label color is set */
static int fold_arrowc_set;   /* ... arrow color is set      */
static int fold_barrowc_set;  /* ... back arrow color is set */
static int fold_borderc_set;  /* ... border color is set     */


/*  Driver of tree analysis
 *  =======================
 */


#ifdef ANSI_C
void	step0_main(void)
#else
void	step0_main()
#endif
{
	int i;
	struct gnode defaultnode;
	struct gedge defaultedge;

	start_time();
	debugmessage("step0_main","");
	
	assert((nodelist==NULL));
	assert((graphlist==NULL));
	assert((edgelist==NULL));
	assert((near_edge_list==NULL));
	assert((back_edge_list==NULL));
	assert((bent_near_edge_list==NULL));

	/* Initialize the color map from the original color map */

	cmap_changed = 1;
	cmap_size = BASECMAPSIZE;
	for (i=0; i<BASECMAPSIZE; i++) {
		redmap[i]   = origredmap[i];
		greenmap[i] = origgreenmap[i];
		bluemap[i]  = origbluemap[i];
	}

	/* Calculate the number of edge classes */

	max_eprio = 0;
	max_nr_classes = 1;
	calc_nr_classes(Syntax_Tree);

	if (max_nr_classes>128) {
		FPRINTF(stderr,"\nWarning: you use %d edge classes !\n", 
			max_nr_classes);
		FPRINTF(stderr,"This may produce a segmentation fault on some systems.\n"); 
		FPRINTF(stderr,"Please do not use more than 128 classes.\n"); 
	}

	/* Allocate the string field for edge classes */

	if (class_names) free(class_names);
	class_names = (char **)malloc(max_nr_classes * sizeof(char *));
	for (i=0; i<max_nr_classes; i++) class_names[i]=0;
	if (hide_class) free(hide_class);
	hide_class = (int *)malloc(max_nr_classes * sizeof(int));

	/* Initialize folding attributes of nodes and edges */

	foldnodedefaults(&foldnode);
	foldedgedefaults(&foldedge);

	fold_elcol_set   = 0;
	fold_arrowc_set  = 0;
	fold_barrowc_set = 0;
	fold_borderc_set = 0;

	/* Initialize the folding keepers */

	clear_hide_class();
	clear_folding_keepers();

	/* Initialize the hash table of stable nodes */

	init_hashtable();

	/* Analyze the syntax tree: the values given here are the
	 * default values.
         */

	subg_bit    = 0;	/* We are outside the top graph       */
	nodedefaults(&defaultnode);
        node_analyse(
		  Syntax_Tree,	/* syntax tree node       */
		  NULL,		/* root node              */
		  &defaultnode);

	edgedefaults(&defaultedge);
        edge_analyse(
		  Syntax_Tree,	/* syntax tree node       */
		  &defaultedge);

	/* For debugging only */

	debug_init_checknode();

	stop_time("step0_main without folding");
}



/*--------------------------------------------------------------------*/

/* Check number of necessary nodes
 * ===============================
 * The result is in numnodes. We traverse the syntax tree and look
 * for T_Co_graph and T_Co_node parts.
 */

static int numnodes;    /* Number of necessary nodes */

#ifdef ANSI_C
static void estimate_num_nodes(yysyntaxtree x)
#else
static void estimate_num_nodes(x)
yysyntaxtree x;
#endif
{
        register int j,len;

        if (!x) return;
        len = nr_of_sons(x);

	if (tag(x)==T_Co_graph) numnodes++;
        else if (tag(x)==T_Co_node)  numnodes++; 

        for (j=1; j<=len; j++) {
                estimate_num_nodes(son(x,j));
        }
}


/*--------------------------------------------------------------------*/

/*  Node analysis per subgraph
 *  ==========================
 */

/*  The function node_analyse traverses the syntax tree starting at `node', 
 *  collects the information and allocates the appropriate data structures.
 *  node is assumed to be a T_Co_graph_entry representing a graph.
 *  The situation is always the following:
 *      all simple  nodes are collected into the node list
 *      all summary nodes are collected into the graph list
 *
 *  Inside the function, we build NSGRAPH(root).
 *      `root'        points to the actual summary node of the subgraph
 *      NSGRAPH(root) points to the actual list of nodes of this subgraph
 *      rootend       points to the end of this list
 *
 */


/*  Abbreviation to add v into the nodelist NSGRAPH(root) */

#define add_to_nodelist_of_root(v)  \
	{ if ( subg_bit > 0 ) {                     \
		NROOT(v)	= root;             \
		l = nodelist_alloc(v);              \
		if (rootend) 	GNNEXT(rootend) = l;\
		else 		NSGRAPH(root) = l;  \
		rootend = l;                        \
	}}



#ifdef ANSI_C
static void	node_analyse(yysyntaxtree node, GNODE root, GNODE defnode) 
#else
static void	node_analyse(node, root, defnode)
yysyntaxtree node;	/* syntax tree node                   */
GNODE	root;		/* graph summary node                 */
GNODE   defnode;	/* default node attributes	      */
#endif
{
	register yysyntaxtree	node1, node2;	/* auxiliary variables */
	struct 	gnode defaultnode;
	GNODE	v;
	GNLIST  l;
	int 	invis;
	GNLIST  rootend;	/* end of the actual node list */
	int     rootshrink;	/* shrink factor of the root   */
	int 	rootstretch;	/* stretch factor of the root  */
	int     h;
	int 	borderc_set;	/* Flag, whether the default border color */
				/* was set explicitely                    */

	debugmessage("node_analyse","");

	gs_wait_message('a');

	/* Copy the old default attributes into the new default node */

	copy_nodeattributes(defnode, &defaultnode);

	/* Initialize shrink and stretch factors of the root node */

	if (root) {
		rootshrink  = (NSHRINK(root)>0  ? NSHRINK(root) : 1);
		rootstretch = (NSTRETCH(root)>0 ? NSTRETCH(root): 1);
	}
	else    rootshrink = rootstretch = 1;

	/* traverse iteratively the syntax tree: 
 	 * on graph entries, we create a graph summary node, fetch the
	 * 	attributes valid for the summary node and we go into recursion.
	 * on node entries, we create a new node and fetch its attributes.
	 * on graph attributes, we fetch the attributes valid as default 
	 *	values of the whole graph and set the graph attributes 
	 *	G_x, G_y, etc.
 	 * on node default attributes, we change the attributes of the
 	 *    	defaultnode. The attributes of each new node are inherited
	 *	from the default node. 
 	 * on foldnode attributes, we change the attributes of the
 	 *    	foldnode. The attributes of each folded summary region
	 *	node are inherited from the foldnode. 
	 *
	 * Nodes come into the hashtable after fetching all its attributes.
	 * See the corresponding fetch routines.
	 */

	borderc_set = 0;
	rootend = NULL;
	assert((node && (tag(node) == T_Co_graph_entry)));
	while ( node && (tag(node) == T_Co_graph_entry) ) {
		node1 = son1(node);
		assert(node1);
	        switch (tag(node1)) {
		case T_Co_graph:
			v = graphalloc(&defaultnode);

			add_to_nodelist_of_root(v);

			graph_attributes(son1(node1),v,rootshrink,rootstretch);
			subg_bit++;
			node_analyse(son1(node1),v,&defaultnode);
			subg_bit--;
			break;

		case T_Co_graph_attribute:

			/* This is only used for the top level graph */

			node2 = son1(node1);
			assert((node2));
			switch(tag(node2)) {
                	case T_Co_title:
				if (!subg_bit) G_title = SDecode(son1(node2));
                        	break;
                	case T_Co_label:
                	case T_Co_info1:
                	case T_Co_info2:
                	case T_Co_info3:
                	case T_Co_level:
                	case T_Co_shape:
                	case T_Co_horizontal_order:
                	case T_Co_loc  :
                	case T_Co_textmode:
                	case T_Co_borderwidth:
                	case T_Co_textcolor:
                	case T_Co_bordercolor:
                        	break;
                	case T_Co_xdef:
                        	if (!subg_bit)	G_x = FETCHNUM();
                        	break;
                 	case T_Co_ydef:
                        	if (!subg_bit)  G_y = FETCHNUM();	
                        	break;
                	case T_Co_width:
                        	if (!subg_bit)	{ 	
					G_width  = (int)FETCHNUM();
					if (G_width <= 0) G_width = 100;
					G_width_set = 1;
				}
                        	break;
                	case T_Co_height:
                        	if (!subg_bit) {
					G_height = (int)FETCHNUM();
					if (G_height <= 0) G_height = 100;
					G_height_set = 1;
				}
                        	break;
			case T_Co_xmax:
				if (!subg_bit) {
					if (!G_xymax_final)
						G_xmax = (int)FETCHNUM();
					if (G_xmax <= 200) G_xmax = 200;
					if (G_xmax > (2*ScreenWidth))
						G_xmax = 2*ScreenWidth;
					if (G_width>G_xmax) {
						G_width = G_xmax;
						G_width_set = 1;
					}
				}
			 	break;
			case T_Co_ymax:
				if (!subg_bit) {
					if (!G_xymax_final)
						G_ymax = (int)FETCHNUM();
					if (G_ymax <= 200) G_ymax = 200;
					if (G_ymax > (2*ScreenHeight))
						G_ymax = 2*ScreenHeight;
					if (G_height>G_ymax) {
						G_height = G_ymax;
						G_height_set = 1;
					}
				}
				break;
			case T_Co_infoname:
				if (!subg_bit) {
					h = (int)FETCHNUM();
					if ((h>0)&&(h<=3)) {
						info_name_available = 1;
						info_names[h-1] = 
							SDecode(son2(node2));
					}
				}
                        	break;
			case T_Co_classname:
				if (!subg_bit) {
					h = (int)FETCHNUM();
					if ((h>0)&&(h<=max_nr_classes)) {
						class_name_available = 1;
						class_names[h-1] = 
							SDecode(son2(node2));
					}
				}
                        	break;
                	case T_Co_xbase:
                        	if (!subg_bit)	G_xbase = (int)FETCHNUM();
                        	break;
                	case T_Co_ybase:
                        	if (!subg_bit)	G_ybase = (int)FETCHNUM();
                        	break;
                	case T_Co_xspace:
                        	if (!subg_bit)	G_xspace= (int)FETCHNUM();
                        	break;
                	case T_Co_xlspace:
                        	if (!subg_bit)	G_dspace= (int)FETCHNUM();
                        	break;
                	case T_Co_yspace:
                        	if (!subg_bit)	G_yspace= (int)FETCHNUM();
                        	break;
                	case T_Co_xraster:
                        	if (!subg_bit)	G_xraster= (int)FETCHNUM();
                        	break;
                	case T_Co_xlraster:
                        	if (!subg_bit)	G_dxraster= (int)FETCHNUM();
                        	break;
                	case T_Co_yraster:
                        	if (!subg_bit)	G_yraster= (int)FETCHNUM();
                        	break;
			case T_Co_splinefactor:
                        	if (!subg_bit)	G_flat_factor= (int)FETCHNUM();
                        	break;
			case T_Co_downfactor:
                        	if (!subg_bit)	layout_downfactor= (int)FETCHNUM();
                        	break;
			case T_Co_upfactor:
                        	if (!subg_bit)	layout_upfactor= (int)FETCHNUM();
                        	break;
			case T_Co_nearfactor:
                        	if (!subg_bit)	layout_nearfactor= (int)FETCHNUM();
                        	break;
			case T_Co_bend_max:
                        	if (!subg_bit)	max_edgebendings= (int)FETCHNUM();
                        	break;
			case T_Co_cross_min:
                        	if (!subg_bit)	min_baryiterations= (int)FETCHNUM();
                        	break;
			case T_Co_cross_max:
                        	if (!subg_bit)	max_baryiterations= (int)FETCHNUM();
                        	break;
			case T_Co_pendel_min:
                        	if (!subg_bit)	min_mediumshifts= (int)FETCHNUM();
                        	break;
			case T_Co_pendel_max:
                        	if (!subg_bit)	max_mediumshifts= (int)FETCHNUM();
                        	break;
			case T_Co_rubber_min:
                        	if (!subg_bit)	min_centershifts= (int)FETCHNUM();
                        	break;
			case T_Co_rubber_max:
                        	if (!subg_bit)	max_centershifts= (int)FETCHNUM();
                        	break;
			case T_Co_straight_max:
                        	if (!subg_bit)	max_straighttune= (int)FETCHNUM();
                        	break;
                	case T_Co_node_alignment:
				if (!subg_bit) {
                        		switch(tag(son1(node2))) {
                               	    	case T_Co_top:
						G_yalign = AL_TOP;
						break;
                               	    	case T_Co_bottom:
						G_yalign = AL_BOTTOM;
						break;
                               	    	case T_Co_center:
						G_yalign = AL_CENTER;
						break;
                               	    	default:
						assert((0));
					}
                        	}
                        	break;
                	case T_Co_orientation:
				if (!subg_bit) {
                        		switch(tag(son1(node2))) {
                               	    	case T_Co_top_to_bottom:
                                       		G_orientation = TOP_TO_BOTTOM;
                                       		break;
                               	    	case T_Co_left_to_right:
                                       		G_orientation = LEFT_TO_RIGHT;
                                       		break;
                               	    	case T_Co_right_to_left:
                                       		G_orientation = RIGHT_TO_LEFT;
                                       		break;
                               	    	case T_Co_bottom_to_top:
                                       		G_orientation = BOTTOM_TO_TOP;
                                       		break;
                               	    	default:
						assert((0));
					}
                        	}
                        	break;
                	case T_Co_port_sharing:
				if (!subg_bit) 
                                       	G_portsharing = get_yesno(son1(node2));
                        	break;
                	case T_Co_arrow_mode:
				if (!subg_bit) {
                        		switch(tag(son1(node2))) {
                               	    	case T_Co_fixed:
                                       		G_arrowmode = AMFIXED;
                                       		break;
                               	    	case T_Co_free:
                                       		G_arrowmode = AMFREE;
                                       		break;
                               	    	default:
						assert((0));
					}
				}
                        	break;
			case T_Co_color:
				if (!subg_bit) 	
					G_color = get_color(son1(node2));
				break;
			case T_Co_folding:
				if (!subg_bit){
					SYERR(node2,
				"attribute `folding' at top level graph");
				}
				break;
			case T_Co_status:
				if (!subg_bit){
					SYERR(node2,
				"attribute `status' at top level graph");
				}
				break;
			case T_Co_hidden:
				invis = (int)FETCHNUM()-1;
				if (!subg_bit) {
					if ((0<=invis) && (invis<max_nr_classes))
                                        	hide_class[invis] = 1;
					else SYERR(node2,
					 "attribute `hidden' out of range (1-max_nr_classes)");
				}
				break;
                	case T_Co_late_edge_label:
				if (!subg_bit) 
                                       	edge_label_phase = get_yesno(son1(node2));
                        	break;
                	case T_Co_display_edge_label:
				if (!subg_bit) 
                                       	G_displayel = get_yesno(son1(node2));
                        	break;
                	case T_Co_dirty_edge_label:
				if (!subg_bit) {
                                       	G_dirtyel = get_yesno(son1(node2));
					if (G_dirtyel==YES) G_displayel = YES;
				}
                        	break;
                	case T_Co_finetuning:
				if (!subg_bit) 
                                       	fine_tune_layout = get_yesno(son1(node2));
                        	break;
                	case T_Co_crossing_phase2:
				if (!subg_bit) 
                                       	skip_baryphase2 = 1-get_yesno(son1(node2));
                        	break;
                	case T_Co_crossing_opt:
				if (!subg_bit) 
                                       	local_unwind = get_yesno(son1(node2));
                        	break;
                	case T_Co_crossing_weight:
				if (!subg_bit) {
                        		switch(tag(son1(node2))) {
                               	    	case T_Co_bary:
                                       		crossing_heuristics = 0;
						break;
				    	case T_Co_median:
                                       		crossing_heuristics = 1;
                                       		break;
				    	case T_Co_barymedian:
                                       		crossing_heuristics = 2;
                                       		break;
				    	case T_Co_medianbary:
                                       		crossing_heuristics = 3;
                                       		break;
                               	    	default:
						assert((0));
                        		}
				}
                        	break;
                	case T_Co_manhatten:
				if (!subg_bit) 
                                       	manhatten_edges = 2-get_yesno(son1(node2));
                        	break;
                	case T_Co_smanhatten:
				if (!subg_bit) 
                                       	one_line_manhatten = 2-get_yesno(son1(node2));
                        	break;
                	case T_Co_straightphase:
				if (!subg_bit) 
                                       	straight_phase = 2-get_yesno(son1(node2));
                        	break;
                	case T_Co_priophase:
				if (!subg_bit) 
                                       	prio_phase = 2-get_yesno(son1(node2));
                        	break;
                	case T_Co_hidesingles:
				if (!subg_bit) 
                                       	hide_single_nodes = get_yesno(son1(node2));
                        	break;
                	case T_Co_view_nodes:
				if (!subg_bit) 
                                       	supress_nodes = 1-get_yesno(son1(node2));
                        	break;
                	case T_Co_view_edges:
				if (!subg_bit) 
                                       	supress_edges = 1-get_yesno(son1(node2));
                        	break;
                	case T_Co_view_splines:
				if (!subg_bit) 
                                       	G_spline = get_yesno(son1(node2));
                        	break;
                	case T_Co_view_method:
				if (!subg_bit) {
                        		switch(tag(son1(node2))) {
                               	    	case T_Co_cfish:
                                       		fisheye_view = CSCF_VIEW;
						break;
                               	    	case T_Co_pfish:
                                       		fisheye_view = PSCF_VIEW;
						break;
                               	    	case T_Co_fcfish:
                                       		fisheye_view = FCSCF_VIEW;
						break;
                               	    	case T_Co_fpfish:
                                       		fisheye_view = FPSCF_VIEW;
						break;
                               	    	default:
						assert((0));
                        		}
				}
                        	break;
			case T_Co_dummy  :
				break;
			case T_Co_nonearedges  :
				if (!subg_bit) {
					near_edge_layout = 0;
				}
				break;
                	case T_Co_colentry:
				{ 	int idx, rd, bl, gr;

				   	idx = (int)get_lnum(son1(node2));
				   	rd  = (int)get_lnum(son2(node2));
				   	gr  = (int)get_lnum(son3(node2));
				   	bl  = (int)get_lnum(son4(node2));
					if (idx>=CMAPSIZE-1) {
						SYERR(node2,"Only 254 color entries allowed");
					}
					if (idx+2>cmap_size) cmap_size = idx+2;
					redmap[  idx] = rd;
					bluemap[ idx] = bl;
					greenmap[idx] = gr;
					redmap[  cmap_size-1] = 0;
					bluemap[ cmap_size-1] = 0;
					greenmap[cmap_size-1] = 0;
					cmap_changed = 1;
				}
				break;
                	case T_Co_scaling:
				if (!subg_bit) {
					double hhx;

                        		hhx = FETCHFLOAT();
					if (hhx<0) hhx= -hhx;
					if (hhx>1.0) {
						G_stretch = 100;
						G_shrink = (int)(100.0/hhx);
					}
					else {
						G_stretch = (int)(hhx*100.0);
						G_shrink = 100;
					}
					G_shrink = rootshrink*G_shrink;
					G_stretch= rootstretch*G_stretch;
					if (G_shrink == 0) G_shrink = 1;
					if (G_stretch == 0) G_stretch = 1;
				}
                        	break;
                	case T_Co_shrink:
				if (!subg_bit) {
                        		G_shrink = rootshrink * (int)FETCHNUM();
					if (G_shrink <= 0) G_shrink = 1;
				}
                        	break;
                	case T_Co_stretch:
				if (!subg_bit) {
					G_stretch = rootstretch * (int)FETCHNUM();
					if (G_stretch <= 0) G_stretch = 1;
				}
                        	break;
                	case T_Co_spreadlevel:
				if (!subg_bit) {
					spread_level = (int)FETCHNUM();
				}
                        	break;
                	case T_Co_treefactor:
				if (!subg_bit) {
					tree_factor = FETCHFLOAT();
				}
                        	break;
			case T_Co_layoutalgorithm:
				if (!subg_bit) {
					assert((son1(node2)));
					switch (tag(son1(node2))) {
					case T_Co_maxdepth :
						layout_flag = 1;
						break;
					case T_Co_mindepth :
						layout_flag = 2;
						break;
					case T_Co_minbackwards :
						layout_flag = 3;
						break;
					case T_Co_maxdepthslow :
						layout_flag = 4;
						break;
					case T_Co_mindepthslow :
						layout_flag = 5;
						break;
					case T_Co_maxdegree :
						layout_flag = 11;
						break;
					case T_Co_mindegree :
						layout_flag = 10;
						break;
					case T_Co_maxindegree :
						layout_flag = 7;
						break;
					case T_Co_minindegree :
						layout_flag = 6;
						break;
					case T_Co_maxoutdegree :
						layout_flag = 9;
						break;
					case T_Co_minoutdegree :
						layout_flag = 8;
						break;
					case T_Co_tree :
						layout_flag = TREE_LAYOUT;
						break;
					case T_Co_depthfirst :
						layout_flag = 12;
						break;
					}	
				}
                        	break;

			default:
				if (silent) break;
				FPRINTF(stderr,"Line %d: attribute %s ",
					xfirst_line(node2),ConstructorName(tag(node2)));
				FPRINTF(stderr,"currently not implemented !\n");
			}
			break;	

		/*---------------- end of T_Co_graph_attribute ----------------*/

		case T_Co_foldnode_defaults:
			if (subg_bit) { 
				SYERR(node2,"folding defaults  allowed only on top level");
				break;
			}
			node2 = son1(node1);
			assert((node2));

			switch(tag(node2)) {
			case T_Co_title:
				SYERR(node2,"foldnode.title not allowed");
				break;
			case T_Co_folding:
				SYERR(node2,"foldnode.fold not allowed");
				break;
			}
			one_node_attribute(node2,&foldnode,
				rootshrink,rootstretch, &fold_borderc_set);

			break; 	

		/*---------------- end of T_Co_foldnode_defaults -----------------*/

		case T_Co_node_defaults:
			node2 = son1(node1);
			assert((node2));

			switch(tag(node2)) {
			case T_Co_title:
				SYERR(node2,"node.title not allowed");
				break;
			}
			one_node_attribute(node2,&defaultnode,
				rootshrink,rootstretch, &borderc_set);

			break; 	

		/*---------------- end of T_Co_node_defaults ------------------*/

		case T_Co_node:
			v = nodealloc(&defaultnode);
			add_to_nodelist_of_root(v);
			node_attributes(son1(node1),v,rootshrink,rootstretch);
			break;

		/*---------------- end of T_Co_node ---------------------------*/

		}
		node = son2(node);
	} /* while */	
}



/*--------------------------------------------------------------------*/

/*  Edge analysis per subgraph
 *  ==========================
 */

/*  The function edge_analyse traverses the syntax tree starting at `node', 
 *  collects the information and allocates the appropriate data structures.
 *  node is assumed to be a T_Co_graph_entry representing a graph.
 *  The situation is always the following:
 *      all edges are collected into the edge list
 */



#ifdef ANSI_C
static void	edge_analyse(yysyntaxtree node, GEDGE	defedge)
#else
static void	edge_analyse(node, defedge)
	yysyntaxtree node;	/* syntax tree node                   */
	GEDGE 	defedge;	/* default edge attributes	      */
#endif
{
	register yysyntaxtree	node1, node2;	/* auxiliary variables */
	struct gedge defaultedge;
	GEDGE	e;
	int elcol_set, arrowc_set, barrowc_set;
	                /* Flags, whether the default edge label color */
			/* etc. were set explicitely                   */

	debugmessage("edge_analyse","");

	gs_wait_message('a');

	/* Copy the old default attributes into the new default edge */

	copy_edgeattributes(defedge, &defaultedge);

	/* traverse iteratively the syntax tree: 
	 * on edge entries, we create a new edge and fetch its attributes.
	 * on edge default attributes, we change the attributes of the
 	 *    	defaultedge. The attributes of each new edge are inherited
	 *	from the default edge. 
 	 * on foldedge attributes, we change the attributes of the
 	 *    	foldedge. The attributes of each folded summary edge 
	 *	are inherited from the foldedge. 
 	 * Backedges, nearedges and bent nearedges are traeted like normal
	 * edges, but are additionally stored in the corresponding lists.
	 */

	elcol_set   = 0;
	arrowc_set  = 0;
	barrowc_set = 0;
	assert((node && (tag(node) == T_Co_graph_entry)));
	while ( node && (tag(node) == T_Co_graph_entry) ) {
		node1 = son1(node);
		assert(node1);
	        switch (tag(node1)) {
		case T_Co_graph:
			subg_bit++;
			edge_analyse(son1(node1), &defaultedge);
			subg_bit--;
			break;

		/*---------------- end of T_Co_node_defaults ------------------*/

		case T_Co_foldedge_defaults:
			if (subg_bit) { 
                //SYERR(node2,"folding defaults  allowed only on top level");
				break;
			}
			node2 = son1(node1);
			assert((node2));

			switch(tag(node2)) {
			case T_Co_sourcename:
			case T_Co_targetname:
				SYERR(node2,"foldedge.sourcename and foldedge.targetname not allowed");
				break;
			case T_Co_anchor:
				SYERR(node2,"foldedge.anchor not allowed");
				break;
			}

			one_edge_attribute(node2, &foldedge, &fold_elcol_set, 
				&fold_arrowc_set, &fold_barrowc_set);

			break;	/* T_Co_foldedge_defaults */

		/*---------------- end of T_Co_foldedge_defaults --------------*/

		case T_Co_edge_defaults:
			node2 = son1(node1);
			assert((node2));
			switch(tag(node2)) {
			case T_Co_sourcename:
			case T_Co_targetname:
				SYERR(node2,"edge.sourcename and edge.targetname not allowed");
				break;
			case T_Co_anchor:
				SYERR(node2,"edge.anchor not allowed");
				break;
			}
			one_edge_attribute(node2, &defaultedge, &elcol_set, 
				&arrowc_set, &barrowc_set);

			break;	/* T_Co_edge_defaults */

		/*---------------- end of T_Co_edge_defaults ------------------*/

		case T_Co_edge:
			e = edgealloc(&defaultedge);
			edge_attributes(son1(node1),e);
			break;

		/*---------------- end of T_Co_edge ---------------------------*/
		case T_Co_near_edge:
			e = edgealloc(&defaultedge);
			edge_attributes(son1(node1),e);
			if (ESTART(e)==EEND(e)) {
				if (silent) break;
				FPRINTF(stderr,"Line %d: self loop near edge neighbouring ignored !\n",
					xfirst_line(node2));
			}
			else if (EANCHOR(e)) {
				if (silent) break;
				FPRINTF(stderr,"Line %d: near edge with anchorpoint neighbouring ignored !\n",
					xfirst_line(node2));
			}
			else if (ELABEL(e)) {
				if (silent) break;
				FPRINTF(stderr,"Line %d: near edge with label ignored !\n",
					xfirst_line(node2));
			}
			else near_edge_insert(e);
			break;

		/*---------------- end of T_Co_near_edge ----------------------*/
		case T_Co_bent_near_edge:
			e = edgealloc(&defaultedge);
			edge_attributes(son1(node1),e);
			if (ESTART(e)==EEND(e)) {
				if (silent) break;
				FPRINTF(stderr,"Line %d: self loop near edge neighbouring ignored !\n",
					xfirst_line(node2));
			}
			else if (EANCHOR(e)) {
				if (silent) break;
				FPRINTF(stderr,"Line %d: near edge with anchorpoint neighbouring ignored !\n",
					xfirst_line(node2));
			}
			else bentnear_edge_insert(e);
			break;

		/*---------------- end of T_Co_near_edge ----------------------*/
		case T_Co_back_edge:
			e = edgealloc(&defaultedge);
			edge_attributes(son1(node1),e);
			back_edge_insert(e);
			break;

		/*---------------- end of T_Co_back_edge ----------------------*/
		}
		node = son2(node);
	} /* while */	
}



/*--------------------------------------------------------------------*/

/*  Analysis of the graph attributes
 *  --------------------------------
 *  node is a syntax tree node of sort graph_entry.
 *  v is a summary node whose attributes are now filled.
 *  default title is its title, if no other title is specified.
 *  We look in the graph_entry list for graph_attributes, and
 *  analyze these. Not all graph attributes are relevant for
 *  summary nodes, e.g. these that can only defined for
 *  the outermost graph are irrellevent.
 *  At the end of this function, we insert the summary node into the hash
 *  table by the function `check_node' and recognize its folding 
 *  attribute for the folding keepers.
 */          	


#ifdef ANSI_C
static void	graph_attributes( 
			yysyntaxtree node,
			GNODE v,
			int rootshrink,
			int rootstretch)
#else
static void	graph_attributes(node,v,rootshrink,rootstretch)
yysyntaxtree	node;
GNODE	v;
int     rootshrink;	/* shrink factor of the root   */
int 	rootstretch;	/* stretch factor of the root  */
#endif
{
	register yysyntaxtree	node1, node2;
	int borderc_set;

	debugmessage("graph_attributes","");
	assert((!node)||(tag(node)==T_Co_graph_entry));

	borderc_set = 0;
	node1 = node;
	while ( node1 && (tag(node1) == T_Co_graph_entry) ) {
		node2 = son1(node1);
		assert(node2);
		switch(tag(node2)) {
		case T_Co_graph_attribute:
			node2 = son1(node2);
			assert(node2);
			one_node_attribute(node2,v,
				rootshrink,rootstretch,&borderc_set);

			switch(tag(node2)) {
			case T_Co_status:
				switch(tag(son1(node2))) {
				case T_Co_black: NSTATE(v) = 2; break;
				case T_Co_grey:  NSTATE(v) = 1; break;
				case T_Co_white: NSTATE(v) = 0; break;
				default: assert((0));
				}
				break;
			case T_Co_infoname:
			case T_Co_classname:
			case T_Co_xmax:
			case T_Co_ymax:
			case T_Co_xbase:
			case T_Co_ybase:
			case T_Co_xspace:
			case T_Co_xlspace:
			case T_Co_yspace:
			case T_Co_xraster:
			case T_Co_xlraster:
			case T_Co_yraster:
			case T_Co_orientation:
			case T_Co_node_alignment:
                	case T_Co_port_sharing:
                	case T_Co_arrow_mode:
			case T_Co_hidden:
			case T_Co_late_edge_label:
			case T_Co_display_edge_label:
			case T_Co_dirty_edge_label:
                	case T_Co_hidesingles:
                	case T_Co_straightphase:
                	case T_Co_priophase:
                	case T_Co_smanhatten:
                	case T_Co_manhatten:
			case T_Co_bend_max:
			case T_Co_cross_min:
			case T_Co_cross_max:
			case T_Co_pendel_min:
			case T_Co_pendel_max:
			case T_Co_rubber_min:
			case T_Co_rubber_max:
			case T_Co_straight_max:
                	case T_Co_view_edges:
                	case T_Co_view_nodes:
                	case T_Co_view_splines:
                	case T_Co_view_method:
                	case T_Co_finetuning:
                	case T_Co_crossing_opt:
                	case T_Co_crossing_weight:
			case T_Co_nonearedges:
			case T_Co_dummy:
                	case T_Co_colentry:
			case T_Co_layoutalgorithm:
			case T_Co_splinefactor:
			case T_Co_downfactor:
			case T_Co_upfactor:
			case T_Co_nearfactor:
                	case T_Co_treefactor:
                	case T_Co_spreadlevel:
				if (subg_bit!=0) {
					SYERR(node2,"attribute allowed only on top level");
				}
				break;
			default:
				one_node_attribute(node2,v,
					rootshrink,rootstretch,&borderc_set);
			}
			break;
		case T_Co_constraint:	
			if (silent) break;
			FPRINTF(stderr,"Line %d: constraint ",
				xfirst_line(node2));
			FPRINTF(stderr,"currently not implemented !\n");
		}
		node1 = son2(node1);
    	}

	/* The default title inherit mechanism */

	if (NTITLE(v)==NULL) NTITLE(v) = G_title;
	if (NLABEL(v)==NULL) NLABEL(v) = NTITLE(v);

	/* For the folding keepers */

	if (NFOLDING(v)==0) {
		/* for the initial folding: we must recognize the
		 * foldstoppers.
		 */
		NFOLDING(v) = -1;
		add_foldstop(v);
	}
	if (NSTATE(v)>0) NFOLDING(v)=1;
	if (NFOLDING(v)>0) {
		/* and the subgraph fold starters */
		add_sgfoldstart(v);
	}
	NFOLDING(v) = -1;
	/* subgraph summary nodes are never region foldstarters */

	check_node(node,v);  /* check node and init into to hashtable */
}


/*--------------------------------------------------------------------*/

/*  Analysis of the node attributes
 *  -------------------------------
 *  node is a syntax tree node of sort node_attribute_list.
 *  v is a node whose attributes are now filled.
 *  At the end of this function, we insert the node into the hash
 *  table by the function `check_node' and recognize its folding 
 *  attribute for the folding keepers.
 */          	


#ifdef ANSI_C
static void	node_attributes(
			yysyntaxtree node,
			GNODE v,
			int rootshrink,
			int rootstretch)
#else
static void	node_attributes(node,v,rootshrink,rootstretch)
yysyntaxtree	node;
GNODE	v;
int     rootshrink;	/* shrink factor of the root   */
int 	rootstretch;	/* stretch factor of the root  */
#endif
{
	register yysyntaxtree	node1, node2;
	int borderc_set;

	debugmessage("node_attributes","");
	assert((!node)||(tag(node)==T_Co_node_attribute));

	borderc_set = 0;
	node1 = node;
	while ( node1 && (tag(node1) == T_Co_node_attribute) ) {
		node2 = son1(node1);
		assert(node2);
		one_node_attribute(node2,v,rootshrink,rootstretch,&borderc_set);

		node1 = son2(node1);
    	}

	/* The default title inherit mechanism */

	if (NTITLE(v)==NULL) SYERR(node,"Missing title of a node");
	if (NLABEL(v)==NULL) NLABEL(v) = NTITLE(v);

	/* For the folding keepers */

	if (NFOLDING(v)==0) {
		/* for the initial folding: we must recognize the
		 * foldstoppers.
		 */
		NFOLDING(v) = -1;
		add_foldstop(v);
	}
	else if (NFOLDING(v)>0) {
		/* and the fold starters */
		add_foldstart(v);
	}
	check_node(node,v);  /* check node and init into to hashtable */
}



/*  Analysis of one node attribute
 *  ------------------------------
 *  node2 is a syntax tree node, which is an attribute.
 *  v is a node whose attributes are now filled.
 */          	

#ifdef ANSI_C
static void	one_node_attribute(
			yysyntaxtree node2,
			GNODE v,
			int rootshrink,
			int rootstretch,
			int *borderc_set)
#else
static void	one_node_attribute(node2,v,rootshrink,rootstretch, borderc_set)
yysyntaxtree	node2;
GNODE	v;
int     rootshrink;	/* shrink factor of the root   */
int 	rootstretch;	/* stretch factor of the root  */
int	*borderc_set;	/* flag if borderc was set     */
#endif
{
	debugmessage("one_node_attribute","");
	assert((node2));

	switch(tag(node2)) {
	case T_Co_title:
		NTITLE(v) = SDecode(son1(node2));
	    	break;
	case T_Co_label:
		NLABEL(v) = SDecode(son1(node2));
		break;
	case T_Co_info1:
		NINFO1(v) = SDecode(son1(node2));
		break;
	case T_Co_info2:
		NINFO2(v) = SDecode(son1(node2));
		break;
	case T_Co_info3:
		NINFO3(v) = SDecode(son1(node2));
		break;
	case T_Co_level:
		NLEVEL(v) = (int)FETCHNUM();
		break;
	case T_Co_shape:
		switch(tag(son1(node2))) {
                case T_Co_box:      NSHAPE(v) = BOX;      break;
                case T_Co_rhomb:    NSHAPE(v) = RHOMB;    break;
                case T_Co_ellipse:  NSHAPE(v) = ELLIPSE;  break;
              	case T_Co_triangle: NSHAPE(v) = TRIANGLE; break;
		}
		break;
	case T_Co_horizontal_order:
		NHORDER(v) = (int)FETCHNUM();
		break;
	case T_Co_textmode:
		switch(tag(son1(node2))) {
		case T_Co_center:        NTEXTMODE(v) = CENTER; break;
		case T_Co_left_justify:  NTEXTMODE(v) = LEFT;   break;
		case T_Co_right_justify: NTEXTMODE(v) = RIGHT;  break;
	    	}
		break;
	case T_Co_width:
		NWIDTH(v) = (int)FETCHNUM();          
		break;
	case T_Co_height:
		NHEIGHT(v) = (int)FETCHNUM();          
		break;
	case T_Co_borderwidth:
		NBORDERW(v) = (int)FETCHNUM();          
		break;  
	case T_Co_loc:
		NSX(v) = get_lnum(son1(node2));
		NSY(v) = get_lnum(son2(node2));
		break;
	case T_Co_xdef:
		NSX(v) = FETCHNUM();
		break;
	case T_Co_ydef:
		NSY(v) = FETCHNUM();          
		break;
	case T_Co_folding:
		NFOLDING(v) = (int)FETCHNUM();          
		break;
	case T_Co_color:
		NCOLOR(v) = get_color(son1(node2));
		break;
	case T_Co_textcolor:
		NTCOLOR(v) = get_color(son1(node2));
		if (!(*borderc_set))
			NBCOLOR(v) = get_color(son1(node2));
		break;
	case T_Co_bordercolor:
		NBCOLOR(v) = get_color(son1(node2));
		*borderc_set = 1;
		break;
                case T_Co_scaling:
		{
			double hhx;

                     	hhx = FETCHFLOAT();
			if (hhx<0) hhx= -hhx;
			if (hhx>1.0) {
				NSTRETCH(v) = 100;
				NSHRINK(v) = (int)(100.0/hhx);
			}
			else {
				NSTRETCH(v) = (int)(hhx*100.0);
				NSHRINK(v) = 100;
			}
			NSHRINK(v) = rootshrink*NSHRINK(v);
			NSTRETCH(v)= rootstretch*NSTRETCH(v);
			if (NSHRINK(v)==0)  NSHRINK(v) = 1;
			if (NSTRETCH(v)==0) NSTRETCH(v) = 1;
		}
	case T_Co_shrink:
		NSHRINK(v) = rootshrink * (int)FETCHNUM();          
		if (NSHRINK(v) == 0) NSHRINK(v) = 1;
		break;
	case T_Co_stretch:
		NSTRETCH(v) = rootstretch * (int)FETCHNUM();          
		if (NSTRETCH(v) == 0) NSTRETCH(v) = 1;
		break;
	default:
		if (silent) break;
		FPRINTF(stderr,"Line %d: attribute %s ",
			xfirst_line(node2),ConstructorName(tag(node2)));
		FPRINTF(stderr,"currently not implemented !\n");
	}
}




/*--------------------------------------------------------------------*/

/*  Analysis of the edge attributes
 *  -------------------------------
 *  node is a syntax tree node of sort edge_attribute_list.
 *  e is an edge whose attributes are now filled.
 *  Additionally, it is checked whether the edge has valid start and 
 *  end nodes.
 */          	


#ifdef ANSI_C
static void	edge_attributes(yysyntaxtree node, GEDGE e)
#else
static void	edge_attributes(node,e)
yysyntaxtree	node;
GEDGE	e;
#endif
{
	register yysyntaxtree	node1, node2;
	int elcol_set, arrowc_set, barrowc_set;

	debugmessage("edge_attributes","");
	assert((!node)||(tag(node)==T_Co_edge_attribute));
	elcol_set   = 0;
	arrowc_set  = 0;
	barrowc_set = 0;
	node1 = node;
	while ( node1 && (tag(node1)==T_Co_edge_attribute) ) {
	    	node2 = son1(node1);
		assert(node2);
		one_edge_attribute(node2,e,&elcol_set,&arrowc_set,&barrowc_set);

		node1 = son2(node1);
	}
	if ((ESTART(e)==NULL)||(EEND(e)==NULL)) 
		SYERR(node,"Missing source or target of an edge");
	if (EPRIO(e) > max_eprio) max_eprio = EPRIO(e);
}



/*  Analysis of one edge attribute
 *  ------------------------------
 *  node2 is a syntax tree node, which is an attribute.
 *  e is an edge whose attributes are now filled.
 */          	

#ifdef ANSI_C
static void	one_edge_attribute(
			yysyntaxtree node2,
			GEDGE e,
			int *elcol_set,
			int *arrowc_set,
			int *barrowc_set)
#else
static void	one_edge_attribute(node2,e,elcol_set,arrowc_set, barrowc_set)
yysyntaxtree	node2;
GEDGE	e;
int	*elcol_set;	/* flag if edge label color was set */
int	*arrowc_set;	/* flag if arrow color was set      */
int	*barrowc_set;	/* flag if back arrow color was set */
#endif
{
	debugmessage("one_edge_attribute","");
	assert((node2));

	switch(tag(node2)) {
	case T_Co_sourcename:
		ESTART(e) = search_node(node2,SDecode(son1(node2)));
		break;
	case T_Co_targetname:
		EEND(e)   = search_node(node2,SDecode(son1(node2)));
		break;
	case T_Co_linestyle:
		switch(tag(son1(node2))) {
                case T_Co_continuous: ELSTYLE(e) = SOLID;     break;
                case T_Co_dotted:     ELSTYLE(e) = DOTTED;    break; 
                case T_Co_dashed:     ELSTYLE(e) = DASHED;    break; 
		case T_Co_invisible:  ELSTYLE(e) = UNVISIBLE; break;
		default: assert((0));
                }
		break;
	case T_Co_label:
		ELABEL(e) = SDecode(son1(node2));
		break;
	case T_Co_thickness:
		ETHICKNESS(e) = (int)FETCHNUM();          
		break;
	case T_Co_class:
		ECLASS(e)     = (int)FETCHNUM();          
		if (ECLASS(e)<=0) ECLASS(e) = 1;
		break;
	case T_Co_priority:
		EPRIO(e)     = (int)FETCHNUM();          
		if (EPRIO(e)<=0) EPRIO(e) = 1;
		break;
	case T_Co_color:
		ECOLOR(e) = get_color(son1(node2));
		if (!(*elcol_set))
			ELABELCOL(e) = get_color(son1(node2));
		if (!(*arrowc_set))
			EARROWCOL(e) = get_color(son1(node2));
		if (!(*barrowc_set))
			EARROWBCOL(e) = get_color(son1(node2));
		break;
	case T_Co_textcolor:
		ELABELCOL(e) = get_color(son1(node2));
		*elcol_set = 1;
		break;
        case T_Co_horizontal_order:
		EHORDER(e) = (int)FETCHNUM();
		break;
	case T_Co_arrowsize:
		EARROWSIZE(e) = (int)FETCHNUM();          
		break;
	case T_Co_barrowsize:
		EARROWBSIZE(e) = (int)FETCHNUM();          
		break;
	case T_Co_arrowcolor:
		EARROWCOL(e) = get_color(son1(node2));
		*arrowc_set = 1;
		break;
	case T_Co_barrowcolor:
		EARROWBCOL(e) = get_color(son1(node2));
		*barrowc_set = 1;
		break;
	case T_Co_arrowstyle:
		switch(tag(son1(node2))) {
		case T_Co_none:  EARROWSTYLE(e) = ASNONESPEC; break;
		case T_Co_line:  EARROWSTYLE(e) = ASLINE;     break;
		case T_Co_solid: EARROWSTYLE(e) = ASSOLID;    break;
		default: assert((0));
		}
		break;
	case T_Co_barrowstyle:
		switch(tag(son1(node2))) {
		case T_Co_none:  EARROWBSTYLE(e) = ASNONESPEC; break;
		case T_Co_line:  EARROWBSTYLE(e) = ASLINE;     break;
		case T_Co_solid: EARROWBSTYLE(e) = ASSOLID;    break;
		default: assert((0));
		}
		break;
	case T_Co_anchor:
		EANCHOR(e) = (int)FETCHNUM();          
		if ((EANCHOR(e)<=0)||(EANCHOR(e)>=63)) 
			SYERR(node2,"Illegal anchorpoint");
		break;
	default:
		FPRINTF(stderr,"Line %d: attribute %s ",
			xfirst_line(node2),ConstructorName(tag(node2)));
		FPRINTF(stderr,"currently not implemented !\n");
	}
}


/*--------------------------------------------------------------------*/

/*  Analysis of the Color
 *  ---------------------
 *  node is a syntax tree node of sort enum_color.
 *  The return value is the analyzed color.
 */          	


#ifdef ANSI_C
static int get_color(yysyntaxtree node)
#else
static int get_color(node)
yysyntaxtree node;
#endif
{
	int res;

	debugmessage("get_color","");
	switch(tag(node)) {
	case T_Co_black:
		return(BLACK);
	case T_Co_blue:
		return(BLUE);
	case T_Co_red:
		return(RED);
	case T_Co_green:
		return(GREEN);
	case T_Co_yellow:
		return(YELLOW);
	case T_Co_magenta:
		return(MAGENTA);
	case T_Co_cyan:
		return(CYAN);
	case T_Co_white:
		return(WHITE);
	case T_Co_darkgrey:
		return(DARKGREY);
	case T_Co_darkblue:
		return(DARKBLUE);
	case T_Co_darkred:
		return(DARKRED);
	case T_Co_darkgreen:
		return(DARKGREEN);
	case T_Co_darkyellow:
		return(DARKYELLOW);
	case T_Co_darkmagenta:
		return(DARKMAGENTA);
	case T_Co_darkcyan:
		return(DARKCYAN);
	case T_Co_gold:
		return(GOLD);
	case T_Co_lightgrey:
		return(LIGHTGREY);
	case T_Co_lightblue:
		return(LIGHTBLUE);
	case T_Co_lightred:
		return(LIGHTRED);
	case T_Co_lightgreen:
		return(LIGHTGREEN);
	case T_Co_lightyellow:
		return(LIGHTYELLOW);
	case T_Co_lightmagenta:
		return(LIGHTMAGENTA);
	case T_Co_lightcyan:
		return(LIGHTCYAN);
	case T_Co_lilac:
		return(LILAC);
	case T_Co_turquoise:
		return(TURQUOISE);
	case T_Co_aquamarine:
		return(AQUAMARINE);
	case T_Co_khaki:
		return(KHAKI);
	case T_Co_purple:
		return(PURPLE);
	case T_Co_yellowgreen:
		return(YELLOWGREEN);
	case T_Co_pink:
		return(PINK);
	case T_Co_orange:
		return(ORANGE);
	case T_Co_orchid:
		return(ORCHID);
        case T_Co_colindex:
		res = (int)get_lnum(son1(node));
		if (res >= cmap_size) {
			SYERR(node,"Illegal color index.\nColor entries must be declared first");
		}
		return(res);
	}
	assert((0));  /* we should never come to this point */
	return(BLACK);
}

/*--------------------------------------------------------------------*/

/*  Analysis of a yes/no switch
 *  ---------------------------
 *  return 1 for yes and 0 for no.
 */


#ifdef ANSI_C
static int get_yesno(yysyntaxtree node)
#else
static int get_yesno(node)
yysyntaxtree node;
#endif
{
	debugmessage("get_yesno","");

        switch(tag(node)) {
    	case T_Co_yes:  return(YES);
    	case T_Co_no:   return(NO);
	default: assert((0));
	}
	return(0);
}


/*--------------------------------------------------------------------*/

/*   Calculate the maximal number of classes
 *   ---------------------------------------
 *   We traverse recursively the syntax tree to look for class declarations.
 *   The maximal number that occurs is stored into max_nr_classes. 
 */


#ifdef ANSI_C
static void calc_nr_classes(yysyntaxtree node)
#else
static void calc_nr_classes(node)
yysyntaxtree node;
#endif
{
	yysyntaxtree node1, node2;
	int h;

	debugmessage("calc_nr_classes","");

	assert((node && (tag(node) == T_Co_graph_entry)));
	while ( node && (tag(node) == T_Co_graph_entry) ) {
		node1 = son1(node);
		assert(node1);
	        switch (tag(node1)) {
		case T_Co_graph:
			calc_nr_classes(son1(node1));
			break;

		case T_Co_graph_attribute:

			node2 = son1(node1);
			assert((node2));
			switch(tag(node2)) {
			case T_Co_classname:
			case T_Co_hidden:
				h = (int)FETCHNUM();
				if (h>max_nr_classes) max_nr_classes = h;
                        	break;
			}
			break;

		case T_Co_foldedge_defaults:
		case T_Co_edge_defaults:

			node2 = son1(node1);
			assert((node2));
			switch(tag(node2)) {
			case T_Co_class:
				h = (int)FETCHNUM();
				if (h>max_nr_classes) max_nr_classes = h;
				break;
			}
			break;

		case T_Co_edge:
		case T_Co_near_edge:
		case T_Co_bent_near_edge:
		case T_Co_back_edge:

			node1 = son1(node1);
			while ( node1 && (tag(node1)==T_Co_edge_attribute) ) {
	   		 	node2 = son1(node1);
				assert(node2);
	    			switch(tag(node2)) {
				case T_Co_class:
					h = (int)FETCHNUM();
					if (h>max_nr_classes) 
						max_nr_classes = h;
					break;
				}
				node1 = son2(node1);
			}
		}
		node = son2(node);
	} /* while */	
}

/*--------------------------------------------------------------------*/

/*  Node Search functions 
 *  =====================
 */          	

static char buffer[1024];      /* Buffer to create error messages */


/*  Node Search 
 *  ----------- 
 *  Search a node in the nodelist and graphlist.
 *  Creates an error message, if the node is not avalable.
 */          	

#ifdef ANSI_C
static GNODE search_node(yysyntaxtree x,char *title)
#else
static GNODE search_node(x,title)
yysyntaxtree x;
char *title;
#endif
{
	GNODE n;
	debugmessage("search_node",(title?title:"(null)"));
	n = lookup_hashnode(title);
	if (n==NULL) {
		SPRINTF(buffer,"Undefined node %s",title);
		SYERR(x,buffer);
	}
        return(n);
}


/*  Visible Node Search
 *  -------------------
 *  Search a node in the nodelist and graphlist.
 *  Return NULL, if the node is not avalable or invisible.
 */

#ifdef ANSI_C
GNODE   search_visible_node(char *title)
#else
GNODE   search_visible_node(title)
char    *title;
#endif
{
	GNODE   n;
	if (!title) return(NULL);
	debugmessage("search_visible_node",title);
	n = lookup_hashnode(title);

	/* note: at that time point are NINLIST and NINVISIBLE inverse */

	return(n);
}



/*  Node Check  
 *  - - - - -  
 *  Check whether a node already exists in the node list and graphlist.
 *  Creates an error message, if the node is avalable.
 *  Furthermore: insert the node into the hash table.
 */          	
 
#ifdef ANSI_C
static void check_node(yysyntaxtree x, GNODE m)
#else
static void check_node(x,m)
yysyntaxtree x;
GNODE m;
#endif
{
	char *title;
	GNODE n;

	assert((m));
	title = NTITLE(m);
	debugmessage("check_node",(title?title:"(null)"));
	if (title && (!fastflag)) {
		n = lookup_hashnode(title);
		if (n!=NULL) {
			SPRINTF(buffer,"Double defined node %s",title);
			SYERR(x,buffer);
		}
	} 
	insert_hashnode(m);
}
 

/*--------------------------------------------------------------------*/

/*  Node Lookup Hashtable
 *  =====================
 *  To find a node very fast, we use a hash table.
 *  Because in the hash table are no temporary nodes, we can
 *  misuse the NINTERN field of nodes.
 *  The hash table is an array whose entries are lists of GNODE
 *  objects, linked by the NINTERN field. 
 *  The are hashed with respect to the title.
 */

/* The size of the hashtable maxhashtable should be a prime number. 
 * Good candidates are: 101, 211, 307, 503, 1013, 1511, 2003, 3001, 5003, ...  
 */


/*  The hash table 
 */

static int maxhashtable = 211;
static GNODE *hashtable = 0;


/*  Initialization of the Hashtable
 *  -------------------------------
 */ 

#ifdef ANSI_C
static void init_hashtable(void)
#else
static void init_hashtable()
#endif
{
	int i;
	if (!hashtable) {
		numnodes = 0;
		estimate_num_nodes(Syntax_Tree);

		if (numnodes>30000)      maxhashtable = 32003; 
		else if (numnodes>25000) maxhashtable = 30011; 
		else if (numnodes>20000) maxhashtable = 25013; 
		else if (numnodes>15000) maxhashtable = 20011; 
		else if (numnodes>10000) maxhashtable = 15013; 
		else if (numnodes>7500)  maxhashtable = 10007;
		else if (numnodes>5000)  maxhashtable = 7507;
		else if (numnodes>2000)  maxhashtable = 5003;
		else maxhashtable = 2003;
		hashtable = (GNODE *)malloc(maxhashtable*sizeof(GNODE));
		if (!hashtable) {
			Fatal_error("memory exhausted","");
		}
#ifdef DEBUG
		PRINTF("Number of detected nodes: %d\n", numnodes);
               	PRINTF("Sizeof tables `hashtable': %ld Bytes\n",
                        (maxhashtable)*sizeof(GNODE));
#endif
	}
	for (i=0; i<maxhashtable; i++) hashtable[i]=NULL;
}


/*  Hashvalue calculation.
 *  ---------------------
 *  The get an useful dispersion, we add for each character 101 and
 *  the ASCII value of the character.  
 *  The hashvalue is between 0 and maxhashtable-1.
 */

#ifdef ANSI_C
static int hashval(char *s)
#else
static int hashval(s)
char *s;
#endif
{
	register int res;

	assert((s));
	res = 0;
	while (*s) { res = (10 * res + *s + 101) % maxhashtable; s++; }
	if (res<0) res = -res;
	return(res % maxhashtable);
}

/*  Insert a node into the hash table.
 *  ----------------------------------
 *  x is the node. The insert is done blind, i.e. it is not checked
 *  whether x is already in the table.
 *  Usually, graph summary nodes and real nodes are inserted after
 *  fetching all attributes.
 */


#ifdef ANSI_C
static void insert_hashnode(GNODE x)
#else
static void insert_hashnode(x)
GNODE x;
#endif
{
	char *title;
	int val;

	assert((x));
	title = NTITLE(x);
	debugmessage("insert_hashnode",(title?title:"(null)"));
	if (!title) {
		FPRINTF(stderr,"Missing title of a node");
		return;
	}
	val = hashval(title);
	assert((NINTERN(x)==NULL));
	NINTERN(x) = hashtable[val];		
	hashtable[val] = x;
}

/*  Lookup in the hash table
 *  ------------------------
 *  returns the first node that has the title, or NULL.
 */

#ifdef ANSI_C
GNODE lookup_hashnode(char *title)
#else
GNODE lookup_hashnode(title)
char *title;
#endif
{
	GNODE h;

	debugmessage("lookup_hashnode",(title?title:"(null)"));
	h = hashtable[hashval(title)];
	while (h) {
                if (title==NTITLE(h)) return(h);
                if (strcmp(title,NTITLE(h))==0) return(h);
		h = NINTERN(h);
	}
	return(NULL);	
}


/*--------------------------------------------------------------------*/

/*  Hashtable Cursor
 *  ================
 *
 *  The hash table cursor points into the hashtable and can be
 *  moved backwards and forewards through the hashtable.
 *  It is used to allow easy selection of nodes.
 */

static GNODE act_hash_cursor;
static int   act_hash_pos;
int 	     act_hash_size;


/*  Initialize the cursor
 *  ---------------------
 */

#ifdef ANSI_C
void init_hash_cursor(void)
#else
void init_hash_cursor()
#endif
{
	GNODE h;
	int i;

	debugmessage("init_hash_cursor","");

	act_hash_cursor = NULL;
	act_hash_pos    = 0;
	act_hash_size   = 0;
	for (i=0; i<maxhashtable; i++)
               	if (hashtable[i]!=NULL) {
			if (!act_hash_cursor) {
				act_hash_cursor = hashtable[act_hash_pos];
				act_hash_pos = i;
			}
			h = hashtable[i];
			while (h) { act_hash_size++; h=NINTERN(h); }
		} 
}


/*  Position cursor 
 *  ---------------
 */

#ifdef ANSI_C
void position_hash_cursor(int j)
#else
void position_hash_cursor(j)
int j;
#endif
{
	GNODE h;
	int i;

	debugmessage("position_hash_cursor","");

	assert((j>=0));
	if (!act_hash_cursor) return;
	if (act_hash_size == 0) return;

	for (i=0; i<maxhashtable; i++)
               	if (hashtable[i]!=NULL) {
			h = hashtable[i];
			while (h) { 
				if (j==0) {
		                	act_hash_cursor = h;
                                	act_hash_pos    = i;
					return;
				}
				j--;
				h=NINTERN(h); 
			}
		} 
}



/*  Get i.th node after the cursor
 *  ------------------------------
 */

#ifdef ANSI_C
GNODE get_hash_cursor_succ(int i)
#else
GNODE get_hash_cursor_succ(i)
int i;
#endif
{
	GNODE h;
	int j;

	debugmessage("get_hash_cursor_succ","");

	if (!act_hash_cursor) return(NULL);
	if (i<0) i=0;

	j = act_hash_pos;
	h = act_hash_cursor;

	while (i>0) {
		i--;
		if (NINTERN(h)) h = NINTERN(h);
		else {
			j++;
			h = NULL;
			for (; j<maxhashtable; j++)
       		         	if (hashtable[j]!=NULL) {
					h = hashtable[j];
					break;
				} 
			if (!h) return(NULL); 
		}
	}
	return(h);
}


/*--------------------------------------------------------------------*/

/*  For debugging only:
 *  ------------------
 *  Routine that looks for a checknode. This is helpful if we need
 *  to inspect a certain node.
 */

#ifdef CHECKNODE

GNODE debug_checknode = NULL;


#ifdef ANSI_C
static void debug_init_checknode(void)
#else
static void debug_init_checknode()
#endif
{
	GNODE h;

	h = nodelist;
	while (h) {
		if (NREFNUM(h)==1L) {
			debug_checknode = h;
			return;
		}
		h = NNEXT(h);
	}
}

#endif /* CHECKNODE */


