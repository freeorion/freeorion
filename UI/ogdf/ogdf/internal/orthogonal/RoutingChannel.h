/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class RoutingChannel which maintains
 *        required size of routing channels and separation, cOverhang.
 * 
 * \author Carsten Gutwenger
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


#ifndef OGDF_ROUTING_CHANNEL_H
#define OGDF_ROUTING_CHANNEL_H


#include <ogdf/orthogonal/OrthoRep.h>


namespace ogdf {

//---------------------------------------------------------
// RoutingChannel
// maintains input sizes for constructive compaction (size
// of routing channels, separation, cOverhang)
//---------------------------------------------------------
template<class ATYPE>
class RoutingChannel
{
public:
	// constructor
	RoutingChannel(const Graph &G, ATYPE sep, double cOver) :
		m_channel(G), m_separation(sep), m_cOverhang(cOver) { }

	// size of routing channel of side dir of node v
	const ATYPE &operator()(node v, OrthoDir dir) const {
		return m_channel[v].rc[dir];
	}

	ATYPE &operator()(node v, OrthoDir dir) {
		return m_channel[v].rc[dir];
	}

	// returns separation (minimum distance between vertices/edges)
	ATYPE separation() const {
		return m_separation;
	}

	// returns cOverhang (such that overhang = separation * cOverhang)
	double cOverhang() const {
		return m_cOverhang;
	}

	// returns overhang (distance between vertex corners and edges)
	ATYPE overhang() const {
		return ATYPE(m_cOverhang * m_separation);
	}

	void computeRoutingChannels(const OrthoRep &OR, bool align = false)
	{
		const Graph &G = OR;

		node v;
		forall_nodes(v,G)
		{
			const OrthoRep::VertexInfoUML *pInfo = OR.cageInfo(v);

			if (pInfo) {
				const OrthoRep::SideInfoUML &sNorth = pInfo->m_side[odNorth];
				const OrthoRep::SideInfoUML &sSouth = pInfo->m_side[odSouth];
				const OrthoRep::SideInfoUML &sWest  = pInfo->m_side[odWest];
				const OrthoRep::SideInfoUML &sEast  = pInfo->m_side[odEast];

				(*this)(v,odNorth) = computeRoutingChannel(sNorth,sSouth,align);
				(*this)(v,odSouth) = computeRoutingChannel(sSouth,sNorth,align);
				(*this)(v,odWest ) = computeRoutingChannel(sWest ,sEast ,align);
				(*this)(v,odEast ) = computeRoutingChannel(sEast ,sWest ,align);
			}
		}
	}

private:
	// computes required size of routing channel at side si with opposite side siOpp
	int computeRoutingChannel(
		const OrthoRep::SideInfoUML &si,
		const OrthoRep::SideInfoUML &siOpp,
		bool align = false)
	{
		if (si.m_adjGen == 0)
		{
			int k = si.m_nAttached[0];
			if (k == 0 || 
				((k == 1 && siOpp.totalAttached() == 0) && !align) )
				return 0;
			else
				return (k+1)*m_separation;

		} else {
			int m = max(si.m_nAttached[0],si.m_nAttached[1]);
			if (m == 0)
				return 0;
			else
				return (m+1)*m_separation;
		}
	}

	struct vInfo {
		ATYPE rc[4];
		vInfo() {
			rc[0] = rc[1] = rc[2] = rc[3];
		}
	};

	NodeArray<vInfo> m_channel;
	ATYPE m_separation;
	double m_cOverhang;
};


} // end namespace ogdf


#endif
