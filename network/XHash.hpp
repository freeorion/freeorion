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

#ifndef   __XHASH__
#define   __XHASH__

#include <string>

/**
  * XHash is an implementation of DES
  */
class XHash
{
public:
   static void initialize();
   static void initialize(unsigned long long key);
   static void finish();
   
   static unsigned long long hash(std::string text);

private:
   static const unsigned long long _initialKey;
   
   // Permuted Choices #1 and #2
   static const unsigned int _PC_1[56];
   static const unsigned int _PC_2[48];
   static const unsigned int _subKeyRotation[16];
   
   // Initial Permutation and Final Permutation (Inverse IP, or IP^(-1))
   static const unsigned int _IP[64];
   static const unsigned int _FP[64];
   
   static const unsigned int _sBox[8][64];
   
   static unsigned long long _key;
   static unsigned long long _keys[];
   
   static void makeKeys(unsigned long long key);
   static unsigned long long permutate(unsigned long long k, const unsigned int p[], unsigned int plen);
   static unsigned long long rotate(unsigned int l, unsigned int r, unsigned int s);
   
   static unsigned long long des(unsigned long long data);
   static int desCore(unsigned int x, unsigned long long k);
};

#endif

