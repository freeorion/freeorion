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

#include "XParser.hpp"
#include "XTree.hpp"
#include "XHash.hpp"

#include "XMLDoc.h"

#include <string>
#include <fstream>
#include <sstream>

using std::string;
using std::vector;
using std::ifstream;

XParser* XParser::s_curr_parser = 0;

namespace {
const int STACK_SZ = 64;
bool found_first_quote = false;
bool found_last_quote = false;
}

XTree* XParser::parse(const string& filename)
{
    GG::XMLDoc doc;
    ifstream ifs(filename.c_str());
    doc.ReadDoc(ifs);
    ifs.close();

    return parse(doc);
}

XTree* XParser::parse(const GG::XMLDoc& doc)
{
    _idStack = vector<int>(STACK_SZ, 0);
    _lsidStack = vector<int>(STACK_SZ, 0);
    _valueStack = vector<unsigned long long>(STACK_SZ, 0);
    _stackTop = 0;
    _currentNodeID = XTree::NULL_NODE;
    _idStack[_stackTop] = XTree::NULL_NODE;
    _readElement = false;
    _xtree = new XTree();

    recursive_parse(doc.root_node);

    return _xtree;
}

void XParser::recursive_parse(const GG::XMLElement& elem)
{
    // if text is mixed with elements.
    if (!_elementBuffer.empty()) {
        unsigned long long value = XHash::hash(_elementBuffer);
        int tid = _xtree->addText(_idStack[_stackTop],
                                  _lsidStack[_stackTop],
                                  _elementBuffer, value);
        _lsidStack[_stackTop] = tid;
        _currentNodeID = tid;
        _valueStack[_stackTop] += value;
    }

    int eid = _xtree->addElement(_idStack[_stackTop], _lsidStack[_stackTop], elem.Tag());
    // Update last sibling info.
    _lsidStack[_stackTop] = eid;

    // Push
    _idStack[++_stackTop] = eid;
    _currentNodeID = eid;
    _lsidStack[_stackTop] = XTree::NULL_NODE;
    _valueStack[_stackTop] = XHash::hash(elem.Tag());

    // Take care of attributes
    for (GG::XMLElement::const_attr_iterator it = elem.attr_begin(); it != elem.attr_end(); ++it) {
        unsigned long long namehash = XHash::hash(it->first);
        unsigned long long attrhash = namehash;
        attrhash += XHash::hash(it->second);
        int aid = _xtree->addAttribute(eid, _lsidStack[_stackTop], it->first, it->second, namehash, attrhash);
        _lsidStack[_stackTop] = aid;
        _currentNodeID = aid + 1;
        _valueStack[_stackTop] += attrhash;
    }

    _readElement = true;
    _elementBuffer = elem.Text();


    for (GG::XMLElement::const_child_iterator it = elem.child_begin(); it != elem.child_end(); ++it) {
        recursive_parse(*it);
    }


    if (_readElement) {
        if (!_elementBuffer.empty()) {
            unsigned long long value = XHash::hash(_elementBuffer);
            _currentNodeID = _xtree->addText(_idStack[_stackTop],
                                             _lsidStack[_stackTop],
                                             _elementBuffer, value);
            _valueStack[_stackTop] += value;
        } else {
            _currentNodeID = _xtree->addText(_idStack[_stackTop],
                                             _lsidStack[_stackTop],
                                             "", 0);
        }
        _readElement = false;
    } else { // mixed contents
        if (!_elementBuffer.empty()) {
            unsigned long long value = XHash::hash(_elementBuffer);
            _currentNodeID = _xtree->addText(_idStack[_stackTop],
                                             _lsidStack[_stackTop],
                                             _elementBuffer, value);
            _valueStack[_stackTop] += value;
        }
    }

    _elementBuffer = "";
    _xtree->addHashValue(_idStack[_stackTop], _valueStack[_stackTop]);
    _valueStack[_stackTop - 1] += _valueStack[_stackTop];
    _lsidStack[_stackTop - 1] = _idStack[_stackTop];

    // Pop
    _stackTop--;
}

