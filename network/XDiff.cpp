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

#include "XDiff.hpp"
#include "XTree.hpp"
#include "XLut.hpp"
#include "XParser.hpp"

#include "../GG/XML/XMLDoc.h"

#include <ctime>
#include <sstream>

namespace {

using std::cout;
using std::cerr;
using std::ios;
using std::ostream;
using std::istream;
using std::ofstream;
using std::ifstream;
using std::string;
using std::vector;

const int CIRCUIT_SIZE = 2048;
const int MATRIX_SIZE = 1024;
const int ATTRIBUTE_SIZE = 1024;
const int TEXT_SIZE = 1024;

/* You can use a different number for sampling here, as sqrt(n) is a safe
   one, though 3 works pretty well. */
const int      sampleCount = 3;
const string   INDENT_STR("  ");
const string   CHANGE_ATTR_STR("XDchg");
const string   INSERT_PREFIX("INS ");
const string   DELETE_PREFIX("DEL ");
const string   UPDATE_TEXT_PREFIX("TXT ");
const string   DELETEATTR_PREFIX("DELATTR ");
const string   UPDATEATTR_PREFIX("UPDATTR ");
const string   REDOATTR_PREFIX("REDOATTR ");
double         NO_MATCH_THRESHOLD = 0.3;
bool           oFlag = false;
bool           gFlag = false;
}

XDiff::XDiff(const string& input_file_1, const string& input_file_2, const string& output_file) : 
   _attrList1(ATTRIBUTE_SIZE),
   _attrList2(ATTRIBUTE_SIZE),
   _textList1(TEXT_SIZE),
   _textList2(TEXT_SIZE),
   _circuit(CIRCUIT_SIZE),
   _seed(time(0)),
   _leastCostMatrix(MATRIX_SIZE, vector<int>(MATRIX_SIZE)),
   _pathMatrix(MATRIX_SIZE, vector<int>(MATRIX_SIZE)),
   _attrMatch(ATTRIBUTE_SIZE),
   _textMatch1(TEXT_SIZE),
   _textMatch2(TEXT_SIZE),
   _needNewLine(false),
   _attrHash(ATTRIBUTE_SIZE),
   _textHash(TEXT_SIZE),
   _attrTag(ATTRIBUTE_SIZE),
   _xtree1(0),
   _xtree2(0),
   _xlut(0)
{
   // parse the first file.
   XParser parser1;
   _xtree1 = parser1.parse(input_file_1);

   // parse the second file.
   XParser parser2;
   _xtree2 = parser2.parse(input_file_2);

   // check both root nodes.
   int root1 = _xtree1->getRoot();
   int root2 = _xtree2->getRoot();
   if (_xtree1->getHashValue(root1) != _xtree2->getHashValue(root2)) {
      if (_xtree1->getTag(root1).compare(_xtree2->getValue(root2)) != 0) {
         _xtree1->addMatching(root1, XTree::DELETE, root2);
         _xtree2->addMatching(root2, XTree::INSERT, root1);
      } else {
         _xtree1->addMatching(root1, XTree::CHANGE, root2);
         _xtree2->addMatching(root2, XTree::CHANGE, root1);
         _xlut = new XLut((_xtree1->getNodeCount() > 0xffff) ||
                (_xtree2->getNodeCount() > 0xffff));
         xdiff(root1, root2, false);
      }
      writeDiff(input_file_1, output_file);
   }
}

XDiff::XDiff(const GG::XMLDoc& doc1, const GG::XMLDoc& doc2, string& output, bool whitespace/* = false*/) : 
   _attrList1(ATTRIBUTE_SIZE),
   _attrList2(ATTRIBUTE_SIZE),
   _textList1(TEXT_SIZE),
   _textList2(TEXT_SIZE),
   _circuit(CIRCUIT_SIZE),
   _seed(time(0)), 
   _leastCostMatrix(MATRIX_SIZE, vector<int>(MATRIX_SIZE)),
   _pathMatrix(MATRIX_SIZE, vector<int>(MATRIX_SIZE)),
   _attrMatch(ATTRIBUTE_SIZE),
   _textMatch1(TEXT_SIZE),
   _textMatch2(TEXT_SIZE),
   _needNewLine(false),
   _attrHash(ATTRIBUTE_SIZE),
   _textHash(TEXT_SIZE),
   _attrTag(ATTRIBUTE_SIZE),
   _xtree1(0),
   _xtree2(0),
   _xlut(0)
{
   // parse the first file.
   XParser parser1;
   _xtree1 = parser1.parse(doc1);

   // parse the second file.
   XParser parser2;
   _xtree2 = parser2.parse(doc2);

   // check both root nodes.
   int root1 = _xtree1->getRoot();
   int root2 = _xtree2->getRoot();
   if (_xtree1->getHashValue(root1) != _xtree2->getHashValue(root2)) {
      if (_xtree1->getTag(root1).compare(_xtree2->getValue(root2)) != 0) {
         _xtree1->addMatching(root1, XTree::DELETE, root2);
         _xtree2->addMatching(root2, XTree::INSERT, root1);
      } else {
         _xtree1->addMatching(root1, XTree::CHANGE, root2);
         _xtree2->addMatching(root2, XTree::CHANGE, root1);
         _xlut = new XLut((_xtree1->getNodeCount() > 0xffff) ||
                (_xtree2->getNodeCount() > 0xffff));
         xdiff(root1, root2, false);
      }
      std::stringstream diff_result;
      writeDocDiff(diff_result, whitespace);
      output = diff_result.str();
   }
}

XDiff::XDiff(const GG::XMLDoc& doc1, const GG::XMLDoc& doc2, GG::XMLDoc& output, bool whitespace/* = false*/) : 
   _attrList1(ATTRIBUTE_SIZE),
   _attrList2(ATTRIBUTE_SIZE),
   _textList1(TEXT_SIZE),
   _textList2(TEXT_SIZE),
   _circuit(CIRCUIT_SIZE),
   _seed(time(0)), 
   _leastCostMatrix(MATRIX_SIZE, vector<int>(MATRIX_SIZE)),
   _pathMatrix(MATRIX_SIZE, vector<int>(MATRIX_SIZE)),
   _attrMatch(ATTRIBUTE_SIZE),
   _textMatch1(TEXT_SIZE),
   _textMatch2(TEXT_SIZE),
   _needNewLine(false),
   _attrHash(ATTRIBUTE_SIZE),
   _textHash(TEXT_SIZE),
   _attrTag(ATTRIBUTE_SIZE),
   _xtree1(0),
   _xtree2(0),
   _xlut(0)
{
   // parse the first file.
   XParser parser1;
   _xtree1 = parser1.parse(doc1);

   // parse the second file.
   XParser parser2;
   _xtree2 = parser2.parse(doc2);

   // check both root nodes.
   int root1 = _xtree1->getRoot();
   int root2 = _xtree2->getRoot();
   if (_xtree1->getHashValue(root1) != _xtree2->getHashValue(root2)) {
      if (_xtree1->getTag(root1).compare(_xtree2->getValue(root2)) != 0) {
         _xtree1->addMatching(root1, XTree::DELETE, root2);
         _xtree2->addMatching(root2, XTree::INSERT, root1);
      } else {
         _xtree1->addMatching(root1, XTree::CHANGE, root2);
         _xtree2->addMatching(root2, XTree::CHANGE, root1);
         _xlut = new XLut((_xtree1->getNodeCount() > 0xffff) ||
                (_xtree2->getNodeCount() > 0xffff));
         xdiff(root1, root2, false);
      }
      std::stringstream diff_result;
      writeDocDiff(diff_result, whitespace);
      output.ReadDoc(diff_result);
   }
}

XDiff::~XDiff()
{
   delete _xtree1;
   delete _xtree2;
   delete _xlut;
}

void XDiff::xdiff(int pid1, int pid2, bool matchFlag)
{
   // diff attributes.
   int   attrCount1 = 0, attrCount2 = 0;
   int   attr1 = _xtree1->getFirstAttribute(pid1);
   while (attr1 != XTree::NULL_NODE)
   {
      _attrList1[attrCount1++] = attr1;
      attr1 = _xtree1->getNextAttribute(attr1);
   }
   int   attr2 = _xtree2->getFirstAttribute(pid2);
   while (attr2 != XTree::NULL_NODE)
   {
      _attrList2[attrCount2++] =  attr2;
      attr2 = _xtree2->getNextAttribute(attr2);
   }
   if (attrCount1 > 0)
   {
      if (attrCount2 > 0)
         diffAttributes(attrCount1, attrCount2);
      else
      {
         for (int i = 0; i < attrCount1; i++)
            _xtree1->addMatching(_attrList1[i],
                       XTree::NO_MATCH);
      }
   }
   else if (attrCount2 > 0) // attrCount1 == 0
   {
      for (int i = 0; i < attrCount2; i++)
         _xtree2->addMatching(_attrList2[i], XTree::NO_MATCH);
   }

   // Match element nodes.
   int   count1 = _xtree1->getChildrenCount(pid1) - attrCount1;
   int   count2 = _xtree2->getChildrenCount(pid2) - attrCount2;

   if ((count1 == 0) && (count2 == 0))
   {
   }
   else if (count1 == 0)
   {
      int   node2 = _xtree2->getFirstChild(pid2);
      _xtree2->addMatching(node2, XTree::NO_MATCH);
      for (int i = 1; i < count2; i++)
      {
         node2 = _xtree2->getNextSibling(node2);
         _xtree2->addMatching(attr2, XTree::NO_MATCH);
      }
   }
   else if (count2 == 0)
   {
      int   node1 = _xtree1->getFirstChild(pid1);
      _xtree1->addMatching(node1, XTree::NO_MATCH);
      for (int i = 0; i < count1; i++)
      {
         node1 = _xtree1->getNextSibling(node1);
         _xtree1->addMatching(node1, XTree::NO_MATCH);
      }
   }
   else if ((count1 == 1) && (count2 == 1))
   {
      int   node1 = _xtree1->getFirstChild(pid1);
      int   node2 = _xtree2->getFirstChild(pid2);

      if (_xtree1->getHashValue(node1) == _xtree2->getHashValue(node2))
         return;

      bool   isE1 = _xtree1->isElement(node1);
      bool   isE2 = _xtree2->isElement(node2);

      if (isE1 && isE2)
      {
         if (_xtree1->getTag(node1).compare(_xtree2->getTag(node2)) == 0)
         {
            _xtree1->addMatching(node1, XTree::CHANGE, node2);
            _xtree2->addMatching(node2, XTree::CHANGE, node1);
            xdiff(node1, node2, matchFlag);
         }
         else
         {
            _xtree1->addMatching(node1, XTree::NO_MATCH);
            _xtree2->addMatching(node2, XTree::NO_MATCH);
         }
      }
      else if (!isE1 && !isE2)
      {
         _xtree1->addMatching(node1, XTree::CHANGE, node2);
         _xtree2->addMatching(node2, XTree::CHANGE, node1);
      }
      else
      {
         _xtree1->addMatching(node1, XTree::NO_MATCH);
         _xtree2->addMatching(node2, XTree::NO_MATCH);
      }
   }
   else
   {
      int   *elements1 = new int[count1];
      int   *elements2 = new int[count2];
      int   elementCount1 = 0, textCount1 = 0;
      int   elementCount2 = 0, textCount2 = 0;

      int   child1 = _xtree1->getFirstChild(pid1);
      if (_xtree1->isElement(child1))
         elements1[elementCount1++] = child1;
      else
         _textList1[textCount1++] = child1;
      for (int i = 1; i < count1; i++)
      {
         child1 = _xtree1->getNextSibling(child1);
         if (_xtree1->isElement(child1))
            elements1[elementCount1++] = child1;
         else
            _textList1[textCount1++] = child1;
      }

      int   child2 = _xtree2->getFirstChild(pid2);
      if (_xtree2->isElement(child2))
         elements2[elementCount2++] = child2;
      else
         _textList2[textCount2++] = child2;
      for (int i = 1; i < count2; i++)
      {
         child2 = _xtree2->getNextSibling(child2);
         if (_xtree2->isElement(child2))
            elements2[elementCount2++] = child2;
         else
            _textList2[textCount2++] = child2;
      }

      // Match text nodes.
      if (textCount1 > 0)
      {
         if (textCount2 > 0)
            diffText(textCount1, textCount2);
         else
         {
            for (int i = 0; i < textCount1; i++)
               _xtree1->addMatching(_textList1[i],
                          XTree::NO_MATCH);
         }
      }
      else if (textCount2 > 0)   // textCount1 == 0
      {
         for (int i = 0; i < textCount2; i++)
            _xtree2->addMatching(_textList2[i],
                       XTree::NO_MATCH);
      }

      bool   *matched1 = new bool[elementCount1];
      bool   *matched2 = new bool[elementCount2];
      int   mcount = _matchFilter(elements1, elements2,
                     matched1, matched2,
                     elementCount1, elementCount2);

      if ((elementCount1 == mcount) &&
          (elementCount2 == mcount))
      {
      }
      else if (elementCount1 == mcount)
      {
         for (int i = 0; i < elementCount2; i++)
         {
            if (!matched2[i])
               _xtree2->addMatching(elements2[i],
                          XTree::NO_MATCH);
         }
      }
      else if (elementCount2 == mcount)
      {
         for (int i = 0; i < elementCount1; i++)
         {
            if (!matched1[i])
               _xtree1->addMatching(elements1[i],
                          XTree::NO_MATCH);
         }
      }
      else
      {
         // Write the list of unmatched nodes.
         int   ucount1 = elementCount1 - mcount;
         int   ucount2 = elementCount2 - mcount;
         int   *unmatched1 = new int[ucount1];
         int   *unmatched2 = new int[ucount2];
         int   muc1 = 0, muc2 = 0;
         int   start = 0;

         while ((muc1 < ucount1) && (muc2 < ucount2))
         {
            for (; (start < elementCount1) && matched1[start]; start++);
            string startTag = _xtree1->getTag(elements1[start]);
            int uele1 = 0, uele2 = 0;
            muc1++;
            unmatched1[uele1++] = elements1[start];
            matched1[start++] = true;

            for (int i = start; (i < elementCount1) && (muc1 < ucount1); i++)
            {
               if (!matched1[i] && (startTag.compare(_xtree1->getTag(elements1[i])) == 0))
               {
                  matched1[i] = true;
                  muc1++;
                  unmatched1[uele1++] = elements1[i];
               }
            }

            for (int i = 0; (i < elementCount2) && (muc2 < ucount2); i++)
            {
               if (!matched2[i] && (startTag.compare(_xtree2->getTag(elements2[i])) == 0))
               {
                  matched2[i] = true;
                  muc2++;
                  unmatched2[uele2++] = elements2[i];
               }
            }

            if (uele2 == 0)
            {
               for (int i = 0; i < uele1; i++)
                  _xtree1->addMatching(unmatched1[i], XTree::NO_MATCH);
            }
            else
            {
               if ((uele1 == 1) && (uele2 == 1))
               {
                  _xtree1->addMatching(unmatched1[0], XTree::CHANGE, unmatched2[0]);
                  _xtree2->addMatching(unmatched2[0], XTree::CHANGE, unmatched1[0]);
                  xdiff(unmatched1[0],
                     unmatched2[0], matchFlag);
               }
               // To find minimal-cost matching between those unmatched elements (with the same tag name.
               else if (uele1 >= uele2)
               {
                  if ((uele2 <= sampleCount) ||
                      !gFlag)
                     matchListO(unmatched1, unmatched2, uele1, uele2, true, matchFlag);
                  else
                     matchList(unmatched1, unmatched2, uele1, uele2, true, matchFlag);
               }
               else
               {
                  if ((uele1 <= sampleCount) ||
                      !gFlag)
                     matchListO(unmatched2, unmatched1, uele2, uele1, false, matchFlag);
                  else
                     matchList(unmatched2, unmatched1, uele2, uele1, false, matchFlag);
               }
            }
         }

         if (muc1 < ucount1)
         {
            for (int i = start; i < elementCount1; i++)
            {
               if (!matched1[i])
                  _xtree1->addMatching(elements1[i], XTree::NO_MATCH);
            }
         }   
         else if (muc2 < ucount2)
         {
            for (int i = 0; i < elementCount2; i++)
            {
               if (!matched2[i])
                  _xtree2->addMatching(elements2[i], XTree::NO_MATCH);
            }
         }

         delete[] unmatched1;
         delete[] unmatched2;
      }

      delete[] elements1;
      delete[] elements2;
      delete[] matched1;
      delete[] matched2;
   }
}

void XDiff::diffAttributes(int attrCount1, int attrCount2)
{
   if ((attrCount1 == 1) && (attrCount2 == 1))
   {
      int   a1 = _attrList1[0];
      int   a2 = _attrList2[0];
      if (_xtree1->getHashValue(a1) == _xtree2->getHashValue(a2))
         return;

      if (_xtree1->getTag(a1).compare(_xtree2->getTag(a2)) == 0)
      {
         _xtree1->addMatching(a1, XTree::CHANGE, a2);
         _xtree2->addMatching(a2, XTree::CHANGE, a1);
         _xtree1->addMatching(_xtree1->getFirstChild(a1),
                    XTree::CHANGE,
                    _xtree2->getFirstChild(a2));
         _xtree2->addMatching(_xtree2->getFirstChild(a2),
                    XTree::CHANGE,
                    _xtree1->getFirstChild(a1));
      }
      else
      {
         _xtree1->addMatching(a1, XTree::NO_MATCH);
         _xtree2->addMatching(a2, XTree::NO_MATCH);
      }
      return;
   }

   for (int i = 0; i < attrCount2; i++)
   {
      _attrHash[i] = _xtree2->getHashValue(_attrList2[i]);
      _attrTag[i] = _xtree2->getTag(_attrList2[i]);
      _attrMatch[i] = false;
   }

   int   matchCount = 0;
   for (int i = 0; i < attrCount1; i++)
   {
      int   attr1 = _attrList1[i];
      unsigned long long   ah1 = _xtree1->getHashValue(attr1);
      string   tag1 = _xtree1->getTag(attr1);

      bool   found = false;
      for (int j = 0; j < attrCount2; j++)
      {
         int   attr2 = _attrList2[j];
         if (_attrMatch[j])
            continue;
         else if (ah1 == _attrHash[j])
         {
            _attrMatch[j] = true;
            matchCount++;
            found = true;
            break;
         }
         else if (tag1.compare(_attrTag[j]) == 0)
         {
            _attrMatch[j] = true;
            matchCount++;

            _xtree1->addMatching(attr1, XTree::CHANGE, attr2);
            _xtree2->addMatching(attr2, XTree::CHANGE, attr1);
            _xtree1->addMatching(_xtree1->getFirstChild(attr1), XTree::CHANGE, _xtree2->getFirstChild(attr2));
            _xtree2->addMatching(_xtree2->getFirstChild(attr2), XTree::CHANGE, _xtree1->getFirstChild(attr1));

            found = true;
            break;
         }
      }

      if (!found)
         _xtree1->addMatching(attr1, XTree::NO_MATCH);
   }

   if (matchCount != attrCount2)
   {
      for (int i = 0; i < attrCount2; i++)
      {
         if (!_attrMatch[i])
            _xtree2->addMatching(_attrList2[i], XTree::NO_MATCH);
      }
   }
}

void XDiff::diffText(int textCount1, int textCount2)
{
   for (int i = 0; i < textCount1; i++)
      _textMatch1[i] = false;
   for (int i = 0; i < textCount2; i++)
   {
      _textMatch2[i] = false;
      _textHash[i] = _xtree2->getHashValue(_textList2[i]);
   }

   int   mcount = 0;
   for (int i = 0; i < textCount1; i++)
   {
      unsigned long long   hash1 = _xtree1->getHashValue(_textList1[i]);
      for (int j = 0; j < textCount2; j++)
      {
         if (!_textMatch2[j] && (hash1 == _textHash[j]))
         {
            _textMatch1[i] = true;
            _textMatch2[j] = true;
            mcount++;
            break;
         }
      }

      if (mcount == textCount2)
         break;
   }

   if ((mcount < textCount1) && (textCount1 <= textCount2))
   {
      for (int i = 0, j = 0;
           (i < textCount1) && (mcount < textCount1); i++)
      {
         if (_textMatch1[i])
            continue;
         for (; _textMatch2[j]; j++);
         _xtree1->addMatching(_textList1[i], XTree::CHANGE,
                    _textList2[j]);
         _textMatch1[i] = true;
         _xtree2->addMatching(_textList2[j], XTree::CHANGE,
                    _textList1[i]);
         _textMatch2[j] = true;
         mcount++;
      }
   }
   else if ((mcount < textCount2) && (textCount2 < textCount1))
   {
      for (int i = 0, j = 0;
           (i < textCount2) && (mcount < textCount2); i++)
      {
         if (_textMatch2[i])
            continue;
         for (; _textMatch1[j]; j++);
         _xtree2->addMatching(_textList2[i], XTree::CHANGE,
                    _textList1[j]);
         _textMatch2[i] = true;
         _xtree1->addMatching(_textList1[j], XTree::CHANGE,
                    _textList2[i]);
         _textMatch1[j] = true;
         mcount++;
      }
   }

   if (mcount < textCount1)
   {
      for (int i = 0; i < textCount1; i++)
      {
         if (!_textMatch1[i])
            _xtree1->addMatching(_textList1[i],
                       XTree::NO_MATCH);
      }
   }
   else if (mcount < textCount2)
   {
      for (int i = 0; i < textCount2; i++)
      {
         if (!_textMatch2[i])
            _xtree2->addMatching(_textList2[i],
                       XTree::NO_MATCH);
      }
   }
}

int XDiff::_matchFilter(int *elements1, int *elements2, bool *matched1,
         bool *matched2, int count1, int count2)
{
   unsigned long long *value1 = new unsigned long long[count1];
   unsigned long long *value2 = new unsigned long long[count2];

   for (int i = 0; i < count1; i++)
   {
      value1[i] = _xtree1->getHashValue(elements1[i]);
      matched1[i] = false;
   }
   for (int i = 0; i < count2; i++)
   {
      value2[i] = _xtree2->getHashValue(elements2[i]);
      matched2[i] = false;
   }

   int   mcount = 0;
   for (int i = 0; i < count2; i++)
      for (int j = 0; j < count1; j++)
      {
         if (!matched1[j] && !matched2[i] &&
             (value1[j] == value2[i]))
         {
            matched1[j] = true;
            matched2[i] = true;
                mcount++;
            break;
         }
      }

   delete[]   value1;
   delete[]   value2;

   return mcount;
}

void XDiff::matchListO(int *nodes1, int *nodes2, int count1, int count2,
             bool treeOrder, bool matchFlag)
{
   int   **distanceMatrix = new int*[count1+1];
   int   *matching1 = new int[count1];
   int   *matching2 = new int[count2];

   // insert cost.
   distanceMatrix[count1] = new int[count2+1];
   for (int i = 0; i < count2; i++)
      distanceMatrix[count1][i] = (treeOrder ? _xtree2->getDecendentsCount(nodes2[i]) : _xtree1->getDecendentsCount(nodes2[i])) + 1;

   for (int i = 0; i < count1; i++)
   {
      distanceMatrix[i] = new int[count2+1];
      int   deleteCost = (treeOrder ? _xtree1->getDecendentsCount(nodes1[i]) : _xtree2->getDecendentsCount(nodes1[i])) + 1;
      for (int j = 0; j < count2; j++)
      {
         int   dist;
         if (matchFlag)
            dist = treeOrder ? _xlut->get(nodes1[i], nodes2[j]) : _xlut->get(nodes2[j], nodes1[i]);
         else
         {
            dist = treeOrder ? distance(nodes1[i], nodes2[j], true, XTree::NO_CONNECTION) : distance(nodes2[j], nodes1[i], true, XTree::NO_CONNECTION);

            // the default mode.
            if (!oFlag && (dist > 1) && (dist >= NO_MATCH_THRESHOLD * (deleteCost + distanceMatrix[count1][j])))
               dist = XTree::NO_CONNECTION;
            if (dist < XTree::NO_CONNECTION)
            {
               if (treeOrder)
                  _xlut->add(nodes1[i],
                        nodes2[j],
                        dist);
               else
                  _xlut->add(nodes2[j],
                        nodes1[i],
                        dist);
            }
         }
         distanceMatrix[i][j] = dist;
      }
      // delete cost.
      distanceMatrix[i][count2] = deleteCost;
   }

   // compute the minimal cost matching.
   findMatching(count1, count2, distanceMatrix, matching1, matching2);

   for (int i = 0; i < count1; i++)
   {
      if (matching1[i] == XTree::NO_MATCH)
      {
         if (treeOrder)
            _xtree1->addMatching(nodes1[i], XTree::NO_MATCH);
         else
            _xtree2->addMatching(nodes1[i], XTree::NO_MATCH);
      }
      else
      {
         if (treeOrder)
            _xtree1->addMatching(nodes1[i], XTree::CHANGE,
                       nodes2[matching1[i]]);
         else
            _xtree2->addMatching(nodes1[i], XTree::CHANGE,
                       nodes2[matching1[i]]);
      }
   }

   for (int i = 0; i < count2; i++)
   {
      if (matching2[i] == XTree::NO_MATCH)
      {
         if (treeOrder)
            _xtree2->addMatching(nodes2[i], XTree::NO_MATCH);
         else
            _xtree1->addMatching(nodes2[i], XTree::NO_MATCH);
      }
      else
      {
         if (treeOrder)
            _xtree2->addMatching(nodes2[i], XTree::CHANGE,
                       nodes1[matching2[i]]);
         else
            _xtree1->addMatching(nodes2[i], XTree::CHANGE,
                       nodes1[matching2[i]]);
      }
   }

   for (int i = 0; i < count1; i++)
   {
      if (matching1[i] != XTree::NO_MATCH)
      {
         int   todo1 = nodes1[i];
         int   todo2 = nodes2[matching1[i]];
         if (treeOrder)
         {
            if (_xtree1->isElement(todo1) &&
                _xtree2->isElement(todo2))
               xdiff(todo1, todo2, true);
         }
         else
         {
            if (_xtree1->isElement(todo2) &&
                _xtree2->isElement(todo1))
               xdiff(todo2, todo1, true);
         }
      }
   }

   delete[]   matching1;
   delete[]   matching2;
   for (int i = 0; i <= count1; i++)
      delete[]   distanceMatrix[i];
   delete[]   distanceMatrix;
}

void XDiff::matchList(int *nodes1, int *nodes2, int count1, int count2,
            bool treeOrder, bool matchFlag)
{
   int   *matching1 = new int[count1];
   int   *matching2 = new int[count2];
   for (int i = 0; i < count1; i++)
      matching1[i] = XTree::NO_MATCH;
   for (int i = 0; i < count2; i++)
      matching2[i] = XTree::NO_MATCH;

   if (matchFlag)
   {
      for (int i = 0; i < count1; i++)
      {
         for (int j = 0; j < count2; j++)
         {
            int   d = treeOrder ? _xlut->get(nodes1[i], nodes2[j]) : _xlut->get(nodes2[j], nodes1[i]);
            if (d != XTree::NO_CONNECTION)
            {
               matching1[i] = j;
               matching2[j] = i;
               break;
            }
         }
      }
   }
   else
   {
      int   scount1 = 0;
      int   scount2 = 0;
      int   matchingThreshold = 0;
      for (int i = 0; ((i < sampleCount) && (scount2 < count2)); scount2++)
      {
         srand(_seed);
         _seed = rand();
         int   snode = (int)((long long)_seed * (count2 - scount2) / (RAND_MAX + 1)) + scount2;
         int   dist = XTree::NO_CONNECTION;
         int   bestmatch = XTree::NULL_NODE;
         for (int j = scount1; j < count1; j++)
         {
            int   d = treeOrder ? distance(nodes1[j], nodes2[snode], false, dist) : distance(nodes2[snode], nodes1[j], false, dist);
            if (d < dist)
            {
               dist = d;
               bestmatch = j;
               if (d == 1)
                  break;
            }
         }

         int   deleteCost = (treeOrder ? _xtree2->getDecendentsCount(nodes2[snode]) : _xtree1->getDecendentsCount(nodes2[snode])) + 1;
         if ((dist > 1) &&
             (dist > NO_MATCH_THRESHOLD * deleteCost))
         {
            int   tmp = nodes2[snode];
            nodes2[snode] = nodes2[scount2];
            nodes2[scount2] = tmp;
         }
         else
         {
            int   tmp = nodes1[bestmatch];
            nodes1[bestmatch] = nodes1[scount1];
            nodes1[scount1] = tmp;
            tmp = nodes2[snode];
            nodes2[snode] = nodes2[scount2];
            nodes2[scount2] = tmp;

            if (treeOrder)
               _xlut->add(nodes1[scount1], nodes2[scount2], dist);
            else
               _xlut->add(nodes2[scount2], nodes1[scount1], dist);
            matching1[scount1] = scount2;
            matching2[scount2] = scount1;

            i++;
            scount1++;
            if (matchingThreshold < dist)
               matchingThreshold = dist;
         }
      }

      for (;scount2 < count2; scount2++)
      {
         int   dist = XTree::NO_CONNECTION;
         int   bestmatch = XTree::NO_MATCH;
         for (int i = scount1; i < count1; i++)
         {
            int   d = treeOrder ? distance(nodes1[i], nodes2[scount2], false, dist) : distance(nodes2[scount2], nodes1[i], false, dist);
            if (d <= matchingThreshold)
            {
               dist = d;
               bestmatch = i;
               break;
            }
            else if (d < dist)
            {
               dist = d;
               bestmatch = i;
            }
         }

         if (bestmatch != XTree::NO_MATCH)
         {
            int   tmp = nodes1[bestmatch];
            nodes1[bestmatch] = nodes1[scount1];
            nodes1[scount1] = tmp;

            if (treeOrder)
               _xlut->add(nodes1[scount1], nodes2[scount2], dist);
            else
               _xlut->add(nodes2[scount2], nodes1[scount1], dist);
            matching1[scount1] = scount2;
            matching2[scount2] = scount1;
            scount1++;
         }
      }
   }

   // Record matching
   for (int i = 0; i < count1; i++)
   {
      if (matching1[i] == XTree::NO_MATCH)
      {
         if (treeOrder)
            _xtree1->addMatching(nodes1[i], XTree::NO_MATCH);
         else
            _xtree2->addMatching(nodes1[i], XTree::NO_MATCH);
      }
      else
      {
         if (treeOrder)
            _xtree1->addMatching(nodes1[i], XTree::CHANGE,
                       nodes2[matching1[i]]);
         else
            _xtree2->addMatching(nodes1[i], XTree::CHANGE,
                       nodes2[matching1[i]]);
      }
   }

   for (int i = 0; i < count2; i++)
   {
      if (matching2[i] == XTree::NO_MATCH)
      {
         if (treeOrder)
            _xtree2->addMatching(nodes2[i], XTree::NO_MATCH);
         else
            _xtree1->addMatching(nodes2[i], XTree::NO_MATCH);
      }
      else
      {
         if (treeOrder)
            _xtree2->addMatching(nodes2[i], XTree::CHANGE,
                       nodes1[matching2[i]]);
         else
            _xtree1->addMatching(nodes2[i], XTree::CHANGE,
                       nodes1[matching2[i]]);
      }
   }

   for (int i = 0; i < count1; i++)
   {
      if (matching1[i] != XTree::NO_MATCH)
      {
         int   todo1 = nodes1[i];
         int   todo2 = nodes2[matching1[i]];
         if (treeOrder)
         {
            if (_xtree1->isElement(todo1) &&
                _xtree2->isElement(todo2))
               xdiff(todo1, todo2, true);
         }
         else
         {
            if (_xtree1->isElement(todo2) &&
                _xtree2->isElement(todo1))
               xdiff(todo2, todo1, true);
         }
      }
   }

   delete[]   matching1;
   delete[]   matching2;
}

int XDiff::distance(int eid1, int eid2, bool toRecord, int threshold)
{
   bool   isE1 = _xtree1->isElement(eid1);
   bool   isE2 = _xtree2->isElement(eid2);
   if (isE1 && isE2)
   {
      if (_xtree1->getTag(eid1).compare(_xtree2->getTag(eid2)) != 0)
         return XTree::NO_CONNECTION;
      else 
      {
         int   dist = _xdiff(eid1, eid2, threshold);
         if (toRecord && (dist < XTree::NO_CONNECTION))
            _xlut->add(eid1, eid2, dist);
         return dist;
      }
   }
   else if (!isE1 && !isE2)
      return 1;
   else
      return XTree::NO_CONNECTION;
}

int XDiff::_xdiff(int pid1, int pid2, int threshold/* = XTree::NO_CONNECTION*/)
{
   int   dist = 0;

   // diff attributes.
   int   attrCount1 = 0, attrCount2 = 0;
   int   attr1 = _xtree1->getFirstAttribute(pid1);
   while (attr1 != XTree::NULL_NODE)
   {
      _attrList1[attrCount1++] = attr1;
      attr1 = _xtree1->getNextAttribute(attr1);
   }
   int   attr2 = _xtree2->getFirstAttribute(pid2);
   while (attr2 != XTree::NULL_NODE)
   {
      _attrList2[attrCount2++] =  attr2;
      attr2 = _xtree2->getNextAttribute(attr2);
   }

   if (attrCount1 == 0)
      dist = attrCount2 * 2;
   else if (attrCount2 == 0)
      dist = attrCount1 * 2;
   else
      dist = _diffAttributes(attrCount1, attrCount2);
   if (!gFlag && (dist >= threshold))
      return XTree::NO_CONNECTION;

   // Match element nodes.
   int   count1 = _xtree1->getChildrenCount(pid1) - attrCount1;
   int   count2 = _xtree2->getChildrenCount(pid2) - attrCount2;

   if (count1 == 0)
   {
      int   node2 = _xtree2->getFirstChild(pid2);
      while (node2 != XTree::NULL_NODE)
      {
         dist += _xtree2->getDecendentsCount(node2) + 1;
         if (!gFlag && (dist >= threshold))
            return XTree::NO_CONNECTION;
         node2 = _xtree2->getNextSibling(node2);
      }
   }
   else if (count2 == 0)
   {
      int   node1 = _xtree1->getFirstChild(pid1);
      while (node1 != XTree::NULL_NODE)
      {
         dist += _xtree1->getDecendentsCount(node1) + 1;
         if (!gFlag && (dist >= threshold))
            return XTree::NO_CONNECTION;
         node1 = _xtree1->getNextSibling(node1);
      }
   }
   else if ((count1 == 1) && (count2 == 1))
   {
      int   node1 = _xtree1->getFirstChild(pid1);
      int   node2 = _xtree2->getFirstChild(pid2);

      if (_xtree1->getHashValue(node1) == _xtree2->getHashValue(node2))
         return dist;

      bool   isE1 = _xtree1->isElement(node1);
      bool   isE2 = _xtree2->isElement(node2);

      if (isE1 && isE2)
      {
         if (_xtree1->getTag(node1).compare(_xtree2->getTag(node2)) == 0)
            dist += _xdiff(node1, node2, threshold - dist);
         else
            dist += _xtree1->getDecendentsCount(node1) + 
               _xtree2->getDecendentsCount(node2) + 2;
      }
      else if (!isE1 && !isE2)
         dist++;
      else
         dist += _xtree1->getDecendentsCount(node1) +
            _xtree2->getDecendentsCount(node2) + 2;
   }
   else
   {
      int   *elements1 = new int[count1];
      int   *elements2 = new int[count2];
      int   elementCount1 = 0, textCount1 = 0;
      int   elementCount2 = 0, textCount2 = 0;

      int     child1 = _xtree1->getFirstChild(pid1);
      if (_xtree1->isElement(child1))
         elements1[elementCount1++] = child1;
      else
         _textList1[textCount1++] = child1;
      for (int i = 1; i < count1; i++)
      {
         child1 = _xtree1->getNextSibling(child1);
         if (_xtree1->isElement(child1))
            elements1[elementCount1++] = child1;
         else
            _textList1[textCount1++] = child1;
      }

      int     child2 = _xtree2->getFirstChild(pid2);
      if (_xtree2->isElement(child2))
         elements2[elementCount2++] = child2;
      else
         _textList2[textCount2++] = child2;
      for (int i = 1; i < count2; i++)
      {
         child2 = _xtree2->getNextSibling(child2);
         if (_xtree2->isElement(child2))
            elements2[elementCount2++] = child2;
         else
            _textList2[textCount2++] = child2;
      }

      // Match text nodes.
      if (textCount1 == 0)
      {
         dist += textCount2;
      }
      else if (textCount2 == 0)
      {
         dist += textCount1;
      }
      else
         dist += _diffText(textCount1, textCount2);

      if (gFlag && (dist >= threshold))
         return XTree::NO_CONNECTION;

      bool   *matched1 = new bool[elementCount1];
      bool   *matched2 = new bool[elementCount2];
      int   mcount = _matchFilter(elements1, elements2,
                     matched1, matched2,
                     elementCount1, elementCount2);

      if ((elementCount1 == mcount) &&
          (elementCount2 == mcount))
      {
      }
      else if (elementCount1 == mcount)
      {
         for (int i = 0; i < elementCount2; i++)
         {
            if (!matched2[i])
            {
               dist += _xtree2->getDecendentsCount(elements2[i]) + 1;
               if (gFlag && (dist >= threshold))
               {
                  dist = XTree::NO_CONNECTION;
                  break;
               }
            }
         }
      }
      else if (elementCount2 == mcount)
      {
         for (int i = 0; i < elementCount1; i++)
         {
            if (!matched1[i])
            {
               dist += _xtree1->getDecendentsCount(elements1[i]) + 1;
               if (gFlag && (dist >= threshold))
               {
                  dist = XTree::NO_CONNECTION;
                  break;
               }
            }
         }
      }
      else //if ((count1 > mcount) && (count2 > mcount))
      {
         // Write the list of unmatched nodes.
         int   ucount1 = elementCount1 - mcount;
         int   ucount2 = elementCount2 - mcount;
         int   *unmatched1 = new int[ucount1];
         int   *unmatched2 = new int[ucount2];
         int   muc1 = 0, muc2 = 0, start = 0;

         while ((muc1 < ucount1) && (muc2 < ucount2))
         {
            for (; (start < elementCount1) && matched1[start]; start++);
            string   startTag = _xtree1->getTag(elements1[start]);
            int   uele1 = 0, uele2 = 0;
            muc1++;
            unmatched1[uele1++] = elements1[start];
            matched1[start++] = true;

            for (int i = start; (i < elementCount1) && (muc1 < ucount1); i++)
            {
               if (!matched1[i] && (startTag.compare(_xtree1->getTag(elements1[i])) == 0))
               {
                  matched1[i] = true;
                  muc1++;
                  unmatched1[uele1++] = elements1[i];
               }
            }

            for (int i = 0; (i < elementCount2) && (muc2 < ucount2); i++)
            {
               if (!matched2[i] && (startTag.compare(_xtree2->getTag(elements2[i])) == 0))
               {
                  matched2[i] = true;
                  muc2++;
                  unmatched2[uele2++] = elements2[i];
               }
            }

            if (uele2 == 0)
            {
               for (int i = 0; i < uele1; i++)
                  dist += _xtree1->getDecendentsCount(unmatched1[i]);
            }
            else
            {
/*
               if ((uele1 == 1) && (uele2 == 1))
               {
                  dist += _xdiff(unmatched1[0],
                            unmatched2[0],
                            threshold-dist);
               }
               // To find minimal-cost matching between those unmatched elements (with the same tag name.
               else if (uele1 >= uele2)
               */
               if (uele1 >= uele2)
               {
                  if ((uele2 <= sampleCount) ||
                      !gFlag)
                     dist += _matchListO(unmatched1, unmatched2, uele1, uele2, true);
                  else
                     dist += _matchList(unmatched1, unmatched2, uele1, uele2, true, threshold - dist);
               }
               else
               {
                  if ((uele1 <= sampleCount) ||
                      !gFlag)
                     dist += _matchListO(unmatched2, unmatched1, uele2, uele1, false);
                  else
                     dist += _matchList(unmatched2, unmatched1, uele2, uele1, false, threshold - dist);
               }
            }

            if (gFlag && (dist >= threshold))
            {
               dist = XTree::NO_CONNECTION;
               break;
            }
         }

         if (dist < XTree::NO_CONNECTION)
         {
            if (muc1 < ucount1)
            {
               for (int i = start; i < elementCount1; i++)
               {
                  if (!matched1[i])
                  {
                     dist =+ _xtree1->getDecendentsCount(elements1[i]);
                     if (gFlag && (dist >= threshold))
                     {
                        dist = XTree::NO_CONNECTION;
                        break;
                     }
                  }
               }
            }   
            else if (muc2 < ucount2)
            {
               for (int i = 0; i < elementCount2; i++)
               {
                  if (!matched2[i])
                  {
                     dist += _xtree2->getDecendentsCount(elements2[i]);
                     if (gFlag && (dist >= threshold))
                     {
                        dist = XTree::NO_CONNECTION;
                        break;
                     }
                  }
               }
            }
         }

         delete[] unmatched1;
         delete[] unmatched2;
      }

      delete[] elements1;
      delete[] elements2;
      delete[] matched1;
      delete[] matched2;
   }

   if (!gFlag || (dist < threshold))
      return dist;
   else
      return XTree::NO_CONNECTION;
}

int XDiff::_diffAttributes(int attrCount1, int attrCount2)
{
   if ((attrCount1 == 1) && (attrCount2 == 1))
   {
      int   a1 = _attrList1[0];
      int   a2 = _attrList2[0];
      if (_xtree1->getHashValue(a1) == _xtree2->getHashValue(a2))
         return 0;

      if (_xtree1->getTag(a1).compare(_xtree2->getTag(a2)) == 0)
         return 1;
      else
         return 2;
   }

   int   dist = 0;
   for (int i = 0; i < attrCount2; i++)
   {
      _attrHash[i] = _xtree2->getHashValue(_attrList2[i]);
      _attrTag[i] = _xtree2->getTag(_attrList2[i]);
      _attrMatch[i] = false;
   }

   int   matchCount = 0;
   for (int i = 0; i < attrCount1; i++)
   {
      int   attr1 = _attrList1[i];
      unsigned long long   ah1 = _xtree1->getHashValue(attr1);
      string   tag1 = _xtree1->getTag(attr1);

      bool   found = false;
      for (int j = 0; j < attrCount2; j++)
      {
         if (_attrMatch[j])
            continue;
         else if (ah1 == _attrHash[j])
         {
            _attrMatch[j] = true;
            matchCount++;
            found = true;
            break;
         }
         else if (tag1.compare(_attrTag[j]) == 0)
         {
            _attrMatch[j] = true;
            matchCount++;
            dist++;
            found = true;
            break;
         }
      }

      if (!found)
         dist += 2;
   }

   dist += (attrCount2 - matchCount) * 2;
   return dist;
}

int XDiff::_diffText(int textCount1, int textCount2)
{
   for (int i = 0; i < textCount2; i++)
   {
      _textMatch2[i] = false;
      _textHash[i] = _xtree2->getHashValue(_textList2[i]);
   }

   int   mcount = 0;
   for (int i = 0; i < textCount1; i++)
   {
      unsigned long long   hash1 = _xtree1->getHashValue(_textList1[i]);
      for (int j = 0; j < textCount2; j++)
      {
         if (!_textMatch2[j] && (hash1 == _textHash[j]))
         {
            _textMatch2[j] = true;
            mcount++;
            break;
         }
      }

      if (mcount == textCount2)
         break;
   }

   if (textCount1 >= textCount2)
      return textCount1 - mcount;
   else
      return textCount2 - mcount;
}

int XDiff::_matchListO(int *nodes1, int *nodes2, int count1, int count2,
             bool treeOrder)
{
   int   **distanceMatrix = new int*[count1+1];
   int   *matching1 = new int[count1];
   int   *matching2 = new int[count2];

   // insert cost.
   distanceMatrix[count1] = new int[count2+1];
   for (int i = 0; i < count2; i++)
      distanceMatrix[count1][i] = (treeOrder ? _xtree2->getDecendentsCount(nodes2[i]) : _xtree1->getDecendentsCount(nodes2[i])) + 1;

   for (int i = 0; i < count1; i++)
   {
      distanceMatrix[i] = new int[count2+1];
      int   deleteCost = (treeOrder ? _xtree1->getDecendentsCount(nodes1[i]) : _xtree2->getDecendentsCount(nodes1[i])) + 1;
      for (int j = 0; j < count2; j++)
      {
         int   dist = treeOrder ? distance(nodes1[i], nodes2[j], true, XTree::NO_CONNECTION) : distance(nodes2[j], nodes1[i], true, XTree::NO_CONNECTION);

         // the default mode.
         if (!oFlag && (dist > 1) && 
             (dist < XTree::NO_CONNECTION) &&
             (dist >= NO_MATCH_THRESHOLD *
              (deleteCost + distanceMatrix[count1][j])))
            dist = XTree::NO_CONNECTION;

         if (dist < XTree::NO_CONNECTION)
         {
            if (treeOrder)
               _xlut->add(nodes1[i], nodes2[j], dist);
            else
               _xlut->add(nodes2[j], nodes1[i], dist);
         }
         distanceMatrix[i][j] = dist;
      }
      // delete cost.
      distanceMatrix[i][count2] = deleteCost;
   }

   // compute the minimal cost matching.
   int   dist_value = findMatching(count1, count2, distanceMatrix,
                 matching1, matching2);

   delete[]   matching1;
   delete[]   matching2;
   for (int i = 0; i <= count1; i++)
      delete[]   distanceMatrix[i];
   delete[]   distanceMatrix;

   return dist_value;
}

int XDiff::_matchList(int *nodes1, int *nodes2, int count1, int count2,
            bool treeOrder, int threshold)
{
   int   *matching1 = new int[count1];
   int   *matching2 = new int[count2];
   for (int i = 0; i < count1; i++)
      matching1[i] = XTree::NULL_NODE;
   for (int i = 0; i < count2; i++)
      matching2[i] = XTree::NULL_NODE;

   int   Distance = 0;
   int   scount1 = 0;
   int   scount2 = 0;
   int   matchingThreshold = 0;

   for (int i = 0; ((i < sampleCount) && (scount2 < count2)); scount2++)
   {
      int   snode = rand() % (count2 - scount2) + scount2;
      int   dist = XTree::NO_CONNECTION;
      int   bestmatch = XTree::NULL_NODE;
      for (int j = scount1; j < count1; j++)
      {
         int   d = treeOrder ? distance(nodes1[j], nodes2[snode], false, threshold - Distance) : distance(nodes2[snode], nodes1[j], false, threshold - Distance);
         if (d < dist)
         {
            dist = d;
            bestmatch = j;
            if (d == 1)
               break;
         }
      }

      int   deleteCost = (treeOrder ? _xtree2->getDecendentsCount(nodes2[snode]) : _xtree1->getDecendentsCount(nodes2[snode])) + 1;

      if ((dist > 1) && (dist > NO_MATCH_THRESHOLD * deleteCost))
      {
         int   tmp = nodes2[snode];
         nodes2[snode] = nodes2[scount2];
         nodes2[scount2] = tmp;
         Distance += deleteCost;
      }
      else
      {
         int   tmp = nodes1[bestmatch];
         nodes1[bestmatch] = nodes1[scount1];
         nodes1[scount1] = tmp;
         tmp = nodes2[snode];
         nodes2[snode] = nodes2[scount2];
         nodes2[scount2] = tmp;

         if (treeOrder)
            _xlut->add(nodes1[scount1], nodes2[scount2], dist);
         else
            _xlut->add(nodes2[scount2], nodes1[scount1], dist);
         matching1[scount1] = scount2;
         matching2[scount2] = scount1;

         i++;
         scount1++;
         if (matchingThreshold < dist)
            matchingThreshold = dist;
         Distance += dist;
      }

      if (Distance >= threshold)
      {
         delete[]   matching1;
         delete[]   matching2;
         return XTree::NO_CONNECTION;
      }
   }

   for (;scount2 < count2; scount2++)
   {
      int   deleteCost = (treeOrder ? _xtree2->getDecendentsCount(nodes2[scount2]) : _xtree1->getDecendentsCount(nodes2[scount2])) + 1;
      int   dist = XTree::NO_CONNECTION;
      int   bestmatch = XTree::NULL_NODE;
      for (int i = scount1; i < count1; i++)
      {
         int   d = treeOrder ? distance(nodes1[i], nodes2[scount2], false, threshold - Distance) : distance(nodes2[scount2], nodes1[i], false, threshold - Distance);
         if (d <= matchingThreshold)
         {
            dist = d;
            bestmatch = i;
            break;
         }
         else if ((d == 1) || (d < NO_MATCH_THRESHOLD * dist))
         {
            dist = d;
            bestmatch = i;
         }
      }

      if (bestmatch == XTree::NO_MATCH)
      {
         Distance += deleteCost;
      }
      else
      {
         int   tmp = nodes1[bestmatch];
         nodes1[bestmatch] = nodes1[scount1];
         nodes1[scount1] = tmp;

         if (treeOrder)
            _xlut->add(nodes1[scount1], nodes2[scount2], dist);
         else
            _xlut->add(nodes2[scount2], nodes1[scount1], dist);

         matching1[scount1] = scount2;
         matching2[scount2] = scount1;
         scount1++;
         Distance += dist;
      }

      if (Distance >= threshold)
      {
         delete[]   matching1;
         delete[]   matching2;
         return XTree::NO_CONNECTION;
      }
   }

   for (int i = 0; i < count1; i++)
   {
      if (matching1[i] == XTree::NO_MATCH)
      {
         Distance += (treeOrder ? _xtree1->getDecendentsCount(nodes1[i]) : _xtree2->getDecendentsCount(nodes1[i])) + 1;
         if (Distance >= threshold)
         {
            delete[]   matching1;
            delete[]   matching2;
            return XTree::NO_CONNECTION;
         }
      }
   }

   delete[]   matching1;
   delete[]   matching2;
   return Distance;
}

int XDiff::findMatching(int count1, int count2, int** dist,
         int* matching1, int* matching2)
{
   if (count1 == 1)
   {
      // count2 == 1
      if (dist[0][0] < XTree::NO_CONNECTION)
      {
         matching1[0] = 0;
         matching2[0] = 0;
      }
      else
      {
         matching1[0] = XTree::DELETE;
         matching2[0] = XTree::DELETE;
      }

      return dist[0][0];
   }
   else if (count2 == 1)
   {
      int   distance = 0;
      int   mate = 0;
      int   mindist = XTree::NO_CONNECTION;
      matching2[0] = XTree::DELETE;

      for (int i = 0; i < count1; i++)
      {
         matching1[i] = XTree::DELETE;
         if (mindist > dist[i][0])
         {
            mindist = dist[i][0];
            mate = i;
         }

         // Suppose we delete every node on list1.
         distance += dist[i][1];
      }

      if (mindist < XTree::NO_CONNECTION)
      {
         matching1[mate] = 0;
         matching2[0] = mate;
         distance += mindist - dist[mate][1];
      }
      else
      {
         // Add the delete cost of the single node on list2.
         distance += dist[count1][0];
      }

      return distance;
   }
   else if ((count1 == 2) && (count2 == 2))
   {
      int   distance1 = dist[0][0] + dist[1][1];
      int   distance2 = dist[0][1] + dist[1][0];
      if (distance1 < distance2)
      {
         if (dist[0][0] < XTree::NO_CONNECTION)
         {
            matching1[0] = 0;
            matching2[0] = 0;
            distance1 = dist[0][0];
         }
         else
         {
            matching1[0] = XTree::DELETE;
            matching2[0] = XTree::DELETE;
            distance1 = dist[0][2] + dist[2][0];
         }

         if (dist[1][1] < XTree::NO_CONNECTION)
         {
            matching1[1] = 1;
            matching2[1] = 1;
            distance1 += dist[1][1];
         }
         else
         {
            matching1[1] = XTree::DELETE;
            matching2[1] = XTree::DELETE;
            distance1 += dist[1][2] + dist[2][1];
         }

         return distance1;
      }
      else
      {
         if (dist[0][1] < XTree::NO_CONNECTION)
         {
            matching1[0] = 1;
            matching2[1] = 0;
            distance2 = dist[0][1];
         }
         else
         {
            matching1[0] = XTree::DELETE;
            matching2[1] = XTree::DELETE;
            distance2 = dist[0][2] + dist[2][1];
         }

         if (dist[1][0] < XTree::NO_CONNECTION)
         {
            matching1[1] = 0;
            matching2[0] = 1;
            distance2 += dist[1][0];
         }
         else
         {
            matching1[1] = XTree::DELETE;
            matching2[0] = XTree::DELETE;
            distance2 += dist[1][2] + dist[2][0];
         }

         return distance2;
      }
   }
   else
   {
      return optimalMatching(count1, count2, dist,
                   matching1, matching2);
   }
}

int XDiff::optimalMatching(int count1, int count2, int** dist,
            int* matching1, int* matching2)
{
   // Initialize matching. 
   // Initial guess will be pair-matching between two lists.
   // Others will be insertion or deletion
   for (int i = 0; i < count2; i++)
      matching1[i] = i;
   for (int i = count2; i < count1; i++)
      matching1[i] = XTree::DELETE;

   // Three artificial nodes: "start", "end" and "delete".
   int count = count1 + count2 + 3;

   // Initialize least cost matrix and path matrix.
   // Both have been initialized at the very beginning.

   // Start algorithm.
   while (true)
   {
      // Construct least cost matrix.
      constructLCM(dist, matching1, count1, count2);

      // Initialize path matrix.
      for (int i = 0; i < count; i++)
         for (int j = 0; j < count; j++)
            _pathMatrix[i][j] = i;

      // Search negative cost circuit.
      int   clen = searchNCC(count);
      if (clen > 0)
      {
         // Modify matching.
         for (int i = 0, next = 0; i < clen - 1; i++)
         {
            int   n1 = _circuit[next];
            next = _circuit[next+1];
            // Node in node list 1.
            if ((n1 > 0) && (n1 <= count1))
            {
               int   nid1 = n1 - 1;
               int   nid2 = _circuit[next] - count1 - 1;
               if (nid2 == count2)
                  nid2 = XTree::DELETE;

               matching1[nid1] = nid2;
            }
         }
      }
      else // Stop.
         break;
   }

   int   distance = 0;
   // Suppose all insertion on list2
   for (int i = 0; i < count2; i++)
   {
      matching2[i] = XTree::INSERT;
      distance += dist[count1][i];
   }

   // update distance by looking at matching pairs.
   for (int i = 0; i < count1; i++)
   {
      int   mmm = matching1[i];
      if (mmm == XTree::DELETE)
         distance += dist[i][count2];
      else
      {
         matching2[mmm] = i;
         distance += dist[i][mmm] -
                dist[count1][mmm];
      }
   }

   return distance;
}

void XDiff::constructLCM(int** costMatrix, int* matching,
          int nodeCount1, int nodeCount2)
{
   // Three artificial nodes: "start", "end" and "delete".
   int nodeCount = nodeCount1 + nodeCount2 + 3;

   // Initialize.
   for (int i = 0; i < nodeCount; i++)
   {
      for (int j = 0; j < nodeCount; j++)
      _leastCostMatrix[i][j] = XTree::NO_CONNECTION;

      // self.
      _leastCostMatrix[i][i] = 0;
   }

   // Between start node and nodes in list 1.
   // Start -> node1 = Infinity; node1 -> Start = -0.
   for (int i = 0; i < nodeCount1; i++)
      _leastCostMatrix[i+1][0] = 0;

   // Between nodes in list2 and the end node.
   // Unless matched (later), node2 -> end = 0;
   // end -> node2 = Infinity.
   for (int i = 0; i < nodeCount2; i++)
      _leastCostMatrix[i+nodeCount1+1][nodeCount-1] = 0;

   int deleteCount = 0;

   // Between nodes in list1 and nodes in list2.
   // For matched, node1 -> node2 = Infinity;
   // node2 -> node1 = -1 * distance
   // For unmatched, node1 -> node2 = distance;
   // node2 -> node1 = Infinity
   for (int i = 0; i < nodeCount1; i++)
   {
      int node1 = i + 1;
      int node2;

      // According to cost matrix.
      for (int j = 0; j < nodeCount2; j++)
      {
         node2 = j + nodeCount1 + 1;
         _leastCostMatrix[node1][node2] = costMatrix[i][j];
      }

      // According to matching.
      if (matching[i] == XTree::DELETE)
      {
         deleteCount++;

         // node1 -> Delete = Infinity;
         // Delete -> node1 = -1 * DELETE_COST
         _leastCostMatrix[nodeCount-2][node1] = -1 * costMatrix[i][nodeCount2];
      }
      else
      {
         node2 = matching[i] + nodeCount1 + 1;

         // Between node1 and node2.
         _leastCostMatrix[node1][node2] = XTree::NO_CONNECTION;
         _leastCostMatrix[node2][node1] = costMatrix[i][matching[i]] * -1;

         // Between node1 and delete.
         _leastCostMatrix[node1][nodeCount-2] = costMatrix[i][nodeCount2];

         // Between node2 and end.
         _leastCostMatrix[node2][nodeCount-1] = XTree::NO_CONNECTION;
         _leastCostMatrix[nodeCount-1][node2] = 0;
      }
   }

   // Between the "Delete" and the "End".
   // If delete all, delete -> end = Infinity; end -> delete = 0.
   if (deleteCount == nodeCount1)
      _leastCostMatrix[nodeCount-1][nodeCount-2] = 0;
   // if no delete, delete -> end = 0; end -> delete = Infinity.
   else if (deleteCount == 0)
      _leastCostMatrix[nodeCount-2][nodeCount-1] = 0;
   // else, both 0;
   else
   {
      _leastCostMatrix[nodeCount-2][nodeCount-1] = 0;
      _leastCostMatrix[nodeCount-1][nodeCount-2] = 0;
   }
}

int XDiff::searchNCC(int nodeCount)
{
   for (int k = 0; k < nodeCount; k++)
   {
       for (int i = 0; i < nodeCount; i++)
       {
      if ((i != k) &&
          (_leastCostMatrix[i][k] != XTree::NO_CONNECTION))
      {
          for (int j = 0; j < nodeCount; j++)
          {
         if ((j != k) &&
             (_leastCostMatrix[k][j] != XTree::NO_CONNECTION))
         {
             int   less = _leastCostMatrix[i][k] +
                   _leastCostMatrix[k][j];
             if (less < _leastCostMatrix[i][j])
             {
            _leastCostMatrix[i][j] = less;
            _pathMatrix[i][j] = k;

            // Found!
                   if ((i == j) && (less < 0))
            {
                int   clen = 0; // the length of the circuit.

                // Locate the circuit.
                //circuit.addElement(new Integer(i));
                _circuit[0] = i;
                _circuit[1] = 2;

                //circuit.addElement(new Integer(pathMatrix[i][i]));
                _circuit[2] = _pathMatrix[i][i];
                _circuit[3] = 4;

                //circuit.addElement(new Integer(i));
                _circuit[4] = i;
                _circuit[5] = -1;

                clen = 3;

                bool   finish;

                do
                {
               finish = true;
               for (int cit = 0, n = 0; cit < clen - 1; cit++)
               {
                   int   left = _circuit[n];
                   int   next = _circuit[n+1];
                   int   right = (next == -1)? -1 : _circuit[next];

                   //int middle = pathMatrix[circuit[n-1]][circuit[n]];
                   int   middle = _pathMatrix[left][right];

                   if (middle != left)
                   {
                  //circuit.insert( cit, middle );
                  _circuit[clen*2] = middle;
                  _circuit[clen*2+1] = next;
                  _circuit[n+1] = clen*2;
                  clen++;

                  finish = false;
                  break;
                   }
                   n = next;
               }
                } while (!finish);

                return clen;
            }
             }
         }
          }
      }
       }
   }
   return 0;
}

void XDiff::writeDiff(const string& input_file, const string& output_file)
{
   ifstream   in(input_file.c_str());
   ofstream   out(output_file.c_str(), ios::out|ios::trunc);

   int root1 = _xtree1->getRoot();
   int root2 = _xtree2->getRoot();
   // the header
   // XXX <root > is valid and should be treated as <root>;
   // < root> is NOT valid, so use <root as the comparison key.
   string rootTag = "<" + _xtree1->getTag(root1);
   string line;
   while (getline(in, line)) {
      if (line.find(rootTag) != string::npos)
         break;
      out << line << "\n";
   }
   in.close();

   int matchType, matchNode;
   _xtree1->getMatching(root1, matchType, matchNode);
   if (matchType == XTree::DELETE) {
      writeDeleteNode(out, root1, 0, true);
      writeInsertNode(out, root2, 0, true);
   } else {
      writeDiffNode(out, root1, root2, 0, true);
   }

   out.close();
}

void XDiff::writeDocDiff(std::stringstream& out, bool whitespace/* = false*/)
{
   out << "<?xml version=\"1.0\"?>";
   if (whitespace) out << "\n";
   
   int root1 = _xtree1->getRoot();
   int root2 = _xtree2->getRoot();

   int matchType, matchNode;
   _xtree1->getMatching(root1, matchType, matchNode);
   if (matchType == XTree::DELETE) {
      writeDeleteNode(out, root1, 0, whitespace);
      writeInsertNode(out, root2, 0, whitespace);
   } else {
      writeDiffNode(out, root1, root2, 0, whitespace);
   }
}

void XDiff::writeDeleteNode(ostream &out, int node, int indent, bool whitespace/* = false*/)
{
   if (_xtree1->isElement(node)) {
      string tag = _xtree1->getTag(node);
      if (whitespace) {
         if (_needNewLine)
            out << "\n";
         for (int i = 0; i < indent; ++i)
            out << INDENT_STR;
      }
      
      out << "<" << tag << " " << CHANGE_ATTR_STR << "=\"" << DELETE_PREFIX << "\"" << ">";
      _needNewLine = true;
   } else {
      out << "<" << _xtree1->getText(node) << " " << CHANGE_ATTR_STR << "=\"" << DELETE_PREFIX << "\"" << ">";
      if (whitespace)
         out << "\n";
      _needNewLine = false;
   }
}

void XDiff::writeInsertNode(ostream &out, int node, int indent, bool whitespace/* = false*/)
{
   if (_xtree2->isElement(node)) {
      string tag = _xtree2->getTag(node);
      if (whitespace) {
         if (_needNewLine)
            out << "\n";
         for (int i = 0; i < indent; ++i)
            out << INDENT_STR;
      }
            
      out << "<" << tag;
      _needNewLine = true;
      out << " " << CHANGE_ATTR_STR << "=\"" << INSERT_PREFIX << "\"";

      // Attributes.
      int attr = _xtree2->getFirstAttribute(node);
      while (attr > 0) {
         out << " " << _xtree2->getTag(attr) << "=\"" << _xtree2->getAttributeValue(attr) << "\"";
         attr = _xtree2->getNextAttribute(attr);
      }

      // Child nodes.
      int child = _xtree2->getFirstChild(node);
      if (child < 0) {
         out << "/>";
         if (whitespace)
            out << "\n";
         _needNewLine = false;
         return;
      }

      out << ">";
      if (whitespace)
         out << "\n";
      _needNewLine = false;

      bool child_elements = false;
      while (child > 0) {
         if (_xtree2->isElement(child))
            child_elements = true;
         writeMatchNode(out, _xtree2, child, indent + 1, whitespace);
         child = _xtree2->getNextSibling(child);
      }

      if (whitespace && _needNewLine) {
         out << "\n";
         _needNewLine = false;
      }

      if (whitespace && child_elements) {
         for (int i = 0; i < indent; ++i)
            out << INDENT_STR;
      }
      out << "</" << tag << ">";
      if (whitespace)
         out << "\n";
   } else {
      out << _xtree2->getText(node);
      if (whitespace)
         out << "\n";
      _needNewLine = false;
   }
}

void XDiff::writeMatchNode(ostream &out, XTree *xtree, int node, int indent, bool whitespace/* = false*/)
{
   if (xtree->isElement(node)) {
      string tag = xtree->getTag(node);
      if (whitespace) {
         if (_needNewLine)
            out << "\n";
         for (int i = 0; i < indent; ++i)
            out << INDENT_STR;
      }

      out << "<" << tag;

      // Attributes.
      int attr = xtree->getFirstAttribute(node);
      while (attr > 0) {
         out << " " << xtree->getTag(attr) << "=\"" << xtree->getAttributeValue(attr) << "\"";
         attr = xtree->getNextAttribute(attr);
      }

      bool child_elements = false;
      int child = xtree->getFirstChild(node);
      while (!child_elements && child > 0) {
         if (xtree->isElement(child))
            child_elements = true;
         child = xtree->getNextSibling(child);
      }

      // Child nodes.
      child = xtree->getFirstChild(node);
      if (!child_elements) {
         out << "/>";
         if (whitespace)
            out << "\n";
         _needNewLine = false;
         return;
      }

      out << ">";
      _needNewLine = true;

      child = xtree->getFirstChild(node);
      while (child > 0) {
         writeMatchNode(out, xtree, child, indent + 1, whitespace);
         child = xtree->getNextSibling(child);
      }

      if (whitespace && _needNewLine) {
         out << "\n";
         _needNewLine = false;
      }
      
      if (whitespace && child_elements) {
         for (int i = 0; i < indent; ++i)
            out << INDENT_STR;
      }
      out << "</" << tag << ">";
      if (whitespace)
         out << "\n";
   } else {
      out << xtree->getText(node);
      _needNewLine = false;
   }
}

void XDiff::writeDiffNode(ostream &out, int node1, int node2, int indent, bool whitespace/* = false*/)
{
   if (_xtree1->isElement(node1)) {
      string tag = _xtree1->getTag(node1);
      if (whitespace) {
         if (_needNewLine)
            out << "\n";
         for (int i = 0; i < indent; ++i)
            out << INDENT_STR;
      }

      // Attributes.
      // first, determine diff types for all attributes
      int deletes = 0, updates = 0, inserts = 0;
      int attr1 = _xtree1->getFirstAttribute(node1);
      while (attr1 > 0) {
         int matchType, matchNode;
         _xtree1->getMatching(attr1, matchType, matchNode);
         if (matchType != XTree::MATCH) {
            if (matchType == XTree::DELETE)
              ++deletes;
            else
              ++inserts;
         }
         attr1 = _xtree1->getNextAttribute(attr1);
      }

      int attr2 = _xtree2->getFirstAttribute(node2);
      while (attr2 > 0) {
         int matchType, matchNode;
         _xtree2->getMatching(attr2, matchType, matchNode);
         if (matchType == XTree::INSERT)
            ++inserts;
         attr2 = _xtree2->getNextAttribute(attr2);
      }

      // now determine whether the text has changed
      bool child_elements = false;
      bool has_different_text = false;
      int child1 = _xtree1->getFirstChild(node1);
      while (!has_different_text && !child_elements && child1 > 0) {
         int matchType, matchNode;
         _xtree1->getMatching(child1, matchType, matchNode);
         if (_xtree1->isElement(child1))
            child_elements = true;
         if (matchType == XTree::MATCH)
            ;
         else if (_xtree1->isLeaf(child1) && !_xtree1->isAttribute(child1))
            has_different_text = true;
         child1 = _xtree1->getNextSibling(child1);
      }
      
      int child2 = _xtree2->getFirstChild(node2);
      while (!has_different_text && !child_elements && child2 > 0) {
         int matchType, matchNode;
         _xtree2->getMatching(child2, matchType, matchNode);
         if (_xtree1->isElement(child1))
            child_elements = true;
         if (matchType == XTree::INSERT && _xtree2->isLeaf(child2) && !_xtree2->isAttribute(child2))
            has_different_text = true;
         child2 = _xtree2->getNextSibling(child2);
      }
      
      // based on this, include necessary attributes in output
      enum AttributeMode {ATTR_NONE, ATTR_REDO, ATTR_DELETE, ATTR_UPDATE};
      AttributeMode mode = ATTR_NONE;
      string change_attr_str = " " + CHANGE_ATTR_STR + "=\"";
      if (deletes && (updates || inserts)) { // mixed bag of deletes and others
         out << "<" << tag;
         if (has_different_text)
            change_attr_str += UPDATE_TEXT_PREFIX;
         change_attr_str += REDOATTR_PREFIX;
         mode = ATTR_REDO;
      } else if (deletes) { // only deletes
         out << "<" << tag;
         if (has_different_text)
            change_attr_str += UPDATE_TEXT_PREFIX;
         change_attr_str += DELETEATTR_PREFIX;
         mode = ATTR_DELETE;
      } else if (updates || inserts) { // changes, but no deletes
         out << "<" << tag;
         if (has_different_text)
            change_attr_str += UPDATE_TEXT_PREFIX;
         change_attr_str += UPDATEATTR_PREFIX;
         mode = ATTR_UPDATE;
      } else { // no attribute changes
         out << "<" << tag;
         if (has_different_text)
            change_attr_str += UPDATE_TEXT_PREFIX;
      }
      
      if (change_attr_str != " " + CHANGE_ATTR_STR + "=\"")
         out << change_attr_str + "\"";
      
      attr1 = _xtree1->getFirstAttribute(node1);
      while (attr1 > 0) {
         string atag = _xtree1->getTag(attr1);
         string value = _xtree1->getAttributeValue(attr1);
         int matchType, matchNode;
         _xtree1->getMatching(attr1, matchType, matchNode);
         if (matchType == XTree::MATCH) {
            if (mode == ATTR_REDO)
               out << " " << atag << "=\"" << value << "\"";
         } else if (matchType == XTree::DELETE) {
            out << " " << atag << "=\"" << value << "\"";
         } else {
            out << " " << atag << "=\"" << _xtree2->getAttributeValue(matchNode) << "\"";
         }

         attr1 = _xtree1->getNextAttribute(attr1);
      }

      attr2 = _xtree2->getFirstAttribute(node2);
      while (attr2 > 0) {
         int matchType, matchNode;
         _xtree2->getMatching(attr2, matchType, matchNode);
         if (matchType == XTree::INSERT) {
            out << " " << _xtree2->getTag(attr2) << "=\"" << _xtree2->getAttributeValue(attr2) << "\"";
         }

         attr2 = _xtree2->getNextAttribute(attr2);
      }

      // Child nodes.
      if (!has_different_text && !child_elements) {
         out << "/>";
         if (whitespace)
            out << "\n";
         _needNewLine = false;
         return;
      }

      out << ">";
      _needNewLine = true;

      child1 = _xtree1->getFirstChild(node1);
      while (child1 > 0) {
         int matchType, matchNode;
         _xtree1->getMatching(child1, matchType, matchNode);
         if (matchType == XTree::MATCH)
            ;
         else if (matchType == XTree::DELETE)
            writeDeleteNode(out, child1, indent + 1, whitespace);
         else
            writeDiffNode(out, child1, matchNode, indent + 1, whitespace);

         child1 = _xtree1->getNextSibling(child1);
      }

      child2 = _xtree2->getFirstChild(node2);
      while (child2 > 0) {
         int matchType, matchNode;
         _xtree2->getMatching(child2, matchType, matchNode);
         if (matchType == XTree::INSERT)
            writeInsertNode(out, child2, indent + 1, whitespace);

         child2 = _xtree2->getNextSibling(child2);
      }

      if (whitespace && _needNewLine) {
         out << "\n";
         _needNewLine = false;
      }
      if (whitespace && child_elements) {
         for (int i = 0; i < indent; ++i)
            out << INDENT_STR;
      }
      out << "</" << tag << ">";
      if (whitespace)
         out << "\n";
   } else {
      out << "\"" << _xtree2->getText(node2) << "\"";
      _needNewLine = false;
   }
}

