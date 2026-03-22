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
#include <numeric>


namespace {
    constexpr int sitrep_row_margin(1);
    constexpr int sitrep_edge_to_outline_spacing(2);
    constexpr int sitrep_edge_to_content_spacing(sitrep_edge_to_outline_spacing + 1 + 2);
    constexpr int sitrep_spacing(2);

#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif

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

    std::map<int, std::set<std::string>> snoozed_sitreps; // for each turn, which sitreps are hidden?
    std::set<std::string> permanently_snoozed_sitreps;    // sitreps hidden on all turns

    void SnoozeSitRepForNTurns(std::string sitrep_text, int start_turn, int num_turns) {
        for (int turn = start_turn; turn < start_turn + num_turns; turn++)
            snoozed_sitreps[turn].insert(sitrep_text);
    }

    void HandleLinkClick(const std::string& link_type, const std::string& data) {
        auto& app = GetApp();
        auto& context = app.GetContext();
        auto& universe = context.ContextUniverse();
        auto client_empire_id = app.EmpireID();
        auto& ui = app.GetUI();
        const auto data_int = [&data]() { return boost::lexical_cast<int>(data); };

        try {
            if (link_type == VarText::PLANET_ID_TAG) {
                ui.ZoomToPlanet(data_int(), context);
            } else if (link_type == VarText::SYSTEM_ID_TAG) {
                ui.ZoomToSystem(data_int(), context);
            } else if (link_type == VarText::FLEET_ID_TAG) {
                ui.ZoomToFleet(data_int(), context, client_empire_id);
            } else if (link_type == VarText::SHIP_ID_TAG) {
                ui.ZoomToShip(data_int(), context, client_empire_id);
            } else if (link_type == VarText::BUILDING_ID_TAG) {
                ui.ZoomToBuilding(data_int(), context);

            } else if (link_type == VarText::COMBAT_ID_TAG) {
                ui.ZoomToCombatLog(data_int());

            } else if (link_type == VarText::EMPIRE_ID_TAG) {
                ui.ZoomToEmpire(data_int());
            } else if (link_type == VarText::DESIGN_ID_TAG) {
                ui.ZoomToShipDesign(data_int());
            } else if (link_type == VarText::PREDEFINED_DESIGN_TAG) {
                if (const ShipDesign* design = universe.GetGenericShipDesign(data))
                    ui.ZoomToShipDesign(design->ID());

            } else if (link_type == VarText::TECH_TAG) {
                ui.ZoomToTech(data);
            } else if (link_type == VarText::POLICY_TAG) {
                ui.ZoomToPolicy(data);
            } else if (link_type == VarText::BUILDING_TYPE_TAG) {
                ui.ZoomToBuildingType(data);
            } else if (link_type == VarText::SPECIAL_TAG) {
                ui.ZoomToSpecial(data);
            } else if (link_type == VarText::SHIP_HULL_TAG) {
                ui.ZoomToShipHull(data);
            } else if (link_type == VarText::SHIP_PART_TAG) {
                ui.ZoomToShipPart(data);
            } else if (link_type == VarText::SPECIES_TAG) {
                ui.ZoomToSpecies(data);
            } else if (link_type == VarText::METER_TYPE_TAG) {
                ui.ZoomToMeterTypeArticle(data);
            } else if (link_type == TextLinker::ENCYCLOPEDIA_TAG) {
                ui.ZoomToEncyclopediaEntry(data);
            } else if (link_type == TextLinker::BROWSE_PATH_TAG) {
                app.BrowsePath(FilenameToPath(data));
            }
        } catch (const boost::bad_lexical_cast&) {
            ErrorLogger() << "SitrepPanel.cpp HandleLinkClick caught lexical cast exception for link type: " << link_type << " and data: " << data;
        }
    }

    std::vector<std::string> OrderedSitrepLabels() {
        // extract determine sitrep ordering from stringtable entry
        std::istringstream order_stream(UserString("FUNCTIONAL_SITREP_PRIORITY_ORDER"));
        std::vector<std::string> label_order;
        label_order.reserve(80); // guesstimate that is a bit more than the number of results (72) in tests at time of writing
        std::copy(std::istream_iterator<std::string>(order_stream), // split at newlines into separate strings
                  std::istream_iterator<std::string>(),
                  std::back_inserter<std::vector<std::string>>(label_order));  // TODO: reimplement using std::views::split if possible?
        return label_order;
    }


    constexpr auto to_sitrep_label = [](const SitRepEntry::FixedInfo& info) noexcept -> const std::string& { return info.m_label; };

    using SitRepsEntryT = std::decay_t<decltype(GetApp().GetEmpire(ALL_EMPIRES)->SitReps().front())>;
    constexpr auto to_fixed_idx = [](const SitRepsEntryT& entry) noexcept -> uint32_t { return entry.first.second; };

    void Uniquify(auto& vec) {
        if (vec.empty())
            return;
        std::sort(vec.begin(), vec.end());
        const auto unique_it = std::unique(vec.begin(), vec.end());
        vec.erase(unique_it, vec.end());
    }


    // all sitrep labels the empire has a sitrep for, and whether they should be looked up in the stringtable before displaying
    std::vector<std::string> EmpireSitRepLabels(const Empire& empire) {
        auto fixed_idxs = empire.SitReps() | range_transform(to_fixed_idx) | range_to_vec;
        Uniquify(fixed_idxs);

        const auto& fixed_infos = empire.SitRepFixedInfos();
        const auto in_range = [sz{fixed_infos.size()}](uint32_t idx) noexcept { return idx < sz; };
        const auto to_fixed_info = [&fixed_infos](const uint32_t idx) -> const SitRepEntry::FixedInfo& { return fixed_infos[idx]; };

        auto effective_labels_vec = fixed_idxs | range_filter(in_range) | range_transform(to_fixed_info) |
                                    range_transform(to_sitrep_label) | range_to_vec;

        Uniquify(effective_labels_vec);
        return effective_labels_vec;
    }

    constexpr auto not_null = [](const auto& p) -> bool { return !!p; };

    std::vector<std::string> AllSitRepLabels() {
        // get templates for each empire
        std::vector<std::string> label_set;
        for (const auto& empire : Empires() | range_values | range_filter(not_null)) {
            auto empire_labels = EmpireSitRepLabels(*empire);
            label_set.insert(label_set.end(), std::make_move_iterator(empire_labels.begin()),
                             std::make_move_iterator(empire_labels.end()));
        }
        Uniquify(label_set);

        std::vector<std::string> ordered_labels{OrderedSitrepLabels()};

        std::vector<std::string> retval;
        retval.reserve(ordered_labels.size() + label_set.size());

        // first add templates that are in both the ordered template strings and
        // in the set of templates from empires. don't add any twice. add in the
        // order they are in the ordered template strings.
        for (auto& label : ordered_labels) { // TODO: reimplement using range_copy_if ?
            if (range_contains(label_set, label) && !range_contains(retval, label))
                retval.push_back(std::move(label));
        }

        // next add templates from empires that were not in the ordered template strings
        for (auto& label : label_set) {
            if (!range_contains(retval, label))
                retval.push_back(std::move(label));
        }

        return retval;
    }

    std::set<std::string> HiddenSitRepLabelsFromOptions() {
        std::set<std::string> result;
        std::string saved_labels = GetOptionsDB().Get<std::string>("ui.map.sitrep.hidden.stringlist");

        // Split a space-delimited sequence of strings.
        std::istringstream ss(saved_labels);
        std::copy(
            std::istream_iterator<std::string>(ss),
            std::istream_iterator<std::string>(),
            std::inserter(result, result.begin())
        );
        return result;
    }

    void SetHiddenSitRepLabelsInOptions(const std::set<std::string>& set) {
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
        SitRepLinkText(GG::X x, GG::Y y, GG::X w, std::string str, std::shared_ptr<const GG::Font> font,
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

    const std::string GENERIC_ICON = "/icons/sitrep/generic.png";

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

            auto& app = GetApp();

            const int icon_dim = GetIconSize();
            const auto& entry_icon = m_sitrep_entry.GetIcon();
            const auto& icon_texture = entry_icon.empty() ? GENERIC_ICON : entry_icon;
            auto icon = app.GetUI().GetTexture(ClientUI::ArtDir() / icon_texture, true);
            m_icon = GG::Wnd::Create<GG::StaticGraphic>(std::move(icon), GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            AttachChild(m_icon);
            m_icon->Resize(GG::Pt(GG::X(icon_dim), GG::Y(icon_dim)));

            GG::Pt spacer = GG::Pt(GG::X(sitrep_edge_to_content_spacing), GG::Y(sitrep_edge_to_content_spacing));
            GG::X icon_left(spacer.x);
            GG::X text_left(icon_left + GG::X(icon_dim) + sitrep_spacing);
            GG::X text_width(ClientWidth() - text_left - spacer.x);

            m_link_text = GG::Wnd::Create<SitRepLinkText>(
                GG::X0, GG::Y0, text_width, m_sitrep_entry.GetText(app.GetContext()) + " ", app.GetUI().GetFont(),
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
        SitRepRow(GG::X w, GG::Y h, SitRepEntry sitrep) :
            GG::ListBox::Row(w, h),
            m_sitrep(std::move(sitrep))
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
    m_hidden_sitrep_labels(HiddenSitRepLabelsFromOptions())
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
    using EmpireSitrepsContainer = std::decay_t<decltype(GetEmpire(ALL_EMPIRES)->SitReps())>;

    /** Search forward (if \a forward is true) or backward from \a turn (but not including \a turn)
      * for the next/previous turn with any valid not-hidden sitreps. */
    int GetNextNonEmptySitrepsTurn(const EmpireSitrepsContainer& sitreps, int turn,
                                   const std::vector<SitRepEntry::FixedInfo>& fixed_infos,
                                   bool forward, const std::set<std::string>& hidden_labels)
    {
        if (sitreps.empty())
            return INVALID_GAME_TURN;
        const auto& context = GetApp().GetContext();

        // checks that fixed info idx is in range in container
        const auto fixed_info_idx_in_range = [sz{fixed_infos.size()}](const EmpireSitrepsContainer::value_type& x_fixedid_x) noexcept
        { return x_fixedid_x.first.second < sz; };

        // look up fixed info for id. assumes idx is in range.
        const auto get_info = [&fixed_infos](const EmpireSitrepsContainer::value_type& x_fixedid_x) -> const SitRepEntry::FixedInfo& {
            const uint32_t fixed_idx = x_fixedid_x.first.second;
            return fixed_infos[fixed_idx];
        };

        const auto not_hidden = [&hidden_labels, get_info](const EmpireSitrepsContainer::value_type& x_fixedid_x) {
            bool retval = !hidden_labels.contains(get_info(x_fixedid_x).m_label);
            //std::cout << "label: " << get_info(x_fixedid_x).m_label << (retval ? " SHOWN" : " HIDDEN") << std::endl;
            return retval;
        };

        // search only in range of turns before or after \a turn depending on \a forward
        const auto turn_dir_ok = [forward, turn](const EmpireSitrepsContainer::value_type& turn_x) {
            bool retval = forward ? (turn_x.first.first > turn) : (turn_x.first.first < turn);
            //std::cout << "want turn: " << turn << " have: " << turn_x.first.first << ":  " << (retval ? " OK" : " WRONG") << std::endl;
            return retval;
        };

        const bool show_invalid = GetOptionsDB().Get<bool>("ui.map.sitrep.invalid.shown");
        const auto has_valid_sitrep = [show_invalid, &context, get_info](const EmpireSitrepsContainer::value_type& x_fixedid_params_lists) {
            const auto& [turn_fixedid, params_lists] = x_fixedid_params_lists;
            //std::cout << "params lists: " << params_lists.size() << "  " << (show_invalid ? "SHOWING_INVALID" : "HIDING_INVALID") << std::endl;

            if (params_lists.empty())
                return false; // there are no parameter sets for this turn, so no possible (valid or otherwise) sitreps
            if (show_invalid)
                return true; // there is at least one parameter set for this turn, and don't care if it's valid

            const auto& fixed_info = get_info(x_fixedid_params_lists); // assume index was range-checked elsewhere

            using ParamsLists = std::decay_t<decltype(x_fixedid_params_lists.second)>;
            using ParamSetT = ParamsLists::value_type;

            // check if any of the parameter sets make a valid sitrep
            const auto makes_valid_sitrep = [rep_turn{turn_fixedid.first}, &fixed_info, context{&context}](const ParamSetT& p) {
                const auto [views, count] = p.ToViewsAndCount();
                const std::span param_views(views.begin(), count);
                bool retval = SitRepEntry::IsValidSitrep(fixed_info, param_views, context);
                //std::cout << (retval ? " VALID" : " INVALID") << std::endl;
                return retval;
            };

            return range_any_of(params_lists, makes_valid_sitrep);
        };


        auto rng = sitreps | range_filter(turn_dir_ok) | range_filter(fixed_info_idx_in_range)
                           | range_filter(not_hidden) | range_filter(has_valid_sitrep);

        auto is_empty = range_empty(rng);

        //std::cout << "rng empty? " << (is_empty ? "EMPTY" : "HAS_STUFF") << std::endl;

        if (is_empty)
            return INVALID_GAME_TURN;

        //std::cout << "next turn with valid shown rep: " << (forward ? rng.front() : rng.back()).first.first << std::endl;

        // if searching backwards, the last item in the range is the closest to \a turn
        // if searching fowards, the first item in the range is the closest to \a turn
        return (forward ? rng.front() : rng.back()).first.first;
    }
}

void SitRepPanel::CloseClicked()
{ ClosingSignal(); }

void SitRepPanel::PrevClicked() {
    const Empire* empire = GetEmpire(GetApp().EmpireID());
    if (!empire)
        return; // TODO: for all empires?
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(empire->SitReps(), m_showing_turn, empire->SitRepFixedInfos(),
                                                  false, m_hidden_sitrep_labels));
}

void SitRepPanel::NextClicked() {
    const Empire* empire = GetEmpire(GetApp().EmpireID());
    if (!empire)
        return; // TODO: for all empires?
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(empire->SitReps(), m_showing_turn, empire->SitRepFixedInfos(),
                                                  true, m_hidden_sitrep_labels));
}

void SitRepPanel::LastClicked() {
    const Empire* empire = GetEmpire(GetApp().EmpireID());
    if (!empire)
        return; // TODO: for all empires?
    ShowSitRepsForTurn(GetNextNonEmptySitrepsTurn(empire->SitReps(), GetApp().CurrentTurn() + 1,
                                                  empire->SitRepFixedInfos(), false, m_hidden_sitrep_labels));
}

void SitRepPanel::FilterClicked() {
    SectionedScopedTimer filter_click_timer{"SitRepPanel::FilterClicked"};
    std::map<int, std::string> menu_index_labels; // TODO: these can probably be vectors as indices are sequential ints
    std::map<int, bool> menu_index_checked;
    int index = 1;
    bool all_checked = true;

    filter_click_timer.EnterSection("get labels");
    auto all_labels = AllSitRepLabels();

    filter_click_timer.EnterSection("create popupmenu");
    auto popup = GG::Wnd::Create<CUIPopupMenu>(m_filter_button->Left(), m_filter_button->Bottom());

    filter_click_timer.EnterSection("add labels");
    for (const std::string& label : all_labels) {
        menu_index_labels[index] = label;
        bool check = true;
        if (m_hidden_sitrep_labels.contains(label)) {
            check = false;
            all_checked = false;
        }
        menu_index_checked[index] = check;

        auto select_label_action = [index, this, &menu_index_labels, &menu_index_checked]() {
            // select / deselect the chosen template
            const std::string& selected_label = menu_index_labels[index];
            if (menu_index_checked[index]) {
                // disable showing this template string
                m_hidden_sitrep_labels.insert(selected_label);
            } else {
                // re-enabled showing this template string
                m_hidden_sitrep_labels.erase(selected_label);
            }
        };

        popup->AddMenuItem(UserString(label), false, check, select_label_action);
        ++index;
    }

    auto all_labels_action = [all_checked, &all_labels, this]() {
        // select / deselect all laels
        if (all_checked) {
            // deselect all
            m_hidden_sitrep_labels.insert(std::make_move_iterator(all_labels.begin()),
                                          std::make_move_iterator(all_labels.end()));
        } else {
            // select all
            m_hidden_sitrep_labels.clear();
        }
    };
    popup->AddMenuItem(all_checked ? UserString("NONE") : UserString("ALL"),
                       false, false, all_labels_action);

    filter_click_timer.EnterSection("run menu");
    if (!popup->Run())
        return;

    filter_click_timer.EnterSection("cleanup");
    SetHiddenSitRepLabelsInOptions(m_hidden_sitrep_labels);

    Update();
}

void SitRepPanel::IgnoreSitRep(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> mod) {
    SitRepRow* sitrep_row = dynamic_cast<SitRepRow*>(it->get());
    if (!sitrep_row)
        return;

    const SitRepEntry& sitrep = sitrep_row->GetSitRepEntry();
    if (sitrep.GetTurn() <= 0)
        return;

    snoozed_sitreps[sitrep.GetTurn()].insert(sitrep.GetText(GetApp().GetContext()));

    Update();
}

namespace {
    const std::string entry_margin("  ");

    constexpr auto snooze_clear_action = []() { snoozed_sitreps.clear(); };
    constexpr auto snooze_clear_indefinite_action = []() {
        snoozed_sitreps.clear();
        permanently_snoozed_sitreps.clear();
    };

    constexpr auto help_action = []() { GetApp().GetUI().ZoomToEncyclopediaEntry("SITREP_IGNORE_BLOCK_TITLE"); };
}

void SitRepPanel::DismissalMenu(GG::ListBox::iterator it, GG::Pt pt, GG::Flags<GG::ModKey> mod) {
    GG::MenuItem menu_contents, submenu_ignore, submenu_block;

    std::string sitrep_text, sitrep_template;
    int start_turn = 0;

    const SitRepRow* sitrep_row = nullptr;
    if (it != m_sitreps_lb->end()) 
        sitrep_row = dynamic_cast<const SitRepRow*>(it->get());
    submenu_ignore.label = entry_margin + UserString("SITREP_IGNORE_MENU");

    if (sitrep_row) {
        const SitRepEntry& sitrep_entry = sitrep_row->GetSitRepEntry();
        sitrep_text = sitrep_entry.GetText(GetApp().GetContext());
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
            submenu_ignore.next_level.emplace_back(GG::MenuItem::menu_separator);
        }
    }

    submenu_ignore.next_level.emplace_back(entry_margin + UserString("SITREP_SNOOZE_CLEAR_ALL"),
                                           false, false, snooze_clear_action);
    submenu_ignore.next_level.emplace_back(entry_margin + UserString("SITREP_SNOOZE_CLEAR_INDEFINITE"),
                                           false, false, snooze_clear_indefinite_action);
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    popup->AddMenuItem(std::move(submenu_ignore));

    submenu_block.label = entry_margin + UserString("SITREP_BLOCK_MENU");
    if (sitrep_row) {
        const SitRepEntry& sitrep_entry = sitrep_row->GetSitRepEntry();
        const auto& sitrep_label = sitrep_entry.GetLabel();
        const auto& displayed_label = sitrep_entry.GetStringtableLookupFlag() ? UserString(sitrep_label) : sitrep_label;

        if (!displayed_label.empty() && !sitrep_label.empty()) {
            auto hide_template_action = [&sitrep_label, this]() {
                m_hidden_sitrep_labels.insert(sitrep_label);
                SetHiddenSitRepLabelsInOptions(m_hidden_sitrep_labels);
                Update();
            };
            submenu_block.next_level.emplace_back(
                entry_margin + str(FlexibleFormat(UserString("SITREP_HIDE_TEMPLATE")) % displayed_label),
                false, false, hide_template_action);
        }
    }
    if (!m_hidden_sitrep_labels.empty()) {
        if (sitrep_row)
            submenu_block.next_level.emplace_back(GG::MenuItem::menu_separator);
        auto showall_action = [this]() {
            m_hidden_sitrep_labels.clear();
            SetHiddenSitRepLabelsInOptions(m_hidden_sitrep_labels);
            Update();
        };
        submenu_block.next_level.emplace_back(
            entry_margin + UserString("SITREP_SHOWALL_TEMPLATES"),
            false, false, showall_action);
    }
    popup->AddMenuItem(std::move(submenu_block));

    if (!sitrep_text.empty()) {
        auto copy_action = [&sitrep_text]() { GetApp().SetClipboardText(GG::Font::StripTags(sitrep_text)); };
        popup->AddMenuItem(entry_margin + UserString("HOTKEY_COPY"), false, false, copy_action);
    }

    popup->AddMenuItem(entry_margin + UserString("POPUP_MENU_PEDIA_PREFIX") + UserString("SITREP_IGNORE_BLOCK_TITLE"),
                       false, false, help_action);

    if (!popup->Run())
        return;
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

    const auto& app = GetApp();
    const auto& context = app.GetContext();

    const auto empire = context.GetEmpire(app.EmpireID());
    if (!empire)
        return;
    const auto& sitreps = empire->SitReps();
    const auto& fixed_infos = empire->SitRepFixedInfos();

    m_showing_turn = GetNextNonEmptySitrepsTurn(sitreps, m_showing_turn - 1, fixed_infos, true, m_hidden_sitrep_labels); // TODO: pass in app?
    if (m_showing_turn < 1)
        this->SetName(UserString("SITREP_PANEL_TITLE"));
    else
        this->SetName(boost::io::str(FlexibleFormat(UserString("SITREP_PANEL_TITLE_TURN")) % m_showing_turn));

    //std::cout << "Update shown turn: " << m_showing_turn << std::endl;

    // picks only m_showing_turn
    const auto is_showing_turn = [turn{m_showing_turn}](const EmpireSitrepsContainer::value_type& turn_xx) noexcept
    { return turn_xx.first.first == turn; };

    // checks that fixed info idx is in range in container
    const auto fixed_info_id_in_range = [sz{fixed_infos.size()}](const EmpireSitrepsContainer::value_type& x_fixedid_x) noexcept
    { return x_fixedid_x.first.second < sz; };

    // look up fixed info for id. assumes idx is in range.
    const auto get_info = [&fixed_infos](const EmpireSitrepsContainer::value_type& x_fixedid_x) -> const SitRepEntry::FixedInfo& {
        const uint32_t fixed_idx = x_fixedid_x.first.second;
        return fixed_infos[fixed_idx];
    };

    const auto not_hidden = [this, get_info](const EmpireSitrepsContainer::value_type& x_fixedid_x) {
        const auto& fixed_info = get_info(x_fixedid_x);
        //std::cout << "display string " << to_sitrep_label(fixed_info) << ": "
        //          << (m_hidden_sitrep_labels.contains(to_sitrep_label(fixed_info)) ? "hidden" : "not hidden") << std::endl;

        return !m_hidden_sitrep_labels.contains(to_sitrep_label(fixed_info));
    };


    //std::cout << "hidden sitrep labels:" << [this]() {
    //    std::string retval;
    //    for (const auto& hst : m_hidden_sitrep_labels)
    //        retval.append(" ").append(hst);
    //    return retval;
    //}() << std::endl;

    // range over sitrep templates for current turn, with valid fixed info ID, not hidden
    auto ok_turn_not_hidden_label_rng = sitreps | range_filter(is_showing_turn) |
                                        range_filter(fixed_info_id_in_range) | range_filter(not_hidden);

    // extract label from and pointer to FixedInfo and parameters sets list
    using ChunkedViewsVec = EmpireSitrepsContainer::value_type::second_type;
    using ChunkedViews = ChunkedViewsVec::value_type;
    static_assert(std::is_same_v<ChunkedViews, Empire::ChunkedStringAndViews>);
    const auto to_fixed_info_and_param_ptrs = [get_info](const EmpireSitrepsContainer::value_type& x_fixedid_params)
        -> std::pair<std::pair<std::string_view, const SitRepEntry::FixedInfo*>, const ChunkedViewsVec*>
    {
        const auto& fixed_info = get_info(x_fixedid_params); // assuming already range checked
        return std::pair(std::pair(std::string_view(to_sitrep_label(fixed_info)), std::addressof(fixed_info)),
                         std::addressof(x_fixedid_params.second));
    };

    // collect and resort ok labels into display order
    auto ok_labels_and_params_vec = ok_turn_not_hidden_label_rng | range_transform(to_fixed_info_and_param_ptrs) | range_to_vec;

    //DebugLogger() << "OK sitrep labels filtered on turn: " << m_showing_turn;
    //for (const auto& [fixed_info, params_lists] : ok_label_and_params_vec)
    //    DebugLogger() << fixed_info.first << "  param lists: " << params_lists->size();

    if (range_empty(ok_labels_and_params_vec))
        return;

    // arrange templates so ok templates are first and the rest are after but in the same order.
    const auto ordered_labels = OrderedSitrepLabels();
    const auto in_ordered = [&](const auto& t) { return range_contains(ordered_labels, t.first.first); };
    const auto part_it = std::stable_partition(ok_labels_and_params_vec.begin(), ok_labels_and_params_vec.end(), in_ordered);

    if (part_it != ok_labels_and_params_vec.begin()) {
        auto target_pos_it = ok_labels_and_params_vec.begin();

        for (const auto& sts : ordered_labels) {
            const auto is_sts = [stsv{std::string_view{sts}}](const auto& tpv) noexcept { return tpv.first.first == stsv; };
            auto sts_in_ok_it = std::find_if(ok_labels_and_params_vec.begin(), part_it, is_sts); // only reorder within range of ordered template strings
            if (sts_in_ok_it == part_it)
                continue; // don't have this template in the selected templates & params

            std::swap(*target_pos_it, *sts_in_ok_it); // swap into position
            ++target_pos_it; // move to next position
            if (target_pos_it == part_it)
                break; // no more ordered template strings
        }
    }

    //DebugLogger() << "OK sitrep templates ordered:";
    //for (const auto& [template_fixed_info, params_lists] : ok_templates_and_params_vec)
    //    DebugLogger() << template_fixed_info.first << "  param lists: " << params_lists->size();

    const GG::X width = m_sitreps_lb->ClientWidth();
    const GG::Y height{ClientUI::Pts()*2};
    const bool show_invalid = GetOptionsDB().Get<bool>("ui.map.sitrep.invalid.shown");

    // create sitreps and UI rows for them, if valid or showing invalid
    for (const auto& [template_fixed_info, params_lists] : ok_labels_and_params_vec) {
        if (!template_fixed_info.second || !params_lists)
            continue;
        const auto& fixed_info = *template_fixed_info.second;

        for (const auto& params : *params_lists) {
            SitRepEntry sitrep(fixed_info, SitRepEntry::UniqueInfo(params.ToStringVector(), m_showing_turn));
            if (show_invalid || sitrep.GetValidity(context))
                m_sitreps_lb->Insert(GG::Wnd::Create<SitRepRow>(width, height, std::move(sitrep)));
        }
    }

    if (m_sitreps_lb->NumRows() > first_visible_row)
        m_sitreps_lb->SetFirstRowShown(std::next(m_sitreps_lb->begin(), first_visible_row));
    else if (!m_sitreps_lb->Empty())
        m_sitreps_lb->BringRowIntoView(--m_sitreps_lb->end());

    // if at first turn with visible sitreps, disable back button
    const int prev_turn_with_sitrep = GetNextNonEmptySitrepsTurn(sitreps, m_showing_turn, fixed_infos, false, m_hidden_sitrep_labels);

    const bool disable_prev_turn = prev_turn_with_sitrep == INVALID_GAME_TURN;
    m_prev_turn_button->Disable(disable_prev_turn);

    // if at last turn with visible sitreps, disable forward button
    const int next_turn_with_sitrep = GetNextNonEmptySitrepsTurn(sitreps, m_showing_turn, fixed_infos, true, m_hidden_sitrep_labels);

    const bool disable_next_turn = next_turn_with_sitrep == INVALID_GAME_TURN;
    m_next_turn_button->Disable(disable_next_turn);
    m_last_turn_button->Disable(disable_next_turn);
}

void SitRepPanel::ShowSitRepsForTurn(int turn) {
     const bool is_different_turn(m_showing_turn != turn);
     m_showing_turn = turn;
     Update();
     if (is_different_turn)
         m_sitreps_lb->SetFirstRowShown(m_sitreps_lb->begin());
}

void SitRepPanel::SetHiddenSitRepLabels(const std::set<std::string>& labels) {
    const auto old_hidden_sitrep_labels{m_hidden_sitrep_labels};
    m_hidden_sitrep_labels = labels;
    if (old_hidden_sitrep_labels != m_hidden_sitrep_labels)
        Update();
}

std::size_t SitRepPanel::NumVisibleSitrepsThisTurn() const {
    const auto& app = GetApp();
    const auto& context = app.GetContext();
    const auto empire = context.GetEmpire(app.EmpireID());
    if (!empire)
        return 0; // TODO: for all empires?
    const auto current_turn = app.CurrentTurn();

    const auto& sitreps = empire->SitReps();
    const auto& fixed_infos = empire->SitRepFixedInfos();

    // checks turn of sitrep is current turn
    const auto is_this_turn = [current_turn](const EmpireSitrepsContainer::value_type& turn_x_x)
    { return turn_x_x.first.first == current_turn; };

    // checks that fixed info idx is in range in container, gets it, and checks that it is not hidden
    const auto in_range_not_hidden = [this, &fixed_infos](const EmpireSitrepsContainer::value_type& x_fixedid_x) {
        const auto fixed_idx = x_fixedid_x.first.second;
        if (fixed_idx >= fixed_infos.size())
            return false;
        const auto& fixed_info = fixed_infos[fixed_idx];
        return !m_hidden_sitrep_labels.contains(fixed_info.m_label);
    };

    const auto to_this_turn_valid_sitrep_count =
        [current_turn, &context, &fixed_infos]
        (const EmpireSitrepsContainer::value_type& turn_fixedid_params_lists) -> std::size_t
    {
        const auto& [turn_idx, params_lists] = turn_fixedid_params_lists;
        const auto& [turn, fixed_idx] = turn_idx;
        if (fixed_idx >= fixed_infos.size())
            return 0;
        const auto& fixed_info = fixed_infos[fixed_idx];

        const auto makes_valid_sitrep = [current_turn, &fixed_info, context{&context}](const Empire::ChunkedStringAndViews& p) -> bool {
            const auto [views, count] = p.ToViewsAndCount();
            const std::span param_views(views.begin(), count);
            return SitRepEntry::IsValidSitrep(fixed_info, param_views, context);
        };

        return range_count_if(params_lists, makes_valid_sitrep);
    };

    auto valid_not_hidden_sitrep_counts_rng = sitreps | range_filter(is_this_turn) | range_filter(in_range_not_hidden) |
                                              range_transform(to_this_turn_valid_sitrep_count);

    return std::accumulate(valid_not_hidden_sitrep_counts_rng.begin(), valid_not_hidden_sitrep_counts_rng.end(), std::size_t{0});
}

