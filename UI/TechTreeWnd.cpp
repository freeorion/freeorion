#include "TechTreeWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "CUIWnd.h"
#include "Sound.h"
#include "IconTextBrowseWnd.h"
#include "EncyclopediaDetailPanel.h"
#include "../client/human/HumanClientApp.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../universe/Tech.h"
#include "../universe/Effect.h"
#include "../universe/ValueRef.h"
#include "../Empire/Empire.h"
#include "TechTreeLayout.h"
#include "TechTreeArcs.h"
#include "Hotkeys.h"

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>
#include <GG/GLClientAndServerBuffer.h>

#include <algorithm>

#include <boost/timer.hpp>

namespace {
    const std::string RES_PEDIA_WND_NAME = "research.pedia";
    const std::string RES_CONTROLS_WND_NAME = "research.tech-controls";

    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add("UI.tech-layout-horz-spacing",   UserStringNop("OPTIONS_DB_UI_TECH_LAYOUT_HORZ_SPACING"), 0.25,  RangedStepValidator<double>(0.25, 0.25, 4.0));
        db.Add("UI.tech-layout-vert-spacing",   UserStringNop("OPTIONS_DB_UI_TECH_LAYOUT_VERT_SPACING"), 0.75, RangedStepValidator<double>(0.25, 0.25, 4.0));
        db.Add("UI.tech-layout-zoom-scale",     UserStringNop("OPTIONS_DB_UI_TECH_LAYOUT_ZOOM_SCALE"),   1.0,  RangedStepValidator<double>(1.0, -25.0, 10.0));
        db.Add("UI.tech-controls-graphic-size", UserStringNop("OPTIONS_DB_UI_TECH_CTRL_ICON_SIZE"),      3.0,  RangedStepValidator<double>(0.25, 0.5,  12.0));
        db.Add("UI.windows." + RES_PEDIA_WND_NAME + ".persistently-hidden", UserStringNop("OPTIONS_DB_RESEARCH_PEDIA_HIDDEN"), false, Validator<bool>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    GG::X   TechPanelWidth()
    { return GG::X(ClientUI::Pts()*38); }
    GG::Y   TechPanelHeight()
    { return GG::Y(ClientUI::Pts()*6); }

    const double ZOOM_STEP_SIZE = 1.12;
    const double MIN_SCALE = std::pow(ZOOM_STEP_SIZE, -25.0);
    const double MAX_SCALE = std::pow(ZOOM_STEP_SIZE, 10.0);
    const double INITIAL_SCALE = std::pow(ZOOM_STEP_SIZE, -5.0);

    const double PI = 3.1415926535897932384626433;

    bool    TechVisible(const std::string& tech_name,
                        const std::set<std::string>& categories_shown,
                        const std::set<TechStatus>& statuses_shown)
    {
        const Tech* tech = GetTech(tech_name);
        if (!tech)
            return false;

        // Unresearchable techs are never to be shown on tree
        if (!tech->Researchable())
            return false;

        // check that category is visible
        if (categories_shown.find(tech->Category()) == categories_shown.end())
            return false;

        // check tech status
        const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
        if (!empire)
            return true;    // if no empire, techs have no status, so just return true
        if (statuses_shown.find(empire->GetTechStatus(tech_name)) == statuses_shown.end())
            return false;

        // all tests pass, so tech is visible
        return true;
    }


    struct SetCategoryViewFunctor {
        SetCategoryViewFunctor(TechTreeWnd* tree_wnd, const std::string& category) :
            m_tree_wnd(tree_wnd),
            m_category(category)
        {}
        void operator()(bool b) {
            if (m_tree_wnd)
                b ? m_tree_wnd->ShowCategory(m_category) : m_tree_wnd->HideCategory(m_category);
        }
        TechTreeWnd* const m_tree_wnd;
        const std::string m_category;
    };

    struct SetAllCategoryViewsFunctor {
        SetAllCategoryViewsFunctor(TechTreeWnd* tree_wnd) :
            m_tree_wnd(tree_wnd)
        {}
        void operator()(bool b) {
            if (m_tree_wnd)
                b ? m_tree_wnd->ShowAllCategories() : m_tree_wnd->HideAllCategories();
        }
        TechTreeWnd* const m_tree_wnd;
    };

    struct SetTechStatusViewFunctor {
        SetTechStatusViewFunctor(TechTreeWnd* tree_wnd, TechStatus status) :
            m_tree_wnd(tree_wnd),
            m_status(status)
        {}
        void operator()(bool b) {
            if (m_tree_wnd)
                b ? m_tree_wnd->ShowStatus(m_status) : m_tree_wnd->HideStatus(m_status);
        }
        TechTreeWnd* const m_tree_wnd;
        const TechStatus m_status;
    };
}

///////////////////////////
// TechPanelRowBrowseWnd //
///////////////////////////
boost::shared_ptr<GG::BrowseInfoWnd> TechPanelRowBrowseWnd(const std::string& tech_name, int empire_id) {
    const Empire* empire = GetEmpire(empire_id);
    const Tech* tech = GetTech(tech_name);
    if (!tech) {
        boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd;
        return browse_wnd;
    }

    std::string main_text;

    main_text += UserString(tech->Category()) + " ";
    main_text += UserString(tech->ShortDescription()) + "\n";

    if (empire) {
        TechStatus tech_status = empire->GetTechStatus(tech_name);
        if (!tech->Researchable()) {
            main_text += UserString("TECH_WND_UNRESEARCHABLE") + "\n";

        } else if (tech_status == TS_UNRESEARCHABLE) {
            main_text += UserString("TECH_WND_STATUS_LOCKED") + "\n";

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

        } else if (tech_status == TS_RESEARCHABLE) {
            main_text += UserString("TECH_WND_STATUS_RESEARCHABLE") + "\n";

        } else if (tech_status == TS_COMPLETE) {
            main_text += UserString("TECH_WND_STATUS_COMPLETED") + "\n";

        } else if (tech_status == TS_HAS_RESEARCHED_PREREQ) {
            main_text += UserString("TECH_WND_STATUS_PARTIAL_UNLOCK") + "\n";
        }

        const ResearchQueue& queue = empire->GetResearchQueue();
        ResearchQueue::const_iterator queue_it = queue.find(tech_name);
        if (queue_it != queue.end()) {
            main_text += UserString("TECH_WND_ENQUEUED") + "\n";

            float progress = empire->ResearchProgress(tech_name);
            float total_cost = tech->ResearchCost(empire_id);
            float allocation = queue_it->allocated_rp;
            float max_allocation = tech->PerTurnCost(empire_id);

            // %1% / %2%  +  %3% / %4% RP/turn
            main_text += boost::io::str(FlexibleFormat(UserString("TECH_WND_PROGRESS"))
                    % DoubleToString(progress, 3, false)
                    % DoubleToString(total_cost, 3, false)
                    % DoubleToString(allocation, 3, false)
                    % DoubleToString(max_allocation, 3, false)) + "\n";

            int ETA = queue_it->turns_left;
            if (ETA != -1)
                main_text += boost::io::str(FlexibleFormat(UserString("TECH_WND_ETA"))
                    % ETA);

        } else if (tech->Researchable()) {
            int turns = tech->ResearchTime(empire_id);
            float cost = tech->ResearchCost(empire_id);
            const std::string& cost_units = UserString("ENC_RP");

            main_text += boost::io::str(FlexibleFormat(UserString("ENC_COST_AND_TURNS_STR"))
                % DoubleToString(cost, 3, false)
                % cost_units
                % turns);
        }

    } else if (tech->Researchable()) {
        int turns = tech->ResearchTime(empire_id);
        float cost = tech->ResearchCost(empire_id);
        const std::string& cost_units = UserString("ENC_RP");

        main_text += boost::io::str(FlexibleFormat(UserString("ENC_COST_AND_TURNS_STR"))
            % DoubleToString(cost, 3, false)
            % cost_units
            % turns);
    }

    boost::shared_ptr<GG::BrowseInfoWnd> browse_wnd(new IconTextBrowseWnd(
        ClientUI::TechIcon(tech_name), UserString(tech_name), main_text));
    return browse_wnd;
}


//////////////////////////////////////////////////
// TechTreeWnd::TechTreeControls                //
//////////////////////////////////////////////////
/** A panel of buttons that control how the tech tree is displayed: what
  * categories, statuses and types of techs to show. */
class TechTreeWnd::TechTreeControls : public CUIWnd {
public:
    //! \name Structors //@{
    TechTreeControls(const std::string& config_name = "");
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

    /** These values are used for rendering separator lines between groups of buttons */
    static const int BUTTON_SEPARATION; // vertical or horizontal sepration between adjacent buttons
    static const int UPPER_LEFT_PAD;    // offset of buttons' position from top left of controls box

    // TODO: replace all the above stored information with a vector of pairs of GG::Pt (or perhaps GG::Rect)
    // This will contain the start and end points of all separator lines that need to be drawn.  This will be
    // calculated by SizeMove, and stored, so that start and end positions don't need to be recalculated each
    // time Render is called.

    GG::StateButton*                             m_view_type_button;
    GG::StateButton*                             m_all_cat_button;
    std::map<std::string, GG::StateButton*>      m_cat_buttons;
    std::map<TechStatus, GG::StateButton*>       m_status_buttons;

    friend class TechTreeWnd;               // so TechTreeWnd can access buttons
};

const int TechTreeWnd::TechTreeControls::BUTTON_SEPARATION = 2;
const int TechTreeWnd::TechTreeControls::UPPER_LEFT_PAD = 2;

TechTreeWnd::TechTreeControls::TechTreeControls(const std::string& config_name) :
    CUIWnd(UserString("TECH_DISPLAY"), GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP, config_name)
{
    const int tooltip_delay = GetOptionsDB().Get<int>("UI.tooltip-delay");
    const boost::filesystem::path icon_dir = ClientUI::ArtDir() / "icons" / "tech" / "controls";

    // create a button for each tech category...
    const std::vector<std::string>& cats = GetTechManager().CategoryNames();
    for (unsigned int i = 0; i < cats.size(); ++i) {
        GG::Clr icon_clr = ClientUI::CategoryColor(cats[i]);
        boost::shared_ptr<GG::SubTexture> icon = boost::make_shared<GG::SubTexture>(ClientUI::CategoryIcon(cats[i]));
        m_cat_buttons[cats[i]] = new GG::StateButton("", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
                                                     boost::make_shared<CUIToggleRepresenter>(icon, icon_clr));
        m_cat_buttons[cats[i]]->SetBrowseInfoWnd(boost::make_shared<TextBrowseWnd>(UserString(cats[i]), ""));
        m_cat_buttons[cats[i]]->SetBrowseModeTime(tooltip_delay);
        AttachChild(m_cat_buttons[cats[i]]);
    }

    GG::Clr icon_color = GG::Clr(113, 150, 182, 255);
    // and one for "ALL"
    m_all_cat_button = new GG::StateButton("", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
                                           boost::make_shared<CUIToggleRepresenter>(boost::make_shared<GG::SubTexture>(ClientUI::GetTexture(icon_dir / "00_all_cats.png", true)), icon_color));
    m_all_cat_button->SetBrowseInfoWnd(boost::make_shared<TextBrowseWnd>(UserString("ALL"), ""));
    m_all_cat_button->SetBrowseModeTime(tooltip_delay);
    m_all_cat_button->SetCheck(true);
    AttachChild(m_all_cat_button);

    // create a button for each tech status
    m_status_buttons[TS_UNRESEARCHABLE] = new GG::StateButton("", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
                                                              boost::make_shared<CUIToggleRepresenter>(boost::make_shared<GG::SubTexture>(ClientUI::GetTexture(icon_dir / "01_locked.png", true)), icon_color));
    m_status_buttons[TS_UNRESEARCHABLE]->SetBrowseInfoWnd(boost::make_shared<TextBrowseWnd>(UserString("TECH_WND_STATUS_LOCKED"), ""));
    m_status_buttons[TS_UNRESEARCHABLE]->SetBrowseModeTime(tooltip_delay);
    m_status_buttons[TS_UNRESEARCHABLE]->SetCheck(false);
    AttachChild(m_status_buttons[TS_UNRESEARCHABLE]);

    m_status_buttons[TS_HAS_RESEARCHED_PREREQ] = new GG::StateButton("", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
                                                                 boost::make_shared<CUIToggleRepresenter>(boost::make_shared<GG::SubTexture>(ClientUI::GetTexture(icon_dir / "02_partial.png", true)), icon_color));
    m_status_buttons[TS_HAS_RESEARCHED_PREREQ]->SetBrowseInfoWnd(boost::make_shared<TextBrowseWnd>(UserString("TECH_WND_STATUS_PARTIAL_UNLOCK"), ""));
    m_status_buttons[TS_HAS_RESEARCHED_PREREQ]->SetBrowseModeTime(tooltip_delay);
    m_status_buttons[TS_HAS_RESEARCHED_PREREQ]->SetCheck(true);
    AttachChild(m_status_buttons[TS_HAS_RESEARCHED_PREREQ]);

    m_status_buttons[TS_RESEARCHABLE] = new GG::StateButton("", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
                                                            boost::make_shared<CUIToggleRepresenter>(boost::make_shared<GG::SubTexture>(ClientUI::GetTexture(icon_dir / "03_unlocked.png", true)), icon_color));
    m_status_buttons[TS_RESEARCHABLE]->SetBrowseInfoWnd(boost::make_shared<TextBrowseWnd>(UserString("TECH_WND_STATUS_PARTIAL_UNLOCK"), ""));
    m_status_buttons[TS_RESEARCHABLE]->SetBrowseModeTime(tooltip_delay);
    m_status_buttons[TS_RESEARCHABLE]->SetCheck(true);
    AttachChild(m_status_buttons[TS_RESEARCHABLE]);

    m_status_buttons[TS_COMPLETE] = new GG::StateButton("", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
                                                        boost::make_shared<CUIToggleRepresenter>(boost::make_shared<GG::SubTexture>(ClientUI::GetTexture(icon_dir / "04_completed.png", true)), icon_color));
    m_status_buttons[TS_COMPLETE]->SetBrowseInfoWnd(boost::make_shared<TextBrowseWnd>(UserString("TECH_WND_STATUS_COMPLETED"), ""));
    m_status_buttons[TS_COMPLETE]->SetBrowseModeTime(tooltip_delay);
    m_status_buttons[TS_COMPLETE]->SetCheck(true);
    AttachChild(m_status_buttons[TS_COMPLETE]);

    // create button to switch between tree and list views
    m_view_type_button = new GG::StateButton("", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
                                             boost::make_shared<CUIToggleRepresenter>(boost::make_shared<GG::SubTexture>(ClientUI::GetTexture(icon_dir / "06_view_tree.png", true)), icon_color,
                                                                                      boost::make_shared<GG::SubTexture>(ClientUI::GetTexture(icon_dir / "05_view_list.png", true)), GG::Clr(110, 172, 150, 255)));
    m_view_type_button->SetBrowseInfoWnd(boost::make_shared<TextBrowseWnd>(UserString("TECH_WND_VIEW_TYPE"), ""));
    m_view_type_button->SetBrowseModeTime(tooltip_delay);
    m_view_type_button->SetCheck(false);
    AttachChild(m_view_type_button);

    SetChildClippingMode(ClipToClient);
    DoButtonLayout();
}

void TechTreeWnd::TechTreeControls::DoButtonLayout() {
    const int PTS = ClientUI::Pts();
    const GG::X RIGHT_EDGE_PAD(PTS / 3);
    const GG::X USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, GG::X1);   // space in which to do layout
    const GG::X BUTTON_WIDTH = GG::X(PTS * std::max(GetOptionsDB().Get<double>("UI.tech-controls-graphic-size"), 0.5));
    const GG::Y BUTTON_HEIGHT = GG::Y(Value(BUTTON_WIDTH));

    m_col_offset = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    m_row_offset = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons
    m_buttons_per_row = std::max(Value(USABLE_WIDTH / (m_col_offset)), 1);

    const int NUM_NON_CATEGORY_BUTTONS = 6;  //  ALL, Locked, Partial, Unlocked, Complete, ViewType

    // place category buttons: fill each row completely before starting next row
    int row = 0, col = -1;
    for (std::map<std::string, GG::StateButton*>::iterator it = m_cat_buttons.begin();
         it != m_cat_buttons.end(); ++it)
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

    // add ALL button
    ++col;
    if (col >= m_buttons_per_row) {
        ++row;
        col =0;
    }
    GG::Pt all_cats_ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
    GG::Pt all_cats_lr = all_cats_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_all_cat_button->SizeMove(all_cats_ul, all_cats_lr);

    // rowbreak after category buttons, before type and status buttons, unless all buttons fit on one row
    if (m_buttons_per_row < (static_cast<int>(m_cat_buttons.size()) + NUM_NON_CATEGORY_BUTTONS)) {
        col = -1;
        ++row;
    }

    // place status buttons: fill each row completely before starting next row
    for (std::map<TechStatus, GG::StateButton*>::iterator it = m_status_buttons.begin();
         it != m_status_buttons.end(); ++it)
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

    // place view type button
    ++col;
    if (col + 1 > m_buttons_per_row) {
        col = 0;
        ++row;
    }
    GG::Pt view_type_ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
    GG::Pt view_type_lr = view_type_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
    m_view_type_button->SizeMove(view_type_ul, view_type_lr);

    // prevent window from being shrunk less than one button width, or current number of rows of height
    SetMinSize(GG::Pt(UPPER_LEFT_PAD + BUTTON_WIDTH + 3*RIGHT_EDGE_PAD,
                      TopBorder() + BottomBorder() + UPPER_LEFT_PAD + (++row)*m_row_offset));
}

void TechTreeWnd::TechTreeControls::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    m_config_save = false;
    // maybe later do something interesting with docking
    CUIWnd::SizeMove(ul, lr);                               // set width and upper left as user-requested
    DoButtonLayout();                                       // given set width, position buttons and set appropriate minimum height
    m_config_save = true;
    CUIWnd::SizeMove(ul, GG::Pt(lr.x, ul.y + MinSize().y)); // width and upper left unchanged.  set height to minimum height
}

void TechTreeWnd::TechTreeControls::Render() {
    CUIWnd::Render();

    //GG::Pt cl_ul = ClientUpperLeft();
    //GG::Pt cl_lr = ClientLowerRight();

    //glColor(ClientUI::WndOuterBorderColor());

    //GG::Y category_bottom = cl_ul.y + m_category_button_rows*m_row_offset - BUTTON_SEPARATION/2 + UPPER_LEFT_PAD;
    //GG::Line(cl_ul.x, category_bottom, cl_lr.x - 1, category_bottom);

    //if (m_buttons_per_row >= 6) {
    //    // all six status and type buttons are on one row, and need a vertical separator between them
    //    GG::X middle = cl_ul.x + m_col_offset*3 - BUTTON_SEPARATION/2 + UPPER_LEFT_PAD;
    //    GG::Line(middle, category_bottom, middle, cl_lr.y - 1);
    //} else {
    //    // the status and type buttons are split into separate vertical groups, and need a horiztonal separator between them
    //    GG::Y status_bottom = category_bottom + m_status_button_rows*m_row_offset;
    //    GG::Line(cl_ul.x, status_bottom, cl_lr.x - 1, status_bottom);
    //}
}

void TechTreeWnd::TechTreeControls::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) {
    if (m_drag_offset != GG::Pt(-GG::X1, -GG::Y1)) {  // resize-dragging
        GG::Pt new_lr = pt - m_drag_offset;

        new_lr.y = Bottom();    // ignore y-resizes

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
    mutable TechTreeWnd::TechClickSignalType    TechLeftClickedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechRightClickedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechDoubleClickedSignal;
    mutable TechTreeWnd::TechSignalType         TechPediaDisplaySignal;
    //@}

    //! \name Mutators //@{
    virtual void Render();
    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);

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
    void DoZoom(const GG::Pt &pt) const;
    void UndoZoom() const;

    // Converts between screen coordinates and virtual coordiantes
    // doing the inverse or same transformation as DoZoom does with gl calls
    GG::Pt ConvertPtScreenToZoomed(const GG::Pt& pt) const;
    GG::Pt ConvertPtZoomedToScreen(const GG::Pt& pt) const;
    //@}

private:
    class TechPanel;

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
        mutable boost::signals2::signal<void (int)>           ZoomedSignal;
        mutable boost::signals2::signal<void (const GG::Pt&)> DraggedSignal;
        mutable boost::signals2::signal<void (const GG::Pt&)> ButtonDownSignal;
        mutable boost::signals2::signal<void (const GG::Pt&)> ButtonUpSignal;
    };

    void Layout(bool keep_position);    // lays out tech panels

    void DoLayout();    // arranges child controls (scrolls, buttons) to account for window size

    void ScrolledSlot(int, int, int, int);

    void TechBrowsedSlot(const std::string& tech_name);
    void TechLeftClickedSlot(const std::string& tech_name,
                             const GG::Flags<GG::ModKey>& modkeys);
    void TechRightClickedSlot(const std::string& tech_name,
                              const GG::Flags<GG::ModKey>& modkeys);
    void TechDoubleClickedSlot(const std::string& tech_name,
                               const GG::Flags<GG::ModKey>& modkeys);
    void TechPediaDisplaySlot(const std::string& tech_name);

    void TreeDraggedSlot(const GG::Pt& move);
    void TreeDragBegin(const GG::Pt& move);
    void TreeDragEnd(const GG::Pt& move);
    void TreeZoomedSlot(int move);
    void TreeZoomInClicked();
    void TreeZoomOutClicked();
    bool TreeZoomInKeyboard();
    bool TreeZoomOutKeyboard();
    void ConnectKeyboardAcceleratorSignals();

    double                  m_scale;
    std::set<std::string>   m_categories_shown;
    std::set<TechStatus>    m_tech_statuses_shown;
    std::string             m_selected_tech_name;
    std::string             m_browsed_tech_name;
    TechTreeLayout          m_graph;

    std::map<std::string, TechPanel*>   m_techs;
    TechTreeArcs                        m_dependency_arcs;

    LayoutSurface* m_layout_surface;
    GG::Scroll*    m_vscroll;
    GG::Scroll*    m_hscroll;
    double         m_scroll_position_x;     //actual scroll position
    double         m_scroll_position_y;
    double         m_drag_scroll_position_x;//position when drag started
    double         m_drag_scroll_position_y;
    GG::Button*    m_zoom_in_button;
    GG::Button*    m_zoom_out_button;
};


//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel::TechPanel          //
//////////////////////////////////////////////////
/** Represents a single tech in the LayoutPanel. */
class TechTreeWnd::LayoutPanel::TechPanel : public GG::Wnd {
public:
    TechPanel(const std::string& tech_name, const TechTreeWnd::LayoutPanel* panel);
    virtual         ~TechPanel();

    virtual bool    InWindow(const GG::Pt& pt) const;
    virtual void    Render();
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
    { ForwardEventToParent(); }
    virtual void    LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
    { ForwardEventToParent(); }
    virtual void    LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
    { ForwardEventToParent(); }
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    MouseLeave();
    virtual void    MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
    { ForwardEventToParent(); }
    void            Update();
    void            Select(bool select);
    int             FontSize() const;

    mutable TechTreeWnd::TechSignalType         TechBrowsedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechLeftClickedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechRightClickedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechDoubleClickedSignal;
    mutable TechTreeWnd::TechSignalType         TechPediaDisplaySignal;

private:
    void            InitBuffers();

    GG::GL2DVertexBuffer            m_border_buffer;
    GG::GL2DVertexBuffer            m_eta_border_buffer;
    GG::GL2DVertexBuffer            m_enqueued_indicator_buffer;

    const std::string&              m_tech_name;
    std::string                     m_name_text;
    std::string                     m_cost_and_duration_text;
    std::string                     m_eta_text;
    const TechTreeWnd::LayoutPanel* m_layout_panel;
    GG::StaticGraphic*              m_icon;
    std::vector<GG::StaticGraphic*> m_unlock_icons;
    GG::TextControl*                m_name_label;
    GG::TextControl*                m_cost_and_duration_label;
    GG::TextControl*                m_eta_label;
    GG::Clr                         m_colour;
    TechStatus                      m_status;
    bool                            m_browse_highlight;
    bool                            m_selected;
    int                             m_eta;
    bool                            m_enqueued;
};

TechTreeWnd::LayoutPanel::TechPanel::TechPanel(const std::string& tech_name, const LayoutPanel* panel) :
    GG::Wnd(GG::X0, GG::Y0, TechPanelWidth(), TechPanelHeight(), GG::INTERACTIVE),
    m_tech_name(tech_name),
    m_name_text(),
    m_cost_and_duration_text(),
    m_eta_text(),
    m_layout_panel(panel),
    m_icon(0),
    m_unlock_icons(),
    m_name_label(0),
    m_cost_and_duration_label(0),
    m_eta_label(0),
    m_colour(GG::CLR_GRAY),
    m_status(TS_RESEARCHABLE),
    m_browse_highlight(false),
    m_selected(false),
    m_eta(-1),
    m_enqueued(false)
{
    const int GRAPHIC_SIZE = Value(TechPanelHeight());
    m_icon = new GG::StaticGraphic(ClientUI::TechIcon(m_tech_name), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_icon->Resize(GG::Pt(GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE)));

    m_name_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(FontSize()), ClientUI::TextColor(), GG::FORMAT_WORDBREAK | GG::FORMAT_VCENTER | GG::FORMAT_LEFT);
    m_cost_and_duration_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(FontSize()), ClientUI::TextColor(), GG::FORMAT_VCENTER | GG::FORMAT_RIGHT);
    m_eta_label = new GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(FontSize()),ClientUI::TextColor());

    // intentionally not attaching as child; TechPanel::Render the child Render() function instead.

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    Update();
}

TechTreeWnd::LayoutPanel::TechPanel::~TechPanel() {
    delete m_icon;
    delete m_name_label;
    delete m_cost_and_duration_label;
    delete m_eta_label;
    for (std::vector<GG::StaticGraphic*>::iterator it = m_unlock_icons.begin();
         it != m_unlock_icons.end(); ++it)
    { delete *it; }
}

int TechTreeWnd::LayoutPanel::TechPanel::FontSize() const
{ return ClientUI::Pts() * 3 / 2; }

bool TechTreeWnd::LayoutPanel::TechPanel::InWindow(const GG::Pt& pt) const {
    const GG::Pt p = m_layout_panel->ConvertPtScreenToZoomed(pt) - UpperLeft();
    if (m_icon->InWindow(p))
        return true;
    return GG::Pt(GG::X0, GG::Y0) <= p && p < GG::Pt(TechPanelWidth(), TechPanelHeight());
}

void TechTreeWnd::LayoutPanel::TechPanel::Render() {
    const int PAD = 8;
    GG::X text_left(GG::X(Value(TechPanelHeight())) + PAD);
    GG::Y text_top(0);
    GG::X text_width(TechPanelWidth() - text_left);
    GG::Y text_height(TechPanelHeight()/2);

    GG::Pt ul = GG::Pt(text_left, text_top);
    GG::Pt lr = ul + GG::Pt(text_width + PAD, TechPanelHeight());
    GG::Clr border_colour = GG::CLR_WHITE;

    m_layout_panel->DoZoom(UpperLeft());

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0);

    // size of tech name text
    int font_pts = static_cast<int>(FontSize() * m_layout_panel->Scale() + 0.5);

    // cancel out dependency line under tech icon
    glColor(ClientUI::CtrlColor());
    PartlyRoundedRect(m_icon->UpperLeft(), m_icon->LowerRight(), PAD, true, true, true, true, true);

    // Render text part of tech panel, but only if zoomed in so the text is legible
    if (font_pts > 6) {
        // black out dependency lines under panel
        glColor(GG::CLR_BLACK);
        PartlyRoundedRect(ul, lr + GG::Pt(GG::X(4), GG::Y0), PAD, true, true, true, true, true);

        // background of panel
        glColor(m_colour);
        PartlyRoundedRect(ul, lr + GG::Pt(GG::X(4), GG::Y0), PAD, true, true, true, true, true);

        // panel border
        if (m_browse_highlight) {
            glColor(border_colour);
            PartlyRoundedRect(ul, lr + GG::Pt(GG::X(4), GG::Y0), PAD, true, true, true, true, false);
        }
        else if (m_status == TS_COMPLETE || m_status == TS_RESEARCHABLE) {
            border_colour = m_colour;
            border_colour.a = 255;
            glColor(border_colour);
            PartlyRoundedRect(ul, lr + GG::Pt(GG::X(4), GG::Y0), PAD, true, true, true, true, false);
        }
        else {
            border_colour = m_colour;
            border_colour.a = 127;
            // don't render border
        }

        // render tech panel text; for small font sizes, remove shadow
        glEnable(GL_TEXTURE_2D);

        if (font_pts < 10) {
            m_name_label->SetText(m_name_text);
            m_cost_and_duration_label->SetText(m_cost_and_duration_text);
        }
        else {
            m_name_label->SetText("<s>" + m_name_text + "</s>");
            m_cost_and_duration_label->SetText("<s>" + m_cost_and_duration_text + "</s>");
        }

        GG::Pt text_ul(text_left + PAD/2, text_top);
        GG::Pt text_size(text_width + PAD, m_unlock_icons.empty() ? text_height*2 : text_height - PAD/2);
        m_name_label->SizeMove(text_ul, text_ul + text_size);

        // show cost and duration for unresearched techs
        if (const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID()))
            if (empire->TechResearched(m_tech_name))
                m_cost_and_duration_label->Hide();
            else {
                GG::Pt text_ll(text_left + PAD / 2, TechPanelHeight() - font_pts - PAD);
                text_size = GG::Pt(text_width + PAD / 2, GG::Y(font_pts));
                m_cost_and_duration_label->SizeMove(text_ll, text_ll + text_size);
                m_cost_and_duration_label->Show();
            }

        /// Need to render children too
        GG::GUI::GetGUI()->RenderWindow(m_name_label);
        GG::GUI::GetGUI()->RenderWindow(m_cost_and_duration_label);

        // box around whole panel to indicate enqueue
        if (m_enqueued) {
            glColor(GG::CLR_WHITE);
            GG::Pt gap = GG::Pt(GG::X(2 * PAD), GG::Y(2 * PAD));
            GG::Pt enc_ul(-gap);
            GG::Pt enc_lr(lr + gap);
            PartlyRoundedRect(enc_ul, enc_lr, PAD + 6, true, true, true, true, false);
        }

        // ETA background and text
        if (m_eta != -1 && font_pts > 10) {
            GG::Pt panel_size = lr - ul;
            GG::Pt eta_ul = ul + GG::Pt(panel_size.x * 3 / 4, panel_size.y * 3 / 4) - GG::Pt(GG::X(2), GG::Y(2));
            GG::Pt eta_lr = eta_ul + GG::Pt(panel_size.x / 2, panel_size.y / 2) + GG::Pt(GG::X(2), GG::Y(2));

            glColor(GG::CLR_BLACK);
            CircleArc(eta_ul, eta_lr, 0, 2 * PI, true);
            glColor(border_colour);
            CircleArc(eta_ul, eta_lr, 0, 2 * PI, true);

            glEnable(GL_TEXTURE_2D);

            m_eta_label->SizeMove(eta_ul, eta_lr);

            /// Need to render text too
            GG::GUI::GetGUI()->RenderWindow(m_eta_label);
            glDisable(GL_TEXTURE_2D);
        }

    } else {
        // box only around icon to indicate enqueue (when zoomed far out)
        if (m_enqueued) {
            glColor(GG::CLR_WHITE);
            GG::Pt gap = GG::Pt(GG::X(2 * PAD), GG::Y(2 * PAD));
            GG::Pt enc_ul(-gap);
            GG::Pt enc_lr(m_icon->LowerRight() + gap);
            PartlyRoundedRect(enc_ul, enc_lr, PAD + 6, true, true, true, true, false);
        }
    }

    // selection indicator
    if (m_selected) {
        // nothing!
    }

    // render tech icon
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    m_icon->Render();

    for (std::vector<GG::StaticGraphic*>::iterator it = m_unlock_icons.begin();
         it != m_unlock_icons.end(); ++it)
    { (*it)->Render(); }

    m_layout_panel->UndoZoom();
}

void TechTreeWnd::LayoutPanel::TechPanel::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (m_layout_panel->m_selected_tech_name != m_tech_name)
        TechLeftClickedSignal(m_tech_name, mod_keys);
}

void TechTreeWnd::LayoutPanel::TechPanel::RClick(const GG::Pt& pt,
                                                 GG::Flags<GG::ModKey> mod_keys)
{
    TechRightClickedSignal(m_tech_name, mod_keys);

    GG::MenuItem menu_contents;
    if (!(m_enqueued) && !(m_status == TS_COMPLETE))
        menu_contents.next_level.push_back(GG::MenuItem(UserString("PRODUCTION_DETAIL_ADD_TO_QUEUE"),   1, false, false));

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(m_tech_name));
    menu_contents.next_level.push_back(GG::MenuItem(popup_label, 2, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: {
            LDoubleClick(pt, mod_keys);
            break;
        }
        case 2: {
            TechPediaDisplaySignal(m_tech_name);
            break;
        }
        default:
            break;
        }
    }
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

void TechTreeWnd::LayoutPanel::TechPanel::Update() {
    Select(m_layout_panel->m_selected_tech_name == m_tech_name);

    int client_empire_id = HumanClientApp::GetApp()->EmpireID();

    if (const Empire* empire = GetEmpire(client_empire_id)) {
        m_status = empire->GetTechStatus(m_tech_name);
        m_enqueued = empire->GetResearchQueue().InQueue(m_tech_name);

        const ResearchQueue& queue = empire->GetResearchQueue();
        ResearchQueue::const_iterator queue_it = queue.find(m_tech_name);
        if (queue_it != queue.end()) {
            m_eta = queue_it->turns_left;
            if (m_eta != -1)
                m_eta_text = boost::lexical_cast<std::string>(m_eta);
            else
                m_eta_text.clear();
        }
    }

    GG::Clr icon_colour = GG::CLR_WHITE;
    if (const Tech* tech = GetTech(m_tech_name)) {
        m_colour = ClientUI::CategoryColor(tech->Category());
        icon_colour = m_colour;

        if (m_status == TS_UNRESEARCHABLE || m_status == TS_HAS_RESEARCHED_PREREQ) {
            icon_colour = GG::CLR_GRAY;
            m_colour.a = 64;
        } else if (m_status == TS_RESEARCHABLE) {
            icon_colour = GG::CLR_GRAY;
            m_colour.a = 128;
        } else {
            m_colour.a = 255;
        }

        if (m_unlock_icons.empty()) {
            const int PAD = 8;
            GG::X icon_left(GG::X(Value(TechPanelHeight())) + PAD*3/2);
            GG::Y icon_height = TechPanelHeight()/2;
            GG::X icon_width = GG::X(Value(icon_height));
            GG::Y icon_bottom = TechPanelHeight() - PAD/2;
            GG::Y icon_top = icon_bottom - icon_height;

            // add icons for unlocked items
            const std::vector<ItemSpec>& items = tech->UnlockedItems();
            for (std::vector<ItemSpec>::const_iterator item_it = items.begin(); item_it != items.end(); ++item_it) {
                boost::shared_ptr<GG::Texture> texture;
                switch (item_it->type) {
                case UIT_BUILDING:  texture = ClientUI::BuildingIcon(item_it->name);    break;
                case UIT_SHIP_PART: texture = ClientUI::PartIcon(item_it->name);        break;
                case UIT_SHIP_HULL: texture = ClientUI::HullIcon(item_it->name);        break;
                default:    break;
                }

                if (texture) {
                    GG::StaticGraphic* graphic = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
                    m_unlock_icons.push_back(graphic);
                    graphic->SizeMove(GG::Pt(icon_left, icon_top), GG::Pt(icon_left + icon_width, icon_top + icon_height));
                    icon_left += icon_width + PAD;
                }
            }
            // add icons for modified part meters / specials
            std::set<MeterType> meters_affected;
            std::set<std::string> specials_affected;
            std::set<std::string> parts_whose_meters_are_affected;
            const std::vector<boost::shared_ptr<Effect::EffectsGroup> >& effects_groups = tech->Effects();
            for (std::vector<boost::shared_ptr<Effect::EffectsGroup> >::const_iterator effects_group_it = effects_groups.begin();
                 effects_group_it != effects_groups.end(); ++effects_group_it)
            {
                const std::vector<Effect::EffectBase*>& effects = (*effects_group_it)->EffectsList();
                for (std::vector<Effect::EffectBase*>::const_iterator effect_it = effects.begin();
                     effect_it != effects.end(); ++effect_it)
                {
                    if (const Effect::SetMeter* set_meter_effect = dynamic_cast<const Effect::SetMeter*>(*effect_it)) {
                        meters_affected.insert(set_meter_effect->GetMeterType());

                    } else if (const Effect::SetShipPartMeter* set_ship_part_meter_effect = dynamic_cast<const Effect::SetShipPartMeter*>(*effect_it)) {
                        const ValueRef::ValueRefBase<std::string>* part_name = set_ship_part_meter_effect->GetPartName();
                        if (part_name && ValueRef::ConstantExpr(part_name))
                            parts_whose_meters_are_affected.insert(part_name->Eval());

                    } else if (const Effect::AddSpecial* add_special_effect = dynamic_cast<const Effect::AddSpecial*>(*effect_it)) {
                        const ValueRef::ValueRefBase<std::string>* special_name = add_special_effect->GetSpecialName();
                        if (special_name && ValueRef::ConstantExpr(special_name))
                            specials_affected.insert(special_name->Eval());
                    }
                }
            }

            for (std::set<std::string>::const_iterator part_it = parts_whose_meters_are_affected.begin();
                 part_it != parts_whose_meters_are_affected.end(); ++part_it)
            {
                boost::shared_ptr<GG::Texture> texture = ClientUI::PartIcon(*part_it);
                if (texture) {
                    GG::StaticGraphic* graphic = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
                    m_unlock_icons.push_back(graphic);
                    graphic->SizeMove(GG::Pt(icon_left, icon_top), GG::Pt(icon_left + icon_width, icon_top + icon_height));
                    icon_left += icon_width + PAD;
                }
            }
            for (std::set<MeterType>::const_iterator meter_it = meters_affected.begin();
                 meter_it != meters_affected.end(); ++meter_it)
            {
                boost::shared_ptr<GG::Texture> texture = ClientUI::MeterIcon(*meter_it);
                if (texture) {
                    GG::StaticGraphic* graphic = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
                    m_unlock_icons.push_back(graphic);
                    graphic->SizeMove(GG::Pt(icon_left, icon_top), GG::Pt(icon_left + icon_width, icon_top + icon_height));
                    icon_left += icon_width + PAD;
                }
            }
            for (std::set<std::string>::const_iterator special_it = specials_affected.begin();
                 special_it != specials_affected.end(); ++special_it)
            {
                boost::shared_ptr<GG::Texture> texture = ClientUI::SpecialIcon(*special_it);
                if (texture) {
                    GG::StaticGraphic* graphic = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
                    m_unlock_icons.push_back(graphic);
                    graphic->SizeMove(GG::Pt(icon_left, icon_top), GG::Pt(icon_left + icon_width, icon_top + icon_height));
                    icon_left += icon_width + PAD;
                }
            }
        }
    }
    m_icon->SetColor(icon_colour);

    m_name_text = UserString(m_tech_name);
    m_name_label->SetText("<s>" + m_name_text + "</s>");

    if (const Tech* tech = GetTech(m_tech_name))
        m_cost_and_duration_text = boost::io::str(FlexibleFormat(UserString("TECH_TOTAL_COST_ALT_STR"))
            % DoubleToString(tech->ResearchCost(client_empire_id), 1, false)
            % std::to_string((int)tech->ResearchTime(client_empire_id)));
    m_cost_and_duration_label->SetText("<s>" + m_cost_and_duration_text + "<s>");

    m_eta_label->SetText("<s>" + m_eta_text + "</s>");

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

    m_scale = std::pow(ZOOM_STEP_SIZE, GetOptionsDB().Get<double>("UI.tech-layout-zoom-scale")); //(LATHANDA) Initialise Fullzoom and do real zooming using GL. TODO: Check best size

    m_layout_surface = new LayoutSurface();

    m_vscroll = new CUIScroll(GG::VERTICAL);
    m_hscroll = new CUIScroll(GG::HORIZONTAL);

    m_zoom_in_button = new CUIButton("+");
    m_zoom_in_button->SetColor(ClientUI::WndColor());
    m_zoom_out_button = new CUIButton("-");
    m_zoom_out_button->SetColor(ClientUI::WndColor());

    DoLayout();

    AttachChild(m_layout_surface);
    AttachChild(m_vscroll);
    AttachChild(m_hscroll);
    AttachChild(m_zoom_in_button);
    AttachChild(m_zoom_out_button);

    GG::Connect(m_layout_surface->DraggedSignal,        &TechTreeWnd::LayoutPanel::TreeDraggedSlot,     this);
    GG::Connect(m_layout_surface->ButtonUpSignal,       &TechTreeWnd::LayoutPanel::TreeDragEnd,         this);
    GG::Connect(m_layout_surface->ButtonDownSignal,     &TechTreeWnd::LayoutPanel::TreeDragBegin,       this);
    GG::Connect(m_layout_surface->ZoomedSignal,         &TechTreeWnd::LayoutPanel::TreeZoomedSlot,      this);
    GG::Connect(m_vscroll->ScrolledSignal,              &TechTreeWnd::LayoutPanel::ScrolledSlot,        this);
    GG::Connect(m_hscroll->ScrolledSignal,              &TechTreeWnd::LayoutPanel::ScrolledSlot,        this);
    GG::Connect(m_zoom_in_button->LeftClickedSignal,    &TechTreeWnd::LayoutPanel::TreeZoomInClicked,   this);
    GG::Connect(m_zoom_out_button->LeftClickedSignal,   &TechTreeWnd::LayoutPanel::TreeZoomOutClicked,  this);

    ConnectKeyboardAcceleratorSignals();

    // show all categories...
    m_categories_shown.clear();
    const std::vector<std::string> categories = GetTechManager().CategoryNames();
    for (std::vector<std::string>::const_iterator it = categories.begin(); it != categories.end(); ++it)
        m_categories_shown.insert(*it);

    // show statuses
    m_tech_statuses_shown.clear();
    //m_tech_statuses_shown.insert(TS_UNRESEARCHABLE);
    m_tech_statuses_shown.insert(TS_RESEARCHABLE);
    m_tech_statuses_shown.insert(TS_COMPLETE);
}

void TechTreeWnd::LayoutPanel::ConnectKeyboardAcceleratorSignals() {
    HotkeyManager* hkm = HotkeyManager::GetManager();

    hkm->Connect(this, &TechTreeWnd::LayoutPanel::TreeZoomInKeyboard,   "map.zoom_in",
                 new AndCondition(new VisibleWindowCondition(this), new NoModalWndsOpenCondition()));
    hkm->Connect(this, &TechTreeWnd::LayoutPanel::TreeZoomInKeyboard,   "map.zoom_in_alt",
                 new AndCondition(new VisibleWindowCondition(this), new NoModalWndsOpenCondition()));
    hkm->Connect(this, &TechTreeWnd::LayoutPanel::TreeZoomOutKeyboard,  "map.zoom_out",
                 new AndCondition(new VisibleWindowCondition(this), new NoModalWndsOpenCondition()));
    hkm->Connect(this, &TechTreeWnd::LayoutPanel::TreeZoomOutKeyboard,  "map.zoom_out_alt",
                 new AndCondition(new VisibleWindowCondition(this), new NoModalWndsOpenCondition()));

    hkm->RebuildShortcuts();
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
    GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::CtrlColor(), GG::CLR_ZERO);

    BeginClipping();

    // render dependency arcs
    DoZoom(ClientUpperLeft());

    m_dependency_arcs.Render(m_scale);

    EndClipping();

    UndoZoom();
    GG::GUI::RenderWindow(m_vscroll);
    GG::GUI::RenderWindow(m_hscroll);
}

void TechTreeWnd::LayoutPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void TechTreeWnd::LayoutPanel::DoLayout() {
    const int SCRLWDTH = ClientUI::ScrollWidth();

    GG::Pt vscroll_ul = GG::Pt(Width() - SCRLWDTH, GG::Y0);
    GG::Pt vscroll_lr = GG::Pt(Width(), Height() - SCRLWDTH);
    m_vscroll->SizeMove(vscroll_ul, vscroll_lr);

    GG::Pt hscroll_ul = GG::Pt(GG::X0, Height() - SCRLWDTH);
    GG::Pt hscroll_lr = GG::Pt(Width() - SCRLWDTH, Height());
    m_hscroll->SizeMove(hscroll_ul, hscroll_lr);

    const GG::X ZBSIZE(ClientUI::ScrollWidth() * 2);
    const int ZBOFFSET = ClientUI::ScrollWidth() / 2;

    GG::Pt button_ul = GG::Pt(Width() - ZBSIZE - ZBOFFSET - SCRLWDTH, GG::Y(ZBOFFSET));
    m_zoom_in_button->MoveTo(button_ul);
    m_zoom_in_button->Resize(GG::Pt(ZBSIZE, m_zoom_in_button->MinUsableSize().y));
    button_ul += GG::Pt(GG::X0, m_zoom_in_button->Height() + ZBOFFSET);
    m_zoom_out_button->MoveTo(button_ul);
    m_zoom_out_button->Resize(GG::Pt(ZBSIZE, m_zoom_out_button->MinUsableSize().y));
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

    m_dependency_arcs.Reset();

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
    m_scale = scale;
    GetOptionsDB().Set<double>("UI.tech-layout-zoom-scale", std::floor(0.1 + (std::log(m_scale) / std::log(ZOOM_STEP_SIZE))));
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
{ TechLeftClickedSlot(tech_name, GG::Flags<GG::ModKey>()); }

void TechTreeWnd::LayoutPanel::CenterOnTech(const std::string& tech_name) {
    std::map<std::string, TechPanel*>::const_iterator it = m_techs.find(tech_name);
    if (it == m_techs.end()) {
        DebugLogger() << "TechTreeWnd::LayoutPanel::CenterOnTech couldn't centre on " << tech_name
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

void TechTreeWnd::LayoutPanel::DoZoom(const GG::Pt& pt) const {
    glPushMatrix();
    //center to panel
    glTranslated(Value(Width()/2.0), Value(Height()/2.0), 0);
    //zoom
    glScaled(m_scale, m_scale, 1);
    //translate to actual scroll position
    glTranslated(-m_scroll_position_x, -m_scroll_position_y, 0);
    glTranslated(Value(pt.x), Value(pt.y), 0);
}

void TechTreeWnd::LayoutPanel::UndoZoom() const
{ glPopMatrix(); }

GG::Pt TechTreeWnd::LayoutPanel::ConvertPtScreenToZoomed(const GG::Pt& pt) const {
    double x = Value(pt.x);
    double y = Value(pt.y);
    x -= Value(Width()/2.0);
    y -= Value(Height()/2.0);
    x /= m_scale;
    y /= m_scale;
    x += m_scroll_position_x;
    y += m_scroll_position_y;
    return GG::Pt(GG::X(static_cast<int>(x)), GG::Y(static_cast<int>(y)));
}

GG::Pt TechTreeWnd::LayoutPanel::ConvertPtZoomedToScreen(const GG::Pt& pt) const {
    double x = Value(pt.x);
    double y = Value(pt.y);
    x -= m_scroll_position_x;
    y -= m_scroll_position_y;
    x *= m_scale;
    y *= m_scale;
    x += Value(Width()/2.0);
    y += Value(Height()/2.0);
    return GG::Pt(GG::X(static_cast<int>(x)), GG::Y(static_cast<int>(y)));
}

void TechTreeWnd::LayoutPanel::Layout(bool keep_position) {
    const GG::X TECH_PANEL_MARGIN_X(ClientUI::Pts()*16);
    const GG::Y TECH_PANEL_MARGIN_Y(ClientUI::Pts()*16 + 100);
    const double RANK_SEP = Value(TechPanelWidth()) * GetOptionsDB().Get<double>("UI.tech-layout-horz-spacing");
    const double NODE_SEP = Value(TechPanelHeight()) * GetOptionsDB().Get<double>("UI.tech-layout-vert-spacing");
    const double WIDTH = Value(TechPanelWidth());
    const double HEIGHT = Value(TechPanelHeight());
    const double X_MARGIN(12);

    // view state initial data
    int initial_hscroll_pos = m_hscroll->PosnRange().first;
    int initial_vscroll_pos = m_vscroll->PosnRange().first;
    double initial_hscroll_page_size = m_hscroll->PageSize();
    double initial_vscroll_page_size = m_vscroll->PageSize();
    const std::string selected_tech = m_selected_tech_name;

    // cleanup old data for new layout
    Clear();

    DebugLogger() << "Tech Tree Layout Preparing Tech Data";

    // create a node for every tech
    TechManager& manager = GetTechManager();
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        const Tech* tech = *it;
        if (!tech) continue;
        const std::string& tech_name = tech->Name();
        if (!TechVisible(tech_name, m_categories_shown, m_tech_statuses_shown)) continue;
        m_techs[tech_name] = new TechPanel(tech_name, this);
        m_graph.AddNode(tech_name, m_techs[tech_name]->Width(), m_techs[tech_name]->Height());
    }

    // create an edge for every prerequisite
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        const Tech* tech = *it;
        if (!tech) continue;
        const std::string& tech_name = tech->Name();
        if (!TechVisible(tech_name, m_categories_shown, m_tech_statuses_shown)) continue;
        for (std::set<std::string>::const_iterator prereq_it = tech->Prerequisites().begin();
             prereq_it != tech->Prerequisites().end(); ++prereq_it)
        {
            if (!TechVisible(*prereq_it, m_categories_shown, m_tech_statuses_shown)) continue;
            m_graph.AddEdge(*prereq_it, tech_name);
        }
    }

    DebugLogger() << "Tech Tree Layout Doing Graph Layout";

    //calculate layout
    m_graph.DoLayout(static_cast<int>(WIDTH + RANK_SEP),
                     static_cast<int>(HEIGHT + NODE_SEP),
                     static_cast<int>(X_MARGIN));

    DebugLogger() << "Tech Tree Layout Creating Panels";

    std::set<std::string> visible_techs;

    // create new tech panels and new dependency arcs 
    for (TechManager::iterator it = manager.begin(); it != manager.end(); ++it) {
        const Tech* tech = *it;
        if (!tech) continue;
        const std::string& tech_name = tech->Name();
        if (!TechVisible(tech_name, m_categories_shown, m_tech_statuses_shown)) continue;
        //techpanel
        const TechTreeLayout::Node* node = m_graph.GetNode(tech_name);
        //move TechPanel
        TechPanel* tech_panel = m_techs[tech_name];
        tech_panel->MoveTo(GG::Pt(node->GetX(), node->GetY()));
        m_layout_surface->AttachChild(tech_panel);
        GG::Connect(tech_panel->TechBrowsedSignal,          &TechTreeWnd::LayoutPanel::TechBrowsedSlot,         this);
        GG::Connect(tech_panel->TechLeftClickedSignal,      &TechTreeWnd::LayoutPanel::TechLeftClickedSlot,     this);
        GG::Connect(tech_panel->TechRightClickedSignal,     &TechTreeWnd::LayoutPanel::TechRightClickedSlot,    this);
        GG::Connect(tech_panel->TechDoubleClickedSignal,    &TechTreeWnd::LayoutPanel::TechDoubleClickedSlot,   this);
        GG::Connect(tech_panel->TechPediaDisplaySignal,     &TechTreeWnd::LayoutPanel::TechPediaDisplaySlot,    this);

        visible_techs.insert(tech_name);
    }

    m_dependency_arcs.Reset(m_graph, visible_techs);

    // format window
    GG::Pt client_sz = ClientSize();
    GG::Pt layout_size(client_sz.x + m_graph.GetWidth(), client_sz.y + m_graph.GetHeight());
    m_layout_surface->Resize(layout_size);
    // format scrollbar
    m_vscroll->SizeScroll(0, Value(layout_size.y - 1), std::max(50, Value(std::min(layout_size.y / 10, client_sz.y))), Value(client_sz.y));
    m_hscroll->SizeScroll(0, Value(layout_size.x - 1), std::max(50, Value(std::min(layout_size.x / 10, client_sz.x))), Value(client_sz.x));

    DebugLogger() << "Tech Tree Layout Done";

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
            if (TechVisible(tech_name, m_categories_shown, m_tech_statuses_shown)) {
                CenterOnTech(tech_name);
                break;
            }
        }
    }

    // ensure that the scrolls stay on top
    MoveChildUp(m_vscroll);
    MoveChildUp(m_hscroll);
}

void TechTreeWnd::LayoutPanel::ScrolledSlot(int, int, int, int) {
    m_scroll_position_x = m_hscroll->PosnRange().first;
    m_scroll_position_y = m_vscroll->PosnRange().first;
}

void TechTreeWnd::LayoutPanel::TechBrowsedSlot(const std::string& tech_name)
{ TechBrowsedSignal(tech_name); }

void TechTreeWnd::LayoutPanel::TechLeftClickedSlot(const std::string& tech_name,
                                                   const GG::Flags<GG::ModKey>& modkeys)
{
    // deselect previously-selected tech panel
    if (m_techs.find(m_selected_tech_name) != m_techs.end())
        m_techs[m_selected_tech_name]->Select(false);
    // select clicked on tech
    if (m_techs.find(tech_name) != m_techs.end())
        m_techs[tech_name]->Select(true);
    m_selected_tech_name = tech_name;
    TechLeftClickedSignal(tech_name, modkeys);
}

void TechTreeWnd::LayoutPanel::TechRightClickedSlot(const std::string& tech_name,
                                                    const GG::Flags<GG::ModKey>& modkeys)
{ TechRightClickedSignal(tech_name, modkeys); }

void TechTreeWnd::LayoutPanel::TechDoubleClickedSlot(const std::string& tech_name,
                                                     const GG::Flags<GG::ModKey>& modkeys)
{ TechDoubleClickedSignal(tech_name, modkeys); }

void TechTreeWnd::LayoutPanel::TechPediaDisplaySlot(const std::string& tech_name)
{ TechPediaDisplaySignal(tech_name); }


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

// The bool return value is to re-use the MapWnd::KeyboardZoomIn hotkey
bool TechTreeWnd::LayoutPanel::TreeZoomInKeyboard() {
    TreeZoomedSlot(1);
    return true;
}

bool TechTreeWnd::LayoutPanel::TreeZoomOutKeyboard() {
    TreeZoomedSlot(-1);
    return true;
}

//////////////////////////////////////////////////
// TechTreeWnd::TechListBox                     //
//////////////////////////////////////////////////
class TechTreeWnd::TechListBox : public CUIListBox {
public:
    /** \name Structors */ //@{
    TechListBox(GG::X w, GG::Y h);
    virtual ~TechListBox();
    //@}

    /** \name Accessors */ //@{
    std::set<std::string>   GetCategoriesShown() const;
    std::set<TechStatus>    GetTechStatusesShown() const;
    //@}

    //! \name Mutators //@{
    void    Reset();
    void    Update();

    void    ShowCategory(const std::string& category);
    void    ShowAllCategories();
    void    HideCategory(const std::string& category);
    void    HideAllCategories();
    void    ShowStatus(TechStatus status);
    void    HideStatus(TechStatus status);
    //@}

    mutable TechSignalType      TechBrowsedSignal;      ///< emitted when the mouse rolls over a technology
    mutable TechClickSignalType TechLeftClickedSignal;  ///< emitted when a technology is single-left-clicked
    mutable TechClickSignalType TechRightClickedSignal; ///< emitted when a technology is single-right-clicked
    mutable TechClickSignalType TechDoubleClickedSignal;///< emitted when a technology is double-clicked
    mutable TechSignalType      TechPediaDisplaySignal; ///< emitted when requesting a pedia lookup

private:
    class TechRow : public CUIListBox::Row {
    public:
        TechRow(GG::X w, const std::string& tech_name);
        const std::string&          GetTech() { return m_tech; }
        virtual void                Render();
        static std::vector<GG::X>   ColWidths(GG::X total_width);
        void                        Update();

    private:
        std::string m_tech;
    };

    void    Populate();
    void    TechDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    TechLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    TechRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys);
    void    TechPediaDisplay(const std::string& tech_name);

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

    const GG::X DESC_WIDTH = std::max(GG::X1, total_width - GRAPHIC_WIDTH - NAME_WIDTH - COST_WIDTH - TIME_WIDTH - CATEGORY_WIDTH);
    std::vector<GG::X> retval;
    retval.push_back(GRAPHIC_WIDTH);
    retval.push_back(NAME_WIDTH);
    retval.push_back(COST_WIDTH);
    retval.push_back(TIME_WIDTH);
    retval.push_back(CATEGORY_WIDTH);
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
    const GG::X DESC_WIDTH =      col_widths[5];
    const GG::Y HEIGHT(Value(GRAPHIC_WIDTH));

    GG::StaticGraphic* graphic = new GG::StaticGraphic(ClientUI::TechIcon(m_tech), GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC);
    graphic->Resize(GG::Pt(GRAPHIC_WIDTH, HEIGHT));
    graphic->SetColor(ClientUI::CategoryColor(this_row_tech->Category()));
    push_back(graphic);

    GG::Label* text = new CUILabel(UserString(m_tech), GG::FORMAT_LEFT);
    text->Resize(GG::Pt(NAME_WIDTH, HEIGHT));
    text->ClipText(true);
    push_back(text);

    std::string cost_str = boost::lexical_cast<std::string>(static_cast<int>(this_row_tech->ResearchCost(HumanClientApp::GetApp()->EmpireID()) + 0.5));
    text = new CUILabel(cost_str, GG::FORMAT_LEFT);
    text->Resize(GG::Pt(COST_WIDTH, HEIGHT));
    push_back(text);

    std::string time_str = boost::lexical_cast<std::string>(this_row_tech->ResearchTime(HumanClientApp::GetApp()->EmpireID()));
    text = new CUILabel(time_str, GG::FORMAT_LEFT);
    text->Resize(GG::Pt(TIME_WIDTH, HEIGHT));
    push_back(text);

    text = new CUILabel(UserString(this_row_tech->Category()), GG::FORMAT_LEFT);
    text->Resize(GG::Pt(CATEGORY_WIDTH, HEIGHT));
    push_back(text);

    text = new CUILabel(UserString(this_row_tech->ShortDescription()), GG::FORMAT_LEFT);
    text->Resize(GG::Pt(DESC_WIDTH, HEIGHT));
    push_back(text);
}

void TechTreeWnd::TechListBox::TechRow::Update() {
    const Tech* this_row_tech = ::GetTech(m_tech);
    if (!this || !this_row_tech || this->size() < 4)
        return;

    std::string cost_str = boost::lexical_cast<std::string>(static_cast<int>(this_row_tech->ResearchCost(HumanClientApp::GetApp()->EmpireID()) + 0.5));
    if (GG::TextControl* tc = dynamic_cast<GG::TextControl*>((*this)[2]))
        tc->SetText(cost_str);

    std::string time_str = boost::lexical_cast<std::string>(this_row_tech->ResearchTime(HumanClientApp::GetApp()->EmpireID()));
    if (GG::TextControl* tc = dynamic_cast<GG::TextControl*>((*this)[3]))
        tc->SetText(time_str);
}

TechTreeWnd::TechListBox::TechListBox(GG::X w, GG::Y h) :
    CUIListBox()
{
    Resize(GG::Pt(w, h));
    GG::Connect(DoubleClickedSignal,    &TechListBox::TechDoubleClicked,    this);
    GG::Connect(LeftClickedSignal,      &TechListBox::TechLeftClicked,      this);
    GG::Connect(RightClickedSignal,     &TechListBox::TechRightClicked,     this);

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

TechTreeWnd::TechListBox::~TechListBox() {
    for (std::multimap<std::string, TechRow*>::iterator it = m_all_tech_rows.begin();
         it != m_all_tech_rows.end(); ++it)
    { delete it->second; }
    m_all_tech_rows.clear();
}

std::set<std::string> TechTreeWnd::TechListBox::GetCategoriesShown() const
{ return m_categories_shown; }

std::set<TechStatus> TechTreeWnd::TechListBox::GetTechStatusesShown() const
{ return m_tech_statuses_shown; }

void TechTreeWnd::TechListBox::Reset()
{ Populate(); }

void TechTreeWnd::TechListBox::Update() {
    for (std::multimap<std::string, TechRow*>::iterator it = m_all_tech_rows.begin();
         it != m_all_tech_rows.end(); ++it)
    {
        TechRow* tech_row = it->second;
        if (TechVisible(tech_row->GetTech(), m_categories_shown, m_tech_statuses_shown))
            tech_row->Update();
    }
}

void TechTreeWnd::TechListBox::Populate() {
    // abort of not visible to see results
    if (!Visible())
        return;

    DebugLogger() << "Tech List Box Populating";

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
        if (TechVisible(tech_row->GetTech(), m_categories_shown, m_tech_statuses_shown)) {
            tech_row->Update();
            insertion_timer.restart();
            Insert(tech_row);
            insertion_elapsed += insertion_timer.elapsed();
        }
    }

    DebugLogger() << "Tech List Box Done Populating";
    DebugLogger() << "    Creation time=" << (creation_elapsed * 1000) << "ms";
    DebugLogger() << "    Insertion time=" << (insertion_elapsed * 1000) << "ms";
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

void TechTreeWnd::TechListBox::TechLeftClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    // determine type of row that was clicked, and emit appropriate signal
    if (TechRow* tech_row = dynamic_cast<TechRow*>(*it))
        TechLeftClickedSignal(tech_row->GetTech(), GG::Flags<GG::ModKey>());
}

void TechTreeWnd::TechListBox::TechRightClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    if ((*it)->Disabled())
        return;
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;

    TechRow* tech_row = dynamic_cast<TechRow*>(*it);
    if (!tech_row)
        return;
    const std::string& tech_name = tech_row->GetTech();

    const ResearchQueue& rq = empire->GetResearchQueue();
    if (rq.find(tech_name) != rq.end())
        return;

    GG::MenuItem menu_contents;

    if (!empire->TechResearched(tech_name))
        menu_contents.next_level.push_back(GG::MenuItem(UserString("PRODUCTION_DETAIL_ADD_TO_QUEUE"),   1, false, false));

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(tech_name));
    menu_contents.next_level.push_back(GG::MenuItem(popup_label, 2, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: {
            TechDoubleClicked(it, pt, GG::Flags<GG::ModKey>());
            break;
        }
        case 2: {
            TechPediaDisplay(tech_name);
            break;
        }
        default:
            break;
        }
    }
}

void TechTreeWnd::TechListBox::TechPediaDisplay(const std::string& tech_name) {
    TechPediaDisplaySignal(tech_name);
}

void TechTreeWnd::TechListBox::TechDoubleClicked(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
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
    m_tech_tree_controls(0),
    m_enc_detail_panel(0),
    m_layout_panel(0),
    m_tech_list(0)
{
    Sound::TempUISoundDisabler sound_disabler;

    m_layout_panel = new LayoutPanel(w, h);
    GG::Connect(m_layout_panel->TechBrowsedSignal,      &TechTreeWnd::TechBrowsedSlot,          this);
    GG::Connect(m_layout_panel->TechLeftClickedSignal,  &TechTreeWnd::TechLeftClickedSlot,      this);
    GG::Connect(m_layout_panel->TechRightClickedSignal, &TechTreeWnd::TechRightClickedSlot,     this);
    GG::Connect(m_layout_panel->TechDoubleClickedSignal,&TechTreeWnd::TechDoubleClickedSlot,    this);
    GG::Connect(m_layout_panel->TechPediaDisplaySignal, &TechTreeWnd::TechPediaDisplaySlot,     this);
    AttachChild(m_layout_panel);

    m_tech_list = new TechListBox(w, h);
    GG::Connect(m_tech_list->TechBrowsedSignal,         &TechTreeWnd::TechBrowsedSlot,          this);
    GG::Connect(m_tech_list->TechLeftClickedSignal,     &TechTreeWnd::TechLeftClickedSlot,      this);
    GG::Connect(m_tech_list->TechRightClickedSignal,    &TechTreeWnd::TechRightClickedSlot,     this);
    GG::Connect(m_tech_list->TechDoubleClickedSignal,   &TechTreeWnd::TechDoubleClickedSlot,    this);
    GG::Connect(m_tech_list->TechPediaDisplaySignal,    &TechTreeWnd::TechPediaDisplaySlot,     this);

    m_enc_detail_panel = new EncyclopediaDetailPanel(GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE, RES_PEDIA_WND_NAME);
    m_tech_tree_controls = new TechTreeControls(RES_CONTROLS_WND_NAME);

    GG::Connect(m_enc_detail_panel->ClosingSignal, boost::bind(&TechTreeWnd::HidePedia, this));

    InitializeWindows();
    // Make sure the controls don't overlap the bottom scrollbar
    if (m_tech_tree_controls->Bottom() > m_layout_panel->Bottom() - ClientUI::ScrollWidth()) {
        m_tech_tree_controls->MoveTo(GG::Pt(m_tech_tree_controls->Left(),
                                            m_layout_panel->Bottom() - ClientUI::ScrollWidth() - m_tech_tree_controls->Height()));
    }
    GG::Connect(HumanClientApp::GetApp()->RepositionWindowsSignal, &TechTreeWnd::InitializeWindows, this);

    AttachChild(m_enc_detail_panel);
    AttachChild(m_tech_tree_controls);

    // connect category button clicks to update display
    for (std::map<std::string, GG::StateButton*>::iterator it = m_tech_tree_controls->m_cat_buttons.begin();
        it != m_tech_tree_controls->m_cat_buttons.end(); ++it)
    { GG::Connect(it->second->CheckedSignal, SetCategoryViewFunctor(this, it->first)); }

    // connect button for all categories to update display
    GG::Connect(m_tech_tree_controls->m_all_cat_button->CheckedSignal, SetAllCategoryViewsFunctor(this));

    // connect status and type button clicks to update display
    for (std::map<TechStatus, GG::StateButton*>::iterator it = m_tech_tree_controls->m_status_buttons.begin();
         it != m_tech_tree_controls->m_status_buttons.end(); ++it)
    { GG::Connect(it->second->CheckedSignal, SetTechStatusViewFunctor(this, it->first)); }

    // connect view type selector
    GG::Connect(m_tech_tree_controls->m_view_type_button->CheckedSignal, &TechTreeWnd::ToggleViewType, this);

    ShowAllCategories();
    ShowStatus(TS_RESEARCHABLE);
    ShowStatus(TS_HAS_RESEARCHED_PREREQ);
    ShowStatus(TS_COMPLETE);
    // leave unresearchable hidden by default

    ShowTreeView();
}

TechTreeWnd::~TechTreeWnd() {
    delete m_tech_list;
    delete m_layout_panel;
}

void TechTreeWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size()) {
        m_enc_detail_panel->ValidatePosition();
        m_tech_tree_controls->ValidatePosition();
        m_layout_panel->Resize(this->Size());
        m_tech_list->Resize(this->Size());
    }
}

double TechTreeWnd::Scale() const
{ return m_layout_panel->Scale(); }

std::set<std::string> TechTreeWnd::GetCategoriesShown() const
{ return m_layout_panel->GetCategoriesShown(); }

std::set<TechStatus> TechTreeWnd::GetTechStatusesShown() const
{ return m_layout_panel->GetTechStatusesShown(); }

void TechTreeWnd::Update() {
    m_layout_panel->Update();
    m_tech_list->Update();
}

void TechTreeWnd::Clear() {
    m_enc_detail_panel->OnIndex();
    m_layout_panel->Clear();
}

void TechTreeWnd::Reset() {
    m_layout_panel->Reset();
    m_tech_list->Reset();
}

void TechTreeWnd::InitializeWindows() {
    const GG::Pt pedia_ul(GG::X0,  GG::Y0);
    const GG::Pt pedia_wh(GG::X(480), GG::Y(240));

    // Don't know this wnd's height in advance so place it off the bottom edge,
    // it subclasses CUIWnd so it will reposition itself to be visible.
    const GG::Pt controls_ul(GG::X1, m_layout_panel->Height());
    const GG::Pt controls_wh((m_layout_panel->Width() * 0.6) - ClientUI::ScrollWidth(), GG::Y0);

    m_enc_detail_panel->InitSizeMove(pedia_ul,  pedia_ul + pedia_wh);
    m_tech_tree_controls->InitSizeMove(controls_ul,  controls_ul + controls_wh);
}

void TechTreeWnd::ShowCategory(const std::string& category) {
    m_layout_panel->ShowCategory(category);
    m_tech_list->ShowCategory(category);

    std::map<std::string, GG::StateButton*>::iterator
        maybe_button = m_tech_tree_controls->m_cat_buttons.find(category);
    if (maybe_button != m_tech_tree_controls->m_cat_buttons.end())
        maybe_button->second->SetCheck(true);
}

void TechTreeWnd::ShowAllCategories() {
    m_layout_panel->ShowAllCategories();
    m_tech_list->ShowAllCategories();

    for (std::map<std::string, GG::StateButton*>::const_iterator it = m_tech_tree_controls->m_cat_buttons.begin();
         it != m_tech_tree_controls->m_cat_buttons.end(); ++it)
    { it->second->SetCheck(true); }
}

void TechTreeWnd::HideCategory(const std::string& category) {
    m_layout_panel->HideCategory(category);
    m_tech_list->HideCategory(category);

    std::map<std::string, GG::StateButton*>::iterator
        maybe_button = m_tech_tree_controls->m_cat_buttons.find(category);
    if (maybe_button != m_tech_tree_controls->m_cat_buttons.end())
        maybe_button->second->SetCheck(false);
}

void TechTreeWnd::HideAllCategories() {
    m_layout_panel->HideAllCategories();
    m_tech_list->HideAllCategories();

    for (std::map<std::string, GG::StateButton*>::const_iterator it = m_tech_tree_controls->m_cat_buttons.begin();
         it != m_tech_tree_controls->m_cat_buttons.end(); ++it)
    { it->second->SetCheck(false); }
}

void TechTreeWnd::ToggleAllCategories() {
    std::set<std::string> shown_cats = m_layout_panel->GetCategoriesShown();
    const std::vector<std::string> all_cats = GetTechManager().CategoryNames();

    if (shown_cats.size() == all_cats.size())
        HideAllCategories();
    else
        ShowAllCategories();
}

void TechTreeWnd::ShowStatus(TechStatus status) {
    m_layout_panel->ShowStatus(status);
    m_tech_list->ShowStatus(status);
}

void TechTreeWnd::HideStatus(TechStatus status) {
    m_layout_panel->HideStatus(status);
    m_tech_list->HideStatus(status);
}

void TechTreeWnd::ToggleViewType(bool show_list_view)
{ show_list_view ? ShowListView() : ShowTreeView(); }

void TechTreeWnd::ShowTreeView() {
    AttachChild(m_layout_panel);
    MoveChildDown(m_layout_panel);
    DetachChild(m_tech_list);
    MoveChildUp(m_tech_tree_controls);
}

void TechTreeWnd::ShowListView() {
    m_tech_list->Reset();
    AttachChild(m_tech_list);
    MoveChildDown(m_tech_list);
    DetachChild(m_layout_panel);
    MoveChildUp(m_tech_tree_controls);
}

void TechTreeWnd::SetScale(double scale)
{ m_layout_panel->SetScale(scale); }

void TechTreeWnd::CenterOnTech(const std::string& tech_name) {
    // ensure tech exists and is visible
    const Tech* tech = ::GetTech(tech_name);
    if (!tech) return;
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
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

void TechTreeWnd::ShowPedia() {
    m_enc_detail_panel->Refresh();
    m_enc_detail_panel->Show();

    OptionsDB& db = GetOptionsDB();
    db.Set("UI.windows." + RES_PEDIA_WND_NAME + ".persistently-hidden", false);
}

void TechTreeWnd::HidePedia() {
    m_enc_detail_panel->Hide();

    OptionsDB& db = GetOptionsDB();
    db.Set("UI.windows." + RES_PEDIA_WND_NAME + ".persistently-hidden", true);
}

void TechTreeWnd::TogglePedia() {
    if (!m_enc_detail_panel->Visible())
        ShowPedia();
    else
        HidePedia();
}

bool TechTreeWnd::PediaVisible()
{ return m_enc_detail_panel->Visible(); }

void TechTreeWnd::TechBrowsedSlot(const std::string& tech_name)
{ TechBrowsedSignal(tech_name); }

void TechTreeWnd::TechLeftClickedSlot(const std::string& tech_name,
                                  const GG::Flags<GG::ModKey>& modkeys)
{
    if (modkeys & GG::MOD_KEY_SHIFT) {
        TechDoubleClickedSlot(tech_name, modkeys);
    } else {
        SetEncyclopediaTech(tech_name);
        TechSelectedSignal(tech_name);
    }
}

void TechTreeWnd::TechRightClickedSlot(const std::string& tech_name,
                                       const GG::Flags<GG::ModKey>& modkeys)
{}

void TechTreeWnd::TechDoubleClickedSlot(const std::string& tech_name,
                                        const GG::Flags<GG::ModKey>& modkeys)
{
    const Tech* tech = GetTech(tech_name);
    if (!tech) return;
    const Empire* empire = GetEmpire(HumanClientApp::GetApp()->EmpireID());
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

    if (tech_status != TS_UNRESEARCHABLE && tech_status != TS_HAS_RESEARCHED_PREREQ)
        return;

    // if tech can't yet be researched, add any prerequisites it requires (recursively) and then add it
    TechManager& manager = GetTechManager();
    int empire_id = HumanClientApp::GetApp()->EmpireID();
    std::vector<std::string> tech_vec = manager.RecursivePrereqs(tech_name, empire_id);
    tech_vec.push_back(tech_name);
    AddTechsToQueueSignal(tech_vec, queue_pos);
}

void TechTreeWnd::TechPediaDisplaySlot(const std::string& tech_name) {
    SetEncyclopediaTech(tech_name);
    if (!PediaVisible())
        ShowPedia();
}
