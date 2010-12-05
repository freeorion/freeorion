/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements graph generator for hierarchical graphs.
 * 
 * \author Carsten Gutwenger, Christoph Buchheim
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


#include <ogdf/basic/graph_generators.h>


namespace ogdf {


class BEdge {
public:
  int head, tail, id, pos;
  BEdge *next;
  BEdge(int t,int h,int c) : head(h), tail(t), id(c), pos(-1), next(0) {};
  OGDF_NEW_DELETE
};

typedef BEdge *bEdge;


int cmpId(const bEdge &a, const bEdge &b) {
  return (a->id < b->id ? -1 : (a->id > b->id ? 1 : 0));
}

class CmpTail {
public:
	static int compare(const bEdge &a, const bEdge &b) {
		return (a->tail < b->tail ? -1 : (a->tail > b->tail ? 1 : cmpId(a,b)));
	}
	OGDF_AUGMENT_STATICCOMPARER(bEdge)
};


class CmpHead {
public:
	static int compare(const bEdge &a, const bEdge &b) {
		return (a->head < b->head ? -1 : (a->head > b->head ? 1 : cmpId(a,b)));
	}
	OGDF_AUGMENT_STATICCOMPARER(bEdge)
};

void randomHierarchy(Graph &G,
	int numberOfNodes,int numberOfEdges,
	bool planar,bool singleSource,bool longEdges)
{
	G.clear();

	node *nnr = new node[3*numberOfNodes];
	int  *vrt = new int[3*numberOfNodes];
	int  *fst = new int[numberOfNodes+1];
	List<bEdge> startEdges;
	bEdge actEdge, nextEdge, toDelete;
	node v;
	int act, next, n1, n2, idc=0;
	double x1, x2, r;
	bool connected;

	/** Place nodes **/

	for(int i = 0; i < numberOfNodes; i++) 
		G.newNode();

	int numberOfLayers=0, totNumber=0, realCount=0;
	fst[0] = 0;
	forall_nodes(v,G) {
		if(longEdges&&numberOfLayers) vrt[totNumber++] = 1;

		nnr[totNumber] = v;
		vrt[totNumber++] = 0;
		realCount++;
		r = double(randomNumber(0,1000)) / 1000.0;
		if(totNumber == 1 && singleSource || realCount == numberOfNodes || r*r*numberOfNodes < 1)
		{
			if(longEdges && numberOfLayers)
				vrt[totNumber++] = 1;
			fst[++numberOfLayers] = totNumber;
		}
	}

	/** Determine allowed neighbours **/

	int *leftN  = new int[totNumber];
	int *rightN = new int[totNumber];
	for(int l = 1; l < numberOfLayers; l++)
	{
		if(planar) {
			n1 = fst[l-1];
			n2 = fst[l];
			leftN[n2] = n1;
			while(n1 < fst[l] && n2 < fst[l+1]) {
				r = double(randomNumber(0,1000)) / 1000.0;
				if(n1 != fst[l]-1 &&
					(n2 == fst[l+1]-1 ||
					r < (double)(fst[l]-fst[l-1])/(double)(fst[l+1]-fst[l-1])))
					n1++;
				else {
					rightN[n2] = n1;
					if(++n2 < fst[l+1])
						leftN[n2] = n1;
				}
			}
		}
		else
			for(n2 = fst[l]; n2 < fst[l+1]; n2++) {
				leftN [n2] = fst[l-1];
				rightN[n2] = fst[l]-1;
			}
	}

	/** Insert edges **/

	SList<bEdge> *edgeIn  = new SList<bEdge>[totNumber];
	SList<bEdge> *edgeOut = new SList<bEdge>[totNumber];
	if(numberOfLayers) {
		x1 = numberOfEdges;
		x2 = 0;
		for(n2 = fst[1]; n2 < totNumber; n2++)
			if(!vrt[n2])
				x2 += rightN[n2] - leftN[n2]+1;

		for(n2 = fst[1]; n2 < totNumber; n2++)
			if(!vrt[n2]) {
				connected = !singleSource;
				for(n1 = leftN[n2]; n1 <= rightN[n2] || !connected; n1++) {
					r = double(randomNumber(0,1000)) / 1000.0;
					if(r < x1/x2 || n1 > rightN[n2]) {
						next = (n1 <= rightN[n2] ? n1 : randomNumber(leftN[n2],rightN[n2]));
						act = n2;
						nextEdge = OGDF_NEW BEdge(next,act,idc++);
						while(vrt[next]) {
							act = next;
							next = randomNumber(leftN[act],rightN[act]);
							edgeOut[act].pushBack(nextEdge);
							nextEdge = OGDF_NEW BEdge(next,act,idc++);
							edgeIn[act].pushBack(nextEdge);
						}
						startEdges.pushBack(nextEdge);
						connected = 1;
						x1 -= 1;
					}
					if(n1<=rightN[n2])
						x2-=1;
				}
			}
		}

	delete[] leftN;
	delete[] rightN;

	if(planar) 
		for(act = 0; act < totNumber; act++) {
			CmpTail cmpTail;
			edgeIn[act].quicksort(cmpTail);
			CmpHead cmpHead;
			edgeOut[act].quicksort(cmpHead);
		}

	for(act = 0; act < totNumber; act++) {
		SListIterator<bEdge> it;
		for(it = edgeIn[act].begin(); it.valid(); ++it) {
			nextEdge = *it;
			nextEdge->next = edgeOut[act].popFrontRet();
		}
	}

	delete[] edgeOut;

	ListIterator<bEdge> it;
	for(it = startEdges.begin(); it.valid(); ++it) {
		actEdge = *it;
		nextEdge = actEdge;
		while(vrt[nextEdge->head])
			nextEdge = nextEdge->next;
		G.newEdge(nnr[actEdge->tail], nnr[nextEdge->head]);
	}

	/** Clean up **/
	for(it = startEdges.begin(); it.valid(); ++it) {
		nextEdge = *it;
		toDelete = nextEdge;
		while(vrt[nextEdge->head]) {
			nextEdge = nextEdge->next;
			delete toDelete;
			toDelete = nextEdge;
		}
		delete toDelete;
	}

	delete[] edgeIn;
	delete[] fst;
	delete[] vrt;
	delete[] nnr;
}


} // end namespace ogdf
