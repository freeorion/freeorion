/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of OGML parser.
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

#include <ogdf/fileformats/OgmlParser.h>

namespace ogdf {

//Definition of Hashtables
Hashing < int, OgmlTag > OgmlParser::tags;
Hashing < int, OgmlAttribute > OgmlParser::attributes;
Hashing < int, OgmlAttributeValue > OgmlParser::attValues;
bool OgmlParser::hashTablesBuilt;

// ***********************************************************
//
// b u i l d H a s h T a b l e s
//
// ***********************************************************
void OgmlParser::buildHashTables()
{

	if(!hashTablesBuilt) {

		int i;
		OgmlAttributeValue *val;
		OgmlAttribute *att;
		OgmlTag *tag;

		/**Create OgmlAttributeValue objects and fill hashtable attValues.
		 */
		for (i = 0; i < ATT_VAL_NUM; i++) {
			
			val = new OgmlAttributeValue(i);
			OgmlParser::attValues.fastInsert(i, *val);
			
		}

		for (i = 0; i < ATT_NUM; i++) {
			
			att = new OgmlAttribute(i);
			OgmlParser::attributes.fastInsert(i, *att);
			
		}


		/**Create OgmlAttribute objects and fill hashtable attributes.
		 */
		for (i = 0; i < ATT_NUM; i++) {

			att = &attributes.lookup(i)->info();

			switch (i) {

			case a_alignment:
				att->pushValues(attValues, 
								av_left, 
								av_center, 
								av_right, 
								av_justify, 
								-1);
				break;

			case a_angle:
				att->pushValues(attValues, 
								av_int, 
								-1);
				break;

			case a_color:
				att->pushValues(attValues, 
								av_hex, 
								-1);
				break;

			case a_decoration:
				att->pushValues(attValues,
								av_underline, 
								av_overline, 
								av_lineThrough, 
								av_blink, 
								av_none, 
								-1);
				break;

			case a_defaultEdgeTemplate:
				att->pushValues(attValues, av_any, -1);
				break;

			case a_defaultLabelTemplate:
				att->pushValues(attValues, av_any, -1);
				break;

			case a_defaultNodeTemplate:
				att->pushValues(attValues, av_any, -1);
				break;

			case a_family:
				att->pushValues(attValues,
								av_serif, 
								av_sansSerif, 
								av_cursive, 
								av_fantasy, 
								av_monospace, 
								-1);
				break;

			case a_height:
				att->pushValues(attValues, av_num, -1);
				break;

			case a_id:
				att->pushValues(attValues, av_id, -1);
				break;

			case a_nodeIdRef:
				att->pushValues(attValues, av_nodeIdRef, -1);
				break;

			case a_edgeIdRef:
				att->pushValues(attValues, av_edgeIdRef, -1);
				break;

			case a_labelIdRef:
				att->pushValues(attValues, av_labelIdRef, -1);
				break;

			case a_sourceIdRef:
				att->pushValues(attValues, av_nodeIdRef, av_edgeIdRef, -1);
				break;

			case a_targetIdRef:
				att->pushValues(attValues, av_nodeIdRef, av_edgeIdRef, -1);
				break;

			case a_nodeStyleTemplateIdRef:
				att->pushValues(attValues, av_nodeStyleTemplateIdRef, -1);
				break;

			case a_edgeStyleTemplateIdRef:
				att->pushValues(attValues, av_edgeStyleTemplateIdRef, -1);
				break;

			case a_labelStyleTemplateIdRef:
				att->pushValues(attValues, av_labelStyleTemplateIdRef, -1);
				break;

			case a_endpointIdRef:
				att->pushValues(attValues, av_pointIdRef, av_sourceIdRef, av_targetIdRef, -1);
				break;

			case a_name:
				att->pushValues(attValues, av_any, -1);
				break;

				//attribute type of subelement line of tag nodeStyleTemplate
			case a_nLineType:
				att->pushValues(attValues,
						av_solid,
						av_dotted,
						av_dashed,
						av_double,
						av_triple, 
						av_groove, 
						av_ridge, 
						av_inset, 
						av_outset, 
						av_none,
						av_esNoPen,
						av_esSolid,
						av_esDash,
						av_esDot,
						av_esDashdot,
						av_esDashdotdot, 
						-1);
				break;

				//attribute type of subelement shape of tag nodeStyleTemplate
			case a_nShapeType:
				att->pushValues(attValues,
						av_rect,
						av_rectSimple,
						av_triangle,
						av_circle,
						av_ellipse,
						av_hexagon,
						av_rhomb,
						av_trapeze,
						av_upTrapeze,
						av_lParallelogram,
						av_rParallelogram, 
						av_pentagon, 
						av_octagon, 
						av_umlClass, 
						av_image, 
						-1);
				break;

			case a_pattern:
				att->pushValues(attValues, 
								av_solid, 
								av_striped, 
								av_checked, 
								av_dotted, 
								av_none,
								av_bpNone,
								av_bpSolid,
								av_bpDense1,
								av_bpDense2,
								av_bpDense3,
								av_bpDense4,
								av_bpDense5,
								av_bpDense6,
								av_bpDense7,
								av_bpHorizontal,
								av_bpVertical,
								av_bpCross,
								av_bpBackwardDiagonal,
								av_bpForwardDiagonal,
								av_bpDiagonalCross,-1);
				break;

			case a_patternColor:
				att->pushValues(attValues, av_hex, -1);
				break;

			case a_rotation:
				att->pushValues(attValues, av_int, -1);
				break;

			case a_size:
				att->pushValues(attValues, av_int, -1);
				break;

			case a_stretch:
				att->pushValues(attValues,
						av_normal,
						av_wider,
						av_narrower,
						av_ultraCondensed,
						av_extraCondensed,
						av_condensed,
						av_semiCondensed,
						av_semiExpanded, av_expanded, av_extraExpanded, av_ultraExpanded, -1);
				break;

			case a_style:
				att->pushValues(attValues, av_normal, av_italic, av_oblique, -1);
				break;

			case a_transform:
				att->pushValues(attValues, av_capitalize, av_uppercase, av_lowercase, av_none, -1);
				break;

				//attribute type of subelements source-/targetStyle of tag edgeStyleTemplate
			case a_type:
				att->pushValues(attValues,
						av_circle,
						av_halfCircle,
						av_filledCircle,
						av_filledHalfCircle,
						av_box,
						av_halfBox,
						av_filledBox,
						av_filledHalfBox,
						av_rhomb,
						av_halfRhomb,
						av_filledRhomb,
						av_filledHalfRhomb,
						av_diamond,
						av_halfDiamond,
						av_filledDiamond,
						av_filledHalfDiamond,
					    av_smurf,
    					av_arrow,
						av_slash, 
						av_doubleSlash, 
						av_solid, 
						av_line, 
						av_none, -1);
				break;

			case a_uri:
				att->pushValues(attValues, av_uri, -1);
				break;

			case a_intValue:
				att->pushValues(attValues, av_int, -1);
				break;

			case a_numValue:
				att->pushValues(attValues, av_num, -1);
				break;

			case a_boolValue:
				att->pushValues(attValues, av_bool, -1);
				break;

			case a_variant:
				att->pushValues(attValues, av_normal, av_smallCaps, -1);
				break;

			case a_weight:
				att->pushValues(attValues, 
								av_normal, 
								av_bold, 
								av_bolder, 
								av_lighter, 
								av_int, 
								-1);
				break;

			case a_width:
				att->pushValues(attValues, av_num, -1);
				break;

			case a_x:
				att->pushValues(attValues, av_num, -1);
				break;

			case a_y:
				att->pushValues(attValues, av_num, -1);
				break;

			case a_z:
				att->pushValues(attValues, av_num, -1);
			
			case a_imageUri:
				att->pushValues(attValues, av_string, -1);
			
			case a_imageStyle:
				att->pushValues(attValues, av_freeScale, av_fixScale, -1);
			
			case a_imageAlignment:
				att->pushValues(attValues, av_topLeft, 
										av_topCenter, 
										av_topRight, 
										av_centerLeft,
										av_center, 
										av_centerRight, 
										av_bottomLeft, 
										av_bottomCenter, 
										av_bottomRight, -1);
			
			case a_imageDrawLine:
				att->pushValues(attValues, av_bool, -1);
			
			case a_imageWidth:
				att->pushValues(attValues, av_num, -1);
			
			case a_imageHeight:
				att->pushValues(attValues, av_num, -1);
			
			case a_constraintType:
				att->pushValues(attValues, av_constraintAlignment, av_constraintAnchor, av_constraintSequence, -1);

			case a_disabled:
				att->pushValues(attValues, av_bool, -1);

			}//switch

		}


		/**Create OgmlTag objects and fill hashtable tags.
		 */
		for (i = 0; i < TAG_NUM; i++) {

			tag = new OgmlTag(i);
			OgmlParser::tags.fastInsert(i, *tag);

		}

		enum Mode { compMode = 0, choiceMode, optMode };

		/**Create tag relations.
		 */
		for (i = 0; i < TAG_NUM; i++) {

			tag = &tags.lookup(i)->info();

			switch (i) {
			case t_bool:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_boolValue, -1);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				break;

			case t_composed:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				tag->pushTags(choiceMode, tags, t_num, t_int, t_bool,
					      t_string, t_nodeRef, t_edgeRef, t_labelRef, t_composed, -1);
				break;

			case t_constraint:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_constraintType, -1);
				tag->pushAttributes(choiceMode, attributes, a_id, a_name, a_disabled, -1);
				tag->pushTags(choiceMode, tags, t_num, t_int, t_bool,
					      t_string, t_nodeRef, t_edgeRef, t_labelRef, t_composed, t_constraint, -1);
				break;

			case t_constraints:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushTags(compMode, tags, t_constraint, -1);
				break;

			case t_content:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->setIgnoreContent(true);
				break;

			case t_data:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				tag->pushTags(choiceMode, tags, t_int, t_bool, t_num, t_string, t_data, -1);
				break;

			case t_default:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				break;

			case t_edge:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_id, -1);
				tag->pushTags(choiceMode, tags, t_source, t_target, -1);
				tag->pushTags(optMode, tags, t_data, t_label, -1);
				break;

			case t_edgeRef:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_edgeIdRef, -1);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				break;

			case t_edgeStyle:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_edgeIdRef, -1);
				tag->pushTags(choiceMode, tags, t_edgeStyleTemplateRef,
					      t_line, t_sourceStyle, t_targetStyle, t_point, t_segment, -1);
				tag->pushTags(optMode, tags, t_data, -1);
				break;

			case t_edgeStyleTemplate:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_id, -1);
				tag->pushTags(choiceMode, tags, t_line, t_sourceStyle, t_targetStyle, -1);
				tag->pushTags(optMode, tags, t_data, t_edgeStyleTemplateRef, -1);
				break;

			case t_endpoint:
				tag->setMinOccurs(2);
				tag->setMaxOccurs(2);
				tag->pushAttributes(compMode, attributes, a_endpointIdRef, -1);
				tag->pushAttributes(optMode, attributes, a_type, a_color, a_size, -1);
				break;

			case t_fill:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(compMode, attributes, a_color, a_pattern, a_patternColor, -1);
				break;

			case t_font:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(compMode, attributes, a_family, -1);
				tag->pushAttributes(optMode, attributes, a_style,
						    a_variant, a_weight, a_stretch, a_size, a_color, -1);
				break;

			case t_graph:
				tag->setMinOccurs(1);
				tag->setMaxOccurs(1);
				tag->pushTags(compMode, tags, t_structure, -1);
				tag->pushTags(optMode, tags, t_layout, t_data, -1);
				break;

			case t_graphStyle:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(choiceMode, attributes,
						    a_defaultNodeTemplate,
						    a_defaultEdgeTemplate, a_defaultLabelTemplate, -1);
				break;

			case t_int:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_intValue, -1);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				break;

			case t_label:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_id, -1);
				tag->pushTags(compMode, tags, t_content, -1);
				tag->pushTags(optMode, tags, t_data, -1);
				break;

			case t_labelRef:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_labelIdRef, -1);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				break;

			case t_labelStyle:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_labelIdRef, -1);
				tag->pushTags(choiceMode, tags, t_labelStyleTemplateRef,
					      t_data, t_text, t_font, t_location, -1);
				break;

			case t_labelStyleTemplate:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_id, -1);
				tag->pushTags(compMode, tags, t_text, t_font, -1);
				tag->pushTags(optMode, tags, t_data, t_labelStyleTemplateRef, -1);
				break;

			case t_layout:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushTags(optMode, tags, t_data, t_styleTemplates, t_styles, t_constraints, -1);
				break;

			case t_line:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(choiceMode, attributes, a_nLineType, a_width, a_color, -1);
				break;

			case t_location:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(compMode, attributes, a_x, a_y, -1);
				tag->pushAttributes(optMode, attributes, a_z, -1);
				break;

			case t_node:
				tag->setMinOccurs(1);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_id, -1);
				tag->pushTags(optMode, tags, t_data, t_label, t_node, -1);
				break;

			case t_nodeRef:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_nodeIdRef, -1);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				break;

			case t_nodeStyle:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_nodeIdRef, -1);
				tag->pushTags(choiceMode, tags, t_location, t_shape, t_fill, t_line, t_image, -1);
				tag->pushTags(optMode, tags, t_data, t_nodeStyleTemplateRef, -1);
				break;


			case t_nodeStyleTemplate:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_id, -1);
				tag->pushTags(choiceMode, tags, t_shape, t_fill, t_line, -1);
				tag->pushTags(optMode, tags, t_data, t_nodeStyleTemplateRef, -1);
				break;

			case t_num:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_numValue, -1);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				break;

			case t_ogml:
				tag->setMinOccurs(1);
				tag->setMaxOccurs(1);
				tag->pushTags(compMode, tags, t_graph, -1);
				break;

			case t_point:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_id, a_x, a_y, -1);
				tag->pushAttributes(optMode, attributes, a_z, -1);
				tag->pushTags(optMode, tags, t_data, -1);
				break;

			case t_port:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_id, a_x, a_y, -1);
				break;

			case t_segment:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushTags(compMode, tags, t_endpoint, -1);
				tag->pushTags(optMode, tags, t_data, t_line, -1);
				break;

			case t_shape:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(choiceMode, attributes, a_nShapeType, a_width, a_height, /*a_uri,*/ -1);
					// comment (BZ): uri is obsolete, images got an own tag
				break;

			case t_source:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_sourceIdRef, -1);
				tag->pushAttributes(optMode, attributes, a_id, -1);
				tag->pushTags(optMode, tags, t_data, t_label, -1);
				break;

			case t_sourceStyle:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(choiceMode, attributes, a_type, a_color, a_size, -1);
				break;

			case t_string:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(optMode, attributes, a_name, -1);
				tag->setIgnoreContent(true);
				break;

			case t_structure:
				tag->setMinOccurs(1);
				tag->setMaxOccurs(1);
				tag->pushTags(compMode, tags, t_node, -1);
				tag->pushTags(optMode, tags, t_edge, t_label, t_data, -1);
				break;

			case t_styles:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushTags(choiceMode, tags, t_nodeStyle, t_edgeStyle, t_labelStyle, -1);
				tag->pushTags(optMode, tags, t_graphStyle, t_data, -1);
				break;

			case t_styleTemplates:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushTags(choiceMode, tags, t_nodeStyleTemplate,
					      t_edgeStyleTemplate, t_labelStyleTemplate, -1);
				tag->pushTags(optMode, tags, t_data, -1);
				break;

			case t_target:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(MAX_TAG_COUNT);
				tag->pushAttributes(compMode, attributes, a_targetIdRef, -1);
				tag->pushAttributes(optMode, attributes, a_id, -1);
				tag->pushTags(optMode, tags, t_data, t_label, -1);
				break;

			case t_targetStyle:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(choiceMode, attributes, a_type, a_color, a_size, -1);
				break;

			case t_labelStyleTemplateRef:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(compMode, attributes, a_labelStyleTemplateIdRef, -1);
				break;

			case t_nodeStyleTemplateRef:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(compMode, attributes, a_nodeStyleTemplateIdRef, -1);
				break;

			case t_edgeStyleTemplateRef:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(compMode, attributes, a_edgeStyleTemplateIdRef, -1);
				break;

			case t_text:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(choiceMode, attributes, a_alignment,
						    a_decoration, a_transform, a_rotation, -1);
				break;

			case t_image:
				tag->setMinOccurs(0);
				tag->setMaxOccurs(1);
				tag->pushAttributes(compMode, attributes, a_imageUri, -1);
				tag->pushAttributes(optMode, attributes, 
											a_imageStyle,
										 	a_imageAlignment, 
											a_imageDrawLine, 
											a_imageWidth, 
											a_imageHeight, -1);
				break;
			}	//switch

		}		//for*/
		hashTablesBuilt = true;
	}			//if
}

// ***********************************************************
//
// v a l i d a t e
//
// ***********************************************************
int OgmlParser::validate(const XmlTagObject * xmlTag, int ogmlTagId)
{

	OgmlTag *ogmlTag = &tags.lookup(ogmlTagId)->info();
	ListConstIterator < OgmlTag * >it;
	XmlTagObject *sonTag;
	int valid;

	//Perhaps xmlTag is already valid
	if(xmlTag->valid())
		return valid = vs_valid;

	if(!ogmlTag) {
		cerr << "Didn't found tag with id \"" << ogmlTagId << "\" in hashtable in OgmlParser::validate! Aborting.\n";
		return false;
	}

	if((valid = ogmlTag->validTag(*xmlTag, ids)) < 0) {
		this->printValidityInfo(*ogmlTag, *xmlTag, valid, __LINE__);
		return valid;
	}
	//if tag ignores its content simply return
	if(ogmlTag->ignoresContent()) {
		xmlTag->setValid();
		#ifdef OGDF_DEBUG
			this->printValidityInfo(*ogmlTag, *xmlTag, valid = vs_valid, __LINE__);
		#endif
		return valid = vs_valid;
	}

	//Check if all required son tags exist
	if(ogmlTag->ownsCompulsiveTags()) {

		//find all obligatoric sons: all obligatoric sons
		for (it = ogmlTag->getCompulsiveTags().begin(); it.valid(); it++) {

			int cnt = 0;

			//search for untested sons
			sonTag = xmlTag->m_pFirstSon;
			while(sonTag) {
				if(sonTag->getName() == (**it).getName()) {
					cnt++;
					if((valid = validate(sonTag, (**it).getId())) < 0)
						return valid;
				}
				sonTag = sonTag->m_pBrother;
			}

			//Exp. son not found
			if(cnt == 0) {
				this->printValidityInfo(*ogmlTag, *xmlTag, valid = vs_expTagNotFound, __LINE__);
				return valid;
			}
			//Check cardinality
			if(cnt < (**it).getMinOccurs() || cnt > (**it).getMaxOccurs()) {
				this->printValidityInfo((**it), *xmlTag, valid = vs_cardErr, __LINE__);
				return valid;
			}

		}

	}			//Check required son tags

	//Check if choice son tags exist
	if(ogmlTag->ownsChoiceTags()) {

		bool tookChoice = false;

		//find all obligatoric sons: all obligatoric sons
		for (it = ogmlTag->getChoiceTags().begin(); it.valid(); it++) {

			int cnt = 0;

			//search for untested sons
			sonTag = xmlTag->m_pFirstSon;
			while(sonTag) {
				if(sonTag->getName() == (**it).getName()) {
					if((valid = validate(sonTag, (**it).getId())) < 0)
						return valid;
					tookChoice = true;
					cnt++;
				}
				sonTag = sonTag->m_pBrother;
			}

			//Check cardinality
			if(cnt > 0 && (cnt < (**it).getMinOccurs()
				       || cnt > (**it).getMaxOccurs())) {
				this->printValidityInfo((**it), *xmlTag, valid = vs_cardErr, __LINE__);
				return valid;
			}

		}
		if ((!tookChoice) && (xmlTag->m_pFirstSon)) {
			this->printValidityInfo((**it), *xmlTag, valid = vs_tagEmptIncl, __LINE__);
			return valid;
		}

	}			//Check choice son tags

	//Check if opt son tags exist
	if(ogmlTag->ownsOptionalTags()) {

		//find all obligatoric sons: all obligatoric sons
		for (it = ogmlTag->getOptionalTags().begin(); it.valid(); ++it) {

			int cnt = 0;

			//search for untested sons
			sonTag = xmlTag->m_pFirstSon;
			while(sonTag) {

				if(sonTag->getName() == (**it).getName()) {
					if((valid = validate(sonTag, (**it).getId())) < 0)
						return valid;
					cnt++;
				}
				sonTag = sonTag->m_pBrother;
			}

			//Check cardinality
			//if( (cnt<(**it).getMinOccurs() || cnt>(**it).getMaxOccurs()) ) {
			if(cnt > (**it).getMaxOccurs()) {
				this->printValidityInfo((**it), *xmlTag, valid = vs_cardErr, __LINE__);
				return valid;
			}

		}
		

	}			//Check opt son tags

	//Are there invalid son tags left?
	sonTag = xmlTag->m_pFirstSon;
	while(sonTag) {
		//tag already valid
		if(!sonTag->valid()) {
			this->printValidityInfo(*ogmlTag, *xmlTag, valid = vs_unexpTag, __LINE__);
			return valid;
		}
		sonTag = sonTag->m_pBrother;
	}

	//Finally xmlTag is valid :-)
	xmlTag->setValid();
	#ifdef OGDF_DEBUG
		this->printValidityInfo(*ogmlTag, *xmlTag, valid = vs_valid, __LINE__);
	#endif

	return vs_valid;

}

//
// v a l i d a t e
//
void OgmlParser::validate(const char *fileName)
{
	DinoXmlParser p(fileName);
	p.createParseTree();
	const XmlTagObject *root = &p.getRootTag();
	OgmlParser::buildHashTables();
	validate(root, t_ogml);
}


//
// o p e r a t o r < <
//
ostream& operator<<(ostream& os, const OgmlAttribute& oa) {
				oa.print(os);
				return os;
}

//
// o p e r a t o r < <
//
ostream& operator<<(ostream& os, const OgmlTag& ot) {
				ot.printOwnedTags(os);
				ot.printOwnedAttributes(os);
				return os;
}

// ***********************************************************
//
// p r i n t V a l i d i t y I n f o
//
// ***********************************************************
void OgmlParser::printValidityInfo(const OgmlTag & ot, const XmlTagObject & xto, int valStatus, int line)
{
	String ogmlTagName = ot.getName();

	switch (valStatus) {

	case vs_tagEmptIncl:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" expects tag(s) to include! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			ot.printOwnedTags(cerr);
			break;
		}
	case vs_idNotUnique:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" owns already assigned id! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			break;
		}
	case vs_idRefErr:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" references unknown or wrong id! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			break;
		}
	case vs_unexpTag:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" owns unexpected tag! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			ot.printOwnedTags(cerr);
			break;
		}
	case vs_unexpAtt:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" owns unexpected attribute(s)! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			ot.printOwnedAttributes(cerr);
			break;
		}
	case vs_expTagNotFound:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" doesn't own compulsive tag(s)! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			ot.printOwnedTags(cerr);
			break;
		}
	case vs_expAttNotFound:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" doesn't own compulsive attribute(s)! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			ot.printOwnedAttributes(cerr);
			break;
		}
	case vs_attValueErr:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" owns attribute with wrong value! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			ot.printOwnedAttributes(cerr);
			break;
		}
	case vs_cardErr:{
			cerr << "ERROR: tag \"<" << ogmlTagName <<
				">\" occurence exceeds the number of min. (" << ot.
				getMinOccurs() << ") or max. (" << ot.getMaxOccurs() << ") occurences in its context! ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			break;
		}
	case vs_invalid:{
			cerr << "ERROR: tag \"<" << ogmlTagName << ">\" is invalid! No further information available. ";
			cerr << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			cerr << ot;
			break;
		}
	case vs_valid:{
			cout << "INFO: tag \"<" << ogmlTagName << ">\" is valid :-) ";
			cout << "(Input source line: " << xto.
				getLine() << ", recursion depth: " << xto.getDepth() << ")\n";
			break;
		}
	}
	#ifdef OGDF_DEBUG
		cout << "(Line OgmlParser::validate: " << line << ")\n";
	#endif
}			//printValidityInfo

// ***********************************************************
//
// i s G r a p h H i e r a r c h i c a l
//
// ***********************************************************
bool OgmlParser::isGraphHierarchical(const XmlTagObject *xmlTag) const
{
	bool ret = false;
	
	if(xmlTag->getName() == ogmlTagNames[t_node] &&
	   isNodeHierarchical(xmlTag)) ret = true;
			
	// Depth-Search only if ret!=true
	if(xmlTag->m_pFirstSon && 
	   !ret &&
	   isGraphHierarchical(xmlTag->m_pFirstSon)) ret = true;
	   
	// Breadth-Search only if ret!=true
	if(xmlTag->m_pBrother && 
	   !ret &&
	   isGraphHierarchical(xmlTag->m_pBrother)) ret = true;

	return ret;
};				//isGraphHierarchical

// ***********************************************************
//
// i s N o d e H i e r a r c h i c a l
//
// ***********************************************************
bool OgmlParser::isNodeHierarchical(const XmlTagObject *xmlTag) const
{
	bool ret = false;
	if(xmlTag->getName() == ogmlTagNames[t_node]) {
		
		XmlTagObject* dum;
		//check if an ancestor is a node
		ret = xmlTag->findSonXmlTagObjectByName(ogmlTagNames[t_node], dum);
	}
	return ret;
};				//isNodeHierarchical

// ***********************************************************
//
// c h e c k G r a p h T y p e
//
// ***********************************************************
bool OgmlParser::checkGraphType(const XmlTagObject *xmlTag) const {
	
	if(xmlTag->getName() != ogmlTagNames[t_ogml]) {
		cerr << "ERROR: Expecting root tag \"" << ogmlTagNames[t_ogml]	<< "\" in OgmlParser::checkGraphType!\n";
		return false;
	}
	//Normal graph present
	if(!isGraphHierarchical(xmlTag)) {
		graphType = graph;
		return true;	
	}
	//Cluster-/Compound graph present
	else {
		graphType = clusterGraph;
		
		//Traverse the parse tree and collect all edge tags
		List<const XmlTagObject*> edges;
		if(xmlTag->getName() == ogmlTagNames[t_edge]) edges.pushBack(xmlTag);
		XmlTagObject* son = xmlTag->m_pFirstSon;
		while(son) {
			if(son->getName() == ogmlTagNames[t_edge]) edges.pushBack(xmlTag);
			son = son->m_pBrother;
		}
		
		//Cluster graph already present
		if(edges.empty()) return true;
		
		//Traverse edges
		ListConstIterator<const XmlTagObject*> edgeIt;
		for(edgeIt = edges.begin(); edgeIt.valid() && graphType!=compoundGraph; edgeIt++) {
			
			//Traverse the sources/targets
			son = (*edgeIt)->m_pFirstSon;
			//Parse tree is valid so one edge contains at least one source/target 
			//with idRef attribute
			while(son) {
				XmlAttributeObject* att;
				if(son->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeIdRef], att)) {
					const XmlTagObject *refTag = ids.lookup(att->getValue())->info();
					if(isNodeHierarchical(refTag)) {
						graphType = compoundGraph;
						break;
					}
				}
				son = son->m_pBrother;
			}//while
		}//for
		
		return true;
		
	}//else	
};



// ***********************************************************
//
// a u x i l i a r y    m e t h o d s 
//
// ***********************************************************
//   => Mapping of OGML to OGDF <=


// Mapping Brush Pattern
int OgmlParser::getBrushPatternAsInt(String s){
	if (s==ogmlAttributeValueNames[av_bpNone])
		return 0;
	if (s==ogmlAttributeValueNames[av_bpSolid])
		return 1;
	if (s==ogmlAttributeValueNames[av_bpDense1])
		return 2;
	if (s==ogmlAttributeValueNames[av_bpDense2])
		return 3;
	if (s==ogmlAttributeValueNames[av_bpDense3])
		return 4;		
	if (s==ogmlAttributeValueNames[av_bpDense4])
		return 5;		
	if (s==ogmlAttributeValueNames[av_bpDense5])
		return 6;
	if (s==ogmlAttributeValueNames[av_bpDense6])
		return 7;
	if (s==ogmlAttributeValueNames[av_bpDense7])
		return 8;		
	if (s==ogmlAttributeValueNames[av_bpHorizontal])
		return 9;		
	if (s==ogmlAttributeValueNames[av_bpVertical])
		return 10;		
	if (s==ogmlAttributeValueNames[av_bpCross])
		return 11;		
	if (s==ogmlAttributeValueNames[av_bpBackwardDiagonal])
		return 12;
	if (s==ogmlAttributeValueNames[av_bpForwardDiagonal])
		return 13;		
	if (s==ogmlAttributeValueNames[av_bpDiagonalCross])
		return 14;	
	// default return bpSolid
	return 1;
}

// Mapping Shape to Integer
int OgmlParser::getShapeAsInt(String s){
//TODO: has to be completed if other shape types are implemented!!!
// SMALL PROBLEM: OGML DOESN'T SUPPORT SHAPES
// -> change XSD, if necessary

	if (s=="rect" || s=="rectangle")
		return GraphAttributes::rectangle;
	// default return rectangle
	
	return GraphAttributes::rectangle;
}

// Mapping OgmlNodeShape to OGDF::NodeTemplate 
String OgmlParser::getNodeTemplateFromOgmlValue(String s){
	// Mapping OGML-Values to ogdf
	// rect | triangle | circle | ellipse | hexagon | rhomb 
	//	    | trapeze | upTrapeze | lParallelogram | rParallelogram | pentagon 
	//		| octagon | umlClass | image
	if (s == ogmlAttributeValueNames[av_rect])
		return "ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_rectSimple])
		return "ogdf:std:rect simple";
	if (s == ogmlAttributeValueNames[av_triangle])
		s = "ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_circle])
		return "ogdf:std:ellipse";
	if (s == ogmlAttributeValueNames[av_ellipse])
		return "ogdf:std:ellipse";
	if (s == ogmlAttributeValueNames[av_hexagon])
		return "ogdf:std:hexagon";
	if (s == ogmlAttributeValueNames[av_rhomb])
		return "ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_trapeze])
		return "ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_upTrapeze])
		return "ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_lParallelogram])
		return "ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_rParallelogram])
		return "ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_pentagon])
		return "ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_octagon])
		return"ogdf:std:rect";
	if (s == ogmlAttributeValueNames[av_umlClass])
		return "ogdf:std:UML class";
	if (s == ogmlAttributeValueNames[av_image])
		return "ogdf:std:rect";
	// default
	return "ogdf:std:rect";
}

// Mapping Line type to Integer
int OgmlParser::getLineTypeAsInt(String s){
	if (s==ogmlAttributeValueNames[av_esNoPen])
		return 0;
	if (s==ogmlAttributeValueNames[av_esSolid])
		return 1;
	if (s==ogmlAttributeValueNames[av_esDash])
		return 2;
	if (s==ogmlAttributeValueNames[av_esDot])
		return 3;
	if (s==ogmlAttributeValueNames[av_esDashdot])
		return 4;		
	if (s==ogmlAttributeValueNames[av_esDashdotdot])
		return 5;
	// Mapping OGML-Values to ogdf
	// solid | dotted | dashed | double | triple 
	//		 | groove | ridge | inset | outset | none	
	if (s==ogmlAttributeValueNames[av_solid])
		return 1;
	if (s==ogmlAttributeValueNames[av_dotted])
		return 3;
	if (s==ogmlAttributeValueNames[av_dashed])
		return 2;
	if (s==ogmlAttributeValueNames[av_double])
		return 4;
	if (s==ogmlAttributeValueNames[av_triple])
		return 5;
	if (s==ogmlAttributeValueNames[av_groove])
		return 5;
	if (s==ogmlAttributeValueNames[av_ridge])
		return 1;
	if (s==ogmlAttributeValueNames[av_inset])
		return 1;
	if (s==ogmlAttributeValueNames[av_outset])
		return 1;
	if (s==ogmlAttributeValueNames[av_none])
		return 0;
	//default return bpSolid
	return 1;	
}

// Mapping ArrowStyles to Integer
int OgmlParser::getArrowStyleAsInt(String s, String sot){
	// sot = "source" or "target", actually not necessary
	// TODO: Complete, if new arrow styles are implemented in ogdf
	if (s == "none")
		return 0;
	else
		return 1;
	// default return 0
	return 0;
}

// Mapping ArrowStyles to EdgeArrow
GraphAttributes::EdgeArrow OgmlParser::getArrowStyle(int i){
	switch (i){
		case 0 : return GraphAttributes::none; break;
		case 1 : return GraphAttributes::last; break;
		case 2 : return GraphAttributes::first;break;
		case 3 : return GraphAttributes::both; break;
		default: return GraphAttributes::last; break;
	}//switch
}//getArrowStyle


// Mapping Image Style to Integer
int OgmlParser::getImageStyleAsInt(String s){
	if (s==ogmlAttributeValueNames[av_freeScale])
		return 0;
	if (s==ogmlAttributeValueNames[av_fixScale])
		return 1;
	//default return freeScale
	return 0;	
}

// Mapping Image Alignment to Integer
int OgmlParser::getImageAlignmentAsInt(String s){
	if (s==ogmlAttributeValueNames[av_topLeft])
		return 0;
	if (s==ogmlAttributeValueNames[av_topCenter])
		return 1;
	if (s==ogmlAttributeValueNames[av_topRight])
		return 2;
	if (s==ogmlAttributeValueNames[av_centerLeft])
		return 3;
	if (s==ogmlAttributeValueNames[av_center])
		return 4;		
	if (s==ogmlAttributeValueNames[av_centerRight])
		return 5;
	if (s==ogmlAttributeValueNames[av_bottomLeft])
		return 6;
	if (s==ogmlAttributeValueNames[av_bottomCenter])
		return 7;
	if (s==ogmlAttributeValueNames[av_bottomRight])
		return 8;
	//default return center
	return 4;	
}

// returns the string with "<" substituted for "&lt;"
//  and ">" substituted for "&gt;"
String OgmlParser::getLabelCaptionFromString(String str){
	String output;
	size_t i=0;
	while (i<str.length()){
	
		if (str[i] == '&'){
			if (i+3 < str.length()){
				if ((str[i+1] == 'l') && (str[i+2] == 't') && (str[i+3] == ';')){
					// found char sequence "&lt;"
					output += "<";
				}
				else
					if ((str[i+1] == 'g') && (str[i+2] == 't') && (str[i+3] == ';')){
						// found char sequence "&gt;"
						// \n newline is required!!!
						output += ">\n";

					}
				i = i + 4;
			}
		}
		else{
			char c = str[i];
			output += c;
			i++;
		}
	}
	str += "\n";
	return output;	
}


// returns the integer value of the id at the end of the string - if existent
// the return value is 'id', the boolean return value is for checking existance of an integer value
//  why do we need such a function?
//  in OGML every id is globally unique, so we write a char-prefix 
//   to the ogdf-id's ('n' for node, 'e' for edge, ...) 
bool OgmlParser::getIdFromString(String str, int id){
	if (str.length() == 0)
		return false;
	String strId;
	size_t i=0;
	while (i<str.length()){
		// if act char is a digit append it to the strId
		if (isdigit(str[i]))
			strId += str[i];
		i++;	
	}
	if (strId.length() == 0)
		return false;
	// transform str to int
	id = atoi(strId.cstr());
	// no errors occured, so return true
	return true;
}


// ***********************************************************
//
// B U I L D    A T T R I B U T E D    C L U S T E R -- G R A P H 
//
//
// ***********************************************************
bool OgmlParser::buildAttributedClusterGraph(
							Graph &G,
							ClusterGraphAttributes &CGA, 
							const XmlTagObject *root) 
{
	
	HashConstIterator<String, const XmlTagObject*> it;
	
	if(!root) {
		cout << "WARNING: can't determine layout information, no parse tree available!\n";
	}
	else {
		// root tag isn't a NULL pointer... let's start...
		XmlTagObject* son = root->m_pFirstSon;
		// first traverse to the structure- and the layout block
		if (son->getName() != ogmlTagNames[t_graph]){
			while (son->getName() != ogmlTagNames[t_graph]){
				son = son->m_pFirstSon;
				if (!son){
					// wrong rootTag given or graph tag wasn't found
					return false;
				}
			} //while
		} //if
	
		//now son is the graph tag which first child is structure
		XmlTagObject* structure = son->m_pFirstSon;
		if (structure->getName() != ogmlTagNames[t_structure]){
			return false;	
		}
		// now structure is what it is meant to be
		// traverse the children of structure
		// and set the labels
		son = structure->m_pFirstSon;
		while(son){


			//Set labels of nodes
			if ((son->getName() == ogmlTagNames[t_node]) && (CGA.attributes() & GraphAttributes::nodeLabel)){
				
				if (!isNodeHierarchical(son)){
					// get the id of the actual node
					XmlAttributeObject *att;
					if(son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
						// lookup for node
						node actNode = (m_nodes.lookup(att->getValue()))->info();
						// find label tag
						XmlTagObject* label;
						if (son->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
							// get content tag
							XmlTagObject* content = label->m_pFirstSon;
							// get the content as string
							if (content->m_pTagValue){
								String str = content->getValue();
								String labelStr = getLabelCaptionFromString(str);
								// now set the label of the node
								CGA.labelNode(actNode) = labelStr;
							}
						}
					}
				}// "normal" nodes
				else
				{
					// get the id of the actual cluster
					XmlAttributeObject *att;
					if(son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
						// lookup for cluster
						cluster actCluster = (m_clusters.lookup(att->getValue()))->info();
						// find label tag
						XmlTagObject* label;
						if (son->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
							// get content tag
							XmlTagObject* content = label->m_pFirstSon;
							// get the content as string
							if (content->m_pTagValue){
								String str = content->getValue();
								String labelStr = getLabelCaptionFromString(str);
								// now set the label of the node
								CGA.clusterLabel(actCluster) = labelStr;
							}
						}
					}
					// hierSon = hierarchical Son
					XmlTagObject *hierSon;
					if (son->m_pFirstSon){
						hierSon = son->m_pFirstSon;
						while(hierSon){
							// recursive call for setting labels of child nodes
							if (!setLabelsRecursive(G, CGA, hierSon))
								return false;
							hierSon = hierSon->m_pBrother;
						}	
					}
					
				}//cluster nodes
			}// node labels

			//Set labels of edges
			if ((son->getName() == ogmlTagNames[t_edge]) && (CGA.attributes() & GraphAttributes::edgeLabel)) {
				// get the id of the actual edge
				XmlAttributeObject *att;
				if (son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
					// lookup for edge
					edge actEdge = (m_edges.lookup(att->getValue()))->info();
					// find label tag
					XmlTagObject* label;
					if(son->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
						// get content tag
						XmlTagObject* content = label->m_pFirstSon;
						// get the content as string
						if (content->m_pTagValue){
							String str = content->getValue();
							String labelStr = getLabelCaptionFromString(str);
							// now set the label of the node
							CGA.labelEdge(actEdge) = labelStr;
						}
					}
				}
			}// edge labels

			// Labels
			// ACTUALLY NOT IMPLEMENTED IN OGDF
			//if (son->getName() == ogmlTagNames[t_label]) {
				// get the id of the actual edge
				//XmlAttributeObject *att;
				//if (son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
					// lookup for label
					//label actLabel = (labels.lookup(att->getValue()))->info();
					// get content tag
					//XmlTagObject* content = son->m_pFirstSon;
					// get the content as string
					//if (content->m_pTagValue){
					//String str = content->getValue();
					//String labelStr = getLabelCaptionFromString(str);
					//now set the label of the node
					//	CGA.labelLabel(actLabel) = labelStr;
					//}
				//}
			//}// Labels

			// go to the next brother
			son = son->m_pBrother;
		}// while(son) // son <=> children of structure

		// get the layout tag
		XmlTagObject* layout;
		if (structure->m_pBrother != NULL){
			layout = structure->m_pBrother;
		}
		if ((layout) && (layout->getName() == ogmlTagNames[t_layout])){
			// layout exists
			
			// first get the styleTemplates
			XmlTagObject *layoutSon;
			if (layout->m_pFirstSon){
				// layout has at least one child-tag
				layoutSon = layout->m_pFirstSon;
				// ->loop through all of them
				while (layoutSon){

					// style templates
					if (layoutSon->getName() == ogmlTagNames[t_styleTemplates]){
						// has children data, nodeStyleTemplate, edgeStyleTemplate, labelStyleTemplate
						XmlTagObject *styleTemplatesSon;
						if (layoutSon->m_pFirstSon){
							styleTemplatesSon = layoutSon->m_pFirstSon;
							
							while (styleTemplatesSon){
			
								// nodeStyleTemplate
								if (styleTemplatesSon->getName() == ogmlTagNames[t_nodeStyleTemplate]){
									OgmlNodeTemplate *actTemplate;
									XmlAttributeObject *actAtt;
									String actKey;

									if (styleTemplatesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
										actKey = actAtt->getValue();
										actTemplate = new OgmlNodeTemplate(actKey);
									
										XmlTagObject *actTag;

										// template inheritance
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_nodeStyleTemplateRef], actTag)){
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeStyleTemplateIdRef], actAtt)){
												// actual template references another
												// get it from the hash table
												OgmlNodeTemplate *refTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
												if (refTemplate){
													// the referenced template was inserted into the hash table
													// so copy the values
													String actId = actTemplate->m_id;
													*actTemplate = *refTemplate;
													actTemplate->m_id = actId;
												}
											}
										}// template inheritance

//										// data
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//											// found data for nodeStyleTemplate
//											// no implementation required for ogdf
//										}// data

										// shape tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_shape], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nShapeType], actAtt)){
												// TODO: change, if shapes are expanded
												// actually shape and template are calculated from the same value!!!
												actTemplate->m_nodeTemplate = getNodeTemplateFromOgmlValue(actAtt->getValue());
												actTemplate->m_shapeType = getShapeAsInt(actAtt->getValue());
											}
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												actTemplate->m_width = atof(actAtt->getValue().cstr());
											// height
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_height], actAtt))
												actTemplate->m_height = atof(actAtt->getValue().cstr());
											// uri
											//ACTUALLY NOT SUPPORTED
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_uri], actAtt))
											//	CGA.uri(actNode) = actAtt->getValue();											
										}// shape
										
										// fill tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_fill], actTag)){
											// fill color
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												actTemplate->m_color = actAtt->getValue();
											// fill pattern
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_pattern], actAtt))
												actTemplate->m_pattern = GraphAttributes::intToPattern(getBrushPatternAsInt(actAtt->getValue()));
											// fill patternColor
											//TODO: check if pattern color exists
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_patternColor], actAtt));
											//	actTemplate->m_patternColor = actAtt->getValue());
										}// fill
										
										// line tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
												actTemplate->m_lineType = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												actTemplate->m_lineWidth = atof(actAtt->getValue().cstr());
											// color
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												actTemplate->m_lineColor = actAtt->getValue();
										}// line

										//insert actual template into hash table
										m_ogmlNodeTemplates.fastInsert(actKey, actTemplate);
									}
								}//nodeStyleTemplate

								// edgeStyleTemplate
								if (styleTemplatesSon->getName() == ogmlTagNames[t_edgeStyleTemplate]){

									OgmlEdgeTemplate *actTemplate;
									XmlAttributeObject *actAtt;
									String actKey;
									// set id
									if (styleTemplatesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
										actKey = actAtt->getValue();
										actTemplate = new OgmlEdgeTemplate(actKey);
									
										XmlTagObject *actTag;									

										// template inheritance
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_edgeStyleTemplateRef], actTag)){
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeStyleTemplateIdRef], actAtt)){
												// actual template references another
												// get it from the hash table
												OgmlEdgeTemplate *refTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info();
												if (refTemplate){
													// the referenced template was inserted into the hash table
													// so copy the values
													String actId = actTemplate->m_id;
													*actTemplate = *refTemplate;
													actTemplate->m_id = actId;
												}
											}
										}// template inheritance

//										// data
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//											// found data for edgeStyleTemplate
//											// no implementation required for ogdf
//										}// data

										// line tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
												actTemplate->m_lineType = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												actTemplate->m_lineWidth = atof(actAtt->getValue().cstr());
											// color
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												actTemplate->m_color = actAtt->getValue();	
										}// line
										
										// sourceStyle tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_sourceStyle], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
												actTemplate->m_sourceType = getArrowStyleAsInt(actAtt->getValue(), ogmlTagNames[t_source]);
											// color
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
											//	actTemplate->m_sourceColor = actAtt->getValue();
											// size
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
											//	actTemplate->m_sourceSize = atof(actAtt->getValue());
										}// fill
										
										// targetStyle tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_targetStyle], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
												actTemplate->m_targetType = getArrowStyleAsInt(actAtt->getValue(), ogmlTagNames[t_target]);
											// color
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
											//	actTemplate->m_targetColor = actAtt->getValue();
											// size
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
											//	actTemplate->m_targetSize = atof(actAtt->getValue());
										}// fill

										//insert actual template into hash table
										m_ogmlEdgeTemplates.fastInsert(actKey, actTemplate);
									}	
				
								}//edgeStyleTemplate			
								
								// labelStyleTemplate
								if (styleTemplatesSon->getName() == ogmlTagNames[t_labelStyleTemplate]){
									// ACTUALLY NOT SUPPORTED
								}//labelStyleTemplate
														
								styleTemplatesSon = styleTemplatesSon->m_pBrother;	
							}
						}
					}// styleTemplates

					//STYLES
					if (layoutSon->getName() == ogmlTagNames[t_styles]){
						// has children graphStyle, nodeStyle, edgeStyle, labelStyle
						XmlTagObject *stylesSon;
						if (layoutSon->m_pFirstSon){
							stylesSon = layoutSon->m_pFirstSon;
							
							while (stylesSon){

								// GRAPHSTYLE
								if (stylesSon->getName() == ogmlTagNames[t_graphStyle]){
									XmlAttributeObject *actAtt;
									// defaultNodeTemplate
									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultNodeTemplate], actAtt)){

										OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
										
//										XmlTagObject *actTag;
//										// data
//										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//											// found data for graphStyle
//											// no implementation required for ogdf
//										}// data

										// set values for ALL nodes
										node v;
										forall_nodes(v, G){

											if (CGA.attributes() & GraphAttributes::nodeType){
												CGA.templateNode(v) = actTemplate->m_nodeTemplate;
												CGA.shapeNode(v) = actTemplate->m_shapeType;
											}
											if (CGA.attributes() & GraphAttributes::nodeGraphics){
												CGA.width(v) = actTemplate->m_width;
												CGA.height(v) = actTemplate->m_height;
											}
											if (CGA.attributes() & GraphAttributes::nodeColor)
												CGA.colorNode(v) = actTemplate->m_color;
											if (CGA.attributes() & GraphAttributes::nodeStyle){
												CGA.nodePattern(v) = actTemplate->m_pattern;
												//CGA.nodePatternColor(v) = actTemplate->m_patternColor;
												CGA.styleNode(v) = actTemplate->m_lineType;
												CGA.lineWidthNode(v) = actTemplate->m_lineWidth;
												CGA.nodeLine(v) = actTemplate->m_lineColor;
											}
										}// forall_nodes
									}// defaultNodeTemplate

//									// defaultClusterTemplate
//									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultCompoundTemplate], actAtt)){
//
//										OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
//
//										// set values for ALL Cluster
//										cluster c;
//										forall_clusters(c, G){
//											
//											if (CGA.attributes() & GraphAttributes::nodeType){
//												CGA.templateCluster(c) = actTemplate->m_nodeTemplate;
//												// no shape definition for clusters
//												//CGA.shapeNode(c) = actTemplate->m_shapeType;
//											}
//											if (CGA.attributes() & GraphAttributes::nodeGraphics){
//													CGA.clusterWidth(c) = actTemplate->m_width;
//													CGA.clusterHeight(c) = actTemplate->m_height;
//											}
//											if (CGA.attributes() & GraphAttributes::nodeColor)
//												CGA.clusterFillColor(c) = actTemplate->m_color;
//											if (CGA.attributes() & GraphAttributes::nodeStyle){
//												CGA.clusterFillPattern(c) = actTemplate->m_pattern;
//												CGA.clusterBackColor(c) = actTemplate->m_patternColor;
//												CGA.clusterLineStyle(c) = actTemplate->m_lineType;
//												CGA.clusterLineWidth(c) = actTemplate->m_lineWidth;
//												CGA.clusterColor(c) = actTemplate->m_lineColor;
//											}
//										}// forall_clusters
//									}// defaultClusterTemplate

									
									// defaultEdgeTemplate
									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultEdgeTemplate], actAtt)){

										OgmlEdgeTemplate* actTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info();

										// set values for ALL edges
										edge e;
										forall_edges(e, G){
											
												if (CGA.attributes() & GraphAttributes::edgeStyle){
													CGA.styleEdge(e) = actTemplate->m_lineType;
													CGA.edgeWidth(e) = actTemplate->m_lineWidth;
												}
												if (CGA.attributes() & GraphAttributes::edgeColor){
													CGA.colorEdge(e) = actTemplate->m_color;
												}
												
												//edgeArrow
												if ((CGA.attributes()) & (GraphAttributes::edgeArrow)){
													if (actTemplate->m_sourceType == 0){
														if (actTemplate->m_targetType == 0){
															// source = no_arrow, target = no_arrow // =>none
															CGA.arrowEdge(e) = GraphAttributes::none;
														}
														else{
															// source = no_arrow, target = arrow // =>last
															CGA.arrowEdge(e) = GraphAttributes::last;
														}
													}
													else{
														if (actTemplate->m_targetType == 0){
															// source = arrow, target = no_arrow // =>first
															CGA.arrowEdge(e) = GraphAttributes::first;
														}
														else{
															// source = arrow, target = arrow // =>both
															CGA.arrowEdge(e) = GraphAttributes::both;
														}
													}
												}//edgeArrow	
										}//forall_edges
									}//defaultEdgeTemplate

									// defaultLabelTemplate
									//if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultLabelTemplate], actAtt)){
									//	// set values for ALL labels
									//	// ACTUALLY NOT IMPLEMENTED
									//  label l;
									//  forall_labels(l, G){
									//		
									//	}
									//}//defaultLabelTemplate
								}// graphStyle

								// NODESTYLE
								if (stylesSon->getName() == ogmlTagNames[t_nodeStyle]){

									// get the id of the actual node
									XmlAttributeObject *att;
									if(stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeIdRef], att)){
											
										// check if referenced id is a node or a cluster/compound
										if (m_nodes.lookup(att->getValue())){
											
											// lookup for node
											node actNode = (m_nodes.lookup(att->getValue()))->info();

											// actTag is the actual tag that is considered
											XmlTagObject* actTag;
											XmlAttributeObject *actAtt;

//											// data
//											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//												// found data for nodeStyle
//												// no implementation required for ogdf
//											}// data

											// check if actual nodeStyle references a template
											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_nodeStyleTemplateRef], actTag)){
												// get referenced template id
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeStyleTemplateIdRef], actAtt)){
													// actual nodeStyle references a template
													OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
													if (CGA.attributes() & GraphAttributes::nodeType){
														CGA.templateNode(actNode) = actTemplate->m_nodeTemplate;
														CGA.shapeNode(actNode) = actTemplate->m_shapeType;
													}
													if (CGA.attributes() & GraphAttributes::nodeGraphics){
														CGA.width(actNode) = actTemplate->m_width;
														CGA.height(actNode) = actTemplate->m_height;
													}
													if (CGA.attributes() & GraphAttributes::nodeColor)
														CGA.colorNode(actNode) = actTemplate->m_color;
													if (CGA.attributes() & GraphAttributes::nodeStyle){
														CGA.nodePattern(actNode) = actTemplate->m_pattern;
														//CGA.nodePatternColor(actNode) = actTemplate->m_patternColor;
														CGA.styleNode(actNode) = actTemplate->m_lineType;
														CGA.lineWidthNode(actNode) = actTemplate->m_lineWidth;
														CGA.nodeLine(actNode) = actTemplate->m_lineColor;
													}										
												}
											}//template

											// Graph::nodeType
											//TODO: COMPLETE, IF NECESSARY
											CGA.type(actNode) = Graph::vertex;

											// location tag
											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_location], actTag)) 
												&& (CGA.attributes() & GraphAttributes::nodeGraphics)){
												// set location of node
												// x
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
													CGA.x(actNode) = atof(actAtt->getValue().cstr());
												// y
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_y], actAtt))
													CGA.y(actNode) = atof(actAtt->getValue().cstr());
												// z
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
													//CGA.z(actNode) = atof(actAtt->getValue());
											}// location

											// shape tag
											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_shape], actTag))
												&& (CGA.attributes() & GraphAttributes::nodeType)){
												// set shape of node
												// type
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nShapeType], actAtt)){
													CGA.templateNode(actNode) = getNodeTemplateFromOgmlValue(actAtt->getValue());
													// TODO: change, if shapes are expanded
													// actually shape and template are calculated from the same value!!!
													CGA.shapeNode(actNode) = getShapeAsInt(actAtt->getValue());
												}
												// width
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
													CGA.width(actNode) = atof(actAtt->getValue().cstr());
												// height
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_height], actAtt))
													CGA.height(actNode) = atof(actAtt->getValue().cstr());
												// uri
												//ACTUALLY NOT SUPPORTED
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_uri], actAtt))
												//	CGA.uri(actNode) = actAtt->getValue();											
											}// shape

											// fill tag
											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_fill], actTag))
												&& (CGA.attributes() & GraphAttributes::nodeStyle)){
												// fill color
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
													CGA.colorNode(actNode) = actAtt->getValue();
												// fill pattern
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_pattern], actAtt))
													CGA.nodePattern(actNode) = GraphAttributes::intToPattern(getBrushPatternAsInt(actAtt->getValue()));
												// fill patternColor
												//TODO: check if pattern color exists
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_patternColor], actAtt))
												//	CGA.nodePatternColor(actNode) = actAtt->getValue());
											}// fill

											// line tag
											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag))
												&& (CGA.attributes() & GraphAttributes::nodeStyle)){
												// type
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
													CGA.styleNode(actNode) = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
												// width
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
													CGA.lineWidthNode(actNode) = atof(actAtt->getValue().cstr());
												// color
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
													CGA.nodeLine(actNode) = actAtt->getValue().cstr();
											}// line										
											
//											// ports
//											// go through all ports with dummy tagObject port
//											XmlTagObject* port = stylesSon->m_pFirstSon;
//											while(port){
//												if (port->getName() == ogmlTagObjects[t_port]){
//													// TODO: COMPLETE
//													// ACTUALLY NOT IMPLEMENTED IN OGDF
//												}
//												
//												// go to next tag
//												port = port->m_pBrother;	
//											}

										}
										else
											
											// CLUSTER NODE STYLE
											{
											// get the id of the cluster/compound
											XmlAttributeObject *att;
											if(stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeIdRef], att)){
												
												// lookup for node
												cluster actCluster = (m_clusters.lookup(att->getValue()))->info();
												// actTag is the actual tag that is considered
												XmlTagObject* actTag;
												XmlAttributeObject *actAtt;
												
//												// data
//												if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//													// found data for nodeStyle (CLuster/Compound)
//													// no implementation required for ogdf
//												}// data
												
												// check if actual nodeStyle (equal to cluster) references a template
												if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_nodeStyleTemplateRef], actTag)){
													// get referenced template id
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeStyleTemplateIdRef], actAtt)){
														// actual nodeStyle references a template
														OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
														if (CGA.attributes() & GraphAttributes::nodeType){
															CGA.templateCluster(actCluster) = actTemplate->m_nodeTemplate;
															// no shape definition for clusters
															//CGA.shapeNode(actCluster) = actTemplate->m_shapeType;
														}
														if (CGA.attributes() & GraphAttributes::nodeGraphics){
																CGA.clusterWidth(actCluster) = actTemplate->m_width;
																CGA.clusterHeight(actCluster) = actTemplate->m_height;
														}
														if (CGA.attributes() & GraphAttributes::nodeColor)
															CGA.clusterFillColor(actCluster) = actTemplate->m_color;
														if (CGA.attributes() & GraphAttributes::nodeStyle){
															CGA.clusterFillPattern(actCluster) = actTemplate->m_pattern;
															CGA.clusterBackColor(actCluster) = actTemplate->m_patternColor;
															CGA.clusterLineStyle(actCluster) = actTemplate->m_lineType;
															CGA.clusterLineWidth(actCluster) = actTemplate->m_lineWidth;
															CGA.clusterColor(actCluster) = actTemplate->m_lineColor;
														}
													}
												}//template
		
												// Graph::nodeType
												//TODO: COMPLETE, IF NECESSARY
												// not supported for clusters!!!
												//CGA.type(actCluster) = Graph::vertex;

												// location tag
												if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_location], actTag)) 
													&& (CGA.attributes() & GraphAttributes::nodeGraphics)){
													// set location of node
													// x
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
														CGA.clusterXPos(actCluster) = atof(actAtt->getValue().cstr());
													// y
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_y], actAtt))
														CGA.clusterYPos(actCluster) = atof(actAtt->getValue().cstr());
													// z
													//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
														//CGA.clusterZPos(actCluster) = atof(actAtt->getValue());
												}// location
																						
												// shape tag
												if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_shape], actTag))
													&& (CGA.attributes() & GraphAttributes::nodeType)){
													// set shape of node
													// type
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nShapeType], actAtt)){
														CGA.templateCluster(actCluster) = getNodeTemplateFromOgmlValue(actAtt->getValue().cstr());
														// no shape definition for clusters
														//CGA.shapeNode(actCluster) = getShapeAsInt(actAtt->getValue());
													}
													// width
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
														CGA.clusterWidth(actCluster) = atof(actAtt->getValue().cstr());
													// height
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_height], actAtt))
														CGA.clusterHeight(actCluster) = atof(actAtt->getValue().cstr());
													// uri
													//ACTUALLY NOT SUPPORTED
													//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_uri], actAtt))
													//	CGA.uriCluster(actCluster) = actAtt->getValue();											
												}// shape
												
												// fill tag
												if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_fill], actTag))
													&& (CGA.attributes() & GraphAttributes::nodeStyle)){
													// fill color
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
														CGA.clusterFillColor(actCluster) = actAtt->getValue().cstr();
													// fill pattern
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_pattern], actAtt))
														CGA.clusterFillPattern(actCluster) = GraphAttributes::intToPattern(getBrushPatternAsInt(actAtt->getValue()));
													// fill patternColor
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_patternColor], actAtt))
														CGA.clusterBackColor(actCluster) = actAtt->getValue().cstr();
												}// fill
												
												// line tag
												if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag))
													&& (CGA.attributes() & GraphAttributes::nodeStyle)){
													// type
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
														CGA.clusterLineStyle(actCluster) = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
													// width
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
														CGA.clusterLineWidth(actCluster) = atof(actAtt->getValue().cstr());
													// color
													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
														CGA.clusterColor(actCluster) = actAtt->getValue();
												}// line										
												

//												// ports
//												// go through all ports with dummy tagObject port
//												XmlTagObject* port = stylesSon->m_pFirstSon;
//												while(port){
//													if (port->getName() == ogmlTagObjects[t_port]){
//														// TODO: COMPLETE
//														// no implementation required for ogdf
//													}
//													
//													// go to next tag
//													port = port->m_pBrother;	
//												}
												
											}//nodeIdRef (with cluster)
											
											}// nodeStyle for cluster					
										}//nodeIdRef
										
								}//nodeStyle

								// EDGESTYLE
								if (stylesSon->getName() == ogmlTagNames[t_edgeStyle]){
									
									// get the id of the actual edge
									XmlAttributeObject *att;		
									if(stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeIdRef], att)){
										// lookup for edge
										edge actEdge = (m_edges.lookup(att->getValue()))->info();
										
										// actTag is the actual tag that is considered
										XmlTagObject* actTag;
										XmlAttributeObject *actAtt;

//										// data
//										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//											// found data for edgeStyle
//											// no implementation required for ogdf
//										}// data

										// check if actual edgeStyle references a template
										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_edgeStyleTemplateRef], actTag)){
											// get referenced template id
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeStyleTemplateIdRef], actAtt)){
												// actual edgeStyle references a template
												OgmlEdgeTemplate* actTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info();
												if (CGA.attributes() & GraphAttributes::edgeStyle){
													CGA.styleEdge(actEdge) = actTemplate->m_lineType;
													CGA.edgeWidth(actEdge) = actTemplate->m_lineWidth;
												}
												if (CGA.attributes() & GraphAttributes::edgeColor){
													CGA.colorEdge(actEdge) = actTemplate->m_color;
												}
												
												//edgeArrow
												if ((CGA.attributes()) & (GraphAttributes::edgeArrow)){
													if (actTemplate->m_sourceType == 0){
														if (actTemplate->m_targetType == 0){
															// source = no_arrow, target = no_arrow // =>none
															CGA.arrowEdge(actEdge) = GraphAttributes::none;
														}
														else{
															// source = no_arrow, target = arrow // =>last
															CGA.arrowEdge(actEdge) = GraphAttributes::last;
														}
													}
													else{
														if (actTemplate->m_targetType == 0){
															// source = arrow, target = no_arrow // =>first
															CGA.arrowEdge(actEdge) = GraphAttributes::first;
														}
														else{
															// source = arrow, target = arrow // =>both
															CGA.arrowEdge(actEdge) = GraphAttributes::both;
														}
													}
												}//edgeArrow

											}
										}//template
										
										// Graph::edgeType
										//TODO: COMPLETE, IF NECESSARY
										CGA.type(actEdge) = Graph::association;
										
										// line tag
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag))
											&& (CGA.attributes() & GraphAttributes::edgeType)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
												CGA.styleEdge(actEdge) = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												CGA.edgeWidth(actEdge) = atof(actAtt->getValue().cstr());
											// color
											if ((actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												&& (CGA.attributes() & GraphAttributes::edgeType))
												CGA.colorEdge(actEdge) = actAtt->getValue();
										}// line

										// mapping of arrows
										if (CGA.attributes() & GraphAttributes::edgeArrow){
											
											// values for mapping edge arrows to GDE
											// init to -1 for a simple check
											int sourceInt = -1;
											int targetInt = -1;

											// sourceStyle tag
											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_sourceStyle], actTag)){
												// type
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
													sourceInt = getArrowStyleAsInt((actAtt->getValue()), ogmlAttributeNames[t_source]);
												// color
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												//	;
												// size
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
												//	;
											}// sourceStyle

											// targetStyle tag
											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_targetStyle], actTag)){
												// type
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
													targetInt = getArrowStyleAsInt((actAtt->getValue()), ogmlAttributeNames[t_target]);
												// color
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												//	;
												// size
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
												//	;
											}// targetStyle
											
											// map edge arrows
											if ((sourceInt != -1) || (targetInt != -1)){
												if (sourceInt <= 0){
													if (targetInt <= 0){
														//source=no arrow, target=no arrow // => none
														CGA.arrowEdge(actEdge) = GraphAttributes::none;
													}
													else{
														// source=no arrow, target=arrow // => last
														CGA.arrowEdge(actEdge) = GraphAttributes::last;
													}
												}
												else{
													if (targetInt <= 0){
														//source=arrow, target=no arrow // => first
														CGA.arrowEdge(actEdge) = GraphAttributes::first;
													}
													else{
														//source=target=arrow // => both
														CGA.arrowEdge(actEdge) = GraphAttributes::both;
													}
												}
											}
										}//arrow
										
										// points & segments
										// bool value for checking if segments exist
										bool segmentsExist = stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_segment], actTag);
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_point], actTag))
											&& (CGA.attributes() & GraphAttributes::edgeGraphics)){
											// at least one point exists
											XmlTagObject *pointTag = stylesSon->m_pFirstSon;
											DPolyline dpl;
											dpl.clear();
											// traverse all points in the order given in the ogml file
											while (pointTag){
												if (pointTag->getName() == ogmlTagNames[t_point]){
													
													if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
														DPoint dp;
														// here we have a point
														if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt)){
															dp.m_x = atof(actAtt->getValue().cstr());
														}
														if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_y], actAtt)){
															dp.m_y = atof(actAtt->getValue().cstr());
														}
														//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_z], actAtt))
														//	dp.m_z = atof(actAtt->getValue());
														// insert point into hash table
														pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt);
														points.fastInsert(actAtt->getValue(), dp);
														//insert point into polyline
														if (!segmentsExist)
															dpl.pushBack(dp);
													}
												}
												// go to next tag
												pointTag = pointTag->m_pBrother;	
											}// while (pointTag)
											//concatenate polyline
											if (!segmentsExist){
												CGA.bends(actEdge).conc(dpl);
											}
											else{
												// work with segments
												// one error can occur:
												// if a segments is going to be inserted,
												// which doesn't match with any other,
												// the order can be not correct at the end
												// then the edge is relly corrupted!!
												
												// TODO: this implementation doesn't work with hyperedges
												//       cause hyperedges have more than one source/target
												
												// segmentsUnsorted stores all found segments
												List<OgmlSegment> segmentsUnsorted;
												XmlTagObject *segmentTag = stylesSon->m_pFirstSon;
												while (segmentTag){
													if (segmentTag->getName() == ogmlTagNames[t_segment]){
														XmlTagObject *endpointTag = segmentTag->m_pFirstSon;
														OgmlSegment actSeg;
														int endpointsSet = 0;
														while ((endpointTag) && (endpointsSet <2)){
															if (endpointTag->getName() == ogmlTagNames[t_endpoint]){
																// get the referenced point
																endpointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_endpointIdRef], actAtt);
																DPoint dp = (points.lookup(actAtt->getValue()))->info();

																if (endpointsSet == 0)
																	actSeg.point1 = dp;
																else
																	actSeg.point2 = dp;
																endpointsSet++;
															}
															endpointTag = endpointTag->m_pBrother;	
														}// while
														// now we created a segment
														// we can insert this easily into in segmentsUnsorted														
														if (actSeg.point1 != actSeg.point2){
															segmentsUnsorted.pushBack(actSeg);
														} // point1 != point2
													}// if (segment)
													// go to next tag
													segmentTag = segmentTag->m_pBrother;
												}// while (segmentTag)
												// now are the segments stored in the segmentsUnsorted list
												//  but we have to sort it in segments list while inserting
												List<OgmlSegment> segments;
												ListIterator<OgmlSegment> segIt;
												// check the number of re-insertions
												int checkNumOfSegReInserts = segmentsUnsorted.size()+2;
												while ((segmentsUnsorted.size() > 0) && (checkNumOfSegReInserts > 0)){
													OgmlSegment actSeg = segmentsUnsorted.front();
													segmentsUnsorted.popFront();
													// actSeg has to be inserted in correct order
													//  and then being deleted
													//  OR waiting in list until it can be inserted
													// size == 0 => insert
													if (segments.size() == 0){
														segments.pushFront(actSeg);
													}
													else{
														// segments contains >1 segment
														segIt = segments.begin();
														bool inserted = false;
														while (segIt.valid() && !inserted){
															if ((actSeg.point1 == (*segIt).point1) ||
															    (actSeg.point1 == (*segIt).point2) ||
															    (actSeg.point2 == (*segIt).point1) ||
															    (actSeg.point2 == (*segIt).point2)){							
																	// found two matching segments
																	// now we can insert
																	// there are some cases to check
																	if (actSeg.point1 == (*segIt).point1){
																		DPoint dumP = actSeg.point1;
																		actSeg.point1 = actSeg.point2;
																		actSeg.point2 = dumP;
																		segments.insertBefore(actSeg, segIt);
																	}
																	else
																		if (actSeg.point2 == (*segIt).point1){
																			segments.insertBefore(actSeg, segIt);
																		}
																		else
																			if ((actSeg.point2 == (*segIt).point2)){
																				DPoint dumP = actSeg.point1;
																				actSeg.point1 = actSeg.point2;
																				actSeg.point2 = dumP;
																				segments.insertAfter(actSeg, segIt);	
																			}
																			else{
																				segments.insertAfter(actSeg, segIt);
																			}
																	inserted = true;
															    } // first if
															segIt++;
														} //while
														if (!inserted){
															// segment doesn't found matching segment,
															//  so insert it again into unsorted segments list
															//  so it will be inserted later
															segmentsUnsorted.pushBack(actSeg);
															checkNumOfSegReInserts--;
														}
													}//else
												}//while segmentsUnsorted.size() > 0


												if (checkNumOfSegReInserts==0){
													cout << "WARNING! Segment definition is not correct" << endl << flush;
													cout << "Not able to work with #"<< segmentsUnsorted.size() << " segments" << endl << flush;
													cout << "Please check connection and sorting of segments!" << endl << flush;
//													// inserting the bends although there might be an error
//													// I commented this, because in this case in ogdf the edge will 
//													//   be a straight edge and there will not be any artefacts
//													// TODO: uncomment if desired
// 													for (segIt = segments.begin(); segIt.valid(); segIt++){
//														dpl.pushBack((*segIt).point1);
//														dpl.pushBack((*segIt).point2);
												}
												else{
													// the segments are now ordered (perhaps in wrong way)...
													// so we have to check if the first and last point
													//  are graphically laying in the source- and target- node
													bool invertSegments = false;
													segIt = segments.begin();
													node target = actEdge->target();
													node source = actEdge->source();
													// check if source is a normal node or a cluster
													//if (...){
													
													//}
													//else{
														// big if-check: if (first point is in target
														//                   and not in source)
														//                   AND 
														//                   (last point is in source
														//                   and not in target)
														if (( ( (CGA.x(target) + CGA.width(target))>= (*segIt).point1.m_x )
														  &&   (CGA.x(target)                      <= (*segIt).point1.m_x )
													      && ( (CGA.y(target) + CGA.height(target))>= (*segIt).point1.m_y )
													      &&   (CGA.y(target)                      <= (*segIt).point1.m_y ) )
													      &&
													      (!( ( (CGA.x(source) + CGA.width(source))>= (*segIt).point1.m_x )
														  &&   (CGA.x(source)                      <= (*segIt).point1.m_x )
													      && ( (CGA.y(source) + CGA.height(source))>= (*segIt).point1.m_y )
													      &&   (CGA.y(source)                      <= (*segIt).point1.m_y ) )))
													      {
													      	segIt = segments.rbegin();
															if (( ( (CGA.x(source) + CGA.width(source))>= (*segIt).point2.m_x )
															  &&   (CGA.x(source)                      <= (*segIt).point2.m_x )
														      && ( (CGA.y(source) + CGA.height(source))>= (*segIt).point2.m_y )
														      &&   (CGA.y(source)                      <= (*segIt).point2.m_y ) )
														      &&
														      (!( ( (CGA.x(target) + CGA.width(source))>= (*segIt).point2.m_x )
															  &&   (CGA.x(target)                      <= (*segIt).point2.m_x )
														      && ( (CGA.y(target) + CGA.height(source))>= (*segIt).point2.m_y )
														      &&   (CGA.y(target)                      <= (*segIt).point2.m_y ) ))){
														      	// invert the segment-line
														      	invertSegments = true;
														      }
													      }
													//}
													if (!invertSegments){
	 													for (segIt = segments.begin(); segIt.valid(); segIt++){
															dpl.pushBack((*segIt).point1);
															dpl.pushBack((*segIt).point2);
														}
													}
													else{
	 													for (segIt = segments.rbegin(); segIt.valid(); segIt--){
															dpl.pushBack((*segIt).point2);
															dpl.pushBack((*segIt).point1);
														}
													}
													// unify bends = delete superfluous points
													dpl.unify();
													// finally concatenate/set the bends
													CGA.bends(actEdge).conc(dpl);
												}// else (checkNumOfSegReInserts==0)												
											}// else (segments exist)
										}// points & segments
																				
									}//edgeIdRef
									
								}// edgeStyle

//								// LABELSTYLE
//								if (stylesSon->getName() == ogmlTagNames[t_labelStyle]){
//									// labelStyle
//									// ACTUALLY NOT SUPPORTED
//								}// labelStyle
														
								stylesSon = stylesSon->m_pBrother;	
							} // while
							
						}
				} //styles

				// CONSTRAINTS
				if (layoutSon->getName() == ogmlTagNames[t_constraints]){

					// this code is encapsulated in the method
					// OgmlParser::buildConstraints
					// has to be called by read methods after building

					// here we only set the pointer,
					//  so we don't have to traverse the parse tree
					//  to the constraints tag later
					m_constraintsTag = layoutSon;
		
				}// constraints


				// go to next brother
				layoutSon = layoutSon->m_pBrother;
				}// while(layoutSon)
			}//if (layout->m_pFirstSon)
		}// if ((layout) && (layout->getName() == ogmlTagNames[t_layout]))
				
		
	}// else			



			

//	cout << "buildAttributedClusterGraph COMPLETE. Check... " << endl << flush;
//	edge e;
//	forall_edges(e, G){
//		//cout << "CGA.labelEdge" << e << " = " << CGA.labelEdge(e) << endl << flush;
//		cout << "CGA.arrowEdge" << e << " = " << CGA.arrowEdge(e) << endl << flush;
//		cout << "CGA.styleEdge" << e << " = " << CGA.styleEdge(e) << endl << flush;
//		cout << "CGA.edgeWidth" << e << " = " << CGA.edgeWidth(e) << endl << flush;
//		cout << "CGA.colorEdge" << e << " = " << CGA.colorEdge(e) << endl << flush;			
//		cout << "CGA.type     " << e << " = " << CGA.type(e) << endl << flush;	
//		ListConstIterator<DPoint> it;
//		for(it = CGA.bends(e).begin(); it!=CGA.bends(e).end(); ++it) {
//			cout << "point " << " x=" << (*it).m_x << " y=" << (*it).m_y << endl << flush;
//		}
//				
//	}
//
//	node n;
//	forall_nodes(n, G){
//		cout << "CGA.labelNode(" << n << ")     = " << CGA.labelNode(n) << endl << flush;
//		cout << "CGA.templateNode(" << n << ")  = " << CGA.templateNode(n) << endl << flush;
//		cout << "CGA.shapeNode(" << n << ")     = " << CGA.shapeNode(n) << endl << flush;
//		cout << "CGA.width(" << n << ")         = " << CGA.width(n) << endl << flush;
//		cout << "CGA.height(" << n << ")        = " << CGA.height(n) << endl << flush;
//		cout << "CGA.colorNode(" << n << ")     = " << CGA.colorNode(n) << endl << flush;
//		cout << "CGA.nodePattern(" << n << ")   = " << CGA.nodePattern(n) << endl << flush;
//		cout << "CGA.styleNode(" << n << ")     = " << CGA.styleNode(n) << endl << flush;
//		cout << "CGA.lineWidthNode(" << n << ") = " << CGA.lineWidthNode(n) << endl << flush;
//		cout << "CGA.nodeLine(" << n << ")      = " << CGA.nodeLine(n) << endl << flush;
//		cout << "CGA.x(" << n << ")             = " << CGA.x(n) << endl << flush;
//		cout << "CGA.y(" << n << ")             = " << CGA.y(n) << endl << flush;
//		cout << "CGA.type(" << n << ")          = " << CGA.type(n) << endl << flush;
//	}
//	
//	cluster c;
//	forall_clusters(c, CGA.constClusterGraph()){
//		cout << "CGA.templateCluster(" << c << ")    = " << CGA.templateCluster(c) << endl << flush;
//		cout << "CGA.clusterWidth(" << c << ")       = " << CGA.clusterWidth(c) << endl << flush;
//		cout << "CGA.clusterHeight(" << c << ")      = " << CGA.clusterHeight(c) << endl << flush;
//		cout << "CGA.clusterFillColor(" << c << ")   = " << CGA.clusterFillColor(c) << endl << flush;
//		cout << "CGA.clusterFillPattern(" << c << ") = " << CGA.clusterFillPattern(c) << endl << flush;
//		cout << "CGA.clusterBackColor(" << c << ")   = " << CGA.clusterBackColor(c) << endl << flush;
//		cout << "CGA.clusterLineStyle(" << c << ")   = " << CGA.clusterLineStyle(c) << endl << flush;
//		cout << "CGA.clusterLineWidth(" << c << ")   = " << CGA.clusterLineWidth(c) << endl << flush;
//		cout << "CGA.clusterColor(" << c << ")       = " << CGA.clusterColor(c) << endl << flush;
//		cout << "CGA.clusterXPos(" << c << ")        = " << CGA.clusterXPos(c) << endl << flush;
//		cout << "CGA.clusterYPos(" << c << ")        = " << CGA.clusterYPos(c) << endl << flush;
//	}
	
//	cout << "buildAttributedClusterGraph COMPLETE... Check COMPLETE... Let's have fun in GDE ;) " << endl << flush;

	// building terminated, so return true
	return true;
	
}//buildAttributedClusterGraph



// ***********************************************************
//
// s e t    l a b e l s    r e c u r s i v e     f o r     c l u s t e r s 
//
// ***********************************************************
// sets the labels of hierarchical nodes => cluster
bool OgmlParser::setLabelsRecursive(Graph &G, ClusterGraphAttributes &CGA, XmlTagObject *root){
	if ((root->getName() == ogmlTagNames[t_node]) && (CGA.attributes() & GraphAttributes::nodeLabel)){
		if (!isNodeHierarchical(root)){
			// get the id of the actual node
			XmlAttributeObject *att;
			if(root->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
				// lookup for node
				node actNode = (m_nodes.lookup(att->getValue()))->info();
				// find label tag
				XmlTagObject* label;
				if (root->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
					// get content tag
					XmlTagObject* content = label->m_pFirstSon;
					// get the content as string
					if (content->m_pTagValue){
						String str = content->getValue();
						String labelStr = getLabelCaptionFromString(str);
						// now set the label of the node
						CGA.labelNode(actNode) = labelStr;
					}
				}
			}
		}// "normal" nodes
		else
		{
			// get the id of the actual cluster
			XmlAttributeObject *att;
			if(root->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
				// lookup for cluster
				cluster actCluster = (m_clusters.lookup(att->getValue()))->info();
				// find label tag
				XmlTagObject* label;
				if (root->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
					// get content tag
					XmlTagObject* content = label->m_pFirstSon;
					// get the content as string
					if (content->m_pTagValue){
						String str = content->getValue();
						String labelStr = getLabelCaptionFromString(str);
						// now set the label of the node
						CGA.clusterLabel(actCluster) = labelStr;
					}
				}
			}
			// hierSon = hierarchical Son
			XmlTagObject *hierSon;
			if (root->m_pFirstSon){
				hierSon = root->m_pFirstSon;
				while(hierSon){
					// recursive call for setting labels of child nodes
					if (!setLabelsRecursive(G, CGA, hierSon))
						return false;
					hierSon = hierSon->m_pBrother;
				}	
			}
			
		}//cluster nodes
	}
	return true;
};// setLabelsRecursive




// ***********************************************************
//
// s e t    l a b e l s    r e c u r s i v e     f o r     c o m p o u n d s 
//
// ***********************************************************
// sets the labels of hierarchical nodes => compounds
//Commented out due to missing compound graph in OGDF
/*
bool OgmlParser::setLabelsRecursiveForCompounds(Graph &G, CompoundGraphAttributes &CGA, XmlTagObject *root){
	if ((root->getName() == ogmlTagNames[t_node]) && (CGA.attributes() & GraphAttributes::nodeLabel)){
		// get the id of the actual node
		XmlAttributeObject *att;
		if(root->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
			// lookup for node
			node actNode = (m_nodes.lookup(att->getValue()))->info();
			// find label tag
			XmlTagObject* label;
			if (root->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
				// get content tag
				XmlTagObject* content = label->m_pFirstSon;
				// get the content as string
				if (content->m_pTagValue){
					String str = content->getValue();
					String labelStr = getLabelCaptionFromString(str);
					// now set the label of the node
					CGA.labelNode(actNode) = labelStr;
				}
			}
			if (isNodeHierarchical(root)){
				// hierSon = hierarchical Son
				XmlTagObject *hierSon;
				if (root->m_pFirstSon){
					hierSon = root->m_pFirstSon;
					while(hierSon){
						// recursive call for setting labels of child nodes
						if (!setLabelsRecursiveForCompounds(G, CGA, hierSon))
							return false;
						hierSon = hierSon->m_pBrother;
					}	
				}
			}//compound nodes
		}
	}

	return true;
};// setLabelsRecursiveForCompounds
*/


// ***********************************************************
//
// b u i l d     g r a p h 
//
// ***********************************************************
bool OgmlParser::buildGraph(Graph &G) {
	
	G.clear();
	
	int id = 0;
	
	//Build nodes first
	HashConstIterator<String, const XmlTagObject*> it;
	
	for(it = ids.begin(); it.valid(); ++it) {
		if( it.info()->getName() == ogmlTagNames[t_node] && !isNodeHierarchical(it.info())) {
			// get id string from xmlTag
			XmlAttributeObject *idAtt;
			if ( (it.info())->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], idAtt) 
			  && (getIdFromString(idAtt->getValue(), id)) ){
				// now we got an id from the id-string
				// we have to check, if this id was assigned
				if (m_nodeIds.lookup(id)){
					// new id was assigned to another node
					id = G.maxNodeIndex() + 1;
				}
			}
			else{
				// default id setting
				id = G.maxNodeIndex() + 1;	
			}
			m_nodes.fastInsert(it.key(), G.newNode(id));
			m_nodeIds.fastInsert(id, idAtt->getValue());			
		}
	}//for nodes
	
	id = 0;
	
	//Build edges second
	for(it = ids.begin(); it.valid(); ++it) {
		if( it.info()->getName() == ogmlTagNames[t_edge] ) {
			
			//Check sources/targets
			Stack<node> srcTgt;
			const XmlTagObject* son = it.info()->m_pFirstSon;
			while(son) {
				if( son->getName() == ogmlTagNames[t_source] ||
				    son->getName() == ogmlTagNames[t_target] ) {
				  	XmlAttributeObject *att;
				  	son->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeIdRef], att);
				  	//Validate if source/target is really a node
				  	if(ids.lookup(att->getValue())->info()->getName() != ogmlTagNames[t_node]) {
				  		cout << "WARNING: edge relation between graph elements of none type node " <<
				  		        "are temporarily not supported!\n";
				  	}
				    else {
				    	srcTgt.push(m_nodes.lookup(att->getValue())->info());	
				    }
				 }
				 son = son->m_pBrother;
			}
			if(srcTgt.size() != 2) {
				cout << "WARNING: hyperedges are temporarily not supported! Discarding edge.\n";	
			}
			else{
				// create edge
				
				// get id string from xmlTag
				XmlAttributeObject *idAtt;
				if ( (it.info())->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], idAtt) 
				  && (getIdFromString(idAtt->getValue(), id)) ){
					if (m_edgeIds.lookup(id)){
						// new id was assigned to another edge
						id = G.maxEdgeIndex() + 1;
					}
				}
				else{
					// default id setting
					id = G.maxEdgeIndex() + 1;
				}
				m_edges.fastInsert(it.key(), G.newEdge(srcTgt.pop(), srcTgt.pop(), id));
				m_edgeIds.fastInsert(id, idAtt->getValue());
			}
		}
	}//for edges
	
	//Structure data determined, so building the graph was successfull.
	return true;
};//buildGraph



// ***********************************************************
//
// b u i l d    c l u s t e r -- g r a p h 
//
// ***********************************************************
bool OgmlParser::buildClusterRecursive(
						const XmlTagObject *xmlTag, 
						cluster parent, 
						Graph &G,
						ClusterGraph &CG){
	// create new cluster
	
	// first get the id
	int id = -1;
	
	XmlAttributeObject *idAtt;
	if (  (xmlTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], idAtt))
	  && (getIdFromString(idAtt->getValue(), id)) ){	
		if (m_clusterIds.lookup(id)){
			// id was assigned to another cluster
			id = CG.maxClusterIndex() + 1;
		}
	}
	else{
		// default id setting
		id = CG.maxClusterIndex() + 1;	
	}
	// create cluster and insert into hash tables
	cluster actCluster = CG.newCluster(parent, id);
	m_clusters.fastInsert(idAtt->getValue(), actCluster);
	m_clusterIds.fastInsert(id, idAtt->getValue());
		
	// check children of cluster tag
	XmlTagObject *son = xmlTag->m_pFirstSon;

	while (son) {
		if (son->getName() == ogmlTagNames[t_node]){
			if (isNodeHierarchical(son))
					// recursive call
					buildClusterRecursive(son, actCluster, G, CG);
			else {
				// the actual node tag is a child of the cluster
				XmlAttributeObject *att;				
				//parse tree is valid so tag owns id attribute
				son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att);
				// get node from lookup table with the id in att
				node v = m_nodes.lookup(att->getValue())->info();
				// assign node to actual cluster
				CG.reassignNode(v, actCluster);
			}
		}
		
		son = son->m_pBrother;
	}//while
	
	return true;
}//buildClusterRecursive


bool OgmlParser::buildCluster(
					const XmlTagObject *rootTag, 
					Graph &G, 
					ClusterGraph &CG){
	CG.clear();
	CG.init(G);
	
	if(rootTag->getName() != ogmlTagNames[t_ogml]) {
		cerr << "ERROR: Expecting root tag \"" << ogmlTagNames[t_ogml]	<< "\" in OgmlParser::buildCluster!\n";
		return false;
	}
	
	//Search for first node tag
	XmlTagObject *nodeTag;
	rootTag->findSonXmlTagObjectByName(ogmlTagNames[t_graph], nodeTag);
	nodeTag->findSonXmlTagObjectByName(ogmlTagNames[t_structure], nodeTag);
	nodeTag->findSonXmlTagObjectByName(ogmlTagNames[t_node], nodeTag);

	while (nodeTag) {
		if(nodeTag->getName() == ogmlTagNames[t_node] && isNodeHierarchical(nodeTag)) {
			if (!buildClusterRecursive(nodeTag, CG.rootCluster(), G, CG))
				return false;
		}
		
		nodeTag = nodeTag->m_pBrother;
	}
	
	return true;
};//buildCluster




// INFO: this is the "old" version for building attributed compound graphs
//       In the new implementation of compounds every node has a corresponding
//		 compound and backwards. So every check whether a node is hierarchical
//		 (= is a compound) or not is superfluous
// (BZ:) I commented the code and do not delete it because maybe in later 
// 		 implementations this has to be distinguished
//       I also stopped updating this method and made new changes
//       only in the new, following method!
// ***********************************************************
//
// B U I L D    A T T R I B U T E D    C O M P O U N D -- G R A P H 
//
//                 O L D     V E R S I O N !!!! 
//
// ***********************************************************
//bool OgmlParser::buildAttributedCompoundGraph(
//							Graph &G,
//							CompoundGraphAttributes &CGA, 
//							XmlTagObject *root) 
//{
//
//
//	HashConstIterator<String, const XmlTagObject*> it;
//	
//	if(!root) {
//		cout << "WARNING: can't determine layout information, no parse tree available!\n";
//	}
//	else {
//		// root tag isn't a NULL pointer... let's start...
//		XmlTagObject* son = root->m_pFirstSon;
//		// first traverse to the structure- and the layout block
//		if (son->getName() != ogmlTagNames[t_graph]){
//			while (son->getName() != ogmlTagNames[t_graph]){
//				son = son->m_pFirstSon;
//				if (!son){
//					// wrong rootTag given or graph tag wasn't found
//					return false;
//				}
//			} //while
//		} //if
//	
//		//now son is the graph tag which first child is structure
//		XmlTagObject* structure = son->m_pFirstSon;
//		if (structure->getName() != ogmlTagNames[t_structure]){
//			return false;	
//		}
//		// now structure is what it is meant to be
//		// traverse the children of structure
//		// and set the labels
//		son = structure->m_pFirstSon;
//		while(son){
//
//			//Set labels of nodes
//			if ((son->getName() == ogmlTagNames[t_node]) && (CGA.attributes() & GraphAttributes::nodeLabel)){
//				
//				if (!isNodeHierarchical(son)){
//					// get the id of the actual node
//					XmlAttributeObject *att;
//					if(son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
//						// lookup for node
//						node actNode = (m_nodes.lookup(att->getValue()))->info();
//						// find label tag
//						XmlTagObject* label;
//						if (son->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
//							// get content tag
//							XmlTagObject* content = label->m_pFirstSon;
//							// get the content as string
//							if (content->m_pTagValue){
//								String str = content->getValue();
//								String labelStr = getLabelCaptionFromString(str);
//								// now set the label of the node
//								CGA.labelNode(actNode) = labelStr;
//							}
//						}
//					}
//				}// "normal" nodes
//				else
//				{
//					// get the id of the actual compound
//					XmlAttributeObject *att;
//					if(son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
//						// lookup for compound
//						compound actCompound = (m_compounds.lookup(att->getValue()))->info();
//						// find label tag
//						XmlTagObject* label;
//						if (son->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
//							// get content tag
//							XmlTagObject* content = label->m_pFirstSon;
//							// get the content as string
//							if (content->m_pTagValue){
//								String str = content->getValue();
//								String labelStr = getLabelCaptionFromString(str);
//								// now set the label of the node
//								CGA.compoundLabel(actCompound) = labelStr;
//							}
//						}
//					}
//					// hierSon = hierarchical Son
//					XmlTagObject *hierSon;
//					if (son->m_pFirstSon){
//						hierSon = son->m_pFirstSon;
//						while(hierSon){
//							// recursive call for setting labels of child nodes
//							if (!setLabelsRecursiveForCompounds(G, CGA, hierSon))
//								return false;
//							hierSon = hierSon->m_pBrother;
//						}	
//					}
//					
//				}//compound nodes
//			}// node labels
//
//			//Set labels of edges
//			if ((son->getName() == ogmlTagNames[t_edge]) && (CGA.attributes() & GraphAttributes::edgeLabel)) {
//				// get the id of the actual edge
//				XmlAttributeObject *att;
//				if (son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
//					// lookup for edge
//					edge actEdge = (m_edges.lookup(att->getValue()))->info();
//					// find label tag
//					XmlTagObject* label;
//					if(son->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
//						// get content tag
//						XmlTagObject* content = label->m_pFirstSon;
//						// get the content as string
//						if (content->m_pTagValue){
//							String str = content->getValue();
//							String labelStr = getLabelCaptionFromString(str);
//							// now set the label of the node
//							CGA.labelEdge(actEdge) = labelStr;
//						}
//					}
//				}
//			}// edge labels
//
//			// Labels
//			// ACTUALLY NOT IMPLEMENTED IN OGDF
//			//if (son->getName() == ogmlTagNames[t_label]) {
//				// get the id of the actual edge
//				//XmlAttributeObject *att;
//				//if (son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
//					// lookup for label
//					//label actLabel = (labels.lookup(att->getValue()))->info();
//					// get content tag
//					//XmlTagObject* content = son->m_pFirstSon;
//					// get the content as string
//					//if (content->m_pTagValue){
//					//String str = content->getValue();
//					//String labelStr = getLabelCaptionFromString(str);
//					//now set the label of the node
//					//	CGA.labelLabel(actLabel) = labelStr;
//					//}
//				//}
//			//}// Labels
//
//			// go to the next brother
//			son = son->m_pBrother;
//		}// while(son) // son <=> children of structure
//
//		// get the layout tag
//		XmlTagObject* layout = NULL;
//		if (structure->m_pBrother != NULL){
//			layout = structure->m_pBrother;
//		}
//		if ((layout) && (layout->getName() == ogmlTagNames[t_layout])){
//			// layout exists
//			
//			// first get the styleTemplates
//			XmlTagObject *layoutSon;
//			if (layout->m_pFirstSon){
//				// layout has at least one child-tag
//				layoutSon = layout->m_pFirstSon;
//				// ->loop through all of them
//				while (layoutSon){
//
//					// style templates
//					if (layoutSon->getName() == ogmlTagNames[t_styleTemplates]){
//						// has children data, nodeStyleTemplate, edgeStyleTemplate, labelStyleTemplate
//						XmlTagObject *styleTemplatesSon;
//						if (layoutSon->m_pFirstSon){
//							styleTemplatesSon = layoutSon->m_pFirstSon;
//							
//							while (styleTemplatesSon){
//			
//								// nodeStyleTemplate
//								if (styleTemplatesSon->getName() == ogmlTagNames[t_nodeStyleTemplate]){
//									OgmlNodeTemplate *actTemplate;
//									XmlAttributeObject *actAtt;
//									String actKey;
//
//									if (styleTemplatesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
//										actKey = actAtt->getValue();
//										actTemplate = new OgmlNodeTemplate(actKey);
//									
//										XmlTagObject *actTag;
//
//										// template inheritance
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_nodeStyleTemplateRef], actTag)){
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeStyleTemplateIdRef], actAtt)){
//												// actual template references another
//												// get it from the hash table
//												OgmlNodeTemplate *refTemplate;
//												if (refTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info()){
//													// the referenced template was inserted into the hash table
//													// so copy the values
//													String actId = actTemplate->m_id;
//													*actTemplate = *refTemplate;
//													actTemplate->m_id = actId;
//												}
//											}
//										}// template inheritance
//
////										// data
////										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
////											// found data for nodeStyleTemplate
////											// no implementation required for ogdf
////										}// data
//
//										// shape tag
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_shape], actTag)){
//											// type
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nShapeType], actAtt)){
//												// TODO: change, if shapes are expanded
//												// actually shape and template are calculated from the same value!!!
//												actTemplate->m_nodeTemplate = getNodeTemplateFromOgmlValue(actAtt->getValue());
//												actTemplate->m_shapeType = getShapeAsInt(actAtt->getValue());
//											}
//											// width
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
//												actTemplate->m_width = atof(actAtt->getValue());
//											// height
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_height], actAtt))
//												actTemplate->m_height = atof(actAtt->getValue());
//											// uri
//											//ACTUALLY NOT SUPPORTED
//											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_uri], actAtt))
//											//	CGA.uri(actNode) = actAtt->getValue();											
//										}// shape
//										
//										// fill tag
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_fill], actTag)){
//											// fill color
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//												actTemplate->m_color = actAtt->getValue();
//											// fill pattern
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_pattern], actAtt))
//												actTemplate->m_pattern = GraphAttributes::intToPattern(getBrushPatternAsInt(actAtt->getValue()));
//											// fill patternColor
//											//TODO: check if pattern color exists
//											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_patternColor], actAtt))
//											//	actTemplate->m_patternColor = actAtt->getValue());
//										}// fill
//										
//										// line tag
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag)){
//											// type
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
//												actTemplate->m_lineType = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
//											// width
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
//												actTemplate->m_lineWidth = atof(actAtt->getValue());
//											// color
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//												actTemplate->m_lineColor = actAtt->getValue();
//										}// line
//
//										//insert actual template into hash table
//										m_ogmlNodeTemplates.fastInsert(actKey, actTemplate);
//									}
//								}//nodeStyleTemplate
//
//								// edgeStyleTemplate
//								if (styleTemplatesSon->getName() == ogmlTagNames[t_edgeStyleTemplate]){
//
//									OgmlEdgeTemplate *actTemplate;
//									XmlAttributeObject *actAtt;
//									String actKey;
//									// set id
//									if (styleTemplatesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
//										actKey = actAtt->getValue();
//										actTemplate = new OgmlEdgeTemplate(actKey);
//									
//										XmlTagObject *actTag;									
//
//										// template inheritance
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_edgeStyleTemplateRef], actTag)){
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeStyleTemplateIdRef], actAtt)){
//												// actual template references another
//												// get it from the hash table
//												OgmlEdgeTemplate *refTemplate;
//												if (refTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info()){
//													// the referenced template was inserted into the hash table
//													// so copy the values
//													String actId = actTemplate->m_id;
//													*actTemplate = *refTemplate;
//													actTemplate->m_id = actId;
//												}
//											}
//										}// template inheritance
//
////										// data
////										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
////											// found data for edgeStyleTemplate
////											// no implementation required for ogdf
////										}// data
//
//										// line tag
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag)){
//											// type
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
//												actTemplate->m_lineType = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
//											// width
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
//												actTemplate->m_lineWidth = atof(actAtt->getValue());
//											// color
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//												actTemplate->m_color = actAtt->getValue();	
//										}// line
//										
//										// sourceStyle tag
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_sourceStyle], actTag)){
//											// type
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
//												actTemplate->m_sourceType = getArrowStyleAsInt(actAtt->getValue(), ogmlTagNames[t_source]);
//											// color
//											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//											//	actTemplate->m_sourceColor = actAtt->getValue();
//											// size
//											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
//											//	actTemplate->m_sourceSize = atof(actAtt->getValue());
//										}// fill
//										
//										// targetStyle tag
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_targetStyle], actTag)){
//											// type
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
//												actTemplate->m_targetType = getArrowStyleAsInt(actAtt->getValue(), ogmlTagNames[t_target]);
//											// color
//											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//											//	actTemplate->m_targetColor = actAtt->getValue();
//											// size
//											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
//											//	actTemplate->m_targetSize = atof(actAtt->getValue());
//										}// fill
//
//										//insert actual template into hash table
//										m_ogmlEdgeTemplates.fastInsert(actKey, actTemplate);
//									}	
//				
//								}//edgeStyleTemplate			
//								
//								// labelStyleTemplate
//								if (styleTemplatesSon->getName() == ogmlTagNames[t_labelStyleTemplate]){
//									// ACTUALLY NOT SUPPORTED
//								}//labelStyleTemplate
//														
//								styleTemplatesSon = styleTemplatesSon->m_pBrother;	
//							}
//						}
//					}// styleTemplates
//
//					//STYLES
//					if (layoutSon->getName() == ogmlTagNames[t_styles]){
//						// has children graphStyle, nodeStyle, edgeStyle, labelStyle
//						XmlTagObject *stylesSon;
//						if (layoutSon->m_pFirstSon){
//							stylesSon = layoutSon->m_pFirstSon;
//							
//							while (stylesSon){
//
//								// GRAPHSTYLE
//								if (stylesSon->getName() == ogmlTagNames[t_graphStyle]){
//									XmlAttributeObject *actAtt;
//									// defaultNodeTemplate
//									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultNodeTemplate], actAtt)){
//
//										OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
//										
////										XmlTagObject *actTag;
////										// data
////										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
////											// found data for graphStyle
////											// no implementation required for ogdf
////										}// data
//
//										// set values for ALL nodes
//										node v;
//										forall_nodes(v, G){
//
//											if (CGA.attributes() & GraphAttributes::nodeType){
//												CGA.templateNode(v) = actTemplate->m_nodeTemplate;
//												CGA.shapeNode(v) = actTemplate->m_shapeType;
//											}
//											if (CGA.attributes() & GraphAttributes::nodeGraphics){
//												CGA.width(v) = actTemplate->m_width;
//												CGA.height(v) = actTemplate->m_height;
//											}
//											if (CGA.attributes() & GraphAttributes::nodeColor)
//												CGA.colorNode(v) = actTemplate->m_color;
//											if (CGA.attributes() & GraphAttributes::nodeStyle){
//												CGA.nodePattern(v) = actTemplate->m_pattern;
//												//CGA.nodePatternColor(v) = actTemplate->m_patternColor;
//												CGA.styleNode(v) = actTemplate->m_lineType;
//												CGA.lineWidthNode(v) = actTemplate->m_lineWidth;
//												CGA.nodeLine(v) = actTemplate->m_lineColor;
//											}
//										}// forall_nodes
//									}// defaultNodeTemplate
//
////									// defaultCompoundTemplate
////									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultCompoundTemplate], actAtt)){
////
////										OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
////
////										// set values for ALL Compounds
////										compound c;
////										forall_compounds(c, G){
////											
////											if (CGA.attributes() & CompoundGraphAttributes::nodeType){
////												CGA.templateCompound(c) = actTemplate->m_nodeTemplate;
////												// no shape definition for compounds
////												//CGA.shapeNode(c) = actTemplate->m_shapeType;
////											}
////											if (CGA.attributes() & CompoundGraphAttributes::nodeGraphics){
////													CGA.compoundWidth(c) = actTemplate->m_width;
////													CGA.compoundHeight(c) = actTemplate->m_height;
////											}
////											if (CGA.attributes() & CompoundGraphAttributes::nodeColor)
////												CGA.compoundFillColor(c) = actTemplate->m_color;
////											if (CGA.attributes() & CompoundGraphAttributes::nodeStyle){
////												CGA.compoundFillPattern(c) = actTemplate->m_pattern;
////												CGA.compoundBackColor(c) = actTemplate->m_patternColor;
////												CGA.compoundLineStyle(c) = actTemplate->m_lineType;
////												CGA.compoundLineWidth(c) = actTemplate->m_lineWidth;
////												CGA.compoundColor(c) = actTemplate->m_lineColor;
////											}
////										}// forall_compounds
////									}// defaultCompoundTemplate
//
//									
//									// defaultEdgeTemplate
//									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultEdgeTemplate], actAtt)){
//
//										OgmlEdgeTemplate* actTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info();
//
//										// set values for ALL edges
//										edge e;
//										forall_edges(e, G){
//											
//												if (CGA.attributes() & GraphAttributes::edgeStyle){
//													CGA.styleEdge(e) = actTemplate->m_lineType;
//													CGA.edgeWidth(e) = actTemplate->m_lineWidth;
//												}
//												if (CGA.attributes() & GraphAttributes::edgeColor){
//													CGA.colorEdge(e) = actTemplate->m_color;
//												}
//												
//												//edgeArrow
//												if ((CGA.attributes()) & (GraphAttributes::edgeArrow)){
//													if (actTemplate->m_sourceType == 0){
//														if (actTemplate->m_targetType == 0){
//															// source = no_arrow, target = no_arrow // =>none
//															CGA.arrowEdge(e) = GraphAttributes::none;
//														}
//														else{
//															// source = no_arrow, target = arrow // =>last
//															CGA.arrowEdge(e) = GraphAttributes::last;
//														}
//													}
//													else{
//														if (actTemplate->m_targetType == 0){
//															// source = arrow, target = no_arrow // =>first
//															CGA.arrowEdge(e) = GraphAttributes::first;
//														}
//														else{
//															// source = arrow, target = arrow // =>both
//															CGA.arrowEdge(e) = GraphAttributes::both;
//														}
//													}
//												}//edgeArrow	
//										}//forall_edges
//									}//defaultEdgeTemplate
//
//									// defaultLabelTemplate
//									//if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultLabelTemplate], actAtt)){
//									//	// set values for ALL labels
//									//	// ACTUALLY NOT IMPLEMENTED
//									//  label l;
//									//  forall_labels(l, G){
//									//		
//									//	}
//									//}//defaultLabelTemplate
//								}// graphStyle
//
//								// NODESTYLE
//								if (stylesSon->getName() == ogmlTagNames[t_nodeStyle]){
//
//									// get the id of the actual node
//									XmlAttributeObject *att;
//									if(stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeIdRef], att)){
//											
//										// check if referenced id is a node or a cluster/compound
//										if ((m_nodes.lookup(att->getValue())) && (!m_compounds.lookup(att->getValue()))){
//											
//											// lookup for node
//											node actNode = (m_nodes.lookup(att->getValue()))->info();
//
//											// actTag is the actual tag that is considered
//											XmlTagObject* actTag;
//											XmlAttributeObject *actAtt;
//
////											// data
////											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
////												// found data for nodeStyle
////												// no implementation required for ogdf
////											}// data
//
//											// check if actual nodeStyle references a template
//											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_nodeStyleTemplateRef], actTag)){
//												// get referenced template id
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeStyleTemplateIdRef], actAtt)){
//													// actual nodeStyle references a template
//													OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
//													if (CGA.attributes() & GraphAttributes::nodeType){
//														CGA.templateNode(actNode) = actTemplate->m_nodeTemplate;
//														CGA.shapeNode(actNode) = actTemplate->m_shapeType;
//													}
//													if (CGA.attributes() & GraphAttributes::nodeGraphics){
//														CGA.width(actNode) = actTemplate->m_width;
//														CGA.height(actNode) = actTemplate->m_height;
//													}
//													if (CGA.attributes() & GraphAttributes::nodeColor)
//														CGA.colorNode(actNode) = actTemplate->m_color;
//													if (CGA.attributes() & GraphAttributes::nodeStyle){
//														CGA.nodePattern(actNode) = actTemplate->m_pattern;
//														//CGA.nodePatternColor(actNode) = actTemplate->m_patternColor;
//														CGA.styleNode(actNode) = actTemplate->m_lineType;
//														CGA.lineWidthNode(actNode) = actTemplate->m_lineWidth;
//														CGA.nodeLine(actNode) = actTemplate->m_lineColor;
//													}										
//												}
//											}//template
//
//											// Graph::nodeType
//											//TODO: COMPLETE, IF NECESSARY
//											CGA.type(actNode) = Graph::vertex;
//											
//											// location tag
//											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_location], actTag)) 
//												&& (CGA.attributes() & GraphAttributes::nodeGraphics)){
//												// set location of node
//												// x
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
//													CGA.x(actNode) = atof(actAtt->getValue());
//												// y
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_y], actAtt))
//													CGA.y(actNode) = atof(actAtt->getValue());
//												// z
//												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
//													//CGA.z(actNode) = atof(actAtt->getValue());
//											}// location
//
//											// shape tag
//											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_shape], actTag))
//												&& (CGA.attributes() & GraphAttributes::nodeType)){
//												// set shape of node
//												// type
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nShapeType], actAtt)){
//													CGA.templateNode(actNode) = getNodeTemplateFromOgmlValue(actAtt->getValue());
//													// TODO: change, if shapes are expanded
//													// actually shape and template are calculated from the same value!!!
//													CGA.shapeNode(actNode) = getShapeAsInt(actAtt->getValue());
//												}
//												// width
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
//													CGA.width(actNode) = atof(actAtt->getValue());
//												// height
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_height], actAtt))
//													CGA.height(actNode) = atof(actAtt->getValue());
//												// uri
//												//ACTUALLY NOT SUPPORTED
//												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_uri], actAtt))
//												//	CGA.uri(actNode) = actAtt->getValue();											
//											}// shape
//
//											// fill tag
//											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_fill], actTag))
//												&& (CGA.attributes() & GraphAttributes::nodeStyle)){
//												// fill color
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//													CGA.colorNode(actNode) = actAtt->getValue();
//												// fill pattern
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_pattern], actAtt))
//													CGA.nodePattern(actNode) = GraphAttributes::intToPattern(getBrushPatternAsInt(actAtt->getValue()));
//												// fill patternColor
//												//TODO: check if pattern color exists
//												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_patternColor], actAtt))
//												//	CGA.nodePatternColor(actNode) = actAtt->getValue());
//											}// fill
//
//											// line tag
//											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag))
//												&& (CGA.attributes() & GraphAttributes::nodeStyle)){
//												// type
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
//													CGA.styleNode(actNode) = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
//												// width
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
//													CGA.lineWidthNode(actNode) = atof(actAtt->getValue());
//												// color
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//													CGA.nodeLine(actNode) = actAtt->getValue();
//											}// line
//											
//											// image tag
//											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_image], actTag))
//												&& (CGA.attributes() & GraphAttributes::nodeStyle)){
//												// uri
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageUri], actAtt))
//													CGA.imageUriNode(actNode) = actAtt->getValue();
//												// style
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageStyle], actAtt))
//													CGA.imageStyleNode(actNode) = GraphAttributes::intToImageStyle(getImageStyleAsInt(actAtt->getValue()));
//												// alignment
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageAlignment], actAtt))
//													CGA.imageAlignmentNode(actNode) = GraphAttributes::intToImageAlignment(getImageAlignmentAsInt(actAtt->getValue()));
//												// drawLine
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageDrawLine], actAtt)){
//													if ((actAtt->getValue() == "true") || (actAtt->getValue() == "1"))
//														CGA.imageDrawLineNode(actNode) = true;
//													else
//														CGA.imageDrawLineNode(actNode) = false;
//												}
//												// width
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageWidth], actAtt))
//													CGA.imageWidthNode(actNode) = atof(actAtt->getValue());
//												// height
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageHeight], actAtt))
//													CGA.imageHeightNode(actNode) = atof(actAtt->getValue());
//											}// image
//											
////											// ports
////											// go through all ports with dummy tagObject port
////											XmlTagObject* port = stylesSon->m_pFirstSon;
////											while(port){
////												if (port->getName() == ogmlTagObjects[t_port]){
////													// TODO: COMPLETE
////													// ACTUALLY NOT IMPLEMENTED IN OGDF
////												}
////												
////												// go to next tag
////												port = port->m_pBrother;	
////											}
//
//										}
//										else
//											
//											// COMPOUND NODE STYLE
//											{
//											// get the id of the cluster/compound
//											XmlAttributeObject *att;
//											if(stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeIdRef], att)){
//	
//												// lookup for node
//												compound actCompound = (m_compounds.lookup(att->getValue()))->info();
//												// actTag is the actual tag that is considered
//												XmlTagObject* actTag;
//												XmlAttributeObject *actAtt;
//												
////												// data
////												if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
////													// found data for nodeStyle (Compound)
////													// no implementation required for ogdf
////												}// data
//												
//												// check if actual nodeStyle (equal to compound) references a template
//												if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_nodeStyleTemplateRef], actTag)){
//													// get referenced template id
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeStyleTemplateIdRef], actAtt)){
//														// actual nodeStyle references a template
//														OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
//														if (CGA.attributes() & GraphAttributes::nodeType){
//															CGA.templateCompound(actCompound) = actTemplate->m_nodeTemplate;
//															// no shape definition for compounds
//															//  so set shape for the node
//															CGA.shapeNode(actCompound->getNode()) = actTemplate->m_shapeType;
//														}
//														if (CGA.attributes() & GraphAttributes::nodeGraphics){
//																CGA.compoundWidth(actCompound) = actTemplate->m_width;
//																CGA.compoundHeight(actCompound) = actTemplate->m_height;
//														}
//														if (CGA.attributes() & GraphAttributes::nodeColor)
//															CGA.compoundFillColor(actCompound) = actTemplate->m_color;
//														if (CGA.attributes() & GraphAttributes::nodeStyle){
//															CGA.compoundFillPattern(actCompound) = actTemplate->m_pattern;
//															CGA.compoundBackColor(actCompound) = actTemplate->m_patternColor;
//															CGA.compoundLineStyle(actCompound) = actTemplate->m_lineType;
//															CGA.compoundLineWidth(actCompound) = actTemplate->m_lineWidth;
//															CGA.compoundColor(actCompound) = actTemplate->m_lineColor;
//														}
//													}
//												}//template
//		
//
//												// Graph::nodeType
//												//TODO: COMPLETE, IF NECESSARY
//												// not supported for compounds!!!
//												//CGA.type(actCluster) = Graph::vertex;
//
//
//												// location tag
//												if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_location], actTag)) 
//													&& (CGA.attributes() & GraphAttributes::nodeGraphics)){
//													// set location of node
//													// x
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
//														CGA.compoundXPos(actCompound) = atof(actAtt->getValue());
//													// y
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_y], actAtt))
//														CGA.compoundYPos(actCompound) = atof(actAtt->getValue());
//													// z
//													//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
//														//CGA.compoundZPos(actCluster) = atof(actAtt->getValue());
//												}// location
//																						
//												// shape tag
//												if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_shape], actTag))
//													&& (CGA.attributes() & GraphAttributes::nodeType)){
//													// set shape of node
//													// type
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nShapeType], actAtt)){
//														CGA.templateCompound(actCompound) = getNodeTemplateFromOgmlValue(actAtt->getValue());
//														// no shape definition for compounds
//														// so set the shape for the node
//														CGA.shapeNode(actCompound->getNode()) = getShapeAsInt(actAtt->getValue());
//													}
//													// width
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
//														CGA.compoundWidth(actCompound) = atof(actAtt->getValue());
//													// height
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_height], actAtt))
//														CGA.compoundHeight(actCompound) = atof(actAtt->getValue());
//													// uri
//													//ACTUALLY NOT SUPPORTED
//													//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_uri], actAtt))
//													//	CGA.uriCompound(actCompound) = actAtt->getValue();											
//												}// shape
//												
//												// fill tag
//												if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_fill], actTag))
//													&& (CGA.attributes() & GraphAttributes::nodeStyle)){
//													// fill color
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//														CGA.compoundFillColor(actCompound) = actAtt->getValue();
//													// fill pattern
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_pattern], actAtt))
//														CGA.compoundFillPattern(actCompound) = GraphAttributes::intToPattern(getBrushPatternAsInt(actAtt->getValue()));
//													// fill patternColor
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_patternColor], actAtt))
//														CGA.compoundBackColor(actCompound) = actAtt->getValue();
//												}// fill
//												
//												// line tag
//												if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag))
//													&& (CGA.attributes() & GraphAttributes::nodeStyle)){
//													// type
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
//														CGA.compoundLineStyle(actCompound) = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
//													// width
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
//														CGA.compoundLineWidth(actCompound) = atof(actAtt->getValue());
//													// color
//													if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//														CGA.compoundColor(actCompound) = actAtt->getValue();
//												}// line										
//												
//											// image tag
//											// images are only implemented in GraphAttributes,
//											//  not in CompoundGraphAttributes
//											//  so we have to map image-information to the correspondign node
//											if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_image], actTag))
//												&& (CGA.attributes() & GraphAttributes::nodeStyle)){
//												// uri
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageUri], actAtt))
//													CGA.imageUriNode(actCompound->getNode()) = actAtt->getValue();
//												// style
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageStyle], actAtt))
//													CGA.imageStyleNode(actCompound->getNode()) = GraphAttributes::intToImageStyle(getImageStyleAsInt(actAtt->getValue()));
//												// alignment
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageAlignment], actAtt))
//													CGA.imageAlignmentNode(actCompound->getNode()) = GraphAttributes::intToImageAlignment(getImageAlignmentAsInt(actAtt->getValue()));
//												// drawLine
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageAlignment], actAtt)){
//													if ((actAtt->getValue() == "true") || (actAtt->getValue() == "1"))
//														CGA.imageDrawLineNode(actCompound->getNode()) = true;
//													else
//														CGA.imageDrawLineNode(actCompound->getNode()) = false;
//												}
//												// width
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageWidth], actAtt))
//													CGA.imageWidthNode(actCompound->getNode()) = atof(actAtt->getValue());
//												// height
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageHeight], actAtt))
//													CGA.imageHeightNode(actCompound->getNode()) = atof(actAtt->getValue());
//											}// image
//
////												// ports
////												// go through all ports with dummy tagObject port
////												XmlTagObject* port = stylesSon->m_pFirstSon;
////												while(port){
////													if (port->getName() == ogmlTagObjects[t_port]){
////														// TODO: COMPLETE
////														// no implementation required for ogdf
////													}
////													
////													// go to next tag
////													port = port->m_pBrother;	
////												}
//												
//											}//nodeIdRef (with cluster)
//											
//											}// nodeStyle for cluster					
//										}//nodeIdRef
//										
//								}//nodeStyle
//
//								// EDGESTYLE
//								if (stylesSon->getName() == ogmlTagNames[t_edgeStyle]){
//									
//									// get the id of the actual edge
//									XmlAttributeObject *att;		
//									if(stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeIdRef], att)){
//										// lookup for edge
//										edge actEdge = (m_edges.lookup(att->getValue()))->info();
//										
//										// actTag is the actual tag that is considered
//										XmlTagObject* actTag;
//										XmlAttributeObject *actAtt;
//
////										// data
////										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
////											// found data for edgeStyle
////											// no implementation required for ogdf
////										}// data
//
//										// check if actual edgeStyle references a template
//										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_edgeStyleTemplateRef], actTag)){
//											// get referenced template id
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeStyleTemplateIdRef], actAtt)){
//												// actual edgeStyle references a template
//												OgmlEdgeTemplate* actTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info();
//												if (CGA.attributes() & GraphAttributes::edgeStyle){
//													CGA.styleEdge(actEdge) = actTemplate->m_lineType;
//													CGA.edgeWidth(actEdge) = actTemplate->m_lineWidth;
//												}
//												if (CGA.attributes() & GraphAttributes::edgeColor){
//													CGA.colorEdge(actEdge) = actTemplate->m_color;
//												}
//												
//												//edgeArrow
//												if ((CGA.attributes()) & (GraphAttributes::edgeArrow)){
//													if (actTemplate->m_sourceType == 0){
//														if (actTemplate->m_targetType == 0){
//															// source = no_arrow, target = no_arrow // =>none
//															CGA.arrowEdge(actEdge) = GraphAttributes::none;
//														}
//														else{
//															// source = no_arrow, target = arrow // =>last
//															CGA.arrowEdge(actEdge) = GraphAttributes::last;
//														}
//													}
//													else{
//														if (actTemplate->m_targetType == 0){
//															// source = arrow, target = no_arrow // =>first
//															CGA.arrowEdge(actEdge) = GraphAttributes::first;
//														}
//														else{
//															// source = arrow, target = arrow // =>both
//															CGA.arrowEdge(actEdge) = GraphAttributes::both;
//														}
//													}
//												}//edgeArrow
//
//											}
//										}//template
//										
//										// Graph::edgeType
//										//TODO: COMPLETE, IF NECESSARY
//										CGA.type(actEdge) = Graph::association;
//										
//										// line tag
//										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag))
//											&& (CGA.attributes() & GraphAttributes::edgeType)){
//											// type
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
//												CGA.styleEdge(actEdge) = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
//											// width
//											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
//												CGA.edgeWidth(actEdge) = atof(actAtt->getValue());
//											// color
//											if ((actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//												&& (CGA.attributes() & GraphAttributes::edgeType))
//												CGA.colorEdge(actEdge) = actAtt->getValue();
//										}// line
//
//										// mapping of arrows
//										if (CGA.attributes() & GraphAttributes::edgeArrow){
//											
//											// values for mapping edge arrows to GDE
//											// init to -1 for a simple check
//											int sourceInt = -1;
//											int targetInt = -1;
//
//											// sourceStyle tag
//											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_sourceStyle], actTag)){
//												// type
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
//													sourceInt = getArrowStyleAsInt((actAtt->getValue()), ogmlAttributeNames[t_source]);
//												// color
//												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//												//	;
//												// size
//												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
//												//	;
//											}// sourceStyle
//
//											// targetStyle tag
//											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_targetStyle], actTag)){
//												// type
//												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
//													targetInt = getArrowStyleAsInt((actAtt->getValue()), ogmlAttributeNames[t_target]);
//												// color
//												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
//												//	;
//												// size
//												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
//												//	;
//											}// targetStyle
//											
//											// map edge arrows
//											if ((sourceInt != -1) || (targetInt != -1)){
//												if (sourceInt <= 0){
//													if (targetInt <= 0){
//														//source=no arrow, target=no arrow // => none
//														CGA.arrowEdge(actEdge) = GraphAttributes::none;
//													}
//													else{
//														// source=no arrow, target=arrow // => last
//														CGA.arrowEdge(actEdge) = GraphAttributes::last;
//													}
//												}
//												else{
//													if (targetInt <= 0){
//														//source=arrow, target=no arrow // => first
//														CGA.arrowEdge(actEdge) = GraphAttributes::first;
//													}
//													else{
//														//source=target=arrow // => both
//														CGA.arrowEdge(actEdge) = GraphAttributes::both;
//													}
//												}
//											}
//										}//arrow
//										
//										// points & segments
//										// bool value for checking if segments exist
//										bool segmentsExist = stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_segment], actTag);
//										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_point], actTag))
//											&& (CGA.attributes() & GraphAttributes::edgeGraphics)){
//											// at least one point exists
//											XmlTagObject *pointTag = stylesSon->m_pFirstSon;
//											DPolyline dpl;
//											dpl.clear();
//											// traverse all points in the order given in the ogml file
//											while (pointTag){
//												if (pointTag->getName() == ogmlTagNames[t_point]){
//													
//													if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
//														DPoint dp;
//														// here we have a point
//														if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt)){
//															dp.m_x = atof(actAtt->getValue());
//														}
//														if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_y], actAtt)){
//															dp.m_y = atof(actAtt->getValue());
//														}
//														//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_z], actAtt))
//														//	dp.m_z = atof(actAtt->getValue());
//														// insert point into hash table
//														pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt);
//														points.fastInsert(actAtt->getValue(), dp);
//														//insert point into polyline
//														if (!segmentsExist)
//															dpl.pushBack(dp);
//													}
//												}
//												// go to next tag
//												pointTag = pointTag->m_pBrother;	
//											}// while (pointTag)
//											//concatenate polyline
//											if (!segmentsExist){
//												CGA.bends(actEdge).conc(dpl);
//											}
//											else{
//												// work with segments
//												// one error can occur:
//												// if a segments is going to be inserted,
//												// which doesn't match with any other,
//												// the order can be not correct at the end
//												// then the edge is really corrupted!!
//												
//												// TODO: this implementation doesn't work with hyperedges
//												//       cause hyperedges have more than one source/target
//												
//												// segmentsUnsorted stores all found segments
//												List<OgmlSegment> segmentsUnsorted;
//												XmlTagObject *segmentTag = stylesSon->m_pFirstSon;
//												while (segmentTag){
//													if (segmentTag->getName() == ogmlTagNames[t_segment]){
//														XmlTagObject *endpointTag = segmentTag->m_pFirstSon;
//														OgmlSegment actSeg;
//														int endpointsSet = 0;
//														while ((endpointTag) && (endpointsSet <2)){
//															if (endpointTag->getName() == ogmlTagNames[t_endpoint]){
//																// get the referenced point
//																endpointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_endpointIdRef], actAtt);
//																DPoint dp = (points.lookup(actAtt->getValue()))->info();
//
//																if (endpointsSet == 0)
//																	actSeg.point1 = dp;
//																else
//																	actSeg.point2 = dp;
//																endpointsSet++;
//															}
//															endpointTag = endpointTag->m_pBrother;	
//														}// while
//														// now we created a segment
//														// we can insert this easily into in segmentsUnsorted														
//														if (actSeg.point1 != actSeg.point2){
//															segmentsUnsorted.pushBack(actSeg);
//														} // point1 != point2
//													}// if (segment)
//													// go to next tag
//													segmentTag = segmentTag->m_pBrother;
//												}// while (segmentTag)
//												// now are the segments stored in the segmentsUnsorted list
//												//  but we have to sort it in segments list while inserting
//												List<OgmlSegment> segments;
//												ListIterator<OgmlSegment> segIt;
//												// check the number of re-insertions
//												int checkNumOfSegReInserts = segmentsUnsorted.size()+2;
//												while ((segmentsUnsorted.size() > 0) && (checkNumOfSegReInserts > 0)){
//													OgmlSegment actSeg = segmentsUnsorted.front();
//													segmentsUnsorted.popFront();
//													// actSeg has to be inserted in correct order
//													//  and then being deleted
//													//  OR waiting in list until it can be inserted
//													// size == 0 => insert
//													if (segments.size() == 0){
//														segments.pushFront(actSeg);
//													}
//													else{
//														// segments contains >1 segment
//														segIt = segments.begin();
//														bool inserted = false;
//														while (segIt.valid() && !inserted){
//															if ((actSeg.point1 == (*segIt).point1) ||
//															    (actSeg.point1 == (*segIt).point2) ||
//															    (actSeg.point2 == (*segIt).point1) ||
//															    (actSeg.point2 == (*segIt).point2)){							
//																	// found two matching segments
//																	// now we can insert
//																	// there are some cases to check
//																	if (actSeg.point1 == (*segIt).point1){
//																		DPoint dumP = actSeg.point1;
//																		actSeg.point1 = actSeg.point2;
//																		actSeg.point2 = dumP;
//																		segments.insertBefore(actSeg, segIt);
//																	}
//																	else
//																		if (actSeg.point2 == (*segIt).point1){
//																			segments.insertBefore(actSeg, segIt);
//																		}
//																		else
//																			if ((actSeg.point2 == (*segIt).point2)){
//																				DPoint dumP = actSeg.point1;
//																				actSeg.point1 = actSeg.point2;
//																				actSeg.point2 = dumP;
//																				segments.insertAfter(actSeg, segIt);	
//																			}
//																			else{
//																				segments.insertAfter(actSeg, segIt);
//																			}
//																	inserted = true;
//															    } // first if
//															segIt++;
//														} //while
//														if (!inserted){
//															// segment doesn't found matching segment,
//															//  so insert it again into unsorted segments list
//															//  so it will be inserted later
//															segmentsUnsorted.pushBack(actSeg);
//															checkNumOfSegReInserts--;
//														}
//													}//else
//												}//while segmentsUnsorted.size() > 0
//
//
//												if (checkNumOfSegReInserts==0){
//													cout << "WARNING! Segment definition is not correct" << endl << flush;
//													cout << "Not able to work with #"<< segmentsUnsorted.size() << " segments" << endl << flush;
//													cout << "Please check connection and sorting of segments!" << endl << flush;
////													// inserting the bends although there might be an error
////													// I commented this, because in this case in ogdf the edge will 
////													//   be a straight edge and there will not be any artefacts
////													// TODO: uncomment if desired
//// 													for (segIt = segments.begin(); segIt.valid(); segIt++){
////														dpl.pushBack((*segIt).point1);
////														dpl.pushBack((*segIt).point2);
//												}
//												else{
//													// the segments are now ordered (perhaps in wrong way)...
//													// so we have to check if the first and last point
//													//  are graphically laying in the source- and target- node
//													bool invertSegments = false;
//													segIt = segments.begin();
//													node target = actEdge->target();
//													node source = actEdge->source();
//													// check if source is a normal node or a cluster
//													//if (...){
//													
//													//}
//													//else{
//														// big if-check: if (first point is in target
//														//                   and not in source)
//														//                   AND 
//														//                   (last point is in source
//														//                   and not in target)
//														if (( ( (CGA.x(target) + CGA.width(target))>= (*segIt).point1.m_x )
//														  &&   (CGA.x(target)                      <= (*segIt).point1.m_x )
//													      && ( (CGA.y(target) + CGA.height(target))>= (*segIt).point1.m_y )
//													      &&   (CGA.y(target)                      <= (*segIt).point1.m_y ) )
//													      &&
//													      (!( ( (CGA.x(source) + CGA.width(source))>= (*segIt).point1.m_x )
//														  &&   (CGA.x(source)                      <= (*segIt).point1.m_x )
//													      && ( (CGA.y(source) + CGA.height(source))>= (*segIt).point1.m_y )
//													      &&   (CGA.y(source)                      <= (*segIt).point1.m_y ) )))
//													      {
//													      	segIt = segments.rbegin();
//															if (( ( (CGA.x(source) + CGA.width(source))>= (*segIt).point2.m_x )
//															  &&   (CGA.x(source)                      <= (*segIt).point2.m_x )
//														      && ( (CGA.y(source) + CGA.height(source))>= (*segIt).point2.m_y )
//														      &&   (CGA.y(source)                      <= (*segIt).point2.m_y ) )
//														      &&
//														      (!( ( (CGA.x(target) + CGA.width(source))>= (*segIt).point2.m_x )
//															  &&   (CGA.x(target)                      <= (*segIt).point2.m_x )
//														      && ( (CGA.y(target) + CGA.height(source))>= (*segIt).point2.m_y )
//														      &&   (CGA.y(target)                      <= (*segIt).point2.m_y ) ))){
//														      	// invert the segment-line
//														      	invertSegments = true;
//														      }
//													      }
//													//}
//													if (!invertSegments){
//	 													for (segIt = segments.begin(); segIt.valid(); segIt++){
//															dpl.pushBack((*segIt).point1);
//															dpl.pushBack((*segIt).point2);
//														}
//													}
//													else{
//	 													for (segIt = segments.rbegin(); segIt.valid(); segIt--){
//															dpl.pushBack((*segIt).point2);
//															dpl.pushBack((*segIt).point1);
//														}
//													}
//													// unify bends = delete superfluous points
//													dpl.unify();
//													// finally concatenate/set the bends
//													CGA.bends(actEdge).conc(dpl);
//												}// else (checkNumOfSegReInserts==0)												
//											}// else (segments exist)
//										}// points & segments
//																				
//									}//edgeIdRef
//									
//								}// edgeStyle
//
////								// LABELSTYLE
////								if (stylesSon->getName() == ogmlTagNames[t_labelStyle]){
////									// labelStyle
////									// ACTUALLY NOT SUPPORTED
////								}// labelStyle
//														
//								stylesSon = stylesSon->m_pBrother;	
//							} // while
//							
//						}
//				} //styles
//
//				// CONSTRAINTS
//				if (layoutSon->getName() == ogmlTagNames[t_constraints]){
//
//					// this code is encapsulated in the method
//					// OgmlParser::buildConstraints
//					// has to be called by read methods after building
//					
//					// here we only set the pointer,
//					//  so we don't have to traverse the parse tree
//					//  to the constraints tag later
//					m_constraintsTag = layoutSon;
//											
//				}// constraints
//
//
//				// go to next brother
//				layoutSon = layoutSon->m_pBrother;
//				}// while(layoutSon)
//			}//if (layout->m_pFirstSon)
//		}// if ((layout) && (layout->getName() == ogmlTagNames[t_layout]))
//	
//	
//	}// else			
//
//////	cout << "buildAttributedCompoundGraph COMPLETE. Check... " << endl << flush;
//////	edge e;
//////	forall_edges(e, G){
//////		//cout << "CGA.labelEdge" << e << " = " << CGA.labelEdge(e) << endl << flush;
//////		cout << "CGA.arrowEdge" << e << " = " << CGA.arrowEdge(e) << endl << flush;
//////		cout << "CGA.styleEdge" << e << " = " << CGA.styleEdge(e) << endl << flush;
//////		cout << "CGA.edgeWidth" << e << " = " << CGA.edgeWidth(e) << endl << flush;
//////		cout << "CGA.colorEdge" << e << " = " << CGA.colorEdge(e) << endl << flush;			
//////		cout << "CGA.type     " << e << " = " << CGA.type(e) << endl << flush;	
//////		ListConstIterator<DPoint> it;
//////		for(it = CGA.bends(e).begin(); it!=CGA.bends(e).end(); ++it) {
//////			cout << "point " << " x=" << (*it).m_x << " y=" << (*it).m_y << endl << flush;
//////		}
//////				
//////	}
//////
//////	node n;
//////	forall_nodes(n, G){
//////		cout << "CGA.labelNode(" << n << ")     = " << CGA.labelNode(n) << endl << flush;
//////		cout << "CGA.templateNode(" << n << ")  = " << CGA.templateNode(n) << endl << flush;
//////		cout << "CGA.shapeNode(" << n << ")     = " << CGA.shapeNode(n) << endl << flush;
//////		cout << "CGA.width(" << n << ")         = " << CGA.width(n) << endl << flush;
//////		cout << "CGA.height(" << n << ")        = " << CGA.height(n) << endl << flush;
//////		cout << "CGA.colorNode(" << n << ")     = " << CGA.colorNode(n) << endl << flush;
//////		cout << "CGA.nodePattern(" << n << ")   = " << CGA.nodePattern(n) << endl << flush;
//////		cout << "CGA.styleNode(" << n << ")     = " << CGA.styleNode(n) << endl << flush;
//////		cout << "CGA.lineWidthNode(" << n << ") = " << CGA.lineWidthNode(n) << endl << flush;
//////		cout << "CGA.nodeLine(" << n << ")      = " << CGA.nodeLine(n) << endl << flush;
//////		cout << "CGA.x(" << n << ")             = " << CGA.x(n) << endl << flush;
//////		cout << "CGA.y(" << n << ")             = " << CGA.y(n) << endl << flush;
//////		cout << "CGA.type(" << n << ")          = " << CGA.type(n) << endl << flush;
//////	}
//////	
//////	cluster c;
//////	forall_compounds(c, CGA.constClusterGraph()){
//////		cout << "CGA.templateCluster(" << c << ")    = " << CGA.templateCluster(c) << endl << flush;
//////		cout << "CGA.compoundWidth(" << c << ")       = " << CGA.compoundWidth(c) << endl << flush;
//////		cout << "CGA.compoundHeight(" << c << ")      = " << CGA.compoundHeight(c) << endl << flush;
//////		cout << "CGA.compoundFillColor(" << c << ")   = " << CGA.compoundFillColor(c) << endl << flush;
//////		cout << "CGA.compoundFillPattern(" << c << ") = " << CGA.compoundFillPattern(c) << endl << flush;
//////		cout << "CGA.compoundBackColor(" << c << ")   = " << CGA.compoundBackColor(c) << endl << flush;
//////		cout << "CGA.compoundLineStyle(" << c << ")   = " << CGA.compoundLineStyle(c) << endl << flush;
//////		cout << "CGA.compoundLineWidth(" << c << ")   = " << CGA.compoundLineWidth(c) << endl << flush;
//////		cout << "CGA.compoundColor(" << c << ")       = " << CGA.compoundColor(c) << endl << flush;
//////		cout << "CGA.compoundXPos(" << c << ")        = " << CGA.compoundXPos(c) << endl << flush;
//////		cout << "CGA.compoundYPos(" << c << ")        = " << CGA.compoundYPos(c) << endl << flush;
//////	}
////	
//////	cout << "buildAttributedCompoundGraph COMPLETE... Check COMPLETE... Let's have fun in GDE ;) " << endl << flush;
//
//
////	// building terminated, so return true
//	return true;
//	
//}//buildAttributedCompoundGraph




// new version
// ***********************************************************
//
// B U I L D    A T T R I B U T E D    C O M P O U N D -- G R A P H 
//
// ***********************************************************
//Commented out due to missing compound graph in OGDF
/*
bool OgmlParser::buildAttributedCompoundGraph(
							Graph &G,
							CompoundGraphAttributes &CGA, 
							XmlTagObject *root) 
{


	HashConstIterator<String, const XmlTagObject*> it;
	
	if(!root) {
		cout << "WARNING: can't determine layout information, no parse tree available!\n";
	}
	else {
		// root tag isn't a NULL pointer... let's start...
		XmlTagObject* son = root->m_pFirstSon;
		// first traverse to the structure- and the layout block
		if (son->getName() != ogmlTagNames[t_graph]){
			while (son->getName() != ogmlTagNames[t_graph]){
				son = son->m_pFirstSon;
				if (!son){
					// wrong rootTag given or graph tag wasn't found
					return false;
				}
			} //while
		} //if
	
		//now son is the graph tag which first child is structure
		XmlTagObject* structure = son->m_pFirstSon;
		if (structure->getName() != ogmlTagNames[t_structure]){
			return false;	
		}
		// now structure is what it is meant to be
		// traverse the children of structure
		// and set the labels
		son = structure->m_pFirstSon;
		while(son){

			//Set labels of nodes
			if ((son->getName() == ogmlTagNames[t_node]) && (CGA.attributes() & GraphAttributes::nodeLabel)){
				// get the id of the actual node
				XmlAttributeObject *att;
				if(son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
					// lookup for node
					node actNode = (m_nodes.lookup(att->getValue()))->info();
					// find label tag
					XmlTagObject* label;
					if (son->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
						// get content tag
						XmlTagObject* content = label->m_pFirstSon;
						// get the content as string
						if (content->m_pTagValue){
							String str = content->getValue();
							String labelStr = getLabelCaptionFromString(str);
							// now set the label of the node
							CGA.labelNode(actNode) = labelStr;
						}
					}
					// check whether the node is hierarchical and call recursive label-
					//  setting if necessary
					if (isNodeHierarchical(son)){
						// hierSon = hierarchical Son
						XmlTagObject *hierSon;
						if (son->m_pFirstSon){
							hierSon = son->m_pFirstSon;
							while(hierSon){
								// recursive call for setting labels of child nodes
								if (!setLabelsRecursiveForCompounds(G, CGA, hierSon))
									return false;
								hierSon = hierSon->m_pBrother;
							}	
						}
					} // isNodeHierarchical()
				}	
			}// node labels

			//Set labels of edges
			if ((son->getName() == ogmlTagNames[t_edge]) && (CGA.attributes() & GraphAttributes::edgeLabel)) {
				// get the id of the actual edge
				XmlAttributeObject *att;
				if (son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
					// lookup for edge
					edge actEdge = (m_edges.lookup(att->getValue()))->info();
					// find label tag
					XmlTagObject* label;
					if(son->findSonXmlTagObjectByName(ogmlTagNames[t_label], label)){
						// get content tag
						XmlTagObject* content = label->m_pFirstSon;
						// get the content as string
						if (content->m_pTagValue){
							String str = content->getValue();
							String labelStr = getLabelCaptionFromString(str);
							// now set the label of the node
							CGA.labelEdge(actEdge) = labelStr;
						}
					}
				}
			}// edge labels

			// Labels
			// ACTUALLY NOT IMPLEMENTED IN OGDF
			//if (son->getName() == ogmlTagNames[t_label]) {
				// get the id of the actual edge
				//XmlAttributeObject *att;
				//if (son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att)){
					// lookup for label
					//label actLabel = (labels.lookup(att->getValue()))->info();
					// get content tag
					//XmlTagObject* content = son->m_pFirstSon;
					// get the content as string
					//if (content->m_pTagValue){
					//String str = content->getValue();
					//String labelStr = getLabelCaptionFromString(str);
					//now set the label of the node
					//	CGA.labelLabel(actLabel) = labelStr;
					//}
				//}
			//}// Labels

			// go to the next brother
			son = son->m_pBrother;
		}// while(son) // son <=> children of structure

		// get the layout tag
		XmlTagObject* layout = NULL;
		if (structure->m_pBrother != NULL){
			layout = structure->m_pBrother;
		}
		if ((layout) && (layout->getName() == ogmlTagNames[t_layout])){
			// layout exists
			
			// first get the styleTemplates
			XmlTagObject *layoutSon;
			if (layout->m_pFirstSon){
				// layout has at least one child-tag
				layoutSon = layout->m_pFirstSon;
				// ->loop through all of them
				while (layoutSon){

					// style templates
					if (layoutSon->getName() == ogmlTagNames[t_styleTemplates]){
						// has children data, nodeStyleTemplate, edgeStyleTemplate, labelStyleTemplate
						XmlTagObject *styleTemplatesSon;
						if (layoutSon->m_pFirstSon){
							styleTemplatesSon = layoutSon->m_pFirstSon;
							
							while (styleTemplatesSon){
			
								// nodeStyleTemplate
								if (styleTemplatesSon->getName() == ogmlTagNames[t_nodeStyleTemplate]){
									OgmlNodeTemplate *actTemplate;
									XmlAttributeObject *actAtt;
									String actKey;

									if (styleTemplatesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
										actKey = actAtt->getValue();
										actTemplate = new OgmlNodeTemplate(actKey);
									
										XmlTagObject *actTag;

										// template inheritance
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_nodeStyleTemplateRef], actTag)){
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeStyleTemplateIdRef], actAtt)){
												// actual template references another
												// get it from the hash table
												OgmlNodeTemplate *refTemplate;
												if (refTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info()){
													// the referenced template was inserted into the hash table
													// so copy the values
													String actId = actTemplate->m_id;
													*actTemplate = *refTemplate;
													actTemplate->m_id = actId;
												}
											}
										}// template inheritance

//										// data
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//											// found data for nodeStyleTemplate
//											// no implementation required for ogdf
//										}// data

										// shape tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_shape], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nShapeType], actAtt)){
												// TODO: change, if shapes are expanded
												// actually shape and template are calculated from the same value!!!
												actTemplate->m_nodeTemplate = getNodeTemplateFromOgmlValue(actAtt->getValue());
												actTemplate->m_shapeType = getShapeAsInt(actAtt->getValue());
											}
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												actTemplate->m_width = atof(actAtt->getValue());
											// height
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_height], actAtt))
												actTemplate->m_height = atof(actAtt->getValue());
											// uri
											//ACTUALLY NOT SUPPORTED
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_uri], actAtt))
											//	CGA.uri(actNode) = actAtt->getValue();											
										}// shape
										
										// fill tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_fill], actTag)){
											// fill color
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												actTemplate->m_color = actAtt->getValue();
											// fill pattern
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_pattern], actAtt))
												actTemplate->m_pattern = GraphAttributes::intToPattern(getBrushPatternAsInt(actAtt->getValue()));
											// fill patternColor
											//TODO: check if pattern color exists
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_patternColor], actAtt))
											//	actTemplate->m_patternColor = actAtt->getValue());
										}// fill
										
										// line tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
												actTemplate->m_lineType = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												actTemplate->m_lineWidth = atof(actAtt->getValue());
											// color
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												actTemplate->m_lineColor = actAtt->getValue();
										}// line

										//insert actual template into hash table
										m_ogmlNodeTemplates.fastInsert(actKey, actTemplate);
									}
								}//nodeStyleTemplate

								// edgeStyleTemplate
								if (styleTemplatesSon->getName() == ogmlTagNames[t_edgeStyleTemplate]){

									OgmlEdgeTemplate *actTemplate;
									XmlAttributeObject *actAtt;
									String actKey;
									// set id
									if (styleTemplatesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
										actKey = actAtt->getValue();
										actTemplate = new OgmlEdgeTemplate(actKey);
									
										XmlTagObject *actTag;									

										// template inheritance
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_edgeStyleTemplateRef], actTag)){
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeStyleTemplateIdRef], actAtt)){
												// actual template references another
												// get it from the hash table
												OgmlEdgeTemplate *refTemplate;
												if (refTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info()){
													// the referenced template was inserted into the hash table
													// so copy the values
													String actId = actTemplate->m_id;
													*actTemplate = *refTemplate;
													actTemplate->m_id = actId;
												}
											}
										}// template inheritance

//										// data
//										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//											// found data for edgeStyleTemplate
//											// no implementation required for ogdf
//										}// data

										// line tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
												actTemplate->m_lineType = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												actTemplate->m_lineWidth = atof(actAtt->getValue());
											// color
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												actTemplate->m_color = actAtt->getValue();	
										}// line
										
										// sourceStyle tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_sourceStyle], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
												actTemplate->m_sourceType = getArrowStyleAsInt(actAtt->getValue(), ogmlTagNames[t_source]);
											// color
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
											//	actTemplate->m_sourceColor = actAtt->getValue();
											// size
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
											//	actTemplate->m_sourceSize = atof(actAtt->getValue());
										}// fill
										
										// targetStyle tag
										if (styleTemplatesSon->findSonXmlTagObjectByName(ogmlTagNames[t_targetStyle], actTag)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
												actTemplate->m_targetType = getArrowStyleAsInt(actAtt->getValue(), ogmlTagNames[t_target]);
											// color
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
											//	actTemplate->m_targetColor = actAtt->getValue();
											// size
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
											//	actTemplate->m_targetSize = atof(actAtt->getValue());
										}// fill

										//insert actual template into hash table
										m_ogmlEdgeTemplates.fastInsert(actKey, actTemplate);
									}	
				
								}//edgeStyleTemplate			
								
								// labelStyleTemplate
								if (styleTemplatesSon->getName() == ogmlTagNames[t_labelStyleTemplate]){
									// ACTUALLY NOT SUPPORTED
								}//labelStyleTemplate
														
								styleTemplatesSon = styleTemplatesSon->m_pBrother;	
							}
						}
					}// styleTemplates

					//STYLES
					if (layoutSon->getName() == ogmlTagNames[t_styles]){
						// has children graphStyle, nodeStyle, edgeStyle, labelStyle
						XmlTagObject *stylesSon;
						if (layoutSon->m_pFirstSon){
							stylesSon = layoutSon->m_pFirstSon;
							
							while (stylesSon){

								// GRAPHSTYLE
								if (stylesSon->getName() == ogmlTagNames[t_graphStyle]){
									XmlAttributeObject *actAtt;
									// defaultNodeTemplate
									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultNodeTemplate], actAtt)){

										OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
										
//										XmlTagObject *actTag;
//										// data
//										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//											// found data for graphStyle
//											// no implementation required for ogdf
//										}// data

										// set values for ALL nodes
										node v;
										forall_nodes(v, G){

											if (CGA.attributes() & GraphAttributes::nodeType){
												CGA.templateNode(v) = actTemplate->m_nodeTemplate;
												CGA.shapeNode(v) = actTemplate->m_shapeType;
											}
											if (CGA.attributes() & GraphAttributes::nodeGraphics){
												CGA.width(v) = actTemplate->m_width;
												CGA.height(v) = actTemplate->m_height;
											}
											if (CGA.attributes() & GraphAttributes::nodeColor)
												CGA.colorNode(v) = actTemplate->m_color;
											if (CGA.attributes() & GraphAttributes::nodeStyle){
												CGA.nodePattern(v) = actTemplate->m_pattern;
												//CGA.nodePatternColor(v) = actTemplate->m_patternColor;
												CGA.styleNode(v) = actTemplate->m_lineType;
												CGA.lineWidthNode(v) = actTemplate->m_lineWidth;
												CGA.nodeLine(v) = actTemplate->m_lineColor;
											}
										}// forall_nodes
									}// defaultNodeTemplate

//									// defaultCompoundTemplate
//									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultCompoundTemplate], actAtt)){
//
//										OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
//
//										// set values for ALL Compounds
//										compound c;
//										forall_compounds(c, G){
//											
//											if (CGA.attributes() & CompoundGraphAttributes::nodeType){
//												CGA.templateCompound(c) = actTemplate->m_nodeTemplate;
//												// no shape definition for compounds
//												//CGA.shapeNode(c) = actTemplate->m_shapeType;
//											}
//											if (CGA.attributes() & CompoundGraphAttributes::nodeGraphics){
//													CGA.compoundWidth(c) = actTemplate->m_width;
//													CGA.compoundHeight(c) = actTemplate->m_height;
//											}
//											if (CGA.attributes() & CompoundGraphAttributes::nodeColor)
//												CGA.compoundFillColor(c) = actTemplate->m_color;
//											if (CGA.attributes() & CompoundGraphAttributes::nodeStyle){
//												CGA.compoundFillPattern(c) = actTemplate->m_pattern;
//												CGA.compoundBackColor(c) = actTemplate->m_patternColor;
//												CGA.compoundLineStyle(c) = actTemplate->m_lineType;
//												CGA.compoundLineWidth(c) = actTemplate->m_lineWidth;
//												CGA.compoundColor(c) = actTemplate->m_lineColor;
//											}
//										}// forall_compounds
//									}// defaultCompoundTemplate

									
									// defaultEdgeTemplate
									if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultEdgeTemplate], actAtt)){

										OgmlEdgeTemplate* actTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info();

										// set values for ALL edges
										edge e;
										forall_edges(e, G){
											
												if (CGA.attributes() & GraphAttributes::edgeStyle){
													CGA.styleEdge(e) = actTemplate->m_lineType;
													CGA.edgeWidth(e) = actTemplate->m_lineWidth;
												}
												if (CGA.attributes() & GraphAttributes::edgeColor){
													CGA.colorEdge(e) = actTemplate->m_color;
												}
												
												//edgeArrow
												if ((CGA.attributes()) & (GraphAttributes::edgeArrow)){
													if (actTemplate->m_sourceType == 0){
														if (actTemplate->m_targetType == 0){
															// source = no_arrow, target = no_arrow // =>none
															CGA.arrowEdge(e) = GraphAttributes::none;
														}
														else{
															// source = no_arrow, target = arrow // =>last
															CGA.arrowEdge(e) = GraphAttributes::last;
														}
													}
													else{
														if (actTemplate->m_targetType == 0){
															// source = arrow, target = no_arrow // =>first
															CGA.arrowEdge(e) = GraphAttributes::first;
														}
														else{
															// source = arrow, target = arrow // =>both
															CGA.arrowEdge(e) = GraphAttributes::both;
														}
													}
												}//edgeArrow	
										}//forall_edges
									}//defaultEdgeTemplate

									// defaultLabelTemplate
									//if (stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_defaultLabelTemplate], actAtt)){
									//	// set values for ALL labels
									//	// ACTUALLY NOT IMPLEMENTED
									//  label l;
									//  forall_labels(l, G){
									//		
									//	}
									//}//defaultLabelTemplate
								}// graphStyle

								// NODESTYLE
								// get the id of the actual node
								XmlAttributeObject *att;
								if(stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeIdRef], att)){
										
									if (m_nodes.lookup(att->getValue())){
										
										// lookup for node
										node actNode = (m_nodes.lookup(att->getValue()))->info();

										// actTag is the actual tag that is considered
										XmlTagObject* actTag;
										XmlAttributeObject *actAtt;

//											// data
//											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//												// found data for nodeStyle
//												// no implementation required for ogdf
//											}// data

										// check if actual nodeStyle references a template
										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_nodeStyleTemplateRef], actTag)){
											// get referenced template id
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeStyleTemplateIdRef], actAtt)){
												// actual nodeStyle references a template
												OgmlNodeTemplate* actTemplate = m_ogmlNodeTemplates.lookup(actAtt->getValue())->info();
												if (CGA.attributes() & GraphAttributes::nodeType){
													CGA.templateNode(actNode) = actTemplate->m_nodeTemplate;
													CGA.shapeNode(actNode) = actTemplate->m_shapeType;
												}
												if (CGA.attributes() & GraphAttributes::nodeGraphics){
													CGA.width(actNode) = actTemplate->m_width;
													CGA.height(actNode) = actTemplate->m_height;
												}
												if (CGA.attributes() & GraphAttributes::nodeColor)
													CGA.colorNode(actNode) = actTemplate->m_color;
												if (CGA.attributes() & GraphAttributes::nodeStyle){
													CGA.nodePattern(actNode) = actTemplate->m_pattern;
													//CGA.nodePatternColor(actNode) = actTemplate->m_patternColor;
													CGA.styleNode(actNode) = actTemplate->m_lineType;
													CGA.lineWidthNode(actNode) = actTemplate->m_lineWidth;
													CGA.nodeLine(actNode) = actTemplate->m_lineColor;
												}										
											}
										}//template

										// Graph::nodeType
										//TODO: COMPLETE, IF NECESSARY
										CGA.type(actNode) = Graph::vertex;
										
										// location tag
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_location], actTag)) 
											&& (CGA.attributes() & GraphAttributes::nodeGraphics)){
											// set location of node
											// x
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
												CGA.x(actNode) = atof(actAtt->getValue());
											// y
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_y], actAtt))
												CGA.y(actNode) = atof(actAtt->getValue());
											// z
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt))
												//CGA.z(actNode) = atof(actAtt->getValue());
										}// location

										// shape tag
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_shape], actTag))
											&& (CGA.attributes() & GraphAttributes::nodeType)){
											// set shape of node
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nShapeType], actAtt)){
												CGA.templateNode(actNode) = getNodeTemplateFromOgmlValue(actAtt->getValue());
												// TODO: change, if shapes are expanded
												// actually shape and template are calculated from the same value!!!
												CGA.shapeNode(actNode) = getShapeAsInt(actAtt->getValue());
											}
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												CGA.width(actNode) = atof(actAtt->getValue());
											// height
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_height], actAtt))
												CGA.height(actNode) = atof(actAtt->getValue());
											// uri
											//ACTUALLY NOT SUPPORTED
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_uri], actAtt))
											//	CGA.uri(actNode) = actAtt->getValue();											
										}// shape

										// fill tag
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_fill], actTag))
											&& (CGA.attributes() & GraphAttributes::nodeStyle)){
											// fill color
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												CGA.colorNode(actNode) = actAtt->getValue();
											// fill pattern
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_pattern], actAtt))
												CGA.nodePattern(actNode) = GraphAttributes::intToPattern(getBrushPatternAsInt(actAtt->getValue()));
											// fill patternColor
											//TODO: check if pattern color exists
											//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_patternColor], actAtt))
											//	CGA.nodePatternColor(actNode) = actAtt->getValue());
										}// fill

										// line tag
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag))
											&& (CGA.attributes() & GraphAttributes::nodeStyle)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
												CGA.styleNode(actNode) = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												CGA.lineWidthNode(actNode) = atof(actAtt->getValue());
											// color
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												CGA.nodeLine(actNode) = actAtt->getValue();
										}// line
										
										// image tag
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_image], actTag))
											&& (CGA.attributes() & GraphAttributes::nodeStyle)){
											// uri
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageUri], actAtt))
												CGA.imageUriNode(actNode) = actAtt->getValue();
											// style
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageStyle], actAtt))
												CGA.imageStyleNode(actNode) = GraphAttributes::intToImageStyle(getImageStyleAsInt(actAtt->getValue()));
											// alignment
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageAlignment], actAtt))
												CGA.imageAlignmentNode(actNode) = GraphAttributes::intToImageAlignment(getImageAlignmentAsInt(actAtt->getValue()));
											// drawLine
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageDrawLine], actAtt)){
												if ((actAtt->getValue() == "true") || (actAtt->getValue() == "1"))
													CGA.imageDrawLineNode(actNode) = true;
												else
													CGA.imageDrawLineNode(actNode) = false;
											}
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageWidth], actAtt))
												CGA.imageWidthNode(actNode) = atof(actAtt->getValue());
											// height
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_imageHeight], actAtt))
												CGA.imageHeightNode(actNode) = atof(actAtt->getValue());
										}// image
										
//											// ports
//											// go through all ports with dummy tagObject port
//											XmlTagObject* port = stylesSon->m_pFirstSon;
//											while(port){
//												if (port->getName() == ogmlTagObjects[t_port]){
//													// TODO: COMPLETE
//													// ACTUALLY NOT IMPLEMENTED IN OGDF
//												}
//												
//												// go to next tag
//												port = port->m_pBrother;	
//											}
										
									}// m_nodes.lookup
								}// nodeStyle

								// EDGESTYLE
								if (stylesSon->getName() == ogmlTagNames[t_edgeStyle]){
									
									// get the id of the actual edge
									XmlAttributeObject *att;		
									if(stylesSon->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeIdRef], att)){
										// lookup for edge
										edge actEdge = (m_edges.lookup(att->getValue()))->info();
										
										// actTag is the actual tag that is considered
										XmlTagObject* actTag;
										XmlAttributeObject *actAtt;

//										// data
//										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_data], actTag)){
//											// found data for edgeStyle
//											// no implementation required for ogdf
//										}// data

										// check if actual edgeStyle references a template
										if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_edgeStyleTemplateRef], actTag)){
											// get referenced template id
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_edgeStyleTemplateIdRef], actAtt)){
												// actual edgeStyle references a template
												OgmlEdgeTemplate* actTemplate = m_ogmlEdgeTemplates.lookup(actAtt->getValue())->info();
												if (CGA.attributes() & GraphAttributes::edgeStyle){
													CGA.styleEdge(actEdge) = actTemplate->m_lineType;
													CGA.edgeWidth(actEdge) = actTemplate->m_lineWidth;
												}
												if (CGA.attributes() & GraphAttributes::edgeColor){
													CGA.colorEdge(actEdge) = actTemplate->m_color;
												}
												
												//edgeArrow
												if ((CGA.attributes()) & (GraphAttributes::edgeArrow)){
													if (actTemplate->m_sourceType == 0){
														if (actTemplate->m_targetType == 0){
															// source = no_arrow, target = no_arrow // =>none
															CGA.arrowEdge(actEdge) = GraphAttributes::none;
														}
														else{
															// source = no_arrow, target = arrow // =>last
															CGA.arrowEdge(actEdge) = GraphAttributes::last;
														}
													}
													else{
														if (actTemplate->m_targetType == 0){
															// source = arrow, target = no_arrow // =>first
															CGA.arrowEdge(actEdge) = GraphAttributes::first;
														}
														else{
															// source = arrow, target = arrow // =>both
															CGA.arrowEdge(actEdge) = GraphAttributes::both;
														}
													}
												}//edgeArrow

											}
										}//template
										
										// Graph::edgeType
										//TODO: COMPLETE, IF NECESSARY
										CGA.type(actEdge) = Graph::association;
										
										// line tag
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_line], actTag))
											&& (CGA.attributes() & GraphAttributes::edgeType)){
											// type
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_nLineType], actAtt))
												CGA.styleEdge(actEdge) = GraphAttributes::intToStyle(getLineTypeAsInt(actAtt->getValue()));
											// width
											if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_width], actAtt))
												CGA.edgeWidth(actEdge) = atof(actAtt->getValue());
											// color
											if ((actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												&& (CGA.attributes() & GraphAttributes::edgeType))
												CGA.colorEdge(actEdge) = actAtt->getValue();
										}// line

										// mapping of arrows
										if (CGA.attributes() & GraphAttributes::edgeArrow){
											
											// values for mapping edge arrows to GDE
											// init to -1 for a simple check
											int sourceInt = -1;
											int targetInt = -1;

											// sourceStyle tag
											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_sourceStyle], actTag)){
												// type
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
													sourceInt = getArrowStyleAsInt((actAtt->getValue()), ogmlAttributeNames[t_source]);
												// color
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												//	;
												// size
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
												//	;
											}// sourceStyle

											// targetStyle tag
											if (stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_targetStyle], actTag)){
												// type
												if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
													targetInt = getArrowStyleAsInt((actAtt->getValue()), ogmlAttributeNames[t_target]);
												// color
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_color], actAtt))
												//	;
												// size
												//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_size], actAtt))
												//	;
											}// targetStyle
											
											// map edge arrows
											if ((sourceInt != -1) || (targetInt != -1)){
												if (sourceInt <= 0){
													if (targetInt <= 0){
														//source=no arrow, target=no arrow // => none
														CGA.arrowEdge(actEdge) = GraphAttributes::none;
													}
													else{
														// source=no arrow, target=arrow // => last
														CGA.arrowEdge(actEdge) = GraphAttributes::last;
													}
												}
												else{
													if (targetInt <= 0){
														//source=arrow, target=no arrow // => first
														CGA.arrowEdge(actEdge) = GraphAttributes::first;
													}
													else{
														//source=target=arrow // => both
														CGA.arrowEdge(actEdge) = GraphAttributes::both;
													}
												}
											}
										}//arrow
										
										// points & segments
										// bool value for checking if segments exist
										bool segmentsExist = stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_segment], actTag);
										if ((stylesSon->findSonXmlTagObjectByName(ogmlTagNames[t_point], actTag))
											&& (CGA.attributes() & GraphAttributes::edgeGraphics)){
											// at least one point exists
											XmlTagObject *pointTag = stylesSon->m_pFirstSon;
											DPolyline dpl;
											dpl.clear();
											// traverse all points in the order given in the ogml file
											while (pointTag){
												if (pointTag->getName() == ogmlTagNames[t_point]){
													
													if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt)){
														DPoint dp;
														// here we have a point
														if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_x], actAtt)){
															dp.m_x = atof(actAtt->getValue());
														}
														if (pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_y], actAtt)){
															dp.m_y = atof(actAtt->getValue());
														}
														//if (actTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_z], actAtt))
														//	dp.m_z = atof(actAtt->getValue());
														// insert point into hash table
														pointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt);
														points.fastInsert(actAtt->getValue(), dp);
														//insert point into polyline
														if (!segmentsExist)
															dpl.pushBack(dp);
													}
												}
												// go to next tag
												pointTag = pointTag->m_pBrother;	
											}// while (pointTag)
											//concatenate polyline
											if (!segmentsExist){
												CGA.bends(actEdge).conc(dpl);
											}
											else{
												// work with segments
												// one error can occur:
												// if a segments is going to be inserted,
												// which doesn't match with any other,
												// the order can be not correct at the end
												// then the edge is really corrupted!!
												
												// TODO: this implementation doesn't work with hyperedges
												//       cause hyperedges have more than one source/target
												
												// segmentsUnsorted stores all found segments
												List<OgmlSegment> segmentsUnsorted;
												XmlTagObject *segmentTag = stylesSon->m_pFirstSon;
												while (segmentTag){
													if (segmentTag->getName() == ogmlTagNames[t_segment]){
														XmlTagObject *endpointTag = segmentTag->m_pFirstSon;
														OgmlSegment actSeg;
														int endpointsSet = 0;
														while ((endpointTag) && (endpointsSet <2)){
															if (endpointTag->getName() == ogmlTagNames[t_endpoint]){
																// get the referenced point
																endpointTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_endpointIdRef], actAtt);
																DPoint dp = (points.lookup(actAtt->getValue()))->info();

																if (endpointsSet == 0)
																	actSeg.point1 = dp;
																else
																	actSeg.point2 = dp;
																endpointsSet++;
															}
															endpointTag = endpointTag->m_pBrother;	
														}// while
														// now we created a segment
														// we can insert this easily into in segmentsUnsorted														
														if (actSeg.point1 != actSeg.point2){
															segmentsUnsorted.pushBack(actSeg);
														} // point1 != point2
													}// if (segment)
													// go to next tag
													segmentTag = segmentTag->m_pBrother;
												}// while (segmentTag)
												// now are the segments stored in the segmentsUnsorted list
												//  but we have to sort it in segments list while inserting
												List<OgmlSegment> segments;
												ListIterator<OgmlSegment> segIt;
												// check the number of re-insertions
												int checkNumOfSegReInserts = segmentsUnsorted.size()+2;
												while ((segmentsUnsorted.size() > 0) && (checkNumOfSegReInserts > 0)){
													OgmlSegment actSeg = segmentsUnsorted.front();
													segmentsUnsorted.popFront();
													// actSeg has to be inserted in correct order
													//  and then being deleted
													//  OR waiting in list until it can be inserted
													// size == 0 => insert
													if (segments.size() == 0){
														segments.pushFront(actSeg);
													}
													else{
														// segments contains >1 segment
														segIt = segments.begin();
														bool inserted = false;
														while (segIt.valid() && !inserted){
															if ((actSeg.point1 == (*segIt).point1) ||
															    (actSeg.point1 == (*segIt).point2) ||
															    (actSeg.point2 == (*segIt).point1) ||
															    (actSeg.point2 == (*segIt).point2)){							
																	// found two matching segments
																	// now we can insert
																	// there are some cases to check
																	if (actSeg.point1 == (*segIt).point1){
																		DPoint dumP = actSeg.point1;
																		actSeg.point1 = actSeg.point2;
																		actSeg.point2 = dumP;
																		segments.insertBefore(actSeg, segIt);
																	}
																	else
																		if (actSeg.point2 == (*segIt).point1){
																			segments.insertBefore(actSeg, segIt);
																		}
																		else
																			if ((actSeg.point2 == (*segIt).point2)){
																				DPoint dumP = actSeg.point1;
																				actSeg.point1 = actSeg.point2;
																				actSeg.point2 = dumP;
																				segments.insertAfter(actSeg, segIt);	
																			}
																			else{
																				segments.insertAfter(actSeg, segIt);
																			}
																	inserted = true;
															    } // first if
															segIt++;
														} //while
														if (!inserted){
															// segment doesn't found matching segment,
															//  so insert it again into unsorted segments list
															//  so it will be inserted later
															segmentsUnsorted.pushBack(actSeg);
															checkNumOfSegReInserts--;
														}
													}//else
												}//while segmentsUnsorted.size() > 0


												if (checkNumOfSegReInserts==0){
													cout << "WARNING! Segment definition is not correct" << endl << flush;
													cout << "Not able to work with #"<< segmentsUnsorted.size() << " segments" << endl << flush;
													cout << "Please check connection and sorting of segments!" << endl << flush;
//													// inserting the bends although there might be an error
//													// I commented this, because in this case in ogdf the edge will 
//													//   be a straight edge and there will not be any artefacts
//													// TODO: uncomment if desired
// 													for (segIt = segments.begin(); segIt.valid(); segIt++){
//														dpl.pushBack((*segIt).point1);
//														dpl.pushBack((*segIt).point2);
												}
												else{
													// the segments are now ordered (perhaps in wrong way)...
													// so we have to check if the first and last point
													//  are graphically laying in the source- and target- node
													bool invertSegments = false;
													segIt = segments.begin();
													node target = actEdge->target();
													node source = actEdge->source();
													// check if source is a normal node or a cluster
													//if (...){
													
													//}
													//else{
														// big if-check: if (first point is in target
														//                   and not in source)
														//                   AND 
														//                   (last point is in source
														//                   and not in target)
														if (( ( (CGA.x(target) + CGA.width(target))>= (*segIt).point1.m_x )
														  &&   (CGA.x(target)                      <= (*segIt).point1.m_x )
													      && ( (CGA.y(target) + CGA.height(target))>= (*segIt).point1.m_y )
													      &&   (CGA.y(target)                      <= (*segIt).point1.m_y ) )
													      &&
													      (!( ( (CGA.x(source) + CGA.width(source))>= (*segIt).point1.m_x )
														  &&   (CGA.x(source)                      <= (*segIt).point1.m_x )
													      && ( (CGA.y(source) + CGA.height(source))>= (*segIt).point1.m_y )
													      &&   (CGA.y(source)                      <= (*segIt).point1.m_y ) )))
													      {
													      	segIt = segments.rbegin();
															if (( ( (CGA.x(source) + CGA.width(source))>= (*segIt).point2.m_x )
															  &&   (CGA.x(source)                      <= (*segIt).point2.m_x )
														      && ( (CGA.y(source) + CGA.height(source))>= (*segIt).point2.m_y )
														      &&   (CGA.y(source)                      <= (*segIt).point2.m_y ) )
														      &&
														      (!( ( (CGA.x(target) + CGA.width(source))>= (*segIt).point2.m_x )
															  &&   (CGA.x(target)                      <= (*segIt).point2.m_x )
														      && ( (CGA.y(target) + CGA.height(source))>= (*segIt).point2.m_y )
														      &&   (CGA.y(target)                      <= (*segIt).point2.m_y ) ))){
														      	// invert the segment-line
														      	invertSegments = true;
														      }
													      }
													//}
													if (!invertSegments){
	 													for (segIt = segments.begin(); segIt.valid(); segIt++){
															dpl.pushBack((*segIt).point1);
															dpl.pushBack((*segIt).point2);
														}
													}
													else{
	 													for (segIt = segments.rbegin(); segIt.valid(); segIt--){
															dpl.pushBack((*segIt).point2);
															dpl.pushBack((*segIt).point1);
														}
													}
													// unify bends = delete superfluous points
													dpl.unify();
													// finally concatenate/set the bends
													CGA.bends(actEdge).conc(dpl);
												}// else (checkNumOfSegReInserts==0)												
											}// else (segments exist)
										}// points & segments
																				
									}//edgeIdRef
									
								}// edgeStyle

//								// LABELSTYLE
//								if (stylesSon->getName() == ogmlTagNames[t_labelStyle]){
//									// labelStyle
//									// ACTUALLY NOT SUPPORTED
//								}// labelStyle
														
								stylesSon = stylesSon->m_pBrother;	
							} // while
							
						}
				} //styles

				// CONSTRAINTS
				if (layoutSon->getName() == ogmlTagNames[t_constraints]){

					// this code is encapsulated in the method
					// OgmlParser::buildConstraints
					// has to be called by read methods after building
					
					// here we only set the pointer,
					//  so we don't have to traverse the parse tree
					//  to the constraints tag later
					m_constraintsTag = layoutSon;
											
				}// constraints


				// go to next brother
				layoutSon = layoutSon->m_pBrother;
				}// while(layoutSon)
			}//if (layout->m_pFirstSon)
		}// if ((layout) && (layout->getName() == ogmlTagNames[t_layout]))
	
	
	}// else			

////	cout << "buildAttributedCompoundGraph COMPLETE. Check... " << endl << flush;
////	edge e;
////	forall_edges(e, G){
////		//cout << "CGA.labelEdge" << e << " = " << CGA.labelEdge(e) << endl << flush;
////		cout << "CGA.arrowEdge" << e << " = " << CGA.arrowEdge(e) << endl << flush;
////		cout << "CGA.styleEdge" << e << " = " << CGA.styleEdge(e) << endl << flush;
////		cout << "CGA.edgeWidth" << e << " = " << CGA.edgeWidth(e) << endl << flush;
////		cout << "CGA.colorEdge" << e << " = " << CGA.colorEdge(e) << endl << flush;			
////		cout << "CGA.type     " << e << " = " << CGA.type(e) << endl << flush;	
////		ListConstIterator<DPoint> it;
////		for(it = CGA.bends(e).begin(); it!=CGA.bends(e).end(); ++it) {
////			cout << "point " << " x=" << (*it).m_x << " y=" << (*it).m_y << endl << flush;
////		}
////				
////	}
////
////	node n;
////	forall_nodes(n, G){
////		cout << "CGA.labelNode(" << n << ")     = " << CGA.labelNode(n) << endl << flush;
////		cout << "CGA.templateNode(" << n << ")  = " << CGA.templateNode(n) << endl << flush;
////		cout << "CGA.shapeNode(" << n << ")     = " << CGA.shapeNode(n) << endl << flush;
////		cout << "CGA.width(" << n << ")         = " << CGA.width(n) << endl << flush;
////		cout << "CGA.height(" << n << ")        = " << CGA.height(n) << endl << flush;
////		cout << "CGA.colorNode(" << n << ")     = " << CGA.colorNode(n) << endl << flush;
////		cout << "CGA.nodePattern(" << n << ")   = " << CGA.nodePattern(n) << endl << flush;
////		cout << "CGA.styleNode(" << n << ")     = " << CGA.styleNode(n) << endl << flush;
////		cout << "CGA.lineWidthNode(" << n << ") = " << CGA.lineWidthNode(n) << endl << flush;
////		cout << "CGA.nodeLine(" << n << ")      = " << CGA.nodeLine(n) << endl << flush;
////		cout << "CGA.x(" << n << ")             = " << CGA.x(n) << endl << flush;
////		cout << "CGA.y(" << n << ")             = " << CGA.y(n) << endl << flush;
////		cout << "CGA.type(" << n << ")          = " << CGA.type(n) << endl << flush;
////	}
////	
////	cluster c;
////	forall_compounds(c, CGA.constClusterGraph()){
////		cout << "CGA.templateCluster(" << c << ")    = " << CGA.templateCluster(c) << endl << flush;
////		cout << "CGA.compoundWidth(" << c << ")       = " << CGA.compoundWidth(c) << endl << flush;
////		cout << "CGA.compoundHeight(" << c << ")      = " << CGA.compoundHeight(c) << endl << flush;
////		cout << "CGA.compoundFillColor(" << c << ")   = " << CGA.compoundFillColor(c) << endl << flush;
////		cout << "CGA.compoundFillPattern(" << c << ") = " << CGA.compoundFillPattern(c) << endl << flush;
////		cout << "CGA.compoundBackColor(" << c << ")   = " << CGA.compoundBackColor(c) << endl << flush;
////		cout << "CGA.compoundLineStyle(" << c << ")   = " << CGA.compoundLineStyle(c) << endl << flush;
////		cout << "CGA.compoundLineWidth(" << c << ")   = " << CGA.compoundLineWidth(c) << endl << flush;
////		cout << "CGA.compoundColor(" << c << ")       = " << CGA.compoundColor(c) << endl << flush;
////		cout << "CGA.compoundXPos(" << c << ")        = " << CGA.compoundXPos(c) << endl << flush;
////		cout << "CGA.compoundYPos(" << c << ")        = " << CGA.compoundYPos(c) << endl << flush;
////	}
//	
////	cout << "buildAttributedCompoundGraph COMPLETE... Check COMPLETE... Let's have fun in GDE ;) " << endl << flush;


//	// building terminated, so return true
	return true;
	
}//buildAttributedCompoundGraph
*/



// ***********************************************************
//
// b u i l d     g r a p h     f o r     c o m p o u n d s 
//
// ***********************************************************
//Info: need to be a bit different from buildGraph
//      because hierarchical nodes can be ignored!!!
//Commented out due to missing compound graph in OGDF
/*
bool OgmlParser::buildGraphForCompounds(Graph &G) {
	
	G.clear();
	
	int id = 0;
	
	//Build nodes first
	HashConstIterator<String, const XmlTagObject*> it;

	for(it = ids.begin(); it.valid(); ++it) {
		if(it.info()->getName() == ogmlTagNames[t_node]) {
			// get id string from xmlTag
			XmlAttributeObject *idAtt;
			if ( (it.info())->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], idAtt) 
			  && (getIdFromString(idAtt->getValue(), id)) ){
				// now we got an id from the id-string
				// we have to check, if this id was assigned
				if (m_nodeIds.lookup(id)){
					// new id was assigned to another node
					id = G.maxNodeIndex() + 1;
				}
			}
			else{
				// default id setting
				id = G.maxNodeIndex() + 1;	
			}
			m_nodes.fastInsert(it.key(), G.newNode(id));
			m_nodeIds.fastInsert(id, idAtt->getValue());			
		}
	}//for nodes

	id = 0;
	
	//Build edges second
	for(it = ids.begin(); it.valid(); ++it) {
		if( it.info()->getName() == ogmlTagNames[t_edge] ) {
			
			//Check sources/targets
			Stack<node> srcTgt;
			const XmlTagObject* son = it.info()->m_pFirstSon;
			while(son) {
				if( son->getName() == ogmlTagNames[t_source] ||
				    son->getName() == ogmlTagNames[t_target] ) {
				  	XmlAttributeObject *att;
				  	son->findXmlAttributeObjectByName(ogmlAttributeNames[a_nodeIdRef], att);
				  	//Validate if source/target is really a node
				  	if(ids.lookup(att->getValue())->info()->getName() != ogmlTagNames[t_node]) {
				  		cout << "WARNING: edge relation between graph elements of none type node " <<
				  		        "are temporarily not supported!\n";
				  	}
				    else {
				    	srcTgt.push(m_nodes.lookup(att->getValue())->info());	
				    }
				 }
				 son = son->m_pBrother;
			}
			if(srcTgt.size() != 2) {
				cout << "WARNING: hyperedges are temporarily not supported! Discarding edge.\n";	
			}
			else{
				// create edge
				
				// get id string from xmlTag
				XmlAttributeObject *idAtt;
				if ( (it.info())->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], idAtt) 
				  && (getIdFromString(idAtt->getValue(), id)) ){
					if (m_edgeIds.lookup(id)){
						// new id was assigned to another edge
						id = G.maxEdgeIndex() + 1;
					}
				}
				else{
					// default id setting
					id = G.maxEdgeIndex() + 1;
				}
				m_edges.fastInsert(it.key(), G.newEdge(srcTgt.pop(), srcTgt.pop(), id));
				m_edgeIds.fastInsert(id, idAtt->getValue());
			}
		}
	}//for edges

	//Structure data determined, so building the graph was successfull.
	return true;
};//buildGraph for compounds

*/



// ***********************************************************
//
// b u i l d    c o m p o u n d -- graph
// 
// ***********************************************************
//Commented out due to missing compound graph in OGDF
/*
bool OgmlParser::buildCompoundRecursive(
						XmlTagObject *xmlTag, 
						compound parent, 
						Graph &G,
						CompoundGraph &CG){
	
	// create compound and insert into hash tables
	XmlAttributeObject* att;
	xmlTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att);
	node v = m_nodes.lookup(att->getValue())->info();

	// INFO: (HS) compound is already existent
	// compound actCompound = CG.newCompound(v, parent, false);
	compound actCompound = CG.theCompound(v);
	CG.moveCompound(actCompound, parent);
	
	m_compounds.fastInsert(att->getValue(), actCompound);

	// check children of compound tag
	XmlTagObject *son = xmlTag->m_pFirstSon;

	while (son) {
		if (son->getName() == ogmlTagNames[t_node]){
			if (isNodeHierarchical(son))
				// recursive call
				buildCompoundRecursive(son, actCompound, G, CG);
			else {
				// the actual node tag is a child of the compound
				XmlAttributeObject *att;				
				//parse tree is valid so tag owns id attribute
				son->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att);
				// get node from lookup table with the id in att
				node v = m_nodes.lookup(att->getValue())->info();
				// create compound and insert into hash tables
		        // INFO: (HS) compound is already existent
		        // compound actCompound = CG.newCompound(v, parent, true);
		        
		        CG.moveCompound(CG.theCompound(v), actCompound);
			}
		}
		
		son = son->m_pBrother;
	}//while
	
	return true;
}//buildCompoundRecursive
*/

//Commented out due to missing compound graph in OGDF
/*
bool OgmlParser::buildCompound(
					XmlTagObject *rootTag, 
					Graph &G, 
					CompoundGraph &CG){
	CG.semiClear();
  // CG.init(G);
	
	if(rootTag->getName() != ogmlTagNames[t_ogml]) {
		cerr << "ERROR: Expecting root tag \"" << ogmlTagNames[t_ogml]	<< "\" in OgmlParser::buildCompound!\n";
		return false;
	}
	
	//Search for first node tag
	XmlTagObject *nodeTag;
	XmlAttributeObject* att;
	
	rootTag->findSonXmlTagObjectByName(ogmlTagNames[t_graph], nodeTag);
	nodeTag->findSonXmlTagObjectByName(ogmlTagNames[t_structure], nodeTag);
	nodeTag->findSonXmlTagObjectByName(ogmlTagNames[t_node], nodeTag);

	while (nodeTag) {
		if (nodeTag->getName() == ogmlTagNames[t_node]) {
			
			if (isNodeHierarchical(nodeTag)){
				if (!buildCompoundRecursive(nodeTag, CG.rootCompound(), G, CG))
					return false;
			}
			else{
				nodeTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], att);
				node v = m_nodes.lookup(att->getValue())->info();

		        // INFO: (HS) compound for every node is already existent
		        // compound actCompound = CG.newCompound(v, CG.rootCompound(), true);
		        compound actCompound = CG.theCompound(v);
		        CG.moveCompound(actCompound, CG.rootCompound());
			}
		}
		nodeTag = nodeTag->m_pBrother;
	}
	return true;
};//buildCompound
*/

// ***********************************************************
//
// b u i l d     c o n s t r a i n t s 
// 
// ***********************************************************
//Commented out due to missing graphconstraints in OGDF
/*
bool OgmlParser::buildConstraints(Graph& G, GraphConstraints &GC) {

	// constraints-tag was already set
	// if not, then return... job's done
	if (!m_constraintsTag)
		return true;

	if (m_constraintsTag->getName() != ogmlTagNames[t_constraints]){
		cerr << "Error: constraints tag is not the required tag!" << endl;
		return false;
	}

	XmlTagObject* constraintTag;
	if(! m_constraintsTag->findSonXmlTagObjectByName(ogmlTagNames[t_constraint], constraintTag) ) {
		cerr << "Error: no constraint block in constraints block of valid parse tree found!" << endl;
		return false;
	}


	while(constraintTag) {
	
//		// found data
//		if (constraintTag->getName() == ogmlTagNames[t_data]){
//			// found data for constraints in general
//			// no implementation required for ogdf
//		}//data

		if(constraintTag->getName() == ogmlTagNames[t_constraint]) {
		
			XmlAttributeObject* actAtt;
			String cId;
			String cType;
			
			if (constraintTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_id], actAtt))
				// set id of the constraint
				cId = actAtt->getValue();
			
			if (constraintTag->findXmlAttributeObjectByName(ogmlAttributeNames[a_type], actAtt))
				cType = actAtt->getValue();
			else {
			 	cerr << "Error: constraint doesn't own compulsive attribute \'type\' in valid parse tree!" << endl;
				return false;
			}
			// now we need a constraint manager to create a constraint 
			//  with the type of the name stored in cType
			// create the constraint
		    Constraint* c = ConstraintManager::createConstraintByName(G, &cType);
			// check if the constraintManager doesn't return a null pointer
			//  that occurs if cM doesn't know the constraint name
			if (c) {
				// let the constraint load itself
				if (c->buildFromOgml(constraintTag, &m_nodes)){
					// add constraint if true is returned
					GC.addConstraint(c);
				}
				else
					cerr << "Error while building constraint with name \""<<cType<<"\"!" << endl;
			}
			else
				cerr << "Error: constraint type \""<<cType<<"\" is unknown!" << endl;

		}//constraint

		// go to next constraint tag
		constraintTag = constraintTag->m_pBrother;
	}//while

	// terminated, so return true
	return true;	
		
}
*/



// ***********************************************************
//
// p u b l i c    r e a d     m e t h o d s
//
// ***********************************************************
bool OgmlParser::read(
					const char* fileName, 
					Graph &G, 
					ClusterGraph &CG){

	DinoXmlParser *p;

	try{
		// DinoXmlParser for parsing the ogml file
		p = new DinoXmlParser(fileName);
		p->createParseTree();
		// get root object of the parse tree
		const XmlTagObject *root = &p->getRootTag();
		// build the required hash tables
		OgmlParser::buildHashTables();
		// valide the document
		if ( validate(root, t_ogml) == vs_valid )	{
			OgmlParser::checkGraphType(root);
			// build graph
			if (buildGraph(G)){
				GraphType gt = getGraphType();
				// switch GraphType
				switch (gt){
				//normal graph
				case graph:
					break;
				// cluster graph
				case clusterGraph:
					// build cluster
					if (!buildCluster(root, G, CG))
						return false;
					break;
				//compound graph
				case compoundGraph:
					// build cluster because we got a cluster graph variable
					//  although we have a compound graph in the ogml file
					if (!buildCluster(root, G, CG))
						return false;
					break;		
				//corrupt compound graph
				case corruptCompoundGraph:
					// build cluster because we got a cluster graph variable
					//  although we have a corrupted compound graph in the ogml file
					if (!buildCluster(root, G, CG))
						return false;
					break;
				}//switch
			}
			else
				return false;
		}
		else
			return false;
	}// try
	catch(const char *error){
		cout << error << endl << flush;
		return false;	
	}//catch
		
	delete(p);
	return true;
};


bool OgmlParser::read(
					const char* fileName, 
					Graph &G, 
					ClusterGraph &CG, 
					ClusterGraphAttributes &CGA){

	DinoXmlParser *p;
	try{
		// DinoXmlParser for parsing the ogml file
		p = new DinoXmlParser(fileName);
		p->createParseTree();
		// get root object of the parse tree
		const XmlTagObject *root = &p->getRootTag();
		// build the required hash tables
		OgmlParser::buildHashTables();
		// valide the document
		if ( validate(root, t_ogml) == vs_valid ){
			OgmlParser::checkGraphType(root);
			// build graph
			if (buildGraph(G)){
				GraphType gt = getGraphType();
				// switch GraphType
				switch (gt){
				//normal graph
				case graph:
					if (!buildAttributedClusterGraph(G, CGA, root))
						return false;
					break;
				// cluster graph
				case clusterGraph:
					if (!buildCluster(root, G, CG))
						return false;
					if (!buildAttributedClusterGraph(G, CGA, root))
						return false;
					break;
				//compound graph
				case compoundGraph:
					// build cluster because we got a cluster graph variable
					//  although we have a compound graph in the ogml file
					if (!buildCluster(root, G, CG))
						return false;
					if (!buildAttributedClusterGraph(G, CGA, root))
						return false;
					break;
				//corrupt compound graph
				case corruptCompoundGraph:
					// build cluster because we got a cluster graph variable
					//  although we have a corrupted compound graph in the ogml file
					if (!buildCluster(root, G, CG))
						return false;
					if (!buildAttributedClusterGraph(G, CGA, root))
						return false;
				}//switch
			}
			else
				return false;
		}
		else
			return false;
	}// try
	catch(const char *error){
		cout << error << endl << flush;
		return false;	
	}//catch
		
	delete(p);
	return true;
};


//Commented out due to missing compound graph in OGDF
/*
bool OgmlParser::read(
					const char* fileName, 
					Graph &G, 
					CompoundGraph &CG,
					CompoundGraphAttributes &CGA){
						
	DinoXmlParser *p;
	try{
		// DinoXmlParser for parsing the ogml file
		p = new DinoXmlParser(fileName);
		p->createParseTree();
		// get root object of the parse tree
		XmlTagObject *root = &p->getRootTag();
		// build the required hash tables
		OgmlParser::buildHashTables();
		// valide the document
		if ( validate(root, t_ogml) == vs_valid ){
			OgmlParser::checkGraphType(root);
			// build graph
			if (buildGraphForCompounds(G)){
				GraphType gt = getGraphType();
				// switch GraphType
				switch (gt){
				//normal graph
				case graph:
					if (!buildAttributedCompoundGraph(G, CGA, root))
						return false;
					break;
				// cluster graph
				case clusterGraph:
					if (!buildCompound(root, G, CG))
						return false;
					if (!buildAttributedCompoundGraph(G, CGA, root))
						return false;
					break;
				//compound graph
				case compoundGraph:
					if (!buildCompound(root, G, CG))
						return false;
					if (!buildAttributedCompoundGraph(G, CGA, root))
						return false;
					break;
				//corrupt compound graph
				case corruptCompoundGraph:
					if (!buildCompound(root, G, CG))
						return false;
					if (!buildAttributedCompoundGraph(G, CGA, root))
						return false;
				}//switch
			}
			else
				return false;
		}
		else
			return false;
	}// try
	catch(const char *error){
		cout << error << endl << flush;
		return false;	
	}//catch
		
	delete(p);
	return true;
};


bool OgmlParser::read(
					const char* fileName, 
					Graph &G, 
					CompoundGraph &CG,
					CompoundGraphAttributes &CGA,
					GraphConstraints &GC){

	DinoXmlParser *p;
	try{
		// DinoXmlParser for parsing the ogml file
		p = new DinoXmlParser(fileName);
		p->createParseTree();
		// get root object of the parse tree
		XmlTagObject *root = &p->getRootTag();
		// build the required hash tables
		OgmlParser::buildHashTables();
		// valide the document
		if ( validate(root, t_ogml) == vs_valid ){
			OgmlParser::checkGraphType(root);
			// build graph
			if (buildGraphForCompounds(G)){
				GraphType gt = getGraphType();
				// switch GraphType
				switch (gt){
				//normal graph
				case graph:
					if (!buildAttributedCompoundGraph(G, CGA, root))
						return false;
					break;
				// cluster graph
				case clusterGraph:
					if (!buildCompound(root, G, CG))
						return false;
					if (!buildAttributedCompoundGraph(G, CGA, root))
						return false;
					break;
				//compound graph
				case compoundGraph:
					if (!buildCompound(root, G, CG))
						return false;
					if (!buildAttributedCompoundGraph(G, CGA, root))
						return false;
					break;
				//corrupt compound graph
				case corruptCompoundGraph:
					if (!buildCompound(root, G, CG))
						return false;
					if (!buildAttributedCompoundGraph(G, CGA, root))
						return false;
				}//switch
				// finally build constraints
				if (!buildConstraints(G, GC))
					return false;
			}
			else
				return false;

		}
		else
			return false;
	}// try
	catch(const char *error){
		cout <<"OgmlParser::read => " << error << endl << flush;
		return false;	
	}//catch
	delete(p);
	return true;
};

*/

}//namespace ogdf

