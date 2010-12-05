/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of auxiliary classes OgmlAttributeValue,
 *        OgmlAttribute and OgmlTag.
 * 
 * \author Christian Wolf and Bernd Zey
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 *
 * Copyright (C). All rights reserved.
 * See README.txt in the root directory of the OGDF installation for details.
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#ifdef _MSC_VER
#pragma once
#endif

//KK: Commented out the compound and constraint stuff using //o

#ifndef OGDF_OGMLPARSER_H
#define OGDF_OGMLPARSER_H

#include <ogdf/fileformats/Ogml.h>
#include <ogdf/fileformats/DinoXmlParser.h>
#include <ogdf/basic/Hashing.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/String.h>
// graph types
#include <ogdf/cluster/ClusterGraph.h>
//o#include <ogdf/CompoundGraph.h>
// graph attributes
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/cluster/ClusterGraphAttributes.h>
//o#include <ogdf/CompoundGraphAttributes.h>
// constraints
//o#include <ogdf/Constraints.h>
// stdlib for converting strings to basic types
#include <stdlib.h>
#include <stdarg.h>
#include <sstream>

namespace ogdf {
	
	// struct definitions for mapping of templates	
	struct OgmlNodeTemplate{ 
		String m_id;
		int m_shapeType;
		double m_width;
		double m_height;
		String m_color;
		GraphAttributes::BrushPattern m_pattern;
		String m_patternColor;
		GraphAttributes::EdgeStyle m_lineType;
		double m_lineWidth;
		String m_lineColor;
		// nodeTemplate stores the graphical type
		// e.g. rectangle, ellipse, hexagon, ...
		String m_nodeTemplate;
		
		//Constructor:
		OgmlNodeTemplate(String id): m_id(id) {};
	};//struct OgmlNodeTemplate
	
	struct OgmlEdgeTemplate{
		String m_id;
		GraphAttributes::EdgeStyle m_lineType;
		double m_lineWidth;
		String m_color;
		int m_sourceType; // actually this is only a boolean value 0 or 1
		// ogdf doesn't support source-arrow-color and size
//		String m_sourceColor;
//		double m_sourceSize;
		int m_targetType; // actually this is only a boolean value 0 or 1
		// ogdf doesn't support target-arrow-color and size
//		String m_targetColor;
//		double m_targetSize;
		
		//Constructor:
		OgmlEdgeTemplate(String id): m_id(id) {};	
	};//struct OgmlEdgeTemplate


//	struct OgmlLabelTemplate{
//		String m_id;
//	};//struct OgmlLabelTemplate

	struct OgmlSegment{
		DPoint point1, point2;
	};
	
	
	//
	// ---------- O g m l A t t r i b u t e V a l u e ------------------------
	//
	
	/**Objects of this class represent a value set of an attribute in Ogml.
	 */
	class OgmlAttributeValue {
		
		/**Id of the attribute value.
		 * For possible ones see Ogml.h.
		 */
		int id;
		
		public:
			//Construction
			OgmlAttributeValue() : id(av_any) {}
			OgmlAttributeValue(int id) {
				if(id>=0 && id<ATT_VAL_NUM) this->id = id;
				else id = av_any;
			}
			//Destruction
			~OgmlAttributeValue() {}
			//Getter
			const int& getId() const { return id; }
			const String& getValue() const { return ogmlAttributeValueNames[id]; }		
			//Setter
			void setId(int id) {
				if(id>=0 && id<ATT_VAL_NUM) this->id = id;
				else id = av_any;
			}			

			
			/* Checks the type of the input given in string
			 * and returns an OgmlAttributeValueId defined in Ogml.h
			 */	
			OgmlAttributeValueId getTypeOfString(const String& input) const {
				
				// |--------------------|
				// | char | ascii-value |
				// |--------------------|
				// | '.'  |     46      |
				// | '-'  |     45      |
				// | '+'  |     43      |
				// | '#'  |		35		|
				
				// bool values
				bool isInt = true;
				bool isNum = true;
				bool isHex = true;
				// value for point seperator
				bool numPoint = false;	
				
				// input is a boolean value
				if (input == "true" || input == "false" /*|| input == "0" || input == "1"*/)
					return av_bool;
	
				if (input.length() > 0){
					char actChar = input[0];
					int actCharInt = static_cast<int>(actChar);	
					//check the first char
					if (!isalnum(actChar)){

						if ((actCharInt == 35)){
							// support hex values with starting "#"
							isInt = false;
							isNum = false;
						}
						else
							{
	
							// (actChar != '-') and (actChar != '+')
							if (!(actCharInt == 45) && !(actChar == 43)){
								isInt = isNum = false;
							}
							else
							{
								// input[0] == '-' or '+'
								if (input.length() > 1){
									// 2nd char have to be a digit or xdigit
									actChar = input[1];
									actCharInt = static_cast<int>(actChar);
									if (!isdigit(actChar)){
										isInt = false;
										isNum = false;
										if (!isxdigit(actChar))
											return av_string;
									}
								}
								else
									return av_string;
							} // else... (input[0] == '-')
						}
					}
					else{				
						if (!isdigit(actChar)){
							isInt = false;	
							isNum = false;
						}
						if (!isxdigit(actChar)){
							isHex = false;
						}
					}
					
					// check every input char
					// and set bool value to false if char-type is wrong
					for(size_t it=1; ( (it<input.length()) && ((isInt) || (isNum) || (isHex)) ); it++)
					{
						actChar = input[it];
						actCharInt = static_cast<int>(actChar);
						
						// actChar == '.'
						if (actChar == 46){
							isInt = false;
							isHex = false;
							if (!numPoint){
								numPoint = true;
							}
							else
								isNum = false;
						}// if (actChar == '.')
						else {
							if (!(isdigit(actChar))){
								isInt = false;
								isNum = false;
							}
							if (!(isxdigit(actChar)))
								isHex = false;
						}//else... (actChar != '.')
					}//for
				}//if (input.length() > 0)
				else{
					// input.length() == 0
					return av_none;	
				}
				// return correct value
				if (isInt) return av_int;
				if (isNum) return av_num;
				if (isHex) return av_hex;
				// if all bool values are false return av_string
				return av_string;
				
			}//getTypeOfString
					
					
			/**According to id this method proofs whether s is a valid value
			 * of the value set.
			 * E.g. if id=av_int s should contain an integer value.
			 * It returns the following validity states:
			 * 		vs_idNotUnique    =-10, //id already exhausted
			 * 		vs_idRefErr       = -9, //referenced id wasn't found or wrong type of referenced tag
			 * 		vs_idRefErr       = -8, //referenced id wrong
			 *      vs_attValueErr    = -3, //attribute-value error
			 * 		vs_valid		  =  1  //attribute-value is valid
			 * 
			 * TODO: Completion of the switch-case statement.
			 */
			int validValue(const String &attributeValue, 
						   const XmlTagObject* xmlTag,		        //owns an attribute with attributeValue
					       Hashing<String, const XmlTagObject*>& ids) const  { 	//hashtable with id-tagName pairs
				
				//get attribute value type of string
				OgmlAttributeValueId stringType = getTypeOfString(attributeValue);

				HashElement<String, const XmlTagObject*>* he;
				
				int valid = vs_attValueErr;
				
				switch(id) {
					case av_any: {
						valid = vs_valid;
						break;
					}
					case av_int: {
						if (stringType == av_int) valid = vs_valid;
						break;
					}
					case av_num: {
						if (stringType == av_num) valid = vs_valid;
						if (stringType == av_int) valid = vs_valid;
						break;
					}
					case av_bool: {
						if (stringType == av_bool) valid = vs_valid;
						break;
					}
					case av_string: {
						valid = vs_valid;
						break;
					}
					case av_hex: {
						if (stringType == av_hex) valid = vs_valid;
						if (stringType == av_int) valid = vs_valid;
						break;
					}
					case av_oct: {
						valid = vs_attValueErr;
						break;
					}
					case av_id: {
						//id mustn't exist
						if( !(he = ids.lookup(attributeValue)) ) {
							ids.fastInsert(attributeValue, xmlTag);
							valid = vs_valid;
						}
						else valid = vs_idNotUnique;
						break;
					}
	
					 //attribute idRef of elements source, target, nodeRef, nodeStyle    
					case av_nodeIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_node]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;	
					}
					//attribute idRef of elements edgeRef, edgeStyle
					case av_edgeIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_edge]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;	
					}
					//attribute idRef of elements labelRef, labelStyle
					case av_labelIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_label]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;	
					}
					//attribute idRef of element endpoint
					case av_sourceIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_source]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;	
					}
					//attribute idRef of element endpoint
					case av_targetIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_target]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;	
					}
					//attribute idRef of subelement template of element nodeStyle
					case av_nodeStyleTemplateIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_nodeStyleTemplate]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;	
					}
					//attribute idRef of subelement template of element edgeStyle
					case av_edgeStyleTemplateIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_edgeStyleTemplate]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;
					}
					//attribute idRef of subelement template of element labelStyle
					case av_labelStyleTemplateIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_labelStyleTemplate]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;
					}
					case av_pointIdRef: {
						//element exists && is tagname expected
						if( (he = ids.lookup(attributeValue)) && (he->info()->getName() == ogmlTagNames[t_point]) ) valid = vs_valid;
						else valid = vs_idRefErr;
						break;
					}
					default: {
						//Proof string for equality
						if(getValue() == attributeValue) valid = vs_valid;
						break;
					}
				}
				
				return valid;
			}
		
	};//class OgmlAttributeValue
	
	
	
	
	
	//
	// ---------- O g m l A t t r i b u t e ------------------------
	//
	
	/**Objects of this class represent an attribute and its value set in Ogml.
	 */
	class OgmlAttribute {
		
		/**Integer identifier of object.
		 * For possible ids see Ogml.h.
		 */
		int id;
		/**Represents the value set of this attribute.
		 */
		List<OgmlAttributeValue*> values;
		
		public:

			//Construction
			OgmlAttribute() : id(a_none), values() {}
			OgmlAttribute(int id) : values() {
				if(id>=0 && id<ATT_NUM) this->id = id;
				else this->id = a_none;
			}
			//Destruction
			~OgmlAttribute() {}
			//Getter
			const int& getId() const { return id; }
			const String& getName() const { return ogmlAttributeNames[id]; }
			const List<OgmlAttributeValue*>& getValueList() const { return values; }
			
			//Setter
			void setId(int id) {
				if(id>=0 && id<ATT_NUM) this->id = id;
				else this->id = a_none;
			}
	
			/**Pushes pointers to OgmlAttributeValue objects back to list values.
			 * These value objects are looked up in hashtable values.
			 * 
			 * NOTE: This method uses a variable parameter list. The last parameter
			 *       need to be -1!
			 */
			 void pushValues(Hashing<int, OgmlAttributeValue> &val, int key, ...) {
				va_list argp;
				int arg = key;
				HashElement<int, OgmlAttributeValue>* he;
				va_start(argp, key);
				while(arg!=-1) {
					if((he = val.lookup(arg))) values.pushBack( &(he->info()) );
					arg = va_arg(argp,int);
				}
				va_end(argp);
			}
		
			/**Prints the value set of the attribute.
			 */
			void print(ostream &os) const {
				ListConstIterator<OgmlAttributeValue*> it;
				os << "\"" << getName() << "\"={ ";
				for(it = values.begin(); it.valid(); it++) {
					os << (**it).getValue() << " ";
				}
				os << "}\n";
			}
		
			/**This method proofs whether o is a valid attribute in comparison
			 * to this object.
			 * That means if the name of o and this object are equal and if
		 	 * o has a valid value.
		 	 * It returns a validity state code (see Ogml.h).
		 	 **/
			int validAttribute(const XmlAttributeObject &xmlAttribute,
							   const XmlTagObject* xmlTag,
							   Hashing<String, const XmlTagObject*>& ids) const {
							   	
				int valid = vs_expAttNotFound;
				
				if( xmlAttribute.getName() == getName() ) {
					ListConstIterator<OgmlAttributeValue*> it;
					for(it = values.begin(); it.valid(); it++) {
						if ( (valid = (**it).validValue( xmlAttribute.getValue(), xmlTag, ids )) == vs_valid ) break;
					}
				}
				
				return valid;
			}
			
		
	};//class OgmlAttribute
	
	/* Overloaded <<-operator to print an attribute.
	 */
	ostream& operator<<(ostream& os, const OgmlAttribute& oa);
	
	
	
	//
	// ---------- O g m l T a g ------------------------
	//
	
	/**Objects of this class represent a tag in Ogml with attributes.
	 */
	class OgmlTag {
		
		/**Integer identifier of object.
		 * For possible ids see Ogml.h.
		 */
		int id;
		/**Min. occurs and max. occurs of this tag.
		 */
		int minOccurs, maxOccurs;
		/**Flag denotes whether tag content can be ignored.
		 * It is possible to exchange this flag by a list of contents for more
		 * complex purposes ;-)
		 */
		bool ignoreContent;
		/**Represents the compulsive attributes of this object.
		 */ 
		List<OgmlAttribute*> compulsiveAttributes;	
		/**Represents the attributes of this object of which at least
		 * one needs to exist.
		 */ 
		List<OgmlAttribute*> choiceAttributes;
		/**Represents the optional attributes of this object.
		 */ 
		List<OgmlAttribute*> optionalAttributes;
		
		List<OgmlTag*> compulsiveTags;
		
		List<OgmlTag*> choiceTags;
		
		List<OgmlTag*> optionalTags;	
		
		
		void printOwnedTags(ostream &os, int mode) const {
		
			String s;
			const List<OgmlTag*> *list;
			
			switch(mode) {
				case 0: {
					list = &compulsiveTags;
					s+="compulsive";
					break;
				}
				case 1: {
					list = &choiceTags;
					s+="selectable";
					break;
				}
				case 2: {
					list = &optionalTags;
					s+="optional";
					break;
				}
			}
			
			if(list->empty()) os << "Tag \"<" << getName() <<">\" doesn't include " << s << " tag(s).\n";
			else {
				os << "Tag \"<" << getName() <<">\" includes the following " << s << " tag(s): \n";
				ListConstIterator<OgmlTag*> currTag;
				for(currTag = list->begin(); currTag.valid(); currTag++) os << "\t<" << (**currTag).getName() << ">\n";
			}
		}		
		
		void printOwnedAttributes(ostream &os, int mode) const {

			String s;
			const List<OgmlAttribute*> *list;
			
			switch(mode) {
				case 0: {
					list = &compulsiveAttributes;
					s+="compulsive";
					break;
				}
				case 1: {
					list = &choiceAttributes;
					s+="selectable";
					break;
				}
				case 2: {
					list = &optionalAttributes;
					s+="optional";
					break;
				}
			}
			
			if(list->empty()) os << "Tag \"<" << getName() <<">\" doesn't include " << s << " attribute(s).\n";
			else {
				cout << "Tag \"<" << getName() <<">\" includes the following " << s << " attribute(s): \n";
				ListConstIterator<OgmlAttribute*> currAtt;
				for(currAtt = list->begin(); currAtt.valid(); currAtt++) os << "\t"  << (**currAtt);
			}	
		}
		
		
		public:
			
			bool ownsCompulsiveTags() {
				return !compulsiveTags.empty();	
			}
			
			bool ownsChoiceTags() {
				return !choiceTags.empty();	
			}
			
			bool ownsOptionalTags() {
				return !optionalTags.empty();	
			}
			
			const List<OgmlTag*>& getCompulsiveTags() const {return compulsiveTags;}
			
			const List<OgmlTag*>& getChoiceTags() const {return choiceTags;}
			
			const List<OgmlTag*>& getOptionalTags() const {return optionalTags;}

			
			const int& getMinOccurs() const { return minOccurs; }
			
			const int& getMaxOccurs() const { return maxOccurs; }

			const bool& ignoresContent() const { return ignoreContent; }
			
			void setMinOccurs(int occurs) { minOccurs = occurs; }
			
			void setMaxOccurs(int occurs) { maxOccurs = occurs; }
			
			void setIgnoreContent(bool ignore) { ignoreContent = ignore; }
			
			//Construction
			OgmlTag() : id(t_none), ignoreContent(0) { }
			OgmlTag(int id) : id(t_none), ignoreContent(0) {
				if(id>=0 && id<TAG_NUM) this->id = id;
				else id = a_none;
			}
			//Destruction
			~OgmlTag() {}
			//Getter
			const int& getId() const { return id; }
			const String& getName() const { return ogmlTagNames[id]; }
			//Setter	
			void setId(int id){
				if(id>=0 && id<TAG_NUM) this->id = id;
				else id = a_none;
			}
			
			
			void printOwnedTags(ostream& os) const {
				printOwnedTags(os, 0);
				printOwnedTags(os, 1);
				printOwnedTags(os, 2);
			}
			
			void printOwnedAttributes(ostream& os) const {
				printOwnedAttributes(os, 0);
				printOwnedAttributes(os, 1);
				printOwnedAttributes(os, 2);
			}
			
			/**Pushes pointers to OgmlAttribute objects back to list reqAttributes.
			 * These value objects are looked up in hashtable attrib.
			 * 
			 * NOTE: This method uses a variable parameter list. The last parameter
			 *       need to be -1!
			 */
			void pushAttributes(int mode, Hashing<int, OgmlAttribute> &attrib, int key, ...) {
				
				List<OgmlAttribute*>* list;
				
				if(mode==0) list = &compulsiveAttributes;
				else if(mode==1) list = &choiceAttributes;
				else list = &optionalAttributes;
				
				va_list argp;
				int arg = key;
				HashElement<int, OgmlAttribute>* he;
				va_start(argp, key);
				while(arg!=-1) {
					if((he = attrib.lookup(arg))) (*list).pushBack( &(he->info()) );
					arg = va_arg(argp,int);
				}
				va_end(argp);
			}
			
			/**Pushes pointers to OgmlAttribute objects back to list reqAttributes.
			 * These value objects are looked up in hashtable attrib.
			 * 
			 * NOTE: This method uses a variable parameter list. The last parameter
			 *       need to be -1!
			 */
			void pushTags(int mode, Hashing<int, OgmlTag> &tag, int key, ...) {
				
				List<OgmlTag*>* list;
				
				if(mode==0) list = &compulsiveTags;
				else if(mode==1) list = &choiceTags;
				else list = &optionalTags;
				
				va_list argp;
				int arg = key;
				HashElement<int, OgmlTag>* he;
				va_start(argp, key);
				
				while(arg!=-1) {
					if((he = tag.lookup(arg))) (*list).pushBack( &(he->info()) );
					arg = va_arg(argp,int);
				}
				va_end(argp);
			}
			
			/**This method proofs whether o is a valid tag in comparison
			 * to this object.
			 * That means if the name of o and this object are equal and if
		 	 * the attribute list of o is valid (see also validAttribute(...)
		 	 * in OgmlAttribute.h). Otherwise false.
		 	 */
			int validTag(const XmlTagObject &o,
			              Hashing<String, const XmlTagObject*>& ids) const {
				
				int valid = vs_unexpTag;
				
				if( o.getName() == getName() ) {
					
					ListConstIterator<OgmlAttribute*> it;
					XmlAttributeObject* att;
					
					//Tag requires attributes
					if(!compulsiveAttributes.empty()) {
											
						for(it = compulsiveAttributes.begin(); it.valid(); it++) {
							//Att not found or invalid
							if(!o.findXmlAttributeObjectByName((**it).getName(), att) ) return valid = vs_expAttNotFound;
							if( (valid = (**it).validAttribute(*att, &o, ids) ) <0 ) return valid;
							//Att is valid
							att->setValid();
						}
						
					}
					
					//Choice attributes
					if(!choiceAttributes.empty()) {
						
						bool tookChoice = false;
						
						for(it = choiceAttributes.begin(); it.valid(); it++) {
							//Choice att found
							if( o.findXmlAttributeObjectByName((**it).getName(), att) ) {
								//Proof if valid
								if( (valid = (**it).validAttribute(*att, &o, ids)) <0 ) return valid;
								tookChoice = true;
								att->setValid();
							}
						}
						
						if(!tookChoice) return valid = vs_expAttNotFound;
						
					}
					
					if(!optionalAttributes.empty() && !o.isAttributeLess()) {
						
						//Check optional attributes
						for(it = optionalAttributes.begin(); it.valid(); it++) {
							if( o.findXmlAttributeObjectByName((**it).getName(), att) ) {
								if( (valid = (**it).validAttribute(*att, &o, ids)) <0 ) return valid;
								att->setValid();
							}
						}
					}

					//Are there still invalid attributes?
					att = o.m_pFirstAttribute;
					while(att) {
						if(!att->valid()) return valid = vs_unexpAtt;
						att = att->m_pNextAttribute;
					}
					
					valid = vs_valid;
				}
				
				return valid;
				
			}
		

		
		
	};//class OgmlTag
	
	/* Overloaded <<-operator to print a tag.
	 */
	ostream& operator<<(ostream& os, const OgmlTag& ot);
	
	
		
	//
	// ---------- O g m l P a r s e r ------------------------
	//
	
	/**Objects of this class represent a validating parser for files in Ogml.
	 */
	class OgmlParser {
	
	private:
	   /**Hashtable for saving all ogml tags.
		*/
		static Hashing<int, OgmlTag> tags;
		
		/**Hashtable for saving all ogml attributes.
		*/
		static Hashing<int, OgmlAttribute> attributes;
		
		/**Hashtable for saving all values of ogml attributes.
		*/
		static Hashing<int, OgmlAttributeValue> attValues;
		
		/**Flag denotes whether hashtables above were built or not.
		*/
		static bool hashTablesBuilt;
		
		/**Builds all hashtables above.
		*/
		static void buildHashTables();
		
		/**Saves a graph type. Is set by checkGraphType.
		 */
		mutable GraphType graphType;	
		
		/**Saves all ids of an ogml-file.
		 */
		Hashing<String, const XmlTagObject*> ids;
		
		/**Checks if all tags (XmlTagObject), their attributes (XmlAttributeObject) and 
		 * their values are valid (are tags expected, do they own the rigth attributes...)
		 * and sets a valid flag to these. Furthermore it checks if ids of tags are 
		 * unique and if id references are valid.
		 * See OgmlTag.h for semantics of the encodings.
		 * Returns the validity state of the current processed tag.
		 */
		int validate(const XmlTagObject *xmlTag, int ogmlTag);
		
		/**Wrapper method for validate method above.
		 * Returns true when validation is successfull, false otherwise.
		 */ 
		//bool validate(const char* fileName);
		/**Prints some useful information about un-/successful validation.
		 */
		void printValidityInfo(const OgmlTag &ot,
								const XmlTagObject &xto,
								int valStatus,
								int line);
	
		/**Finds the OGML-tag in the parse tree with the specified id,
		 * stores the tag in xmlTag
		 * recTag is the tag for recursive calls
		 * returns false if something goes wrong
		 */ 
		//bool getXmlTagObjectById(XmlTagObject *recTag, String id, XmlTagObject *&xmlTag);

		/**Checks the graph type and stores it in the member variable m_graphType
		 * xmlTag has to be the root or the graph or the structure Ogml-tag
		 * returns false if something goes wrong
		 */ 
		bool checkGraphType(const XmlTagObject *xmlTag) const;
	
		/**true if subgraph is an hierarchical graph
		 */
		//bool isHierarchical(XmlTagObject *xmlTag);
		bool isGraphHierarchical(const XmlTagObject *xmlTag) const;
	
		/**true if node contains other nodes
		 */
		bool isNodeHierarchical(const XmlTagObject *xmlTag) const; 
		
		GraphType getGraphType() { return graphType; };
	
	
	// id hash tables
		// required variables for building
		// hash table with id from file and node
		Hashing<String, node> m_nodes;
		Hashing<String, edge> m_edges;
		Hashing<String, cluster> m_clusters;
//o		Hashing<String, compound> m_compounds;
		// hash table for bend-points
		Hashing<String, DPoint> points;
		
		// hash table for checking uniqueness of ids
		// (key:) int = id in the created graph
		// (info:) String = id in the ogml file
		Hashing<int, String> m_nodeIds;
		Hashing<int, String> m_edgeIds;
		Hashing<int, String> m_clusterIds;
//o		Hashing<int, String> m_compoundIds;
		
	// build methods
		// build graph
		//  ignores nodes which have hierarchical structure
		bool buildGraph(Graph &G);
		
		// build clusterGraph
		bool buildCluster(
					const XmlTagObject *rootTag, 
					Graph &G, 
					ClusterGraph &CG);
		
		// recursive part of buildCluster
		bool buildClusterRecursive(
					const XmlTagObject *xmlTag, 
					cluster parent, 
					Graph &G, 
					ClusterGraph &CG);				

		// builds graph
		//  does not ignore hierarchical nodes
//o		bool buildGraphForCompounds(Graph &G);

		// build compoundGraph
//o		bool buildCompound(
//o					XmlTagObject *rootTag, 
//o					Graph &G, 
//o					CompoundGraph &CG);
		
		// recursive part of buildCompound
//o		bool buildCompoundRecursive(
//o					XmlTagObject *xmlTag, 
//o					compound parent, 
//o					Graph &G, 
//o					CompoundGraph &CG);	

		// big attributed graph building method
		bool buildAttributedClusterGraph(
					Graph &G, 
					ClusterGraphAttributes &CGA, 
					const XmlTagObject *root);

		// 2nd big attributed graph building method
//o		bool buildAttributedCompoundGraph(
//o					Graph &G, 
//o					CompoundGraphAttributes &CGA, 
//o					XmlTagObject *root);
		
		// method for setting labels of clusters
		bool setLabelsRecursive(
					Graph &G, 
					ClusterGraphAttributes &CGA, 
					XmlTagObject *root);

		// method for setting labels of compounds
//o		bool setLabelsRecursiveForCompounds(
//o					Graph &G, 
//o					CompoundGraphAttributes &CGA, 
//o					XmlTagObject *root);
		
		// constraints-builder
//o		bool buildConstraints(
//o					Graph& G, 
//o					GraphConstraints &GC);
		
		// helping pointer for constraints-loading
		// this pointer is set in the building methods
		// so we don't have to traverse the tree in buildConstraints
		XmlTagObject* m_constraintsTag;
					
		// hashing lists for templates
		//  string = id
		Hashing<String, OgmlNodeTemplate*> m_ogmlNodeTemplates;
		Hashing<String, OgmlEdgeTemplate*> m_ogmlEdgeTemplates;
		//Hashing<String, OgmlLabelTemplate> m_ogmlLabelTemplates;

	// auxiliary methods for mapping graph attributes
		// returns int value for the pattern
		int getBrushPatternAsInt(String s);
		
		// returns the shape as an integer value
		int getShapeAsInt(String s);
		
		// maps the OGML attribute values to corresponding GDE values
		String getNodeTemplateFromOgmlValue(String s);
		
		// line type
		int getLineTypeAsInt(String s);

		// image style
		int getImageStyleAsInt(String s);
		
		// alignment of image
		int getImageAlignmentAsInt(String s);
		
		// arrow style, actually a "boolean" function
		//  because it returns only 0 or 1 according to GDE
		// sot <=> source or target
		int getArrowStyleAsInt(String s, String sot);
		
		// the matching method to getArrowStyleAsInt
		GraphAttributes::EdgeArrow getArrowStyle(int i);
		
		// function that operates on a string
		// the input string contains "&lt;" instead of "<"
		//  and "&gt;" instead of ">"
		//  to disable interpreting the string as xml-tags (by DinoXmlParser)
		// so this function substitutes  "<" for "&lt;"
		String getLabelCaptionFromString(String str);
		
		// returns the integer value of the id at the end of the string - if existent		
		bool getIdFromString(String str, int id);
		
		//validiation method
		void validate(const char* fileName);
		
	public:
		
		//Construction
		OgmlParser() { }
		//Destruction
		~OgmlParser() {}
		
		
		/* Builds a graph from file with name fileName and saves
		 * the graph in G.
		 * Returns true when successful, false otherwise.
		 */	
		bool read(
				const char* fileName, 
				Graph &G, 
				ClusterGraph &CG);

		bool read(
				const char* fileName, 
				Graph &G, 
				ClusterGraph &CG,
				ClusterGraphAttributes &CGA);
		
//o		bool read(
//o				const char* fileName, 
//o				Graph &G, 
//o				CompoundGraph &CG,
//o				CompoundGraphAttributes &CGA);

//o		bool read(
//o				const char* fileName, 
//o				Graph &G, 
//o				CompoundGraph &CG,
//o				CompoundGraphAttributes &CGA,
//o				GraphConstraints &GC);
		  		
	
	};//end class OGMLParser
	
}//end namespace ogdf

#endif

