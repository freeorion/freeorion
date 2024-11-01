#include "TechTreeWnd.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "CUIDrawUtil.h"
#include "CUIWnd.h"
#include "Sound.h"
#include "IconTextBrowseWnd.h"
#include "EncyclopediaDetailPanel.h"
#include "../client/human/GGHumanClientApp.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/ScopedTimer.h"
#include "../universe/Effects.h"
#include "../universe/Tech.h"
#include "../universe/UnlockableItem.h"
#include "../universe/ValueRef.h"
#include "../Empire/Empire.h"
#include "TechTreeLayout.h"
#include "TechTreeArcs.h"
#include "Hotkeys.h"

#include <GG/GLClientAndServerBuffer.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/StaticGraphic.h>

#include <boost/algorithm/string/trim.hpp>
#include <algorithm>

namespace {
    const std::string RES_PEDIA_WND_NAME = "research.pedia";
    const std::string RES_CONTROLS_WND_NAME = "research.control";

    // command-line options
    void AddOptions(OptionsDB& db) {
        db.Add("ui.research.tree.spacing.horizontal",           UserStringNop("OPTIONS_DB_UI_TECH_LAYOUT_HORZ_SPACING"),
               0.25,                    RangedStepValidator<double>(0.25, 0.25, 4.0));
        db.Add("ui.research.tree.spacing.vertical",             UserStringNop("OPTIONS_DB_UI_TECH_LAYOUT_VERT_SPACING"),
               0.75,                    RangedStepValidator<double>(0.25, 0.25, 4.0));
        db.Add("ui.research.tree.zoom.scale",                   UserStringNop("OPTIONS_DB_UI_TECH_LAYOUT_ZOOM_SCALE"),
               1.0,                     RangedStepValidator<double>(1.0, -25.0, 10.0));
        db.Add("ui.research.control.graphic.size",              UserStringNop("OPTIONS_DB_UI_TECH_CTRL_ICON_SIZE"),
               3.0,                     RangedStepValidator<double>(0.25, 0.5,  12.0));
        db.Add("ui." + RES_PEDIA_WND_NAME + ".hidden.enabled",  UserStringNop("OPTIONS_DB_RESEARCH_PEDIA_HIDDEN"), false);

        // TechListBox::TechRow column widths
        int default_pts = 16;
        db.Add("ui.research.list.column.graphic.width",         UserStringNop("OPTIONS_DB_UI_TECH_LISTBOX_COL_WIDTH_GRAPHIC"),
               default_pts * 3,         StepValidator<int>(1));
        db.Add("ui.research.list.column.name.width",            UserStringNop("OPTIONS_DB_UI_TECH_LISTBOX_COL_WIDTH_NAME"),
               default_pts * 18,        StepValidator<int>(1));
        db.Add("ui.research.list.column.cost.width",            UserStringNop("OPTIONS_DB_UI_TECH_LISTBOX_COL_WIDTH_COST"),
               default_pts * 8,         StepValidator<int>(1));
        db.Add("ui.research.list.column.time.width",            UserStringNop("OPTIONS_DB_UI_TECH_LISTBOX_COL_WIDTH_TIME"),
               default_pts * 6,         StepValidator<int>(1));
        db.Add("ui.research.list.column.category.width",        UserStringNop("OPTIONS_DB_UI_TECH_LISTBOX_COL_WIDTH_CATEGORY"),
               default_pts * 12,        StepValidator<int>(1));
        db.Add("ui.research.list.column.description.width",     UserStringNop("OPTIONS_DB_UI_TECH_LISTBOX_COL_WIDTH_DESCRIPTION"),
               default_pts * 18,        StepValidator<int>(1));

        // Default status for TechTreeControl filter.
        db.Add<bool>("ui.research.status.unresearchable.shown", UserStringNop("OPTIONS_DB_UI_TECH_TREE_STATUS_UNRESEARCHABLE"),         false);
        db.Add<bool>("ui.research.status.partial.shown",        UserStringNop("OPTIONS_DB_UI_TECH_TREE_STATUS_HAS_RESEARCHED_PREREQ"),  true);
        db.Add<bool>("ui.research.status.researchable.shown",   UserStringNop("OPTIONS_DB_UI_TECH_TREE_STATUS_RESEARCHABLE"),           true);
        db.Add<bool>("ui.research.status.completed.shown",      UserStringNop("OPTIONS_DB_UI_TECH_TREE_STATUS_COMPLETED"),              true);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    GG::X TechPanelWidth()
    { return GG::X(ClientUI::Pts()*38); }
    GG::Y TechPanelHeight()
    { return GG::Y(ClientUI::Pts()*6); }

    constexpr double ConstExprPow(double base, int pow) {
        bool invert_at_end = false;
        if (pow == 0) {
            return 1.0;
        } else if (pow < 0) {
            invert_at_end = true;
            pow = -pow;
        }
        double retval = base;
        while (--pow)
            retval *= base;
        return invert_at_end ? 1.0/retval : retval;
    }

    constexpr double ZOOM_STEP_SIZE = 1.12;
    constexpr double MIN_SCALE = ConstExprPow(ZOOM_STEP_SIZE, -25);
    constexpr double MAX_SCALE = ConstExprPow(ZOOM_STEP_SIZE, 10);
    constexpr double INITIAL_SCALE = ConstExprPow(ZOOM_STEP_SIZE, -5);

    constexpr double PI = 3.1415926535897932384626433;

    bool TechVisible(const std::string& tech_name,
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
        if (!categories_shown.contains(tech->Category()))
            return false;

        // check tech status
        const Empire* empire = GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
        if (!empire)
            return true;    // if no empire, techs have no status, so just return true
        if (!statuses_shown.contains(empire->GetTechStatus(tech_name)))
            return false;

        // all tests pass, so tech is visible
        return true;
    }
}

///////////////////////////
//   TechRowBrowseWnd    //
///////////////////////////
std::shared_ptr<GG::BrowseInfoWnd> TechRowBrowseWnd(const std::string& tech_name, int empire_id) {
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    auto empire = context.GetEmpire(empire_id);
    const Tech* tech = GetTech(tech_name);
    if (!tech)
        return nullptr;

    std::string main_text;

    main_text += UserString(tech->Category()) + " - ";
    main_text += UserString(tech->ShortDescription()) + "\n";

    if (empire) {
        TechStatus tech_status = empire->GetTechStatus(tech_name);
        if (!tech->Researchable()) {
            main_text += UserString("TECH_WND_UNRESEARCHABLE") + "\n";

        } else if (tech_status == TechStatus::TS_UNRESEARCHABLE) {
            main_text += UserString("TECH_WND_STATUS_LOCKED") + "\n";

            std::vector<std::string> unresearched_prereqs;
            for (const std::string& prereq : tech->Prerequisites()) {
                TechStatus prereq_status = empire->GetTechStatus(prereq);
                if (prereq_status != TechStatus::TS_COMPLETE)
                    unresearched_prereqs.push_back(prereq);
            }
            if (!unresearched_prereqs.empty()) {
                main_text += UserString("TECH_WND_UNRESEARCHED_PREREQUISITES");
                for (const std::string& prereq : unresearched_prereqs)
                { main_text += UserString(prereq) + "  "; }
                main_text += "\n";
            }

        } else if (tech_status == TechStatus::TS_RESEARCHABLE) {
            main_text += UserString("TECH_WND_STATUS_RESEARCHABLE") + "\n";

        } else if (tech_status == TechStatus::TS_COMPLETE) {
            main_text += UserString("TECH_WND_STATUS_COMPLETED") + "\n";

        } else if (tech_status == TechStatus::TS_HAS_RESEARCHED_PREREQ) {
            main_text += UserString("TECH_WND_STATUS_PARTIAL_UNLOCK") + "\n";
        }

        const ResearchQueue& queue = empire->GetResearchQueue();
        auto queue_it = queue.find(tech_name);
        if (queue_it != queue.end()) {
            main_text += UserString("TECH_WND_ENQUEUED") + "\n";

            float progress = empire->ResearchProgress(tech_name, context);
            float total_cost = tech->ResearchCost(empire_id, context);
            float allocation = queue_it->allocated_rp;
            float max_allocation = tech->PerTurnCost(empire_id, context);

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
            int turns = tech->ResearchTime(empire_id, context);
            float cost = tech->ResearchCost(empire_id, context);
            const std::string& cost_units = UserString("ENC_RP");

            main_text += boost::io::str(FlexibleFormat(UserString("ENC_COST_AND_TURNS_STR"))
                % DoubleToString(cost, 3, false)
                % cost_units
                % turns);
        }

    } else if (tech->Researchable()) {
        int turns = tech->ResearchTime(empire_id, context);
        float cost = tech->ResearchCost(empire_id, context);
        const std::string& cost_units = UserString("ENC_RP");

        main_text += boost::io::str(FlexibleFormat(UserString("ENC_COST_AND_TURNS_STR"))
            % DoubleToString(cost, 3, false)
            % cost_units
            % turns);
    }

    return GG::Wnd::Create<IconTextBrowseWnd>(
        ClientUI::TechIcon(tech_name), UserString(tech_name), main_text);
}


//////////////////////////////////////////////////
// TechTreeWnd::TechTreeControls                //
//////////////////////////////////////////////////
/** A panel of buttons that control how the tech tree is displayed: what
  * categories, statuses and types of techs to show. */
class TechTreeWnd::TechTreeControls : public CUIWnd {
public:
    TechTreeControls(std::string_view config_name = "");
    void CompleteConstruction() override;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Render() override;

    void LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) override;

    /** Set checked value of control for TechStatus @p status to @p state */
    void SetTechStatus(TechStatus status, bool state);

    void RefreshCategoryButtons(const std::set<std::string>& cats);

    boost::signals2::signal<void (std::string, bool)> CategoryCheckedSignal;

private:
    void DoButtonLayout();
    void CategoryButtonCheckedSlot(std::string category, bool check);

    /** These values are determined when doing button layout, and stored.
      * They are later used when rendering separator lines between the groups
      * of buttons */
    int m_buttons_per_row = 1;          // number of buttons that can fit into available horizontal space
    GG::X m_col_offset;                 // horizontal distance between each column of buttons
    GG::Y m_row_offset;                 // vertical distance between each row of buttons

    /** These values are used for rendering separator lines between groups of buttons */
    static constexpr int BUTTON_SEPARATION = 2; // vertical or horizontal sepration between adjacent buttons
    static constexpr int UPPER_LEFT_PAD = 2;    // offset of buttons' position from top left of controls box

    // TODO: replace all the above stored information with a vector of pairs of GG::Pt (or perhaps GG::Rect)
    // This will contain the start and end points of all separator lines that need to be drawn.  This will be
    // calculated by SizeMove, and stored, so that start and end positions don't need to be recalculated each
    // time Render is called.

    std::shared_ptr<GG::StateButton>                        m_view_type_button;
    std::shared_ptr<GG::StateButton>                        m_all_cat_button;
    std::map<std::string, std::shared_ptr<GG::StateButton>> m_cat_buttons;
    std::map<TechStatus, std::shared_ptr<GG::StateButton>>  m_status_buttons;

    friend class TechTreeWnd;               // so TechTreeWnd can access buttons
};

TechTreeWnd::TechTreeControls::TechTreeControls(std::string_view config_name) :
    CUIWnd(UserString("TECH_DISPLAY"), GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | GG::ONTOP, config_name)
{}

void TechTreeWnd::TechTreeControls::CompleteConstruction() {
    const int tooltip_delay = GetOptionsDB().Get<int>("ui.tooltip.delay");
    const boost::filesystem::path icon_dir = ClientUI::ArtDir() / "icons" / "tech" / "controls";

    GG::Clr icon_color = GG::Clr{113, 150, 182, 255};
    // and one for "ALL"
    m_all_cat_button = GG::Wnd::Create<GG::StateButton>(
        "", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
        std::make_shared<CUIIconButtonRepresenter>(std::make_shared<GG::SubTexture>(
            ClientUI::GetTexture(icon_dir / "00_all_cats.png", true)), icon_color));
    m_all_cat_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString("ALL"), ""));
    m_all_cat_button->SetBrowseModeTime(tooltip_delay);
    m_all_cat_button->SetCheck(true);
    AttachChild(m_all_cat_button);

    // create a button for each tech status
    m_status_buttons[TechStatus::TS_UNRESEARCHABLE] = GG::Wnd::Create<GG::StateButton>(
        "", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
         std::make_shared<CUIIconButtonRepresenter>(std::make_shared<GG::SubTexture>(
             ClientUI::GetTexture(icon_dir / "01_locked.png", true)), icon_color));
    m_status_buttons[TechStatus::TS_UNRESEARCHABLE]->SetBrowseInfoWnd(
        GG::Wnd::Create<TextBrowseWnd>(UserString("TECH_WND_STATUS_LOCKED"), ""));
    m_status_buttons[TechStatus::TS_UNRESEARCHABLE]->SetBrowseModeTime(tooltip_delay);
    m_status_buttons[TechStatus::TS_UNRESEARCHABLE]->SetCheck(
        GetOptionsDB().Get<bool>("ui.research.status.unresearchable.shown"));
    AttachChild(m_status_buttons[TechStatus::TS_UNRESEARCHABLE]);

    m_status_buttons[TechStatus::TS_HAS_RESEARCHED_PREREQ] = GG::Wnd::Create<GG::StateButton>(
        "", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
        std::make_shared<CUIIconButtonRepresenter>(std::make_shared<GG::SubTexture>(
            ClientUI::GetTexture(icon_dir / "02_partial.png", true)), icon_color));
    m_status_buttons[TechStatus::TS_HAS_RESEARCHED_PREREQ]->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString("TECH_WND_STATUS_PARTIAL_UNLOCK"), ""));
    m_status_buttons[TechStatus::TS_HAS_RESEARCHED_PREREQ]->SetBrowseModeTime(tooltip_delay);
    m_status_buttons[TechStatus::TS_HAS_RESEARCHED_PREREQ]->SetCheck(
        GetOptionsDB().Get<bool>("ui.research.status.partial.shown"));
    AttachChild(m_status_buttons[TechStatus::TS_HAS_RESEARCHED_PREREQ]);

    m_status_buttons[TechStatus::TS_RESEARCHABLE] = GG::Wnd::Create<GG::StateButton>(
        "", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
        std::make_shared<CUIIconButtonRepresenter>(std::make_shared<GG::SubTexture>(
            ClientUI::GetTexture(icon_dir / "03_unlocked.png", true)), icon_color));
    m_status_buttons[TechStatus::TS_RESEARCHABLE]->SetBrowseInfoWnd(
        GG::Wnd::Create<TextBrowseWnd>(UserString("TECH_WND_STATUS_RESEARCHABLE"), ""));
    m_status_buttons[TechStatus::TS_RESEARCHABLE]->SetBrowseModeTime(tooltip_delay);
    m_status_buttons[TechStatus::TS_RESEARCHABLE]->SetCheck(
        GetOptionsDB().Get<bool>("ui.research.status.researchable.shown"));
    AttachChild(m_status_buttons[TechStatus::TS_RESEARCHABLE]);

    m_status_buttons[TechStatus::TS_COMPLETE] = GG::Wnd::Create<GG::StateButton>(
        "", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
        std::make_shared<CUIIconButtonRepresenter>(std::make_shared<GG::SubTexture>(
            ClientUI::GetTexture(icon_dir / "04_completed.png", true)), icon_color));
    m_status_buttons[TechStatus::TS_COMPLETE]->SetBrowseInfoWnd(
        GG::Wnd::Create<TextBrowseWnd>(UserString("TECH_WND_STATUS_COMPLETED"), ""));
    m_status_buttons[TechStatus::TS_COMPLETE]->SetBrowseModeTime(tooltip_delay);
    m_status_buttons[TechStatus::TS_COMPLETE]->SetCheck(
        GetOptionsDB().Get<bool>("ui.research.status.completed.shown"));
    AttachChild(m_status_buttons[TechStatus::TS_COMPLETE]);

    // create button to switch between tree and list views
    m_view_type_button = GG::Wnd::Create<GG::StateButton>(
        "", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
        std::make_shared<CUIIconButtonRepresenter>(std::make_shared<GG::SubTexture>(
            ClientUI::GetTexture(icon_dir / "06_view_tree.png", true)),
        icon_color,
        std::make_shared<GG::SubTexture>(ClientUI::GetTexture(icon_dir / "05_view_list.png", true)),
        GG::Clr{110, 172, 150, 255}));
    m_view_type_button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString("TECH_WND_VIEW_TYPE"), ""));
    m_view_type_button->SetBrowseModeTime(tooltip_delay);
    m_view_type_button->SetCheck(false);
    AttachChild(m_view_type_button);

    SetChildClippingMode(ChildClippingMode::ClipToClient);

    CUIWnd::CompleteConstruction();

    SaveDefaultedOptions();
    SaveOptions();
}

void TechTreeWnd::TechTreeControls::RefreshCategoryButtons(const std::set<std::string>& cats_shown) {
    for (auto& button : m_cat_buttons)
        DetachChildAndReset(button.second);
    m_cat_buttons.clear();

    const int tooltip_delay = GetOptionsDB().Get<int>("ui.tooltip.delay");

    // create a button for each tech category...
    for (const auto& cat_view : GetTechManager().CategoryNames()) {
        std::string category{cat_view};
        auto& button = m_cat_buttons[category];

        const GG::Clr icon_clr = ClientUI::CategoryColor(category);
        auto icon = std::make_shared<GG::SubTexture>(ClientUI::CategoryIcon(category));
        button = GG::Wnd::Create<GG::StateButton>(
            "", ClientUI::GetFont(), GG::FORMAT_NONE, GG::CLR_ZERO,
            std::make_shared<CUIIconButtonRepresenter>(std::move(icon), icon_clr));

        button->SetBrowseInfoWnd(GG::Wnd::Create<TextBrowseWnd>(UserString(category), ""));
        button->SetBrowseModeTime(tooltip_delay);

        AttachChild(button);

        button->SetCheck(cats_shown.contains(category));

        button->CheckedSignal.connect(
            boost::bind(&TechTreeControls::CategoryButtonCheckedSlot, this, std::move(category), boost::placeholders::_1));
    }

    DoButtonLayout();
}

void TechTreeWnd::TechTreeControls::CategoryButtonCheckedSlot(std::string category, bool check)
{ CategoryCheckedSignal(std::move(category), check); }

void TechTreeWnd::TechTreeControls::DoButtonLayout() {
    const int PTS = ClientUI::Pts();
    const GG::X RIGHT_EDGE_PAD{PTS / 3};
    const GG::X USABLE_WIDTH = std::max(ClientWidth() - RIGHT_EDGE_PAD, GG::X1);   // space in which to do layout
    const GG::X BUTTON_WIDTH = GG::X{static_cast<int>(
        PTS * std::max(GetOptionsDB().Get<double>("ui.research.control.graphic.size"), 0.5))};
    const GG::Y BUTTON_HEIGHT = GG::Y{Value(BUTTON_WIDTH)};

    m_col_offset = BUTTON_WIDTH + BUTTON_SEPARATION;    // horizontal distance between each column of buttons
    m_row_offset = BUTTON_HEIGHT + BUTTON_SEPARATION;   // vertical distance between each row of buttons
    m_buttons_per_row = std::max(1, USABLE_WIDTH / m_col_offset);

    static constexpr int NUM_NON_CATEGORY_BUTTONS = 6;  //  ALL, Locked, Partial, Unlocked, Complete, ViewType

    // place category buttons: fill each row completely before starting next row
    int row = 0, col = -1;
    for (auto& cat_button : m_cat_buttons) {
         ++col;
        if (col >= m_buttons_per_row) {
            ++row;
            col = 0;
        }
        GG::Pt ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        cat_button.second->SizeMove(ul, lr);
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
    for (auto& status_button : m_status_buttons) {
        ++col;
        if (col >= m_buttons_per_row) {
            ++row;
            col = 0;
        }
        GG::Pt ul(UPPER_LEFT_PAD + col*m_col_offset, UPPER_LEFT_PAD + row*m_row_offset);
        GG::Pt lr = ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);
        status_button.second->SizeMove(ul, lr);
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

void TechTreeWnd::TechTreeControls::SizeMove(GG::Pt ul, GG::Pt lr) {
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

void TechTreeWnd::TechTreeControls::LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) {
    if (m_drag_offset != GG::Pt(-GG::X1, -GG::Y1)) {  // resize-dragging
        GG::Pt new_lr = pt - m_drag_offset;

        new_lr.y = Bottom();    // ignore y-resizes

        // constrain to within parent
        if (auto&& parent = Parent()) {
            GG::Pt max_lr = parent->ClientLowerRight();
            new_lr.x = std::min(new_lr.x, max_lr.x);
        }

        Resize(new_lr - UpperLeft());
    } else {    // normal-dragging
        GG::Pt final_move = move;

        if (auto&& parent = Parent()) {
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

void TechTreeWnd::TechTreeControls::SetTechStatus(TechStatus status, bool state) {
    auto control_it = m_status_buttons.find(status);
    if (control_it == m_status_buttons.end()) {
        WarnLogger() << "No control for status " << status << " found";
        return;
    }

    control_it->second->SetCheck(state);
}


//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel                     //
//////////////////////////////////////////////////
/** The window that contains the actual tech panels and dependency arcs. */
class TechTreeWnd::LayoutPanel : public GG::Wnd {
public:
    LayoutPanel(GG::X w, GG::Y h);

    void CompleteConstruction() override;

    GG::Pt ClientLowerRight() const override;

    double      Scale() const noexcept { return m_scale; }
    const auto& GetCategoriesShown() const noexcept { return m_categories_shown; }
    const auto& GetTechStatusesShown() const noexcept { return m_tech_statuses_shown; }

    mutable TechTreeWnd::TechClickSignalType    TechSelectedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechDoubleClickedSignal;
    mutable TechTreeWnd::TechSignalType         TechPediaDisplaySignal;

    void Render() override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void Update();
    void Clear();   ///< remove all tech panels
    void Reset();   ///< redo layout, recentre on a tech
    void SetScale(double scale);
    void ShowCategory(std::string category);
    void ShowAllCategories();
    void HideCategory(const std::string& category);
    void HideAllCategories();
    void ShowStatus(TechStatus status);
    void HideStatus(TechStatus status);
    void SelectTech(std::string tech_name);
    void CenterOnTech(const std::string& tech_name);
    void DoZoom(GG::Pt pt) const;
    void UndoZoom() const;

    // Converts between screen coordinates and virtual coordiantes
    // doing the inverse or same transformation as DoZoom does with gl calls
    GG::Pt ConvertPtScreenToZoomed(GG::Pt pt) const;
    GG::Pt ConvertPtZoomedToScreen(GG::Pt pt) const;

private:
    class TechPanel;

    class LayoutSurface : public GG::Wnd {
    public:
        LayoutSurface() :
            Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE | GG::DRAGABLE)
        {}

        void LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) override
        { DraggedSignal(move); }

        void LButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override
        { ButtonDownSignal(pt); }

        void LButtonUp(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override
        { ButtonUpSignal(pt); }

        void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override
        { ZoomedSignal(move); }

        mutable boost::signals2::signal<void (int)>    ZoomedSignal;
        mutable boost::signals2::signal<void (GG::Pt)> DraggedSignal;
        mutable boost::signals2::signal<void (GG::Pt)> ButtonDownSignal;
        mutable boost::signals2::signal<void (GG::Pt)> ButtonUpSignal;
    };

    void Layout(bool keep_position);    // lays out tech panels

    void DoLayout();    // arranges child controls (scrolls, buttons) to account for window size

    void ScrolledSlot(int, int, int, int);

    void TreeDraggedSlot(GG::Pt move);
    void TreeDragBegin(GG::Pt move);
    void TreeDragEnd(GG::Pt move);
    void TreeZoomedSlot(int move);
    bool TreeZoomInKeyboard();
    bool TreeZoomOutKeyboard();
    void ConnectKeyboardAcceleratorSignals();

    double                  m_scale = INITIAL_SCALE;
    std::set<std::string>   m_categories_shown;
    std::set<TechStatus>    m_tech_statuses_shown;
    std::string             m_selected_tech_name;
    std::string             m_browsed_tech_name;
    TechTreeLayout          m_graph;

    std::map<std::string, std::shared_ptr<TechPanel>> m_techs;
    TechTreeArcs                    m_dependency_arcs;

    std::shared_ptr<LayoutSurface>  m_layout_surface;
    std::shared_ptr<GG::Scroll>     m_vscroll;
    std::shared_ptr<GG::Scroll>     m_hscroll;
    double                          m_scroll_position_x = 0.0;      //actual scroll position
    double                          m_scroll_position_y = 0.0;
    double                          m_drag_scroll_position_x = 0.0; //position when drag started
    double                          m_drag_scroll_position_y = 0.0;
    std::shared_ptr<GG::Button>     m_zoom_in_button;
    std::shared_ptr<GG::Button>     m_zoom_out_button;
};


//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel::TechPanel          //
//////////////////////////////////////////////////
/** Represents a single tech in the LayoutPanel. */
class TechTreeWnd::LayoutPanel::TechPanel : public GG::Wnd {
public:
    TechPanel(const std::string& tech_name, const TechTreeWnd::LayoutPanel* panel);
    void CompleteConstruction() override;

    bool InWindow(GG::Pt pt) const override;

    /** Update layout and format only if required.*/
    void PreRender() override;
    void Render() override;

    void LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) override
    { ForwardEventToParent(); }

    void LButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override
    { ForwardEventToParent(); }

    void LButtonUp(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override
    { ForwardEventToParent(); }

    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;

    void MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys) override
    { ForwardEventToParent(); }

    void Update();
    void Select(bool select);
    int  FontSize() const;

    mutable TechTreeWnd::TechClickSignalType    TechLeftClickedSignal;
    mutable TechTreeWnd::TechClickSignalType    TechDoubleClickedSignal;
    mutable TechTreeWnd::TechSignalType         TechPediaDisplaySignal;

private:
    GG::GL2DVertexBuffer            m_border_buffer;
    GG::GL2DVertexBuffer            m_eta_border_buffer;
    GG::GL2DVertexBuffer            m_enqueued_indicator_buffer;

    const std::string&              m_tech_name;
    std::string                     m_name_text;
    std::string                     m_cost_and_duration_text;
    std::string                     m_eta_text;
    const TechTreeWnd::LayoutPanel* m_layout_panel;
    std::shared_ptr<GG::StaticGraphic>              m_icon;
    std::vector<std::shared_ptr<GG::StaticGraphic>> m_unlock_icons;
    std::shared_ptr<GG::TextControl>                m_name_label;
    std::shared_ptr<GG::TextControl>                m_cost_and_duration_label;
    std::shared_ptr<GG::TextControl>                m_eta_label;
    GG::Clr                         m_colour = GG::CLR_GRAY;
    TechStatus                      m_status = TechStatus::TS_RESEARCHABLE;
    bool                            m_browse_highlight = false;
    bool                            m_selected = false;
    int                             m_eta = -1;
    bool                            m_enqueued = false;
};

TechTreeWnd::LayoutPanel::TechPanel::TechPanel(const std::string& tech_name, const LayoutPanel* panel) :
    GG::Wnd(GG::X0, GG::Y0, TechPanelWidth(), TechPanelHeight(), GG::INTERACTIVE),
    m_tech_name(tech_name),
    m_layout_panel(panel)
{
    const int GRAPHIC_SIZE = Value(TechPanelHeight());
    m_icon = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::TechIcon(m_tech_name), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
    m_icon->Resize(GG::Pt(GG::X(GRAPHIC_SIZE), GG::Y(GRAPHIC_SIZE)));

    m_name_label = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(FontSize()), ClientUI::TextColor(), GG::FORMAT_WORDBREAK | GG::FORMAT_VCENTER | GG::FORMAT_LEFT);
    m_cost_and_duration_label = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(FontSize()), ClientUI::TextColor(), GG::FORMAT_VCENTER | GG::FORMAT_RIGHT);
    m_eta_label = GG::Wnd::Create<GG::TextControl>(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(FontSize()),ClientUI::TextColor());

    // intentionally not attaching as child; TechPanel::Render the child Render() function instead.

    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
}

void TechTreeWnd::LayoutPanel::TechPanel::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    Update();
}

int TechTreeWnd::LayoutPanel::TechPanel::FontSize() const
{ return ClientUI::Pts() * 3 / 2; }

bool TechTreeWnd::LayoutPanel::TechPanel::InWindow(GG::Pt pt) const {
    const GG::Pt p = m_layout_panel->ConvertPtScreenToZoomed(pt) - UpperLeft();
    if (m_icon->InWindow(p))
        return true;
    return GG::Pt0 <= p && p < GG::Pt(TechPanelWidth(), TechPanelHeight());
}

void TechTreeWnd::LayoutPanel::TechPanel::PreRender() {
    GG::Wnd::PreRender();

    static constexpr int PAD = 8;
    GG::X text_left(GG::X(Value(TechPanelHeight())) + PAD);
    GG::Y text_top(GG::Y0);
    GG::X text_width(TechPanelWidth() - text_left);
    GG::Y text_height(TechPanelHeight()/2);

    GG::Pt ul = GG::Pt(text_left, text_top);
    GG::Pt lr = ul + GG::Pt(text_width + PAD, TechPanelHeight());

    // size of tech name text
    int font_pts = static_cast<int>(FontSize() * m_layout_panel->Scale() + 0.5);

    if (font_pts > 6) {
        if (font_pts < 10) {
            m_name_label->SetText(m_name_text);
            m_cost_and_duration_label->SetText(m_cost_and_duration_text);
        } else {
            m_name_label->SetText("<s>" + m_name_text + "</s>");
            m_cost_and_duration_label->SetText("<s>" + m_cost_and_duration_text + "</s>");
        }

        GG::Pt text_ul(text_left + PAD/2, text_top);
        GG::Pt text_size(text_width + PAD, m_unlock_icons.empty() ? text_height*2 : text_height - PAD/2);
        m_name_label->SizeMove(text_ul, text_ul + text_size);

        // show cost and duration for unresearched techs
        const Empire* empire = GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
        if (empire) {
            if (empire->TechResearched(m_tech_name))
                m_cost_and_duration_label->Hide();
            else {
                GG::Pt text_ll(text_left + PAD / 2, TechPanelHeight() - font_pts - PAD);
                text_size = GG::Pt(text_width + PAD / 2, GG::Y(font_pts));
                m_cost_and_duration_label->SizeMove(text_ll, text_ll + text_size);
                m_cost_and_duration_label->Show();
            }
        }

        // ETA background and text
        if (m_eta != -1 && font_pts > 10) {
            GG::Pt panel_size = lr - ul;
            GG::Pt eta_ul = ul + GG::Pt(panel_size.x * 3 / 4, panel_size.y * 3 / 4) - GG::Pt(GG::X(2), GG::Y(2));
            GG::Pt eta_lr = eta_ul + GG::Pt(panel_size.x / 2, panel_size.y / 2) + GG::Pt(GG::X(2), GG::Y(2));

            m_eta_label->SizeMove(eta_ul, eta_lr);
        }
    }
}

void TechTreeWnd::LayoutPanel::TechPanel::Render() {
    static constexpr int PAD = 8;
    GG::X text_left(GG::X(Value(TechPanelHeight())) + PAD);
    GG::Y text_top(GG::Y0);
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
        else if (m_status == TechStatus::TS_COMPLETE || m_status == TechStatus::TS_RESEARCHABLE) {
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

        GG::Pt text_ul(text_left + PAD/2, text_top);
        GG::Pt text_size(text_width + PAD, m_unlock_icons.empty() ? text_height*2 : text_height - PAD/2);
        m_name_label->SizeMove(text_ul, text_ul + text_size);

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

    for (auto& icon : m_unlock_icons)
    { icon->Render(); }

    m_layout_panel->UndoZoom();
}

void TechTreeWnd::LayoutPanel::TechPanel::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (m_layout_panel->m_selected_tech_name != m_tech_name)
        TechLeftClickedSignal(m_tech_name, mod_keys);
}

void TechTreeWnd::LayoutPanel::TechPanel::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto dclick_action = [this, pt]() { LDoubleClick(pt, GG::Flags<GG::ModKey>()); };
    auto ctrl_dclick_action = [this, pt]() { LDoubleClick(pt, GG::MOD_KEY_CTRL); };
    auto pedia_display_action = [this]() { TechPediaDisplaySignal(m_tech_name); };

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    if (!(m_enqueued) && !(m_status == TechStatus::TS_COMPLETE)) {
        popup->AddMenuItem(GG::MenuItem(UserString("PRODUCTION_DETAIL_ADD_TO_QUEUE"),   false, false, dclick_action));
        popup->AddMenuItem(GG::MenuItem(UserString("PRODUCTION_DETAIL_ADD_TO_TOP_OF_QUEUE"), false, false,
                                        ctrl_dclick_action));
    }

    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(m_tech_name));
    popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false, pedia_display_action));
    popup->Run();
}

void TechTreeWnd::LayoutPanel::TechPanel::LDoubleClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ TechDoubleClickedSignal(m_tech_name, mod_keys); }

void TechTreeWnd::LayoutPanel::TechPanel::MouseEnter(GG::Pt, GG::Flags<GG::ModKey>)
{ m_browse_highlight = true; }

void TechTreeWnd::LayoutPanel::TechPanel::MouseLeave()
{ m_browse_highlight = false; }

void TechTreeWnd::LayoutPanel::TechPanel::Select(bool select)
{ m_selected = select; }

void TechTreeWnd::LayoutPanel::TechPanel::Update() {
    Select(m_layout_panel->m_selected_tech_name == m_tech_name);

    const int client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
    const ScriptingContext& context = IApp::GetApp()->GetContext();

    if (auto empire = context.GetEmpire(client_empire_id)) {
        m_status = empire->GetTechStatus(m_tech_name);
        m_enqueued = empire->GetResearchQueue().InQueue(m_tech_name);

        const ResearchQueue& queue = empire->GetResearchQueue();
        auto queue_it = queue.find(m_tech_name);
        if (queue_it != queue.end()) {
            m_eta = queue_it->turns_left;
            if (m_eta != -1)
                m_eta_text = std::to_string(m_eta);
            else
                m_eta_text.clear();
        }
    }

    GG::Clr icon_colour = GG::CLR_WHITE;
    if (const Tech* tech = GetTech(m_tech_name)) {
        m_colour = ClientUI::CategoryColor(tech->Category());
        icon_colour = m_colour;

        if (m_status == TechStatus::TS_UNRESEARCHABLE || m_status == TechStatus::TS_HAS_RESEARCHED_PREREQ) {
            icon_colour = GG::CLR_GRAY;
            m_colour.a = 64;
        } else if (m_status == TechStatus::TS_RESEARCHABLE) {
            icon_colour = GG::CLR_GRAY;
            m_colour.a = 128;
        } else {
            m_colour.a = 255;
        }

        if (m_unlock_icons.empty()) {
            static constexpr int PAD = 8;
            GG::X icon_left(GG::X(Value(TechPanelHeight())) + PAD*3/2);
            GG::Y icon_height = TechPanelHeight()/2;
            GG::X icon_width = GG::X(Value(icon_height));
            GG::Y icon_bottom = TechPanelHeight() - PAD/2;
            GG::Y icon_top = icon_bottom - icon_height;

            // add icons for unlocked items
            for (const UnlockableItem& item : tech->UnlockedItems()) {
                std::shared_ptr<GG::Texture> texture;
                switch (item.type) {
                case UnlockableItemType::UIT_BUILDING:  texture = ClientUI::BuildingIcon(item.name);    break;
                case UnlockableItemType::UIT_SHIP_PART: texture = ClientUI::PartIcon(item.name);        break;
                case UnlockableItemType::UIT_SHIP_HULL: texture = ClientUI::HullIcon(item.name);        break;
                case UnlockableItemType::UIT_POLICY:    texture = ClientUI::PolicyIcon(item.name);      break;
                case UnlockableItemType::UIT_TECH:
                case UnlockableItemType::UIT_SHIP_DESIGN:
                default: break;
                }

                if (texture) {
                    auto graphic = GG::Wnd::Create<GG::StaticGraphic>(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
                    m_unlock_icons.push_back(graphic);
                    graphic->SizeMove(GG::Pt(icon_left, icon_top), GG::Pt(icon_left + icon_width, icon_top + icon_height));
                    icon_left += icon_width + PAD;
                }
            }
            // add icons for modified part meters / specials
            std::set<MeterType> meters_affected;
            std::set<std::string> specials_affected;
            std::set<std::string> parts_whose_meters_are_affected;
            for (const auto& effects_group : tech->Effects()) {
                for (const auto& effect : effects_group.Effects()) {
                    if (auto set_meter_effect = dynamic_cast<const Effect::SetMeter*>(effect.get())) {
                        meters_affected.insert(set_meter_effect->GetMeterType());

                    } else if (auto set_ship_part_meter_effect = dynamic_cast<const Effect::SetShipPartMeter*>(effect.get())) {
                        auto part_name = set_ship_part_meter_effect->GetPartName();
                        if (part_name && part_name->ConstantExpr())
                            parts_whose_meters_are_affected.insert(part_name->Eval());

                    } else if (auto add_special_effect = dynamic_cast<const Effect::AddSpecial*>(effect.get())) {
                        auto special_name = add_special_effect->GetSpecialName();
                        if (special_name && special_name->ConstantExpr())
                            specials_affected.insert(special_name->Eval());
                    }
                }
            }

            for (const std::string& part_name : parts_whose_meters_are_affected) {
                std::shared_ptr<GG::Texture> texture = ClientUI::PartIcon(part_name);
                if (texture) {
                    auto graphic = GG::Wnd::Create<GG::StaticGraphic>(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
                    m_unlock_icons.push_back(graphic);
                    graphic->SizeMove(GG::Pt(icon_left, icon_top), GG::Pt(icon_left + icon_width, icon_top + icon_height));
                    icon_left += icon_width + PAD;
                }
            }
            for (MeterType meter_type : meters_affected) {
                auto texture = ClientUI::MeterIcon(meter_type);
                if (texture) {
                    auto graphic = GG::Wnd::Create<GG::StaticGraphic>(std::move(texture),
                                                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
                    m_unlock_icons.push_back(graphic);
                    graphic->SizeMove(GG::Pt(icon_left, icon_top), GG::Pt(icon_left + icon_width, icon_top + icon_height));
                    icon_left += icon_width + PAD;
                }
            }

            for (const std::string& special_name : specials_affected) {
                auto texture = ClientUI::SpecialIcon(special_name);
                if (texture) {
                    auto graphic = GG::Wnd::Create<GG::StaticGraphic>(std::move(texture),
                                                                      GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
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
                                                  % DoubleToString(tech->ResearchCost(client_empire_id, context), 1, false)
                                                  % tech->ResearchTime(client_empire_id, context));
    m_cost_and_duration_label->SetText("<s>" + m_cost_and_duration_text + "<s>");

    m_eta_label->SetText("<s>" + m_eta_text + "</s>");

    ClearBrowseInfoWnd();
    SetBrowseInfoWnd(TechRowBrowseWnd(m_tech_name, client_empire_id));

    RequirePreRender();
}


//////////////////////////////////////////////////
// TechTreeWnd::LayoutPanel                     //
//////////////////////////////////////////////////
TechTreeWnd::LayoutPanel::LayoutPanel(GG::X w, GG::Y h) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE)
{}

void TechTreeWnd::LayoutPanel::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    SetChildClippingMode(ChildClippingMode::ClipToClient);

    m_scale = std::pow(ZOOM_STEP_SIZE, GetOptionsDB().Get<double>("ui.research.tree.zoom.scale")); // (LATHANDA) Initialise Fullzoom and do real zooming using GL. TODO: Check best size

    m_layout_surface = GG::Wnd::Create<LayoutSurface>();

    m_vscroll = GG::Wnd::Create<CUIScroll>(GG::Orientation::VERTICAL);
    m_hscroll = GG::Wnd::Create<CUIScroll>(GG::Orientation::HORIZONTAL);

    m_zoom_in_button = Wnd::Create<CUIButton>("+");
    m_zoom_in_button->SetColor(ClientUI::WndColor());
    m_zoom_out_button = Wnd::Create<CUIButton>("-");
    m_zoom_out_button->SetColor(ClientUI::WndColor());

    DoLayout();

    AttachChild(m_layout_surface);
    AttachChild(m_vscroll);
    AttachChild(m_hscroll);
    AttachChild(m_zoom_in_button);
    AttachChild(m_zoom_out_button);

    using boost::placeholders::_1;
    using boost::placeholders::_2;
    using boost::placeholders::_3;
    using boost::placeholders::_4;

    m_layout_surface->DraggedSignal.connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeDraggedSlot, this, _1));
    m_layout_surface->ButtonUpSignal.connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeDragEnd, this, _1));
    m_layout_surface->ButtonDownSignal.connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeDragBegin, this, _1));
    m_layout_surface->ZoomedSignal.connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeZoomedSlot, this, _1));
    m_vscroll->ScrolledSignal.connect(boost::bind(&TechTreeWnd::LayoutPanel::ScrolledSlot, this, _1, _2, _3, _4));
    m_hscroll->ScrolledSignal.connect(boost::bind(&TechTreeWnd::LayoutPanel::ScrolledSlot, this, _1, _2, _3, _4));
    m_zoom_in_button->LeftClickedSignal.connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeZoomedSlot, this, 1));
    m_zoom_out_button->LeftClickedSignal.connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeZoomedSlot, this, -1));

    ConnectKeyboardAcceleratorSignals();

    // show all categories...
    m_categories_shown.clear();
    for (const auto& category_name : GetTechManager().CategoryNames())
        m_categories_shown.insert(std::string{category_name});

    // show statuses
    m_tech_statuses_shown.clear();
    //m_tech_statuses_shown.insert(TS_UNRESEARCHABLE);
    m_tech_statuses_shown.insert(TechStatus::TS_RESEARCHABLE);
    m_tech_statuses_shown.insert(TechStatus::TS_COMPLETE);
}

void TechTreeWnd::LayoutPanel::ConnectKeyboardAcceleratorSignals() {
    auto& hkm = HotkeyManager::GetManager();

    hkm.Connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeZoomInKeyboard, this), "ui.zoom.in",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeZoomInKeyboard, this), "ui.zoom.in.alt",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeZoomOutKeyboard, this), "ui.zoom.out",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));
    hkm.Connect(boost::bind(&TechTreeWnd::LayoutPanel::TreeZoomOutKeyboard, this), "ui.zoom.out.alt",
                AndCondition(VisibleWindowCondition(this), NoModalWndsOpenCondition));

    hkm.RebuildShortcuts();
}

GG::Pt TechTreeWnd::LayoutPanel::ClientLowerRight() const
{ return LowerRight() - GG::Pt(GG::X(ClientUI::ScrollWidth()), GG::Y(ClientUI::ScrollWidth())); }

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

void TechTreeWnd::LayoutPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void TechTreeWnd::LayoutPanel::DoLayout() {
    const int SCRLWDTH = ClientUI::ScrollWidth();

    const auto vscroll_ul = GG::Pt(Width() - SCRLWDTH, GG::Y0);
    const auto vscroll_lr = GG::Pt(Width(), Height() - SCRLWDTH);
    m_vscroll->SizeMove(vscroll_ul, vscroll_lr);

    const auto hscroll_ul = GG::Pt(GG::X0, Height() - SCRLWDTH);
    const auto hscroll_lr = GG::Pt(Width() - SCRLWDTH, Height());
    m_hscroll->SizeMove(hscroll_ul, hscroll_lr);

    const GG::X ZBSIZE{ClientUI::ScrollWidth() * 2};
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
    for (const auto& tech_panel: m_techs)
        m_layout_surface->DetachChild(tech_panel.second);
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
    GetOptionsDB().Set<double>("ui.research.tree.zoom.scale", std::floor(0.1 + (std::log(m_scale) / std::log(ZOOM_STEP_SIZE))));

    for (auto& entry : m_techs)
        entry.second->RequirePreRender();
}

void TechTreeWnd::LayoutPanel::ShowCategory(std::string category) {
    if (m_categories_shown.emplace(std::move(category)).second)
        Layout(true);
}

void TechTreeWnd::LayoutPanel::ShowAllCategories() {
    const auto all_cats = GetTechManager().CategoryNames();
    if (all_cats.size() == m_categories_shown.size())
        return;
    for (const auto& category_name : all_cats)
        m_categories_shown.insert(std::string{category_name});
    Layout(true);
}

void TechTreeWnd::LayoutPanel::HideCategory(const std::string& category) {
    auto removed_count = m_categories_shown.erase(category);
    if (removed_count > 0)
        Layout(true);
}

void TechTreeWnd::LayoutPanel::HideAllCategories() {
    if (m_categories_shown.empty())
        return;
    m_categories_shown.clear();
    Layout(true);
}

void TechTreeWnd::LayoutPanel::ShowStatus(TechStatus status) {
    if (!m_tech_statuses_shown.contains(status)) {
        m_tech_statuses_shown.insert(status);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::HideStatus(TechStatus status) {
    auto it = m_tech_statuses_shown.find(status);
    if (it != m_tech_statuses_shown.end()) {
        m_tech_statuses_shown.erase(it);
        Layout(true);
    }
}

void TechTreeWnd::LayoutPanel::CenterOnTech(const std::string& tech_name) {
    const auto it = m_techs.find(tech_name);
    if (it == m_techs.end()) {
        DebugLogger() << "TechTreeWnd::LayoutPanel::CenterOnTech couldn't centre on " << tech_name
                               << " due to lack of such a tech panel";
        return;
    }

    auto& tech_panel = it->second;
    GG::Pt center_point = tech_panel->UpperLeft();
    m_hscroll->ScrollTo(Value(center_point.x));
    GG::SignalScroll(*m_hscroll, true);
    m_vscroll->ScrollTo(Value(center_point.y));
    GG::SignalScroll(*m_vscroll, true);
}

void TechTreeWnd::LayoutPanel::DoZoom(GG::Pt pt) const {
    glPushMatrix();
    //center to panel
    glTranslated(Width()/2.0, Height()/2.0, 0.0);
    //zoom
    glScaled(m_scale, m_scale, 1);
    //translate to actual scroll position
    glTranslated(-m_scroll_position_x, -m_scroll_position_y, 0.0);
    glTranslated(Value(pt.x), Value(pt.y), 0.0);
}

void TechTreeWnd::LayoutPanel::UndoZoom() const
{ glPopMatrix(); }

GG::Pt TechTreeWnd::LayoutPanel::ConvertPtScreenToZoomed(GG::Pt pt) const {
    double x = Value(pt.x);
    double y = Value(pt.y);
    x -= Width()/2.0;
    y -= Height()/2.0;
    x /= m_scale;
    y /= m_scale;
    x += m_scroll_position_x;
    y += m_scroll_position_y;
    return GG::Pt(GG::ToX(x), GG::ToY(y));
}

GG::Pt TechTreeWnd::LayoutPanel::ConvertPtZoomedToScreen(GG::Pt pt) const {
    double x = Value(pt.x);
    double y = Value(pt.y);
    x -= m_scroll_position_x;
    y -= m_scroll_position_y;
    x *= m_scale;
    y *= m_scale;
    x += Width()/2.0;
    y += Height()/2.0;
    return GG::Pt(GG::ToX(x), GG::ToY(y));
}

void TechTreeWnd::LayoutPanel::Layout(bool keep_position) {
    //const GG::X TECH_PANEL_MARGIN_X{ClientUI::Pts()*16};
    //const GG::Y TECH_PANEL_MARGIN_Y{ClientUI::Pts()*16 + 100};
    const double RANK_SEP = Value(TechPanelWidth()) * GetOptionsDB().Get<double>("ui.research.tree.spacing.horizontal");
    const double NODE_SEP = Value(TechPanelHeight()) * GetOptionsDB().Get<double>("ui.research.tree.spacing.vertical");
    const double WIDTH = Value(TechPanelWidth());
    const double HEIGHT = Value(TechPanelHeight());
    static constexpr double X_MARGIN{12};

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
    const TechManager& manager = GetTechManager();
    for (const auto& [tech_name, tech] : manager) {
        if (!TechVisible(tech_name, m_categories_shown, m_tech_statuses_shown)) continue;
        m_techs[tech_name] = GG::Wnd::Create<TechPanel>(tech_name, this);
        m_graph.AddNode(tech_name, m_techs[tech_name]->Width(), m_techs[tech_name]->Height());
    }

    // create an edge for every prerequisite
    for (const auto& [tech_name, tech] : manager) {
        if (!TechVisible(tech_name, m_categories_shown, m_tech_statuses_shown)) continue;
        for (const std::string& prereq : tech.Prerequisites()) {
            if (!TechVisible(prereq, m_categories_shown, m_tech_statuses_shown)) continue;
            m_graph.AddEdge(prereq, tech_name);
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
    for (const auto& [tech_name, tech] : manager) {
        if (!TechVisible(tech_name, m_categories_shown, m_tech_statuses_shown)) continue;
        //techpanel
        const TechTreeLayout::Node* node = m_graph.GetNode(tech_name);
        //move TechPanel
        auto& tech_panel = m_techs[tech_name];
        tech_panel->MoveTo(GG::Pt(node->GetX(), node->GetY()));
        m_layout_surface->AttachChild(tech_panel);
        tech_panel->TechLeftClickedSignal.connect(
            boost::bind(&TechTreeWnd::LayoutPanel::SelectTech, this, boost::placeholders::_1));
        tech_panel->TechDoubleClickedSignal.connect(TechDoubleClickedSignal);
        tech_panel->TechPediaDisplaySignal.connect(TechPediaDisplaySignal);

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
        if (m_techs.contains(m_selected_tech_name))
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
        for (const auto& [tech_name, tech] : manager) {
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

void TechTreeWnd::LayoutPanel::SelectTech(std::string tech_name) {
    // deselect previously-selected tech panel
    if (m_techs.contains(m_selected_tech_name))
        m_techs[std::move(m_selected_tech_name)]->Select(false);
    // select clicked on tech
    if (m_techs.contains(tech_name))
        m_techs[tech_name]->Select(true);
    m_selected_tech_name = std::move(tech_name);
    TechSelectedSignal(m_selected_tech_name, GG::Flags<GG::ModKey>());
}

void TechTreeWnd::LayoutPanel::TreeDraggedSlot(GG::Pt move) {
    m_hscroll->ScrollTo(static_cast<int>(m_drag_scroll_position_x - move.x / m_scale));
    m_vscroll->ScrollTo(static_cast<int>(m_drag_scroll_position_y - move.y / m_scale));
    m_scroll_position_x = m_hscroll->PosnRange().first;
    m_scroll_position_y = m_vscroll->PosnRange().first;
}

void TechTreeWnd::LayoutPanel::TreeDragBegin(GG::Pt pt) {
    m_drag_scroll_position_x = m_scroll_position_x;
    m_drag_scroll_position_y = m_scroll_position_y;
}

void TechTreeWnd::LayoutPanel::TreeDragEnd(GG::Pt pt) {
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
    TechListBox(GG::X w, GG::Y h);
    void CompleteConstruction() override;

    bool TechRowCmp(const GG::ListBox::Row& lhs, const GG::ListBox::Row& rhs, std::size_t column);

    void Reset();
    void Update(bool populate = true);
    void ShowCategory(std::string category);
    void ShowAllCategories();
    void HideCategory(const std::string& category);
    void HideAllCategories();
    void ShowStatus(TechStatus status);
    void HideStatus(TechStatus status);

    mutable TechClickSignalType TechLeftClickedSignal;  ///< emitted when a technology is single-left-clicked
    mutable TechClickSignalType TechDoubleClickedSignal;///< emitted when a technology is double-clicked
    mutable TechSignalType      TechPediaDisplaySignal; ///< emitted when requesting a pedia lookup

private:
    class TechRow : public CUIListBox::Row {
    public:
        TechRow(GG::X w, const std::string& tech_name);
        void CompleteConstruction() override;

        void Render() override;

        const std::string&                  GetTech() { return m_tech; }
        static std::vector<GG::X>           ColWidths(GG::X total_width);
        static std::vector<GG::Alignment>   ColAlignments();
        void                                Update();

    private:
        std::string m_tech;
        GG::Clr m_background_color = {{0, 0, 0, 128}};
        bool m_enqueued = false;
    };

    void Populate(bool update = true);
    void TechDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void TechLeftClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void TechRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys);
    void ToggleSortCol(unsigned int col);

    std::set<std::string>                                       m_categories_shown;
    std::set<TechStatus>                                        m_tech_statuses_shown;
    std::unordered_map<std::string, std::shared_ptr<TechRow>>   m_tech_row_cache;
    std::shared_ptr<GG::ListBox::Row>                           m_header_row;
    std::size_t                                                 m_previous_sort_col = 0;
};

void TechTreeWnd::TechListBox::TechRow::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt offset{GG::X(3), GG::Y(3)};
    if (m_enqueued) {
        GG::FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_WHITE, 1);
        GG::FlatRoundedRectangle(ul + offset, lr - offset, m_background_color, GG::CLR_WHITE);
    } else {
        GG::FlatRectangle(ul, lr, m_background_color, GG::CLR_WHITE, 1);
    }
}

std::vector<GG::X> TechTreeWnd::TechListBox::TechRow::ColWidths(GG::X total_width) {
    GG::X graphic_width(GetOptionsDB().Get<GG::X>("ui.research.list.column.graphic.width"));
    GG::X name_width(GetOptionsDB().Get<GG::X>("ui.research.list.column.name.width"));
    GG::X cost_width(GetOptionsDB().Get<GG::X>("ui.research.list.column.cost.width"));
    GG::X time_width(GetOptionsDB().Get<GG::X>("ui.research.list.column.time.width"));
    GG::X category_width(GetOptionsDB().Get<GG::X>("ui.research.list.column.category.width"));

    GG::X cols_width_sum = graphic_width + name_width + cost_width + time_width + category_width;

    GG::X desc_width{std::max(GetOptionsDB().Get<int>("ui.research.list.column.description.width"),
                              Value(total_width - cols_width_sum))};

    return {graphic_width, name_width, cost_width, time_width, category_width, desc_width};
}

std::vector<GG::Alignment> TechTreeWnd::TechListBox::TechRow::ColAlignments() {
    //              graphic,         name,           cost,           time,         category,     description
    return {GG::ALIGN_CENTER, GG::ALIGN_LEFT, GG::ALIGN_RIGHT, GG::ALIGN_RIGHT, GG::ALIGN_LEFT, GG::ALIGN_LEFT};
}

bool TechTreeWnd::TechListBox::TechRowCmp(const GG::ListBox::Row& lhs, const GG::ListBox::Row& rhs, std::size_t column) {
    const std::string lhs_key = boost::trim_copy(lhs.SortKey(column));
    const std::string rhs_key = boost::trim_copy(rhs.SortKey(column));

    // When equal, sort by previous sorted column
    if ((lhs_key == rhs_key) && (m_previous_sort_col != column))
        return TechRowCmp(lhs, rhs, m_previous_sort_col);

    try {  // attempt compare by int
        return boost::lexical_cast<int>(lhs_key) < boost::lexical_cast<int>(rhs_key);
    } catch (...) { // compare as strings
        return GetLocale().operator()(lhs_key, rhs_key);
    }
}

TechTreeWnd::TechListBox::TechRow::TechRow(GG::X w, const std::string& tech_name) :
    CUIListBox::Row(w, GG::Y(ClientUI::Pts() * 2 + 5)),
    m_tech(tech_name),
    m_background_color(ClientUI::WndColor()),
    m_enqueued(false)
{ SetDragDropDataType("TechListBox::TechRow"); }

void TechTreeWnd::TechListBox::TechRow::CompleteConstruction() {
    CUIListBox::Row::CompleteConstruction();

    const Tech* this_row_tech = ::GetTech(m_tech);
    if (!this_row_tech)
        return;
    const ScriptingContext& context = IApp::GetApp()->GetContext();

    const auto col_widths = ColWidths(Width());
    const GG::X GRAPHIC_WIDTH = col_widths.empty() ? GG::X{48} : col_widths.front();
    const GG::Y ICON_HEIGHT{std::min(Value(Height() - 12),
                                     std::max(ClientUI::Pts(), Value(GRAPHIC_WIDTH) - 6))};

    static constexpr auto just_pad = "    "; // TODO: replace string padding with new TextFormat flag
    static constexpr auto graphic_style = GG::GRAPHIC_VCENTER | GG::GRAPHIC_CENTER | GG::GRAPHIC_PROPSCALE | GG::GRAPHIC_FITGRAPHIC;

    auto graphic = GG::Wnd::Create<GG::StaticGraphic>(ClientUI::TechIcon(m_tech), graphic_style);
    graphic->Resize(GG::Pt(GRAPHIC_WIDTH, ICON_HEIGHT));
    graphic->SetColor(ClientUI::CategoryColor(this_row_tech->Category()));
    push_back(std::move(graphic));

    auto text = GG::Wnd::Create<CUILabel>(just_pad + UserString(m_tech), GG::FORMAT_LEFT);
    text->SetResetMinSize(false);
    text->ClipText(true);
    text->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    push_back(std::move(text));

    const std::string cost_str = std::to_string(std::lround(
        this_row_tech->ResearchCost(GGHumanClientApp::GetApp()->EmpireID(), context)));
    text = GG::Wnd::Create<CUILabel>(cost_str + just_pad + just_pad, GG::FORMAT_RIGHT);
    text->SetResetMinSize(false);
    text->ClipText(true);
    text->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    push_back(std::move(text));

    const std::string time_str = std::to_string(
        this_row_tech->ResearchTime(GGHumanClientApp::GetApp()->EmpireID(), context));
    text = GG::Wnd::Create<CUILabel>(time_str + just_pad + just_pad, GG::FORMAT_RIGHT);
    text->SetResetMinSize(false);
    text->ClipText(true);
    text->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    push_back(std::move(text));

    text = GG::Wnd::Create<CUILabel>(just_pad + UserString(this_row_tech->Category()), GG::FORMAT_LEFT);
    text->SetResetMinSize(false);
    text->ClipText(true);
    text->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    push_back(std::move(text));

    text = GG::Wnd::Create<CUILabel>(just_pad + UserString(this_row_tech->ShortDescription()), GG::FORMAT_LEFT);
    text->ClipText(true);
    text->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    push_back(std::move(text));
}

void TechTreeWnd::TechListBox::TechRow::Update() {
    const Tech* this_row_tech = ::GetTech(m_tech);
    if (!this_row_tech || this->size() < 4)
        return;
    // TODO replace string padding with new TextFormat flag
    std::string just_pad = "    ";

    auto client_empire_id = GGHumanClientApp::GetApp()->EmpireID();
    const ScriptingContext& context = IApp::GetApp()->GetContext();
    auto empire = context.GetEmpire(client_empire_id);

    std::string cost_str = std::to_string(std::lround(
        this_row_tech->ResearchCost(client_empire_id, context)));
    if (GG::Button* cost_btn = dynamic_cast<GG::Button*>((size() >= 3) ? at(2) : nullptr))
        cost_btn->SetText(cost_str + just_pad + just_pad);

    std::string time_str = std::to_string(this_row_tech->ResearchTime(client_empire_id, context));
    if (GG::Button* time_btn = dynamic_cast<GG::Button*>((size() >= 4) ? at(3) : nullptr))
        time_btn->SetText(time_str + just_pad + just_pad);

    // Adjust colors for tech status
    auto foreground_color = ClientUI::CategoryColor(this_row_tech->Category());
    auto this_row_status = empire ? empire->GetTechStatus(m_tech) : TechStatus::TS_RESEARCHABLE;
    if (this_row_status == TechStatus::TS_COMPLETE) {
        foreground_color.a = m_background_color.a;  // preserve users 'wnd-color' trasparency
        m_background_color = AdjustBrightness(foreground_color, 0.3);
        foreground_color = ClientUI::TextColor();

    } else if (this_row_status == TechStatus::TS_UNRESEARCHABLE ||
               this_row_status == TechStatus::TS_HAS_RESEARCHED_PREREQ)
    { foreground_color.a = 96; }

    for (std::size_t i = 0; i < size(); ++i)
        at(i)->SetColor(foreground_color);

    if (empire) {
        const ResearchQueue& rq = empire->GetResearchQueue();
        m_enqueued = rq.InQueue(m_tech);
    } else {
        m_enqueued = false;
    }

    ClearBrowseInfoWnd();
    SetBrowseInfoWnd(TechRowBrowseWnd(m_tech, client_empire_id));
}

void TechTreeWnd::TechListBox::ToggleSortCol(unsigned int col) {
    if (SortCol() != col) {
        m_previous_sort_col = SortCol();
        SetSortCol(col);
    } else {  // toggle the sort direction
        SetStyle(Style() ^ GG::LIST_SORTDESCENDING);
    }
}

TechTreeWnd::TechListBox::TechListBox(GG::X w, GG::Y h) :
    CUIListBox()
{ Resize(GG::Pt(w, h)); }

void TechTreeWnd::TechListBox::CompleteConstruction() {
    CUIListBox::CompleteConstruction();

    using boost::placeholders::_1;
    using boost::placeholders::_2;
    using boost::placeholders::_3;

    DoubleClickedRowSignal.connect(boost::bind(&TechListBox::TechDoubleClicked, this, _1, _2, _3));
    LeftClickedRowSignal.connect(boost::bind(&TechListBox::TechLeftClicked, this, _1, _2, _3));
    RightClickedRowSignal.connect(boost::bind(&TechListBox::TechRightClicked, this, _1, _2, _3));

    SetStyle(GG::LIST_NOSEL);

    // show all categories...
    m_categories_shown.clear();
    for (auto& category_name : GetTechManager().CategoryNames())
        m_categories_shown.insert(std::string{category_name});

    // show all statuses except unreasearchable
    m_tech_statuses_shown.clear();
    //m_tech_statuses_shown.insert(TS_UNRESEARCHABLE);
    m_tech_statuses_shown.insert(TechStatus::TS_RESEARCHABLE);
    m_tech_statuses_shown.insert(TechStatus::TS_COMPLETE);

    GG::X row_width = Width() - ClientUI::ScrollWidth() - ClientUI::Pts();
    std::vector<GG::X> col_widths = TechRow::ColWidths(row_width);
    const GG::Y HEIGHT{Value(col_widths[0])};
    m_header_row = GG::Wnd::Create<GG::ListBox::Row>(row_width, HEIGHT);

    auto graphic_col = GG::Wnd::Create<CUILabel>("");  // graphic
    graphic_col->Resize(GG::Pt(col_widths[0], HEIGHT));
    graphic_col->ClipText(true);
    graphic_col->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    m_header_row->push_back(graphic_col);

    auto name_col = Wnd::Create<CUIButton>(UserString("TECH_WND_LIST_COLUMN_NAME"));
    name_col->Resize(GG::Pt(col_widths[1], HEIGHT));
    name_col->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    name_col->LeftClickedSignal.connect([this]() { ToggleSortCol(1); });
    m_header_row->push_back(name_col);

    auto cost_col = Wnd::Create<CUIButton>(UserString("TECH_WND_LIST_COLUMN_COST"));
    cost_col->Resize(GG::Pt(col_widths[2], HEIGHT));
    cost_col->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    cost_col->LeftClickedSignal.connect([this]() { ToggleSortCol(2); });
    m_header_row->push_back(cost_col);

    auto time_col = Wnd::Create<CUIButton>(UserString("TECH_WND_LIST_COLUMN_TIME"));
    time_col->Resize(GG::Pt(col_widths[3], HEIGHT));
    time_col->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    time_col->LeftClickedSignal.connect([this]() { ToggleSortCol(3); });
    m_header_row->push_back(time_col);

    auto category_col = Wnd::Create<CUIButton>( UserString("TECH_WND_LIST_COLUMN_CATEGORY"));
    category_col->Resize(GG::Pt(col_widths[4], HEIGHT));
    category_col->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    category_col->LeftClickedSignal.connect([this]() { ToggleSortCol(4); });
    m_header_row->push_back(category_col);

    auto descr_col = Wnd::Create<CUIButton>(UserString("TECH_WND_LIST_COLUMN_DESCRIPTION"));
    descr_col->Resize(GG::Pt(col_widths[5], HEIGHT));
    descr_col->SetChildClippingMode(ChildClippingMode::ClipToWindow);
    descr_col->LeftClickedSignal.connect([this]() { ToggleSortCol(5); });
    m_header_row->push_back(descr_col);

    m_header_row->Resize(GG::Pt(row_width, HEIGHT));

    // Initialize column widths before setting header
    int num_cols = m_header_row->size();
    std::vector<GG::Alignment> col_alignments = TechRow::ColAlignments();
    SetNumCols(num_cols);
    for (int i = 0; i < num_cols; ++i) {
        SetColWidth(i, m_header_row->at(i)->Width());
        SetColAlignment(i, col_alignments[i]);
    }

    SetColHeaders(m_header_row);
    LockColWidths();

    // Initialize sorting
    SetSortCol(2);
    m_previous_sort_col = 3;
    SetSortCmp([&](const GG::ListBox::Row& lhs, const GG::ListBox::Row& rhs, std::size_t col) { return TechRowCmp(lhs, rhs, col); });
}

void TechTreeWnd::TechListBox::Reset() {
    m_tech_row_cache.clear();
    Populate();
}

void TechTreeWnd::TechListBox::Update(bool populate) {
    if (populate)
        Populate(false);

    DebugLogger() << "Tech List Box Updating";

    double insertion_elapsed = 0.0;
    ScopedTimer insertion_timer;
    GG::X row_width = Width() - ClientUI::ScrollWidth() - ClientUI::Pts();

    // Try to preserve the first row, only works if a row for the tech is still visible
    std::string first_tech_shown;
    if (FirstRowShown() != end())
        if (auto first_row = FirstRowShown()->get())
            if (auto first_row_shown = dynamic_cast<TechRow*>(first_row))
                first_tech_shown = first_row_shown->GetTech();

    // Skip setting first row during insertion
    bool first_tech_set = first_tech_shown.empty();

    // remove techs in listbox, then reset the rest of its state
    for (iterator it = begin(); it != end();) {
        iterator temp_it = it++;
        Erase(temp_it);
    }

    Clear();

    // Add rows from cache
    for (const auto& [tech_name, tech_row] : m_tech_row_cache) {
        if (tech_row && TechVisible(tech_row->GetTech(), m_categories_shown, m_tech_statuses_shown)) {
            tech_row->Update();
            insertion_timer.restart();
            auto listbox_row_it = Insert(tech_row);
            insertion_elapsed += insertion_timer.duration();
            if (!first_tech_set && tech_name == first_tech_shown) {
                first_tech_set = true;
                SetFirstRowShown(listbox_row_it);
            }
        }
    }

    // set attributes after clear and insert, cached rows may have incorrect widths
    std::vector<GG::X> col_widths = TechRow::ColWidths(row_width);
    int num_cols = static_cast<int>(col_widths.size());
    for (int i = 0; i < num_cols; ++i) {
        SetColWidth(i, col_widths[i]);
        // only stretch the last column
        SetColStretch(i, (i < num_cols - 1) ? 0.0 : 1.0);
    }
    if (SortCol() < 1)
        SetSortCol(2);

    if (!first_tech_set || first_tech_shown.empty())
        BringRowIntoView(begin());

    DebugLogger() << "Tech List Box Updating Done, Insertion time = " << (insertion_elapsed * 1000) << " ms";
}

void TechTreeWnd::TechListBox::Populate(bool update ) {
    DebugLogger() << "Tech List Box Populating";

    const GG::X row_width = Width() - ClientUI::ScrollWidth() - ClientUI::Pts();

    const ScopedTimer creation_timer;

    // Skip lookup check when starting with empty cache

    for (const auto& tech_name : GetTechManager() | range_keys)
        m_tech_row_cache.emplace(tech_name, GG::Wnd::Create<TechRow>(row_width, tech_name));

    DebugLogger() << "Tech List Box Populating Done,  Creation time = " << creation_timer.DurationString();

    if (update)
        Update(false);
}

void TechTreeWnd::TechListBox::ShowCategory(std::string category) {
    if (m_categories_shown.insert(std::move(category)).second)
        Populate();
}

void TechTreeWnd::TechListBox::ShowAllCategories() {
    const auto all_cats = GetTechManager().CategoryNames();
    if (all_cats.size() == m_categories_shown.size())
        return;
    for (auto& category_name : all_cats)
        m_categories_shown.emplace(category_name);
    Populate();
}

void TechTreeWnd::TechListBox::HideCategory(const std::string& category) {
    auto removed_count = m_categories_shown.erase(category);
    if (removed_count > 0)
        Populate();
}

void TechTreeWnd::TechListBox::HideAllCategories() {
    if (m_categories_shown.empty())
        return;
    m_categories_shown.clear();
    Populate();
}

void TechTreeWnd::TechListBox::ShowStatus(TechStatus status) {
    if (m_tech_statuses_shown.insert(status).second)
        Populate();
}

void TechTreeWnd::TechListBox::HideStatus(TechStatus status) {
    const auto removed_count = m_tech_statuses_shown.erase(status);
    if (removed_count > 0)
        Populate();
}

void TechTreeWnd::TechListBox::TechLeftClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) {
    // determine type of row that was clicked, and emit appropriate signal
    if (TechRow* tech_row = dynamic_cast<TechRow*>(it->get()))
        TechLeftClickedSignal(tech_row->GetTech(), GG::Flags<GG::ModKey>());
}

void TechTreeWnd::TechListBox::TechRightClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) {
    if ((*it)->Disabled())
        return;
    const Empire* empire = GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;

    TechRow* tech_row = dynamic_cast<TechRow*>(it->get());
    if (!tech_row)
        return;
    const auto& tech_name = tech_row->GetTech();

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    const ResearchQueue& rq = empire->GetResearchQueue();
    if (!rq.InQueue(tech_name)) {
        auto tech_dclick_action = [this, it, pt]() { TechDoubleClicked(it, pt, GG::Flags<GG::ModKey>()); };
        auto tech_ctrl_dclick_action = [this, it, pt]() { TechDoubleClicked(it, pt, GG::MOD_KEY_CTRL); };

        if (!empire->TechResearched(tech_name)) {
            popup->AddMenuItem(GG::MenuItem(UserString("PRODUCTION_DETAIL_ADD_TO_QUEUE"), false, false,
                                            tech_dclick_action));
            popup->AddMenuItem(GG::MenuItem(UserString("PRODUCTION_DETAIL_ADD_TO_TOP_OF_QUEUE"), false, false,
                                            tech_ctrl_dclick_action));
        }
    }

    auto pedia_display_action = [this, &tech_name]() { TechPediaDisplaySignal(tech_name); };
    std::string popup_label = boost::io::str(FlexibleFormat(UserString("ENC_LOOKUP")) % UserString(tech_name));
    popup->AddMenuItem(GG::MenuItem(std::move(popup_label), false, false, pedia_display_action));

    popup->Run();
}

void TechTreeWnd::TechListBox::TechDoubleClicked(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> modkeys) {
    // determine type of row that was clicked, and emit appropriate signal
    if (TechRow* tech_row = dynamic_cast<TechRow*>(it->get()))
        TechDoubleClickedSignal(tech_row->GetTech(), modkeys);
}


//////////////////////////////////////////////////
// TechTreeWnd                                  //
//////////////////////////////////////////////////
TechTreeWnd::TechTreeWnd(GG::X w, GG::Y h, bool initially_hidden) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_init_flag(initially_hidden)
{}

void TechTreeWnd::CompleteConstruction() {
    GG::Wnd::CompleteConstruction();

    Sound::TempUISoundDisabler sound_disabler;

    using boost::placeholders::_1;
    using boost::placeholders::_2;

    m_layout_panel = GG::Wnd::Create<LayoutPanel>(Width(), Height());
    m_layout_panel->TechSelectedSignal.connect(boost::bind(&TechTreeWnd::TechLeftClickedSlot, this, _1, _2));
    m_layout_panel->TechDoubleClickedSignal.connect(
        [this](std::string tech_name, GG::Flags<GG::ModKey> modkeys)
        { AddTechToResearchQueue(std::move(tech_name), modkeys & GG::MOD_KEY_CTRL); });
    m_layout_panel->TechPediaDisplaySignal.connect(boost::bind(&TechTreeWnd::TechPediaDisplaySlot, this, _1));
    AttachChild(m_layout_panel);

    m_tech_list = GG::Wnd::Create<TechListBox>(Width(), Height());
    m_tech_list->TechLeftClickedSignal.connect(boost::bind(&TechTreeWnd::TechLeftClickedSlot, this, _1, _2));
    m_tech_list->TechDoubleClickedSignal.connect(
        [this](std::string tech_name, GG::Flags<GG::ModKey> modkeys)
        { AddTechToResearchQueue(std::move(tech_name), modkeys & GG::MOD_KEY_CTRL); });
    m_tech_list->TechPediaDisplaySignal.connect(
        boost::bind(&TechTreeWnd::TechPediaDisplaySlot, this, _1));

    m_enc_detail_panel = GG::Wnd::Create<EncyclopediaDetailPanel>(GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE, RES_PEDIA_WND_NAME);
    m_tech_tree_controls = GG::Wnd::Create<TechTreeControls>(RES_CONTROLS_WND_NAME);

    m_enc_detail_panel->ClosingSignal.connect(boost::bind(&TechTreeWnd::HidePedia, this));

    InitializeWindows();
    // Make sure the controls don't overlap the bottom scrollbar
    if (m_tech_tree_controls->Bottom() > m_layout_panel->Bottom() - ClientUI::ScrollWidth()) {
        m_tech_tree_controls->MoveTo(GG::Pt(m_tech_tree_controls->Left(),
                                            m_layout_panel->Bottom() - ClientUI::ScrollWidth() - m_tech_tree_controls->Height()));
        m_tech_tree_controls->SaveDefaultedOptions();
    }

    GGHumanClientApp::GetApp()->RepositionWindowsSignal.connect(
        boost::bind(&TechTreeWnd::InitializeWindows, this));

    AttachChild(m_enc_detail_panel);
    AttachChild(m_tech_tree_controls);

    // connect category button clicks to update display
    m_tech_tree_controls->CategoryCheckedSignal.connect(
        [this](std::string&& category_name, bool checked) {
            if (checked)
                this->ShowCategory(std::move(category_name));
            else
                this->HideCategory(category_name);
        });

    // connect button for all categories to update display
    m_tech_tree_controls->m_all_cat_button->CheckedSignal.connect(
        [this](bool checked) { checked ? ShowAllCategories() : HideAllCategories(); }
    );

    // connect status and type button clicks to update display
    for (auto& status_button : m_tech_tree_controls->m_status_buttons) {
        TechStatus tech_status = status_button.first;
        status_button.second->CheckedSignal.connect(
            [this, tech_status](bool checked){ SetTechStatus(tech_status, checked); });
    }

    // connect view type selector
    m_tech_tree_controls->m_view_type_button->CheckedSignal.connect(
        boost::bind(&TechTreeWnd::ToggleViewType, this, _1));

    //TechTreeWnd in typically constructed before the UI client has
    //accesss to the technologies so showing these categories takes a
    //long time and generates errors, but is never seen by the user.
    if (!m_init_flag) {
        ShowAllCategories();
        SetTechStatus(TechStatus::TS_COMPLETE,              GetOptionsDB().Get<bool>("ui.research.status.completed.shown"));
        SetTechStatus(TechStatus::TS_UNRESEARCHABLE,        GetOptionsDB().Get<bool>("ui.research.status.unresearchable.shown"));
        SetTechStatus(TechStatus::TS_HAS_RESEARCHED_PREREQ, GetOptionsDB().Get<bool>("ui.research.status.partial.shown"));
        SetTechStatus(TechStatus::TS_RESEARCHABLE,          GetOptionsDB().Get<bool>("ui.research.status.researchable.shown"));
    }

    ShowTreeView();
}

void TechTreeWnd::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_size = Size();
    GG::Wnd::SizeMove(ul, lr);
    if (old_size != Size()) {
        m_enc_detail_panel->ValidatePosition();
        m_tech_tree_controls->ValidatePosition();
        m_layout_panel->Resize(Size());
        m_tech_list->Resize(Size());
    }
}

double TechTreeWnd::Scale() const
{ return m_layout_panel->Scale(); }

void TechTreeWnd::Update() {
    m_layout_panel->Update();
    m_tech_list->Update();
    m_tech_tree_controls->RefreshCategoryButtons(m_layout_panel->GetCategoriesShown());
}

void TechTreeWnd::Clear() {
    m_enc_detail_panel->OnIndex();
    m_layout_panel->Clear();
}

void TechTreeWnd::Reset() {
    m_layout_panel->Reset();
    m_tech_list->Reset();
    m_tech_tree_controls->RefreshCategoryButtons(m_layout_panel->GetCategoriesShown());
    ShowAllCategories();
}

void TechTreeWnd::InitializeWindows() {
    static constexpr GG::Pt pedia_ul(GG::X0, GG::Y0);
    static constexpr GG::Pt pedia_wh(GG::X{480}, GG::Y{240});

    // Don't know this wnd's height in advance so place it off the bottom edge,
    // it subclasses CUIWnd so it will reposition itself to be visible.
    const GG::Pt controls_ul(GG::X1, m_layout_panel->Height());
    const GG::Pt controls_wh(GG::ToX(m_layout_panel->Width()*0.6) - ClientUI::ScrollWidth(), GG::Y0);

    m_enc_detail_panel->InitSizeMove(pedia_ul,  pedia_ul + pedia_wh);
    m_tech_tree_controls->InitSizeMove(controls_ul,  controls_ul + controls_wh);
}

void TechTreeWnd::Show() {
    GG::Wnd::Show();

    // When Show() is called for TechTree the ClientUI should now have
    // access to the technologies so that parsing does not generate
    // errors.
    if (m_init_flag) {
        m_init_flag = false;
        ShowAllCategories();
        SetTechStatus(TechStatus::TS_COMPLETE,              GetOptionsDB().Get<bool>("ui.research.status.completed.shown"));
        SetTechStatus(TechStatus::TS_UNRESEARCHABLE,        GetOptionsDB().Get<bool>("ui.research.status.unresearchable.shown"));
        SetTechStatus(TechStatus::TS_HAS_RESEARCHED_PREREQ, GetOptionsDB().Get<bool>("ui.research.status.partial.shown"));
        SetTechStatus(TechStatus::TS_RESEARCHABLE,          GetOptionsDB().Get<bool>("ui.research.status.researchable.shown"));
    }
}

void TechTreeWnd::ShowCategory(std::string category) {
    const auto button_it = m_tech_tree_controls->m_cat_buttons.find(category);

    m_layout_panel->ShowCategory(category);
    m_tech_list->ShowCategory(std::move(category));

    if (button_it != m_tech_tree_controls->m_cat_buttons.end())
        button_it->second->SetCheck(true);
}

void TechTreeWnd::ShowAllCategories() {
    m_layout_panel->ShowAllCategories();
    m_tech_list->ShowAllCategories();

    for (auto& button : m_tech_tree_controls->m_cat_buttons | range_values)
        button->SetCheck(true);
}

void TechTreeWnd::HideCategory(const std::string& category) {
    m_layout_panel->HideCategory(category);
    m_tech_list->HideCategory(category);

    const auto button_it = m_tech_tree_controls->m_cat_buttons.find(category);
    if (button_it != m_tech_tree_controls->m_cat_buttons.end())
        button_it->second->SetCheck(false);
}

void TechTreeWnd::HideAllCategories() {
    m_layout_panel->HideAllCategories();
    m_tech_list->HideAllCategories();

    for (auto& button : m_tech_tree_controls->m_cat_buttons | range_values)
        button->SetCheck(false);
}

void TechTreeWnd::ToggleAllCategories() {
    const auto num_casts_shown = m_layout_panel->GetCategoriesShown().size();
    const auto num_cats = GetTechManager().CategoryNames().size();

    if (num_casts_shown == num_cats)
        HideAllCategories();
    else
        ShowAllCategories();
}

void TechTreeWnd::SetTechStatus(const TechStatus status, const bool state) {
    switch (status) {
    case TechStatus::TS_UNRESEARCHABLE:
        GetOptionsDB().Set("ui.research.status.unresearchable.shown", state);
        break;
    case TechStatus::TS_HAS_RESEARCHED_PREREQ:
        GetOptionsDB().Set("ui.research.status.partial.shown", state);
        break;
    case TechStatus::TS_RESEARCHABLE:
        GetOptionsDB().Set("ui.research.status.researchable.shown", state);
        break;
    case TechStatus::TS_COMPLETE:
        GetOptionsDB().Set("ui.research.status.completed.shown", state);
        break;
    default:
        ; // do nothing
    }
    GetOptionsDB().Commit();

    if (state) {
        m_layout_panel->ShowStatus(status);
        m_tech_list->ShowStatus(status);
    } else {
        m_layout_panel->HideStatus(status);
        m_tech_list->HideStatus(status);
    }

    m_tech_tree_controls->SetTechStatus(status, state);
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

void TechTreeWnd::CenterOnTech(const std::string& tech_name) {
    // ensure tech exists and is visible
    const Tech* tech = ::GetTech(tech_name);
    if (!tech) return;
    if (const Empire* empire = GetEmpire(GGHumanClientApp::GetApp()->EmpireID()))
        SetTechStatus(empire->GetTechStatus(tech_name), true);
    ShowCategory(tech->Category());

    // centre on it
    m_layout_panel->CenterOnTech(tech_name);
}

void TechTreeWnd::SetEncyclopediaTech(std::string tech_name)
{ m_enc_detail_panel->SetTech(std::move(tech_name)); }

void TechTreeWnd::SelectTech(std::string tech_name)
{ m_layout_panel->SelectTech(std::move(tech_name)); }

void TechTreeWnd::ShowPedia() {
    m_enc_detail_panel->Refresh();
    m_enc_detail_panel->Show();

    OptionsDB& db = GetOptionsDB();
    db.Set("ui." + RES_PEDIA_WND_NAME + ".hidden.enabled", false);
}

void TechTreeWnd::HidePedia() {
    m_enc_detail_panel->Hide();

    OptionsDB& db = GetOptionsDB();
    db.Set("ui." + RES_PEDIA_WND_NAME + ".hidden.enabled", true);
}

void TechTreeWnd::TogglePedia() {
    if (!m_enc_detail_panel->Visible())
        ShowPedia();
    else
        HidePedia();
}

bool TechTreeWnd::PediaVisible()
{ return m_enc_detail_panel->Visible(); }

bool TechTreeWnd::TechIsVisible(const std::string& tech_name) const
{ return TechVisible(tech_name, m_layout_panel->GetCategoriesShown(), m_layout_panel->GetTechStatusesShown()); }

void TechTreeWnd::TechLeftClickedSlot(std::string tech_name, GG::Flags<GG::ModKey> modkeys) {
    if (modkeys & GG::MOD_KEY_SHIFT) {
        AddTechToResearchQueue(std::move(tech_name), modkeys & GG::MOD_KEY_CTRL);
    } else {
        SetEncyclopediaTech(tech_name);
        TechSelectedSignal(std::move(tech_name));
    }
}

void TechTreeWnd::AddTechToResearchQueue(std::string tech_name, bool to_front) {
    const Tech* tech = GetTech(tech_name);
    if (!tech) return;

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    const auto empire = context.GetEmpire(GGHumanClientApp::GetApp()->EmpireID());
    if (!empire)
        return;
    const TechStatus tech_status = empire->GetTechStatus(tech_name);
    const int queue_pos = to_front ? 0 : -1;

    // if tech can be researched already, just add it
    if (tech_status == TechStatus::TS_RESEARCHABLE) {
        AddTechsToQueueSignal(std::vector{std::move(tech_name)}, queue_pos);
        return;
    }

    if (tech_status != TechStatus::TS_UNRESEARCHABLE &&
        tech_status != TechStatus::TS_HAS_RESEARCHED_PREREQ)
    { return; }

    // if tech can't yet be researched, add any prerequisites it requires (recursively) and then add it
    const TechManager& manager = GetTechManager();
    const int empire_id = GGHumanClientApp::GetApp()->EmpireID();
    auto all_prereqs = manager.RecursivePrereqs(tech_name, empire_id, context);
    std::vector<std::string> unresearched_techs;
    unresearched_techs.reserve(all_prereqs.size() + 1);
    std::copy_if(std::make_move_iterator(all_prereqs.begin()), std::make_move_iterator(all_prereqs.end()),
                 std::back_inserter(unresearched_techs),
                 [&empire](const auto& tech_name) { return !empire->TechResearched(tech_name); });
    unresearched_techs.push_back(std::move(tech_name));
    AddTechsToQueueSignal(std::move(unresearched_techs), queue_pos);
}

void TechTreeWnd::TechPediaDisplaySlot(std::string tech_name) {
    SetEncyclopediaTech(std::move(tech_name));
    if (!PediaVisible())
        ShowPedia();
}
