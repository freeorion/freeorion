#include "TechWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "GGDrawUtil.h"
#include "../util/MultiplayerCommon.h"
#include "../universe/Tech.h"

extern "C" {
#include "vcg/globals.h"
#include "vcg/options.h"
}
#include "vcg/main.h"
#include "vcg/grprint.h"

extern int gs_stringw;
extern int gs_stringh;

namespace {
    std::pair<int, int> LayoutTechGraph(const std::string& category_shown, TechTreeWnd::TechTypesShown techs_shown,
                                        TechVertexMap& vertices, TechEdgeMap& edges)
    {
        TechManager& manager = GetTechManager();
        std::stringstream stream;
        stream << "graph: {\n"
               << "title: \"FreeOrion Tech Graph\"\n"
               << "color: lightgreen\n"
            //<< "width: 700\n"
            //<< "height: 700\n"
            //<< "xmax: 1190\n"
            //<< "ymax: 934\n"
               << "xbase: 5\n"
               << "ybase: 5\n"
               << "xspace: 20\n"
               << "xlspace: 10\n"
               << "yspace: 70\n"
            //<< "xraster: 1\n"
            //<< "xlraster: 1\n"
            //<< "yraster: 1\n"
            //<< "x: 30\n"
            //<< "y: 30\n"
               << "shrink:  1\n"
               << "stretch: 1\n"
               << "layout_downfactor: 1\n"
               << "layout_upfactor: 1\n"
               << "layout_nearfactor: 1\n"
            //<< "layout_splinefactor: 70\n"
               << "spreadlevel: 1\n"
            //<< "treefactor: 0.500000\n"
            //<< "bmax: 2147483647\n"
            //<< "cmin: 0\n"
            //<< "cmax: 2147483647\n"
            //<< "pmin: 0\n"
            //<< " pmax: 2147483647\n"
            //<< "rmin: 0\n"
            //<< "rmax: 0\n"
            //<< "smax: 2147483647\n"
               << "node_alignment: center\n"
               << "orientation: left_to_right\n"
            //<< "late_edge_labels: no\n"
            //<< "display_edge_labels: no\n"
            //<< "dirty_edge_labels: no\n"
               << "finetuning: yes\n"
            //<< "nearedges: no\n"
            //<< "splines: yes\n"
               << "ignoresingles: no\n"
               << "straight_phase: yes\n"
               << "priority_phase: yes\n"
            //<< "manhatten_edges: no\n"
            //<< "smanhatten_edges: no\n"
               << "port_sharing: yes\n"
               << "crossingphase2: no\n"
               << "crossingoptimization: yes\n"
               << "crossingweight: median\n"
            //<< "arrow_mode: fixed\n"
               << "layoutalgorithm: minbackward\n"
               << "\n";
        int i = 0;
        std::map<std::string, std::string> label_to_title_map;
        for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
            /*if (((techs_shown - TechTreeWnd::THEORY_TECHS) < (*it)->Type() - TT_THEORY))
                continue;
            if (category_shown != "ALL" && (*it)->Category() != category_shown)
                continue;*/
            std::string title = boost::lexical_cast<std::string>(i++);
            label_to_title_map[(*it)->Name()] = title;
            stream << "node: { title: \"" << title << "\" label: \"" << (*it)->Name() << "\" }\n";
        }
        stream << "\n";
        for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
            /*if (((techs_shown - TechTreeWnd::THEORY_TECHS) < (*it)->Type() - TT_THEORY))
                continue;
            if (category_shown != "ALL" && (*it)->Category() != category_shown)
                continue;*/
            for (std::set<std::string>::const_iterator prereq_it = (*it)->Prerequisites().begin();
                 prereq_it != (*it)->Prerequisites().end();
                 ++prereq_it) {
                stream << "edge: { color: lightblue sourcename: \"" << label_to_title_map[*prereq_it] << "\" targetname:\""
                       << label_to_title_map[(*it)->Name()] << "\" }\n";
            }
            stream << "\n";
        }
        stream << "}\n";

        char testvar = -1;
        if (testvar != -1) {
            FPRINTF(stderr,"Warning: On this system, chars are unsigned.\n");
            FPRINTF(stderr,"This may yield problems with the graph folding operation.\n");
        }

#if 0
        std::cout << "\nString passed to parse_part():\n" << stream.str() << "\n\n";
#endif

        parse_part(stream.str());
        visualize_part();
        print_graph(vertices, edges);

        return std::pair<int, int>(G_xmax, G_ymax);
    }

    const int TECH_PANEL_WIDTH = 80;
    const int TECH_PANEL_HEIGHT = 30;
}

class TechPanel : public GG::Wnd
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void (const Tech*)>      TechBrowsedSignalType;       ///< emitted when a technology is single-clicked
    typedef boost::signal<void (const Tech*)>      TechClickedSignalType;       ///< emitted when the mouse rolls over a technology
    typedef boost::signal<void (const Tech*)>      TechDoubleClickedSignalType; ///< emitted when a technology is double-clicked
    //@}

    /** \name Slot Types */ //@{
    typedef TechBrowsedSignalType::slot_type       TechBrowsedSlotType;       ///< type of functor(s) invoked on a TechBrowsedSignalType
    typedef TechClickedSignalType::slot_type       TechClickedSlotType;       ///< type of functor(s) invoked on a TechClickedSignalType
    typedef TechDoubleClickedSignalType::slot_type TechDoubleClickedSlotType; ///< type of functor(s) invoked on a TechDoubleClickedSignalType
    //@}

    TechPanel(const Tech* tech) :
        GG::Wnd(0, 0, 1, 1, GG::Wnd::CLICKABLE),
        m_tech(tech),
        m_tech_name(0)
        {
            Resize(TECH_PANEL_WIDTH, TECH_PANEL_HEIGHT);
            m_tech_name = new GG::TextControl(0, 0, Width(), Height(), UserString(m_tech->Name()), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);

            AttachChild(m_tech_name);
        }

    virtual bool Render()
        {
            GG::Pt ul = UpperLeft(), lr = LowerRight();
            GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::CTRL_COLOR, ClientUI::CTRL_BORDER_COLOR, 1);
            return true;
        }
    virtual void LClick(const GG::Pt& pt, Uint32 keys)       {TechClickedSignal(m_tech);}
    virtual void LDoubleClick(const GG::Pt& pt, Uint32 keys) {TechDoubleClickedSignal(m_tech);}
    virtual void MouseHere(const GG::Pt& pt, Uint32 keys)    {TechBrowsedSignal(m_tech);}

    mutable boost::signal<void (const Tech*)> TechBrowsedSignal;
    mutable boost::signal<void (const Tech*)> TechClickedSignal;
    mutable boost::signal<void (const Tech*)> TechDoubleClickedSignal;

private:
    const Tech*      m_tech;
    GG::TextControl* m_tech_name;
};

/* This is defined here in order to localize control over the layout of the tech tree to this file.  This
   function is used by VCG to determine the size of a specific node in a graph, based on its label string.  We
   can pervert this a bit and use the Tech associated with the string parameter "s" to determine the size that
   the corresponding TechPanel should be, in the coordinate space of the VCG graph (even if all TechPanels are
   the same size, which is not the case for VCG node labels). */
extern "C"
void gs_calcstringsize(char *s)
{
    // TODO: put code here to determine the actual size of a tech; for now, we'll just use a constant size for all techs
    //gs_stringw = (20*8*mystretch)/myshrink;
	//gs_stringh = (1*16*mystretch)/myshrink;
    gs_stringw = TECH_PANEL_WIDTH;
    gs_stringh = TECH_PANEL_HEIGHT;
}

//////////////////////////////////////////////////
// TechTreeView                                 //
//////////////////////////////////////////////////
TechTreeWnd::TechTreeWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::Wnd::CLICKABLE),
    m_category_shown("ALL"),
    m_tech_types_shown(ALL_TECHS),
    m_vscroll(0),
    m_hscroll(0)
{
    m_vscroll = new CUIScroll(w - ClientUI::SCROLL_WIDTH, 0, ClientUI::SCROLL_WIDTH, h - ClientUI::SCROLL_WIDTH, GG::Scroll::VERTICAL);
    m_hscroll = new CUIScroll(0, h - ClientUI::SCROLL_WIDTH, w - ClientUI::SCROLL_WIDTH, ClientUI::SCROLL_WIDTH, GG::Scroll::HORIZONTAL);

    AttachChild(m_vscroll);
    AttachChild(m_hscroll);

    GG::Connect(m_vscroll->ScrolledSignal(), &TechTreeWnd::ScrolledSlot, this);
    GG::Connect(m_hscroll->ScrolledSignal(), &TechTreeWnd::ScrolledSlot, this);

    Layout();
}

GG::Pt TechTreeWnd::ClientLowerRight() const
{
    return LowerRight() - GG::Pt(ClientUI::SCROLL_WIDTH, ClientUI::SCROLL_WIDTH);
}

const std::string& TechTreeWnd::CategoryShown() const
{
    return m_category_shown;
}

TechTreeWnd::TechTypesShown TechTreeWnd::GetTechTypesShown() const
{
    return m_tech_types_shown;
}

bool TechTreeWnd::Render()
{
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(m_hscroll->LowerRight().x, m_vscroll->LowerRight().y, lr.x, lr.y, ClientUI::CTRL_COLOR, GG::CLR_ZERO, 0);
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslated(m_scroll_position.x, m_scroll_position.y, 0);
    for (DependencyArcsMap::const_iterator it = m_dependency_arcs.begin(); it != m_dependency_arcs.end(); ++it) {
        glColor4ubv(ClientUI::CTRL_BORDER_COLOR.v);
        glBegin(GL_LINE_STRIP);
        for (unsigned int i = 0; i < it->second.second.size(); ++i) {
            glVertex2i(it->second.second[i].first, it->second.second[i].second);
        }
        glEnd();
    }
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
    return true;
}

void TechTreeWnd::ShowCategory(const std::string& category)
{
    m_category_shown = category;
}

void TechTreeWnd::SetTechTypesShown(TechTypesShown tech_types)
{
    m_tech_types_shown = tech_types;
}

void TechTreeWnd::Layout()
{
    TechVertexMap vertices;
    TechEdgeMap edges;
    std::pair<int, int> layout_size = LayoutTechGraph(m_category_shown, m_tech_types_shown, vertices, edges);

    // TODO: add scaling?

    GG::Pt client_sz = ClientSize();
    m_vscroll->SizeScroll(0, layout_size.second - 1, std::max(50, std::min(layout_size.second / 10, client_sz.y)), client_sz.y);
    m_hscroll->SizeScroll(0, layout_size.first - 1, std::max(50, std::min(layout_size.first / 10, client_sz.x)), client_sz.x);

    // clear out old tech panels, create tech panels
    for (std::map<const Tech*, TechPanel*>::const_iterator it = m_techs.begin(); it != m_techs.end(); ++it) {
        DeleteChild(it->second);
    }
    m_techs.clear();
    for (TechVertexMap::const_iterator it = vertices.begin(); it != vertices.end(); ++it) {
        const Tech* tech = GetTech(it->first);
        assert(tech);
        m_techs[tech] = new TechPanel(tech);
        m_techs[tech]->MoveTo(it->second.first, it->second.second);
        AttachChild(m_techs[tech]);
        GG::Connect(m_techs[tech]->TechBrowsedSignal, &TechTreeWnd::TechBrowsedSlot, this);
        GG::Connect(m_techs[tech]->TechClickedSignal, &TechTreeWnd::TechClickedSlot, this);
        GG::Connect(m_techs[tech]->TechDoubleClickedSignal, &TechTreeWnd::TechDoubleClickedSlot, this);
    }

    // create dependency arcs
    m_dependency_arcs.clear();
    for (TechEdgeMap::iterator it = edges.begin(); it != edges.end(); ++it) {
        const Tech* from = GetTech(it->first.first);
        const Tech* to = GetTech(it->first.second);
        assert(from && to);
        assert(m_techs[from] && m_techs[to]);
        assert(2 <= it->second.size());
        // TODO: do splining
        it->second.front().first += m_techs[from]->Width() / 2;
        it->second.front().second += m_techs[from]->Height() / 2;
        it->second.back().first += m_techs[to]->Width() / 2;
        it->second.back().second += m_techs[to]->Height() / 2;
        m_dependency_arcs.insert(std::make_pair(m_techs[from], std::make_pair(m_techs[to], it->second)));
    }
}

void TechTreeWnd::ScrolledSlot(int, int, int, int)
{
    int scroll_x = m_hscroll->PosnRange().first;
    int scroll_y = m_vscroll->PosnRange().first;
    int delta_x = scroll_x - m_scroll_position.x;
    int delta_y = scroll_y - m_scroll_position.y;
    m_scroll_position.x = scroll_x;
    m_scroll_position.y = scroll_y;
    std::cout << "scrolled to (" << m_scroll_position.x << ", " << m_scroll_position.y << ")\n";
    std::cout << "    delta: (" << delta_x << ", " << delta_y << ")\n";
    for (std::map<const Tech*, TechPanel*>::iterator it = m_techs.begin(); it != m_techs.end(); ++it) {
        it->second->OffsetMove(delta_x, delta_y);
    }
}

void TechTreeWnd::TechBrowsedSlot(const Tech* t)
{
    m_tech_browsed_sig(t);
}

void TechTreeWnd::TechClickedSlot(const Tech* t)
{
    m_tech_clicked_sig(t);
}

void TechTreeWnd::TechDoubleClickedSlot(const Tech* t)
{
    m_tech_double_clicked_sig(t);
}
