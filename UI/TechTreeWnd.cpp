#include "TechTreeWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "CUIWnd.h"
#include "Sound.h"
#include "EncyclopediaDetailPanel.h"
#include "../client/human/HumanClientApp.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../universe/Tech.h"
#include "../universe/Effect.h"
#include "../Empire/Empire.h"
#include "TechTreeLayout.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#include <algorithm>

#include <boost/timer.hpp>

namespace {
    // command-line options
    void AddOptions(OptionsDB& db)
    {
        db.Add("UI.tech-layout-horz-spacing", "OPTIONS_DB_UI_TECH_LAYOUT_HORZ_SPACING", 0.5, RangedStepValidator<double>(0.25, 0.25, 10.0));
        db.Add("UI.tech-layout-vert-spacing", "OPTIONS_DB_UI_TECH_LAYOUT_VERT_SPACING", 1.0, RangedStepValidator<double>(0.25, 0.25, 10.0));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const GG::X PROGRESS_PANEL_WIDTH(94);
    const GG::Y PROGRESS_PANEL_HEIGHT(18);
    const GG::X PROGRESS_PANEL_LEFT_EXTRUSION(21);
    const GG::Y PROGRESS_PANEL_BOTTOM_EXTRUSION(9);
    const int MAIN_PANEL_CORNER_RADIUS = 5;
    const int PROGRESS_PANEL_CORNER_RADIUS = 5;

    const GG::X THEORY_TECH_PANEL_LAYOUT_WIDTH = 250 + PROGRESS_PANEL_LEFT_EXTRUSION;
    const GG::Y THEORY_TECH_PANEL_LAYOUT_HEIGHT = 50 + PROGRESS_PANEL_BOTTOM_EXTRUSION;
    const GG::X APPLICATION_TECH_PANEL_LAYOUT_WIDTH(250);
    const GG::Y APPLICATION_TECH_PANEL_LAYOUT_HEIGHT(44);
    const GG::X REFINEMENT_TECH_PANEL_LAYOUT_WIDTH(250);
    const GG::Y REFINEMENT_TECH_PANEL_LAYOUT_HEIGHT(44);

    const GG::X TECH_PANEL_LAYOUT_WIDTH = THEORY_TECH_PANEL_LAYOUT_WIDTH;
    const GG::Y TECH_PANEL_LAYOUT_HEIGHT = THEORY_TECH_PANEL_LAYOUT_HEIGHT - PROGRESS_PANEL_BOTTOM_EXTRUSION;

    const int    HORIZONTAL_LINE_LENGTH = 12;
    const float OUTER_LINE_THICKNESS = 1.5;
    const float ARC_THICKNESS = 3.0;

    const double TECH_NAVIGATOR_ROLLOVER_BRIGHTENING_FACTOR = 1.5;

    const double MIN_SCALE = 0.1073741824;  // = 1.0/(1.25)^10
    const double MAX_SCALE = 1.0;

    struct pointf { double x, y; };

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
            const int SUBDIVISIONS = 6;
            for (int step = 1; step <= SUBDIVISIONS; ++step) {
                pointf pt = Bezier(patch, static_cast<double>(step) / SUBDIVISIONS);
                retval.push_back(std::make_pair(pt.x, pt.y));
            }
        }
        return retval;
    }

    void RenderTechPanel(TechType tech_type, const GG::Rect& main_panel, const GG::Rect& progress_panel,
                         GG::Clr interior_color, GG::Clr border_color, bool show_progress, double progress)
    {
        glDisable(GL_TEXTURE_2D);

        // main panel background
        glColor(interior_color);
        if (tech_type == TT_THEORY) {
            PartlyRoundedRect(main_panel.ul,    main_panel.lr,  MAIN_PANEL_CORNER_RADIUS,   true,   true,   true,   true,   true);
        } else if (tech_type == TT_APPLICATION) {
            PartlyRoundedRect(main_panel.ul,    main_panel.lr,  MAIN_PANEL_CORNER_RADIUS,   true,   true,   false,  false,  true);
        } else { // tech_type == TT_REFINEMENT
            PartlyRoundedRect(main_panel.ul,    main_panel.lr,  MAIN_PANEL_CORNER_RADIUS,   false,  false,  false,  false,  true);
        }

        // main panel border
        glColor(border_color);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(OUTER_LINE_THICKNESS);
        if (tech_type == TT_THEORY) {
            PartlyRoundedRect(main_panel.ul,    main_panel.lr,  MAIN_PANEL_CORNER_RADIUS,   true,   true,   true,   true,   false);
        } else if (tech_type == TT_APPLICATION) {
            PartlyRoundedRect(main_panel.ul,    main_panel.lr,  MAIN_PANEL_CORNER_RADIUS,   true,   true,   false,  false,  false);
        } else { // tech_type == TT_REFINEMENT
            PartlyRoundedRect(main_panel.ul,    main_panel.lr,  MAIN_PANEL_CORNER_RADIUS,   false,  false,  false,  false,  false);
        }
        glLineWidth(1.0);
        glDisable(GL_LINE_SMOOTH);

        if (show_progress) {
            // progress panel background
            glColor(ClientUI::TechWndProgressBarBackgroundColor());
            PartlyRoundedRect(progress_panel.ul,    progress_panel.lr,  PROGRESS_PANEL_CORNER_RADIUS,   true,   true,   true,   true,   true);

            GG::X progress_extent((0.0 < progress && progress < 1.0) ? (progress_panel.ul.x + progress * progress_panel.Width() + 0.5) : GG::X_d(0));
            if (progress_extent) {
                // progress bar
                glColor(ClientUI::TechWndProgressBarColor());
                GG::BeginScissorClipping(progress_panel.ul, GG::Pt(progress_extent, progress_panel.lr.y));
                PartlyRoundedRect(progress_panel.ul,    progress_panel.lr,  PROGRESS_PANEL_CORNER_RADIUS,   true,   true,   true,   true,   true);
                GG::EndScissorClipping();
            }

            // progress panel border
            glColor(border_color);
            glEnable(GL_LINE_SMOOTH);
            glLineWidth(OUTER_LINE_THICKNESS);
            PartlyRoundedRect(progress_panel.ul,    progress_panel.lr,  PROGRESS_PANEL_CORNER_RADIUS,   true,   true,   true,   true,   false);
            glLineWidth(1.0);
            glDisable(GL_LINE_SMOOTH);
        }

        glEnable(GL_TEXTURE_2D);
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
    TechTreeControls(GG::X x, GG::Y y, GG::X w);
    //@}

    //! \name Mutators //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    Render();
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    //@}

private:
    void            DoButtonLayout();

    /** These values are determined when doing button layout, and stored.  They are later
      * used when rendering separator lines between the groups of buttons */
    int m_buttons_per_row;                  // number of buttons that can fit into available horizontal space
    GG::X m_col_offset;                // horizontal distance between each column of buttons
    GG::Y m_row_offset;                // vertical distance between each row of buttons
    int m_category_button_rows;             // number of rows used for category buttons
    int m_status_or_type_button_rows;       // number of rows used for status buttons and for type buttons (both groups have the same number of buttons (three) so use the same number of rows)

    /** These values are used for rendering separator lines between groups of buttons */
    static const int BUTTON_SEPARATION; // vertical or horizontal sepration between adjacent buttons
    static const int UPPER_LEFT_PAD;    // offset of buttons' position from top left of controls box

    // TODO: replace all the above stored information with a vector of pairs of GG::Pt (or perhaps GG::Rect)
    // This will contain the start and end points of all separator lines that need to be drawn.  This will be
    // calculated by SizeMove, and stored, so that start and end positions don't need to be recalculated each
    // time Render is called.

    std::vector<CUIButton*>             m_category_buttons;
    std::map<TechType, CUIButton*>      m_tech_type_buttons;
    std::map<TechStatus, CUIButton*>    m_tech_status_buttons;
    CUIButton*                          m_list_view_button;
    CUIButton*                          m_tree_view_button;

    friend class TechTreeWnd;               // so TechTreeWnd can access buttons
};
const int TechTreeWnd::TechTreeControls::BUTTON_SEPARATION = 3;
const int TechTreeWnd::TechTreeControls::UPPER_LEFT_PAD = 2;

TechTreeWnd::TechTreeControls::TechTreeControls(GG::X x, GG::Y y, GG::X w) :
    CUIWnd(UserString("TECH_DISPLAY"), x, y, w, GG::Y(10), GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP)
{
    // create a button for each tech category...
    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    for (unsigned int i = 0; i < cats.size(); ++i) {
        m_category_buttons.push_back(new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString(cats[i])));
        AttachChild(m_category_buttons.back());
        m_category_buttons.back()->MarkNotSelected();
    }
    // and one for "ALL"
    m_category_buttons.push_back(new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("ALL")));
    AttachChild(m_category_buttons.back());
    m_category_buttons.back()->MarkNotSelected();

    // create a button for each tech type
    m_tech_type_buttons[TT_THEORY] = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("TECH_WND_TYPE_THEORIES"));
    AttachChild(m_tech_type_buttons[TT_THEORY]);
    m_tech_type_buttons[TT_APPLICATION] = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("TECH_WND_TYPE_APPLICATIONS"));
    AttachChild(m_tech_type_buttons[TT_APPLICATION]);
    m_tech_type_buttons[TT_REFINEMENT] = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("TECH_WND_TYPE_REFINEMENTS"));
    AttachChild(m_tech_type_buttons[TT_REFINEMENT]);
    // colour
    for (std::map<TechType, CUIButton*>::iterator it = m_tech_type_buttons.begin(); it != m_tech_type_buttons.end(); ++it)
        it->second->MarkNotSelected();

    // create a button for each tech status
    m_tech_status_buttons[TS_UNRESEARCHABLE] = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("TECH_WND_STATUS_UNRESEARCHABLE"));
    AttachChild(m_tech_status_buttons[TS_UNRESEARCHABLE]);
    m_tech_status_buttons[TS_RESEARCHABLE] = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("TECH_WND_STATUS_RESEARCHABLE"));
    AttachChild(m_tech_status_buttons[TS_RESEARCHABLE]);
    m_tech_status_buttons[TS_COMPLETE] = new CUIButton(GG::X0, GG::Y0, GG::X(20), UserString("TECH_WND_STATUS_COMPLETED"));
    AttachChild(m_tech_status_buttons[TS_COMPLETE]);
    // colour
    for (std::map<TechStatus, CUIButton*>::iterator it = m_tech_status_buttons.begin(); it != m_tech_status_buttons.end(); ++it)
        it->second->MarkNotSelected();

    // create buttons to switch between tree and list views
    m_list_view_button = new CUIButton(GG::X0, GG::Y0, GG::X(80), UserString("TECH_WND_LIST_VIEW"));
    m_list_view_button->MarkNotSelected();
    AttachChild(m_list_view_button);
    m_tree_view_button = new CUIButton(GG::X0, GG::Y(30), GG::X(80), UserString("TECH_WND_TREE_VIEW"));
    m_tree_view_button->MarkNotSelected();
    AttachChild(m_tree_view_button);

    SetChildClippingMode(ClipToClient);
    DoButtonLayout();
    Resize(GG::Pt(Width(), MinSize().y));
}

void TechTreeWnd::TechTreeControls::DoButtonLayout()
{
    const GG::X RIGHT_EDGE_PAD(8);
    const GG::X USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, GG::X1);   // space in which to do layout
    const int PTS = ClientUI::Pts();
    const GG::X PTS_WIDE(PTS/2);  // how wide per character the font needs... not sure how better to get this
    const GG::X MIN_BUTTON_WIDTH = PTS_WIDE*18;    // rough guesstimate...
    const int MAX_BUTTONS_PER_ROW = std::max(Value(USABLE_WIDTH / (MIN_BUTTON_WIDTH + BUTTON_SEPARATION)), 1);

    const float NUM_CATEGORY_BUTTONS = static_cast<float>(m_category_buttons.size());
    const int ROWS = static_cast<int>(std::ceil(NUM_CATEGORY_BUTTONS / MAX_BUTTONS_PER_ROW));
    m_buttons_per_row = static_cast<int>(std::ceil(NUM_CATEGORY_BUTTONS / ROWS));   // number of buttons in a typical row

    const GG::X BUTTON_WIDTH = (USABLE_WIDTH - (m_buttons_per_row - 1)*BUTTON_SEPARATION) / m_buttons_per_row;
    const GG::Y BUTTON_HEIGHT = m_category_buttons.back()->Height();

    m_col_offset = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    m_row_offset = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons

    // place category buttons: fill each row completely before starting next row
    int row = 0, col = -1;
    for (std::vector<CUIButton*>::iterator it = m_category_buttons.begin(); it != m_category_buttons.end(); ++it) {
        ++col;
        if (col >= m_buttons_per_row) {
            ++row;
            col = 0;
        }
        GG::Pt ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        (*it)->SizeMove(ul, lr);
    }

    // rowbreak after category buttons, before type and status buttons
    col = -1;
    m_category_button_rows = ++row;

    // place type buttons: fill each row completely before starting next row
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
    if (m_buttons_per_row < static_cast<int>(m_tech_type_buttons.size() + m_tech_status_buttons.size())) {
        col = -1;
        ++row;
    }

    // place status buttons: fill each row completely before starting next row
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

    // rowbreak after status buttons, before view toggles
    col = 0;
    ++row;

    // place view type buttons buttons
    GG::Pt ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
    GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_list_view_button->SizeMove(ul, lr);
    ++col;
    if (col >= m_buttons_per_row) {
        ++row;
        col = 0;
    }
    ul = GG::Pt(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
    lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_tree_view_button->SizeMove(ul, lr);

    // keep track of layout.  Used to draw lines between groups of buttons when rendering
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

        GG::Y category_bottom = cl_ul.y + m_category_button_rows*m_row_offset - BUTTON_SEPARATION/2 + UPPER_LEFT_PAD;

        glVertex(cl_ul.x, category_bottom);
        glVertex(cl_lr.x - 1, category_bottom);

        if (m_buttons_per_row >= 6) {
            // all six status and type buttons are on one row, and need a vertical separator between them
            GG::X middle = cl_ul.x + m_col_offset*3 - BUTTON_SEPARATION/2 + UPPER_LEFT_PAD;
            glVertex(middle, category_bottom);
            glVertex(middle, cl_lr.y - 1);

        } else {
            // the status and type buttons are split into separate vertical groups, and need a horiztonal separator between them
            GG::Y status_bottom = category_bottom + m_status_or_type_button_rows*m_row_offset;
            glVertex(cl_ul.x, status_bottom);
            glVertex(cl_lr.x - 1, status_bottom);
        }
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void TechTreeWnd::TechTreeControls::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_drag_offset != GG::Pt(-GG::X1, -GG::Y1)) {  // resize-dragging
        GG::Pt new_lr = pt - m_drag_offset;

        new_lr.y = LowerRight().y;    // ignore y-resizes

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

            GG::Pt min_ul = parent->ClientUpperLeft() + GG::Pt(GG::X1, GG::Y1);
            GG::Pt max_lr = parent->ClientLowerRight();
            GG::Pt max_ul = max_lr - Size();

            new_ul.x = std::max(min_ul.x, std::min(max_ul.x, new_ul.x));
            new_ul.y = std::max(min_ul.y, std::min(max_ul.y, new_ul.y));

            final_move = new_ul - ul;
        }

        GG::Wnd::LDrag(pt, final_move, mod_keys);
    }
}


//////////////////////////////////////////////////
// TechTreeWnd::TechNavigator                   //
//////////////////////////////////////////////////
/** A window with a single lisbox in it.  The listbox represents the techs that
  * are required for and are unlocked by some tech.  Clicking on a prereq or
  * unlocked tech will bring up that tech. */
class TechTreeWnd::TechNavigator : public CUIWnd
{
public:
    TechNavigator(GG::X w, GG::Y h);

    const Tech* CurrentTech() const {return m_current_tech;}
    void SetTech(const Tech* tech) {m_current_tech = tech; Reset();}
    void TechClickedSlot(const Tech* tech) {TechClickedSignal(tech);}

    mutable boost::signal<void (const Tech*)> TechClickedSignal;

    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);

private:
    /** A control with a label \a str on it, and that is rendered partially
      * onto the next row.  The "Requires" and "Unlocks" rows are in of this
      * class. */
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
        virtual GG::Pt  ClientUpperLeft() const {return UpperLeft() + GG::Pt(GG::X(3), GG::Y(2));}
        virtual GG::Pt  ClientLowerRight() const {return LowerRight() - GG::Pt(GG::X(2), GG::Y(2));}
        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
        virtual void    Render();
        virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {ClickedSignal(m_tech);}
        virtual void    MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {m_selected = true;}
        virtual void    MouseLeave() {m_selected = false;}

        mutable boost::signal<void (const Tech*)> ClickedSignal;

    private:
        const Tech * const  m_tech;
        GG::Clr             m_border_color;
        GG::TextControl*    m_name_text;
        bool                m_selected;
    };

    static const GG::X  TECH_ROW_INDENTATION;
    static const GG::X  LB_MARGIN_X;
    static const GG::Y  LB_MARGIN_Y;

    GG::ListBox::Row*   NewSectionHeaderRow(const std::string& str);
    GG::ListBox::Row*   NewTechRow(const Tech* tech);
    void                Reset();

    void                DoLayout();

    const Tech*         m_current_tech;
    GG::ListBox*        m_lb;
};
const GG::X TechTreeWnd::TechNavigator::TECH_ROW_INDENTATION(8);
const GG::X TechTreeWnd::TechNavigator::LB_MARGIN_X(5);
const GG::Y TechTreeWnd::TechNavigator::LB_MARGIN_Y(5);

TechTreeWnd::TechNavigator::TechNavigator(GG::X w, GG::Y h) :
    CUIWnd(UserString("TECH_NAVIGATION"), GG::X0, GG::Y0, w, h, GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE),
    m_current_tech(0)
{
    m_lb = new CUIListBox(LB_MARGIN_X, LB_MARGIN_Y, GG::X(100), GG::Y(100));    // resized later when TechNavigator is SizeMoved
    m_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
    AttachChild(m_lb);
}

GG::ListBox::Row* TechTreeWnd::TechNavigator::NewSectionHeaderRow(const std::string& str)
{
    GG::ListBox::Row* retval = new GG::ListBox::Row(m_lb->Width(), GG::Y(3*ClientUI::Pts()/2 + 4), "");
    retval->push_back(new SectionHeaderControl(str));
    return retval;
}

GG::ListBox::Row* TechTreeWnd::TechNavigator::NewTechRow(const Tech* tech)
{
    TechControl* control = new TechControl(tech);
    GG::Connect(control->ClickedSignal, &TechTreeWnd::TechNavigator::TechClickedSlot, this);
    GG::ListBox::Row* retval = new GG::ListBox::Row(m_lb->Width(), GG::Y(3*ClientUI::Pts()/2 + 4), "");
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

    for (GG::ListBox::iterator it = m_lb->begin(); it != m_lb->end(); ++it) {
        GG::ListBox::Row& row = **it;
        GG::Pt size = GG::Pt(m_lb->Width() - 4*LB_MARGIN_X, row.Height());
        row.Resize(size);
        GG::Control* control = row.at(0);
        control->Resize(size);
    }
}

void TechTreeWnd::TechNavigator::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_drag_offset != GG::Pt(-GG::X1, -GG::Y1)) {  // resize-dragging
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

            GG::Pt min_ul = parent->ClientUpperLeft() + GG::Pt(GG::X1, GG::Y1);
            GG::Pt max_lr = parent->ClientLowerRight();
            GG::Pt max_ul = max_lr - this->Size();

            new_ul.x = std::max(min_ul.x, std::min(max_ul.x, new_ul.x));
            new_ul.y = std::max(min_ul.y, std::min(max_ul.y, new_ul.y));

            final_move = new_ul - ul;
        }

        GG::Wnd::LDrag(pt, final_move, mod_keys);
    }
}

TechTreeWnd::TechNavigator::SectionHeaderControl::SectionHeaderControl(const std::string& str) :
    GG::Control(GG::X0, GG::Y0, GG::X(10), GG::Y(3*ClientUI::Pts()/2 + 4))
{
    m_label = new GG::TextControl(GG::X0, GG::Y0, GG::X(10), GG::Y(3*ClientUI::Pts()/2 + 4),
                                  str, ClientUI::GetFont(),
                                  ClientUI::KnownTechTextAndBorderColor(), GG::FORMAT_LEFT);
    AttachChild(m_label);
}

void TechTreeWnd::TechNavigator::SectionHeaderControl::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight() + GG::Pt(GG::X0, GG::Y(11));
    glDisable(GL_TEXTURE_2D);
    const int CORNER_RADIUS = 5;

    glColor(ClientUI::KnownTechFillColor());
    PartlyRoundedRect(ul,   lr,   CORNER_RADIUS,    true,   true,   false,  false,  true);

    glLineWidth(OUTER_LINE_THICKNESS);
    glColor4ub(ClientUI::KnownTechTextAndBorderColor().r, ClientUI::KnownTechTextAndBorderColor().g, ClientUI::KnownTechTextAndBorderColor().b, 127);
    PartlyRoundedRect(ul,   lr,   CORNER_RADIUS,    true,   true,   false,  false,  false);

    glEnable(GL_TEXTURE_2D);
    glLineWidth(1.0);
}

void TechTreeWnd::TechNavigator::SectionHeaderControl::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Wnd::SizeMove(ul, lr);
    GG::Pt label_ul(TECH_ROW_INDENTATION, GG::Y0);
    GG::Pt label_lr(ClientWidth(), ClientHeight());
    m_label->SizeMove(label_ul, label_lr);
}

TechTreeWnd::TechNavigator::TechControl::TechControl(const Tech* tech) :
    GG::Control(GG::X0, GG::Y0, GG::X(10), GG::Y(3*ClientUI::Pts()/2 + 4)),
    m_tech(tech),
    m_selected(false)
{
    SetChildClippingMode(ClipToClient);
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire && m_tech && empire->TechResearched(m_tech->Name())) {
        SetColor(ClientUI::KnownTechFillColor());
        m_border_color = ClientUI::KnownTechTextAndBorderColor();
    } else if (empire && m_tech && empire->ResearchableTech(m_tech->Name())) {
        SetColor(ClientUI::ResearchableTechFillColor());
        m_border_color = ClientUI::ResearchableTechTextAndBorderColor();
    } else {
        SetColor(ClientUI::UnresearchableTechFillColor());
        m_border_color = ClientUI::UnresearchableTechTextAndBorderColor();
    }
    GG::Pt client_size = ClientSize();
    m_name_text = new GG::TextControl(GG::X0, GG::Y0, GG::X(10), GG::Y(3*ClientUI::Pts()/2 + 4),
                                      (m_tech ? UserString(m_tech->Name()) : UserString("ERROR")), ClientUI::GetFont(),
                                      m_border_color, GG::FORMAT_LEFT);
    AttachChild(m_name_text);
}

void TechTreeWnd::TechNavigator::TechControl::Render()
{
    GG::Rect rect(UpperLeft(), LowerRight());
    rect += GG::Pt(TECH_ROW_INDENTATION, GG::Y0);
    TechType tech_type = m_tech->Type();
    GG::Clr color_to_use = Color();
    GG::Clr border_color_to_use = m_border_color;
    if (!Disabled() && m_selected) {
        AdjustBrightness(color_to_use, TECH_NAVIGATOR_ROLLOVER_BRIGHTENING_FACTOR);
        AdjustBrightness(border_color_to_use, TECH_NAVIGATOR_ROLLOVER_BRIGHTENING_FACTOR);
    }
    RenderTechPanel(tech_type, rect, GG::Rect(), color_to_use, border_color_to_use, false, 0.0);
}

void TechTreeWnd::TechNavigator::TechControl::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Wnd::SizeMove(ul, lr);
    GG::Pt text_ul(TECH_ROW_INDENTATION, GG::Y0);
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
    typedef boost::signal<void (const Tech*)>      TechBrowsedSignalType;       ///< emitted when the mouse rolls over a technology
    typedef boost::signal<void (const Tech*)>      TechClickedSignalType;       ///< emitted when a technology is single-clicked
    typedef boost::signal<void (const Tech*)>      TechDoubleClickedSignalType; ///< emitted when a technology is double-clicked
    //@}

    /** \name Structors */ //@{
    LayoutPanel(GG::X w, GG::Y h);
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt          ClientLowerRight() const;

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

    void Update(const Tech* tech = 0);  ///< update indicated \a tech panel or all panels if \a tech is 0, without redoing layout
    void Clear();                       ///< remove all tech panels
    void Reset();                       ///< redo layout, recentre on a tech
    void SetScale(double scale);
    void ShowCategory(const std::string& category);
    void ShowAllCategories();
    void HideCategory(const std::string& category);
    void HideAllCategories();
    void ShowType(TechType type);
    void HideType(TechType type);
    void ShowStatus(TechStatus status);
    void HideStatus(TechStatus status);
    void ShowTech(const Tech* tech);
    void CenterOnTech(const Tech* tech);
    void DoZoom(const GG::Pt & p) const;
    void UndoZoom() const;
    GG::Pt Convert(const GG::Pt & p) const;
    //@}

    static const double ZOOM_STEP_SIZE;

private:
    class TechPanel;
    typedef std::multimap<const Tech*,
                          std::pair<const Tech*,
                                    std::vector<std::pair<double, double> > > > DependencyArcsMap;
    typedef std::map<TechStatus, DependencyArcsMap> DependencyArcsMapsByArcType;

    class LayoutSurface : public GG::Wnd {
    public:
        LayoutSurface() : Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE | GG::DRAGABLE) {}
        virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) {DraggedSignal(move);}
         virtual void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys){ButtonDownSignal(pt);}
        virtual void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {ButtonUpSignal(pt);}
        virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) {ZoomedSignal(move);}
        mutable boost::signal<void (int)> ZoomedSignal;
        mutable boost::signal<void (const GG::Pt&)> DraggedSignal;
        mutable boost::signal<void (const GG::Pt&)> ButtonDownSignal;
        mutable boost::signal<void (const GG::Pt&)> ButtonUpSignal;
    };

    void Layout(bool keep_position);
    bool TechVisible(const Tech* tech);
    void DrawArc(DependencyArcsMap::const_iterator it, GG::Clr color, bool with_arrow_head);
    void ScrolledSlot(int, int, int, int);
    void TechBrowsedSlot(const Tech* tech);
    void TechClickedSlot(const Tech* tech);
    void TechDoubleClickedSlot(const Tech* tech);
    void TreeDraggedSlot(const GG::Pt& move);
    void TreeDragBegin(const GG::Pt& move);
    void TreeDragEnd(const GG::Pt& move);
    void TreeZoomedSlot(int move);
    void TreeZoomInClicked();
    void TreeZoomOutClicked();

    double                  m_scale;
    std::set<std::string>   m_categories_shown;
    std::set<TechType>      m_tech_types_shown;
    std::set<TechStatus>    m_tech_statuses_shown;
    const Tech*             m_selected_tech;
    TechTreeLayout          m_graph;

    std::map<const Tech*, TechPanel*>   m_techs;
    DependencyArcsMapsByArcType m_dependency_arcs;

    LayoutSurface* m_layout_surface;
    CUIScroll*     m_vscroll;
    CUIScroll*     m_hscroll;
    double         m_scroll_position_x; //actual scroll position
    double         m_scroll_position_y;
    double         m_drag_scroll_position_x; //position when drag started
    double         m_drag_scroll_position_y;
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

    TechPanel(const Tech* tech, const LayoutPanel* panel);

    virtual bool InWindow(const GG::Pt& pt) const;
    virtual void Render();
    virtual void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) {ForwardEventToParent();}

    void Update();

    void Select(bool select);

    mutable boost::signal<void (const Tech*)> TechBrowsedSignal;
    mutable boost::signal<void (const Tech*)> TechClickedSignal;
    mutable boost::signal<void (const Tech*)> TechDoubleClickedSignal;

private:
    GG::Rect ProgressPanelRect(const GG::Pt& ul, const GG::Pt& lr);

    const Tech*           m_tech;
    const LayoutPanel*    m_panel;
    double                m_progress; // in [0.0, 1.0]
    GG::Clr               m_fill_color;
    GG::Clr               m_text_and_border_color;
    GG::StaticGraphic*    m_icon;
    GG::TextControl*      m_tech_name_text;
    GG::TextControl*      m_tech_cost_text;
    GG::TextControl*      m_progress_text;
};

TechTreeWnd::LayoutPanel::TechPanel::TechPanel(const Tech* tech, const LayoutPanel* panel) :
    GG::Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
    m_tech(tech),
    m_panel(panel),
    m_progress(0.0),
    m_tech_name_text(0),
    m_tech_cost_text(0),
    m_progress_text(0)
{
    GG::Pt size;
    if (m_tech->Type() == TT_THEORY) {
        size = GG::Pt(THEORY_TECH_PANEL_LAYOUT_WIDTH, THEORY_TECH_PANEL_LAYOUT_HEIGHT);
    } else if (m_tech->Type() == TT_APPLICATION) {
        size = GG::Pt(APPLICATION_TECH_PANEL_LAYOUT_WIDTH, APPLICATION_TECH_PANEL_LAYOUT_HEIGHT);
    } else { // m_tech->Type() == TT_REFINEMENT
        size = GG::Pt(REFINEMENT_TECH_PANEL_LAYOUT_WIDTH, REFINEMENT_TECH_PANEL_LAYOUT_HEIGHT);
    }
    Resize(size);

    const int FONT_PTS = ClientUI::Pts();
    boost::shared_ptr<GG::Font> font = ClientUI::GetFont(FONT_PTS);

    //REMARK: do not use AttachChild but add child->Render() to method render, as the component is zoomed
    // tech icon
    const int GRAPHIC_SIZE = static_cast<int>(Value(size.y - PROGRESS_PANEL_BOTTOM_EXTRUSION - 2));
    m_icon = new GG::StaticGraphic(GG::X1, GG::Y1, GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE), ClientUI::TechTexture(m_tech->Name()), GG::GRAPHIC_FITGRAPHIC);
    m_icon->SetColor(ClientUI::CategoryColor(m_tech->Category()));

    // tech name text
    GG::Pt UPPER_TECH_TEXT_OFFSET(GG::X(4), GG::Y(2));
    GG::Pt LOWER_TECH_TEXT_OFFSET(GG::X(4), GG::Y(0));
    UPPER_TECH_TEXT_OFFSET = GG::Pt(static_cast<GG::X>(UPPER_TECH_TEXT_OFFSET.x), static_cast<GG::Y>(UPPER_TECH_TEXT_OFFSET.y));
    LOWER_TECH_TEXT_OFFSET = GG::Pt(static_cast<GG::X>(LOWER_TECH_TEXT_OFFSET.x), static_cast<GG::Y>(LOWER_TECH_TEXT_OFFSET.y));

    GG::X text_left(m_icon->LowerRight().x + 4);
    m_tech_name_text = new GG::TextControl(text_left, UPPER_TECH_TEXT_OFFSET.y,
                                           Width() - m_icon->LowerRight().x - static_cast<GG::X>(PROGRESS_PANEL_LEFT_EXTRUSION),
                                           font->Lineskip(), UserString(m_tech->Name()), font,
                                           m_text_and_border_color, GG::FORMAT_TOP | GG::FORMAT_LEFT);
    m_tech_name_text->ClipText(true);

    // cost text box
    m_tech_cost_text = new GG::TextControl(text_left, GG::Y0,
                                           Width() - text_left,
                                           static_cast<GG::Y>(Height() - LOWER_TECH_TEXT_OFFSET.y - PROGRESS_PANEL_BOTTOM_EXTRUSION),
                                           "Tech Cost Text", font, m_text_and_border_color,
                                           GG::FORMAT_BOTTOM | GG::FORMAT_LEFT);


    // progress text box
    GG::Rect progress_panel = ProgressPanelRect(UpperLeft(), LowerRight());
    m_progress_text = new GG::TextControl(static_cast<GG::X>(progress_panel.ul.x - PROGRESS_PANEL_LEFT_EXTRUSION),
                                          static_cast<GG::Y>(progress_panel.ul.y - PROGRESS_PANEL_BOTTOM_EXTRUSION),
                                          progress_panel.Width(), progress_panel.Height(),
                                          "Progress Panel", font, m_text_and_border_color);

    // constrain long text that would otherwise overflow planel boundaries
    SetChildClippingMode(ClipToClient);


    // set text box text
    Update();
}

bool TechTreeWnd::LayoutPanel::TechPanel::InWindow(const GG::Pt& pt) const
{
    GG::Pt lr = LowerRight();
    const GG::Pt p = m_panel->Convert(pt);
    return GG::Wnd::InWindow(p) &&
        (p.x <= lr.x - PROGRESS_PANEL_LEFT_EXTRUSION || lr.y - PROGRESS_PANEL_HEIGHT <= p.y) &&
        (lr.x - PROGRESS_PANEL_WIDTH <= p.x || p.y <= lr.y - PROGRESS_PANEL_BOTTOM_EXTRUSION);
}

void TechTreeWnd::LayoutPanel::TechPanel::Render()
{
    GG::Pt      ul(GG::X(0), GG::Y(0));
    GG::Pt      lr(Width(), Height());
    
    GG::Clr     interior_color_to_use;
    GG::Clr     border_color_to_use;
    if ( m_panel->m_selected_tech == m_tech) {
        interior_color_to_use = GG::LightColor(m_fill_color);
        border_color_to_use   = GG::LightColor(m_text_and_border_color);
    } else {
        interior_color_to_use = m_fill_color;
        border_color_to_use   = m_text_and_border_color;
    }

    GG::Rect    main_panel(ul, lr);
    GG::Rect    progress_panel = ProgressPanelRect(ul, lr);
    TechType    tech_type = m_tech->Type();
    bool        show_progress = !m_progress_text->Empty();
    //YYY use m_panel in order to scale graphics TODO
    m_panel->DoZoom(UpperLeft());
    //draw frame
    RenderTechPanel(tech_type, main_panel, progress_panel, interior_color_to_use, border_color_to_use, show_progress, m_progress);
    //draw children
    m_icon->Render();
    m_tech_name_text->Render();
    m_tech_cost_text->Render();
    m_progress_text->Render();
    m_panel->UndoZoom();
}

void TechTreeWnd::LayoutPanel::TechPanel::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_panel->m_selected_tech != m_tech)
        TechClickedSignal(m_tech);
}

void TechTreeWnd::LayoutPanel::TechPanel::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    TechDoubleClickedSignal(m_tech);
}

void TechTreeWnd::LayoutPanel::TechPanel::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    TechBrowsedSignal(m_tech);
}

void TechTreeWnd::LayoutPanel::TechPanel::Select(bool select)
{
    if (select) {
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

void TechTreeWnd::LayoutPanel::TechPanel::Update()
{
    bool known_tech = false;
    bool queued_tech = false;
    bool researchable_tech = false;

    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire) {
        // determine if tech is known, researchable or queued
        if (empire->TechResearched(m_tech->Name())) {
            known_tech = true;
        } else {
            const ResearchQueue& queue = empire->GetResearchQueue();
            if (queue.InQueue(m_tech))
                queued_tech = true;
            double rps_spent = empire->ResearchStatus(m_tech->Name());
            if (0.0 <= rps_spent) {
                m_progress = rps_spent / (m_tech->ResearchTime() * m_tech->ResearchCost());
                assert(0.0 <= m_progress && m_progress <= 1.0);
            }
            researchable_tech = empire->ResearchableTech(m_tech->Name());
        }
    } else { // (!empire)
        researchable_tech = true;
    }


    // set colours according to whether tech is known, researchable or queued
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


    // update cost text
    std::string cost_str;
    if (!known_tech)
        cost_str = boost::io::str(FlexibleFormat(UserString("TECH_TOTAL_COST_STR")) %
                                  static_cast<int>(m_tech->ResearchCost() + 0.5) %
                                  m_tech->ResearchTime());
    m_tech_cost_text->SetText(cost_str);


    // update progress text
    std::string progress_str;
    if (known_tech)
        progress_str = UserString("TECH_WND_TECH_COMPLETED");
    else if (queued_tech)
        progress_str = UserString("TECH_WND_TECH_QUEUED");
    else if (m_progress)
        progress_str = UserString("TECH_WND_TECH_INCOMPLETE");
    m_progress_text->SetText(progress_str);


    Select(m_panel->m_selected_tech == m_tech);
}

GG::Rect TechTreeWnd::LayoutPanel::TechPanel::ProgressPanelRect(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Rect retval;
    //(Lathanda) removed m_scale
    retval.lr = lr + GG::Pt(static_cast<GG::X>(PROGRESS_PANEL_LEFT_EXTRUSION), static_cast<GG::Y>(PROGRESS_PANEL_BOTTOM_EXTRUSION));
    retval.ul = retval.lr - GG::Pt(static_cast<GG::X>(PROGRESS_PANEL_WIDTH), static_cast<GG::Y>(PROGRESS_PANEL_HEIGHT));
    return retval;
}

//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel                     //
//////////////////////////////////////////////////
const double TechTreeWnd::LayoutPanel::ZOOM_STEP_SIZE = 1.125;
TechTreeWnd::LayoutPanel::LayoutPanel(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_scale(1.0),
    m_categories_shown(),
    m_tech_types_shown(),
    m_tech_statuses_shown(),
    m_selected_tech(0),
    m_graph(),
    m_layout_surface(0),
    m_vscroll(0),
    m_hscroll(0),
    m_zoom_in_button(0),
    m_zoom_out_button(0)
{
    SetChildClippingMode(ClipToClient);

//    m_scale = 1.0 / ZOOM_STEP_SIZE; // because fully zoomed in is too close
    m_scale = 1.0; //(LATHANDA) Initialise Fullzoom an do real zooming using GL TODO CHec best size

    m_layout_surface = new LayoutSurface();
    m_vscroll = new CUIScroll(w - ClientUI::ScrollWidth(), GG::Y0, GG::X(ClientUI::ScrollWidth()), h - ClientUI::ScrollWidth(), GG::VERTICAL);
    m_hscroll = new CUIScroll(GG::X0, h - ClientUI::ScrollWidth(), w - ClientUI::ScrollWidth(), GG::Y(ClientUI::ScrollWidth()), GG::HORIZONTAL);

    const GG::X ZBSIZE(ClientUI::ScrollWidth() * 2);
    const int ZBOFFSET = ClientUI::ScrollWidth() / 2;
    const GG::Y TOP = UpperLeft().y;
    const GG::X LEFT = UpperLeft().x;

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

    m_zoom_in_button = new CUIButton(w - ZBSIZE - ZBOFFSET - ClientUI::ScrollWidth(), GG::Y(ZBOFFSET), ZBSIZE, "+", font, ClientUI::WndColor(), ClientUI::CtrlBorderColor(), 1, ClientUI::TextColor(), GG::INTERACTIVE | GG::ONTOP);

    m_zoom_out_button = new CUIButton(m_zoom_in_button->UpperLeft().x - LEFT, 
                                      m_zoom_in_button->LowerRight().y + ZBOFFSET - TOP, ZBSIZE, "-", font, ClientUI::WndColor(), ClientUI::CtrlBorderColor(), 1, ClientUI::TextColor(), GG::INTERACTIVE | GG::ONTOP);

    AttachChild(m_layout_surface);
    AttachChild(m_vscroll);
    AttachChild(m_hscroll);
    AttachChild(m_zoom_in_button);
    AttachChild(m_zoom_out_button);

    GG::Connect(m_layout_surface->DraggedSignal,    &TechTreeWnd::LayoutPanel::TreeDraggedSlot,     this);
    GG::Connect(m_layout_surface->ButtonUpSignal,   &TechTreeWnd::LayoutPanel::TreeDragEnd,         this);
    GG::Connect(m_layout_surface->ButtonDownSignal, &TechTreeWnd::LayoutPanel::TreeDragBegin,       this);
    GG::Connect(m_layout_surface->ZoomedSignal,     &TechTreeWnd::LayoutPanel::TreeZoomedSlot,      this);
    GG::Connect(m_vscroll->ScrolledSignal,          &TechTreeWnd::LayoutPanel::ScrolledSlot,        this);
    GG::Connect(m_hscroll->ScrolledSignal,          &TechTreeWnd::LayoutPanel::ScrolledSlot,        this);
    GG::Connect(m_zoom_in_button->ClickedSignal,    &TechTreeWnd::LayoutPanel::TreeZoomInClicked,   this);
    GG::Connect(m_zoom_out_button->ClickedSignal,   &TechTreeWnd::LayoutPanel::TreeZoomOutClicked,  this);


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
    return LowerRight() - GG::Pt(GG::X(ClientUI::ScrollWidth()), GG::Y(ClientUI::ScrollWidth()));
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
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    glDisable(GL_TEXTURE_2D);
    GLint initial_modes[2];
    glGetIntegerv(GL_POLYGON_MODE, initial_modes);

    // draw background
    glPolygonMode(GL_BACK, GL_FILL);
    glBegin(GL_POLYGON);
        glColor(ClientUI::CtrlColor());
        glVertex(ul.x, ul.y);
        glVertex(lr.x, ul.y);
        glVertex(lr.x, lr.y);
        glVertex(ul.x, lr.y);
        glVertex(ul.x, ul.y);
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);



    BeginClipping();
    // render dependency arcs
    DoZoom(GG::Pt());

    // first, draw arc with thick, half-alpha line
    glEnable(GL_LINE_SMOOTH);
    glLineWidth((int)(ARC_THICKNESS * m_scale));
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
   
    glEnable(GL_TEXTURE_2D);
    EndClipping();

    glLineWidth(1.0);
    UndoZoom();
    GG::GUI::RenderWindow(m_vscroll);
    GG::GUI::RenderWindow(m_hscroll);
}

void TechTreeWnd::LayoutPanel::Update(const Tech* tech)
{
    if (!tech) {
        // redo entire layout
        Layout(true);
    } else {
        // update just specified tech's panel
        std::map<const Tech*, TechPanel*>::iterator tech_it = m_techs.find(tech);
        if (tech_it != m_techs.end())
            tech_it->second->Update();
    }
}

void TechTreeWnd::LayoutPanel::Clear()
{
    m_vscroll->ScrollTo(0);
    m_hscroll->ScrollTo(0);
    m_vscroll->SizeScroll(0, 1, 1, 1);
    m_hscroll->SizeScroll(0, 1, 1, 1);
    GG::SignalScroll(*m_vscroll, true);
    GG::SignalScroll(*m_hscroll, true);

    // delete all panels
    for (std::map<const Tech*, TechPanel*>::const_iterator it = m_techs.begin(); it != m_techs.end(); ++it)
        delete it->second;
    m_techs.clear();
    m_graph.Clear();

    m_dependency_arcs.clear();

    m_selected_tech = 0;
}

void TechTreeWnd::LayoutPanel::Reset()
{
    // regenerate graph of panels and dependency lines
    Layout(false);
}

void TechTreeWnd::LayoutPanel::SetScale(double scale)
{
    if (scale < MIN_SCALE)
        scale = MIN_SCALE;
    if (MAX_SCALE < scale)
        scale = MAX_SCALE;
    if (m_scale != scale)
        m_scale = scale;
}

void TechTreeWnd::LayoutPanel::ShowCategory(const std::string& category)
{
    if (m_categories_shown.find(category) == m_categories_shown.end()) {
        m_categories_shown.insert(category);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::ShowAllCategories()
{
    const std::vector<std::string> all_cats = GetTechManager().CategoryNames();
    if (all_cats.size() == m_categories_shown.size())
        return;
    for (std::vector<std::string>::const_iterator it = all_cats.begin(); it != all_cats.end(); ++it)
        m_categories_shown.insert(*it);
    Layout(true);
}

void TechTreeWnd::LayoutPanel::HideCategory(const std::string& category)
{
    std::set<std::string>::iterator it = m_categories_shown.find(category);
    if (it != m_categories_shown.end()) {
        m_categories_shown.erase(it);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::HideAllCategories()
{
    if (m_categories_shown.empty())
        return;
    m_categories_shown.clear();
    Layout(true);
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
        m_hscroll->ScrollTo(Value(center_point.x - client_size.x / 2));
        GG::SignalScroll(*m_hscroll, true);
        m_vscroll->ScrollTo(Value(center_point.y - client_size.y / 2));
        GG::SignalScroll(*m_vscroll, true);
    } else {
        Logger().debugStream() << "TechTreeWnd::LayoutPanel::CenterOnTech couldn't centre on " << (tech ? tech->Name() : "Null tech pointer") << " due to lack of such a tech panel";
    }
}

void TechTreeWnd::LayoutPanel::DoZoom(const GG::Pt & p) const {
    glPushMatrix();
    //center to panel
    glTranslated(Value(Width()/2.0),Value(Height()/2.0),0);
    //zoom
    glScaled(m_scale, m_scale, 1);
    //translate to actual scroll position
    glTranslated(-m_scroll_position_x, -m_scroll_position_y, 0);
    glTranslated(Value(p.x), Value(p.y), 0);
}

void TechTreeWnd::LayoutPanel::UndoZoom() const {
    glPopMatrix();
}

GG::Pt TechTreeWnd::LayoutPanel::Convert(const GG::Pt & p) const {
/*
* Converts screen coordinate into virtual coordiante
* doing the inverse transformation as DoZoom in the same order
*/    
    double x = Value(p.x);
    double y = Value(p.y);
    x -= Value(Width()/2.0);
    y -= Value(Height()/2.0);
    x /= m_scale;
    y /= m_scale;
    x += m_scroll_position_x;
    y += m_scroll_position_y;
    return GG::Pt(GG::X((int)x),GG::Y((int)y));

}

void TechTreeWnd::LayoutPanel::Layout(bool keep_position) 
{
    //constants
    const GG::X TECH_PANEL_MARGIN_X(ClientUI::Pts()*16);
    const GG::Y TECH_PANEL_MARGIN_Y(ClientUI::Pts()*16 + 100);
    const double RANK_SEP = Value(TECH_PANEL_LAYOUT_WIDTH) * GetOptionsDB().Get<double>("UI.tech-layout-horz-spacing");
    const double NODE_SEP = Value(TECH_PANEL_LAYOUT_HEIGHT) * GetOptionsDB().Get<double>("UI.tech-layout-vert-spacing");
    const double WIDTH = Value(TECH_PANEL_LAYOUT_WIDTH);
    const double HEIGHT = Value(TECH_PANEL_LAYOUT_HEIGHT);
    const double X_MARGIN = HORIZONTAL_LINE_LENGTH;

    //store data that may be restored
    double relativ_scroll_position_x = m_hscroll->PosnRange().first / m_hscroll->ScrollRange().second;
    double relativ_scroll_position_y = m_vscroll->PosnRange().first / m_vscroll->ScrollRange().second;
    const Tech* selected_tech = m_selected_tech;

    //cleanup old data for new layout
    Clear();

    Logger().debugStream() << "Tech Tree Layout Preparing Tech Data";

    //create a node for every tech
    TechManager& manager = GetTechManager();
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        if (!TechVisible(*it))
            continue;
        m_techs[*it] = new TechPanel(*it, this);
        m_graph.AddNode(*it, m_techs[*it]->Width(), m_techs[*it]->Height());
    }

    //create an edge for every prerequisite
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        if (!TechVisible(*it))
            continue;
        for (std::set<std::string>::const_iterator prereq_it = (*it)->Prerequisites().begin();
             prereq_it != (*it)->Prerequisites().end();
             ++prereq_it)
        {
            const Tech* tech = GetTech(*prereq_it);
            if (!tech || !TechVisible(tech))
                continue;
            m_graph.AddEdge(*prereq_it, (*it)->Name());
        }
    }

    Logger().debugStream() << "Tech Tree Layout Doing Graph Layout";

    //calculate layout
    m_graph.DoLayout((int)(WIDTH + RANK_SEP), (int)(HEIGHT + NODE_SEP), (int)(X_MARGIN));

    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());

    Logger().debugStream() << "Tech Tree Layout Creating Panels";

    // create new tech panels and new dependency arcs 
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        if (!TechVisible(*it))
            continue;
        //techpanel
        const Tech* tech = *it;
        const TechTreeLayout::Node* node = m_graph.GetNode(tech->Name());
        assert(tech);
        //move TechPanel
        m_techs[tech]->MoveTo(GG::Pt(
            node->GetX( ),
            node->GetY( )
        ));
        m_layout_surface->AttachChild(m_techs[tech]);
        GG::Connect(m_techs[tech]->TechBrowsedSignal,       &TechTreeWnd::LayoutPanel::TechBrowsedSlot,         this);
        GG::Connect(m_techs[tech]->TechClickedSignal,       &TechTreeWnd::LayoutPanel::TechClickedSlot,         this);
        GG::Connect(m_techs[tech]->TechDoubleClickedSignal, &TechTreeWnd::LayoutPanel::TechDoubleClickedSlot,   this);

        const std::vector<TechTreeLayout::Edge*> edges = m_graph.GetOutEdges( tech->Name( ) );
        //prerequisite edge
        for (std::vector<TechTreeLayout::Edge*>::const_iterator edge = edges.begin(); edge != edges.end(); edge++) {
            std::vector<std::pair<double, double> > points;
            const Tech* from = (*edge)->GetTechFrom( );
            const Tech* to   = (*edge)->GetTechTo( );
            assert(from && to);
            (*edge)->ReadPoints( points );
            TechStatus arc_type = TS_RESEARCHABLE;
            if (empire)
                arc_type = empire->GetTechStatus(to->Name( ));
            m_dependency_arcs[arc_type].insert(std::make_pair(from, std::make_pair(to, points)));
        }
        
    }
    //format window
    GG::Pt client_sz = ClientSize();
    GG::Pt layout_size(
        std::max(client_sz.x, m_graph.GetWidth( ) + 2 * TECH_PANEL_MARGIN_X + PROGRESS_PANEL_LEFT_EXTRUSION),
        std::max(client_sz.y, m_graph.GetHeight( ) + 2 * TECH_PANEL_MARGIN_Y + PROGRESS_PANEL_BOTTOM_EXTRUSION)
    );
    m_layout_surface->Resize(layout_size);
    //format scrollbar
    m_vscroll->SizeScroll(0, Value(layout_size.y - 1), std::max(50, Value(std::min(layout_size.y / 10, client_sz.y))), Value(client_sz.y));
    m_hscroll->SizeScroll(0, Value(layout_size.x - 1), std::max(50, Value(std::min(layout_size.x / 10, client_sz.x))), Value(client_sz.x));

    Logger().debugStream() << "Tech Tree Layout Done";

    //restore save data
    if (keep_position) {
        m_selected_tech = selected_tech;
        m_hscroll->ScrollTo((int)(m_hscroll->ScrollRange().second * relativ_scroll_position_x));
        m_vscroll->ScrollTo((int)(m_vscroll->ScrollRange().second * relativ_scroll_position_y));
    } else {
        m_selected_tech = 0;
        // find a tech to centre view on
        for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
            if (TechVisible(*it)) {
                CenterOnTech(*it);
                break;
            }
        }
    }

    // ensure that the scrolls stay on top
    MoveChildUp(m_vscroll);
    MoveChildUp(m_hscroll);
}

bool TechTreeWnd::LayoutPanel::TechVisible(const Tech* tech)
{
    if (!tech)
        return false;
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return true;

    // Unresearchable techs are never to be shown on tree
    if (!tech->Researchable())
        return false;

    // check that tech's type, category and status are all visible
    if (m_tech_types_shown.find(tech->Type()) == m_tech_types_shown.end())
        return false;
    if (m_categories_shown.find(tech->Category()) == m_categories_shown.end())
        return false;
    if (m_tech_statuses_shown.find(empire->GetTechStatus(tech->Name())) == m_tech_statuses_shown.end())
        return false;

    // all tests pass, so tech is visible
    return true;
}

void TechTreeWnd::LayoutPanel::DrawArc(DependencyArcsMap::const_iterator it, GG::Clr color, bool with_arrow_head)
{
    GG::Pt ul = UpperLeft();
    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i < it->second.second.size(); ++i) {
        glVertex(it->second.second[i].first + ul.x, it->second.second[i].second + ul.y);
    }
    glEnd();
    if (with_arrow_head) {
        double final_point_x = Value(it->second.second.back().first + ul.x);
        double final_point_y = Value(it->second.second.back().second + ul.y);
        double second_to_final_point_x = Value(it->second.second[it->second.second.size() - 2].first + ul.x);
        double second_to_final_point_y = Value(it->second.second[it->second.second.size() - 2].second + ul.y);
        const double ARROW_LENGTH = 10 * m_scale;
        const double ARROW_WIDTH = 9 * m_scale;
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        glTranslated(final_point_x, final_point_y, 0.0);
        glRotated(std::atan2(final_point_y - second_to_final_point_y, final_point_x - second_to_final_point_x) * 180.0 / 3.141594, 0.0, 0.0, 1.0);
        glTranslated(-final_point_x, -final_point_y, 0.0);
        IsoscelesTriangle(GG::Pt(GG::X(static_cast<int>(final_point_x - ARROW_LENGTH + 0.5)),
                                 GG::Y(static_cast<int>(final_point_y - ARROW_WIDTH / 2.0 + 0.5))),
                          GG::Pt(GG::X(static_cast<int>(final_point_x + 0.5)),
                                 GG::Y(static_cast<int>(final_point_y + ARROW_WIDTH / 2.0 + 0.5))),
                          SHAPE_RIGHT, color, false);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);
    }
}

void TechTreeWnd::LayoutPanel::ScrolledSlot(int, int, int, int)
{
    m_scroll_position_x = m_hscroll->PosnRange().first;
    m_scroll_position_y = m_vscroll->PosnRange().first;
   // m_layout_surface->MoveTo(GG::Pt(-scroll_x, -scroll_y));
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
    m_hscroll->ScrollTo(m_drag_scroll_position_x - Value(move.x / m_scale));
    m_vscroll->ScrollTo(m_drag_scroll_position_y - Value(move.y / m_scale));
    m_scroll_position_x = m_hscroll->PosnRange().first;
    m_scroll_position_y = m_vscroll->PosnRange().first;
}

void TechTreeWnd::LayoutPanel::TreeDragBegin(const GG::Pt& pt)
{
    m_drag_scroll_position_x = m_scroll_position_x;
    m_drag_scroll_position_y = m_scroll_position_y;
}

void TechTreeWnd::LayoutPanel::TreeDragEnd(const GG::Pt& pt)
{
    m_drag_scroll_position_x = m_scroll_position_x;
    m_drag_scroll_position_y = m_scroll_position_y;
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
// TechTreeWnd::TechListBox                     //
//////////////////////////////////////////////////
class TechTreeWnd::TechListBox : public CUIListBox
{
public:
    /** \name Structors */ //@{
    TechListBox(GG::X x, GG::Y y, GG::X w, GG::Y h);
    //@}

    /** \name Accessors */ //@{
    std::set<std::string>   GetCategoriesShown() const;
    std::set<TechType>      GetTechTypesShown() const;
    std::set<TechStatus>    GetTechStatusesShown() const;
    //@}

    //! \name Mutators //@{
    void    Reset();

    void    ShowCategory(const std::string& category);
    void    ShowAllCategories();
    void    HideCategory(const std::string& category);
    void    HideAllCategories();
    void    ShowType(TechType type);
    void    HideType(TechType type);
    void    ShowStatus(TechStatus status);
    void    HideStatus(TechStatus status);

    bool    TechVisible(const Tech* tech);
    //@}

    mutable boost::signal<void (const Tech*)>   TechBrowsedSignal;          ///< emitted when the mouse rolls over a technology
    mutable boost::signal<void (const Tech*)>   TechClickedSignal;          ///< emitted when a technology is single-clicked
    mutable boost::signal<void (const Tech*)>   TechDoubleClickedSignal;    ///< emitted when a technology is double-clicked

private:
    class TechRow : public CUIListBox::Row {
    public:
        TechRow(GG::X w, const Tech* tech);
        const Tech*                    GetTech() { return m_tech; }
        virtual void                   Render();
        static std::vector<GG::X> ColWidths(GG::X total_width);

    private:
        const Tech*     m_tech;
    };

    void    Populate();
    void    PropagateDoubleClickSignal(GG::ListBox::iterator it);
    void    PropagateLeftClickSignal(GG::ListBox::iterator it, const GG::Pt& pt);

    std::set<std::string> m_categories_shown;
    std::set<TechType>    m_tech_types_shown;
    std::set<TechStatus>  m_tech_statuses_shown;
    std::vector<TechRow*> m_all_tech_rows;
};

void TechTreeWnd::TechListBox::TechRow::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_WHITE, 1);
}

std::vector<GG::X> TechTreeWnd::TechListBox::TechRow::ColWidths(GG::X total_width)
{
    const GG::X GRAPHIC_WIDTH(ClientUI::Pts() * 2);
    const GG::X NAME_WIDTH(ClientUI::Pts() * 18);
    const GG::X COST_WIDTH(ClientUI::Pts() * 2);
    const GG::X TIME_WIDTH(ClientUI::Pts() * 2);
    const GG::X CATEGORY_WIDTH(ClientUI::Pts() * 8);
    const GG::X TYPE_WIDTH(ClientUI::Pts() * 8);

    const GG::X DESC_WIDTH = std::max(GG::X1, total_width - GRAPHIC_WIDTH - NAME_WIDTH - COST_WIDTH - TIME_WIDTH - CATEGORY_WIDTH - TYPE_WIDTH);
    std::vector<GG::X> retval;
    retval.push_back(GRAPHIC_WIDTH);
    retval.push_back(NAME_WIDTH);
    retval.push_back(COST_WIDTH);
    retval.push_back(TIME_WIDTH);
    retval.push_back(CATEGORY_WIDTH);
    retval.push_back(TYPE_WIDTH);
    retval.push_back(DESC_WIDTH);
    return retval;
}

TechTreeWnd::TechListBox::TechRow::TechRow(GG::X w, const Tech* tech) :
    CUIListBox::Row(w, GG::Y(ClientUI::Pts() * 2 + 5), "TechListBox::TechRow"),
    m_tech(tech)
{
    if (!tech)
        return;

    std::vector<GG::X> col_widths = ColWidths(w);
    const GG::X GRAPHIC_WIDTH =   col_widths[0];
    const GG::X NAME_WIDTH =      col_widths[1];
    const GG::X COST_WIDTH =      col_widths[2];
    const GG::X TIME_WIDTH =      col_widths[3];
    const GG::X CATEGORY_WIDTH =  col_widths[4];
    const GG::X TYPE_WIDTH =      col_widths[5];
    const GG::X DESC_WIDTH =      col_widths[6];
    const GG::Y HEIGHT(Value(GRAPHIC_WIDTH));

    GG::StaticGraphic* graphic = new GG::StaticGraphic(GG::X0, GG::Y0, GRAPHIC_WIDTH, HEIGHT, ClientUI::TechTexture(m_tech->Name()), GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
    graphic->SetColor(ClientUI::CategoryColor(m_tech->Category()));
    push_back(graphic);

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

    GG::TextControl* text = new GG::TextControl(GG::X0, GG::Y0, NAME_WIDTH, HEIGHT, UserString(m_tech->Name()), font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    text->ClipText(true);
    push_back(text);

    std::string cost_str = boost::lexical_cast<std::string>(static_cast<int>(m_tech->ResearchCost() + 0.5));
    text = new GG::TextControl(GG::X0, GG::Y0, COST_WIDTH, HEIGHT, cost_str, font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);

    std::string time_str = boost::lexical_cast<std::string>(m_tech->ResearchTime());
    text = new GG::TextControl(GG::X0, GG::Y0, TIME_WIDTH, HEIGHT, time_str, font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);

    std::string category_str = UserString(m_tech->Category());
    text = new GG::TextControl(GG::X0, GG::Y0, CATEGORY_WIDTH, HEIGHT, category_str, font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);

    std::string type_str = UserString(boost::lexical_cast<std::string>(m_tech->Type()));
    text = new GG::TextControl(GG::X0, GG::Y0, TYPE_WIDTH, HEIGHT, type_str, font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);

    std::string desc_str = UserString(m_tech->ShortDescription());
    text = new GG::TextControl(GG::X0, GG::Y0, DESC_WIDTH, HEIGHT, desc_str, font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);
}

TechTreeWnd::TechListBox::TechListBox(GG::X x, GG::Y y, GG::X w, GG::Y h) :
    CUIListBox(x, y, w, h),
    m_categories_shown(),
    m_tech_types_shown(),
    m_tech_statuses_shown()
{
    GG::Connect(DoubleClickedSignal,    &TechListBox::PropagateDoubleClickSignal,   this);
    GG::Connect(LeftClickedSignal,      &TechListBox::PropagateLeftClickSignal,     this);

    SetStyle(GG::LIST_NOSORT);

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

    std::vector<GG::X> col_widths = TechRow::ColWidths(w - ClientUI::ScrollWidth() - 6);
    SetNumCols(col_widths.size());
    LockColWidths();
    for (unsigned int i = 0; i < col_widths.size(); ++i) {
        SetColWidth(i, col_widths[i]);
        SetColAlignment(i, GG::ALIGN_LEFT);
    }
}

std::set<std::string> TechTreeWnd::TechListBox::GetCategoriesShown() const
{
    return m_categories_shown;
}

std::set<TechType> TechTreeWnd::TechListBox::GetTechTypesShown() const
{
    return m_tech_types_shown;
}

std::set<TechStatus> TechTreeWnd::TechListBox::GetTechStatusesShown() const
{
    return m_tech_statuses_shown;
}

void TechTreeWnd::TechListBox::Reset()
{
    Populate();
}

void TechTreeWnd::TechListBox::Populate()
{
    // abort of not visible to see results
    if (!Visible())
        return;

    Logger().debugStream() << "Tech List Box Populating";

    double creation_elapsed = 0.0;
    double insertion_elapsed = 0.0;
    boost::timer creation_timer;
    boost::timer insertion_timer;
    // HACK! This caching of TechRows works only if there are no "hidden" techs
    // that are added to the manager mid-game.
    TechManager& manager = GetTechManager();
    if (m_all_tech_rows.empty()) {
        for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
            const Tech* tech = *it;
            creation_timer.restart();
            m_all_tech_rows.push_back(new TechRow(Width() - ClientUI::ScrollWidth() - 6, tech));
            creation_elapsed += creation_timer.elapsed();
        }
    }

    // remove techs in listbox, then reset the rest of its state
    for (iterator it = begin(); it != end(); ) {
        iterator temp_it = it++;
        Erase(temp_it);
    }
    Clear();

    for (std::vector<TechRow*>::iterator it = m_all_tech_rows.begin();
         it != m_all_tech_rows.end();
         ++it) {
        TechRow* tech_row = *it;
        if (TechVisible(tech_row->GetTech())) {
            insertion_timer.restart();
            Insert(*it);
            insertion_elapsed += insertion_timer.elapsed();
        }
    }

    Logger().debugStream() << "Tech List Box Done Populating";
    Logger().debugStream() << "    Creation time=" << (creation_elapsed * 1000) << "ms";
    Logger().debugStream() << "    Insertion time=" << (insertion_elapsed * 1000) << "ms";
}

void TechTreeWnd::TechListBox::ShowCategory(const std::string& category)
{
    if (m_categories_shown.find(category) == m_categories_shown.end()) {
        m_categories_shown.insert(category);
        Populate();
    }
}

void TechTreeWnd::TechListBox::ShowAllCategories()
{
    const std::vector<std::string> all_cats = GetTechManager().CategoryNames();
    if (all_cats.size() == m_categories_shown.size())
        return;
    for (std::vector<std::string>::const_iterator it = all_cats.begin(); it != all_cats.end(); ++it)
        m_categories_shown.insert(*it);
    Populate();
}

void TechTreeWnd::TechListBox::HideCategory(const std::string& category)
{
    std::set<std::string>::iterator it = m_categories_shown.find(category);
    if (it != m_categories_shown.end()) {
        m_categories_shown.erase(it);
        Populate();
    }
}

void TechTreeWnd::TechListBox::HideAllCategories()
{
    if (m_categories_shown.empty())
        return;
    m_categories_shown.clear();
    Populate();
}

void TechTreeWnd::TechListBox::ShowType(TechType type)
{
    if (m_tech_types_shown.find(type) == m_tech_types_shown.end()) {
        m_tech_types_shown.insert(type);
        Populate();
    }
}

void TechTreeWnd::TechListBox::HideType(TechType type)
{
    std::set<TechType>::iterator it = m_tech_types_shown.find(type);
    if (it != m_tech_types_shown.end()) {
        m_tech_types_shown.erase(it);
        Populate();
    }
}

void TechTreeWnd::TechListBox::ShowStatus(TechStatus status)
{
    if (m_tech_statuses_shown.find(status) == m_tech_statuses_shown.end()) {
        m_tech_statuses_shown.insert(status);
        Populate();
    }
}

void TechTreeWnd::TechListBox::HideStatus(TechStatus status)
{
    std::set<TechStatus>::iterator it = m_tech_statuses_shown.find(status);
    if (it != m_tech_statuses_shown.end()) {
        m_tech_statuses_shown.erase(it);
        Populate();
    }
}

bool TechTreeWnd::TechListBox::TechVisible(const Tech* tech)
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return true;

    // check that tech's type, category and status are all visible

    if (m_tech_types_shown.find(tech->Type()) == m_tech_types_shown.end())
        return false;

    if (m_categories_shown.find(tech->Category()) == m_categories_shown.end())
        return false;

    if (m_tech_statuses_shown.find(empire->GetTechStatus(tech->Name())) == m_tech_statuses_shown.end())
        return false;

    // all tests pass, so tech is visible
    return true;
}

void TechTreeWnd::TechListBox::PropagateLeftClickSignal(GG::ListBox::iterator it, const GG::Pt& pt) {
    // determine type of row that was clicked, and emit appropriate signal

    TechRow* tech_row = dynamic_cast<TechRow*>(*it);
    if (tech_row)
        TechClickedSignal(tech_row->GetTech());
}

void TechTreeWnd::TechListBox::PropagateDoubleClickSignal(GG::ListBox::iterator it) {
    // determine type of row that was clicked, and emit appropriate signal

    TechRow* tech_row = dynamic_cast<TechRow*>(*it);
    if (tech_row)
        TechDoubleClickedSignal(tech_row->GetTech());
}


//////////////////////////////////////////////////
// TechTreeWnd                                  //
//////////////////////////////////////////////////
const GG::Y TechTreeWnd::NAVIGATOR_AND_DETAIL_HEIGHT(200);

TechTreeWnd::TechTreeWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_enc_detail_panel(0),
    m_tech_navigator(0),
    m_layout_panel(0),
    m_tech_list(0)
{
    Sound::TempUISoundDisabler sound_disabler;

    const GG::X NAVIGATOR_WIDTH(214);

    m_layout_panel = new LayoutPanel(w, h);
    GG::Connect(m_layout_panel->TechBrowsedSignal, &TechTreeWnd::TechBrowsedSlot, this);
    GG::Connect(m_layout_panel->TechClickedSignal, &TechTreeWnd::TechClickedSlot, this);
    GG::Connect(m_layout_panel->TechDoubleClickedSignal, &TechTreeWnd::TechDoubleClickedSlot, this);
    AttachChild(m_layout_panel);

    m_tech_list = new TechListBox(GG::X0, GG::Y0, w, h);
    GG::Connect(m_tech_list->TechBrowsedSignal, &TechTreeWnd::TechBrowsedSlot, this);
    GG::Connect(m_tech_list->TechClickedSignal, &TechTreeWnd::TechClickedSlot, this);
    GG::Connect(m_tech_list->TechDoubleClickedSignal, &TechTreeWnd::TechDoubleClickedSlot, this);

    m_enc_detail_panel = new EncyclopediaDetailPanel(m_layout_panel->ClientWidth() - NAVIGATOR_WIDTH, NAVIGATOR_AND_DETAIL_HEIGHT);
    AttachChild(m_enc_detail_panel);

    m_tech_navigator = new TechNavigator(NAVIGATOR_WIDTH, NAVIGATOR_AND_DETAIL_HEIGHT);
    m_tech_navigator->MoveTo(GG::Pt(m_enc_detail_panel->Width(), GG::Y1));
    GG::Connect(m_tech_navigator->TechClickedSignal, &TechTreeWnd::CenterOnTech, this);
    m_layout_panel->AttachChild(m_tech_navigator);

    m_tech_tree_controls = new TechTreeControls(GG::X1, NAVIGATOR_AND_DETAIL_HEIGHT, m_layout_panel->Width() - ClientUI::ScrollWidth());
    m_tech_tree_controls->MoveTo(GG::Pt(GG::X1, m_layout_panel->Height() - ClientUI::ScrollWidth() - m_tech_tree_controls->Height()));
    AttachChild(m_tech_tree_controls);

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
    // connect view type selectors
    GG::Connect(m_tech_tree_controls->m_tree_view_button->ClickedSignal, &TechTreeWnd::ShowTreeView, this);
    GG::Connect(m_tech_tree_controls->m_list_view_button->ClickedSignal, &TechTreeWnd::ShowListView, this);

    ShowAllCategories();
    for (TechStatus status = TechStatus(0); status != NUM_TECH_STATUSES; status = TechStatus(status + 1))
        ShowStatus(status);
    for (TechType type = TechType(0); type != NUM_TECH_TYPES; type = TechType(type + 1))
        ShowType(type);

    ShowTreeView();
}

TechTreeWnd::~TechTreeWnd()
{
    delete m_tech_list;
    delete m_layout_panel;
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

void TechTreeWnd::Update(const Tech* tech)
{
    m_layout_panel->Update(tech);
    // no list update available or needed as of this writing
}

void TechTreeWnd::Clear()
{
    m_tech_navigator->SetTech(0);
    m_enc_detail_panel->OnUp();
    m_layout_panel->Clear();
}

void TechTreeWnd::Reset()
{
    m_layout_panel->Reset();
    m_tech_list->Reset();
}

void TechTreeWnd::ShowCategory(const std::string& category)
{
    m_layout_panel->ShowCategory(category);
    m_tech_list->ShowCategory(category);

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
    m_layout_panel->ShowAllCategories();
    m_tech_list->ShowAllCategories();

    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    int i = 0;
    for (std::vector<std::string>::const_iterator cats_it = cats.begin(); cats_it != cats.end(); ++cats_it, ++i) {
        CUIButton* button = m_tech_tree_controls->m_category_buttons[i];
        button->MarkSelectedTechCategoryColor(*cats_it);
    }
}

void TechTreeWnd::HideCategory(const std::string& category)
{
    m_layout_panel->HideCategory(category);
    m_tech_list->HideCategory(category);

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
    m_layout_panel->HideAllCategories();
    m_tech_list->HideAllCategories();

    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    int i = 0;
    for (std::vector<std::string>::const_iterator cats_it = cats.begin(); cats_it != cats.end(); ++cats_it, ++i) {
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
    m_tech_list->ShowStatus(status);

    CUIButton* button = m_tech_tree_controls->m_tech_status_buttons[status];
    button->MarkSelectedGray();
}

void TechTreeWnd::HideStatus(TechStatus status)
{
    m_layout_panel->HideStatus(status);
    m_tech_list->HideStatus(status);

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
    m_tech_list->ShowType(type);

    CUIButton* button = m_tech_tree_controls->m_tech_type_buttons[type];
    button->MarkSelectedGray();
}

void TechTreeWnd::HideType(TechType type)
{
    m_layout_panel->HideType(type);
    m_tech_list->HideType(type);

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

void TechTreeWnd::ShowTreeView()
{
    AttachChild(m_layout_panel);
    MoveChildDown(m_layout_panel);
    DetachChild(m_tech_list);
    m_tech_tree_controls->m_list_view_button->MarkNotSelected();
    m_tech_tree_controls->m_tree_view_button->MarkSelectedGray();
    MoveChildUp(m_tech_tree_controls);
}

void TechTreeWnd::ShowListView()
{
    AttachChild(m_tech_list);
    MoveChildDown(m_tech_list);
    DetachChild(m_layout_panel);
    m_tech_tree_controls->m_list_view_button->MarkSelectedGray();
    m_tech_tree_controls->m_tree_view_button->MarkNotSelected();
    MoveChildUp(m_tech_tree_controls);
}

void TechTreeWnd::SetScale(double scale)
{
    m_layout_panel->SetScale(scale);
}

void TechTreeWnd::CenterOnTech(const Tech* tech)
{
    // ensure tech is visible
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire)
        ShowStatus(empire->GetTechStatus(tech->Name()));
    ShowCategory(tech->Category());
    ShowType(tech->Type());

    // centre on it
    //m_layout_panel->ShowTech(tech);   // show tech is detail window.  not really necessary for centring
    m_layout_panel->CenterOnTech(tech);
}

void TechTreeWnd::SetEncyclopediaTech(const Tech* tech)
{
    m_enc_detail_panel->SetItem(tech);
}

void TechTreeWnd::SelectTech(const Tech* tech)
{
    m_tech_navigator->SetTech(tech);
    m_layout_panel->ShowTech(tech);
}

void TechTreeWnd::TechBrowsedSlot(const Tech* tech)
{
    TechBrowsedSignal(tech);
}

void TechTreeWnd::TechClickedSlot(const Tech* tech)
{
    m_tech_navigator->SetTech(tech);
    SetEncyclopediaTech(tech);
    TechSelectedSignal(tech);
}

void TechTreeWnd::TechDoubleClickedSlot(const Tech* tech)
{
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    std::string name = tech->Name();
    TechStatus tech_status = TS_UNRESEARCHABLE;
    if (empire)
        tech_status = empire->GetTechStatus(name);

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
                techs_to_add_map.insert(std::pair<double, const Tech*>(cur_tech->ResearchTime(), cur_tech));
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
                techs_to_add_map.insert(std::pair<double, const Tech*>(cur_tech->ResearchTime(), cur_tech));
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
