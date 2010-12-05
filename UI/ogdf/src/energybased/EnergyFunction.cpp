/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief TO DO: DESCRIBE WHAT IS IMPLEMENTED
 * 
 * \author Rene Weiskircher
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


#include <ogdf/internal/energybased/EnergyFunction.h>
namespace ogdf {

EnergyFunction::EnergyFunction(const String &funcname, GraphAttributes &AG) :
m_G(AG.constGraph()),
m_name(funcname),
m_candidateEnergy(0),
m_energy(0),
m_AG(AG),
m_testNode(NULL),
m_testPos(0.0,0.0)
 {}


void EnergyFunction::candidateTaken() {
	m_energy=m_candidateEnergy;
	m_candidateEnergy = 0.0;
	m_AG.x(m_testNode)=m_testPos.m_x;
	m_AG.y(m_testNode)=m_testPos.m_y;
	m_testPos = DPoint(0.0,0.0);
	internalCandidateTaken();
	m_testNode=NULL;
}

double EnergyFunction::computeCandidateEnergy(const node v, const DPoint &testPos)
{
	m_testPos = testPos;
	m_testNode = v;
	compCandEnergy();
	OGDF_ASSERT(m_candidateEnergy >= 0.0);
	return m_candidateEnergy;
}
#ifdef OGDF_DEBUG
void EnergyFunction::printStatus() const{
	cout << "\nEnergy function name: " << m_name;
	cout << "\nCurrent energy: " << m_energy;
	node v;
	cout << "\nPosition of nodes in current solution:";
	NodeArray<int> num(m_G);
	int count = 1;
	forall_nodes(v,m_G) num[v] = count ++;
	forall_nodes(v,m_G) {
		cout << "\nNode: " << num[v] << " Position: " << currentPos(v);
	}
	cout << "\nTest Node: " << m_testNode << " New coordinates: " << m_testPos;
	cout << "\nCandidate energy: " << m_candidateEnergy;
	printInternalData();
}
#endif

} //namespace
