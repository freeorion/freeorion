#include "SitRepPanel.h"

#include "CUIControls.h"
#include "LinkText.h"
#include "Sound.h"
#include "../client/human/GGHumanClientApp.h"
#include "../Empire/Empire.h"
#include "../util/i18n.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/SitRepEntry.h"
#include "../universe/ShipDesign.h"

#include "../util/ScopedTimer.h"

#include <GG/Layout.h>

#include <boost/lexical_cast.hpp>

#include <iterator>


namespace {
    constexpr int sitrep_row_margin(1);
    constexpr int sitrep_edge_to_outline_spacing(2);
    constexpr int sitrep_edge_to_content_spacing(sitrep_edge_to_outline_spacing + 1 + 2);
    constexpr int sitrep_spacing(2);

    /** Adds options related to SitRepPanel to Options DB. */
    void AddOptions(OptionsDB& db) {
        db.Add("ui.map.sitrep.invalid.shown",                   UserStringNop("OPTIONS_DB_VERBOSE_SITREP_DESC"),            false);
        db.Add<std::string>("ui.map.sitrep.hidden.stringlist",  UserStringNop("OPTIONS_DB_HIDDEN_SITREP_TEMPLATES_DESC"),   "");
        db.Add("ui.map.sitrep.icon.size",                       UserStringNop("OPTIONS_DB_UI_SITREP_ICONSIZE"),
               24, RangedValidator<int>(12, 64));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    int GetIconSize()
    { return GetOptionsDB().Get<int>("ui.map.sitrep.icon.size"); }

    std::map<std::string, std::string> label_display_map;
    std::map<int, std::set<std::string>> snoozed_sitreps;
    std::set<std::string> permanently_snoozed_sitreps;

    void SnoozeSitRepForNTurns(std::string sitrep_text, int start_turn, int num_turns) {
        for (int turn = start_turn; turn < start_turn + num_turns; turn++)
            snoozed_sitreps[turn].insert(sitrep_text);
    }

    void HandleLinkClick(const std::string& link_type, const std::string& data) {
        using boost::lexical_cast;
        try {
            if (link_type == VarText::PLANET_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToPlanet(lexical_cast<int>(data));
            } else if (link_type == VarText::SYSTEM_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToSystem(lexical_cast<int>(data));
            } else if (link_type == VarText::FLEET_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToFleet(lexical_cast<int>(data));
            } else if (link_type == VarText::SHIP_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToShip(lexical_cast<int>(data));
            } else if (link_type == VarText::BUILDING_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToBuilding(lexical_cast<int>(data));

            } else if (link_type == VarText::COMBAT_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToCombatLog(lexical_cast<int>(data));

            } else if (link_type == VarText::EMPIRE_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToEmpire(lexical_cast<int>(data));
            } else if (link_type == VarText::DESIGN_ID_TAG) {
                ClientUI::GetClientUI()->ZoomToShipDesign(lexical_cast<int>(data));
            } else if (link_type == VarText::PREDEFINED_DESIGN_TAG) {
                if (const ShipDesign* design = GetUniverse().GetGenericShipDesign(data))
                    ClientUI::GetClientUI()->ZoomToShipDesign(design->ID());

            } else if (link_type == VarText::TECH_TAG) {
                ClientUI::GetClientUI()->ZoomToTech(data);
            } else if (link_type == VarText::POLICY_TAG) {
                ClientUI::GetClientUI()->ZoomToPolicy(data);
            } else if (link_type == VarText::BUILDING_TYPE_TAG) {
                ClientUI::GetClientUI()->ZoomToBuildingType(data);
            } else if (link_type == VarText::SPECIAL_TAG) {
                ClientUI::GetClientUI()->ZoomToSpecial(data);
            } else if (link_type == VarText::SHIP_HULL_TAG) {
                ClientUI::GetClientUI()->ZoomToShipHull(data);
            } else if (link_type == VarText::SHIP_PART_TAG) {
                ClientUI::GetClientUI()->ZoomToShipPart(data);
            } else if (link_type == VarText::SPECIES_TAG) {
                ClientUI::GetClientUI()->ZoomToSpecies(data);
            } else if (link_type == VarText::METER_TYPE_TAG) {
                ClientUI::GetClientUI()->ZoomToMeterTypeArticle(data);
            } else if (link_type == TextLinker::ENCYCLOPEDIA_TAG) {
                ClientUI::GetClientUI()->ZoomToEncyclopediaEntry(data);
            } else if (link_type == TextLinker::BROWSE_PATH_TAG) {
                GGHumanClientApp::GetApp()->BrowsePath(FilenameToPath(data));
            }
        } catch (const boost::bad_lexical_cast&) {
            ErrorLogger() << "SitrepPanel.cpp HandleLinkClick caught lexical cast exception for link type: " << link_type << " and data: " << data;
        }
    }

    std::vector<std::string> OrderedSitrepTemplateStrings() {
        // determine sitrep order
        std::istringstream template_stream(UserString("FUNCTIONAL_SITREP_PRIORITY_ORDER"));
        std::vector<std::string> sitrep_order;
        std::copy(std::istream_iterator<std::string>(template_stream),
                  std::istream_iterator<std::string>(),
                  std::back_inserter<std::vector<std::string>>(sitrep_order));
        return sitrep_order;
    }

    std::set<std::string> EmpireSitRepTemplateStrings(int empire_id) {
        std::set<std::string> template_set;

        const Empire* empire = GGHumanClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire)
            return template_set;

        for (const auto& sitrep : empire->SitReps()) {
            std::string label;
            if (sitrep.GetLabelString().empty()) {
                label = sitrep.GetTemplateString();
                label_display_map[label] = UserString(label + "_LABEL");
            } else {
                label = sitrep.GetLabelString();
                label_display_map[label] = sitrep.GetStringtableLookupFlag()? UserString(label) : label;
            }
            template_set.insert(std::move(label));
        }

        return template_set;
    }

    std::vector<std::string> AllSitRepTemplateStrings() {
        // get templates for each empire
        std::set<std::string> template_set;
        for (const auto& entry : Empires())
            template_set.merge(EmpireSitRepTemplateStrings(entry.first));

        auto ordered_template_strings{OrderedSitrepTemplateStrings()};

        std::vector<std::string> retval;
        retval.reserve(ordered_template_strings.size() + template_set.size());

        // first add only use those ordered templates actually in the current set of sitrep templates
        for (std::string& templ : OrderedSitrepTemplateStrings()) {
            if (template_set.contains(templ) &&
                !std::count(retval.begin(), retval.end(), templ))
            { retval.push_back(std::move(templ)); }
        }

        // next add the current templates that did not have a specified order
        for (auto it = template_set.begin(); it != template_set.end();) {
            if (!std::count(retval.begin(), retval.end(), *it))
                retval.push_back(std::move(template_set.extract(it++).value()));
            else
                ++it;
        }

        return retval;
    }

    std::set<std::string> HiddenSitRepTemplateStringsFromOptions() {
        std::set<std::string> result;
        std::string saved_template_string = GetOptionsDB().Get<std::string>("ui.map.sitrep.hidden.stringlist");

        // Split a space-delimited sequence of strings.
        std::istringstream ss(saved_template_string);
        std::copy(
            std::istream_iterator<std::string>(ss),
            std::istream_iterator<std::string>(),
            std::inserter(result, result.begin())
        );
        return result;
    }

    void SetHiddenSitRepTemplateStringsInOptions(const std::set<std::string>& set) {
        std::stringstream ss;

        // Join a set of strings into a space-delimited sequence of strings.
        std::copy(set.begin(), set.end(), std::ostream_iterator<std::string>(ss, " "));

        GetOptionsDB().Set<std::string>("ui.map.sitrep.hidden.stringlist", ss.str());
        GetOptionsDB().Commit();
    }

    //////////////////////////////////
    // SitRepLinkText
    //////////////////////////////////
    class SitRepLinkText final : public LinkText {
    public:
        SitRepLinkText(GG::X x, GG::Y y, GG::X w, std::string str, std::shared_ptr<GG::Font> font,
                       GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE, GG::Clr color = GG::CLR_BLACK) :
            LinkText(x, y, w, std::move(str), std::move(font), format, color)
        {}

        mutable boost::signals2::signal<void(GG::Pt, GG::Flags<GG::ModKey>)> RightClickedSignal;

    protected:
        void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override {
            if (GetLinkUnderPt(pt) != -1) {
                LinkText::RClick(pt, mod_keys);
            } else {
                RightClickedSignal(pt, mod_keys);
            }
        }
    };

    //////////////////////////////////
    // SitRepDataPanel
    //////////////////////////////////
    class SitRepDataPanel : public GG::Control {
    public:
        SitRepDataPanel(GG::X left, GG::Y top, GG::X w, GG::Y h, const SitRepEntry& sitrep) :
            Control(left, top, w, h, GG::NO_WND_FLAGS),
            m_sitrep_entry(sitrep)
        {}

        void CompleteConstruction() override {
            GG::Control::CompleteConstruction();

            SetChildClippingMode(ChildClippingMode::ClipToClient);

            const int icon_dim = GetIconSize();
            std::string icon_texture = (m_sitrep_entry.GetIcon().empty() ?
                "/icons/sitrep/generic.png" : m_sitrep_entry.GetIcon());
            std::shared_ptr<GG::Texture> icon = ClientUI::GetTexture(ClientUI::ArtDir() / icon_texture, true);
            m_icon = GG::Wnd::Create<GG::StaticGraphic>(std::move(icon), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            AttachChild(m_icon);
            m_icon->Resize(GG::Pt(GG::X(icon_dim), GG::Y(icon_dim)));

            GG::Pt spacer = GG::Pt(GG::X(sitrep_edge_to_content_spacing), GG::Y(sitrep_edge_to_content_spacing));
            GG::X icon_left(spacer.x);
            GG::X text_left(icon_left + GG::X(icon_dim) + sitrep_spacing);
            GG::X text_width(ClientWidth() - text_left - spacer.x);

            m_link_text = GG::Wnd::Create<SitRepLinkText>(
                GG::X0, GG::Y0, text_width, m_sitrep_entry.GetText(IApp::GetApp()->GetContext()) + " ", ClientUI::GetFont(),
                GG::FORMAT_LEFT | GG::FORMAT_VCENTER | GG::FORMAT_WORDBREAK, ClientUI::TextColor());
            m_link_text->SetDecorator(VarText::EMPIRE_ID_TAG, TextLinker::DecoratorType::ColorByEmpire);
            m_link_text->SetDecorator(TextLinker::BROWSE_PATH_TAG, TextLinker::DecoratorType::PathType);
            m_link_text->SetDecorator(VarText::FOCS_VALUE_TAG, TextLinker::DecoratorType::ValueRef);
            AttachChild(m_link_text);

            namespace ph = boost::placeholders;

            m_link_text->LinkClickedSignal.connect(&HandleLinkClick);
            m_link_text->LinkDoubleClickedSignal.connect(&HandleLinkClick);
            m_link_text->LinkRightClickedSignal.connect(&HandleLinkClick);
            m_link_text->RightClickedSignal.connect(
                boost::bind(&SitRepDataPanel::RClick, this, ph::_1, ph::_2));

            DoLayout(UpperLeft(), Width());
        }

        void Render() override {
            GG::Clr background_clr = this->Disabled() ? ClientUI::WndColor() : ClientUI::CtrlColor();
            GG::Pt spacer = GG::Pt(GG::X(sitrep_edge_to_outline_spacing), GG::Y(sitrep_edge_to_outline_spacing));
            GG::FlatRectangle(UpperLeft() + spacer, LowerRight() - spacer,
                              background_clr, ClientUI::WndOuterBorderColor(), 1u);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            if (ul != ClientUpperLeft() || (lr.x - ul.x) != Width())
                DoLayout(ul, lr.x - ul.x);
        }

        const SitRepEntry& GetSitRepEntry() const noexcept { return m_sitrep_entry; }

        mutable boost::signals2::signal<void(GG::Pt, GG::Flags<GG::ModKey>)> RightClickedSignal;

    protected:
        void RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod) override
        { RightClickedSignal(pt, mod); }

    private:
        void DoLayout(GG::Pt ul, GG::X width) {
            // Resize the text
            GG::Pt spacer = GG::Pt(GG::X(sitrep_edge_to_content_spacing), GG::Y(sitrep_edge_to_content_spacing));
            GG::X icon_left(ul.x + spacer.x);
            int icon_dim = GetIconSize();
            GG::X text_left(icon_left + GG::X(icon_dim) + sitrep_spacing);
            GG::X text_width(width - text_left - spacer.x);
            m_link_text->SizeMove(GG::Pt(text_left, GG::Y0), GG::Pt(text_left, GG::Y0) + GG::Pt(text_width, GG::Y1));

            // Choose height so that text always fits
            GG::Y text_height = m_link_text->MinUsableSize().y;
            GG::Y panel_height = std::max(text_height, GG::Y(icon_dim)) + 2 * spacer.y;

            // Move the elements into place.
            GG::Y icon_top(ul.y + panel_height/2 - GG::Y(icon_dim)/2);
            m_icon->MoveTo(GG::Pt(icon_left, icon_top));

            GG::Y text_top(ul.y + panel_height/2 - GG::Y(text_height)/2);
            GG::Pt text_ul(text_left, text_top);
            m_link_text->SizeMove(text_ul, text_ul + m_link_text->MinUsableSize());

            //Resize control to fit
            GG::Pt new_size = GG::Pt(width, panel_height);
            GG::Control::SizeMove(ul, ul + new_size);
        }

        const SitRepEntry&                  m_sitrep_entry;
        std::shared_ptr<GG::StaticGraphic>  m_icon;
        std::shared_ptr<SitRepLinkText>     m_link_text;
    };

    ////////////////////////////////////////////////
    // SitRepRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to display SitReps. */
    class SitRepRow : public GG::ListBox::Row {
    public:
        SitRepRow(GG::X w, GG::Y h, const SitRepEntry& sitrep) :
            GG::ListBox::Row(w, h),
            m_sitrep(sitrep)
        {
            SetName("SitRepRow");
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();

            SetMargin(sitrep_row_margin);
            SetChildClippingMode(ChildClippingMode::ClipToClient);
            SetMinSize(GG::Pt(GG::X(2 * GetIconSize() + 2 * sitrep_edge_to_content_spacing),
                              GG::Y(std::max(GetIconSize(), ClientUI::Pts() * 2) + 2 * sitrep_edge_to_content_spacing)));
            RequirePreRender();
        }

        void PreRender() override {
            GG::ListBox::Row::PreRender();

            if (!m_panel)
                Init();

            // Resize to fit panel after text reflows
            const int margin = 2 * GetLayout()->BorderMargin();
            const GG::Pt border{GG::X{margin}, GG::Y{margin}};
            m_panel->Resize(Size() - border);
            GG::ListBox::Row::Resize(m_panel->Size() + border);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            if (!m_panel || (Size() != (lr - ul)))
                RequirePreRender();
            GG::ListBox::Row::SizeMove(ul, lr);
        }

        void Init() {
            const int margin = GetLayout()->BorderMargin();
            const auto margin2 = 2 * margin;
            m_panel = GG::Wnd::Create<SitRepDataPanel>(GG::X(margin), GG::Y(margin),
                                                       ClientWidth() - GG::X(margin2),
                                                       ClientHeight() - GG::Y(margin2),
                                                       m_sitrep);
            push_back(m_panel);
            m_panel->RightClickedSignal.connect([this](auto pt, auto flags) { return RClick(pt, flags); });
        }

        auto& GetSitRepEntry() const noexcept { return m_sitrep; }

    private:
        std::shared_ptr<SitRepDataPanel>    m_panel;
        const SitRepEntry                   m_sitrep;
    };
}

SitRepPanel::SitRepPanel(std::string_view config_name) :
    CUIWnd(UserString("SITREP_PANEL_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE,
           config_name),
    m_showing_turn(INVALID_GAME_TURN),
    m_hidden_sitrep_templates(HiddenSitRepTemplateStringsFromOptions())
{}

void SitRepPanel::CompleteConstruction() {
    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(ChildClippingMode::DontClip);

    m_sitreps_lb = GG::Wnd::Create<CUIListBox>();
    m_sitreps_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
    m_sitreps_lb->SetVScrollWheelIncrement(ClientUI::Pts()*4.5);
    m_sitreps_lb->ManuallyManageColProps();
    m_sitreps_lb->NormalizeRowsOnInsert(false);
    AttachChild(m_sitreps_lb);

    m_prev_turn_button = Wnd::Create<CUIButton>(UserString("BACK"));
    AttachChild(m_prev_turn_button);
    m_next_turn_button = Wnd::Create<CUIButton>(UserString("NEXT"));
    AttachChild(m_next_turn_button);
    m_last_turn_button = Wnd::Create<CUIButton>(UserString("LAST"));
    AttachChild(m_last_turn_button);
    m_filter_button = Wnd::Create<CUIButton>(UserString("FILTERS"));
    AttachChild(m_filter_button);

    using boost::placeholders::_1;
    using boost::placeholders::_2;
    using boost::placeholders::_3;

    m_prev_turn_button->LeftClickedSignal.connect(boost::bind(&SitRepPanel::PrevClicked, this));
    m_next_turn_button->LeftClickedSignal.connect(boost::bind(&SitRepPanel::NextClicked, this));
    m_last_turn_button->LeftClickedSignal.connect(boost::bind(&SitRepPanel::LastClicked, this));
    m_filter_button->LeftClickedSignal.connect(boost::bind(&SitRepPanel::FilterClicked, this));
    m_sitreps_lb->DoubleClickedRowSignal.connect(boost::bind(&SitRepPanel::IgnoreSitRep, this, _1, _2, _3));
    m_sitreps_lb->RightClickedRowSignal.connect(boost::bind(&SitRepPanel::DismissalMenu, this, _1, _2, _3));

    CUIWnd::CompleteConstruction();

    DoLayout();
}

void SitRepPanel::DoLayout() {
    GG::X BUTTON_WIDTH{ClientUI::Pts()*4};
    GG::Y BUTTON_HEIGHT = m_last_turn_button->MinUsableSize().y;
    static constexpr GG::X SITREP_PANEL_PAD{3};

    GG::Pt button_ul(ClientWidth() - GG::X(INNER_BORDER_ANGLE_OFFSET) - BUTTON_WIDTH,
                     ClientHeight() - BUTTON_HEIGHT);

    m_last_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + SITREP_PANEL_PAD, GG::Y0);
    m_next_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + SITREP_PANEL_PAD, GG::Y0);
    m_prev_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + SITREP_PANEL_PAD, GG::Y0);

    m_sitreps_lb->SizeMove(GG::Pt0, GG::Pt(ClientWidth() - 1, button_ul.y));

    m_filter_button->SizeMove(GG::Pt(GG::X0, button_ul.y), GG::Pt(BUTTON_WIDTH*2, button_ul.y + BUTTON_HEIGHT));

    SetMinSize(GG::Pt(6*BUTTON_WIDTH, 6*BUTTON_HEIGHT));
}

void SitRepPanel::KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    switch (key) {
    case GG::Key::GGK_RETURN:
    case GG::Key::GGK_KP_ENTER:
    case GG::Key::GGK_ESCAPE:{
        CloseClicked();
        break;
    }
    default:
        break;
    }
}

void SitRepPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    const GG::Pt old_size = GG::Wnd::Size();
    const std::size_t first_visible_queue_row = std::distance(m_sitreps_lb->begin(), m_sitreps_lb->FirstRowShown());

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        DoLayout();
        if (!m_sitreps_lb->Empty())
            m_sitreps_lb->SetFirstRowShown(std::next(m_sitreps_lb->begin(),
                                                     first_visible_queue_row));
    }
}

namespace {
    /* Sort sitreps for each turn.
     * Note: Validating requires substituting all of the variables which is time
     * consuming for sitreps that the player will never view. */
    auto GetUnvalidatedSitRepsSortedByTurn(int empire_id) {
        std::map<int, std::vector<const SitRepEntry*>> turns;

        auto empire_sitrep_inserter = [&turns](const Empire* e) {
            const auto& sitreps = e->SitReps();
            std::for_each(sitreps.begin(), sitreps.end(),
                          [&turns](const auto& s) { turns[s.GetTurn()].push_back(&s); });
        };
        auto empire_pair_sitrep_inserter = [empire_sitrep_inserter](const auto& ep)
        { empire_sitrep_inserter(ep.second.get()); };


        if (const Empire* empire = GetEmpire(empire_id)) {
            empire_sitrep_inserter(empire);

        } else {
            // Observer / Moderator mode, sort sitreps from all empires
            const auto& empires = Empires();
            std::for_each(empires.begin(), empires.end(), empire_pair_sitrep_inserter);
        }

        return turns;
    }

    /** Return true iff the \a sitrep is not hidden, validates and is not snoozed. */
    bool IsSitRepInvalid(const SitRepEntry& sitrep, const std::set<std::string>& hidden_templates) {
        auto& label = sitrep.GetLabelString().empty() ? sitrep.GetTemplateString() : sitrep.GetLabelString();
        if (hidden_templates.contains(label))
            return true;

        const ScriptingContext& context = IApp::GetApp()->GetContext();

        // Validation is time consuming because all variables are substituted.
        // Having ui.map.sitrep.invalid.shown off / disabled will hide sitreps that do not
        // validate. Having it on will skip the validation check.
        if (!GetOptionsDB().Get<bool>("ui.map.sitrep.invalid.shown") && !sitrep.Validate(context))
            return true;

        // Check for snoozing.
        if (permanently_snoozed_sitreps.contains(sitrep.GetText(context)))
            return true;

        auto sitrep_set_it = snoozed_sitreps.find(sitrep.GetTurn());
        if (sitrep_set_it != snoozed_sitreps.end()
            && sitrep_set_it->second.contains(sitrep.GetText(context)))
        { return true; }

        return false;
    }

    /** Search forward (if \a forward is true) or backward from \a turn for the next
    * turn with one or more valid sitreps (not including \a turn itself). */
    int GetNextNonEmptySitrepsTurn(const std::map<int, std::vector<const SitRepEntry*>>& turns,
                                   int turn, bool forward, const std::set<std::string>& hidden_templates)
    {
        if (turns.empty())
            return INVALID_GAME_TURN;

        auto contains_valid_sitrep = [&hidden_templates](const auto& p) {
            return std::any_of(p.cbegin(), p.cend(),
                               [&hidden_templates](const auto* s) {
                                   return s && !IsSitRepInvalid(*s, hidden_templates);
                               });
        };

        auto turn_ok = [=](int t)
        { return forward ? (t > turn) : (t < turn); };

        auto ok_and_contains_valid = [=](const auto& p)
        { return turn_ok(p.first) && contains_valid_sitrep(p.second); };

        if (forward) {
            auto found_it = std::find_if(turns.lower_bound(turn), turns.end(), ok_and_contains_valid);
            if (found_it != turns.end())
                return found_it->first;

        } else {
            auto found_it = std::find_if(turns.crbegin(), turns.crend(), ok_and_contains_valid);
            if (found_it != turns.rend())
                return found_it->first;
        }

        return INVALID_GAME_TURN;
    }
}

void SitRepPanel::CloseClicked()
{ ClosingSignal(); }

void SitRepPanel::PrevClicked() {
    auto turns = GetUnvalidatedSitRepsSortedByTurn(GGHumanClientApp::GetApp()->EmpireID());
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(turns, m_showing_turn, false, m_hidden_sitrep_templates));
}

void SitRepPanel::NextClicked() {
    auto turns = GetUnvalidatedSitRepsSortedByTurn(GGHumanClientApp::GetApp()->EmpireID());
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(turns, m_showing_turn, true, m_hidden_sitrep_templates));
}

void SitRepPanel::LastClicked() {
    auto turns = GetUnvalidatedSitRepsSortedByTurn(GGHumanClientApp::GetApp()->EmpireID());
    // search backwards from current turn for a non-empty sitrep turn
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(turns, GGHumanClientApp::GetApp()->CurrentTurn() + 1,
                                                  false, m_hidden_sitrep_templates));
}

void SitRepPanel::FilterClicked() {
    SectionedScopedTimer filter_click_timer{"SitRepPanel::FilterClicked"};
    std::map<int, std::string> menu_index_templates;
    std::map<int, bool> menu_index_checked;
    int index = 1;
    bool all_checked = true;

    filter_click_timer.EnterSection("get templates");
    auto all_templates = AllSitRepTemplateStrings();

    filter_click_timer.EnterSection("create popupmenu");
    auto popup = GG::Wnd::Create<CUIPopupMenu>(m_filter_button->Left(), m_filter_button->Bottom());

    filter_click_timer.EnterSection("add templates");
    for (const std::string& templ : all_templates) {
        menu_index_templates[index] = templ;
        bool check = true;
        if (m_hidden_sitrep_templates.contains(templ)) {
            check = false;
            all_checked = false;
        }
        menu_index_checked[index] = check;
        const std::string& menu_label =  label_display_map[templ];

        auto select_template_action = [index, this, &menu_index_templates, &menu_index_checked]() {
            // select / deselect the chosen template
            const std::string& selected_template_string = menu_index_templates[index];
            if (menu_index_checked[index]) {
                // disable showing this template string
                m_hidden_sitrep_templates.insert(selected_template_string);
            } else {
                // re-enabled showing this template string
                m_hidden_sitrep_templates.erase(selected_template_string);
            }
        };

        popup->AddMenuItem(menu_label, false, check, select_template_action);
        ++index;
    }

    auto all_templates_action = [all_checked, &all_templates, this]() {
        // select / deselect all templates
        if (all_checked) {
            // deselect all
            m_hidden_sitrep_templates.insert(std::make_move_iterator(all_templates.begin()),
                                             std::make_move_iterator(all_templates.end()));
        } else {
            // select all
            m_hidden_sitrep_templates.clear();
        }
    };
    popup->AddMenuItem((all_checked ? UserString("NONE") : UserString("ALL")),
                       false, false, all_templates_action);

    filter_click_timer.EnterSection("run menu");
    if (!popup->Run())
        return;

    filter_click_timer.EnterSection("cleanup");
    SetHiddenSitRepTemplateStringsInOptions(m_hidden_sitrep_templates);

    Update();
}

void SitRepPanel::IgnoreSitRep(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> mod) {
    SitRepRow* sitrep_row = dynamic_cast<SitRepRow*>(it->get());
    if (!sitrep_row)
        return;

    const SitRepEntry& sitrep = sitrep_row->GetSitRepEntry();
    if (sitrep.GetTurn() <= 0)
        return;

    snoozed_sitreps[sitrep.GetTurn()].insert(sitrep.GetText(IApp::GetApp()->GetContext()));

    Update();
}

void SitRepPanel::DismissalMenu(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> mod) {
    GG::MenuItem menu_contents, submenu_ignore, submenu_block, separator_item;
    std::string sitrep_text, sitrep_template;
    std::string entry_margin("  ");
    separator_item.separator = true;
    int start_turn = 0;
    SitRepRow* sitrep_row = nullptr;
    if (it != m_sitreps_lb->end()) 
        sitrep_row = dynamic_cast<SitRepRow*>(it->get());
    submenu_ignore.label = entry_margin + UserString("SITREP_IGNORE_MENU");

    if (sitrep_row) {
        const SitRepEntry& sitrep_entry = sitrep_row->GetSitRepEntry();
        sitrep_text = sitrep_entry.GetText(IApp::GetApp()->GetContext());
        start_turn = sitrep_entry.GetTurn();
        if (start_turn > 0) {
            auto snooze5_action = [&sitrep_text, start_turn]() { SnoozeSitRepForNTurns(sitrep_text, start_turn, 5); };
            auto snooze10_action = [&sitrep_text, start_turn]() { SnoozeSitRepForNTurns(sitrep_text, start_turn, 10); };
            auto snooze_indefinite_action = [&sitrep_text]() { permanently_snoozed_sitreps.insert(sitrep_text); };

            submenu_ignore.next_level.emplace_back(entry_margin + UserString("SITREP_SNOOZE_5_TURNS"),
                                                   false, false, snooze5_action);
            submenu_ignore.next_level.emplace_back(entry_margin + UserString("SITREP_SNOOZE_10_TURNS"),
                                                   false, false, snooze10_action);
            submenu_ignore.next_level.emplace_back(entry_margin + UserString("SITREP_SNOOZE_INDEFINITE"),
                                                   false, false, snooze_indefinite_action);
            submenu_ignore.next_level.emplace_back(separator_item);
        }
    }


    auto snooze_clear_action = []() { snoozed_sitreps.clear(); };
    auto snooze_clear_indefinite_action = []() {
        snoozed_sitreps.clear();
        permanently_snoozed_sitreps.clear();
    };
    submenu_ignore.next_level.emplace_back(entry_margin + UserString("SITREP_SNOOZE_CLEAR_ALL"),
                                           false, false, snooze_clear_action);
    submenu_ignore.next_level.emplace_back(entry_margin + UserString("SITREP_SNOOZE_CLEAR_INDEFINITE"),
                                           false, false, snooze_clear_indefinite_action);
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    popup->AddMenuItem(std::move(submenu_ignore));

    submenu_block.label = entry_margin + UserString("SITREP_BLOCK_MENU");
    if (sitrep_row) {
        const SitRepEntry& sitrep_entry = sitrep_row->GetSitRepEntry();
        sitrep_template = sitrep_entry.GetLabelString();
        std::string sitrep_label = sitrep_entry.GetStringtableLookupFlag() ? UserString(sitrep_template) : sitrep_template;

        if (!sitrep_label.empty() && !sitrep_template.empty()) {
            auto hide_template_action = [&sitrep_template, this]() {
                m_hidden_sitrep_templates.insert(sitrep_template);
                SetHiddenSitRepTemplateStringsInOptions(m_hidden_sitrep_templates);
                Update();
            };
            submenu_block.next_level.emplace_back(
                entry_margin + str(FlexibleFormat(UserString("SITREP_HIDE_TEMPLATE"))
                                   % sitrep_label),
                false, false, hide_template_action);
        }
    }
    if (m_hidden_sitrep_templates.size() > 0) {
        if (sitrep_row)
            submenu_block.next_level.emplace_back(separator_item);
        auto showall_action = [this]() {
            m_hidden_sitrep_templates.clear();
            SetHiddenSitRepTemplateStringsInOptions(m_hidden_sitrep_templates);
            Update();
        };
        submenu_block.next_level.emplace_back(
            entry_margin + UserString("SITREP_SHOWALL_TEMPLATES"),
            false, false, showall_action);
    }
    popup->AddMenuItem(std::move(submenu_block));

    auto copy_action = [&sitrep_text]() {
        if (sitrep_text.empty())
            return;
        GG::GUI::GetGUI()->SetClipboardText(GG::Font::StripTags(sitrep_text));
    };
    auto help_action = []() { ClientUI::GetClientUI()->ZoomToEncyclopediaEntry("SITREP_IGNORE_BLOCK_TITLE"); };
    popup->AddMenuItem(entry_margin + UserString("HOTKEY_COPY"), false, false, copy_action);
    popup->AddMenuItem(entry_margin + UserString("POPUP_MENU_PEDIA_PREFIX") + UserString("SITREP_IGNORE_BLOCK_TITLE"),
                       false, false, help_action);

    if (!popup->Run())
        return;
    Update();
}

void SitRepPanel::Update() {
    DebugLogger() << "SitRepPanel::Update()";

    std::size_t first_visible_row = std::distance(m_sitreps_lb->begin(),
                                                  m_sitreps_lb->FirstRowShown());
    m_sitreps_lb->Clear();
    m_sitreps_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
    m_sitreps_lb->SetVScrollWheelIncrement(ClientUI::Pts()*4.5);
    m_sitreps_lb->ManuallyManageColProps();
    m_sitreps_lb->NormalizeRowsOnInsert(false);

    // Get back to sane default
    if (m_showing_turn == INVALID_GAME_TURN)
        m_showing_turn = 1;

    // get sitrep entries for this client's player empire, or for all empires
    // if this client is an observer or moderator.
    // todo: double check that no-empire players are actually moderator or
    //       observers, instead of just passing the client empire id.
    auto sitreps_by_turn = GetUnvalidatedSitRepsSortedByTurn(GGHumanClientApp::GetApp()->EmpireID());

    m_showing_turn = GetNextNonEmptySitrepsTurn(sitreps_by_turn, m_showing_turn - 1, true, m_hidden_sitrep_templates);
    if (m_showing_turn < 1)
        this->SetName(UserString("SITREP_PANEL_TITLE"));
    else
        this->SetName(boost::io::str(FlexibleFormat(UserString("SITREP_PANEL_TITLE_TURN")) % m_showing_turn));


    auto& all_current_turn_sitreps = sitreps_by_turn[m_showing_turn];


    // filter for valid / visible sitreps
    std::vector<const SitRepEntry*> current_turn_sitreps;
    current_turn_sitreps.reserve(all_current_turn_sitreps.size());
    std::copy_if(std::make_move_iterator(all_current_turn_sitreps.begin()),
                 std::make_move_iterator(all_current_turn_sitreps.end()),
                 std::back_inserter(current_turn_sitreps),
                 [this](const auto* s) { return s && !IsSitRepInvalid(*s, m_hidden_sitrep_templates); });

    // order sitreps for display
    const auto ordered_template_strings = OrderedSitrepTemplateStrings();
    std::vector<std::vector<const SitRepEntry*>> sorted_sitreps;
    sorted_sitreps.resize(ordered_template_strings.size());
    std::vector<const SitRepEntry*> remaining_unordered_sitreps;
    remaining_unordered_sitreps.reserve(current_turn_sitreps.size());

    for (auto* sitrep : current_turn_sitreps) {
        if (!sitrep)
            continue;
        const auto& s{*sitrep};
        std::string_view label_string = s.GetLabelString().empty() ? s.GetTemplateString() : s.GetLabelString();
        auto it = std::find_if(ordered_template_strings.begin(), ordered_template_strings.end(),
                               [label_string](const auto& ts) { return label_string == ts; });
        if (it == ordered_template_strings.end()) {
            remaining_unordered_sitreps.push_back(sitrep);
        } else {
            auto idx = std::distance(ordered_template_strings.begin(), it);
            sorted_sitreps[idx].push_back(sitrep);
        }
    }

    // flatten vector of vectors to single vector
    std::vector<const SitRepEntry*> ordered_sitreps;
    ordered_sitreps.reserve(current_turn_sitreps.size());
    std::for_each(sorted_sitreps.begin(), sorted_sitreps.end(),
                  [&ordered_sitreps](auto& sitrep_vec) {
                      ordered_sitreps.insert(ordered_sitreps.end(), sitrep_vec.begin(), sitrep_vec.end());
                  });

    // create UI rows for all sitrps
    const GG::X width = m_sitreps_lb->ClientWidth();
    const GG::Y height{ClientUI::Pts()*2};
    // first the ordered sitreps
    for (auto* sitrep : ordered_sitreps)
        m_sitreps_lb->Insert(GG::Wnd::Create<SitRepRow>(width, height, *sitrep));
    // then the remaining unordered sitreps
    for (auto* sitrep : remaining_unordered_sitreps)
        m_sitreps_lb->Insert(GG::Wnd::Create<SitRepRow>(width, height, *sitrep));


    if (m_sitreps_lb->NumRows() > first_visible_row) {
        m_sitreps_lb->SetFirstRowShown(std::next(m_sitreps_lb->begin(), first_visible_row));
    } else if (!m_sitreps_lb->Empty()) {
        m_sitreps_lb->BringRowIntoView(--m_sitreps_lb->end());
    }

    // if at first turn with visible sitreps, disable back button
    int prev_turn_with_sitrep = GetNextNonEmptySitrepsTurn(sitreps_by_turn, m_showing_turn, false, m_hidden_sitrep_templates);

    bool disable_prev_turn = prev_turn_with_sitrep == INVALID_GAME_TURN;
    m_prev_turn_button->Disable(disable_prev_turn);

    // if at last turn with visible sitreps, disable forward button
    int next_turn_with_sitrep = GetNextNonEmptySitrepsTurn(sitreps_by_turn, m_showing_turn, true, m_hidden_sitrep_templates);

    bool disable_next_turn = next_turn_with_sitrep == INVALID_GAME_TURN;
    m_next_turn_button->Disable(disable_next_turn);
    m_last_turn_button->Disable(disable_next_turn);
}

void SitRepPanel::ShowSitRepsForTurn(int turn) {
     bool is_different_turn(m_showing_turn != turn);
     m_showing_turn = turn;
     Update();
     if (is_different_turn)
         m_sitreps_lb->SetFirstRowShown(m_sitreps_lb->begin());
}

void SitRepPanel::SetHiddenSitRepTemplates(const std::set<std::string>& templates) {
    auto old_hidden_sitrep_templates = m_hidden_sitrep_templates;
    m_hidden_sitrep_templates = templates;
    if (old_hidden_sitrep_templates != m_hidden_sitrep_templates)
        Update();
}

int SitRepPanel::NumVisibleSitrepsThisTurn() const {
    auto turns = GetUnvalidatedSitRepsSortedByTurn(GGHumanClientApp::GetApp()->EmpireID());
    auto& this_turn_sitreps = turns[GGHumanClientApp::GetApp()->CurrentTurn()];
    auto is_valid = [this](const auto* s) { return s && !IsSitRepInvalid(*s, m_hidden_sitrep_templates); };

    return std::count_if(this_turn_sitreps.begin(), this_turn_sitreps.end(), is_valid);
}

