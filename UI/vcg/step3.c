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
#include "folding.h"
#include "drawlib.h"
#include "steps.h"
#include "timelim.h"
#include "timing.h"
#undef DRAWDEBUG
#ifdef DRAWDEBUG
extern void debug_display_part _PP ((void));
extern void display_part _PP ((void));
static void drawdebug_show_graph _PP ((char *m, int i, int j, int k));
#else
#define		drawdebug_show_graph(a,b,c,d)
#endif
static void gs_ide1017 _PP ((void));
static void gs_ide1001 _PP ((void));
static void gs_ide1043 _PP ((void));
static void gs_ide1045 _PP ((void));
static void gs_ide1042 _PP ((int t));
static void gs_ide1044 _PP ((GNODE node, int i));
static void gs_ide1027 _PP ((GNODE node));
static void gs_ide1041 _PP ((void));
static void gs_ide1004 _PP ((GNODE v));
static void gs_ide1051 _PP ((GNODE v));
static void gs_ide1048 _PP ((void));
static int gs_ide1008 _PP ((GNODE v));
static int gs_ide1014 _PP ((GNODE sw, int sxpos, int dir));
static void gs_ide1049 _PP ((GNODE sw, int sxpos, int dir));
static void gs_ide1022 _PP ((void));
static int gs_ide1010 _PP ((int f));
static void gs_ide1021 _PP ((void));
static int gs_ide1003 _PP ((void));
static int gs_ide1036 _PP ((int i, int dir));
static int gs_ide1034 _PP ((int i, int dir));
static int gs_ide1032 _PP ((int i, int dir));
static int gs_ide1050 _PP ((int i, int dir));
static int gs_ide1039 _PP ((int i, int dir));
static int gs_ide1005 _PP ((int i));
static int gs_ide1002 _PP ((void));
static int gs_ide1000 _PP ((int i));
static int gs_ide1006 _PP ((int i));
static int gs_ide1007 _PP ((int i));
static int gs_ide1030 _PP ((GNODE node));
static int gs_ide1031 _PP ((GNODE node, GNODE lnode, GNODE rnode));
static int gs_ide1012 _PP ((GEDGE edge));
static int gs_ide1035 _PP ((GNODE node));
static int gs_ide1011 _PP ((GEDGE edge));
static int gs_ide1033 _PP ((GNODE node));
static void gs_ide1040 _PP ((int i));
static int gs_ide1046;
static int *gs_ide1025 = NULL;
static int *gs_ide1026 = NULL;
static GNODE *gs_ide1047 = NULL;
GNLIST *tpred_connection1 = NULL;
GNLIST *tpred_connection2 = NULL;
#define forward_connection1(c)  (((( c )->edge)  )&& ((( (( c )->edge)   )->end)  ==(( c )->target)  ))
#define forward_connection2(c)  (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  ==(( c )->target2)  ))
#define backward_connection1(c) (((( c )->edge)  )&& ((( (( c )->edge)   )->end)  !=(( c )->target)  ))
#define backward_connection2(c) (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  !=(( c )->target2)  ))
#define TMINX(x) (( x ).cross)
#ifdef DRAWDEBUG
static int gs_ide1013 = 1;
#endif
#ifdef ANSI_C
void
step3_main (void)
#else
void
step3_main ()
#endif
{
  start_time ();;
  assert ((layer));
#ifdef DRAWDEBUG
  if (gs_ide1013)
    {
      gs_ide1013 = 0;
      PRINTF ("Enter ` ` into the drawwindow\n");
      display_part ();
      return;
    }
#endif
  calc_all_ports (0);
  calc_all_node_sizes ();
  gs_ide1017 ();
  gs_ide1001 ();
  alloc_levelshift ();
  gs_ide1022 ();
  gs_ide1021 ();
  if (straight_phase == 1)
    gs_ide1048 ();
  gs_ide1043 ();
  gs_ide1045 ();
  gs_ide1041 ();
  calc_all_ports (1);
  stop_time ("step3_main");
}

#ifdef ANSI_C
void
alloc_levelshift (void)
#else
void
alloc_levelshift ()
#endif
{;
  if (sizeof (GNLIST) < sizeof (GNODE))
    Fatal_error ("Assertion 1 about structs is wrong.", "");
  if (sizeof (GNLIST) < sizeof (int))
    Fatal_error ("Assertion 2 about structs is wrong.", "");
#ifdef TESTLAYOUT
  PRINTF ("Maximal nodes per layer: %d\n", max_nodes_per_layer);
  PRINTF ("Maximal layers: %d\n", maxdepth);
#endif
  if (max_nodes_per_layer + 2 > gs_ide1046)
    {
      if (gs_ide1025)
	free (gs_ide1025);
      if (tpred_connection1)
	free (tpred_connection1);
      if (tpred_connection2)
	free (tpred_connection2);
      gs_ide1025 = (int *) malloc ((max_nodes_per_layer + 2) * sizeof (int));
      tpred_connection1 =
	(GNLIST *) malloc ((max_nodes_per_layer + 2) * sizeof (GNLIST));
      tpred_connection2 =
	(GNLIST *) malloc ((max_nodes_per_layer + 2) * sizeof (GNLIST));
      gs_ide1026 = (int *) tpred_connection1;
      gs_ide1047 = (GNODE *) tpred_connection2;
      if ((!gs_ide1025) || (!gs_ide1026) || (!gs_ide1047))
	Fatal_error ("memory exhausted", "");
      gs_ide1046 = max_nodes_per_layer + 2;
#ifdef DEBUG
      PRINTF
	("Sizeof tables `levelshift',`levelweight',`slayer_array': %ld Bytes\n",
	 (max_nodes_per_layer + 2) * sizeof (int));
#endif
    }
}

#ifdef ANSI_C
void
calc_all_node_sizes (void)
#else
void
calc_all_node_sizes ()
#endif
{
  GNODE v;
  int h, hh, hhh;
  ADJEDGE a;;
  v = nodelist;
  while (v)
    {
      calc_node_size (v);
      if ((G_orientation == 1) || (G_orientation == 2))
	{
	  h = ((v)->width);
	  ((v)->width) = ((v)->height);
	  ((v)->height) = h;
	}
      v = ((v)->next);
    }
  if ((G_orientation == 1) || (G_orientation == 2))
    {
      v = labellist;
      while (v)
	{
	  h = ((v)->width);
	  ((v)->width) = ((v)->height);
	  ((v)->height) = h;
	  v = ((v)->next);
	}
    }
  v = dummylist;
  while (v)
    {
      if (((v)->anchordummy))
	{
	  a = ((v)->pred);
	  h = 0;
	  while (a)
	    {
	      a = ((a)->next);
	      h++;
	    }
	  a = ((v)->succ);
	  hh = 0;
	  while (a)
	    {
	      a = ((a)->next);
	      hh++;
	    }
	  hhh = h;
	  if (hh > h)
	    hhh = hh;
	  assert ((((v)->connection)));
	  ((v)->height) = ((((((v)->connection))->target))->height);
	  ((v)->width) = (hhh + 1) * G_xspace;
	  if (manhatten_edges == 1)
	    {
	      if ((h <= 1) && (hh == 0))
		((v)->width) = 0;
	      if ((h == 0) && (hh <= 1))
		((v)->width) = 0;
	    }
	}
      else
	{
	  ((v)->width) = 0;
	  ((v)->height) = 0;
	}
      if ((G_orientation == 1) || (G_orientation == 2))
	{
	  h = ((v)->width);
	  ((v)->width) = ((v)->height);
	  ((v)->height) = h;
	}
      v = ((v)->next);
    }
}

#define xralign(a)  ((((a)+G_xraster-1)/G_xraster)*G_xraster)
#define yralign(a)  ((((a)+G_yraster-1)/G_yraster)*G_yraster)
#define dxralign(a) ((((a)+G_dxraster-1)/G_dxraster)*G_dxraster)
#define dyralign(a) ((((a)+G_dyraster-1)/G_dyraster)*G_dyraster)
#define xllalign(a)  ((((a)-G_xraster+1)/G_xraster)*G_xraster)
#define xlalign(a)  ((((a))/G_xraster)*G_xraster)
#define ylalign(a)  ((((a))/G_yraster)*G_yraster)
#define dxlalign(a) ((((a))/G_dxraster)*G_dxraster)
#define dylalign(a) ((((a))/G_dyraster)*G_dyraster)
#ifdef ANSI_C
static void
gs_ide1017 (void)
#else
static void
gs_ide1017 ()
#endif
{
  int actxpos, actypos;
  int maxboxheight;
  GNLIST li;
  GNODE v;
  int i;;
  if (G_yspace < 5)
    G_yspace = 5;
  if (G_xspace < 5)
    G_xspace = 5;
  if (G_dspace == 0)
    {
      if (G_spline)
	G_dspace = 4 * G_xspace / 5;
      else
	G_dspace = G_xspace / 2;
    }
  if (G_flat_factor < 1)
    G_flat_factor = 1;
  if (G_flat_factor > 100)
    G_flat_factor = 100;
  actypos = G_ybase;
  actypos = yralign (actypos);
  for (i = 0; i <= maxdepth + 1; i++)
    {
      actxpos = G_xbase;
      maxboxheight = 0;
      li = ((layer[i]).succlist);
      while (li)
	{
	  v = ((li)->node);
	  if ((((v)->width) == 0) && (((v)->height) == 0))
	    ((v)->xloc) =
	      dxralign (actxpos + ((v)->width) / 2) - ((v)->width) / 2;
	  else
	    ((v)->xloc) =
	      xralign (actxpos + ((v)->width) / 2) - ((v)->width) / 2;
	  ((v)->yloc) = actypos;
	  actxpos = ((v)->xloc) + ((v)->width) + G_xspace;
	  if (maxboxheight < ((v)->height))
	    maxboxheight = ((v)->height);
	  li = ((li)->next);
	}
      if (G_yalign == 1)
	{
	  li = ((layer[i]).succlist);
	  while (li)
	    {
	      ((((li)->node))->yloc) +=
		(maxboxheight - ((((li)->node))->height)) / 2;
	      li = ((li)->next);
	    }
	}
      else if (G_yalign == 2)
	{
	  li = ((layer[i]).succlist);
	  while (li)
	    {
	      ((((li)->node))->yloc) +=
		(maxboxheight - ((((li)->node))->height));
	      li = ((li)->next);
	    }
	}
      actypos += (maxboxheight + G_yspace);
      actypos = yralign (actypos);
    }
}

#ifdef ANSI_C
static void
gs_ide1001 (void)
#else
static void
gs_ide1001 ()
#endif
{
  GNLIST li;
  int i;
  int shift_value;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).predlist);
      if (li)
	{
	  assert ((((layer[i]).succlist)));
	  shift_value =
	    (((((li)->node))->xloc) + ((((li)->node))->width)) / 2;
	  shift_value = xlalign (shift_value);
	  while (li)
	    {
	      ((((li)->node))->xloc) -= shift_value;
	      li = ((li)->next);
	    }
	}
    }
}

#ifdef ANSI_C
static void
gs_ide1043 (void)
#else
static void
gs_ide1043 ()
#endif
{
  GNLIST li;
  int i;
  int minx, miny;;
  minx = miny = MAXINT;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  if (((((li)->node))->xloc) < minx)
	    minx = ((((li)->node))->xloc);
	  if (((((li)->node))->yloc) < miny)
	    miny = ((((li)->node))->yloc);
	  li = ((li)->next);
	}
    }
  minx = minx - G_xbase;
  miny = miny - G_ybase;
  minx = xlalign (minx);
  miny = xlalign (miny);
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  ((((li)->node))->xloc) -= minx;
	  ((((li)->node))->yloc) -= miny;
	  li = ((li)->next);
	}
    }
}
static int gs_ide1015;
#ifdef ANSI_C
static void
gs_ide1045 (void)
#else
static void
gs_ide1045 ()
#endif
{
  GNLIST li;
  int i;
  int minx, part_is_missing;
  GNODE node;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  ((((li)->node))->markiert) = 0;
	  li = ((li)->next);
	}
    }
  gs_ide1015 = G_xbase - G_xspace - 5;
  part_is_missing = 1;
  while (part_is_missing)
    {
      part_is_missing = 0;
      minx = MAXINT;
      node = (GNODE) 0;
      for (i = 0; i <= maxdepth + 1; i++)
	{
	  li = ((layer[i]).succlist);
	  while (li)
	    {
	      if (((((li)->node))->markiert) == 0)
		{
		  if (minx > ((((li)->node))->xloc))
		    {
		      node = ((li)->node);
		      minx = ((node)->xloc);
		    }
		  break;
		}
	      li = ((li)->next);
	    }
	}
      if (node)
	{
	  assert ((((node)->markiert) == 0));
	  part_is_missing = 1;
	  if (minx > gs_ide1015 + G_xspace + 5)
	    {
	      i = minx - gs_ide1015 - G_xspace - 5;
	      i = xlalign (i);
	      gs_ide1044 (node, i);
	      gs_ide1042 (i);
	    }
	  gs_ide1027 (node);
	}
#ifdef DRAWDEBUG
#ifdef NEVER
      PRINTF ("After one shift left together\n");
      PRINTF ("Enter CR into the text window\n");
      step4_main ();
      debug_display_part ();
      fgetc (stdin);
#endif
#endif
    }
}

#ifdef ANSI_C
static void
gs_ide1042 (int t)
#else
static void
gs_ide1042 (t)
     int t;
#endif
{
  int i, intermixed_part_found;
  GNLIST li;
  GNODE node;
  intermixed_part_found = 1;
  while (intermixed_part_found)
    {
      intermixed_part_found = 0;
      node = NULL;
      for (i = 0; i <= maxdepth + 1; i++)
	{
	  li = ((layer[i]).succlist);
	  while (li)
	    {
	      if (((((li)->node))->markiert) == 0)
		{
		  if ((((li)->next))
		      && (((((((li)->next))->node))->markiert)))
		    {
		      node = ((li)->node);
		      intermixed_part_found = 1;
		      break;
		    }
		}
	      li = ((li)->next);
	    }
	  if (intermixed_part_found)
	    break;
	}
      if (intermixed_part_found)
	{
	  assert ((((node)->markiert) == 0));
	  gs_ide1044 (node, t);
	  gs_ide1027 (node);
	}
    }
}

#ifdef ANSI_C
static void
gs_ide1044 (GNODE node, int i)
#else
static void
gs_ide1044 (node, i)
     GNODE node;
     int i;
#endif
{
  ADJEDGE e;;
  if (((node)->markiert))
    return;
  ((node)->markiert) = 1;
  ((node)->xloc) -= i;
  if (((node)->connection))
    {
      if (((((node)->connection))->target))
	gs_ide1044 (((((node)->connection))->target), i);
      if (((((node)->connection))->target2))
	gs_ide1044 (((((node)->connection))->target2), i);
    }
  e = ((node)->succ);
  while (e)
    {
      gs_ide1044 ((((((e)->kante))->end)), i);
      e = ((e)->next);
    }
  e = ((node)->pred);
  while (e)
    {
      gs_ide1044 ((((((e)->kante))->start)), i);
      e = ((e)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide1027 (GNODE node)
#else
static void
gs_ide1027 (node)
     GNODE node;
#endif
{
  ADJEDGE e;;
  if (((node)->markiert) == 2)
    return;
  ((node)->markiert) = 2;
  if (((node)->xloc) + ((node)->width) > gs_ide1015)
    gs_ide1015 = ((node)->xloc) + ((node)->width);
  if (((node)->connection))
    {
      if (((((node)->connection))->target))
	gs_ide1027 (((((node)->connection))->target));
      if (((((node)->connection))->target2))
	gs_ide1027 (((((node)->connection))->target2));
    }
  e = ((node)->succ);
  while (e)
    {
      gs_ide1027 ((((((e)->kante))->end)));
      e = ((e)->next);
    }
  e = ((node)->pred);
  while (e)
    {
      gs_ide1027 ((((((e)->kante))->start)));
      e = ((e)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide1041 (void)
#else
static void
gs_ide1041 ()
#endif
{
  GNLIST li;
  int i;
  int weight, found;
  GNODE node;
  CONNECT c;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  weight = 0;
	  node = ((li)->node);
	  c = ((node)->connection);
	  found = 0;
	  if (c)
	    {
	      if (((c)->target))
		{
		  if (((((c)->target))->xloc) < ((node)->xloc))
		    {
		      weight = ((((c)->edge))->priority);
		      weight *= layout_nearfactor;
		      if (weight == 0)
			found = 1;
		    }
		}
	      if (((c)->target2))
		{
		  if (((((c)->target2))->xloc) < ((node)->xloc))
		    {
		      weight = ((((c)->edge2))->priority);
		      weight *= layout_nearfactor;
		      if (weight == 0)
			found = 1;
		    }
		}
	    }
	  if (found)
	    gs_ide1004 (node);
	  li = ((li)->next);
	}
    }
}

#ifdef ANSI_C
static void
gs_ide1004 (GNODE v)
#else
static void
gs_ide1004 (v)
     GNODE v;
#endif
{
  GNLIST li;
  int i;
  int mindist, found;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).succlist);
      ((layer[i]).actx) = MAXINT;
      TMINX (layer[i]) = MAXINT;
      ((layer[i]).refnode) = NULL;
      while (li)
	{
	  ((((li)->node))->markiert) = 0;
	  li = ((li)->next);
	}
    }
  gs_ide1051 (v);
  mindist = MAXINT;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      if (((layer[i]).actx) < mindist)
	mindist = ((layer[i]).actx);
    }
  if ((mindist > 0) && (mindist != MAXINT))
    {
      for (i = 0; i <= maxdepth + 1; i++)
	{
	  li = ((layer[i]).succlist);
	  found = 0;
	  while (li)
	    {
	      if (((li)->node) == ((layer[i]).refnode))
		found = 1;
	      if (found)
		((((li)->node))->xloc) -= mindist;
	      li = ((li)->next);
	    }
	}
    }
}

#ifdef ANSI_C
static void
gs_ide1051 (GNODE v)
#else
static void
gs_ide1051 (v)
     GNODE v;
#endif
{
  int level, weight;
  GNLIST li;
  ADJEDGE e;
  CONNECT c;;
  if (!v)
    return;
  if (((v)->markiert))
    return;
  ((v)->markiert) = 1;
  level = ((v)->tiefe);
  if (((v)->xloc) < TMINX (layer[level]))
    {
      ((layer[level]).refnode) = v;
      TMINX (layer[level]) = ((v)->xloc);
      li = ((layer[level]).predlist);
      while (li)
	{
	  if (((li)->node) == v)
	    break;
	  li = ((li)->next);
	}
      assert ((li));
      li = ((li)->next);
      if (li)
	{
	  ((layer[level]).actx) =
	    ((v)->xloc) - G_xspace - ((((li)->node))->xloc) -
	    ((((li)->node))->width);
	}
      else
	((layer[level]).actx) = ((v)->xloc) - G_xbase;
    }
  c = ((v)->connection);
  if (c)
    {
      if (((c)->target))
	{
	  if (((((c)->target))->xloc) > ((v)->xloc))
	    weight = 1;
	  else
	    weight = ((((c)->edge))->priority) * layout_nearfactor;
	  if (weight != 0)
	    gs_ide1051 (((c)->target));
	}
      if (((c)->target2))
	{
	  if (((((c)->target2))->xloc) > ((v)->xloc))
	    weight = 1;
	  else
	    weight = ((((c)->edge2))->priority) * layout_nearfactor;
	  if (weight != 0)
	    gs_ide1051 (((c)->target2));
	}
    }
  e = ((v)->succ);
  while (e)
    {
      gs_ide1051 ((((((e)->kante))->end)));
      e = ((e)->next);
    }
  e = ((v)->pred);
  while (e)
    {
      gs_ide1051 ((((((e)->kante))->start)));
      e = ((e)->next);
    }
}
static int gs_ide1009;
static int gs_ide1037 = MAXINT;
static int gs_ide1028 = MAXINT;
static int gs_ide1038 = MAXINT;
static int gs_ide1029 = MAXINT;
static int gs_ide1023 = 0;
static int gs_ide1024 = 0;
#ifdef ANSI_C
static void
gs_ide1022 (void)
#else
static void
gs_ide1022 ()
#endif
{
  int count;
  int changed;
  int tryout;;
  gs_ide1037 = MAXINT;
  gs_ide1028 = MAXINT;
  gs_ide1038 = MAXINT;
  gs_ide1029 = MAXINT;
  gs_ide1023 = 0;
  gs_ide1024 = 0;
  count = 0;
  tryout = 2;
  gs_ide1009 = 2;
  while (1)
    {
      if (count % 5 == 0)
	{
	  gs_wait_message ('m');
	  gs_ide1043 ();
	}
      count++;
      drawdebug_show_graph ("dump mediumshift", count, 0, tryout);
      changed = gs_ide1010 (0);
      changed += gs_ide1010 (1);
      gs_ide1009 = 1;
      if ((!changed) && (count >= min_mediumshifts))
	break;
      if (count >= max_mediumshifts)
	{
	  gs_wait_message ('t');
	  break;
	}
      if (G_timelimit > 0)
	if (test_timelimit (85))
	  {
	    gs_wait_message ('t');
	    break;
	  }
      if (count >= min_mediumshifts)
	{
	  if (!gs_ide1003 ())
	    {
	      tryout--;
	      if (tryout == 0)
		break;
	    }
	  else
	    tryout = 2;
	}
    }
}

#ifdef ANSI_C
static int
gs_ide1003 (void)
#else
static int
gs_ide1003 ()
#endif
{
  int i, nwval1, nwval2, k;
  int changed;
  GNLIST li;;
  changed = nwval1 = nwval2 = 0;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  k = gs_ide1035 (((li)->node));
	  if (k < 0)
	    k = -k;
	  nwval1 += k;
	  k = gs_ide1033 (((li)->node));
	  if (k < 0)
	    k = -k;
	  nwval2 += k;
	  li = ((li)->next);
	}
    }
  if (nwval1 < gs_ide1028)
    {
      changed = 1;
      gs_ide1037 = gs_ide1028 = nwval1;
      gs_ide1023 = 0;
    }
  else
    {
      if (nwval1 < gs_ide1037)
	{
	  changed = 1;
	  gs_ide1037 = nwval1;
	}
      if (gs_ide1023 < 1)
	{
	  gs_ide1037 = nwval1;
	  gs_ide1023++;
	}
    }
  if (nwval2 < gs_ide1029)
    {
      changed = 1;
      gs_ide1038 = gs_ide1029 = nwval2;
      gs_ide1024 = 0;
    }
  else
    {
      if (nwval2 < gs_ide1038)
	{
	  changed = 1;
	  gs_ide1038 = nwval2;
	}
      if (gs_ide1024 < 1)
	{
	  gs_ide1038 = nwval2;
	  gs_ide1024++;
	}
    }
#ifdef DRAWDEBUG
  printf ("Graph Weight: minimal: %d %d old: %d %d actual: %d %d\n",
	  gs_ide1028, gs_ide1029, gs_ide1037, gs_ide1038, nwval1, nwval2);
#endif
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1010 (int first)
#else
static int
gs_ide1010 (first)
     int first;
#endif
{
  int i;
  int layer_changed;
  int dir;;
  layer_changed = 0;
  dir = 0;
  if (first)
    {
      for (i = 1; i <= maxdepth + 1; i++)
	{
	  dir = 1 - dir;
	  layer_changed += gs_ide1034 (i, dir);
	  dir = 1 - dir;
	  layer_changed += gs_ide1034 (i, dir);
	}
      if (prio_phase == 1)
	{
	  for (i = 0; i <= maxdepth + 1; i++)
	    layer_changed += gs_ide1005 (i);
	}
    }
  for (i = maxdepth; i >= 0; i--)
    {
      dir = 1 - dir;
      layer_changed += gs_ide1036 (i, dir);
      dir = 1 - dir;
      layer_changed += gs_ide1036 (i, dir);
    }
  if (prio_phase == 1)
    {
      for (i = 0; i <= maxdepth + 1; i++)
	layer_changed += gs_ide1005 (i);
    }
  if (!first)
    {
      for (i = 1; i <= maxdepth + 1; i++)
	{
	  dir = 1 - dir;
	  layer_changed += gs_ide1034 (i, dir);
	  dir = 1 - dir;
	  layer_changed += gs_ide1034 (i, dir);
	}
      if (prio_phase == 1)
	{
	  for (i = 0; i <= maxdepth + 1; i++)
	    layer_changed += gs_ide1005 (i);
	}
    }
  if (nwdumping_phase)
    {
      for (i = maxdepth; i >= 0; i--)
	{
	  dir = 1 - dir;
	  layer_changed += gs_ide1032 (i, dir);
	  dir = 1 - dir;
	  layer_changed += gs_ide1032 (i, dir);
	}
      if (prio_phase == 1)
	{
	  for (i = 0; i <= maxdepth + 1; i++)
	    layer_changed += gs_ide1005 (i);
	}
    }
  if (layer_changed)
    return (1);
  return (0);
}

#ifdef ANSI_C
static int
gs_ide1018 (GNODE v)
#else
static int
gs_ide1018 (v)
     GNODE v;
#endif
{
  int nr_edges;
  int pr;
  ADJEDGE a;
  if (!v)
    return (0);
  pr = 1;
  a = ((v)->succ);
  nr_edges = 0;
  while (a)
    {
      nr_edges++;
      a = ((a)->next);
    }
  if (nr_edges > 1)
    pr = 0;
  a = ((v)->pred);
  nr_edges = 0;
  while (a)
    {
      nr_edges++;
      a = ((a)->next);
    }
  if (nr_edges > 1)
    pr = 0;
  return (pr);
}

#ifdef ANSI_C
static int
gs_ide1020 (GNODE v)
#else
static int
gs_ide1020 (v)
     GNODE v;
#endif
{
  int nr_edges;
  ADJEDGE a;
  if (!v)
    return (0);
  a = ((v)->succ);
  nr_edges = 0;
  while (a)
    {
      nr_edges++;
      a = ((a)->next);
    }
  return ((nr_edges == 1));
}

#ifdef ANSI_C
static int
gs_ide1019 (GNODE v)
#else
static int
gs_ide1019 (v)
     GNODE v;
#endif
{
  int nr_edges;
  ADJEDGE a;
  if (!v)
    return (0);
  a = ((v)->pred);
  nr_edges = 0;
  while (a)
    {
      nr_edges++;
      a = ((a)->next);
    }
  return ((nr_edges == 1));
}

#ifdef ANSI_C
static int
gs_ide1036 (int i, int dir)
#else
static int
gs_ide1036 (i, dir)
     int i;
     int dir;
#endif
{
  GNLIST li;
  int j;
  int sign;;
  assert ((i <= maxdepth));
  gs_ide1040 (i);
  li = ((layer[i]).succlist);
  j = 0;
  sign = 1;
  while (li)
    {
      gs_ide1025[j] = gs_ide1035 (((li)->node));
      if ((sign < 0) && (gs_ide1025[j] >= 0))
	gs_ide1026[j] = 1;
      else
	gs_ide1026[j] = MAXINT;
      if (gs_ide1025[j] < 0)
	sign = -1;
      else
	sign = 1;
      j++;
      li = ((li)->next);
    }
  gs_ide1026[0] = 1;
  if (prio_phase == 1)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  ((((li)->node))->dfsnum) = gs_ide1020 (((li)->node));
	  li = ((li)->next);
	}
      return (gs_ide1039 (i, dir));
    }
  return (gs_ide1050 (i, dir));
}

#ifdef ANSI_C
static int
gs_ide1034 (int i, int dir)
#else
static int
gs_ide1034 (i, dir)
     int i;
     int dir;
#endif
{
  GNLIST li;
  int j;
  int sign;;
  assert ((i > 0));
  gs_ide1040 (i);
  li = ((layer[i]).succlist);
  j = 0;
  sign = 1;
  while (li)
    {
      gs_ide1025[j] = gs_ide1033 (((li)->node));
      if ((sign < 0) && (gs_ide1025[j] >= 0))
	gs_ide1026[j] = 1;
      else
	gs_ide1026[j] = MAXINT;
      if (gs_ide1025[j] < 0)
	sign = -1;
      else
	sign = 1;
      j++;
      li = ((li)->next);
    }
  gs_ide1026[0] = 1;
  if (prio_phase == 1)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  ((((li)->node))->dfsnum) = gs_ide1019 (((li)->node));
	  li = ((li)->next);
	}
      return (gs_ide1039 (i, dir));
    }
  return (gs_ide1050 (i, dir));
}

#ifdef ANSI_C
static int
gs_ide1032 (int i, int dir)
#else
static int
gs_ide1032 (i, dir)
     int i;
     int dir;
#endif
{
  GNLIST li, li1;
  int j;
  int sign;
  GNODE lnode, node, rnode;;
  assert ((i <= maxdepth));
  gs_ide1040 (i);
  li = ((layer[i]).succlist);
  j = 0;
  sign = 1;
  lnode = NULL;
  while (li)
    {
      node = ((li)->node);
      if (((node)->width) == 0)
	{
	  li1 = ((li)->next);
	  rnode = NULL;
	  while (li1)
	    {
	      if (((((li1)->node))->width) != 0)
		{
		  rnode = ((li1)->node);
		  break;
		}
	      li1 = ((li1)->next);
	    }
	  gs_ide1025[j] = gs_ide1031 (node, lnode, rnode);
	}
      else
	{
	  gs_ide1025[j] = gs_ide1030 (node);
	  lnode = node;
	}
      if ((sign < 0) && (gs_ide1025[j] >= 0))
	gs_ide1026[j] = 1;
      else
	gs_ide1026[j] = MAXINT;
      if (gs_ide1025[j] < 0)
	sign = -1;
      else
	sign = 1;
      j++;
      li = ((li)->next);
    }
  gs_ide1026[0] = 1;
  return (gs_ide1050 (i, dir));
}

#define touching(v,w)  (  ((( v )->xloc)  +(( v )->width)  +G_xspace >= (( w )->xloc)  )       			||((( w )->xloc)  - (( v )->xloc)  - (( v )->width)  <=2*G_xraster))
#ifdef ANSI_C
static int
gs_ide1050 (int i, int dir)
#else
static int
gs_ide1050 (i, dir)
     int i;
     int dir;
#endif
{
  GNLIST li;
  int j;
  int changed;
  int oldpos, sum, nrnodes;
  GNODE v, w;;
  li = ((layer[i]).succlist);
  j = 0;
  while (li)
    {
      v = ((li)->node);
      if (((li)->next))
	{
	  w = ((((li)->next))->node);
	  if (!touching (v, w))
	    gs_ide1026[j + 1] = 1;
	}
      j++;
      li = ((li)->next);
    }
  changed = 1;
  while (changed)
    {
      changed = 0;
      li = ((layer[i]).succlist);
      j = 0;
      oldpos = -1;
      nrnodes = 1;
      while (li)
	{
	  if (gs_ide1026[j] != MAXINT)
	    {
	      if (oldpos != -1)
		gs_ide1026[oldpos] = sum / nrnodes;
	      sum = gs_ide1025[j];
	      nrnodes = 1;
	      oldpos = j;
	    }
	  else
	    {
	      sum += gs_ide1025[j];
	      nrnodes++;
	    }
	  j++;
	  li = ((li)->next);
	}
      if (oldpos != -1)
	gs_ide1026[oldpos] = sum / nrnodes;
      li = ((layer[i]).succlist);
      j = 0;
      sum = 0;
      while (li)
	{
	  if (gs_ide1026[j] != MAXINT)
	    sum = gs_ide1026[j];
	  if (((li)->next) && (gs_ide1026[j + 1] != MAXINT))
	    {
	      v = ((li)->node);
	      w = ((((li)->next))->node);
	      if (touching (v, w))
		{
		  if ((sum >= 0) && (gs_ide1026[j + 1] >= 0)
		      && (sum >= gs_ide1026[j + 1]))
		    {
		      gs_ide1026[j + 1] = MAXINT;
		      changed = 1;
		    }
		  if ((sum < 0) && (gs_ide1026[j + 1] < 0)
		      && (sum >= gs_ide1026[j + 1]))
		    {
		      gs_ide1026[j + 1] = MAXINT;
		      changed = 1;
		    }
		  if ((sum > 0) && (gs_ide1026[j + 1] < 0))
		    {
		      gs_ide1026[j + 1] = MAXINT;
		      changed = 1;
		    }
		}
	    }
	  j++;
	  li = ((li)->next);
	}
    }
  li = ((layer[i]).succlist);
  j = 0;
  assert ((gs_ide1026[0] != MAXINT));
  while (li)
    {
      if (gs_ide1026[j] != MAXINT)
	sum = gs_ide1009 * gs_ide1026[j];
      gs_ide1025[j] = sum;
      gs_ide1026[j] = 1;
      j++;
      li = ((li)->next);
    }
  if (dir)
    {
      changed = gs_ide1007 (i);
      changed += gs_ide1006 (i);
    }
  else
    {
      changed = gs_ide1006 (i);
      changed += gs_ide1007 (i);
    }
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1039 (int i, int dir)
#else
static int
gs_ide1039 (i, dir)
     int i;
     int dir;
#endif
{
  GNLIST li;
  int j;
  int changed;
  int oldpos, sum, nrnodes;
  int priosum, nrprionodes, lprio, rprio;
  GNODE v, w;;
  li = ((layer[i]).succlist);
  j = 0;
  while (li)
    {
      v = ((li)->node);
      if (((li)->next))
	{
	  w = ((((li)->next))->node);
	  if (!touching (v, w))
	    gs_ide1026[j + 1] = 1;
	}
      j++;
      li = ((li)->next);
    }
  changed = 1;
  while (changed)
    {
      changed = 0;
      li = ((layer[i]).succlist);
      j = 0;
      while (li)
	{
	  if (((((li)->node))->dfsnum) && (gs_ide1025[j] == 0))
	    {
	      gs_ide1026[j] = 1;
	      gs_ide1026[j + 1] = 1;
	    }
	  j++;
	  li = ((li)->next);
	}
      li = ((layer[i]).succlist);
      j = 0;
      oldpos = -1;
      nrnodes = 1;
      priosum = 0;
      lprio = 0;
      rprio = 0;
      nrprionodes = 0;
      while (li)
	{
	  if (gs_ide1026[j] != MAXINT)
	    {
	      if (oldpos != -1)
		{
		  gs_ide1026[oldpos] = sum / nrnodes;
		  if (nrprionodes != 0)
		    {
		      if (priosum > 0)
			{
			  gs_ide1026[oldpos] = lprio;
			}
		      else
			{
			  gs_ide1026[oldpos] = rprio;
			}
		    }
		}
	      lprio = 0;
	      rprio = 0;
	      priosum = 0;
	      nrprionodes = 0;
	      sum = 0;
	      nrnodes = 0;
	      oldpos = j;
	    }
	  if (((((li)->node))->dfsnum))
	    {
	      priosum += gs_ide1025[j];
	      if (lprio == 0)
		lprio = gs_ide1025[j];
	      rprio = gs_ide1025[j];
	      nrprionodes++;
	    }
	  sum += gs_ide1025[j];
	  nrnodes++;
	  j++;
	  li = ((li)->next);
	}
      if (oldpos != -1)
	gs_ide1026[oldpos] = sum / nrnodes;
      li = ((layer[i]).succlist);
      j = 0;
      sum = 0;
      while (li)
	{
	  if (gs_ide1026[j] != MAXINT)
	    sum = gs_ide1026[j];
	  if (((li)->next) && (gs_ide1026[j + 1] != MAXINT))
	    {
	      v = ((li)->node);
	      w = ((((li)->next))->node);
	      if ((touching (v, w)) && (!((v)->dfsnum)) && (!((w)->dfsnum)))
		{
		  if ((sum >= 0) && (gs_ide1026[j + 1] >= 0)
		      && (sum >= gs_ide1026[j + 1]))
		    {
		      gs_ide1026[j + 1] = MAXINT;
		      changed = 1;
		    }
		  if ((sum < 0) && (gs_ide1026[j + 1] < 0)
		      && (sum >= gs_ide1026[j + 1]))
		    {
		      gs_ide1026[j + 1] = MAXINT;
		      changed = 1;
		    }
		  if ((sum > 0) && (gs_ide1026[j + 1] < 0))
		    {
		      gs_ide1026[j + 1] = MAXINT;
		      changed = 1;
		    }
		}
	    }
	  j++;
	  li = ((li)->next);
	}
    }
  li = ((layer[i]).succlist);
  j = 0;
  assert ((gs_ide1026[0] != MAXINT));
  while (li)
    {
      if (gs_ide1026[j] != MAXINT)
	sum = gs_ide1009 * gs_ide1026[j];
      gs_ide1025[j] = sum;
      gs_ide1026[j] = 1;
      j++;
      li = ((li)->next);
    }
  if (dir)
    {
      changed = gs_ide1007 (i);
      changed += gs_ide1006 (i);
    }
  else
    {
      changed = gs_ide1006 (i);
      changed += gs_ide1007 (i);
    }
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1005 (int i)
#else
static int
gs_ide1005 (i)
     int i;
#endif
{
  GNLIST li;
  int j, k, changed;
  int ldiff, rdiff;
  int smove, pmove;
  GNODE v, w, oldv;
  int dist;;
  assert ((i >= 0));
#ifdef DRAWDEBUG
#ifdef NEVER
  PRINTF ("Before correct priority nodes line %d\n", i);
  PRINTF ("Enter CR into the text window\n");
  step4_main ();
  debug_display_part ();
  fgetc (stdin);
#endif
#endif
  gs_ide1040 (i);
  k = 5;
  changed = 1;
  while (changed)
    {
      k--;
      if (k < 0)
	break;
      changed = 0;
      j = 0;
      oldv = NULL;
      li = ((layer[i]).succlist);
      while (li)
	{
	  gs_ide1025[j] = 0;
	  gs_ide1026[j] = 1;
	  v = ((li)->node);
	  rdiff = ldiff = MAXINT;
	  if (((li)->next))
	    {
	      dist = G_xspace;
	      if ((((v)->width) == 0) && (((v)->height) == 0))
		dist = G_dspace;
	      w = ((((li)->next))->node);
	      if ((((w)->width) == 0) && (((w)->height) == 0))
		dist = G_dspace;
	      rdiff = ((w)->xloc) - (((v)->xloc) + ((v)->width) + dist);
	    }
	  if (oldv)
	    {
	      dist = G_xspace;
	      if ((((v)->width) == 0) && (((v)->height) == 0))
		dist = G_dspace;
	      w = oldv;
	      if ((((w)->width) == 0) && (((w)->height) == 0))
		dist = G_dspace;
	      ldiff = ((v)->xloc) - (((w)->xloc) + ((w)->width) + dist);
	    }
	  if (gs_ide1020 (v) && gs_ide1019 (v))
	    {
	      smove = gs_ide1035 (v);
	      pmove = gs_ide1033 (v);
	      if ((pmove > 0) && (smove != 0) && (pmove <= rdiff))
		{
		  gs_ide1025[j] = pmove;
		}
	      else if ((pmove < 0) && (smove != 0) && (-pmove <= ldiff))
		{
		  gs_ide1025[j] = pmove;
		}
	      else if ((smove > 0) && (pmove != 0) && (smove <= rdiff))
		{
		  gs_ide1025[j] = smove;
		}
	      else if ((smove < 0) && (pmove != 0) && (-smove <= ldiff))
		{
		  gs_ide1025[j] = smove;
		}
	    }
	  else if (gs_ide1020 (v) && (((v)->pred) == NULL))
	    {
	      smove = gs_ide1035 (v);
	      if ((smove > 0) && (smove <= rdiff))
		{
		  gs_ide1025[j] = smove;
		}
	      else if ((smove < 0) && (-smove <= ldiff))
		{
		  gs_ide1025[j] = smove;
		}
	    }
	  else if (gs_ide1019 (v) && (((v)->succ) == NULL))
	    {
	      pmove = gs_ide1033 (v);
	      if ((pmove > 0) && (pmove <= rdiff))
		{
		  gs_ide1025[j] = pmove;
		}
	      else if ((pmove < 0) && (-pmove <= ldiff))
		{
		  gs_ide1025[j] = pmove;
		}
	    }
	  oldv = v;
	  li = ((li)->next);
	  j++;
	}
      changed += gs_ide1007 (i);
      changed += gs_ide1006 (i);
    }
#ifdef DRAWDEBUG
#ifdef NEVER
  PRINTF ("After correct priority nodes line %d\n", i);
  PRINTF ("Enter CR into the text window\n");
  step4_main ();
  debug_display_part ();
  fgetc (stdin);
#endif
#endif
  return (changed);
}

#ifdef ANSI_C
static void
gs_ide1048 (void)
#else
static void
gs_ide1048 ()
#endif
{
  int i, count;
  int changed;
  GNLIST li;;
  count = 0;
  changed = 1;
  while (changed)
    {
      count++;
      gs_wait_message ('S');
      if (count > max_straighttune)
	{
	  gs_wait_message ('t');
	  return;
	}
      if (G_timelimit > 0)
	if (test_timelimit (90))
	  {
	    gs_wait_message ('t');
	    return;
	  }
      changed = 0;
      for (i = 0; i <= maxdepth + 1; i++)
	{
	  li = ((layer[i]).succlist);
	  while (li)
	    {
	      if (gs_ide1018 (((li)->node)))
		{
		  changed += gs_ide1008 (((li)->node));
		}
	      li = ((li)->next);
	    }
	}
    }
}

#ifdef ANSI_C
static int
gs_ide1008 (GNODE v)
#else
static int
gs_ide1008 (v)
     GNODE v;
#endif
{
  GNODE w, sw, tw, minw, maxw;
  ADJEDGE a;
  int sx, tx, sminx, smaxx, diff;
  int possible, allzero, h2;
  int changed;;
  sminx = smaxx = sx = tx = ((v)->xloc) + ((v)->width) / 2;
  sw = tw = minw = maxw = v;
  w = v;
  while (w && (gs_ide1018 (w)))
    {
      ((layer[((w)->tiefe)]).actx) = ((w)->xloc);
      sw = w;
      sx = ((w)->xloc) + ((w)->width) / 2;
      if (sx < sminx)
	{
	  sminx = sx;
	  minw = w;
	}
      if (sx > smaxx)
	{
	  smaxx = sx;
	  maxw = w;
	}
      a = ((w)->pred);
      if (a)
	w = (((((a)->kante))->start));
      else
	w = NULL;
    }
  w = v;
  while (w && (gs_ide1018 (w)))
    {
      ((layer[((w)->tiefe)]).actx) = ((w)->xloc);
      tw = w;
      tx = ((w)->xloc) + ((w)->width) / 2;
      if (tx < sminx)
	{
	  sminx = tx;
	  minw = w;
	}
      if (tx > smaxx)
	{
	  smaxx = tx;
	  maxw = w;
	}
      a = ((w)->succ);
      if (a)
	w = (((((a)->kante))->end));
      else
	w = NULL;
    }
  if (sw == tw)
    return (0);
  possible = gs_ide1014 (sw, sx, 0);
  possible &= gs_ide1014 (sw, sx, 1);
  if (possible)
    {
      allzero = 1;
      w = sw;
      while (w && (gs_ide1018 (w)))
	{
	  diff = sx - ((w)->xloc) - ((w)->width) / 2;
	  h2 = ((w)->xloc) + diff;
	  if (diff < 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  else if (diff > 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  a = ((w)->pred);
	  if (a)
	    w = (((((a)->kante))->start));
	  else
	    w = NULL;
	}
      w = sw;
      while (w && (gs_ide1018 (w)))
	{
	  diff = sx - ((w)->xloc) - ((w)->width) / 2;
	  h2 = ((w)->xloc) + diff;
	  if (diff < 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  else if (diff > 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  a = ((w)->succ);
	  if (a)
	    w = (((((a)->kante))->end));
	  else
	    w = NULL;
	}
      return (1 - allzero);
    }
  possible = gs_ide1014 (tw, tx, 0);
  possible &= gs_ide1014 (tw, tx, 1);
  if (possible)
    {
      allzero = 1;
      w = tw;
      while (w && (gs_ide1018 (w)))
	{
	  diff = tx - ((w)->xloc) - ((w)->width) / 2;
	  h2 = ((w)->xloc) + diff;
	  if (diff < 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  else if (diff > 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  a = ((w)->pred);
	  if (a)
	    w = (((((a)->kante))->start));
	  else
	    w = NULL;
	}
      w = tw;
      while (w && (gs_ide1018 (w)))
	{
	  diff = tx - ((w)->xloc) - ((w)->width) / 2;
	  h2 = ((w)->xloc) + diff;
	  if (diff < 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  else if (diff > 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  a = ((w)->succ);
	  if (a)
	    w = (((((a)->kante))->end));
	  else
	    w = NULL;
	}
      return (1 - allzero);
    }
  possible = gs_ide1014 (minw, sminx, 0);
  possible &= gs_ide1014 (minw, sminx, 1);
  if (possible)
    {
      allzero = 1;
      w = minw;
      while (w && (gs_ide1018 (w)))
	{
	  diff = sminx - ((w)->xloc) - ((w)->width) / 2;
	  h2 = ((w)->xloc) + diff;
	  if (diff < 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  else if (diff > 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  a = ((w)->pred);
	  if (a)
	    w = (((((a)->kante))->start));
	  else
	    w = NULL;
	}
      w = minw;
      while (w && (gs_ide1018 (w)))
	{
	  diff = sminx - ((w)->xloc) - ((w)->width) / 2;
	  h2 = ((w)->xloc) + diff;
	  if (diff < 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  else if (diff > 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  a = ((w)->succ);
	  if (a)
	    w = (((((a)->kante))->end));
	  else
	    w = NULL;
	}
      return (1 - allzero);
    }
  possible = gs_ide1014 (maxw, smaxx, 0);
  possible &= gs_ide1014 (maxw, smaxx, 1);
  if (possible)
    {
      allzero = 1;
      w = maxw;
      while (w && (gs_ide1018 (w)))
	{
	  diff = smaxx - ((w)->xloc) - ((w)->width) / 2;
	  h2 = ((w)->xloc) + diff;
	  if (diff < 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  else if (diff > 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  a = ((w)->pred);
	  if (a)
	    w = (((((a)->kante))->start));
	  else
	    w = NULL;
	}
      w = maxw;
      while (w && (gs_ide1018 (w)))
	{
	  diff = smaxx - ((w)->xloc) - ((w)->width) / 2;
	  h2 = ((w)->xloc) + diff;
	  if (diff < 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  else if (diff > 0)
	    {
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      ((w)->xloc) = h2;
	      allzero = 0;
	    }
	  a = ((w)->succ);
	  if (a)
	    w = (((((a)->kante))->end));
	  else
	    w = NULL;
	}
      return (1 - allzero);
    }
  gs_ide1049 (minw, sminx, 0);
  gs_ide1049 (minw, sminx, 1);
  gs_ide1049 (maxw, smaxx, 0);
  gs_ide1049 (maxw, smaxx, 1);
  gs_ide1049 (tw, tx, 0);
  gs_ide1049 (tw, tx, 1);
  gs_ide1049 (sw, sx, 0);
  gs_ide1049 (sw, sx, 1);
  changed = 0;
  w = v;
  while (w && (gs_ide1018 (w)))
    {
      if (((layer[((w)->tiefe)]).actx) != ((w)->xloc))
	changed = 1;
      a = ((w)->pred);
      if (a)
	w = (((((a)->kante))->start));
      else
	w = NULL;
    }
  w = v;
  while (w && (gs_ide1018 (w)))
    {
      if (((layer[((w)->tiefe)]).actx) != ((w)->xloc))
	changed = 1;
      a = ((w)->succ);
      if (a)
	w = (((((a)->kante))->end));
      else
	w = NULL;
    }
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1014 (GNODE sw, int sxpos, int dir)
#else
static int
gs_ide1014 (sw, sxpos, dir)
     GNODE sw;
     int sxpos;
     int dir;
#endif
{
  GNODE w, gs_ide1030;
  GNLIST li;
  ADJEDGE a;
  int diff;
  int sxpos_possible, h1, h2;;
  sxpos_possible = 1;
  w = sw;
  while (w && (gs_ide1018 (w)))
    {
      if (!sxpos_possible)
	break;
      diff = sxpos - ((w)->xloc) - ((w)->width) / 2;
      h2 = ((w)->xloc) + diff;
      if (diff < 0)
	li = ((layer[((w)->tiefe)]).predlist);
      else
	li = ((layer[((w)->tiefe)]).succlist);
      while (li)
	{
	  if (((li)->node) == w)
	    break;
	  li = ((li)->next);
	}
      assert ((li));
      li = ((li)->next);
      if (diff < 0)
	{
	  if (li)
	    {
	      gs_ide1030 = ((li)->node);
	      if ((((w)->width) == 0) || (((gs_ide1030)->width) == 0))
		h1 = ((gs_ide1030)->xloc) + ((gs_ide1030)->width) + G_dspace;
	      else
		h1 = ((gs_ide1030)->xloc) + ((gs_ide1030)->width) + G_xspace;
	      if (((w)->width) == 0)
		h2 = dxlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      if (h2 < h1)
		sxpos_possible = 0;
	    }
	}
      else if (diff > 0)
	{
	  if (li)
	    {
	      gs_ide1030 = ((li)->node);
	      if ((((w)->width) == 0) || (((gs_ide1030)->width) == 0))
		h1 = ((gs_ide1030)->xloc) - G_dspace - ((w)->width);
	      else
		h1 = ((gs_ide1030)->xloc) - G_xspace - ((w)->width);
	      if (((w)->width) == 0)
		h2 = dxralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      else
		h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	      if (h2 > h1)
		sxpos_possible = 0;
	    }
	}
      if (dir)
	{
	  a = ((w)->pred);
	  if (a)
	    w = (((((a)->kante))->start));
	  else
	    w = NULL;
	}
      else
	{
	  a = ((w)->succ);
	  if (a)
	    w = (((((a)->kante))->end));
	  else
	    w = NULL;
	}
    }
  return (sxpos_possible);
}

#ifdef ANSI_C
static void
gs_ide1049 (GNODE sw, int sxpos, int dir)
#else
static void
gs_ide1049 (sw, sxpos, dir)
     GNODE sw;
     int sxpos;
     int dir;
#endif
{
  GNODE w, gs_ide1030;
  GNLIST li;
  ADJEDGE a;
  int diff;
  int sxpos_possible, h1, h2;;
  sxpos_possible = 1;
  w = sw;
  while (w && (gs_ide1018 (w)))
    {
      if (!sxpos_possible)
	break;
      diff = sxpos - ((w)->xloc) - ((w)->width) / 2;
      h2 = ((w)->xloc) + diff;
      if (diff < 0)
	li = ((layer[((w)->tiefe)]).predlist);
      else
	li = ((layer[((w)->tiefe)]).succlist);
      while (li)
	{
	  if (((li)->node) == w)
	    break;
	  li = ((li)->next);
	}
      assert ((li));
      li = ((li)->next);
      if (diff < 0)
	{
	  if (((w)->width) == 0)
	    h2 = dxlalign (h2);
	  else
	    h2 = xlalign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	  if (!li)
	    {
	      ((w)->xloc) = h2;
	    }
	  else
	    {
	      gs_ide1030 = ((li)->node);
	      if ((((w)->width) == 0) || (((gs_ide1030)->width) == 0))
		h1 = ((gs_ide1030)->xloc) + ((gs_ide1030)->width) + G_dspace;
	      else
		h1 = ((gs_ide1030)->xloc) + ((gs_ide1030)->width) + G_xspace;
	      if (h2 >= h1)
		{
		  ((w)->xloc) = h2;
		}
	      else
		sxpos_possible = 0;
	    }
	}
      else if (diff > 0)
	{
	  if (((w)->width) == 0)
	    h2 = dxralign (h2);
	  else
	    h2 = xralign (h2 + ((w)->width) / 2) - ((w)->width) / 2;
	  if (!li)
	    {
	      ((w)->xloc) = h2;
	    }
	  else
	    {
	      gs_ide1030 = ((li)->node);
	      if ((((w)->width) == 0) || (((gs_ide1030)->width) == 0))
		h1 = ((gs_ide1030)->xloc) - G_dspace - ((w)->width);
	      else
		h1 = ((gs_ide1030)->xloc) - G_xspace - ((w)->width);
	      if (h2 <= h1)
		{
		  ((w)->xloc) = h2;
		}
	      else
		sxpos_possible = 0;
	    }
	}
      if (dir)
	{
	  a = ((w)->pred);
	  if (a)
	    w = (((((a)->kante))->start));
	  else
	    w = NULL;
	}
      else
	{
	  a = ((w)->succ);
	  if (a)
	    w = (((((a)->kante))->end));
	  else
	    w = NULL;
	}
    }
}

#ifdef ANSI_C
static void
gs_ide1021 (void)
#else
static void
gs_ide1021 ()
#endif
{
  int i, count;
  int weight, h;
  int second_try;;
  if (prio_phase == 1)
    return;
  weight = gs_ide1002 ();
  second_try = 2;
  count = 0;
  while (1)
    {
      if (count % 5 == 0)
	gs_wait_message ('c');
      count++;
      if (count >= max_centershifts)
	{
	  gs_wait_message ('t');
	  break;
	}
      if (G_timelimit > 0)
	if (test_timelimit (90))
	  {
	    gs_wait_message ('t');
	    break;
	  }
      for (i = 0; i <= maxdepth + 1; i++)
	gs_ide1000 (i);
      for (i = maxdepth + 1; i >= 0; i--)
	gs_ide1000 (i);
      h = gs_ide1002 ();
      drawdebug_show_graph ("centershift", count, h, weight);
      if (count >= min_centershifts)
	{
	  if (h < weight)
	    {
	      weight = h;
	      second_try = 2;
	    }
	  else if (h == weight)
	    {
	      second_try--;
	      if (second_try <= 0)
		break;
	    }
	  else
	    break;
	}
    }
}

#ifdef ANSI_C
static int
gs_ide1002 (void)
#else
static int
gs_ide1002 ()
#endif
{
  int i;
  GNLIST li;
  ADJEDGE a;
  int weight, h;;
  weight = 0;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((layer[i]).succlist);
      while (li)
	{
	  h = 0;
	  a = ((((li)->node))->succ);
	  while (a)
	    {
	      h += gs_ide1012 (((a)->kante));
	      a = ((a)->next);
	    }
	  a = ((((li)->node))->pred);
	  while (a)
	    {
	      h += gs_ide1011 (((a)->kante));
	      a = ((a)->next);
	    }
	  if (h < 0)
	    h = -h;
	  weight += h;
	  li = ((li)->next);
	}
    }
  return (weight);
}

#ifdef ANSI_C
static int
gs_ide1000 (int i)
#else
static int
gs_ide1000 (i)
     int i;
#endif
{
  GNLIST li, li1;
  int j;
  int changed;
  int dir;
  GNODE lnode, node, rnode;;
  gs_ide1040 (i);
  li = ((layer[i]).succlist);
  j = 0;
  dir = 0;
  lnode = NULL;
  while (li)
    {
      node = ((li)->node);
      if (((node)->width) == 0)
	{
	  li1 = ((li)->next);
	  rnode = NULL;
	  while (li1)
	    {
	      if (((((li1)->node))->width) != 0)
		{
		  rnode = ((li1)->node);
		  break;
		}
	      li1 = ((li1)->next);
	    }
	  gs_ide1025[j] = gs_ide1031 (node, lnode, rnode);
	}
      else
	{
	  gs_ide1025[j] = gs_ide1030 (node);
	  lnode = node;
	}
      dir += gs_ide1025[j];
      gs_ide1026[j++] = 1;
      li = ((li)->next);
    }
  if (dir >= 0)
    {
      changed = gs_ide1007 (i);
      changed += gs_ide1006 (i);
    }
  else
    {
      changed = gs_ide1006 (i);
      changed += gs_ide1007 (i);
    }
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1006 (int i)
#else
static int
gs_ide1006 (i)
     int i;
#endif
{
  int j;
  int diff;
  int oldx;
  GNODE node, lnode;
  int changed;
  assert ((i >= 0) && (i <= maxdepth + 1));
  lnode = NULL;
  for (j = 0; j < ((layer[i]).anz); j++)
    {
      if (gs_ide1026[j])
	diff = gs_ide1025[j] / gs_ide1026[j];
      else
	diff = 0;
      node = gs_ide1047[j];
      if (diff < 0)
	{
	  oldx = ((node)->xloc);
	  ((node)->xloc) = oldx + diff;
	  if (lnode)
	    {
	      if ((((node)->width) == 0) || (((lnode)->width) == 0))
		{
		  if (((node)->xloc) <
		      ((lnode)->xloc) + ((lnode)->width) + G_dspace)
		    ((node)->xloc) =
		      ((lnode)->xloc) + ((lnode)->width) + G_dspace;
		}
	      else
		{
		  if (((node)->xloc) <
		      ((lnode)->xloc) + ((lnode)->width) + G_xspace)
		    ((node)->xloc) =
		      ((lnode)->xloc) + ((lnode)->width) + G_xspace;
		}
	    }
	  if ((((node)->width) == 0) && (((node)->height) == 0))
	    ((node)->xloc) =
	      dxralign (((node)->xloc) + ((node)->width) / 2) -
	      ((node)->width) / 2;
	  else
	    ((node)->xloc) =
	      xralign (((node)->xloc) + ((node)->width) / 2) -
	      ((node)->width) / 2;
	  if (((node)->xloc) < oldx)
	    changed = 1;
	}
      lnode = node;
    }
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1007 (int i)
#else
static int
gs_ide1007 (i)
     int i;
#endif
{
  int j;
  int diff;
  int oldx;
  GNODE node, rnode;
  int changed;
  assert ((i >= 0) && (i <= maxdepth + 1));
  rnode = NULL;
  changed = 0;
  for (j = ((layer[i]).anz) - 1; j >= 0; j--)
    {
      if (gs_ide1026[j])
	diff = gs_ide1025[j] / gs_ide1026[j];
      else
	diff = 0;
      node = gs_ide1047[j];
      if (diff > 0)
	{
	  oldx = ((node)->xloc);
	  ((node)->xloc) = oldx + diff;
	  if (rnode)
	    {
	      if ((((node)->width) == 0) || (((rnode)->width) == 0))
		{
		  if (((node)->xloc) + ((node)->width) + G_dspace >
		      ((rnode)->xloc))
		    ((node)->xloc) =
		      ((rnode)->xloc) - ((node)->width) - G_dspace;
		}
	      else
		{
		  if (((node)->xloc) + ((node)->width) + G_xspace >
		      ((rnode)->xloc))
		    ((node)->xloc) =
		      ((rnode)->xloc) - ((node)->width) - G_xspace;
		}
	    }
	  if ((((node)->width) == 0) && (((node)->height) == 0))
	    ((node)->xloc) =
	      dxlalign (((node)->xloc) + ((node)->width) / 2) -
	      ((node)->width) / 2;
	  else
	    ((node)->xloc) =
	      xlalign (((node)->xloc) + ((node)->width) / 2) -
	      ((node)->width) / 2;
	  if (((node)->xloc) > oldx)
	    changed = 1;
	}
      rnode = node;
    }
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1012 (GEDGE edge)
#else
static int
gs_ide1012 (edge)
     GEDGE edge;
#endif
{
  GNODE start, ende;
  int x1, x2;
  start = ((edge)->start);
  ende = ((edge)->end);
  x1 =
    ((ende)->xloc) +
    (((ende)->width) * ((edge)->weightp) / (((ende)->weightp) + 1));
  x2 =
    ((start)->xloc) +
    (((start)->width) * ((edge)->weights) / (((start)->weights) + 1));
  return (x1 - x2);
}

#ifdef ANSI_C
static int
gs_ide1011 (GEDGE edge)
#else
static int
gs_ide1011 (edge)
     GEDGE edge;
#endif
{
  GNODE start, ende;
  int x1, x2;
  ende = ((edge)->end);
  start = ((edge)->start);
  x1 =
    ((ende)->xloc) +
    (((ende)->width) * ((edge)->weightp) / (((ende)->weightp) + 1));
  x2 =
    ((start)->xloc) +
    (((start)->width) * ((edge)->weights) / (((start)->weights) + 1));
  return (x2 - x1);
}

#ifdef ANSI_C
static int
gs_ide1035 (GNODE node)
#else
static int
gs_ide1035 (node)
     GNODE node;
#endif
{
  int h;
  int weight;
  int nr_edges;
  ADJEDGE a;
  CONNECT c;
  a = ((node)->succ);
  weight = 0;
  nr_edges = 0;
  while (a)
    {
      h =
	(((((a)->kante))->priority) * layout_downfactor) *
	gs_ide1012 (((a)->kante));
      weight += h;
      nr_edges += (((((a)->kante))->priority) * layout_downfactor);
      a = ((a)->next);
    }
  c = ((node)->connection);
  if (c)
    {
      if (((c)->target))
	{
	  if (((((c)->target))->xloc) < ((node)->xloc))
	    weight -=
	      (((((c)->edge))->priority) * layout_nearfactor *
	       (((node)->xloc) -
		(((((c)->target))->xloc) + ((((c)->target))->width) +
		 G_xspace)));
	  else
	    weight +=
	      (((((c)->edge))->priority) * layout_nearfactor *
	       (((((c)->target))->xloc) -
		(((node)->xloc) + ((node)->width) + G_xspace)));
	  nr_edges += (((((c)->edge))->priority) * layout_nearfactor);
	}
      if (((c)->target2))
	{
	  if (((((c)->target2))->xloc) < ((node)->xloc))
	    weight -=
	      (((((c)->edge2))->priority) * layout_nearfactor *
	       (((node)->xloc) -
		(((((c)->target2))->xloc) + ((((c)->target2))->width) +
		 G_xspace)));
	  else
	    weight +=
	      (((((c)->edge2))->priority) * layout_nearfactor *
	       (((((c)->target2))->xloc) -
		(((node)->xloc) + ((node)->width) + G_xspace)));
	  nr_edges += (((((c)->edge2))->priority) * layout_nearfactor);
	}
    }
  if (nr_edges)
    return (weight / nr_edges);
  else
    return (0);
}

#ifdef ANSI_C
static int
gs_ide1033 (GNODE node)
#else
static int
gs_ide1033 (node)
     GNODE node;
#endif
{
  int h;
  int weight;
  int nr_edges;
  ADJEDGE a;
  CONNECT c;
  a = ((node)->pred);
  weight = 0;
  nr_edges = 0;
  while (a)
    {
      h =
	(((((a)->kante))->priority) * layout_upfactor) *
	gs_ide1011 (((a)->kante));
      weight += h;
      nr_edges += (((((a)->kante))->priority) * layout_upfactor);
      a = ((a)->next);
    }
  c = ((node)->connection);
  if (c)
    {
      if (((c)->target))
	{
	  if (((((c)->target))->xloc) < ((node)->xloc))
	    weight -=
	      (((((c)->edge))->priority) * layout_nearfactor *
	       (((node)->xloc) -
		(((((c)->target))->xloc) + ((((c)->target))->width) +
		 G_xspace)));
	  else
	    weight +=
	      (((((c)->edge))->priority) * layout_nearfactor *
	       (((((c)->target))->xloc) -
		(((node)->xloc) + ((node)->width) + G_xspace)));
	  nr_edges += (((((c)->edge))->priority) * layout_nearfactor);
	}
      if (((c)->target2))
	{
	  if (((((c)->target2))->xloc) < ((node)->xloc))
	    weight -=
	      (((((c)->edge2))->priority) * layout_nearfactor *
	       (((node)->xloc) -
		(((((c)->target2))->xloc) + ((((c)->target2))->width) +
		 G_xspace)));
	  else
	    weight +=
	      (((((c)->edge2))->priority) * layout_nearfactor *
	       (((((c)->target2))->xloc) -
		(((node)->xloc) + ((node)->width) + G_xspace)));
	  nr_edges += (((((c)->edge2))->priority) * layout_nearfactor);
	}
    }
  if (nr_edges)
    return (weight / nr_edges);
  else
    return (0);
}

#ifdef ANSI_C
static int
gs_ide1030 (GNODE node)
#else
static int
gs_ide1030 (node)
     GNODE node;
#endif
{
  int weight;
  int dx1, dx2, dy1, dy2;
  int nr_edges, p1, p2;
  GEDGE edge;
  GNODE v;
  ADJEDGE a;
  CONNECT c;
  weight = 0;
  nr_edges = 0;
  if ((layout_downfactor == 1) && (layout_upfactor == 1) && ((node)->succ)
      && (((((node)->succ))->next) == 0) && ((node)->pred)
      && (((((node)->pred))->next) == 0)
      && (((((((node)->succ))->kante))->priority) ==
	  ((((((node)->pred))->kante))->priority))
      && ((node)->connection) == NULL)
    {
      edge = ((((node)->pred))->kante);
      v = ((edge)->start);
      p1 =
	((node)->xloc) +
	(((node)->width) * ((edge)->weightp) / (((node)->weightp) + 1));
      p2 =
	((v)->xloc) +
	(((v)->width) * ((edge)->weights) / (((v)->weights) + 1));
      dx1 = p1 - p2;
      edge = ((((node)->succ))->kante);
      v = ((edge)->end);
      p1 =
	((node)->xloc) +
	(((node)->width) * ((edge)->weights) / (((node)->weights) + 1));
      p2 =
	((v)->xloc) +
	(((v)->width) * ((edge)->weightp) / (((v)->weightp) + 1));
      dx2 = p2 - p1;
      p1 = ((node)->yloc) + ((node)->height) / 2;
      p2 =
	(((((((((node)->pred))->kante))->start)))->yloc) +
	(((((((((node)->pred))->kante))->start)))->height) / 2;
      dy1 = p1 - p2;
      p2 =
	(((((((((node)->succ))->kante))->end)))->yloc) +
	(((((((((node)->succ))->kante))->end)))->height) / 2;
      dy2 = p2 - p1;
      weight = (dx2 * dy1 - dx1 * dy2) / (dy1 + dy2);
      return (weight);
    }
  a = ((node)->succ);
  while (a)
    {
      weight +=
	(gs_ide1012 (((a)->kante)) * ((((a)->kante))->priority) *
	 layout_downfactor);
      nr_edges += (((((a)->kante))->priority) * layout_downfactor);
      a = ((a)->next);
    }
  a = ((node)->pred);
  while (a)
    {
      weight +=
	(gs_ide1011 (((a)->kante)) * ((((a)->kante))->priority) *
	 layout_upfactor);
      nr_edges += (((((a)->kante))->priority) * layout_upfactor);
      a = ((a)->next);
    }
  c = ((node)->connection);
  if (c)
    {
      if (((c)->target))
	{
	  if (((((c)->target))->xloc) < ((node)->xloc))
	    weight -=
	      (((((c)->edge))->priority) * layout_nearfactor *
	       (((node)->xloc) -
		(((((c)->target))->xloc) + ((((c)->target))->width) +
		 G_xspace)));
	  else
	    weight +=
	      (((((c)->edge))->priority) * layout_nearfactor *
	       (((((c)->target))->xloc) -
		(((node)->xloc) + ((node)->width) + G_xspace)));
	  nr_edges += (((((c)->edge))->priority) * layout_nearfactor);
	}
      if (((c)->target2))
	{
	  if (((((c)->target2))->xloc) < ((node)->xloc))
	    weight -=
	      (((((c)->edge2))->priority) * layout_nearfactor *
	       (((node)->xloc) -
		(((((c)->target2))->xloc) + ((((c)->target2))->width) +
		 G_xspace)));
	  else
	    weight +=
	      (((((c)->edge2))->priority) * layout_nearfactor *
	       (((((c)->target2))->xloc) -
		(((node)->xloc) + ((node)->width) + G_xspace)));
	  nr_edges += (((((c)->edge2))->priority) * layout_nearfactor);
	}
    }
  if (nr_edges)
    return (weight / nr_edges);
  else
    return (0);
}

#ifdef ANSI_C
static int
gs_ide1031 (GNODE node, GNODE lnode, GNODE rnode)
#else
static int
gs_ide1031 (node, lnode, rnode)
     GNODE node, lnode, rnode;
#endif
{
  GNODE pred, succ;
  int ax, ay, mx, my, bx, by, kx, h, dist;
  int act_nw;
  act_nw = gs_ide1030 (node);
  if (((node)->pred))
    pred = (((((((node)->pred))->kante))->start));
  else
    return (act_nw);
  if (((node)->succ))
    succ = (((((((node)->succ))->kante))->end));
  else
    return (act_nw);
  ax = ((pred)->xloc);
  ay = ((pred)->yloc);
  mx = ((node)->xloc);
  my = ((node)->yloc);
  bx = ((succ)->xloc);
  by = ((succ)->yloc);
  if ((ax < mx) && (bx < mx))
    return (act_nw);
  if ((ax > mx) && (bx > mx))
    return (act_nw);
  if (ax < bx)
    {
      if (rnode && act_nw > 0)
	{
	  kx = ((rnode)->xloc);
	  h = ((rnode)->yloc) + ((rnode)->height) - my;
	  if (h - by + my == 0)
	    return (act_nw);
	  dist = (bx * h - kx * (by - my)) / (h - by + my) - mx;
	  if (dist < act_nw)
	    act_nw = dist;
	}
      else if (lnode && act_nw < 0)
	{
	  kx = ((lnode)->xloc) + ((lnode)->width);
	  h = ((lnode)->yloc) - my;
	  if (h - ay + my == 0)
	    return (act_nw);
	  dist = (ax * h - kx * (ay - my)) / (h - ay + my) - mx;
	  if (dist > act_nw)
	    act_nw = dist;
	}
    }
  else if (ax > bx)
    {
      if (lnode && act_nw < 0)
	{
	  kx = ((lnode)->xloc) + ((lnode)->width);
	  h = ((lnode)->yloc) + ((lnode)->height) - my;
	  if (h - by + my == 0)
	    return (act_nw);
	  dist = (bx * h - kx * (by - my)) / (h - by + my) - mx;
	  if (dist > act_nw)
	    act_nw = dist;
	}
      else if (rnode && act_nw > 0)
	{
	  kx = ((rnode)->xloc);
	  h = ((rnode)->yloc) - my;
	  if (h - ay + my == 0)
	    return (act_nw);
	  dist = (ax * h - kx * (ay - my)) / (h - ay + my) - mx;
	  if (dist < act_nw)
	    act_nw = dist;
	}
    }
  return (act_nw);
}

#ifdef ANSI_C
static void
gs_ide1040 (int i)
#else
static void
gs_ide1040 (i)
     int i;
#endif
{
  int j;
  GNLIST hn;;
  j = 0;
  hn = ((layer[i]).succlist);
  while (hn)
    {
      assert ((((((hn)->node))->position) == j));
      gs_ide1047[j] = ((hn)->node);
      gs_ide1025[j] = 0;
      gs_ide1026[j++] = 0;
      hn = ((hn)->next);
    }
  assert (j == ((layer[i]).anz));
}

#ifdef DRAWDEBUG
#ifdef ANSI_C
static void
drawdebug_show_graph (char *m, int i, int j, int k)
#else
static void
drawdebug_show_graph (m, i, j, k)
     char *m;
     int i, j, k;
#endif
{
  PRINTF ("%s %d (%d %d)\n", m, i, j, k);
  PRINTF ("Enter CR into the text window\n");
  gs_ide1043 ();
  step4_main ();
  debug_display_part ();
  fgetc (stdin);
}
#endif
