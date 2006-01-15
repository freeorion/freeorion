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

#ifndef   __XPARSER__
#define __XPARSER__

#include <vector>
#include <string>


class XMLDoc;
class XMLElement;
class XTree;

/** takes an XML file or XMLDoc and constructs an XTree from it. */
class XParser
{
public:
    XTree* parse(const std::string& filename);
    XTree* parse(const XMLDoc& doc);

private:
    void recursive_parse(const XMLElement& elem);
   
    XTree*                           _xtree;
    std::vector<int>                 _idStack;
    std::vector<int>                 _lsidStack; // id and left sibling
    std::vector<unsigned long long>  _valueStack;
    int                              _stackTop;
    int                              _currentNodeID;
    bool                             _readElement;
    std::string                      _elementBuffer;
   
    static XParser* s_curr_parser;
};

#endif


