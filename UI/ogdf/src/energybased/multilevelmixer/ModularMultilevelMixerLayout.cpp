/*
 * $Revision: 2062 $
 *
 * last checkin:
 *   $Author: tschaefer $
 *   $Date: 2010-10-16 19:11:08 +0200 (Sat, 16 Oct 2010) $
 ***************************************************************/

/** \file
 * \brief MMM is a Multilevel Graph drawing Algorithm that can use different modules.
 *
 * \author Gereon Bartel
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

//#define USE_OGL_DRAWER

#include <ogdf/basic/basic.h>
#include <ogdf/energybased/multilevelmixer/ModularMultilevelMixerLayout.h>
#include <time.h>
#ifdef OGDF_DEBUG
#include <sstream>
#include <string>
#endif
#ifdef USE_OGL_DRAWER
	#include <../ModularMultilevelMixer/ModularMultilevelMixer/mmm/DrawOGLGraph.h>
#endif

namespace ogdf {

ModularMultilevelMixer::ModularMultilevelMixer()
: m_multilevelBuilder(0), m_initialPlacement(0), m_oneLevelLayoutModule(0),
m_finalLayoutModule(0), m_postLayoutModule(0), m_times(1), m_fixedEdgeLength(-1.0f), 
m_fixedNodeSize(-1.0f), m_postIterations(-1), m_postTimeFactor(-1.0f),
m_coarseningRatio(1.0),
m_postAfterEveryStep(false), m_levelBound(false), m_randomize(false)
{

}


void ModularMultilevelMixer::call(GraphAttributes &GA)
{
	MultilevelGraph MLG(GA);
	call(MLG);
	MLG.exportAttributes(GA);
}


void ModularMultilevelMixer::call(MultilevelGraph &MLG)
{
	m_errorCode = ercNone; 
	clock_t time = clock();
	if ((m_multilevelBuilder == 0 || m_initialPlacement == 0) && m_oneLevelLayoutModule == 0 && m_postLayoutModule == 0) {
		OGDF_THROW(AlgorithmFailureException);
		return;
	}

	if (m_fixedEdgeLength > 0.0) {
		edge e;
		forall_edges(e, MLG.getGraph()) {
			MLG.weight(e, m_fixedEdgeLength);
		}
	}

	if (m_fixedNodeSize > 0.0) {
		node v;
		forall_nodes(v, MLG.getGraph()) {
			MLG.radius(v, m_fixedNodeSize);
		}
	}

	m_multilevelGraph = &MLG;

	if (m_multilevelBuilder != 0 && m_initialPlacement != 0) {
#ifdef USE_OGL_DRAWER
		DrawOGLGraph::startStopTimer(0);
#endif
		double lbound = 16.0 * log(double(m_multilevelGraph->getGraph().numberOfNodes()))/log(2.0);
		m_multilevelGraph = &MLG;
		m_multilevelBuilder->buildAllLevels(*m_multilevelGraph);

		//Part for experiments: Stop if number of levels too high
#ifdef OGDF_DEBUG
		int nlevels = m_multilevelBuilder->getNumLevels();
#endif 
		if (m_levelBound)
		{
			if ( m_multilevelBuilder->getNumLevels() > lbound)
			{
				m_errorCode = ercLevelBound;
				return;
			} 
		}
		node v;
		if (m_randomize)
		{
			forall_nodes(v, MLG.getGraph()) {
				MLG.x(v, (float)randomDouble(-1.0, 1.0));
				MLG.y(v, (float)randomDouble(-1.0, 1.0));
			}
		}

#ifdef USE_OGL_DRAWER
		DrawOGLGraph::startStopTimer(0);
		DrawOGLGraph::startStopTimer(1);
#endif

		while(m_multilevelGraph->getLevel() > 0) {
			if (m_oneLevelLayoutModule != 0) {
				for(int i = 1; i <= m_times; i++) {
					m_oneLevelLayoutModule->call(*m_multilevelGraph);
				}
			}

#ifdef OGDF_MMM_LEVEL_OUTPUTS
			//Debugging output
			std::stringstream ss;
		        ss << nlevels--;
			std::string s;
			ss >> s;
			s = "LevelLayout"+s;
			String fs(s.c_str());
			fs += ".gml";
			m_multilevelGraph->writeGML(fs);
#endif
			if (m_postAfterEveryStep) {
				callPost(*m_multilevelGraph, 0);
			}

			m_multilevelGraph->moveToZero();

#ifdef USE_OGL_DRAWER
			DrawOGLGraph::startStopTimer(1);
			DrawOGLGraph::setGraph(m_multilevelGraph);
			DrawOGLGraph::drawUntilEsc();
			DrawOGLGraph::startStopTimer(0);
#endif
			int nNodes = m_multilevelGraph->getGraph().numberOfNodes();
			m_initialPlacement->placeOneLevel(*m_multilevelGraph);
			m_coarseningRatio = float(m_multilevelGraph->getGraph().numberOfNodes()) / nNodes;

#ifdef OGDF_MMM_LEVEL_OUTPUTS
			//debug only 
			s = s+"_placed.gml";
			m_multilevelGraph->writeGML(String(s.c_str()));
#endif

#ifdef USE_OGL_DRAWER
			DrawOGLGraph::startStopTimer(0);
			DrawOGLGraph::setGraph(m_multilevelGraph);
			DrawOGLGraph::drawUntilEsc();
			DrawOGLGraph::startStopTimer(1);
#endif

		}
#ifdef USE_OGL_DRAWER
		DrawOGLGraph::startStopTimer(1);
#endif
	}
	
	//Final level

#ifdef USE_OGL_DRAWER
	DrawOGLGraph::startStopTimer(1);
#endif

	LayoutModule *lastLayoutModule = (m_finalLayoutModule != 0 ? m_finalLayoutModule 
		: m_oneLevelLayoutModule);
	if (lastLayoutModule != 0) {
		for(int i = 1; i <= m_times; i++) {
			lastLayoutModule->call(*m_multilevelGraph);
		}
	}

	time = clock() - time;
	callPost(*m_multilevelGraph, time);

#ifdef USE_OGL_DRAWER
	DrawOGLGraph::startStopTimer(1);
	DrawOGLGraph::setGraph(m_multilevelGraph);
	DrawOGLGraph::drawUntilEsc();
#endif
}


void ModularMultilevelMixer::callPost(MultilevelGraph &MLG, clock_t time)
{
	clock_t time_start = 0;
	clock_t time_now = clock();
	double time_used = 0.0;
	long iterations = 0;
	if (m_postLayoutModule != 0) {
		do {
			iterations++;
			time_used = double(time_now - time_start) / CLOCKS_PER_SEC;
			m_postLayoutModule->call(MLG);
		} while((m_postTimeFactor < 0 || time == 0 || time_used < time * m_postTimeFactor)
			&& (m_postIterations < 0 || iterations < m_postIterations)
			&& ((m_postTimeFactor >= 0 && time > 0) || m_postIterations >= 0));
	}
}


void ModularMultilevelMixer::setLevelLayoutModule(LayoutModule * levelLayout)
{
	m_oneLevelLayoutModule = levelLayout;
}


void ModularMultilevelMixer::setPostLayoutModule(LayoutModule * postLayout)
{
	m_postLayoutModule = postLayout;
}

void ModularMultilevelMixer::setFinalLayoutModule(LayoutModule * finalLayout)
{
	m_finalLayoutModule = finalLayout;
}


void ModularMultilevelMixer::setMultilevelBuilder(MultilevelBuilder * levelBuilder)
{
	m_multilevelBuilder = levelBuilder;
}


void ModularMultilevelMixer::setInitialPlacer(InitialPlacer * placement)
{
	m_initialPlacement = placement;
}


void ModularMultilevelMixer::setLayoutRepeats(int times)
{
	m_times = times;
}


void ModularMultilevelMixer::setAllEdgeLengths(float len)
{
	m_fixedEdgeLength = len;
}


void ModularMultilevelMixer::setAllNodeSizes(float size)
{
	m_fixedNodeSize = size;
}


void ModularMultilevelMixer::setPostIterations(long iter)
{
	m_postIterations = iter;
}


void ModularMultilevelMixer::setPostTimeFactor(float timeFactor)
{
	m_postTimeFactor = timeFactor;
}


void ModularMultilevelMixer::setPostProcessingAfterEveryStep(bool on)
{
	m_postAfterEveryStep = on;
}

} // namespace ogdf
