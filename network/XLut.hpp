/**
  * XDiff -- A part of Niagara Project
  * Author:   Yuan Wang
  *
  * Copyright (c)   Computer Sciences Department,
  *         University of Wisconsin -- Madison
  * All Rights Reserved._
  *
  * Permission to use, copy, modify and distribute this software and
  * its documentation is hereby granted, provided that both the copyright
  * notice and this permission notice appear in all copies of the software,
  * derivative works or modified versions, and any portions thereof, and
  * that both notices appear in supporting documentation._
  *
  * THE AUTHOR AND THE COMPUTER SCIENCES DEPARTMENT OF THE UNIVERSITY OF
  * WISCONSIN - MADISON ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
  * CONDITION, AND THEY DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES
  * WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE._
  *
  * This software was developed with support by DARPA through Rome Research
  * Laboratory Contract No.F30602-97-2-0247.
  *
  * Please report bugs or send your comments to yuanwang@cs.wisc.edu
  */

#ifndef   __XLUT__
#define   __XLUT__

#include <stdio.h>

#ifndef   __XTREE__
#include "XTree.hpp"
#endif

/**
  * XLut is the hash lookup table for distance between nodes. It can be
  * extended to contain more buckets.
  */
class XLut
{
public:

   XLut(bool con);
   ~XLut() {};
   
   /**
     * Add a node pair and their distance to this table.
     * @param   eid1   element id #1
     * @param   eid2   element id #2
     * @param   dist   distance
     */
   void add(int eid1, int eid2, int dist);
   
   /**
     * Get the distance of a node pair.
     * @param   eid1   element id #1
     * @param   eid2   element id #2
     * @return   distance or -1 if not found
     */
   int get(int eid1, int eid2);
   
private:
   typedef std::map<unsigned int, unsigned long long> TableMapType; // TODO: std::hash_map<unsigned int, unsigned long long>
   
   bool _possibleConflict;
   bool _conflict;
   TableMapType _xTable;
};

#endif

