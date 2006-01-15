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

#ifndef   __XDIFF__
#define   __XDIFF__

#include <fstream>
#include <vector>

#ifndef   __XTREE__
#include "XTree.hpp"
#endif


class XLut;
class XMLDoc;

/** computes the difference between two XML documents.  The result of this computation reflects the changes that 
   must be made to argument 1 to recreate argument 2, so the result is analogous to arg2 - arg1. Only the 
   information absolutely necessary to communicate the difference is included in the result.  Usually this means 
   that only the parts that differ are included, but sometimes elements of the original document's structure must 
   be included to indicate where the differences are located.  Any element that represents a difference will have 
   an attribute XDchg="..." ("XDiff change").  
   The change types are:
   - \verbatim "INS " \endverbatim
   This element is an insertion.  A single item such as "\<TagName XDchg="INS "/\>" may be inserted, or a complex 
   object such as 
   \verbatim
   <TagName XDchg="INS ">
      <sub_item1 value="foo">
         <sub_sub_item value="0"/>
      </sub_item1>
      ...
      <sub_itemN value="bar">"Text"</sub_itemN>
   </TagName>\endverbatim
   - \verbatim "DEL " \endverbatim
   This element is a deletion.  Even if the item to be deleted has attributes and children, only its tag name 
   will be included, since this is all that is needed to find and delete it (e.g. "\<TagName XDchg="DEL "/\>").
   - \verbatim "TXT " \endverbatim
   This element's text is the updated value for the text of an element appearing in the first argument (e.g. 
   "\<TagName XDchg="TXT "\>"New text here."</item>").  Items without a "TXT " update code will not have 
   accompanying text.
   - \verbatim "DELATTR " \endverbatim
   This element has attributes which should be deleted from the attributes for an element appearing in the first 
   argument.  All attributes in this element (except "XDchg") are to be deleted.
   - \verbatim "UPDATTR " \endverbatim
   This element has attributes which should be updated in or added to the attributes for an element appearing in 
   the first argument.  Any attributes which appear in the original element should be updated to the values 
   presented in this element; any attributes not in the original element should be added to it.
   - \verbatim "REDOATTR " \endverbatim
   This element's attributes should replace all the attributes in the original element.  This is necessary when 
   there are some attributes that must be deleted, and some that must be updated or added.  Since it is not 
   possible to annotate individual attributes, the entire set of original attributes must be replaced with the 
   new sequence of attributes.
   <br>
   TXT codes may appear with exactly one of the *ATTR codes, but no more than one *ATTR code may appear in an 
   update, and all INS and DEL codes will always appear alone.
   <br>
   Note that the names of all sibling elements must be unique for this algorithm to work.*/
class XDiff
{
public:
   /** ctor that takes two XMLDoc's as input, and writes the result of the xdiff operation to \a output. */
   XDiff(const XMLDoc& doc1, const XMLDoc& doc2, XMLDoc& output);
   
   ~XDiff(); ///< dtor

private:
   void xdiff(int pid1, int pid2, bool matchFlag);
   void diffAttributes(int attrCount1, int attrCount2);
   void diffText(int textCount1, int textCount2);
   int _matchFilter(int *elements1, int *elements2, bool *matched1, bool *matched2, int count1, int count2);
   void matchListO(int *nodes1, int *nodes2, int count1, int count2, bool treeOrder, bool matchFlag);
   void matchList(int *nodes1, int *nodes2, int count1, int count2, bool treeOrder, bool matchFlag);
   int distance(int eid1, int eid2, bool toRecord, int threshold);
   int _xdiff(int pid1, int pid2, int threshold);
   int _diffAttributes(int attrCount1, int attrCount2);
   int _diffText(int textCount1, int textCount2);
   int _matchListO(int *nodes1, int *nodes2, int count1, int count2, bool treeOrder);
   int _matchList(int *nodes1, int *nodes2, int count1, int count2, bool treeOrder, int threshold);
   int findMatching(int count1, int count2, int** dist, int* matching1, int* matching2);
   int optimalMatching(int count1, int count2, int** dist, int* matching1, int* matching2);
   void constructLCM(int** costMatrix, int* matching, int nodeCount1, int nodeCount2);
   int searchNCC(int nodeCount);
   
   void writeDiff(std::istream& in, std::ostream& out);
   void writeDeleteNode(std::ostream &out, int node);
   void writeInsertNode(std::ostream &out, int node);
   void writeMatchNode(std::ostream &out, XTree *xtree, int node);
   void writeDiffNode(std::ostream &out, int node1, int node2);
   
   std::vector<int>                 _attrList1;
   std::vector<int>                 _attrList2;
   std::vector<int>                 _textList1;
   std::vector<int>                 _textList2;
   std::vector<int>                 _circuit;
   int                              _seed;
   std::vector<std::vector<int> >   _leastCostMatrix;
   std::vector<std::vector<int> >   _pathMatrix;
   std::vector<bool>                _attrMatch;
   std::vector<bool>                _textMatch1;
   std::vector<bool>                _textMatch2;
   bool                             _needNewLine;
   std::vector<unsigned long long>  _attrHash;
   std::vector<unsigned long long>  _textHash;
   std::vector<std::string>         _attrTag;
   XTree*                           _xtree1;
   XTree*                           _xtree2;
   XLut*                            _xlut;
};

/** patches an old version of an XML doc with a diff to produce the new version.  Here new and old versions of
   a doc refer to the parameters in the creation of the XDiff object that created \a diff.  So for XMLDocs 
   doc1, doc2, and diff, after executing XDiff(doc1, doc2, diff) and XPatch(doc1, diff), doc1 == doc2.*/
void XPatch(XMLDoc& old, const XMLDoc& diff);

#endif

