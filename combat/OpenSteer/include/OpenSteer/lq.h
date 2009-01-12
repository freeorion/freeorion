/*
// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2005, Sony Computer Entertainment America
// Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// ----------------------------------------------------------------------------
*/

/* ------------------------------------------------------------------ */
/*                                                                    */
/*                   Locality Query (LQ) Facility                     */
/*                                                                    */
/* ------------------------------------------------------------------ */
/*

    This utility is a spatial database which stores objects each of
    which is associated with a 3d point (a location in a 3d space).
    The points serve as the "search key" for the associated object.
    It is intended to efficiently answer "sphere inclusion" queries,
    also known as range queries: basically questions like:

        Which objects are within a radius R of the location L?

    In this context, "efficiently" means significantly faster than the
    naive, brute force O(n) testing of all known points.  Additionally
    it is assumed that the objects move along unpredictable paths, so
    that extensive preprocessing (for example, constructing a Delaunay
    triangulation of the point set) may not be practical.

    The implementation is a "bin lattice": a 3d rectangular array of
    brick-shaped (rectangular parallelepipeds) regions of space.  Each
    region is represented by a pointer to a (possibly empty) doubly-
    linked list of objects.  All of these sub-bricks are the same
    size.  All bricks are aligned with the global coordinate axes.

    Terminology used here: the region of space associated with a bin
    is called a sub-brick.  The collection of all sub-bricks is called
    the super-brick.  The super-brick should be specified to surround
    the region of space in which (almost) all the key-points will
    exist.  If key-points move outside the super-brick everything will
    continue to work, but without the speed advantage provided by the
    spatial subdivision.  For more details about how to specify the
    super-brick's position, size and subdivisions see lqCreateDatabase
    below.

    Overview of usage: an application using this facility would first
    create a database with lqCreateDatabase.  For each client object
    the application wants to put in the database it creates a
    lqClientProxy and initializes it with lqInitClientProxy.  When a
    client object moves, the application calls lqUpdateForNewLocation.
    To perform a query lqMapOverAllObjectsInLocality is passed an
    application-supplied call-back function to be applied to all
    client objects in the locality.  See lqCallBackFunction below for
    more detail.  The lqFindNearestNeighborWithinRadius function can
    be used to find a single nearest neighbor using the database.

    Note that "locality query" is also known as neighborhood query,
    neighborhood search, near neighbor search, and range query.  For
    additional information on this and related topics see:
    http://www.red3d.com/cwr/boids/ips.html

    For some description and illustrations of this database in use,
    see this paper: http://www.red3d.com/cwr/papers/2000/pip.html

*/

#ifndef	_lq_h
#define	_lq_h

#ifdef __cplusplus
extern "C"
{
#endif


/* ------------------------------------------------------------------ */
/*                                                                    */
/*                       Data types use by LQ                         */
/*                                                                    */
/* ------------------------------------------------------------------ */
/* This structure represents the spatial database.  Typically one of
   these would be created (by a call to lqCreateDatabase) for a given
   application.  */


typedef struct lqInternalDB lqDB;


/* ------------------------------------------------------------------ */
/* This structure is a proxy for (and contains a pointer to) a client
   (application) object in the spatial database.  One of these exists
   for each client object.  This might be included within the
   structure of a client object, or could be allocated separately.  */


typedef struct lqClientProxy
{
    /* previous object in this bin, or NULL */
    struct lqClientProxy*  prev;

    /* next object in this bin, or NULL */
    struct lqClientProxy*  next;

    /* bin ID (pointer to pointer to bin contents list) */
    struct lqClientProxy** bin;

    /* pointer to client object */
    void* object;

    /* the object's location ("key point") used for spatial sorting */
    float x;
    float y;
    float z;
} lqClientProxy;


/* ------------------------------------------------------------------ */
/*                                                                    */
/*                            Basic API                               */
/*                                                                    */
/* ------------------------------------------------------------------ */
/* Allocate and initialize an LQ database, returns a pointer to it.
   The application needs to call this before using the LQ facility.
   The nine parameters define the properties of the "super-brick":
      (1) origin: coordinates of one corner of the super-brick, its
          minimum x, y and z extent.
      (2) size: the width, height and depth of the super-brick.
      (3) the number of subdivisions (sub-bricks) along each axis.
   This routine also allocates the bin array, and initialize its
   contents. */


lqDB* lqCreateDatabase (float originx, float originy, float originz,
			float sizex,   float sizey,   float sizez,
			int   divx,    int   divy,    int   divz);


/* ------------------------------------------------------------------ */
/* Deallocates the LQ database */


void lqDeleteDatabase (lqDB*);


/* ------------------------------------------------------------------ */
/* The application needs to call this once on each lqClientProxy at
   setup time to initialize its list pointers and associate the proxy
   with its client object. */ 


void lqInitClientProxy (lqClientProxy* proxy, void* clientObject);


/* ------------------------------------------------------------------ */
/* Call for each client object every time its location changes.  For
   example, in an animation application, this would be called each
   frame for every moving object.  */


void lqUpdateForNewLocation (lqDB* lq, 
			     lqClientProxy* object, 
			     float x, float y, float z);


/* ------------------------------------------------------------------ */
/* Apply an application-specific function to all objects in a certain
   locality.  The locality is specified as a sphere with a given
   center and radius.  All objects whose location (key-point) is
   within this sphere are identified and the function is applied to
   them.  The application-supplied function takes three arguments:

     (1) a void* pointer to an lqClientProxy's "object".
     (2) the square of the distance from the center of the search
         locality sphere (x,y,z) to object's key-point.
     (3) a void* pointer to the caller-supplied "client query state"
         object -- typically NULL, but can be used to store state
         between calls to the lqCallBackFunction.

   This routine uses the LQ database to quickly reject any objects in
   bins which do not overlap with the sphere of interest.  Incremental
   calculation of index values is used to efficiently traverse the
   bins of interest. */


/* type for a pointer to a function used to map over client objects */
typedef void (* lqCallBackFunction)  (void* clientObject,
				      float distanceSquared,
				      void* clientQueryState);


void lqMapOverAllObjectsInLocality (lqDB* lq, 
				    float x, float y, float z,
				    float radius,
				    lqCallBackFunction func,
				    void* clientQueryState);


/* ------------------------------------------------------------------ */
/*                                                                    */
/*                            Other API                               */
/*                                                                    */
/* ------------------------------------------------------------------ */
/* Search the database to find the object whose key-point is nearest
   to a given location yet within a given radius.  That is, it finds
   the object (if any) within a given search sphere which is nearest
   to the sphere's center.  The ignoreObject argument can be used to
   exclude an object from consideration (or it can be NULL).  This is
   useful when looking for the nearest neighbor of an object in the
   database, since otherwise it would be its own nearest neighbor.
   The function returns a void* pointer to the nearest object, or
   NULL if none is found.  */


void* lqFindNearestNeighborWithinRadius (lqDB* lq, 
					 float x, float y, float z,
					 float radius,
					 void* ignoreObject);


/* ------------------------------------------------------------------ */
/* Adds a given client object to a given bin, linking it into the bin
   contents list. */


void lqAddToBin (lqClientProxy* object, lqClientProxy** bin);


/* ------------------------------------------------------------------ */
/* Removes a given client object from its current bin, unlinking it
   from the bin contents list. */


void lqRemoveFromBin (lqClientProxy* object);


/* ------------------------------------------------------------------ */
/* Given an LQ database object and the nine basic parameters: fill in
   the object's slots, allocate the bin array, and initialize its
   contents.  Normally the application does NOT call this directly, it
   is called by lqCreateDatabase.  */


void lqInitDatabase (lqDB* lq,
		     float originx, float originy, float originz,
		     float sizex, float sizey, float sizez,
		     int divx, int divy, int divz);


/* ------------------------------------------------------------------ */
/* Find the bin ID for a location in space.  The location is given in
   terms of its XYZ coordinates.  The bin ID is a pointer to a pointer
   to the bin contents list.  */


lqClientProxy** lqBinForLocation (lqDB* lq, float x, float y, float z);


/* ------------------------------------------------------------------ */
/* Apply a user-supplied function to all objects in the database,
   regardless of locality (cf lqMapOverAllObjectsInLocality) */


void lqMapOverAllObjects (lqDB* lq, 
			  lqCallBackFunction func,
			  void* clientQueryState);


/* ------------------------------------------------------------------ */
/* Removes (all proxies for) all objects from all bins */


void lqRemoveAllObjects (lqDB* lq);


/* ------------------------------------------------------------------ */
/* Get statistics about bin populations: min, max and average of
   non-empty bins. */


#ifndef NO_LQ_BIN_STATS
void lqGetBinPopulationStats (lqDB* lq,
                              int* min,
                              int* max,
                              float* average);
#endif /* NO_LQ_BIN_STATS */

/* ------------------------------------------------------------------ */


#ifndef	NULL
#define NULL 0
#endif


/* ------------------------------------------------------------------ */

#ifdef __cplusplus
}
#endif

#endif /* _lq_h */
