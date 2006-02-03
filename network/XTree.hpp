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

#ifndef   __XTREE__
#define   __XTREE__

#include <iostream>
#include <string>
#include <map> 
#include <vector>

// TODO: use hash map
//#include <hash_map>
/*class HashString
{
public:
   bool operator()(std::string const &str) const
   {
      return std::hash<char const *>()(str.c_str());
   }
};*/

/**
  * XTree provides the storage for input XML documents. Basically, an XML
  * document is parsed by a SAX parser and stored in an XTree.
  */
class XTree
{
public:
   XTree();
   XTree(int topcap, int botcap);

   // Start  -- methods for constructing a tree.

   /**
     * Add a new element to the tree.
     * @param   pid      parent id
     * @param   lsid      left-side sibling id
     * @param   tagName      element name
     * @return   the element id in the tree.
     */
   int addElement(int pid, int lsid, std::string tagName);

   // Add a text node.
   int addText(int eid, int lsid, std::string text, unsigned long long value);

   /**
     * Add an attribute.
     * @param   eid   element id
     * @param   lsid   the sibling id on the left
     * @param   name   attribute name
     * @param   value   attribute value
     * @param   valuehash   hash value of the value
     * @param   attrhash   hash value of the entire attribute
     * @return   the element id of the attribute
     */
   int addAttribute(int eid, int lsid, std::string name, std::string value,
                    unsigned long long valuehash, unsigned long long attrhash);

   // Add more information (hash value) to the tree.
   void addHashValue(int eid, unsigned long long value);

   /**
     * Add a CDATA section (either a start or an end) to the CDATA
     * hashtable, in which each entry should have an even number of
     * position slots. This value is interpreted as a string.
     * @param   eid      The text node id
     * @param   position   the section tag position
     */
   void addCDATA(int eid, size_t position);

   /**
     * Add matching information.
     * @param   eid      element id
     * @param   matchType   match type
     * @param   matchNode   matched element id
     */
   void addMatching(int eid, int matchType, int matchNode = -1);
   // End  -- methods for constructing a tree.

   // Start -- methods for accessing a tree.
   // Get matching information.
   void getMatching(int eid, int &matchType, int &matchNode);

   int getRoot();

   int getFirstChild(int eid);

   int getNextSibling(int eid);

   int getFirstAttribute(int eid);

   int getNextAttribute(int aid);

   std::string getAttributeValue(int aid);

   unsigned long long getHashValue(int eid);

   /**
     * Get the CDATA position list for a text node.
     * @param   eid      The text node id
     * @return   the position vector.
     */
   std::vector<size_t>& getCDATA(int eid);

   int getChildrenCount(int eid);

   int getDecendentsCount(int eid);

   int getValueIndex(int eid);

   std::string getTag(int eid);

   // Get the value of a leaf node using the value index
   std::string getValue(int index);

   // Get the value of a leaf node using the node id
   std::string getText(int eid);

   // Check if a node is an element node.
   bool isElement(int eid);

   // Check if a node is a leaf text node.
   bool isLeaf(int eid);

   // Check if a node is an attribute node
   bool isAttribute(int eid);

   // Return the number of nodes in the tree.
   int getNodeCount();

   // End  -- methods for accessing a tree.
   // For testing purpose.
   void dump();

   static const int MATCH;
   static const int NO_MATCH;
   static const int INSERT;
   static const int DELETE_NODE;
   static const int CHANGE; 
   static const int NULL_NODE;
   static const int NO_CONNECTION;

private:
   typedef std::map<std::string, int> TagMapType; // TODO: use std::hash_map<std::string, int, HashString>
   typedef std::map<int, std::vector<size_t> > CDataMapType; // TODO: use std::hash_map<int, std::vector<size_t> >
   
   void _initialize();
   void _expand(int topid);

   const int   _topCap;
   const int   _botCap;
   int   _elementIndex;
   int   _tagIndex;
   int   _valueCount;
   
   std::vector<std::vector<int> >                  _firstChild;
   std::vector<std::vector<int> >                  _nextSibling;
   std::vector<std::vector<int> >                  _childrenCount;
   std::vector<std::vector<int> >                  _valueIndex;
   std::vector<std::vector<int> >                  _matching;
   std::vector<std::vector<bool> >                 _isAttribute;
   std::vector<std::vector<unsigned long long> >   _hashValue;
   std::vector<std::vector<std::string> >          _value;
   TagMapType                                      _tagNames;
   CDataMapType                                    _cdataTable;
};

#endif

