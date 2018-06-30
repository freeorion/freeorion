#include "OptionsWnd.h"

#include "../client/human/HumanClientApp.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/LoggerWithOptionsDB.h"

#include "ClientUI.h"
#include "CUISpin.h"
#include "CUISlider.h"
#include "Sound.h"
#include "Hotkeys.h"

#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/TabWnd.h>

//boost::spirit::classic pulls in windows.h which in turn defines the macros
//MessageBox, PlaySound, min and max. Disabling the generation of the min and
// max macros and undefining those should avoid name collisions with std c++
// library and FreeOrion function names.
#define NOMINMAX
#include <boost/spirit/include/classic.hpp>
#ifdef FREEORION_WIN32
#  undef MessageBox
#  undef PlaySound
#endif

#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <regex>


namespace fs = boost::filesystem;

namespace {
    const GG::X PAGE_WIDTH(400);
    const GG::Y PAGE_HEIGHT(520);
    const GG::X INDENTATION(20);
    const GG::X ROW_WIDTH(PAGE_WIDTH - 4 - 14 - 5);
    const GG::X COLOR_SELECTOR_WIDTH(75);
    const GG::X SPIN_WIDTH(92);
    const int LAYOUT_MARGIN = 5;

    const std::string OPTIONS_WND_NAME = "options";

    const std::string STRINGTABLE_FILE_SUFFIX = ".txt";
    const std::string MUSIC_FILE_SUFFIX = ".ogg";
    const std::string SOUND_FILE_SUFFIX = ".ogg";
    const std::string FONT_FILE_SUFFIX = ".ttf";

    class RowContentsWnd : public GG::Control {
    public:
        RowContentsWnd(GG::X w, GG::Y h, std::shared_ptr<Wnd> contents, int indentation_level) :
            Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
            m_contents(std::forward<std::shared_ptr<Wnd>>(contents)),
            m_indentation_level(indentation_level)
        {}

        void CompleteConstruction() override {
            GG::Control::CompleteConstruction();

            if (!m_contents)
                return;
            AttachChild(m_contents);
            m_contents->MoveTo(GG::Pt(GG::X(m_indentation_level * INDENTATION), GG::Y0));
            DoLayout();
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            const GG::Pt old_size = Size();
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

        void Render() override {
            //GG::FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_DARK_RED, GG::CLR_PINK, 1);
        }
    private:
        std::shared_ptr<Wnd> m_contents;
        int m_indentation_level;
    };

    struct BrowseForPathButtonFunctor {
        BrowseForPathButtonFunctor(const fs::path& path, const std::vector<std::pair<std::string, std::string>>& filters,
                                   std::shared_ptr<GG::Edit> edit, bool directory, bool return_relative_path) :
            m_path(path),
            m_filters(filters),
            m_edit(std::forward<std::shared_ptr<GG::Edit>>(edit)),
            m_directory(directory),
            m_return_relative_path(return_relative_path)
        {}

        void operator()() {
            try {
                auto dlg = GG::Wnd::Create<FileDlg>(m_path.string(), m_edit->Text(), false, false, m_filters);
                if (m_directory)
                    dlg->SelectDirectories(true);
                dlg->Run();
                if (!dlg->Result().empty()) {
                    fs::path path = m_return_relative_path ?
                        RelativePath(m_path, fs::path(*(dlg->Result().begin()))) :
                        fs::absolute(*(dlg->Result().begin()));
                    *m_edit << path.string();
                    m_edit->EditedSignal(m_edit->Text());
                }
            } catch (const std::exception& e) {
                ClientUI::MessageBox(e.what(), true);
            }
        }

        fs::path                                            m_path;
        std::vector<std::pair<std::string, std::string>>    m_filters;
        std::shared_ptr<GG::Edit>                                           m_edit;
        bool                                                m_directory;
        bool                                                m_return_relative_path;
    };

    bool ValidStringtableFile(const std::string& file) {
        // putting this in try-catch block prevents crash with error output along the lines of:
        // main() caught exception(std::exception): boost::filesystem::path: invalid name ":" in path: ":\FreeOrion\default"
        try {
            fs::path path = FilenameToPath(file);
            return boost::algorithm::ends_with(file, STRINGTABLE_FILE_SUFFIX) &&
                fs::exists(path) && !fs::is_directory(path);
        } catch (std::exception ex) {
        }
        return false;
    }

    bool ValidFontFile(const std::string& file) {
        // putting this in try-catch block prevents crash with error output along the lines of:
        // main() caught exception(std::exception): boost::filesystem::path: invalid name ":" in path: ":\FreeOrion\default"
        try {
            fs::path path = FilenameToPath(file);
            return boost::algorithm::ends_with(file, FONT_FILE_SUFFIX) &&
                fs::exists(path) && !fs::is_directory(path);
        } catch (std::exception ex) {
        }
        return false;
    }

    bool ValidMusicFile(const std::string& file) {
        // putting this in try-catch block prevents crash with error output along the lines of:
        // main() caught exception(std::exception): boost::filesystem::path: invalid name ":" in path: ":\FreeOrion\default"
        try {
            fs::path path = FilenameToPath(file);
            return boost::algorithm::ends_with(file, MUSIC_FILE_SUFFIX) &&
                fs::exists(path) && !fs::is_directory(path);
        } catch (std::exception ex) {
        }
        return false;
    }

    bool ValidSoundFile(const std::string& file) {
        // putting this in try-catch block prevents crash with error output along the lines of:
        // main() caught exception(std::exception): boost::filesystem::path: invalid name ":" in path: ":\FreeOrion\default"
        try {
            fs::path path = FilenameToPath(file);
            return boost::algorithm::ends_with(file, SOUND_FILE_SUFFIX) &&
                fs::exists(path) && !fs::is_directory(path);
        } catch (std::exception ex) {
        }
        return false;
    }

    bool ValidDirectory(const std::string& file) {
        // putting this in try-catch block prevents crash with error output along the lines of:
        // main() caught exception(std::exception): boost::filesystem::path: invalid name ":" in path: ":\FreeOrion\default"
        try {
            fs::path path = FilenameToPath(file);
            return fs::exists(path) && fs::is_directory(path);
        } catch (std::exception ex) {
        }
        return false;
    }

    // Small window that will grab a unique key press.
    class KeyPressCatcher : public GG::Wnd {
        GG::Key                 m_key;
        std::uint32_t m_code_point;
        GG::Flags<GG::ModKey>   m_mods;

    public:
        KeyPressCatcher() :
            Wnd(GG::X0, GG::Y0, GG::X0, GG::Y0, GG::Flags<GG::WndFlag>(GG::MODAL))
        {};

        void Render() override
        {}

        void KeyPress(GG::Key key, std::uint32_t key_code_point,
                      GG::Flags<GG::ModKey> mod_keys) override
        {
            m_key = key;
            m_code_point = key_code_point;
            m_mods = mod_keys;
            // exit modal loop only if not a modifier
            if (!(m_key >= GG::GGK_NUMLOCK && m_key <= GG::GGK_COMPOSE))
                m_done = true;

            /// @todo Clean up, ie transform LCTRL or RCTRL into CTRL and
            /// the like...
        };

        static std::pair<GG::Key, GG::Flags<GG::ModKey>> GetKeypress() {
            auto ct = GG::Wnd::Create<KeyPressCatcher>();
            ct->Run();
            return std::make_pair(ct->m_key, ct->m_mods);
        };
    };

    // Displays current font textures
    class FontTextureWnd : public CUIWnd {
    public:
        FontTextureWnd() :
            CUIWnd(UserString("OPTIONS_FONTS"),
                   GG::GUI::GetGUI()->AppWidth() / 6,       GG::GUI::GetGUI()->AppHeight() / 6,
                   GG::GUI::GetGUI()->AppWidth() * 2 / 3,   GG::GUI::GetGUI()->AppHeight() * 2 / 3,
                   GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE | CLOSABLE),
            m_font_graphic(nullptr),
            m_title_font_graphic(nullptr),
            m_hscroll(nullptr)
        {}

        void CompleteConstruction() override {
            CUIWnd::CompleteConstruction();

            GG::Y top = GG::Y1;

            std::shared_ptr<GG::Font> font = ClientUI::GetFont();
            std::shared_ptr<GG::Texture> texture;
            if (font)
                texture = font->GetTexture();
            if (texture) {
                m_font_graphic = GG::Wnd::Create<GG::StaticGraphic>(texture);
                m_font_graphic->MoveTo(GG::Pt(GG::X0, top));
                m_font_graphic->Resize(GG::Pt(texture->Width(), texture->Height()));
                AttachChild(m_font_graphic);
                top += m_font_graphic->Height() + 1;
            }

            font = ClientUI::GetTitleFont();
            texture.reset();
            if (font)
                texture = font->GetTexture();
            if (texture) {
                m_title_font_graphic = GG::Wnd::Create<GG::StaticGraphic>(texture);
                m_title_font_graphic->MoveTo(GG::Pt(GG::X0, top));
                m_title_font_graphic->Resize(GG::Pt(texture->Width(), texture->Height()));
                AttachChild(m_title_font_graphic);
            }


            m_hscroll =  GG::Wnd::Create<CUIScroll>(GG::HORIZONTAL);
            AttachChild(m_hscroll);

            m_hscroll->ScrolledSignal.connect(
                boost::bind(&FontTextureWnd::ScrolledSlot, this, _1, _2, _3, _4));
            DoLayout();
        }

    public:
        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            GG::Pt old_size = GG::Wnd::Size();

            CUIWnd::SizeMove(ul, lr);

            if (old_size != GG::Wnd::Size())
                DoLayout();
        }

        void DoLayout() {
            m_hscroll->SizeMove(GG::Pt(GG::X0,          ClientHeight() - ClientUI::ScrollWidth()),
                                GG::Pt(ClientWidth(),   ClientHeight()));

            int texture_width = 1;
            if (m_font_graphic)
                texture_width = std::max(texture_width, Value(m_font_graphic->Width()));
            if (m_title_font_graphic)
                texture_width = std::max(texture_width, Value(m_title_font_graphic->Width()));

             m_hscroll->SizeScroll(0, texture_width - Value(ClientWidth()) / 2, 1, 50);
        }

        void ScrolledSlot(int tab_low, int tab_high, int low, int high) {
            m_font_graphic->MoveTo(      GG::Pt(GG::X(-tab_low), GG::Y1));
            m_title_font_graphic->MoveTo(GG::Pt(GG::X(-tab_low), m_font_graphic->Height() + 2));
        }

    private:
        std::shared_ptr<GG::StaticGraphic>  m_font_graphic;
        std::shared_ptr<GG::StaticGraphic>  m_title_font_graphic;
        std::shared_ptr<GG::Scroll>         m_hscroll;
    };

    void ShowFontTextureWnd() {
        auto font_wnd =  GG::Wnd::Create<FontTextureWnd>();
        font_wnd->Run();
    }

    class OptionsListRow : public GG::ListBox::Row {
    public:
        OptionsListRow(GG::X w, GG::Y h, std::shared_ptr<RowContentsWnd> contents) :
            GG::ListBox::Row(w, h, ""),
            m_contents(std::forward<std::shared_ptr<RowContentsWnd>>(contents))
        {
            SetChildClippingMode(ClipToClient);
        }

        OptionsListRow(GG::X w, GG::Y h, std::shared_ptr<Wnd> contents, int indentation = 0) :
            GG::ListBox::Row(w, h, ""),
            m_contents(nullptr)
        {
            SetChildClippingMode(ClipToClient);
            if (contents)
                m_contents = GG::Wnd::Create<RowContentsWnd>(w, h, contents, indentation);
        }

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();
            if (m_contents)
                push_back(m_contents);
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            //std::cout << "OptionsListRow::SizeMove(" << ul << ", " << lr << ")" << std::endl;
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            if (!empty() && old_size != Size() && m_contents)
                m_contents->Resize(Size());
        }

        void Render() override {
            //GG::FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_DARK_BLUE, GG::CLR_YELLOW, 1);
        }
    private:
        std::shared_ptr<RowContentsWnd> m_contents;
    };

    class OptionsList : public CUIListBox {
    public:
        OptionsList() :
            CUIListBox()
        {
            InitRowSizes();

            SetColor(GG::CLR_ZERO);
            SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
            SetVScrollWheelIncrement(ClientUI::Pts() * 10);
        }

        void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override {
            const GG::Pt old_size = Size();
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

    /**Create UI controls for a logger level option.*/
    void LoggerLevelOption(GG::ListBox& page, bool is_sink,
                           const std::string& label, const std::string& option_name)
    {
        // Create the label
        auto logger_label = GG::Wnd::Create<CUILabel>(label, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);

        // Create a drop down list for the filtering levels
        auto num_log_levels = 1 + static_cast<std::size_t>(LogLevel::max) - static_cast<std::size_t>(LogLevel::min);
        auto drop_list = GG::Wnd::Create<CUIDropDownList>(num_log_levels);
        drop_list->Resize(GG::Pt(drop_list->MinUsableSize().x, GG::Y(ClientUI::Pts() + 4)));
        drop_list->SetMaxSize(GG::Pt(drop_list->MaxSize().x, drop_list->Size().y));
        drop_list->SetStyle(GG::LIST_NOSORT);
        drop_list->SetOnlyMouseScrollWhenDropped(true);

        // Insert the levels into the list
        for (auto ii = static_cast<std::size_t>(LogLevel::min); ii <= static_cast<std::size_t>(LogLevel::max); ++ii) {
            auto level_name = to_string(static_cast<LogLevel>(ii));
            auto priority_row = GG::Wnd::Create<CUISimpleDropDownListRow>(level_name);
            // use the row's name to store the option value.
            priority_row->SetName(level_name);
            drop_list->Insert(priority_row);
        }

        // Select the current filtering level in the list
        auto selected_level = static_cast<std::size_t>(to_LogLevel(GetOptionsDB().GetValueString(option_name)));
        if (drop_list->NumRows() >= 1)
            drop_list->Select(selected_level);

        // Make a layout with a row etc. for this option
        auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, LAYOUT_MARGIN);
        layout->Add(logger_label, 0, 0);
        layout->Add(drop_list,    0, 1, 1, 1, GG::ALIGN_VCENTER);

        auto row = GG::Wnd::Create<GG::ListBox::Row>();
        // row->Resize(GG::Pt(ROW_WIDTH, drop_list->MinUsableSize().y + LAYOUT_MARGIN + drop_list->MaxSize().y + 6));
        row->Resize(GG::Pt(ROW_WIDTH, drop_list->MinUsableSize().y + LAYOUT_MARGIN));

        auto row_wnd = GG::Wnd::Create<RowContentsWnd>(row->Width(), row->Height(), layout, 0);
        row_wnd->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
        row_wnd->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
        row->push_back(row_wnd);

        page.Insert(row);

        // Connect to Options DB
        drop_list->SelChangedSignal.connect(
            [option_name, drop_list](const GG::ListBox::const_iterator& it) {
                if (it == drop_list->end())
                    return;
                const auto dropdown_row = dynamic_cast<CUISimpleDropDownListRow* const>(it->get());
                const auto& option_value = dropdown_row->Name();
                HumanClientApp::GetApp()->ChangeLoggerThreshold(option_name, to_LogLevel(option_value));
            });
    }
}

OptionsWnd::OptionsWnd(bool is_game_running_):
    CUIWnd(UserString("OPTIONS_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE,
           OPTIONS_WND_NAME),
    is_game_running(is_game_running_),
    m_tabs(nullptr),
    m_done_button(nullptr)
{}

void OptionsWnd::CompleteConstruction() {
    m_done_button = Wnd::Create<CUIButton>(UserString("DONE"));
    // FIXME: PAGE_WIDTH is needed to prevent triggering an assert within the TabBar class.
    // The placement of the tab register buttons assumes that the whole TabWnd is at least
    // wider than the first tab button.
    m_tabs = GG::Wnd::Create<GG::TabWnd>(GG::X0, GG::Y0, PAGE_WIDTH, GG::Y1,
                                         ClientUI::GetFont(), ClientUI::WndColor(),
                                         ClientUI::TextColor());

    CUIWnd::CompleteConstruction();

    ResetDefaultPosition();
    SetMinSize(GG::Pt(PAGE_WIDTH + 20, PAGE_HEIGHT + 70));

    AttachChild(m_done_button);
    AttachChild(m_tabs);

    bool UI_sound_enabled = GetOptionsDB().Get<bool>("audio.effects.enabled");
    GG::ListBox* current_page = nullptr;

    Sound::TempUISoundDisabler sound_disabler;

    // Video settings tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_VIDEO"));
    ResolutionOption(current_page, 0);
    m_tabs->SetCurrentWnd(0);

    // Audio settings tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_AUDIO"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_VOLUME_AND_MUSIC"));
    MusicVolumeOption(current_page, 0, m_sound_feedback);
    VolumeOption(current_page, 0, "audio.effects.enabled", "audio.effects.volume", UserString("OPTIONS_UI_SOUNDS"), UI_sound_enabled, m_sound_feedback);
    FileOption(current_page, 0, "audio.music.path", UserString("OPTIONS_BACKGROUND_MUSIC"), ClientUI::SoundDir(),
               {UserString("OPTIONS_MUSIC_FILE"), "*" + MUSIC_FILE_SUFFIX},
               ValidMusicFile);

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_SOUNDS"));
    CreateSectionHeader(current_page, 1, UserString("OPTIONS_UI_SOUNDS"));
    SoundFileOption(current_page, 1, "ui.alert.sound.path", UserString("OPTIONS_SOUND_ALERT"));
    SoundFileOption(current_page, 1, "ui.input.keyboard.sound.path", UserString("OPTIONS_SOUND_TYPING"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_SOUND_NEWTURN"));
    BoolOption(current_page, 1, "ui.turn.start.sound.enabled", UserString("OPTIONS_SOUND_NEWTURN_TOGGLE"));
    SoundFileOption(current_page, 1, "ui.turn.start.sound.path", UserString("OPTIONS_SOUND_NEWTURN_FILE"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_SOUND_WINDOW"));
    SoundFileOption(current_page, 1, "ui.window.close.sound.path", UserString("OPTIONS_SOUND_CLOSE"));
    SoundFileOption(current_page, 1, "ui.window.maximize.sound.path", UserString("OPTIONS_SOUND_MAXIMIZE"));
    SoundFileOption(current_page, 1, "ui.window.minimize.sound.path", UserString("OPTIONS_SOUND_MINIMIZE"));
    SoundFileOption(current_page, 1, "ui.map.sidepanel.open.sound.path", UserString("OPTIONS_SOUND_SIDEPANEL"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_SOUND_LIST"));
    SoundFileOption(current_page, 1, "ui.listbox.drop.sound.path", UserString("OPTIONS_SOUND_DROP"));
    SoundFileOption(current_page, 1, "ui.dropdownlist.select.sound.path", UserString("OPTIONS_SOUND_PULLDOWN"));
    SoundFileOption(current_page, 1, "ui.listbox.select.sound.path", UserString("OPTIONS_SOUND_SELECT"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_SOUND_BUTTON"));
    SoundFileOption(current_page, 1, "ui.button.press.sound.path", UserString("OPTIONS_SOUND_CLICK"));
    SoundFileOption(current_page, 1, "ui.button.rollover.sound.path", UserString("OPTIONS_SOUND_ROLLOVER"));
    SoundFileOption(current_page, 1, "ui.map.fleet.button.press.sound.path", UserString("OPTIONS_SOUND_FLEET_CLICK"));
    SoundFileOption(current_page, 1, "ui.map.fleet.button.rollover.sound.path", UserString("OPTIONS_SOUND_FLEET_ROLLOVER"));
    SoundFileOption(current_page, 1, "ui.map.system.icon.rollover.sound.path", UserString("OPTIONS_SOUND_SYSTEM_ROLLOVER"));
    SoundFileOption(current_page, 1, "ui.button.turn.press.sound.path", UserString("OPTIONS_SOUND_TURN"));

    m_tabs->SetCurrentWnd(0);

    // UI settings tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_UI"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_MISC_UI"));
    BoolOption(current_page, 0, "ui.input.mouse.button.swap.enabled", UserString("OPTIONS_SWAP_MOUSE_LR"));
    BoolOption(current_page, 0, "ui.fleet.multiple.enabled", UserString("OPTIONS_MULTIPLE_FLEET_WNDS"));
    BoolOption(current_page, 0, "ui.quickclose.enabled", UserString("OPTIONS_QUICK_CLOSE_WNDS"));
    BoolOption(current_page, 0, "ui.map.sidepanel.planet.shown", UserString("OPTIONS_SHOW_SIDEPANEL_PLANETS"));
    BoolOption(current_page, 0, "ui.reposition.auto.enabled", UserString("OPTIONS_AUTO_REPOSITION_WINDOWS"));
    BoolOption(current_page, 0, "ui.map.messages.timestamp.shown", UserString("OPTIONS_DISPLAY_TIMESTAMP"));

    // manual reposition windows button
    auto window_reset_button = Wnd::Create<CUIButton>(UserString("OPTIONS_WINDOW_RESET"));
    auto row = GG::Wnd::Create<OptionsListRow>(
        ROW_WIDTH, window_reset_button->MinUsableSize().y + LAYOUT_MARGIN + 6,
        window_reset_button, 0);
    current_page->Insert(row);
    window_reset_button->LeftClickedSignal.connect(
        HumanClientApp::GetApp()->RepositionWindowsSignal);

    FileOption(current_page, 0, "resource.stringtable.path",    UserString("OPTIONS_LANGUAGE"),
               GetRootDataDir() / "default" / "stringtables",
               {UserString("OPTIONS_LANGUAGE_FILE"), "*" + STRINGTABLE_FILE_SUFFIX},
               &ValidStringtableFile);

    // flush stringtable button
    auto flush_button = Wnd::Create<CUIButton>(UserString("OPTIONS_FLUSH_STRINGTABLE"));
    row = GG::Wnd::Create<OptionsListRow>(
        ROW_WIDTH, flush_button->MinUsableSize().y + LAYOUT_MARGIN + 6,
        flush_button, 0);
    current_page->Insert(row);
    flush_button->LeftClickedSignal.connect(&FlushLoadedStringTables);

    IntOption(current_page, 0, "ui.tooltip.delay",                      UserString("OPTIONS_TOOLTIP_DELAY"));
    IntOption(current_page, 0, "ui.input.keyboard.repeat.delay",        UserString("OPTIONS_KEYPRESS_REPEAT_DELAY"));
    IntOption(current_page, 0, "ui.input.keyboard.repeat.interval",     UserString("OPTIONS_KEYPRESS_REPEAT_INTERVAL"));
    IntOption(current_page, 0, "ui.input.mouse.button.repeat.delay",    UserString("OPTIONS_MOUSE_REPEAT_DELAY"));
    IntOption(current_page, 0, "ui.input.mouse.button.repeat.interval", UserString("OPTIONS_MOUSE_REPEAT_INTERVAL"));

    CreateSectionHeader(current_page, 0,                                UserString("OPTIONS_FONTS"));
    FontOption(current_page, 0, "ui.font.path",                         UserString("OPTIONS_FONT_TEXT"));
    FontOption(current_page, 0, "ui.font.title.path",                   UserString("OPTIONS_FONT_TITLE"));

    // show font texture button
    auto show_font_texture_button = Wnd::Create<CUIButton>(UserString("SHOW_FONT_TEXTURES"));
    row = GG::Wnd::Create<OptionsListRow>(
        ROW_WIDTH, show_font_texture_button ->MinUsableSize().y + LAYOUT_MARGIN + 6,
        show_font_texture_button , 0);
    current_page->Insert(row);
    show_font_texture_button->LeftClickedSignal.connect(
        &ShowFontTextureWnd);

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_FONT_SIZES"));
    IntOption(current_page,    0, "ui.font.size",                       UserString("OPTIONS_FONT_TEXT"));
    IntOption(current_page,    0, "ui.font.title.size",                 UserString("OPTIONS_FONT_TITLE"));

    CreateSectionHeader(current_page, 0,                                UserString("OPTIONS_RESEARCH_WND"));
    DoubleOption(current_page, 0, "ui.research.tree.spacing.horizontal",UserString("OPTIONS_TECH_SPACING_HORIZONTAL"));
    DoubleOption(current_page, 0, "ui.research.tree.spacing.vertical",  UserString("OPTIONS_TECH_SPACING_VERTICAL"));
    DoubleOption(current_page, 0, "ui.research.tree.zoom.scale",        UserString("OPTIONS_TECH_LAYOUT_ZOOM"));
    DoubleOption(current_page, 0, "ui.research.control.graphic.size",   UserString("OPTIONS_TECH_CTRL_ICON_SIZE"));

    CreateSectionHeader(current_page, 0,                                UserString("OPTIONS_QUEUES"));
    IntOption(current_page,    0, "ui.queue.width",                     UserString("OPTIONS_UI_QUEUE_WIDTH"));
    BoolOption(current_page,   0, "ui.queue.production_location.shown", UserString("OPTIONS_UI_PROD_QUEUE_LOCATION"));

    CreateSectionHeader(current_page, 0,                                UserString("OPTIONS_DESCRIPTIONS"));
    BoolOption(current_page,   0, "resource.effects.description.shown", UserString("OPTIONS_DUMP_EFFECTS_GROUPS_DESC"));
    BoolOption(current_page,   0, "ui.map.sitrep.invalid.shown",        UserString("OPTIONS_VERBOSE_SITREP_DESC"));
    BoolOption(current_page,   0, "ui.name.id.shown",                   UserString("OPTIONS_SHOW_IDS_AFTER_NAMES"));

    m_tabs->SetCurrentWnd(0);

    // Galaxy Map Page
    current_page = CreatePage(UserString("OPTIONS_GALAXY_MAP"));
    CreateSectionHeader(current_page, 0,                                UserString("OPTIONS_SYSTEM_ICONS"));
    IntOption(current_page,    0, "ui.map.system.icon.size",            UserString("OPTIONS_UI_SYSTEM_ICON_SIZE"));
    BoolOption(current_page,   0, "ui.map.system.circle.shown",         UserString("OPTIONS_UI_SYSTEM_CIRCLES"));
    DoubleOption(current_page, 0, "ui.map.system.circle.size",          UserString("OPTIONS_UI_SYSTEM_CIRCLE_SIZE"));
    DoubleOption(current_page, 0, "ui.map.system.select.indicator.size",UserString("OPTIONS_UI_SYSTEM_SELECTION_INDICATOR_SIZE"));
    IntOption(current_page,    0, "ui.map.system.select.indicator.rpm", UserString("OPTIONS_UI_SYSTEM_SELECTION_INDICATOR_FPS"));
    IntOption(current_page,    0, "ui.map.system.icon.tiny.threshold",  UserString("OPTIONS_UI_SYSTEM_TINY_ICON_SIZE_THRESHOLD"));
    ColorOption(current_page,  0, "ui.map.system.unowned.name.color",   UserString("OPTIONS_UI_SYSTEM_NAME_UNOWNED_COLOR"));
    BoolOption(current_page,   0, "ui.map.scanlines.shown",             UserString("OPTIONS_UI_SYSTEM_FOG"));
    DoubleOption(current_page, 0, "ui.map.system.scanlines.spacing",    UserString("OPTIONS_UI_SYSTEM_FOG_SPACING"));

    CreateSectionHeader(current_page, 0,                                        UserString("OPTIONS_FLEET_ICONS"));
    DoubleOption(current_page, 0, "ui.map.fleet.button.tiny.zoom.threshold",    UserString("OPTIONS_UI_TINY_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption(current_page, 0, "ui.map.fleet.button.small.zoom.threshold",   UserString("OPTIONS_UI_SMALL_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption(current_page, 0, "ui.map.fleet.button.medium.zoom.threshold",  UserString("OPTIONS_UI_MEDIUM_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption(current_page, 0, "ui.map.fleet.select.indicator.size",         UserString("OPTIONS_UI_FLEET_SELECTION_INDICATOR_SIZE"));

    CreateSectionHeader(current_page, 0,                                UserString("OPTIONS_STARLANES"));
    DoubleOption(current_page, 0, "ui.map.starlane.thickness",          UserString("OPTIONS_STARLANE_THICKNESS"));
    BoolOption(current_page,   0, "ui.map.starlane.empire.color.shown", UserString("OPTIONS_RESOURCE_STARLANE_COLOURING"));
    DoubleOption(current_page, 0, "ui.map.starlane.thickness.factor",   UserString("OPTIONS_DB_STARLANE_CORE"));
    BoolOption(current_page,   0, "ui.map.fleet.supply.shown",          UserString("OPTIONS_FLEET_SUPPLY_LINES"));
    DoubleOption(current_page, 0, "ui.map.fleet.supply.width",          UserString("OPTIONS_FLEET_SUPPLY_LINE_WIDTH"));
    IntOption(current_page,    0, "ui.map.fleet.supply.dot.spacing",    UserString("OPTIONS_FLEET_SUPPLY_LINE_DOT_SPACING"));
    DoubleOption(current_page, 0, "ui.map.fleet.supply.dot.rate",       UserString("OPTIONS_FLEET_SUPPLY_LINE_DOT_RATE"));
    ColorOption(current_page,  0, "ui.map.starlane.color",              UserString("OPTIONS_UNOWNED_STARLANE_COLOUR"));

    CreateSectionHeader(current_page, 0,                                        UserString("OPTIONS_GALAXY_MAP_GENERAL"));
    BoolOption(current_page,   0, "ui.map.background.gas.shown",                UserString("OPTIONS_GALAXY_MAP_GAS"));
    BoolOption(current_page,   0, "ui.map.background.starfields.shown",         UserString("OPTIONS_GALAXY_MAP_STARFIELDS"));
    BoolOption(current_page,   0, "ui.map.scale.legend.shown",                  UserString("OPTIONS_GALAXY_MAP_SCALE_LINE"));
    BoolOption(current_page,   0, "ui.map.scale.circle.shown",                  UserString("OPTIONS_GALAXY_MAP_SCALE_CIRCLE"));
    BoolOption(current_page,   0, "ui.map.zoom.slider.shown",                   UserString("OPTIONS_GALAXY_MAP_ZOOM_SLIDER"));
    BoolOption(current_page,   0, "ui.map.detection.range.shown",               UserString("OPTIONS_GALAXY_MAP_DETECTION_RANGE"));
    IntOption(current_page,    0, "ui.map.detection.range.opacity",             UserString("OPTIONS_GALAXY_MAP_DETECTION_RANGE_OPACITY"));
    BoolOption(current_page,   0, "ui.map.menu.enabled",                        UserString("OPTIONS_GALAXY_MAP_POPUP"));
    BoolOption(current_page,   0, "ui.map.system.unexplored.rollover.enabled",  UserString("OPTIONS_UI_SYSTEM_UNEXPLORED_OVERLAY"));
    BoolOption(current_page,   0, "ui.production.mappanels.removed",            UserString("OPTIONS_UI_HIDE_MAP_PANELS"));

    m_tabs->SetCurrentWnd(0);

    // Objects List Page
    current_page = CreatePage(UserString("OPTIONS_PAGE_OBJECTS_WINDOW"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_COLUMNS"));
    for (unsigned int i = 0; i < 12u; ++i) {
        std::string col_width_opt_name = "ui.objects.columns.c" + std::to_string(i) + ".width";
        if (!GetOptionsDB().OptionExists(col_width_opt_name))
            break;
        std::string col_opt_name = "ui.objects.columns.c" + std::to_string(i) + ".stringkey";
        if (!GetOptionsDB().OptionExists(col_opt_name))
            break;
        std::string col_contents = GetOptionsDB().GetValueString(col_opt_name);
        const std::string& tx_contents = (col_contents.empty() ? "" : UserString(col_contents));

        IntOption(current_page, 0, col_width_opt_name, tx_contents);
    }

    m_tabs->SetCurrentWnd(0);

    // Colors tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_COLORS"));
    CreateSectionHeader(current_page, 0,                        UserString("OPTIONS_GENERAL_COLORS"));
    ColorOption(current_page, 0, "ui.font.color",               UserString("OPTIONS_TEXT_COLOR"));
    ColorOption(current_page, 0, "ui.font.link.color",          UserString("OPTIONS_DEFAULT_LINK_COLOR"));
    ColorOption(current_page, 0, "ui.font.link.rollover.color", UserString("OPTIONS_ROLLOVER_LINK_COLOR"));

    CreateSectionHeader(current_page, 0,                        UserString("OPTIONS_WINDOW_COLORS"));
    ColorOption(current_page, 0, "ui.window.background.color",  UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 0, "ui.window.border.inner.color",UserString("OPTIONS_INNER_BORDER_COLOR"));
    ColorOption(current_page, 0, "ui.window.border.outer.color",UserString("OPTIONS_OUTER_BORDER_COLOR"));

    CreateSectionHeader(current_page, 0,                            UserString("OPTIONS_CONTROL_COLORS"));
    ColorOption(current_page, 0, "ui.control.background.color",     UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 0, "ui.control.border.color",         UserString("OPTIONS_BORDER_COLOR"));
    ColorOption(current_page, 0, "ui.control.edit.highlight.color", UserString("OPTIONS_HIGHLIGHT_COLOR"));
    ColorOption(current_page, 0, "ui.dropdownlist.arrow.color",     UserString("OPTIONS_DROPLIST_ARROW_COLOR"));
    ColorOption(current_page, 0, "ui.button.state.color",           UserString("OPTIONS_STATE_BUTTON_COLOR"));
    ColorOption(current_page, 0, "ui.font.stat.increase.color",     UserString("OPTIONS_STAT_INCREASE_COLOR"));
    ColorOption(current_page, 0, "ui.font.stat.decrease.color",     UserString("OPTIONS_STAT_DECREASE_COLOR"));

    CreateSectionHeader(current_page, 0,                                UserString("OPTIONS_COMBAT_COLORS"));
    ColorOption(current_page, 0, "ui.combat.summary.dead.color",        UserString("OPTIONS_COMBAT_SUMMARY_DEAD_COLOR"));
    ColorOption(current_page, 0, "ui.combat.summary.damaged.color",     UserString("OPTIONS_COMBAT_SUMMARY_WOUND_COLOR"));
    ColorOption(current_page, 0, "ui.combat.summary.undamaged.color",   UserString("OPTIONS_COMBAT_SUMMARY_HEALTH_COLOR"));

    CreateSectionHeader(current_page, 0,                                            UserString("OPTIONS_TECH_COLORS"));
    CreateSectionHeader(current_page, 1,                                            UserString("OPTIONS_KNOWN_TECH_COLORS"));
    ColorOption(current_page, 1, "ui.research.status.completed.background.color",   UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 1, "ui.research.status.completed.border.color",       UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));

    CreateSectionHeader(current_page, 1,                                            UserString("OPTIONS_RESEARCHABLE_TECH_COLORS"));
    ColorOption(current_page, 1, "ui.research.status.researchable.background.color",UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 1, "ui.research.status.researchable.border.color",    UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_UNRESEARCHABLE_TECH_COLORS"));
    ColorOption(current_page, 1, "ui.research.status.unresearchable.background.color", UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 1, "ui.research.status.unresearchable.border.color", UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_TECH_PROGRESS_COLORS"));
    ColorOption(current_page, 1, "ui.research.status.progress.color", UserString("OPTIONS_PROGRESS_BAR_COLOR"));
    ColorOption(current_page, 1, "ui.research.status.progress.background.color", UserString("OPTIONS_PROGRESS_BACKGROUND_COLOR"));
    m_tabs->SetCurrentWnd(0);

    // Ausosave settings tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_AUTOSAVE"));
    BoolOption(current_page, 0, "save.auto.turn.start.enabled", UserString("OPTIONS_DB_AUTOSAVE_SINGLE_PLAYER_TURN_START"));
    BoolOption(current_page, 0, "save.auto.turn.end.enabled", UserString("OPTIONS_DB_AUTOSAVE_SINGLE_PLAYER_TURN_END"));
    BoolOption(current_page, 0, "save.auto.turn.multiplayer.start.enabled", UserString("OPTIONS_DB_AUTOSAVE_MULTIPLAYER_TURN_START"));
    IntOption(current_page,  0, "save.auto.turn.interval", UserString("OPTIONS_AUTOSAVE_TURNS_BETWEEN"));
    IntOption(current_page,  0, "save.auto.file.limit",     UserString("OPTIONS_AUTOSAVE_LIMIT"));
    BoolOption(current_page, 0, "save.auto.initial.enabled", UserString("OPTIONS_DB_AUTOSAVE_GALAXY_CREATION"));
    BoolOption(current_page, 0, "save.auto.exit.enabled",   UserString("OPTIONS_DB_AUTOSAVE_GAME_CLOSE"));
    m_tabs->SetCurrentWnd(0);

    // Keyboard shortcuts tab
    HotkeysPage();

    // Directories tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_DIRECTORIES"));
    /** GetRootDataDir() returns the default browse path when modifying this directory option.
     *  The actual default directory (before modifying) is gotten from the specified option name "resource.path" */
    DirectoryOption(current_page, 0, "resource.path", UserString("OPTIONS_FOLDER_SETTINGS"), GetRootDataDir(), is_game_running);
    DirectoryOption(current_page, 0, "save.path", UserString("OPTIONS_FOLDER_SAVE"), GetUserDataDir());
    DirectoryOption(current_page, 0, "save.server.path", UserString("OPTIONS_SERVER_FOLDER_SAVE"), GetUserDataDir());
    m_tabs->SetCurrentWnd(0);

    // Logging page
    current_page = CreatePage(UserString("OPTIONS_PAGE_LOGS"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_DB_UI_LOGGER_THRESHOLDS"),
                        UserString("OPTIONS_DB_UI_LOGGER_THRESHOLD_TOOLTIP"));

    const auto log_file_sinks = LoggerOptionsLabelsAndLevels(LoggerTypes::exec);
    for (const auto& sink : log_file_sinks) {
        const auto& option = std::get<0>(sink);
        const auto& option_label = std::get<1>(sink);
        const auto&& full_label = str(FlexibleFormat(UserString("OPTIONS_DB_UI_LOGGER_PER_PROCESS_GENERAL")) % option_label);
        LoggerLevelOption(*current_page, true, full_label, option);
    }

    const auto log_file_sources = LoggerOptionsLabelsAndLevels(LoggerTypes::named);
    for (const auto& source : log_file_sources) {
        const auto& option = std::get<0>(source);
        const auto& option_label = std::get<1>(source);
        LoggerLevelOption(*current_page, false, option_label, option);
    }

    // Misc
    current_page = CreatePage(UserString("OPTIONS_PAGE_MISC"));
    IntOption(current_page, 0, "effects.ui.threads",        UserString("OPTIONS_EFFECTS_THREADS_UI"));
    IntOption(current_page, 0, "effects.server.threads",    UserString("OPTIONS_EFFECTS_THREADS_SERVER"));
    IntOption(current_page, 0, "effects.ai.threads",        UserString("OPTIONS_EFFECTS_THREADS_AI"));
    BoolOption(current_page, 0, "resource.shipdesign.saved.enabled",    UserString("OPTIONS_ADD_SAVED_DESIGNS"));
    //BoolOption(current_page, 0, "resource.shipdesign.default.enabled",  UserString("OPTIONS_ADD_DEFAULT_DESIGNS"));   // hidden due to issues with implementation when not enabled preventing designs from being added or recreated
    BoolOption(current_page, 0, "save.format.binary.enabled",    UserString("OPTIONS_USE_BINARY_SERIALIZATION"));
    BoolOption(current_page, 0, "save.format.xml.zlib.enabled", UserString("OPTIONS_USE_XML_ZLIB_SERIALIZATION"));
    BoolOption(current_page, 0, "ui.map.sitrep.invalid.shown", UserString("OPTIONS_VERBOSE_SITREP_DESC"));
    BoolOption(current_page, 0, "effects.accounting.enabled", UserString("OPTIONS_EFFECT_ACCOUNTING"));

    // Create persistent config button
    auto persistent_config_button = GG::Wnd::Create<CUIButton>(UserString("OPTIONS_CREATE_PERSISTENT_CONFIG"));
    persistent_config_button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    persistent_config_button->SetBrowseInfoWnd(
        GG::Wnd::Create<TextBrowseWnd>(UserString("OPTIONS_CREATE_PERSISTENT_CONFIG_TOOLTIP_TITLE"),
                                       UserString("OPTIONS_CREATE_PERSISTENT_CONFIG_TOOLTIP_DESC"), ROW_WIDTH));
    persistent_config_button->LeftClickedSignal.connect([]() {
        if (GetOptionsDB().CommitPersistent())
            ClientUI::MessageBox(UserString("OPTIONS_CREATE_PERSISTENT_CONFIG_SUCCESS"));
        else
            ClientUI::MessageBox(UserString("OPTIONS_CREATE_PERSISTENT_CONFIG_FAILURE"));
    });
    current_page->Insert(GG::Wnd::Create<OptionsListRow>(ROW_WIDTH,
                                                         persistent_config_button->MinUsableSize().y + LAYOUT_MARGIN + 6,
                                                         persistent_config_button, 0));
    m_tabs->SetCurrentWnd(0);

    DoLayout();
    SaveDefaultedOptions();
    SaveOptions();

    // Connect the done and cancel button
    m_done_button->LeftClickedSignal.connect(
        boost::bind(&OptionsWnd::DoneClicked, this));
}

void OptionsWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void OptionsWnd::DoLayout() {
    const GG::X BUTTON_WIDTH(75);
    const GG::Y BUTTON_HEIGHT(ClientUI::GetFont()->Lineskip() + 6);

    GG::Pt done_button_lr = ScreenToClient(ClientLowerRight()) - GG::Pt(GG::X(LAYOUT_MARGIN), GG::Y(LAYOUT_MARGIN));
    GG::Pt done_button_ul = done_button_lr - GG::Pt(BUTTON_WIDTH, BUTTON_HEIGHT);

    m_done_button->SizeMove(done_button_ul, done_button_lr);

    GG::Pt tabs_lr = ScreenToClient(ClientLowerRight()) - GG::Pt(GG::X(LAYOUT_MARGIN), GG::Y(LAYOUT_MARGIN + BUTTON_HEIGHT + LAYOUT_MARGIN));
    m_tabs->SizeMove(GG::Pt(GG::X(LAYOUT_MARGIN), GG::Y(LAYOUT_MARGIN)), tabs_lr);
}

GG::Rect OptionsWnd::CalculatePosition() const {
    GG::Pt ul((GG::GUI::GetGUI()->AppWidth() - (PAGE_WIDTH + 20)) / 2,
              (GG::GUI::GetGUI()->AppHeight() - (PAGE_HEIGHT + 70)) / 2);
    GG::Pt wh(PAGE_WIDTH + 20, PAGE_HEIGHT + 70);
    return GG::Rect(ul, ul + wh);
}

GG::ListBox* OptionsWnd::CreatePage(const std::string& name) {
    auto page = GG::Wnd::Create<OptionsList>();
    m_tabs->AddWnd(page, name);
    m_tabs->SetCurrentWnd(m_tabs->NumWnds() - 1);
    return page.get();
}

void OptionsWnd::CreateSectionHeader(GG::ListBox* page, int indentation_level,
                                     const std::string& name, const std::string& tooltip)
{
    assert(0 <= indentation_level);
    auto heading_text = GG::Wnd::Create<CUILabel>(name, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
    heading_text->SetFont(ClientUI::GetFont(ClientUI::Pts() * 4 / 3));

    auto row = GG::Wnd::Create<OptionsListRow>(ROW_WIDTH, heading_text->MinUsableSize().y + LAYOUT_MARGIN + 6,
                                               heading_text, indentation_level);

    if (!tooltip.empty()) {
        row->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
        row->SetBrowseText(tooltip);
    }

    page->Insert(row);
}

GG::StateButton* OptionsWnd::BoolOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    auto button = GG::Wnd::Create<CUIStateButton>(text, GG::FORMAT_LEFT, std::make_shared<CUICheckBoxRepresenter>());
    auto row = GG::Wnd::Create<OptionsListRow>(ROW_WIDTH, button->MinUsableSize().y + LAYOUT_MARGIN + 6,
                                               button, indentation_level);
    page->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    button->SetCheck(GetOptionsDB().Get<bool>(option_name));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    button->CheckedSignal.connect(
        [option_name](const bool& value){ GetOptionsDB().Set(option_name, value); });
    return button.get();
}

namespace {
    void HandleSetHotkeyOption(const std::string & hk_name, GG::Button* button) {
        std::pair<GG::Key, GG::Flags<GG::ModKey>> kp = KeyPressCatcher::GetKeypress();

        // abort of escape was pressed...
        if (kp.first == GG::GGK_ESCAPE)
            return;

        // check if pressed key is different from existing setting...
        const Hotkey& hotkey = Hotkey::NamedHotkey(hk_name);
        if (hotkey.m_key == kp.first && hotkey.m_mod_keys == kp.second)
            return; // nothing to change


        // set hotkey to new pressed key / modkey combination
        Hotkey::SetHotkey(hotkey, kp.first, kp.second);

        // indicate new hotkey on button
        button->SetText(Hotkey::NamedHotkey(hk_name).PrettyPrint());

        // update shortcuts for new hotkey
        HotkeyManager::GetManager()->RebuildShortcuts();
    }

    void HandleResetHotkeyOption(const std::string & hk_name, GG::Button* button) {
        const Hotkey& hotkey = Hotkey::NamedHotkey(hk_name);
        if (hotkey.IsDefault())
            hotkey.ClearHotkey(hotkey);
        else
            hotkey.ResetHotkey(hotkey);

        // indicate new hotkey on button
        button->SetText(Hotkey::NamedHotkey(hk_name).PrettyPrint());

        // update shortcuts for new hotkey
        HotkeyManager::GetManager()->RebuildShortcuts();
    }
}

void OptionsWnd::HotkeyOption(GG::ListBox* page, int indentation_level, const std::string& hotkey_name) {
    const Hotkey & hk = Hotkey::NamedHotkey(hotkey_name);
    std::string text = UserString(hk.GetDescription());
    auto text_control = GG::Wnd::Create<CUILabel>(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    auto button = Wnd::Create<CUIButton>(hk.PrettyPrint());

    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, ROW_WIDTH, std::max(button->MinUsableSize().y, text_control->MinUsableSize().y),
                                              1, 2, 0, 5);
    layout->Add(text_control,   0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(button,         0, 1, GG::ALIGN_VCENTER | GG::ALIGN_RIGHT);

    auto row = GG::Wnd::Create<OptionsListRow>(ROW_WIDTH, std::max(button->MinUsableSize().y, text_control->MinUsableSize().y) + 6,
                                               layout, indentation_level);

    button->LeftClickedSignal.connect(
        boost::bind(HandleSetHotkeyOption, hotkey_name, button.get()));
    button->RightClickedSignal.connect(
        boost::bind(HandleResetHotkeyOption, hotkey_name, button.get()));

    page->Insert(row);
}

GG::Spin<int>* OptionsWnd::IntOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    auto text_control = GG::Wnd::Create<CUILabel>(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    std::shared_ptr<const ValidatorBase> validator = GetOptionsDB().GetValidator(option_name);
    std::shared_ptr<GG::Spin<int>> spin;
    int value = GetOptionsDB().Get<int>(option_name);
    if (std::shared_ptr<const RangedValidator<int>> ranged_validator = std::dynamic_pointer_cast<const RangedValidator<int>>(validator))
        spin = GG::Wnd::Create<CUISpin<int>>(value, 1, ranged_validator->m_min, ranged_validator->m_max, true);
    else if (std::shared_ptr<const StepValidator<int>> step_validator = std::dynamic_pointer_cast<const StepValidator<int>>(validator))
        spin = GG::Wnd::Create<CUISpin<int>>(value, step_validator->m_step_size, -1000000, 1000000, true);
    else if (std::shared_ptr<const RangedStepValidator<int>> ranged_step_validator = std::dynamic_pointer_cast<const RangedStepValidator<int>>(validator))
        spin = GG::Wnd::Create<CUISpin<int>>(value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);
    else if (std::shared_ptr<const Validator<int>> int_validator = std::dynamic_pointer_cast<const Validator<int>>(validator))
        spin = GG::Wnd::Create<CUISpin<int>>(value, 1, -1000000, 1000000, true);
    if (!spin) {
        ErrorLogger() << "Unable to create IntOption spin";
        return nullptr;
    }
    spin->Resize(GG::Pt(SPIN_WIDTH, spin->MinUsableSize().y));
    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, ROW_WIDTH, spin->MinUsableSize().y, 1, 2, 0, 5);
    layout->Add(spin, 0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(text_control, 0, 1, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    layout->SetChildClippingMode(ClipToClient);

    auto row = GG::Wnd::Create<OptionsListRow>(ROW_WIDTH, spin->MinUsableSize().y, layout, indentation_level);
    page->Insert(row);

    spin->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    spin->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    spin->ValueChangedSignal.connect(
        [option_name](const int& new_value){ GetOptionsDB().Set(option_name, new_value); });
    return spin.get();
}

GG::Spin<double>* OptionsWnd::DoubleOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    auto text_control = GG::Wnd::Create<CUILabel>(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    std::shared_ptr<const ValidatorBase> validator = GetOptionsDB().GetValidator(option_name);
    std::shared_ptr<GG::Spin<double>> spin;
    double value = GetOptionsDB().Get<double>(option_name);
    if (std::shared_ptr<const RangedValidator<double>> ranged_validator = std::dynamic_pointer_cast<const RangedValidator<double>>(validator))
        spin = GG::Wnd::Create<CUISpin<double>>(value, 1, ranged_validator->m_min, ranged_validator->m_max, true);
    else if (std::shared_ptr<const StepValidator<double>> step_validator = std::dynamic_pointer_cast<const StepValidator<double>>(validator))
        spin = GG::Wnd::Create<CUISpin<double>>(value, step_validator->m_step_size, -1000000, 1000000, true);
    else if (std::shared_ptr<const RangedStepValidator<double>> ranged_step_validator = std::dynamic_pointer_cast<const RangedStepValidator<double>>(validator))
        spin = GG::Wnd::Create<CUISpin<double>>(value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);
    else if (std::shared_ptr<const Validator<double>> double_validator = std::dynamic_pointer_cast<const Validator<double>>(validator))
        spin = GG::Wnd::Create<CUISpin<double>>(value, 1, -1000000, 1000000, true);
    if (!spin) {
        ErrorLogger() << "Unable to create DoubleOption spin";
        return nullptr;
    }
    spin->Resize(GG::Pt(SPIN_WIDTH, spin->MinUsableSize().y));
    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, ROW_WIDTH, spin->MinUsableSize().y, 1, 2, 0, 5);
    layout->Add(spin, 0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(text_control, 0, 1, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    layout->SetChildClippingMode(ClipToClient);

    auto row = GG::Wnd::Create<OptionsListRow>(ROW_WIDTH, spin->MinUsableSize().y, layout, indentation_level);
    page->Insert(row);

    spin->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    spin->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    spin->ValueChangedSignal.connect(
        [option_name](const double& new_value){ GetOptionsDB().Set(option_name, new_value); });
    return spin.get();
}

void OptionsWnd::MusicVolumeOption(GG::ListBox* page, int indentation_level, SoundOptionsFeedback &fb) {
    auto row = GG::Wnd::Create<GG::ListBox::Row>();
    auto button = GG::Wnd::Create<CUIStateButton>(UserString("OPTIONS_MUSIC"), GG::FORMAT_LEFT, std::make_shared<CUICheckBoxRepresenter>());
    button->Resize(button->MinUsableSize());
    button->SetCheck(GetOptionsDB().Get<bool>("audio.music.enabled"));
    std::shared_ptr<const RangedValidator<int>> validator = std::dynamic_pointer_cast<const RangedValidator<int>>(GetOptionsDB().GetValidator("audio.music.volume"));
    assert(validator);
    auto slider = GG::Wnd::Create<CUISlider<int>>(validator->m_min, validator->m_max, GG::HORIZONTAL);
    slider->SlideTo(GetOptionsDB().Get<int>("audio.music.volume"));
    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, 5);
    layout->Add(button, 0, 0);
    layout->Add(slider, 0, 1);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(button->MinUsableSize().y, slider->MinUsableSize().y) + 6));
    row->push_back(GG::Wnd::Create<RowContentsWnd>(row->Width(), row->Height(), layout, indentation_level));
    page->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription("audio.music.enabled")));
    slider->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    slider->SetBrowseText(UserString(GetOptionsDB().GetDescription("audio.music.volume")));
    button->CheckedSignal.connect(
        boost::bind(&OptionsWnd::SoundOptionsFeedback::MusicClicked, &fb, _1));
    slider->SlidSignal.connect(
        boost::bind(&OptionsWnd::SoundOptionsFeedback::MusicVolumeSlid, &fb, _1, _2, _3));
    fb.SetMusicButton(std::move(button));
}

void OptionsWnd::VolumeOption(GG::ListBox* page, int indentation_level, const std::string& toggle_option_name,
                              const std::string& volume_option_name, const std::string& text,
                              bool toggle_value, SoundOptionsFeedback &fb)
{
    auto row = GG::Wnd::Create<GG::ListBox::Row>();
    auto button = GG::Wnd::Create<CUIStateButton>(text, GG::FORMAT_LEFT, std::make_shared<CUICheckBoxRepresenter>());
    button->Resize(button->MinUsableSize());
    button->SetCheck(toggle_value);
    std::shared_ptr<const RangedValidator<int>> validator = std::dynamic_pointer_cast<const RangedValidator<int>>(GetOptionsDB().GetValidator(volume_option_name));
    assert(validator);
    auto slider = GG::Wnd::Create<CUISlider<int>>(validator->m_min, validator->m_max, GG::HORIZONTAL);
    slider->SlideTo(GetOptionsDB().Get<int>(volume_option_name));
    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, 5);
    layout->Add(button, 0, 0);
    layout->Add(slider, 0, 1);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(button->MinUsableSize().y, slider->MinUsableSize().y) + 6));
    row->push_back(GG::Wnd::Create<RowContentsWnd>(row->Width(), row->Height(), layout, indentation_level));
    page->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(toggle_option_name)));
    slider->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    slider->SetBrowseText(UserString(GetOptionsDB().GetDescription(volume_option_name)));
    button->CheckedSignal.connect(
        boost::bind(&OptionsWnd::SoundOptionsFeedback::SoundEffectsEnableClicked, &fb, _1));
    slider->SlidAndStoppedSignal.connect(
        boost::bind(&OptionsWnd::SoundOptionsFeedback::UISoundsVolumeSlid, &fb, _1, _2, _3));
    fb.SetEffectsButton(std::move(button));
}

void OptionsWnd::FileOptionImpl(GG::ListBox* page, int indentation_level, const std::string& option_name,
                                const std::string& text, const fs::path& path,
                                const std::vector<std::pair<std::string, std::string>>& filters,
                                std::function<bool (const std::string&)> string_validator,
                                bool directory, bool relative_path,
                                bool disabled)
{
    auto text_control = GG::Wnd::Create<CUILabel>(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    auto edit = GG::Wnd::Create<CUIEdit>(GetOptionsDB().Get<std::string>(option_name));
    edit->Resize(GG::Pt(50*SPIN_WIDTH, edit->Height())); // won't resize within layout bigger than its initial size, so giving a big initial size here
    auto button = Wnd::Create<CUIButton>("...");
    if (disabled) {
        edit->Disable();
        button->Disable();
    }

    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, ROW_WIDTH, button->MinUsableSize().y,
                                              1, 3, 0, 5);

    layout->Add(text_control,   0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(edit,           0, 1, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(button,         0, 2, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetMinimumColumnWidth(1, SPIN_WIDTH);
    layout->SetMinimumColumnWidth(2, button->Width());
    layout->SetColumnStretch(0, 0.5);
    layout->SetColumnStretch(1, 1.0);
    layout->SetColumnStretch(2, 0.0);

    auto row = GG::Wnd::Create<OptionsListRow>(ROW_WIDTH, layout->Height() + 6, layout, indentation_level);
    page->Insert(row);

    edit->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    edit->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    edit->EditedSignal.connect(
        [option_name, edit, string_validator](const std::string& str) {
            if (string_validator && !string_validator(str)) {
                edit->SetTextColor(GG::CLR_RED);
            } else {
                edit->SetTextColor(ClientUI::TextColor());
                GetOptionsDB().Set<std::string>(option_name, str);
            }
        }
    );
    button->LeftClickedSignal.connect(
        BrowseForPathButtonFunctor(path, filters, edit, directory, relative_path));
    if (string_validator && !string_validator(edit->Text()))
        edit->SetTextColor(GG::CLR_RED);
}

void OptionsWnd::FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path,
                            std::function<bool (const std::string&)> string_validator/* = 0*/)
{ FileOption(page, indentation_level, option_name, text, path, std::vector<std::pair<std::string, std::string>>(), string_validator); }

void OptionsWnd::FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path,
                            const std::pair<std::string, std::string>& filter, std::function<bool (const std::string&)> string_validator/* = 0*/)
{ FileOption(page, indentation_level, option_name, text, path, std::vector<std::pair<std::string, std::string>>(1, filter), string_validator); }

void OptionsWnd::FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path,
                            const std::vector<std::pair<std::string, std::string>>& filters, std::function<bool (const std::string&)> string_validator/* = 0*/)
{ FileOptionImpl(page, indentation_level, option_name, text, path, filters, string_validator, false, false, false); }

void OptionsWnd::SoundFileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    FileOption(page, indentation_level, option_name, text, ClientUI::SoundDir(),
               {UserString("OPTIONS_SOUND_FILE"), "*" + SOUND_FILE_SUFFIX}, ValidSoundFile);
}

void OptionsWnd::DirectoryOption(GG::ListBox* page, int indentation_level, const std::string& option_name,
                                 const std::string& text, const fs::path& path, bool disabled /*= false*/)
{
    FileOptionImpl(page, indentation_level, option_name, text, path, std::vector<std::pair<std::string, std::string>>(),
                   ValidDirectory, true, false, disabled);
}

void OptionsWnd::ColorOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    auto row = GG::Wnd::Create<GG::ListBox::Row>();
    auto text_control = GG::Wnd::Create<CUILabel>(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    auto color_selector = GG::Wnd::Create<ColorSelector>(GetOptionsDB().Get<GG::Clr>(option_name),
                                                         GetOptionsDB().GetDefault<GG::Clr>(option_name));
    color_selector->Resize(GG::Pt(color_selector->Width(), GG::Y(ClientUI::Pts() + 4)));
    color_selector->SetMaxSize(GG::Pt(color_selector->MaxSize().x, color_selector->Size().y));
    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2);
    layout->Add(text_control, 0, 0);
    layout->Add(color_selector, 0, 1, 1, 1, GG::ALIGN_VCENTER);
    layout->SetMinimumColumnWidth(1, COLOR_SELECTOR_WIDTH);
    layout->SetColumnStretch(0, 1.0);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(text_control->MinUsableSize().y, color_selector->MinUsableSize().y) + 6));
    row->push_back(GG::Wnd::Create<RowContentsWnd>(row->Width(), row->Height(), layout, indentation_level));
    page->Insert(row);
    color_selector->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    color_selector->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    color_selector->ColorChangedSignal.connect(
        [option_name](const GG::Clr& clr) { GetOptionsDB().Set<GG::Clr>(option_name, clr); });
}

void OptionsWnd::FontOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    FileOption(page, indentation_level, option_name, text, GetRootDataDir() / "default",
               {std::string(option_name), "*" + FONT_FILE_SUFFIX},
               &ValidFontFile);
}

void OptionsWnd::ResolutionOption(GG::ListBox* page, int indentation_level) {
    std::shared_ptr<const RangedValidator<int>> width_validator =
        std::dynamic_pointer_cast<const RangedValidator<int>>(
            GetOptionsDB().GetValidator("video.fullscreen.width"));
    std::shared_ptr<const RangedValidator<int>> height_validator =
        std::dynamic_pointer_cast<const RangedValidator<int>>(
            GetOptionsDB().GetValidator("video.fullscreen.height"));
    std::shared_ptr<const RangedValidator<int>> windowed_width_validator =
        std::dynamic_pointer_cast<const RangedValidator<int>>(
            GetOptionsDB().GetValidator("video.windowed.width"));
    std::shared_ptr<const RangedValidator<int>> windowed_height_validator =
        std::dynamic_pointer_cast<const RangedValidator<int>>(
            GetOptionsDB().GetValidator("video.windowed.height"));
    std::shared_ptr<const RangedValidator<int>> windowed_left_validator =
        std::dynamic_pointer_cast<const RangedValidator<int>>(
            GetOptionsDB().GetValidator("video.windowed.left"));
    std::shared_ptr<const RangedValidator<int>> windowed_top_validator =
        std::dynamic_pointer_cast<const RangedValidator<int>>(
            GetOptionsDB().GetValidator("video.windowed.top"));

    // compile list of resolutions available on this system

    std::vector<std::string> resolutions = GG::GUI::GetGUI()->GetSupportedResolutions();

    // find text representation of current fullscreen resolution selection
    int width = GetOptionsDB().Get<int>("video.fullscreen.width");
    int height = GetOptionsDB().Get<int>("video.fullscreen.height");
    std::string current_video_mode_str = boost::io::str(boost::format("%1% x %2%") % width % height);

    // find which index in list, if any, represents current fullscreen resolution selection
    int current_resolution_index = -1, loop_res_index = 0;
    for (const std::string& resolution : resolutions) {
        if (resolution == current_video_mode_str)
            current_resolution_index = loop_res_index;
        ++loop_res_index;
    }


    // drop list and label
    auto drop_list_label = GG::Wnd::Create<CUILabel>(UserString("OPTIONS_VIDEO_MODE"), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    drop_list_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    drop_list_label->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_LIST_DESCRIPTION"));

    auto drop_list = GG::Wnd::Create<CUIDropDownList>(6);
    drop_list->Resize(GG::Pt(drop_list->MinUsableSize().x, GG::Y(ClientUI::Pts() + 4)));
    drop_list->SetMaxSize(GG::Pt(drop_list->MaxSize().x, drop_list->Size().y));
    drop_list->SetStyle(GG::LIST_NOSORT);
    drop_list->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    drop_list->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_LIST_DESCRIPTION"));
    drop_list->SetOnlyMouseScrollWhenDropped(true);

    auto layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0, GG::X1, GG::Y1, 2, 1, 0, LAYOUT_MARGIN);
    layout->Add(drop_list_label, 0, 0);
    layout->Add(drop_list, 1, 0, 1, 1, GG::ALIGN_VCENTER);

    auto row = GG::Wnd::Create<GG::ListBox::Row>();
    row->Resize(GG::Pt(ROW_WIDTH, drop_list_label->MinUsableSize().y + LAYOUT_MARGIN + drop_list->MaxSize().y + 6));
    row->push_back(GG::Wnd::Create<RowContentsWnd>(row->Width(), row->Height(), layout, indentation_level));

    page->Insert(row);


    // selectable rows in video modes list box...
    for (const std::string& resolution : resolutions) {
        auto video_mode_row = GG::Wnd::Create<CUISimpleDropDownListRow>(resolution);
        video_mode_row->SetName(resolution);
        drop_list->Insert(video_mode_row);
    }

    if (drop_list->NumRows() >= 1 && current_resolution_index != -1)
        drop_list->Select(current_resolution_index);

    // fullscreen / windowed toggle
    BoolOption(page, indentation_level, "video.fullscreen.enabled", UserString("OPTIONS_FULLSCREEN"));
    // Fake mode change is not possible without the opengl frame buffer extension
    if (GG::SDLGUI::GetGUI()->FramebuffersAvailable()) {
        BoolOption(page, indentation_level, "video.fullscreen.fake.enabled", UserString("OPTIONS_FAKE_MODE_CHANGE"));
    } else {
        GetOptionsDB().Set<bool>("video.fullscreen.fake.enabled", false);
    }
    IntOption(page, indentation_level, "video.monitor.id", UserString("OPTIONS_FULLSCREEN_MONITOR_ID"));


    // customizable windowed width and height
    auto windowed_spinner_label = GG::Wnd::Create<CUILabel>(UserString("OPTIONS_VIDEO_MODE_WINDOWED"), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    windowed_spinner_label->SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
    windowed_spinner_label->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_WINDOWED_SPINNERS_DESCRIPTION"));

    row = GG::Wnd::Create<GG::ListBox::Row>();
    row->Resize(GG::Pt(ROW_WIDTH, windowed_spinner_label->MinUsableSize().y + LAYOUT_MARGIN + 6));
    row->push_back(GG::Wnd::Create<RowContentsWnd>(row->Width(), row->Height(), windowed_spinner_label, indentation_level));
    page->Insert(row);

    IntOption(page, indentation_level, "video.windowed.width",  UserString("OPTIONS_APP_WIDTH_WINDOWED"));
    IntOption(page, indentation_level, "video.windowed.height", UserString("OPTIONS_APP_HEIGHT_WINDOWED"));
    IntOption(page, indentation_level, "video.windowed.left", UserString("OPTIONS_APP_LEFT_WINDOWED"));
    IntOption(page, indentation_level, "video.windowed.top", UserString("OPTIONS_APP_TOP_WINDOWED"));

    // fps
    BoolOption(page, indentation_level, "video.fps.shown", UserString("OPTIONS_SHOW_FPS"));

    //GG::StateButton* limit_FPS_button = BoolOption(page, indentation_level, "video.fps.max.enabled", UserString("OPTIONS_LIMIT_FPS"));
    //GG::Spin<double>* max_fps_spin =
    DoubleOption(page, indentation_level,  "video.fps.max",    UserString("OPTIONS_MAX_FPS"));
    //limit_FPS_button->CheckedSignal, [max_fps_spin](bool checked) { max_fps_spin->Disable(!checked); });
    //limit_FPS_button->SetCheck(GetOptionsDB().Get<bool>("video.fps.max.enabled"));
    //limit_FPS_button->CheckedSignal(limit_FPS_button->Checked());

    //GG::StateButton* limit_FPS_nofocus_button = BoolOption(page, indentation_level, "video.fps.unfocused.enabled", UserString("OPTIONS_LIMIT_FPS_NO_FOCUS"));
    //GG::Spin<double>* max_fps_nofocus_spin =
    DoubleOption(page, indentation_level,  "video.fps.unfocused", UserString("OPTIONS_MAX_FPS_NO_FOCUS"));
    //limit_FPS_nofocus_button->SetCheck(GetOptionsDB().Get<bool>("video.fps.unfocused.enabled"));


    // apply button, sized to fit text
    auto apply_button = Wnd::Create<CUIButton>(UserString("OPTIONS_APPLY"));
    row = GG::Wnd::Create<OptionsListRow>(ROW_WIDTH, apply_button->MinUsableSize().y + LAYOUT_MARGIN + 6,
                                          apply_button, indentation_level);
    page->Insert(row);
    apply_button->LeftClickedSignal.connect(
        boost::bind(&HumanClientApp::Reinitialize, HumanClientApp::GetApp()));

    drop_list->SelChangedSignal.connect(
        [drop_list](GG::ListBox::iterator it) {
            if (it == drop_list->end())
                return;
            const auto& drop_list_row = *it;
            if (!drop_list_row)
                return;
            int w, h;
            namespace classic = boost::spirit::classic;
            classic::rule<> resolution_p = classic::int_p[classic::assign_a(w)] >> classic::str_p(" x ") >> classic::int_p[classic::assign_a(h)];
            classic::parse(drop_list_row->Name().c_str(), resolution_p);
            GetOptionsDB().Set<int>("video.fullscreen.width", w);
            GetOptionsDB().Set<int>("video.fullscreen.height", h);
        }
    );
}

namespace {
    std::string ValidSectionForHotkey(const std::string& hotkey_name) {
        std::string retval { "HOTKEYS_GENERAL" };

        std::string name { hotkey_name };
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);

        const std::regex dot { "\\.+" };
        const std::vector<std::string> nodes {
            std::sregex_token_iterator(name.cbegin(), name.cend(), dot, -1),
            std::sregex_token_iterator()
        };

        std::string current_node { "" };
        for (const auto& node : nodes) {
            if (current_node == name)
                break;

            current_node.append( "_" + node);
            if (UserStringExists("HOTKEYS" + current_node))
                retval = "HOTKEYS" + current_node;
        }

        return retval;
    }

    std::map<std::string, std::set<std::string>> HotkeysBySection() {
        std::map<std::string, std::set<std::string>> retval;
        for (const auto& entry : Hotkey::DefinedHotkeys()) {
            retval[ValidSectionForHotkey(entry)].insert(entry);
        }
        return retval;
    }
}

void OptionsWnd::HotkeysPage() {

    GG::ListBox* page = CreatePage(UserString("OPTIONS_PAGE_HOTKEYS"));
    for (const auto& class_hotkeys : HotkeysBySection()) {
        CreateSectionHeader(page, 0, UserString(class_hotkeys.first));
        for (const std::string& hotkey : class_hotkeys.second)
            HotkeyOption(page, 0, hotkey);
    }
    m_tabs->SetCurrentWnd(0);
}

OptionsWnd::~OptionsWnd()
{}

void OptionsWnd::KeyPress(GG::Key key, std::uint32_t key_code_point,
                          GG::Flags<GG::ModKey> mod_keys)
{
    if (key == GG::GGK_ESCAPE || key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) // Same behaviour as if "done" was pressed
        DoneClicked();
}

void OptionsWnd::DoneClicked() {
    GetOptionsDB().Commit();
    m_done = true;
}

void OptionsWnd::SoundOptionsFeedback::SoundEffectsEnableClicked(bool checked) {
    if (checked) {
        try {
            Sound::GetSound().Enable();
            GetOptionsDB().Set("audio.effects.enabled", true);
            Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.button.press.sound.path"), true);
        } catch (Sound::InitializationFailureException const &e) {
            SoundInitializationFailure(e);
        }
    } else {
        GetOptionsDB().Set("audio.effects.enabled", false);
        if (!GetOptionsDB().Get<bool>("audio.music.enabled"))
            Sound::GetSound().Disable();
    }
}

void OptionsWnd::SoundOptionsFeedback::MusicClicked(bool checked) {
    if (checked) {
        try {
            Sound::GetSound().Enable();
            GetOptionsDB().Set("audio.music.enabled", true);
            Sound::GetSound().PlayMusic(GetOptionsDB().Get<std::string>("audio.music.path"), -1);
        } catch (Sound::InitializationFailureException const &e) {
            SoundInitializationFailure(e);
        }
    } else {
        GetOptionsDB().Set("audio.music.enabled", false);
        Sound::GetSound().StopMusic();
        if (!GetOptionsDB().Get<bool>("audio.effects.enabled"))
            Sound::GetSound().Disable();
    }
}

void OptionsWnd::SoundOptionsFeedback::MusicVolumeSlid(int pos, int low, int high) const {
    GetOptionsDB().Set("audio.music.volume", pos);
    Sound::GetSound().SetMusicVolume(pos);
}

void OptionsWnd::SoundOptionsFeedback::UISoundsVolumeSlid(int pos, int low, int high) const {
    GetOptionsDB().Set("audio.effects.volume", pos);
    Sound::GetSound().SetUISoundsVolume(pos);
    Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.button.press.sound.path"), true);
}

void OptionsWnd::SoundOptionsFeedback::SetMusicButton(std::shared_ptr<GG::StateButton> button)
{ m_music_button = button; }

void OptionsWnd::SoundOptionsFeedback::SetEffectsButton(std::shared_ptr<GG::StateButton> button)
{ m_effects_button = button; }

void OptionsWnd::SoundOptionsFeedback::SoundInitializationFailure(Sound::InitializationFailureException const &e) {
    GetOptionsDB().Set("audio.effects.enabled", false);
    GetOptionsDB().Set("audio.music.enabled", false);
    if (m_effects_button)
        m_effects_button->SetCheck(false);
    if (m_music_button)
        m_music_button->SetCheck(false);
    ClientUI::MessageBox(UserString(e.what()), false);
}
