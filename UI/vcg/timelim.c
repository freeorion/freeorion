/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         timelim.c                                          */
/*   version:      1.00.00                                            */
/*   creation:     14.4.1993                                          */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Time Limit Management                              */
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
 * Revision 1.6  1995/02/09  20:15:52  sander
 * Portability problem with HPUX.
 *
 * Revision 1.2  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 1.1  1994/12/23  18:12:45  sander
 * Initial revision
 *
 */


/****************************************************************************
 * This file is a collection of auxiliary functions that implement the
 * time limit management. Note that this feature need not to be available
 * on every computer.
 * The time limit managements allows to limit the layout time. If the time
 * limit is exceeded, the layout automatically switches to fast and ugly 
 * layout. Further, a time limit is not a hard limit: E.g. the layout of
 * 10000 nodes always needs more than 1 second, even if we set the time
 * limit to 1 second.
 *
 * Time limits are real time !!!
 *
 * This file provides the following functions:
 *
 * init_timelimit       gets the limit and initializes the time limit function,
 *			in the case that it was not initialized before.
 * free_timelimit       release the time limit function. This means that the
 *                      next call of reinit_timelimit recognizes, that the
 *                      time limit must be initialized again.
 * test_timelimit       gets the percentual part of the limit and returns 
 *			true, if the time limit is exceeded.
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#ifndef HPUX
#include <sys/time.h>
#else
#ifdef ANSI_C
/* for definitions of timeval and timezone: THIS IS DUE TO A BUG IN THE
 * HP_UX SOURCES. IF THIS BUG DOES NOT EXIST IN NEWER VERSIONS OF THE
 * HP_UX INCLUDES, THIS MUST BE REMOVED. 
 */
     struct timeval {
          unsigned long tv_sec;
          long          tv_usec;
     };
     struct timezone {
        int     tz_minuteswest;
        int     tz_dsttime;
     };
   extern int gettimeofday _PP((struct timeval *, struct timezone *));
#else
#include <sys/time.h>
#endif
#endif
#include "timelim.h"


#undef DEBUG
#undef debugmessage
#ifdef DEBUG
#define debugmessage(a,b) {FPRINTF(stderr,"Debug: %s %s\n",a,b);}
#else
#define debugmessage(a,b) /**/
#endif


/* Local Variables
 * ---------------
 */

static unsigned long timelimit = 0L;	/* the actual time limit in sec */

static struct timeval  tpxstart;	/* the start time     */
static struct timezone tzpxstart;	/* and its time zone  */

static struct timeval  tpxend;  	/* the stop time      */
static struct timezone tzpxend;  	/* and its time zone  */


#ifdef NOTIMELIMIT

/*--------------------------------------------------------------------*/
/* Time limit functions: Dummy's if the time limit is not available   */
/*--------------------------------------------------------------------*/

#ifdef ANSI_C
void init_timelimit(int x)
#else
void init_timelimit(x)
int x;
#endif
{
	debugmessage("init_timelimit","");
}

#ifdef ANSI_C
void free_timelimit(void)
#else
void free_timelimit()
#endif
{
	debugmessage("free_timelimit","");
}


#ifdef ANSI_C
int test_timelimit(int perc)
#else
int test_timelimit(perc)
int perc;
#endif
{
	debugmessage("test_timelimit","");
	return(0);
}

#else

/* Set the time limit to x seconds and start the clock
 * ---------------------------------------------------
 * This is only done if it was not done before.
 */

#ifdef ANSI_C
void init_timelimit(int x)
#else
void init_timelimit(x)
int x;
#endif
{
	debugmessage("init_timelimit","");
	if (timelimit>0L) return;	
	if (x<1) x = 1;
	timelimit = (unsigned long)x;
	gettimeofday(&tpxstart,&tzpxstart);
}



/* Free the actual time limit
 * --------------------------
 */

#ifdef ANSI_C
void free_timelimit(void)
#else
void free_timelimit()
#endif
{
	debugmessage("free_timelimit","");
	timelimit = (unsigned long)0;
}




/* Check whether the percentual time limit is reached
 * --------------------------------------------------
 * Return true (1) if yes.
 */

#ifdef ANSI_C
int test_timelimit(int perc)
#else
int test_timelimit(perc)
int perc;
#endif
{
	unsigned long sec;
	long int usec;
	unsigned long tval;

	debugmessage("test_timelimit","");

	gettimeofday(&tpxend,&tzpxend);
	sec = tpxend.tv_sec-tpxstart.tv_sec;	
	usec = tpxend.tv_usec-tpxstart.tv_usec;	
	if (usec<0) { sec--; usec += 1000000; }
	tval = (perc*timelimit)/100L;
	if (tval==0L) tval = 1L;
	if (sec > tval) return(1); 
	return(0);
}

#endif /* NOTIMELIMIT */


