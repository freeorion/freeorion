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
static void gs_ide1508 _PP ((void));
static void gs_ide1515 _PP ((int le, int mr, int miy, int may));
static int gs_ide1516 _PP ((int level));
static void gs_ide1518 _PP ((GNODE v, int xpos));
static void gs_ide1517 _PP ((GNODE v, int xpos));
static void gs_ide1514 _PP ((DLLIST x));
static void gs_ide1513 _PP ((DLLIST x));
static void gs_ide1501 _PP ((GEDGE e, GNODE n));
static void gs_ide1500 _PP ((GEDGE e, GNODE n));
static void gs_ide1503 _PP ((void));
static void gs_ide1502 _PP ((void));
static void gs_ide1537 _PP ((void));
static int gs_ide1509 _PP ((GNLIST li));
static int gs_ide1505 _PP ((GNLIST li));
static int gs_ide1507 _PP ((GEDGE e, GNLIST li));
static int gs_ide1506 _PP ((GEDGE e, GNLIST li));
static int gs_ide1534 _PP ((GNLIST li, int bendp));
static int gs_ide1533 _PP ((GNLIST li, int bendp));
static void gs_ide1504 _PP ((void));
static void gs_ide1511 _PP ((GEDGE e));
static void gs_ide1512 _PP ((ADJEDGE a));
static void gs_ide1510 _PP ((ADJEDGE a));
static void gs_ide1521 _PP ((void));
static void gs_ide1519 _PP ((GNODE v));
static void gs_ide1520 _PP ((GEDGE e));
static void gs_ide1524 _PP ((void));
static void gs_ide1522 _PP ((GNODE v));
static void gs_ide1523 _PP ((GEDGE e));
#ifdef ANSI_C
void
step4_main (void)
#else
void
step4_main ()
#endif
{
  start_time ();;
  assert ((layer));
  gs_ide1503 ();
  if (manhatten_edges != 1)
    {
      gs_ide1502 ();
      gs_ide1537 ();
    }
  else
    {
      gs_ide1508 ();
    }
  gs_ide1504 ();
  if ((G_orientation == 3) || (G_orientation == 2))
    gs_ide1524 ();
  if ((G_orientation == 1) || (G_orientation == 2))
    gs_ide1521 ();
  calc_max_xy_pos ();
  stop_time ("step4_main");
}

#ifdef ANSI_C
void
calc_all_ports (int xypos_avail)
#else
void
calc_all_ports (xypos_avail)
     int xypos_avail;
#endif
{
  GNODE v;;
  v = nodelist;
  while (v)
    {
      calc_node_ports (v, xypos_avail);
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      calc_node_ports (v, xypos_avail);
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      calc_node_ports (v, xypos_avail);
      v = ((v)->next);
    }
}

#ifdef ANSI_C
void
calc_node_ports (GNODE v, int xypos_avail)
#else
void
calc_node_ports (v, xypos_avail)
     GNODE v;
     int xypos_avail;
#endif
{
  int act_port;
  int act_pcol, act_pstyle, act_psize;
  int pcol, pstyle, psize;
  int midport, nullport, portpos;
  ADJEDGE a;;
  act_port = 1;
  if (G_portsharing != 1)
    act_port = 0;
  if (((v)->anchordummy))
    act_port = 0;
  a = ((v)->succ);
  if (a)
    {
      act_pstyle = ((((a)->kante))->arrowstyle2);
      act_pcol = ((((a)->kante))->arrowcolor2);
      act_psize = ((((a)->kante))->arrowsize2);
      if ((((((a)->kante))->kantenart)) == 'r')
	act_pstyle = -1;
      else if ((((((a)->kante))->kantenart)) == 'l')
	act_pstyle = -1;
    }
  while (a)
    {
      pstyle = ((((a)->kante))->arrowstyle2);
      pcol = ((((a)->kante))->arrowcolor2);
      psize = ((((a)->kante))->arrowsize2);
      if ((((((a)->kante))->kantenart)) == 'r')
	pstyle = -1;
      else if ((((((a)->kante))->kantenart)) == 'l')
	pstyle = -1;
      if (((v)->anchordummy))
	pstyle = 5 + (act_pstyle + 1) % 20;
      if (G_portsharing != 1)
	pstyle = 5 + (act_pstyle + 1) % 20;
      if ((psize == act_psize) && (pcol == act_pcol)
	  && (pstyle == act_pstyle))
	{
	  if (pstyle != -1)
	    ((((a)->kante))->weights) = act_port;
	}
      else
	{
	  act_pstyle = pstyle;
	  act_psize = psize;
	  act_pcol = pcol;
	  if (pstyle != -1)
	    {
	      act_port++;
	      ((((a)->kante))->weights) = act_port;
	    }
	}
      a = ((a)->next);
    }
  ((v)->weights) = act_port;
  if (xypos_avail)
    {
      switch (((v)->shape))
	{
	case 2:
	case 1:
	  midport = 1;
	  nullport = 0;
	  a = ((v)->succ);
	  while (a)
	    {
	      if ((((((((a)->kante))->end)))->xloc) +
		  (((((((a)->kante))->end)))->width) < ((v)->xloc))
		portpos = -1;
	      else if ((((((((a)->kante))->end)))->xloc) >
		       ((v)->xloc) + ((v)->width))
		portpos = 1;
	      else
		portpos = 0;
	      if (portpos < 0)
		midport = ((((a)->kante))->weights);
	      if (portpos == 0)
		{
		  if (!nullport)
		    midport = nullport = ((((a)->kante))->weights);
		  else
		    midport = (((((a)->kante))->weights) + nullport) / 2;
		}
	      else
		nullport = 0;
	      a = ((a)->next);
	    }
	  if (act_port - midport > midport - 1)
	    portpos = 2 * act_port - 3 * midport + 1;
	  else
	    portpos = midport - 1;
	  ((v)->weights) = 2 * (midport + portpos) - 1;
	  if (portpos)
	    {
	      a = ((v)->succ);
	      while (a)
		{
		  ((((a)->kante))->weights) += portpos;
		  a = ((a)->next);
		}
	    }
	  break;
	}
    }
  act_port = 1;
  if (((v)->anchordummy))
    act_port = 0;
  if (G_portsharing != 1)
    act_port = 0;
  a = ((v)->pred);
  if (a)
    {
      act_pstyle = ((((a)->kante))->arrowstyle1);
      act_pcol = ((((a)->kante))->arrowcolor1);
      act_psize = ((((a)->kante))->arrowsize1);
      if ((((((a)->kante))->kantenart)) == 'r')
	act_pstyle = -1;
      else if ((((((a)->kante))->kantenart)) == 'l')
	act_pstyle = -1;
    }
  while (a)
    {
      pstyle = ((((a)->kante))->arrowstyle1);
      pcol = ((((a)->kante))->arrowcolor1);
      psize = ((((a)->kante))->arrowsize1);
      if ((((((a)->kante))->kantenart)) == 'r')
	pstyle = -1;
      else if ((((((a)->kante))->kantenart)) == 'l')
	pstyle = -1;
      if (((v)->anchordummy))
	pstyle = 5 + (act_pstyle + 1) % 20;
      if (G_portsharing != 1)
	pstyle = 5 + (act_pstyle + 1) % 20;
      if ((psize == act_psize) && (pcol == act_pcol)
	  && (pstyle == act_pstyle))
	{
	  if (pstyle != -1)
	    ((((a)->kante))->weightp) = act_port;
	}
      else
	{
	  act_pstyle = pstyle;
	  act_psize = psize;
	  act_pcol = pcol;
	  if (pstyle != -1)
	    {
	      act_port++;
	      ((((a)->kante))->weightp) = act_port;
	    }
	}
      a = ((a)->next);
    }
  ((v)->weightp) = act_port;
  if (xypos_avail)
    {
      switch (((v)->shape))
	{
	case 2:
	case 3:
	case 1:
	  midport = 1;
	  nullport = 0;
	  a = ((v)->pred);
	  while (a)
	    {
	      if ((((((((a)->kante))->start)))->xloc) +
		  (((((((a)->kante))->start)))->width) < ((v)->xloc))
		portpos = -1;
	      else if ((((((((a)->kante))->start)))->xloc) >
		       ((v)->xloc) + ((v)->width))
		portpos = 1;
	      else
		portpos = 0;
	      if (portpos < 0)
		midport = ((((a)->kante))->weightp);
	      if (portpos == 0)
		{
		  if (!nullport)
		    midport = nullport = ((((a)->kante))->weightp);
		  else
		    midport = (((((a)->kante))->weightp) + nullport) / 2;
		}
	      else
		nullport = 0;
	      a = ((a)->next);
	    }
	  if (act_port - midport > midport - 1)
	    portpos = 2 * act_port - 3 * midport + 1;
	  else
	    portpos = midport - 1;
	  ((v)->weightp) = 2 * (midport + portpos) - 1;
	  if (portpos)
	    {
	      a = ((v)->pred);
	      while (a)
		{
		  ((((a)->kante))->weightp) += portpos;
		  a = ((a)->next);
		}
	    }
	  break;
	}
    }
}

#ifdef ANSI_C
static void
gs_ide1503 (void)
#else
static void
gs_ide1503 ()
#endif
{
  GNODE v;;
  v = nodelist;
  while (v)
    {
      calc_edge_xy (v);
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      calc_edge_xy (v);
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      calc_edge_xy (v);
      v = ((v)->next);
    }
}

#ifdef ANSI_C
void
calc_edge_xy (GNODE v)
#else
void
calc_edge_xy (v)
     GNODE v;
#endif
{
  int node_x, node_y, node_width, node_height;
  int node_predports, node_succports, dist, dist1;
  CONNECT c;
  GEDGE e;
  ADJEDGE a;;
  assert ((v));
  node_width = ((v)->width);
  node_height = ((v)->height);
  node_x = ((v)->xloc);
  node_y = ((v)->yloc);
  node_predports = ((v)->weightp);
  node_succports = ((v)->weights);
  switch (((v)->shape))
    {
    case 3:
      dist1 = node_width / 4;
      break;
    default:
      dist1 = 0;
    }
  c = ((v)->connection);
  if (c)
    {
      if (((c)->target))
	{
	  e = ((c)->edge);
	  if (((e)->start) == v)
	    {
	      ((e)->btyloc) = ((e)->syloc) = node_y + node_height / 2;
	      if (((((e)->end))->xloc) < node_x)
		((e)->btxloc) = ((e)->sxloc) = node_x + dist1;
	      else
		((e)->btxloc) = ((e)->sxloc) = node_x - dist1 + node_width;
	    }
	  else
	    {
	      ((e)->bbyloc) = ((e)->eyloc) = node_y + node_height / 2;
	      if (((((e)->start))->xloc) < node_x)
		((e)->bbxloc) = ((e)->exloc) = node_x + dist1;
	      else
		((e)->bbxloc) = ((e)->exloc) = node_x - dist1 + node_width;
	    }
	}
      if (((c)->target2))
	{
	  e = ((c)->edge2);
	  if (((e)->start) == v)
	    {
	      ((e)->btyloc) = ((e)->syloc) = node_y + node_height / 2;
	      if (((((e)->end))->xloc) < node_x)
		((e)->btxloc) = ((e)->sxloc) = node_x + dist1;
	      else
		((e)->btxloc) = ((e)->sxloc) = node_x - dist1 + node_width;
	    }
	  else
	    {
	      ((e)->bbyloc) = ((e)->eyloc) = node_y + node_height / 2;
	      if (((((e)->start))->xloc) < node_x)
		((e)->bbxloc) = ((e)->exloc) = node_x + dist1;
	      else
		((e)->bbxloc) = ((e)->exloc) = node_x - dist1 + node_width;
	    }
	}
    }
  a = ((v)->succ);
  while (a)
    {
      e = ((a)->kante);
      switch (((e)->kantenart))
	{
	case 'l':
	  ((e)->btyloc) = ((e)->syloc) = node_y + node_height / 2;
	  ((e)->btxloc) = ((e)->sxloc) = node_x + dist1;
	  break;
	case 'r':
	  ((e)->btyloc) = ((e)->syloc) = node_y + node_height / 2;
	  ((e)->btxloc) = ((e)->sxloc) = node_x + node_width - dist1;
	  break;
	default:
	  ((e)->btxloc) = ((e)->sxloc) =
	    node_x + node_width * ((e)->weights) / (node_succports + 1);
	  switch (((v)->shape))
	    {
	    case 1:
	      if (((e)->sxloc) - node_x < node_width / 2)
		dist =
		  ((node_width + 1) / 2 - ((e)->sxloc) +
		   node_x) * node_height / node_width;
	      else
		dist =
		  (-(node_width + 1) / 2 + ((e)->sxloc) -
		   node_x) * node_height / node_width;
	      ((e)->syloc) = node_y + node_height - dist;
	      break;
	    case 2:
	      dist =
		node_height / 2 -
		gstoint (sqrt
			 ((double) (node_height * node_height) / 4.0 -
			  (double) (node_height * node_height) /
			  (double) (node_width * node_width) * (((e)->sxloc) -
								node_x -
								(double)
								node_width /
								2.0) *
			  (((e)->sxloc) - node_x -
			   (double) node_width / 2.0)));
	      ((e)->syloc) = node_y + node_height - dist;
	      break;
	    default:
	      ((e)->syloc) = node_y + node_height;
	    }
	  ((e)->btyloc) = ((e)->syloc);
	}
      a = ((a)->next);
    }
  a = ((v)->pred);
  while (a)
    {
      e = ((a)->kante);
      switch (((e)->kantenart))
	{
	case 'l':
	  ((e)->bbyloc) = ((e)->eyloc) = node_y + node_height / 2;
	  ((e)->bbxloc) = ((e)->exloc) = node_x + node_width - dist1;
	  break;
	case 'r':
	  ((e)->bbyloc) = ((e)->eyloc) = node_y + node_height / 2;
	  ((e)->bbxloc) = ((e)->exloc) = node_x + dist1;
	  break;
	default:
	  ((e)->bbxloc) = ((e)->exloc) =
	    node_x + node_width * ((e)->weightp) / (node_predports + 1);
	  switch (((v)->shape))
	    {
	    case 1:
	      if (((e)->exloc) - node_x < node_width / 2)
		dist =
		  ((node_width + 1) / 2 - ((e)->exloc) +
		   node_x) * node_height / node_width;
	      else
		dist =
		  (-(node_width + 1) / 2 + ((e)->exloc) -
		   node_x) * node_height / node_width;
	      ((e)->eyloc) = node_y + dist;
	      break;
	    case 2:
	      dist =
		node_height / 2 -
		gstoint (sqrt
			 ((double) (node_height * node_height) / 4.0 -
			  (double) (node_height * node_height) /
			  (double) (node_width * node_width) * (((e)->exloc) -
								node_x -
								(double)
								node_width /
								2.0) *
			  (((e)->exloc) - node_x -
			   (double) node_width / 2.0)));
	      ((e)->eyloc) = node_y + dist;
	      break;
	    case 3:
	      if (((e)->exloc) - node_x < node_width / 2)
		dist =
		  ((node_width + 1) / 2 - ((e)->exloc) +
		   node_x) * node_height * 2 / node_width;
	      else
		dist =
		  (-(node_width + 1) / 2 + ((e)->exloc) -
		   node_x) * node_height * 2 / node_width;
	      ((e)->eyloc) = node_y + dist;
	      break;
	    default:
	      ((e)->eyloc) = node_y;
	    }
	  ((e)->bbyloc) = ((e)->eyloc);
	}
      a = ((a)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide1508 (void)
#else
static void
gs_ide1508 ()
#endif
{
  GNLIST li;
  int topbendp, botbendp;
  int maxrow, i;;
  assert ((layer));
  for (i = 0; i <= maxdepth; i++)
    {
      if (i % 20 == 0)
	gs_wait_message ('e');
      topbendp = 0;
      botbendp = MAXINT;
      li = ((layer[i]).predlist);
      while (li)
	{
	  tpred_connection1[((((li)->node))->position)] = li;
	  if (((((li)->node))->yloc) + ((((li)->node))->height) > topbendp)
	    topbendp = ((((li)->node))->yloc) + ((((li)->node))->height);
	  li = ((li)->next);
	}
      li = ((layer[i + 1]).predlist);
      while (li)
	{
	  tpred_connection2[((((li)->node))->position)] = li;
	  if (((((li)->node))->yloc) < botbendp)
	    botbendp = ((((li)->node))->yloc);
	  li = ((li)->next);
	}
      maxrow = gs_ide1516 (i);
      if (botbendp - topbendp > 20)
	{
	  botbendp = botbendp - 10;
	  topbendp = topbendp + 10;
	}
      gs_ide1515 (i, maxrow, topbendp, botbendp);
    }
  gs_ide1515 (maxdepth + 1, maxrow, topbendp, botbendp);
}
static int gs_ide1535;
static int gs_ide1536;
static int gs_ide1529;
static int gs_ide1531;
static int gs_ide1530;
static DLLIST gs_ide1526 = NULL;
static DLLIST gs_ide1527 = NULL;
static DLLIST gs_ide1538 = NULL;
static DLLIST gs_ide1539 = NULL;
static int gs_ide1540;
static int gs_ide1528;
#ifdef ANSI_C
static int
gs_ide1516 (int level)
#else
static int
gs_ide1516 (level)
     int level;
#endif
{
  GNLIST li1, li2;
  ADJEDGE a1, a2;;
  assert ((level >= 0));
  assert ((level <= maxdepth));
  gs_ide1536 = gs_ide1535 = 0;
  gs_ide1529 = gs_ide1531 = 0;
  gs_ide1526 = gs_ide1527 = NULL;
  gs_ide1538 = gs_ide1539 = NULL;
  gs_ide1530 = 0;
  gs_ide1540 = -1;
  gs_ide1528 = -1;
  li1 = ((layer[level]).succlist);
  li2 = ((layer[level + 1]).succlist);
  a1 = NULL;
  while (li1 && (!a1))
    {
      a1 = ((((li1)->node))->succ);
      if (!a1)
	li1 = ((li1)->next);
    }
  a2 = NULL;
  while (li2 && (!a2))
    {
      a2 = ((((li2)->node))->pred);
      if (!a2)
	li2 = ((li2)->next);
    }
  while ((li1) || (li2))
    {
      if (a1 && a2)
	{
	  if (((((a1)->kante))->sxloc) < ((((a2)->kante))->exloc))
	    {
	      gs_ide1518 (((li1)->node), ((((a1)->kante))->sxloc));
	      a1 = ((a1)->next);
	    }
	  else
	    {
	      gs_ide1517 (((li2)->node), ((((a2)->kante))->exloc));
	      a2 = ((a2)->next);
	    }
	}
      else if (a1)
	{
	  gs_ide1518 (((li1)->node), ((((a1)->kante))->sxloc));
	  a1 = ((a1)->next);
	}
      else if (a2)
	{
	  gs_ide1517 (((li2)->node), ((((a2)->kante))->exloc));
	  a2 = ((a2)->next);
	}
      if (!a1)
	{
	  if (li1)
	    li1 = ((li1)->next);
	  while (li1 && (!a1))
	    {
	      a1 = ((((li1)->node))->succ);
	      if (!a1)
		li1 = ((li1)->next);
	    }
	}
      if (!a2)
	{
	  if (li2)
	    li2 = ((li2)->next);
	  while (li2 && (!a2))
	    {
	      a2 = ((((li2)->node))->pred);
	      if (!a2)
		li2 = ((li2)->next);
	    }
	}
    }
  return (gs_ide1530);
}

#ifdef ANSI_C
static void
gs_ide1518 (GNODE v, int xpos)
#else
static void
gs_ide1518 (v, xpos)
     GNODE v;
     int xpos;
#endif
{
  ADJEDGE a;
  GEDGE ee;
  DLLIST n, m;
  int k;;
  assert ((v));
  k = gs_ide1531 + gs_ide1529;
  if (k > gs_ide1530)
    gs_ide1530 = k;
  if (xpos <= gs_ide1540)
    return;
  gs_ide1540 = xpos;
  a = ((v)->succ);
  while (a)
    {
      ee = ((a)->kante);
      if ((((ee)->sxloc) == xpos) && (((ee)->exloc) > ((ee)->sxloc)))
	{
	  gs_ide1500 (ee, (((((a)->kante))->end)));
	}
      a = ((a)->next);
    }
  k = gs_ide1531 + gs_ide1529;
  if (k > gs_ide1530)
    gs_ide1530 = k;
  n = gs_ide1538;
  while (n)
    {
      m = ((n)->succ);
      if (((n)->dlx) <= xpos)
	gs_ide1514 (n);
      n = m;
    }
  gs_ide1531 = 0;
  n = gs_ide1538;
  while (n)
    {
      if (((n)->dlinfo) > gs_ide1531)
	gs_ide1531 = ((n)->dlinfo);
      n = ((n)->succ);
    }
}

#ifdef ANSI_C
static void
gs_ide1517 (GNODE v, int xpos)
#else
static void
gs_ide1517 (v, xpos)
     GNODE v;
     int xpos;
#endif
{
  ADJEDGE a;
  GEDGE ee;
  DLLIST n, m;
  int k;;
  assert ((v));
  k = gs_ide1531 + gs_ide1529;
  if (k > gs_ide1530)
    gs_ide1530 = k;
  if (xpos <= gs_ide1528)
    return;
  gs_ide1528 = xpos;
  a = ((v)->pred);
  while (a)
    {
      ee = ((a)->kante);
      if ((((ee)->exloc) == xpos) && (((ee)->exloc) <= ((ee)->sxloc)))
	{
	  gs_ide1501 (ee, (((((a)->kante))->start)));
	}
      a = ((a)->next);
    }
  k = gs_ide1531 + gs_ide1529;
  if (k > gs_ide1530)
    gs_ide1530 = k;
  n = gs_ide1526;
  while (n)
    {
      m = ((n)->succ);
      if (((n)->dlx) <= xpos)
	gs_ide1513 (n);
      n = m;
    }
  gs_ide1529 = 0;
  n = gs_ide1526;
  while (n)
    {
      if (((n)->dlinfo) > gs_ide1529)
	gs_ide1529 = ((n)->dlinfo);
      n = ((n)->succ);
    }
}

#ifdef ANSI_C
static void
gs_ide1501 (GEDGE e, GNODE n)
#else
static void
gs_ide1501 (e, n)
     GEDGE e;
     GNODE n;
#endif
{
  DLLIST d;
  assert ((n));;
  d = dllist_alloc (n, gs_ide1539);
  if (!gs_ide1538)
    gs_ide1538 = d;
  if (gs_ide1539)
    ((gs_ide1539)->succ) = d;
  gs_ide1539 = d;
  gs_ide1536++;
  if (((e)->sxloc) != ((e)->exloc))
    gs_ide1531++;
  ((d)->dlinfo) = gs_ide1531;
  ((e)->btyloc) = gs_ide1531;
  ((d)->dlx) = ((e)->sxloc);
}

#ifdef ANSI_C
static void
gs_ide1500 (GEDGE e, GNODE n)
#else
static void
gs_ide1500 (e, n)
     GEDGE e;
     GNODE n;
#endif
{
  DLLIST d;
  assert ((n));;
  d = dllist_alloc (n, gs_ide1527);
  if (!gs_ide1526)
    gs_ide1526 = d;
  if (gs_ide1527)
    ((gs_ide1527)->succ) = d;
  gs_ide1527 = d;
  gs_ide1535++;
  if (((e)->sxloc) != ((e)->exloc))
    gs_ide1529++;
  ((d)->dlinfo) = gs_ide1529;
  ((e)->btyloc) = gs_ide1529;
  ((d)->dlx) = ((e)->exloc);
}

#ifdef ANSI_C
static void
gs_ide1514 (DLLIST x)
#else
static void
gs_ide1514 (x)
     DLLIST x;
#endif
{
  assert ((x));
  assert ((((x)->node)));;
  if (((x)->pred))
    ((((x)->pred))->succ) = ((x)->succ);
  else
    gs_ide1538 = ((x)->succ);
  if (((x)->succ))
    ((((x)->succ))->pred) = ((x)->pred);
  else
    gs_ide1539 = ((x)->pred);
  dllist_free (x);
  gs_ide1536--;
}

#ifdef ANSI_C
static void
gs_ide1513 (DLLIST x)
#else
static void
gs_ide1513 (x)
     DLLIST x;
#endif
{
  assert ((x));
  assert ((((x)->node)));;
  if (((x)->pred))
    ((((x)->pred))->succ) = ((x)->succ);
  else
    gs_ide1526 = ((x)->succ);
  if (((x)->succ))
    ((((x)->succ))->pred) = ((x)->pred);
  else
    gs_ide1527 = ((x)->pred);
  dllist_free (x);
  gs_ide1535--;
}

#ifdef ANSI_C
static void
gs_ide1515 (int level, int maxr, int miny, int maxy)
#else
static void
gs_ide1515 (level, maxr, miny, maxy)
     int level, maxr, miny, maxy;
#endif
{
  GNLIST li;
  ADJEDGE a;
  GEDGE ee;
  GNODE node;
  int k, k1, k2;;
  li = ((layer[level]).succlist);
  while (li)
    {
      a = ((((li)->node))->succ);
      while (a)
	{
	  ee = ((a)->kante);
	  if (one_line_manhatten == 1)
	    {
	      k = maxy - (maxy - miny) / 2;
	    }
	  else
	    {
	      if (((ee)->sxloc) >= ((ee)->exloc))
		k =
		  maxy - (maxy - miny) * (maxr + 1 - ((ee)->btyloc)) / (maxr +
									1);
	      else
		k =
		  miny + (maxy - miny) * (maxr + 1 - ((ee)->btyloc)) / (maxr +
									1);
	    }
	  ((ee)->btyloc) = k;
	  ((ee)->bbyloc) = k;
	  a = ((a)->next);
	}
      li = ((li)->next);
    }
  li = ((layer[level]).succlist);
  while (li)
    {
      node = ((li)->node);
      if ((((node)->width) == 0) && (((node)->height) == 0)
	  && (((node)->succ)) && (((((node)->succ))->next))
	  && (((((((node)->succ))->next))->next) == NULL)
	  && ((((node)->pred)) == NULL))
	{
	  k1 = ((((((node)->succ))->kante))->btyloc);
	  k2 = ((((((((node)->succ))->next))->kante))->btyloc);
	  if (k1 < k2)
	    k = k1;
	  else
	    k = k2;
	  ((node)->yloc) = k;
	  ((((((node)->succ))->kante))->syloc) = k;
	  ((((((((node)->succ))->next))->kante))->syloc) = k;
	}
      if ((((node)->width) == 0) && (((node)->height) == 0)
	  && (((node)->pred)) && (((((node)->pred))->next))
	  && (((((((node)->pred))->next))->next) == NULL)
	  && ((((node)->succ)) == NULL))
	{
	  k1 = ((((((node)->pred))->kante))->bbyloc);
	  k2 = ((((((((node)->pred))->next))->kante))->bbyloc);
	  if (k1 > k2)
	    k = k1;
	  else
	    k = k2;
	  ((node)->yloc) = k;
	  ((((((node)->pred))->kante))->eyloc) = k;
	  ((((((((node)->pred))->next))->kante))->eyloc) = k;
	}
      li = ((li)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide1502 (void)
#else
static void
gs_ide1502 ()
#endif
{
  int i, j;
  GNLIST li;
  GNODE node;
  int h, changed;
  int topbendp;
  int botbendp;
  int minx, maxx;;
  assert ((layer));
  for (i = 0; i <= maxdepth; i++)
    {
      if (i % 10 == 0)
	gs_wait_message ('e');
      li = ((layer[i]).succlist);
      minx = MAXINT;
      maxx = 0;
      while (li)
	{
	  j = ((((li)->node))->yloc);
	  if (j < minx)
	    minx = j;
	  j = j + ((((li)->node))->height);
	  if (j > maxx)
	    maxx = j;
	  li = ((li)->next);
	}
      li = ((layer[i]).succlist);
      while (li)
	{
	  node = ((li)->node);
	  if ((((node)->width) == 0) && (((node)->height) == 0)
	      && (((node)->succ)) && (((((node)->succ))->next))
	      && (((((((node)->succ))->next))->next) == NULL)
	      && ((((node)->pred)) == NULL))
	    {
	      ((node)->yloc) = maxx;
	      ((((((node)->succ))->kante))->syloc) = maxx;
	      ((((((node)->succ))->kante))->btyloc) = maxx;
	      ((((((((node)->succ))->next))->kante))->syloc) = maxx;
	      ((((((((node)->succ))->next))->kante))->btyloc) = maxx;
	    }
	  if ((((node)->width) == 0) && (((node)->height) == 0)
	      && (((node)->pred)) && (((((node)->pred))->next))
	      && (((((((node)->pred))->next))->next) == NULL)
	      && ((((node)->succ)) == NULL))
	    {
	      ((node)->yloc) = minx;
	      ((((((node)->pred))->kante))->eyloc) = minx;
	      ((((((node)->pred))->kante))->bbyloc) = minx;
	      ((((((((node)->pred))->next))->kante))->eyloc) = minx;
	      ((((((((node)->pred))->next))->kante))->bbyloc) = minx;
	    }
	  li = ((li)->next);
	}
      topbendp = 0;
      botbendp = MAXINT;
      li = ((layer[i]).predlist);
      while (li)
	{
	  tpred_connection1[((((li)->node))->position)] = li;
	  if (((((li)->node))->yloc) + ((((li)->node))->height) > topbendp)
	    topbendp = ((((li)->node))->yloc) + ((((li)->node))->height);
	  li = ((li)->next);
	}
      li = ((layer[i + 1]).predlist);
      while (li)
	{
	  tpred_connection2[((((li)->node))->position)] = li;
	  if (((((li)->node))->yloc) < botbendp)
	    botbendp = ((((li)->node))->yloc);
	  li = ((li)->next);
	}
      j = 1;
      changed = 1;
      while (changed)
	{
	  j++;
	  if (j > max_edgebendings)
	    {
	      gs_wait_message ('t');
	      break;
	    }
	  if (G_timelimit > 0)
	    if (test_timelimit (100))
	      {
		gs_wait_message ('t');
		break;
	      }
	  changed = 0;
	  li = ((layer[i]).succlist);
	  while (li)
	    {
	      h = gs_ide1509 (li);
	      changed += gs_ide1534 (li, h);
	      li = ((li)->next);
	    }
	  li = ((layer[i + 1]).succlist);
	  while (li)
	    {
	      h = gs_ide1505 (li);
	      changed += gs_ide1533 (li, h);
	      li = ((li)->next);
	    }
	}
      if (changed)
	{
	  li = ((layer[i]).succlist);
	  while (li)
	    {
	      (void) gs_ide1534 (li, topbendp);
	      li = ((li)->next);
	    } li = ((layer[i + 1]).succlist);
	  while (li)
	    {
	      (void) gs_ide1533 (li, botbendp);
	      li = ((li)->next);
	}}
}}

#ifdef ANSI_C
static int
gs_ide1534 (GNLIST li, int bendp)
#else
static int
gs_ide1534 (li, bendp)
     GNLIST li;
     int bendp;
#endif
{
  ADJEDGE a;
  int changed;;
  assert ((li));
  assert ((((li)->node)));
  changed = 0;
  a = ((((li)->node))->succ);
  while (a)
    {
      if (((((a)->kante))->btyloc) < bendp)
	{
	  ((((a)->kante))->btyloc) = bendp;
	  changed = 1;
	}
      a = ((a)->next);
    }
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1533 (GNLIST li, int bendp)
#else
static int
gs_ide1533 (li, bendp)
     GNLIST li;
     int bendp;
#endif
{
  ADJEDGE a;
  int changed;;
  assert ((li));
  assert ((((li)->node)));
  changed = 0;
  a = ((((li)->node))->pred);
  while (a)
    {
      if (((((a)->kante))->bbyloc) > bendp)
	{
	  ((((a)->kante))->bbyloc) = bendp;
	  changed = 1;
	}
      a = ((a)->next);
    }
  return (changed);
}

#ifdef ANSI_C
static int
gs_ide1509 (GNLIST li)
#else
static int
gs_ide1509 (li)
     GNLIST li;
#endif
{
  int bendp, h;
  GEDGE e;;
  assert ((li));
  assert ((((li)->node)));
  bendp = 0;
  e = ((((li)->node))->succleft);
  if (!e)
    return (bendp);
  bendp = gs_ide1507 (e, li);
  e = ((((li)->node))->succright);
  assert ((e));
  if (e == ((((li)->node))->succleft))
    return (bendp);
  h = gs_ide1507 (e, li);
  if (h > bendp)
    bendp = h;
  return (bendp);
}

#define bendformula  ((sx-tx)*(ky-ty))/(kx-tx)+ty
#ifdef ANSI_C
static int
gs_ide1507 (GEDGE e, GNLIST li)
#else
static int
gs_ide1507 (e, li)
     GEDGE e;
     GNLIST li;
#endif
{
  int bendp, h;
  GEDGE e2;
  int sx, sy, tx, ty;
  int kx, ky;
  GNODE node;
  int offset, myoff, cross;
  GNLIST li2;;
  bendp = 0;
  sx = ((e)->sxloc);
  sy = ((e)->syloc);
  tx = ((e)->bbxloc);
  ty = ((e)->bbyloc);
  offset = 7 * ((e)->arrowsize2) / 10 + 2;
  if (sx < tx)
    {
      li2 = ((li)->next);
      while (li2)
	{
	  node = ((li2)->node);
	  if (!((node)->anchordummy))
	    {
	      kx = ((node)->xloc);
	      if (kx >= tx)
		break;
	      if (((node)->shape) == 1)
		{
		  kx -= 3;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) / 2 + myoff;
		  if (kx >= sx)
		    {
		      h = bendformula;
		      if (h > bendp)
			bendp = h;
		    }
		  kx += ((node)->width) / 2;
		  if (kx >= tx)
		    break;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) + myoff;
		  if (kx >= sx)
		    {
		      h = bendformula;
		      if (h > bendp)
			bendp = h;
		    }
		}
	      else
		{
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) + myoff;
		  if (kx >= sx)
		    {
		      h = bendformula;
		      if (h > bendp)
			bendp = h;
		    }
		}
	    }
	  e2 = ((node)->succleft);
	  if ((e2) && (((e2)->btyloc) != ((e2)->syloc)))
	    {
	      cross = 0;
	      if ((((((e2)->start))->position) < ((((e)->start))->position))
		  && (((((e2)->end))->position) > ((((e)->end))->position)))
		cross = 1;
	      if ((((((e2)->start))->position) > ((((e)->start))->position))
		  && (((((e2)->end))->position) < ((((e)->end))->position)))
		cross = 1;
	      kx = ((e2)->btxloc) - G_dspace + 3;
	      if (kx >= tx)
		break;
	      ky = ((e2)->btyloc);
	      if ((kx >= sx) && (!cross))
		{
		  h = bendformula;
		  if (h > bendp)
		    bendp = h;
		}
	    }
	  li2 = ((li2)->next);
	}
    }
  else if (tx < sx)
    {
      li2 = tpred_connection1[((((li)->node))->position)];
      li2 = ((li2)->next);
      while (li2)
	{
	  node = ((li2)->node);
	  if (!((node)->anchordummy))
	    {
	      kx = ((node)->xloc) + ((node)->width);
	      if (kx <= tx)
		break;
	      if (((node)->shape) == 1)
		{
		  kx += 3;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) / 2 + myoff;
		  if (kx <= sx)
		    {
		      h = bendformula;
		      if (h > bendp)
			bendp = h;
		    }
		  kx -= ((node)->width) / 2;
		  if (kx <= tx)
		    break;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) + myoff;
		  if (kx <= sx)
		    {
		      h = bendformula;
		      if (h > bendp)
			bendp = h;
		    }
		}
	      else
		{
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) + myoff;
		  if (kx <= sx)
		    {
		      h = bendformula;
		      if (h > bendp)
			bendp = h;
		    }
		}
	    }
	  e2 = ((node)->succright);
	  if ((e2) && (((e2)->btyloc) != ((e2)->syloc)))
	    {
	      cross = 0;
	      if ((((((e2)->start))->position) < ((((e)->start))->position))
		  && (((((e2)->end))->position) > ((((e)->end))->position)))
		cross = 1;
	      if ((((((e2)->start))->position) > ((((e)->start))->position))
		  && (((((e2)->end))->position) < ((((e)->end))->position)))
		cross = 1;
	      kx = ((e2)->btxloc) + G_dspace - 3;
	      if (kx <= tx)
		break;
	      ky = ((e2)->btyloc);
	      if ((kx <= sx) && (!cross))
		{
		  h = bendformula;
		  if (h > bendp)
		    bendp = h;
		}
	    }
	  li2 = ((li2)->next);
	}
    }
  return (bendp);
}

#ifdef ANSI_C
static int
gs_ide1505 (GNLIST li)
#else
static int
gs_ide1505 (li)
     GNLIST li;
#endif
{
  int bendp, h;
  GEDGE e;;
  assert ((li));
  assert ((((li)->node)));
  bendp = MAXINT;
  e = ((((li)->node))->predleft);
  if (!e)
    return (bendp);
  bendp = gs_ide1506 (e, li);
  e = ((((li)->node))->predright);
  assert ((e));
  if (e == ((((li)->node))->predleft))
    return (bendp);
  h = gs_ide1506 (e, li);
  if (h < bendp)
    bendp = h;
  return (bendp);
}

#ifdef ANSI_C
static int
gs_ide1506 (GEDGE e, GNLIST li)
#else
static int
gs_ide1506 (e, li)
     GEDGE e;
     GNLIST li;
#endif
{
  int bendp, h;
  GEDGE e2;
  int sx, sy, tx, ty;
  int kx, ky;
  GNODE node;
  int offset, myoff, cross;
  GNLIST li2;;
  bendp = MAXINT;
  sx = ((e)->exloc);
  sy = ((e)->eyloc);
  tx = ((e)->btxloc);
  ty = ((e)->btyloc);
  offset = 7 * ((e)->arrowsize1) / 10 + 2;
  if (sx < tx)
    {
      li2 = ((li)->next);
      while (li2)
	{
	  node = ((li2)->node);
	  if (!((node)->anchordummy))
	    {
	      kx = ((node)->xloc);
	      if (kx >= tx)
		break;
	      if (((node)->shape) == 1)
		{
		  kx -= 3;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) / 2 - myoff;
		  if (kx >= sx)
		    {
		      h = bendformula;
		      if (h < bendp)
			bendp = h;
		    }
		  kx += ((node)->width) / 2;
		  if (kx >= tx)
		    break;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) - myoff;
		  if (kx >= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		}
	      else if (((node)->shape) == 3)
		{
		  kx -= 3;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) - myoff;
		  if (kx >= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		  kx += ((node)->width) / 2;
		  if (kx >= tx)
		    break;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) - myoff;
		  if (kx >= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		}
	      else
		{
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) - myoff;
		  if (kx >= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		}
	    }
	  e2 = ((node)->predleft);
	  if ((e2) && (((e2)->bbyloc) != ((e2)->eyloc)))
	    {
	      cross = 0;
	      if ((((((e2)->start))->position) < ((((e)->start))->position))
		  && (((((e2)->end))->position) > ((((e)->end))->position)))
		cross = 1;
	      if ((((((e2)->start))->position) > ((((e)->start))->position))
		  && (((((e2)->end))->position) < ((((e)->end))->position)))
		cross = 1;
	      kx = ((e2)->bbxloc) - G_dspace + 3;
	      if (kx >= tx)
		break;
	      ky = ((e2)->bbyloc);
	      if ((kx >= sx) && (!cross))
		{
		  h = bendformula;
		  if ((h > 0) && (h < bendp))
		    bendp = h;
		}
	    }
	  li2 = ((li2)->next);
	}
    }
  else if (tx < sx)
    {
      li2 = tpred_connection2[((((li)->node))->position)];
      li2 = ((li2)->next);
      while (li2)
	{
	  node = ((li2)->node);
	  if (!((node)->anchordummy))
	    {
	      kx = ((node)->xloc) + ((node)->width);
	      if (kx <= tx)
		break;
	      if (((node)->shape) == 1)
		{
		  kx += 3;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) / 2 - myoff;
		  if (kx <= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		  kx -= ((node)->width) / 2;
		  if (kx <= tx)
		    break;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) - myoff;
		  if (kx <= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		}
	      else if (((node)->shape) == 3)
		{
		  kx += 3;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) + ((node)->height) - myoff;
		  if (kx <= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		  kx -= ((node)->width) / 2;
		  if (kx <= tx)
		    break;
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) - myoff;
		  if (kx <= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		}
	      else
		{
		  myoff = (kx - sx) * offset / (tx - sx);
		  ky = ((node)->yloc) - myoff;
		  if (kx <= sx)
		    {
		      h = bendformula;
		      if ((h > 0) && (h < bendp))
			bendp = h;
		    }
		}
	    }
	  e2 = ((node)->predright);
	  if ((e2) && (((e2)->bbyloc) != ((e2)->eyloc)))
	    {
	      cross = 0;
	      if ((((((e2)->start))->position) < ((((e)->start))->position))
		  && (((((e2)->end))->position) > ((((e)->end))->position)))
		cross = 1;
	      if ((((((e2)->start))->position) > ((((e)->start))->position))
		  && (((((e2)->end))->position) < ((((e)->end))->position)))
		cross = 1;
	      kx = ((e2)->bbxloc) + G_dspace - 3;
	      if (kx <= tx)
		break;
	      ky = ((e2)->bbyloc);
	      if ((kx <= sx) && (!cross))
		{
		  h = bendformula;
		  if ((h > 0) && (h < bendp))
		    bendp = h;
		}
	    }
	  li2 = ((li2)->next);
	}
    }
  return (bendp);
}

#ifdef ANSI_C
static void
gs_ide1537 (void)
#else
static void
gs_ide1537 ()
#endif
{
  int i;
  GNLIST li, li2;
  GNODE node;
  GEDGE e1, e2, e3;
  int ax, ay, mx, my, by, b2y, okay;
  int kx, ky, h;;
  assert ((layer));
  for (i = 0; i <= maxdepth + 1; i++)
    {
      if (i % 10 == 0)
	gs_wait_message ('e');
      li = ((layer[i]).predlist);
      while (li)
	{
	  tpred_connection1[((((li)->node))->position)] = li;
	  li = ((li)->next);
	}
      li = ((layer[i]).succlist);
      while (li)
	{
	  node = ((li)->node);
	  if ((((node)->width) == 0) && (((node)->succ))
	      && (((((node)->succ))->next) == NULL) && (((node)->pred))
	      && (((((node)->pred))->next) == NULL))
	    {
	      e1 = ((((node)->pred))->kante);
	      e2 = ((((node)->succ))->kante);
	      my = ((e2)->syloc);
	      by = ((e2)->btyloc);
	      b2y = ((e1)->bbyloc);
	      ay = ((e1)->btyloc);
	      if ((b2y == ((e1)->eyloc)) && (by > my) && (by > ay))
		{
		  ax = ((e1)->btxloc);
		  mx = ((e2)->sxloc);
		  okay = 1;
		  if (((ax < mx) && (mx < ((e2)->bbxloc)))
		      || ((ax < mx) && (G_spline)))
		    {
		      li2 = tpred_connection1[((node)->position)];
		      li2 = ((li2)->next);
		      while (li2)
			{
			  if (okay == 0)
			    break;
			  if (!((((li2)->node))->anchordummy))
			    {
			      kx =
				((((li2)->node))->xloc) +
				((((li2)->node))->width);
			      if (((((li2)->node))->width) == 0)
				kx += G_dspace;
			      if (kx <= ax)
				break;
			      ky = ((((li2)->node))->yloc) - 2;
			      if (kx <= mx)
				{
				  h =
				    ((mx - kx) * (ay - by)) / (mx - ax) + by;
				  if (ky <= h)
				    okay = 0;
				}
			    }
			  e3 = ((((li2)->node))->predright);
			  if ((e3) && (((e3)->bbyloc) != ((e3)->eyloc)))
			    {
			      kx = ((e3)->bbxloc) + G_dspace - 3;
			      if (kx <= ax)
				break;
			      ky = ((e3)->bbyloc);
			      if (kx <= mx)
				{
				  h =
				    ((mx - kx) * (ay - by)) / (mx - ax) + by;
				  if (ky <= h)
				    okay = 0;
				}
			    }
			  li2 = ((li2)->next);
			}
		      if (okay)
			{
			  ((e2)->syloc) = ((e1)->bbyloc) = ((e1)->eyloc) =
			    ((e2)->btyloc);
			}
		    }
		  else if (((ax > mx) && (mx > ((e2)->bbxloc)))
			   || ((ax > mx) && (G_spline)))
		    {
		      li2 = ((li)->next);
		      while (li2)
			{
			  if (okay == 0)
			    break;
			  if (!((((li2)->node))->anchordummy))
			    {
			      kx = ((((li2)->node))->xloc);
			      if (((((li2)->node))->width) == 0)
				kx -= G_dspace;
			      if (kx >= ax)
				break;
			      ky = ((((li2)->node))->yloc) - 2;
			      if (kx >= mx)
				{
				  h =
				    ((mx - kx) * (ay - by)) / (mx - ax) + by;
				  if (ky <= h)
				    okay = 0;
				}
			    }
			  e3 = ((((li2)->node))->predleft);
			  if ((e3) && (((e3)->bbyloc) != ((e3)->eyloc)))
			    {
			      kx = ((e3)->bbxloc) - G_dspace + 3;
			      if (kx >= ax)
				break;
			      ky = ((e3)->bbyloc);
			      if (kx >= mx)
				{
				  h =
				    ((mx - kx) * (ay - by)) / (mx - ax) + by;
				  if (ky <= h)
				    okay = 0;
				}
			    }
			  li2 = ((li2)->next);
			}
		      if (okay)
			{
			  ((e2)->syloc) = ((e1)->bbyloc) = ((e1)->eyloc) =
			    ((e2)->btyloc);
			}
		    }
		}
	      ay = ((e2)->bbyloc);
	      if ((by == ((e2)->syloc)) && (b2y < my) && (b2y < ay))
		{
		  ax = ((e2)->bbxloc);
		  mx = ((e2)->sxloc);
		  by = b2y;
		  okay = 1;
		  if (((ax < mx) && (mx < ((e1)->btxloc)))
		      || ((ax < mx) && (G_spline)))
		    {
		      li2 = tpred_connection1[((node)->position)];
		      li2 = ((li2)->next);
		      while (li2)
			{
			  if (okay == 0)
			    break;
			  if (!((((li2)->node))->anchordummy))
			    {
			      kx =
				((((li2)->node))->xloc) +
				((((li2)->node))->width);
			      if (((((li2)->node))->width) == 0)
				kx += G_dspace;
			      if (kx <= ax)
				break;
			      ky =
				((((li2)->node))->yloc) +
				((((li2)->node))->height) + 2;
			      if (kx <= mx)
				{
				  h =
				    ((mx - kx) * (ay - by)) / (mx - ax) + by;
				  if (ky >= h)
				    okay = 0;
				}
			    }
			  e3 = ((((li2)->node))->succright);
			  if ((e3) && (((e3)->btyloc) != ((e3)->syloc)))
			    {
			      kx = ((e3)->btxloc) + G_dspace - 3;
			      if (kx <= ax)
				break;
			      ky = ((e3)->btyloc);
			      if (kx <= mx)
				{
				  h =
				    ((mx - kx) * (ay - by)) / (mx - ax) + by;
				  if (ky >= h)
				    okay = 0;
				}
			    }
			  li2 = ((li2)->next);
			}
		      if (okay)
			{
			  ((e2)->syloc) = ((e2)->btyloc) = ((e1)->eyloc) =
			    ((e1)->bbyloc);
			}
		    }
		  else if (((ax > mx) && (mx > ((e1)->btxloc)))
			   || ((ax > mx) && (G_spline)))
		    {
		      li2 = ((li)->next);
		      while (li2)
			{
			  if (okay == 0)
			    break;
			  if (!((((li2)->node))->anchordummy))
			    {
			      kx = ((((li2)->node))->xloc);
			      if (((((li2)->node))->width) == 0)
				kx -= G_dspace;
			      if (kx >= ax)
				break;
			      ky =
				((((li2)->node))->yloc) +
				((((li2)->node))->height) + 2;
			      if (kx >= mx)
				{
				  h =
				    ((mx - kx) * (ay - by)) / (mx - ax) + by;
				  if (ky >= h)
				    okay = 0;
				}
			    }
			  e3 = ((((li2)->node))->succleft);
			  if ((e3) && (((e3)->btyloc) != ((e3)->syloc)))
			    {
			      kx = ((e3)->btxloc) - G_dspace + 3;
			      if (kx >= ax)
				break;
			      ky = ((e3)->btyloc);
			      if (kx >= mx)
				{
				  h =
				    ((mx - kx) * (ay - by)) / (mx - ax) + by;
				  if (ky >= h)
				    okay = 0;
				}
			    }
			  li2 = ((li2)->next);
			}
		      if (okay)
			{
			  ((e2)->syloc) = ((e2)->btyloc) = ((e1)->eyloc) =
			    ((e1)->bbyloc);
			}
		    }
		}
	    }
	  li = ((li)->next);
	}
    }
}

#ifdef ANSI_C
static void
gs_ide1504 (void)
#else
static void
gs_ide1504 ()
#endif
{
  GNODE v;;
  v = nodelist;
  while (v)
    {
      calc_edgearrow (v);
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      calc_edgearrow (v);
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      calc_edgearrow (v);
      v = ((v)->next);
    }
}

#ifdef ANSI_C
void
calc_edgearrow (GNODE v)
#else
void
calc_edgearrow (v)
     GNODE v;
#endif
{
  ADJEDGE a;
  int act_port, j;
  CONNECT c;;
  c = ((v)->connection);
  if (c)
    {
      if ((((c)->target)) && (((((c)->edge))->end) == v))
	gs_ide1511 (((c)->edge));
      if ((((c)->target2)) && (((((c)->edge2))->end) == v))
	gs_ide1511 (((c)->edge2));
    }
  a = ((v)->succ);
  act_port = -1;
  while (a)
    {
      if (((((((a)->kante))->kantenart)) == 'l')
	  || ((((((a)->kante))->kantenart)) == 'r'));
      else if (act_port != ((((a)->kante))->weights))
	{
	  act_port = ((((a)->kante))->weights);
	  gs_ide1512 (a);
	}
      a = ((a)->next);
    }
  a = ((v)->pred);
  act_port = -1;
  while (a)
    {
      if (((((((a)->kante))->kantenart)) == 'l')
	  || ((((((a)->kante))->kantenart)) == 'r'))
	gs_ide1511 (((a)->kante));
      else if (act_port != ((((a)->kante))->weightp))
	{
	  act_port = ((((a)->kante))->weightp);
	  gs_ide1510 (a);
	}
      a = ((a)->next);
    }
  if (((v)->anchordummy))
    {
      c = ((v)->connection);
      assert (c && (((c)->target)));
      assert ((((((c)->edge))->end) == v));
      if (((((c)->edge))->sxloc) < ((((c)->edge))->exloc))
	j = 8;
      else
	j = 7;
      a = ((v)->succ);
      while (a)
	{
	  ((((a)->kante))->orientation2) = j;
	  a = ((a)->next);
	}
      a = ((v)->pred);
      while (a)
	{
	  ((((a)->kante))->orientation) = j;
	  a = ((a)->next);
	}
      if (((c)->target2))
	{
	  assert ((((((c)->edge2))->start) == v));
	  ((((c)->edge2))->orientation2) = j;
	}
    }
}

#ifdef ANSI_C
static void
gs_ide1511 (GEDGE e)
#else
static void
gs_ide1511 (e)
     GEDGE e;
#endif
{
  float fval;;
  assert ((((e)->sxloc) != ((e)->exloc)));
  fval =
    (float) (((e)->eyloc) - ((e)->syloc)) / (float) (((e)->exloc) -
						     ((e)->sxloc));
  if (((e)->sxloc) < ((e)->exloc))
    {
      if (0.5 <= fval)
	((e)->orientation) = 5;
      else if (-0.5 < fval)
	((e)->orientation) = 7;
      else
	((e)->orientation) = 2;
    }
  else
    {
      if (0.5 <= fval)
	((e)->orientation) = 3;
      else if (-0.5 < fval)
	((e)->orientation) = 8;
      else
	((e)->orientation) = 6;
    }
  switch (((e)->orientation))
    {
    case 5:
      ((e)->orientation2) = 3;
      break;
    case 7:
      ((e)->orientation2) = 8;
      break;
    case 2:
      ((e)->orientation2) = 6;
      break;
    case 6:
      ((e)->orientation2) = 2;
      break;
    case 8:
      ((e)->orientation2) = 7;
      break;
    case 3:
      ((e)->orientation2) = 5;
      break;
    }
}

#ifdef ANSI_C
static void
gs_ide1512 (ADJEDGE a)
#else
static void
gs_ide1512 (a)
     ADJEDGE a;
#endif
{
  int port;
  int is_north, is_northeast, is_northwest;
  ADJEDGE b;
  GEDGE e;
  float fval;;
  port = ((((a)->kante))->weights);
  b = a;
  is_north = is_northeast = is_northwest = 1;
  while (b)
    {
      if (port != ((((b)->kante))->weights))
	break;
      e = ((b)->kante);
      if (((e)->btyloc) != ((e)->syloc))
	fval =
	  (float) (((e)->btxloc) - ((e)->sxloc)) / (float) (((e)->btyloc) -
							    ((e)->syloc));
      else
	fval =
	  (float) (((e)->bbxloc) - ((e)->sxloc)) / (float) (((e)->bbyloc) -
							    ((e)->syloc));
      if (!((-0.5 < fval) && (fval < 0.5)))
	is_north = 0;
      if (!(0.1 < fval))
	is_northwest = 0;
      if (!(fval < -0.1))
	is_northeast = 0;
      b = ((b)->next);
    }
  if (is_north)
    {
      b = a;
      while (b)
	{
	  if (port != ((((b)->kante))->weights))
	    break;
	  ((((b)->kante))->orientation2) = 1;
	  b = ((b)->next);
	}
      return;
    }
  if (is_northwest)
    {
      b = a;
      while (b)
	{
	  if (port != ((((b)->kante))->weights))
	    break;
	  ((((b)->kante))->orientation2) = 3;
	  b = ((b)->next);
	}
      return;
    }
  if (is_northeast)
    {
      b = a;
      while (b)
	{
	  if (port != ((((b)->kante))->weights))
	    break;
	  ((((b)->kante))->orientation2) = 2;
	  b = ((b)->next);
	}
      return;
    }
  b = a;
  while (b)
    {
      if (port != ((((b)->kante))->weights))
	break;
      ((((b)->kante))->orientation2) = 1;
      b = ((b)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide1510 (ADJEDGE a)
#else
static void
gs_ide1510 (a)
     ADJEDGE a;
#endif
{
  int port;
  int is_south, is_southwest, is_southeast;
  ADJEDGE b;
  GEDGE e;
  float fval;;
  port = ((((a)->kante))->weightp);
  b = a;
  is_south = is_southeast = is_southwest = 1;
  while (b)
    {
      if (port != ((((b)->kante))->weightp))
	break;
      e = ((b)->kante);
      if (((e)->bbyloc) != ((e)->eyloc))
	fval =
	  (float) (((e)->exloc) - ((e)->bbxloc)) / (float) (((e)->eyloc) -
							    ((e)->bbyloc));
      else
	fval =
	  (float) (((e)->exloc) - ((e)->btxloc)) / (float) (((e)->eyloc) -
							    ((e)->btyloc));
      if (!((-0.5 < fval) && (fval < 0.5)))
	is_south = 0;
      if (!(0.1 < fval))
	is_southeast = 0;
      if (!(fval < -0.1))
	is_southwest = 0;
      b = ((b)->next);
    }
  if (is_south)
    {
      b = a;
      while (b)
	{
	  if (port != ((((b)->kante))->weightp))
	    break;
	  ((((b)->kante))->orientation) = 4;
	  b = ((b)->next);
	}
      return;
    }
  if (is_southeast)
    {
      b = a;
      while (b)
	{
	  if (port != ((((b)->kante))->weightp))
	    break;
	  ((((b)->kante))->orientation) = 5;
	  b = ((b)->next);
	}
      return;
    }
  if (is_southwest)
    {
      b = a;
      while (b)
	{
	  if (port != ((((b)->kante))->weightp))
	    break;
	  ((((b)->kante))->orientation) = 6;
	  b = ((b)->next);
	}
      return;
    }
  b = a;
  while (b)
    {
      if (port != ((((b)->kante))->weightp))
	break;
      ((((b)->kante))->orientation) = 4;
      b = ((b)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide1521 (void)
#else
static void
gs_ide1521 ()
#endif
{;
  gs_ide1519 (nodelist);
  gs_ide1519 (labellist);
  gs_ide1519 (dummylist);
}

#define backward_connection1(c) (((( c )->edge)  )&& ((( (( c )->edge)   )->end)   ==v))
#define backward_connection2(c) (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  ==v))
#ifdef ANSI_C
static void
gs_ide1519 (GNODE v)
#else
static void
gs_ide1519 (v)
     GNODE v;
#endif
{
  int h;
  CONNECT c;
  ADJEDGE li;;
  while (v)
    {
      h = ((v)->xloc);
      ((v)->xloc) = ((v)->yloc);
      ((v)->yloc) = h;
      h = ((v)->width);
      ((v)->width) = ((v)->height);
      ((v)->height) = h;
      c = ((v)->connection);
      if (c)
	{
	  if (backward_connection1 (c))
	    gs_ide1520 (((c)->edge));
	  if (backward_connection2 (c))
	    gs_ide1520 (((c)->edge2));
	}
      li = ((v)->pred);
      while (li)
	{
	  gs_ide1520 (((li)->kante));
	  li = ((li)->next);
	}
      v = ((v)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide1520 (GEDGE e)
#else
static void
gs_ide1520 (e)
     GEDGE e;
#endif
{
  int h;;
  h = ((e)->sxloc);
  ((e)->sxloc) = ((e)->syloc);
  ((e)->syloc) = h;
  h = ((e)->btxloc);
  ((e)->btxloc) = ((e)->btyloc);
  ((e)->btyloc) = h;
  h = ((e)->bbxloc);
  ((e)->bbxloc) = ((e)->bbyloc);
  ((e)->bbyloc) = h;
  h = ((e)->exloc);
  ((e)->exloc) = ((e)->eyloc);
  ((e)->eyloc) = h;
  switch (((e)->orientation))
    {
    case 0:
      break;
    case 1:
      ((e)->orientation) = 8;
      break;
    case 2:
      ((e)->orientation) = 6;
      break;
    case 3:
      ((e)->orientation) = 3;
      break;
    case 4:
      ((e)->orientation) = 7;
      break;
    case 5:
      ((e)->orientation) = 5;
      break;
    case 6:
      ((e)->orientation) = 2;
      break;
    case 7:
      ((e)->orientation) = 4;
      break;
    case 8:
      ((e)->orientation) = 1;
      break;
    }
  switch (((e)->orientation2))
    {
    case 0:
      break;
    case 1:
      ((e)->orientation2) = 8;
      break;
    case 2:
      ((e)->orientation2) = 6;
      break;
    case 3:
      ((e)->orientation2) = 3;
      break;
    case 4:
      ((e)->orientation2) = 7;
      break;
    case 5:
      ((e)->orientation2) = 5;
      break;
    case 6:
      ((e)->orientation2) = 2;
      break;
    case 7:
      ((e)->orientation2) = 4;
      break;
    case 8:
      ((e)->orientation2) = 1;
      break;
    }
}
static int gs_ide1532;
#ifdef ANSI_C
static void
gs_ide1524 (void)
#else
static void
gs_ide1524 ()
#endif
{
  int i;
  GNLIST k;;
  gs_ide1532 = 0;
  for (i = maxdepth + 1; i >= 0; i--)
    if (((layer[i]).succlist))
      break;
  k = ((layer[i]).succlist);
  while (k)
    {
      if (((((k)->node))->yloc) + ((((k)->node))->height) > gs_ide1532)
	gs_ide1532 = ((((k)->node))->yloc) + ((((k)->node))->height);
      k = ((k)->next);
    }
  gs_ide1532 = gs_ide1532 + G_ybase;
  gs_ide1522 (nodelist);
  gs_ide1522 (labellist);
  gs_ide1522 (dummylist);
}

#define backward_connection1(c) (((( c )->edge)  )&& ((( (( c )->edge)   )->end)   ==v))
#define backward_connection2(c) (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  ==v))
#ifdef ANSI_C
static void
gs_ide1522 (GNODE v)
#else
static void
gs_ide1522 (v)
     GNODE v;
#endif
{
  CONNECT c;
  ADJEDGE li;;
  while (v)
    {
      ((v)->yloc) = gs_ide1532 - ((v)->yloc) - ((v)->height);
      c = ((v)->connection);
      if (c)
	{
	  if (backward_connection1 (c))
	    gs_ide1523 (((c)->edge));
	  if (backward_connection2 (c))
	    gs_ide1523 (((c)->edge2));
	}
      li = ((v)->pred);
      while (li)
	{
	  gs_ide1523 (((li)->kante));
	  li = ((li)->next);
	}
      v = ((v)->next);
    }
}

#ifdef ANSI_C
static void
gs_ide1523 (GEDGE e)
#else
static void
gs_ide1523 (e)
     GEDGE e;
#endif
{;
  ((e)->syloc) = gs_ide1532 - ((e)->syloc);
  ((e)->bbyloc) = gs_ide1532 - ((e)->bbyloc);
  ((e)->btyloc) = gs_ide1532 - ((e)->btyloc);
  ((e)->eyloc) = gs_ide1532 - ((e)->eyloc);
  switch (((e)->orientation))
    {
    case 0:
      break;
    case 1:
      ((e)->orientation) = 4;
      break;
    case 2:
      ((e)->orientation) = 5;
      break;
    case 3:
      ((e)->orientation) = 6;
      break;
    case 4:
      ((e)->orientation) = 1;
      break;
    case 5:
      ((e)->orientation) = 2;
      break;
    case 6:
      ((e)->orientation) = 3;
      break;
    }
  switch (((e)->orientation2))
    {
    case 0:
      break;
    case 1:
      ((e)->orientation2) = 4;
      break;
    case 2:
      ((e)->orientation2) = 5;
      break;
    case 3:
      ((e)->orientation2) = 6;
      break;
    case 4:
      ((e)->orientation2) = 1;
      break;
    case 5:
      ((e)->orientation2) = 2;
      break;
    case 6:
      ((e)->orientation2) = 3;
      break;
    }
}
int maximal_xpos;
int maximal_ypos;
#ifdef ANSI_C
void
calc_max_xy_pos (void)
#else
void
calc_max_xy_pos ()
#endif
{
  GNODE v;;
  maximal_xpos = 0;
  maximal_ypos = 0;
  v = nodelist;
  while (v)
    {
      if (((v)->xloc) + ((v)->width) > maximal_xpos)
	maximal_xpos = ((v)->xloc) + ((v)->width);
      if (((v)->yloc) + ((v)->height) > maximal_ypos)
	maximal_ypos = ((v)->yloc) + ((v)->height);
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      if (((v)->xloc) + ((v)->width) > maximal_xpos)
	maximal_xpos = ((v)->xloc) + ((v)->width);
      if (((v)->yloc) + ((v)->height) > maximal_ypos)
	maximal_ypos = ((v)->yloc) + ((v)->height);
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      if (((v)->xloc) + ((v)->width) > maximal_xpos)
	maximal_xpos = ((v)->xloc) + ((v)->width);
      if (((v)->yloc) + ((v)->height) > maximal_ypos)
	maximal_ypos = ((v)->yloc) + ((v)->height);
      v = ((v)->next);
    }
  maximal_xpos += G_xbase;
  maximal_ypos += G_ybase;;
}

int st_nr_vis_nodes;
int st_nr_vis_edges;
int st_nr_vis_nearedges;
int st_nr_vis_dummies;
int st_nr_vis_labels;
int st_nr_invis_graphs;
int st_nr_invis_nodes;
int st_nr_invis_edges;
int st_max_indeg;
int st_max_outdeg;
int st_max_degree;
#define backward_connection1(c) (((( c )->edge)  )&& ((( (( c )->edge)   )->end)   ==v))
#define backward_connection2(c) (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  ==v))
#ifdef ANSI_C
void
statistics (void)
#else
void
statistics ()
#endif
{
  GNODE v;
  GEDGE e;
  ADJEDGE li;
  CONNECT c;
  int maxdegree;
  int maxindeg;
  int maxoutdeg;;
  st_nr_vis_nodes = 0;
  st_nr_vis_edges = 0;
  st_nr_vis_nearedges = 0;
  st_nr_vis_dummies = 0;
  st_nr_vis_labels = 0;
  st_nr_invis_graphs = 0;
  st_nr_invis_nodes = 0;
  st_nr_invis_edges = 0;
  st_max_indeg = 0;
  st_max_outdeg = 0;
  st_max_degree = 0;
  v = nodelist;
  while (v)
    {
      c = ((v)->connection);
      if (c)
	{
	  if (backward_connection1 (c))
	    st_nr_vis_nearedges++;
	  if (backward_connection2 (c))
	    st_nr_vis_nearedges++;
	}
      maxdegree = maxindeg = maxoutdeg = 0;
      li = ((v)->succ);
      while (li)
	{
	  maxdegree++;
	  maxoutdeg++;
	  li = ((li)->next);
	}
      li = ((v)->pred);
      while (li)
	{
	  st_nr_vis_edges++;
	  maxdegree++;
	  maxindeg++;
	  li = ((li)->next);
	}
      if (maxdegree > st_max_degree)
	st_max_degree = maxdegree;
      if (maxindeg > st_max_indeg)
	st_max_indeg = maxindeg;
      if (maxoutdeg > st_max_outdeg)
	st_max_outdeg = maxoutdeg;
      st_nr_vis_nodes++;
      v = ((v)->next);
    }
  v = labellist;
  while (v)
    {
      c = ((v)->connection);
      if (c)
	{
	  if (backward_connection1 (c))
	    st_nr_vis_nearedges++;
	  if (backward_connection2 (c))
	    st_nr_vis_nearedges++;
	}
      maxdegree = maxindeg = maxoutdeg = 0;
      li = ((v)->succ);
      while (li)
	{
	  maxdegree++;
	  maxoutdeg++;
	  li = ((li)->next);
	}
      li = ((v)->pred);
      while (li)
	{
	  st_nr_vis_edges++;
	  maxdegree++;
	  maxindeg++;
	  li = ((li)->next);
	}
      if (maxdegree > st_max_degree)
	st_max_degree = maxdegree;
      if (maxindeg > st_max_indeg)
	st_max_indeg = maxindeg;
      if (maxoutdeg > st_max_outdeg)
	st_max_outdeg = maxoutdeg;
      st_nr_vis_labels++;
      v = ((v)->next);
    }
  v = dummylist;
  while (v)
    {
      c = ((v)->connection);
      if (c)
	{
	  if (backward_connection1 (c))
	    st_nr_vis_nearedges++;
	  if (backward_connection2 (c))
	    st_nr_vis_nearedges++;
	}
      maxdegree = maxindeg = maxoutdeg = 0;
      li = ((v)->succ);
      while (li)
	{
	  maxdegree++;
	  maxoutdeg++;
	  li = ((li)->next);
	}
      li = ((v)->pred);
      while (li)
	{
	  st_nr_vis_edges++;
	  maxdegree++;
	  maxindeg++;
	  li = ((li)->next);
	}
      if (maxdegree > st_max_degree)
	st_max_degree = maxdegree;
      if (maxindeg > st_max_indeg)
	st_max_indeg = maxindeg;
      if (maxoutdeg > st_max_outdeg)
	st_max_outdeg = maxoutdeg;
      st_nr_vis_dummies++;
      v = ((v)->next);
    }
  v = invis_nodes;
  while (v)
    {
      st_nr_invis_nodes++;
      v = ((v)->next);
    }
  v = graphlist;
  while (v)
    {
      st_nr_invis_graphs++;
      v = ((v)->next);
    }
  e = edgelist;
  while (e)
    {
      if (((e)->invisible))
	st_nr_invis_edges++;
      e = ((e)->next);
    }
}
