/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Provide an interface for edge label information
 * 
 * \author Karsten Klein
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

#ifndef OGDF_E_LABEL_INTERFACE_H
#define OGDF_E_LABEL_INTERFACE_H

#include <ogdf/orthogonal/OrthoLayout.h>
#include <ogdf/basic/GridLayout.h>
#include <ogdf/basic/GridLayoutMapped.h>
#include <ogdf/planarity/PlanRepUML.h>


namespace ogdf {

//********************************************************
//global defs
//the available labels
//the five basic labels are not allowed to be changed,
//cause the have a special meaning/position, insert
//other labels between mult1/End2
const int labelNum = 5;
//Note: eLabelTyp is used for index computation (increment +1) in arrays
//of type 0..labelNum, whereas eUsedLabels can be used as bitfield
//for matching, setting and comparison (|-op)
enum eLabelTyp {elEnd1, elMult1, elName, elEnd2, elMult2};
enum eUsedLabels {lName = 4, lEnd1 = 1, lMult1 = 2, lEnd2 = 8, lMult2 = 16, lAll = 31};


//*************************************
//the basic single label defining class
//holds info about all labels for one edge
template <class coordType>
class OGDF_EXPORT EdgeLabel {

public:

    //construction and destruction
    EdgeLabel() {m_edge = 0; m_usedLabels = 0;}
    //bit pattern 2^labelenumpos bitwise
    EdgeLabel(edge e, int usedLabels = lAll) : m_usedLabels(usedLabels) 
    {
      m_edge = e;
      for(int i = 0; i < labelNum; i++)
      {
		//zu testzwecken randoms
        m_xSize[i] = double(randomNumber(5,13))/50.0; //1
        m_ySize[i] = double(randomNumber(3,7))/50.0;  //1

        m_xPos[i] = 0;
        m_yPos[i] = 0;
      }//for
    
    }//constructor
	//direkte Groesseneingabe, Felder der Laenge labelnum
	EdgeLabel(edge e, coordType w[], coordType h[], int usedLabels = lAll) : m_usedLabels(usedLabels) 
    {
      m_edge = e;
      for(int i = 0; i < labelNum; i++)
      {
        m_xSize[i] = w[i];
        m_ySize[i] = h[i];
        m_xPos[i] = 0;
        m_yPos[i] = 0;
      }//for
    
    }//constructor

    EdgeLabel(edge e, coordType w, coordType h, int usedLabels) : m_usedLabels(usedLabels)
    {
        m_edge = e;
        for (int i = 0; i < labelNum; i++)
            if (m_usedLabels & (1 << i)) {
                m_xPos[i] = 0.0;
                m_yPos[i] = 0.0;
                m_xSize[i] = w;
                m_ySize[i] = h;
            }
    }

    ~EdgeLabel() {}

    //copy constructor
    inline EdgeLabel(const EdgeLabel& rhs)
    {
      m_usedLabels = rhs.m_usedLabels;
      m_edge = rhs.m_edge;
      int i;
      for(i = 0; i < labelNum; i++)
      {
        m_xPos[i] = rhs.m_xPos[i];
        m_yPos[i] = rhs.m_yPos[i];
        m_xSize[i] = rhs.m_xSize[i];
        m_ySize[i] = rhs.m_ySize[i];
      }
    }//copy con
    //assignment
    inline EdgeLabel& operator=(const EdgeLabel& rhs)
    {
      if ( this != &rhs)
      {
         m_usedLabels = rhs.m_usedLabels;
         m_edge = rhs.m_edge;
        int i;
        for(i = 0; i < labelNum; i++)
		{
          m_xPos[i] = rhs.m_xPos[i];
          m_yPos[i] = rhs.m_yPos[i];
          m_xSize[i] = rhs.m_xSize[i];
          m_ySize[i] = rhs.m_ySize[i];
		}
      }
      return *this;
    }//assignment

    inline EdgeLabel& operator|=(const EdgeLabel& rhs)
    {
        if (m_edge) {
            OGDF_ASSERT(m_edge == rhs.m_edge);
        }
        else
            m_edge = rhs.m_edge;
        if (this != &rhs)
        {
            m_usedLabels |= rhs.m_usedLabels;
            for (int i = 0; i < labelNum; i++)
                if (rhs.m_usedLabels & (1 << i)) {
                    m_xPos[i] = rhs.m_xPos[i];
                    m_yPos[i] = rhs.m_yPos[i];
                    m_xSize[i] = rhs.m_xSize[i];
                    m_ySize[i] = rhs.m_ySize[i];
                }
        }
        return *this;
    }


    //set
    void setX(eLabelTyp elt, coordType x) {m_xPos[elt] = x;}
    void setY(eLabelTyp elt, coordType y) {m_yPos[elt] = y;}
    void setHeight(eLabelTyp elt, coordType h) {m_ySize[elt] = h;}
    void setWidth(eLabelTyp elt, coordType w) {m_xSize[elt] = w;}
    void setEdge(edge e) {m_edge = e;}
    void addType(eLabelTyp elt) { m_usedLabels |= (1<<elt); }

    //get
    coordType getX(eLabelTyp elt) {return m_xPos[elt];}
    coordType getY(eLabelTyp elt) {return m_yPos[elt];}
    coordType getWidth(eLabelTyp elt) {return m_xSize[elt];}
    coordType getHeight(eLabelTyp elt) {return m_ySize[elt];}
    edge theEdge() {return m_edge;}
	bool usedLabel(eLabelTyp elt) 
		{return ( ( m_usedLabels & (1<<elt) )>0 );}

    int &usedLabel()
    { return m_usedLabels; }



private:
     

    //the positions of the labels
    coordType m_xPos[labelNum];
    coordType m_yPos[labelNum];

    //the input label sizes
    coordType m_xSize[labelNum];
    coordType m_ySize[labelNum];

    //which labels have to be placed bit pattern 2^labelenumpos bitwise
    int m_usedLabels; //1 = only name, 5 = name and end2, ...

    //the edge of heaven
    edge m_edge;

	//the label text
	//String m_string;


};//edgelabel


//*********************
//Interface to algorithm
template <class coordType>
class ELabelInterface {

public:
    //constructor
    ELabelInterface(PlanRepUML& pru) 
    {
		//the PRU should not work with real world data but with 
		//normalized integer values
		m_distDefault = 2; 
		m_minFeatDist = 1;
		m_labels.init(pru.original());
		m_ug = 0;

		//temporary
		edge e;
		forall_edges(e, pru.original())
			setLabel(e, EdgeLabel<coordType>(e, 0));
    }
    //constructor on GraphAttributes
    ELabelInterface(GraphAttributes& uml) : m_ug(&uml)
    {
		//the GraphAttributes should work on real world data,
		//which can be floats or ints
		m_distDefault = 0.002; 
		m_minFeatDist = 0.003;
		m_labels.init(uml.constGraph());

		//temporary
		edge e;
		forall_edges(e, uml.constGraph())
			setLabel(e, EdgeLabel<coordType>(e, 0));
    }

    GraphAttributes& graph() {return *m_ug;}

    //set new EdgeLabel 
    void setLabel(const edge &e, const EdgeLabel<coordType>& el)
        {m_labels[e] = el;}

    void addLabel(const edge &e, const EdgeLabel<coordType>& el)
    { m_labels[e] |= el; }

    //get info about current EdgeLabel
    EdgeLabel<coordType>& getLabel(edge e) {return m_labels[e];}

    coordType getWidth(edge e, eLabelTyp elt)
        {return m_labels[e].getWidth(elt);}
    coordType getHeight(edge e, eLabelTyp elt)
        {return m_labels[e].getHeight(elt);}

    //get general information
    coordType& minFeatDist() {return m_minFeatDist;}
    coordType& distDefault() {return m_distDefault;}

private:

    EdgeArray<EdgeLabel<coordType> > m_labels; //holds all labels for original edges
    //the base graph
    GraphAttributes* m_ug;

    coordType m_distDefault; //default distance label/edge for positioner
    coordType m_minFeatDist; //min Distance label/feature in candidate posit.
};//ELabelInterface


}//end namespace

#endif
