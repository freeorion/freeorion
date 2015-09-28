#include "TechTreeArcs.h"

#include "../util/Logger.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Tech.h"
#include "../client/human/HumanClientApp.h"
#include "TechTreeLayout.h"

#include <GG/ClrConstants.h>
#include <GG/GLClientAndServerBuffer.h>
#include <GG/DrawUtil.h>

#include <boost/graph/graph_concepts.hpp>

namespace {
    const float ARC_THICKNESS = 3.0;
}


/////////////////////////////////
//// Implementation ////////////
///////////////////////////////
class TechTreeArcsImplementation {
public:
    TechTreeArcsImplementation(const TechTreeLayout& layout, const std::set<std::string>& techs_to_show) :
        m_layout(layout)
    {
        FillArcBuffer(m_buffer, techs_to_show);
        RefreshHighlights();
    }

    void Render(double scale) {
        // draw arcs with thick, half-alpha line
        GG::Clr arc_colour = GG::CLR_GRAY;              arc_colour.a = 127;
        // redraw thicker highlight arcs
        GG::Clr arc_highlight_colour = GG::CLR_WHITE;   arc_highlight_colour.a = 127;

        glEnable(GL_LINE_SMOOTH);
        glDisable(GL_TEXTURE_2D);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);

        // First draw transparent wide lines

        //Normal lines
        glColor(arc_colour);
        glLineWidth(ARC_THICKNESS * scale);
        m_buffer.activate();
        glDrawArrays(GL_LINES, 0, m_buffer.size());

        // Highlights
        glColor(arc_highlight_colour);
        glLineWidth(ARC_THICKNESS * scale * 2);
        m_highlight_buffer.activate();
        glDrawArrays(GL_LINES, 0, m_highlight_buffer.size());

        // Next we draw opaque thinner lines

        // Normal lines
        arc_colour.a = 255;
        glColor(arc_colour);
        glLineWidth(ARC_THICKNESS * scale * 0.5);
        m_buffer.activate();
        glDrawArrays(GL_LINES, 0, m_buffer.size());

        // Highlights
        arc_highlight_colour.a = 255;
        glColor(arc_highlight_colour);
        glLineWidth(ARC_THICKNESS * scale);
        m_highlight_buffer.activate();
        glDrawArrays(GL_LINES, 0, m_highlight_buffer.size());

        glLineWidth(1.0);
        glPopClientAttrib();
        glEnable(GL_TEXTURE_2D);
    }

    // Fill m_highlight_buffer with the lines that should be highlighted
    void RefreshHighlights() {
        std::set<std::string> highlights;

        // We highlight lines that lead to techs that are queued for research
        if (const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID())) {
            const ResearchQueue& queue = empire->GetResearchQueue();
            for(std::map<std::string, std::set<std::string> >::const_iterator set_it = m_edges_to_show.begin();
                set_it != m_edges_to_show.end(); ++set_it) {

                std::string tech1 = set_it->first;
                const std::set<std::string>& heads = set_it->second;

                for (std::set<std::string>::const_iterator it = heads.begin(); it != heads.end(); ++it) {
                    if (queue.InQueue(*it) && (queue.InQueue(tech1) || empire->GetTechStatus(tech1) == TS_COMPLETE)) {
                        // FillArcBuffer will put lines whose both ends are in highlights
                        // into the buffer
                        highlights.insert(tech1);
                        highlights.insert(*it);
                    }
                }
            }
        }
        FillArcBuffer(m_highlight_buffer, highlights);
    }

private:
    const TechTreeLayout& m_layout;
    std::map<std::string, std::set<std::string> > m_edges_to_show;

    GG::GL2DVertexBuffer m_buffer;
    GG::GL2DVertexBuffer m_highlight_buffer;

    /// Fills \a buffer with the ends points for the lines that connect
    /// technologies in \a techs
    void FillArcBuffer(GG::GL2DVertexBuffer& buffer, const std::set<std::string>& techs) {
        for (std::set<std::string>::const_iterator it = techs.begin(); it != techs.end(); ++it) {

            const std::vector<TechTreeLayout::Edge*> edges = m_layout.GetOutEdges(*it);
            //prerequisite edge
            for (std::vector<TechTreeLayout::Edge*>::const_iterator edge = edges.begin();
                 edge != edges.end(); edge++)
            {
                std::vector<std::pair<double, double> > points;
                const std::string& from = (*edge)->GetTechFrom();
                const std::string& to   = (*edge)->GetTechTo();
                // Do not show lines leading to techs
                // we are not showing
                if (techs.find(to) == techs.end()) {
                    continue;
                }
                // Remember what edges we are showing so
                // we can eventually highlight them
                m_edges_to_show[from].insert(to);
                if (!GetTech(from) || !GetTech(to)) {
                    ErrorLogger() << "TechTreeArcs::FillArcBuffer missing arc endpoint tech " << from << "->" << to;
                    continue;
                }
                (*edge)->ReadPoints(points);
                // To be able to draw all the lines in one call,
                // we will draw the with GL_LINES, which means all
                // vertices except the first and the last must occur twice
                for (unsigned i = 0; i < points.size() - 1; ++i){
                    buffer.store(points[i].first, points[i].second);
                    buffer.store(points[i+1].first, points[i+1].second);
                }
            }
        }
        buffer.createServerBuffer();
    }
};

/////////////////////////////////
//// Public interface //////////
///////////////////////////////
TechTreeArcs::TechTreeArcs() :
    m_impl(0)
{}

TechTreeArcs::TechTreeArcs(const TechTreeLayout& layout, const std::set<std::string>& techs_to_show) :
    m_impl(new TechTreeArcsImplementation(layout, techs_to_show))
{}

TechTreeArcs::~TechTreeArcs() {
    if (m_impl != 0) {
        delete m_impl;
    }
    m_impl = 0;
}

void TechTreeArcs::Render(double scale)
{ m_impl->Render(scale); }

void TechTreeArcs::Reset()
{ m_impl = 0; }

void TechTreeArcs::Reset(const TechTreeLayout& layout, const std::set< std::string >& techs_to_show) {
    if (m_impl != 0) {
        delete m_impl;
        m_impl = 0;
    }
    m_impl = new TechTreeArcsImplementation(layout, techs_to_show);
}


