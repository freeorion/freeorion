/* SCCS-info %W% %E% */
/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         This file is uglified by UGLIFIER V 1.0            */
/*   version:      1.00.00                                            */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
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


/*--------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "alloc.h"
#include "main.h"
#include "options.h"
#include "timelim.h"
#include "folding.h"
#include "steps.h"
#include "timing.h"
#ifdef DEBUG
static void gs_ide18 _PP ((GNODE node));
static void gs_ide16 _PP ((GEDGE edge));
static void gs_ide17 _PP ((void));
static void gs_ide19 _PP ((char *fn));
#endif
static void gs_ide30 _PP ((GNODE node));
static void gs_ide28 _PP ((GNODE node));
static void gs_ide35 _PP ((void));
static void gs_ide34 _PP ((GEDGE edge));
static void gs_ide29 _PP ((void));
static void gs_ide27 _PP ((void));
static int gs_ide8 _PP ((GNODE v, GNODE w, GNODE z));
static void gs_ide33 _PP ((void));
static void gs_ide20 _PP ((GNODE node));
static void gs_ide5 _PP ((GNODE node));
static void gs_ide43 _PP ((ADJEDGE edge, GNODE node, int prio));
static void gs_ide46 _PP ((void));
static void gs_ide4 _PP ((GNODE v));
static GNODE gs_ide24 _PP ((void));
static int gs_ide49 _PP ((GNODE node1, GNODE node2));
static void gs_ide50 _PP ((GNODE node1, GNODE node2, int l));
static void gs_ide47 _PP ((GNODE node1, GNODE node2));
static int gs_ide48 _PP ((GNODE node1, GNODE node2));
static void gs_ide3 _PP ((GNODE v, GNLIST * l));
static GNODE gs_ide23 _PP ((GNLIST * l));
static void gs_ide37 _PP ((void));
static void gs_ide7 _PP ((GNLIST * nlist));
static int gs_ide12 _PP ((GNLIST nlist));
static int gs_ide39 _PP ((GNODE v, GNODE w, int prio));
static int gs_ide40 _PP ((GNODE v, GNODE w));
static int gs_ide38 _PP ((GNODE v, GNODE w));
static void gs_ide36 _PP ((GNODE v, GNODE w));
static void gs_ide41 _PP ((GNODE n, long *d, GNLIST * p));
static void gs_ide2 _PP ((void));
static void gs_ide52 _PP ((void));
static int gs_ide51 _PP ((GNODE v, int lab));
static void gs_ide13 _PP ((void));
static void gs_ide11 _PP ((void));
static void gs_ide6 _PP ((GNODE v, GNODE w, GNODE predw));
static void gs_ide32 _PP ((void));
static void gs_ide10 _PP ((GNODE n, ADJEDGE e, int l));
static void gs_ide31 _PP ((void));
static void gs_ide9 _PP ((ADJEDGE e));
static GNODE gs_ide14 _PP ((int t));
static ADJEDGE gs_ide15 _PP ((GNODE x, GNODE y, GEDGE e, int t));
int maxindeg;
int maxoutdeg;
DEPTH *layer = NULL;
int maxdepth = 0;
static int gs_ide42 = 0;

#define forward_connection1(c)  (((( c )->edge)  )&& ((( (( c )->edge)   )->end)  ==(( c )->target)  ))
#define forward_connection2(c)  (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  ==(( c )->target2)  ))
#define backward_connection1(c) (((( c )->edge)  )&& ((( (( c )->edge)   )->end)  !=(( c )->target)  ))
#define backward_connection2(c) (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  !=(( c )->target2)  ))

#ifdef ANSI_C
void
step1_main (void)
#else
void
step1_main ()
#endif
{
  int i;
  start_time ();;
  assert ((dummylist == NULL));
  prepare_back_edges ();
  gs_ide35 ();
  insert_anchor_edges ();
  gs_ide29 ();
  gs_ide27 ();
  if (layout_flag == 3)
    gs_ide46 ();
  else if (layout_flag == 0)
    gs_ide37 ();
  else
    gs_ide33 ();
  if (edge_label_phase == 1)
    gs_ide2 ();
  if (fine_tune_layout == 1)
    gs_ide52 ();
  if (maxdepth + 2 > gs_ide42)
    {
      if (layer)
	free (layer);
      layer = (DEPTH *) malloc ((maxdepth + 2) * sizeof (struct depth_entry));
      if (!layer)
	Fatal_error ("memory exhausted", "");
      gs_ide42 = maxdepth + 2;
#ifdef DEBUG
      PRINTF ("Maxdepth of this layout: %d \n", maxdepth);
      PRINTF ("Sizeof table `layer': %ld Bytes\n",
	      (maxdepth + 2) * sizeof (struct depth_entry));
#endif
    }
  for (i = 0; i < maxdepth + 2; i++)
    {
      ((layer[i]).anz) = 0;
      ((layer[i]).predlist) = NULL;
      ((layer[i]).succlist) = NULL;
      ((layer[i]).resort_necessary) = 1;
    }
  gs_ide13 ();
  gs_ide32 ();
  gs_ide31 ();
  if (layout_flag == TREE_LAYOUT)
    {
      stop_time ("step1_main");
      layout_flag = tree_main ();
      if (layout_flag != TREE_LAYOUT)
	{
	  FPRINTF (stderr, "\nThis is not a downward tree. ");
	  FPRINTF (stderr, "Continuing with normal layout\n");
	}
    }
  calc_number_reversions ();
  if (layout_flag != TREE_LAYOUT)
    {
      gs_ide11 ();
      stop_time ("step1_main");
    }
}

static GNODE gs_ide44;
static GNODE gs_ide45;

#ifdef ANSI_C
static void
gs_ide30 (GNODE node)
#else
static void
gs_ide30 (node)
     GNODE node;
#endif
{
  GNODE h, *hp;
  assert ((node));;
  if (((node)->next))
    ((((node)->next))->before) = ((node)->before);
  else
    nodelistend = ((node)->before);
  if (((node)->before))
    ((((node)->before))->next) = ((node)->next);
  else
    nodelist = ((node)->next);
  h = gs_ide44;
  hp = &gs_ide44;
  while (h)
    {
      if (((h)->refnum) >= ((node)->refnum))
	break;
      hp = &(((h)->next));
      h = ((h)->next);
    }
  *hp = node;
  if (h)
    ((node)->before) = ((h)->before);
  else
    ((node)->before) = gs_ide45;
  ((node)->next) = h;
  if (h)
    ((h)->before) = node;
  else
    gs_ide45 = node;
}

static GNODE gs_ide21;
static GNODE gs_ide22;

#ifdef ANSI_C
static void
gs_ide28 (GNODE node)
#else
static void
gs_ide28 (node)
     GNODE node;
#endif
{
  GNODE h, *hp;
  assert ((node));;
  if (((node)->next))
    ((((node)->next))->before) = ((node)->before);
  else
    nodelistend = ((node)->before);
  if (((node)->before))
    ((((node)->before))->next) = ((node)->next);
  else
    nodelist = ((node)->next);
  h = gs_ide21;
  hp = &gs_ide21;
  while (h)
    {
      if (((h)->refnum) >= ((node)->refnum))
	break;
      hp = &(((h)->next));
      h = ((h)->next);
    }
  *hp = node;
  if (h)
    ((node)->before) = ((h)->before);
  else
    ((node)->before) = gs_ide22;
  ((node)->next) = h;
  if (h)
    ((h)->before) = node;
  else
    gs_ide22 = node;
}

#ifdef ANSI_C
static void
gs_ide35 (void)
#else
static void
gs_ide35 ()
#endif
{
  GNODE node, nxt_node;;
  gs_ide44 = NULL;
  gs_ide45 = NULL;
  gs_ide21 = NULL;
  gs_ide22 = NULL;
  node = nodelist;
  while (node)
    {
      nxt_node = ((node)->next);
      if (!((node)->pred))
	gs_ide30 (node);
      else if (!((node)->succ))
	gs_ide28 (node);
      node = nxt_node;
    }
  if (gs_ide44)
    {
      if (nodelist)
	((nodelist)->before) = gs_ide45;
      ((gs_ide45)->next) = nodelist;
      nodelist = gs_ide44;
      if (!nodelistend)
	nodelistend = gs_ide45;
    }
  if (gs_ide21)
    {
      if (nodelistend)
	((nodelistend)->next) = gs_ide21;
      ((gs_ide21)->before) = nodelistend;
      nodelistend = gs_ide22;
      if (!nodelist)
	nodelist = gs_ide21;
    }
}

#ifdef ANSI_C
void
insert_anchor_edges (void)
#else
void
insert_anchor_edges ()
#endif
{
  GEDGE edge;;
  assert ((dummylist == NULL));
  edge = edgelist;
  while (edge)
    {
      if ((((edge)->anchor) <= 64) && (((edge)->anchor) > 0)
	  && (!((edge)->invisible)))
	gs_ide34 (edge);
      edge = ((edge)->next);
    }
  edge = tmpedgelist;
  while (edge)
    {
      if ((((edge)->anchor) <= 64) && (((edge)->anchor) > 0)
	  && (!((edge)->invisible)))
	gs_ide34 (edge);
      edge = ((edge)->internal_next);
    }
}

#ifdef ANSI_C
static void
gs_ide34 (GEDGE edge)
#else
static void
gs_ide34 (edge)
     GEDGE edge;
#endif
{
  GEDGE h;
  GNODE v;
  CONNECT c1, c2;;
  if ((G_orientation == 1) || (G_orientation == 2))
    {
      G_orientation = 0;
      if (!silent)
	{
	  FPRINTF (stderr, "Orientation ignored, because");
	  FPRINTF (stderr, " edge attribute `anchor' used !\n");
	}
    }
  c1 = ((((edge)->start))->connection);
  if (!c1)
    {
      v = gs_ide14 (-1);
      ((v)->invisible) = 0;
      ((v)->level) = ((((edge)->start))->level);
      ((v)->nhorder) = ((((edge)->start))->nhorder);
      ((v)->anchordummy) = 1;
      h =
	tmpedgealloc (((edge)->linestyle), ((edge)->thickness),
		      ((edge)->eclass), 200, ((edge)->color),
		      ((edge)->labelcolor), 0, ((edge)->arrowsize2), 0,
		      ((edge)->arrowstyle2), ((edge)->arrowcolor1),
		      ((edge)->arrowcolor2), ((edge)->horder));
      ((h)->anchor) = 66;
      ((h)->start) = ((edge)->start);
      ((h)->end) = v;
      ((h)->label) = NULL;
      c1 = connectalloc (((h)->start));
      ((c1)->target) = v;
      ((c1)->edge) = h;
      c2 = connectalloc (v);
      ((c2)->target) = ((h)->start);
      ((c2)->edge) = h;
    }
  v = ((c1)->target);
  assert ((v));
  assert ((((v)->anchordummy)));
  h =
    tmpedgealloc (((edge)->linestyle), ((edge)->thickness), ((edge)->eclass),
		  ((edge)->priority), ((edge)->color), ((edge)->labelcolor),
		  ((edge)->arrowsize1), ((edge)->arrowsize2),
		  ((edge)->arrowstyle1), ((edge)->arrowstyle2),
		  ((edge)->arrowcolor1), ((edge)->arrowcolor2),
		  ((edge)->horder));
  ((h)->anchor) = -((edge)->anchor);
  ((h)->start) = v;
  ((h)->end) = ((edge)->end);
  ((h)->label) = ((edge)->label);
  delete_adjedge (edge);
  ((edge)->invisible) = 0;
  create_adjedge (h);
}

#ifdef ANSI_C
void
prepare_back_edges (void)
#else
void
prepare_back_edges ()
#endif
{
  ADJEDGE e;
  ADJEDGE a1, a2;;
  e = back_edge_list;
  while (e)
    {
      if (!((((e)->kante))->invisible))
	revert_edge (((e)->kante));
      else if (((((e)->kante))->labelnode))
	{
	  a1 = ((((((e)->kante))->labelnode))->succ);
	  a2 = ((((((e)->kante))->labelnode))->pred);
	  if (a1)
	    if (!((((a1)->kante))->invisible))
	      revert_edge (((a1)->kante));
	  if (a2)
	    if (!((((a2)->kante))->invisible))
	      revert_edge (((a2)->kante));
	}
      e = ((e)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide27 (void)
#else
static void
gs_ide27 ()
#endif
{
  GNODE v;
  ADJEDGE edge1;
  ADJEDGE edge;
  CONNECT c1, c2;
  int connection_possible, invisible;;
  edge1 = bent_near_edge_list;
  while (edge1)
    {
      invisible = ((((edge1)->kante))->invisible);
      v = ((((edge1)->kante))->labelnode);
      if (v)
	{
	  edge = ((v)->succ);
	  if (!edge)
	    invisible = 1;
	  else
	    invisible = ((((edge)->kante))->invisible);
	  edge = ((v)->pred);
	  if (!edge)
	    invisible |= 1;
	  else
	    invisible |= ((((edge)->kante))->invisible);
	}
      else if (!invisible)
	{
	  if (G_displayel == 1)
	    v = create_labelnode (((edge1)->kante));
	  else
	    {
	      v = gs_ide14 (-1);
	      ((v)->invisible) = 0;
	    }
	  ((v)->level) = (((((((edge1)->kante))->start)))->level);
	  ((v)->nhorder) = (((((((edge1)->kante))->start)))->nhorder);
	  edge =
	    gs_ide15 (v, (((((edge1)->kante))->end)), ((edge1)->kante), 1);
	  ((((edge)->kante))->label) = NULL;
	  edge =
	    gs_ide15 ((((((edge1)->kante))->start)), v, ((edge1)->kante), 0);
	  ((((edge)->kante))->label) = NULL;
	  ((((edge)->kante))->priority) = 0;
	  delete_adjedge (((edge1)->kante));
	  ((((edge1)->kante))->invisible) = 0;
	}
      if (!invisible)
	{
	  c1 = (((((((edge)->kante))->start)))->connection);
	  c2 = (((((((edge)->kante))->end)))->connection);
	  connection_possible = 1;
	  if ((c1) && (((c1)->target)) && (((c1)->target2)))
	    connection_possible = 0;
	  if ((c2) && (((c2)->target)) && (((c2)->target2)))
	    connection_possible = 0;
	  if (gs_ide8
	      ((((((edge)->kante))->end)), NULL,
	       (((((edge)->kante))->start))))
	    connection_possible = 0;
	  if (connection_possible)
	    {
	      if (!c1)
		{
		  c1 = connectalloc ((((((edge)->kante))->start)));
		  ((c1)->target) = (((((edge)->kante))->end));
		  ((c1)->edge) = ((edge)->kante);
		}
	      else if (!((c1)->target2))
		{
		  ((c1)->target2) = (((((edge)->kante))->end));
		  ((c1)->edge2) = ((edge)->kante);
		}
	      if (!c2)
		{
		  c2 = connectalloc ((((((edge)->kante))->end)));
		  ((c2)->target) = (((((edge)->kante))->start));
		  ((c2)->edge) = ((edge)->kante);
		}
	      else if (!((c2)->target2))
		{
		  ((c2)->target2) = (((((edge)->kante))->start));
		  ((c2)->edge2) = ((edge)->kante);
		}
	      delete_adjedge (((edge)->kante));
	      ((((edge)->kante))->invisible) = 0;
	    }
	  else if (!silent)
	    {
	      FPRINTF (stderr, "Nearedge connection (");
	      if ((((((((edge)->kante))->start)))->title)[0])
		FPRINTF (stderr, "%s",
			 (((((((edge)->kante))->start)))->title));
	      FPRINTF (stderr, " , ");
	      if ((((((((edge)->kante))->end)))->title)[0])
		FPRINTF (stderr, "%s", (((((((edge)->kante))->end)))->title));
	      FPRINTF (stderr, ") ignored ! Sorry !\n");
	    }
	}
      edge1 = ((edge1)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide29 (void)
#else
static void
gs_ide29 ()
#endif
{
  ADJEDGE edge;
  CONNECT c1, c2;
  int connection_possible;;
  edge = near_edge_list;
  while (edge)
    {
      if (!((((edge)->kante))->invisible))
	{
	  c1 = (((((((edge)->kante))->start)))->connection);
	  c2 = (((((((edge)->kante))->end)))->connection);
	  connection_possible = 1;
	  if ((c1) && (((c1)->target)) && (((c1)->target2)))
	    connection_possible = 0;
	  if ((c2) && (((c2)->target)) && (((c2)->target2)))
	    connection_possible = 0;
	  if (gs_ide8
	      ((((((edge)->kante))->end)), NULL,
	       (((((edge)->kante))->start))))
	    connection_possible = 0;
	  if (connection_possible)
	    {
	      if (!c1)
		{
		  c1 = connectalloc ((((((edge)->kante))->start)));
		  ((c1)->target) = (((((edge)->kante))->end));
		  ((c1)->edge) = ((edge)->kante);
		}
	      else if (!((c1)->target2))
		{
		  ((c1)->target2) = (((((edge)->kante))->end));
		  ((c1)->edge2) = ((edge)->kante);
		}
	      if (!c2)
		{
		  c2 = connectalloc ((((((edge)->kante))->end)));
		  ((c2)->target) = (((((edge)->kante))->start));
		  ((c2)->edge) = ((edge)->kante);
		}
	      else if (!((c2)->target2))
		{
		  ((c2)->target2) = (((((edge)->kante))->start));
		  ((c2)->edge2) = ((edge)->kante);
		}
	      if (((((((((edge)->kante))->start)))->level) >= 0)
		  && ((((((((edge)->kante))->end)))->level) >= 0)
		  && ((((((((edge)->kante))->start)))->level) !=
		      (((((((edge)->kante))->end)))->level) >= 0))
		{
		  if (!silent)
		    {
		      FPRINTF (stderr, "Nearedge connection (");
		      if ((((((((edge)->kante))->start)))->title)[0])
			FPRINTF (stderr, "%s",
				 (((((((edge)->kante))->start)))->title));
		      FPRINTF (stderr, " , ");
		      if ((((((((edge)->kante))->end)))->title)[0])
			FPRINTF (stderr, "%s",
				 (((((((edge)->kante))->end)))->title));
		      FPRINTF (stderr,
			       "): level of target ignored ! Sorry !\n");
		    }
		}
	      if ((((((((edge)->kante))->start)))->level) >= 0)
		{
		  (((((((edge)->kante))->end)))->level) =
		    (((((((edge)->kante))->start)))->level);
		}
	      if ((((((((edge)->kante))->end)))->level) >= 0)
		{
		  (((((((edge)->kante))->start)))->level) =
		    (((((((edge)->kante))->end)))->level);
		}
	      delete_adjedge (((edge)->kante));
	      ((((edge)->kante))->invisible) = 0;
	    }
	  else if (!silent)
	    {
	      FPRINTF (stderr, "Nearedge connection (");
	      if ((((((((edge)->kante))->start)))->title)[0])
		FPRINTF (stderr, "%s",
			 (((((((edge)->kante))->start)))->title));
	      FPRINTF (stderr, " , ");
	      if ((((((((edge)->kante))->end)))->title)[0])
		FPRINTF (stderr, "%s", (((((((edge)->kante))->end)))->title));
	      FPRINTF (stderr, ") ignored ! Sorry !\n");
	    }
	}
      edge = ((edge)->next);
    }
}

#ifdef ANSI_C
static int
gs_ide8 (GNODE v, GNODE w, GNODE z)
#else
static int
gs_ide8 (v, w, z)
     GNODE v;
     GNODE w;
     GNODE z;
#endif
{
  CONNECT c;
  int r;;
  if (!near_edge_layout)
    return (1);
  if (!v)
    return (0);
  if (v == z)
    return (1);
  r = 0;
  c = ((v)->connection);
  if (!c)
    return (0);
  if (((c)->target) && (((c)->target) != w))
    r |= gs_ide8 (((c)->target), v, z);
  if (((c)->target2) && (((c)->target2) != w))
    r |= gs_ide8 (((c)->target2), v, z);
  return (r);
}

static long gs_ide0;
static int gs_ide1;

#ifdef ANSI_C
static void
gs_ide33 (void)
#else
static void
gs_ide33 ()
#endif
{
  GNODE node;
  int depth1;;
  gs_ide0 = 1L;
  maxdepth = 0;
  gs_wait_message ('p');
  node = nodelist;
  while (node)
    {
      if (!((node)->markiert))
	{
	  gs_ide1 = 0;
	  gs_ide20 (node);
	}
      node = ((node)->next);
    }
#ifdef CHECK_ASSERTIONS
  node = labellist;
  while (node)
    {
      if (!((node)->markiert))
	assert ((0));
      node = ((node)->next);
    }
#endif
  if (layout_flag == 12)
    return;
  if (layout_flag == TREE_LAYOUT)
    return;
  if (G_timelimit > 0)
    if (test_timelimit (30))
      {
	gs_wait_message ('t');
	return;
      }
  depth1 = maxdepth;
  gs_wait_message ('p');
  node = nodelist;
  while (node)
    {
      ((node)->markiert) = 0;
      node = ((node)->next);
    }
  node = labellist;
  while (node)
    {
      ((node)->markiert) = 0;
      node = ((node)->next);
    }
  node = dummylist;
  while (node)
    {
      ((node)->markiert) = 0;
      node = ((node)->next);
    }
  gs_ide0 = 1L;
  maxdepth = 0;
  node = nodelist;
  while (node)
    {
      if (!((node)->markiert))
	{
	  gs_ide1 = 0;
	  gs_ide5 (node);
	}
      node = ((node)->next);
    }
#ifdef CHECK_ASSERTIONS
  node = labellist;
  while (node)
    {
      if (!((node)->markiert))
	assert ((0));
      node = ((node)->next);
    }
#endif
  if ((layout_flag == 1) && (depth1 <= maxdepth))
    return;
  if ((layout_flag == 2) && (depth1 >= maxdepth))
    return;
  gs_wait_message ('p');
  node = nodelist;
  while (node)
    {
      ((node)->markiert) = 0;
      node = ((node)->next);
    }
  node = labellist;
  while (node)
    {
      ((node)->markiert) = 0;
      node = ((node)->next);
    }
  node = dummylist;
  while (node)
    {
      ((node)->markiert) = 0;
      node = ((node)->next);
    }
  gs_ide0 = 1L;
  maxdepth = 0;
  node = nodelist;
  while (node)
    {
      if (!((node)->markiert))
	{
	  gs_ide1 = 0;
	  gs_ide20 (node);
	}
      node = ((node)->next);
    }
#ifdef CHECK_ASSERTIONS
  node = labellist;
  while (node)
    {
      if (!((node)->markiert))
	assert ((0));
      node = ((node)->next);
    }
#endif
}

#ifdef ANSI_C
static void
gs_ide20 (GNODE node)
#else
static void
gs_ide20 (node)
     GNODE node;
#endif
{
  GNODE kn;
  ADJEDGE edge;
  int priority;
  assert ((node));;
  if (((node)->markiert))
    return;
  ((node)->markiert) = 1;
  ((node)->dfsnum) = gs_ide0++;
  if (((node)->level) >= 0)
    gs_ide1 = ((node)->level);
  ((node)->tiefe) = gs_ide1;
  maxdepth = (gs_ide1 > maxdepth) ? gs_ide1 : maxdepth;
  if (((node)->connection))
    {
      if (((((node)->connection))->target))
	gs_ide20 (((((node)->connection))->target));
      if (((((node)->connection))->target2))
	gs_ide20 (((((node)->connection))->target2));
    }
  priority = 1;
  while (priority > -1)
    {
      priority = -1;
      edge = ((node)->succ);
      while (edge)
	{
	  assert (((((((edge)->kante))->start)) == node));
	  kn = (((((edge)->kante))->end));
	  if ((!((kn)->markiert))
	      && (((((edge)->kante))->priority) > priority))
	    priority = ((((edge)->kante))->priority);
	  edge = ((edge)->next);
	}
      if (priority == -1)
	break;
      edge = ((node)->succ);
      while (edge)
	{
	  if (((((edge)->kante))->priority) != priority)
	    {
	      edge = ((edge)->next);
	      continue;
	    }
	  assert (((((((edge)->kante))->start)) == node));
	  kn = (((((edge)->kante))->end));
	  if (!((kn)->markiert))
	    {
	      gs_ide1++;
	      gs_ide20 (kn);
	      gs_ide1 = ((node)->tiefe);
	    }
	  else
	    {
	      if (kn == node)
		(((((edge)->kante))->kantenart)) = 'S';
	    }
	  edge = ((edge)->next);
	}
    }
}

#ifdef ANSI_C
static void
gs_ide5 (GNODE node)
#else
static void
gs_ide5 (node)
     GNODE node;
#endif
{
  GNODE kn;
  ADJEDGE edge;
  int priority;
  assert ((node));;
  if (((node)->markiert))
    return;
  ((node)->markiert) = 1;
  ((node)->dfsnum) = gs_ide0++;
  if (((node)->level) >= 0)
    gs_ide1 = ((node)->level);
  ((node)->tiefe) = gs_ide1;
  maxdepth = (gs_ide1 > maxdepth) ? gs_ide1 : maxdepth;
  if (((node)->connection))
    {
      if (((((node)->connection))->target))
	gs_ide5 (((((node)->connection))->target));
      if (((((node)->connection))->target2))
	gs_ide5 (((((node)->connection))->target2));
    }
  priority = 1;
  while (priority > -1)
    {
      priority = -1;
      edge = ((node)->succ);
      while (edge)
	{
	  assert (((((((edge)->kante))->start)) == node));
	  kn = (((((edge)->kante))->end));
	  if ((!((kn)->markiert))
	      && (((((edge)->kante))->priority) > priority))
	    priority = ((((edge)->kante))->priority);
	  edge = ((edge)->next);
	}
      if (priority == -1)
	break;
      gs_ide43 (((node)->succ), node, priority);
    }
}

#ifdef ANSI_C
static void
gs_ide43 (ADJEDGE edge, GNODE node, int priority)
#else
static void
gs_ide43 (edge, node, priority)
     ADJEDGE edge;
     GNODE node;
     int priority;
#endif
{
  GNODE kn;;
  if (!edge)
    return;
  gs_ide43 (((edge)->next), node, priority);
  if (((((edge)->kante))->priority) != priority)
    return;
  kn = (((((edge)->kante))->end));
  if (!((kn)->markiert))
    {
      gs_ide1++;
      gs_ide5 (kn);
      gs_ide1 = ((node)->tiefe);
    }
  else
    {
      if (kn == node)
	(((((edge)->kante))->kantenart)) = 'S';
    }
}

static GNLIST gs_ide54;
static GNLIST gs_ide53;

#ifdef ANSI_C
static void
gs_ide4 (GNODE v)
#else
static void
gs_ide4 (v)
     GNODE v;
#endif
{
  GNLIST h;;
  if (gs_ide53)
    {
      h = gs_ide53;
      gs_ide53 = ((gs_ide53)->next);
    }
  else
    h = tmpnodelist_alloc ();
  ((h)->node) = v;
  ((h)->next) = gs_ide54;
  gs_ide54 = h;
}

#ifdef ANSI_C
static GNODE
gs_ide24 (void)
#else
static GNODE
gs_ide24 ()
#endif
{
  GNLIST h;;
  h = gs_ide54;
  if (h)
    {
      gs_ide54 = ((h)->next);
      ((h)->next) = gs_ide53;
      gs_ide53 = h;
      return (((h)->node));
    }
  return (NULL);
}

#ifdef ANSI_C
static void
gs_ide46 (void)
#else
static void
gs_ide46 ()
#endif
{
  GNODE v;
  int not_ready, actlevel;;
  gs_ide54 = NULL;
  gs_ide53 = NULL;
  v = nodelist;
  while (v)
    {
      if (gs_ide48 (v, v) == 0)
	gs_ide4 (v);
      v = ((v)->next);
    }
  maxdepth = 0;
  gs_wait_message ('p');
  not_ready = 1;
  while (not_ready)
    {
      while ((v = gs_ide24 ()) != NULL)
	{
	  actlevel = gs_ide49 (v, v);
	  if (((v)->level) >= 0)
	    actlevel = ((v)->level);
	  if (maxdepth < actlevel)
	    maxdepth = actlevel;
	  gs_ide50 (v, v, actlevel);
	  gs_ide47 (v, v);
	}
      gs_wait_message ('p');
      not_ready = 0;
      v = nodelist;
      while (v && !not_ready)
	{
	  if (!((v)->markiert))
	    {
	      gs_ide4 (v);
	      not_ready = 1;
	    }
	  v = ((v)->next);
	}
      v = labellist;
      while (v && !not_ready)
	{
	  if (!((v)->markiert))
	    {
	      gs_ide4 (v);
	      not_ready = 1;
	    }
	  v = ((v)->next);
	}
      v = dummylist;
      while (v && !not_ready)
	{
	  if (!((v)->markiert))
	    {
	      gs_ide4 (v);
	      not_ready = 1;
	    }
	  v = ((v)->next);
	}
    }
}

#ifdef ANSI_C
static int
gs_ide49 (GNODE node1, GNODE node2)
#else
static int
gs_ide49 (node1, node2)
     GNODE node1;
     GNODE node2;
#endif
{
  int result, h;
  ADJEDGE a;
  CONNECT c;;
  result = 0;
  a = ((node1)->pred);
  while (a)
    {
      if ((((((a)->kante))->start)) == node1)
	(((((a)->kante))->kantenart)) = 'S';
      else if ((((((((a)->kante))->start)))->markiert))
	{
	  if ((((((((a)->kante))->start)))->tiefe) >= result)
	    result = (((((((a)->kante))->start)))->tiefe) + 1;
	}
      a = ((a)->next);
    }
  c = ((node1)->connection);
  if (c && ((c)->target) && (((c)->target) != node2))
    {
      h = gs_ide49 (((c)->target), node1);
      if (h > result)
	result = h;
    }
  if (c && ((c)->target2) && (((c)->target2) != node2))
    {
      h = gs_ide49 (((c)->target2), node1);
      if (h > result)
	result = h;
    }
  return (result);
}

#ifdef ANSI_C
static void
gs_ide50 (GNODE node1, GNODE node2, int level)
#else
static void
gs_ide50 (node1, node2, level)
     GNODE node1;
     GNODE node2;
     int level;
#endif
{
  CONNECT c;;
  if ((((node1)->level) >= 0) && (level != ((node1)->level)))
    {
      if (!silent)
	{
	  FPRINTF (stderr, "Level specification ignored, ");
	  FPRINTF (stderr, "because nearedge was specified !\n");
	}
    }
  ((node1)->tiefe) = level;
  ((node1)->markiert) = 1;
  c = ((node1)->connection);
  if (c && ((c)->target) && (((c)->target) != node2))
    gs_ide50 (((c)->target), node1, level);
  if (c && ((c)->target2) && (((c)->target2) != node2))
    gs_ide50 (((c)->target2), node1, level);
}

#ifdef ANSI_C
static void
gs_ide47 (GNODE node1, GNODE node2)
#else
static void
gs_ide47 (node1, node2)
     GNODE node1;
     GNODE node2;
#endif
{
  ADJEDGE a;
  CONNECT c;;
  a = ((node1)->succ);
  while (a)
    {
      if ((gs_ide48 ((((((a)->kante))->end)), (((((a)->kante))->end))) == 0)
	  && (!(((((((a)->kante))->end)))->markiert)))
	gs_ide4 ((((((a)->kante))->end)));
      a = ((a)->next);
    }
  c = ((node1)->connection);
  if (c && ((c)->target) && (((c)->target) != node2))
    gs_ide47 (((c)->target), node1);
  if (c && ((c)->target2) && (((c)->target2) != node2))
    gs_ide47 (((c)->target2), node1);
}

#ifdef ANSI_C
static int
gs_ide48 (GNODE node1, GNODE node2)
#else
static int
gs_ide48 (node1, node2)
     GNODE node1;
     GNODE node2;
#endif
{
  int result;
  ADJEDGE a;
  CONNECT c;;
  result = 0;
  a = ((node1)->pred);
  while (a)
    {
      if (!(((((((a)->kante))->start)))->markiert))
	result++;
      a = ((a)->next);
    }
  c = ((node1)->connection);
  if (c && ((c)->target) && (((c)->target) != node2))
    result += gs_ide48 (((c)->target), node1);
  if (c && ((c)->target2) && (((c)->target2) != node2))
    result += gs_ide48 (((c)->target2), node1);
  return (result);
}

static GNLIST gs_ide25;

#ifdef ANSI_C
static void
gs_ide3 (GNODE v, GNLIST * l)
#else
static void
gs_ide3 (v, l)
     GNODE v;
     GNLIST *l;
#endif
{
  GNLIST h;;
  if (gs_ide53)
    {
      h = gs_ide53;
      gs_ide53 = ((gs_ide53)->next);
    }
  else
    h = tmpnodelist_alloc ();
  ((h)->node) = v;
  ((h)->next) = *l;
  *l = h;
}

#ifdef ANSI_C
static GNODE
gs_ide23 (GNLIST * l)
#else
static GNODE
gs_ide23 (l)
     GNLIST *l;
#endif
{
  GNLIST h;;
  h = *l;
  if (h)
    {
      *l = ((h)->next);
      ((h)->next) = gs_ide53;
      gs_ide53 = h;
      return (((h)->node));
    }
  return (NULL);
}

#ifdef ANSI_C
static void
gs_ide37 (void)
#else
static void
gs_ide37 ()
#endif
{
  GNODE v;;
  gs_ide53 = NULL;
  gs_ide25 = NULL;
  maxdepth = 1;
  v = nodelist;
  while (v)
    {
      if (!((v)->invisible))
	gs_ide3 (v, &gs_ide25);
      ((v)->tiefe) = 1;
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      gs_ide3 (v, &gs_ide25);
      ((v)->tiefe) = 1;
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      gs_ide3 (v, &gs_ide25);
      ((v)->tiefe) = 1;
      v = ((v)->next);
    }
  gs_ide7 (&gs_ide25);
}

#ifdef ANSI_C
static void
gs_ide7 (GNLIST * nlist)
#else
static void
gs_ide7 (nlist)
     GNLIST *nlist;
#endif
{
  GNODE v;
  GNLIST h;
  GNLIST open_scc_list;
  long mydfsnum;;
#ifdef SCCDEBUG
  PRINTF ("Calc SCC:\n");
#endif
  h = *nlist;
  while (h)
    {
      v = ((h)->node);
      ((v)->markiert) = ((v)->tiefe) = 0;
      ((v)->weights) = ((v)->weightp) = 0L;
#ifdef SCCDEBUG
      PRINTF ("[%ld|%s] ", v, (((v)->title) ? ((v)->title) : "null"));
#endif
      h = ((h)->next);
    }
#ifdef SCCDEBUG
  PRINTF ("\n");
#endif
  open_scc_list = NULL;
  mydfsnum = 0;
  gs_wait_message ('p');
  v = gs_ide23 (nlist);
  while (v)
    {
      gs_ide41 (v, &mydfsnum, &open_scc_list);
      v = gs_ide23 (nlist);
    }
}

#ifdef ANSI_C
static void
gs_ide41 (GNODE node, long *dfsnum, GNLIST * open_sccp)
#else
static void
gs_ide41 (node, dfsnum, open_sccp)
     GNODE node;
     long *dfsnum;
     GNLIST *open_sccp;
#endif
{
  GNODE kn;
  GNLIST h;
  GNLIST closed_scc_list;
  ADJEDGE edge;
  int mylevel;
  GNODE actrev;
  int degree;
  int minindeg;
  int maxoutdeg;
  int maxpreindeg;
  int minlevel;
  assert ((node));;
  if (((node)->markiert))
    return;
  ((node)->markiert) = 1;
  ((node)->weightp) = 1L;
  ((node)->dfsnum) = *dfsnum;
  ((node)->weights) = *dfsnum;
  (*dfsnum)++;
  gs_ide3 (node, open_sccp);
  if (((node)->connection))
    {
      if (((((node)->connection))->target))
	{
	  kn = ((((node)->connection))->target);
	  gs_ide41 (kn, dfsnum, open_sccp);
	  if ((((kn)->weightp)) && (((kn)->weights) < ((node)->weights)))
	    ((node)->weights) = ((kn)->weights);
	}
      if (((((node)->connection))->target2))
	{
	  kn = ((((node)->connection))->target2);
	  gs_ide41 (kn, dfsnum, open_sccp);
	  if ((((kn)->weightp)) && (((kn)->weights) < ((node)->weights)))
	    ((node)->weights) = ((kn)->weights);
	}
    }
  edge = ((node)->pred);
  while (edge)
    {
      assert (((((((edge)->kante))->end)) == node));
      kn = (((((edge)->kante))->start));
      if (((kn)->tiefe) == 0)
	{
	  gs_ide41 (kn, dfsnum, open_sccp);
	  if ((((kn)->weightp)) && (((kn)->weights) < ((node)->weights)))
	    ((node)->weights) = ((kn)->weights);
	}
      edge = ((edge)->next);
    }
  if (((node)->weights) == ((node)->dfsnum))
    {
      h = closed_scc_list = *open_sccp;
      assert ((h));
      kn = ((h)->node);
      while (kn != node)
	{
	  ((kn)->weightp) = 0L;
	  h = ((h)->next);
	  assert ((h));
	  kn = ((h)->node);
	}
      assert ((kn == node));
      ((node)->weightp) = 0L;
      *open_sccp = ((h)->next);
      ((h)->next) = 0;
#ifdef SCCDEBUG
      PRINTF ("Test SCC:\n");
      h = closed_scc_list;
      while (h)
	{
	  kn = ((h)->node);
	  PRINTF ("[%ld|%s] ", kn, (((kn)->title) ? ((kn)->title) : "null"));
	  h = ((h)->next);
	}
#endif
      minlevel = -1;
      h = closed_scc_list;
      while (h)
	{
	  node = ((h)->node);
	  edge = ((node)->pred);
	  while (edge)
	    {
	      kn = (((((edge)->kante))->start));
	      mylevel = ((kn)->tiefe);
	      if (mylevel > minlevel)
		minlevel = mylevel;
	      edge = ((edge)->next);
	    }
	  h = ((h)->next);
	}
#ifdef SCCDEBUG
      PRINTF (" minlevel: %d\n", minlevel);
#endif
      assert ((closed_scc_list));
      degree = gs_ide12 (closed_scc_list);
      if (degree)
	{
#ifdef SCCDEBUG
	  PRINTF ("Next complete SCC:\n");
#endif
	  minlevel++;
	  kn = gs_ide23 (&closed_scc_list);
	  while (kn)
	    {
	      if (((kn)->level) >= 0)
		((kn)->tiefe) = ((kn)->level);
	      else
		((kn)->tiefe) = minlevel;
	      maxdepth =
		(((kn)->tiefe) > maxdepth) ? ((kn)->tiefe) : maxdepth;
#ifdef SCCDEBUG
	      PRINTF ("[%ld|%s] (%d) (max %d)\n", kn,
		      (((kn)->title) ? ((kn)->title) : "null"), ((kn)->tiefe),
		      maxdepth);
#endif
	      kn = gs_ide23 (&closed_scc_list);
	    }
	  return;
	}
      h = closed_scc_list;
      while (h)
	{
	  ((((h)->node))->tiefe) = MAXINT;
	  h = ((h)->next);
	}
      actrev = ((closed_scc_list)->node);
      minindeg = MAXINT;
      maxoutdeg = 0;
      maxpreindeg = 0;
      h = closed_scc_list;
      while (h)
	{
	  node = ((h)->node);
	  degree = gs_ide39 (node, NULL, 1);
	  if (degree < minindeg)
	    {
	      minindeg = degree;
	      actrev = node;
	      h = ((h)->next);
	      continue;
	    }
	  else if (degree > minindeg)
	    {
	      h = ((h)->next);
	      continue;
	    }
	  degree = gs_ide38 (node, NULL);
	  if (degree > maxoutdeg)
	    {
	      maxoutdeg = degree;
	      actrev = node;
	      h = ((h)->next);
	      continue;
	    }
	  else if (degree < maxoutdeg)
	    {
	      h = ((h)->next);
	      continue;
	    }
	  degree = gs_ide40 (node, NULL);
	  if (degree > maxpreindeg)
	    {
	      maxpreindeg = degree;
	      actrev = node;
	    }
	  h = ((h)->next);
	}
#ifdef SCCDEBUG
      PRINTF ("Revert Preds on [%d|%s] %d %d %d\n", actrev,
	      (((actrev)->title) ? ((actrev)->title) : "null"), minindeg,
	      maxoutdeg, maxpreindeg);
#endif
      gs_ide36 (actrev, NULL);
      gs_ide7 (&closed_scc_list);
    }
}

#ifdef ANSI_C
static int
gs_ide12 (GNLIST nlist)
#else
static int
gs_ide12 (nlist)
     GNLIST nlist;
#endif
{
  GNODE v;
  GNLIST h;
  int res, count;
  CONNECT c;;
  assert ((nlist));
  count = 0;
  res = 1;
  h = nlist;
  while (h)
    {
      count++;
      if (((((h)->node))->level) < 0)
	res = 0;
      h = ((h)->next);
    }
  if (res)
    return (1);
  if (count == 1)
    return (1);
  h = nlist;
  while (h)
    {
      assert ((((((h)->node))->markiert) == 1));
      ((((h)->node))->markiert) = 0;
      h = ((h)->next);
    }
  count = 1;
  ((((nlist)->node))->markiert) = 1;
  while (count)
    {
      count = 0;
      h = nlist;
      while (h)
	{
	  v = ((h)->node);
	  if (((v)->markiert))
	    {
	      c = ((v)->connection);
	      if ((c) && ((c)->edge))
		{
		  if (!((((((c)->edge))->start))->markiert))
		    count++;
		  if (!((((((c)->edge))->end))->markiert))
		    count++;
		  ((((((c)->edge))->start))->markiert) = 1;
		  ((((((c)->edge))->end))->markiert) = 1;
		}
	      if ((c) && ((c)->edge2))
		{
		  if (!((((((c)->edge2))->start))->markiert))
		    count++;
		  if (!((((((c)->edge2))->end))->markiert))
		    count++;
		  ((((((c)->edge2))->start))->markiert) = 1;
		  ((((((c)->edge2))->end))->markiert) = 1;
		}
	    }
	  h = ((h)->next);
	}
    }
  res = 1;
  h = nlist;
  while (h)
    {
      if (!((((h)->node))->markiert))
	{
	  res = 0;
	  break;
	}
      h = ((h)->next);
    }
  h = nlist;
  while (h)
    {
      ((((h)->node))->markiert) = 1;
      h = ((h)->next);
    }
  return (res);
}

#ifdef ANSI_C
static int
gs_ide39 (GNODE v, GNODE w, int prio)
#else
static int
gs_ide39 (v, w, prio)
     GNODE v;
     GNODE w;
     int prio;
#endif
{
  int degree;
  ADJEDGE e;
  CONNECT c;
  degree = 0;
  e = ((v)->succ);
  while (e)
    {
      if ((((((((e)->kante))->end)))->tiefe) == MAXINT)
	{
	  if (prio)
	    degree += ((((e)->kante))->priority);
	  else
	    degree++;
	}
      e = ((e)->next);
    }
  c = ((v)->connection);
  if (!c)
    return (degree);
  if (((c)->target) && (w != ((c)->target)))
    degree += gs_ide39 (((c)->target), v, prio);
  if (((c)->target2) && (w != ((c)->target2)))
    degree += gs_ide39 (((c)->target2), v, prio);
  return (degree);
}

#ifdef ANSI_C
static int
gs_ide40 (GNODE v, GNODE w)
#else
static int
gs_ide40 (v, w)
     GNODE v;
     GNODE w;
#endif
{
  int degree;
  ADJEDGE e;
  CONNECT c;
  degree = 0;
  e = ((v)->succ);
  while (e)
    {
      if ((((((((e)->kante))->end)))->tiefe) == MAXINT)
	{
	  degree += gs_ide39 ((((((e)->kante))->start)), NULL, 0);
	}
      e = ((e)->next);
    }
  c = ((v)->connection);
  if (!c)
    return (degree);
  if (((c)->target) && (w != ((c)->target)))
    degree += gs_ide40 (((c)->target), v);
  if (((c)->target2) && (w != ((c)->target2)))
    degree += gs_ide40 (((c)->target2), v);
  return (degree);
}

#ifdef ANSI_C
static int
gs_ide38 (GNODE v, GNODE w)
#else
static int
gs_ide38 (v, w)
     GNODE v;
     GNODE w;
#endif
{
  int degree;
  ADJEDGE e;
  CONNECT c;
  degree = 0;
  e = ((v)->pred);
  while (e)
    {
      if ((((((((e)->kante))->start)))->tiefe) == MAXINT)
	{
	  degree++;
	}
      e = ((e)->next);
    }
  c = ((v)->connection);
  if (!c)
    return (degree);
  if (((c)->target) && (w != ((c)->target)))
    degree += gs_ide38 (((c)->target), v);
  if (((c)->target2) && (w != ((c)->target2)))
    degree += gs_ide38 (((c)->target2), v);
  return (degree);
}

#ifdef ANSI_C
static void
gs_ide36 (GNODE v, GNODE w)
#else
static void
gs_ide36 (v, w)
     GNODE v;
     GNODE w;
#endif
{
  ADJEDGE e, en;
  CONNECT c;
  e = ((v)->succ);
  while (e)
    {
      en = ((e)->next);
      if ((((((((e)->kante))->end)))->tiefe) == MAXINT)
	{
	  revert_edge (((e)->kante));
	}
      e = en;
    }
  c = ((v)->connection);
  if (!c)
    return;
  if (((c)->target) && (w != ((c)->target)))
    gs_ide36 (((c)->target), v);
  if (((c)->target2) && (w != ((c)->target2)))
    gs_ide36 (((c)->target2), v);
}

#ifdef ANSI_C
static void
gs_ide2 (void)
#else
static void
gs_ide2 ()
#endif
{
  GNODE v, vl, vt;
  ADJEDGE edge, edgenext;;
  v = nodelist;
  while (v)
    {
      ((v)->markiert) = 0;
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      ((v)->markiert) = 0;
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      ((v)->markiert) = 0;
      v = ((v)->next);
    }
  maxdepth = 2 * maxdepth;
  v = nodelist;
  while (v)
    {
      if (!((v)->markiert))
	{
	  ((v)->tiefe) = 2 * ((v)->tiefe);
	  ((v)->markiert) = 1;
	}
      else
	assert ((0));;
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      if (!((v)->markiert))
	{
	  ((v)->tiefe) = 2 * ((v)->tiefe);
	  ((v)->markiert) = 1;
	}
      else
	assert ((0));;
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      if (!((v)->markiert))
	{
	  ((v)->tiefe) = 2 * ((v)->tiefe);
	  ((v)->markiert) = 1;
	}
      else
	assert ((0));;
      v = ((v)->next);
    }
  v = nodelist;
  while (v)
    {
      edge = ((v)->succ);
      while (edge)
	{
	  edgenext = ((edge)->next);
	  if (((((edge)->kante))->label))
	    {
	      vl = create_labelnode (((edge)->kante));
	      vt = (((((edge)->kante))->end));
	      ((vl)->tiefe) =
		((((((((edge)->kante))->start)))->tiefe) +
		 (((((((edge)->kante))->end)))->tiefe)) / 2;
	      (void) gs_ide15 (v, vl, ((edge)->kante), 0);
	      (void) gs_ide15 (vl, vt, ((edge)->kante), 1);
	      delete_adjedge (((edge)->kante));
	    }
	  edge = edgenext;
	} v = ((v)->next);
    } v = tmpnodelist;
  while (v)
    {
      edge = ((v)->succ);
      while (edge)
	{
	  edgenext = ((edge)->next);
	  if (((((edge)->kante))->label))
	    {
	      vl = create_labelnode (((edge)->kante));
	      vt = (((((edge)->kante))->end));
	      ((vl)->tiefe) =
		((((((((edge)->kante))->start)))->tiefe) +
		 (((((((edge)->kante))->end)))->tiefe)) / 2;
	      (void) gs_ide15 (v, vl, ((edge)->kante), 0);
	      (void) gs_ide15 (vl, vt, ((edge)->kante), 1);
	      delete_adjedge (((edge)->kante));
	    }
	  edge = edgenext;
	} v = ((v)->next);
}}

#ifdef ANSI_C
static void
gs_ide52 (void)
#else
static void
gs_ide52 ()
#endif
{
  GNODE v;
  int changed, count;;
  count = 0;
  changed = 1;
  while (changed)
    {
      changed = 0;
      gs_wait_message ('p');
      if (G_timelimit > 0)
	if (test_timelimit (30))
	  {
	    gs_wait_message ('t');
	    return;
	  }
      v = nodelist;
      while (v)
	{
	  if (!((v)->connection))
	    changed += gs_ide51 (v, 0);
	  v = ((v)->next);
	}
      v = labellist;
      while (v)
	{
	  if (!((v)->connection))
	    changed += gs_ide51 (v, 1);
	  v = ((v)->next);
	}
      count++;
      if (count >= 50)
	return;
    }
}

#ifdef ANSI_C
static int
gs_ide51 (GNODE v, int lab)
#else
static int
gs_ide51 (v, lab)
     GNODE v;
     int lab;
#endif
{
  int nodelevel, leveldiff, nr_edges, nr_redges, changed, delta, hdelta;
  ADJEDGE edge, hedge;
  GNODE hh;;
  if (((v)->level) >= 0)
    return (0);
  nr_redges = nr_edges = leveldiff = 0;
  nodelevel = ((v)->tiefe);
  if ((!((v)->succ)) || (!((v)->pred)))
    delta = 1;
  else
    delta = 0;
  edge = ((v)->pred);
  while (edge)
    {
      nr_edges += ((((edge)->kante))->priority);
      nr_redges++;
      if ((((((((edge)->kante))->start)))->tiefe) != nodelevel)
	leveldiff +=
	  (((((edge)->kante))->priority) *
	   ((((((((edge)->kante))->start)))->tiefe) - nodelevel + delta));
      edge = ((edge)->next);
    }
  edge = ((v)->succ);
  while (edge)
    {
      nr_edges += ((((edge)->kante))->priority);
      nr_redges++;
      if ((((((((edge)->kante))->end)))->tiefe) != nodelevel)
	leveldiff +=
	  (((((edge)->kante))->priority) *
	   ((((((((edge)->kante))->end)))->tiefe) - nodelevel - delta));
      edge = ((edge)->next);
    }
  if (nr_redges == 0)
    {
      ((v)->tiefe) = 0;
      return (0);
    }
  if (nr_edges == 0)
    return (0);
  changed = 0;
  if (leveldiff / nr_edges <= -2)
    {
      changed = 1;
      nodelevel += (leveldiff / nr_edges);
    }
  else if (leveldiff / nr_edges > 0)
    {
      changed = 1;
      nodelevel += (leveldiff / nr_edges);
    }
#ifdef TUNEDEBUG
  PRINTF ("Wish %sto move \"%s\" from level %d to level %d\n",
	  (changed ? "" : "not "), (((v)->title) ? ((v)->title) : "(null)"),
	  ((v)->tiefe), nodelevel);
#endif
  if (!changed)
    return (0);
  if ((layout_flag == 3) || (layout_flag == 0))
    {
      if (near_edge_layout)
	delta = 0;
      else
	delta = 1;
      if (nodelevel > ((v)->tiefe))
	{
	  edge = ((v)->succ);
	  while (edge)
	    {
	      if (((((((((edge)->kante))->end)))->tiefe) > ((v)->tiefe))
		  && (nodelevel >= (((((((edge)->kante))->end)))->tiefe)))
		nodelevel = (((((((edge)->kante))->end)))->tiefe) - delta;
	      edge = ((edge)->next);
	    }
	  edge = ((v)->pred);
	  while (edge)
	    {
	      if (((((((((edge)->kante))->start)))->tiefe) > ((v)->tiefe))
		  && (nodelevel >= (((((((edge)->kante))->start)))->tiefe)))
		nodelevel = (((((((edge)->kante))->start)))->tiefe) - delta;
	      edge = ((edge)->next);
	    }
	  if (nodelevel <= ((v)->tiefe))
	    return (0);
	}
      if (nodelevel < ((v)->tiefe))
	{
	  edge = ((v)->succ);
	  while (edge)
	    {
	      if (((((((((edge)->kante))->end)))->tiefe) < ((v)->tiefe))
		  && (nodelevel <= (((((((edge)->kante))->end)))->tiefe)))
		nodelevel = (((((((edge)->kante))->end)))->tiefe) + delta;
	      edge = ((edge)->next);
	    }
	  edge = ((v)->pred);
	  while (edge)
	    {
	      if (((((((((edge)->kante))->start)))->tiefe) < ((v)->tiefe))
		  && (nodelevel <= (((((((edge)->kante))->start)))->tiefe)))
		nodelevel = (((((((edge)->kante))->start)))->tiefe) + delta;
	      edge = ((edge)->next);
	    }
	  if (nodelevel >= ((v)->tiefe))
	    return (0);
	}
      edge = ((v)->pred);
      while (edge)
	{
	  if ((((((edge)->kante))->kantenart)) != 'R')
	    {
	      if ((nodelevel < (((((((edge)->kante))->start)))->tiefe))
		  && (((v)->tiefe) >=
		      (((((((edge)->kante))->start)))->tiefe)))
		return (0);
	    }
	  edge = ((edge)->next);
	}
      edge = ((v)->succ);
      while (edge)
	{
	  if ((((((edge)->kante))->kantenart)) != 'R')
	    {
	      if ((nodelevel > (((((((edge)->kante))->end)))->tiefe))
		  && (((v)->tiefe) <= (((((((edge)->kante))->end)))->tiefe)))
		return (0);
	    }
	  edge = ((edge)->next);
	}
    }
  else
    {
      edge = ((v)->pred);
      while (edge)
	{
	  if (nodelevel == (((((((edge)->kante))->start)))->tiefe))
	    return (0);
	  edge = ((edge)->next);
	}
      edge = ((v)->succ);
      while (edge)
	{
	  if (nodelevel == (((((((edge)->kante))->end)))->tiefe))
	    return (0);
	  edge = ((edge)->next);
	}
    }
  delta = 0;
  edge = ((v)->pred);
  while (edge)
    {
      hh = (((((edge)->kante))->start));
      if (nodelevel == ((hh)->tiefe))
	{
	  delta++;
	  if (((hh)->connection))
	    return (0);
	  hdelta = 0;
	  hedge = ((hh)->pred);
	  while (hedge)
	    {
	      if (((hh)->tiefe) == (((((((hedge)->kante))->start)))->tiefe))
		hdelta++;
	      hedge = ((hedge)->next);
	    }
	  hedge = ((hh)->succ);
	  while (hedge)
	    {
	      if (((hh)->tiefe) == (((((((hedge)->kante))->end)))->tiefe))
		hdelta++;
	      hedge = ((hedge)->next);
	    }
	  if (hdelta > 1)
	    return (0);
	}
      edge = ((edge)->next);
    }
  edge = ((v)->succ);
  while (edge)
    {
      hh = (((((edge)->kante))->end));
      if (nodelevel == ((hh)->tiefe))
	{
	  delta++;
	  if (((hh)->connection))
	    return (0);
	  hdelta = 0;
	  hedge = ((hh)->pred);
	  while (hedge)
	    {
	      if (((hh)->tiefe) == (((((((hedge)->kante))->start)))->tiefe))
		hdelta++;
	      hedge = ((hedge)->next);
	    }
	  hedge = ((hh)->succ);
	  while (hedge)
	    {
	      if (((hh)->tiefe) == (((((((hedge)->kante))->end)))->tiefe))
		hdelta++;
	      hedge = ((hedge)->next);
	    }
	  if (hdelta > 1)
	    return (0);
	}
      edge = ((edge)->next);
    }
  if (delta > 2)
    return (0);
  if (nodelevel < 0)
    return (0);
  if (nodelevel > maxdepth)
    return (0);
#ifdef TUNEDEBUG
  PRINTF ("Move \"%s\" from level %d to level %d\n",
	  (((v)->title) ? ((v)->title) : "(null)"), ((v)->tiefe), nodelevel);
#endif
  if (((v)->tiefe) != nodelevel)
    changed = 1;
  ((v)->tiefe) = nodelevel;
  return (changed);
}

#ifdef ANSI_C
static void
gs_ide13 (void)
#else
static void
gs_ide13 ()
#endif
{
  GNODE h;
  GNLIST hl;
  int t;;
  h = nodelist;
  while (h)
    {
      t = ((h)->tiefe);
      assert ((t <= maxdepth));
      hl = tmpnodelist_alloc ();
      ((hl)->next) = ((layer[t]).succlist);
      ((layer[t]).succlist) = hl;
      ((hl)->node) = h;
      h = ((h)->next);
    }
  h = labellist;
  while (h)
    {
      t = ((h)->tiefe);
      assert ((t <= maxdepth + 1));
      hl = tmpnodelist_alloc ();
      ((hl)->next) = ((layer[t]).succlist);
      ((layer[t]).succlist) = hl;
      ((hl)->node) = h;
      h = ((h)->next);
    }
  h = dummylist;
  while (h)
    {
      t = ((h)->tiefe);
      assert ((t <= maxdepth + 1));
      hl = tmpnodelist_alloc ();
      ((hl)->next) = ((layer[t]).succlist);
      ((layer[t]).succlist) = hl;
      ((hl)->node) = h;
      h = ((h)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide11 (void)
#else
static void
gs_ide11 ()
#endif
{
  int i;
  GNLIST n, hl;
  GNODE node;
  ADJEDGE edge;
  int backward_connection;
  int forward_connection;
  CONNECT c;;
  maxindeg = maxoutdeg = 0;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      n = ((layer[i]).succlist);
      while (n)
	{
	  ((((n)->node))->markiert) = 0;
	  n = ((n)->next);
	}
      n = ((layer[i]).succlist);
      assert ((((layer[i]).predlist) == NULL));
      while (n)
	{
	  node = ((n)->node);
	  edge = ((node)->succ);
	  backward_connection = 0;
	  forward_connection = 0;
	  c = ((node)->connection);
	  if (c)
	    {
	      if (backward_connection1 (c))
		backward_connection = 1;
	      if (backward_connection2 (c))
		backward_connection = 1;
	      if (forward_connection1 (c))
		forward_connection = 1;
	      if (forward_connection2 (c))
		forward_connection = 1;
	    }
	  if ((forward_connection) && (!backward_connection)
	      && (((((n)->node))->markiert) == 0))
	    gs_ide6 (node, node, NULL);
	  if ((!backward_connection) && (((((n)->node))->markiert) == 0))
	    {
	      hl = tmpnodelist_alloc ();
	      ((hl)->next) = ((layer[i]).predlist);
	      ((layer[i]).predlist) = hl;
	      ((hl)->node) = node;
	    }
	  edge = ((node)->succ);
	  while (edge)
	    {
	      ((node)->outdegree)++;
	      assert (((((((((edge)->kante))->end)))->tiefe) >= i));
	      edge = ((edge)->next);
	    }
	  edge = ((node)->pred);
	  while (edge)
	    {
	      ((node)->indegree)++;
	      edge = ((edge)->next);
	    }
	  if (((node)->outdegree) > maxoutdeg)
	    maxoutdeg = ((node)->outdegree);
	  if (((node)->indegree) > maxindeg)
	    maxindeg = ((node)->indegree);
	  n = ((n)->next);
	}
    }
}

#ifdef ANSI_C
static void
gs_ide6 (GNODE v, GNODE w, GNODE predw)
#else
static void
gs_ide6 (v, w, predw)
     GNODE v;
     GNODE w;
     GNODE predw;
#endif
{
  ADJEDGE edge;
  CONNECT c;;
  if (v != w)
    ((w)->markiert) = 1;
  if (v == w)
    {
      ((w)->savesucc) = ((w)->succ);
      ((v)->savepred) = ((v)->pred);
    }
  edge = ((w)->succ);
  if (v == w)
    ((v)->succ) = NULL;
  while (edge)
    {
      succedgealloc (v, ((edge)->kante));
      (((((edge)->kante))->start)) = v;
      edge = ((edge)->next);
    }
  edge = ((w)->pred);
  if (v == w)
    ((v)->pred) = NULL;
  while (edge)
    {
      prededgealloc (v, ((edge)->kante));
      (((((edge)->kante))->end)) = v;
      edge = ((edge)->next);
    }
  c = ((w)->connection);
  if (!c)
    return;
  if (((c)->target) && (((c)->target) != predw))
    gs_ide6 (v, ((c)->target), w);
  if (((c)->target2) && (((c)->target2) != predw))
    gs_ide6 (v, ((c)->target2), w);
}

#ifdef ANSI_C
static void
gs_ide32 (void)
#else
static void
gs_ide32 ()
#endif
{
  int i;
  GNLIST li;
  GNODE node;
  ADJEDGE edge, nextedge;
  GNODE d1, d2;
  ADJEDGE e1, e2, e3, e4;
  GEDGE a1, a2;;
  for (i = 0; i <= maxdepth; i++)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  node = ((li)->node);
	  edge = ((node)->succ);
	  while (edge)
	    {
	      assert (((((((edge)->kante))->start)) == node));
	      nextedge = ((edge)->next);
	      gs_ide10 (node, edge, i);
	      edge = nextedge;
	    }
	  li = ((li)->next);
	}
    }
  node = labellist;
  while (node)
    {
      edge = ((node)->succ);
      if (edge && (((edge)->next)))
	{
	  a1 = ((edge)->kante);
	  a2 = ((((edge)->next))->kante);
	  d1 = gs_ide14 (((node)->tiefe));
	  d2 = gs_ide14 (((node)->tiefe));
	  e1 = gs_ide15 (node, d1, a1, 0);
	  e2 = gs_ide15 (d1, ((a1)->end), a1, 1);
	  e3 = gs_ide15 (node, d2, a2, 0);
	  e4 = gs_ide15 (d2, ((a2)->end), a2, 1);
	  gs_ide10 ((((((e1)->kante))->start)), e1, ((node)->tiefe));
	  gs_ide10 ((((((e2)->kante))->start)), e2, ((node)->tiefe));
	  gs_ide10 ((((((e3)->kante))->start)), e3, ((node)->tiefe));
	  gs_ide10 ((((((e4)->kante))->start)), e4, ((node)->tiefe));
	  delete_adjedge (a1);
	  delete_adjedge (a2);
	}
      edge = ((node)->pred);
      if (edge && (((edge)->next)))
	{
	  a1 = ((edge)->kante);
	  a2 = ((((edge)->next))->kante);
	  d1 = gs_ide14 (((node)->tiefe));
	  d2 = gs_ide14 (((node)->tiefe));
	  e1 = gs_ide15 (d1, node, a1, 1);
	  e2 = gs_ide15 (((a1)->start), d1, a1, 0);
	  e3 = gs_ide15 (d2, node, a2, 1);
	  e4 = gs_ide15 (((a2)->start), d2, a2, 0);
	  gs_ide10 ((((((e1)->kante))->start)), e1, ((node)->tiefe));
	  gs_ide10 ((((((e2)->kante))->start)), e2, ((node)->tiefe) - 1);
	  gs_ide10 ((((((e3)->kante))->start)), e3, ((node)->tiefe));
	  gs_ide10 ((((((e4)->kante))->start)), e4, ((node)->tiefe) - 1);
	  delete_adjedge (a1);
	  delete_adjedge (a2);
	}
      node = ((node)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide10 (GNODE node, ADJEDGE edge, int level)
#else
static void
gs_ide10 (node, edge, level)
     GNODE node;
     ADJEDGE edge;
     int level;
#endif
{
  int edgelen;
  int i, j;
  GNODE d1, d2;
  ADJEDGE e1, e2, e3;
  CONNECT c1, c2;
  int connection_possible, lab_set;;
  assert ((node));
  assert ((edge));
  assert ((((edge)->kante)));
  assert ((((node)->tiefe) == level));
  assert (((((((edge)->kante))->start)) == node));
  edgelen = (((((((edge)->kante))->end))->tiefe)) - level;
  if (edgelen < 0)
    {
      e1 = revert_edge (((edge)->kante));
      gs_ide10 ((((((e1)->kante))->start)), e1,
		(((((((e1)->kante))->start)))->tiefe));
    }
  else if (edgelen == 0)
    {
      if ((((((edge)->kante))->kantenart)) == 'R')
	{
	  e1 = revert_edge (((edge)->kante));
	  gs_ide10 ((((((e1)->kante))->start)), e1,
		    (((((((e1)->kante))->start)))->tiefe));
	}
      else if ((((((edge)->kante))->start)) == (((((edge)->kante))->end)))
	{
	  assert (((((((edge)->kante))->end)) == node));
	  assert ((level <= maxdepth));
	  d1 = gs_ide14 (level + 1);
	  d2 = gs_ide14 (level + 1);
	  ((d1)->nhorder) = ((d2)->nhorder) = ((((edge)->kante))->horder);
	  e1 = gs_ide15 (node, d1, ((edge)->kante), 0);
	  e2 = gs_ide15 (d1, d2, ((edge)->kante), 2);
	  e3 = gs_ide15 (d2, node, ((edge)->kante), 1);
	  ((((e2)->kante))->label) = ((((edge)->kante))->label);
	  gs_ide10 ((((((e2)->kante))->start)), e2,
		    (((((((e2)->kante))->start)))->tiefe));
	  gs_ide10 ((((((e3)->kante))->start)), e3,
		    (((((((e3)->kante))->start)))->tiefe));
	  delete_adjedge (((edge)->kante));
	}
      else
	{
	  c1 = ((node)->connection);
	  c2 = (((((((edge)->kante))->end)))->connection);
	  connection_possible = 1;
	  if ((c1) && (((c1)->target)) && (((c1)->target2)))
	    connection_possible = 0;
	  if ((c2) && (((c2)->target)) && (((c2)->target2)))
	    connection_possible = 0;
	  if (gs_ide8
	      ((((((edge)->kante))->end)), NULL,
	       (((((edge)->kante))->start))))
	    connection_possible = 0;
	  if (connection_possible)
	    {
	      if (!c1)
		{
		  c1 = connectalloc (node);
		  ((c1)->target) = (((((edge)->kante))->end));
		  ((c1)->edge) = ((edge)->kante);
		}
	      else if (!((c1)->target2))
		{
		  ((c1)->target2) = (((((edge)->kante))->end));
		  ((c1)->edge2) = ((edge)->kante);
		}
	      if (!c2)
		{
		  c2 = connectalloc ((((((edge)->kante))->end)));
		  ((c2)->target) = node;
		  ((c2)->edge) = ((edge)->kante);
		}
	      else if (!((c2)->target2))
		{
		  ((c2)->target2) = node;
		  ((c2)->edge2) = ((edge)->kante);
		}
	      delete_adjedge (((edge)->kante));
	      ((((edge)->kante))->invisible) = 0;
	    }
	  else
	    {
	      if (level <= maxdepth)
		d1 = gs_ide14 (level + 1);
	      else if (level > 0)
		d1 = gs_ide14 (level - 1);
	      else
		{
		  assert ((0));
		}
	      ((d1)->nhorder) = ((((edge)->kante))->horder);
	      e1 = gs_ide15 (node, d1, ((edge)->kante), 0);
	      e2 =
		gs_ide15 (d1, (((((edge)->kante))->end)), ((edge)->kante), 1);
	      ((((e2)->kante))->label) = ((((edge)->kante))->label);
	      gs_ide10 ((((((e1)->kante))->start)), e1,
			(((((((e1)->kante))->start)))->tiefe));
	      gs_ide10 ((((((e2)->kante))->start)), e2,
			(((((((e2)->kante))->start)))->tiefe));
	      delete_adjedge (((edge)->kante));
	    }
	}
    }
  else if (edgelen == 1)
    {
    }
  else if (edgelen > 1)
    {
      d1 = node;
      j = lab_set = 0;
      for (i = 1; i < edgelen; i++)
	{
	  d2 = gs_ide14 (level + i);
	  ((d2)->nhorder) = ((((edge)->kante))->horder);
	  e1 = gs_ide15 (d1, d2, ((edge)->kante), j);
	  if (i == (edgelen + 1) / 2)
	    {
	      ((((e1)->kante))->label) = ((((edge)->kante))->label);
	      lab_set = 1;
	    }
	  d1 = d2;
	  j = 2;
	}
      e1 = gs_ide15 (d1, (((((edge)->kante))->end)), ((edge)->kante), 1);
      if (!lab_set)
	((((e1)->kante))->label) = ((((edge)->kante))->label);
      delete_adjedge (((edge)->kante));
    }
}

#ifdef ANSI_C
ADJEDGE
revert_edge (GEDGE edge)
#else
ADJEDGE
revert_edge (edge)
     GEDGE edge;
#endif
{
  GNODE h;
  char hh;;
  delete_adjedge (edge);
  h = ((edge)->start);
  ((edge)->start) = ((edge)->end);
  ((edge)->end) = h;
  if (((edge)->kantenart) == 'R')
    ((edge)->kantenart) = 'U';
  else if (((edge)->kantenart) == 'S')
    ((edge)->kantenart) = 'S';
  else
    ((edge)->kantenart) = 'R';
  hh = ((edge)->arrowsize1);
  ((edge)->arrowsize1) = ((edge)->arrowsize2);
  ((edge)->arrowsize2) = hh;
  hh = ((edge)->arrowcolor1);
  ((edge)->arrowcolor1) = ((edge)->arrowcolor2);
  ((edge)->arrowcolor2) = hh;
  hh = ((edge)->arrowstyle1);
  ((edge)->arrowstyle1) = ((edge)->arrowstyle2);
  ((edge)->arrowstyle2) = hh;
  create_adjedge (edge);
  return (((((edge)->start))->succ));
}

#ifdef ANSI_C
static ADJEDGE
gs_ide15 (GNODE start, GNODE end, GEDGE edge, int arrow)
#else
static ADJEDGE
gs_ide15 (start, end, edge, arrow)
     GNODE start;
     GNODE end;
     GEDGE edge;
     int arrow;
#endif
{
  GEDGE h;
  h =
    tmpedgealloc (((edge)->linestyle), ((edge)->thickness), ((edge)->eclass),
		  ((edge)->priority), ((edge)->color), ((edge)->labelcolor),
		  ((edge)->arrowsize1), ((edge)->arrowsize2),
		  ((edge)->arrowstyle1), ((edge)->arrowstyle2),
		  ((edge)->arrowcolor1), ((edge)->arrowcolor2),
		  ((edge)->horder));
  ((h)->label) = ((edge)->label);
  ((h)->anchor) = ((edge)->anchor);
  ((h)->start) = start;
  ((h)->end) = end;
  switch (arrow)
    {
    case 0:
      ((h)->arrowstyle1) = 0;
      ((h)->arrowsize1) = 0;
      ((h)->label) = NULL;
      break;
    case 1:
      ((h)->arrowstyle2) = 0;
      ((h)->arrowsize2) = 0;
      ((h)->label) = NULL;
      break;
    case 2:
      ((h)->arrowstyle1) = 0;
      ((h)->arrowsize1) = 0;
      ((h)->label) = NULL;
      ((h)->arrowstyle2) = 0;
      ((h)->arrowsize2) = 0;
      ((h)->label) = NULL;
      break;
    }
  if (((edge)->kantenart) == 'S')
    ((h)->kantenart) = 'U';
  else
    ((h)->kantenart) = ((edge)->kantenart);
  if (start == end)
    ((h)->kantenart) = 'S';
  create_adjedge (h);
  return (((((h)->start))->succ));
}

#ifdef ANSI_C
static GNODE
gs_ide14 (int t)
#else
static GNODE
gs_ide14 (t)
     int t;
#endif
{
  GNLIST hl;
  GNODE v;;
  assert ((t <= maxdepth + 1));
  v = tmpnodealloc (0, -1, -1, 0, -1, G_color, G_color, G_color, 1, 1, -1);
  ((v)->title) = "";
  ((v)->label) = "";
  ((v)->tiefe) = t;
  ((v)->nhorder) = -1;
  ((v)->before) = NULL;
  ((v)->next) = dummylist;
  if (dummylist)
    ((dummylist)->before) = v;
  dummylist = v;
  dummyanz++;
  if (t < 0)
    return (v);
  hl = tmpnodelist_alloc ();
  ((hl)->next) = ((layer[t]).succlist);
  ((layer[t]).succlist) = hl;
  ((hl)->node) = v;
  return (v);
}

#ifdef ANSI_C
static void
gs_ide31 (void)
#else
static void
gs_ide31 ()
#endif
{
  int i;
  GNLIST li;
  GNODE node;
  ADJEDGE edge, nextedge;;
  for (i = 0; i <= maxdepth; i++)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  node = ((li)->node);
	  edge = ((node)->succ);
	  while (edge)
	    {
	      assert (((((((edge)->kante))->start)) == node));
	      nextedge = ((edge)->next);
	      gs_ide9 (edge);
	      edge = nextedge;
	    }
	  li = ((li)->next);
	}
    }
}

#ifdef ANSI_C
static void
gs_ide9 (ADJEDGE edge)
#else
static void
gs_ide9 (edge)
     ADJEDGE edge;
#endif
{
  ADJEDGE l, lnext;
  GNODE d1;
  ADJEDGE e1;
  GEDGE f1, f2;
  int ide, aside1, aside2, tide;;
  f1 = ((edge)->kante);
  l = (((((((edge)->kante))->start)))->succ);
  while (l)
    {
      lnext = ((l)->next);
      f2 = ((l)->kante);
      if (f1 != f2)
	{
	  tide = ide = aside1 = aside2 = 1;
	}
      else
	{
	  tide = ide = aside1 = aside2 = 0;
	}
      if (((f1)->start) != ((f2)->start))
	tide = 0;
      if (((f1)->end) != ((f2)->end))
	tide = 0;
      if (((f1)->linestyle) != ((f2)->linestyle))
	ide = 0;
      if (((f1)->thickness) > ((f2)->thickness))
	ide = 0;
      if (((f1)->priority) > ((f2)->priority))
	ide = 0;
      if (((f1)->horder) != ((f2)->horder))
	ide = 0;
      if (((f1)->eclass) != ((f2)->eclass))
	ide = 0;
      if (((f1)->color) != ((f2)->color))
	ide = 0;
      if (((f1)->arrowsize1) != ((f2)->arrowsize1))
	aside1 = 0;
      if (((f1)->arrowsize2) != ((f2)->arrowsize2))
	aside2 = 0;
      if (((f1)->arrowstyle1) != ((f2)->arrowstyle1))
	aside1 = 0;
      if (((f1)->arrowstyle2) != ((f2)->arrowstyle2))
	aside2 = 0;
      if (((f1)->arrowcolor1) != ((f2)->arrowcolor1))
	aside1 = 0;
      if (((f1)->arrowcolor2) != ((f2)->arrowcolor2))
	aside2 = 0;
      if (((f1)->anchor) != ((f2)->anchor))
	ide = 0;
      if (tide && ide && aside1 && aside2 && summarize_double_edges)
	{
	  delete_adjedge (f1);
	  return;
	}
      if (tide)
	{
	  d1 = gs_ide14 ((((((((l)->kante))->end)))->tiefe));
	  ((d1)->nhorder) = ((f1)->horder);
	  e1 = gs_ide15 ((((((edge)->kante))->start)), d1, f1, 0);
	  gs_ide10 ((((((e1)->kante))->start)), e1,
		    (((((((e1)->kante))->start)))->tiefe));
	  e1 = gs_ide15 (d1, (((((edge)->kante))->end)), ((edge)->kante), 1);
	  ((((e1)->kante))->label) = ((f1)->label);
	  gs_ide10 ((((((e1)->kante))->start)), e1,
		    (((((((e1)->kante))->start)))->tiefe));
	  delete_adjedge (f1);
	  return;
	}
      l = lnext;
    }
}

int number_reversions;

#ifdef ANSI_C
void
calc_number_reversions (void)
#else
void
calc_number_reversions ()
#endif
{
  GNODE v;
  ADJEDGE e;;
  number_reversions = 0;
  v = nodelist;
  while (v)
    {
      e = ((v)->pred);
      while (e)
	{
	  if ((((((e)->kante))->kantenart)) == 'R')
	    number_reversions++;
	  e = ((e)->next);
	}
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      e = ((v)->pred);
      while (e)
	{
	  if ((((((e)->kante))->kantenart)) == 'R')
	    number_reversions++;
	  e = ((e)->next);
	}
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      e = ((v)->pred);
      while (e)
	{
	  if ((((((e)->kante))->kantenart)) == 'R')
	    number_reversions++;
	  e = ((e)->next);
	}
      v = ((v)->next);
    }
}

#ifdef DEBUG
#ifdef ANSI_C
void
db_output_graph (void)
#else
void
db_output_graph ()
#endif
{
  PRINTF ("Nodes:\n");
  gs_ide18 (nodelist);
  PRINTF ("Labels:\n");
  gs_ide18 (labellist);
  PRINTF ("Dummynodes:\n");
  gs_ide18 (dummylist);
  PRINTF ("\n\n");
  PRINTF ("Edges:\n");
  gs_ide16 (edgelist);
  PRINTF ("Temporary edges:\n");
  gs_ide16 (tmpedgelist);
}
#endif

#ifdef DEBUG
#ifdef ANSI_C
static void
gs_ide18 (GNODE n)
#else
static void
gs_ide18 (n)
     GNODE n;
#endif
{
  if (!n)
    return;
  gs_ide18 (((n)->next));
  PRINTF ("%s, ", ((n)->title));
}
#endif

#ifdef DEBUG
#ifdef ANSI_C
static void
gs_ide16 (GEDGE e)
#else
static void
gs_ide16 (e)
     GEDGE e;
#endif
{
  if (!e)
    return;
  gs_ide16 (((e)->next));
  if (((e)->invisible) == 0)
    PRINTF ("(%s -> %s)\n", ((e)->start)->title, ((e)->end)->title);
}
#endif

#ifdef DEBUG
#ifdef ANSI_C
void
db_output_adjacencies (void)
#else
void
db_output_adjacencies ()
#endif
{
  GNODE node;
  ADJEDGE edge;
  PRINTF ("\n\nAdjacency lists: ");
  node = nodelist;
  while (node)
    {
      PRINTF ("\n%s(%d)%ld\n", ((node)->title), ((node)->tiefe), node);
      PRINTF ("(in:%d,out:%d)\n", ((node)->indegree), ((node)->outdegree));
      PRINTF ("Succs:");
      edge = ((node)->succ);
      while (edge)
	{
	  PRINTF ("|%s(%ld) ", (((((((edge)->kante))->end)))->title),
		  (((((edge)->kante))->end)));
	  edge = ((edge)->next);
	}
      PRINTF ("\nPreds:");
      edge = ((node)->pred);
      while (edge)
	{
	  PRINTF ("|%s(%ld) ", (((((((edge)->kante))->start)))->title),
		  (((((edge)->kante))->start)));
	  edge = ((edge)->next);
	}
      node = ((node)->next);
    }
  node = labellist;
  while (node)
    {
      PRINTF ("\n%s(%d)%ld\n", ((node)->title), ((node)->tiefe), node);
      PRINTF ("(in:%d,out:%d)\n", ((node)->indegree), ((node)->outdegree));
      PRINTF ("Succs:");
      edge = ((node)->succ);
      while (edge)
	{
	  PRINTF ("|%s(%ld) ", (((((((edge)->kante))->end)))->title),
		  (((((edge)->kante))->end)));
	  edge = ((edge)->next);
	}
      PRINTF ("\nPreds:");
      edge = ((node)->pred);
      while (edge)
	{
	  PRINTF ("|%s(%ld) ", (((((((edge)->kante))->start)))->title),
		  (((((edge)->kante))->start)));
	  edge = ((edge)->next);
	}
      node = ((node)->next);
    }
  node = dummylist;
  while (node)
    {
      PRINTF ("\n%s(%d)%ld\n", ((node)->title), ((node)->tiefe), node);
      PRINTF ("(in:%d,out:%d)\n", ((node)->indegree), ((node)->outdegree));
      PRINTF ("Succs:");
      edge = ((node)->succ);
      while (edge)
	{
	  PRINTF ("|%s(%ld) ", (((((((edge)->kante))->end)))->title),
		  (((((edge)->kante))->end)));
	  edge = ((edge)->next);
	}
      PRINTF ("\nPreds:");
      edge = ((node)->pred);
      while (edge)
	{
	  PRINTF ("|%s(%ld) ", (((((((edge)->kante))->start)))->title),
		  (((((edge)->kante))->start)));
	  edge = ((edge)->next);
	}
      node = ((node)->next);
    }
  PRINTF ("\n");
}
#endif

#ifdef DEBUG
#define gtitle(v)  ((( v )->title)  ?(( v )->title)  :"??")
#ifdef ANSI_C
void
db_output_adjacency (GNODE node, int f)
#else
void
db_output_adjacency (node, f)
     GNODE node;
     int f;
#endif
{
  ADJEDGE edge;
  PRINTF ("\n\nAdjacency lists: ");
  PRINTF ("\n%ld %s(%d)\n", node, gtitle (node), ((node)->tiefe));
  if (f != 1)
    {
      PRINTF ("Succs:");
      edge = ((node)->succ);
      while (edge)
	{
	  PRINTF ("%ld %c:", edge, (((((edge)->kante))->kantenart)));
	  PRINTF ("(%ld)%s-", (((((edge)->kante))->start)),
		  gtitle ((((((edge)->kante))->start))));
	  PRINTF ("(%ld)%s ", (((((edge)->kante))->end)),
		  gtitle ((((((edge)->kante))->end))));
	  edge = ((edge)->next);
	}
    }
  if (f != 0)
    {
      PRINTF ("\nPreds:");
      edge = ((node)->pred);
      while (edge)
	{
	  PRINTF ("%ld %c:", edge, (((((edge)->kante))->kantenart)));
	  PRINTF ("(%ld)%s-", (((((edge)->kante))->start)),
		  gtitle ((((((edge)->kante))->start))));
	  PRINTF ("(%ld)%s ", (((((edge)->kante))->end)),
		  gtitle ((((((edge)->kante))->end))));
	  edge = ((edge)->next);
	}
      node = ((node)->next);
      PRINTF ("End\n");
    }
}
#endif

#ifdef DEBUG
#define mtitle(v)  ((( v )->title)  ?(( v )->title)  :"??")
#ifdef ANSI_C
static void
gs_ide17 (void)
#else
static void
gs_ide17 ()
#endif
{
  int i;
  GNLIST li;
  GNODE node;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      PRINTF ("Layer %d:\n", i);
      li = ((layer[i]).succlist);
      while (li)
	{
	  node = ((li)->node);
	  PRINTF ("\n%ld %s(%d)\n", node, mtitle (node), ((node)->tiefe));
	  li = ((li)->next);
	}
      PRINTF ("-----------------\n");
    }
}
#endif

#ifdef DEBUG
#ifdef ANSI_C
static void
gs_ide19 (char *fn)
#else
static void
gs_ide19 (fn)
     char *fn;
#endif
{
  int i, j;
  GNLIST li;
  GNODE node;
  ADJEDGE edge;
  CONNECT c1;
  FILE *f;
  f = fopen (fn, "w");
  FPRINTF (f, "graph: { title: \"db_%s\" \n", fn);
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).succlist);
      j = 1;
      while (li)
	{
	  node = ((li)->node);
	  FPRINTF (f, "\nnode: { title: \"%ld\" ", node);
	  if ((((node)->title)) && (((node)->title)[0]))
	    FPRINTF (f, "label: \"%s\" ", ((node)->title));
	  FPRINTF (f, "level: %d ", i);
	  FPRINTF (f, "horizontal_order: %d ", j);
	  FPRINTF (f, "loc: { x: %d y: %d } ", 10 + j * 80, 10 + i * 60);
	  FPRINTF (f, "}\n");
	  edge = ((node)->succ);
	  while (edge)
	    {
	      FPRINTF (f, "edge: { ");
	      FPRINTF (f, "sourcename: \"%ld\" ",
		       (((((edge)->kante))->start)));
	      FPRINTF (f, "targetname: \"%ld\" ", (((((edge)->kante))->end)));
	      FPRINTF (f, "}\n");
	      edge = ((edge)->next);
	    }
	  c1 = ((node)->connection);
	  if (c1)
	    {
	      if ((((c1)->target)) && (((c1)->target) != node))
		{
		  FPRINTF (f, "edge: { ");
		  FPRINTF (f, "sourcename: \"%ld\" ", node);
		  FPRINTF (f, "targetname: \"%ld\" ", ((c1)->target));
		  FPRINTF (f, "linestyle: dashed }\n");
		}
	      if ((((c1)->target2)) && (((c1)->target2) != node))
		{
		  FPRINTF (f, "edge: { ");
		  FPRINTF (f, "sourcename: \"%ld\" ", node);
		  FPRINTF (f, "targetname: \"%ld\" ", ((c1)->target2));
		  FPRINTF (f, "linestyle: dashed }\n");
		}
	    }
	  j++;
	  li = ((li)->next);
	}
    }
  FPRINTF (f, "}\n");
  fclose (f);
}
#endif
