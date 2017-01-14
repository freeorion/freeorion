#include "SitRepPanel.h"

#include "CUIControls.h"
#include "LinkText.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../Empire/Empire.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/SitRepEntry.h"
#include "../universe/ShipDesign.h"

#include <GG/DrawUtil.h>
#include <GG/Layout.h>

#include <boost/lexical_cast.hpp>


namespace {
    const int sitrep_row_margin(1);
    const int sitrep_edge_to_outline_spacing(2);
    const int sitrep_edge_to_content_spacing(sitrep_edge_to_outline_spacing + 1 + 2);
    const int sitrep_spacing(2);

    /** Adds options related to SitRepPanel to Options DB. */
    void AddOptions(OptionsDB& db) {
        db.Add("verbose-sitrep", UserStringNop("OPTIONS_DB_VERBOSE_SITREP_DESC"),  false,  Validator<bool>());
        db.Add<std::string>("hidden-sitrep-templates", UserStringNop("OPTIONS_DB_HIDDEN_SITREP_TEMPLATES_DESC"), "");
        db.Add("UI.sitrep-icon-size", UserStringNop("OPTIONS_DB_UI_SITREP_ICONSIZE"), 24, RangedValidator<int>(12, 64));
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    int GetIconSize()
    { return GetOptionsDB().Get<int>("UI.sitrep-icon-size"); }

    std::map<std::string, std::string> label_display_map;
    std::map<int, std::set<std::string> > snoozed_sitreps;
    std::set<std::string> permanently_snoozed_sitreps;

    void SnoozeSitRepForNTurns(std::string sitrep_text, int start_turn, int num_turns) {
        for (int turn=start_turn; turn < start_turn + num_turns; turn++)
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

            } else if (link_type == TextLinker::ENCYCLOPEDIA_TAG) {
                ClientUI::GetClientUI()->ZoomToEncyclopediaEntry(data);
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
                  std::back_inserter<std::vector<std::string> >(sitrep_order));
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
        for (std::map<int, Empire*>::value_type& entry : Empires()) {
            std::set<std::string> empire_strings = EmpireSitRepTemplateStrings(entry.first);
            template_set.insert(empire_strings.begin(), empire_strings.end());
        }

        std::vector<std::string> retval;

        // only use those ordered templates actually in the current set of sitrep templates
        for (const std::string& templ : OrderedSitrepTemplateStrings()) {
            if ( (template_set.find(templ) != template_set.end()) &&
                 (std::find(retval.begin(), retval.end(), templ) == retval.end()) )
            { retval.push_back(templ); }
        }

        //now add the current templates that did not have a specified order
        for (const std::string& templ : template_set)
            if (std::find(retval.begin(), retval.end(), templ) == retval.end())
                retval.push_back(templ);

        return retval;
    }

    std::set<std::string> HiddenSitRepTemplateStringsFromOptions() {
        std::set<std::string> result;
        std::string saved_template_string = GetOptionsDB().Get<std::string>("hidden-sitrep-templates");

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
        std::copy(
            set.begin(),
            set.end(),
            std::ostream_iterator<std::string>(ss, " ")
        );

        GetOptionsDB().Set<std::string>("hidden-sitrep-templates", ss.str());
        GetOptionsDB().Commit();
    }

    class ColorEmpire : public LinkDecorator {
    public:
        virtual std::string Decorate(const std::string& target, const std::string& content) const {
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
        SitRepLinkText(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font,
                       GG::Flags<GG::TextFormat> format = GG::FORMAT_NONE, GG::Clr color = GG::CLR_BLACK) :
            LinkText(x, y, w, str, font, format, color) {}

        mutable boost::signals2::signal<void(const GG::Pt&, GG::Flags<GG::ModKey>)> RightClickedSignal;

    protected:
        void RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
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
            m_initialized(false),
            m_sitrep_entry(sitrep),
            m_icon(0),
            m_link_text(0)
        {
            SetChildClippingMode(ClipToClient);
            Init();
            DoLayout(GG::Pt(left, top), w);
        }

        virtual void        Render() {
            GG::Clr background_clr = this->Disabled() ? ClientUI::WndColor() : ClientUI::CtrlColor();
            GG::Pt spacer = GG::Pt(GG::X(sitrep_edge_to_outline_spacing), GG::Y(sitrep_edge_to_outline_spacing));
            GG::FlatRectangle(UpperLeft() + spacer, LowerRight() - spacer,
                              background_clr, ClientUI::WndOuterBorderColor(), 1u);
        }

        virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            if (ul != ClientUpperLeft() || (lr.x - ul.x) != Width())
                DoLayout(ul, lr.x - ul.x);
        }

        const SitRepEntry&  GetSitRepEntry() const { return m_sitrep_entry; }

        mutable boost::signals2::signal<void(const GG::Pt&, GG::Flags<GG::ModKey>)> RightClickedSignal;

    protected:
        void            RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod) { RightClickedSignal(pt, mod); }

    private:
        void            DoLayout(const GG::Pt& ul, const GG::X& width) {
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

        void            Init() {
            m_initialized = true;

            int icon_dim = GetIconSize();
            std::string icon_texture = (m_sitrep_entry.GetIcon().empty() ?
                "/icons/sitrep/generic.png" : m_sitrep_entry.GetIcon());
            boost::shared_ptr<GG::Texture> icon = ClientUI::GetTexture(
                ClientUI::ArtDir() / icon_texture, true);
            m_icon = new GG::StaticGraphic(icon, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            AttachChild(m_icon);
            m_icon->Resize(GG::Pt(GG::X(icon_dim), GG::Y(icon_dim)));

            GG::Pt spacer = GG::Pt(GG::X(sitrep_edge_to_content_spacing), GG::Y(sitrep_edge_to_content_spacing));
            GG::X icon_left(spacer.x);
            GG::X text_left(icon_left + GG::X(icon_dim) + sitrep_spacing);
            GG::X text_width(ClientWidth() - text_left - spacer.x);
            m_link_text = new SitRepLinkText(GG::X0, GG::Y0, text_width, m_sitrep_entry.GetText() + " ",
                                             ClientUI::GetFont(),
                                             GG::FORMAT_LEFT | GG::FORMAT_VCENTER | GG::FORMAT_WORDBREAK, ClientUI::TextColor());
            m_link_text->SetDecorator(VarText::EMPIRE_ID_TAG, new ColorEmpire());
            AttachChild(m_link_text);

            GG::Connect(m_link_text->LinkClickedSignal,       &HandleLinkClick);
            GG::Connect(m_link_text->LinkDoubleClickedSignal, &HandleLinkClick);
            GG::Connect(m_link_text->LinkRightClickedSignal,  &HandleLinkClick);
            GG::Connect(m_link_text->RightClickedSignal,      &SitRepDataPanel::RClick,     this);
        }

        bool                m_initialized;

        const SitRepEntry&  m_sitrep_entry;
        GG::StaticGraphic*  m_icon;
        SitRepLinkText*     m_link_text;
    };

    ////////////////////////////////////////////////
    // SitRepRow
    ////////////////////////////////////////////////
    /** A ListBox::Row subclass used to display SitReps. */
    class SitRepRow : public GG::ListBox::Row {
    public:
        SitRepRow(GG::X w, GG::Y h, const SitRepEntry& sitrep) :
            GG::ListBox::Row(w, h, ""),
            m_panel(0),
            m_sitrep(sitrep)
        {
            SetName("SitRepRow");
            SetMargin(sitrep_row_margin);
            SetChildClippingMode(ClipToClient);
            SetMinSize(GG::Pt(GG::X(2 * GetIconSize() + 2 * sitrep_edge_to_content_spacing),
                              GG::Y(std::max(GetIconSize(), ClientUI::Pts() * 2) + 2 * sitrep_edge_to_content_spacing)));
            RequirePreRender();
        }

        virtual void        PreRender() {
            GG::ListBox::Row::PreRender();

            if (!m_panel)
                Init();

            // Resize to fit panel after text reflows
            GG::Pt border(GG::X(2 * GetLayout()->BorderMargin()), GG::Y(2 * GetLayout()->BorderMargin()));
            m_panel->Resize(Size() - border);
            GG::ListBox::Row::Resize(m_panel->Size() + border);
        }

        virtual void        SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            if (!m_panel || (Size() != (lr - ul)))
                RequirePreRender();
            GG::ListBox::Row::SizeMove(ul, lr);
        }

        void            Init() {
            m_panel = new SitRepDataPanel(GG::X(GetLayout()->BorderMargin()), GG::Y(GetLayout()->BorderMargin()),
                                          ClientWidth() - GG::X(2 * GetLayout()->BorderMargin()),
                                          ClientHeight() - GG::Y(2 * GetLayout()->BorderMargin()), m_sitrep);
            push_back(m_panel);
            GG::Connect(m_panel->RightClickedSignal, &SitRepRow::RClick, this);
        }

        const SitRepEntry&  GetSitRepEntry() const { return m_panel->GetSitRepEntry(); }

    private:
        SitRepDataPanel*    m_panel;
        const SitRepEntry   m_sitrep;
    };
}

SitRepPanel::SitRepPanel(const std::string& config_name) :
    CUIWnd(UserString("SITREP_PANEL_TITLE"),
           GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE | GG::RESIZABLE | CLOSABLE | PINABLE,
           config_name),
    m_sitreps_lb(0),
    m_prev_turn_button(0),
    m_next_turn_button(0),
    m_last_turn_button(0),
    m_filter_button(0),
    m_showing_turn(INVALID_GAME_TURN),
    m_hidden_sitrep_templates(HiddenSitRepTemplateStringsFromOptions())
{
    Sound::TempUISoundDisabler sound_disabler;
    SetChildClippingMode(DontClip);

    m_sitreps_lb = new CUIListBox();
    m_sitreps_lb->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
    m_sitreps_lb->SetVScrollWheelIncrement(ClientUI::Pts()*4.5);
    m_sitreps_lb->ManuallyManageColProps();
    m_sitreps_lb->NormalizeRowsOnInsert(false);
    AttachChild(m_sitreps_lb);

    m_prev_turn_button = new CUIButton(UserString("BACK"));
    AttachChild(m_prev_turn_button);
    m_next_turn_button = new CUIButton(UserString("NEXT"));
    AttachChild(m_next_turn_button);
    m_last_turn_button = new CUIButton(UserString("LAST"));
    AttachChild(m_last_turn_button);
    m_filter_button = new CUIButton(UserString("FILTERS"));
    AttachChild(m_filter_button);

    GG::Connect(m_prev_turn_button->LeftClickedSignal,  &SitRepPanel::PrevClicked,          this);
    GG::Connect(m_next_turn_button->LeftClickedSignal,  &SitRepPanel::NextClicked,          this);
    GG::Connect(m_last_turn_button->LeftClickedSignal,  &SitRepPanel::LastClicked,          this);
    GG::Connect(m_filter_button->LeftClickedSignal,     &SitRepPanel::FilterClicked,        this);
    GG::Connect(m_sitreps_lb->DoubleClickedSignal,      &SitRepPanel::IgnoreSitRep,         this);
    GG::Connect(m_sitreps_lb->RightClickedSignal,       &SitRepPanel::DismissalMenu,        this);

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

void SitRepPanel::KeyPress(GG::Key key, boost::uint32_t key_code_point,
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
    std::size_t first_visible_queue_row = std::distance(m_sitreps_lb->begin(), m_sitreps_lb->FirstRowShown());

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size()) {
        DoLayout();
        Update();
        if (!m_sitreps_lb->Empty())
            m_sitreps_lb->SetFirstRowShown(boost::next(m_sitreps_lb->begin(), first_visible_queue_row));
    }
}

namespace {
    /* Sort empire's sitreps for each turn */
    std::map<int, std::list<SitRepEntry> > GetSitRepsSortedByTurn(int empire_id, std::set<std::string> hidden_sitrep_templates) {
        std::map<int, std::list<SitRepEntry> > turns;
        bool verbose_sitrep = GetOptionsDB().Get<bool>("verbose-sitrep");
        std::set<Empire*> sr_empires;
        Empire* empire = GetEmpire(empire_id);
        if (empire) {
            sr_empires.insert(empire);
        } else {
            // Moderator mode, sort sitreps from all empires
            EmpireManager& empires = Empires();
            for (std::map<int, Empire*>::value_type& entry : empires)
                sr_empires.insert(entry.second);
        }
        for (Empire* empire : sr_empires) {
            for (Empire::SitRepItr sitrep_it = empire->SitRepBegin(); sitrep_it != empire->SitRepEnd(); ++sitrep_it) {
                if (!verbose_sitrep) {
                    if (!sitrep_it->Validate())
                        continue;
                }
                if (hidden_sitrep_templates.find(sitrep_it->GetLabelString().empty() ? sitrep_it->GetTemplateString() : sitrep_it->GetLabelString()) != hidden_sitrep_templates.end())
                    continue;
                if (permanently_snoozed_sitreps.find(sitrep_it->GetText()) != permanently_snoozed_sitreps.end())
                    continue;
                std::map< int, std::set< std::string > >::iterator sitrep_set_it = snoozed_sitreps.find(sitrep_it->GetTurn());
                if (sitrep_set_it != snoozed_sitreps.end() && sitrep_set_it->second.find(sitrep_it->GetText()) != sitrep_set_it->second.end())
                    continue;
                turns[sitrep_it->GetTurn()].push_back(*sitrep_it);
            }
        }
        return turns;
    }
}

int SitRepPanel::GetNextNonEmptySitrepsTurn(const std::map<int, std::list<SitRepEntry> >& turns, int turn, bool forward) const {
    // All sitreps filtered out ?
    if (turns.size() == 0)
        return INVALID_GAME_TURN;
    // Only one turn with visible sitreps
    if (turns.size() == 1)
        return turns.begin()->first;
    // Before first turn with visible sitreps
    if (turn < turns.begin()->first)
        return turns.begin()->first;
    // After last turn with visible sitreps
    if (turn > (--turns.end())->first)
        return (--turns.end())->first;

    // Search a suitable turn
    std::map<int, std::list<SitRepEntry> >::const_iterator it = turns.find(turn);
    if (it != turns.end()) {
        int step = forward ? 1 : -1;

        for (std::advance(it, step); it != turns.end(); std::advance(it, step)) {
            if (it->second.size() > 0)
                return it->first;
        }
    }

    // Not found, choose a default value
    int ret = forward ? (--turns.end())->first : turns.begin()->first;
    if (turn != ret || turns.find(ret) == turns.end())
        ret = turns.begin()->first;
    return ret;
}

void SitRepPanel::CloseClicked()
{ ClosingSignal(); }

void SitRepPanel::PrevClicked() {
    std::map<int, std::list<SitRepEntry> > turns = GetSitRepsSortedByTurn(HumanClientApp::GetApp()->EmpireID(), m_hidden_sitrep_templates);
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(turns, m_showing_turn, false));
}

void SitRepPanel::NextClicked() {
    std::map<int, std::list<SitRepEntry> > turns = GetSitRepsSortedByTurn(HumanClientApp::GetApp()->EmpireID(), m_hidden_sitrep_templates);
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(turns, m_showing_turn, true));
}

void SitRepPanel::LastClicked() {
    ShowSitRepsForTurn(CurrentTurn());
}

void SitRepPanel::FilterClicked() {
    std::map<int, std::string> menu_index_templates;
    std::map<int, bool> menu_index_checked;
    int index = 1;
    bool all_checked = true;
    int ALL_INDEX = 9999;

    std::vector<std::string> all_templates = AllSitRepTemplateStrings();

    GG::MenuItem menu_contents;
    for (const std::string& templ : all_templates) {
        menu_index_templates[index] = templ;
        bool check = true;
        if (m_hidden_sitrep_templates.find(templ) != m_hidden_sitrep_templates.end()) {
            check = false;
            all_checked = false;
        }
        menu_index_checked[index] = check;
        const std::string& menu_label =  label_display_map[templ];
        menu_contents.next_level.push_back(GG::MenuItem(menu_label, index, false, check));
        ++index;
    }
    menu_contents.next_level.push_back(GG::MenuItem((all_checked ? UserString("NONE") : UserString("ALL")),
                                       ALL_INDEX, false, false));

    CUIPopupMenu popup(m_filter_button->Left(), m_filter_button->Bottom(), menu_contents);
    if (!popup.Run())
        return;
    int selected_menu_item = popup.MenuID();
    if (selected_menu_item == 0)
        return; // nothing was selected

    if (selected_menu_item == ALL_INDEX) {
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
    } else {
        // select / deselect the chosen template
        const std::string& selected_template_string = menu_index_templates[selected_menu_item];
        if (menu_index_checked[selected_menu_item]) {
            // disable showing this template string
            m_hidden_sitrep_templates.insert(selected_template_string);
        } else {
            // re-enabled showing this template string
            m_hidden_sitrep_templates.erase(selected_template_string);
        }
    }
    SetHiddenSitRepTemplateStringsInOptions(m_hidden_sitrep_templates);

    Update();
}

void SitRepPanel::IgnoreSitRep(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& mod) {
    SitRepRow* sitrep_row = dynamic_cast<SitRepRow*>(*it);
    if (!sitrep_row) {
        return;
    }
    const SitRepEntry& sitrep = sitrep_row->GetSitRepEntry();
    if (sitrep.GetTurn() <= 0)
        return;
    snoozed_sitreps[sitrep.GetTurn()].insert(sitrep.GetText());

    Update();
}

void SitRepPanel::DismissalMenu(GG::ListBox::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& mod) {

    GG::MenuItem menu_contents, submenu_ignore, submenu_block, separator_item;
    std::string sitrep_text, sitrep_template;
    std::string entry_margin("  ");
    separator_item.separator = true;
    int start_turn = 0;
    SitRepRow* sitrep_row(0);
    if (it != m_sitreps_lb->end()) 
        sitrep_row = dynamic_cast<SitRepRow*>(*it);
    submenu_ignore.label = entry_margin + UserString("SITREP_IGNORE_MENU");
    if (sitrep_row) {
        const SitRepEntry& sitrep_entry = sitrep_row->GetSitRepEntry();
        sitrep_text = sitrep_entry.GetText();
        start_turn = sitrep_entry.GetTurn();
        if (start_turn > 0) {
            submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_5_TURNS"),      1,
                                                             false, false));
            submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_10_TURNS"),     2,
                                                             false, false));
            submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_INDEFINITE"),   3,
                                                             false, false));
            submenu_ignore.next_level.push_back(separator_item);
        }
    }
    submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_CLEAR_ALL"),            4,
                                                     false, false));
    submenu_ignore.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SNOOZE_CLEAR_INDEFINITE"),     5,
                                                     false, false));
    menu_contents.next_level.push_back(submenu_ignore);

    submenu_block.label = entry_margin + UserString("SITREP_BLOCK_MENU");
    if (sitrep_row) {
        const SitRepEntry& sitrep_entry = sitrep_row->GetSitRepEntry();
        sitrep_template = sitrep_entry.GetLabelString();
        std::string sitrep_label = sitrep_entry.GetStringtableLookupFlag() ? UserString(sitrep_template) : sitrep_template;

        if (!sitrep_label.empty() && !sitrep_template.empty()) {
            submenu_block.next_level.push_back(GG::MenuItem(entry_margin + str(FlexibleFormat(UserString("SITREP_HIDE_TEMPLATE"))
                                                                               % sitrep_label),                       7,
                                                            false, false));
        }
    }
    if (m_hidden_sitrep_templates.size() > 0) {
        if (sitrep_row)
            submenu_block.next_level.push_back(separator_item);
        submenu_block.next_level.push_back(GG::MenuItem(entry_margin + UserString("SITREP_SHOWALL_TEMPLATES"),        8,
                                                        false, false));
    }
    menu_contents.next_level.push_back(submenu_block);

    menu_contents.next_level.push_back(GG::MenuItem(entry_margin + UserString("HOTKEY_COPY"),                         9,
                                                    false, false));
    menu_contents.next_level.push_back(GG::MenuItem(entry_margin + UserString("POPUP_MENU_PEDIA_PREFIX") +
                                                    UserString("SITREP_IGNORE_BLOCK_TITLE"),                         10,
                                                    false, false));

    CUIPopupMenu popup(pt.x, pt.y, menu_contents);
    if (!popup.Run())
        return;
    int selected_menu_item = popup.MenuID();
    if (selected_menu_item == 0)
        return; // nothing was selected

    switch (popup.MenuID()) {
    case 1: { //
        SnoozeSitRepForNTurns(sitrep_text, start_turn, 5);
        break;
    }
    case 2: { //
        SnoozeSitRepForNTurns(sitrep_text, start_turn, 10);
        break;
    }
    case 3: { //
        permanently_snoozed_sitreps.insert(sitrep_text);
        break;
    }
    case 4: { //
        snoozed_sitreps.clear();
    }
    case 5: { //
        permanently_snoozed_sitreps.clear();
        break;
    }
    case 7: { // Add template for the row entry to hidden templates
        m_hidden_sitrep_templates.insert(sitrep_template);
        SetHiddenSitRepTemplateStringsInOptions(m_hidden_sitrep_templates);
        Update();
        break;
    }
    case 8: { // Show all hidden templates
        m_hidden_sitrep_templates.clear();
        SetHiddenSitRepTemplateStringsInOptions(m_hidden_sitrep_templates);
        Update();
        break;
    }
    case 9: { // Copy text of sitrep
        if (sitrep_text.empty())
            break;
        GG::GUI::GetGUI()->SetClipboardText(GG::Font::StripTags(sitrep_text));
        break;
    }
    case 10: { // Display help article
        ClientUI::GetClientUI()->ZoomToEncyclopediaEntry("SITREP_IGNORE_BLOCK_TITLE");
        break;
    }

    default:
        break;
    }
    Update();
}

void SitRepPanel::Update() {
    DebugLogger() << "SitRepPanel::Update()";

    std::size_t first_visible_row = std::distance(m_sitreps_lb->begin(), m_sitreps_lb->FirstRowShown());
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
    std::map<int, std::list<SitRepEntry> > turns = GetSitRepsSortedByTurn(HumanClientApp::GetApp()->EmpireID(), m_hidden_sitrep_templates);

    std::list<SitRepEntry> currentTurnSitreps;
    if (turns.find(m_showing_turn) != turns.end())
        currentTurnSitreps = turns[m_showing_turn];
    if (currentTurnSitreps.size() == 0) {
        m_showing_turn = GetNextNonEmptySitrepsTurn(turns, m_showing_turn, false);
        currentTurnSitreps = turns[m_showing_turn];
    }

    if (m_showing_turn < 1)
        this->SetName(UserString("SITREP_PANEL_TITLE"));
    else
        this->SetName(boost::io::str(FlexibleFormat(UserString("SITREP_PANEL_TITLE_TURN")) % m_showing_turn));

    // order sitreps for display
    std::vector<SitRepEntry> orderedSitreps;
    for (const std::string& templ : OrderedSitrepTemplateStrings()) {
        for (std::list<SitRepEntry>::iterator sitrep_it = currentTurnSitreps.begin();
             sitrep_it != currentTurnSitreps.end(); ++sitrep_it)
        {
            if ((sitrep_it->GetLabelString().empty() ? sitrep_it->GetTemplateString() : sitrep_it->GetLabelString()) == templ) {
                //DebugLogger() << "saving into orderedSitreps -  sitrep of template " << templ << " with full string "<< sitrep_it->GetText();
                orderedSitreps.push_back(*sitrep_it);
                //DebugLogger()<< "deleting above sitrep from currentTurnSitreps";
                sitrep_it = --currentTurnSitreps.erase(sitrep_it);
            }
        }
    }

    // copy remaining unordered sitreps
    for (const SitRepEntry& sitrep : currentTurnSitreps)
    { orderedSitreps.push_back(sitrep); }

    // create UI rows for all sitrps
    GG::X width = m_sitreps_lb->ClientWidth();
    for (const SitRepEntry& sitrep : orderedSitreps)
    { m_sitreps_lb->Insert(new SitRepRow(width, GG::Y(ClientUI::Pts()*2), sitrep)); }

    if (m_sitreps_lb->NumRows() > first_visible_row) {
        m_sitreps_lb->SetFirstRowShown(boost::next(m_sitreps_lb->begin(), first_visible_row));
    } else if (!m_sitreps_lb->Empty()) {
        m_sitreps_lb->BringRowIntoView(--m_sitreps_lb->end());
    }

    // if at first turn with visible sitreps, disable back button
    int firstTurnWithSR = GetNextNonEmptySitrepsTurn(turns, m_showing_turn, false);
    if ((m_showing_turn < 1) || (turns.size() < 2) || (m_showing_turn == firstTurnWithSR)) {
        m_prev_turn_button->Disable();
    } else {
        m_prev_turn_button->Disable(false);
    }

    // if at last turn with visible sitreps, disable forward button
    int lastTurnWithSR = GetNextNonEmptySitrepsTurn(turns, m_showing_turn, true);
    if ((turns.size() < 2) || (m_showing_turn == lastTurnWithSR)) {
        m_next_turn_button->Disable();
        m_last_turn_button->Disable();
    } else {
        m_next_turn_button->Disable(false);
        m_last_turn_button->Disable(false);
    }
}

void SitRepPanel::ShowSitRepsForTurn(int turn) {
     bool is_different_turn(m_showing_turn != turn);
     m_showing_turn = turn;
     Update();
     if (is_different_turn)
         m_sitreps_lb->SetFirstRowShown(m_sitreps_lb->begin());
}

void SitRepPanel::SetHiddenSitRepTemplates(const std::set<std::string>& templates) {
    std::set<std::string> old_hidden_sitrep_templates = m_hidden_sitrep_templates;
    m_hidden_sitrep_templates = templates;
    if (old_hidden_sitrep_templates != m_hidden_sitrep_templates)
        Update();
}

int SitRepPanel::NumVisibleSitrepsThisTurn() const {
    std::map<int, std::list<SitRepEntry> > turns = GetSitRepsSortedByTurn(HumanClientApp::GetApp()->EmpireID(), m_hidden_sitrep_templates);
    return turns[CurrentTurn()].size();
}

