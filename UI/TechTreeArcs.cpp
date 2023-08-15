#include "TechTreeArcs.h"

#include "../util/Logger.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Tech.h"
#include "../client/human/GGHumanClientApp.h"
#include "TechTreeLayout.h"

#include <GG/ClrConstants.h>
#include <GG/GLClientAndServerBuffer.h>
#include <GG/DrawUtil.h>

#include <boost/graph/graph_concepts.hpp>

namespace {
    constexpr float ARC_THICKNESS = 3.0;
}


/////////////////////////////////
//// Implementation ////////////
///////////////////////////////
class TechTreeArcs::Impl {
public:
    Impl(const TechTreeLayout& layout, const std::set<std::string>& techs_to_show) :
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
        if (const Empire* empire = GetEmpire(GGHumanClientApp::GetApp()->EmpireID())) {
            const ResearchQueue& queue = empire->GetResearchQueue();
            for (const auto& edge : m_edges_to_show) {
                auto& tech1 = edge.first;
                auto& heads = edge.second;

                for (auto& head : heads) {
                    if (queue.InQueue(head) && (
                        queue.InQueue(tech1) || empire->GetTechStatus(tech1) == TechStatus::TS_COMPLETE))
                    {
                        // FillArcBuffer will put lines whose both ends are in highlights
                        // into the buffer
                        highlights.emplace(tech1);
                        highlights.emplace(head);
                    }
                }
            }
        }
        FillArcBuffer(m_highlight_buffer, highlights);
    }

private:
    const TechTreeLayout& m_layout;
    std::map<std::string, std::set<std::string>> m_edges_to_show;

    GG::GL2DVertexBuffer m_buffer;
    GG::GL2DVertexBuffer m_highlight_buffer;

    /// Fills \a buffer with the ends points for the lines that connect
    /// technologies in \a techs
    void FillArcBuffer(GG::GL2DVertexBuffer& buffer, const std::set<std::string>& techs) {
        for (const std::string& tech_name : techs) {
            //prerequisite edge
            for (const auto& edge : m_layout.GetOutEdges(tech_name)) {
                const auto& from = edge.GetTechFrom();
                const auto& to   = edge.GetTechTo();
                // Do not show lines leading to techs we are not showing
                if (!techs.contains(to))
                    continue;

                // Remember what edges we are showing so we can eventually highlight them
                m_edges_to_show[from].insert(to);
                if (!GetTech(from) || !GetTech(to)) {
                    ErrorLogger() << "TechTreeArcs::FillArcBuffer missing arc endpoint tech " << from << "->" << to;
                    continue;
                }

                const auto& points = edge.Points();
                if (points.empty())
                    continue;
                const auto pts_sz_m1 = points.size() - 1u;

                // To be able to draw all the lines in one call,
                // we will draw the with GL_LINES, which means all
                // vertices except the first and the last must occur twice
                for (std::size_t i = 0u; i < pts_sz_m1; ++i) {
                    buffer.store(points[i].first, points[i].second);
                    buffer.store(points[i+1].first, points[i+1].second);
                }
            }
        }
        buffer.createServerBuffer();
    }
};


TechTreeArcs::TechTreeArcs() = default;

TechTreeArcs::TechTreeArcs(const TechTreeLayout& layout, const std::set<std::string>& techs_to_show) :
    m_impl(std::make_unique<Impl>(layout, techs_to_show))
{}

TechTreeArcs::~TechTreeArcs() = default;

void TechTreeArcs::Render(double scale) {
    if (m_impl)
        m_impl->Render(scale);
}

void TechTreeArcs::Reset()
{ m_impl.reset(); }

void TechTreeArcs::Reset(const TechTreeLayout& layout, const std::set< std::string >& techs_to_show)
{ m_impl = std::make_unique<Impl>(layout, techs_to_show); }
