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
#include <math.h>
#include "globals.h"
#include "alloc.h"
#include "main.h"
#include "options.h"
#include "folding.h"
#include "steps.h"
#include "timelim.h"
#include "timing.h"
#undef CHECK_CROSSING
#ifndef ANSI_C
#ifndef const
#define const
#endif
#endif
#ifdef OWN_QUICKSORT
#define quicksort_sort_array(x) { if (x) { gs_ide559  = 211; gs_ide549 (0,x-1);} }
#else
#ifdef ANSI_C
#define quicksort_sort_array(x) { if (x) qsort(gs_ide577 ,x, 	   sizeof(GNODE),(int (*) (const void *, const void *))gs_ide514 ); }
#else
#define quicksort_sort_array(x) { if (x) qsort(gs_ide577 ,x, 	   sizeof(GNODE),gs_ide514 ); }
#endif
#endif
#ifdef ANSI_C
#define quicksort_save_array(x) { if (x) qsort(gs_ide569 ,x, 	   sizeof(GNODE),(int (*) (const void *, const void *))gs_ide515 ); }
#else
#define quicksort_save_array(x) { if (x) qsort(gs_ide569 ,x, 	   sizeof(GNODE),gs_ide515 ); }
#endif
static void gs_ide519 _PP ((void));
static void gs_ide581 _PP ((void));
static void gs_ide582 _PP ((void));
static void gs_ide556 _PP ((void));
static void gs_ide546 _PP ((GNODE node, int i));
static int gs_ide528 _PP ((void));
static void gs_ide511 _PP ((void));
static int gs_ide536 _PP ((int level));
static void gs_ide527 _PP ((GNODE v));
static void gs_ide526 _PP ((GNODE v));
static void gs_ide507 _PP ((GNODE n));
static void gs_ide506 _PP ((GNODE n));
static void gs_ide522 _PP ((DLLIST x));
static void gs_ide521 _PP ((DLLIST x));
static void gs_ide542 _PP ((int nearedges));
static void gs_ide557 _PP ((int level));
static int gs_ide540 _PP ((int level, int nearedges));
static int gs_ide525 _PP ((GNODE C, GNODE D));
static void gs_ide558 _PP ((void));
static int gs_ide535 _PP ((GNODE v));
static int gs_ide513 _PP ((GNODE v1, GNODE v2, int dir));
static void gs_ide523 _PP ((GNODE v1, GNODE v2, int dir));
static void gs_ide543 _PP ((void));
static void gs_ide531 _PP ((void));
static void gs_ide510 _PP ((void));
static void gs_ide500 _PP ((void));
static void gs_ide501 _PP ((void));
static void gs_ide502 _PP ((void));
static void gs_ide503 _PP ((void));
static int gs_ide561 _PP ((int i));
static int gs_ide564 _PP ((int i));
static void gs_ide562 _PP ((int level));
static void gs_ide563 _PP ((int level));
static int gs_ide520 _PP ((int siz));
static float gs_ide578 _PP ((GNODE node));
static float gs_ide554 _PP ((GNODE node));
static float gs_ide579 _PP ((GNODE node));
static float gs_ide555 _PP ((GNODE node));
static void gs_ide541 _PP ((int i, int dir));
static void gs_ide509 _PP ((int i));
static void gs_ide570 _PP ((int i));
static void gs_ide565 _PP ((int i));
static void gs_ide508 _PP ((int i));
#if 0
static int gs_ide514 _PP ((const GNODE * a, const GNODE * b));
#endif
static int gs_ide515 _PP ((const GNODE * a, const GNODE * b));
static void gs_ide518 _PP ((DEPTH * l1, DEPTH * l2));
static void gs_ide532 _PP ((void));
static void gs_ide533 _PP ((int level, GNODE node));
static void gs_ide512 _PP ((int level, GNODE node));
static void gs_ide537 _PP ((GNODE v, GNODE w));
static void gs_ide566 _PP ((GNODE v, GNODE w));
static void gs_ide560 _PP ((void));
static void gs_ide576 _PP ((GNODE v));
static int gs_ide516 _PP ((const GEDGE * a, const GEDGE * b));
static int gs_ide517 _PP ((const GEDGE * a, const GEDGE * b));
#ifdef OWN_QUICKSORT
static void gs_ide549 _PP ((int l, int r));
static int gs_ide559 = 211;
#endif
static DEPTH *gs_ide580 = NULL;
static int gs_ide574 = 0;
int max_nodes_per_layer;
int nr_crossings;
static GNODE *gs_ide504 = NULL;
static GEDGE *gs_ide505 = NULL;
static int gs_ide572 = 0;
static GNODE *gs_ide577 = NULL;
static int gs_ide573 = 0;
static GNODE *gs_ide569 = NULL;
static int gs_ide550;
#ifdef ANSI_C
void
step2_main (void)
#else
void
step2_main ()
#endif
{
  int i;
  int old_nr_crossings;
  start_time ();;
  assert ((layer));
  gs_ide519 ();
  if (max_nodes_per_layer + 2 > gs_ide573)
    {
      if (gs_ide577)
	free (gs_ide577);
      if (gs_ide569)
	free (gs_ide569);
      gs_ide577 =
	(GNODE *) malloc ((max_nodes_per_layer + 2) * sizeof (GNODE));
      gs_ide569 =
	(GNODE *) malloc ((max_nodes_per_layer + 2) * sizeof (GNODE));
      if ((!gs_ide577) || (!gs_ide569))
	Fatal_error ("memory exhausted", "");
      gs_ide573 = max_nodes_per_layer + 2;
#ifdef DEBUG
      PRINTF ("Sizeof tables `sort_array',`save_array': %ld Bytes\n",
	      (max_nodes_per_layer + 2) * sizeof (GNODE));
#endif
    }
  i = (maxindeg > maxoutdeg ? maxindeg : maxoutdeg);
  if (i + 2 > gs_ide572)
    {
      if (gs_ide504)
	free (gs_ide504);
      if (gs_ide505)
	free (gs_ide505);
      gs_ide504 = (GNODE *) malloc ((i + 2) * sizeof (GNODE));
      gs_ide505 = (GEDGE *) malloc ((i + 2) * sizeof (GEDGE));
      if ((!gs_ide504) || (!gs_ide505))
	Fatal_error ("memory exhausted", "");
      gs_ide572 = i + 2;
#ifdef DEBUG
      PRINTF ("Sizeof table `adjarray[12]': %ld Bytes\n",
	      (i + 2) * sizeof (GNODE));
#endif
    }
  gs_ide581 ();
  gs_ide556 ();
  gs_ide582 ();
  gs_ide511 ();
  for (i = 0; i <= maxdepth + 1; i++)
    ((layer[i]).cross) = ((gs_ide580[i]).cross);
  nr_crossings = gs_ide528 ();
#ifdef CHECK_CROSSING
  PRINTF ("Start: nr_crossings %d \n", nr_crossings);
#endif
  gs_ide531 ();
  old_nr_crossings = -1;
  gs_ide550 = 0;
  while (nr_crossings != old_nr_crossings)
    {
      if (gs_ide550 >= max_baryiterations)
	{
	  gs_wait_message ('t');
	  break;
	}
      if (G_timelimit > 0)
	if (test_timelimit (60))
	  {
	    gs_wait_message ('t');
	    break;
	  }
      old_nr_crossings = nr_crossings;
      gs_ide510 ();
      gs_ide550++;
      if (gs_ide550 < min_baryiterations)
	nr_crossings = old_nr_crossings + 1;
    }
#ifdef CHECK_CROSSING
  PRINTF ("After barycentering: nr_crossings %d \n", nr_crossings);
#endif
  old_nr_crossings = nr_crossings;
  if (old_nr_crossings > 0)
    gs_ide542 (0);
  gs_wait_message ('b');
  gs_ide532 ();
  gs_wait_message ('b');
  if (old_nr_crossings > 0)
    gs_ide542 (1);
#ifdef CHECK_CROSSING
  gs_ide511 ();
  nr_crossings = gs_ide528 ();
  PRINTF ("After local optimization: nr_crossings %d \n", nr_crossings);
#endif
  if (local_unwind && (old_nr_crossings > 0))
    gs_ide543 ();
  gs_ide511 ();
  nr_crossings = gs_ide528 ();
#ifdef CHECK_CROSSING
  PRINTF ("Final: nr_crossings %d \n", nr_crossings);
#endif
  gs_ide518 (layer, gs_ide580);
  gs_ide560 ();
  stop_time ("step2_main");
}

#ifdef ANSI_C
static void
gs_ide519 (void)
#else
static void
gs_ide519 ()
#endif
{
  int i, j;
  GNLIST h1, h2;
  if (maxdepth + 2 > gs_ide574)
    {
      if (gs_ide580)
	free (gs_ide580);
      gs_ide580 =
	(DEPTH *) malloc ((maxdepth + 2) * sizeof (struct depth_entry));
      if (!gs_ide580)
	Fatal_error ("memory exhausted", "");
      gs_ide574 = maxdepth + 2;
#ifdef DEBUG
      PRINTF ("Sizeof table `tmp_layer': %ld Bytes\n",
	      (maxdepth + 2) * sizeof (struct depth_entry));
#endif
    }
  max_nodes_per_layer = 0;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      ((gs_ide580[i]).cross) = 0;
      ((gs_ide580[i]).predlist) = NULL;
      ((gs_ide580[i]).succlist) = NULL;
      j = 0;
      h1 = ((layer[i]).succlist);
      while (h1)
	{
	  j++;
	  h1 = ((h1)->next);
	}
      if (j > max_nodes_per_layer)
	max_nodes_per_layer = j;
      j = 0;
      h1 = ((layer[i]).predlist);
      while (h1)
	{
	  j++;
	  h2 = tmpnodelist_alloc ();
	  ((h2)->next) = ((gs_ide580[i]).succlist);
	  ((gs_ide580[i]).succlist) = h2;
	  ((h2)->node) = ((h1)->node);
	  h1 = ((h1)->next);
	}
      ((gs_ide580[i]).anz) = ((layer[i]).anz) = j;
      ((gs_ide580[i]).resort_necessary) = ((layer[i]).resort_necessary);
      if (j > max_nodes_per_layer)
	max_nodes_per_layer = j;
      ((layer[i]).succlist) = NULL;
      h1 = ((layer[i]).predlist);
      while (h1)
	{
	  h2 = tmpnodelist_alloc ();
	  ((h2)->next) = ((layer[i]).succlist);
	  ((layer[i]).succlist) = h2;
	  ((h2)->node) = ((h1)->node);
	  h1 = ((h1)->next);
	}
    }
}

#ifdef ANSI_C
static void
gs_ide581 (void)
#else
static void
gs_ide581 ()
#endif
{
  GNLIST li;
  int i, prio;
  GNODE node;
  ADJEDGE a;
  GEDGE e;
  double maxbary;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((gs_ide580[i]).succlist);
      while (li)
	{
	  assert ((((((li)->node))->markiert) == 0));
	  a = ((((li)->node))->succ);
	  while (a)
	    {
	      ((((a)->kante))->weights) = 0;
	      a = ((a)->next);
	    }
	  li = ((li)->next);
	}
    }
  for (i = 0; i <= maxdepth + 1; i++)
    {
      for (prio = max_eprio; prio >= 0; prio--)
	{
	  li = ((gs_ide580[i]).succlist);
	  while (li)
	    {
	      a = ((((li)->node))->succ);
	      while (a)
		{
		  e = ((a)->kante);
		  if (((e)->priority) == prio)
		    {
		      node = (((((a)->kante))->end));
		      if (!((node)->markiert))
			{
			  ((node)->markiert) = 1;
			  ((e)->weights) = 1;
			}
		    }
		  a = ((a)->next);
		}
	      li = ((li)->next);
	    }
	}
    }
  for (i = 1; i <= maxdepth + 1; i++)
    {
      gs_ide541 (i, 'd');
      maxbary = 0.1;
      li = ((gs_ide580[i]).succlist);
      while (li)
	{
	  node = ((li)->node);
	  ((node)->bary) = gs_ide554 (node);
	  if (((node)->bary) > maxbary)
	    maxbary = ((node)->bary);
	  li = ((li)->next);
	}
      maxbary = maxbary + 1.0;
      li = ((gs_ide580[i - 1]).succlist);
      while (li)
	{
	  a = ((((li)->node))->succ);
	  while (a)
	    {
	      e = ((a)->kante);
	      if (((e)->weights))
		{
		  node = (((((a)->kante))->end));
		  ((node)->bary) =
		    (double) (((((li)->node))->position)) +
		    ((node)->bary) / maxbary;
		}
	      a = ((a)->next);
	    } li = ((li)->next);
	} quicksort_sort_array (((layer[i]).anz));
      gs_ide509 (i);
  } for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((gs_ide580[i]).succlist);
      while (li)
	{
	  ((((li)->node))->markiert) = 0;
	  li = ((li)->next);
	}
    }
  gs_ide518 (layer, gs_ide580);
}
static int gs_ide547;
#ifdef ANSI_C
static void
gs_ide556 (void)
#else
static void
gs_ide556 ()
#endif
{
  GNLIST li;
  int i, reorder_necessary;
  GNODE node;;
  gs_ide547 = 0;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((gs_ide580[i]).succlist);
      reorder_necessary = 0;
      while (li)
	{
	  node = ((li)->node);
	  if (((node)->nhorder) >= 0)
	    reorder_necessary = 1;
	  if (((node)->nhorder) > gs_ide547)
	    gs_ide547 = ((node)->nhorder);
	  li = ((li)->next);
	}
      ((gs_ide580[i]).resort_necessary) = ((layer[i]).resort_necessary)
	= reorder_necessary;
    }
}

#ifdef ANSI_C
static void
gs_ide582 (void)
#else
static void
gs_ide582 ()
#endif
{
  GNLIST li;
  int i, j;
  int part_is_missing;
  GNODE node;
  int act_graph;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((gs_ide580[i]).succlist);
      while (li)
	{
	  ((((li)->node))->bary) = 0;
	  li = ((li)->next);
	}
    }
  act_graph = 1;
  part_is_missing = 1;
  while (part_is_missing)
    {
      gs_wait_message ('u');
      part_is_missing = 0;
      node = (GNODE) 0;
      act_graph++;
      for (i = 0; i <= maxdepth + 1; i++)
	{
	  li = ((layer[i]).succlist);
	  while (li)
	    {
	      if (((((li)->node))->bary) == 0)
		{
		  node = ((li)->node);
		  break;
		}
	      li = ((li)->next);
	    }
	  if (node)
	    break;
	}
      if (node)
	{
	  assert ((((node)->bary) == 0));
	  part_is_missing = 1;
	  gs_ide546 (node, act_graph);
	}
    }
  for (i = 0; i <= maxdepth + 1; i++)
    {
      li = ((gs_ide580[i]).succlist);
      j = 0;
      while (li)
	{
	  gs_ide577[j++] = ((li)->node);
	  li = ((li)->next);
	}
      quicksort_sort_array (((layer[i]).anz));
      gs_ide509 (i);
    }
  gs_ide518 (layer, gs_ide580);
}

#ifdef ANSI_C
static void
gs_ide546 (GNODE node, int i)
#else
static void
gs_ide546 (node, i)
     GNODE node;
     int i;
#endif
{
  ADJEDGE e;;
  if (((node)->bary))
    return;

  if (((node)->nhorder) >= 0)
    {
      ((node)->bary) =
	(float) i *(float) (gs_ide547 + 1) + (float) ((node)->nhorder);
    }
  else
    {
      ((node)->bary) = (float) i *(float) (gs_ide547 + 1);
    }
  e = ((node)->succ);
  while (e)
    {
      gs_ide546 ((((((e)->kante))->end)), i);
      e = ((e)->next);
    }
  e = ((node)->pred);
  while (e)
    {
      gs_ide546 ((((((e)->kante))->start)), i);
      e = ((e)->next);
    }
}

#ifdef ANSI_C
static int
gs_ide528 (void)
#else
static int
gs_ide528 ()
#endif
{
  int i;
  int sumC;;
  assert ((gs_ide580));
  sumC = 0;
  for (i = 0; i <= maxdepth; i++)
    sumC += ((gs_ide580[i]).cross);
  assert ((((gs_ide580[maxdepth + 1]).cross) == 0));
  return (sumC);
}

#ifdef ANSI_C
static void
gs_ide511 (void)
#else
static void
gs_ide511 ()
#endif
{
  int i;;
  assert ((gs_ide580));
  for (i = 0; i <= maxdepth; i++)
    ((gs_ide580[i]).cross) = gs_ide536 (i);
}
static int gs_ide571;
static int gs_ide575;
static DLLIST gs_ide544 = NULL;
static DLLIST gs_ide545 = NULL;
static DLLIST gs_ide584 = NULL;
static DLLIST gs_ide585 = NULL;
static int gs_ide551;
#ifdef ANSI_C
static int
gs_ide536 (int level)
#else
static int
gs_ide536 (level)
     int level;
#endif
{
  GNLIST vl1, vl2;
  ADJEDGE a;
  int i;;
  assert ((gs_ide580));
  assert ((gs_ide544 == NULL));
  assert ((gs_ide584 == NULL));
  assert ((level >= 0));
  assert ((level <= maxdepth));
  vl1 = ((gs_ide580[level]).succlist);
  while (vl1)
    {
      ((((vl1)->node))->tmpadj) = ((((vl1)->node))->succ);
      vl1 = ((vl1)->next);
    }
  vl2 = ((gs_ide580[level + 1]).succlist);
  while (vl2)
    {
      ((((vl2)->node))->tmpadj) = ((((vl2)->node))->pred);
      vl2 = ((vl2)->next);
    }
  i = 1;
  vl1 = ((gs_ide580[level]).succlist);
  while (vl1)
    {
      ((((vl1)->node))->Vpointer) = NULL;
      ((((vl1)->node))->position) = i;
      a = ((((vl1)->node))->succ);
      while (a)
	{
	  assert (((((((((a)->kante))->end)))->tmpadj)));
	  (((((((((a)->kante))->end)))->tmpadj))->kante) = ((a)->kante);
	  (((((((a)->kante))->end)))->tmpadj) = (((((((((a)->kante))->
						      end)))->tmpadj))->next);
	  a = ((a)->next);
	}
      vl1 = ((vl1)->next);
      i = i + 2;
    }
  i = 2;
  vl2 = ((gs_ide580[level + 1]).succlist);
  while (vl2)
    {
      ((((vl2)->node))->Vpointer) = NULL;
      ((((vl2)->node))->position) = i;
      a = ((((vl2)->node))->pred);
      while (a)
	{
	  assert (((((((((a)->kante))->start)))->tmpadj)));
	  (((((((((a)->kante))->start)))->tmpadj))->kante) = ((a)->kante);
	  (((((((a)->kante))->start)))->tmpadj)
	    = (((((((((a)->kante))->start)))->tmpadj))->next);
	  a = ((a)->next);
	}
      vl2 = ((vl2)->next);
      i = i + 2;
    }
  gs_ide551 = 0;
  gs_ide575 = gs_ide571 = 0;
  gs_ide544 = gs_ide545 = NULL;
  gs_ide584 = gs_ide585 = NULL;
  vl1 = ((gs_ide580[level]).succlist);
  vl2 = ((gs_ide580[level + 1]).succlist);
  while ((vl1) || (vl2))
    {
      if (vl1)
	{
	  gs_ide527 (((vl1)->node));
	  vl1 = ((vl1)->next);
	}
      if (vl2)
	{
	  gs_ide526 (((vl2)->node));
	  vl2 = ((vl2)->next);
	}
    }
  assert ((gs_ide575 == 0));
  assert ((gs_ide571 == 0));
  assert ((gs_ide544 == NULL));
  assert ((gs_ide584 == NULL));
  return (gs_ide551);
}

#ifdef ANSI_C
static void
gs_ide527 (GNODE v)
#else
static void
gs_ide527 (v)
     GNODE v;
#endif
{
  ADJEDGE a;
  DLLIST n, m;
  int k1;
  int k2;
  int k3;;
  assert ((v));
  k1 = k2 = k3 = 0;
  if (((v)->Vpointer))
    {
      n = gs_ide584;
      while (n)
	{
	  m = ((n)->succ);
	  if (((n)->node) == v)
	    {
	      k1++;
	      k3 += k2;
	      gs_ide522 (n);
	    }
	  else
	    k2++;
	  if (n == ((v)->Vpointer))
	    break;
	  n = m;
	}
      gs_ide551 += (k1 * gs_ide571 + k3);
    }
  a = ((v)->succ);
  while (a)
    {
      if ((((((((a)->kante))->end)))->position) > ((v)->position))
	{
	  gs_ide506 ((((((a)->kante))->end)));
	  (((((((a)->kante))->end)))->Vpointer) = gs_ide545;
	}
      a = ((a)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide526 (GNODE v)
#else
static void
gs_ide526 (v)
     GNODE v;
#endif
{
  ADJEDGE a;
  DLLIST n, m;
  int k1;
  int k2;
  int k3;;
  assert ((v));
  k1 = k2 = k3 = 0;
  if (((v)->Vpointer))
    {
      n = gs_ide544;
      while (n)
	{
	  m = ((n)->succ);
	  if (((n)->node) == v)
	    {
	      k1++;
	      k3 += k2;
	      gs_ide521 (n);
	    }
	  else
	    k2++;
	  if (n == ((v)->Vpointer))
	    break;
	  n = m;
	}
      gs_ide551 += (k1 * gs_ide575 + k3);
    }
  a = ((v)->pred);
  while (a)
    {
      if ((((((((a)->kante))->start)))->position) > ((v)->position))
	{
	  gs_ide507 ((((((a)->kante))->start)));
	  (((((((a)->kante))->start)))->Vpointer) = gs_ide585;
	}
      a = ((a)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide507 (GNODE n)
#else
static void
gs_ide507 (n)
     GNODE n;
#endif
{
  DLLIST d;
  assert ((n));;
  d = dllist_alloc (n, gs_ide585);
  if (!gs_ide584)
    gs_ide584 = d;
  if (gs_ide585)
    ((gs_ide585)->succ) = d;
  gs_ide585 = d;
  gs_ide575++;
}

#ifdef ANSI_C
static void
gs_ide506 (GNODE n)
#else
static void
gs_ide506 (n)
     GNODE n;
#endif
{
  DLLIST d;
  assert ((n));;
  d = dllist_alloc (n, gs_ide545);
  if (!gs_ide544)
    gs_ide544 = d;
  if (gs_ide545)
    ((gs_ide545)->succ) = d;
  gs_ide545 = d;
  gs_ide571++;
}

#ifdef ANSI_C
static void
gs_ide522 (DLLIST x)
#else
static void
gs_ide522 (x)
     DLLIST x;
#endif
{
  assert ((x));
  assert ((((x)->node)));;
  if (((x)->pred))
    ((((x)->pred))->succ) = ((x)->succ);
  else
    gs_ide584 = ((x)->succ);
  if (((x)->succ))
    ((((x)->succ))->pred) = ((x)->pred);
  else
    gs_ide585 = ((x)->pred);
  dllist_free (x);
  gs_ide575--;
}

#ifdef ANSI_C
static void
gs_ide521 (DLLIST x)
#else
static void
gs_ide521 (x)
     DLLIST x;
#endif
{
  assert ((x));
  assert ((((x)->node)));;
  if (((x)->pred))
    ((((x)->pred))->succ) = ((x)->succ);
  else
    gs_ide544 = ((x)->succ);
  if (((x)->succ))
    ((((x)->succ))->pred) = ((x)->pred);
  else
    gs_ide545 = ((x)->pred);
  dllist_free (x);
  gs_ide571--;
}

#ifdef ANSI_C
static void
gs_ide557 (int level)
#else
static void
gs_ide557 (level)
     int level;
#endif
{
  int i;
  GNLIST vl;;
  if (level > 0)
    {
      i = 1;
      vl = ((gs_ide580[level - 1]).succlist);
      while (vl)
	{
	  ((((vl)->node))->position) = i++;
	  vl = ((vl)->next);
	}
    }
  if (level <= maxdepth)
    {
      i = 1;
      vl = ((gs_ide580[level + 1]).succlist);
      while (vl)
	{
	  ((((vl)->node))->position) = i++;
	  vl = ((vl)->next);
	}
    }
}

#ifdef ANSI_C
static int
gs_ide525 (GNODE C, GNODE D)
#else
static int
gs_ide525 (C, D)
     GNODE C;
     GNODE D;
#endif
{
  GNODE n;
  DLLIST actlistC, h;
  ADJEDGE a;
  int Sum1, Sum2;;
  Sum1 = Sum2 = 0;
  actlistC = NULL;
  a = ((C)->pred);
  while (a)
    {
      actlistC = dllist_alloc ((((((a)->kante))->start)), actlistC);
      ((actlistC)->succ) = ((actlistC)->pred);
      a = ((a)->next);
    }
  a = ((D)->pred);
  while (a)
    {
      n = (((((a)->kante))->start));
      h = actlistC;
      while (h)
	{
	  if (((n)->position) < ((((h)->node))->position))
	    Sum1++;
	  else if (((n)->position) > ((((h)->node))->position))
	    Sum2++;
	  h = ((h)->succ);
	}
      a = ((a)->next);
    }
  dllist_free_all (actlistC);
  actlistC = NULL;
  a = ((C)->succ);
  while (a)
    {
      actlistC = dllist_alloc ((((((a)->kante))->end)), actlistC);
      ((actlistC)->succ) = ((actlistC)->pred);
      a = ((a)->next);
    }
  a = ((D)->succ);
  while (a)
    {
      n = (((((a)->kante))->end));
      h = actlistC;
      while (h)
	{
	  if (((n)->position) < ((((h)->node))->position))
	    Sum1++;
	  else if (((n)->position) > ((((h)->node))->position))
	    Sum2++;
	  h = ((h)->succ);
	}
      a = ((a)->next);
    }
  dllist_free_all (actlistC);
  return (Sum1 > Sum2);
}

#ifdef ANSI_C
static int
gs_ide540 (int level, int nearedges)
#else
static int
gs_ide540 (level, nearedges)
     int level;
     int nearedges;
#endif
{
  int changed, possible;
  GNLIST vl1, vl2;
  GNODE n1, n2;;
  assert ((level >= 0));
  assert ((level <= maxdepth + 1));
  gs_ide557 (level);
  changed = 0;
  vl1 = NULL;
  vl2 = ((gs_ide580[level]).succlist);
  while (vl2)
    {
      n2 = ((vl2)->node);
      if (vl1)
	{
	  n1 = ((vl1)->node);
	  possible = 1;
	  if (nearedges)
	    {
	      if ((((n1)->connection)) || (((n2)->connection)))
		possible = 0;
	    }
	  if ((possible) && (gs_ide525 (n1, n2)))
	    {
	      ((vl1)->node) = n2;
	      ((vl2)->node) = n1;
	      changed = 1;
	    }
	}
      vl1 = vl2;
      vl2 = ((vl2)->next);
    }
  return (changed);
}

#ifdef ANSI_C
static void
gs_ide542 (int nearedges)
#else
static void
gs_ide542 (nearedges)
     int nearedges;
#endif
{
  int i;;
  if (!local_unwind)
    return;
  if (G_timelimit > 0)
    if (test_timelimit (60))
      {
	gs_wait_message ('t');
	return;
      }
  gs_wait_message ('l');
  for (i = 0; i <= maxdepth + 1; i++)
    while (gs_ide540 (i, nearedges));
}

#ifdef ANSI_C
static void
gs_ide558 (void)
#else
static void
gs_ide558 ()
#endif
{
  GNLIST h1;
  int i, j;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      h1 = ((gs_ide580[i]).succlist);
      j = 0;
      while (h1)
	{
	  ((((h1)->node))->position) = j++;
	  h1 = ((h1)->next);
	}
    }
}

#ifdef ANSI_C
static int
gs_ide535 (GNODE v)
#else
static int
gs_ide535 (v)
     GNODE v;
#endif
{
  ADJEDGE a;;
  if (((v)->connection))
    return (1);
  a = ((v)->succ);
  if (!a)
    return (1);
  if (((a)->next))
    return (1);
  a = ((v)->pred);
  if (!a)
    return (1);
  if (((a)->next))
    return (1);
  return (0);
}

#ifdef ANSI_C
static int
gs_ide513 (GNODE v1, GNODE v2, int dir)
#else
static int
gs_ide513 (v1, v2, dir)
     GNODE v1;
     GNODE v2;
     int dir;
#endif
{
  int d1, d2;
  GNODE n1, n2;;
  if (gs_ide535 (v1))
    return (0);
  if (gs_ide535 (v2))
    return (0);
  d1 = ((v1)->position) - ((v2)->position);
  if (d1 * d1 != 1)
    return (0);
  if (dir == 'S')
    {
      n1 = (((((((v1)->succ))->kante))->end));
      n2 = (((((((v2)->succ))->kante))->end));
    }
  else
    {
      n1 = (((((((v1)->pred))->kante))->start));
      n2 = (((((((v2)->pred))->kante))->start));
    }
  d2 = ((n1)->position) - ((n2)->position);
  if (d1 * d2 < 0)
    return (1);
  return (gs_ide513 (n1, n2, dir));
}

#ifdef ANSI_C
static void
gs_ide523 (GNODE v1, GNODE v2, int dir)
#else
static void
gs_ide523 (v1, v2, dir)
     GNODE v1;
     GNODE v2;
     int dir;
#endif
{
  int d1, d2, h;
  GNODE n1, n2;
  GNLIST vl1, vl2, vl3;;
  if (gs_ide535 (v1))
    return;
  if (gs_ide535 (v2))
    return;
  d1 = ((v1)->position) - ((v2)->position);
  if (d1 * d1 != 1)
    return;
  assert ((((v1)->tiefe) == ((v2)->tiefe)));
  vl1 = vl2 = NULL;
  vl3 = ((gs_ide580[((v1)->tiefe)]).succlist);
  while (vl3)
    {
      if (((vl3)->node) == v1)
	vl1 = vl3;
      if (((vl3)->node) == v2)
	vl2 = vl3;
      vl3 = ((vl3)->next);
    }
  assert ((vl1));
  assert ((vl2));
  ((vl1)->node) = v2;
  ((vl2)->node) = v1;
  h = ((v1)->position);
  ((v1)->position) = ((v2)->position);
  ((v2)->position) = h;
  if (dir == 'S')
    {
      n1 = (((((((v1)->succ))->kante))->end));
      n2 = (((((((v2)->succ))->kante))->end));
    }
  else
    {
      n1 = (((((((v1)->pred))->kante))->start));
      n2 = (((((((v2)->pred))->kante))->start));
    }
  d2 = ((n1)->position) - ((n2)->position);
  if (d1 * d2 < 0)
    return;
  gs_ide523 (n1, n2, dir);
}

#ifdef ANSI_C
static void
gs_ide583 (GNODE v)
#else
static void
gs_ide583 (v)
     GNODE v;
#endif
{
  ADJEDGE a1, a2;;
  a1 = ((v)->succ);
  while (a1)
    {
      a2 = ((v)->succ);
      while (a2)
	{
	  if (gs_ide513 ((((((a1)->kante))->end)), (((((a2)->kante))->
						     end)), 'S'))
	    {
	      gs_ide523 ((((((a1)->kante))->end)), (((((a2)->kante))->end)),
			 'S');
	    }
	  a2 = ((a2)->next);
	}
      a1 = ((a1)->next);
    }
  a1 = ((v)->pred);
  while (a1)
    {
      a2 = ((v)->pred);
      while (a2)
	{
	  if (gs_ide513
	      ((((((a1)->kante))->start)), (((((a2)->kante))->start)), 'P'))
	    {
	      gs_ide523 ((((((a1)->kante))->start)),
			 (((((a2)->kante))->start)), 'P');
	    }
	  a2 = ((a2)->next);
	}
      a1 = ((a1)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide543 (void)
#else
static void
gs_ide543 ()
#endif
{
  GNODE v;;
  if (G_timelimit > 0)
    if (test_timelimit (60))
      {
	gs_wait_message ('t');
	return;
      }
  gs_wait_message ('l');
  gs_ide558 ();
  v = nodelist;
  while (v)
    {
      gs_ide583 (v);
      v = ((v)->next);
    }
}
static int gs_ide529;
static int gs_ide552;
#ifdef ANSI_C
static void
gs_ide531 (void)
#else
static void
gs_ide531 ()
#endif
{
  gs_ide529 = 0;
  gs_ide552 = 1;
}

#define tmp_layer_is_better() {          	changed = 1;		         	copy_tmp_layer_to_layer();       	nr_crossings = cross;            	alt_startlevel = tmp_startlevel; }
#define tmp_layer_is_worser() {          	copy_layer_to_tmp_layer();       	tmp_startlevel = alt_startlevel; }
#define	copy_tmp_layer_to_layer()  { gs_ide518 (layer,gs_ide580 ); }
#define	copy_layer_to_tmp_layer()  { gs_ide518 (gs_ide580 ,layer); }
static int gs_ide553;
#ifdef ANSI_C
static void
gs_ide510 (void)
#else
static void
gs_ide510 ()
#endif
{
  int alt;
  int cross;
  int changed;
  int tmp_startlevel, alt_startlevel;;
  assert (gs_ide580);
  tmp_startlevel = alt_startlevel = 0;
  changed = 1;
  while (gs_ide552 && changed)
    {
      gs_ide550++;
      if (gs_ide550 >= max_baryiterations)
	{
	  gs_wait_message ('t');
	  break;
	}
      if (G_timelimit > 0)
	if (test_timelimit (60))
	  {
	    gs_wait_message ('t');
	    break;
	  }
      changed = 0;
      gs_ide500 ();
      cross = gs_ide528 ();
      alt = 0;
      if ((cross < nr_crossings) || (gs_ide550 < min_baryiterations))
	{
	  tmp_layer_is_better ();
	}
      else if (gs_ide529)
	{
	  copy_layer_to_tmp_layer ();
	  gs_ide500 ();
	  cross = gs_ide528 ();
	  if (cross < nr_crossings)
	    {
	      tmp_layer_is_better ();
	    }
	  else if (cross > nr_crossings)
	    {
	      tmp_layer_is_worser ();
	    }
	  else
	    alt = 1;
	}
      else if (cross == nr_crossings)
	alt = 1;
      else
	{
	  tmp_layer_is_worser ();
	}
      gs_ide529 = alt;
      gs_ide501 ();
      cross = gs_ide528 ();
      alt = 0;
      if ((cross < nr_crossings) || (gs_ide550 < min_baryiterations))
	{
	  tmp_layer_is_better ();
	}
      else if (gs_ide529)
	{
	  copy_layer_to_tmp_layer ();
	  gs_ide501 ();
	  cross = gs_ide528 ();
	  if (cross < nr_crossings)
	    {
	      tmp_layer_is_better ();
	    }
	  else if (cross > nr_crossings)
	    {
	      tmp_layer_is_worser ();
	    }
	  else
	    alt = 1;
	}
      else if (cross == nr_crossings)
	alt = 1;
      else
	{
	  tmp_layer_is_worser ();
	}
      gs_ide529 = alt;
      if (nr_crossings == 0)
	return;
    }
  if (nr_crossings == 0)
    return;
  if (skip_baryphase2)
    return;
  gs_ide552 = 0;
  tmp_startlevel = alt_startlevel = 0;
  changed = 1;
  while (changed)
    {
      gs_ide550++;
      if (gs_ide550 >= max_baryiterations)
	{
	  gs_wait_message ('t');
	  break;
	}
      if (G_timelimit > 0)
	if (test_timelimit (60))
	  {
	    gs_wait_message ('t');
	    break;
	  }
      changed = 0;
      gs_ide553 = tmp_startlevel;
      gs_ide502 ();
      tmp_startlevel = gs_ide553;
      if (tmp_startlevel > maxdepth)
	tmp_startlevel = 0;
      cross = gs_ide528 ();
      alt = 0;
      if (cross < nr_crossings)
	{
	  tmp_layer_is_better ();
	}
      else if (gs_ide529)
	{
	  copy_layer_to_tmp_layer ();
	  gs_ide553 = tmp_startlevel = alt_startlevel;
	  gs_ide502 ();
	  tmp_startlevel = gs_ide553;
	  if (tmp_startlevel > maxdepth)
	    tmp_startlevel = 0;
	  cross = gs_ide528 ();
	  if (cross < nr_crossings)
	    {
	      tmp_layer_is_better ();
	    }
	  else if (cross > nr_crossings)
	    {
	      tmp_layer_is_worser ();
	    }
	  else
	    alt = 1;
	}
      else if (cross == nr_crossings)
	alt = 1;
      else
	{
	  tmp_layer_is_worser ();
	}
      gs_ide529 = alt;
      if (nr_crossings == 0)
	return;
    }
  tmp_startlevel = alt_startlevel = maxdepth + 1;
  changed = 1;
  while (changed)
    {
      gs_ide550++;
      if (gs_ide550 >= max_baryiterations)
	{
	  gs_wait_message ('t');
	  break;
	}
      if (G_timelimit > 0)
	if (test_timelimit (60))
	  {
	    gs_wait_message ('t');
	    break;
	  }
      changed = 0;
      gs_ide553 = tmp_startlevel;
      gs_ide503 ();
      tmp_startlevel = gs_ide553;
      if (tmp_startlevel < 0)
	tmp_startlevel = maxdepth;
      cross = gs_ide528 ();
      alt = 0;
      if (cross < nr_crossings)
	{
	  tmp_layer_is_better ();
	}
      else if (gs_ide529)
	{
	  copy_layer_to_tmp_layer ();
	  gs_ide553 = tmp_startlevel = alt_startlevel;
	  gs_ide503 ();
	  tmp_startlevel = gs_ide553;
	  if (tmp_startlevel < 0)
	    tmp_startlevel = maxdepth;
	  cross = gs_ide528 ();
	  if (cross < nr_crossings)
	    {
	      tmp_layer_is_better ();
	    }
	  else if (cross > nr_crossings)
	    {
	      tmp_layer_is_worser ();
	    }
	  else
	    alt = 1;
	}
      else if (cross == nr_crossings)
	alt = 1;
      else
	{
	  tmp_layer_is_worser ();
	}
      gs_ide529 = alt;
      if (nr_crossings == 0)
	return;
    }
}

#ifdef ANSI_C
static void
gs_ide500 (void)
#else
static void
gs_ide500 ()
#endif
{
  int i;;
  gs_wait_message ('b');
  for (i = 0; i <= maxdepth; i++)
    (void) gs_ide561 (i);
#ifdef CHECK_CROSSING
  i = gs_ide528 ();
  gs_ide511 ();
  assert ((i == gs_ide528 ()));
  PRINTF ("Phase1_down: nr_crossings old: %d new: %d\n", nr_crossings, i);
#endif
}

#ifdef ANSI_C
static void
gs_ide501 (void)
#else
static void
gs_ide501 ()
#endif
{
  int i;;
  gs_wait_message ('b');
  for (i = maxdepth; i >= 0; i--)
    (void) gs_ide564 (i);
#ifdef CHECK_CROSSING
  i = gs_ide528 ();
  gs_ide511 ();
  assert ((i == gs_ide528 ()));
  PRINTF ("Phase1_up: nr_crossings old: %d new: %d\n", nr_crossings, i);
#endif
}

#ifdef ANSI_C
static int
gs_ide561 (int i)
#else
static int
gs_ide561 (i)
     int i;
#endif
{
  int c;
  int j;;
  assert ((i >= 0));
  assert ((i <= maxdepth));
  gs_ide541 (i + 1, 'd');
  switch (crossing_heuristics)
    {
    case 0:
      for (j = 0; j < ((layer[i + 1]).anz); j++)
	((gs_ide577[j])->bary) = gs_ide554 (gs_ide577[j]);
      break;
    case 1:
      for (j = 0; j < ((layer[i + 1]).anz); j++)
	((gs_ide577[j])->bary) = gs_ide555 (gs_ide577[j]);
      break;
    case 2:
      for (j = 0; j < ((layer[i + 1]).anz); j++)
	((gs_ide577[j])->bary) =
	  gs_ide554 (gs_ide577[j]) + gs_ide555 (gs_ide577[j]) / 10000.0;
      break;
    case 3:
      for (j = 0; j < ((layer[i + 1]).anz); j++)
	((gs_ide577[j])->bary) = gs_ide555 (gs_ide577[j]) +
	  gs_ide554 (gs_ide577[j]) / 10000.0;
      break;
    }
  quicksort_sort_array (((layer[i + 1]).anz));
  gs_ide570 (i + 1);
  gs_ide509 (i + 1);
  if (((layer[i + 1]).resort_necessary))
    gs_ide508 (i + 1);
  c = gs_ide536 (i);
  if (c <= ((gs_ide580[i]).cross))
    {
      ((gs_ide580[i]).cross) = c;
      if (i < maxdepth)
	((gs_ide580[i + 1]).cross) = gs_ide536 (i + 1);
      return (1);
    }
  gs_ide565 (i + 1);
  return (0);
}

#ifdef ANSI_C
static int
gs_ide564 (int i)
#else
static int
gs_ide564 (i)
     int i;
#endif
{
  int c;
  int j;;
  assert ((i >= 0));
  assert ((i <= maxdepth));
  gs_ide541 (i, 'u');
  switch (crossing_heuristics)
    {
    case 0:
      for (j = 0; j < ((layer[i]).anz); j++)
	((gs_ide577[j])->bary) = gs_ide578 (gs_ide577[j]);
      break;
    case 1:
      for (j = 0; j < ((layer[i]).anz); j++)
	((gs_ide577[j])->bary) = gs_ide579 (gs_ide577[j]);
      break;
    case 2:
      for (j = 0; j < ((layer[i]).anz); j++)
	((gs_ide577[j])->bary) =
	  gs_ide578 (gs_ide577[j]) + gs_ide579 (gs_ide577[j]) / 10000.0;
      break;
    case 3:
      for (j = 0; j < ((layer[i]).anz); j++)
	((gs_ide577[j])->bary) = gs_ide579 (gs_ide577[j]) +
	  gs_ide578 (gs_ide577[j]) / 10000.0;
      break;
    }
  quicksort_sort_array (((layer[i]).anz));
  gs_ide570 (i);
  gs_ide509 (i);
  if (((layer[i]).resort_necessary))
    gs_ide508 (i);
  c = gs_ide536 (i);
  if (c <= ((gs_ide580[i]).cross))
    {
      ((gs_ide580[i]).cross) = c;
      if (i > 0)
	((gs_ide580[i - 1]).cross) = gs_ide536 (i - 1);
      return (1);
    }
  gs_ide565 (i);
  return (0);
}

#ifdef ANSI_C
static void
gs_ide502 (void)
#else
static void
gs_ide502 ()
#endif
{
  int i, j;
  int cross;;
  gs_wait_message ('B');
  if (gs_ide553 <= maxdepth)
    for (i = gs_ide553; i <= maxdepth; i++)
      {
	if (G_timelimit > 0)
	  if (test_timelimit (60))
	    {
	      gs_wait_message ('t');
	      break;
	    }
	gs_ide541 (i, 'u');
	switch (crossing_heuristics)
	  {
	  case 0:
	    for (j = 0; j < ((layer[i]).anz); j++)
	      ((gs_ide577[j])->bary) = gs_ide578 (gs_ide577[j]);
	    break;
	  case 1:
	    for (j = 0; j < ((layer[i]).anz); j++)
	      ((gs_ide577[j])->bary) = gs_ide579 (gs_ide577[j]);
	    break;
	  case 2:
	    for (j = 0; j < ((layer[i]).anz); j++)
	      ((gs_ide577[j])->bary) =
		gs_ide578 (gs_ide577[j]) + gs_ide579 (gs_ide577[j]) / 10000.0;
	    break;
	  case 3:
	    for (j = 0; j < ((layer[i]).anz); j++)
	      ((gs_ide577[j])->bary) = gs_ide579 (gs_ide577[j]) +
		gs_ide578 (gs_ide577[j]) / 10000.0;
	    break;
	  }
	quicksort_sort_array (((layer[i]).anz));
	if (gs_ide520 (((layer[i]).anz)))
	  {
	    gs_ide509 (i);
	    if (((layer[i]).resort_necessary))
	      gs_ide508 (i);
	    if (i > 0)
	      ((gs_ide580[i - 1]).cross) = gs_ide536 (i - 1);
	    if (i <= maxdepth)
	      ((gs_ide580[i]).cross) = gs_ide536 (i);
	    gs_ide563 (i);
	    cross = gs_ide528 ();
	    if (cross < nr_crossings)
	      {
#ifdef CHECK_CROSSING
		j = gs_ide528 ();
		gs_ide511 ();
		assert ((j == gs_ide528 ()));
		PRINTF ("Phase2_down: nr_crossings old: %d new: %d\n",
			nr_crossings, j);
#endif
		gs_ide553 = i + 1;
		return;
	      }
	  }
      }
  for (i = 0; (i < gs_ide553) && (i <= maxdepth); i++)
    {
      if (G_timelimit > 0)
	if (test_timelimit (60))
	  {
	    gs_wait_message ('t');
	    break;
	  }
      gs_ide541 (i, 'u');
      switch (crossing_heuristics)
	{
	case 0:
	  for (j = 0; j < ((layer[i]).anz); j++)
	    ((gs_ide577[j])->bary) = gs_ide554 (gs_ide577[j]);
	  break;
	case 1:
	  for (j = 0; j < ((layer[i]).anz); j++)
	    ((gs_ide577[j])->bary) = gs_ide555 (gs_ide577[j]);
	  break;
	case 2:
	  for (j = 0; j < ((layer[i]).anz); j++)
	    ((gs_ide577[j])->bary) =
	      gs_ide554 (gs_ide577[j]) + gs_ide555 (gs_ide577[j]) / 10000.0;

	  break;
	case 3:
	  for (j = 0; j < ((layer[i]).anz); j++)
	    ((gs_ide577[j])->bary) = gs_ide555 (gs_ide577[j]) +
	      gs_ide554 (gs_ide577[j]) / 10000.0;
	  break;
	}
      quicksort_sort_array (((layer[i]).anz));
      if (gs_ide520 (((layer[i]).anz)))
	{
	  gs_ide509 (i);
	  if (((layer[i]).resort_necessary))
	    gs_ide508 (i);
	  if (i > 0)
	    ((gs_ide580[i - 1]).cross) = gs_ide536 (i - 1);
	  if (i <= maxdepth)
	    ((gs_ide580[i]).cross) = gs_ide536 (i);
	  gs_ide563 (i);
	  cross = gs_ide528 ();
	  if (cross < nr_crossings)
	    {
#ifdef CHECK_CROSSING
	      j = gs_ide528 ();
	      gs_ide511 ();
	      assert ((j == gs_ide528 ()));
	      PRINTF ("Phase2_down: nr_crossings old: %d new: %d\n",
		      nr_crossings, j);
#endif
	      gs_ide553 = i + 1;
	      return;
	    }
	}
    }
#ifdef CHECK_CROSSING
  i = gs_ide528 ();
  gs_ide511 ();
  assert ((i == gs_ide528 ()));
  PRINTF ("Phase2_down: nr_crossings old: %d new: %d\n", nr_crossings, i);
#endif
}

#ifdef ANSI_C
static void
gs_ide503 (void)
#else
static void
gs_ide503 ()
#endif
{
  int i, j;
  int cross;;
  gs_wait_message ('B');
  if (gs_ide553 > 0)
    for (i = gs_ide553; i > 0; i--)
      {
	if (G_timelimit > 0)
	  if (test_timelimit (60))
	    {
	      gs_wait_message ('t');
	      break;
	    }
	gs_ide541 (i, 'd');
	switch (crossing_heuristics)
	  {
	  case 0:
	    for (j = 0; j < ((layer[i]).anz); j++)
	      ((gs_ide577[j])->bary) = gs_ide554 (gs_ide577[j]);
	    break;
	  case 1:
	    for (j = 0; j < ((layer[i]).anz); j++)
	      ((gs_ide577[j])->bary) = gs_ide555 (gs_ide577[j]);
	    break;
	  case 2:
	    for (j = 0; j < ((layer[i]).anz); j++)
	      ((gs_ide577[j])->bary) =
		gs_ide554 (gs_ide577[j]) + gs_ide555 (gs_ide577[j]) / 10000.0;
	    break;
	  case 3:
	    for (j = 0; j < ((layer[i]).anz); j++)
	      ((gs_ide577[j])->bary) = gs_ide579 (gs_ide577[j]) +
		gs_ide554 (gs_ide577[j]) / 10000.0;
	    break;
	  }
	quicksort_sort_array (((layer[i]).anz));
	if (gs_ide520 (((layer[i]).anz)))
	  {
	    gs_ide509 (i);
	    if (((layer[i]).resort_necessary))
	      gs_ide508 (i);
	    if (i > 0)
	      ((gs_ide580[i - 1]).cross) = gs_ide536 (i - 1);
	    if (i <= maxdepth)
	      ((gs_ide580[i]).cross) = gs_ide536 (i);
	    gs_ide562 (i);
	    cross = gs_ide528 ();
	    if (cross < nr_crossings)
	      {
#ifdef CHECK_CROSSING
		j = gs_ide528 ();
		gs_ide511 ();
		assert ((j == gs_ide528 ()));
		PRINTF ("Phase2_up: nr_crossings old: %d new: %d\n",
			nr_crossings, j);
#endif
		gs_ide553 = i - 1;
		return;
	      }
	  }
      }
  for (i = maxdepth + 1; (i > gs_ide553) && (i > 0); i--)
    {
      if (G_timelimit > 0)
	if (test_timelimit (60))
	  {
	    gs_wait_message ('t');
	    break;
	  }
      gs_ide541 (i, 'd');
      switch (crossing_heuristics)
	{
	case 0:
	  for (j = 0; j < ((layer[i]).anz); j++)
	    ((gs_ide577[j])->bary) = gs_ide554 (gs_ide577[j]);
	  break;
	case 1:
	  for (j = 0; j < ((layer[i]).anz); j++)
	    ((gs_ide577[j])->bary) = gs_ide555 (gs_ide577[j]);
	  break;
	case 2:
	  for (j = 0; j < ((layer[i]).anz); j++)
	    ((gs_ide577[j])->bary) =
	      gs_ide554 (gs_ide577[j]) + gs_ide555 (gs_ide577[j]) / 10000.0;

	  break;
	case 3:
	  for (j = 0; j < ((layer[i]).anz); j++)
	    ((gs_ide577[j])->bary) = gs_ide555 (gs_ide577[j]) +
	      gs_ide554 (gs_ide577[j]) / 10000.0;
	  break;
	}
      quicksort_sort_array (((layer[i]).anz));
      if (gs_ide520 (((layer[i]).anz)))
	{
	  gs_ide509 (i);
	  if (((layer[i]).resort_necessary))
	    gs_ide508 (i);
	  if (i > 0)
	    ((gs_ide580[i - 1]).cross) = gs_ide536 (i - 1);
	  if (i <= maxdepth)
	    ((gs_ide580[i]).cross) = gs_ide536 (i);
	  gs_ide562 (i);
	  cross = gs_ide528 ();
	  if (cross < nr_crossings)
	    {
#ifdef CHECK_CROSSING
	      j = gs_ide528 ();
	      gs_ide511 ();
	      assert ((j == gs_ide528 ()));
	      PRINTF ("Phase2_up: nr_crossings old: %d new: %d\n",
		      nr_crossings, j);
#endif
	      gs_ide553 = i - 1;
	      return;
	    }
	}
    }
#ifdef CHECK_CROSSING
  i = gs_ide528 ();
  gs_ide511 ();
  assert ((i == gs_ide528 ()));
  PRINTF ("Phase2_up: nr_crossings old: %d new: %d\n", nr_crossings, i);
#endif
}

#ifdef ANSI_C
static void
gs_ide563 (int level)
#else
static void
gs_ide563 (level)
     int level;
#endif
{
  int change;
  int i;;
  change = 1;
  if (level > 0)
    {
      for (i = level - 1; i >= 0; i--)
	{
	  change = gs_ide564 (i);
	  if (!change)
	    break;
	}
    }
  if (level <= maxdepth)
    {
      for (i = level; i <= maxdepth; i++)
	{
	  change = gs_ide561 (i);
	  if (!change)
	    break;
	}
    }
}

#ifdef ANSI_C
static void
gs_ide562 (int level)
#else
static void
gs_ide562 (level)
     int level;
#endif
{
  int change;
  int i;;
  change = 1;
  if (level <= maxdepth)
    {
      for (i = level; i <= maxdepth; i++)
	{
	  change = gs_ide561 (i);
	  if (!change)
	    break;
	}
    }
  if (level > 0)
    {
      for (i = level - 1; i >= 0; i--)
	{
	  change = gs_ide564 (i);
	  if (!change)
	    break;
	}
    }
}

#ifdef ANSI_C
static int
gs_ide520 (int siz)
#else
static int
gs_ide520 (siz)
     int siz;
#endif
{
  int j, k;
  int original_sit;
  int start_region;
  GNODE h;;
  original_sit = 1;
  start_region = -1;
  for (j = 0; j < siz - 1; j++)
    {
      if (((gs_ide577[j])->bary) == ((gs_ide577[j + 1])->bary))
	{
	  if (start_region == -1)
	    start_region = j;
	}
      else if (start_region != -1)
	{
	  h = gs_ide577[j];
	  for (k = j; k > start_region; k--)
	    gs_ide577[k] = gs_ide577[k - 1];
	  gs_ide577[start_region] = h;
	  start_region = -1;
	  original_sit = 0;
	}
    }
  if (start_region != -1)
    {
      h = gs_ide577[j];
      for (k = j; k > start_region; k--)
	gs_ide577[k] = gs_ide577[k - 1];
      gs_ide577[start_region] = h;
      original_sit = 0;
    }
  return (!original_sit);
}

#ifdef ANSI_C
static float
gs_ide578 (GNODE node)
#else
static float
gs_ide578 (node)
     GNODE node;
#endif
{
  int Sum;
  ADJEDGE w;
  assert ((node));;
  if (((node)->outdegree) == 0)
    return (0.0);
  Sum = 0;
  w = ((node)->succ);
  while (w)
    {
      Sum += (256 * (((((((w)->kante))->end)))->position));
      Sum -= ((((w)->kante))->anchor);
      w = ((w)->next);
    }
  return (((float) Sum) / ((float) (256 * ((node)->outdegree))));
}

#ifdef ANSI_C
static float
gs_ide554 (GNODE node)
#else
static float
gs_ide554 (node)
     GNODE node;
#endif
{
  int Sum;
  ADJEDGE w;
  assert ((node));;
  if (((node)->indegree) == 0)
    return (0.0);
  Sum = 0;
  w = ((node)->pred);
  while (w)
    {
      Sum += (256 * (((((((w)->kante))->start)))->position));
      Sum += ((((w)->kante))->anchor);
      w = ((w)->next);
    }
  return (((float) Sum) / ((float) (256 * ((node)->indegree))));
}

#ifdef ANSI_C
static float
gs_ide579 (GNODE node)
#else
static float
gs_ide579 (node)
     GNODE node;
#endif
{
  int i, leftpart, rightpart;
  ADJEDGE w;
  assert ((node));;
  switch (((node)->outdegree))
    {
    case 0:
      return (0.0);
    case 1:
      return ((float) (((((((((node)->succ))->kante))->end)))->position) -
	      (float) ((((((node)->succ))->kante))->anchor) / 256.0);
    case 2:
      w = ((node)->succ);
      i = (((((((w)->kante))->end)))->position);
      w = ((w)->next);
      i += (((((((w)->kante))->end)))->position);
      return (((float) i) / 2.0);
    } i = 0;
  w = ((node)->succ);
  while (w)
    {
      gs_ide569[i++] = (((((w)->kante))->end));
      w = ((w)->next);
    }
  quicksort_save_array (i);
  if (i % 2)
    return ((float) ((gs_ide569[i / 2])->position));
  leftpart = ((gs_ide569[i / 2 - 1])->position) - ((gs_ide569[0])->position);
  rightpart = ((gs_ide569[i - 1])->position) - ((gs_ide569[i / 2])->position);
  return (((float)
	   (((gs_ide569[i / 2 - 1])->position) * rightpart +
	    ((gs_ide569[i / 2])->position) * leftpart)) / ((float) (leftpart +
								    rightpart)));
}

#ifdef ANSI_C
static float
gs_ide555 (GNODE node)
#else
static float
gs_ide555 (node)
     GNODE node;
#endif
{
  int i, leftpart, rightpart;
  ADJEDGE w;
  assert ((node));;
  switch (((node)->indegree))
    {
    case 0:
      return (0.0);
    case 1:
      return ((float) (((((((((node)->pred))->kante))->start)))->position) +
	      (float) ((((((node)->pred))->kante))->anchor) / 256.0);
    case 2:
      w = ((node)->pred);
      i = (((((((w)->kante))->start)))->position);
      w = ((w)->next);
      i += (((((((w)->kante))->start)))->position);
      return (((float) i) / 2.0);
    } i = 0;
  w = ((node)->pred);
  while (w)
    {
      gs_ide569[i++] = (((((w)->kante))->start));
      w = ((w)->next);
    }
  quicksort_save_array (i);
  if (i % 2)
    return ((float) ((gs_ide569[i / 2])->position));
  leftpart = ((gs_ide569[i / 2 - 1])->position) - ((gs_ide569[0])->position);
  rightpart = ((gs_ide569[i - 1])->position) - ((gs_ide569[i / 2])->position);
  return (((float)
	   (((gs_ide569[i / 2 - 1])->position) * rightpart +
	    ((gs_ide569[i / 2])->position) * leftpart)) / ((float) (leftpart +
								    rightpart)));
}

#if 0
#ifdef ANSI_C
static int
gs_ide514 (const GNODE * a, const GNODE * b)
#else
static int
gs_ide514 (a, b)
     GNODE *a;
     GNODE *b;
#endif
{
  if ((((*a)->bary) == 0.0) || (((*b)->bary) == 0.0))
    return (0);
  if (((*a)->bary) > ((*b)->bary))
    return (1);
  if (((*a)->bary) < ((*b)->bary))
    return (-1);
  return (0);
}
#endif

#ifdef ANSI_C
static int
gs_ide515 (const GNODE * a, const GNODE * b)
#else
static int
gs_ide515 (a, b)
     GNODE *a;
     GNODE *b;
#endif
{
  if (((*a)->position) > ((*b)->position))
    return (1);
  if (((*a)->position) < ((*b)->position))
    return (-1);
  return (0);
}

#ifdef OWN_QUICKSORT
#define exchange(a,b) { t=a; a=b; b=t; }
#ifdef ANSI_C
static void
gs_ide549 (int l, int r)
#else
static void
gs_ide549 (l, r)
     int l, r;
#endif
{
  int i, j, k;
  GNODE v, t;
  k = r - l;
  if (k > 5)
    {
      j = l + (gs_ide559 % k);
      gs_ide559 = (gs_ide559 + 10891) % MAXINT;
      exchange (gs_ide577[l], gs_ide577[j]);
    }
  v = gs_ide577[l];
  i = l;
  j = r + 1;
  if (((v)->bary) == 0.0)
    j = l + k / 2;
  else
    {
      while (i < j)
	{
	  do
	    j--;
	  while ((((gs_ide577[j])->bary) != 0.0)
		 && (((gs_ide577[j])->bary) > ((v)->bary)));
	  do
	    i++;
	  while ((i <= j) && (((gs_ide577[i])->bary) != 0.0)
		 && (((gs_ide577[i])->bary) < ((v)->bary)));
	  if (i < j)
	    exchange (gs_ide577[j], gs_ide577[i]);
	};
    }
  exchange (gs_ide577[l], gs_ide577[j]);
  if (l < j - 1)
    gs_ide549 (l, j - 1);
  if (j + 1 < r)
    gs_ide549 (j + 1, r);
}
#endif
#ifdef ANSI_C
static void
gs_ide541 (int i, int dir)
#else
static void
gs_ide541 (i, dir)
     int i;
     int dir;
#endif
{
  int j;
  GNLIST hn;;
  hn = ((gs_ide580[i]).succlist);
  j = 0;
  while (hn)
    {
      gs_ide577[j++] = ((hn)->node);
      hn = ((hn)->next);
    }
  if (dir == 'd')
    {
      assert ((i > 0));
      hn = ((gs_ide580[i - 1]).succlist);
    }
  else
    {
      assert ((i < maxdepth + 1));
      hn = ((gs_ide580[i + 1]).succlist);
    }
  j = 1;
  while (hn)
    {
      ((((hn)->node))->position) = j++;
      hn = ((hn)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide509 (int i)
#else
static void
gs_ide509 (i)
     int i;
#endif
{
  int j;
  GNLIST hn;;
  j = 0;
  hn = ((gs_ide580[i]).succlist);
  while (hn)
    {
      ((hn)->node) = gs_ide577[j++];
      hn = ((hn)->next);
    }
  assert (j == ((gs_ide580[i]).anz));
}

#ifdef ANSI_C
static void
gs_ide570 (int i)
#else
static void
gs_ide570 (i)
     int i;
#endif
{
  int j;
  GNLIST hn;;
  j = 0;
  hn = ((gs_ide580[i]).succlist);
  while (hn)
    {
      gs_ide569[j++] = ((hn)->node);
      hn = ((hn)->next);
    }
  assert (j == ((gs_ide580[i]).anz));
}

#ifdef ANSI_C
static void
gs_ide565 (int i)
#else
static void
gs_ide565 (i)
     int i;
#endif
{
  int j;
  GNLIST hn;;
  j = 0;
  hn = ((gs_ide580[i]).succlist);
  while (hn)
    {
      ((hn)->node) = gs_ide569[j++];
      hn = ((hn)->next);
    }
  assert (j == ((gs_ide580[i]).anz));
}

#ifdef ANSI_C
static void
gs_ide508 (int i)
#else
static void
gs_ide508 (i)
     int i;
#endif
{
  GNLIST hn;
  int j;;
  hn = ((gs_ide580[i]).succlist);
  j = 0;
  while (hn)
    {
      if (((((hn)->node))->nhorder) >= 0)
	{
	  gs_ide577[j++] = ((hn)->node);
	  ((((hn)->node))->bary) = ((((hn)->node))->nhorder);
	}
      hn = ((hn)->next);
    }
  quicksort_sort_array (j);
  hn = ((gs_ide580[i]).succlist);
  j = 0;
  while (hn)
    {
      if (((((hn)->node))->nhorder) >= 0)
	((hn)->node) = gs_ide577[j++];
      hn = ((hn)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide518 (DEPTH * l1, DEPTH * l2)
#else
static void
gs_ide518 (l1, l2)
     DEPTH *l1;
     DEPTH *l2;
#endif
{
  int i;
  GNLIST h1, h2;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      ((l1[i]).cross) = ((l2[i]).cross);
      h1 = ((l1[i]).succlist);
      h2 = ((l2[i]).succlist);
      if (((l1[i]).anz) == ((l2[i]).anz))
	{
	  while (h2)
	    {
	      assert ((h1));
	      ((h1)->node) = ((h2)->node);
	      h1 = ((h1)->next);
	      h2 = ((h2)->next);
	    }
	}
      else
	{
	  assert ((((l1[i]).anz) < ((l2[i]).anz)));
	  while (h2)
	    {
	      assert ((h1));
	      ((h1)->node) = ((h2)->node);
	      h2 = ((h2)->next);
	      if (h2 && !((h1)->next))
		((h1)->next) = tmpnodelist_alloc ();
	      h1 = ((h1)->next);
	    }
	  ((l1[i]).anz) = ((l2[i]).anz);
	}
    }
}

#define forward_connection1(c)  (((( c )->edge)  )&& ((( (( c )->edge)   )->end)  ==(( c )->target)  ))
#define forward_connection2(c)  (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  ==(( c )->target2)  ))
#ifdef ANSI_C
static void
gs_ide532 (void)
#else
static void
gs_ide532 ()
#endif
{
  int i, j;
  GNLIST hl, hln;
  CONNECT c;
  int forward_conn;
  int changed;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      hl = ((gs_ide580[i]).succlist);
      changed = 0;
      while (hl)
	{
	  hln = ((hl)->next);
	  c = ((((hl)->node))->connection);
	  forward_conn = 0;
	  if (c)
	    {
	      if (forward_connection1 (c))
		forward_conn = 1;
	      if (forward_connection2 (c))
		forward_conn = 1;
	    }
	  if (forward_conn && (((((hl)->node))->markiert) == 0))
	    {
	      changed = 1;
	      gs_ide512 (i, ((hl)->node));
	    }
	  hl = hln;
	}
      if (changed)
	{
	  if (i <= maxdepth)
	    ((gs_ide580[i]).cross) = gs_ide536 (i);
	  for (j = i; j <= maxdepth; j++)
	    (void) gs_ide561 (j);
	}
}} static GNLIST gs_ide538;
static GNLIST gs_ide539;
static GNLIST gs_ide567;
static GNLIST gs_ide568;
static GNLIST *gs_ide534;
static GNLIST gs_ide548;
static GNLIST gs_ide524;
#ifdef ANSI_C
static void
gs_ide512 (int level, GNODE node)
#else
static void
gs_ide512 (level, node)
     int level;
     GNODE node;
#endif
{
  CONNECT c;
  int j, clr, crl;
  ADJEDGE a;;
  c = ((node)->connection);
  ((node)->succ) = ((node)->savesucc);
  ((node)->pred) = ((node)->savepred);
  j = 0;
  a = ((node)->succ);
  while (a)
    {
      j++;
      a = ((a)->next);
    }
  ((node)->outdegree) = j;
  j = 0;
  a = ((node)->pred);
  while (a)
    {
      j++;
      a = ((a)->next);
    }
  ((node)->indegree) = j;
  gs_ide538 = gs_ide539 = gs_ide567 = gs_ide568 = NULL;
  if (forward_connection1 (c))
    gs_ide537 (((c)->target), node);
  if (forward_connection2 (c))
    gs_ide566 (((c)->target2), node);
  gs_ide533 (level, node);
  clr = 0;
  if (level > 0)
    clr += gs_ide536 (level - 1);
  if (level <= maxdepth)
    clr += gs_ide536 (level);
  *gs_ide534 = gs_ide548;
  ((gs_ide548)->next) = gs_ide524;
  gs_ide538 = gs_ide539 = gs_ide567 = gs_ide568 = NULL;
  if (forward_connection1 (c))
    gs_ide566 (((c)->target), node);
  if (forward_connection2 (c))
    gs_ide537 (((c)->target2), node);
  gs_ide533 (level, node);
  crl = 0;
  if (level > 0)
    crl += gs_ide536 (level - 1);
  if (level <= maxdepth)
    crl += gs_ide536 (level);
  if (crl <= clr)
    return;
  *gs_ide534 = gs_ide548;
  ((gs_ide548)->next) = gs_ide524;
  gs_ide538 = gs_ide539 = gs_ide567 = gs_ide568 = NULL;
  if (forward_connection1 (c))
    gs_ide537 (((c)->target), node);
  if (forward_connection2 (c))
    gs_ide566 (((c)->target2), node);
  gs_ide533 (level, node);
}

#ifdef ANSI_C
static void
gs_ide533 (int level, GNODE node)
#else
static void
gs_ide533 (level, node)
     int level;
     GNODE node;
#endif
{
  GNLIST hl, *hlp;
  int j;;
  hlp = &((gs_ide580[level]).succlist);
  hl = ((gs_ide580[level]).succlist);
  while (hl)
    {
      if (((hl)->node) == node)
	break;
      hlp = &((hl)->next);
      hl = ((hl)->next);
    }
  assert ((hl));
  gs_ide534 = hlp;
  gs_ide548 = hl;
  gs_ide524 = ((hl)->next);
  if (gs_ide538)
    {
      *hlp = gs_ide538;
      ((gs_ide539)->next) = hl;
    }
  hlp = &((hl)->next);
  hl = ((hl)->next);
  if (gs_ide567)
    {
      *hlp = gs_ide567;
      ((gs_ide568)->next) = hl;
    }
  j = 0;
  hl = ((gs_ide580[level]).succlist);
  while (hl)
    {
      j++;
      hl = ((hl)->next);
    }
  ((gs_ide580[level]).anz) = j;
}

#ifdef ANSI_C
static void
gs_ide537 (GNODE v, GNODE w)
#else
static void
gs_ide537 (v, w)
     GNODE v;
     GNODE w;
#endif
{
  GNLIST h;
  ADJEDGE e;
  CONNECT c;;
  e = ((v)->succ);
  while (e)
    {
      (((((e)->kante))->start)) = v;
      e = ((e)->next);
    }
  e = ((v)->pred);
  while (e)
    {
      (((((e)->kante))->end)) = v;
      e = ((e)->next);
    }
  h = tmpnodelist_alloc ();
  ((h)->node) = v;
  ((h)->next) = gs_ide538;
  gs_ide538 = h;
  if (!gs_ide539)
    gs_ide539 = h;
  c = ((v)->connection);
  if (!c)
    return;
  if (((c)->target) && (((c)->target) != w))
    gs_ide537 (((c)->target), v);
  if (((c)->target2) && (((c)->target2) != w))
    gs_ide537 (((c)->target2), v);
}

#ifdef ANSI_C
static void
gs_ide566 (GNODE v, GNODE w)
#else
static void
gs_ide566 (v, w)
     GNODE v;
     GNODE w;
#endif
{
  GNLIST h;
  ADJEDGE e;
  CONNECT c;;
  e = ((v)->succ);
  while (e)
    {
      (((((e)->kante))->start)) = v;
      e = ((e)->next);
    }
  e = ((v)->pred);
  while (e)
    {
      (((((e)->kante))->end)) = v;
      e = ((e)->next);
    }
  h = tmpnodelist_alloc ();
  ((h)->node) = v;
  if (gs_ide568)
    ((gs_ide568)->next) = h;
  ((h)->next) = NULL;
  gs_ide568 = h;
  if (!gs_ide567)
    gs_ide567 = h;
  c = ((v)->connection);
  if (!c)
    return;
  if (((c)->target) && (((c)->target) != w))
    gs_ide566 (((c)->target), v);
  if (((c)->target2) && (((c)->target2) != w))
    gs_ide566 (((c)->target2), v);
}

#ifdef ANSI_C
static void
gs_ide560 (void)
#else
static void
gs_ide560 ()
#endif
{
  GNLIST h1, h2;
  int i, j, k;;
  for (i = 0; i <= maxdepth + 1; i++)
    {
      h1 = ((layer[i]).succlist);
      j = 0;
      while (h1)
	{
	  ((((h1)->node))->position) = j++;
	  h1 = ((h1)->next);
	}
    }
  for (i = 0; i <= maxdepth + 1; i++)
    {
      h1 = ((layer[i]).succlist);
      ((layer[i]).predlist) = NULL;
      k = 0;
      while (h1)
	{
	  k++;
	  h2 = tmpnodelist_alloc ();
	  ((h2)->next) = ((layer[i]).predlist);
	  ((layer[i]).predlist) = h2;
	  ((h2)->node) = ((h1)->node);
	  gs_ide576 (((h1)->node));
	  h1 = ((h1)->next);
	}
      assert ((((layer[i]).anz) == k));
    }
}

#ifdef ANSI_C
static void
gs_ide576 (GNODE v)
#else
static void
gs_ide576 (v)
     GNODE v;
#endif
{
  int i;
  ADJEDGE a;;
  assert ((v));
  i = 0;
  a = ((v)->pred);
  while (a)
    {
      gs_ide505[i++] = ((a)->kante);
      a = ((a)->next);
    }
#ifdef ANSI_C
  qsort (gs_ide505, ((v)->indegree), sizeof (GNODE),
	 (int (*)(const void *, const void *)) gs_ide516);
#else
  qsort (gs_ide505, ((v)->indegree), sizeof (GNODE), gs_ide516);
#endif
  i = 0;
  a = ((v)->pred);
  while (a)
    {
      ((a)->kante) = gs_ide505[i++];
      a = ((a)->next);
    }
  ((v)->predleft) = ((v)->predright) = 0;
  if (i)
    {
      ((v)->predleft) = gs_ide505[0];
      ((v)->predright) = gs_ide505[i - 1];
    }
  i = 0;
  a = ((v)->succ);
  while (a)
    {
      gs_ide505[i++] = ((a)->kante);
      a = ((a)->next);
    }
#ifdef ANSI_C
  qsort (gs_ide505, ((v)->outdegree), sizeof (GNODE),
	 (int (*)(const void *, const void *)) gs_ide517);
#else
  qsort (gs_ide505, ((v)->outdegree), sizeof (GNODE), gs_ide517);
#endif
  i = 0;
  a = ((v)->succ);
  while (a)
    {
      ((a)->kante) = gs_ide505[i++];
      a = ((a)->next);
    }
  ((v)->succleft) = ((v)->succright) = 0;
  if (i)
    {
      ((v)->succleft) = gs_ide505[0];
      ((v)->succright) = gs_ide505[i - 1];
    }
}

#ifdef ANSI_C
static int
gs_ide516 (const GEDGE * a, const GEDGE * b)
#else
static int
gs_ide516 (a, b)
     GEDGE *a;
     GEDGE *b;
#endif
{
  if (((((*a)->start))->position) > ((((*b)->start))->position))
    return (1);
  if (((((*a)->start))->position) < ((((*b)->start))->position))
    return (-1);
  return (0);
}

#ifdef ANSI_C
static int
gs_ide517 (const GEDGE * a, const GEDGE * b)
#else
static int
gs_ide517 (a, b)
     GEDGE *a;
     GEDGE *b;
#endif
{
  if (((((*a)->end))->position) > ((((*b)->end))->position))
    return (1);
  if (((((*a)->end))->position) < ((((*b)->end))->position))
    return (-1);
  return (0);
}

#ifdef DEBUG
#ifdef ANSI_C
void
db_output_all_layers (void)
#else
void
db_output_all_layers ()
#endif
{
  int i;
  for (i = 0; i <= maxdepth + 1; i++)
    db_output_layer (i);
  PRINTF ("\n");
}
#endif
#ifdef DEBUG
#ifdef ANSI_C
void
db_output_layer (int i)
#else
void
db_output_layer (i)
     int i;
#endif
{
  GNLIST li;
  ADJEDGE li2;
  PRINTF ("layer[%d]: ", i);
  li = ((gs_ide580[i]).succlist);
  while (li)
    {
      if (((((li)->node))->title)[0])
	PRINTF ("%s[", ((((li)->node))->title));
      else
	PRINTF ("?[");
      li2 = ((((li)->node))->pred);
      while (li2)
	{
	  if ((((((((li2)->kante))->start)))->title)[0])
	    PRINTF ("%s,", (((((((li2)->kante))->start)))->title));
	  else
	    PRINTF ("?,");
	  li2 = ((li2)->next);
	}
      PRINTF ("][");
      li2 = ((((li)->node))->succ);
      while (li2)
	{
	  if ((((((((li2)->kante))->end)))->title)[0])
	    PRINTF ("%s,", (((((((li2)->kante))->end)))->title));
	  else
	    PRINTF ("?,");
	  li2 = ((li2)->next);
	}
      PRINTF ("]");
      PRINTF ("b(%f)p(%d) ", ((((li)->node))->bary),
	      ((((li)->node))->position));
      li = ((li)->next);
    }
  PRINTF ("\n");
}
#endif
#ifdef DEBUG
#ifdef ANSI_C
int
db_check_proper (GNODE v, int level)
#else
int
db_check_proper (v, level)
     GNODE v;
     int level;
#endif
{
  int t;
  char *title, *st, *tt;
  ADJEDGE li;
  if (((v)->title)[0])
    title = ((v)->title);
  else
    title = "?";
  t = ((v)->tiefe);
  if (level != t)
    PRINTF ("%s at level %d, expected %d\n", title, t, level);
  li = ((v)->succ);
  while (li)
    {
      if (!((li)->kante))
	{
	  PRINTF ("%s missing edge\n", title);
	  break;
	}
      if (!(((((li)->kante))->start)))
	{
	  PRINTF ("Succedge at %s missing source\n", title);
	  break;
	}
      if (!(((((li)->kante))->end)))
	{
	  PRINTF ("Succedge at %s missing source\n", title);
	  break;
	}
      if ((((((((li)->kante))->start)))->title)[0])
	st = (((((((li)->kante))->start)))->title);
      else
	st = "?";
      if ((((((((li)->kante))->end)))->title)[0])
	tt = (((((((li)->kante))->end)))->title);
      else
	tt = "?";
      if ((((((li)->kante))->start)) != v)
	{
	  PRINTF ("Succedge (%s,%s) at %s wrong source\n", st, tt, title);
	  break;
	}
      if ((((((((li)->kante))->end)))->tiefe) != t + 1)
	{
	  PRINTF ("Succedge (%s,%s) depth %d (%d expected)\n", st, tt,
		  (((((((li)->kante))->end)))->tiefe), t + 1);
	  break;
	}
      li = ((li)->next);
    }
  li = ((v)->pred);
  while (li)
    {
      if (!((li)->kante))
	{
	  PRINTF ("%s missing edge\n", title);
	  break;
	}
      if (!(((((li)->kante))->start)))
	{
	  PRINTF ("Prededge at %s missing source\n", title);
	  break;
	}
      if (!(((((li)->kante))->end)))
	{
	  PRINTF ("Prededge at %s missing source\n", title);
	  break;
	}
      if ((((((((li)->kante))->start)))->title)[0])
	st = (((((((li)->kante))->start)))->title);
      else
	st = "?";
      if ((((((((li)->kante))->end)))->title)[0])
	tt = (((((((li)->kante))->end)))->title);
      else
	tt = "?";
      if ((((((li)->kante))->end)) != v)
	{
	  PRINTF ("Prededge (%s,%s) at %s wrong target\n", st, tt, title);
	  break;
	}
      if ((((((((li)->kante))->start)))->tiefe) != t - 1)
	{
	  PRINTF ("Succedge (%s,%s) depth %d (%d expected)\n", st, tt,
		  (((((((li)->kante))->start)))->tiefe), t - 1);
	  break;
	}
      li = ((li)->next);
    }
  return (1);
}
#endif
