/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         timing.h                                           */
/*   version:      1.00.00                                            */
/*   creation:     11.11.93                                           */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Time measurement for debugging/profiling           */
/*   status:       in work                                            */
/*                                                                    */
/*--------------------------------------------------------------------*/


/* $Id$*/


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
 * Revision 3.3  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.2  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 * infobox behaviour improved.
 * First version of fisheye (carthesian).
 * Options Noedge and nonode.
 * Titles in the node title box are now sorted.
 * Timelimit functionality improved.
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
 * Revision 2.1  1993/12/08  21:21:34  sander
 * Reasonable fast and stable version
 *
 *
 */

#ifndef TIMING_H
#define TIMING_H

#ifndef CHECK_TIMING
#define start_time() /**/
#define stop_time(x) /**/
#else

/************************************************************************
 *	Time measurement for debugging. 
 *	This works with Sun Os 4.2.1 on Sparcstations. I don't know
 *	whether it works anywhere else.
 *	No further comments: THIS UGLY FILE IS ONLY FOR ME !!! (GS)
 ************************************************************************/


#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>

#ifndef RUSAGE_SELF
#define RUSAGE_SELF 0
#endif
#ifndef RUSAGE_CHILDREN
#define RUSAGE_CHILDREN -1
#endif

static unsigned long start_syssec;
static unsigned long start_sysmycsec;
static unsigned long start_usersec;
static unsigned long start_usermycsec;
struct timeval  tpstart;
struct timezone tzpstart;

static void start_time	_PP((void));

extern int  getrusage	 _PP((int x,struct rusage *r));
#ifdef __cplusplus 
#define  gettimeofday(a,b)  (0)
#else
extern int  gettimeofday _PP((struct timeval *tp, struct timezone *tzp));
#endif


#ifdef ANSI_C
static void start_time(void)
#else
static void start_time()
#endif
{
	struct rusage r;

	getrusage(RUSAGE_SELF,&r);

	start_syssec     = r.ru_stime.tv_sec;
	start_sysmycsec  = r.ru_stime.tv_usec;
	start_usersec    = r.ru_utime.tv_sec;
	start_usermycsec = r.ru_utime.tv_usec;
	gettimeofday(&tpstart,&tzpstart);
}

static unsigned long stop_syssec;
static unsigned long stop_sysmycsec;
static unsigned long stop_usersec;
static unsigned long stop_usermycsec;
struct timeval  tpend;
struct timezone tzpend;

static void stop_time	_PP((char *x));

#ifdef ANSI_C
static void stop_time(char *x)
#else
static void stop_time(x)
char *x;
#endif
{
	struct rusage r;
	unsigned long sec;
	long int usec;

	gettimeofday(&tpend,&tzpend);
	getrusage(RUSAGE_SELF,&r);

	stop_syssec     = r.ru_stime.tv_sec;
	stop_sysmycsec  = r.ru_stime.tv_usec;
	stop_usersec    = r.ru_utime.tv_sec;
	stop_usermycsec = r.ru_utime.tv_usec;

	sec = stop_usersec - start_usersec;
	usec = stop_usermycsec - start_usermycsec;
	if (usec<0) { sec--; usec += 1000000; }
	(void)printf( "%s:\n",x);
	(void)printf( "Time: User: %ld.%03ld sec ",
		sec, usec/1000);
	sec = stop_syssec - start_syssec;
	usec = stop_sysmycsec - start_sysmycsec;
	if (usec<0) { sec--; usec += 1000000; }
	(void)printf( "System: %ld.%03ld sec ",
		sec, usec/1000);
	sec = tpend.tv_sec-tpstart.tv_sec;	
	usec = tpend.tv_usec-tpstart.tv_usec;	
	if (usec<0) { sec--; usec += 1000000; }
	(void)printf( "Real: %ld.%03ld sec\n",sec,usec/1000);
}

#endif /* CHECK_TIMING */

#endif /* TIMING_H */

