#include "TechTreeWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "CUIWnd.h"
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
#include <algorithm>


namespace {
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("UI.tech-layout-horz-spacing", "OPTIONS_DB_UI_TECH_LAYOUT_HORZ_SPACING", 0.75, RangedValidator<double>(0.1, 10.0));
        db.Add("UI.tech-layout-vert-spacing", "OPTIONS_DB_UI_TECH_LAYOUT_VERT_SPACING", 1.0, RangedValidator<double>(0.1, 10.0));
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

    const double MIN_SCALE = 0.1073741824;  // = 1.0/(1.25)^10
    const double MAX_SCALE = 1.0;

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
        GG::Clr progress_background_color = ClientUI::TechWndProgressBarBackground();
        GG::Clr progress_color = ClientUI::TechWndProgressBar();
        glColor(color);
        int progress_extent = (0.0 < progress && progress < 1.0) ? (progress_panel.ul.x + static_cast<int>(progress * PROGRESS_PANEL_WIDTH + 0.5)) : 0;
        if (tech_type == TT_THEORY) {
            FillTheoryPanel(main_panel, MAIN_PANEL_CORNER_RADIUS);
            if (show_progress) {
                if (progress_extent) {
                    glColor(progress_background_color);
                    FillTheoryPanel(progress_panel, PROGRESS_PANEL_CORNER_RADIUS);
                    glColor(progress_color);
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
                    glColor(progress_background_color);
                    FillApplicationPanel(progress_panel, PROGRESS_PANEL_CORNER_RADIUS);
                    glColor(progress_color);
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
                    glColor(progress_background_color);
                    FillRefinementPanel(progress_panel);
                    glColor(progress_color);
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

    struct ToggleCategoryFunctor
    {
        ToggleCategoryFunctor(TechTreeWnd* tree_wnd, const std::string& category) : m_tree_wnd(tree_wnd), m_category(category) {}
        void operator()() {m_tree_wnd->ToggleCategory(m_category);}
        TechTreeWnd* const m_tree_wnd;
        const std::string m_category;
    };

    struct ToggleAllCategoriesFunctor
    {
        ToggleAllCategoriesFunctor(TechTreeWnd* tree_wnd) : m_tree_wnd(tree_wnd) {}
        void operator()() {m_tree_wnd->ToggleAllCategories();}
        TechTreeWnd* const m_tree_wnd;
    };

    struct ToggleTechStatusFunctor
    {
        ToggleTechStatusFunctor(TechTreeWnd* tree_wnd, TechStatus status) : m_tree_wnd(tree_wnd), m_status(status) {}
        void operator()() {m_tree_wnd->ToggleStatus(m_status);}
        TechTreeWnd* const m_tree_wnd;
        const TechStatus m_status;
    };

    struct ToggleTechTypeFunctor
    {
        ToggleTechTypeFunctor(TechTreeWnd* tree_wnd, TechType type) : m_tree_wnd(tree_wnd), m_type(type) {}
        void operator()() {m_tree_wnd->ToggleType(m_type);}
        TechTreeWnd* const m_tree_wnd;
        const TechType m_type;
    };
}

//////////////////////////////////////////////////
// TechTreeWnd::TechTreeControls                //
//////////////////////////////////////////////////
/** A panel of buttons that control how the tech tree is displayed: what categories, statuses and
    types of techs to show. */
class TechTreeWnd::TechTreeControls : public CUIWnd
{
public:
    //! \name Structors //@{
    TechTreeControls(int x, int y, int w);
    //@}

    //! \name Mutators //@{
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void Render();
    virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys);
    //@}

private:
    void DoButtonLayout();

    int m_buttons_per_row;
    int m_col_offset;  //!< horizontal distance between each column of buttons
    int m_row_offset;  //!< vertical distance between each row of buttons
    int m_category_button_rows;
    int m_status_or_type_button_rows;

    static const int BUTTON_SEPARATION = 3;     // vertical or horizontal sepration between adjacent buttons
    static const int UPPER_LEFT_PAD = 2;        // offset of buttons' position from top left of controls box

    std::vector<CUIButton*> m_category_buttons;
    std::map<TechType, CUIButton*> m_tech_type_buttons;
    std::map<TechStatus, CUIButton*> m_tech_status_buttons;

    friend class TechTreeWnd;                   // so TechTreeWnd can access buttons
};

TechTreeWnd::TechTreeControls::TechTreeControls(int x, int y, int w) :
    CUIWnd(UserString("TECH_DISPLAY"), x, y, w, 10, GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE)
{
    // create a button for each tech category...
    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    for (unsigned int i = 0; i < cats.size(); ++i) {
        m_category_buttons.push_back(new CUIButton(0, 0, 20, UserString(cats[i])));
        AttachChild(m_category_buttons.back());
        m_category_buttons.back()->MarkSelectedTechCategoryColor(cats[i]);
    }
    // and one for "ALL"
    m_category_buttons.push_back(new CUIButton(0, 0, 20, UserString("ALL")));
    AttachChild(m_category_buttons.back());
    m_category_buttons.back()->MarkNotSelected();

    // create a button for each tech type
    m_tech_type_buttons[TT_THEORY] = new CUIButton(0, 0, 20, UserString("TECH_WND_TYPE_THEORIES"));
    AttachChild(m_tech_type_buttons[TT_THEORY]);
    m_tech_type_buttons[TT_APPLICATION] = new CUIButton(0, 0, 20, UserString("TECH_WND_TYPE_APPLICATIONS"));
    AttachChild(m_tech_type_buttons[TT_APPLICATION]);
    m_tech_type_buttons[TT_REFINEMENT] = new CUIButton(0, 0, 20, UserString("TECH_WND_TYPE_REFINEMENTS"));
    AttachChild(m_tech_type_buttons[TT_REFINEMENT]);
    // colour
    for (std::map<TechType, CUIButton*>::iterator it = m_tech_type_buttons.begin(); it != m_tech_type_buttons.end(); ++it)
        it->second->MarkSelectedGray();
    
    // create a button for each tech status
    m_tech_status_buttons[TS_UNRESEARCHABLE] = new CUIButton(0, 0, 20, UserString("TECH_WND_STATUS_UNRESEARCHABLE"));
    AttachChild(m_tech_status_buttons[TS_UNRESEARCHABLE]);
    m_tech_status_buttons[TS_RESEARCHABLE] = new CUIButton(0, 0, 20, UserString("TECH_WND_STATUS_RESEARCHABLE"));
    AttachChild(m_tech_status_buttons[TS_RESEARCHABLE]);
    m_tech_status_buttons[TS_COMPLETE] = new CUIButton(0, 0, 20, UserString("TECH_WND_STATUS_COMPLETED"));
    AttachChild(m_tech_status_buttons[TS_COMPLETE]);
    // colour
    for (std::map<TechStatus, CUIButton*>::iterator it = m_tech_status_buttons.begin(); it != m_tech_status_buttons.end(); ++it)
        it->second->MarkSelectedGray();
    
    EnableChildClipping(true);
    DoButtonLayout();
    Resize(GG::Pt(Width(), MinSize().y));
}

void TechTreeWnd::TechTreeControls::DoButtonLayout()
{
    const int RIGHT_EDGE_PAD = 8;
    const int USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, 1);   // space in which to do layout
    const int PTS = ClientUI::Pts();
    const int PTS_WIDE = PTS/2;  // how wide per character the font needs... not sure how better to get this
    const int MIN_BUTTON_WIDTH = PTS_WIDE*18;    // rough guesstimate...
    const int MAX_BUTTONS_PER_ROW = std::max(USABLE_WIDTH / (MIN_BUTTON_WIDTH + BUTTON_SEPARATION), 1);

    const float NUM_CATEGORY_BUTTONS = static_cast<float>(m_category_buttons.size());
    const int ROWS = static_cast<int>(std::ceil(NUM_CATEGORY_BUTTONS / MAX_BUTTONS_PER_ROW));
    m_buttons_per_row = static_cast<int>(std::ceil(NUM_CATEGORY_BUTTONS / ROWS));   // number of buttons in a typical row
    
    const int BUTTON_WIDTH = (USABLE_WIDTH - (m_buttons_per_row - 1)*BUTTON_SEPARATION) / m_buttons_per_row;
    const int BUTTON_HEIGHT = m_category_buttons.back()->Height();

    m_col_offset = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    m_row_offset = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons
    
    int row = 0, col = -1;
    for (std::vector<CUIButton*>::iterator it = m_category_buttons.begin(); it != m_category_buttons.end(); ++it) {
        ++col;
        if (col >= m_buttons_per_row) {
            ++row;
            col = 0;
        }
        GG::Pt ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD+ row*m_row_offset);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        (*it)->SizeMove(ul, lr);
    }

    col = -1;
    m_category_button_rows = ++row;  // rowbreak after category buttons, before type and status buttons
    for (std::map<TechType, CUIButton*>::iterator it = m_tech_type_buttons.begin(); it != m_tech_type_buttons.end(); ++it) {
        ++col;
        if (col >= m_buttons_per_row) {
            ++row;
            col = 0;
        }
        GG::Pt ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        it->second->SizeMove(ul, lr);
    }

    // if all six status + type buttons can't fit on one row put an extra row break between them
    if (m_buttons_per_row < 6) {
        col = -1;
        ++row;
    }
    for (std::map<TechStatus, CUIButton*>::iterator it = m_tech_status_buttons.begin(); it != m_tech_status_buttons.end(); ++it) {
        ++col;
        if (col >= m_buttons_per_row) {
            ++row;
            col = 0;
        }
        GG::Pt ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        it->second->SizeMove(ul, lr);
    }

    if (m_buttons_per_row == 1)
        m_status_or_type_button_rows = 3;   // three rows, one button per row
    else if (m_buttons_per_row == 2)
        m_status_or_type_button_rows = 2;   // two rows, one with two buttons, one with one button
    else
        m_status_or_type_button_rows = 1;   // only one row, three buttons per row

    // prevent window from being shrunk less than one button width, or current number of rows of height
    SetMinSize(GG::Pt(UPPER_LEFT_PAD + MIN_BUTTON_WIDTH + 3*RIGHT_EDGE_PAD, CUIWnd::BORDER_TOP + CUIWnd::BORDER_BOTTOM + UPPER_LEFT_PAD + (++row)*m_row_offset));
}

void TechTreeWnd::TechTreeControls::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    // maybe later do something interesting with docking
    CUIWnd::SizeMove(ul, lr);                               // set width and upper left as user-requested
    DoButtonLayout();                                       // given set width, position buttons and set appropriate minimum height
    CUIWnd::SizeMove(ul, GG::Pt(lr.x, ul.y + MinSize().y)); // width and upper left unchanged.  set height to minimum height
}

void TechTreeWnd::TechTreeControls::Render()
{
    CUIWnd::Render();

    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft();
    GG::Pt cl_lr = ClientLowerRight();

    // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    glBegin(GL_LINES);
        glColor(ClientUI::WndOuterBorderColor());

        int category_bottom = cl_ul.y + m_category_button_rows*m_row_offset - BUTTON_SEPARATION/2 + UPPER_LEFT_PAD;
        
        glVertex2i(cl_ul.x, category_bottom);
        glVertex2i(cl_lr.x - 1, category_bottom);

        if (m_buttons_per_row >= 6) {
            // all six status and type buttons are on one row, and need a vertical separator between them
            int middle = cl_ul.x + m_col_offset*3 - BUTTON_SEPARATION/2 + UPPER_LEFT_PAD;
            glVertex2i(middle, category_bottom);
            glVertex2i(middle, cl_lr.y - 1);
            
        } else {
            // the status and type buttons are split into separate vertical groups, and need a horiztonal separator between them
            int status_bottom = category_bottom + m_status_or_type_button_rows*m_row_offset;
            glVertex2i(cl_ul.x, status_bottom);
            glVertex2i(cl_lr.x - 1, status_bottom);
        }
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}


void TechTreeWnd::TechTreeControls::LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys)
{
    if (m_drag_offset != GG::Pt(-1, -1)) {  // resize-dragging
        GG::Pt new_lr = pt - m_drag_offset;

        new_lr.y = this->LowerRight().y;    // ignore y-resizes

        // constrain to within parent
        if (GG::Wnd* parent = Parent()) {
            GG::Pt max_lr = parent->ClientLowerRight();
            new_lr.x = std::min(new_lr.x, max_lr.x);
        }        

        Resize(new_lr - UpperLeft());
    } else {    // normal-dragging
        GG::Pt final_move = move;

        if (GG::Wnd* parent = Parent()) {
            GG::Pt ul = UpperLeft(), lr = LowerRight();
            GG::Pt new_ul = ul + move, new_lr = lr + move;

            GG::Pt min_ul = parent->ClientUpperLeft() + GG::Pt(1, 1);
            GG::Pt max_lr = parent->ClientLowerRight();
            GG::Pt max_ul = max_lr - this->Size();

            new_ul.x = std::max(min_ul.x, std::min(max_ul.x, new_ul.x));
            new_ul.y = std::max(min_ul.y, std::min(max_ul.y, new_ul.y));

            final_move = new_ul - ul;
        }

        GG::Wnd::LDrag(pt, final_move, keys);
    }
}


//////////////////////////////////////////////////
// TechTreeWnd::TechDetailPanel                 //
//////////////////////////////////////////////////
/** A panel of text controls and/or graphics that shows a greater level of detail about a tech (such
    as its full description text and effects text) than what appears in the tech graph. */
class TechTreeWnd::TechDetailPanel : public CUIWnd
{
public:
    TechDetailPanel(int w, int h);

    const Tech* CurrentTech() const {return m_tech;}

    void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    void Render();
    void LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys);

    /* need to redefine this so that icons and name can be put at the top of the Wnd, rather
       than being restricted to the client area of a CUIWnd */
    GG::Pt ClientUpperLeft() const;

    void SetTech(const Tech* tech)  {m_tech = tech; Reset();}

private:
    static const int TEXT_MARGIN_X = 3;
    static const int TEXT_MARGIN_Y = 3;

    void Reset();
    void DoLayout();

    const Tech*         m_tech;
    GG::TextControl*    m_tech_name_text;
    GG::TextControl*    m_cost_text;
    GG::TextControl*    m_summary_text;
    CUIMultiEdit*       m_description_box;
    GG::StaticGraphic*  m_tech_graphic;
    GG::StaticGraphic*  m_category_graphic;
};

TechTreeWnd::TechDetailPanel::TechDetailPanel(int w, int h) :
    CUIWnd("", 1, 1, w - 1, h - 1, GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE),
    m_tech(0)
{
    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int COST_PTS = PTS;
    const int SUMMARY_PTS = PTS*4/3;

    m_tech_name_text = new GG::TextControl(0, 0, 10, 10, "", GG::GUI::GetGUI()->GetFont(ClientUI::FontBold(), NAME_PTS), ClientUI::TextColor());
    m_cost_text =      new GG::TextControl(0, 0, 10, 10, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), COST_PTS), ClientUI::TextColor());
    m_summary_text =   new GG::TextControl(0, 0, 10, 10, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), SUMMARY_PTS), ClientUI::TextColor());
    m_description_box =   new CUIMultiEdit(0, 0, 10, 10, "", GG::TF_WORDBREAK | GG::MultiEdit::READ_ONLY);
    m_description_box->SetColor(GG::CLR_ZERO);
    m_description_box->SetInteriorColor(GG::CLR_ZERO);

    m_tech_graphic = 0;
    m_category_graphic = 0;

    AttachChild(m_tech_name_text);
    AttachChild(m_cost_text);
    AttachChild(m_summary_text);
    AttachChild(m_description_box);

    DoLayout();
}

void TechTreeWnd::TechDetailPanel::DoLayout()
{
    const int PTS = ClientUI::Pts();
    const int NAME_PTS = PTS*3/2;
    const int COST_PTS = PTS;
    const int SUMMARY_PTS = PTS*4/3;

    const int ICON_SIZE = 12 + NAME_PTS + COST_PTS + SUMMARY_PTS;

    // name
    GG::Pt ul = GG::Pt(0, 0);
    GG::Pt lr = ul + GG::Pt(Width(), NAME_PTS + 4);
    m_tech_name_text->SizeMove(ul, lr);

    // cost / turns
    ul += GG::Pt(0, m_tech_name_text->Height());
    lr = ul + GG::Pt(Width(), COST_PTS + 4);
    m_cost_text->SizeMove(ul, lr);

    // one line summary
    ul += GG::Pt(0, m_cost_text->Height());
    lr = ul + GG::Pt(Width(), SUMMARY_PTS + 4);
    m_summary_text->SizeMove(ul, lr);

    // main verbose description (fluff, effects, unlocks, ...)
    ul = GG::Pt(1, ICON_SIZE + TEXT_MARGIN_Y + 1);
    lr = ul + GG::Pt(Width() - TEXT_MARGIN_X - BORDER_RIGHT, Height() - BORDER_BOTTOM - ul.y - TEXT_MARGIN_Y);
    m_description_box->SizeMove(ul, lr);

    // icons
    if (m_tech_graphic) {
        ul = GG::Pt(1, 1);
        lr = ul + GG::Pt(ICON_SIZE, ICON_SIZE);
        m_tech_graphic->SizeMove(ul, lr);
    }
    if (m_category_graphic) {
        lr = GG::Pt(Width() - BORDER_RIGHT, ICON_SIZE + 1);
        ul = lr - GG::Pt(ICON_SIZE, ICON_SIZE);
        m_category_graphic->SizeMove(ul, lr);
    }
}

void TechTreeWnd::TechDetailPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Pt old_size = GG::Wnd::LowerRight() - GG::Wnd::UpperLeft();

    // maybe later do something interesting with docking
    GG::Wnd::SizeMove(ul, lr);

    if (Visible() && old_size != GG::Wnd::Size())
        DoLayout();
}

GG::Pt TechTreeWnd::TechDetailPanel::ClientUpperLeft() const
{
    return GG::Wnd::UpperLeft();
}

void TechTreeWnd::TechDetailPanel::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    const int ICON_SIZE = m_summary_text->LowerRight().y - m_tech_name_text->UpperLeft().y;
    GG::Pt cl_ul = ul + GG::Pt(BORDER_LEFT, ICON_SIZE + BORDER_BOTTOM);
    GG::Pt cl_lr = lr - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);

   // use GL to draw the lines
    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
        glVertex2i(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // draw outer border on pixel inside of the outer edge of the window
    glPolygonMode(GL_BACK, GL_LINE);
    glBegin(GL_POLYGON);
        glColor(ClientUI::WndOuterBorderColor());
        glVertex2i(ul.x, ul.y);
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
        glVertex2i(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
        glVertex2i(ul.x, lr.y);
        glVertex2i(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    // draw inner border, including extra resize-tab lines
    glBegin(GL_LINE_STRIP);
        glColor(ClientUI::WndInnerBorderColor());
        glVertex2i(cl_ul.x, cl_ul.y);
        glVertex2i(cl_lr.x, cl_ul.y);
        glVertex2i(cl_lr.x, cl_lr.y - INNER_BORDER_ANGLE_OFFSET);
        glVertex2i(cl_lr.x - INNER_BORDER_ANGLE_OFFSET, cl_lr.y);
        glVertex2i(cl_ul.x, cl_lr.y);
        glVertex2i(cl_ul.x, cl_ul.y);
    glEnd();
    glBegin(GL_LINES);
        // draw the extra lines of the resize tab
        glColor(ClientUI::WndInnerBorderColor());
        glVertex2i(cl_lr.x, cl_lr.y - RESIZE_HASHMARK1_OFFSET);
        glVertex2i(cl_lr.x - RESIZE_HASHMARK1_OFFSET, cl_lr.y);
        
        glVertex2i(cl_lr.x, cl_lr.y - RESIZE_HASHMARK2_OFFSET);
        glVertex2i(cl_lr.x - RESIZE_HASHMARK2_OFFSET, cl_lr.y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void TechTreeWnd::TechDetailPanel::LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys)
{
    if (m_drag_offset != GG::Pt(-1, -1)) {  // resize-dragging
        GG::Pt new_lr = pt - m_drag_offset;

        // constrain to within parent
        if (GG::Wnd* parent = Parent()) {
            GG::Pt max_lr = parent->ClientLowerRight();
            new_lr.x = std::min(new_lr.x, max_lr.x);
            new_lr.y = std::min(new_lr.y, max_lr.y);
        }        

        Resize(new_lr - UpperLeft());
    } else {    // normal-dragging
        GG::Pt final_move = move;

        if (GG::Wnd* parent = Parent()) {
            GG::Pt ul = UpperLeft(), lr = LowerRight();
            GG::Pt new_ul = ul + move, new_lr = lr + move;

            GG::Pt min_ul = parent->ClientUpperLeft() + GG::Pt(1, 1);
            GG::Pt max_lr = parent->ClientLowerRight();
            GG::Pt max_ul = max_lr - this->Size();

            new_ul.x = std::max(min_ul.x, std::min(max_ul.x, new_ul.x));
            new_ul.y = std::max(min_ul.y, std::min(max_ul.y, new_ul.y));

            final_move = new_ul - ul;
        }

        GG::Wnd::LDrag(pt, final_move, keys);
    }
}

void TechTreeWnd::TechDetailPanel::Reset()
{
    if (m_tech_graphic) {
        DeleteChild(m_tech_graphic);
        m_tech_graphic = 0;
    }
    if (m_category_graphic) {
        DeleteChild(m_category_graphic);
        m_category_graphic = 0;
    }

    if (!m_tech) {
        m_tech_name_text->SetText("");
        m_summary_text->SetText("");
        m_cost_text->SetText("");
        m_description_box->SetText("");
        return;
    }

    m_tech_graphic = new GG::StaticGraphic(0, 0, 10, 10,
                                           ClientUI::TechTexture(m_tech->Name()),
                                           GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
    m_tech_graphic->Show();
    m_tech_graphic->SetColor(ClientUI::CategoryColor(m_tech->Category()));
    AttachChild(m_tech_graphic);

    m_category_graphic = new GG::StaticGraphic(0, 0, 10, 10,
                                               ClientUI::CategoryIcon(m_tech->Category()),
                                               GG::GR_FITGRAPHIC | GG::GR_PROPSCALE);
    m_category_graphic->Show();
    m_category_graphic->SetColor(ClientUI::CategoryColor(m_tech->Category()));
    AttachChild(m_category_graphic);


    DoLayout();

    m_tech_name_text->SetText(UserString(m_tech->Name()));
    using boost::io::str;
    using boost::format;

    m_summary_text->SetText("<i>" + str(format(UserString("TECH_DETAIL_TYPE_STR"))
        % UserString(m_tech->Category())
        % UserString(boost::lexical_cast<std::string>(m_tech->Type()))) + " - "
        + str(format(UserString(m_tech->ShortDescription()))) + "</i>");

    m_summary_text->SetColor(ClientUI::CategoryColor(m_tech->Category()));
 
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

//////////////////////////////////////////////////
// TechTreeWnd::TechNavigator                   //
//////////////////////////////////////////////////
/** A window with a single lisbox in it.  The listbox represents the techs that are required for and are
    unlocked by some tech.  Clicking on a prereq or unlocked tech will bring up that tech. */
class TechTreeWnd::TechNavigator : public CUIWnd
{
public:
    TechNavigator(int w, int h);

    const Tech* CurrentTech() const {return m_current_tech;}
    void SetTech(const Tech* tech) {m_current_tech = tech; Reset();}
    void TechClickedSlot(const Tech* tech) {TechClickedSignal(tech);}

    mutable boost::signal<void (const Tech*)> TechClickedSignal;

    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys);

private:
    /** A control with a label \a str on it, and that is rendered partially onto the next row.
        The "Requires" and "Unlocks" rows are in of this class. */
    class SectionHeaderControl : public GG::Control
    {
    public:
        SectionHeaderControl(const std::string& str);
        virtual void Render();
        virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
        GG::TextControl* m_label;
    };

    /** The control in a single cell of a row with a tech in it. */
    class TechControl : public GG::Control
    {
    public:
        TechControl(const Tech* tech);
        virtual GG::Pt ClientUpperLeft() const {return UpperLeft() + GG::Pt(3, 2);}
        virtual GG::Pt ClientLowerRight() const {return LowerRight() - GG::Pt(2, 2);}
        virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
        virtual void Render();
        virtual void LClick(const GG::Pt& pt, Uint32 keys) {ClickedSignal(m_tech);}
        virtual void MouseEnter(const GG::Pt& pt, Uint32 keys) {m_selected = true;}
        virtual void MouseLeave() {m_selected = false;}

        mutable boost::signal<void (const Tech*)> ClickedSignal;

    private:
        const Tech * const m_tech;
        GG::Clr m_border_color;
        GG::TextControl* m_name_text;
        bool m_selected;
    };

    static const int TECH_ROW_INDENTATION = 8;
    static const int LB_MARGIN_X = 5;
    static const int LB_MARGIN_Y = 5;
    
    GG::ListBox::Row* NewSectionHeaderRow(const std::string& str);
    GG::ListBox::Row* NewTechRow(const Tech* tech);
    void Reset();

    void DoLayout();

    const Tech* m_current_tech;
    GG::ListBox* m_lb;
};

TechTreeWnd::TechNavigator::TechNavigator(int w, int h) :
    CUIWnd(UserString("TECH_NAVIGATION"), 0, 0, w, h, GG::CLICKABLE | GG::DRAGABLE | GG::RESIZABLE),
    m_current_tech(0)
{
    m_lb = new CUIListBox(LB_MARGIN_X, LB_MARGIN_Y, 100, 100);    // resized later when TechNavigator is SizeMoved
    m_lb->SetStyle(GG::LB_NOSORT | GG::LB_NOSEL);
    AttachChild(m_lb);
}

GG::ListBox::Row* TechTreeWnd::TechNavigator::NewSectionHeaderRow(const std::string& str)
{
    GG::ListBox::Row* retval = new GG::ListBox::Row(m_lb->Width(), 3*ClientUI::Pts()/2 + 4, "");
    retval->push_back(new SectionHeaderControl(str));
    return retval;
}

GG::ListBox::Row* TechTreeWnd::TechNavigator::NewTechRow(const Tech* tech)
{
    TechControl* control = new TechControl(tech);
    GG::Connect(control->ClickedSignal, &TechTreeWnd::TechNavigator::TechClickedSlot, this);
    GG::ListBox::Row* retval = new GG::ListBox::Row(m_lb->Width(), 3*ClientUI::Pts()/2 + 4, "");
    retval->push_back(control);
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
    
    DoLayout();
}

void TechTreeWnd::TechNavigator::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    // maybe later do something interesting with docking
    CUIWnd::SizeMove(ul, lr);

    DoLayout();
}

void TechTreeWnd::TechNavigator::DoLayout()
{
    m_lb->Resize(ClientSize() - GG::Pt(2*LB_MARGIN_X, 2*LB_MARGIN_Y));

    for (int i = 0; i < m_lb->NumRows(); ++i) {
        GG::ListBox::Row& row = m_lb->GetRow(i);
        GG::Pt size = GG::Pt(m_lb->Width() - 4*LB_MARGIN_X, row.Height());
        row.Resize(size);
        GG::Control* control = row.at(0);
        control->Resize(size);
    }
}

void TechTreeWnd::TechNavigator::LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys)
{
    if (m_drag_offset != GG::Pt(-1, -1)) {  // resize-dragging
        GG::Pt new_lr = pt - m_drag_offset;

        // constrain to within parent
        if (GG::Wnd* parent = Parent()) {
            GG::Pt max_lr = parent->ClientLowerRight();
            new_lr.x = std::min(new_lr.x, max_lr.x);
            new_lr.y = std::min(new_lr.y, max_lr.y);
        }        

        Resize(new_lr - UpperLeft());
    } else {    // normal-dragging
        GG::Pt final_move = move;

        if (GG::Wnd* parent = Parent()) {
            GG::Pt ul = UpperLeft(), lr = LowerRight();
            GG::Pt new_ul = ul + move, new_lr = lr + move;

            GG::Pt min_ul = parent->ClientUpperLeft() + GG::Pt(1, 1);
            GG::Pt max_lr = parent->ClientLowerRight();
            GG::Pt max_ul = max_lr - this->Size();

            new_ul.x = std::max(min_ul.x, std::min(max_ul.x, new_ul.x));
            new_ul.y = std::max(min_ul.y, std::min(max_ul.y, new_ul.y));

            final_move = new_ul - ul;
        }

        GG::Wnd::LDrag(pt, final_move, keys);
    }
}

TechTreeWnd::TechNavigator::SectionHeaderControl::SectionHeaderControl(const std::string& str) :
    GG::Control(0, 0, 10, 3*ClientUI::Pts()/2 + 4)
{
    m_label = new GG::TextControl(0, 0, 10, 3*ClientUI::Pts()/2 + 4, str,
                                  GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                  ClientUI::KnownTechTextAndBorderColor(), GG::TF_LEFT);
    AttachChild(m_label);
}

void TechTreeWnd::TechNavigator::SectionHeaderControl::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight() + GG::Pt(0, 11);
    glDisable(GL_TEXTURE_2D);
    const int CORNER_RADIUS = 5;
    glColor(ClientUI::KnownTechFillColor());
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
    glColor4ub(ClientUI::KnownTechTextAndBorderColor().r, ClientUI::KnownTechTextAndBorderColor().g, ClientUI::KnownTechTextAndBorderColor().b, 127);
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
    glColor(ClientUI::KnownTechTextAndBorderColor());
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

void TechTreeWnd::TechNavigator::SectionHeaderControl::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Wnd::SizeMove(ul, lr);
    GG::Pt label_ul(TECH_ROW_INDENTATION, 0);
    GG::Pt label_lr(ClientWidth(), ClientHeight());
    m_label->SizeMove(label_ul, label_lr);
}

TechTreeWnd::TechNavigator::TechControl::TechControl(const Tech* tech) :
    GG::Control(0, 0, 10, 3*ClientUI::Pts()/2 + 4),
    m_tech(tech),
    m_selected(false)
{
    EnableChildClipping(true);
#ifndef FREEORION_BUILD_UTIL
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire->TechResearched(m_tech->Name())) {
        SetColor(ClientUI::KnownTechFillColor());
        m_border_color = ClientUI::KnownTechTextAndBorderColor();
    } else if (empire->ResearchableTech(m_tech->Name())) {
        SetColor(ClientUI::ResearchableTechFillColor());
        m_border_color = ClientUI::ResearchableTechTextAndBorderColor();
    } else {
        SetColor(ClientUI::UnresearchableTechFillColor());
        m_border_color = ClientUI::UnresearchableTechTextAndBorderColor();
    }
#else
    // these values are arbitrary; they're only useful for displaying techs in the tech-view utility app
    if (m_tech->Type() == TT_THEORY) {
        SetColor(ClientUI::KnownTechFillColor());
        m_border_color = ClientUI::KnownTechTextAndBorderColor();
    } else if (m_tech->Type() == TT_APPLICATION) {
        SetColor(ClientUI::ResearchableTechFillColor());
        m_border_color = ClientUI::ResearchableTechTextAndBorderColor();
    } else {
        SetColor(ClientUI::UnresearchableTechFillColor());
        m_border_color = ClientUI::UnresearchableTechTextAndBorderColor();
    }
#endif
    GG::Pt client_size = ClientSize();
    m_name_text = new GG::TextControl(0, 0, 10, 3*ClientUI::Pts()/2 + 4, UserString(m_tech->Name()),
                                      GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
                                      m_border_color, GG::TF_LEFT);
    AttachChild(m_name_text);
}

void TechTreeWnd::TechNavigator::TechControl::Render()
{
    GG::Rect rect(UpperLeft(), LowerRight());
    rect += GG::Pt(TECH_ROW_INDENTATION, 0);
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
    glColor(border_color_to_use);
    TraceTechPanelOutline(tech_type, rect, GG::Rect(), false);
    glEnable(GL_TEXTURE_2D);
}

void TechTreeWnd::TechNavigator::TechControl::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Wnd::SizeMove(ul, lr);
    GG::Pt text_ul(TECH_ROW_INDENTATION, 0);
    GG::Pt text_lr(ClientWidth() - TECH_ROW_INDENTATION, ClientHeight());
    m_name_text->SizeMove(text_ul, text_lr);
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

    double                  Scale() const;
    std::set<std::string>   GetCategoriesShown() const;
    std::set<TechType>      GetTechTypesShown() const;
    std::set<TechStatus>    GetTechStatusesShown() const;

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
    void HideCategory(const std::string& category);
    void ShowType(TechType type);
    void HideType(TechType type);
    void ShowStatus(TechStatus status);
    void HideStatus(TechStatus status);
    void ShowTech(const Tech* tech);
    void CenterOnTech(const Tech* tech);
    //@}

    static const double ZOOM_STEP_SIZE;

private:
    class TechPanel;
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
    void DrawArc(DependencyArcsMap::const_iterator it, GG::Clr color, bool with_arrow_head);
    void ScrolledSlot(int, int, int, int);
    void TechBrowsedSlot(const Tech* tech);
    void TechClickedSlot(const Tech* tech);
    void TechDoubleClickedSlot(const Tech* tech);
    void TreeDraggedSlot(const GG::Pt& move);
    void TreeZoomedSlot(int move);
    void TreeZoomInClicked();
    void TreeZoomOutClicked();

    double                  m_scale;
    std::set<std::string>   m_categories_shown;
    std::set<TechType>      m_tech_types_shown;
    std::set<TechStatus>    m_tech_statuses_shown;
    const Tech*             m_selected_tech;

    std::map<const Tech*, TechPanel*> m_techs;
    DependencyArcsMapsByArcType m_dependency_arcs;

    LayoutSurface* m_layout_surface;
    CUIScroll*     m_vscroll;
    CUIScroll*     m_hscroll;
    GG::Pt         m_scroll_position;
    CUIButton*     m_zoom_in_button;
    CUIButton*     m_zoom_out_button;
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

    TechPanel(const Tech* tech, bool selected, std::set<std::string> categories_shown, std::set<TechType> types_shown, std::set<TechStatus> statuses_shown, double scale = 1.0);

    virtual bool InWindow(const GG::Pt& pt) const;
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, Uint32 keys);
    virtual void LDoubleClick(const GG::Pt& pt, Uint32 keys);
    virtual void MouseHere(const GG::Pt& pt, Uint32 keys);
    virtual void MouseWheel(const GG::Pt& pt, int move, Uint32 keys) {ZoomedSignal(move);}
    mutable boost::signal<void (int)> ZoomedSignal;

    void Select(bool select);

    mutable boost::signal<void (const Tech*)> TechBrowsedSignal;
    mutable boost::signal<void (const Tech*)> TechClickedSignal;
    mutable boost::signal<void (const Tech*)> TechDoubleClickedSignal;

private:
    GG::Rect ProgressPanelRect(const GG::Pt& ul, const GG::Pt& lr);

    const Tech*           m_tech;
    double                m_scale;
    double                m_progress; // in [0.0, 1.0]
    GG::Clr               m_fill_color;
    GG::Clr               m_text_and_border_color;
    GG::StaticGraphic*    m_icon;
    GG::TextControl*      m_tech_name_text;
    GG::TextControl*      m_tech_cost_text;
    GG::TextControl*      m_progress_text;
    bool                  m_selected;
};

TechTreeWnd::LayoutPanel::TechPanel::TechPanel(const Tech* tech, bool selected,
                                               std::set<std::string> categories_shown, std::set<TechType> types_shown,
                                               std::set<TechStatus> statuses_shown, double scale/* = 1.0*/) :
    GG::Wnd(0, 0, 1, 1, GG::CLICKABLE),
    m_tech(tech),
    m_scale(scale),
    m_progress(0.0),
    m_tech_name_text(0),
    m_tech_cost_text(0),
    m_progress_text(0),
    m_selected(selected)
{
    const int FONT_PTS = std::max(static_cast<const int>(ClientUI::Pts() * m_scale + 0.5), 3);

    GG::Pt UPPER_TECH_TEXT_OFFSET(4, 2);
    GG::Pt LOWER_TECH_TEXT_OFFSET(4, 0);
    GG::Pt size;
    if (m_tech->Type() == TT_THEORY) {
        size = GG::Pt(static_cast<int>(THEORY_TECH_PANEL_LAYOUT_WIDTH * m_scale + 0.5), static_cast<int>(THEORY_TECH_PANEL_LAYOUT_HEIGHT * m_scale + 0.5));
    } else if (m_tech->Type() == TT_APPLICATION) {
        size = GG::Pt(static_cast<int>(APPLICATION_TECH_PANEL_LAYOUT_WIDTH * m_scale + 0.5), static_cast<int>(APPLICATION_TECH_PANEL_LAYOUT_HEIGHT * m_scale + 0.5));
    } else { // m_tech->Type() == TT_REFINEMENT
        size = GG::Pt(static_cast<int>(REFINEMENT_TECH_PANEL_LAYOUT_WIDTH * m_scale + 0.5), static_cast<int>(REFINEMENT_TECH_PANEL_LAYOUT_HEIGHT * m_scale + 0.5));
    }
    Resize(size);

    UPPER_TECH_TEXT_OFFSET = GG::Pt(static_cast<int>(UPPER_TECH_TEXT_OFFSET.x * m_scale), static_cast<int>(UPPER_TECH_TEXT_OFFSET.y * m_scale));
    LOWER_TECH_TEXT_OFFSET = GG::Pt(static_cast<int>(LOWER_TECH_TEXT_OFFSET.x * m_scale), static_cast<int>(LOWER_TECH_TEXT_OFFSET.y * m_scale));

    using boost::io::str;
    using boost::format;
    bool known_tech = false;
    bool queued_tech = false;
    bool researchable_tech = false;
#ifndef FREEORION_BUILD_UTIL
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire->TechResearched(m_tech->Name())) {
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
        m_fill_color = ClientUI::KnownTechFillColor();
        m_text_and_border_color = ClientUI::KnownTechTextAndBorderColor();
    } else if (researchable_tech) {
        m_fill_color = ClientUI::ResearchableTechFillColor();
        m_text_and_border_color = ClientUI::ResearchableTechTextAndBorderColor();
    } else {
        m_fill_color = ClientUI::UnresearchableTechFillColor();
        m_text_and_border_color = ClientUI::UnresearchableTechTextAndBorderColor();
    }

    int graphic_size = size.y - static_cast<int>(PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale) - 2;
    m_icon = new GG::StaticGraphic(1, 1, graphic_size, graphic_size, ClientUI::TechTexture(m_tech->Name()), GG::GR_FITGRAPHIC);
    m_icon->SetColor(ClientUI::CategoryColor(m_tech->Category()));

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), FONT_PTS);

    int text_left = m_icon->LowerRight().x + static_cast<int>(4 * m_scale);
    m_tech_name_text = new GG::TextControl(text_left, UPPER_TECH_TEXT_OFFSET.y,
                                           Width() - m_icon->LowerRight().x - static_cast<int>(PROGRESS_PANEL_LEFT_EXTRUSION * m_scale), font->Lineskip(),
                                           UserString(m_tech->Name()), font, m_text_and_border_color, GG::TF_TOP | GG::TF_LEFT);
    m_tech_name_text->ClipText(true);
    AttachChild(m_icon);
    AttachChild(m_tech_name_text);

    std::string cost_str;
    if (!known_tech)
        cost_str = str(format(UserString("TECH_TOTAL_COST_STR")) % static_cast<int>(m_tech->ResearchCost() + 0.5) % m_tech->ResearchTurns());
    m_tech_cost_text = new GG::TextControl(text_left, 0,
                                           Width() - text_left, Height() - LOWER_TECH_TEXT_OFFSET.y - static_cast<int>(PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale),
                                           cost_str, font, m_text_and_border_color, GG::TF_BOTTOM | GG::TF_LEFT);
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
                                          progress_str, font, m_text_and_border_color);
    AttachChild(m_progress_text);

    EnableChildClipping(true);

    Select(m_selected);
}

bool TechTreeWnd::LayoutPanel::TechPanel::InWindow(const GG::Pt& pt) const
{
    GG::Pt lr = LowerRight();
    return GG::Wnd::InWindow(pt) &&
        (pt.x <= lr.x - PROGRESS_PANEL_LEFT_EXTRUSION * m_scale || lr.y - PROGRESS_PANEL_HEIGHT * m_scale <= pt.y) &&
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
    glColor(border_color_to_use);
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
    //m_toggle_button->SetSelected(m_selected);
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
const double TechTreeWnd::LayoutPanel::ZOOM_STEP_SIZE = 1.25;
TechTreeWnd::LayoutPanel::LayoutPanel(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::CLICKABLE),
    m_scale(1.0),
    m_categories_shown(),
    m_tech_types_shown(),
    m_tech_statuses_shown(),
    m_selected_tech(0),
    m_layout_surface(0),
    m_vscroll(0),
    m_hscroll(0),
    m_zoom_in_button(0),
    m_zoom_out_button(0)
{
    EnableChildClipping(true);

    m_scale = 1.0 / ZOOM_STEP_SIZE; // because fully zoomed in is too close

    m_layout_surface = new LayoutSurface();
    m_vscroll = new CUIScroll(w - ClientUI::ScrollWidth(), 0, ClientUI::ScrollWidth(), h - ClientUI::ScrollWidth(), GG::VERTICAL);
    m_hscroll = new CUIScroll(0, h - ClientUI::ScrollWidth(), w - ClientUI::ScrollWidth(), ClientUI::ScrollWidth(), GG::HORIZONTAL);

    const unsigned int ZBSIZE = ClientUI::ScrollWidth() * 2;
    const unsigned int ZBOFFSET = ClientUI::ScrollWidth() / 2;
    const unsigned int TOP = UpperLeft().y;
    const unsigned int LEFT = UpperLeft().x;

    boost::shared_ptr<GG::Font> font = GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());

    m_zoom_in_button = new CUIButton(w - ZBSIZE - ZBOFFSET - ClientUI::ScrollWidth(), ZBOFFSET, ZBSIZE, "+", font, ClientUI::ButtonColor(), ClientUI::CtrlBorderColor(), 1, ClientUI::TextColor(), GG::CLICKABLE | GG::ONTOP);
        
    m_zoom_out_button = new CUIButton(m_zoom_in_button->UpperLeft().x - LEFT, 
                                      m_zoom_in_button->LowerRight().y + ZBOFFSET - TOP, ZBSIZE, "-", font, ClientUI::ButtonColor(), ClientUI::CtrlBorderColor(), 1, ClientUI::TextColor(), GG::CLICKABLE | GG::ONTOP);

    AttachChild(m_layout_surface);
    AttachChild(m_vscroll);
    AttachChild(m_hscroll);
    AttachChild(m_zoom_in_button);
    AttachChild(m_zoom_out_button);

    GG::Connect(m_layout_surface->DraggedSignal, &TechTreeWnd::LayoutPanel::TreeDraggedSlot, this);
    GG::Connect(m_layout_surface->ZoomedSignal, &TechTreeWnd::LayoutPanel::TreeZoomedSlot, this);
    GG::Connect(m_vscroll->ScrolledSignal, &TechTreeWnd::LayoutPanel::ScrolledSlot, this);
    GG::Connect(m_hscroll->ScrolledSignal, &TechTreeWnd::LayoutPanel::ScrolledSlot, this);
    GG::Connect(m_zoom_in_button->ClickedSignal, &TechTreeWnd::LayoutPanel::TreeZoomInClicked, this);
    GG::Connect(m_zoom_out_button->ClickedSignal, &TechTreeWnd::LayoutPanel::TreeZoomOutClicked, this);


    // show all categories...
    m_categories_shown.clear();
    const std::vector<std::string> categories = GetTechManager().CategoryNames();
    for (std::vector<std::string>::const_iterator it = categories.begin(); it != categories.end(); ++it)
        m_categories_shown.insert(*it);

    // show all statuses
    m_tech_statuses_shown.clear();
    m_tech_statuses_shown.insert(TS_UNRESEARCHABLE);
    m_tech_statuses_shown.insert(TS_RESEARCHABLE);
    m_tech_statuses_shown.insert(TS_COMPLETE);

    // show all types
    m_tech_types_shown.clear();
    m_tech_types_shown.insert(TT_THEORY);
    m_tech_types_shown.insert(TT_APPLICATION);
    m_tech_types_shown.insert(TT_REFINEMENT);
}

GG::Pt TechTreeWnd::LayoutPanel::ClientLowerRight() const
{
    return LowerRight() - GG::Pt(ClientUI::ScrollWidth(), ClientUI::ScrollWidth());
}

std::set<std::string> TechTreeWnd::LayoutPanel::GetCategoriesShown() const
{
    return m_categories_shown;
}

double TechTreeWnd::LayoutPanel::Scale() const
{
    return m_scale;
}

std::set<TechType> TechTreeWnd::LayoutPanel::GetTechTypesShown() const
{
    return m_tech_types_shown;
}

std::set<TechStatus> TechTreeWnd::LayoutPanel::GetTechStatusesShown() const
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
    GG::Clr known_half_alpha = ClientUI::KnownTechTextAndBorderColor();
    known_half_alpha.a = 127;
    GG::Clr researchable_half_alpha = ClientUI::ResearchableTechTextAndBorderColor();
    researchable_half_alpha.a = 127;
    GG::Clr unresearchable_half_alpha = ClientUI::UnresearchableTechTextAndBorderColor();
    unresearchable_half_alpha.a = 127;
    std::map<TechStatus, std::vector<DependencyArcsMap::const_iterator> > selected_arcs;
    for (DependencyArcsMapsByArcType::const_iterator it = m_dependency_arcs.begin(); it != m_dependency_arcs.end(); ++it) {
        GG::Clr arc_color;
        switch (it->first) {
        case TS_COMPLETE:       arc_color = known_half_alpha; break;
        case TS_RESEARCHABLE:   arc_color = researchable_half_alpha; break;
        default:                arc_color = unresearchable_half_alpha; break;
        }
        glColor(arc_color);
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
        case TS_COMPLETE:       arc_color = known_half_alpha_selected; break;
        case TS_RESEARCHABLE:   arc_color = researchable_half_alpha_selected; break;
        default:                arc_color = unresearchable_half_alpha_selected; break;
        }
        glColor(arc_color);
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
        case TS_COMPLETE:       arc_color = ClientUI::KnownTechTextAndBorderColor(); break;
        case TS_RESEARCHABLE:   arc_color = ClientUI::ResearchableTechTextAndBorderColor(); break;
        default:                arc_color = ClientUI::UnresearchableTechTextAndBorderColor(); break;
        }
        glColor(arc_color);
        for (DependencyArcsMap::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            bool selected_arc = it2->first == m_selected_tech || it2->second.first == m_selected_tech;
            if (selected_arc) {
                selected_arcs[it->first].push_back(it2);
                continue;
            }
            DrawArc(it2, arc_color, true);
        }
    }
    GG::Clr known_selected = GG::LightColor(ClientUI::KnownTechTextAndBorderColor());
    GG::Clr researchable_selected = GG::LightColor(ClientUI::ResearchableTechTextAndBorderColor());
    GG::Clr unresearchable_selected = GG::LightColor(ClientUI::UnresearchableTechTextAndBorderColor());
    for (std::map<TechStatus, std::vector<DependencyArcsMap::const_iterator> >::const_iterator it = selected_arcs.begin();
         it != selected_arcs.end();
         ++it) {
        GG::Clr arc_color;
        switch (it->first) {
        case TS_COMPLETE:       arc_color = known_selected; break;
        case TS_RESEARCHABLE:   arc_color = researchable_selected; break;
        default:                arc_color = unresearchable_selected; break;
        }
        glColor(arc_color);
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
    if (m_categories_shown.find(category) == m_categories_shown.end()) {
        m_categories_shown.insert(category);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::HideCategory(const std::string& category)
{
    std::set<std::string>::iterator it = m_categories_shown.find(category);
    if (it != m_categories_shown.end()) {
        m_categories_shown.erase(it);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::ShowType(TechType type) {
    if (m_tech_types_shown.find(type) == m_tech_types_shown.end()) {
        m_tech_types_shown.insert(type);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::HideType(TechType type) {
    std::set<TechType>::iterator it = m_tech_types_shown.find(type);
    if (it != m_tech_types_shown.end()) {
        m_tech_types_shown.erase(it);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::ShowStatus(TechStatus status) {
    if (m_tech_statuses_shown.find(status) == m_tech_statuses_shown.end()) {
        m_tech_statuses_shown.insert(status);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::HideStatus(TechStatus status) {
    std::set<TechStatus>::iterator it = m_tech_statuses_shown.find(status);
    if (it != m_tech_statuses_shown.end()) {
        m_tech_statuses_shown.erase(it);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::ShowTech(const Tech* tech)
{
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

void TechTreeWnd::LayoutPanel::Layout(bool keep_position, double old_scale/* = -1.0*/)
{
    const int TECH_PANEL_MARGIN_X = ClientUI::Pts()*16;
    const int TECH_PANEL_MARGIN_Y = ClientUI::Pts()*16 + 100;

    if (old_scale < 0.0)
        old_scale = m_scale;
    GG::Pt final_position;
    if (keep_position) {
        if (m_scale == old_scale) {
            final_position = m_scroll_position;
        } else {
            GG::Pt cl_sz = ClientSize();
            GG::Pt center = m_scroll_position + GG::Pt(cl_sz.x / 2, cl_sz.y / 2);
            center.x = static_cast<int>((center.x - TECH_PANEL_MARGIN_X) * m_scale / old_scale + 0.5 + TECH_PANEL_MARGIN_X);
            center.y = static_cast<int>((center.y - TECH_PANEL_MARGIN_Y) * m_scale / old_scale + 0.5 + TECH_PANEL_MARGIN_Y);
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
    m_dependency_arcs.clear();
    //const std::set<const Tech*>& collapsed_subtree_techs = m_collapsed_subtree_techs_per_view[m_category_shown];
    for (Agnode_t* node = agfstnode(graph); node; node = agnxtnode(graph, node)) {
        const Tech* tech = GetTech(node->name);
        assert(tech);
        m_techs[tech] = new TechPanel(tech, tech == m_selected_tech, m_categories_shown, m_tech_types_shown, m_tech_statuses_shown, m_scale);
        m_techs[tech]->MoveTo(GG::Pt(static_cast<int>(PS2INCH(ND_coord_i(node).x) - m_techs[tech]->Width() / 2 + TECH_PANEL_MARGIN_X),
                                     static_cast<int>(PS2INCH(ND_coord_i(node).y) - (m_techs[tech]->Height() - PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale) / 2 + TECH_PANEL_MARGIN_Y)));
        m_layout_surface->AttachChild(m_techs[tech]);
        GG::Connect(m_techs[tech]->TechBrowsedSignal, &TechTreeWnd::LayoutPanel::TechBrowsedSlot, this);
        GG::Connect(m_techs[tech]->TechClickedSignal, &TechTreeWnd::LayoutPanel::TechClickedSlot, this);
        GG::Connect(m_techs[tech]->TechDoubleClickedSignal, &TechTreeWnd::LayoutPanel::TechDoubleClickedSlot, this);
        GG::Connect(m_techs[tech]->ZoomedSignal, &TechTreeWnd::LayoutPanel::TreeZoomedSlot, this);

        for (Agedge_t* edge = agfstout(graph, node); edge; edge = agnxtout(graph, edge)) {
            const Tech* from = tech;
            const Tech* to = GetTech(edge->head->name);
            assert(from && to);
            std::vector<std::vector<std::pair<double, double> > > points;
            for (int i = 0; i < ED_spl(edge)->size; ++i) {
                std::vector<std::pair<int, int> > temp;
                for (int j = 0; j < ED_spl(edge)->list[i].size; ++j) {
                    temp.push_back(std::make_pair(static_cast<int>(PS2INCH(ED_spl(edge)->list[i].list[j].x) + TECH_PANEL_MARGIN_X),
                                                  static_cast<int>(PS2INCH(ED_spl(edge)->list[i].list[j].y) + TECH_PANEL_MARGIN_Y)));
                }
                points.push_back(Spline(temp));
            }
#ifndef FREEORION_BUILD_UTIL
            const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
            
            TechStatus arc_type = empire->GetTechStatus(to->Name());
#else
            // these values are arbitrary; they're only useful for displaying techs in the tech-view utility app
            if (to->Type() == TT_THEORY) {
                arc_type = TS_COMPLETE;
            } else if (to->Type() == TT_APPLICATION) {
                arc_type = TS_RESEARCHABLE;
            }
#endif
            m_dependency_arcs[arc_type].insert(std::make_pair(from, std::make_pair(to, points)));
        }
    }

    GG::Pt client_sz = ClientSize();
    GG::Pt layout_size(std::max(client_sz.x,
                                static_cast<int>(PS2INCH(GD_bb(graph).UR.x - GD_bb(graph).LL.x) +
                                                 2 * TECH_PANEL_MARGIN_X + PROGRESS_PANEL_LEFT_EXTRUSION * m_scale)),
                       std::max(client_sz.y,
                                static_cast<int>(PS2INCH(GD_bb(graph).UR.y - GD_bb(graph).LL.y) +
                                                 2 * TECH_PANEL_MARGIN_Y + PROGRESS_PANEL_BOTTOM_EXTRUSION * m_scale)));
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
    if (m_tech_types_shown.find(tech->Type()) == m_tech_types_shown.end())
        return false;

    if (m_categories_shown.find(tech->Category()) == m_categories_shown.end())
        return false;

    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (m_tech_statuses_shown.find(empire->GetTechStatus(tech->Name())) == m_tech_statuses_shown.end())
        return false;

    return true;
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
    if (0 < move)
        SetScale(m_scale * ZOOM_STEP_SIZE);
    else if (move < 0)
        SetScale(m_scale / ZOOM_STEP_SIZE);
}

void TechTreeWnd::LayoutPanel::TreeZoomInClicked()
{
    TreeZoomedSlot(1);
}

void TechTreeWnd::LayoutPanel::TreeZoomOutClicked()
{
    TreeZoomedSlot(-1);
}

//////////////////////////////////////////////////
// TechTreeWnd                                  //
//////////////////////////////////////////////////
TechTreeWnd::TechTreeWnd(int w, int h) :
    GG::Wnd(0, 0, w, h, GG::CLICKABLE),
    m_tech_detail_panel(0),
    m_tech_navigator(0),
    m_layout_panel(0)
{
    TempUISoundDisabler sound_disabler;

    const int NAVIGATOR_WIDTH = 214;

    m_layout_panel = new LayoutPanel(w, h);
    GG::Connect(m_layout_panel->TechBrowsedSignal, &TechTreeWnd::TechBrowsedSlot, this);
    GG::Connect(m_layout_panel->TechClickedSignal, &TechTreeWnd::TechClickedSlot, this);
    GG::Connect(m_layout_panel->TechDoubleClickedSignal, &TechTreeWnd::TechDoubleClickedSlot, this);
    AttachChild(m_layout_panel);

    m_tech_detail_panel = new TechDetailPanel(m_layout_panel->ClientWidth() - NAVIGATOR_WIDTH, NAVIGATOR_AND_DETAIL_HEIGHT);
    m_layout_panel->AttachChild(m_tech_detail_panel);

    m_tech_navigator = new TechNavigator(NAVIGATOR_WIDTH, NAVIGATOR_AND_DETAIL_HEIGHT);
    m_tech_navigator->MoveTo(GG::Pt(m_tech_detail_panel->Width(), 1));
    GG::Connect(m_tech_navigator->TechClickedSignal, &TechTreeWnd::CenterOnTech, this);
    m_layout_panel->AttachChild(m_tech_navigator);

    m_tech_tree_controls = new TechTreeControls(1, NAVIGATOR_AND_DETAIL_HEIGHT, m_layout_panel->Width() - ClientUI::ScrollWidth()*4);
    const std::vector<std::string>& tech_categories = GetTechManager().CategoryNames();
    // connect category button clicks to update display
    for (unsigned int i = 0; i < m_tech_tree_controls->m_category_buttons.size() - 1; ++i)
        GG::Connect(m_tech_tree_controls->m_category_buttons[i]->ClickedSignal, ToggleCategoryFunctor(this, tech_categories[i]));
    GG::Connect(m_tech_tree_controls->m_category_buttons.back()->ClickedSignal, ToggleAllCategoriesFunctor(this));  // last button should be "All" button
    // connect status and type button clicks to update display
    for (std::map<TechStatus, CUIButton*>::iterator it = m_tech_tree_controls->m_tech_status_buttons.begin();
                                                    it != m_tech_tree_controls->m_tech_status_buttons.end(); ++it)
        GG::Connect(it->second->ClickedSignal, ToggleTechStatusFunctor(this, it->first));
    for (std::map<TechType, CUIButton*>::iterator it = m_tech_tree_controls->m_tech_type_buttons.begin();
                                                  it != m_tech_tree_controls->m_tech_type_buttons.end(); ++it)
        GG::Connect(it->second->ClickedSignal, ToggleTechTypeFunctor(this, it->first));
    m_layout_panel->AttachChild(m_tech_tree_controls);
}

double TechTreeWnd::Scale() const
{
    return m_layout_panel->Scale();
}

std::set<std::string> TechTreeWnd::GetCategoriesShown() const
{
    return m_layout_panel->GetCategoriesShown();
}

std::set<TechType> TechTreeWnd::GetTechTypesShown() const
{
    return m_layout_panel->GetTechTypesShown();
}

std::set<TechStatus> TechTreeWnd::GetTechStatusesShown() const
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
    
    // colour category button with its category colour
    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    int i = 0;
    for (std::vector<std::string>::const_iterator cats_it = cats.begin(); cats_it != cats.end(); ++cats_it, ++i) {
        if (*cats_it == category) {
            CUIButton* button = m_tech_tree_controls->m_category_buttons[i];
            button->MarkSelectedTechCategoryColor(cats[i]);
            break;
        }
    }
}

void TechTreeWnd::ShowAllCategories()
{
    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    int i = 0;
    for (std::vector<std::string>::const_iterator cats_it = cats.begin(); cats_it != cats.end(); ++cats_it, ++i) {
        m_layout_panel->ShowCategory(*cats_it);
        CUIButton* button = m_tech_tree_controls->m_category_buttons[i];
        button->MarkSelectedTechCategoryColor(cats[i]);
    }
}

void TechTreeWnd::HideCategory(const std::string& category)
{
    m_layout_panel->HideCategory(category);

    // colour category button to default colour, to indicate non-selection
    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    int i = 0;
    for (std::vector<std::string>::const_iterator cats_it = cats.begin(); cats_it != cats.end(); ++cats_it, ++i) {
        if (*cats_it == category) {
            m_tech_tree_controls->m_category_buttons[i]->MarkNotSelected();
            break;
        }
    }
}

void TechTreeWnd::HideAllCategories()
{
    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    int i = 0;
    for (std::vector<std::string>::const_iterator cats_it = cats.begin(); cats_it != cats.end(); ++cats_it, ++i) {
        m_layout_panel->HideCategory(*cats_it);
        m_tech_tree_controls->m_category_buttons[i]->MarkNotSelected();
    }
}

void TechTreeWnd::ToggleAllCategories()
{
    std::set<std::string> shown_cats = m_layout_panel->GetCategoriesShown();
    const std::vector<std::string> all_cats = GetTechManager().CategoryNames();

    if (shown_cats.size() == all_cats.size())
        HideAllCategories();
    else
        ShowAllCategories();
}

void TechTreeWnd::ToggleCategory(const std::string& category)
{
    std::set<std::string> shown_cats = m_layout_panel->GetCategoriesShown();

    std::set<std::string>::const_iterator it = shown_cats.find(category);
    if (it == shown_cats.end())
        ShowCategory(category);
    else
        HideCategory(category);
}

void TechTreeWnd::ShowStatus(TechStatus status)
{
    m_layout_panel->ShowStatus(status);
    
    CUIButton* button = m_tech_tree_controls->m_tech_status_buttons[status];
    button->MarkSelectedGray();
}

void TechTreeWnd::HideStatus(TechStatus status)
{
    m_layout_panel->HideStatus(status);

    CUIButton* button = m_tech_tree_controls->m_tech_status_buttons[status];
    button->MarkNotSelected();
}

void TechTreeWnd::ToggleStatus(TechStatus status)
{
    std::set<TechStatus> statuses = m_layout_panel->GetTechStatusesShown();

    std::set<TechStatus>::const_iterator it = statuses.find(status);
    if (it == statuses.end())
        ShowStatus(status);
    else
        HideStatus(status);
}

void TechTreeWnd::ShowType(TechType type)
{
    m_layout_panel->ShowType(type);

    CUIButton* button = m_tech_tree_controls->m_tech_type_buttons[type];
    button->MarkSelectedGray();
}

void TechTreeWnd::HideType(TechType type)
{
    m_layout_panel->HideType(type);

    CUIButton* button = m_tech_tree_controls->m_tech_type_buttons[type];
    button->MarkNotSelected();
}

void TechTreeWnd::ToggleType(TechType type)
{
    std::set<TechType> types = m_layout_panel->GetTechTypesShown();

    std::set<TechType>::const_iterator it = types.find(type);
    if (it == types.end()) {
        ShowType(type);
    } else {
        HideType(type);
    }
}

void TechTreeWnd::SetScale(double scale)
{
    m_layout_panel->SetScale(scale);
}

void TechTreeWnd::CenterOnTech(const Tech* tech)
{
    // ensure tech is visible
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());

    ShowCategory(tech->Category());
    ShowStatus(empire->GetTechStatus(tech->Name()));
    ShowType(tech->Type());

    // centre on it
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
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    std::string name = tech->Name();
    const TechStatus tech_status = empire->GetTechStatus(name);

    // if tech can be researched already, just add it
    if (tech_status == TS_RESEARCHABLE) {
        AddTechToQueueSignal(tech);
        return;
    }

    if (tech_status != TS_UNRESEARCHABLE) return;

    // if tech can't yet be researched, add any prerequisites it requires (recursively) and then add it

    TechManager& manager = GetTechManager();
    // compile set of recursive prereqs
    std::list<std::string> prereqs_list;                    // working list of prereqs as being processed.  may contain duplicates
    std::set<std::string> prereqs_set;                      // set of (unique) prereqs leading to tech
    std::multimap<double, const Tech*> techs_to_add_map;    // indexed and sorted by cost per turn

    // initialize working list with 1st order prereqs
    std::set<std::string> cur_prereqs = tech->Prerequisites();
    std::copy(cur_prereqs.begin(), cur_prereqs.end(), std::back_inserter(prereqs_list));

    // traverse list, appending new prereqs to it, and putting unique prereqs into set
    for (std::list<std::string>::iterator it = prereqs_list.begin(); it != prereqs_list.end(); ++it) {
        std::string cur_name = *it;
        const Tech* cur_tech = manager.GetTech(cur_name);

        // check if this tech is already in the map of prereqs.  If so, it has already been processed, and can be skipped.
        if (prereqs_set.find(cur_name) != prereqs_set.end()) continue;

        // tech is new, so put it into the set of already-processed prereqs
        prereqs_set.insert(cur_name);
        // and the map of techs, sorted by cost
        techs_to_add_map.insert(std::pair<double, const Tech*>(cur_tech->ResearchCost(), cur_tech));

        // get prereqs of new tech, append to list
        cur_prereqs = cur_tech->Prerequisites();
        std::copy(cur_prereqs.begin(), cur_prereqs.end(), std::back_inserter(prereqs_list));
    }

    // extract sorted techs into vector, to be passed to signal...
    std::vector<const Tech*> tech_vec;
    for (std::multimap<double, const Tech*>::const_iterator it = techs_to_add_map.begin(); it != techs_to_add_map.end(); ++it)
        tech_vec.push_back(it->second);
    // put original tech to be enqueued into vector last
    tech_vec.push_back(tech);
    AddMultipleTechsToQueueSignal(tech_vec);

    /* Alternative automatic tech ordering code.  Written first and seemed to work, but had undesirable
       behaviour in practice.  Leaving here, commented out, incase wanted later...
       
    std::map<std::string, TechStatus> prereq_status_map;    // set of (unique) prereqs leading to tech, with TechStatus of each

    // traverse list, appending new prereqs to it, and putting unique prereqs into set
    for (std::list<std::string>::iterator it = prereqs_list.begin(); it != prereqs_list.end(); ++it) {
        std::string cur_name = *it;
        Logger().errorStream() << "traversing list: tech: " << cur_name;
        const Tech* cur_tech = manager.GetTech(cur_name);

        // check if this tech is already in the map of prereqs.  If so, it has already been processed, and can be skipped.
        if (prereq_status_map.find(cur_name) != prereq_status_map.end()) continue;

        // tech is new, so put it into the map of prereqs
        prereq_status_map[cur_name] = empire->GetTechStatus(cur_name);

        Logger().errorStream() << ".. tech is new. adding its prereqs to end of list";
        Logger().errorStream() << ".. and adding its status to prereq_status_map: " << prereq_status_map[cur_name];

        // get prereqs of new tech, append to list
        const std::set<std::string> cur_prereqs = cur_tech->Prerequisites();
        std::copy(cur_prereqs.begin(), cur_prereqs.end(), std::back_inserter(prereqs_list));
    }

    // repeatedly traverse set, enqueuing that are researchable (because their prereqs are complete or they
    // are initially researchable), then marking enqueued techs as complete so the next iteration will
    // find the techs that they are prereqs of to now have completed prereqs and thus enequeue the next layer
    // of techs in the prereqs tree.  within each level, currently enqueues techs in order of cost.
    // in future, could instead determine optimal order of enqueuing to minimize total research time using
    // a more sophisticated scheduling algorithm.
    
    std::multimap<double, const Tech*> techs_to_add_map;  // indexed and sorted by turns to research
    while (true) {
        techs_to_add_map.clear();

        Logger().errorStream() << "determining techs to add";
        for (std::map<std::string, TechStatus>::iterator it = prereq_status_map.begin(); it != prereq_status_map.end(); ++it) {
            const std::string cur_name = it->first;
            const Tech* cur_tech = manager.GetTech(cur_name);

            Logger().errorStream() << ".. considering tech: " << cur_name;
            TechStatus status = it->second;

            // if tech is marked as complete, don't need to do anything with it
            if (status == TS_COMPLETE) {
                Logger().errorStream() << ".... tech is already complete.  skipping.";
                continue;
            } 
            if (status == TS_RESEARCHABLE) {
                Logger().errorStream() << ".... tech is researchable, adding to map of techs to enqueue";
                techs_to_add_map.insert(std::pair<double, const Tech*>(cur_tech->ResearchTurns(), cur_tech));
                continue;
            }

            Logger().errorStream() << ".... tech is not yet researchable.  checking its prereqs";
            // if tech's prereqs are all complete, tech is researchable (even if not marked TS_RESEARCHABLE initially)
            const std::set<std::string> cur_prereqs = cur_tech->Prerequisites();
            bool all_prereqs_complete = true;   // until proven otherwise by a tech being not TS_COMPLETE
            for (std::set<std::string>::const_iterator it = cur_prereqs.begin(); it != cur_prereqs.end(); ++it) {
                if (prereq_status_map[*it] != TS_COMPLETE) {
                    Logger().errorStream() << "...... prereq " << *it << " is not complete.  skipping.";
                    all_prereqs_complete = false;
                    break;
                }
            }
            
            // if all prereqs complete, tech is researchable
            if (all_prereqs_complete) {
                Logger().errorStream() << "...... all prereqs are complete.  adding tech to map of techs to enqueue";
                techs_to_add_map.insert(std::pair<double, const Tech*>(cur_tech->ResearchTurns(), cur_tech));
            }
        }

        Logger().errorStream() << "adding techs, and marking as TS_COMPLETE for next round...";

        // if no techs were researchable, must be done
        if (techs_to_add_map.empty()) {
            Logger().errorStream() << "... no techs to add!  DONE!";
            break;  // exit while loop
        }

        for (std::multimap<double, const Tech*>::const_iterator it = techs_to_add_map.begin(); it != techs_to_add_map.end(); ++it) {
            const Tech* cur_tech = it->second;
            const std::string cur_name = cur_tech->Name();
            tech_vec.push_back(cur_tech);
            prereq_status_map[cur_name] = TS_COMPLETE;
            Logger().errorStream() << ".. adding and marking tech: " << cur_name;;
        }
    }
    */
}
