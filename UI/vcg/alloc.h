/* SCCS-info %W% %E% */
 
/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         alloc.h                                            */
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
 * Revision 3.12  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.11  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 * infobox behaviour improved.
 * First version of fisheye (carthesian).
 * Options Noedge and nonode.
 * Titles in the node title box are now sorted.
 * Timelimit functionality improved.
 *
 * Revision 3.10  1994/08/05  12:13:25  sander
 * Treelayout added. Attributes "treefactor" and "spreadlevel" added.
 * Scaling as abbreviation of "stretch/shrink" added.
 *
 * Revision 3.9  1994/08/03  13:58:44  sander
 * Horizontal order mechanism changed.
 * Attribute horizontal_order for edges added.
 *
 * Revision 3.8  1994/08/02  15:36:12  sander
 * CHECKNODE option added to allow tracing of properties
 * of one single node.
 *
 * Revision 3.7  1994/06/07  14:09:59  sander
 * Splines implemented.
 * HP-UX, Linux, AIX, Sun-Os, IRIX compatibility tested.
 * The tool is now ready to be distributed.
 *
 * Revision 3.6  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
 *
 * Revision 3.5  1994/05/05  08:20:30  sander
 * dllist_free_all added for the local optimization of crossings.
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
 * Revision 3.2  1994/03/03  15:35:39  sander
 * Edge line style `invisible' added.
 *
 * Revision 3.1  1994/03/01  10:59:55  sander
 * Copyright and Gnu Licence message added.
 * Problem with "nearedges: no" and "selfloops" solved.
 *
 * Revision 2.4  1994/01/21  19:33:46  sander
 * VCG Version tested on Silicon Graphics IRIX, IBM R6000 AIX and Sun 3/60.
 * Option handling improved. Option -grabinputfocus installed.
 * X11 Font selection scheme implemented. The user can now select a font
 * during installation.
 * Sun K&R C (a nonansi compiler) tested. Some portabitility problems solved.
 *
 * Revision 2.3  1994/01/03  15:29:06  sander
 * First complete X11 version.
 */

#ifndef ALLOC_H 
#define ALLOC_H 

/*--------------------------------------------------------------------*/

/* Attribute values 
 *-----------------
 */

/* Color definitions 
 * - - - - - - - - -
 */

/*   COFFSET is the vertical offset: we need to make the drawing area 
 *   60 bits larger than specified, because of a bug in Sunview on
 *   color screens. 
 *   Maybe it is not needed for X11 (?)
 */

#define COFFSET		60

/* number of colors */

#define	CMAPSIZE 	256 	
#define	BASECMAPSIZE 	32 	

/* color access */

#define	COLOR(c)	(c)

#define BLACK		31
#define BLUE		1
#define RED		2
#define GREEN		3
#define YELLOW		4
#define MAGENTA		5
#define CYAN		6
#define WHITE		0
#define DARKGREY	7
#define DARKBLUE	8
#define DARKRED		9
#define DARKGREEN	10
#define DARKYELLOW	11
#define DARKMAGENTA	12
#define DARKCYAN	13
#define GOLD		14
#define LIGHTGREY	15
#define LIGHTBLUE	16
#define LIGHTRED	17
#define LIGHTGREEN	18
#define LIGHTYELLOW	19
#define LIGHTMAGENTA	20
#define LIGHTCYAN	21
#define LILAC		22
#define TURQUOISE	23
#define AQUAMARINE	24
#define KHAKI		25
#define PURPLE		26
#define YELLOWGREEN	27
#define PINK		28
#define ORANGE		29
#define ORCHID		30

/* Graph orientation */

#define	TOP_TO_BOTTOM	0
#define	LEFT_TO_RIGHT	1
#define	RIGHT_TO_LEFT	2
#define	BOTTOM_TO_TOP	3

/* Node y-alignment */

#define AL_TOP    0
#define AL_CENTER 1
#define AL_BOTTOM 2

/* Display edge labels */

#define NO	0
#define YES	1

/* Textmodes */

#define CENTER	0
#define	LEFT	1
#define	RIGHT	2

/* Linestyles */

#define	SOLID	   0
#define	DOTTED	   1
#define	DASHED	   2
#define	UNVISIBLE  3

/* Arrowstyles */

#define	ASNONE 	   0
#define	ASSOLID	   1
#define	ASLINE 	   2
#define	ASNONESPEC 3

/* Arrow modi */

#define AMFIXED 0
#define AMFREE  1

/* Shapes */

#define BOX      0
#define RHOMB    1
#define ELLIPSE  2
#define TRIANGLE 3

/* Edge orientation (no orientation, north, north east, ...) */

#define	NO_ORI		0
#define	ORI_NORTH	1
#define	ORI_NORTHEAST	2
#define	ORI_NORTHWEST	3
#define	ORI_SOUTH	4
#define ORI_SOUTHEAST	5
#define	ORI_SOUTHWEST	6
#define	ORI_EAST	7
#define	ORI_WEST	8

/*--------------------------------------------------------------------*/

/*  Auxiliary Structs
 *  =================
 */

/*  Connections
 *  ------------
 */

/*  Connections are necessary for the layout of directly neighboured
 *  connected nodes at the same level. This situation may occur accidently,
 *  but automatical occurs if there is a self loop.
 *
 *  On self loops: 
 *  - - - - - - -      N            A self loop of N is an edge (N,N).
 *                    / ^           To layout this, we create a dummy node
 *                   /   \          D and a pseudo node P. D and P are 
 *                  D-----P         always neighboured at the same level.
 *
 *  Only N and D is layouted. The connection of the dummy node D contains the 
 *  information needed to draw P (that is not layouted), i.e.:
 *     CTARGET(D)  -> P
 *     CEDGE(D)    -> edge (D,P)      
 *     CTARGET(P)  -> D
 *     CEDGE(P)    -> edge (D,P)      
 *
 *  On connected neigboured nodes:
 *  - - - - - - - - - - - - - - - 
 *              /    |    \       or    |     \      or     /     |
 *             A<----B---->C            B----->C           C<-----B
 *
 *  Only B is layouted. The connection of B contains the information
 *  to draw A and/or C, i.e.
 *     CTARGET(B)  -> C
 *     CEDGE(B)    -> edge (B,C)
 *     CTARGET2(B) -> A           or NULL, if not necessary
 *     CEDGE2(B)   -> edge (B,A)  or NULL, if not necessary
 *
 *     CTARGET(C)  -> B
 *     CEDGE(C)    -> edge (B,C)
 *     etc.
 *
 *  We call the connection field of B a `forward connection' and the
 *  connection field of B or A a `backward connection', because its
 *  in converse of the edge direction.
 *  Note that A or C can also contain connections, e.g. on
 *               |    \     \
 *               B---->C---->D           CTARGET(B) = C, CTARGET(C) = D, ...
 *
 *
 */ 
 

typedef struct connect {
	struct gnode	*target;        /* First found target         */
	struct gedge	*edge;          /* and its edge               */
	struct gnode	*target2;       /* Second found target        */
	struct gedge	*edge2;         /* and its edge               */
	struct connect	*internal_next; /* for memory allocation only */
} *CONNECT;

#define	CTARGET(x)	((x)->target)
#define CEDGE(x)	((x)->edge)
#define	CTARGET2(x)	((x)->target2)
#define CEDGE2(x)	((x)->edge2)
#define CINTERN(x)	((x)->internal_next)



/*--------------------------------------------------------------------*/

/*  Graphs             
 *  ======
 *  The complete data structure of a graph is an adjacency list
 *  representation, i.e. all nodes contain adjacency lists of
 *  incoming and outgoing edges.
 *  Furthermore, all nodes contain in NROOT a pointer to the subgraph
 *  summary node they belong to. NROOT of top level nodes is NULL.
 *  Summary nodes of subgraphs contain in NSGRAPH a list of all nodes
 *  that belong to this subgraph.
 *  Furthermore all real nodes are in the node list, while all graph
 *  summary nodes are in the graphlist.
 *  E.g. the top graph contains a node A and a graph S1
 *		which contains nodes B and C and a graph S2
 *				which contains nodes D and E 
 *  NROOT connection:
 *
 *     nodelist --> A --> B ---> C --> D ---> E 
 *                  |      \    /       \    /
 *                 NULL     \  /         \  /NROOT
 *                           \/           \/
 *     graphlist ----------> S1 --------> S2 
 *                            |            |
 *                            V            V
 *                          NULL          NULL
 *
 *                        ,-----------------------------,
 *  NSGRAPH connection:   |            ,-------------,  |
 *                        V            V             |  |
 *                  *->*  *----->*->*  *----->*      |  |
 *                  |  |  |      |  |  |      |      |  |
 *                  V  |  V      V  |  V      V      |  |
 *     nodelist --> A -+> B ---> C -+> D ---> E      |  |
 *                     |            |                |  |
 *                     `-----,      `-----,          |  |
 *                           |            |   NSGRAPH|  |NSGRAPH
 *                           V            V          |  |
 *     graphlist ----------> S1 --------> S2         |  |
 *                           |            |          |  |
 *                           |            `----------'  |
 *                           `--------------------------'
 */


/*  Nodes of a graph
 *  ----------------
 *  Graphs are also represented as nodes, i.e. as summary node.
 *  Graphs s are initially not in the nodelist, but in the graphlist.
 *  In consequence, NINLIST(s) is 0 for them. 
 */

typedef struct gnode {	

	/* Each node has an unique reference number.
 	 * These are used to debug, and to have a stable layout,
	 * because we use the numbers as sorting criteria.
	 */

	long 	refnum;

	/* These attributes come directly from the specification */

	char 	*title;
	char	*label;
	char	*info1;
	char	*info2;
	char	*info3;
	int	width;
	int	height;
	long	sxloc;
	long 	syloc;
	char	textmode;
	char	borderwidth;
	char	folding;
	char	color;
	char	textcolor;
	char	bordercolor;
	char	status;
	char 	shape;
	int	shrink;
	int	stretch;
	int	level;
	int	nhorder;

	/* These attributes are on summary nodes (graphs,regions) */

	struct	gnlist	*subgraph;   /* List of subgraphs       */
	struct	gnode	*root;       /* Root of graph           */
	struct	gnode	*regionrepl; /* Replacement node for    */
				     /* roots of regions        */
	struct	gnlist	*region;     /* List of nodes in region */
	struct 	gnode  	*regroot;    /* Root of region          */

	/* These are the locations used for the layout */

	long	xloc;
	long 	yloc;

	/* nodelist, graphlist links: these are double linked lists */

	struct 	gnode 	*next;	    /* successor in the node/graphlist   */ 
	struct 	gnode	*before;    /* predecessor in the node/graphlist */

	char	in_nodelist;	    /* flag, 1 if node is in nodelist */
	char	invisible;          /* flag, 1 if node is invisible   */

	/* layout attributes: nodes are distributed into layers according
         * to their deeths relatively to the root of the graph, and have
         * positions in these layers.
         */

	char	markiert;           /* marker for depth first search    */    
	char	revert;             /* marker for reverted drawing      */    
	char    anchordummy;	    /* marker for anchor dummy nodes    */
	int	tiefe;		    /* the number (deepth) of the layer */
	int	position;           /* the position in the layer        */
	float	bary;               /* the weight of the bary centering */

	/* The following two fields have sev. purposes: they are the layout
         * weights nws and nwp of the layout algorithm, and on drawing
         * they are used as number of anchor points.
	 * The field weights is also used during the partitioning of
	 * edges as LOWPT of the strongly connected component.
	 * The field weightp is aslo used during the partitioning of
	 * edges as OPENSCC flag of the strongly connected component.
         */

	long	weights;		/* node weight succ. (nws)   */
	long	weightp;		/* node weight pred. (nwp)   */

	/* For partitioning the edges, we need to know cross edges, tree
         * edges, forward edges and backward edges. Thus, we calculate a
         * depth first search (dfs) and protocol by numbers when we entry
         * the dfs for a node and when we leave the dfs of this node.
         */

	long	dfsnum;			/* the dfs entry number      */

	int	indegree;		/* number of incoming edges  */
	int	outdegree;		/* number of outgoing edges  */

	/* To calculate crossings, we need a pointer to the last instance
         * of the upper or lower completion lists. See step2.c
         */

	struct 	dllist	*Vpointer; 	/* this crossing pointer        */

	/* The graph is represented with adjacency lists. We have some
         * fast accesses to the leftest or rightedst prede/successor.
	 * Further, at connections we temporary change the adjacency
	 * lists. Thus we store the original list in save fields.
	 * For crossing calculation, we need to reorder the adjacency
	 * lists. Thus we use a pointer tmpadj to the actual position
	 * in the adjacency list.
         */

	struct	adjedge	*tmpadj;      	/* temporary adjacency list     */
	struct	adjedge	*pred;	       	/* adjacency list: predecessors */
	struct	adjedge	*succ;	       	/* adjacency list: successors   */
	struct	adjedge	*savepred;     	/* adjacency list: predecessors */
	struct	adjedge	*savesucc;     	/* adjacency list: successors   */
	struct	gedge	*predleft;     	/* leftest predecessor          */
	struct	gedge	*predright;    	/* rightest predecessor         */
	struct	gedge	*succleft;	/* leftest successor		*/
	struct	gedge	*succright;	/* rightest successor           */

	struct	connect	*connection;	/* horizontal connection to a   */
					/* neighboured node, see above  */

	/* The next field has two purposes: During parsing, we use it to
         * create a hash table of all nontemporary node, to have fast
	 * access to the title.
	 * Otherwise, it is used for the memory management of temporary 
	 * nodes.
	 */

	struct 	gnode 	*internal_next; 
} *GNODE;

#define	NREFNUM(x)	((x)->refnum)
#define	NTITLE(x)	((x)->title)
#define	NLABEL(x)	((x)->label)
#define	NINFO1(x)	((x)->info1)
#define	NINFO2(x)	((x)->info2)
#define	NINFO3(x)	((x)->info3)
#define NTEXTMODE(x)	((x)->textmode)
#define NWIDTH(x)	((x)->width)
#define	NHEIGHT(x)	((x)->height)
#define	NSTATE(x)	((x)->status)
#define	NSHAPE(x)	((x)->shape)
#define NBORDERW(x)	((x)->borderwidth)
#define	NSX(x)		((x)->sxloc)
#define NSY(x)		((x)->syloc)
#define	NX(x)		((x)->xloc)
#define NY(x)		((x)->yloc)
#define	NFOLDING(x)	((x)->folding)
#define NCOLOR(x)	((x)->color)
#define	NTCOLOR(x)	((x)->textcolor)
#define	NBCOLOR(x)	((x)->bordercolor)
#define	NSHRINK(x)	((x)->shrink)
#define	NSTRETCH(x)	((x)->stretch)
#define	NLEVEL(x)	((x)->level)
#define	NHORDER(x)	((x)->nhorder)
#define	NSGRAPH(x)	((x)->subgraph)
#define	NROOT(x)	((x)->root)
#define NREGREPL(x)	((x)->regionrepl)
#define NREGION(x)	((x)->region)
#define	NREGROOT(x)	((x)->regroot)
#define	NNEXT(x)	((x)->next)
#define NBEFORE(x)	((x)->before)
#define NINLIST(x)	((x)->in_nodelist)
#define	NINVISIBLE(x)	((x)->invisible)
#define	NTIEFE(x)	((x)->tiefe)
#define	NPOS(x)		((x)->position)
#define	NMARK(x)	((x)->markiert)
#define	NREVERT(x)	((x)->revert)
#define	NANCHORNODE(x)	((x)->anchordummy)
#define	NBARY(x)	((x)->bary)
#define	NLOWPT(x)	((x)->weights)
#define	NOPENSCC(x)	((x)->weightp)
#define	NWEIGHTS(x)	((x)->weights)
#define	NWEIGHTP(x)	((x)->weightp)
#define	NDFS(x)		((x)->dfsnum)
#define	NHIGHPRIO(x)	((x)->dfsnum)
#define	NINDEG(x)	((x)->indegree)
#define	NOUTDEG(x)	((x)->outdegree)
#define	NVPTR(x)	((x)->Vpointer)
#define	NTMPADJ(x)	((x)->tmpadj)
#define	NPRED(x)	((x)->pred)
#define NSUCC(x)	((x)->succ)
#define	NSVPRED(x)	((x)->savepred)
#define NSVSUCC(x)	((x)->savesucc)
#define	NPREDL(x)	((x)->predleft)
#define	NPREDR(x)	((x)->predright)
#define	NSUCCL(x)	((x)->succleft)
#define	NSUCCR(x)	((x)->succright)
#define NCONNECT(x)	((x)->connection)
#define NINTERN(x)	((x)->internal_next)

#define	NCONTAR(x)	(CTARGET(NCONNECT(x)))
#define NCONEDG(x)	(CEDGE(NCONNECT(x)))
#define	NCONTAR2(x)	(CTARGET2(NCONNECT(x)))
#define NCONEDG2(x)	(CEDGE2(NCONNECT(x)))

/* For NREVERT, we allow the values:
 * ---------------------------------
 */

#define NOREVERT  0
#define AREVERT   1
#define BREVERT   2


/*--------------------------------------------------------------------*/

/*  List of GNODE-objects 
 *  ---------------------
 *  These are used for various reasons: as list of nodes of a (sub)graph,
 *  as lists of nodes of a region, etc.
 */

typedef struct gnlist {
	struct gnode 	*node;           /* The node              */
	struct gnlist 	*next;           /* The remaining list    */
	struct gnlist	*internal_next;  /* For memory management */
} *GNLIST;

#define	GNNODE(x)	((x)->node)
#define	GNNEXT(x)	((x)->next)
#define	GNINTERN(x)	((x)->internal_next)

/*--------------------------------------------------------------------*/


/*  Edges of a graph
 *  ----------------
 *  During the layout, edges are partitioned into reverted edges,
 *  etc. Thus, we have for `kantenart', or EART, or EKIND, respectively,
 *  the following possibilities:
 *	'U' -> the kind of this edge is undefined, (it is a normal edge)
 *	'S' -> this is a self loop
 *	'D' -> this is a biconnection, i.e. arrows at both sides
 *	'R' -> this is a reverted edge.
 *  If we do not self-layout, we have further:
 *	'l' -> this edge goes to the left
 *	'r' -> this edge goes to the right 
 *
 *  The following tags indicate the initial partitionig, but they are not 
 *  anymore used.
 *	'T' -> this is a tree edge
 *	'F' -> this is a forward edge
 *	'B' -> this is a backward edge (not yet reverted)
 *	'C' -> this is a cross edge
 *
 *  For the x,y co-ordinates, there is a problem if neighoured nodes
 *  are to large. To avoid (1), we allow that edges are bend, see (2) 
 *
 *             -   --------                 -   --------
 *            |A| |        |               |A| |        |
 *             -  |        |                -  |        |
 *              \ |   M    |                |  |   M    |
 *   (1)         \|        |          (2)   |  |        |
 *                \        |                |  |        |
 *                |\       |                |  |        |
 *                 -\------                  \  --------
 *                   \                        \
 *
 *      i.e. each edge have start point and end point, and further a
 *      bend point that has the same x-co-ordinate as the start point
 *      (hold for top_down_layout).
 */

typedef struct gedge {
	struct	gnode	*start;    /* Source node             */
	long	sxloc;             /* and its co-ordinates    */
	long	syloc;
	long	btxloc;		   /* Bend location on top of */
	long	btyloc;		   /* a node                  */
	long	bbxloc;		   /* Bend location on bottom */
	long	bbyloc;
	struct	gnode	*end;      /* Target node             */ 	
	long	exloc;             /* and its co-ordinates    */
	long	eyloc;
	char	orientation; 	   /* updown, northwest, etc. */      
	char	orientation2; 	   /* dito, for double edges  */      

	/* These attributes come directly from the specification */

	char	linestyle;
	char	thickness;
	char	*label;
	int	priority;
	int	horder;
	char	eclass;
	char	color;
	char 	arrowstyle1;	
	char 	arrowstyle2;	
	char	arrowsize1;
	char	arrowcolor1;
	char	arrowsize2;
	char	arrowcolor2;
	char	labelcolor;

#ifdef ANSI_C
#ifdef VMS
	int     anchor;
#else
	signed char    anchor;
#endif
#else
	int     anchor;
#endif

	/* The layout decides wether an edge is visible or reverted, etc. */

	char	kantenart;	    /* flag 'U', 'S' etc. see above  */
	char	invisible;	    /* 1, if the edge is not visible */

	/* The following two fields are the layout weights ews and ewp of 
	 * the layout algorithm. See the documentation of the layout alg.
         * On drawing, they are further used as number of anchor point.
         */

	long	weights;          /* edge weight succ. (ews)   */
	long	weightp;	  /* edge weight pred. (ewp)   */

	struct	gnode	*labelnode; /* Label node if the edge is replaced */ 	

	/* edgelist links: these are double linked lists */

	struct	gedge	*next;		 /* successor in the edgelist   */
	struct 	gedge	*before; 	 /* predecessor in the edgelist */
	struct	gedge	*internal_next;  /* for memory allocation       */
} *GEDGE;

#define	ESTART(x)	((x)->start)
#define	EEND(x)		((x)->end)
#define	ESTARTX(x)	((x)->sxloc)
#define ESTARTY(x)	((x)->syloc)
#define	ETBENDX(x)	((x)->btxloc)
#define ETBENDY(x)	((x)->btyloc)
#define	EBBENDX(x)	((x)->bbxloc)
#define EBBENDY(x)	((x)->bbyloc)
#define	EENDX(x)	((x)->exloc)
#define	EENDY(x)	((x)->eyloc)
#define EORI(x)		((x)->orientation)
#define EORI2(x)	((x)->orientation2)
#define	ELABEL(x)	((x)->label)
#define	ELABELCOL(x)	((x)->labelcolor)
#define	ELSTYLE(x)	((x)->linestyle)
#define	ETHICKNESS(x)	((x)->thickness)
#define	ECLASS(x)	((x)->eclass)
#define	EPRIO(x)	((x)->priority)
#define	EHORDER(x)	((x)->horder)
#define ECOLOR(x)	((x)->color)
#define	EARROWSTYLE(x)	((x)->arrowstyle1)
#define	EARROWBSTYLE(x)	((x)->arrowstyle2)
#define	EARROWSIZE(x)	((x)->arrowsize1)
#define	EARROWBSIZE(x)	((x)->arrowsize2)
#define	EARROWCOL(x)	((x)->arrowcolor1)
#define	EARROWBCOL(x)	((x)->arrowcolor2)
#define	EANCHOR(x)	((x)->anchor)
#define	ELNODE(x)	((x)->labelnode)
#define	ENEXT(x)	((x)->next)
#define	EBEFORE(x)	((x)->before)
#define EART(x)		((x)->kantenart)
#define	EINVISIBLE(x)	((x)->invisible)
#define	EWEIGHTS(x)	((x)->weights)
#define	EWEIGHTP(x)	((x)->weightp)
#define EINTERN(x)	((x)->internal_next)



/*--------------------------------------------------------------------*/

/*  Adjacency lists
 *  ---------------
 *  Graphs are represented as adjacency list, see above.  
 *  The cons cell of the list of edges used in adjacency list
 *  is the struct adjedge.
 */

typedef struct adjedge {
	struct gedge 	*kante;           /* Edge */
	struct adjedge	*next;
	struct adjedge	*internal_next;
} *ADJEDGE;

/* direct accesses */

#define AKANTE(x)	((x)->kante)
#define	ANEXT(x)	((x)->next)
#define	AINTERN(x)	((x)->internal_next)

/* accesses to attributes of source and target of an edge */

#define	SOURCE(x)	(ESTART(AKANTE(x)))
#define	TARGET(x)	(EEND(AKANTE(x)))
#define	ESOURCEX(x)	(ESTARTX(AKANTE(x)))
#define	ESOURCEY(x)	(ESTARTY(AKANTE(x)))
#define ETARGETX(x)	(EENDX(AKANTE(x)))
#define	ETARGETY(x)	(EENDY(AKANTE(x)))
#define	EKIND(x)	(EART(AKANTE(x)))
#define	NSOURTIEFE(x)	(NTIEFE(ESTART(AKANTE(x))))
#define	NTARTIEFE(x)	(NTIEFE(EEND(AKANTE(x))))

/*--------------------------------------------------------------------*/

/*  Layout structs
 *  ==============
 *  To calculate the layout, the nodes are scheduled into layers.
 *  All nodes of one layer have the same vertical position, i.e.
 *  have the same deepth (level number) in the spanning tree of the graph.
 *  Inside the layers, the nodes are scheduled to minimize the crossings
 *  of the edges.
 */


/*  Depth entries 
 *  -------------
 *  This is an entry in the table of existing layers. Each layer 
 *  node contains the list of nodes of this layer, and the layout 
 *  values. For the list of nodes, we use two lists to traverse 
 *  the nodes forward and backward (similar to a double linked list).
 */

typedef struct depth_entry {
	int	anz;		     /* Number of nodes of this layer */
	int	actx;		     /* actual  x coord.in this layer */
	int 	cross;		     /* number of crossings of layer  */
	GNODE   refnode;	     /* Reference node for part 3     */
	struct	gnlist	*predlist;   /* nodes of this layer (forward) */ 
	struct	gnlist	*succlist;   /* nodes of this layer (backward)*/
	char	resort_necessary;    /* indicates if resorting by     */
				     /* barycentering etc. is needed  */
} DEPTH;

#define	TANZ(x)		((x).anz)
#define	TPRED(x)	((x).predlist)
#define	TSUCC(x)	((x).succlist)
#define	TCROSS(x)	((x).cross)
#define	TACTX(x)	((x).actx)
#define	TREFN(x)	((x).refnode)
#define	TRESNEEDED(x)	((x).resort_necessary)


/*--------------------------------------------------------------------*/

/*  Dllists
 *  -------
 *  To calculate the number of crossings, we use a plain sweep algorithm
 *  that uses two lists. These sets are implemented as double
 *  linked lists containing nodes.
 */

typedef	struct dllist {
	struct	gnode	*node;		/* the node              */
	int		dlinfo;		/* the node info	 */
	int		dlx;		/* the info coordinate   */
	struct	dllist	*pred;          /* predecessor cons cell */
	struct	dllist	*succ;          /* successor   cons cell */
} *DLLIST;

#define	DPRED(x)	((x)->pred)
#define	DSUCC(x)	((x)->succ)
#define	DNODE(x)	((x)->node)
#define	DNX(x)		((x)->dlx)
#define	DINFO(x)	((x)->dlinfo)

/*--------------------------------------------------------------------*/


/* see alloc.c for more information */

/* Global Variables
 * ----------------
 */

extern int nodeanz;
extern int dummyanz;
extern GNODE nodelist;
extern GNODE nodelistend;
extern GNODE tmpnodelist;
extern GNODE graphlist;
extern GNODE graphlistend;
extern GNODE invis_nodes;
extern GNODE labellist;
extern GNODE labellistend;
extern GNODE dummylist;
extern int edgeanz;
extern GEDGE edgelist;
extern GEDGE edgelistend;
extern GEDGE tmpedgelist;
extern GEDGE hedgelist;
extern GEDGE hedgelistend;
extern GEDGE invis_edges;
extern ADJEDGE near_edge_list;
extern ADJEDGE bent_near_edge_list;
extern ADJEDGE back_edge_list;

#ifdef CHECKNODE
extern GNODE debug_checknode;
#endif


/* Prototypes
 * ----------
 * See alloc.c for more information.
 */

char *myalloc 		_PP((int x));
void free_memory 	_PP((void));

GNODE nodealloc		_PP((GNODE refnode));
GNODE graphalloc	_PP((GNODE refnode));
void  nodedefaults	_PP((GNODE node));
void  foldnodedefaults	_PP((GNODE node));
void  inherit_foldnode_attributes	_PP((GNODE fn, GNODE y));
void  copy_nodeattributes		_PP((GNODE fn, GNODE y));

GNODE tmpnodealloc	_PP((int textm,int width,int height,int borderw,int fold,int color,int textc,int borderc, int shrink,int stretch,int horder));
void 	free_node	_PP((GNODE v));
void 	free_tmpnodes   _PP((void));
GNODE   search_xy_node	_PP((long x,long y));
void check_graph_consistency _PP((void));


GNLIST  nodelist_alloc		_PP((GNODE v));
GNLIST  tmpnodelist_alloc 	_PP((void));
GNLIST  foldnodelist_alloc 	_PP((void));
void free_regionnodelist	_PP(( GNLIST r));
void free_foldnodelists		_PP((void));

GEDGE edgealloc 	_PP((GEDGE refedge));
void  edgedefaults	_PP((GEDGE edge));
void  foldedgedefaults	_PP((GEDGE edge));
void  inherit_foldedge_attributes	_PP((GEDGE fn, GEDGE y));
void  copy_edgeattributes		_PP((GEDGE fn, GEDGE y));

GEDGE tmpedgealloc _PP((int lstyle,int thick,int xclass,int prio,int ecolor,int elcol,int arrows,int barrows,int arrowsty,int barrowsty,int arrowc,int barrowc,int horder));
void near_edge_insert       _PP((GEDGE e));
void bentnear_edge_insert   _PP((GEDGE e));
void back_edge_insert       _PP((GEDGE e));
ADJEDGE prededgealloc 	    _PP((GNODE node,GEDGE edge));
ADJEDGE succedgealloc 	    _PP((GNODE node,GEDGE edge));

CONNECT	connectalloc	_PP((GNODE node));

DLLIST 	dllist_alloc	_PP((GNODE  node, DLLIST pred));
void    dllist_free	_PP((DLLIST x));
void    dllist_free_all	_PP((DLLIST x));

void	free_all_lists 	 _PP((void));
void    reinit_all_lists _PP((void));

/*--------------------------------------------------------------------*/

#endif /* ALLOC_H  */

