#include "TechTreeWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "CUIWnd.h"
#include "Sound.h"
#include "InfoPanels.h"
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
    void AddOptions(OptionsDB& db) {
        db.Add("UI.tech-layout-horz-spacing", "OPTIONS_DB_UI_TECH_LAYOUT_HORZ_SPACING", 1.0,  RangedStepValidator<double>(0.25, 0.25, 4.0));
        db.Add("UI.tech-layout-vert-spacing", "OPTIONS_DB_UI_TECH_LAYOUT_VERT_SPACING", 0.75, RangedStepValidator<double>(0.25, 0.25, 4.0));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    const int   MAIN_PANEL_CORNER_RADIUS = 8;
    const float ARC_THICKNESS = 3.0;
    const GG::X TECH_PANEL_WIDTH(225);
    const GG::Y TECH_PANEL_HEIGHT(60);

    const double ZOOM_STEP_SIZE = 1.12;
    const double MIN_SCALE = std::pow(ZOOM_STEP_SIZE, -25.0);
    const double MAX_SCALE = std::pow(ZOOM_STEP_SIZE, 10.0);
    const double INITIAL_SCALE = std::pow(ZOOM_STEP_SIZE, -5.0);

    struct ToggleCategoryFunctor {
        ToggleCategoryFunctor(TechTreeWnd* tree_wnd, const std::string& category) : m_tree_wnd(tree_wnd), m_category(category) {}
        void operator()() {m_tree_wnd->ToggleCategory(m_category);}
        TechTreeWnd* const m_tree_wnd;
        const std::string m_category;
    };

    struct ToggleAllCategoriesFunctor {
        ToggleAllCategoriesFunctor(TechTreeWnd* tree_wnd) : m_tree_wnd(tree_wnd) {}
        void operator()() {m_tree_wnd->ToggleAllCategories();}
        TechTreeWnd* const m_tree_wnd;
    };

    struct ToggleTechStatusFunctor {
        ToggleTechStatusFunctor(TechTreeWnd* tree_wnd, TechStatus status) : m_tree_wnd(tree_wnd), m_status(status) {}
        void operator()() {m_tree_wnd->ToggleStatus(m_status);}
        TechTreeWnd* const m_tree_wnd;
        const TechStatus m_status;
    };
}


//////////////////////////////////////////////////
// TechTreeWnd::TechTreeControls                //
//////////////////////////////////////////////////
/** A panel of buttons that control how the tech tree is displayed: what
  * categories, statuses and types of techs to show. */
class TechTreeWnd::TechTreeControls : public CUIWnd {
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

    /** These values are determined when doing button layout, and stored.
      * They are later used when rendering separator lines between the groups
      * of buttons */
    int m_buttons_per_row;              // number of buttons that can fit into available horizontal space
    GG::X m_col_offset;                 // horizontal distance between each column of buttons
    GG::Y m_row_offset;                 // vertical distance between each row of buttons
    int m_category_button_rows;         // number of rows used for category buttons
    int m_status_button_rows;   // number of rows used for status buttons and for type buttons (both groups have the same number of buttons (three) so use the same number of rows)

    /** These values are used for rendering separator lines between groups of buttons */
    static const int BUTTON_SEPARATION; // vertical or horizontal sepration between adjacent buttons
    static const int UPPER_LEFT_PAD;    // offset of buttons' position from top left of controls box

    // TODO: replace all the above stored information with a vector of pairs of GG::Pt (or perhaps GG::Rect)
    // This will contain the start and end points of all separator lines that need to be drawn.  This will be
    // calculated by SizeMove, and stored, so that start and end positions don't need to be recalculated each
    // time Render is called.

    std::vector<CUIButton*>             m_category_buttons;
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

void TechTreeWnd::TechTreeControls::DoButtonLayout() {
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
    for (std::vector<CUIButton*>::iterator it = m_category_buttons.begin();
         it != m_category_buttons.end(); ++it)
    {
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

    // place status buttons: fill each row completely before starting next row
    for (std::map<TechStatus, CUIButton*>::iterator it = m_tech_status_buttons.begin();
         it != m_tech_status_buttons.end(); ++it)
    {
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
        m_status_button_rows = 3;   // three rows, one button per row
    else if (m_buttons_per_row == 2)
        m_status_button_rows = 2;   // two rows, one with two buttons, one with one button
    else
        m_status_button_rows = 1;   // only one row, three buttons per row

    // prevent window from being shrunk less than one button width, or current number of rows of height
    SetMinSize(GG::Pt(UPPER_LEFT_PAD + MIN_BUTTON_WIDTH + 3*RIGHT_EDGE_PAD,
                      CUIWnd::BORDER_TOP + CUIWnd::BORDER_BOTTOM + UPPER_LEFT_PAD + (++row)*m_row_offset));
}

void TechTreeWnd::TechTreeControls::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    // maybe later do something interesting with docking
    CUIWnd::SizeMove(ul, lr);                               // set width and upper left as user-requested
    DoButtonLayout();                                       // given set width, position buttons and set appropriate minimum height
    CUIWnd::SizeMove(ul, GG::Pt(lr.x, ul.y + MinSize().y)); // width and upper left unchanged.  set height to minimum height
}

void TechTreeWnd::TechTreeControls::Render() {
    CUIWnd::Render();

    //GG::Pt ul = UpperLeft();
    //GG::Pt lr = LowerRight();
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
            GG::Y status_bottom = category_bottom + m_status_button_rows*m_row_offset;
            glVertex(cl_ul.x, status_bottom);
            glVertex(cl_lr.x - 1, status_bottom);
        }
    glEnd();

    // reset this to whatever it was initially
    glPolygonMode(GL_BACK, initial_modes[1]);

    glEnable(GL_TEXTURE_2D);
}

void TechTreeWnd::TechTreeControls::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) {
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
            GG::Pt ul = UpperLeft();
            GG::Pt new_ul = ul + move;
            //GG::Pt new_lr = lr + move;

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
// TechTreeWnd::LayoutPanel                     //
//////////////////////////////////////////////////
/** The window that contains the actual tech panels and dependency arcs. */
class TechTreeWnd::LayoutPanel : public GG::Wnd {
public:
    /** \name Structors */ //@{
    LayoutPanel(GG::X w, GG::Y h);
    //@}

    /** \name Accessors */ //@{
    virtual GG::Pt          ClientLowerRight() const;
    double                  Scale() const;
    std::set<std::string>   GetCategoriesShown() const;
    std::set<TechStatus>    GetTechStatusesShown() const;

    mutable TechTreeWnd::TechSignalType         TechBrowsedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechClickedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechDoubleClickedSignal;
    //@}

    //! \name Mutators //@{
    virtual void Render();

    void Update();  ///< update indicated \a tech panel or all panels if \a tech_name is an empty string, without redoing layout
    void Clear();                               ///< remove all tech panels
    void Reset();                               ///< redo layout, recentre on a tech
    void SetScale(double scale);
    void ShowCategory(const std::string& category);
    void ShowAllCategories();
    void HideCategory(const std::string& category);
    void HideAllCategories();
    void ShowStatus(TechStatus status);
    void HideStatus(TechStatus status);
    void ShowTech(const std::string& tech_name);
    void CenterOnTech(const std::string& tech_name);
    void DoZoom(const GG::Pt & p) const;
    void UndoZoom() const;
    GG::Pt Convert(const GG::Pt & p) const;
    //@}

private:
    class TechPanel;
    typedef std::multimap<std::string,
                          std::pair<std::string,
                                    std::vector<std::pair<double, double> > > > DependencyArcsMap;

    class LayoutSurface : public GG::Wnd {
    public:
        LayoutSurface() :
            Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE | GG::DRAGABLE)
        {}
        virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
        { DraggedSignal(move); }
        virtual void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
        { ButtonDownSignal(pt); }
        virtual void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
        { ButtonUpSignal(pt); }
        virtual void MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
        { ZoomedSignal(move); }
        mutable boost::signal<void (int)>           ZoomedSignal;
        mutable boost::signal<void (const GG::Pt&)> DraggedSignal;
        mutable boost::signal<void (const GG::Pt&)> ButtonDownSignal;
        mutable boost::signal<void (const GG::Pt&)> ButtonUpSignal;
    };

    void Layout(bool keep_position);
    bool TechVisible(const std::string& tech_name);
    void DrawArc(DependencyArcsMap::const_iterator it, const GG::Clr& color, bool with_arrow_head);
    void ScrolledSlot(int, int, int, int);

    void TechBrowsedSlot(const std::string& tech_name);
    void TechClickedSlot(const std::string& tech_name,
                         const GG::Flags<GG::ModKey>& modkeys);
    void TechDoubleClickedSlot(const std::string& tech_name,
                               const GG::Flags<GG::ModKey>& modkeys);

    void TreeDraggedSlot(const GG::Pt& move);
    void TreeDragBegin(const GG::Pt& move);
    void TreeDragEnd(const GG::Pt& move);
    void TreeZoomedSlot(int move);
    void TreeZoomInClicked();
    void TreeZoomOutClicked();

    double                  m_scale;
    std::set<std::string>   m_categories_shown;
    std::set<TechStatus>    m_tech_statuses_shown;
    std::string             m_selected_tech_name;
    std::string             m_browsed_tech_name;
    TechTreeLayout          m_graph;

    std::map<std::string, TechPanel*>   m_techs;
    DependencyArcsMap                   m_dependency_arcs;

    LayoutSurface* m_layout_surface;
    CUIScroll*     m_vscroll;
    CUIScroll*     m_hscroll;
    double         m_scroll_position_x;     //actual scroll position
    double         m_scroll_position_y;
    double         m_drag_scroll_position_x;//position when drag started
    double         m_drag_scroll_position_y;
    CUIButton*     m_zoom_in_button;
    CUIButton*     m_zoom_out_button;
};


//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel::TechPanel          //
//////////////////////////////////////////////////
/** Represents a single tech in the LayoutPanel. */
class TechTreeWnd::LayoutPanel::TechPanel : public GG::Wnd {
public:
    TechPanel(const std::string& tech_name, const TechTreeWnd::LayoutPanel* panel);

    virtual bool    InWindow(const GG::Pt& pt) const;
    virtual void    Render();
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
    { ForwardEventToParent(); }
    virtual void    LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
    { ForwardEventToParent(); }
    virtual void    LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
    { ForwardEventToParent(); }
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();
    virtual void    MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
    { ForwardEventToParent(); }
    void            Update();
    void            Select(bool select);
    int             FontSize() const;

    mutable TechTreeWnd::TechSignalType         TechBrowsedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechClickedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechDoubleClickedSignal;

private:
    const std::string&              m_tech_name;
    const TechTreeWnd::LayoutPanel* m_layout_panel;
    GG::StaticGraphic*              m_icon;
    GG::TextControl*                m_tech_name_text;
    GG::Clr                         m_colour;
    TechStatus                      m_status;
    bool                            m_browse_highlight;
    bool                            m_selected;
};

TechTreeWnd::LayoutPanel::TechPanel::TechPanel(const std::string& tech_name, const LayoutPanel* panel) :
    GG::Wnd(GG::X0, GG::Y0, TECH_PANEL_WIDTH, TECH_PANEL_HEIGHT, GG::INTERACTIVE),
    m_tech_name(tech_name),
    m_layout_panel(panel),
    m_icon(0),
    m_tech_name_text(0),
    m_colour(GG::CLR_GRAY),
    m_status(TS_RESEARCHABLE),
    m_browse_highlight(false),
    m_selected(false)
{
    const Tech* tech = GetTech(m_tech_name);
    boost::shared_ptr<GG::Font> font = ClientUI::GetFont(FontSize());

    //REMARK: do not use AttachChild but add child->Render() to method render,
    //        as the component is zoomed tech icon
    const int GRAPHIC_SIZE = Value(TECH_PANEL_HEIGHT);
    m_icon = new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE),
                                   ClientUI::TechIcon(m_tech_name),
                                   GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);

    // tech name text
    const int PAD = 8;
    GG::X text_left(GG::X(GRAPHIC_SIZE) + PAD);
    GG::Y text_top(PAD/2);
    GG::X text_width(TECH_PANEL_WIDTH - text_left);
    GG::Y text_height(TECH_PANEL_HEIGHT - PAD);
    m_tech_name_text = new ShadowedTextControl(text_left, text_top, text_width, text_height,
                                               "", font, ClientUI::TextColor(),
                                               GG::FORMAT_WORDBREAK | GG::FORMAT_VCENTER | GG::FORMAT_LEFT);

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    Update();
}

int TechTreeWnd::LayoutPanel::TechPanel::FontSize() const
{ return ClientUI::Pts() * 3 / 2; }

bool TechTreeWnd::LayoutPanel::TechPanel::InWindow(const GG::Pt& pt) const {
    const GG::Pt p = m_layout_panel->Convert(pt) - UpperLeft();
    //return GG::Wnd::InWindow(p - UpperLeft());
    const int PAD = 8;
    return m_icon->InWindow(p) || m_tech_name_text->InWindow(p + GG::Pt(GG::X(PAD), GG::Y0));   // shift right so clicking in gap between icon and text doesn't miss the panel
}

void TechTreeWnd::LayoutPanel::TechPanel::Render() {
    const int PAD = 8;

    m_layout_panel->DoZoom(UpperLeft());

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0);

    GG::Pt ul(m_icon->Width() + PAD/2, GG::Y0);
    GG::Pt lr(Size());

    glColor(GG::CLR_BLACK);
    PartlyRoundedRect(ul, lr, PAD, true, true, true, true, true);

    glColor(m_colour);
    PartlyRoundedRect(ul, lr, PAD, true, true, true, true, true);

    if (m_browse_highlight) {
        // white border
        glColor(GG::CLR_WHITE);
        PartlyRoundedRect(ul, lr, PAD, true, true, true, true, false);
    } else if (m_status == TS_RESEARCHABLE) {
        // coloured border
        GG::Clr clr = m_colour;
        clr.a = 255;
        glColor(clr);
        PartlyRoundedRect(ul, lr, PAD, true, true, true, true, false);
    }

    if (m_selected) {
        // enclosing larger border / box
        glColor(GG::CLR_WHITE);
        GG::Pt gap = GG::Pt(GG::X(PAD), GG::Y(PAD));
        GG::Pt enc_ul(-gap);
        GG::Pt enc_lr(lr + gap);
        PartlyRoundedRect(enc_ul, enc_lr, PAD + 2, true, true, true, true, false);
    }

    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_TEXTURE_2D);

    m_icon->Render();

    if (FontSize() * m_layout_panel->Scale() > 10)  // in my tests, smaller fonts appear garbled / pixilated due to rescaling for zooming
        m_tech_name_text->Render();

    m_layout_panel->UndoZoom();
}

void TechTreeWnd::LayoutPanel::TechPanel::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (m_layout_panel->m_selected_tech_name != m_tech_name)
        TechClickedSignal(m_tech_name, mod_keys);
}

void TechTreeWnd::LayoutPanel::TechPanel::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ TechDoubleClickedSignal(m_tech_name, mod_keys); }

void TechTreeWnd::LayoutPanel::TechPanel::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    TechBrowsedSignal(m_tech_name);
    m_browse_highlight = true;
}

void TechTreeWnd::LayoutPanel::TechPanel::MouseLeave() {
    TechBrowsedSignal("");
    m_browse_highlight = false;
}

void TechTreeWnd::LayoutPanel::TechPanel::Select(bool select)
{ m_selected = select; }

namespace {
    boost::shared_ptr<GG::BrowseInfoWnd> TechPanelRowBrowseWnd(const std::string& tech_name,
                                                               int empire_id)
    {
        const Empire* empire = Empires().Lookup(empire_id);
        const Tech* tech = GetTech(tech_name);

        std::string main_text;

        if (tech) {
            int turns = tech->ResearchTime(empire_id);
            double cost = tech->ResearchCost(empire_id);
            const std::string& cost_units = UserString("ENC_RP");

            main_text += boost::io::str(FlexibleFormat(UserString("ENC_COST_AND_TURNS_STR"))
                % DoubleToString(cost, 3, false)
                % cost_units
                % turns)
                + "\n";

            // TODO: Receiving: 25/82 RP/turn
            // TODO: Time to Completion: 92 turns

            main_text += UserString(tech->Category()) + " ";
            main_text += UserString(boost::lexical_cast<std::string>(tech->Type())) + "  :  ";
            main_text += UserString(tech->ShortDescription()) + "\n";
        }

        if (empire) {
            TechStatus tech_status = empire->GetTechStatus(tech_name);
            if (tech_status == TS_UNRESEARCHABLE) {
                main_text += UserString("TECH_WND_STATUS_UNRESEARCHABLE") + "\n";
                if (tech) {
                    const std::set<std::string>& prereqs = tech->Prerequisites();
                    std::vector<std::string> unresearched_prereqs;
                    for (std::set<std::string>::const_iterator it = prereqs.begin(); it != prereqs.end(); ++it) {
                        TechStatus prereq_status = empire->GetTechStatus(*it);
                        if (prereq_status != TS_COMPLETE)
                            unresearched_prereqs.push_back(*it);
                    }
                    if (!unresearched_prereqs.empty()) {
                        main_text += UserString("TECH_WND_UNRESEARCHED_PREREQUISITES");
                        for (std::vector<std::string>::const_iterator it = unresearched_prereqs.begin();
                             it != unresearched_prereqs.end(); ++it)
                        { main_text += UserString(*it) + "  "; }
                        main_text += "\n";
                    }
                }
            } else if (tech_status == TS_RESEARCHABLE) {
                main_text += UserString("TECH_WND_STATUS_RESEARCHABLE") + "\n";
            } else if (tech_status == TS_COMPLETE) {
                main_text += UserString("TECH_WND_STATUS_COMPLETED") + "\n";
            }

            // TODO: On Queue At Position:
            // TODO: Researching at 235 RP/turn
        }

        boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(
            ClientUI::TechIcon(tech_name), UserString(tech_name), main_text));
        return browse_wnd;
    }
}

void TechTreeWnd::LayoutPanel::TechPanel::Update() {
    Select(m_layout_panel->m_selected_tech_name == m_tech_name);

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    if (const Empire* empire = Empires().Lookup(client_empire_id)) {
        m_status = empire->GetTechStatus(m_tech_name);
    }

    GG::Clr icon_colour = GG::CLR_WHITE;
    if (const Tech* tech = GetTech(m_tech_name)) {
        m_colour = ClientUI::CategoryColor(tech->Category());
        icon_colour = m_colour;

        if (m_status == TS_UNRESEARCHABLE) {
            icon_colour = GG::CLR_GRAY;
            m_colour.a = 64;
        } else if (m_status == TS_RESEARCHABLE) {
            icon_colour = GG::CLR_GRAY;
            m_colour.a = 144;
        } else {
            m_colour.a = 255;
        }
    }
    m_icon->SetColor(icon_colour);

    m_tech_name_text->SetText(UserString(m_tech_name));

    ClearBrowseInfoWnd();
    SetBrowseInfoWnd(TechPanelRowBrowseWnd(m_tech_name, client_empire_id));
}


//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel                     //
//////////////////////////////////////////////////
TechTreeWnd::LayoutPanel::LayoutPanel(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_scale(INITIAL_SCALE),
    m_categories_shown(),
    m_tech_statuses_shown(),
    m_selected_tech_name(),
    m_browsed_tech_name(),
    m_graph(),
    m_layout_surface(0),
    m_vscroll(0),
    m_hscroll(0),
    m_zoom_in_button(0),
    m_zoom_out_button(0)
{
    SetChildClippingMode(ClipToClient);

    m_scale = 1.0; //(LATHANDA) Initialise Fullzoom and do real zooming using GL. TODO: Check best size

    m_layout_surface = new LayoutSurface();
    m_vscroll = new CUIScroll(w - ClientUI::ScrollWidth(), GG::Y0, GG::X(ClientUI::ScrollWidth()),
                              h - ClientUI::ScrollWidth(), GG::VERTICAL);
    m_hscroll = new CUIScroll(GG::X0, h - ClientUI::ScrollWidth(), w - ClientUI::ScrollWidth(),
                              GG::Y(ClientUI::ScrollWidth()), GG::HORIZONTAL);

    const GG::X ZBSIZE(ClientUI::ScrollWidth() * 2);
    const int ZBOFFSET = ClientUI::ScrollWidth() / 2;
    const GG::Y TOP = UpperLeft().y;
    const GG::X LEFT = UpperLeft().x;

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

    m_zoom_in_button = new CUIButton(w - ZBSIZE - ZBOFFSET - ClientUI::ScrollWidth(), GG::Y(ZBOFFSET),
                                     ZBSIZE, "+", font, ClientUI::WndColor(), ClientUI::CtrlBorderColor(),
                                     1, ClientUI::TextColor(), GG::INTERACTIVE | GG::ONTOP);

    m_zoom_out_button = new CUIButton(m_zoom_in_button->UpperLeft().x - LEFT,
                                      m_zoom_in_button->LowerRight().y + ZBOFFSET - TOP,
                                      ZBSIZE, "-", font, ClientUI::WndColor(),
                                      ClientUI::CtrlBorderColor(), 1, ClientUI::TextColor(),
                                      GG::INTERACTIVE | GG::ONTOP);

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
    //m_tech_statuses_shown.insert(TS_UNRESEARCHABLE);
    m_tech_statuses_shown.insert(TS_RESEARCHABLE);
    m_tech_statuses_shown.insert(TS_COMPLETE);
}

GG::Pt TechTreeWnd::LayoutPanel::ClientLowerRight() const
{ return LowerRight() - GG::Pt(GG::X(ClientUI::ScrollWidth()), GG::Y(ClientUI::ScrollWidth())); }

std::set<std::string> TechTreeWnd::LayoutPanel::GetCategoriesShown() const
{ return m_categories_shown; }

double TechTreeWnd::LayoutPanel::Scale() const
{ return m_scale; }

std::set<TechStatus> TechTreeWnd::LayoutPanel::GetTechStatusesShown() const
{ return m_tech_statuses_shown; }

void TechTreeWnd::LayoutPanel::Render() {
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
    glEnable(GL_LINE_SMOOTH);


    // render dependency arcs
    DoZoom(GG::Pt());

    // draw arcs with thick, half-alpha line
    GG::Clr arc_colour = GG::CLR_GRAY;              arc_colour.a = 127;
    glColor(arc_colour);
    glLineWidth(ARC_THICKNESS * m_scale);
    for (DependencyArcsMap::const_iterator arc_it = m_dependency_arcs.begin();
         arc_it != m_dependency_arcs.end(); ++arc_it)
    { DrawArc(arc_it, arc_colour, false); }

    // redraw thicker highlight arcs
    GG::Clr arc_highlight_colour = GG::CLR_WHITE;   arc_highlight_colour.a = 127;
    glColor(arc_highlight_colour);
    glLineWidth(ARC_THICKNESS * m_scale * 2.0);
    for (DependencyArcsMap::const_iterator arc_it = m_dependency_arcs.begin();
         arc_it != m_dependency_arcs.end(); ++arc_it)
    {
        if (arc_it->first == m_selected_tech_name ||
            arc_it->first == m_browsed_tech_name ||
            arc_it->second.first == m_selected_tech_name ||
            arc_it->second.first == m_browsed_tech_name)
        { DrawArc(arc_it, arc_colour, false); }
    }

    // retrace arcs with thinner full-alpha line
    arc_colour.a = 255;
    glColor(arc_colour);
    glLineWidth(ARC_THICKNESS * m_scale * 0.5);
    for (DependencyArcsMap::const_iterator arc_it = m_dependency_arcs.begin();
         arc_it != m_dependency_arcs.end(); ++arc_it)
    { DrawArc(arc_it, arc_colour, false); }

    // redraw thicker highlight arcs
    arc_highlight_colour.a = 255;
    glColor(arc_highlight_colour);
    glLineWidth(ARC_THICKNESS * m_scale);
    for (DependencyArcsMap::const_iterator arc_it = m_dependency_arcs.begin();
         arc_it != m_dependency_arcs.end(); ++arc_it)
    {
        if (arc_it->first == m_selected_tech_name ||
            arc_it->first == m_browsed_tech_name ||
            arc_it->second.first == m_selected_tech_name ||
            arc_it->second.first == m_browsed_tech_name)
        { DrawArc(arc_it, arc_colour, false); }
    }

    glEnable(GL_TEXTURE_2D);
    EndClipping();

    glLineWidth(1.0);
    UndoZoom();
    GG::GUI::RenderWindow(m_vscroll);
    GG::GUI::RenderWindow(m_hscroll);
}

void TechTreeWnd::LayoutPanel::Update()
{ Layout(true); }

void TechTreeWnd::LayoutPanel::Clear() {
    m_vscroll->ScrollTo(0);
    m_hscroll->ScrollTo(0);
    m_vscroll->SizeScroll(0, 1, 1, 1);
    m_hscroll->SizeScroll(0, 1, 1, 1);
    GG::SignalScroll(*m_vscroll, true);
    GG::SignalScroll(*m_hscroll, true);

    // delete all panels
    for (std::map<std::string, TechPanel*>::const_iterator it = m_techs.begin(); it != m_techs.end(); ++it)
        delete it->second;
    m_techs.clear();
    m_graph.Clear();

    m_dependency_arcs.clear();

    m_selected_tech_name.clear();
}

void TechTreeWnd::LayoutPanel::Reset() {
    // regenerate graph of panels and dependency lines
    Layout(false);
}

void TechTreeWnd::LayoutPanel::SetScale(double scale) {
    if (scale < MIN_SCALE)
        scale = MIN_SCALE;
    if (MAX_SCALE < scale)
        scale = MAX_SCALE;
    if (m_scale != scale)
        m_scale = scale;
}

void TechTreeWnd::LayoutPanel::ShowCategory(const std::string& category) {
    if (m_categories_shown.find(category) == m_categories_shown.end()) {
        m_categories_shown.insert(category);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::ShowAllCategories() {
    const std::vector<std::string> all_cats = GetTechManager().CategoryNames();
    if (all_cats.size() == m_categories_shown.size())
        return;
    for (std::vector<std::string>::const_iterator it = all_cats.begin(); it != all_cats.end(); ++it)
        m_categories_shown.insert(*it);
    Layout(true);
}

void TechTreeWnd::LayoutPanel::HideCategory(const std::string& category) {
    std::set<std::string>::iterator it = m_categories_shown.find(category);
    if (it != m_categories_shown.end()) {
        m_categories_shown.erase(it);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::HideAllCategories() {
    if (m_categories_shown.empty())
        return;
    m_categories_shown.clear();
    Layout(true);
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

void TechTreeWnd::LayoutPanel::ShowTech(const std::string& tech_name)
{ TechClickedSlot(tech_name, GG::Flags<GG::ModKey>()); }

void TechTreeWnd::LayoutPanel::CenterOnTech(const std::string& tech_name) {
    std::map<std::string, TechPanel*>::const_iterator it = m_techs.find(tech_name);
    if (it == m_techs.end()) {
        Logger().debugStream() << "TechTreeWnd::LayoutPanel::CenterOnTech couldn't centre on " << tech_name
                               << " due to lack of such a tech panel";
        return;
    }

    TechPanel* tech_panel = it->second;
    GG::Pt center_point = tech_panel->UpperLeft();
    m_hscroll->ScrollTo(Value(center_point.x));
    GG::SignalScroll(*m_hscroll, true);
    m_vscroll->ScrollTo(Value(center_point.y));
    GG::SignalScroll(*m_vscroll, true);
}

void TechTreeWnd::LayoutPanel::DoZoom(const GG::Pt& p) const {
    glPushMatrix();
    //center to panel
    glTranslated(Value(Width()/2.0), Value(Height()/2.0), 0);
    //zoom
    glScaled(m_scale, m_scale, 1);
    //translate to actual scroll position
    glTranslated(-m_scroll_position_x, -m_scroll_position_y, 0);
    glTranslated(Value(p.x), Value(p.y), 0);
}

void TechTreeWnd::LayoutPanel::UndoZoom() const
{ glPopMatrix(); }

GG::Pt TechTreeWnd::LayoutPanel::Convert(const GG::Pt & p) const {
    // Converts screen coordinate into virtual coordiante
    // doing the inverse transformation as DoZoom in the same order
    double x = Value(p.x);
    double y = Value(p.y);
    x -= Value(Width()/2.0);
    y -= Value(Height()/2.0);
    x /= m_scale;
    y /= m_scale;
    x += m_scroll_position_x;
    y += m_scroll_position_y;
    return GG::Pt(GG::X(static_cast<int>(x)), GG::Y(static_cast<int>(y)));
}

void TechTreeWnd::LayoutPanel::Layout(bool keep_position) {
    const GG::X TECH_PANEL_MARGIN_X(ClientUI::Pts()*16);
    const GG::Y TECH_PANEL_MARGIN_Y(ClientUI::Pts()*16 + 100);
    const double RANK_SEP = Value(TECH_PANEL_WIDTH) * GetOptionsDB().Get<double>("UI.tech-layout-horz-spacing");
    const double NODE_SEP = Value(TECH_PANEL_HEIGHT) * GetOptionsDB().Get<double>("UI.tech-layout-vert-spacing");
    const double WIDTH = Value(TECH_PANEL_WIDTH);
    const double HEIGHT = Value(TECH_PANEL_HEIGHT);
    const double X_MARGIN(12);

    // view state initial data
    int initial_hscroll_pos = m_hscroll->PosnRange().first;
    int initial_vscroll_pos = m_vscroll->PosnRange().first;
    double initial_hscroll_page_size = m_hscroll->PageSize();
    double initial_vscroll_page_size = m_vscroll->PageSize();
    const std::string selected_tech = m_selected_tech_name;

    // cleanup old data for new layout
    Clear();

    Logger().debugStream() << "Tech Tree Layout Preparing Tech Data";

    // create a node for every tech
    TechManager& manager = GetTechManager();
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        const Tech* tech = *it;
        if (!tech) continue;
        const std::string& tech_name = tech->Name();
        if (!TechVisible(tech_name)) continue;
        m_techs[tech_name] = new TechPanel(tech_name, this);
        m_graph.AddNode(tech_name, m_techs[tech_name]->Width(), m_techs[tech_name]->Height());
    }

    // create an edge for every prerequisite
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        const Tech* tech = *it;
        if (!tech) continue;
        const std::string& tech_name = tech->Name();
        if (!TechVisible(tech_name)) continue;
        for (std::set<std::string>::const_iterator prereq_it = tech->Prerequisites().begin();
             prereq_it != tech->Prerequisites().end(); ++prereq_it)
        {
            if (!TechVisible(*prereq_it)) continue;
            m_graph.AddEdge(*prereq_it, tech_name);
        }
    }

    Logger().debugStream() << "Tech Tree Layout Doing Graph Layout";

    //calculate layout
    m_graph.DoLayout(static_cast<int>(WIDTH + RANK_SEP),
                     static_cast<int>(HEIGHT + NODE_SEP),
                     static_cast<int>(X_MARGIN));

    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());

    Logger().debugStream() << "Tech Tree Layout Creating Panels";

    // create new tech panels and new dependency arcs 
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        const Tech* tech = *it;
        if (!tech) continue;
        const std::string& tech_name = tech->Name();
        if (!TechVisible(tech_name)) continue;
        //techpanel
        const TechTreeLayout::Node* node = m_graph.GetNode(tech_name);
        //move TechPanel
        TechPanel* tech_panel = m_techs[tech_name];
        tech_panel->MoveTo(GG::Pt(node->GetX(), node->GetY()));
        m_layout_surface->AttachChild(tech_panel);
        GG::Connect(tech_panel->TechBrowsedSignal,          &TechTreeWnd::LayoutPanel::TechBrowsedSlot,         this);
        GG::Connect(tech_panel->TechClickedSignal,          &TechTreeWnd::LayoutPanel::TechClickedSlot,         this);
        GG::Connect(tech_panel->TechDoubleClickedSignal,    &TechTreeWnd::LayoutPanel::TechDoubleClickedSlot,   this);

        const std::vector<TechTreeLayout::Edge*> edges = m_graph.GetOutEdges(tech_name);
        //prerequisite edge
        for (std::vector<TechTreeLayout::Edge*>::const_iterator edge = edges.begin();
             edge != edges.end(); edge++)
        {
            std::vector<std::pair<double, double> > points;
            const std::string& from = (*edge)->GetTechFrom();
            const std::string& to   = (*edge)->GetTechTo();
            assert(GetTech(from) && GetTech(to));
            (*edge)->ReadPoints(points);
            TechStatus arc_type = TS_RESEARCHABLE;
            if (empire)
                arc_type = empire->GetTechStatus(to);
            m_dependency_arcs.insert(std::make_pair(from, std::make_pair(to, points)));
        }
    }
    // format window
    GG::Pt client_sz = ClientSize();
    GG::Pt layout_size(client_sz.x + m_graph.GetWidth(), client_sz.y + m_graph.GetHeight());
    m_layout_surface->Resize(layout_size);
    // format scrollbar
    m_vscroll->SizeScroll(0, Value(layout_size.y - 1), std::max(50, Value(std::min(layout_size.y / 10, client_sz.y))), Value(client_sz.y));
    m_hscroll->SizeScroll(0, Value(layout_size.x - 1), std::max(50, Value(std::min(layout_size.x / 10, client_sz.x))), Value(client_sz.x));

    Logger().debugStream() << "Tech Tree Layout Done";

    // restore save data
    if (keep_position) {
        m_selected_tech_name = selected_tech;
        // select clicked on tech
        if (m_techs.find(m_selected_tech_name) != m_techs.end())
            m_techs[m_selected_tech_name]->Select(true);
        double hscroll_page_size_ratio = m_hscroll->PageSize() / initial_hscroll_page_size;
        double vscroll_page_size_ratio = m_vscroll->PageSize() / initial_vscroll_page_size;
        m_hscroll->ScrollTo(static_cast<int>(initial_hscroll_pos * hscroll_page_size_ratio));
        m_vscroll->ScrollTo(static_cast<int>(initial_vscroll_pos * vscroll_page_size_ratio));
        GG::SignalScroll(*m_hscroll, true);
        GG::SignalScroll(*m_vscroll, true);
    } else {
        m_selected_tech_name.clear();
        // find a tech to centre view on
        for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
            const Tech* tech = *it;
            const std::string& tech_name = tech->Name();
            if (TechVisible(tech_name)) {
                CenterOnTech(tech_name);
                break;
            }
        }
    }

    // ensure that the scrolls stay on top
    MoveChildUp(m_vscroll);
    MoveChildUp(m_hscroll);
}

bool TechTreeWnd::LayoutPanel::TechVisible(const std::string& tech_name) {
    const Tech* tech = GetTech(tech_name);
    if (!tech)
        return false;

    // Unresearchable techs are never to be shown on tree
    if (!tech->Researchable())
        return false;

    // check that category is visible
    if (m_categories_shown.find(tech->Category()) == m_categories_shown.end())
        return false;

    // check tech status
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return true;    // if no empire, techs have no status, so just return true
    if (m_tech_statuses_shown.find(empire->GetTechStatus(tech->Name())) == m_tech_statuses_shown.end())
        return false;

    // all tests pass, so tech is visible
    return true;
}

void TechTreeWnd::LayoutPanel::DrawArc(DependencyArcsMap::const_iterator it,
                                       const GG::Clr& color, bool with_arrow_head)
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

void TechTreeWnd::LayoutPanel::ScrolledSlot(int, int, int, int) {
    m_scroll_position_x = m_hscroll->PosnRange().first;
    m_scroll_position_y = m_vscroll->PosnRange().first;
}

void TechTreeWnd::LayoutPanel::TechBrowsedSlot(const std::string& tech_name)
{ TechBrowsedSignal(tech_name); }

void TechTreeWnd::LayoutPanel::TechClickedSlot(const std::string& tech_name,
                                               const GG::Flags<GG::ModKey>& modkeys) {
    // deselect previously-selected tech panel
    if (m_techs.find(m_selected_tech_name) != m_techs.end())
        m_techs[m_selected_tech_name]->Select(false);
    // select clicked on tech
    if (m_techs.find(tech_name) != m_techs.end())
        m_techs[tech_name]->Select(true);
    m_selected_tech_name = tech_name;
    TechClickedSignal(tech_name, modkeys);
}

void TechTreeWnd::LayoutPanel::TechDoubleClickedSlot(const std::string& tech_name,
                                                     const GG::Flags<GG::ModKey>& modkeys)
{ TechDoubleClickedSignal(tech_name, modkeys); }

void TechTreeWnd::LayoutPanel::TreeDraggedSlot(const GG::Pt& move) {
    m_hscroll->ScrollTo(m_drag_scroll_position_x - Value(move.x / m_scale));
    m_vscroll->ScrollTo(m_drag_scroll_position_y - Value(move.y / m_scale));
    m_scroll_position_x = m_hscroll->PosnRange().first;
    m_scroll_position_y = m_vscroll->PosnRange().first;
}

void TechTreeWnd::LayoutPanel::TreeDragBegin(const GG::Pt& pt) {
    m_drag_scroll_position_x = m_scroll_position_x;
    m_drag_scroll_position_y = m_scroll_position_y;
}

void TechTreeWnd::LayoutPanel::TreeDragEnd(const GG::Pt& pt) {
    m_drag_scroll_position_x = m_scroll_position_x;
    m_drag_scroll_position_y = m_scroll_position_y;
}

void TechTreeWnd::LayoutPanel::TreeZoomedSlot(int move) {
    if (0 < move)
        SetScale(m_scale * ZOOM_STEP_SIZE);
    else if (move < 0)
        SetScale(m_scale / ZOOM_STEP_SIZE);
    //std::cout << m_scale << std::endl;
}

void TechTreeWnd::LayoutPanel::TreeZoomInClicked()
{ TreeZoomedSlot(1); }

void TechTreeWnd::LayoutPanel::TreeZoomOutClicked()
{ TreeZoomedSlot(-1); }


//////////////////////////////////////////////////
// TechTreeWnd::TechListBox                     //
//////////////////////////////////////////////////
class TechTreeWnd::TechListBox : public CUIListBox {
public:
    /** \name Structors */ //@{
    TechListBox(GG::X x, GG::Y y, GG::X w, GG::Y h);
    //@}

    /** \name Accessors */ //@{
    std::set<std::string>   GetCategoriesShown() const;
    std::set<TechStatus>    GetTechStatusesShown() const;
    //@}

    //! \name Mutators //@{
    void    Reset();

    void    ShowCategory(const std::string& category);
    void    ShowAllCategories();
    void    HideCategory(const std::string& category);
    void    HideAllCategories();
    void    ShowStatus(TechStatus status);
    void    HideStatus(TechStatus status);

    bool    TechVisible(const std::string& tech_name);
    //@}

    mutable TechSignalType      TechBrowsedSignal;      ///< emitted when the mouse rolls over a technology
    mutable TechClickSignalType TechClickedSignal;      ///< emitted when a technology is single-clicked
    mutable TechClickSignalType TechDoubleClickedSignal;///< emitted when a technology is double-clicked

private:
    class TechRow : public CUIListBox::Row {
    public:
        TechRow(GG::X w, const std::string& tech_name);
        const std::string&          GetTech() { return m_tech; }
        virtual void                Render();
        static std::vector<GG::X>   ColWidths(GG::X total_width);

    private:
        std::string m_tech;
    };

    void    Populate();
    void    PropagateDoubleClickSignal(GG::ListBox::iterator it);
    void    PropagateLeftClickSignal(GG::ListBox::iterator it, const GG::Pt& pt);

    std::set<std::string>                   m_categories_shown;
    std::set<TechStatus>                    m_tech_statuses_shown;
    std::multimap<std::string, TechRow*>    m_all_tech_rows;
};

void TechTreeWnd::TechListBox::TechRow::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_WHITE, 1);
}

std::vector<GG::X> TechTreeWnd::TechListBox::TechRow::ColWidths(GG::X total_width) {
    const GG::X GRAPHIC_WIDTH(ClientUI::Pts() * 2);
    const GG::X NAME_WIDTH(ClientUI::Pts() * 18);
    const GG::X COST_WIDTH(ClientUI::Pts() * 4);
    const GG::X TIME_WIDTH(ClientUI::Pts() * 4);
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

TechTreeWnd::TechListBox::TechRow::TechRow(GG::X w, const std::string& tech_name) :
    CUIListBox::Row(w, GG::Y(ClientUI::Pts() * 2 + 5), "TechListBox::TechRow"),
    m_tech(tech_name)
{
    const Tech* this_row_tech = ::GetTech(m_tech);
    if (!this_row_tech)
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

    GG::StaticGraphic* graphic = new GG::StaticGraphic(GG::X0, GG::Y0, GRAPHIC_WIDTH, HEIGHT,
                                                       ClientUI::TechIcon(m_tech),
                                                       GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
    graphic->SetColor(ClientUI::CategoryColor(this_row_tech->Category()));
    push_back(graphic);

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();

    GG::TextControl* text = new GG::TextControl(GG::X0, GG::Y0, NAME_WIDTH, HEIGHT, UserString(m_tech),
                                                font, ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    text->ClipText(true);
    push_back(text);

    std::string cost_str = boost::lexical_cast<std::string>(static_cast<int>(this_row_tech->ResearchCost(HumanClientApp::GetApp()->EmpireID()) + 0.5));
    text = new GG::TextControl(GG::X0, GG::Y0, COST_WIDTH, HEIGHT, cost_str, font, ClientUI::TextColor(),
                               GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);

    std::string time_str = boost::lexical_cast<std::string>(this_row_tech->ResearchTime(HumanClientApp::GetApp()->EmpireID()));
    text = new GG::TextControl(GG::X0, GG::Y0, TIME_WIDTH, HEIGHT, time_str, font, ClientUI::TextColor(),
                               GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);

    std::string category_str = UserString(this_row_tech->Category());
    text = new GG::TextControl(GG::X0, GG::Y0, CATEGORY_WIDTH, HEIGHT, category_str, font, ClientUI::TextColor(),
                               GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);

    std::string type_str = UserString(boost::lexical_cast<std::string>(this_row_tech->Type()));
    text = new GG::TextControl(GG::X0, GG::Y0, TYPE_WIDTH, HEIGHT, type_str, font, ClientUI::TextColor(),
                               GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);

    std::string desc_str = UserString(this_row_tech->ShortDescription());
    text = new GG::TextControl(GG::X0, GG::Y0, DESC_WIDTH, HEIGHT, desc_str, font, ClientUI::TextColor(),
                               GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    push_back(text);
}

TechTreeWnd::TechListBox::TechListBox(GG::X x, GG::Y y, GG::X w, GG::Y h) :
    CUIListBox(x, y, w, h)
{
    GG::Connect(DoubleClickedSignal,    &TechListBox::PropagateDoubleClickSignal,   this);
    GG::Connect(LeftClickedSignal,      &TechListBox::PropagateLeftClickSignal,     this);

    SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);

    // show all categories...
    m_categories_shown.clear();
    const std::vector<std::string> categories = GetTechManager().CategoryNames();
    for (std::vector<std::string>::const_iterator it = categories.begin(); it != categories.end(); ++it)
        m_categories_shown.insert(*it);

    // show all statuses except unreasearchable
    m_tech_statuses_shown.clear();
    //m_tech_statuses_shown.insert(TS_UNRESEARCHABLE);
    m_tech_statuses_shown.insert(TS_RESEARCHABLE);
    m_tech_statuses_shown.insert(TS_COMPLETE);

    std::vector<GG::X> col_widths = TechRow::ColWidths(w - ClientUI::ScrollWidth() - 6);
    SetNumCols(col_widths.size());
    LockColWidths();
    for (unsigned int i = 0; i < col_widths.size(); ++i) {
        SetColWidth(i, col_widths[i]);
        SetColAlignment(i, GG::ALIGN_LEFT);
    }
}

std::set<std::string> TechTreeWnd::TechListBox::GetCategoriesShown() const
{ return m_categories_shown; }

std::set<TechStatus> TechTreeWnd::TechListBox::GetTechStatusesShown() const
{ return m_tech_statuses_shown; }

void TechTreeWnd::TechListBox::Reset()
{ Populate(); }

void TechTreeWnd::TechListBox::Populate() {
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
            const std::string& tech_name = UserString(tech->Name());
            creation_timer.restart();
            m_all_tech_rows.insert(std::make_pair(tech_name,
                new TechRow(Width() - ClientUI::ScrollWidth() - 6, tech->Name())));
            creation_elapsed += creation_timer.elapsed();
        }
    }

    // remove techs in listbox, then reset the rest of its state
    for (iterator it = begin(); it != end(); ) {
        iterator temp_it = it++;
        Erase(temp_it);
    }
    Clear();

    for (std::multimap<std::string, TechRow*>::iterator it = m_all_tech_rows.begin();
         it != m_all_tech_rows.end(); ++it)
    {
        TechRow* tech_row = it->second;
        if (TechVisible(tech_row->GetTech())) {
            insertion_timer.restart();
            Insert(tech_row);
            insertion_elapsed += insertion_timer.elapsed();
        }
    }

    Logger().debugStream() << "Tech List Box Done Populating";
    Logger().debugStream() << "    Creation time=" << (creation_elapsed * 1000) << "ms";
    Logger().debugStream() << "    Insertion time=" << (insertion_elapsed * 1000) << "ms";
}

void TechTreeWnd::TechListBox::ShowCategory(const std::string& category) {
    if (m_categories_shown.find(category) == m_categories_shown.end()) {
        m_categories_shown.insert(category);
        Populate();
    }
}

void TechTreeWnd::TechListBox::ShowAllCategories() {
    const std::vector<std::string> all_cats = GetTechManager().CategoryNames();
    if (all_cats.size() == m_categories_shown.size())
        return;
    for (std::vector<std::string>::const_iterator it = all_cats.begin(); it != all_cats.end(); ++it)
        m_categories_shown.insert(*it);
    Populate();
}

void TechTreeWnd::TechListBox::HideCategory(const std::string& category) {
    std::set<std::string>::iterator it = m_categories_shown.find(category);
    if (it != m_categories_shown.end()) {
        m_categories_shown.erase(it);
        Populate();
    }
}

void TechTreeWnd::TechListBox::HideAllCategories() {
    if (m_categories_shown.empty())
        return;
    m_categories_shown.clear();
    Populate();
}

void TechTreeWnd::TechListBox::ShowStatus(TechStatus status) {
    if (m_tech_statuses_shown.find(status) == m_tech_statuses_shown.end()) {
        m_tech_statuses_shown.insert(status);
        Populate();
    }
}

void TechTreeWnd::TechListBox::HideStatus(TechStatus status) {
    std::set<TechStatus>::iterator it = m_tech_statuses_shown.find(status);
    if (it != m_tech_statuses_shown.end()) {
        m_tech_statuses_shown.erase(it);
        Populate();
    }
}

bool TechTreeWnd::TechListBox::TechVisible(const std::string& tech_name) {
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return true;
    const Tech* tech = ::GetTech(tech_name);
    if (!tech)
        return false;

    // check that category and status are visible
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
        TechClickedSignal(tech_row->GetTech(), GG::Flags<GG::ModKey>());
}

void TechTreeWnd::TechListBox::PropagateDoubleClickSignal(GG::ListBox::iterator it) {
    // determine type of row that was clicked, and emit appropriate signal

    TechRow* tech_row = dynamic_cast<TechRow*>(*it);
    if (tech_row)
        TechDoubleClickedSignal(tech_row->GetTech(), GG::Flags<GG::ModKey>());
}


//////////////////////////////////////////////////
// TechTreeWnd                                  //
//////////////////////////////////////////////////
TechTreeWnd::TechTreeWnd(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_enc_detail_panel(0),
    m_layout_panel(0),
    m_tech_list(0)
{
    Sound::TempUISoundDisabler sound_disabler;

    m_layout_panel = new LayoutPanel(w, h);
    GG::Connect(m_layout_panel->TechBrowsedSignal,          &TechTreeWnd::TechBrowsedSlot, this);
    GG::Connect(m_layout_panel->TechClickedSignal,          &TechTreeWnd::TechClickedSlot, this);
    GG::Connect(m_layout_panel->TechDoubleClickedSignal,    &TechTreeWnd::TechDoubleClickedSlot, this);
    AttachChild(m_layout_panel);

    m_tech_list = new TechListBox(GG::X0, GG::Y0, w, h);
    GG::Connect(m_tech_list->TechBrowsedSignal,             &TechTreeWnd::TechBrowsedSlot, this);
    GG::Connect(m_tech_list->TechClickedSignal,             &TechTreeWnd::TechClickedSlot, this);
    GG::Connect(m_tech_list->TechDoubleClickedSignal,       &TechTreeWnd::TechDoubleClickedSlot, this);

    GG::X ENC_WIDTH(480);
    GG::Y ENC_HEIGHT(240);

    m_enc_detail_panel = new EncyclopediaDetailPanel(ENC_WIDTH, ENC_HEIGHT);
    AttachChild(m_enc_detail_panel);

    m_tech_tree_controls = new TechTreeControls(GG::X1, GG::Y1, m_layout_panel->Width() - ClientUI::ScrollWidth());
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
    { GG::Connect(it->second->ClickedSignal, ToggleTechStatusFunctor(this, it->first)); }

    // connect view type selectors
    GG::Connect(m_tech_tree_controls->m_tree_view_button->ClickedSignal, &TechTreeWnd::ShowTreeView, this);
    GG::Connect(m_tech_tree_controls->m_list_view_button->ClickedSignal, &TechTreeWnd::ShowListView, this);

    ShowAllCategories();
    ShowStatus(TS_RESEARCHABLE);
    ShowStatus(TS_COMPLETE);
    // leave unresearchable hidden by default

    ShowTreeView();
}

TechTreeWnd::~TechTreeWnd() {
    delete m_tech_list;
    delete m_layout_panel;
}

double TechTreeWnd::Scale() const
{ return m_layout_panel->Scale(); }

std::set<std::string> TechTreeWnd::GetCategoriesShown() const
{ return m_layout_panel->GetCategoriesShown(); }

std::set<TechStatus> TechTreeWnd::GetTechStatusesShown() const
{ return m_layout_panel->GetTechStatusesShown(); }

void TechTreeWnd::Update()
{ m_layout_panel->Update(); }

void TechTreeWnd::Clear() {
    m_enc_detail_panel->OnIndex();
    m_layout_panel->Clear();
}

void TechTreeWnd::Reset() {
    m_layout_panel->Reset();
    m_tech_list->Reset();
}

void TechTreeWnd::ShowCategory(const std::string& category) {
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

void TechTreeWnd::ShowAllCategories() {
    m_layout_panel->ShowAllCategories();
    m_tech_list->ShowAllCategories();

    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    int i = 0;
    for (std::vector<std::string>::const_iterator cats_it = cats.begin(); cats_it != cats.end(); ++cats_it, ++i) {
        CUIButton* button = m_tech_tree_controls->m_category_buttons[i];
        button->MarkSelectedTechCategoryColor(*cats_it);
    }
}

void TechTreeWnd::HideCategory(const std::string& category) {
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

void TechTreeWnd::HideAllCategories() {
    m_layout_panel->HideAllCategories();
    m_tech_list->HideAllCategories();

    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    int i = 0;
    for (std::vector<std::string>::const_iterator cats_it = cats.begin(); cats_it != cats.end(); ++cats_it, ++i) {
        m_tech_tree_controls->m_category_buttons[i]->MarkNotSelected();
    }
}

void TechTreeWnd::ToggleAllCategories() {
    std::set<std::string> shown_cats = m_layout_panel->GetCategoriesShown();
    const std::vector<std::string> all_cats = GetTechManager().CategoryNames();

    if (shown_cats.size() == all_cats.size())
        HideAllCategories();
    else
        ShowAllCategories();
}

void TechTreeWnd::ToggleCategory(const std::string& category) {
    std::set<std::string> shown_cats = m_layout_panel->GetCategoriesShown();

    std::set<std::string>::const_iterator it = shown_cats.find(category);
    if (it == shown_cats.end())
        ShowCategory(category);
    else
        HideCategory(category);
}

void TechTreeWnd::ShowStatus(TechStatus status) {
    m_layout_panel->ShowStatus(status);
    m_tech_list->ShowStatus(status);

    CUIButton* button = m_tech_tree_controls->m_tech_status_buttons[status];
    button->MarkSelectedGray();
}

void TechTreeWnd::HideStatus(TechStatus status) {
    m_layout_panel->HideStatus(status);
    m_tech_list->HideStatus(status);

    CUIButton* button = m_tech_tree_controls->m_tech_status_buttons[status];
    button->MarkNotSelected();
}

void TechTreeWnd::ToggleStatus(TechStatus status) {
    std::set<TechStatus> statuses = m_layout_panel->GetTechStatusesShown();

    std::set<TechStatus>::const_iterator it = statuses.find(status);
    if (it == statuses.end())
        ShowStatus(status);
    else
        HideStatus(status);
}

void TechTreeWnd::ShowTreeView() {
    AttachChild(m_layout_panel);
    MoveChildDown(m_layout_panel);
    DetachChild(m_tech_list);
    m_tech_tree_controls->m_list_view_button->MarkNotSelected();
    m_tech_tree_controls->m_tree_view_button->MarkSelectedGray();
    MoveChildUp(m_tech_tree_controls);
}

void TechTreeWnd::ShowListView() {
    m_tech_list->Reset();
    AttachChild(m_tech_list);
    MoveChildDown(m_tech_list);
    DetachChild(m_layout_panel);
    m_tech_tree_controls->m_list_view_button->MarkSelectedGray();
    m_tech_tree_controls->m_tree_view_button->MarkNotSelected();
    MoveChildUp(m_tech_tree_controls);
}

void TechTreeWnd::SetScale(double scale)
{ m_layout_panel->SetScale(scale); }

void TechTreeWnd::CenterOnTech(const std::string& tech_name) {
    // ensure tech exists and is visible
    const Tech* tech = ::GetTech(tech_name);
    if (!tech) return;
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    if (empire)
        ShowStatus(empire->GetTechStatus(tech_name));
    ShowCategory(tech->Category());

    // centre on it
    m_layout_panel->CenterOnTech(tech_name);
}

void TechTreeWnd::SetEncyclopediaTech(const std::string& tech_name)
{ m_enc_detail_panel->SetTech(tech_name); }

void TechTreeWnd::SelectTech(const std::string& tech_name)
{ m_layout_panel->ShowTech(tech_name); }

void TechTreeWnd::TechBrowsedSlot(const std::string& tech_name)
{ TechBrowsedSignal(tech_name); }

void TechTreeWnd::TechClickedSlot(const std::string& tech_name,
                                  const GG::Flags<GG::ModKey>& modkeys)
{
    SetEncyclopediaTech(tech_name);
    TechSelectedSignal(tech_name);
}

void TechTreeWnd::TechDoubleClickedSlot(const std::string& tech_name,
                                        const GG::Flags<GG::ModKey>& modkeys)
{
    const Tech* tech = GetTech(tech_name);
    if (!tech) return;
    const Empire* empire = Empires().Lookup(HumanClientApp::GetApp()->EmpireID());
    TechStatus tech_status = TS_UNRESEARCHABLE;
    if (empire)
        tech_status = empire->GetTechStatus(tech_name);

    int queue_pos = -1;
    if (modkeys & GG::MOD_KEY_CTRL)
        queue_pos = 0;

    // if tech can be researched already, just add it
    if (tech_status == TS_RESEARCHABLE) {
        std::vector<std::string> techs;
        techs.push_back(tech_name);
        AddTechsToQueueSignal(techs, queue_pos);
        return;
    }

    if (tech_status != TS_UNRESEARCHABLE)
        return;

    // if tech can't yet be researched, add any prerequisites it requires (recursively) and then add it
    TechManager& manager = GetTechManager();
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    std::vector<std::string> tech_vec = manager.RecursivePrereqs(tech_name, empire_id);
    tech_vec.push_back(tech_name);
    AddTechsToQueueSignal(tech_vec, queue_pos);
}
