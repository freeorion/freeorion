/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief declaration and implementation of the third phase of sugiyama
 * 
 * \author Sebastian Leipert
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

#ifndef OGDF_FAST_HIERARCHY_LAYOUT_H
#define OGDF_FAST_HIERARCHY_LAYOUT_H



#include <ogdf/module/HierarchyLayoutModule.h>
#include <ogdf/basic/List.h>


namespace ogdf {

#define ALLOW .00001
	

/**
 * \brief Stores a pair of an integer and a double. 
 *
 * This class is used by the class kList.
 */ 
class withKey {

public:

  int element;
  double key;

  withKey &operator=(const withKey& wk) {
    element=wk.element;
    key=wk.key;
    return *this;
  }
  friend ostream& operator<<(ostream& out,const withKey& wk) {
    out<<wk.element<<"("<<wk.key<<")";
    return out;
  }
  friend istream& operator>>(istream& in,withKey& wk) {
    in>>wk.element>>wk.key;
    return in;
  }
};




class cmpWithKey {
public:
	static int compare(const withKey &wk1, const withKey &wk2) {
		if(wk1.key<wk2.key) return -1;
		if(wk1.key>wk2.key) return 1;
		return 0;
	}
	OGDF_AUGMENT_STATICCOMPARER(withKey)
};





/**
 * \brief Class kList extends the class List by functions needed in the FastHierarchLayout algorithm. 
 *
 * Especially, it computes the median of a list and reduces it.
 */
class kList : public List<withKey> {

public:

	bool pop(int& e,double& k) {
		if(empty()) return 0;
		withKey wk=popFrontRet();
		e=wk.element;
		k=wk.key;
		return 1;
	}

	double peek() {
		return front().key;
	}

	void add(int e,double k) {
		withKey wK;
		wK.element=e;
		wK.key=k;
		pushBack(wK);
	}

	double median() const {
		int sz = size();
		if(!sz) 
			return 0;
		ListConstIterator<withKey> it = get(sz/2);
		double k = (*it).key;
		if (sz == 2*(int) (sz/2)) 
			k = (k + (*it.pred()).key) / 2;
		return k;
	}


	//! Scans the list for pairs of elements with the same double key.
	/**
	 * Replaces them by one element. If integer key is 0, it removes element from list. 
	 * Precondition : list is sorted. 
	 */
	void reduce(kList& newList) {
		if(empty()) return;
		withKey oldWK,newWK;
		newWK = oldWK = popFrontRet();
		
		while(!empty()) {
			oldWK = popFrontRet();
			if ((oldWK.key) > (newWK.key) + ALLOW ||
				(oldWK.key) < (newWK.key) - ALLOW) {
				if(newWK.element) 
					newList.pushBack(newWK);
					newWK = oldWK;
			}
			else 
				newWK.element += oldWK.element;
		}
		if(newWK.element) 
			newList.pushBack(newWK);
	}

	//! remove first element and return it
	withKey popFrontRet() {
		//CG: not referenced: int s = List<withKey>::size();
		withKey t = List<withKey>::front();
		List<withKey>::popFront();
		return t;
	}

};



/**
 * \brief Coordinate assignment phase for the Sugiyama algorithm by Buchheim et al..
 *
 * This class implements a hierarchy layout algorithm, i.e., it layouts
 * hierarchies with a given order of nodes on each layer. It is used as a third
 * phase of the Sugiyama algorithm.
 * 
 * All edges of the layout will have at most two bends. Additionally,
 * for each edge having exactly two bends, the segment between them is 
 * drawn vertically. This applies in particular to the long edges
 * arising in the first phase of the Sugiyama algorithm.
 *
 * The implementation is based on:
 *
 * Christoph Buchheim, Michael J�nger, Sebastian Leipert: <i>A Fast %Layout
 * Algorithm for k-Level Graphs</i>. LNCS 1984 (Proc. %Graph Drawing 2000),
 * pp. 229-240, 2001.
 * 
 * <h3>Optional Parameters</h3>
 * 
 * <table>
 *   <tr>
 *     <th>Option</th><th>Type</th><th>Default</th><th>Description</th>
 *   </tr><tr>
 *     <td><i>node distance</i></td><td>double</td><td>3.0</td>
 *     <td>the minimal horizontal distance between two nodes on the same layer</td>
 *   </tr><tr>
 *     <td><i>layer distance</i></td><td>double</td><td>3.0</td>
 *     <td>the minimal vertical distance between two nodes on neighbored layers</td>
 *   </tr><tr>
 *     <td><i>fixed layer distance</i></td><td>bool</td><td>false</td>
 *     <td>if true, the distance between neighbored layers is fixed, otherwise variable</td>
 *   </tr>
 * </table>
 */
class OGDF_EXPORT FastHierarchyLayout : public HierarchyLayoutModule
{
protected:

	void doCall(const Hierarchy& H,GraphCopyAttributes &AGC);

public:
	//! Creates an instance of fast hierarchy layout.
	FastHierarchyLayout();

	//! Copy constructor.
	FastHierarchyLayout(const FastHierarchyLayout &);

	// destructor
	virtual ~FastHierarchyLayout();


	//! Assignment operator
	FastHierarchyLayout &operator=(const FastHierarchyLayout &);


	//! Returns the option <i>node distance</i>.
	double nodeDistance() const;

	//! Sets the option node distance to \a x.
	void nodeDistance(double x);

	//! Returns the option <i>layer distance</i>.
	double layerDistance() const;

	//! Sets the option layer distance to \a x.
	void layerDistance(double x);

	//! Returns the option <i>fixed layer distance</i>.
	bool fixedLayerDistance() const;

	//! Sets the option fixed layer distance to \a b.
	void fixedLayerDistance(bool b);


private:

	int n;      //!< The number of nodes including virtual nodes.
	int m;      //!< The number edge sections.
    int k;      //!< The number of layers.
	int *layer; //!< Stores for every node its layer.
	int *first; //!< Stores for every layer the index of the first node.


    // nodes are numbered top down and from left to right.
    // Is called "internal numbering".
    // Nodes and Layeras are number 0 to n-1 and 0 to k-1, respectively.                 
    // For thechnical reasons we set first[k] to n. 

	/**
	 * \brief The list of neighbors in previous / next layer.
	 *
	 * for every node : adj[0][node] list of neighbors in previous layer;
	 * for every node : adj[1][node] list of neighbors in next layer
	 */
	List<int> *adj[2];

	/**
	 * \brief The nodes belonging to a long edge.
	 *
	 * for every node : longEdge[node] is a pointer to a list containing all
	 * nodes that belong to the same long edge as node.
	 */
	List<int> **longEdge;

	double m_minNodeDist; //!< The minimal node distance on a layer.
	double m_minLayerDist;//!< The minimal distance between layers.
	double *breadth;      //!< for every node : breadth[node] = width of the node.
	double *height;       //!< for every layer : height[layer] = height of max{height of node on layer}.
	double *y;            //!< for every layer : y coordinate of layer.
	double *x;            //!< for every node : x coordinate of node.
	/**
	 * for every node : minimal possible distance between the center of a node
	 * and first[layer[node]].
	 */
	double *totalB;
    
	double *mDist; //!< Similar to totalB, used for temporary storage.

	bool m_fixedLayerDist; //!< 0 if distance between layers should be variable, 1 otherwise.
	bool *virt; //!< for every node : virt[node] = 1 if node is virtual, 0 otherwise.

	cmpWithKey _cmp; //!< Needed for sorting lists parameterized with withKey.

	void decrTo(double&,double);
	void incrTo(double&,double);
	bool sameLayer(int,int) const;
	bool isFirst(int) const;
	bool isLast(int) const;
	void sortLongEdges(int,int,double*,bool&,double&,int*,bool*);
	bool placeSingleNode(int,int,int,double&,int);
	void placeNodes(int,int,int,int,int);
	void moveLongEdge(int,int,bool*);
	void straightenEdge(int,bool*);
	void findPlacement();
};

} // end namespace ogdf


#endif
