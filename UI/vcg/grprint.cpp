/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*								      */
/*		VCG : Visualization of Compiler Graphs		      */ 
/*		--------------------------------------		      */ 
/*								      */
/*   file:	   grprint.c					      */
/*   version:	   1.00.00					      */
/*   creation:	   14.4.93					      */
/*   author:	   G. Sander (Version 1.00.00-...)		      */ 
/*		   Universitaet des Saarlandes, 66041 Saarbruecken    */
/*		   ESPRIT Project #5399 Compare 		      */ 
/*   description:  Print layout into a file   			      */ 
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
 * Revision 3.8  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.7  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 * infobox behaviour improved.
 * First version of fisheye (carthesian).
 * Options Noedge and nonode.
 *
 * Revision 3.6  1994/08/03  13:58:44  sander
 * Horizontal order mechanism changed.
 * Attribute horizontal_order for edges added.
 *
 * Revision 3.5  1994/06/07  14:09:59  sander
 * Splines implemented.
 * HP-UX, Linux, AIX, Sun-Os, IRIX compatibility tested.
 * The tool is now ready to be distributed.
 *
 * Revision 3.4  1994/05/17  16:39:10  sander
 * attribute node_align added to allow nodes to be centered in the levels.
 *
 * Revision 3.3  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
 *
 * Revision 3.2  1994/05/05  08:20:30  sander
 * Small corrections.
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
 *  This file allows to write a layouted graph into a file in
 *  such a way that this file can be reloaded.
 *
 *  This file here provides the following functions:
 *    print_graph	      prints the full graph in ASCII form to a
 *			      file
 *    color_text	      converts a color into a text
 *    textmode_text	      converts a textmode into a text
 *    linestyle_text	      converts a linestyle into a text
 ************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "main.h"
#include "options.h"
#include "alloc.h"
#include "folding.h"
#include "steps.h"
#include "fisheye.h"
#include "grprint.h"

#include <deque>
#include <iostream>
#include <map>
#include <vector>

namespace {
    struct EdgeVertex
    {
        EdgeVertex() : x(0), y(0) {}
        EdgeVertex(int x_, int y_, const std::string& tech) : x(x_), y(y_), tech_name(tech) {}
        int x;
        int y;
        std::string tech_name;
    };
    bool operator<(const EdgeVertex& lhs, const EdgeVertex& rhs)
    {
        if (!lhs.tech_name.empty() && rhs.tech_name.empty())
            return true;
        else if (lhs.tech_name.empty() && !rhs.tech_name.empty())
            return false;
        if (lhs.x < rhs.x)
            return true;
        else if (lhs.x > rhs.x)
            return false;
        if (lhs.y < rhs.y)
            return true;
        else
            return false;
    }
    bool operator==(const EdgeVertex& lhs, const EdgeVertex& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.tech_name == rhs.tech_name;
    }
    std::ostream& operator<<(std::ostream& os, const EdgeVertex& ev)
    {
        return os << "\"" << ev.tech_name << "\" (" << ev.x << ", " << ev.y << ")";
    }
    typedef std::pair<EdgeVertex, EdgeVertex> EdgeSegment;
    typedef std::map<EdgeVertex, EdgeVertex> EdgeSegments;
    typedef std::deque<EdgeVertex> Edge;
}

/*  Prototypes
 *  ---------- 
 */

static void print_edge  _PP((EdgeSegments& edge_segments, GEDGE e));


/* For indentation */

/*--------------------------------------------------------------------*/
/*   Print all nodes and edges		         		      */
/*--------------------------------------------------------------------*/

/* Macro to detect backward connections */

#define backward_connection1(c) ((CEDGE(c))&& (EEND(CEDGE(c)) ==v))
#define backward_connection2(c) ((CEDGE2(c))&&(EEND(CEDGE2(c))==v))

// TODO: make this take a reference each to a VertexMap and an EdgeMap, in order to return these values
void print_graph(TechVertexMap& vertices, TechEdgeMap& edges)
{
    // print nodes/vertices
	GNODE v = nodelist;
	while (v) {
        vertices[((!NLABEL(v) || !NLABEL(v)[0]) ? "ERROR" : NLABEL(v))] = std::pair<int, int>(NX(v), NY(v));
        v = NNEXT(v); 
    }

    // print edges
    EdgeSegments edge_segments;
	ADJEDGE li;
	CONNECT c;
	v = nodelist;
	while (v) {
        c = NCONNECT(v);
        if (c) {
            if (backward_connection1(c)) 
                print_edge(edge_segments, CEDGE(c));
            if (backward_connection2(c)) 
                print_edge(edge_segments, CEDGE2(c));
        }
        li = NPRED(v);
        while (li) {
            print_edge(edge_segments, AKANTE(li));
            li = ANEXT(li);
        }
        v = NNEXT(v);
	}
#if 0
	v = labellist;
	while (v) {
        c = NCONNECT(v);
        if (c) {
            if (backward_connection1(c)) 
                print_edge(edge_segments, CEDGE(c));
            if (backward_connection2(c)) 
                print_edge(edge_segments, CEDGE2(c));
        }
        li = NPRED(v);
        while (li) {
            print_edge(edge_segments, AKANTE(li));
            li = ANEXT(li);
        }
        v = NNEXT(v);
	}
#endif
	v = dummylist;
	while (v) {
        c = NCONNECT(v);
        if (c) {
            if (backward_connection1(c)) 
                print_edge(edge_segments, CEDGE(c));
            if (backward_connection2(c)) 
                print_edge(edge_segments, CEDGE2(c));
        }
        li = NPRED(v);
        while (li) {
            print_edge(edge_segments, AKANTE(li));
            li = ANEXT(li);
        }
        v = NNEXT(v);
	}

#if 0 // for debugging
    for (EdgeSegments::const_iterator it = edge_segments.begin(); it != edge_segments.end(); ++it) {
        std::cout << "Segment[" << it->first << " - " << it->second << "]\n";
    }
#endif

    EdgeSegments::iterator it = edge_segments.begin();
    EdgeSegments::iterator end_it = edge_segments.begin();
    while (end_it != edge_segments.end() && !end_it->first.tech_name.empty())
        ++end_it;
    for (; it != end_it; ++it) {
        Edge edge;
        edge.push_front(it->first);
        edge.push_front(it->second);
        while (edge.front().tech_name.empty()) {
            EdgeSegments::iterator next_it = edge_segments.find(edge.front());
            assert(next_it != edge_segments.end());
            edge.push_front(next_it->second);
        }
        TechGraphEdge external_edge;
        for (unsigned int i = 0; i < edge.size(); ++i) {
            external_edge.push_back(std::make_pair(edge[i].x, edge[i].y));
        }
        edges[std::make_pair(edge.front().tech_name, edge.back().tech_name)] = external_edge;
    }

#if 0 // for debugging
    for (EdgeMap::const_iterator it = edges.begin(); it != edges.end(); ++it) {
        std::cout << it->first.first << " TO " << it->first.second << ": [ ";
        for (TechGraphEdge::const_iterator edge_it = it->second.begin(); edge_it != it->second.end(); ++edge_it) {
            std::cout << "(" << edge_it->first << ", " << edge_it->second << ") ";
        }
        std::cout << "]\n";
    }
#endif
}

void print_edge(EdgeSegments& edge_segments, GEDGE e)
{
	GNODE vs,ve;
	char  topbend;
	char  botbend;
	int   x1,x2,x3,x4,y1,y2,y3,y4,s;

	if (EANCHOR(e)==66) return;

	vs = ESTART(e);
	if (NANCHORNODE(vs)) {
		vs = ESTART(CEDGE(NCONNECT(vs)));
	}
	ve = EEND(e);

	x1 = ESTARTX(e);
	y1 = ESTARTY(e);
	x2 = EENDX(e);
	y2 = EENDY(e);
	x3 = ETBENDX(e);
	y3 = ETBENDY(e);
	x4 = EBBENDX(e);
	y4 = EBBENDY(e);
	s  = EARROWSIZE(e);
	
	topbend = 0;
	if ((y3!=y1)||(x3!=x1)) topbend = 1; 
	if ((x1<x3) && (x1+s*8/10>x3)) topbend = 0;
	if ((x1>x3) && (x1-s*8/10<x3)) topbend = 0;
	if ((y1<y3) && (y1+s*8/10>y3)) topbend = 0;
	if ((y1>y3) && (y1-s*8/10<y3)) topbend = 0;
	botbend = 0;
	if ((y4!=y2)||(x4!=x2)) botbend = 1; 
	if ((x2<x4) && (x2+s*8/10>x4)) botbend = 0;
	if ((x2>x4) && (x2-s*8/10<x4)) botbend = 0;
	if ((y2<y4) && (y2+s*8/10>y4)) botbend = 0;
	if ((y2>y4) && (y2-s*8/10<y4)) botbend = 0;

	if (topbend)
        edge_segments.insert(EdgeSegment(EdgeVertex(ETBENDX(e), ETBENDY(e), ""), EdgeVertex(NX(vs), NY(vs), NLABEL(vs))));
	if (botbend)
        edge_segments.insert(EdgeSegment(EdgeVertex(EBBENDX(e), EBBENDY(e), ""), EdgeVertex(NX(vs), NY(vs), NLABEL(vs))));
    edge_segments.insert(EdgeSegment(EdgeVertex(NX(ve), NY(ve), NLABEL(ve)), EdgeVertex(NX(vs), NY(vs), NLABEL(vs))));
}

/*--------------------------------------------------------------------*/
/*   Convert a color into its name				      */
/*--------------------------------------------------------------------*/

static char mycolbuf[10];

char *color_text(int c)
{
	switch (c) {
	case BLACK       : return("black");
	case BLUE        : return("blue");
	case RED         : return("red");
	case GREEN       : return("green");
	case YELLOW      : return("yellow");
	case MAGENTA     : return("magenta");
	case CYAN        : return("cyan");
	case WHITE       : return("white");
	case DARKGREY    : return("darkgrey");
	case DARKBLUE    : return("darkblue");
	case DARKRED     : return("darkred");
	case DARKGREEN   : return("darkgreen");
	case DARKYELLOW  : return("darkyellow");
	case DARKMAGENTA : return("darkmagenta");
	case DARKCYAN    : return("darkcyan");
	case GOLD        : return("gold");
	case LIGHTGREY   : return("lightgrey");
	case LIGHTBLUE   : return("lightblue");
	case LIGHTRED    : return("lightred");
	case LIGHTGREEN  : return("lightgreen");
	case LIGHTYELLOW : return("lightyellow");
	case LIGHTMAGENTA: return("lightmagenta");
	case LIGHTCYAN   : return("lightcyan");
	case LILAC       : return("lilac");
	case TURQUOISE   : return("turquoise");
	case AQUAMARINE  : return("aquamarine");
	case KHAKI       : return("khaki");
	case PURPLE      : return("purple");
	case YELLOWGREEN : return("yellowgreen");
	case PINK        : return("pink");
	case ORANGE      : return("orange");
	case ORCHID      : return("orchid");
	}

	SPRINTF(mycolbuf,"%d",c);
	return(mycolbuf);
}



/*--------------------------------------------------------------------*/
/*   Convert a textmode into its name				      */
/*--------------------------------------------------------------------*/

char *textmode_text(int t)
{
	switch (t) {
	case CENTER: return("center");
	case LEFT:   return("left_justify");
	case RIGHT:  return("right_justify");
	}
	return("none");
}

/*--------------------------------------------------------------------*/
/*   Convert a shape into its name				      */
/*--------------------------------------------------------------------*/

char *shape_text(int t)
{
	switch (t) {
	case BOX:      return("box");
	case RHOMB:    return("rhomb");
	case ELLIPSE:  return("ellipse");
	case TRIANGLE: return("triangle");
	}
	return("none");
}



/*--------------------------------------------------------------------*/
/*   Convert a linestyle into its name				      */
/*--------------------------------------------------------------------*/

char *linestyle_text(int t)
{
	switch (t) {
	case SOLID:     return("continuous");
	case DOTTED:    return("dotted");
	case DASHED:    return("dashed");
	case UNVISIBLE: return("invisible");
	}
	return("none");
}


