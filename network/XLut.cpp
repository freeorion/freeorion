/**
  * XDiff -- A part of Niagara Project
  * Author:	Yuan Wang
  *
  * Copyright (c)	Computer Sciences Department,
  *			University of Wisconsin -- Madison
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

#include "XLut.hpp"

XLut::XLut(bool con) : 
   _possibleConflict(con),
   _conflict(false)
{
}

void XLut::add(int eid1, int eid2, int dist)
{
   unsigned int	key = ((unsigned int)eid1 << 16) | (eid2 & 0xffff);
   unsigned long long	value = ((unsigned long long)eid1 << 40) | ((unsigned long long)(eid2 & 0xffffff) << 16) | dist;
   if (_possibleConflict) {
      TableMapType::const_iterator hit = _xTable.find(key); //hash_map<unsigned int, unsigned long long>::const_iterator hit = _xTable.find(key);
      if (hit == _xTable.end()) {
         _xTable[key] = value;
      } else {
         do {
            key++;
            hit = _xTable.find(key);
         } while (hit != _xTable.end());
         _xTable[key] = value;
         _conflict = true;
      }
   } else {
   	_xTable[key] = value;
   }
}

int XLut::get(int eid1, int eid2)
{
   unsigned int	key = ((unsigned int)eid1 << 16) | (eid2 & 0xffff);
   TableMapType::const_iterator hit = _xTable.find(key); //hash_map<unsigned int, unsigned long long>::const_iterator hit = _xTable.find(key);
   if (hit == _xTable.end())
      return XTree::NO_CONNECTION;
   
   if (!_conflict) {
      return (int)(hit->second & 0xffff);
   } else {
      unsigned long long	partialValue = ((unsigned long long)eid1 << 40) | ((unsigned long long)(eid2 & 0xffffff) << 16); 
      unsigned long long	bucket = hit->second;
      if (((partialValue ^ bucket) >> 16) == 0) {
         return (int)(bucket & 0xffff);
      } else {
         do {
            key++;
            hit = _xTable.find(key);
            if (hit == _xTable.end())
               return XTree::NO_CONNECTION;
            bucket = hit->second;
         } while (((partialValue ^ bucket) >> 16) != 0);
         return (int)(bucket & 0xffff);
      }
   }
}
