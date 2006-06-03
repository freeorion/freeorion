#include "TechTreeWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/Tech.h"
#include "../universe/Effect.h"
#include "../Empire/Empire.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#ifndef FREEORION_BUILD_UTIL
#include "../client/human/HumanClientApp.h"
#include "../util/AppInterface.h"
#endif

#include <valarray>

#include <gvc.h>

#include <boost/format.hpp>

// TODO: after v0.3:
// - make the category buttons toggle the visibility of each category
// - render an insertion line between queue items when drag-dropping
// - implement zooming (probably just need a zoomed-out version that shows only the tech icons)

namespace {
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("UI.tech-layout-horz-spacing", "The horizontal spacing to be placed between techs in the tech screen, in multiples of the width of a single theory tech.", 0.75, RangedValidator<double>(0.1, 10.0));
        db.Add("UI.tech-layout-vert-spacing", "The vertical spacing to be placed between techs in the tech screen, in multiples of the height of a single theory tech.", 1.0, RangedValidator<double>(0.1, 10.0));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const int PROGRESS_PANEL_WIDTH = 94;
    const int PROGRESS_PANEL_HEIGHT = 18;
    const int PROGRESS_PANEL_LEFT_EXTRUSION = 21;
    const int PROGRESS_PANEL_BOTTOM_EXTRUSION = 9;
    const int MAIN_PANEL_CORNER_RADIUS = 5;
    const int PROGRESS_PANEL_CORNER_RADIUS = 5;

    const int THEORY_TECH_PANEL_LAYOUT_WIDTH = 250 + PROGRESS_PANEL_LEFT_EXTRUSION;
    const int THEORY_TECH_PANEL_LAYOUT_HEIGHT = 50 + PROGRESS_PANEL_BOTTOM_EXTRUSION;
    const int APPLICATION_TECH_PANEL_LAYOUT_WIDTH = 250;
    const int APPLICATION_TECH_PANEL_LAYOUT_HEIGHT = 44;
    const int REFINEMENT_TECH_PANEL_LAYOUT_WIDTH = 250;
    const int REFINEMENT_TECH_PANEL_LAYOUT_HEIGHT = 44;

    const int TECH_PANEL_LAYOUT_WIDTH = THEORY_TECH_PANEL_LAYOUT_WIDTH;
    const int TECH_PANEL_LAYOUT_HEIGHT = THEORY_TECH_PANEL_LAYOUT_HEIGHT - PROGRESS_PANEL_BOTTOM_EXTRUSION;

    const double OUTER_LINE_THICKNESS = 2.0;
    const double ARC_THICKNESS = 3.0;

    const double TECH_NAVIGATOR_ROLLOVER_BRIGHTENING_FACTOR = 1.5;

    const double MIN_SCALE = 0.209715;  // = 1.0/(1.25)^7  (going to scale = 0.2 crashes client)
    const double MAX_SCALE = 1.0;

    GG::Clr CategoryColor(const std::string& category_name)
    {
        const std::vector<std::string>& tech_categories = GetTechManager().CategoryNames();
        std::vector<std::string>::const_iterator it = std::find(tech_categories.begin(), tech_categories.end(), category_name);
        if (it != tech_categories.end()) {
            int category_index = std::distance(tech_categories.begin(), it) + 1;
            return GetOptionsDB().Get<StreamableColor>("UI.tech-category-" + boost::lexical_cast<std::string>(category_index)).ToClr();
        }
        return GG::Clr();
    }

    boost::shared_ptr<GG::Texture> CategoryIcon(const std::string& category_name)
    {
        std::string icon_filename;
        if (category_name == "CONSTRUCTION_CATEGORY")
            icon_filename = "tech_icons/categories/construction.png";
        if (category_name == "ECONOMICS_CATEGORY")
            icon_filename = "tech_icons/categories/economics.png";
        if (category_name == "GROWTH_CATEGORY")
            icon_filename = "tech_icons/categories/growth.png";
        if (category_name == "LEARNING_CATEGORY")
            icon_filename = "tech_icons/categories/learning.png";
        if (category_name == "PRODUCTION_CATEGORY")
            icon_filename = "tech_icons/categories/production.png";
        return HumanClientApp::GetApp()->GetTextureOrDefault(ClientUI::ART_DIR + icon_filename);
    }

    pointf Bezier(pointf* patch, double t)
    {
        pointf temp[6][6];
        for (int j = 0; j <= 3; j++) {
            temp[0][j] = patch[j];
        }
        for (int i = 1; i <= 3; i++) {
            for (int j = 0; j <= 3 - i; j++) {
                temp[i][j].x = (1.0 - t) * temp[i - 1][j].x + t * temp[i - 1][j + 1].x;
                temp[i][j].y = (1.0 - t) * temp[i - 1][j].y + t * temp[i - 1][j + 1].y;
            }
        }
        return temp[3][0];
    }

    std::vector<std::pair<double, double> > Spline(const std::vector<std::pair<int, int> >& control_points)
    {
        std::vector<std::pair<double, double> > retval;
        pointf patch[4];
        patch[3].x = control_points[0].first;
        patch[3].y = control_points[0].second;
        for (unsigned int i = 0; i + 3 < control_points.size(); i += 3) {
            patch[0] = patch[3];
            for (int j = 1; j <= 3; ++j) {
                patch[j].x = control_points[i + j].first;
                patch[j].y = control_points[i + j].second;
            }
            retval.push_back(std::make_pair(patch[0].x, patch[0].y));
            const int SUBDIVISIONS = 20;
            for (int step = 1; step <= SUBDIVISIONS; ++step) {
                pointf pt = Bezier(patch, static_cast<double>(step) / SUBDIVISIONS);
                retval.push_back(std::make_pair(pt.x, pt.y));
            }
        }
        return retval;
    }

    void FillTheoryPanel(const GG::Rect& panel, int corner_radius)
    {
        CircleArc(panel.lr.x - 2 * corner_radius, panel.ul.y,
                  panel.lr.x, panel.ul.y + 2 * corner_radius,
                  0.0, PI / 2.0, true);
        CircleArc(panel.ul.x, panel.ul.y,
                  panel.ul.x + 2 * corner_radius, panel.ul.y + 2 * corner_radius,
                  PI / 2.0, PI, true);
        CircleArc(panel.ul.x, panel.lr.y - 2 * corner_radius,
                  panel.ul.x + 2 * corner_radius, panel.lr.y,
                  PI, 3.0 * PI / 2.0, true);
        CircleArc(panel.lr.x - 2 * corner_radius, panel.lr.y - 2 * corner_radius,
                  panel.lr.x, panel.lr.y,
                  3.0 * PI / 2.0, 0.0, true);
        glBegin(GL_QUADS);
        glVertex2i(panel.ul.x + corner_radius, panel.ul.y);
        glVertex2i(panel.ul.x + corner_radius, panel.lr.y);
        glVertex2i(panel.lr.x - corner_radius, panel.lr.y);
        glVertex2i(panel.lr.x - corner_radius, panel.ul.y);
        glVertex2i(panel.ul.x, panel.ul.y + corner_radius);
        glVertex2i(panel.ul.x, panel.lr.y - corner_radius);
        glVertex2i(panel.ul.x + corner_radius, panel.lr.y - corner_radius);
        glVertex2i(panel.ul.x + corner_radius, panel.ul.y + corner_radius);
        glVertex2i(panel.lr.x - corner_radius, panel.ul.y + corner_radius);
        glVertex2i(panel.lr.x - corner_radius, panel.lr.y - corner_radius);
        glVertex2i(panel.lr.x, panel.lr.y - corner_radius);
        glVertex2i(panel.lr.x, panel.ul.y + corner_radius);
        glEnd();
    }

    void FillApplicationPanel(const GG::Rect& panel, int corner_radius)
    {
        CircleArc(panel.lr.x - 2 * corner_radius, panel.ul.y,
                  panel.lr.x, panel.ul.y + 2 * corner_radius,
                  0.0, PI / 2.0, true);
        CircleArc(panel.ul.x, panel.ul.y,
                  panel.ul.x + 2 * corner_radius, panel.ul.y + 2 * corner_radius,
                  PI / 2.0, PI, true);
        glBegin(GL_QUADS);
        glVertex2i(panel.ul.x + corner_radius, panel.ul.y + corner_radius);
        glVertex2i(panel.ul.x + corner_radius, panel.ul.y);
        glVertex2i(panel.lr.x - corner_radius, panel.ul.y);
        glVertex2i(panel.lr.x - corner_radius, panel.ul.y + corner_radius);
        glVertex2i(panel.ul.x, panel.ul.y + corner_radius);
        glVertex2i(panel.ul.x, panel.lr.y);
        glVertex2i(panel.lr.x, panel.lr.y);
        glVertex2i(panel.lr.x, panel.ul.y + corner_radius);
        glEnd();
    }

    void FillRefinementPanel(const GG::Rect& panel)
    {
        glBegin(GL_QUADS);
        glVertex2i(panel.ul.x, panel.ul.y);
        glVertex2i(panel.ul.x, panel.lr.y);
        glVertex2i(panel.lr.x, panel.lr.y);
        glVertex2i(panel.lr.x, panel.ul.y);
        glEnd();
    }

    void FillTechPanelInterior(TechType tech_type, const GG::Rect& main_panel, const GG::Rect& progress_panel, GG::Clr color, bool show_progress, double progress)
    {
        GG::Clr progress_background_color = ClientUI::TECH_WND_PROGRESS_BAR_BACKGROUND;
        GG::Clr progress_color = ClientUI::TECH_WND_PROGRESS_BAR;
        glColor4ubv(color.v);
        int progress_extent = (0.0 < progress && progress < 1.0) ? (progress_panel.ul.x + static_cast<int>(progress * PROGRESS_PANEL_WIDTH + 0.5)) : 0;
        if (tech_type == TT_THEORY) {
            FillTheoryPanel(main_panel, MAIN_PANEL_CORNER_RADIUS);
            if (show_progress) {
                if (progress_extent) {
                    glColor4ubv(progress_background_color.v);
                    FillTheoryPanel(progress_panel, PROGRESS_PANEL_CORNER_RADIUS);
                    glColor4ubv(progress_color.v);
                    GG::BeginScissorClipping(progress_panel.ul.x, progress_panel.ul.y, progress_extent, progress_panel.lr.y);
                    FillTheoryPanel(progress_panel, PROGRESS_PANEL_CORNER_RADIUS);
                    GG::EndScissorClipping();
                } else {
                    FillTheoryPanel(progress_panel, PROGRESS_PANEL_CORNER_RADIUS);
                }
            }
        } else if (tech_type == TT_APPLICATION) {
            FillApplicationPanel(main_panel, MAIN_PANEL_CORNER_RADIUS);
            if (show_progress) {
                if (progress_extent) {
                    glColor4ubv(progress_background_color.v);
                    FillApplicationPanel(progress_panel, PROGRESS_PANEL_CORNER_RADIUS);
                    glColor4ubv(progress_color.v);
                    GG::BeginScissorClipping(progress_panel.ul.x, progress_panel.ul.y, progress_extent, progress_panel.lr.y);
                    FillApplicationPanel(progress_panel, PROGRESS_PANEL_CORNER_RADIUS);
                    GG::EndScissorClipping();
                } else {
                    FillApplicationPanel(progress_panel, PROGRESS_PANEL_CORNER_RADIUS);
                }
            }
        } else { // tech_type == TT_REFINEMENT
            FillRefinementPanel(main_panel);
            if (show_progress) {
                if (progress_extent) {
                    glColor4ubv(progress_background_color.v);
                    FillRefinementPanel(progress_panel);
                    glColor4ubv(progress_color.v);
                    GG::BeginScissorClipping(progress_panel.ul.x, progress_panel.ul.y, progress_extent, progress_panel.lr.y);
                    FillRefinementPanel(progress_panel);
                    GG::EndScissorClipping();
                } else {
                    FillRefinementPanel(progress_panel);
                }
            }
        }
    }

    void TraceTechPanelOutline(TechType tech_type, const GG::Rect& main_panel, const GG::Rect& progress_panel, bool show_progress)
    {
        glBegin(GL_LINE_STRIP);
        if (tech_type == TT_THEORY) {
            if (show_progress) {
                glVertex2i(main_panel.lr.x, progress_panel.ul.y);
                CircleArc(main_panel.lr.x - 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.ul.y,
                          main_panel.lr.x, main_panel.ul.y + 2 * MAIN_PANEL_CORNER_RADIUS,
                          0.0, PI / 2.0, false);
                CircleArc(main_panel.ul.x, main_panel.ul.y,
                          main_panel.ul.x + 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.ul.y + 2 * MAIN_PANEL_CORNER_RADIUS,
                          PI / 2.0, PI, false);
                CircleArc(main_panel.ul.x, main_panel.lr.y - 2 * MAIN_PANEL_CORNER_RADIUS,
                          main_panel.ul.x + 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.lr.y,
                          PI, 3.0 * PI / 2.0, false);
                glVertex2i(progress_panel.ul.x, main_panel.lr.y);
                CircleArc(progress_panel.ul.x, progress_panel.lr.y - 2 * PROGRESS_PANEL_CORNER_RADIUS,
                          progress_panel.ul.x + 2 * PROGRESS_PANEL_CORNER_RADIUS, progress_panel.lr.y,
                          PI, 3.0 * PI / 2.0, false);
                CircleArc(progress_panel.lr.x - 2 * PROGRESS_PANEL_CORNER_RADIUS, progress_panel.lr.y - 2 * PROGRESS_PANEL_CORNER_RADIUS,
                          progress_panel.lr.x, progress_panel.lr.y,
                          3.0 * PI / 2.0, 0.0, false);
                CircleArc(progress_panel.lr.x - 2 * PROGRESS_PANEL_CORNER_RADIUS, progress_panel.ul.y,
                          progress_panel.lr.x, progress_panel.ul.y + 2 * PROGRESS_PANEL_CORNER_RADIUS,
                          0.0, PI / 2.0, false);
                CircleArc(progress_panel.ul.x, progress_panel.ul.y,
                          progress_panel.ul.x + 2 * PROGRESS_PANEL_CORNER_RADIUS, progress_panel.ul.y + 2 * PROGRESS_PANEL_CORNER_RADIUS,
                          PI / 2.0, PI, false);
                glVertex2i(progress_panel.ul.x, main_panel.lr.y);
            } else {
                CircleArc(main_panel.lr.x - 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.ul.y,
                          main_panel.lr.x, main_panel.ul.y + 2 * MAIN_PANEL_CORNER_RADIUS,
                          0.0, PI / 2.0, false);
                CircleArc(main_panel.ul.x, main_panel.ul.y,
                          main_panel.ul.x + 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.ul.y + 2 * MAIN_PANEL_CORNER_RADIUS,
                          PI / 2.0, PI, false);
                CircleArc(main_panel.ul.x, main_panel.lr.y - 2 * MAIN_PANEL_CORNER_RADIUS,
                          main_panel.ul.x + 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.lr.y,
                          PI, 3.0 * PI / 2.0, false);
                CircleArc(main_panel.lr.x - 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.lr.y - 2 * MAIN_PANEL_CORNER_RADIUS,
                          main_panel.lr.x, main_panel.lr.y,
                          3.0 * PI / 2.0, 0.0, false);
                glVertex2i(main_panel.lr.x, main_panel.ul.y + MAIN_PANEL_CORNER_RADIUS);
            }
        } else if (tech_type == TT_APPLICATION) {
            if (show_progress) {
                glVertex2i(main_panel.lr.x, progress_panel.ul.y);
                CircleArc(main_panel.lr.x - 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.ul.y,
                          main_panel.lr.x, main_panel.ul.y + 2 * MAIN_PANEL_CORNER_RADIUS,
                          0.0, PI / 2.0, false);
                CircleArc(main_panel.ul.x, main_panel.ul.y,
                          main_panel.ul.x + 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.ul.y + 2 * MAIN_PANEL_CORNER_RADIUS,
                          PI / 2.0, PI, false);
                glVertex2i(main_panel.ul.x, main_panel.lr.y);
                glVertex2i(progress_panel.ul.x, main_panel.lr.y);
                glVertex2i(progress_panel.ul.x, progress_panel.lr.y);
                glVertex2i(progress_panel.lr.x, progress_panel.lr.y);
                CircleArc(progress_panel.lr.x - 2 * PROGRESS_PANEL_CORNER_RADIUS, progress_panel.ul.y,
                          progress_panel.lr.x, progress_panel.ul.y + 2 * PROGRESS_PANEL_CORNER_RADIUS,
                          0.0, PI / 2.0, false);
                CircleArc(progress_panel.ul.x, progress_panel.ul.y,
                          progress_panel.ul.x + 2 * PROGRESS_PANEL_CORNER_RADIUS, progress_panel.ul.y + 2 * PROGRESS_PANEL_CORNER_RADIUS,
                          PI / 2.0, PI, false);
                glVertex2i(progress_panel.ul.x, main_panel.lr.y);
            } else {
                CircleArc(main_panel.lr.x - 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.ul.y,
                          main_panel.lr.x, main_panel.ul.y + 2 * MAIN_PANEL_CORNER_RADIUS,
                          0.0, PI / 2.0, false);
                CircleArc(main_panel.ul.x, main_panel.ul.y,
                          main_panel.ul.x + 2 * MAIN_PANEL_CORNER_RADIUS, main_panel.ul.y + 2 * MAIN_PANEL_CORNER_RADIUS,
                          PI / 2.0, PI, false);
                glVertex2i(main_panel.ul.x, main_panel.lr.y);
                glVertex2i(main_panel.lr.x, main_panel.lr.y);
                glVertex2i(main_panel.lr.x, main_panel.ul.y + MAIN_PANEL_CORNER_RADIUS);
            }
        } else { // tech_type == TT_REFINEMENT
            if (show_progress) {
                glVertex2i(main_panel.lr.x, progress_panel.ul.y);
                glVertex2i(main_panel.lr.x, main_panel.ul.y);
                glVertex2i(main_panel.ul.x, main_panel.ul.y);
                glVertex2i(main_panel.ul.x, main_panel.lr.y);
                glVertex2i(progress_panel.ul.x, main_panel.lr.y);
                glVertex2i(progress_panel.ul.x, progress_panel.lr.y);
                glVertex2i(progress_panel.lr.x, progress_panel.lr.y);
                glVertex2i(progress_panel.lr.x, progress_panel.ul.y);
                glVertex2i(progress_panel.ul.x, progress_panel.ul.y);
                glVertex2i(progress_panel.ul.x, main_panel.lr.y);
            } else {
                glVertex2i(main_panel.ul.x, main_panel.ul.y);
                glVertex2i(main_panel.ul.x, main_panel.lr.y);
                glVertex2i(main_panel.lr.x, main_panel.lr.y);
                glVertex2i(main_panel.lr.x, main_panel.ul.y);
                glVertex2i(main_panel.ul.x, main_panel.ul.y);
            }
        }
        glEnd();
    }

    /** The +/- button that collapses or expands a tech's subtrees, if any. */
    class ExpandCollapseButton : public GG::StateButton
    {
    public:
        // HACK! we're storing the border color here for color, and the color of the +/- symbol in text_color
        ExpandCollapseButton(const GG::Clr& color, const GG::Clr& border_color, double scale) :
            StateButton(0, 0, static_cast<int>(SIZE * scale + 0.5), static_cast<int>(SIZE * scale + 0.5), "",
                        GG::GUI::GetGUI()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::PTS * scale + 0.5)), 0, border_color, color),
            m_scale(scale),
            m_selected(false),
            m_mouse_here(false)
            {}

        virtual bool InWindow(const GG::Pt& pt) const
            {
                GG::Pt ul = UpperLeft(), sz = Size();
                double dx = pt.x - (ul.x + sz.x / 2.0);
                double dy = pt.y - (ul.y + sz.y / 2.0);
                double r = std::min(sz.x / 2.0, sz.y / 2.0);
                return dx * dx + dy * dy < r * r;
            }

        virtual void Render()
            {
                GG::Pt ul = UpperLeft(), lr = LowerRight();
                GG::Clr color = TextColor();
                GG::Clr border_color = Color();
                if (m_selected) {
                    color = GG::LightColor(color);
                    border_color = GG::LightColor(border_color);
                }
                const double ROLLOVER_BRIGHTENING_FACTOR = 1.5;
                if (!Disabled() && m_mouse_here) {
                    AdjustBrightness(color, ROLLOVER_BRIGHTENING_FACTOR);
                    AdjustBrightness(border_color, ROLLOVER_BRIGHTENING_FACTOR);
                }
                glDisable(GL_TEXTURE_2D);
                glColor4ubv(color.v);
                CircleArc(ul.x, ul.y, lr.x, lr.y, 0.0, 0.0, true);
                glEnable(GL_LINE_SMOOTH);
                glLineWidth(OUTER_LINE_THICKNESS * m_scale);
                DrawOutline(ul, lr, border_color, 63, 127);
                glLineWidth(1.0);
                glDisable(GL_LINE_SMOOTH);
                DrawOutline(ul, lr, border_color, 255, 255);
                glEnable(GL_TEXTURE_2D);
            }

        virtual void MouseEnter(const GG::Pt& pt, Uint32 keys) {m_mouse_here = true;}

        virtual void MouseLeave() {m_mouse_here = false;}

        void SetSelected(bool s) {m_selected = s;}

    private:
        void DrawOutline(const GG::Pt& ul, const GG::Pt& lr, GG::Clr color, int alpha1, int alpha2)
            {
                glColor4ub(color.r, color.g, color.b, alpha1);
                glBegin(GL_LINES);
                // trace the lines both ways for symmetry, since the shape is so small
                GG::Pt center(static_cast<int>((lr.x + ul.x) / 2.0 + 0.5),
                              static_cast<int>((lr.y + ul.y) / 2.0 + 0.5));
                glVertex2i(ul.x + 3, center.y);
                glVertex2i(lr.x - 3, center.y);
                glVertex2i(lr.x - 3, center.y);
                glVertex2i(ul.x + 3, center.y);
                if (Checked()) {
                    glVertex2i(center.x, ul.y + 3);
                    glVertex2i(center.x, lr.y - 3);
                    glVertex2i(center.x, lr.y - 3);
                    glVertex2i(center.x, ul.y + 3);
                }
                glEnd();
                if (alpha1 != alpha2)
                    glColor4ub(color.r, color.g, color.b, alpha2);
                glBegin(GL_LINE_STRIP);
                CircleArc(ul.x, ul.y, lr.x, lr.y, 0.0, 0.0, false);
                glEnd();
            }

        double m_scale;
        bool   m_selected;
        bool   m_mouse_here;

        static const int SIZE = 17;
    };

    /* returns true iff this tech is the root of a tree within the overall tech dependency graph.  This will only
       evaluate to true iff this tech has at most one prerequisite, and every tech that depends on it (directly or
       recursively) does as well.  If \a category is not "ALL", only the subgraph for the named category is
       considered. */
    bool RootOfSubtree(const Tech* tech, const std::string& category)
    {
        int category_prereqs = 0;
        if (category == "ALL") {
            category_prereqs = tech->Prerequisites().size();
        } else {
            for (std::set<std::string>::const_iterator it = tech->Prerequisites().begin(); it != tech->Prerequisites().end(); ++it) {
                if (GetTech(*it)->Category() == category)
                    ++category_prereqs;
            }
        }
        if (category_prereqs != 1)
            return false;
        for (std::set<std::string>::const_iterator it = tech->UnlockedTechs().begin(); it != tech->UnlockedTechs().end(); ++it) {
            const Tech* unlocked_tech = GetTech(*it);
            if ((category == "ALL" || unlocked_tech->Category() == category) && !RootOfSubtree(unlocked_tech, category))
                return false;
        }
        return true;
    }

    struct SelectCategoryFunctor
    {
        SelectCategoryFunctor(TechTreeWnd* tree_wnd, const std::string& category) : m_tree_wnd(tree_wnd), m_category(category) {}
        void operator()() {m_tree_wnd->ShowCategory(m_category);}
        TechTreeWnd* const m_tree_wnd;
        const std::string m_category;
    };

}

//////////////////////////////////////////////////
// TechTreeWnd::TechDetailPanel                 //
//////////////////////////////////////////////////
/** A panel of text controls and/or graphics that shows a greater level of detail about a tech (such
    as its full description text and effects text) than what appears in the tech graph. */
class TechTreeWnd::TechDetailPanel : public GG::Wnd
{
public:
    TechDetailPanel(int w, int h);
    const Tech* CurrentTech() const {return m_tech;}
    virtual void Render();
    void SetTech(const Tech* tech)  {m_tech = tech; Reset();}
    mutable boost::signal<void (const Tech*)> CenterOnTechSignal;
    mutable boost::signal<void (const Tech*)> QueueTechSignal;

private:
    GG::Pt TechGraphicUpperLeft() const;
    void CenterClickedSlot() {if (m_tech) CenterOnTechSignal(m_tech);}
    void AddToQueueClickedSlot() {if (m_tech) QueueTechSignal(m_tech);}
    void Reset();

    const Tech*         m_tech;
    GG::TextControl*    m_tech_name_text;
    GG::TextControl*    m_cost_text;
    GG::TextControl*    m_category_and_type_text;
    CUIButton*          m_recenter_button;
    CUIButton*          m_add_to_queue_button;
    CUIMultiEdit*       m_description_box;
    GG::StaticGraphic*  m_tech_graphic;
};

TechTreeWnd::TechDetailPanel::TechDetailPanel(int w, int h) :
    GG::Wnd(0, 0, w, h, 0),
    m_tech(0)
{
    const int NAME_PTS = ClientUI::PTS + 8;
    const int CATEGORY_AND_TYPE_PTS = ClientUI::PTS + 4;
    const int COST_PTS = ClientUI::PTS;
    const int BUTTON_WIDTH = 150;
    const int BUTTON_MARGIN = 5;
    m_tech_name_text = new GG::TextControl(1, 0, w - 1 - BUTTON_WIDTH, NAME_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT_BOLD, NAME_PTS), ClientUI::TEXT_COLOR);
    m_cost_text = new GG::TextControl(1, m_tech_name_text->LowerRight().y, w - 1 - BUTTON_WIDTH, COST_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT, COST_PTS), ClientUI::TEXT_COLOR);
    m_category_and_type_text = new GG::TextControl(1, m_cost_text->LowerRight().y, w - 1 - BUTTON_WIDTH, CATEGORY_AND_TYPE_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::FONT, CATEGORY_AND_TYPE_PTS), ClientUI::TEXT_COLOR);
    m_add_to_queue_button = new CUIButton(w - 1 - BUTTON_WIDTH, 1, BUTTON_WIDTH, UserString("TECH_DETAIL_ADD_TO_QUEUE"));
    m_recenter_button = new CUIButton(w - 1 - BUTTON_WIDTH, m_add_to_queue_button->LowerRight().y + BUTTON_MARGIN, BUTTON_WIDTH, UserString("TECH_DETAIL_CENTER_ON_TECH"));
    m_recenter_button->Hide();
    m_add_to_queue_button->Hide();
    m_recenter_button->Disable();
    m_add_to_queue_button->Disable();
    m_description_box = new CUIMultiEdit(1, m_category_and_type_text->LowerRight().y, w - 2 - BUTTON_WIDTH, h - m_category_and_type_text->LowerRight().y - 2, "", GG::TF_WORDBREAK | GG::MultiEdit::READ_ONLY);
    m_description_box->SetColor(GG::CLR_ZERO);
    m_description_box->SetInteriorColor(GG::CLR_ZERO);

    m_tech_graphic = 0;

    GG::Connect(m_recenter_button->ClickedSignal, &TechTreeWnd::TechDetailPanel::CenterClickedSlot, this);
    GG::Connect(m_add_to_queue_button->ClickedSignal, &TechTreeWnd::TechDetailPanel::AddToQueueClickedSlot, this);

    AttachChild(m_tech_name_text);
    AttachChild(m_cost_text);
    AttachChild(m_category_and_type_text);
    AttachChild(m_recenter_button);
    AttachChild(m_add_to_queue_button);
    AttachChild(m_description_box);
}

void TechTreeWnd::TechDetailPanel::Render()
{
    if (m_tech) {
        GG::Clr category_color = CategoryColor(m_tech->Category());
        category_color.a = 127;
        glColor4ubv(category_color.v);
        GG::Pt ul = m_description_box->ClientUpperLeft(), lr = m_description_box->ClientLowerRight();
        int icon_size = lr.y - ul.y;
        int x1 = (ul.x + lr.x) / 2 - icon_size / 2;
        CategoryIcon(m_tech->Category())->OrthoBlit(x1, ul.y, x1 + icon_size, lr.y, 0, false);
    }
}

void TechTreeWnd::TechDetailPanel::Reset()
{
    m_tech_name_text->SetText("");
    m_category_and_type_text->SetText("");
    m_cost_text->SetText("");
    m_description_box->SetText("");

    if (m_tech_graphic) {
        DeleteChild(m_tech_graphic);
        m_tech_graphic = 0;
    }

    if (!m_tech) {
        m_recenter_button->Hide();
        m_add_to_queue_button->Hide();
        m_recenter_button->Disable();
        m_add_to_queue_button->Disable();
        return;
    }

    m_recenter_button->Show();
    m_add_to_queue_button->Show();
    m_recenter_button->Disable(false);
    m_add_to_queue_button->Disable(false);

    if (!m_tech->Graphic().empty()) {
        GG::Pt ul = TechGraphicUpperLeft();
        m_tech_graphic = new GG::StaticGraphic(ul.x, ul.y, 128, 128,
                                               HumanClientApp::GetApp()->GetTextureOrDefault(ClientUI::ART_DIR + m_tech->Graphic()), 
                                               GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
        m_tech_graphic->Show();
        m_tech_graphic->SetColor(CategoryColor(m_tech->Category()));
        AttachChild(m_tech_graphic);
    }

    m_tech_name_text->SetText(UserString(m_tech->Name()));
    using boost::io::str;
    using boost::format;
    m_category_and_type_text->SetText("<i>" + str(format(UserString("TECH_DETAIL_TYPE_STR"))
        % UserString(m_tech->Category())
        % UserString(boost::lexical_cast<std::string>(m_tech->Type()))) + "</i>");
    m_category_and_type_text->SetColor(CategoryColor(m_tech->Category()));
    m_cost_text->SetText(str(format(UserString("TECH_TOTAL_COST_STR"))
        % static_cast<int>(m_tech->ResearchCost() + 0.5)
        % m_tech->ResearchTurns()));
    std::string description_str = str(format(UserString("TECH_DETAIL_DESCRIPTION_STR"))
                                      % UserString(m_tech->Description()));
    if (!m_tech->Effects().empty()) {
        description_str += str(format(UserString("TECH_DETAIL_EFFECTS_STR"))
                               % EffectsDescription(m_tech->Effects()));
    }
    const std::vector<ItemSpec>& unlocked_items = m_tech->UnlockedItems();
    if (!unlocked_items.empty())
        description_str += UserString("TECH_DETAIL_UNLOCKS_SECTION_STR");
    for (unsigned int i = 0; i < unlocked_items.size(); ++i) {
        description_str += str(format(UserString("TECH_DETAIL_UNLOCKED_ITEM_STR"))
                               % UserString(boost::lexical_cast<std::string>(unlocked_items[i].type))
                               % UserString(unlocked_items[i].name));
    }
    m_description_box->SetText(description_str);
}

GG::Pt TechTreeWnd::TechDetailPanel::TechGraphicUpperLeft() const
{
    return GG::Pt(Width() - 2 - 150 + (150 - 128) / 2, m_recenter_button->LowerRight().y - UpperLeft().y + 5);
}


//////////////////////////////////////////////////
// TechTreeWnd::TechNavigator                   //
//////////////////////////////////////////////////
/** A window with a single lisbox in it.  The listbox represents the techs that are required for and are
    unlocked by some tech.  Clicking on a prereq or unlocked tech will bring up that tech. */
class TechTreeWnd::TechNavigator : public GG::Wnd
{
public:
    enum {ROW_HEIGHT = 25};

    TechNavigator(int w, int h);

    const Tech* CurrentTech() const {return m_current_tech;}
    void SetTech(const Tech* tech) {m_current_tech = tech; Reset();}
    void TechClickedSlot(const Tech* tech) {TechClickedSignal(tech);}

    mutable boost::signal<void (const Tech*)> TechClickedSignal;

private:
    /** A control with a label \a str on it, and that is rendered partially onto the next row.
        The "Requires" and "Unlocks" rows are in of this class. */
    class SectionHeaderControl : public GG::Control
    {
    public:
        SectionHeaderControl(int w, int h, const std::string& str);
        virtual void Render();
        GG::TextControl* m_label;
    };

    /** The control in a single cell of a row with a tech in it. */
    class TechControl : public GG::Control
    {
    public:
        TechControl(int w, int h, const Tech* tech, int indentation);
        virtual GG::Pt ClientUpperLeft() const {return UpperLeft() + GG::Pt(3, 2);}
        virtual GG::Pt ClientLowerRight() const {return LowerRight() - GG::Pt(2, 2);}
        virtual void Render();
        virtual void LClick(const GG::Pt& pt, Uint32 keys) {ClickedSignal(m_tech);}
        virtual void MouseEnter(const GG::Pt& pt, Uint32 keys) {m_selected = true;}
        virtual void MouseLeave() {m_selected = false;}
        const Tech * const m_tech;
        GG::Clr m_border_color;
        GG::TextControl* m_name_text;
        mutable boost::signal<void (const Tech*)> ClickedSignal;
    private:
        int m_indentation;
        bool m_selected;
    };

    static const int TECH_ROW_INDENTATION = 8;

    GG::ListBox::Row* NewSectionHeaderRow(const std::string& str);
    GG::ListBox::Row* NewTechRow(const Tech* tech);
    void Reset();

    const Tech* m_current_tech;
    GG::ListBox* m_lb;
};

TechTreeWnd::TechNavigator::TechNavigator(int w, int h) :
    GG::Wnd(0, 0, w, h),
    m_current_tech(0)
{
    m_lb = new CUIListBox(1, 0, w, h - 2);
    m_lb->SetStyle(GG::LB_NOSORT | GG::LB_NOSEL);
    AttachChild(m_lb);
}

GG::ListBox::Row* TechTreeWnd::TechNavigator::NewSectionHeaderRow(const std::string& str)
{
    GG::ListBox::Row* retval = new GG::ListBox::Row(Width() - 33 - 14, ROW_HEIGHT + 2, "");
    retval->push_back(new SectionHeaderControl(Width() - 33 - 14, ROW_HEIGHT, str));
    return retval;
}

GG::ListBox::Row* TechTreeWnd::TechNavigator::NewTechRow(const Tech* tech)
{
    GG::ListBox::Row* retval = new GG::ListBox::Row(Width() - TECH_ROW_INDENTATION - 8 - 14, ROW_HEIGHT + 2, "");
    TechControl* control = new TechControl(Width() - TECH_ROW_INDENTATION - 8 - 14, ROW_HEIGHT, tech, TECH_ROW_INDENTATION);
    retval->push_back(control);
    GG::Connect(control->ClickedSignal, &TechTreeWnd::TechNavigator::TechClickedSlot, this);
    return retval;
}

void TechTreeWnd::TechNavigator::Reset()
{
    m_lb->Clear();
    if (!m_current_tech)
        return;

    const std::set<std::string>& prereqs = m_current_tech->Prerequisites();
    m_lb->Insert(NewSectionHeaderRow(UserString("TECH_WND_REQUIRES")));
    for (std::set<std::string>::const_iterator it = prereqs.begin(); it != prereqs.end(); ++it) {
        m_lb->Insert(NewTechRow(GetTech(*it)));
    }
    const std::set<std::string>& unlocks = m_current_tech->UnlockedTechs();
    m_lb->Insert(NewSectionHeaderRow(UserString("TECH_WND_UNLOCKS")));
    for (std::set<std::string>::const_iterator it = unlocks.begin(); it != unlocks.end(); ++it) {
        m_lb->Insert(NewTechRow(GetTech(*it)));
    }
}

TechTreeWnd::TechNavigator::SectionHeaderControl::SectionHeaderControl(int w, int h, const std::string& str) :
    GG::Control(0, 0, w, h)
{
    m_label = new GG::TextControl(8, 0, w - 8, h, str, GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR, GG::TF_LEFT);
    AttachChild(m_label);
}

void TechTreeWnd::TechNavigator::SectionHeaderControl::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight() + GG::Pt(0, 11);
    glDisable(GL_TEXTURE_2D);
    const int CORNER_RADIUS = 5;
    glColor4ubv(ClientUI::KNOWN_TECH_FILL_COLOR.v);
    CircleArc(lr.x - 2 * CORNER_RADIUS, ul.y,
                lr.x, ul.y + 2 * CORNER_RADIUS,
                0.0, PI / 2.0, true);
    glBegin(GL_QUADS);
    glVertex2i(lr.x - CORNER_RADIUS, ul.y);
    glVertex2i(ul.x, ul.y);
    glVertex2i(ul.x, lr.y);
    glVertex2i(lr.x - CORNER_RADIUS, lr.y);
    glVertex2i(lr.x, ul.y + CORNER_RADIUS);
    glVertex2i(lr.x - CORNER_RADIUS, ul.y + CORNER_RADIUS);
    glVertex2i(lr.x - CORNER_RADIUS, lr.y);
    glVertex2i(lr.x, lr.y);
    glEnd();
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(OUTER_LINE_THICKNESS);
    glColor4ub(ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.r, ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.g, ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.b, 127);
    glBegin(GL_LINE_STRIP);
    CircleArc(lr.x - 2 * CORNER_RADIUS, ul.y,
                lr.x, ul.y + 2 * CORNER_RADIUS,
                0.0, PI / 2.0, false);
    glVertex2i(ul.x, ul.y);
    glVertex2i(ul.x, lr.y);
    glVertex2i(lr.x, lr.y);
    glVertex2i(lr.x, ul.y + CORNER_RADIUS);
    glEnd();
    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    glColor4ubv(ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR.v);
    glBegin(GL_LINE_STRIP);
    CircleArc(lr.x - 2 * CORNER_RADIUS, ul.y,
                lr.x, ul.y + 2 * CORNER_RADIUS,
                0.0, PI / 2.0, false);
    glVertex2i(ul.x, ul.y);
    glVertex2i(ul.x, lr.y);
    glVertex2i(lr.x, lr.y);
    glVertex2i(lr.x, ul.y + CORNER_RADIUS);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

TechTreeWnd::TechNavigator::TechControl::TechControl(int w, int h, const Tech* tech, int indentation) :
    GG::Control(0, 0, w, h),
    m_tech(tech),
    m_indentation(indentation),
    m_selected(false)
{
    EnableChildClipping(true);
#ifndef FREEORION_BUILD_UTIL
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire->TechAvailable(m_tech->Name())) {
        SetColor(ClientUI::KNOWN_TECH_FILL_COLOR);
        m_border_color = ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR;
    } else if (empire->ResearchableTech(m_tech->Name())) {
        SetColor(ClientUI::RESEARCHABLE_TECH_FILL_COLOR);
        m_border_color = ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    } else {
        SetColor(ClientUI::UNRESEARCHABLE_TECH_FILL_COLOR);
        m_border_color = ClientUI::UNRESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    }
#else
    // these values are arbitrary; they're only useful for displaying techs in the tech-view utility app
    if (m_tech->Type() == TT_THEORY) {
        SetColor(ClientUI::KNOWN_TECH_FILL_COLOR);
        m_border_color = ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR;
    } else if (m_tech->Type() == TT_APPLICATION) {
        SetColor(ClientUI::RESEARCHABLE_TECH_FILL_COLOR);
        m_border_color = ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    } else {
        SetColor(ClientUI::UNRESEARCHABLE_TECH_FILL_COLOR);
        m_border_color = ClientUI::UNRESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    }
#endif
    GG::Pt client_size = ClientSize();
    m_name_text = new GG::TextControl(m_indentation, 0, client_size.x - m_indentation, client_size.y, UserString(m_tech->Name()), GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS), m_border_color, GG::TF_LEFT);
    AttachChild(m_name_text);
}

void TechTreeWnd::TechNavigator::TechControl::Render()
{
    GG::Rect rect(UpperLeft(), LowerRight());
    rect += GG::Pt(m_indentation, 0);
    TechType tech_type = m_tech->Type();
    GG::Clr color_to_use = Color();
    GG::Clr border_color_to_use = m_border_color;
    if (!Disabled() && m_selected) {
        AdjustBrightness(color_to_use, TECH_NAVIGATOR_ROLLOVER_BRIGHTENING_FACTOR);
        AdjustBrightness(border_color_to_use, TECH_NAVIGATOR_ROLLOVER_BRIGHTENING_FACTOR);
    }
    glDisable(GL_TEXTURE_2D);
    FillTechPanelInterior(tech_type, rect, GG::Rect(), color_to_use, false, 0.0);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(OUTER_LINE_THICKNESS);
    glColor4ub(border_color_to_use.r, border_color_to_use.g, border_color_to_use.b, 127);
    TraceTechPanelOutline(tech_type, rect, GG::Rect(), false);
    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    glColor4ubv(border_color_to_use.v);
    TraceTechPanelOutline(tech_type, rect, GG::Rect(), false);
    glEnable(GL_TEXTURE_2D);
}

//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel                     //
//////////////////////////////////////////////////
/** The window that contains the actual tech panels and dependency arcs. */
class TechTreeWnd::LayoutPanel : public GG::Wnd
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

    /** \name Structors */ //@{
    LayoutPanel(int w, int h);
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt ClientLowerRight() const;

    double                         Scale() const;
    const std::string&             CategoryShown() const;
    TechTreeWnd::TechTypesShown    GetTechTypesShown() const;
    TechTreeWnd::TechStatusesShown GetTechStatusesShown() const;

    mutable TechBrowsedSignalType       TechBrowsedSignal;
    mutable TechClickedSignalType       TechClickedSignal;
    mutable TechDoubleClickedSignalType TechDoubleClickedSignal;
    //@}

    //! \name Mutators //@{
    virtual void Render();

    void Update();
    void Clear();
    void Reset();
    void SetScale(double scale);
    void ShowCategory(const std::string& category);
    void ShowTech(const Tech* tech);
    void CenterOnTech(const Tech* tech);
    void SetTechTypesShown(TechTypesShown tech_types);
    void SetTechStatusesShown(TechStatusesShown tech_statuses);
    void UncollapseAll();
    //@}

private:
    class TechPanel;
    struct CollapseSubtreeFunctor;
    enum TechStatus {
        KNOWN,
        RESEARCHABLE,
        UNRESEARCHABLE
    };
    typedef std::multimap<const Tech*,
                          std::pair<const Tech*,
                                    std::vector<std::vector<std::pair<double, double> > > > > DependencyArcsMap;
    typedef std::map<TechStatus, DependencyArcsMap> DependencyArcsMapsByArcType;

    class LayoutSurface : public GG::Wnd
    {
    public:
        LayoutSurface() : Wnd(0, 0, 1, 1, GG::CLICKABLE | GG::DRAGABLE) {}
        virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys) {DraggedSignal(move);}
        virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys) {ZoomedSignal(move);}
        mutable boost::signal<void (int)> ZoomedSignal;
        mutable boost::signal<void (const GG::Pt&)> DraggedSignal;
    };

    void Layout(bool keep_position, double old_scale = -1.0);
    bool TechVisible(const Tech* tech);
    void CollapseTechSubtree(const Tech* tech, bool collapse);
    void DrawArc(DependencyArcsMap::const_iterator it, GG::Clr color, bool with_arrow_head);
    void ScrolledSlot(int, int, int, int);
    void TechBrowsedSlot(const Tech* tech);
    void TechClickedSlot(const Tech* tech);
    void TechDoubleClickedSlot(const Tech* tech);
    void TreeDraggedSlot(const GG::Pt& move);
    void TreeZoomedSlot(int move);

    double            m_scale;
    std::string       m_category_shown;
    TechTypesShown    m_tech_types_shown;
    TechStatusesShown m_tech_statuses_shown;
    const Tech*       m_selected_tech;

    // indexed by category-view (including "ALL"), the techs whose subtrees are desired collapsed
    std::map<std::string, std::set<const Tech*> > m_collapsed_subtree_techs_per_view;

    std::map<const Tech*, TechPanel*> m_techs;
    DependencyArcsMapsByArcType m_dependency_arcs;

    LayoutSurface* m_layout_surface;
    CUIScroll*     m_vscroll;
    CUIScroll*     m_hscroll;
    GG::Pt         m_scroll_position;

    friend struct CollapseSubtreeFunctor;
};

//////////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel::CollapseSubtreeFunctor //
//////////////////////////////////////////////////////
struct TechTreeWnd::LayoutPanel::CollapseSubtreeFunctor
{
    CollapseSubtreeFunctor(TechTreeWnd::LayoutPanel* layout_panel, const Tech* tech) : m_layout_panel(layout_panel), m_tech(tech) {}
    void operator()(bool checked) {m_layout_panel->CollapseTechSubtree(m_tech, checked);}
    TechTreeWnd::LayoutPanel* const m_layout_panel;
    const Tech* const m_tech;
};

//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel::TechPanel          //
//////////////////////////////////////////////////
/** Represents a single tech in the LayoutPanel. */
class TechTreeWnd::LayoutPanel::TechPanel : public GG::Wnd
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

    TechPanel(const Tech* tech, bool selected, bool collapsed_subtree, const std::string& category_shown, TechTypesShown types_shown, TechStatusesShown statuses_shown, double scale = 1.0);

    virtual bool InWindow(const GG::Pt& pt) const;
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, Uint32 keys);
    virtual void LDoubleClick(const GG::Pt& pt, Uint32 keys);
    virtual void MouseHere(const GG::Pt& pt, Uint32 keys);

    void Select(bool select);

    mutable boost::signal<void (const Tech*)> TechBrowsedSignal;
    mutable boost::signal<void (const Tech*)> TechClickedSignal;
    mutable boost::signal<void (const Tech*)> TechDoubleClickedSignal;
    GG::StateButton::CheckedSignalType& CollapseSubtreeSignal() {return m_toggle_button->CheckedSignal;}

private:
    GG::Rect ProgressPanelRect(const GG::Pt& ul, const GG::Pt& lr);

    const Tech*           m_tech;
    double                m_scale;
    double                m_progress; // in [0.0, 1.0]
    GG::Clr               m_fill_color;
    GG::Clr               m_text_and_border_color;
    GG::StaticGraphic*    m_category_icon;
    GG::TextControl*      m_tech_name_text;
    GG::TextControl*      m_tech_cost_text;
    GG::TextControl*      m_progress_text;
    ExpandCollapseButton* m_toggle_button;
    bool                  m_selected;
};

TechTreeWnd::LayoutPanel::TechPanel::TechPanel(const Tech* tech, bool selected, bool collapsed_subtree,
                                               const std::string& category_shown, TechTypesShown types_shown,
                                               TechStatusesShown statuses_shown, double scale/* = 1.0*/) :
    GG::Wnd(0, 0, 1, 1, GG::CLICKABLE),
    m_tech(tech),
    m_scale(scale),
    m_progress(0.0),
    m_tech_name_text(0),
    m_tech_cost_text(0),
    m_progress_text(0),
    m_toggle_button(0),
    m_selected(selected)
{
    int name_font_pts = ClientUI::PTS + 2;
    GG::Pt UPPER_TECH_TEXT_OFFSET(4, 2);
    GG::Pt LOWER_TECH_TEXT_OFFSET(4, 0);
    if (m_tech->Type() == TT_THEORY) {
        Resize(GG::Pt(static_cast<int>(THEORY_TECH_PANEL_LAYOUT_WIDTH * m_scale + 0.5), static_cast<int>(THEORY_TECH_PANEL_LAYOUT_HEIGHT * m_scale + 0.5)));
        name_font_pts += 2;
        UPPER_TECH_TEXT_OFFSET += GG::Pt(2, 2);
        LOWER_TECH_TEXT_OFFSET += GG::Pt(2, 4);
    } else if (m_tech->Type() == TT_APPLICATION) {
        Resize(GG::Pt(static_cast<int>(APPLICATION_TECH_PANEL_LAYOUT_WIDTH * m_scale + 0.5), static_cast<int>(APPLICATION_TECH_PANEL_LAYOUT_HEIGHT * m_scale + 0.5)));
    } else { // m_tech->Type() == TT_REFINEMENT
        Resize(GG::Pt(static_cast<int>(REFINEMENT_TECH_PANEL_LAYOUT_WIDTH * m_scale + 0.5), static_cast<int>(REFINEMENT_TECH_PANEL_LAYOUT_HEIGHT * m_scale + 0.5)));
    }
    name_font_pts = static_cast<int>(name_font_pts * m_scale + 0.5);
    UPPER_TECH_TEXT_OFFSET = GG::Pt(static_cast<int>(UPPER_TECH_TEXT_OFFSET.x * m_scale), static_cast<int>(UPPER_TECH_TEXT_OFFSET.y * m_scale));
    LOWER_TECH_TEXT_OFFSET = GG::Pt(static_cast<int>(LOWER_TECH_TEXT_OFFSET.x * m_scale), static_cast<int>(LOWER_TECH_TEXT_OFFSET.y * m_scale));

    using boost::io::str;
    using boost::format;
    bool known_tech = false;
    bool queued_tech = false;
    bool researchable_tech = false;
#ifndef FREEORION_BUILD_UTIL
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire->TechAvailable(m_tech->Name())) {
        known_tech = true;
    } else {
        ResearchQueue queue = empire->GetResearchQueue();
        if (queue.InQueue(m_tech))
            queued_tech = true;
        double rps_spent = empire->ResearchStatus(m_tech->Name());
        if (0.0 <= rps_spent) {
            m_progress = rps_spent / (m_tech->ResearchTurns() * m_tech->ResearchCost());
            assert(0.0 <= m_progress && m_progress <= 1.0);
        }
        researchable_tech = empire->ResearchableTech(m_tech->Name());
    }
#else
    // these values are arbitrary; they're only useful for displaying techs in the tech-view utility app
    m_progress = 0.2;
    if (m_tech->Type() == TT_THEORY) {
        known_tech = true;
    } else if (m_tech->Type() == TT_APPLICATION) {
        researchable_tech = true;
    }
#endif

    if (known_tech) {
        m_fill_color = ClientUI::KNOWN_TECH_FILL_COLOR;
        m_text_and_border_color = ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR;
    } else if (researchable_tech) {
        m_fill_color = ClientUI::RESEARCHABLE_TECH_FILL_COLOR;
        m_text_and_border_color = ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    } else {
        m_fill_color = ClientUI::UNRESEARCHABLE_TECH_FILL_COLOR;
        m_text_and_border_color = ClientUI::UNRESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    }

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::FONT, name_font_pts);
    m_category_icon = new GG::StaticGraphic(UPPER_TECH_TEXT_OFFSET.x, UPPER_TECH_TEXT_OFFSET.y, font->Lineskip(), font->Lineskip(),
                                            CategoryIcon(m_tech->Category()), GG::GR_FITGRAPHIC);
    m_category_icon->SetColor(CategoryColor(m_tech->Category()));

    m_tech_name_text = new GG::TextControl(m_category_icon->LowerRight().x + static_cast<int>(4 * m_scale), UPPER_TECH_TEXT_OFFSET.y,
                                           Width() - m_category_icon->LowerRight().x + static_cast<int>((4 - PROGRESS_PANEL_LEFT_EXTRUSION - 4) * m_scale), font->Lineskip(),
                                           UserString(m_tech->Name()), font, m_text_and_border_color, GG::TF_TOP | GG::TF_LEFT);
    m_tech_name_text->ClipText(true);
    AttachChild(m_category_icon);
    AttachChild(m_tech_name_text);

    std::string cost_str;
    if (!known_tech)
        cost_str = str(format(UserString("TECH_TOTAL_COST_STR")) % static_cast<int>(m_tech->ResearchCost() + 0.5) % m_tech->ResearchTurns());
    m_tech_cost_text = new GG::TextControl(UPPER_TECH_TEXT_OFFSET.x, 0,
                                           Width() - LOWER_TECH_TEXT_OFFSET.x, Height() - LOWER_TECH_TEXT_OFFSET.y - static_cast<int>(PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale),
                                           cost_str, GG::GUI::GetGUI()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::PTS * m_scale + 0.5)), m_text_and_border_color, GG::TF_BOTTOM | GG::TF_LEFT);
    AttachChild(m_tech_cost_text);

    GG::Rect progress_panel = ProgressPanelRect(UpperLeft(), LowerRight());
    std::string progress_str;
    if (known_tech)
        progress_str = UserString("TECH_WND_TECH_COMPLETED");
    else if (queued_tech)
        progress_str = UserString("TECH_WND_TECH_QUEUED");
    else if (m_progress)
        progress_str = UserString("TECH_WND_TECH_INCOMPLETE");
    m_progress_text = new GG::TextControl(static_cast<int>(progress_panel.ul.x - PROGRESS_PANEL_LEFT_EXTRUSION * m_scale), progress_panel.ul.y - static_cast<int>(PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale),
                                          progress_panel.Width(), progress_panel.Height(),
                                          progress_str, GG::GUI::GetGUI()->GetFont(ClientUI::FONT, static_cast<int>(ClientUI::PTS * m_scale + 0.5)), m_text_and_border_color);
    AttachChild(m_progress_text);

    m_toggle_button = new ExpandCollapseButton(m_fill_color, m_text_and_border_color, m_scale);
    bool show_toggle_button = false;
    const std::set<std::string>& unlocked_techs = m_tech->UnlockedTechs();
    for (std::set<std::string>::const_iterator it = unlocked_techs.begin(); it != unlocked_techs.end(); ++it) {
        const Tech* unlocked_tech = GetTech(*it);
        if ((category_shown == "ALL" || unlocked_tech->Category() == category_shown) &&
            ((unlocked_tech->Type() - TT_THEORY) <= (types_shown - TechTreeWnd::THEORY_TECHS)) &&
            ((statuses_shown == RESEARCHABLE_TECHS && empire->ResearchableTech(unlocked_tech->Name()) && !empire->TechAvailable(unlocked_tech->Name())) ||
             (statuses_shown == COMPLETE_AND_RESEARCHABLE_TECHS && (empire->ResearchableTech(unlocked_tech->Name()) || empire->TechAvailable(unlocked_tech->Name()))) ||
             statuses_shown == ALL_TECH_STATUSES) &&
            RootOfSubtree(unlocked_tech, category_shown)) {
            show_toggle_button = true;
            break;
        }
    }
    if (show_toggle_button) {
        if (collapsed_subtree)
            m_toggle_button->SetCheck();
        m_toggle_button->MoveTo(GG::Pt(Width() + static_cast<int>((-PROGRESS_PANEL_LEFT_EXTRUSION + 3) * m_scale),
                                       UpperLeft().x + static_cast<int>(Height() + (-PROGRESS_PANEL_HEIGHT + PROGRESS_PANEL_BOTTOM_EXTRUSION) * m_scale) / 2 - m_toggle_button->Height() / 2));
    } else {
        m_toggle_button->Hide();
    }
    AttachChild(m_toggle_button);

    Select(m_selected);
}

bool TechTreeWnd::LayoutPanel::TechPanel::InWindow(const GG::Pt& pt) const
{
    GG::Pt lr = LowerRight();
    return GG::Wnd::InWindow(pt) &&
        (pt.x <= lr.x - PROGRESS_PANEL_LEFT_EXTRUSION * m_scale || lr.y - PROGRESS_PANEL_HEIGHT * m_scale <= pt.y || m_toggle_button->InWindow(pt)) &&
        (lr.x - PROGRESS_PANEL_WIDTH * m_scale <= pt.x || pt.y <= lr.y - PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale);
}

void TechTreeWnd::LayoutPanel::TechPanel::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight() - GG::Pt(static_cast<int>(PROGRESS_PANEL_LEFT_EXTRUSION * m_scale), static_cast<int>(PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale));
    GG::Clr interior_color_to_use = m_selected ? GG::LightColor(m_fill_color) : m_fill_color;
    GG::Clr border_color_to_use = m_selected ? GG::LightColor(m_text_and_border_color) : m_text_and_border_color;

    GG::Rect main_panel(ul, lr);
    GG::Rect progress_panel = ProgressPanelRect(ul, lr);
    TechType tech_type = m_tech->Type();
    glDisable(GL_TEXTURE_2D);
    FillTechPanelInterior(tech_type, main_panel, progress_panel, interior_color_to_use, !m_progress_text->Empty(), m_progress);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(OUTER_LINE_THICKNESS * m_scale);
    glColor4ub(border_color_to_use.r, border_color_to_use.g, border_color_to_use.b, 127);
    TraceTechPanelOutline(tech_type, main_panel, progress_panel, !m_progress_text->Empty());
    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    glColor4ubv(border_color_to_use.v);
    TraceTechPanelOutline(tech_type, main_panel, progress_panel, !m_progress_text->Empty());
    glEnable(GL_TEXTURE_2D);
}

void TechTreeWnd::LayoutPanel::TechPanel::LClick(const GG::Pt& pt, Uint32 keys)
{
    if (!m_selected)
        TechClickedSignal(m_tech);
}

void TechTreeWnd::LayoutPanel::TechPanel::LDoubleClick(const GG::Pt& pt, Uint32 keys)
{
    TechDoubleClickedSignal(m_tech);
}

void TechTreeWnd::LayoutPanel::TechPanel::MouseHere(const GG::Pt& pt, Uint32 keys)
{
    TechBrowsedSignal(m_tech);
}

void TechTreeWnd::LayoutPanel::TechPanel::Select(bool select)
{
    if (m_selected = select) {
        m_tech_name_text->SetTextColor(GG::LightColor(m_text_and_border_color));
        m_tech_cost_text->SetTextColor(GG::LightColor(m_text_and_border_color));
        m_progress_text->SetTextColor(GG::LightColor(m_text_and_border_color));
    } else {
        m_tech_name_text->SetTextColor(m_text_and_border_color);
        m_tech_cost_text->SetTextColor(m_text_and_border_color);
        m_progress_text->SetTextColor(m_text_and_border_color);
    }
    m_toggle_button->SetSelected(m_selected);
}

GG::Rect TechTreeWnd::LayoutPanel::TechPanel::ProgressPanelRect(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Rect retval;
    retval.lr = lr + GG::Pt(static_cast<int>(PROGRESS_PANEL_LEFT_EXTRUSION * m_scale), static_cast<int>(PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale));
    retval.ul = retval.lr - GG::Pt(static_cast<int>(PROGRESS_PANEL_WIDTH * m_scale), static_cast<int>(PROGRESS_PANEL_HEIGHT * m_scale));
    return retval;
}

//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel                     //
//////////////////////////////////////////////////
TechTreeWnd::LayoutPanel::LayoutPanel(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::CLICKABLE),
    m_scale(1.0),
    m_category_shown("ALL"),
    m_tech_types_shown(ALL_TECH_TYPES),
    m_tech_statuses_shown(ALL_TECH_STATUSES),
    m_selected_tech(0),
    m_layout_surface(0),
    m_vscroll(0),
    m_hscroll(0)
{
    EnableChildClipping(true);

    m_layout_surface = new LayoutSurface();
    m_vscroll = new CUIScroll(w - ClientUI::SCROLL_WIDTH, 0, ClientUI::SCROLL_WIDTH, h - ClientUI::SCROLL_WIDTH, GG::VERTICAL);
    m_hscroll = new CUIScroll(0, h - ClientUI::SCROLL_WIDTH, w - ClientUI::SCROLL_WIDTH, ClientUI::SCROLL_WIDTH, GG::HORIZONTAL);

    AttachChild(m_layout_surface);
    AttachChild(m_vscroll);
    AttachChild(m_hscroll);

    GG::Connect(m_layout_surface->DraggedSignal, &TechTreeWnd::LayoutPanel::TreeDraggedSlot, this);
    GG::Connect(m_layout_surface->ZoomedSignal, &TechTreeWnd::LayoutPanel::TreeZoomedSlot, this);
    GG::Connect(m_vscroll->ScrolledSignal, &TechTreeWnd::LayoutPanel::ScrolledSlot, this);
    GG::Connect(m_hscroll->ScrolledSignal, &TechTreeWnd::LayoutPanel::ScrolledSlot, this);
}

GG::Pt TechTreeWnd::LayoutPanel::ClientLowerRight() const
{
    return LowerRight() - GG::Pt(ClientUI::SCROLL_WIDTH, ClientUI::SCROLL_WIDTH);
}

const std::string& TechTreeWnd::LayoutPanel::CategoryShown() const
{
    return m_category_shown;
}

double TechTreeWnd::LayoutPanel::Scale() const
{
    return m_scale;
}

TechTreeWnd::TechTypesShown TechTreeWnd::LayoutPanel::GetTechTypesShown() const
{
    return m_tech_types_shown;
}

TechTreeWnd::TechStatusesShown TechTreeWnd::LayoutPanel::GetTechStatusesShown() const
{
    return m_tech_statuses_shown;
}

void TechTreeWnd::LayoutPanel::Render()
{
    GG::Pt lr = LowerRight();

    BeginClipping();
    // render dependency arcs
    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    glTranslated(-m_scroll_position.x, -m_scroll_position.y, 0);

    // first, draw arc with thick, half-alpha line
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(ARC_THICKNESS * m_scale);
    GG::Clr known_half_alpha = ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR;
    known_half_alpha.a = 127;
    GG::Clr researchable_half_alpha = ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    researchable_half_alpha.a = 127;
    GG::Clr unresearchable_half_alpha = ClientUI::UNRESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR;
    unresearchable_half_alpha.a = 127;
    std::map<TechStatus, std::vector<DependencyArcsMap::const_iterator> > selected_arcs;
    for (DependencyArcsMapsByArcType::const_iterator it = m_dependency_arcs.begin(); it != m_dependency_arcs.end(); ++it) {
        GG::Clr arc_color;
        switch (it->first) {
        case KNOWN:          arc_color = known_half_alpha; break;
        case RESEARCHABLE:   arc_color = researchable_half_alpha; break;
        case UNRESEARCHABLE: arc_color = unresearchable_half_alpha; break;
        }
        glColor4ubv(arc_color.v);
        for (DependencyArcsMap::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            bool selected_arc = it2->first == m_selected_tech || it2->second.first == m_selected_tech;
            if (selected_arc) {
                selected_arcs[it->first].push_back(it2);
                continue;
            }
            DrawArc(it2, arc_color, false);
        }
    }
    GG::Clr known_half_alpha_selected = GG::LightColor(known_half_alpha);
    GG::Clr researchable_half_alpha_selected = GG::LightColor(researchable_half_alpha);
    GG::Clr unresearchable_half_alpha_selected = GG::LightColor(unresearchable_half_alpha);
    for (std::map<TechStatus, std::vector<DependencyArcsMap::const_iterator> >::const_iterator it = selected_arcs.begin();
         it != selected_arcs.end();
         ++it) {
        GG::Clr arc_color;
        switch (it->first) {
        case KNOWN:          arc_color = known_half_alpha_selected; break;
        case RESEARCHABLE:   arc_color = researchable_half_alpha_selected; break;
        case UNRESEARCHABLE: arc_color = unresearchable_half_alpha_selected; break;
        }
        glColor4ubv(arc_color.v);
        for (unsigned int i = 0; i < it->second.size(); ++i) {
            DrawArc(it->second[i], arc_color, false);
        }
    }

    // now retrace the arc with a normal-width, full-alpha line
    glLineWidth(ARC_THICKNESS * m_scale * 0.5);
    glDisable(GL_LINE_SMOOTH);
    for (DependencyArcsMapsByArcType::const_iterator it = m_dependency_arcs.begin(); it != m_dependency_arcs.end(); ++it) {
        GG::Clr arc_color;
        switch (it->first) {
        case KNOWN:          arc_color = ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR; break;
        case RESEARCHABLE:   arc_color = ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR; break;
        case UNRESEARCHABLE: arc_color = ClientUI::UNRESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR; break;
        }
        glColor4ubv(arc_color.v);
        for (DependencyArcsMap::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            bool selected_arc = it2->first == m_selected_tech || it2->second.first == m_selected_tech;
            if (selected_arc) {
                selected_arcs[it->first].push_back(it2);
                continue;
            }
            DrawArc(it2, arc_color, true);
        }
    }
    GG::Clr known_selected = GG::LightColor(ClientUI::KNOWN_TECH_TEXT_AND_BORDER_COLOR);
    GG::Clr researchable_selected = GG::LightColor(ClientUI::RESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR);
    GG::Clr unresearchable_selected = GG::LightColor(ClientUI::UNRESEARCHABLE_TECH_TEXT_AND_BORDER_COLOR);
    for (std::map<TechStatus, std::vector<DependencyArcsMap::const_iterator> >::const_iterator it = selected_arcs.begin();
         it != selected_arcs.end();
         ++it) {
        GG::Clr arc_color;
        switch (it->first) {
        case KNOWN:          arc_color = known_selected; break;
        case RESEARCHABLE:   arc_color = researchable_selected; break;
        case UNRESEARCHABLE: arc_color = unresearchable_selected; break;
        }
        glColor4ubv(arc_color.v);
        for (unsigned int i = 0; i < it->second.size(); ++i) {
            DrawArc(it->second[i], arc_color, true);
        }
    }
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
    EndClipping();

    GG::GUI::RenderWindow(m_vscroll);
    GG::GUI::RenderWindow(m_hscroll);
}

void TechTreeWnd::LayoutPanel::Update()
{
    Layout(true);
}

void TechTreeWnd::LayoutPanel::Clear()
{
    m_vscroll->ScrollTo(0);
    m_hscroll->ScrollTo(0);
    m_vscroll->SizeScroll(0, 1, 1, 1);
    m_hscroll->SizeScroll(0, 1, 1, 1);
    for (std::map<const Tech*, TechPanel*>::const_iterator it = m_techs.begin(); it != m_techs.end(); ++it) {
        delete it->second;
    }
    m_techs.clear();
    m_dependency_arcs.clear();
    m_selected_tech = 0;
}

void TechTreeWnd::LayoutPanel::Reset()
{
    Layout(true);
}

void TechTreeWnd::LayoutPanel::SetScale(double scale)
{
    if (scale < MIN_SCALE)
        scale = MIN_SCALE;
    if (MAX_SCALE < scale)
        scale = MAX_SCALE;
    if (m_scale != scale) {
        double old_scale = m_scale;
        m_scale = scale;
        Layout(true, old_scale);
    }
}

void TechTreeWnd::LayoutPanel::ShowCategory(const std::string& category)
{
    if (m_category_shown != category) {
        m_category_shown = category;
        Layout(false);
    }
}

void TechTreeWnd::LayoutPanel::ShowTech(const Tech* tech)
{
    if (m_category_shown != "ALL" && m_category_shown != tech->Category())
        ShowCategory(tech->Category());
    TechClickedSlot(tech);
}

void TechTreeWnd::LayoutPanel::CenterOnTech(const Tech* tech)
{
    std::map<const Tech*, TechPanel*>::const_iterator it = m_techs.find(tech);
    if (it != m_techs.end()) {
        TechPanel* tech_panel = it->second;
        GG::Pt center_point = tech_panel->RelativeUpperLeft() + GG::Pt(tech_panel->Width() / 2, tech_panel->Height() / 2);
        GG::Pt client_size = ClientSize();
        m_hscroll->ScrollTo(center_point.x - client_size.x / 2);
        m_vscroll->ScrollTo(center_point.y - client_size.y / 2);
    }
}

void TechTreeWnd::LayoutPanel::SetTechTypesShown(TechTypesShown tech_types)
{
    m_tech_types_shown = tech_types;
    Layout(true);
}


void TechTreeWnd::LayoutPanel::SetTechStatusesShown(TechStatusesShown tech_statuses)
{
    m_tech_statuses_shown = tech_statuses;
    Layout(true);
}

void TechTreeWnd::LayoutPanel::UncollapseAll()
{
    if (!m_collapsed_subtree_techs_per_view["ALL"].empty()) {
        m_collapsed_subtree_techs_per_view.clear();
        Layout(false);
    }
}

void TechTreeWnd::LayoutPanel::Layout(bool keep_position, double old_scale/* = -1.0*/)
{
    if (old_scale < 0.0)
        old_scale = m_scale;
    GG::Pt final_position;
    if (keep_position) {
        if (m_scale == old_scale) {
            final_position = m_scroll_position;
        } else {
            GG::Pt cl_sz = ClientSize();
            GG::Pt center = m_scroll_position + GG::Pt(cl_sz.x / 2, cl_sz.y / 2);
            center.x = static_cast<int>(center.x * m_scale / old_scale + 0.5);
            center.y = static_cast<int>(center.y * m_scale / old_scale + 0.5);
            final_position = GG::Pt(center.x - cl_sz.x / 2, center.y - cl_sz.y / 2);
        }
    }
    const Tech* selected_tech = keep_position ? m_selected_tech : 0;
    Clear();
    m_selected_tech = selected_tech;

    GVC_t* gvc = gvContext();
    Agraph_t* graph = agopen("FreeOrion Tech Graph", AGDIGRAPHSTRICT);

    const double RANK_SEP = TECH_PANEL_LAYOUT_WIDTH * GetOptionsDB().Get<double>("UI.tech-layout-horz-spacing") * m_scale;
    const double NODE_SEP = TECH_PANEL_LAYOUT_HEIGHT * GetOptionsDB().Get<double>("UI.tech-layout-vert-spacing") * m_scale;
    const double WIDTH = TECH_PANEL_LAYOUT_WIDTH * m_scale;
    const double HEIGHT = TECH_PANEL_LAYOUT_HEIGHT * m_scale;

    // default graph properties
    agraphattr(graph, "rankdir", "LR");
    agraphattr(graph, "ordering", "in");
    agraphattr(graph, "ranksep", const_cast<char*>(boost::lexical_cast<std::string>(RANK_SEP).c_str()));
    agraphattr(graph, "nodesep", const_cast<char*>(boost::lexical_cast<std::string>(NODE_SEP).c_str())); 
    agraphattr(graph, "arrowhead", "none");
    agraphattr(graph, "arrowtail", "none");

    // default node properties
    agnodeattr(graph, "shape", "box");
    agnodeattr(graph, "fixedsize", "true");
    agnodeattr(graph, "width", const_cast<char*>(boost::lexical_cast<std::string>(WIDTH).c_str()));
    agnodeattr(graph, "height", const_cast<char*>(boost::lexical_cast<std::string>(HEIGHT).c_str()));

    // default edge properties
    agedgeattr(graph, "tailclip", "false");

    std::map<std::string, Agnode_t*> name_to_node_map;
    TechManager& manager = GetTechManager();
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        if (!TechVisible(*it))
            continue;
        Agnode_t* node = agnode(graph, const_cast<char*>((*it)->Name().c_str()));
        assert(node);
        name_to_node_map[(*it)->Name()] = node;
    }
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        if (!TechVisible(*it))
            continue;
        for (std::set<std::string>::const_iterator prereq_it = (*it)->Prerequisites().begin();
             prereq_it != (*it)->Prerequisites().end();
             ++prereq_it) {
            if (!TechVisible(GetTech(*prereq_it)))
            continue;
            agedge(graph, name_to_node_map[*prereq_it], name_to_node_map[(*it)->Name()]);
        }
    }

    gvLayout(gvc, graph, "dot");

    // create new tech panels and new dependency arcs
    const int TECH_PANEL_MARGIN = static_cast<int>(10 * m_scale);
    m_dependency_arcs.clear();
    const std::set<const Tech*>& collapsed_subtree_techs = m_collapsed_subtree_techs_per_view[m_category_shown];
    for (Agnode_t* node = agfstnode(graph); node; node = agnxtnode(graph, node)) {
        const Tech* tech = GetTech(node->name);
        assert(tech);
        m_techs[tech] = new TechPanel(tech, tech == m_selected_tech, collapsed_subtree_techs.find(tech) != collapsed_subtree_techs.end(), m_category_shown, m_tech_types_shown, m_tech_statuses_shown, m_scale);
        m_techs[tech]->MoveTo(GG::Pt(static_cast<int>(PS2INCH(ND_coord_i(node).x) - m_techs[tech]->Width() / 2 + TECH_PANEL_MARGIN),
                                     static_cast<int>(PS2INCH(ND_coord_i(node).y) - (m_techs[tech]->Height() - PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale) / 2 + TECH_PANEL_MARGIN)));
        m_layout_surface->AttachChild(m_techs[tech]);
        GG::Connect(m_techs[tech]->TechBrowsedSignal, &TechTreeWnd::LayoutPanel::TechBrowsedSlot, this);
        GG::Connect(m_techs[tech]->TechClickedSignal, &TechTreeWnd::LayoutPanel::TechClickedSlot, this);
        GG::Connect(m_techs[tech]->TechDoubleClickedSignal, &TechTreeWnd::LayoutPanel::TechDoubleClickedSlot, this);
        GG::Connect(m_techs[tech]->CollapseSubtreeSignal(), CollapseSubtreeFunctor(this, tech));

        for (Agedge_t* edge = agfstout(graph, node); edge; edge = agnxtout(graph, edge)) {
            const Tech* from = tech;
            const Tech* to = GetTech(edge->head->name);
            assert(from && to);
            std::vector<std::vector<std::pair<double, double> > > points;
            for (int i = 0; i < ED_spl(edge)->size; ++i) {
                std::vector<std::pair<int, int> > temp;
                for (int j = 0; j < ED_spl(edge)->list[i].size; ++j) {
                    temp.push_back(std::make_pair(static_cast<int>(PS2INCH(ED_spl(edge)->list[i].list[j].x) + TECH_PANEL_MARGIN),
                                                  static_cast<int>(PS2INCH(ED_spl(edge)->list[i].list[j].y) + TECH_PANEL_MARGIN)));
                }
                points.push_back(Spline(temp));
            }
            TechStatus arc_type = UNRESEARCHABLE;
#ifndef FREEORION_BUILD_UTIL
            const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
            if (empire->TechAvailable(to->Name())) {
                arc_type = KNOWN;
            } else if (empire->ResearchableTech(to->Name())) {
                arc_type = RESEARCHABLE;
            }
#else
            // these values are arbitrary; they're only useful for displaying techs in the tech-view utility app
            if (to->Type() == TT_THEORY) {
                arc_type = KNOWN;
            } else if (to->Type() == TT_APPLICATION) {
                arc_type = RESEARCHABLE;
            }
#endif
            m_dependency_arcs[arc_type].insert(std::make_pair(from, std::make_pair(to, points)));
        }
    }

    GG::Pt client_sz = ClientSize();
    GG::Pt layout_size(std::max(client_sz.x,
                                static_cast<int>(PS2INCH(GD_bb(graph).UR.x - GD_bb(graph).LL.x) +
                                                 2 * TECH_PANEL_MARGIN + PROGRESS_PANEL_LEFT_EXTRUSION * m_scale)),
                       std::max(client_sz.y,
                                static_cast<int>(PS2INCH(GD_bb(graph).UR.y - GD_bb(graph).LL.y) +
                                                 2 * TECH_PANEL_MARGIN + PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale)));
    m_layout_surface->Resize(layout_size);
    m_vscroll->SizeScroll(0, layout_size.y - 1, std::max(50, std::min(layout_size.y / 10, client_sz.y)), client_sz.y);
    m_hscroll->SizeScroll(0, layout_size.x - 1, std::max(50, std::min(layout_size.x / 10, client_sz.x)), client_sz.x);

    gvFreeLayout(gvc, graph);
    gvFreeContext(gvc);

    agclose(graph);

    if (keep_position) {
        m_vscroll->ScrollTo(final_position.y);
        m_hscroll->ScrollTo(final_position.x);
    }

    // ensure that the scrolls stay on top
    MoveChildUp(m_vscroll);
    MoveChildUp(m_hscroll);
}

bool TechTreeWnd::LayoutPanel::TechVisible(const Tech* tech)
{
    if ((m_tech_types_shown - TechTreeWnd::THEORY_TECHS) < tech->Type() - TT_THEORY)
        return false;
    if (m_category_shown != "ALL" && tech->Category() != m_category_shown)
        return false;
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (m_tech_statuses_shown == RESEARCHABLE_TECHS) {
        if (empire->TechAvailable(tech->Name()) || !empire->ResearchableTech(tech->Name()))
            return false;
    } else if (m_tech_statuses_shown == COMPLETE_AND_RESEARCHABLE_TECHS) {
        if (!empire->TechAvailable(tech->Name()) && !empire->ResearchableTech(tech->Name()))
            return false;
    }
    std::set<std::string> category_prereqs;
    const std::set<std::string> prereqs = tech->Prerequisites();
    for (std::set<std::string>::const_iterator it = prereqs.begin(); it != prereqs.end(); ++it) {
        const Tech* prereq = GetTech(*it);
        if (m_category_shown == "ALL" || prereq->Category() == m_category_shown)
            category_prereqs.insert(*it);
    }
    if (category_prereqs.size() == 1) {
        const std::set<const Tech*>& collapsed_subtree_techs = m_collapsed_subtree_techs_per_view[m_category_shown];
        const Tech* prereq = GetTech(*category_prereqs.begin());
        if (collapsed_subtree_techs.find(prereq) != collapsed_subtree_techs.end())
            return false;
    }
    return true;
}

void TechTreeWnd::LayoutPanel::CollapseTechSubtree(const Tech* tech, bool collapse)
{
    if (collapse) {
        m_collapsed_subtree_techs_per_view["ALL"].insert(tech);
        m_collapsed_subtree_techs_per_view[tech->Category()].insert(tech);
    } else {
        m_collapsed_subtree_techs_per_view["ALL"].erase(tech);
        m_collapsed_subtree_techs_per_view[tech->Category()].erase(tech);
    }
    Layout(true);
}

void TechTreeWnd::LayoutPanel::DrawArc(DependencyArcsMap::const_iterator it, GG::Clr color, bool with_arrow_head)
{
    GG::Pt ul = UpperLeft();
    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i < it->second.second.size(); ++i) {
        for (unsigned int j = 0; j < it->second.second[i].size(); ++j) {
            glVertex2d(it->second.second[i][j].first + ul.x, it->second.second[i][j].second + ul.y);
        }
    }
    glEnd();
    if (with_arrow_head) {
        double final_point_x = it->second.second.back().back().first + ul.x;
        double final_point_y = it->second.second.back().back().second + ul.y;
        double second_to_final_point_x = it->second.second.back()[it->second.second.back().size() - 2].first + ul.x;
        double second_to_final_point_y = it->second.second.back()[it->second.second.back().size() - 2].second + ul.y;
        const double ARROW_LENGTH = 10 * m_scale;
        const double ARROW_WIDTH = 9 * m_scale;
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        glTranslated(final_point_x, final_point_y, 0.0);
        glRotated(std::atan2(final_point_y - second_to_final_point_y, final_point_x - second_to_final_point_x) * 180.0 / 3.141594, 0.0, 0.0, 1.0);
        glTranslated(-final_point_x, -final_point_y, 0.0);
        IsoscelesTriangle(static_cast<int>(final_point_x - ARROW_LENGTH + 0.5), static_cast<int>(final_point_y - ARROW_WIDTH / 2.0 + 0.5),
                          static_cast<int>(final_point_x + 0.5), static_cast<int>(final_point_y + ARROW_WIDTH / 2.0 + 0.5),
                          SHAPE_RIGHT, color, false);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
    }
}

void TechTreeWnd::LayoutPanel::ScrolledSlot(int, int, int, int)
{
    int scroll_x = m_hscroll->PosnRange().first;
    int scroll_y = m_vscroll->PosnRange().first;
    m_scroll_position.x = scroll_x;
    m_scroll_position.y = scroll_y;
    m_layout_surface->MoveTo(GG::Pt(-scroll_x, -scroll_y));
}

void TechTreeWnd::LayoutPanel::TechBrowsedSlot(const Tech* tech)
{
    TechBrowsedSignal(tech);
}

void TechTreeWnd::LayoutPanel::TechClickedSlot(const Tech* tech)
{
    if (m_selected_tech && m_techs.find(m_selected_tech) != m_techs.end())
        m_techs[m_selected_tech]->Select(false);
    if ((m_selected_tech = tech) && m_techs.find(m_selected_tech) != m_techs.end())
        m_techs[m_selected_tech]->Select(true);
    TechClickedSignal(tech);
}

void TechTreeWnd::LayoutPanel::TechDoubleClickedSlot(const Tech* tech)
{
    TechDoubleClickedSignal(tech);
}

void TechTreeWnd::LayoutPanel::TreeDraggedSlot(const GG::Pt& move)
{
    m_vscroll->ScrollTo(m_vscroll->PosnRange().first - move.y);
    m_hscroll->ScrollTo(m_hscroll->PosnRange().first - move.x);
}

void TechTreeWnd::LayoutPanel::TreeZoomedSlot(int move)
{
    const double ZOOM_STEP_SIZE = 1.25;
    if (0 < move)
        SetScale(m_scale * ZOOM_STEP_SIZE);
    else if (move < 0)
        SetScale(m_scale / ZOOM_STEP_SIZE);
}


//////////////////////////////////////////////////
// TechTreeWnd                                  //
//////////////////////////////////////////////////
TechTreeWnd::TechTreeWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::CLICKABLE),
    m_tech_detail_panel(0),
    m_tech_navigator(0),
    m_layout_panel(0),
    m_tech_type_buttons(0),
    m_tech_status_buttons(0),
    m_uncollapse_all_button(0)
{
    TempUISoundDisabler sound_disabler;

    const int UNCOLLAPSE_ALL_BUTTON_WIDTH = 125;
    const int BUTTON_MARGIN = 4;
    m_uncollapse_all_button = new CUIButton(0, 0, UNCOLLAPSE_ALL_BUTTON_WIDTH, UserString("TECH_WND_UNCOLLAPSE_TECHS"));
    GG::Connect(m_uncollapse_all_button->ClickedSignal, &TechTreeWnd::UncollapseAll, this);
    AttachChild(m_uncollapse_all_button);

    const int NAVIGATOR_WIDTH = 214;
    m_tech_detail_panel = new TechDetailPanel(w - NAVIGATOR_WIDTH, NAVIGATOR_AND_DETAIL_HEIGHT);
    GG::Connect(m_tech_detail_panel->CenterOnTechSignal, &TechTreeWnd::CenterOnTech, this);
    GG::Connect(m_tech_detail_panel->QueueTechSignal, &TechTreeWnd::TechDoubleClickedSlot, this);
    AttachChild(m_tech_detail_panel);
    m_tech_navigator = new TechNavigator(NAVIGATOR_WIDTH, NAVIGATOR_AND_DETAIL_HEIGHT);
    m_tech_navigator->MoveTo(GG::Pt(m_tech_detail_panel->Width(), 0));
    GG::Connect(m_tech_navigator->TechClickedSignal, &TechTreeWnd::CenterOnTech, this);
    AttachChild(m_tech_navigator);

    const int LAYOUT_MARGIN_TOP = NAVIGATOR_AND_DETAIL_HEIGHT + 2 + ClientUI::PTS + 10; // leave room for category buttons at top
    const int LAYOUT_MARGIN_BOTTOM = m_uncollapse_all_button->Height() * 2 + 4; // leave room for tech types checkboxes and uncollapse-all button at bottom
    m_uncollapse_all_button->MoveTo(GG::Pt(w - (UNCOLLAPSE_ALL_BUTTON_WIDTH + 2), h - LAYOUT_MARGIN_BOTTOM / 2 - m_uncollapse_all_button->Height() / 2));

    const std::vector<std::string>& tech_categories = GetTechManager().CategoryNames();
    const int NUM_BUTTONS = tech_categories.size() + 1;
    const int BUTTON_WIDTH = (w - NUM_BUTTONS * BUTTON_MARGIN) / NUM_BUTTONS;
    const int BUTTON_SPACING = BUTTON_WIDTH + BUTTON_MARGIN;
    for (unsigned int i = 0; i < tech_categories.size(); ++i) {
        m_category_buttons.push_back(new CUIButton(BUTTON_MARGIN / 2 + i * BUTTON_SPACING, NAVIGATOR_AND_DETAIL_HEIGHT + 2, BUTTON_WIDTH, UserString(tech_categories[i])));
        GG::Connect(m_category_buttons.back()->ClickedSignal, SelectCategoryFunctor(this, tech_categories[i]));
        AttachChild(m_category_buttons.back());
    }
    m_category_buttons.push_back(new CUIButton(BUTTON_MARGIN / 2 + tech_categories.size() * BUTTON_SPACING, NAVIGATOR_AND_DETAIL_HEIGHT + 2, BUTTON_WIDTH, UserString("ALL")));
    GG::Connect(m_category_buttons.back()->ClickedSignal, SelectCategoryFunctor(this, "ALL"));
    AttachChild(m_category_buttons.back());

    m_layout_panel = new LayoutPanel(w, h - (LAYOUT_MARGIN_TOP + LAYOUT_MARGIN_BOTTOM));
    m_layout_panel->OffsetMove(GG::Pt(0, LAYOUT_MARGIN_TOP));
    GG::Connect(m_layout_panel->TechBrowsedSignal, &TechTreeWnd::TechBrowsedSlot, this);
    GG::Connect(m_layout_panel->TechClickedSignal, &TechTreeWnd::TechClickedSlot, this);
    GG::Connect(m_layout_panel->TechDoubleClickedSignal, &TechTreeWnd::TechDoubleClickedSlot, this);
    AttachChild(m_layout_panel);

    const int RADIO_BUTTON_MARGIN = 30;
    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS);
    int button_start = 2 + 15 + std::max(font->TextExtent(UserString("TECH_WND_TECH_TYPES_TO_SHOW")).x,
                                         font->TextExtent(UserString("TECH_WND_TECH_STATUSES_TO_SHOW")).x);
    int text_width = font->TextExtent(UserString("TECH_WND_TECH_TYPES_TO_SHOW")).x;
    GG::TextControl* tech_types_to_show_label =
        new GG::TextControl(2, h - LAYOUT_MARGIN_BOTTOM, text_width, LAYOUT_MARGIN_BOTTOM / 2,
                            UserString("TECH_WND_TECH_TYPES_TO_SHOW"), GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS),
                            ClientUI::TEXT_COLOR, GG::TF_LEFT);
    int total_text_width =
        font->TextExtent(UserString("TECH_WND_TECH_TYPES_ALL")).x + RADIO_BUTTON_MARGIN +
        font->TextExtent(UserString("TECH_WND_TECH_TYPES_THEORIES_AND_APPS")).x + RADIO_BUTTON_MARGIN +
        font->TextExtent(UserString("TECH_WND_TECH_TYPES_THEORIES_ONLY")).x + RADIO_BUTTON_MARGIN;
    m_tech_type_buttons = new GG::RadioButtonGroup(button_start, h - LAYOUT_MARGIN_BOTTOM, total_text_width, LAYOUT_MARGIN_BOTTOM / 2, GG::HORIZONTAL);
    AttachChild(tech_types_to_show_label);
    text_width = font->TextExtent(UserString("TECH_WND_TECH_TYPES_ALL")).x + RADIO_BUTTON_MARGIN;
    CUIStateButton* button = new CUIStateButton(0, 0, text_width, LAYOUT_MARGIN_BOTTOM / 2, UserString("TECH_WND_TECH_TYPES_ALL"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON);
    m_tech_type_buttons->AddButton(button);
    text_width = font->TextExtent(UserString("TECH_WND_TECH_TYPES_THEORIES_AND_APPS")).x + RADIO_BUTTON_MARGIN;
    button = new CUIStateButton(0, 0, text_width, LAYOUT_MARGIN_BOTTOM / 2, UserString("TECH_WND_TECH_TYPES_THEORIES_AND_APPS"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON);
    m_tech_type_buttons->AddButton(button);
    text_width = font->TextExtent(UserString("TECH_WND_TECH_TYPES_THEORIES_ONLY")).x + RADIO_BUTTON_MARGIN;
    button = new CUIStateButton(0, 0, text_width, LAYOUT_MARGIN_BOTTOM / 2, UserString("TECH_WND_TECH_TYPES_THEORIES_ONLY"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON);
    m_tech_type_buttons->AddButton(button);
    m_tech_type_buttons->SetCheck(0);
    GG::Connect(m_tech_type_buttons->ButtonChangedSignal, &TechTreeWnd::TechTypesShownSlot, this);

    text_width = font->TextExtent(UserString("TECH_WND_TECH_STATUSES_TO_SHOW")).x;
    GG::TextControl* tech_statuses_to_show_label =
        new GG::TextControl(2, h - LAYOUT_MARGIN_BOTTOM / 2, text_width, LAYOUT_MARGIN_BOTTOM / 2,
                            UserString("TECH_WND_TECH_STATUSES_TO_SHOW"), GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS),
                            ClientUI::TEXT_COLOR, GG::TF_LEFT);
    total_text_width =
        font->TextExtent(UserString("TECH_WND_TECH_STATUS_ALL")).x + RADIO_BUTTON_MARGIN +
        font->TextExtent(UserString("TECH_WND_TECH_STATUS_AVAILABLE_AND_COMPLETE")).x + RADIO_BUTTON_MARGIN +
        font->TextExtent(UserString("TECH_WND_TECH_STATUS_AVAILABLE_ONLY")).x + RADIO_BUTTON_MARGIN;
    m_tech_status_buttons = new GG::RadioButtonGroup(button_start, h - LAYOUT_MARGIN_BOTTOM / 2, total_text_width, LAYOUT_MARGIN_BOTTOM / 2, GG::HORIZONTAL);
    AttachChild(tech_statuses_to_show_label);
    text_width = font->TextExtent(UserString("TECH_WND_TECH_STATUS_ALL")).x + RADIO_BUTTON_MARGIN;
    button = new CUIStateButton(0, 0, text_width, LAYOUT_MARGIN_BOTTOM / 2, UserString("TECH_WND_TECH_STATUS_ALL"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON);
    m_tech_status_buttons->AddButton(button);
    text_width = font->TextExtent(UserString("TECH_WND_TECH_STATUS_AVAILABLE_AND_COMPLETE")).x + RADIO_BUTTON_MARGIN;
    button = new CUIStateButton(0, 0, text_width, LAYOUT_MARGIN_BOTTOM / 2, UserString("TECH_WND_TECH_STATUS_AVAILABLE_AND_COMPLETE"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON);
    m_tech_status_buttons->AddButton(button);
    text_width = font->TextExtent(UserString("TECH_WND_TECH_STATUS_AVAILABLE_ONLY")).x + RADIO_BUTTON_MARGIN;
    button = new CUIStateButton(0, 0, text_width, LAYOUT_MARGIN_BOTTOM / 2, UserString("TECH_WND_TECH_STATUS_AVAILABLE_ONLY"), GG::TF_LEFT, CUIStateButton::SBSTYLE_CUI_RADIO_BUTTON);
    m_tech_status_buttons->AddButton(button);
    m_tech_status_buttons->SetCheck(0);
    GG::Connect(m_tech_status_buttons->ButtonChangedSignal, &TechTreeWnd::TechStatusesShownSlot, this);

    AttachChild(m_tech_status_buttons);
    AttachChild(m_tech_type_buttons);
}

double TechTreeWnd::Scale() const
{
    return m_layout_panel->Scale();
}

const std::string& TechTreeWnd::CategoryShown() const
{
    return m_layout_panel->CategoryShown();
}

TechTreeWnd::TechTypesShown TechTreeWnd::GetTechTypesShown() const
{
    return m_layout_panel->GetTechTypesShown();
}

TechTreeWnd::TechStatusesShown TechTreeWnd::GetTechStatusesShown() const
{
    return m_layout_panel->GetTechStatusesShown();
}

void TechTreeWnd::Update()
{
    m_layout_panel->Update();
}

void TechTreeWnd::Clear()
{
    m_tech_navigator->SetTech(0);
    m_tech_detail_panel->SetTech(0);
    m_layout_panel->Clear();
}

void TechTreeWnd::Reset()
{
    m_layout_panel->Reset();
}

void TechTreeWnd::ShowCategory(const std::string& category)
{
    m_layout_panel->ShowCategory(category);
}

void TechTreeWnd::SetScale(double scale)
{
    m_layout_panel->SetScale(scale);
}

void TechTreeWnd::SetTechTypesShown(TechTypesShown tech_types)
{
    m_layout_panel->SetTechTypesShown(tech_types);
}

void TechTreeWnd::SetTechStatusesShown(TechStatusesShown tech_statuses)
{
    m_layout_panel->SetTechStatusesShown(tech_statuses);
}

void TechTreeWnd::UncollapseAll()
{
    m_layout_panel->UncollapseAll();
}

void TechTreeWnd::CenterOnTech(const Tech* tech)
{
    m_layout_panel->ShowTech(tech);
    m_layout_panel->CenterOnTech(tech);
}

void TechTreeWnd::TechBrowsedSlot(const Tech* tech)
{
    TechBrowsedSignal(tech);
}

void TechTreeWnd::TechClickedSlot(const Tech* tech)
{
    m_tech_navigator->SetTech(tech);
    m_tech_detail_panel->SetTech(tech);
    TechSelectedSignal(tech);
}

void TechTreeWnd::TechDoubleClickedSlot(const Tech* tech)
{
    AddTechToQueueSignal(tech);
}

void TechTreeWnd::TechTypesShownSlot(int types)
{
    SetTechTypesShown(TechTypesShown(2 - types));
}

void TechTreeWnd::TechStatusesShownSlot(int statuses)
{
    SetTechStatusesShown(TechStatusesShown(2 - statuses));
}
