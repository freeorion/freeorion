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

#include "XTree.hpp"

using std::string;
using std::vector;
using std::cout;
using std::endl;
//using std::hash_map;

const int XTree::MATCH = 0;
const int XTree::NO_MATCH = -1;
const int XTree::INSERT = -1;
const int XTree::DELETE = -1;
const int XTree::CHANGE = 1;
const int XTree::NULL_NODE = -1;
const int XTree::NO_CONNECTION = 1048576;

namespace {
const int TOP_LEVEL_CAPACITY = 16384;
const int BOT_LEVEL_CAPACITY = 4096;
const int ROOT = 0;
}

XTree::XTree() : 
   _topCap(TOP_LEVEL_CAPACITY),
   _botCap(BOT_LEVEL_CAPACITY),
   _elementIndex(-1),
   _tagIndex(-1),
   _valueCount(BOT_LEVEL_CAPACITY - 1)
{
   _initialize();
}

XTree::XTree(int topcap, int botcap) : 
   _topCap(topcap),
   _botCap(botcap),
   _elementIndex(-1),
   _tagIndex(-1),
   _valueCount(botcap - 1)
{
   _initialize();
}

void XTree::_initialize()
{
   _firstChild.resize(_topCap);
   _nextSibling.resize(_topCap);
   _childrenCount.resize(_topCap);
   _valueIndex.resize(_topCap);
   _matching.resize(_topCap);
   _isAttribute.resize(_topCap);
   _hashValue.resize(_topCap);
   _value.resize(_topCap);
   
   _value[0] = vector<string>(_botCap);
}

void XTree::_expand(int topid)
{
   _firstChild[topid].resize(_botCap, NULL_NODE);
   _nextSibling[topid].resize(_botCap, NULL_NODE);
   _childrenCount[topid].resize(_botCap, 0);
   _valueIndex[topid].resize(_botCap, -1);
   _matching[topid].resize(_botCap, MATCH);
   _isAttribute[topid].resize(_botCap, false);
   _hashValue[topid].resize(_botCap);
}

int XTree::addElement(int pid, int lsid, string tagName)
{
   _elementIndex++;
   
   int topid = _elementIndex / _botCap;
   int botid = _elementIndex % _botCap;
   if (botid == 0)
      _expand(topid);
   
   // Check if we've got the element name
   TagMapType::const_iterator hit = _tagNames.find(tagName);
   if (hit != _tagNames.end()) {
      int   id = hit->second;
      _valueIndex[topid][botid] = id;
   } else {
      _tagIndex++;
      _value[0][_tagIndex] = tagName;
      _tagNames[tagName] = _tagIndex;
      _valueIndex[topid][botid] = _tagIndex;
   }
   
   if (pid == NULL_NODE)
      return _elementIndex;
   
   int ptopid = pid / _botCap;
   int pbotid = pid % _botCap;
   // parent-child relation or sibling-sibling ralation
   if (lsid == NULL_NODE)
      _firstChild[ptopid][pbotid] = _elementIndex;
   else
      _nextSibling[lsid / _botCap][lsid % _botCap] = _elementIndex;
   
   // update children count
   _childrenCount[ptopid][pbotid]++;
   
   return _elementIndex;
}

int XTree::addText(int eid, int lsid, string text, unsigned long long value)
{
   _elementIndex++;
   
   int topid = _elementIndex / _botCap;
   int botid = _elementIndex % _botCap;
   if (botid == 0)
      _expand(topid);
   
   int etopid = eid / _botCap;
   int ebotid = eid % _botCap;
   if (lsid == NULL_NODE)
      _firstChild[etopid][ebotid] = _elementIndex;
   else
      _nextSibling[lsid / _botCap][lsid % _botCap] = _elementIndex;
   
   _childrenCount[etopid][ebotid]++;
   _hashValue[topid][botid] = value;
   
   _valueCount++;
   int vtopid = _valueCount / _botCap;
   int vbotid = _valueCount % _botCap;
   if (vbotid == 0)
      _value[vtopid].resize(_botCap);
   
   _value[vtopid][vbotid] = text;
   _valueIndex[topid][botid] = _valueCount;
   
   return _elementIndex;
}

int XTree::addAttribute(int eid, int lsid, string name, string value, 
                        unsigned long long valuehash, unsigned long long attrhash)
{
   // attribute name first.
   int aid = addElement(eid, lsid, name);
   
   // attribute value second.
   addText(aid, NULL_NODE, value, valuehash);
   
   // hash value third
   int atopid = aid / _botCap;
   int abotid = aid % _botCap;
   _isAttribute[atopid][abotid] = true;
   _hashValue[atopid][abotid] = attrhash;
   
   return aid;
}

void XTree::addHashValue(int eid, unsigned long long value)
{
   _hashValue[eid / _botCap][eid % _botCap] = value;
}

void XTree::addCDATA(int eid, size_t position)
{
   _cdataTable[eid].push_back(position);
}

void XTree::addMatching(int eid, int matchType, int matchNode/* = -1*/)
{
   if (matchType == NO_MATCH)
      _matching[eid / _botCap][eid % _botCap] = NO_MATCH;
   else if (matchType == MATCH)
      _matching[eid / _botCap][eid % _botCap] = MATCH;
   else
      _matching[eid / _botCap][eid % _botCap] = matchNode + 1;
}

void XTree::getMatching(int eid, int &matchType, int &matchNode)
{
   int mid = _matching[eid / _botCap][eid % _botCap];
   if (mid == NO_MATCH) {
      matchType = NO_MATCH;
   } else if (mid == MATCH) {
      matchType = MATCH;
   } else {
      matchType = CHANGE;
      matchNode = mid - 1;
   }
}

int XTree::getRoot()
{
   return ROOT;
}

int XTree::getFirstChild(int eid)
{
   int cid = _firstChild[eid / _botCap][eid % _botCap];
   while (cid > ROOT) {
      int ctopid = cid / _botCap;
      int cbotid = cid % _botCap;
      if (_isAttribute[ctopid][cbotid])
         cid = _nextSibling[ctopid][cbotid];
      else
         return cid;
   }

   return NULL_NODE;
}

int XTree::getNextSibling(int eid)
{
   return _nextSibling[eid / _botCap][eid % _botCap];
}

int XTree::getFirstAttribute(int eid)
{
   int aid = _firstChild[eid / _botCap][eid % _botCap];
   if ((aid > ROOT) && (_isAttribute[aid / _botCap][aid % _botCap]))
      return aid;
   else
      return NULL_NODE;
}

int XTree::getNextAttribute(int aid)
{
   int aid1 = _nextSibling[aid / _botCap][aid % _botCap];
   if ((aid1 > ROOT) && (_isAttribute[aid1 / _botCap][aid1%_botCap]))
      return aid1;
   else
      return NULL_NODE;
}

string XTree::getAttributeValue(int aid)
{
   static const string NULL_STR;
   int cid = _firstChild[aid / _botCap][aid % _botCap];
   int index = _valueIndex[cid / _botCap][cid % _botCap];
   if (index > ROOT)
      return _value[index / _botCap][index % _botCap];
   else
      return NULL_STR;
}

unsigned long long XTree::getHashValue(int eid)
{
   return _hashValue[eid / _botCap][eid % _botCap];
}

vector<size_t>& XTree::getCDATA(int eid)
{
   CDataMapType::iterator hit = _cdataTable.find(eid);
   if (hit != _cdataTable.end()) {
      return hit->second;
   } else {
      static vector<size_t> a;
      return a;
   }
}

int XTree::getChildrenCount(int eid)
{
   return _childrenCount[eid / _botCap][eid % _botCap];
}

int XTree::getDecendentsCount(int eid)
{
   int topid = eid / _botCap;
   int botid = eid % _botCap;
   int count = _childrenCount[topid][botid];
   if (count == 0)
      return 0;
   
   int cid = _firstChild[topid][botid];
   while (cid > ROOT) {
      count += getDecendentsCount(cid);
      cid = _nextSibling[cid / _botCap][cid % _botCap];
   }
   
   return count;
}

int XTree::getValueIndex(int eid)
{
   return _valueIndex[eid / _botCap][eid % _botCap];
}

string XTree::getValue(int index) 
{
   return _value[index / _botCap][index % _botCap];
}

string XTree::getTag(int eid)
{
   int index = _valueIndex[eid / _botCap][eid % _botCap];
   return _value[0][index];
}

string XTree::getText(int eid)
{
   int index = _valueIndex[eid / _botCap][eid % _botCap];
   return _value[index / _botCap][index % _botCap];
}

bool XTree::isElement(int eid)
{
   int index = _valueIndex[eid / _botCap][eid % _botCap];
   if (index < _botCap)
      return true;
   else
      return false;
}

bool XTree::isLeaf(int eid)
{
   int index = _valueIndex[eid / _botCap][eid % _botCap];
   if (index < _botCap)
      return false;
   else
      return true;
}

bool XTree::isAttribute(int eid)
{
   return _isAttribute[eid / _botCap][eid % _botCap];
}

int XTree::getNodeCount()
{
   return _elementIndex;
}

void XTree::dump()
{
   cout << "eid\tfirstC\tnextS\tattr?\tcCount\thash\tmatch\tvalue\n";
   for (int i = ROOT; i <= _elementIndex; i++) {
      int topid = i / _botCap;
      int botid = i % _botCap;
      int vid = _valueIndex[topid][botid];
      int vtopid = vid / _botCap;
      int vbotid = vid % _botCap;
      cout << i << "\t" << _firstChild[topid][botid] << "\t"
         << _nextSibling[topid][botid] << "\t"
         << _isAttribute[topid][botid] << "\t"
         << _childrenCount[topid][botid] << "\t"
         << _hashValue[topid][botid] << "\t"
         << _matching[topid][botid] << "\t"
         << _value[vtopid][vbotid] << endl;
   }
}

