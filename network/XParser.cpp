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

#include "XParser.hpp"
#include "XTree.hpp"
#include "XHash.hpp"

#include "../GG/XML/XMLDoc.h"

#include <expat.h>

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
   _idStack = vector<int>(STACK_SZ, 0);
   _lsidStack = vector<int>(STACK_SZ, 0);
   _valueStack = vector<unsigned long long>(STACK_SZ, 0);
   _stackTop = 0;
   _currentNodeID = XTree::NULL_NODE;
   _idStack[_stackTop] = XTree::NULL_NODE;
   _readElement = false;
   _xtree = new XTree();
   
   XML_Parser p = XML_ParserCreate(0);
   XML_SetElementHandler(p, &XParser::BeginElement, &XParser::EndElement);
   XML_SetCharacterDataHandler(p, &XParser::CharacterData);
   s_curr_parser = this;
   string parse_str;
   ifstream is(filename.c_str());
   while (is) {
      string str;
      getline(is, str);
      parse_str += str + '\n';
   }
   is.close();
   XML_Parse(p, parse_str.c_str(), parse_str.size(), true);
   XML_ParserFree(p);
   s_curr_parser = 0;
   
   return _xtree;
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
   
   std::stringstream is;
   doc.WriteDoc(is);
   
   XML_Parser p = XML_ParserCreate(0);
   XML_SetElementHandler(p, &XParser::BeginElement, &XParser::EndElement);
   XML_SetCharacterDataHandler(p, &XParser::CharacterData);
   s_curr_parser = this;
   string parse_str;
   while (is) {
      string str;
      getline(is, str);
      parse_str += str + '\n';
   }
   XML_Parse(p, parse_str.c_str(), parse_str.size(), true);
   XML_ParserFree(p);
   s_curr_parser = 0;
   
   return _xtree;
}

void XParser::BeginElement(void* user_data, const char* name, const char** attrs) 
{
   if (s_curr_parser) 
      s_curr_parser->BE(name, attrs);
}

void XParser::BE(const char* name, const char** attrs) 
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
   
   int eid = _xtree->addElement(_idStack[_stackTop], _lsidStack[_stackTop], name);
   // Update last sibling info.
   _lsidStack[_stackTop] = eid;
   
   // Push
   _idStack[++_stackTop] = eid;
   _currentNodeID = eid;
   _lsidStack[_stackTop] = XTree::NULL_NODE;
   _valueStack[_stackTop] = XHash::hash(name);
   
   // Take care of attributes
   while (attrs && *attrs) {
      string attr_name(*attrs);
      unsigned long long namehash = XHash::hash(attr_name);
      unsigned long long attrhash = namehash;
      string attr_value(*(attrs + 1));
      attrhash += XHash::hash(attr_value);
      int aid = _xtree->addAttribute(eid, _lsidStack[_stackTop], attr_name, attr_value, namehash, attrhash);
      _lsidStack[_stackTop] = aid;
      _currentNodeID = aid + 1;
      _valueStack[_stackTop] += attrhash;
      attrs += 2;
   }
   
   _readElement = true;
   _elementBuffer = "";
   found_first_quote = found_last_quote = false;
}

void XParser::CharacterData(void *user_data, const char *s, int len)
{
   if (s_curr_parser) 
      s_curr_parser->CD(s, len);
}

void XParser::CD(const char *s, int len)
{
   if (!found_last_quote) {
      string str;
      for (int i = 0; i < len; ++i, ++s) {
         char c = *s;
         if (c == '"') {
            if (!found_first_quote) {
               found_first_quote = true;
               continue;
            } else if (found_first_quote && !found_last_quote && i == len - 1) {
               found_last_quote = true;
               break;
            }
         }
         if (found_first_quote)
            str += c;
      }
      _elementBuffer += str;//s_element_stack.back()->m_text += str;
   }
}

void XParser::EndElement(void* user_data, const char* name)
{
   if (s_curr_parser) 
      s_curr_parser->EE(name);
}

void XParser::EE(const char* name)
{
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

