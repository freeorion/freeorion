#include "GalaxySetupWnd.h"

#include "CUIControls.h"
#include "CUISpin.h"
#include "Sound.h"
#include "../universe/Universe.h"
#include "../client/human/GGHumanClientApp.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/GameRules.h"
#include "../util/OptionsDB.h"
#include "../util/AppInterface.h"

#include <boost/filesystem/fstream.hpp>

#include <GG/StaticGraphic.h>
#include <GG/Layout.h>
#include <GG/TabWnd.h>


namespace {
    constexpr GG::X CONTROL_MARGIN{5};
    constexpr GG::Y CONTROL_VMARGIN{5};
    constexpr GG::Y CONTROL_HEIGHT{30};
    constexpr GG::Y PANEL_CONTROL_SPACING{33};
    constexpr GG::X INDENTATION{20};
    constexpr GG::X SPIN_WIDTH{128};
    constexpr GG::Y GAL_SETUP_PANEL_HT{Value(PANEL_CONTROL_SPACING) * 10};
    GG::X GalSetupWndWidth()
    { return GG::X{445 + FontBasedUpscale(300)}; }
    GG::Y GalSetupWndHeight()
    { return GG::Y{FontBasedUpscale(29) + (PANEL_CONTROL_SPACING * 6) + GAL_SETUP_PANEL_HT}; }
    constexpr GG::Pt PREVIEW_SZ{GG::X{400}, GG::Y{222}};

    class RowContentsWnd : public GG::Control {
    public:
        RowContentsWnd(GG::X w, GG::Y h, std::shared_ptr<GG::Wnd> contents, int indentation_level) :
            Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
            m_contents(std::move(contents))
        {
            if (!m_contents)
                return;
            m_contents->MoveTo(GG::Pt(GG::X(indentation_level * INDENTATION), GG::Y0));
        }

        void CompleteConstruction() override {
            GG::Control::CompleteConstruction();

            if (!m_contents)
                return;
            AttachChild(m_contents);
            DoLayout();
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            const auto old_size = Size();
            GG::Control::SizeMove(ul, lr);
            if (old_size != Size())
                DoLayout();
        }

        void DoLayout() {
            if (m_contents) {
                //std::cout << "RowContentsWnd::DoLayout()" << std::endl;
                m_contents->SizeMove(GG::Pt(), Size());
            }
        }

        void Render() override
        { /*GG::FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_DARK_RED, GG::CLR_PINK, 1);*/ }
    private:
        std::shared_ptr<Wnd> m_contents;
    };

    class RuleListRow : public GG::ListBox::Row {
    public:
        RuleListRow(GG::X w, GG::Y h, std::shared_ptr<RowContentsWnd> contents) :
            GG::ListBox::Row(w, h),
            m_contents(std::move(contents))
        {}

        RuleListRow(GG::X w, GG::Y h, std::shared_ptr<Wnd> contents, int indentation = 0) :
            GG::ListBox::Row(w, h)
        {
            if (contents)
                m_contents = GG::Wnd::Create<RowContentsWnd>(w, h, std::move(contents), indentation);
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();
            SetChildClippingMode(ChildClippingMode::ClipToClient);
            if (m_contents)
                push_back(m_contents);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            //std::cout << "RuleListRow::SizeMove(" << ul << ", " << lr << ")" << std::endl;
            const auto old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            if (!empty() && old_size != Size() && m_contents)
                m_contents->Resize(Size());
        }

        void Render() override
        { /*GG::FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_DARK_BLUE, GG::CLR_YELLOW, 1);*/ }
    private:
        std::shared_ptr<RowContentsWnd> m_contents;
    };

    class GameRulesList : public CUIListBox {
    public:
        GameRulesList() :
            CUIListBox()
        {
            InitRowSizes();

            SetColor(GG::CLR_ZERO);
            SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
            SetVScrollWheelIncrement(ClientUI::Pts() * 10);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            const auto old_size = Size();
            CUIListBox::SizeMove(ul, lr);
            if (old_size != Size()) {
                const GG::X row_width = ListRowWidth();
                for (auto& row : *this)
                    row->Resize(GG::Pt(row_width, row->Height()));
            }
        }

    private:
        GG::X ListRowWidth() const
        { return Width() - RightMargin() - 5; }

        void InitRowSizes() {
            // preinitialize listbox/row column widths, because what
            // ListBox::Insert does on default is not suitable for this case
            SetNumCols(1);
            SetColWidth(0, GG::X0);
            LockColWidths();
        }
    };


    // persistant between-executions game setup settings
    void AddOptions(OptionsDB& db) {
        db.Add("setup.empire.name",             UserStringNop("OPTIONS_DB_GAMESETUP_EMPIRE_NAME"),              std::string(""),            Validator<std::string>());
        db.Add("setup.player.name",             UserStringNop("OPTIONS_DB_GAMESETUP_PLAYER_NAME"),              std::string(""),            Validator<std::string>());
        db.Add("setup.empire.color.index",      UserStringNop("OPTIONS_DB_GAMESETUP_EMPIRE_COLOR"),             9,                          RangedValidator<int>(0, 100));
        db.Add("setup.initial.species",         UserStringNop("OPTIONS_DB_GAMESETUP_STARTING_SPECIES_NAME"),    std::string("SP_HUMAN"),    Validator<std::string>());
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    constexpr std::string_view formatting_chars = "<>;:,.@#$%&*(){}'\"/?\\`[]|\a\b\f\n\r\t\b";
}

////////////////////////////////////////////////
// GameRulesPanel
////////////////////////////////////////////////
GameRulesPanel::GameRulesPanel(GG::X w, GG::Y h) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS)
{}

void GameRulesPanel::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    DebugLogger() << "GalaxySetupPanel::CompleteConstruction";

    m_tabs = GG::Wnd::Create<GG::TabWnd>(GG::X0, GG::Y0, SPIN_WIDTH, GG::Y1,
                                         ClientUI::GetFont(), ClientUI::WndColor(), ClientUI::TextColor());
    AttachChild(m_tabs);

    // create storage and default general rule page
    std::map<std::string, GG::ListBox*> indexed_pages;
    indexed_pages[""] = CreatePage(UserString("GENERAL"));

    // for all rules, add to page
    for (const auto* rule : GetGameRules().GetSortedByCategoryAndRank()) {
        // get or create page for rule
        auto itr = indexed_pages.find(rule->category);
        if (itr == indexed_pages.end()) {
            indexed_pages[rule->category] = CreatePage(UserString(rule->category));
            itr = indexed_pages.find(rule->category);
        }
        if (itr == indexed_pages.end()) {
            ErrorLogger() << "Unable to create and insert and then find new rule page";
            continue;
        }
        auto current_page = itr->second;

        // add rule to page
        switch (rule->type) {
        case GameRule::Type::TOGGLE:
            BoolRuleWidget(current_page, 0, rule->name);
            break;
        case GameRule::Type::INT:
            IntRuleWidget(current_page, 0, rule->name);
            break;
        case GameRule::Type::DOUBLE:
            DoubleRuleWidget(current_page, 0, rule->name);
            break;
        case GameRule::Type::STRING:
            StringRuleWidget(current_page, 0, rule->name);
            break;
        default:
            break;
        }
    }

    DoLayout();
    m_tabs->SetCurrentWnd(0);
}

std::map<std::string, std::string> GameRulesPanel::GetRulesAsStrings() const {
    std::map<std::string, std::string> retval;
    for (const auto& entry : m_rules)
        retval.emplace(entry);
    return retval;
}

void GameRulesPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Control::SizeMove(ul, lr);
    DoLayout();
}

void GameRulesPanel::Disable(bool b) {
    for (auto& child : Children())
        std::static_pointer_cast<GG::Control>(child)->Disable(b);
}

void GameRulesPanel::DoLayout() {
    GG::Pt MARGINS(CONTROL_MARGIN, CONTROL_VMARGIN);
    m_tabs->SizeMove(MARGINS, ClientSize() - MARGINS);
}

void GameRulesPanel::Render() {
    GG::FlatRectangle(m_tabs->CurrentWnd()->UpperLeft() - GG::Pt(GG::X(2), GG::Y0),
                      m_tabs->CurrentWnd()->LowerRight() + GG::Pt(GG::X(2), GG::Y(2)),
                      GG::CLR_BLACK, ClientUI::WndInnerBorderColor(), 1);
}

void GameRulesPanel::SettingChanged() {
    Sound::TempUISoundDisabler sound_disabler;
    SettingsChangedSignal();
}

GG::ListBox* GameRulesPanel::CreatePage(const std::string& name) {
    auto page = GG::Wnd::Create<GameRulesList>();
    if (!page) {
        ErrorLogger() << "GameRulesPanel::CreatePage(" << name << ") failed to create page!";
        return nullptr;
    }
    auto* raw_ptr{page.get()};

    m_tabs->AddWnd(std::move(page), name);

    return raw_ptr;
}

void GameRulesPanel::CreateSectionHeader(GG::ListBox* page, int indentation_level,
                                         const std::string& name, const std::string& tooltip)
{
    assert(0 <= indentation_level);
    auto heading_text = GG::Wnd::Create<CUILabel>(name, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
    heading_text->SetFont(ClientUI::GetFont(ClientUI::Pts() * 4 / 3));

    auto heading_min_y_sz{heading_text->MinUsableSize().y};
    auto row = GG::Wnd::Create<RuleListRow>(Width(),
                                            heading_min_y_sz + CONTROL_VMARGIN + 6,
                                            std::move(heading_text),
                                            indentation_level);

    if (!tooltip.empty()) {
        row->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
        row->SetBrowseText(tooltip);
    }

    page->Insert(std::move(row));
}

GG::StateButton* GameRulesPanel::BoolRuleWidget(GG::ListBox* page, int indentation_level,
                                                const std::string& rule_name)
{
    auto button = GG::Wnd::Create<CUIStateButton>(UserString(rule_name), GG::FORMAT_LEFT,
                                                  std::make_shared<CUICheckBoxRepresenter>());
    auto row = GG::Wnd::Create<RuleListRow>(Width(), button->MinUsableSize().y + CONTROL_VMARGIN + 6,
                                            button, indentation_level);

    button->SetCheck(GetGameRules().Get<bool>(rule_name));
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    button->SetBrowseText(UserString(GetGameRules().GetDescription(rule_name)));
    button->CheckedSignal.connect(boost::bind(&GameRulesPanel::BoolRuleChanged, this, button.get(),
                                              rule_name));

    page->Insert(std::move(row));
    return button.get();
}

GG::Spin<int>* GameRulesPanel::IntRuleWidget(GG::ListBox* page, int indentation_level,
                                             const std::string& rule_name)
{
    auto text_control = GG::Wnd::Create<CUILabel>(UserString(rule_name),
                                                  GG::FORMAT_LEFT | GG::FORMAT_NOWRAP,
                                                  GG::INTERACTIVE);

    auto validator = GetGameRules().GetValidator(rule_name);

    std::shared_ptr<GG::Spin<int>> spin;
    if (auto ranged_validator = dynamic_cast<const RangedValidator<int>*>(validator)) {
        const int value = GetGameRules().Get<int>(rule_name);
        spin = GG::Wnd::Create<CUISpin<int>>(value, 1, ranged_validator->m_min, ranged_validator->m_max, true);

    } else if (auto step_validator = dynamic_cast<const StepValidator<int>*>(validator)) {
        const int value = GetGameRules().Get<int>(rule_name);
        spin = GG::Wnd::Create<CUISpin<int>>(value, step_validator->m_step_size, -1000000, 1000000, true);

    } else if (auto ranged_step_validator = dynamic_cast<const RangedStepValidator<int>*>(validator)) {
        const int value = GetGameRules().Get<int>(rule_name);
        spin = GG::Wnd::Create<CUISpin<int>>(value, ranged_step_validator->m_step_size,
                                             ranged_step_validator->m_min, ranged_step_validator->m_max, true);

    } else if (auto vis_validator = dynamic_cast<const RangedValidator<Visibility>*>(validator)) {
        const int value = static_cast<int>(GetGameRules().Get<Visibility>(rule_name));
        spin = GG::Wnd::Create<CUISpin<int>>(value, 1, static_cast<int>(vis_validator->m_min),
                                             static_cast<int>(vis_validator->m_max), true);

    } else { //if (auto int_validator = dynamic_cast<const Validator<int>*>(validator)) 
        const int value = GetGameRules().Get<int>(rule_name);
        spin = GG::Wnd::Create<CUISpin<int>>(value, 1, -10000, 10000, true);
    }


    if (!spin) {
        ErrorLogger() << "Unable to create IntRuleWidget spin";
        return nullptr;
    }

    spin->Resize(GG::Pt(SPIN_WIDTH, spin->MinUsableSize().y));
    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, Width(),
                                              spin->MinUsableSize().y, 1, 2, 0, 5);
    layout->Add(spin, 0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(text_control, 0, 1, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    layout->SetChildClippingMode(ChildClippingMode::ClipToClient);

    page->Insert(GG::Wnd::Create<RuleListRow>(Width(), spin->MinUsableSize().y + CONTROL_VMARGIN + 6,
                                              std::move(layout), indentation_level));

    spin->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    spin->SetBrowseText(UserString(GetGameRules().GetDescription(rule_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    text_control->SetBrowseText(UserString(GetGameRules().GetDescription(rule_name)));

    spin->ValueChangedSignal.connect(boost::bind(&GameRulesPanel::IntRuleChanged,
                                                 this, spin.get(), rule_name));
    return spin.get();
}

GG::Spin<double>* GameRulesPanel::DoubleRuleWidget(GG::ListBox* page, int indentation_level,
                                                   const std::string& rule_name)
{
    auto text_control = GG::Wnd::Create<CUILabel>(UserString(rule_name), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);

    const ValidatorBase* validator = GetGameRules().GetValidator(rule_name);
    const double value = GetGameRules().Get<double>(rule_name);

    std::shared_ptr<GG::Spin<double>> spin;
    if (auto ranged_validator = dynamic_cast<const RangedValidator<double>*>(validator))
        spin = GG::Wnd::Create<CUISpin<double>>(value, 0.1, ranged_validator->m_min, ranged_validator->m_max, true);

    else if (auto step_validator = dynamic_cast<const StepValidator<double>*>(validator))
        spin = GG::Wnd::Create<CUISpin<double>>(value, step_validator->m_step_size, -1000000, 1000000, true);

    else if (auto ranged_step_validator = dynamic_cast<const RangedStepValidator<double>*>(validator))
        spin = GG::Wnd::Create<CUISpin<double>>(value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);

    else //if (auto int_validator = dynamic_cast<const Validator<double>*>(validator))
        spin = GG::Wnd::Create<CUISpin<double>>(value, 0.1, -1000000, 1000000, true);

    if (!spin) {
        ErrorLogger() << "Unable to create DoubleRuleWidget spin";
        return nullptr;
    }

    spin->Resize(GG::Pt(SPIN_WIDTH, spin->MinUsableSize().y));
    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, Width(), spin->MinUsableSize().y, 1, 2, 0, 5);
    layout->Add(spin, 0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(text_control, 0, 1, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    layout->SetChildClippingMode(ChildClippingMode::ClipToClient);

    page->Insert(GG::Wnd::Create<RuleListRow>(Width(), spin->MinUsableSize().y + CONTROL_VMARGIN + 6,
                                              layout, indentation_level));

    spin->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    spin->SetBrowseText(UserString(GetGameRules().GetDescription(rule_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    text_control->SetBrowseText(UserString(GetGameRules().GetDescription(rule_name)));

    spin->ValueChangedSignal.connect(boost::bind(&GameRulesPanel::DoubleRuleChanged,
                                                 this, spin.get(), rule_name));

    return spin.get();
}

namespace {
    // row type used in the SpeciesSelector
    struct UserStringRow : public GG::ListBox::Row {
        UserStringRow(const std::string& key, GG::X w, GG::Y h) :
            GG::ListBox::Row(w, h)
        {
            SetMargin(0);
            SetRowAlignment(GG::ALIGN_VCENTER);
            GG::Wnd::SetName(key);
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();
            auto species_label = GG::Wnd::Create<CUILabel>(UserString(Name()),
                                                           GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            push_back(std::move(species_label));
            SetChildClippingMode(ChildClippingMode::ClipToClient);
        }
    };
}

GG::DropDownList* GameRulesPanel::StringRuleWidget(GG::ListBox* page, int indentation_level,
                                                   const std::string& rule_name)
{
    auto text_control = GG::Wnd::Create<CUILabel>(UserString(rule_name), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);

    auto validator = GetGameRules().GetValidator(rule_name);
    std::string value = GetGameRules().Get<std::string>(rule_name);

    auto drop = GG::Wnd::Create<CUIDropDownList>(5);
    drop->Resize(GG::Pt(SPIN_WIDTH, drop->MinUsableSize().y));

    if (auto desc_val = dynamic_cast<const DiscreteValidator<std::string>*>(validator)) {
        // add rows for all allowed options
        for (auto& possability : desc_val->m_values)
            drop->Insert(GG::Wnd::Create<UserStringRow>(possability,
                                                        drop->Width(), drop->Height() - 4));
    }
    // select a row by default, preferably based on set rule value
    if (!drop->Empty()) {
        drop->Select(drop->begin()); // default, hopefully fixed below
        for (auto row_it = drop->begin(); row_it != drop->end(); ++row_it) {
            if ((*row_it)->Name() == value) {
                drop->Select(row_it);
                break;
            }
        }
    }


    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, Width(), drop->MinUsableSize().y, 1, 2, 1, 5);
    layout->Add(drop, 0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(text_control, 0, 1, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    layout->SetChildClippingMode(ChildClippingMode::ClipToClient);

    page->Insert(GG::Wnd::Create<RuleListRow>(Width(), drop->MinUsableSize().y + CONTROL_VMARGIN + 6,
                                              layout, indentation_level));

    drop->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    drop->SetBrowseText(UserString(GetGameRules().GetDescription(rule_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    text_control->SetBrowseText(UserString(GetGameRules().GetDescription(rule_name)));

    drop->SelChangedSignal.connect(boost::bind(&GameRulesPanel::StringRuleChanged,
                                               this, drop.get(), rule_name));

    return drop.get();
}

void GameRulesPanel::BoolRuleChanged(const GG::StateButton* button, const std::string& rule_name) {
    auto val = GetGameRules().GetValidator(rule_name);
    if (!val || !button)
        return;
    m_rules[rule_name] = val->String(button->Checked());

    DebugLogger() << "Set Rules:";
    for (auto& [map_rule_name, map_rule_value] : m_rules)
        DebugLogger() << "  " << map_rule_name << " : " << map_rule_value;

    SettingChanged();
}

void GameRulesPanel::IntRuleChanged(const GG::Spin<int>* spin, const std::string& rule_name) {
    auto val = GetGameRules().GetValidator(rule_name);
    if (!val || !spin)
        return;
    m_rules[rule_name] = val->String(spin->Value());

    DebugLogger() << "Set Rules:";
    for (auto& [map_rule_name, map_rule_value] : m_rules)
        DebugLogger() << "  " << map_rule_name << " : " << map_rule_value;

    SettingChanged();
}

void GameRulesPanel::DoubleRuleChanged(const GG::Spin<double>* spin,
                                       const std::string& rule_name)
{
    auto val = GetGameRules().GetValidator(rule_name);
    if (!val || !spin)
        return;
    m_rules[rule_name] = val->String(spin->Value());

    DebugLogger() << "Set Rules:";
    for (auto& [map_rule_name, map_rule_value] : m_rules)
        DebugLogger() << "  " << map_rule_name << " : " << map_rule_value;

    SettingChanged();
}

void GameRulesPanel::StringRuleChanged(const GG::DropDownList* drop,
                                       const std::string& rule_name)
{
    auto val = GetGameRules().GetValidator(rule_name);
    if (!val || !drop)
        return;

    auto it = drop->CurrentItem();
    const auto row = *it;
    if (!row) {
        ErrorLogger() << "GameRulesPanel::StringRuleChanged couldn't get current item due to invalid Row pointer";
        return;
    }
    m_rules[rule_name] = row->Name();

    DebugLogger() << "Set Rules:";
    for (auto& [map_rule_name, map_rule_value] : m_rules)
        DebugLogger() << "  " << map_rule_name << " : " << map_rule_value;

    SettingChanged();
}

////////////////////////////////////////////////
// GalaxySetupPanel
////////////////////////////////////////////////
const GG::X GalaxySetupPanel::DefaultWidth()
{ return GG::X(FontBasedUpscale(305)); }

GalaxySetupPanel::GalaxySetupPanel(GG::X w, GG::Y h) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS)
{}

void GalaxySetupPanel::CompleteConstruction() {
    GG::Control::CompleteConstruction();
    DebugLogger() << "GalaxySetupPanel::CompleteConstruction";
    Sound::TempUISoundDisabler sound_disabler;

    // seed
    m_seed_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_SEED"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_seed_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    TraceLogger() << "GalaxySetupPanel::CompleteConstruction attempting to access common galaxy settings from OptionsDB";
    m_seed_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.seed")));
    m_seed = GetOptionsDB().Get<std::string>("setup.seed");
    TraceLogger() << "GalaxySetupPanel::CompleteConstruction got seed and seed label";
    if (m_seed == "RANDOM" || m_seed.empty()) {
        m_seed = "RANDOM";
        m_seed_edit = GG::Wnd::Create<CUIEdit>(UserString("GSETUP_RANDOM"));
    } else {
        m_seed_edit = GG::Wnd::Create<CUIEdit>(m_seed);
    }
    m_seed_edit->DisallowChars(formatting_chars);

    boost::filesystem::path button_texture_dir = ClientUI::ArtDir() / "icons" / "buttons";

    // random seed button
    m_random = Wnd::Create<CUIButton>(
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "randomize.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "randomize_clicked.png")),
        GG::SubTexture(ClientUI::GetTexture(button_texture_dir / "randomize_mouseover.png")));

    m_random->SetBrowseText(UserString("GSETUP_RANDOM_SEED"));
    m_random->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    // number of stars
    m_stars_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_STARS"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_stars_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_stars_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.star.count")));
    m_stars_spin = GG::Wnd::Create<CUISpin<int>>(100, 1, 10, 5000, true);

    // galaxy shape
    m_galaxy_shapes_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_SHAPE"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_galaxy_shapes_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_galaxy_shapes_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.galaxy.shape")));
    m_galaxy_shapes_list = GG::Wnd::Create<CUIDropDownList>(5);
    m_galaxy_shapes_list->SetStyle(GG::LIST_NOSORT);

    // galaxy age
    m_galaxy_ages_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_AGE"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_galaxy_ages_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_galaxy_ages_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.galaxy.age")));
    m_galaxy_ages_list = GG::Wnd::Create<CUIDropDownList>(5);
    m_galaxy_ages_list->SetStyle(GG::LIST_NOSORT);

    // starlane frequency
    m_starlane_freq_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_STARLANE_FREQ"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_starlane_freq_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_starlane_freq_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.starlane.frequency")));
    m_starlane_freq_list = GG::Wnd::Create<CUIDropDownList>(5);
    m_starlane_freq_list->SetStyle(GG::LIST_NOSORT);

    // planet density
    m_planet_density_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_PLANET_DENSITY"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_planet_density_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_planet_density_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.planet.density")));
    m_planet_density_list = GG::Wnd::Create<CUIDropDownList>(5);
    m_planet_density_list->SetStyle(GG::LIST_NOSORT);

    // specials frequency
    m_specials_freq_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_SPECIALS_FREQ"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_specials_freq_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_specials_freq_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.specials.frequency")));
    m_specials_freq_list = GG::Wnd::Create<CUIDropDownList>(5);
    m_specials_freq_list->SetStyle(GG::LIST_NOSORT);

    // monster frequency
    m_monster_freq_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_MONSTER_FREQ"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_monster_freq_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_monster_freq_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.monster.frequency")));
    m_monster_freq_list = GG::Wnd::Create<CUIDropDownList>(8);
    m_monster_freq_list->SetStyle(GG::LIST_NOSORT);

    // native frequency
    m_native_freq_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_NATIVE_FREQ"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_native_freq_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_native_freq_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.native.frequency")));
    m_native_freq_list = GG::Wnd::Create<CUIDropDownList>(5);
    m_native_freq_list->SetStyle(GG::LIST_NOSORT);

    // ai aggression
    m_ai_aggression_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_AI_AGGR"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_ai_aggression_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_ai_aggression_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.ai.aggression")));
    m_ai_aggression_list = GG::Wnd::Create<CUIDropDownList>(5);
    m_ai_aggression_list->SetStyle(GG::LIST_NOSORT);

    AttachChild(m_seed_label);
    AttachChild(m_seed_edit);
    AttachChild(m_random);
    AttachChild(m_stars_label);
    AttachChild(m_stars_spin);
    AttachChild(m_galaxy_shapes_label);
    AttachChild(m_galaxy_shapes_list);
    AttachChild(m_galaxy_ages_label);
    AttachChild(m_galaxy_ages_list);
    AttachChild(m_starlane_freq_label);
    AttachChild(m_starlane_freq_list);
    AttachChild(m_planet_density_label);
    AttachChild(m_planet_density_list);
    AttachChild(m_specials_freq_label);
    AttachChild(m_specials_freq_list);
    AttachChild(m_monster_freq_label);
    AttachChild(m_monster_freq_list);
    AttachChild(m_native_freq_label);
    AttachChild(m_native_freq_list);
    AttachChild(m_ai_aggression_label);
    AttachChild(m_ai_aggression_list);

    TraceLogger() << "GalaxySetupPanel::CompleteConstruction layout";
    DoLayout();

    using boost::placeholders::_1;

    TraceLogger() << "GalaxySetupPanel::CompleteConstruction connecting signals and loading textures";
    m_random->LeftClickedSignal.connect(            boost::bind(&GalaxySetupPanel::RandomClicked, this));
    m_seed_edit->FocusUpdateSignal.connect(         boost::bind(&GalaxySetupPanel::SetSeed, this, _1, false));
    m_stars_spin->ValueChangedSignal.connect(       boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_galaxy_shapes_list->SelChangedSignal.connect( boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_galaxy_ages_list->SelChangedSignal.connect(   boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_starlane_freq_list->SelChangedSignal.connect( boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_planet_density_list->SelChangedSignal.connect(boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_specials_freq_list->SelChangedSignal.connect( boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_monster_freq_list->SelChangedSignal.connect(  boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_native_freq_list->SelChangedSignal.connect(   boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_ai_aggression_list->SelChangedSignal.connect( boost::bind(&GalaxySetupPanel::SettingChanged, this));
    m_galaxy_shapes_list->SelChangedSignal.connect( boost::bind(&GalaxySetupPanel::ShapeChanged, this, _1));

    // create and load textures
    m_textures.clear();
    m_textures.resize(int(Shape::GALAXY_SHAPES));
    m_textures[int(Shape::SPIRAL_2)] =   ClientUI::GetTexture(ClientUI::ArtDir() / "gp_spiral2.png");
    m_textures[int(Shape::SPIRAL_3)] =   ClientUI::GetTexture(ClientUI::ArtDir() / "gp_spiral3.png");
    m_textures[int(Shape::SPIRAL_4)] =   ClientUI::GetTexture(ClientUI::ArtDir() / "gp_spiral4.png");
    m_textures[int(Shape::CLUSTER)] =    ClientUI::GetTexture(ClientUI::ArtDir() / "gp_cluster.png");
    m_textures[int(Shape::ELLIPTICAL)] = ClientUI::GetTexture(ClientUI::ArtDir() / "gp_elliptical.png");
    m_textures[int(Shape::DISC)] =       ClientUI::GetTexture(ClientUI::ArtDir() / "gp_disc.png");
    m_textures[int(Shape::BOX)] =        ClientUI::GetTexture(ClientUI::ArtDir() / "gp_box.png");
    m_textures[int(Shape::IRREGULAR)] =  ClientUI::GetTexture(ClientUI::ArtDir() / "gp_irregular.png");
    m_textures[int(Shape::RING)] =       ClientUI::GetTexture(ClientUI::ArtDir() / "gp_ring.png");
    m_textures[int(Shape::RANDOM)] =     ClientUI::GetTexture(ClientUI::ArtDir() / "gp_random.png");

    // fill droplists
    TraceLogger() << "GalaxySetupPanel::CompleteConstruction filling droplists";
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_2ARM")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_3ARM")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_4ARM")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_CLUSTER")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_ELLIPTICAL")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_DISC")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_BOX")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_IRREGULAR")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_RING")));
    m_galaxy_shapes_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_RANDOM")));

    m_galaxy_ages_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_YOUNG")));
    m_galaxy_ages_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_MATURE")));
    m_galaxy_ages_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_ANCIENT")));
    m_galaxy_ages_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_RANDOM")));

    m_starlane_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_LOW")));
    m_starlane_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_MEDIUM")));
    m_starlane_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_HIGH")));
    m_starlane_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_RANDOM")));

    m_planet_density_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_LOW")));
    m_planet_density_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_MEDIUM")));
    m_planet_density_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_HIGH")));
    m_planet_density_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_RANDOM")));

    m_specials_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_NONE")));
    m_specials_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_LOW")));
    m_specials_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_MEDIUM")));
    m_specials_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_HIGH")));
    m_specials_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_RANDOM")));

    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_NONE")));
    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_EXTREMELY_LOW")));
    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_VERY_LOW")));
    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_LOW")));
    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_MEDIUM")));
    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_HIGH")));
    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_VERY_HIGH")));
    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_EXTREMELY_HIGH")));
    m_monster_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_RANDOM")));

    m_native_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_NONE")));
    m_native_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_LOW")));
    m_native_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_MEDIUM")));
    m_native_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_HIGH")));
    m_native_freq_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_RANDOM")));

    m_ai_aggression_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_BEGINNER")));
    m_ai_aggression_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_TURTLE")));
    m_ai_aggression_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_CAUTIOUS")));
    m_ai_aggression_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_TYPICAL")));
    m_ai_aggression_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_AGGRESSIVE")));
    m_ai_aggression_list->Insert(GG::Wnd::Create<CUISimpleDropDownListRow>(UserString("GSETUP_MANIACAL")));

    // initial settings from stored results or defaults
    TraceLogger() << "GalaxySetupPanel::CompleteConstruction selecting stored settings or defaults";

    SetSeed(GetOptionsDB().Get<std::string>("setup.seed"), true);
    m_seed_edit->Disable(GetSeed() == "RANDOM");
    m_stars_spin->SetValue(GetOptionsDB().Get<int>("setup.star.count"));
    m_galaxy_shapes_list->Select(int(GetOptionsDB().Get<Shape>("setup.galaxy.shape")));
    ShapeChanged(m_galaxy_shapes_list->CurrentItem());
    m_galaxy_ages_list->Select(int(GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.galaxy.age")) - 1);
    m_starlane_freq_list->Select(int(GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.starlane.frequency")) - 1);
    m_planet_density_list->Select(int(GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.planet.density")) - 1);
    m_specials_freq_list->Select(int(GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.specials.frequency")));
    m_monster_freq_list->Select(int(GetOptionsDB().Get<GalaxySetupOptionMonsterFreq>("setup.monster.frequency")));
    m_native_freq_list->Select(int(GetOptionsDB().Get<GalaxySetupOptionGeneric>("setup.native.frequency")));
    m_ai_aggression_list->Select(int(GetOptionsDB().Get<Aggression>("setup.ai.aggression")));

    TraceLogger() << "GalaxySetupPanel::CompleteConstruction settings changed signal...";
    SettingsChangedSignal();
    TraceLogger() << "GalaxySetupPanel::CompleteConstruction done";
}

void GalaxySetupPanel::RandomClicked() {
    if (m_seed == "RANDOM")
        SetSeed(GetOptionsDB().GetDefault<std::string>("setup.seed"));
    else
        SetSeed("RANDOM");
}

const std::string& GalaxySetupPanel::GetSeed() const
{ return m_seed; }

void GalaxySetupPanel::SetSeed(const std::string& seed, bool inhibit_signal) {
    if (seed == "RANDOM" || seed.empty()) {
        m_seed = "RANDOM";
        m_seed_edit->SetText(UserString("GSETUP_RANDOM"));
        m_seed_edit->Disable();
    } else {
        m_seed = seed;
        m_seed_edit->SetText(m_seed);
        m_seed_edit->Disable(false);
    }

    if (!inhibit_signal)
        SettingChanged();
}

int GalaxySetupPanel::Systems() const
{ return m_stars_spin->Value(); }

Shape GalaxySetupPanel::GetShape() const
{ return Shape(m_galaxy_shapes_list->CurrentItemIndex()); }

GalaxySetupOptionGeneric GalaxySetupPanel::GetAge() const
{ return GalaxySetupOptionGeneric(m_galaxy_ages_list->CurrentItemIndex() + 1); }

GalaxySetupOptionGeneric GalaxySetupPanel::GetStarlaneFrequency() const
{ return GalaxySetupOptionGeneric(m_starlane_freq_list->CurrentItemIndex() + 1); }

GalaxySetupOptionGeneric GalaxySetupPanel::GetPlanetDensity() const
{ return GalaxySetupOptionGeneric(m_planet_density_list->CurrentItemIndex() + 1); }

GalaxySetupOptionGeneric GalaxySetupPanel::GetSpecialsFrequency() const
{ return GalaxySetupOptionGeneric(m_specials_freq_list->CurrentItemIndex()); }

GalaxySetupOptionMonsterFreq GalaxySetupPanel::GetMonsterFrequency() const
{ return GalaxySetupOptionMonsterFreq(m_monster_freq_list->CurrentItemIndex()); }

GalaxySetupOptionGeneric GalaxySetupPanel::GetNativeFrequency() const
{ return GalaxySetupOptionGeneric(m_native_freq_list->CurrentItemIndex()); }

Aggression GalaxySetupPanel::GetAIAggression() const
{ return Aggression(m_ai_aggression_list->CurrentItemIndex()); }

std::shared_ptr<GG::Texture> GalaxySetupPanel::PreviewImage() const
{ return m_textures[int(GetShape())]; }

void GalaxySetupPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Control::SizeMove(ul, lr);
    DoLayout();
}

void GalaxySetupPanel::DoLayout() {
    const GG::X LABELS_WIDTH = (Width() - CONTROL_MARGIN) / 2;
    const GG::X DROPLIST_WIDTH = LABELS_WIDTH;
    const GG::Y DROPLIST_HEIGHT{ClientUI::Pts() + 12};

    GG::Pt row_advance(GG::X0, PANEL_CONTROL_SPACING);

    GG::Pt label_ul(CONTROL_MARGIN, GG::Y0);
    GG::Pt label_lr = label_ul + GG::Pt(LABELS_WIDTH, CONTROL_HEIGHT);

    GG::Pt control_ul(GG::Pt(LABELS_WIDTH + 2 * CONTROL_MARGIN, GG::Y0) + GG::Pt(GG::X0, (PANEL_CONTROL_SPACING - m_seed_edit->MinUsableSize().y) / 2));
    GG::Pt control_lr = control_ul + GG::Pt(LABELS_WIDTH -30, m_seed_edit->MinUsableSize().y);
    GG::Pt button_ul(2 * LABELS_WIDTH + 3 * CONTROL_MARGIN - 30, CONTROL_VMARGIN);
    GG::Pt button_lr = button_ul + GG::Pt(GG::X{20}, GG::Y{20});

    m_seed_label->SizeMove(label_ul, label_lr);
    m_seed_edit->SizeMove(control_ul, control_lr);
    m_random->SizeMove(button_ul, button_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr = control_ul + GG::Pt(GG::X(75),  ClientUI::GetFont()->Height() + 2 * 5);

    m_stars_label->SizeMove(label_ul, label_lr);
    m_stars_spin->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr = control_ul + GG::Pt(DROPLIST_WIDTH, DROPLIST_HEIGHT);

    m_galaxy_shapes_label->SizeMove(label_ul, label_lr);
    m_galaxy_shapes_list->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_galaxy_ages_label->SizeMove(label_ul, label_lr);
    m_galaxy_ages_list->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_starlane_freq_label->SizeMove(label_ul, label_lr);
    m_starlane_freq_list->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_planet_density_label->SizeMove(label_ul, label_lr);
    m_planet_density_list->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_specials_freq_label->SizeMove(label_ul, label_lr);
    m_specials_freq_list->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_monster_freq_label->SizeMove(label_ul, label_lr);
    m_monster_freq_list->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_native_freq_label->SizeMove(label_ul, label_lr);
    m_native_freq_list->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_ai_aggression_label->SizeMove(label_ul, label_lr);
    m_ai_aggression_list->SizeMove(control_ul, control_lr);
}

void GalaxySetupPanel::Disable(bool b) {
    for (auto& child : Children())
        static_cast<GG::Control*>(child.get())->Disable(b);
}

void GalaxySetupPanel::SetFromSetupData(const GalaxySetupData& setup_data) {
    SetSeed(setup_data.seed, true);
    m_stars_spin->SetValue(setup_data.size);
    m_galaxy_shapes_list->Select(int(setup_data.shape));
    ShapeChanged(m_galaxy_shapes_list->CurrentItem());
    m_galaxy_ages_list->Select(int(setup_data.age) - 1);
    m_starlane_freq_list->Select(int(setup_data.starlane_freq) - 1);
    m_planet_density_list->Select(int(setup_data.planet_density) - 1);
    m_specials_freq_list->Select(int(setup_data.specials_freq));
    m_monster_freq_list->Select(int(setup_data.monster_freq));
    m_native_freq_list->Select(int(setup_data.native_freq));
    m_ai_aggression_list->Select(int(setup_data.ai_aggr));
}

void GalaxySetupPanel::GetSetupData(GalaxySetupData& setup_data) const {
    setup_data.SetSeed(GetSeed());
    setup_data.size =           Systems();
    setup_data.shape =          GetShape();
    setup_data.age =            GetAge();
    setup_data.starlane_freq =  GetStarlaneFrequency();
    setup_data.planet_density = GetPlanetDensity();
    setup_data.specials_freq =  GetSpecialsFrequency();
    setup_data.monster_freq =   GetMonsterFrequency();
    setup_data.native_freq =    GetNativeFrequency();
    setup_data.ai_aggr =        GetAIAggression();
}

void GalaxySetupPanel::SettingChanged() {
    Sound::TempUISoundDisabler sound_disabler;
    SettingsChangedSignal();
}

void GalaxySetupPanel::ShapeChanged(GG::DropDownList::iterator it)
{ ImageChangedSignal(m_textures[m_galaxy_shapes_list->IteratorToIndex(it)]); }


////////////////////////////////////////////////
// GalaxySetupWnd
////////////////////////////////////////////////
GalaxySetupWnd::GalaxySetupWnd() :
    CUIWnd(UserString("GSETUP_WINDOW_TITLE"), GG::INTERACTIVE | GG::MODAL | GG::DRAGABLE /*| GG::RESIZABLE*/)
{}

void GalaxySetupWnd::CompleteConstruction() {
    Sound::TempUISoundDisabler sound_disabler;

    DebugLogger() << "GalaxySetupWnd::CompleteConstruction";

    try {
        m_galaxy_setup_panel = GG::Wnd::Create<GalaxySetupPanel>();
    } catch (const boost::bad_any_cast& e) {
        ErrorLogger() << "bad any cast creating galaxy setup panel: " << e.what();
        throw;
    }
    try {
        m_game_rules_panel = GG::Wnd::Create<GameRulesPanel>();
    } catch (const boost::bad_any_cast& e) {
        ErrorLogger() << "bad any cast creating game rules panel: " << e.what();
        throw;
    }

    const GG::X LABELS_WIDTH = (GalaxySetupPanel::DefaultWidth() - 5) / 2;

    // player name
    m_player_name_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_PLAYER_NAME"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_player_name_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_player_name_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.player.name")));
    m_player_name_edit = GG::Wnd::Create<CUIEdit>(GetOptionsDB().Get<std::string>("setup.player.name"));
    m_player_name_edit->DisallowChars(formatting_chars);

    // empire name
    m_empire_name_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_EMPIRE_NAME"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_empire_name_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_empire_name_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.empire.name")));
    m_empire_name_edit = GG::Wnd::Create<CUIEdit>(GetOptionsDB().Get<std::string>("setup.empire.name"));
    m_empire_name_edit->DisallowChars(formatting_chars);

    // empire color
    m_empire_color_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_EMPIRE_COLOR"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_empire_color_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_empire_color_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.empire.color.index")));
    m_empire_color_selector = GG::Wnd::Create<EmpireColorSelector>(CONTROL_HEIGHT - CONTROL_VMARGIN);
    m_empire_color_selector->Select(GetOptionsDB().Get<int>("setup.empire.color.index"));

    // starting species
    m_starting_species_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_SPECIES"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_starting_species_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_starting_species_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.initial.species")));
    std::string default_starting_species = GetOptionsDB().Get<std::string>("setup.initial.species");

    if (default_starting_species.empty() || default_starting_species == "1")
        // kludge / bug workaround for bug with options storage and retreival.
        // Empty-string options are stored, but read in as "true" boolean, and
        // converted to string equal to "1"
        default_starting_species = "SP_HUMAN";

    // Subtract the drop down entry padding for the inner element height.
    m_starting_secies_selector = GG::Wnd::Create<SpeciesSelector>(default_starting_species, LABELS_WIDTH, CONTROL_HEIGHT - 5);

    // number of AIs
    m_number_ais_label = GG::Wnd::Create<CUILabel>(UserString("GSETUP_NUMBER_AIS"), GG::FORMAT_RIGHT, GG::INTERACTIVE);
    m_number_ais_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    m_number_ais_label->SetBrowseText(UserString(GetOptionsDB().GetDescription("setup.ai.player.count")));
    m_number_ais_spin = GG::Wnd::Create<CUISpin<int>>(GetOptionsDB().Get<int>("setup.ai.player.count"), 1, 0, IApp::MAX_AI_PLAYERS(), true);

    // create a temporary texture and static graphic
    static auto temp_tex = std::make_shared<GG::Texture>();
    m_preview_image =  GG::Wnd::Create<GG::StaticGraphic>(temp_tex, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE); // create a blank graphic

    m_ok = Wnd::Create<CUIButton>(UserString("OK"));
    m_cancel = Wnd::Create<CUIButton>(UserString("CANCEL"));

    AttachChild(m_galaxy_setup_panel);
    AttachChild(m_game_rules_panel);
    AttachChild(m_player_name_label);
    AttachChild(m_player_name_edit);
    AttachChild(m_empire_name_label);
    AttachChild(m_empire_name_edit);
    AttachChild(m_empire_color_label);
    AttachChild(m_empire_color_selector);
    AttachChild(m_starting_species_label);
    AttachChild(m_starting_secies_selector);
    AttachChild(m_number_ais_label);
    AttachChild(m_number_ais_spin);
    AttachChild(m_preview_image);
    AttachChild(m_ok);
    AttachChild(m_cancel);

    ResetDefaultPosition();
    DoLayout();
    SaveDefaultedOptions();

    using boost::placeholders::_1;

    m_galaxy_setup_panel->ImageChangedSignal.connect(boost::bind(&GalaxySetupWnd::PreviewImageChanged,  this, _1));
    m_player_name_edit->EditedSignal.connect(        boost::bind(&GalaxySetupWnd::PlayerNameChanged,    this, _1));
    m_empire_name_edit->EditedSignal.connect(        boost::bind(&GalaxySetupWnd::EmpireNameChanged,    this, _1));
    m_ok->LeftClickedSignal.connect(                 boost::bind(&GalaxySetupWnd::OkClicked,            this));
    m_cancel->LeftClickedSignal.connect(             boost::bind(&GalaxySetupWnd::CancelClicked,        this));

    PreviewImageChanged(m_galaxy_setup_panel->PreviewImage());

    CUIWnd::CompleteConstruction();
}

const std::string& GalaxySetupWnd::EmpireName() const
{ return m_empire_name_edit->Text(); }

GG::Clr GalaxySetupWnd::EmpireColor() const
{ return m_empire_color_selector->CurrentColor(); }

const std::string& GalaxySetupWnd::StartingSpeciesName() const
{ return m_starting_secies_selector->CurrentSpeciesName(); }

int GalaxySetupWnd::NumberAIs() const
{ return m_number_ais_spin->Value(); }

std::map<std::string, std::string> GalaxySetupWnd::GetRulesAsStrings() const
{ return m_game_rules_panel->GetRulesAsStrings(); }

void GalaxySetupWnd::Render() {
    CUIWnd::Render();
    GG::FlatRectangle(GG::Pt(ClientUpperLeft().x + m_preview_ul.x - 2,
                             ClientUpperLeft().y + m_preview_ul.y - 2),
                      GG::Pt(ClientUpperLeft().x + m_preview_ul.x + PREVIEW_SZ.x + 2,
                             ClientUpperLeft().y + m_preview_ul.y + PREVIEW_SZ.y + 2),
                      GG::CLR_BLACK, ClientUI::WndInnerBorderColor(), 1);
}

void GalaxySetupWnd::KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    // Enter is no longer accepted as OK as it could clash with ALT-Enter
    if (key == GG::Key::GGK_ESCAPE) // Same behaviour as if "Cancel" was pressed
        CancelClicked();
}

void GalaxySetupWnd::SizeMove(GG::Pt ul, GG::Pt lr) {
    CUIWnd::SizeMove(ul, lr);
    DoLayout();
}

GG::Rect GalaxySetupWnd::CalculatePosition() const {
    GG::Pt new_ul((GGHumanClientApp::GetApp()->AppWidth() - GalSetupWndWidth()) / 2,
                  (GGHumanClientApp::GetApp()->AppHeight() - GalSetupWndHeight()) / 2);
    GG::Pt new_sz(GalSetupWndWidth(), GalSetupWndHeight());
    return GG::Rect(new_ul, new_ul + new_sz);
}

void GalaxySetupWnd::DoLayout() {
    m_galaxy_setup_panel->MoveTo(GG::Pt(GG::X0, GG::Y(4)));

    const GG::X LABELS_WIDTH = (GalaxySetupPanel::DefaultWidth() - 5) / 2;

    GG::Pt row_advance(GG::X0, PANEL_CONTROL_SPACING);
    GG::Pt label_ul(CONTROL_MARGIN, GAL_SETUP_PANEL_HT + (PANEL_CONTROL_SPACING - CONTROL_HEIGHT) / 2 + 4);
    GG::Pt label_lr = label_ul + GG::Pt(LABELS_WIDTH, CONTROL_HEIGHT);
    GG::Pt control_ul(LABELS_WIDTH + 2 * CONTROL_MARGIN, GAL_SETUP_PANEL_HT + (PANEL_CONTROL_SPACING - CONTROL_HEIGHT) / 2 + 7);
    GG::Pt control_lr = control_ul + GG::Pt(LABELS_WIDTH, m_player_name_edit->MinUsableSize().y);

    m_player_name_label->SizeMove(label_ul, label_lr);
    m_player_name_edit->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_empire_name_label->SizeMove(label_ul, label_lr);
    m_empire_name_edit->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_empire_color_label->SizeMove(label_ul, label_lr);
    m_empire_color_selector->SizeMove(control_ul, control_lr - GG::Pt(GG::X(75), GG::Y0));

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr += row_advance;

    m_starting_species_label->SizeMove(label_ul, label_lr);
    m_starting_secies_selector->SizeMove(control_ul, control_lr);

    label_ul += row_advance;
    label_lr += row_advance;
    control_ul += row_advance;
    control_lr = control_ul + GG::Pt(GG::X(75),  ClientUI::GetFont()->Height() + 2 * 5);

    m_number_ais_label->SizeMove(label_ul, label_lr);
    m_number_ais_spin->SizeMove(control_ul, control_lr);

    m_preview_ul = GG::Pt(ClientWidth() - PREVIEW_SZ.x - 7, GG::Y(7));
    m_preview_image->SizeMove(m_preview_ul, m_preview_ul + PREVIEW_SZ);


    GG::Pt rules_ul = m_preview_ul + GG::Pt(GG::X0, PREVIEW_SZ.y);
    GG::Pt rules_lr = ClientSize() - GG::Pt(CONTROL_MARGIN, CONTROL_VMARGIN);
    m_game_rules_panel->SizeMove(rules_ul, rules_lr);

    GG::Pt button_ul(CONTROL_MARGIN * 2, ScreenToClient(ClientLowerRight()).y - CONTROL_VMARGIN * 2 - m_ok->MinUsableSize().y);
    GG::Pt button_lr(m_ok->MinUsableSize());
    button_lr.x = std::max(button_lr.x, m_cancel->MinUsableSize().x);
    button_lr.y = std::max(button_lr.y, m_cancel->MinUsableSize().y);
    button_lr += button_ul;

    m_ok->SizeMove(button_ul, button_lr);

    button_ul.x += m_ok->Width() + CONTROL_MARGIN * 2;
    button_lr.x += m_ok->Width() + CONTROL_MARGIN * 2;
    m_cancel->SizeMove(button_ul, button_lr);
}

void GalaxySetupWnd::PreviewImageChanged(std::shared_ptr<GG::Texture> new_image) {
    if (m_preview_image)
        m_preview_image->SetTexture(new_image);
    DoLayout();
}

void GalaxySetupWnd::EmpireNameChanged(const std::string& name)
{ m_ok->Disable(name.empty()); }

void GalaxySetupWnd::PlayerNameChanged(const std::string& name)
{ m_ok->Disable(name.empty()); }

void GalaxySetupWnd::OkClicked() {
    TraceLogger() << "GalaxySetupWnd::OkClicked start";
    // record selected galaxy setup options as new defaults
    try {
        GetOptionsDB().Set("setup.seed",                m_galaxy_setup_panel->GetSeed());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting setup seed from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.star.count",          m_galaxy_setup_panel->Systems());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting star count from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.galaxy.shape",        m_galaxy_setup_panel->GetShape());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting galaxy shape from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.galaxy.age",          m_galaxy_setup_panel->GetAge());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting galaxy age from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.starlane.frequency",  m_galaxy_setup_panel->GetStarlaneFrequency());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting starlane frequency from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.planet.density",      m_galaxy_setup_panel->GetPlanetDensity());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting planet density from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.specials.frequency",  m_galaxy_setup_panel->GetSpecialsFrequency());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting special frequency from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.monster.frequency",   m_galaxy_setup_panel->GetMonsterFrequency());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting monster frequency from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.native.frequency",    m_galaxy_setup_panel->GetNativeFrequency());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting native frequency from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.ai.aggression",       m_galaxy_setup_panel->GetAIAggression());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting ai aggression from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.player.name",         m_player_name_edit->Text());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting player name from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.empire.name",         EmpireName());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting empire name from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.empire.color.index",  static_cast<int>(m_empire_color_selector->CurrentItemIndex()));
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting color from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.initial.species",     m_starting_secies_selector->CurrentSpeciesName());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting species from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Set("setup.ai.player.count",     m_number_ais_spin->Value());
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting player count from setup panel results: " << e.what();
    }
    try {
        GetOptionsDB().Commit();
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception committing options from panel results: " << e.what();
    }

    TraceLogger() << "GalaxySetupWnd::OkClicked end";

    m_ended_with_ok = true;
    m_modal_done.store(true);
}

void GalaxySetupWnd::CancelClicked() {
    m_ended_with_ok = false;
    m_modal_done.store(true);
}
