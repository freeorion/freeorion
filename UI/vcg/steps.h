/* SCCS-info %W% %E% */
 
/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         steps.h                                            */
/*   version:      1.00.00                                            */
/*   creation:     10.4.1993                                          */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Readin and Layout steps                            */
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
 * Revision 3.7  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.6  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 * infobox behaviour improved.
 * First version of fisheye (carthesian).
 * Options Noedge and nonode.
 * Titles in the node title box are now sorted.
 * Timelimit functionality improved.
 *
 * Revision 2.1  1993/12/08  21:21:34  sander
 * Reasonable fast and stable version
 *
 */

#ifndef STEPS_H 
#define STEPS_H 

/*--------------------------------------------------------------------*/

/* See step[0-4].c for explanation
 * -------------------------------
 */


/* Global Variables
 * ----------------
 */

/* from step0.c */
extern int act_hash_size;
extern struct gnode foldnode;
extern struct gedge foldedge;

/* from step1.c */
extern  int 	number_reversions;
extern 	int     maxindeg;
extern 	int     maxoutdeg; 
extern	DEPTH 	*layer; 	/* This is an array ! */
extern	int    	maxdepth;  

/* from step2.c */
extern int     max_nodes_per_layer;
extern int     nr_crossings;

/* from step3.c */
extern GNLIST *tpred_connection1;
extern GNLIST *tpred_connection2;

/* from step4.c */
extern int maximal_xpos;
extern int maximal_ypos;
extern int st_nr_vis_nodes;
extern int st_nr_vis_edges;
extern int st_nr_vis_nearedges;
extern int st_nr_vis_dummies;
extern int st_nr_vis_labels;
extern int st_nr_invis_graphs;
extern int st_nr_invis_nodes;
extern int st_nr_invis_edges;
extern int st_max_indeg;
extern int st_max_outdeg;
extern int st_max_degree;

/* from tree.c */
extern int spread_level;
extern double tree_factor;


/* Prototypes
 * ----------
 */

/* from step0.c */
void 	step0_main 		_PP((void));
GNODE   search_visible_node	_PP((char *title));
GNODE 	lookup_hashnode   	_PP((char *title));
void 	init_hash_cursor	_PP((void));
void 	position_hash_cursor	_PP((int j));
GNODE 	get_hash_cursor_succ	_PP((int i));


/* from step1.c */
void    step1_main              _PP((void));
ADJEDGE revert_edge             _PP((GEDGE edge));
void	 calc_number_reversions _PP((void));
void     prepare_back_edges     _PP((void));
void    insert_anchor_edges     _PP((void));
#ifdef DEBUG 
void    db_output_graph         _PP((void));
void    db_output_adjacencies   _PP((void));
void 	db_output_adjacency	_PP((GNODE node, int f));
#endif 

/* from step2.c */
void    step2_main              _PP((void));
#ifdef DEBUG
void    db_output_all_layers  	_PP((void));
void    db_output_layer		_PP((int i));
int 	db_check_proper     	_PP((GNODE v,int level));
#endif

/* from step3.c */
void    step3_main              _PP((void));
void    calc_all_node_sizes     _PP((void));
void 	alloc_levelshift	_PP((void));


/* from step4.c */
void    step4_main              _PP((void));
void    calc_all_ports          _PP((int xya));
void    calc_node_ports         _PP((GNODE v, int xya));
void    calc_edge_xy            _PP((GNODE v));
void    calc_edgearrow          _PP((GNODE v));
void 	calc_max_xy_pos		_PP((void));
void	statistics		_PP((void));

/* from tree.c */
int	tree_main		_PP((void));

/* from prepare.c */
void 	prepare_nodes		_PP((void));
void    calc_node_size          _PP((GNODE v));

/*--------------------------------------------------------------------*/

#endif /* STEPS_H  */

