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
#include "steps.h"
#include "timing.h"
#ifndef ANSI_C
#ifndef const
#define const
#endif
#endif
static int gs_ide2009 _PP((void)); static int gs_ide2008 _PP((GNODE v)); /*;*a*b
*a*b;*/ static void gs_ide2000 _PP((void)); static void gs_ide2004 _PP((void)); /*;*a*b
*a*b;*/ static void gs_ide2015 _PP((void)); static int gs_ide2001 _PP((const GNODE *a, const GNODE *b)); /*;*gs_ide2007 = $Id: tree.c,v 1.6 1995*02
09 _PP((void));;*/ static void gs_ide2013 _PP((void)); static void gs_ide2018 _PP((void)); /*;(/N;D+ v));
);;*/ static int gs_ide2005 _PP((GNODE v,int l)); static void gs_ide2002 _PP((GNODE conn,GNODE v)); /*;static void gs_ide2004 _PP((void));
void gs_ide2015 _PP((void));;*/ static int gs_ide2011 _PP((GNODE v,GNODE conn)); static int gs_ide2010 _PP((GNODE v,int depth)); /*;ide2001 _PP((const /N;D+ *a, const /N;D
3 _PP((void));;*/ static void gs_ide2003 _PP((GNODE v,int diff)); static int gs_ide2012 ; /*;void));
int l));;*/   static GNODE *gs_ide2014 = NULL; int spread_level = 1; double tree_factor = 0.5; /*;t gs_ide2011 _PP((/N;D+ v,/N;D+ conn));
e2010 _PP((/N;D+ v,int depth));;*/ static int gs_ide2016 ; static int gs_ide2017 ;
#ifdef ANSI_C
int tree_main(void)
#else
int tree_main()
#endif
{ start_time(); ; assert((layer)); if (tree_factor<0) tree_factor = -tree_factor; /*;void)
int tree_main();*/ if (tree_factor>1.0) { gs_ide2017 = (int)(10.0/tree_factor); gs_ide2016 = 10; /*;ime();
;;*/ } else { gs_ide2016 = (int)(tree_factor*10.0); gs_ide2017 = 10; } if ((gs_ide2016 ==0)||(gs_ide2017 ==0)) { /*;gs_ide2016 = 10;
};*/ tree_factor = 0.5; gs_ide2016 = 1; gs_ide2017 = 2; } gs_wait_message('T'); /*;}
_ide2016 ==0)||(gs_ide2017 ==0)) {;*/ if (!gs_ide2009 ()) { stop_time("tree_main"); return(0); } gs_wait_message('T'); /*;}
_message('T');;*/ gs_ide2000 (); gs_ide2004 (); calc_all_node_sizes(); alloc_levelshift(); /*;return(0);
};*/   gs_ide2014 = (GNODE *)tpred_connection1; gs_ide2018 (); gs_ide2015 (); /*;
gs_ide2004 ();;*/ gs_wait_message('T'); gs_ide2013 (); calc_all_ports(1); nr_crossings = 0; /*;)tpred_connection1;
gs_ide2018 ();;*/ stop_time("tree_main"); return(TREE_LAYOUT); }
#define forward_connection1(c)  (((( c )->edge)  )&& ((( (( c )->edge)   )->end)  !=v))
#define forward_connection2(c)  (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  !=v))
#define backward_connection1(c) (((( c )->edge)  )&& ((( (( c )->edge)   )->end)  ==v))
#define backward_connection2(c) (((( c )->edge2)  )&&((( (( c )->edge2)   )->end)  ==v))
#ifdef ANSI_C
static int gs_ide2009 (void)
#else
static int gs_ide2009 ()
#endif
{ int i; int result; int fresh; GNODE v; GNLIST li; CONNECT c; ; v = nodelist; /*;ult;
int fresh;;*/ while (v) { (( v )->markiert) /*;int fresh;;*/ = 1; v = (( v )->next) /*;
int fresh;;*/ ; } v = labellist; while (v) { (( v )->markiert) /*;/NLIST li;
;*/ = 1; v = (( v )->next) /*;/NLIST li;;*/ ; } v = dummylist; while (v) { /*;;
elist;;*/  (( v )->markiert) /*;;;*/ = 1; v = (( v )->next) /*;;;*/ ; } for (i=0; /*;elist;
v )->markiert) /+;R/SC;yy+NT = 1; v = ;*/  i<= maxdepth+1; i++) { li = (( layer[i] ).succlist) /*; v )->markiert) /+;
R/SC;yy+NT = 1; v = ;*/ ; while (li) { v = (( li )->node) /*;v) { (( v )->markiert) /+;
R/SC;yy+NT = ;*/ ; fresh = (( v )->markiert) /*;;*/ ; c = (( v )->connection) /*;
arkiert) /+;R/SC;yy+NT = 1; v = (( v )-;*/ ; if (c) { if (backward_connection1(c)) fresh = 0; /*;C;yy+NT ;
ile (li) {;*/ if (backward_connection2(c)) fresh = 0; } result = 0; if (fresh) result = gs_ide2008 (v); /*; /+;R/SC;yy+NT ;
if (c) {;*/ if (result) return(0); li = (( li )->next) /*;kward_connection1(c)) fresh = 0;
;*/ ; } } return(1); }
#ifdef ANSI_C
static int gs_ide2008 (GNODE v)
#else
static int gs_ide2008 (v) GNODE v;
#endif
{ int i; ADJEDGE a; CONNECT c; ; if (!(( v )->markiert) /*;/N;D+ v;;
*/ ) return(1); (( v )->markiert) /*;{;*/ = 0; i = 0; a = (( v )->pred) /*;
a;;*/ ; while (a) { i++; a = (( a )->next) /*;C;NN+CT c;;*/ ; } if (i>1) return(1); /*;;
v )->markiert) /+;R/SC;yy+NT ) return(;*/ i = 0; a = (( v )->succ) /*;rt) /+;R/SC;yy+NT = 0;;*/ ; while (a) { i++; /*;v )->pred) /+;R/SC;yy+NT ;
+; a = (( a )->next) /+;R/SC;yy+NT ; };*/ if (gs_ide2008 (((( (( a )->kante) /*;+; a = (( a )->next) /+;R/SC;yy+NT ;
};*/ )->end) /*;+; a = (( a )->next) /+;R/SC;yy+NT ; };*/ ) /*;+; a = (( a )->next) /+;
R/SC;yy+NT ; };*/ )) return(1); a = (( a )->next) /*;if (i>1) return(1);
;*/ ; } c = (( v )->connection) /*;v )->succ) /+;R/SC;yy+NT ;;*/ ; if (c) { /*;while (a) {
i++;;*/ if (forward_connection1(c)) { if (gs_ide2008 ((( (( c )->edge) /*;ide2008 (((( (( a )->kante) /+;
R/SC;yy+;*/ )->end) /*;ide2008 (((( (( a )->kante) /+;R/SC;yy+;*/ )) return(1); /*;ide2008 (((( (( a )->kante) /+;R/SC;yy+
xt) /+;R/SC;yy+NT ;;*/ } if (forward_connection2(c)) { if (gs_ide2008 ((( (( c )->edge2) /*;
v )->connection) /+;R/SC;yy+NT ;;*/ )->end) /*;v )->connection) /+;R/SC;
yy+NT ;;*/ )) return(1); } } return(0); }
#ifdef ANSI_C
static void gs_ide2000 (void)
#else
static void gs_ide2000 ()
#endif
{ int i, j; GNODE v; GNLIST li; ADJEDGE a; ; gs_ide2012 = 0; for (i=0; /*;j;
/N;D+ v;;*/  i<= maxdepth+1; i++) { li = (( layer[i] ).succlist) /*;/N;D+ v;;*/ ; /*;li;
ADJ+D/+ a;;*/  while (li) { v = (( li )->node) /*;ADJ+D/+ a;;*/ ; j = 0; a = (( v )->pred) /*;
012 = 0;;*/ ; while (a) { j++; a = (( a )->next) /*;maxdepth+1; i++) {
;*/ ; } (( v )->indegree) /*;cclist) /+;R/SC;yy+NT ;;*/ = j; assert((j<=1)); /*;while (li) {
li )->node) /+;R/SC;yy+NT ;;*/ j = 0; a = (( v )->succ) /*;j = 0;;*/ ; while (a) { j++; a = (( a )->next) /*;
v )->pred) /+;R/SC;yy+NT ;;*/ ; } (( v )->outdegree) /*;+; a = (( a )->next) /+;
R/SC;yy+NT ; };*/ = j; if (j>gs_ide2012 ) gs_ide2012 = j; li = (( li )->next) /*;
assert((j<=1));;*/ ; } } maxindeg = 1; maxoutdeg = gs_ide2012 ; }
#ifdef ANSI_C
static void gs_ide2004 (void)
#else
static void gs_ide2004 ()
#endif
{ int i, k; GNLIST h1, h2; ; max_nodes_per_layer = 0; for (i=0; i<=maxdepth+1; /*;_ide2004 ()
{;*/  i++) { h1 = (( layer[i] ).succlist) /*;{;*/ ; (( layer[i] ).predlist) /*;
k;;*/ = NULL; k = 0; while (h1) { k++; h2 = tmpnodelist_alloc(); (( h2 )->next) /*;
cclist) /+;R/SC;yy+NT ;;*/ = (( layer[i] ).predlist) /*;cclist) /+;R/SC;
yy+NT ;;*/ ; (( layer[i] ).predlist) /*;/SC;yy+NT = NULL;;*/ = h2; (( h2 )->node) /*;
k = 0;;*/ = (( h1 )->node) /*;k = 0;;*/ ; h1 = (( h1 )->next) /*;h1) {
;*/ ; } (( layer[i] ).anz) /*;pnodelist_alloc();;*/ = k; if (k>max_nodes_per_layer) max_nodes_per_layer = k; /*; /+;R/SC;yy+NT = (( layer[i] ).predlist
t) /+;R/SC;yy+NT = h2;;*/ } }
#ifdef ANSI_C
static void gs_ide2015 (void)
#else
static void gs_ide2015 ()
#endif
{ int i, k, max; GNLIST h1; ; for (i=0; i<=maxdepth+1; i++) { h1 = (( layer[i] ).succlist) /*;
_ide2015 ();*/ ; k = 0; while (h1) { gs_ide2014 [k++] = (( h1 )->node) /*;
/NLIST h1;;*/ ; h1 = (( h1 )->next) /*;;;*/ ; } max = k;
#ifdef ANSI_C
if (k) qsort(gs_ide2014 , k, sizeof(GNODE), (int (*) (const void *, const void *))gs_ide2001 ); /*;h1) {
+] = (( h1 )->node) /+;R/SC;yy+NT ;;*/  
#else
if (k) qsort(gs_ide2014 , k, sizeof(GNODE),gs_ide2001 ); 
#endif
h1 = (( layer[i] ).succlist) /*;+;R/SC;yy+NT ;;*/ ; k = 0; while (h1) { /*;;
s_ide2014 , k, sizeof(/N;D+),;*/ (( gs_ide2014 [k] )->position) /*;s_ide2014 , k, sizeof(/N;D+),;*/ = k; /*;s_ide2014 , k, sizeof(/N;D+),
*, const void *))gs_ide2001 ); ;*/ (( h1 )->node) /*;*, const void *))gs_ide2001 ); ;*/ = gs_ide2014 [k++]; /*;*, const void *))gs_ide2001 ); 
sizeof(/N;D+),gs_ide2001 ); ;*/ h1 = (( h1 )->next) /*; sizeof(/N;D+),gs_ide2001 ); ;*/ ; } assert((k==max)); /*;= 0;
) {;*/ k--; h1 = (( layer[i] ).predlist) /*;] )->position) /+;R/SC;yy+NT = k;
;*/ ; while (h1) { (( h1 )->node) /*;+NT ;;*/ = gs_ide2014 [k--]; h1 = (( h1 )->next) /*;
};*/ ; } assert((k== -1)); } }
#ifdef ANSI_C
static int gs_ide2001 (const GNODE *a,const GNODE *b)
#else
static int gs_ide2001 (a,b) GNODE *a; GNODE *b;
#endif
{  if ((( *a )->xloc) /*;};*/ > (( *b )->xloc) /*;};*/ ) return(1); if ((( *a )->xloc) /*;
int gs_ide2001 (const /N;D+ *a,const /N;*/ < (( *b )->xloc) /*;int gs_ide2001 (const /N;
D+ *a,const /N;*/ ) return(-1); return(0); }
#ifdef ANSI_C
static void gs_ide2013 (void)
#else
static void gs_ide2013 ()
#endif
{ int i; GNLIST h1; ADJEDGE a; ; for (i=0; i<=maxdepth+1; i++) { if (i>0) { /*;{
int i;;*/ h1 = (( layer[i-1] ).succlist) /*;int i;;*/ ; while (h1) { (( (( h1 )->node) /*;
ADJ+D/+ a;;*/ )->tmpadj) /*;ADJ+D/+ a;;*/ = (( (( h1 )->node) /*;ADJ+D/+ a;
;*/ )->succ) /*;ADJ+D/+ a;;*/ ; h1 = (( h1 )->next) /*;;;*/ ; } } if (i<maxdepth+1) { /*; layer[i-1] ).succlist) /+;R/SC;yy+NT ;
while (h1) {;*/ h1 = (( layer[i+1] ).succlist) /*;while (h1) {;*/ ; while (h1) { (( (( h1 )->node) /*;
next) /+;R/SC;yy+NT ;;*/ )->tmpadj) /*;next) /+;R/SC;yy+NT ;;*/ = (( (( h1 )->node) /*;
next) /+;R/SC;yy+NT ;;*/ )->pred) /*;next) /+;R/SC;yy+NT ;;*/ ; h1 = (( h1 )->next) /*;
};*/ ; } } h1 = (( layer[i] ).succlist) /*;i+1] ).succlist) /+;R/SC;
yy+NT ;;*/ ; while (h1) { a = (( (( h1 )->node) /*;1 )->node) /+;R/SC;
yy+NT )->tmpadj) /+;;*/ )->pred) /*;1 )->node) /+;R/SC;yy+NT )->tmpadj) /+;
;*/ ; while (a) { assert(((( ((( (( a )->kante) /*;};*/ )->start) /*;
};*/ ) /*;};*/ )->tmpadj) /*;};*/ )); (( (( ((( (( a )->kante) /*;};
*/ )->start) /*;};*/ ) /*;};*/ )->tmpadj) /*;};*/ )->kante) /*;};*/ = (( a )->kante) /*;
};*/ ; (( ((( (( a )->kante) /*; layer[i] ).succlist) /+;R/SC;yy+NT ;
;*/ )->start) /*; layer[i] ).succlist) /+;R/SC;yy+NT ;;*/ ) /*; layer[i] ).succlist) /+;
R/SC;yy+NT ;;*/ )->tmpadj) /*; layer[i] ).succlist) /+;R/SC;yy+NT ;;
*/ = (( (( ((( (( a )->kante) /*; layer[i] ).succlist) /+;R/SC;yy+NT ;
;*/ )->start) /*; layer[i] ).succlist) /+;R/SC;yy+NT ;;*/ ) /*; layer[i] ).succlist) /+;
R/SC;yy+NT ;;*/ )->tmpadj) /*; layer[i] ).succlist) /+;R/SC;yy+NT ;;
*/ )->next) /*; layer[i] ).succlist) /+;R/SC;yy+NT ;;*/ ; a = (( a )->next) /*;
while (h1) {;*/ ; } a = (( (( h1 )->node) /*;while (a) {;*/ )->succ) /*;
while (a) {;*/ ; while (a) { assert(((( ((( (( a )->kante) /*; )->kante) /+;
R/SC;yy+NT )->start) /+;R;*/ )->end) /*; )->kante) /+;R/SC;yy+NT )->start) /+;
R;*/ ) /*; )->kante) /+;R/SC;yy+NT )->start) /+;R;*/ )->tmpadj) /*; )->kante) /+;
R/SC;yy+NT )->start) /+;R;*/ )); (( (( ((( (( a )->kante) /*; /+;R/SC;
yy+NT )->start) /+;R/SC;yy+NT ;*/ )->end) /*; /+;R/SC;yy+NT )->start) /+;
R/SC;yy+NT ;*/ ) /*; /+;R/SC;yy+NT )->start) /+;R/SC;yy+NT ;*/ )->tmpadj) /*;
/+;R/SC;yy+NT )->start) /+;R/SC;yy+NT ;*/ )->kante) /*; /+;R/SC;yy+NT )->start) /+;
R/SC;yy+NT ;*/ = (( a )->kante) /*; /+;R/SC;yy+NT )->start) /+;R/SC;
yy+NT ;*/ ; (( ((( (( a )->kante) /*;+NT ;;*/ )->end) /*;+NT ;;*/ ) /*;
+NT ;;*/ )->tmpadj) /*;+NT ;;*/ = (( (( ((( (( a )->kante) /*;+NT ;;
*/ )->end) /*;+NT ;;*/ ) /*;+NT ;;*/ )->tmpadj) /*;+NT ;;*/ )->next) /*;
+NT ;;*/ ; a = (( a )->next) /*;};*/ ; } h1 = (( h1 )->next) /*;while (a) {
;*/ ; } } for (i=0; i<=maxdepth+1; i++) { h1 = (( layer[i] ).succlist) /*;
+NT ;;*/ ; while (h1) { (( (( h1 )->node) /*; h1 )->next) /+;R/SC;yy+NT ;
;*/ )->predleft) /*; h1 )->next) /+;R/SC;yy+NT ;;*/ = (( (( h1 )->node) /*;
h1 )->next) /+;R/SC;yy+NT ;;*/ )->predright) /*; h1 )->next) /+;R/SC;
yy+NT ;;*/ = 0; a = (( (( h1 )->node) /*;};*/ )->pred) /*;};*/ ; if (a) { /*;}
0; i<=maxdepth+1; i++) {;*/ (( (( h1 )->node) /*;0; i<=maxdepth+1; i++) {;*/ )->predleft) /*;0; i<=maxdepth+1;
i++) {;*/ = (( a )->kante) /*;0; i<=maxdepth+1; i++) {;*/ ; while ((( a )->next) /*;
i] ).succlist) /+;R/SC;yy+NT ;;*/ ) a = (( a )->next) /*;i] ).succlist) /+;
R/SC;yy+NT ;;*/ ; (( (( h1 )->node) /*;while (h1) {;*/ )->predright) /*;
while (h1) {;*/ = (( a )->kante) /*;while (h1) {;*/ ; } (( (( h1 )->node) /*;
->node) /+;R/SC;yy+NT )->pred) /+;R/SC;;*/ )->succleft) /*;->node) /+;
R/SC;yy+NT )->pred) /+;R/SC;;*/ = (( (( h1 )->node) /*;->node) /+;R/SC;
yy+NT )->pred) /+;R/SC;;*/ )->succright) /*;->node) /+;R/SC;yy+NT )->pred) /+;
R/SC;;*/ = 0; a = (( (( h1 )->node) /*;if (a) {;*/ )->succ) /*;if (a) {
;*/ ; if (a) { (( (( h1 )->node) /*;>next) /+;R/SC;yy+NT ) a = (( a )->next;
*/ )->succleft) /*;>next) /+;R/SC;yy+NT ) a = (( a )->next;*/ = (( a )->kante) /*;
>next) /+;R/SC;yy+NT ) a = (( a )->next;*/ ; while ((( a )->next) /*;
R/SC;yy+NT )->predright) /+;R/SC;yy+NT ;*/ ) a = (( a )->next) /*;R/SC;
yy+NT )->predright) /+;R/SC;yy+NT ;*/ ; (( (( h1 )->node) /*;};*/ )->succright) /*;
};*/ = (( a )->kante) /*;};*/ ; } h1 = (( h1 )->next) /*;->node) /+;
R/SC;yy+NT )->succ) /+;R/SC;;*/ ; } } } static int gs_ide2006 ; 
#define xralign(a)  ((((a)+G_xraster-1)/G_xraster)*G_xraster)
#define xlalign(a)  ((((a)            )/G_xraster)*G_xraster)
#define dxralign(a) ((((a)+G_dxraster-1)/G_dxraster)*G_dxraster)
#define yralign(a)  ((((a)+G_yraster-1)/G_yraster)*G_yraster)
#ifdef ANSI_C
static void gs_ide2018 (void)
#else
static void gs_ide2018 ()
#endif
{ int actypos; int maxboxheight; int i, fresh; GNLIST li; GNODE v; CONNECT c; /*;{
ypos;;*/ ; gs_ide2006 = 0; if (G_yspace<5) G_yspace=5; if (G_xspace<5) G_xspace=5; /*;li;
/N;D+ v;;*/ if (G_dspace==0) { if (G_spline) G_dspace = 4*G_xspace/5; else G_dspace = G_xspace/2; /*;;
006 = 0;;*/ } if (G_flat_factor<1) G_flat_factor = 1; if (G_flat_factor>100) G_flat_factor = 100; /*;ace=5;
if (/_dspace==0) {;*/ actypos = yralign(G_ybase); for (i=0; i<=maxdepth+1; i++) { (( layer[i] ).actx) /*;
= /_xspace*2;;*/ = G_xbase; maxboxheight = 0; li = (( layer[i] ).succlist) /*;
lat_factor<1) /_flat_factor = 1;;*/ ; while (li) { (( (( li )->node) /*;
base);;*/ )->yloc) /*;base);;*/ = actypos; if (maxboxheight<(( (( li )->node) /*;
) {;*/ )->height) /*;) {;*/ ) maxboxheight = (( (( li )->node) /*; /_xbase;
;*/ )->height) /*; /_xbase;;*/ ; li = (( li )->next) /*;xboxheight = 0;
;*/ ; }  if (G_yalign==1 /*;while (li) {;*/ ) { li = (( layer[i] ).succlist) /*;
i )->node) /+;R/SC;yy+NT )->yloc) /+;R/;*/ ; while (li) { (( (( li )->node) /*;
li )->node) /+;R/SC;yy+NT )->height) /+;*/ )->yloc) /*;li )->node) /+;
R/SC;yy+NT )->height) /+;*/ += (maxboxheight- (( (( li )->node) /*;yy+NT ;
;*/ )->height) /*;yy+NT ;;*/ )/2; li = (( li )->next) /*;} ;*/ ; } } /*;while (li) {
i )->node) /+;R/SC;yy+NT )->yloc) /+;R/;*/  else if (G_yalign==2 /*;while (li) {;*/ ) { li = (( layer[i] ).succlist) /*;
i )->node) /+;R/SC;yy+NT )->yloc) /+;R/;*/ ; while (li) { (( (( li )->node) /*;
+;R/SC;yy+NT ;;*/ )->yloc) /*;+;R/SC;yy+NT ;;*/ += (maxboxheight- (( (( li )->node) /*;
};*/ )->height) /*;};*/ ); li = (( li )->next) /*;};*/ ; } } actypos += (maxboxheight + G_yspace); /*;while (li) {
i )->node) /+;R/SC;yy+NT )->yloc) /+;R/;*/ actypos = yralign(actypos); } v = nodelist; while (v) { (( v )->markiert) /*;
};*/ = 1; v = (( v )->next) /*;};*/ ; } v = labellist; while (v) { (( v )->markiert) /*;
+= (maxboxheight + /_yspace);;*/ = 1; v = (( v )->next) /*; += (maxboxheight + /_yspace);
;*/ ; } v = dummylist; while (v) { (( v )->markiert) /*;};*/ = 1; v = (( v )->next) /*;
};*/ ; } gs_ide2012 = 0; for (i=0; i<= maxdepth+1; i++) { li = (( layer[i] ).succlist) /*;
v = labellist;;*/ ; while (li) { v = (( li )->node) /*;;*/ ; fresh = (( v )->markiert) /*;
arkiert) /+;R/SC;yy+NT = 1; v = (( v )-;*/ ; c = (( v )->connection) /*;
gs_ide2012 = 0;;*/ ; if (c) { if (backward_connection1(c)) fresh = 0; /*;while (li) {
li )->node) /+;R/SC;yy+NT ;;*/  if (backward_connection2(c)) fresh = 0; } if (fresh) { gs_wait_message('T'); /*;n) /+;R/SC;yy+NT ;
if (c) {;*/ (void)gs_ide2005 (v,G_xbase+(( v )->width) /*;if (c) {;*/ /2); } li = (( li )->next) /*;
onnection2(c)) fresh = 0;;*/ ; } } }
#ifdef ANSI_C
static int gs_ide2005 (GNODE v, int leftest_pos)
#else
static int gs_ide2005 (v,leftest_pos) GNODE v; int leftest_pos;
#endif
{ int xpos, minpos, maxpos, l, num, i; ADJEDGE a; int minhorder, maxhorder; /*;ide2005 (v,leftest_pos)
/N;D+ v;;*/ GNODE w, actl, actr, actn; GNODE conn1, conn2, leftconn; CONNECT c; ; /*;ADJ+D/+ a;
horder, maxhorder;;*/  if (leftest_pos<0) leftest_pos = 0; if (!v) return(leftest_pos); xpos = leftest_pos; /*; actr, actn;
eftconn;;*/ (( v )->markiert) /*;eftconn;;*/ = 0; if (((( v )->nhorder) /*;C;NN+CT c;
;*/ != -1)&&(!gs_ide2006 )) { gs_ide2006 = 1; if (!silent) { FPRINTF(stderr,"Note: On tree layout "); /*;(leftest_pos);
xpos = leftest_pos;;*/ FPRINTF(stderr,"the attribute `horizontal_order' "); FPRINTF(stderr,"sorts only the child nodes\n"); /*;>markiert) /+;R/SC;yy+NT = 0;
horder) /+;R/SC;yy+NT != -1)&&(!gs_ide2;*/ FPRINTF(stderr,"of a node locally, "); FPRINTF(stderr,"but not the whole layer."); /*;gs_ide2006 = 1;
lent) {;*/ FPRINTF(stderr,"\n"); } } l = (( v )->tiefe) /*;he child nodeszn );;
*/ ; conn1 = conn2 = 0; c = (( v )->connection) /*;RINTF(stderr, but not the whole layer. ;
*/ ; if (c) { if (forward_connection1(c)) { conn1 = (( (( c )->edge) /*;
};*/ )->end) /*;};*/ ; } if (forward_connection2(c)) { conn2 = (( (( c )->edge2) /*;
};*/ )->end) /*;};*/ ; } } if (conn1 && conn2) { if ((( conn1 )->nhorder) /*;
n) /+;R/SC;yy+NT ;;*/ == -1) {  if ((( conn2 )->nhorder) /*;if (c) {
;*/ <(( v )->nhorder) /*;if (c) {;*/ )  gs_ide2005 (conn2,  xpos- (( v )->width) /*;
nnection2(c)) { conn2 = (( (( c )->edge;*/ /2- (( conn2 )->width) /*;
nnection2(c)) { conn2 = (( (( c )->edge;*/ /2-G_xspace); else gs_ide2005 (conn1,  /*;}
n1 && conn2) {;*/ xpos- (( v )->width) /*;n1 && conn2) {;*/ /2- (( conn1 )->width) /*;
n1 && conn2) {;*/ /2-G_xspace); } else if ((( conn1 )->nhorder) /*;er) /+;
R/SC;yy+NT <(( v )->nhorder) /+;;*/ ==(( v )->nhorder) /*;er) /+;R/SC;
yy+NT <(( v )->nhorder) /+;;*/ ) { if ((( conn2 )->nhorder) /*;gs_ide2005 (conn2, ;
*/ <(( v )->nhorder) /*;gs_ide2005 (conn2, ;*/ )  gs_ide2005 (conn2,  xpos- (( v )->width) /*;
5 (conn1, ;*/ /2- (( conn2 )->width) /*;5 (conn1, ;*/ /2-G_xspace); else gs_ide2005 (conn1,  /*;/+;R/SC;yy+NT *2- (( conn1 )->width) /+
};*/ xpos- (( v )->width) /*;};*/ /2- (( conn1 )->width) /*;};*/ /2-G_xspace); /*;}
((( conn1 )->nhorder) /+;R/SC;yy+NT ==;*/ } else if ((( conn1 )->nhorder) /*;->nhorder) /+;R/SC;yy+NT <(( v )->nhord;
*/ <(( v )->nhorder) /*;->nhorder) /+;R/SC;yy+NT <(( v )->nhord;*/ ) { /*;gs_ide2005 (conn2, 
( v )->width) /+;R/SC;yy+NT *2- (( conn;*/  gs_ide2005 (conn1,  xpos- (( v )->width) /*;( v )->width) /+;R/SC;yy+NT *2- (( conn;
*/ /2- (( conn1 )->width) /*;( v )->width) /+;R/SC;yy+NT *2- (( conn;
*/ /2-G_xspace); } else { if ((( conn2 )->nhorder) /*;/+;R/SC;yy+NT *2- (( conn1 )->width) /+;
*/ <(( conn1 )->nhorder) /*;/+;R/SC;yy+NT *2- (( conn1 )->width) /+;
*/ )  gs_ide2005 (conn2,  xpos- (( v )->width) /*; ((( conn1 )->nhorder) /+;
R/SC;yy+NT <(;*/ /2- (( conn2 )->width) /*; ((( conn1 )->nhorder) /+;
R/SC;yy+NT <(;*/ /2-G_xspace); else gs_ide2005 (conn1,  xpos- (( v )->width) /*;
/+;R/SC;yy+NT *2- (( conn1 )->width) /+;*/ /2- (( conn1 )->width) /*;
/+;R/SC;yy+NT *2- (( conn1 )->width) /+;*/ /2-G_xspace); }  } else if (conn1) { /*;nn2, 
/+;R/SC;yy+NT *2- (( conn2 )->width) /+;*/ if ((( conn1 )->nhorder) /*;/+;R/SC;yy+NT *2- (( conn2 )->width) /+;
*/ != -1) { if ((( conn1 )->nhorder) /*;else gs_ide2005 (conn1, ;*/ <(( v )->nhorder) /*;
else gs_ide2005 (conn1, ;*/ ) { gs_ide2005 (conn1, xpos- (( v )->width) /*;
} ;*/ /2- (( conn1 )->width) /*;} ;*/ /2-G_xspace); } } } else if (conn2) { /*;er) /+;R/SC;yy+NT <(( v )->nhorder) /+;
gs_ide2005 (conn1,;*/ if ((( conn2 )->nhorder) /*;gs_ide2005 (conn1,;*/ != -1) { if ((( conn2 )->nhorder) /*;
( v )->width) /+;R/SC;yy+NT *2- (( conn;*/ <(( v )->nhorder) /*;( v )->width) /+;
R/SC;yy+NT *2- (( conn;*/ ) { gs_ide2005 (conn2,  xpos- (( v )->width) /*;
};*/ /2- (( conn2 )->width) /*;};*/ /2-G_xspace); } } } xpos = leftest_pos; /*;er) /+;R/SC;yy+NT <(( v )->nhorder) /+;
gs_ide2005 (conn2, ;*/ if (xpos- (( v )->width) /*;gs_ide2005 (conn2, ;*/ /2 < (( layer[l] ).actx) /*;
gs_ide2005 (conn2, ;*/ )  xpos = (( layer[l] ).actx) /*;( v )->width) /+;
R/SC;yy+NT *2- (( conn;*/ + (( v )->width) /*;( v )->width) /+;R/SC;
yy+NT *2- (( conn;*/ /2; switch ((( v )->outdegree) /*;};*/ ) { case 0: /*;}
};*/ break; case 1: a = (( v )->succ) /*; )->width) /+;R/SC;yy+NT *2 < (( layer[;
*/ ; assert((a)); minpos = gs_ide2005 (((( (( a )->kante) /*;+;R/SC;
yy+NT ) {;*/ )->end) /*;+;R/SC;yy+NT ) {;*/ ) /*;+;R/SC;yy+NT ) {;*/ ,xpos); /*;+;R/SC;yy+NT ) {
case 0:;*/ xpos = minpos; break; default: a = (( v )->succ) /*;cc) /+;R/SC;yy+NT ;
;*/ ; actl = NULL;  minhorder = MAXINT; while (a) { w = ((( (( a )->kante) /*;
break;;*/ )->end) /*;break;;*/ ) /*;break;;*/ ; if ((( w )->nhorder) /*;
:;*/ <minhorder) { minhorder = (( w )->nhorder) /*;cc) /+;R/SC;yy+NT ;
;*/ ; actl = w; } a = (( a )->next) /*;while (a) {;*/ ;  } a = (( v )->succ) /*;
order) /+;R/SC;yy+NT <minhorder) {;*/ ; actr = NULL;  maxhorder = MININT; /*;actl = w;
};*/ while (a) { w = ((( (( a )->kante) /*;a )->next) /+;R/SC;yy+NT ; ;*/ )->end) /*;
a )->next) /+;R/SC;yy+NT ; ;*/ ) /*;a )->next) /+;R/SC;yy+NT ; ;*/ ; /*;}
v )->succ) /+;R/SC;yy+NT ;;*/  if ((w!=actl) && ((( w )->nhorder) /*;};*/ >maxhorder)) { maxhorder = (( w )->nhorder) /*;
v )->succ) /+;R/SC;yy+NT ;;*/ ; actr = w; } a = (( a )->next) /*;while (a) {
;*/ ;  } assert((actl)&&(actr)&&(actl!=actr)); a = (( v )->succ) /*;
horder) /+;R/SC;yy+NT ;;*/ ; num = 0; while (a) { num+=(( ((( (( a )->kante) /*;
a )->next) /+;R/SC;yy+NT ; ;*/ )->end) /*;a )->next) /+;R/SC;yy+NT ;
;*/ ) /*;a )->next) /+;R/SC;yy+NT ; ;*/ )->width) /*;a )->next) /+;
R/SC;yy+NT ; ;*/ ; a = (( a )->next) /*;};*/ ; } num += (((( v )->outdegree) /*;
cc) /+;R/SC;yy+NT ;;*/ -1) * G_xspace); num -= (( actl )->width) /*;
num = 0;;*/ /2; num -= (( actr )->width) /*;a) {;*/ /2; minpos = gs_ide2005 (actl, xpos - (num*gs_ide2016 )/gs_ide2017 ); /*; a )->kante) /+;R/SC;yy+NT )->end) /+;R
R/SC;yy+NT ;;*/   a = (( v )->succ) /*;R/SC;yy+NT ;;*/ ; actn = NULL; minhorder = MAXINT; /*;(((( v )->outdegree) /+;R/SC;yy+NT -1) 
)->width) /+;R/SC;yy+NT *2;;*/ while (a) { w = ((( (( a )->kante) /*;th) /+;R/SC;yy+NT *2;;*/ )->end) /*;
th) /+;R/SC;yy+NT *2;;*/ ) /*;th) /+;R/SC;yy+NT *2;;*/ ; if ((w!=actr) && ((( w )->markiert) /*;
pos - (num*gs_ide2016 )*gs_ide2017 ); ;*/ ) && ((( w )->nhorder) /*;
pos - (num*gs_ide2016 )*gs_ide2017 ); ;*/ <minhorder)) { minhorder = (( w )->nhorder) /*;
a = (( v )->succ) /+;R/SC;yy+NT ;;*/ ; actn = w; } a = (( a )->next) /*;
while (a) {;*/ ;  } i = 2; while (actn) { if (l>spread_level)  maxpos = gs_ide2005 (actn, /*;}
a )->next) /+;R/SC;yy+NT ; ;*/ minpos + (2*(i-1)*(xpos-minpos))/ ((( v )->outdegree) /*;};*/ -1));  else { /*;i = 2;
actn) {;*/ if (i<=((( v )->outdegree) /*;actn) {;*/ +1)/2)  maxpos = gs_ide2005 (actn, /*;evel) 
actn,;*/ minpos + (2*(i-1)*(xpos-minpos))/ ((( v )->outdegree) /*;os))*;*/ -1)); /*;os))*
1)); ;*/   else maxpos = gs_ide2005 (actn, xpos); } if (l>spread_level) { xpos = minpos + ((( v )->outdegree) /*;
-minpos))*;*/ -1)*(maxpos-minpos)/ (2*(i-1)); if (xpos<leftest_pos) xpos = leftest_pos; /*;else
= gs_ide2005 (actn, xpos);;*/ } i++; a = (( v )->succ) /*;pread_level) {;*/ ; actn = NULL; minhorder = MAXINT; /*;(2*(i-1));
s<leftest_pos) xpos = leftest_pos;;*/ while (a) { w = ((( (( a )->kante) /*;};*/ )->end) /*;};*/ ) /*;};*/ ; /*;i++;
v )->succ) /+;R/SC;yy+NT ;;*/  if ( (w!=actr) && ((( w )->markiert) /*;i++;;*/ )  && ((( w )->nhorder) /*;
v )->succ) /+;R/SC;yy+NT ;;*/ <minhorder)) { minhorder = (( w )->nhorder) /*;
actn = NULL;;*/ ; actn = w; } a = (( a )->next) /*; (( a )->kante) /+;
R/SC;yy+NT )->end) /;*/ ;  } } if (l>spread_level)  maxpos = gs_ide2005 (actr, /*;actn = w;
};*/ minpos + (2*(i-1)*(xpos-minpos))/ ((( v )->outdegree) /*;a )->next) /+;
R/SC;yy+NT ; ;*/ -1));  else { if (i<=((( v )->outdegree) /*;};*/ +1)/2)  /*;}
pread_level) ;*/ maxpos = gs_ide2005 (actr, minpos + (2*(i-1)*(xpos-minpos))/ ((( v )->outdegree) /*;
os-minpos))*;*/ -1));  else maxpos = gs_ide2005 (actr, xpos); } if (l>spread_level) { /*;e2005 (actr,
os-minpos))*;*/ xpos = minpos + ((( v )->outdegree) /*;os-minpos))*;*/ -1)*(maxpos-minpos)/ /*;os-minpos))*
yy+NT -1)); ;*/ (2*(i-1)); if (xpos<leftest_pos) xpos = leftest_pos; } xpos = (minpos+maxpos)/2; /*;}
pread_level) {;*/ if (xpos- (( v )->width) /*;pread_level) {;*/ /2 < (( layer[l] ).actx) /*;
pread_level) {;*/ )  xpos = (( layer[l] ).actx) /*;+ ((( v )->outdegree) /+;
R/SC;yy+NT -1);*/ + (( v )->width) /*;+ ((( v )->outdegree) /+;R/SC;
yy+NT -1);*/ /2; } (( v )->xloc) /*;s<leftest_pos) xpos = leftest_pos;
;*/ = xpos - (( v )->width) /*;s<leftest_pos) xpos = leftest_pos;;*/ /2; /*;s<leftest_pos) xpos = leftest_pos;
};*/ if (((( v )->width) /*;};*/ ==0) && ((( v )->height) /*;};*/ ==0)) (( v )->xloc) /*;
(minpos+maxpos)*2;;*/ = dxralign((( v )->xloc) /*;(minpos+maxpos)*2;
;*/ +(( v )->width) /*;(minpos+maxpos)*2;;*/ /2) - (( v )->width) /*;
(minpos+maxpos)*2;;*/ /2; else (( v )->xloc) /*; )->width) /+;R/SC;yy+NT *2 < (( layer[;
*/ = xralign((( v )->xloc) /*; )->width) /+;R/SC;yy+NT *2 < (( layer[;
*/ +(( v )->width) /*; )->width) /+;R/SC;yy+NT *2 < (( layer[;*/ /2) - (( v )->width) /*;
)->width) /+;R/SC;yy+NT *2 < (( layer[;*/ /2; (( layer[l] ).actx) /*;
actx) /+;R/SC;yy+NT + (( v )->width) /+;*/ = (( v )->xloc) /*;actx) /+;
R/SC;yy+NT + (( v )->width) /+;*/ + (( v )->width) /*;actx) /+;R/SC;
yy+NT + (( v )->width) /+;*/ + G_xspace; leftconn = 0; if (conn1 && (!(( conn1 )->markiert) /*;
>xloc) /+;R/SC;yy+NT = xpos - (( v )->w;*/ )) leftconn = conn1; if (conn2 && (!(( conn2 )->markiert) /*;
idth) /+;R/SC;yy+NT ==0) && ((( v )->he;*/ )) leftconn = conn2; if (conn1 && ((( conn1 )->markiert) /*;
;yy+NT = dxralign((( v )->xloc) /+;R/SC;*/ )) (void)gs_ide2005 (conn1,(( layer[l] ).actx) /*;
;yy+NT = dxralign((( v )->xloc) /+;R/SC;*/ ); if (conn2 && ((( conn2 )->markiert) /*;
y+NT = xralign((( v )->xloc) /+;R/SC;yy;*/ )) (void)gs_ide2005 (conn2,(( layer[l] ).actx) /*;
y+NT = xralign((( v )->xloc) /+;R/SC;yy;*/ ); if (leftconn) gs_ide2002 (leftconn,v); /*; (( v )->xloc) /+;R/SC;yy+NT + (( v )->
ftconn = 0;;*/ return(xpos);  }
#define TMINX(x) (( x ).cross)  
#ifdef ANSI_C
static void gs_ide2002 (GNODE conn, GNODE v)
#else
static void gs_ide2002 (conn,v) GNODE conn; GNODE v;
#endif
{ int i, depth, diff; ; gs_wait_message('T'); assert(((( v )->xloc) /*;
/N;D+ conn;;*/ >(( conn )->xloc) /*;/N;D+ conn;;*/ )); for (i=0; i<=maxdepth+1; /*;;
{;*/  i++)  TMINX(layer[i]) = MAXINT;  depth = gs_ide2011 (v,conn); diff = gs_ide2010 (conn,depth); /*;;
_message('T');;*/ if (diff-G_xspace>0) gs_ide2003 (conn, xlalign(diff-G_xspace)); }
#ifdef ANSI_C
static int gs_ide2011 (GNODE v, GNODE conn)
#else
static int gs_ide2011 (v,conn) GNODE v; GNODE conn;
#endif
{ int maxlevel, l, h; ADJEDGE a; CONNECT c; ; if (v==conn) return(0); /*;{
level, l, h;;*/  maxlevel = l = (( v )->tiefe) /*;{;*/ ; if ((( v )->xloc) /*;level, l, h;
;*/ < TMINX(layer[l])) TMINX(layer[l]) = (( v )->xloc) /*;level, l, h;
;*/ ; a = (( v )->succ) /*;ADJ+D/+ a;;*/ ; while (a) { h = gs_ide2011 (((( (( a )->kante) /*;
;;*/ )->end) /*;;;*/ ) /*;;;*/ ,conn); if (h>maxlevel) maxlevel = h; /*; (( v )->tiefe) /+;R/SC;yy+NT ;
R/SC;yy+NT < TyINX(layer[l])) TyINX(lay;*/   a = (( a )->next) /*; (( v )->tiefe) /+;R/SC;yy+NT ;;*/ ; } c = (( v )->connection) /*;
+NT ;;*/ ; if (c) { if (forward_connection1(c)) { h = gs_ide2011 ((( (( c )->edge) /*;
) maxlevel = h; ;*/ )->end) /*;) maxlevel = h; ;*/ ,conn);  if (h>maxlevel) maxlevel = h; /*;R/SC;yy+NT ;
};*/   } if (forward_connection2(c)) { h = gs_ide2011 ((( (( c )->edge2) /*;
if (c) {;*/ )->end) /*;if (c) {;*/ ,conn);  if (h>maxlevel) maxlevel = h; /*;ward_connection1(c)) {
((( (( c )->edge) /+;R/SC;yy+NT )->end;*/   } } return(maxlevel); }
#ifdef ANSI_C
static int gs_ide2010 (GNODE v, int depth)
#else
static int gs_ide2010 (v,depth) GNODE v; int depth;
#endif
{ int mindiff, l, h; ADJEDGE a; CONNECT c; ; l = (( v )->tiefe) /*;th;
;*/ ; if (l>depth) return(0); mindiff = TMINX(layer[l]) - (( v )->xloc) /*;
diff, l, h;;*/ - (( v )->width) /*;diff, l, h;;*/ ; a = (( v )->succ) /*;
ADJ+D/+ a;;*/ ; while (a) { h = gs_ide2010 (((( (( a )->kante) /*;;;
*/ )->end) /*;;;*/ ) /*;;;*/ ,depth); if (h<mindiff) mindiff = h;  a = (( a )->next) /*;
eturn(0);;*/ ; } c = (( v )->connection) /*;+NT ;;*/ ; if (c) { if (forward_connection1(c)) { /*;ide2010 (((( (( a )->kante) /+;R/SC;yy+
mindiff = h; ;*/ h = gs_ide2010 ((( (( c )->edge) /*; mindiff = h; ;*/ )->end) /*; mindiff = h;
;*/ ,depth);  if (h<mindiff) mindiff = h;  } if (forward_connection2(c)) { /*;v )->connection) /+;R/SC;yy+NT ;
if (c) {;*/ h = gs_ide2010 ((( (( c )->edge2) /*;if (c) {;*/ )->end) /*;if (c) {
;*/ ,depth);  if (h<mindiff) mindiff = h;  } } return(mindiff); }
#ifdef ANSI_C
static void gs_ide2003 (GNODE v, int diff)
#else
static void gs_ide2003 (v,diff) GNODE v; int diff;
#endif
{ ADJEDGE a; CONNECT c; ; (( v )->xloc) /*;/N;D+ v;;*/ += diff; a = (( v )->succ) /*;
f;;*/ ; while (a) { gs_ide2003 (((( (( a )->kante) /*; a;;*/ )->end) /*;
a;;*/ ) /*; a;;*/ ,diff); a = (( a )->next) /*;C;NN+CT c;;*/ ; } c = (( v )->connection) /*;
>xloc) /+;R/SC;yy+NT += diff;;*/ ; if (c) { if (forward_connection1(c)) { /*;while (a) {
003 (((( (( a )->kante) /+;R/SC;yy+NT );*/ gs_ide2003 ((( (( c )->edge) /*;003 (((( (( a )->kante) /+;R/SC;yy+NT );
*/ )->end) /*;003 (((( (( a )->kante) /+;R/SC;yy+NT );*/ ,diff);  } if (forward_connection2(c)) { /*;}
v )->connection) /+;R/SC;yy+NT ;;*/ gs_ide2003 ((( (( c )->edge2) /*;v )->connection) /+;R/SC;yy+NT ;;*/ )->end) /*;
v )->connection) /+;R/SC;yy+NT ;;*/ ,diff);  } } }
