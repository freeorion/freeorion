#include "SitRepPanel.h"

#include "CUIControls.h"
#include "LinkText.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../Empire/Empire.h"
#include "../util/i18n.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/SitRepEntry.h"
#include "../universe/ShipDesign.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>

#include <boost/lexical_cast.hpp>

#include <iterator>


namespace {
    const int sitrep_row_margin(1);
    const int sitrep_edge_to_outline_spacing(2);
    const int sitrep_edge_to_content_spacing(sitrep_edge_to_outline_spacing + 1 + 2);
    const int sitrep_spacing(2);

    /** Adds options related to SitRepPanel to Options DB. */
    void AddOptions(OptionsDB& db) {
        db.Add("ui.map.sitrep.invalid.shown",
               UserStringNop("OPTIONS_DB_VERBOSE_SITREP_DESC"),
               false, Validator<bool>());
        db.Add<std::string>("ui.map.sitrep.hidden.stringlist",
                            UserStringNop("OPTIONS_DB_HIDDEN_SITREP_TEMPLATES_DESC"),
                            "");
        db.Add("ui.map.sitrep.icon.size",
               UserStringNop("OPTIONS_DB_UI_SITREP_ICONSIZE"),
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
                if (const ShipDesign* design = GetPredefinedShipDesign(data))
                    ClientUI::GetClientUI()->ZoomToShipDesign(design->ID());

            } else if (link_type == VarText::TECH_TAG) {
                ClientUI::GetClientUI()->ZoomToTech(data);
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
                HumanClientApp::GetApp()->BrowsePath(FilenameToPath(data));
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

        Empire *empire = HumanClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire)
            return template_set;

        for (Empire::SitRepItr sitrep_it = empire->SitRepBegin();
             sitrep_it != empire->SitRepEnd(); ++sitrep_it)
        {
            std::string label;
            if(sitrep_it->GetLabelString().empty()) {
                label = sitrep_it->GetTemplateString();
                label_display_map[label] = UserString(label + "_LABEL");
            } else {
                label = sitrep_it->GetLabelString();
                label_display_map[label] = sitrep_it->GetStringtableLookupFlag()? UserString(label) : label;
            }
            template_set.insert(label);
            
        }

        return template_set;
    }

    std::vector<std::string> AllSitRepTemplateStrings() {
        std::set<std::string> template_set;

        // get templates for each empire
        for (auto& entry : Empires()) {
            auto empire_strings = EmpireSitRepTemplateStrings(entry.first);
            template_set.insert(empire_strings.begin(), empire_strings.end());
        }

        std::vector<std::string> retval;

        // only use those ordered templates actually in the current set of sitrep templates
        for (const std::string& templ : OrderedSitrepTemplateStrings()) {
            if ( template_set.count(templ) &&
                !std::count(retval.begin(), retval.end(), templ))
            { retval.push_back(templ); }
        }

        //now add the current templates that did not have a specified order
        for (const std::string& templ : template_set)
            if (!std::count(retval.begin(), retval.end(), templ))
                retval.push_back(templ);

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

    class ColorEmpire : public LinkDecorator {
    public:
        std::string Decorate(const std::string& target, const std::string& content) const override {
            GG::Clr color = ClientUI::DefaultLinkColor();
            int id = CastStringToInt(target);
            Empire* empire = GetEmpire(id);
            if (empire)
                color = empire->Color();
            return GG::RgbaTag(color) + content + "</rgba>";
        }
    };

    //////////////////////////////////
    // SitRepLinkText
    //////////////////////////////////
    class SitRepLinkText : public LinkText {
    public:
        SitRepLinkText(GG::X x, GG::Y y, GG::X w, const std::string& str, const std::shared_ptr<GG::Font>& font,
                       GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE, GG::Clr color = GG::CLR_BLACK) :
            LinkText(x, y, w, str, font, format, color) {}

        mutable boost::signals2::signal<void(const GG::Pt&, GG::Flags<GG::ModKey>)> RightClickedSignal;

    protected:
        void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override {
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

            SetChildClippingMode(ClipToClient);

            int icon_dim = GetIconSize();
            std::string icon_texture = (m_sitrep_entry.GetIcon().empty() ?
                "/icons/sitrep/generic.png" : m_sitrep_entry.GetIcon());
            std::shared_ptr<GG::Texture> icon = ClientUI::GetTexture(
                ClientUI::ArtDir() / icon_texture, true);
            m_icon = GG::Wnd::Create<GG::StaticGraphic>(icon, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            AttachChild(m_icon);
            m_icon->Resize(GG::Pt(GG::X(icon_dim), GG::Y(icon_dim)));

            GG::Pt spacer = GG::Pt(GG::X(sitrep_edge_to_content_spacing), GG::Y(sitrep_edge_to_content_spacing));
            GG::X icon_left(spacer.x);
            GG::X text_left(icon_left + GG::X(icon_dim) + sitrep_spacing);
            GG::X text_width(ClientWidth() - text_left - spacer.x);
            m_link_text = GG::Wnd::Create<SitRepLinkText>(GG::X0, GG::Y0, text_width, m_sitrep_entry.GetText() + " ",
                                             ClientUI::GetFont(),
                                             GG::FORMAT_LEFT | GG::FORMAT_VCENTER | GG::FORMAT_WORDBREAK, ClientUI::TextColor());
            m_link_text->SetDecorator(VarText::EMPIRE_ID_TAG, new ColorEmpire());
            m_link_text->SetDecorator(TextLinker::BROWSE_PATH_TAG, new PathTypeDecorator());
            AttachChild(m_link_text);

            m_link_text->LinkClickedSignal.connect(
                &HandleLinkClick);
            m_link_text->LinkDoubleClickedSignal.connect(
                &HandleLinkClick);
            m_link_text->LinkRightClickedSignal.connect(
                &HandleLinkClick);
            m_link_text->RightClickedSignal.connect(
                boost::bind(&SitRepDataPanel::RClick, this, _1, _2));

            DoLayout(UpperLeft(), Width());
        }

        void Render() override {
            GG::Clr background_clr = this->Disabled() ? ClientUI::WndColor() : ClientUI::CtrlColor();
            GG::Pt spacer = GG::Pt(GG::X(sitrep_edge_to_outline_spacing), GG::Y(sitrep_edge_to_outline_spacing));
            GG::FlatRectangle(UpperLeft() + spacer, LowerRight() - spacer,
                              background_clr, ClientUI::WndOuterBorderColor(), 1u);
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            if (ul != ClientUpperLeft() || (lr.x - ul.x) != Width())
                DoLayout(ul, lr.x - ul.x);
        }

        const SitRepEntry& GetSitRepEntry() const { return m_sitrep_entry; }

        mutable boost::signals2::signal<void(const GG::Pt&, GG::Flags<GG::ModKey>)> RightClickedSignal;

    protected:
        void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod) override
        { RightClickedSignal(pt, mod); }

    private:
        void DoLayout(const GG::Pt& ul, const GG::X& width) {
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
        std::shared_ptr<GG::StaticGraphic>  m_icon = nullptr;
        std::shared_ptr<SitRepLinkText>     m_link_text = nullptr;
    };

    ////////////////////////////////////////////////
    // SitRepRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to display SitReps. */
    class SitRepRow : public GG::ListBox::Row {
    public:
        SitRepRow(GG::X w, GG::Y h, const SitRepEntry& sitrep) :
            GG::ListBox::Row(w, h, ""),
            m_sitrep(sitrep)
        {
            SetName("SitRepRow");
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();

            SetMargin(sitrep_row_margin);
            SetChildClippingMode(ClipToClient);
            SetMinSize(GG::Pt(GG::X(2 * GetIconSize() + 2 * sitrep_edge_to_content_spacing),
                              GG::Y(std::max(GetIconSize(), ClientUI::Pts() * 2) + 2 * sitrep_edge_to_content_spacing)));
            RequirePreRender();
        }

        void PreRender() override {
            GG::ListBox::Row::PreRender();

            if (!m_panel)
                Init();

            // Resize to fit panel after text reflows
            GG::Pt border(GG::X(2 * GetLayout()->BorderMargin()), GG::Y(2 * GetLayout()->BorderMargin()));
            m_panel->Resize(Size() - border);
            GG::ListBox::Row::Resize(m_panel->Size() + border);
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            if (!m_panel || (Size() != (lr - ul)))
                RequirePreRender();
            GG::ListBox::Row::SizeMove(ul, lr);
        }

        void Init() {
            m_panel = GG::Wnd::Create<SitRepDataPanel>(GG::X(GetLayout()->BorderMargin()), GG::Y(GetLayout()->BorderMargin()),
                                                       ClientWidth() - GG::X(2 * GetLayout()->BorderMargin()),
                                                       ClientHeight() - GG::Y(2 * GetLayout()->BorderMargin()), m_sitrep);
            push_back(m_panel);
            m_panel->RightClickedSignal.connect(
                boost::bind(&SitRepRow::RClick, this, _1, _2));
        }

        const SitRepEntry& GetSitRepEntry() const { return m_panel->GetSitRepEntry(); }

    private:
        std::shared_ptr<SitRepDataPanel>    m_panel = nullptr;
        const SitRepEntry                   m_sitrep;
    };
}

SitRepPanel::SitRepPanel(const std::string& config_name) :
    CUIWnd(UserString("SITREP_PANEL_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE,
           config_name),
    m_showing_turn(INVALID_GAME_TURN),
    m_hidden_sitrep_templates(HiddenSitRepTemplateStringsFromOptions())
{}

void SitRepPanel::CompleteConstruction() {
    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(DontClip);

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

    m_prev_turn_button->LeftClickedSignal.connect(
        boost::bind(&SitRepPanel::PrevClicked, this));
    m_next_turn_button->LeftClickedSignal.connect(
        boost::bind(&SitRepPanel::NextClicked, this));
    m_last_turn_button->LeftClickedSignal.connect(
        boost::bind(&SitRepPanel::LastClicked, this));
    m_filter_button->LeftClickedSignal.connect(
        boost::bind(&SitRepPanel::FilterClicked, this));
    m_sitreps_lb->DoubleClickedRowSignal.connect(
        boost::bind(&SitRepPanel::IgnoreSitRep, this, _1, _2, _3));
    m_sitreps_lb->RightClickedRowSignal.connect(
        boost::bind(&SitRepPanel::DismissalMenu, this, _1, _2, _3));

    CUIWnd::CompleteConstruction();

    DoLayout();
}

void SitRepPanel::DoLayout() {
    GG::X BUTTON_WIDTH(ClientUI::Pts()*4);
    GG::Y BUTTON_HEIGHT = m_last_turn_button->MinUsableSize().y;
    int PAD(3);

    GG::Pt button_ul(ClientWidth() - GG::X(INNER_BORDER_ANGLE_OFFSET) - BUTTON_WIDTH,
                     ClientHeight() - BUTTON_HEIGHT);

    m_last_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_next_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);
    m_prev_turn_button->SizeMove(button_ul, button_ul + GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    button_ul -= GG::Pt(BUTTON_WIDTH + GG::X(PAD), GG::Y0);

    m_sitreps_lb->SizeMove(GG::Pt(GG::X0, GG::Y0), GG::Pt(ClientWidth() - 1, button_ul.y));

    m_filter_button->SizeMove(GG::Pt(GG::X0, button_ul.y), GG::Pt(BUTTON_WIDTH*2, button_ul.y + BUTTON_HEIGHT));

    SetMinSize(GG::Pt(6*BUTTON_WIDTH, 6*BUTTON_HEIGHT));
}

void SitRepPanel::KeyPress(GG::Key key, std::uint32_t key_code_point,
                           GG::Flags<GG::ModKey> mod_keys)
{
    switch (key) {
    case GG::GGK_RETURN:
    case GG::GGK_KP_ENTER:
    case GG::GGK_ESCAPE:{
        CloseClicked();
        break;
    }
    default:
        break;
    }
}

void SitRepPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();
    std::size_t first_visible_queue_row = std::distance(m_sitreps_lb->begin(),
                                                        m_sitreps_lb->FirstRowShown());

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        DoLayout();
        Update();
        if (!m_sitreps_lb->Empty())
            m_sitreps_lb->SetFirstRowShown(std::next(m_sitreps_lb->begin(),
                                                     first_visible_queue_row));
    }
}

namespace {
    /* Sort sitreps for each turn.
     Note: Validating requires substituting all of the variables which is time
     consuming for sitreps that the player will never view. */
    std::map<int, std::list<SitRepEntry>> GetUnvalidatedSitRepsSortedByTurn(int empire_id) {
        std::set<Empire*> sr_empires;
        Empire* empire = GetEmpire(empire_id);
        if (empire) {
            sr_empires.insert(empire);
        } else {
            // Moderator mode, sort sitreps from all empires
            EmpireManager& empires = Empires();
            for (auto& entry : empires)
                sr_empires.insert(entry.second);
        }

        std::map<int, std::list<SitRepEntry>> turns;
        for (auto sitrep_empire : sr_empires) {
            for (auto sitrep_it = sitrep_empire->SitRepBegin(); sitrep_it != sitrep_empire->SitRepEnd(); ++sitrep_it) {
                turns[sitrep_it->GetTurn()].push_back(*sitrep_it);
            }
        }
        return turns;
    }
}

/** Return true if the sitrep is not hidden, validates and is not snoozed. */
bool SitRepPanel::IsSitRepInvalid(SitRepEntry& sitrep) const {
    if (m_hidden_sitrep_templates.count(sitrep.GetLabelString().empty() ?
        sitrep.GetTemplateString() : sitrep.GetLabelString()))
    { return true; }

    // Validation is time consuming because all variables are substituted
    bool validated = sitrep.Validate();

    // having ui.map.sitrep.invalid.shown off / disabled will hide sitreps that do not
    // validate
    bool verbose_sitrep = GetOptionsDB().Get<bool>("ui.map.sitrep.invalid.shown");
    if (!verbose_sitrep && !validated)
        return true;

    // Check for snoozing.
    if (permanently_snoozed_sitreps.count(sitrep.GetText()))
        return true;

    auto sitrep_set_it = snoozed_sitreps.find(sitrep.GetTurn());
    if (sitrep_set_it != snoozed_sitreps.end()
        && sitrep_set_it->second.count(sitrep.GetText()))
    { return true; }

    return false;
}

int SitRepPanel::GetNextNonEmptySitrepsTurn(std::map<int, std::list<SitRepEntry>>& turns,
                                            int turn, bool forward) const
{
    // All sitreps filtered out ?
    if (turns.size() == 0)
        return INVALID_GAME_TURN;

    // Only one turn with visible sitreps
    if (turns.size() == 1) {
        turns.begin()->second.remove_if(std::bind(&SitRepPanel::IsSitRepInvalid,
                                                  this, std::placeholders::_1));
        // With no valid sitreps
        if (turns.begin()->second.empty()) {
            turns.clear();
            return INVALID_GAME_TURN;
        }
        return turns.begin()->first;
    }

    // Before first turn with visible sitreps
    if (turn < turns.begin()->first && !forward)
        return INVALID_GAME_TURN;

    // After last turn with visible sitreps
    if (turn > (--turns.end())->first && forward)
        return INVALID_GAME_TURN;

    // Find a starting point
    auto it = (forward ? turns.upper_bound(turn) : turns.lower_bound(turn));
    if (!forward && it != turns.begin())
        --it;
    int step = forward ? 1 : -1;

    while (it != turns.end()) {
        it->second.remove_if(std::bind(&SitRepPanel::IsSitRepInvalid,
                                       this, std::placeholders::_1));

        // If any valid sitreps, then exit
        if (!it->second.empty())
            return it->first;

        // Get the next candidate and remove the empty cell.
        if (!forward && it == turns.begin()) {
            turns.erase(it);
            break;
        }

        auto erase_it = it;
        std::advance(it, step);
        turns.erase(erase_it);
    }
    return INVALID_GAME_TURN;
}

void SitRepPanel::CloseClicked()
{ ClosingSignal(); }

void SitRepPanel::PrevClicked() {
    auto turns = GetUnvalidatedSitRepsSortedByTurn(HumanClientApp::GetApp()->EmpireID());
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(turns, m_showing_turn, false));
}

void SitRepPanel::NextClicked() {
    auto turns = GetUnvalidatedSitRepsSortedByTurn(HumanClientApp::GetApp()->EmpireID());
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(turns, m_showing_turn, true));
}

void SitRepPanel::LastClicked()
{ ShowSitRepsForTurn(CurrentTurn()); }

void SitRepPanel::FilterClicked() {
    std::map<int, std::string> menu_index_templates;
    std::map<int, bool> menu_index_checked;
    int index = 1;
    bool all_checked = true;

    auto all_templates = AllSitRepTemplateStrings();

    auto popup = GG::Wnd::Create<CUIPopupMenu>(m_filter_button->Left(),
                                               m_filter_button->Bottom());

    for (const std::string& templ : all_templates) {
        menu_index_templates[index] = templ;
        bool check = true;
        if (m_hidden_sitrep_templates.count(templ)) {
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

        popup->AddMenuItem(GG::MenuItem(menu_label, false, check, select_template_action));
        ++index;
    }

    auto all_templates_action = [all_checked, &all_templates, this]() {
        // select / deselect all templates
        if (all_checked) {
            // deselect all
            for (const std::string& templ : all_templates) {
                m_hidden_sitrep_templates.insert(templ);
            }
        } else {
            // select all
            m_hidden_sitrep_templates.clear();
        }
    };
    popup->AddMenuItem(GG::MenuItem((all_checked ? UserString("NONE") : UserString("ALL")),
                                   false, false, all_templates_action));

    if (!popup->Run())
        return;

    SetHiddenSitRepTemplateStringsInOptions(m_hidden_sitrep_templates);

    Update();
}

void SitRepPanel::IgnoreSitRep(GG::ListBox::iterator it, const GG::Pt& pt,
                               const GG::Flags<GG::ModKey>& mod)
{
    SitRepRow* sitrep_row = dynamic_cast<SitRepRow*>(it->get());
    if (!sitrep_row)
        return;

    const SitRepEntry& sitrep = sitrep_row->GetSitRepEntry();
    if (sitrep.GetTurn() <= 0)
        return;
    snoozed_sitreps[sitrep.GetTurn()].insert(sitrep.GetText());

    Update();
}

void SitRepPanel::DismissalMenu(GG::ListBox::iterator it, const GG::Pt& pt,
                                const GG::Flags<GG::ModKey>& mod)
{
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
        sitrep_text = sitrep_entry.GetText();
        start_turn = sitrep_entry.GetTurn();
        if (start_turn > 0) {
            auto snooze5_action = [&sitrep_text, start_turn]() { SnoozeSitRepForNTurns(sitrep_text, start_turn, 5); };
            auto snooze10_action = [&sitrep_text, start_turn]() { SnoozeSitRepForNTurns(sitrep_text, start_turn, 10); };
            auto snooze_indefinite_action = [&sitrep_text]() { permanently_snoozed_sitreps.insert(sitrep_text); };

            submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_5_TURNS"),
                                                             false, false, snooze5_action));
            submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_10_TURNS"),
                                                             false, false, snooze10_action));
            submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_INDEFINITE"),
                                                             false, false, snooze_indefinite_action));
            submenu_ignore.next_level.push_back(separator_item);
        }
    }


    auto snooze_clear_action = []() { snoozed_sitreps.clear(); };
    auto snooze_clear_indefinite_action = []() {
        snoozed_sitreps.clear();
        permanently_snoozed_sitreps.clear();
    };
    submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_CLEAR_ALL"),
                                                     false, false, snooze_clear_action));
    submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_CLEAR_INDEFINITE"),
                                                     false, false, snooze_clear_indefinite_action));
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
            submenu_block.next_level.push_back(GG::MenuItem(
                entry_margin + str(FlexibleFormat(UserString("SITREP_HIDE_TEMPLATE"))
                                   % sitrep_label),
                false, false, hide_template_action));
        }
    }
    if (m_hidden_sitrep_templates.size() > 0) {
        if (sitrep_row)
            submenu_block.next_level.push_back(separator_item);
        auto showall_action = [this]() {
            m_hidden_sitrep_templates.clear();
            SetHiddenSitRepTemplateStringsInOptions(m_hidden_sitrep_templates);
            Update();
        };
        submenu_block.next_level.push_back(GG::MenuItem(
            entry_margin + UserString("SITREP_SHOWALL_TEMPLATES"),
            false, false, showall_action));
    }
    popup->AddMenuItem(std::move(submenu_block));

    auto copy_action = [&sitrep_text]() {
        if (sitrep_text.empty())
            return;
        GG::GUI::GetGUI()->SetClipboardText(GG::Font::StripTags(sitrep_text));
    };
    auto help_action = []() { ClientUI::GetClientUI()->ZoomToEncyclopediaEntry("SITREP_IGNORE_BLOCK_TITLE"); };

    popup->AddMenuItem(GG::MenuItem(entry_margin + UserString("HOTKEY_COPY"),
                                    false, false, copy_action));
    popup->AddMenuItem(GG::MenuItem(entry_margin + UserString("POPUP_MENU_PEDIA_PREFIX") +
                                        UserString("SITREP_IGNORE_BLOCK_TITLE"),
                                    false, false, help_action));

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
    auto turns = GetUnvalidatedSitRepsSortedByTurn(HumanClientApp::GetApp()->EmpireID());

    m_showing_turn = GetNextNonEmptySitrepsTurn(turns, m_showing_turn - 1, true);
    auto& current_turn_sitreps = turns[m_showing_turn];

    if (m_showing_turn < 1)
        this->SetName(UserString("SITREP_PANEL_TITLE"));
    else
        this->SetName(boost::io::str(FlexibleFormat(UserString("SITREP_PANEL_TITLE_TURN")) % m_showing_turn));

    // order sitreps for display
    std::vector<SitRepEntry> ordered_sitreps;
    for (const auto& templ : OrderedSitrepTemplateStrings()) {
        for (auto sitrep_it = current_turn_sitreps.begin();
             sitrep_it != current_turn_sitreps.end(); ++sitrep_it)
        {
            if (templ == (sitrep_it->GetLabelString().empty() ?
                 sitrep_it->GetTemplateString() : sitrep_it->GetLabelString()))
            {
                //DebugLogger() << "saving into ordered_sitreps -  sitrep of template " << templ << " with full string "<< sitrep_it->GetText();
                ordered_sitreps.push_back(*sitrep_it);
                //DebugLogger()<< "deleting above sitrep from current_turn_sitreps";
                sitrep_it = --current_turn_sitreps.erase(sitrep_it);
            }
        }
    }

    // copy remaining unordered sitreps
    for (const SitRepEntry& sitrep : current_turn_sitreps)
    { ordered_sitreps.push_back(sitrep); }

    // create UI rows for all sitrps
    GG::X width = m_sitreps_lb->ClientWidth();
    for (const SitRepEntry& sitrep : ordered_sitreps)
    { m_sitreps_lb->Insert(GG::Wnd::Create<SitRepRow>(width, GG::Y(ClientUI::Pts()*2), sitrep)); }

    if (m_sitreps_lb->NumRows() > first_visible_row) {
        m_sitreps_lb->SetFirstRowShown(std::next(m_sitreps_lb->begin(), first_visible_row));
    } else if (!m_sitreps_lb->Empty()) {
        m_sitreps_lb->BringRowIntoView(--m_sitreps_lb->end());
    }

    // if at first turn with visible sitreps, disable back button
    int prev_turn_with_sitrep = GetNextNonEmptySitrepsTurn(turns, m_showing_turn, false);

    bool disable_prev_turn = prev_turn_with_sitrep == INVALID_GAME_TURN;
    m_prev_turn_button->Disable(disable_prev_turn);

    // if at last turn with visible sitreps, disable forward button
    int next_turn_with_sitrep = GetNextNonEmptySitrepsTurn(turns, m_showing_turn, true);

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
    auto turns = GetUnvalidatedSitRepsSortedByTurn(HumanClientApp::GetApp()->EmpireID());
    auto& turn = turns[CurrentTurn()];
    turn.remove_if(std::bind(&SitRepPanel::IsSitRepInvalid, this, std::placeholders::_1));
    return turn.size();
}

