/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Contains diverse enumerations and string constants.
 *        See comments for further information.
 * 
 * \author Christian Wolf
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

#ifndef OGDF_OGML_H
#define OGDF_OGML_H

#include <ogdf/basic/String.h>

namespace ogdf
{


/**Sizes of vectors below.
 */
  const int TAG_NUM = 47, ATT_NUM = 48, ATT_VAL_NUM = 131;

// max count of ogml tags
  const int MAX_TAG_COUNT = 4000;

/**This enumeration is used for fast switch-case statements of the Ogml parser and
 * to identify Ogml tags.
 */
  enum OgmlTagId
  {
    t_none = -1,
    t_bool,
    t_composed,
    t_constraint,
    t_constraints,
    t_content,
    t_data,
    t_default,
    t_edge,
    t_edgeRef,
    t_edgeStyle,
    t_edgeStyleTemplate,
    t_edgeStyleTemplateRef,	//tag template within tag edgeStyle/-Template
    t_endpoint,
    t_fill,
    t_font,
    t_graph,
    t_graphStyle,
    t_int,
    t_label,
    t_labelRef,
    t_labelStyle,
    t_labelStyleTemplate,
    t_labelStyleTemplateRef,	//tag template within tag labelStyle/-Template 
    t_layout,
    t_line,
    t_location,
    t_node,
    t_nodeRef,
    t_nodeStyle,
    t_nodeStyleTemplate,
    t_nodeStyleTemplateRef,	//tag template within tag nodeStyle/-Template
    t_num,
    t_ogml,
    t_point,
    t_port,
    t_segment,
    t_shape,
    t_source,
    t_sourceStyle,
    t_string,
    t_structure,
    t_styles,
    t_styleTemplates,
    t_target,
    t_targetStyle,
    t_text,
    t_image
  };

/**This vector contains the real names of all Ogml tags.
 */
  const String ogmlTagNames[] = {
    //"none"
    "bool",
    "composed",
    "constraint",
    "constraints",
    "content",
    "data",
    "default",
    "edge",
    "edgeRef",
    "edgeStyle",
    "edgeStyleTemplate",
    "template",
    "endpoint",
    "fill",
    "font",
    "graph",
    "graphStyle",
    "int",
    "label",
    "labelRef",
    "labelStyle",
    "labelStyleTemplate",
    "template",
    "layout",
    "line",
    "location",
    "node",
    "nodeRef",
    "nodeStyle",
    "nodeStyleTemplate",
    "template",
    "num",
    "ogml",
    "point",
    "port",
    "segment",
    "shape",
    "source",
    "sourceStyle",
    "string",
    "structure",
    "styles",
    "styleTemplates",
    "target",
    "targetStyle",
    "text",
    "image"
  };

/**This enumeration is used for fast switch-case statements of the Ogml parser and
 * to identify Ogml attributes.
 */
  enum OgmlAttributeId
  {
    a_none = -1,
    a_alignment,
    a_angle,
    a_color,
    a_decoration,
    a_defaultEdgeTemplate,
    a_defaultLabelTemplate,
    a_defaultNodeTemplate,
    a_family,
    a_height,
    a_id,			//id attribute
    a_nodeIdRef,		//attribute idRef of elements source, target, nodeRef, nodeStyle
    a_edgeIdRef,		//attribute idRef of elements edgeRef, edgeStyle
    a_labelIdRef,		//attribute idRef of elements edgeRef, edgeStyle
    a_sourceIdRef,		//attribute idRef of element endpoint
    a_targetIdRef,		//attribute idRef of element endpoint
    a_nodeStyleTemplateIdRef,	//attribute idRef of subelement template of element nodeStyle
    a_edgeStyleTemplateIdRef,	//attribute idRef of subelement template of element edgeStyle
    a_labelStyleTemplateIdRef,	//attribute idRef of subelement template of element labelStyle
    a_endpointIdRef,		//attribute idRef of subelement endpoint of element segment
    a_name,
    a_nLineType,		//attribute type of subelement line of tag nodeStyleTemplate
    a_nShapeType,		//attribute type of subelement shape of tag nodeStyleTemplate
    a_pattern,
    a_patternColor,
    a_rotation,
    a_size,
    a_stretch,
    a_style,
    a_transform,
    a_type,			//attribute type of subelements source-/targetStyle of tag edgeStyleTemplate
    a_uri,
    a_intValue,
    a_boolValue,
    a_numValue,
    a_variant,
    a_weight,
    a_width,
    a_x,
    a_y,
    a_z,
	a_imageUri,
	a_imageStyle,
	a_imageAlignment,
	a_imageDrawLine,
	a_imageWidth,
	a_imageHeight,
	a_constraintType,
	a_disabled
  };

/**This vector contains the real names of all Ogml attributes.
 */
const String ogmlAttributeNames[] = {
    "alignment",
    "angle",
    "color",
    "decoration",
    "defaultEdgeTemplate",
    "defaultLabelTemplate",
    "defaultNodeTemplate",
    "family",
    "height",
    "id",			//id attribute
    "idRef",			//attribute idRef of elements source, target, nodeRef, nodeStyle
    "idRef",			//attribute idRef of elements edgeRef, edgeStyle
    "idRef",			//attribute idRef of elements edgeRef, edgeStyle
    "idRef",			//attribute idRef of element endpoint
    "idRef",			//attribute idRef of element endpoint
    "idRef",			//attribute idRef of subelement template of element nodeStyle
    "idRef",			//attribute idRef of subelement template of element edgeStyle
    "idRef",			//attribute idRef of subelement template of element labelStyle
    "idRef",			//attribute idRef of subelement endpoint of element segment
    "name",
    "type",			//attribute type of subelement line of tag nodeStyleTemplate
    "type",			//attribute type of subelement shape of tag nodeStyleTemplate
    "pattern",
    "patternColor",
    "rotation",
    "size",
    "stretch",
    "style",
    "transform",
    "type",			//attribute type of subelements source-/targetStyle of tag edgeStyleTemplate
    "uri",
    "value",
    "value",
    "value",
    "variant",
    "weight",
    "width",
    "x",
    "y",
    "z",
	"uri",
	"style",
	"alignment",
	"drawLine",
	"width",
	"height",
	"type",
	"disabled"
  };

/**This enumeration is used for fast switch-case statements of the Ogml parser and
 * to identify Ogml attributes.
 */
  enum OgmlAttributeValueId
  {
    av_any = 0,			//for any attributeValue
    av_blink,
    av_bold,
    av_bolder,
    av_bool,
    av_box,
    av_capitalize,
    av_center,
    av_checked,
    av_circle,
    av_condensed,
    av_cursive,
    av_dashed,
	av_esNoPen,	// values for line style
	av_esSolid,
	av_esDash,
	av_esDot,
	av_esDashdot,
	av_esDashdotdot,
    av_diamond,
    av_dotted,
    av_double,
    av_doubleSlash,
    av_ellipse,
    av_expanded,
    av_extraCondensed,
    av_extraExpanded,
    av_fantasy,
    av_filledBox,
    av_filledCircle,
    av_filledDiamond,
    av_filledHalfBox,
    av_filledHalfCircle,
    av_filledHalfDiamond,
    av_filledHalfRhomb,
    av_filledRhomb,
    av_smurf,
    av_arrow,
    av_groove,
    av_halfBox,
    av_halfCircle,
    av_halfDiamond,
    av_halfRhomb,
    av_hexagon,
    av_hex,			//hexadecimal value
    av_id,
    av_nodeIdRef,		//attribute idRef of elements source, target, nodeRef, nodeStyle
    av_edgeIdRef,		//attribute idRef of elements edgeRef, edgeStyle
    av_labelIdRef,		//attribute idRef of elements edgeRef, edgeStyle
    av_sourceIdRef,		//attribute idRef of element endpoint
    av_targetIdRef,		//attribute idRef of element endpoint
    av_nodeStyleTemplateIdRef,	//attribute idRef of subelement template of element nodeStyle
    av_edgeStyleTemplateIdRef,	//attribute idRef of subelement template of element edgeStyle
    av_labelStyleTemplateIdRef,	//attribute idRef of subelement template of element labelStyle
    av_pointIdRef,		//attribute idRef of subelement endpoint of element segment 
    av_image,
    av_inset,
    av_int,			//integer value
    av_italic,
    av_justify,
    av_left,
    av_lighter,
    av_line,
    av_lineThrough,
    av_lowercase,
    av_lParallelogram,
    av_monospace,
    av_narrower,
    av_none,
    av_normal,
    av_num,			//real value
    av_oblique,
    av_oct,
    av_octagon,
    av_outset,
    av_overline,
    av_pentagon,
    av_rect,
    av_rectSimple,
    av_rhomb,
    av_ridge,
    av_right,
    av_rParallelogram,
    av_sansSerif,
    av_semiCondensed,
    av_semiExpanded,
    av_serif,
    av_slash,
    av_smallCaps,
    av_solid,
    av_bpNone, // values for node patterns
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
    av_bpDiagonalCross,
    av_string,
    av_striped,
    av_trapeze,
    av_triangle,
    av_triple,
    av_ultraCondensed,
    av_ultraExpanded,
    av_umlClass,
    av_underline,
    av_uppercase,
    av_upTrapeze,
    av_uri,
    av_wider,
	av_freeScale,   // image-style
	av_fixScale,	// image-style
	av_topLeft,		// image-alignemnt
	av_topCenter,	// image-alignemnt
	av_topRight,	// image-alignemnt
	av_centerLeft,	// image-alignemnt
//	av_center,	// just defined	// image-alignemnt
	av_centerRight,	// image-alignemnt
	av_bottomLeft,	// image-alignemnt
	av_bottomCenter,// image-alignemnt
	av_bottomRight,	// image-alignemnt
// Constraint-Types:
	av_constraintAlignment,
	av_constraintAnchor,
	av_constraintSequence
  };

/**This vector contains the real names of all Ogml values of attributes.
 */
  const String ogmlAttributeValueNames[] = {
    "any",			//for any attributeValue
    "blink",
    "bold",
    "bolder",
    "bool",
    "box",
    "capitalize",
    "center",
    "checked",
    "circle",
    "condensed",
    "cursive",
    "dashed",
	"esNoPen",	// values for line style
	"esSolid",
	"esDash",
	"esDot",
	"esDashdot",
	"esDashdotdot",
    "diamond",
    "dotted",
    "double",
    "doubleSlash",
    "ellipse",
    "expanded",
    "extraCondensed",
    "extraExpanded",
    "fantasy",
    "filledBox",
    "filledCircle",
    "filledDiamond",
    "filledHalfBox",
    "filledHalfCircle",
    "filledHalfDiamond",
    "filledHalfRhomb",
    "filledRhomb",
    "smurf",
    "arrow",
    "groove",
    "halfBox",
    "halfCircle",
    "halfDiamond",
    "halfRhomb",
    "hexagon",
    "hex",
    "id",
    "nodeId",			//attribute idRef of elements source, target, nodeRef, nodeStyle
    "edgeId",			//attribute idRef of elements edgeRef, edgeStyle
    "labelId",			//attribute idRef of elements edgeRef, edgeStyle
    "sourceId",			//attribute idRef of element endpoint
    "targetId",			//attribute idRef of element endpoint
    "nodeStyleTemplateId",	//attribute idRef of subelement template of element nodeStyle
    "edgeStyleTemplateId",	//attribute idRef of subelement template of element edgeStyle
    "labelStyleTemplateId",	//attribute idRef of subelement template of element labelStyle
    "pointId",			//attribute idRef of subelement endpoint of element segment
    "image",
    "inset",
    "int",
    "italic",
    "justify",
    "left",
    "lighter",
    "line",
    "lineThrough",
    "lowercase",
    "lParallelogram",
    "monospace",
    "narrower",
    "none",
    "normal",
    "num",
    "oblique",
    "oct",
    "octagon",
    "outset",
    "overline",
    "pentagon",
    "rect",
    "rectSimple",
    "rhomb",
    "ridge",
    "right",
    "rParallelogram",
    "sansSerif",
    "semiCondensed",
    "semiExpanded",
    "serif",
    "slash",
    "smallCaps",
    "solid",
    "bpNone", // values for node patterns
    "bpSolid",
    "bpDense1",
    "bpDense2",
    "bpDense3",
    "bpDense4",
    "bpDense5",
    "bpDense6",
    "bpDense7",
    "bpHorizontal",
    "bpVertical",
    "bpCross",
    "bpBackwardDiagonal",
    "bpForwardDiagonal",
    "bpDiagonalCross",
    "string",
    "striped",
    "trapeze",
    "triangle",
    "triple",
    "ultraCondensed",
    "ultraExpanded",
    "umlClass",
    "underline",
    "uppercase",
    "upTrapeze",
    "uri",
    "wider",
	"freeScale",   	// image-style
	"fixScale",		// image-style
	"topLeft",		// image-alignemnt
	"topCenter",	// image-alignemnt
	"topRight",		// image-alignemnt
	"centerLeft",	// image-alignemnt
//	"center",	// just defined	// image-alignemnt
	"centerRight",	// image-alignemnt
	"bottomLeft",	// image-alignemnt
	"bottomCenter",	// image-alignemnt
	"bottomRight",	// image-alignemnt
	"Alignment",
	"Anchor",
	"Sequence"
  };

/**This enumeration is used for encoding diverse validity stati of tags and attributes 
 * after parsing and validating a Xml file accorind to Ogml.
 */
  enum ValidityState
  {
    vs_tagEmptIncl = -10,	//empty tag inclusion
    vs_idNotUnique = -9,	//id already exhausted
    vs_idRefErr = -8,		//referenced id wasn't found or wrong type of referenced tag
    vs_unexpTag = -7,		//tag unexpected
    vs_unexpAtt = -6,		//attribute unexpected
    vs_expTagNotFound = -5,	//expected tag not found
    vs_expAttNotFound = -4,	//expected attribute not found
    vs_attValueErr = -3,	//attribute-value error
    vs_cardErr = -2,		//tag/attribute cardinality error
    vs_invalid = -1,		//tag/attribute is invalid (no detailled information)
    vs_valid = 1		//tag/attribute is valid
  };

/**The parser has to identify which graph type is present.
 * This enumeration allows simple distinction.
 */
  enum GraphType
  {
    graph,
    clusterGraph,
    compoundGraph,
    corruptCompoundGraph
  };

  const String graphTypeS[] = {
  	"graph", 
  	"clusterGraph", 
  	"compoundGraph", 
  	"corruptCompoundGraph"
  };

};				//namspace ogdf

#endif //OGDF_OGML_H
