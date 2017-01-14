#include "OptionsWnd.h"

#include "../client/human/HumanClientApp.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../util/Logger.h"

#include "ClientUI.h"
#include "CUISpin.h"
#include "CUISlider.h"
#include "GraphicsSystem.h"
#include "Sound.h"
#include "Hotkeys.h"

#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/TabWnd.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/utf8/checked.h>

#include <boost/cast.hpp>
#include <boost/format.hpp>
#include <boost/spirit/include/classic.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/system/system_error.hpp>


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

    class PlaceholderWnd : public GG::Wnd {
    public:
        PlaceholderWnd(GG::X w, GG::Y h) : Wnd(GG::X0, GG::Y0, w, h, GG::NO_WND_FLAGS) {}
        virtual void Render() {}
    };

    class RowContentsWnd : public GG::Control {
    public:
        RowContentsWnd(GG::X w, GG::Y h, Wnd* contents, int indentation_level) :
            Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
            m_contents(contents)
        {
            if (!m_contents)
                return;
            AttachChild(m_contents);
            m_contents->MoveTo(GG::Pt(GG::X(indentation_level * INDENTATION), GG::Y0));
            DoLayout();
        }

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
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

        virtual void    Render() {
            //GG::FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_DARK_RED, GG::CLR_PINK, 1);
        }
    private:
        Wnd* m_contents;
    };

    struct BrowseForPathButtonFunctor {
        BrowseForPathButtonFunctor(const fs::path& path, const std::vector<std::pair<std::string, std::string> >& filters,
                                   GG::Edit* edit, bool directory, bool return_relative_path) :
            m_path(path),
            m_filters(filters),
            m_edit(edit),
            m_directory(directory),
            m_return_relative_path(return_relative_path)
        {}

        void operator()() {
            try {
                FileDlg dlg(m_path.string(), m_edit->Text(), false, false, m_filters);
                if (m_directory)
                    dlg.SelectDirectories(true);
                dlg.Run();
                if (!dlg.Result().empty()) {
                    fs::path path = m_return_relative_path ?
                        RelativePath(m_path, fs::path(*dlg.Result().begin())) :
                    fs::absolute(*dlg.Result().begin());
                    *m_edit << path.string();
                    m_edit->EditedSignal(m_edit->Text());
                }
            } catch (const std::exception& e) {
                ClientUI::MessageBox(e.what(), true);
            }
        }

        fs::path                                            m_path;
        std::vector<std::pair<std::string, std::string> >   m_filters;
        GG::Edit*                                           m_edit;
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

    template <class T>
    struct SetOptionFunctor
    {
        SetOptionFunctor(const std::string& option_name) : m_option_name(option_name) {}
        void operator()(const T& value) { GetOptionsDB().Set(m_option_name, value); }
        const std::string m_option_name;
    };

    template <>
    struct SetOptionFunctor<GG::Clr>
    {
        SetOptionFunctor(const std::string& option_name) : m_option_name(option_name) {}
        void operator()(const GG::Clr& clr) { GetOptionsDB().Set<StreamableColor>(m_option_name, clr); }
        const std::string m_option_name;
    };

    template <>
    struct SetOptionFunctor<std::string>
    {
        SetOptionFunctor(const std::string& option_name, GG::Edit* edit = 0, OptionsWnd::StringValidator string_validator = 0) :
            m_option_name(option_name), m_edit(edit), m_string_validator(string_validator)
        { assert(bool(m_edit) == bool(m_string_validator)); }

        void operator()(const std::string& str) {
            if (m_string_validator && !m_string_validator(str)) {
                if (m_edit)
                    m_edit->SetTextColor(GG::CLR_RED);
            } else {
                if (m_edit)
                    m_edit->SetTextColor(ClientUI::TextColor());
                GetOptionsDB().Set<std::string>(m_option_name, str);
            }
        }
        const std::string m_option_name;
        GG::Edit* m_edit;
        OptionsWnd::StringValidator m_string_validator;
    };

    struct ResolutionDropListIndexSetOptionFunctor {
        ResolutionDropListIndexSetOptionFunctor(GG::DropDownList* drop_list) :
            m_drop_list(drop_list)
        {}

        void operator()(GG::ListBox::iterator it) {
            const GG::ListBox::Row* row = *it;
            if (!m_drop_list || it == m_drop_list->end() || !row) {
                ErrorLogger() << "ResolutionDropListIndexSetOptionFunctor couldn't get row from passed ListBox iterator";
                return;
            }
            int w, h, bpp;
            using namespace boost::spirit::classic;
            rule<> resolution_p = int_p[assign_a(w)] >> str_p(" x ") >> int_p[assign_a(h)] >> str_p(" @ ") >> int_p[assign_a(bpp)];
            parse(row->Name().c_str(), resolution_p);
            GetOptionsDB().Set<int>("app-width", w);
            GetOptionsDB().Set<int>("app-height", h);
            GetOptionsDB().Set<int>("color-depth", bpp);
        }

        GG::DropDownList* m_drop_list;
    };

    struct LimitFPSSetOptionFunctor {
        LimitFPSSetOptionFunctor(GG::Spin<double>* max_fps_spin) :
            m_max_fps_spin(max_fps_spin)
        {}
        void operator()(bool b) {
            DebugLogger() << "LimitFPSSetOptionFunction: bool: " << b;
            m_max_fps_spin->Disable(!b);
        }
        GG::Spin<double>* m_max_fps_spin;
    };

    // Small window that will grab a unique key press.
    class KeyPressCatcher : public GG::Wnd {
        GG::Key                 m_key;
        boost::uint32_t         m_code_point;
        GG::Flags<GG::ModKey>   m_mods;

    public:
        KeyPressCatcher() :
            Wnd(GG::X0, GG::Y0, GG::X0, GG::Y0, GG::Flags<GG::WndFlag>(GG::MODAL))
        {};
        virtual void Render() {};

        virtual void KeyPress(GG::Key key, boost::uint32_t key_code_point,
                              GG::Flags<GG::ModKey> mod_keys)
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

        static std::pair<GG::Key, GG::Flags<GG::ModKey> > GetKeypress() {
            KeyPressCatcher ct;
            ct.Run();
            return std::make_pair(ct.m_key, ct.m_mods);
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
            m_font_graphic(0),
            m_title_font_graphic(0),
            m_hscroll(0)
        {
            GG::Y top = GG::Y1;

            boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
            boost::shared_ptr<GG::Texture> texture;
            if (font)
                texture = font->GetTexture();
            if (texture) {
                m_font_graphic = new GG::StaticGraphic(texture);
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
                m_title_font_graphic = new GG::StaticGraphic(texture);
                m_title_font_graphic->MoveTo(GG::Pt(GG::X0, top));
                m_title_font_graphic->Resize(GG::Pt(texture->Width(), texture->Height()));
                AttachChild(m_title_font_graphic);
            }


            m_hscroll = new CUIScroll(GG::HORIZONTAL);
            AttachChild(m_hscroll);

            GG::Connect(m_hscroll->ScrolledSignal, &FontTextureWnd::ScrolledSlot, this);
            DoLayout();
        }

        virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
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
        GG::StaticGraphic*  m_font_graphic;
        GG::StaticGraphic*  m_title_font_graphic;
        GG::Scroll*         m_hscroll;
    };

    void ShowFontTextureWnd() {
        FontTextureWnd* font_wnd = new FontTextureWnd();
        font_wnd->Run();
        delete font_wnd;
    }

    class OptionsListRow : public GG::ListBox::Row {
    public:
        OptionsListRow(GG::X w, GG::Y h, RowContentsWnd* contents) :
            GG::ListBox::Row(w, h, ""),
            m_contents(contents)
        {
            SetChildClippingMode(ClipToClient);
            if (m_contents)
                push_back(m_contents);
        }

        OptionsListRow(GG::X w, GG::Y h, Wnd* contents, int indentation = 0) :
            GG::ListBox::Row(w, h, ""),
            m_contents(0)
        {
            SetChildClippingMode(ClipToClient);
            if (contents) {
                m_contents = new RowContentsWnd(w, h, contents, indentation);
                push_back(m_contents);
            }
        }

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            //std::cout << "OptionsListRow::SizeMove(" << ul << ", " << lr << ")" << std::endl;
            const GG::Pt old_size = Size();
            GG::ListBox::Row::SizeMove(ul, lr);
            if (!empty() && old_size != Size() && m_contents)
                m_contents->Resize(Size());
        }

        virtual void    Render() {
            //GG::FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_DARK_BLUE, GG::CLR_YELLOW, 1);
        }
    private:
        RowContentsWnd* m_contents;
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

        virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
            const GG::Pt old_size = Size();
            CUIListBox::SizeMove(ul, lr);
            if (old_size != Size()) {
                const GG::X row_width = ListRowWidth();
                for (GG::ListBox::Row* row : *this)
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
}

OptionsWnd::OptionsWnd():
    CUIWnd(UserString("OPTIONS_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE,
           OPTIONS_WND_NAME),
    m_tabs(0),
    m_done_button(0)
{
    m_done_button = new CUIButton(UserString("DONE"));
    // FIXME: PAGE_WIDTH is needed to prevent triggering an assert within the TabBar class.
    // The placement of the tab register buttons assumes that the whole TabWnd is at least
    // wider than the first tab button.
    m_tabs = new GG::TabWnd(GG::X0, GG::Y0, PAGE_WIDTH, GG::Y1, ClientUI::GetFont(), ClientUI::WndColor(), ClientUI::TextColor());

    ResetDefaultPosition();
    SetMinSize(GG::Pt(PAGE_WIDTH + 20, PAGE_HEIGHT + 70));

    AttachChild(m_done_button);
    AttachChild(m_tabs);

    bool UI_sound_enabled = GetOptionsDB().Get<bool>("UI.sound.enabled");
    GG::ListBox* current_page = 0;

    Sound::TempUISoundDisabler sound_disabler;

    // Video settings tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_VIDEO"));
    ResolutionOption(current_page, 0);
    m_tabs->SetCurrentWnd(0);

    // Audio settings tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_AUDIO"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_VOLUME_AND_MUSIC"));
    MusicVolumeOption(current_page, 0, m_sound_feedback);
    VolumeOption(current_page, 0, "UI.sound.enabled", "UI.sound.volume", UserString("OPTIONS_UI_SOUNDS"), UI_sound_enabled, m_sound_feedback);
    FileOption(current_page, 0, "UI.sound.bg-music", UserString("OPTIONS_BACKGROUND_MUSIC"), ClientUI::SoundDir(),
               std::make_pair(UserString("OPTIONS_MUSIC_FILE"), "*" + MUSIC_FILE_SUFFIX),
               ValidMusicFile);

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_SOUNDS"));
    CreateSectionHeader(current_page, 1, UserString("OPTIONS_UI_SOUNDS"));
    SoundFileOption(current_page, 1, "UI.sound.alert",       UserString("OPTIONS_SOUND_ALERT"));
    SoundFileOption(current_page, 1, "UI.sound.text-typing", UserString("OPTIONS_SOUND_TYPING"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_SOUND_NEWTURN"));
    BoolOption(current_page,      1, "UI.sound.new-turn.toggle",       UserString("OPTIONS_SOUND_NEWTURN_TOGGLE"));
    SoundFileOption(current_page, 1, "UI.sound.new-turn.sound-file",   UserString("OPTIONS_SOUND_NEWTURN_FILE"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_SOUND_WINDOW"));
    SoundFileOption(current_page, 1, "UI.sound.window-close",    UserString("OPTIONS_SOUND_CLOSE"));
    SoundFileOption(current_page, 1, "UI.sound.window-maximize", UserString("OPTIONS_SOUND_MAXIMIZE"));
    SoundFileOption(current_page, 1, "UI.sound.window-minimize", UserString("OPTIONS_SOUND_MINIMIZE"));
    SoundFileOption(current_page, 1, "UI.sound.sidepanel-open",  UserString("OPTIONS_SOUND_SIDEPANEL"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_SOUND_LIST"));
    SoundFileOption(current_page, 1, "UI.sound.item-drop",     UserString("OPTIONS_SOUND_DROP"));
    SoundFileOption(current_page, 1, "UI.sound.list-pulldown", UserString("OPTIONS_SOUND_PULLDOWN"));
    SoundFileOption(current_page, 1, "UI.sound.list-select",   UserString("OPTIONS_SOUND_SELECT"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_SOUND_BUTTON"));
    SoundFileOption(current_page, 1, "UI.sound.button-click",          UserString("OPTIONS_SOUND_CLICK"));
    SoundFileOption(current_page, 1, "UI.sound.button-rollover",       UserString("OPTIONS_SOUND_ROLLOVER"));
    SoundFileOption(current_page, 1, "UI.sound.fleet-button-click",    UserString("OPTIONS_SOUND_FLEET_CLICK"));
    SoundFileOption(current_page, 1, "UI.sound.fleet-button-rollover", UserString("OPTIONS_SOUND_FLEET_ROLLOVER"));
    SoundFileOption(current_page, 1, "UI.sound.system-icon-rollover",  UserString("OPTIONS_SOUND_SYSTEM_ROLLOVER"));
    SoundFileOption(current_page, 1, "UI.sound.turn-button-click",     UserString("OPTIONS_SOUND_TURN"));
    SoundFileOption(current_page, 1, "UI.sound.planet-button-click",   UserString("OPTIONS_SOUND_PLANET"));

    m_tabs->SetCurrentWnd(0);

    // UI settings tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_UI"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_MISC_UI"));
    BoolOption(current_page, 0, "UI.swap-mouse-lr",              UserString("OPTIONS_SWAP_MOUSE_LR"));
    BoolOption(current_page, 0, "UI.multiple-fleet-windows",     UserString("OPTIONS_MULTIPLE_FLEET_WNDS"));
    BoolOption(current_page, 0, "UI.window-quickclose",          UserString("OPTIONS_QUICK_CLOSE_WNDS"));
    BoolOption(current_page, 0, "UI.sidepanel-planet-shown",     UserString("OPTIONS_SHOW_SIDEPANEL_PLANETS"));
    BoolOption(current_page, 0, "UI.auto-reposition-windows",    UserString("OPTIONS_AUTO_REPOSITION_WINDOWS"));

    // manual reposition windows button
    GG::Button* window_reset_button = new CUIButton(UserString("OPTIONS_WINDOW_RESET"));
    GG::ListBox::Row* row = new OptionsListRow(ROW_WIDTH, window_reset_button->MinUsableSize().y + LAYOUT_MARGIN + 6,
                                               window_reset_button, 0);
    current_page->Insert(row);
    GG::Connect(window_reset_button->LeftClickedSignal, HumanClientApp::GetApp()->RepositionWindowsSignal);

    FileOption(current_page, 0, "stringtable-filename",          UserString("OPTIONS_LANGUAGE"),
               GetRootDataDir() / "default" / "stringtables",
               std::make_pair(UserString("OPTIONS_LANGUAGE_FILE"),
               "*" + STRINGTABLE_FILE_SUFFIX),
               &ValidStringtableFile);

    // flush stringtable button
    GG::Button* flush_button = new CUIButton(UserString("OPTIONS_FLUSH_STRINGTABLE"));
    row = new OptionsListRow(ROW_WIDTH, flush_button->MinUsableSize().y + LAYOUT_MARGIN + 6, flush_button, 0);
    current_page->Insert(row);
    GG::Connect(flush_button->LeftClickedSignal, &FlushLoadedStringTables);

    IntOption(current_page, 0, "UI.tooltip-delay",               UserString("OPTIONS_TOOLTIP_DELAY"));
    IntOption(current_page, 0, "UI.keypress-repeat-delay",       UserString("OPTIONS_KEYPRESS_REPEAT_DELAY"));
    IntOption(current_page, 0, "UI.keypress-repeat-interval",    UserString("OPTIONS_KEYPRESS_REPEAT_INTERVAL"));
    IntOption(current_page, 0, "UI.mouse-click-repeat-delay",    UserString("OPTIONS_MOUSE_REPEAT_DELAY"));
    IntOption(current_page, 0, "UI.mouse-click-repeat-interval", UserString("OPTIONS_MOUSE_REPEAT_INTERVAL"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_FONTS"));
    FontOption(current_page, 0, "UI.font",       UserString("OPTIONS_FONT_TEXT"));
    FontOption(current_page, 0, "UI.title-font", UserString("OPTIONS_FONT_TITLE"));

    // show font texture button
    GG::Button* show_font_texture_button = new CUIButton(UserString("SHOW_FONT_TEXTURES"));
    row = new OptionsListRow(ROW_WIDTH, show_font_texture_button ->MinUsableSize().y + LAYOUT_MARGIN + 6, show_font_texture_button , 0);
    current_page->Insert(row);
    GG::Connect(show_font_texture_button->LeftClickedSignal, &ShowFontTextureWnd);

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_FONT_SIZES"));
    IntOption(current_page,    0, "UI.font-size",                          UserString("OPTIONS_FONT_TEXT"));
    IntOption(current_page,    0, "UI.title-font-size",                    UserString("OPTIONS_FONT_TITLE"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_RESEARCH_WND"));
    DoubleOption(current_page, 0, "UI.tech-layout-horz-spacing",           UserString("OPTIONS_TECH_SPACING_HORIZONTAL"));
    DoubleOption(current_page, 0, "UI.tech-layout-vert-spacing",           UserString("OPTIONS_TECH_SPACING_VERTICAL"));
    DoubleOption(current_page, 0, "UI.tech-layout-zoom-scale",             UserString("OPTIONS_TECH_LAYOUT_ZOOM"));
    DoubleOption(current_page, 0, "UI.tech-controls-graphic-size",         UserString("OPTIONS_TECH_CTRL_ICON_SIZE"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_QUEUES"));
    IntOption(current_page,    0, "UI.queue-width",                        UserString("OPTIONS_UI_QUEUE_WIDTH"));
    BoolOption(current_page,   0, "UI.show-production-location-on-queue",  UserString("OPTIONS_UI_PROD_QUEUE_LOCATION"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_DESCRIPTIONS"));
    BoolOption(current_page,   0, "UI.dump-effects-descriptions",          UserString("OPTIONS_DUMP_EFFECTS_GROUPS_DESC"));
    BoolOption(current_page,   0, "verbose-logging",                       UserString("OPTIONS_VERBOSE_LOGGING_DESC"));
    BoolOption(current_page,   0, "verbose-sitrep",                        UserString("OPTIONS_VERBOSE_SITREP_DESC"));

    m_tabs->SetCurrentWnd(0);

    // Galaxy Map Page
    current_page = CreatePage(UserString("OPTIONS_GALAXY_MAP"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_SYSTEM_ICONS"));
    IntOption(current_page,    0, "UI.system-icon-size",                   UserString("OPTIONS_UI_SYSTEM_ICON_SIZE"));
    BoolOption(current_page,   0, "UI.system-circles",                     UserString("OPTIONS_UI_SYSTEM_CIRCLES"));
    DoubleOption(current_page, 0, "UI.system-circle-size",                 UserString("OPTIONS_UI_SYSTEM_CIRCLE_SIZE"));
    DoubleOption(current_page, 0, "UI.system-selection-indicator-size",    UserString("OPTIONS_UI_SYSTEM_SELECTION_INDICATOR_SIZE"));
    IntOption(current_page,    0, "UI.system-selection-indicator-rpm",     UserString("OPTIONS_UI_SYSTEM_SELECTION_INDICATOR_FPS"));
    IntOption(current_page,    0, "UI.system-tiny-icon-size-threshold",    UserString("OPTIONS_UI_SYSTEM_TINY_ICON_SIZE_THRESHOLD"));
    ColorOption(current_page,  0, "UI.system-name-unowned-color",          UserString("OPTIONS_UI_SYSTEM_NAME_UNOWNED_COLOR"));
    BoolOption(current_page,   0, "UI.system-fog-of-war",                  UserString("OPTIONS_UI_SYSTEM_FOG"));
    DoubleOption(current_page, 0, "UI.system-fog-of-war-spacing",          UserString("OPTIONS_UI_SYSTEM_FOG_SPACING"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_FLEET_ICONS"));
    DoubleOption(current_page, 0, "UI.tiny-fleet-button-minimum-zoom",     UserString("OPTIONS_UI_TINY_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption(current_page, 0, "UI.small-fleet-button-minimum-zoom",    UserString("OPTIONS_UI_SMALL_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption(current_page, 0, "UI.medium-fleet-button-minimum-zoom",   UserString("OPTIONS_UI_MEDIUM_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption(current_page, 0, "UI.fleet-selection-indicator-size",     UserString("OPTIONS_UI_FLEET_SELECTION_INDICATOR_SIZE"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_STARLANES"));
    DoubleOption(current_page, 0, "UI.starlane-thickness",                 UserString("OPTIONS_STARLANE_THICKNESS"));
    BoolOption(current_page,   0, "UI.resource-starlane-colouring",        UserString("OPTIONS_RESOURCE_STARLANE_COLOURING"));
    DoubleOption(current_page, 0, "UI.starlane-core-multiplier",           UserString("OPTIONS_DB_STARLANE_CORE"));
    BoolOption(current_page,   0, "UI.fleet-supply-lines",                 UserString("OPTIONS_FLEET_SUPPLY_LINES"));
    DoubleOption(current_page, 0, "UI.fleet-supply-line-width",            UserString("OPTIONS_FLEET_SUPPLY_LINE_WIDTH"));
    IntOption(current_page,    0, "UI.fleet-supply-line-dot-spacing",      UserString("OPTIONS_FLEET_SUPPLY_LINE_DOT_SPACING"));
    DoubleOption(current_page, 0, "UI.fleet-supply-line-dot-rate",         UserString("OPTIONS_FLEET_SUPPLY_LINE_DOT_RATE"));
    ColorOption(current_page,  0, "UI.unowned-starlane-colour",            UserString("OPTIONS_UNOWNED_STARLANE_COLOUR"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_GALAXY_MAP_GENERAL"));
    BoolOption(current_page,   0, "UI.galaxy-gas-background",              UserString("OPTIONS_GALAXY_MAP_GAS"));
    BoolOption(current_page,   0, "UI.galaxy-starfields",                  UserString("OPTIONS_GALAXY_MAP_STARFIELDS"));
    BoolOption(current_page,   0, "UI.show-galaxy-map-scale",              UserString("OPTIONS_GALAXY_MAP_SCALE_LINE"));
    BoolOption(current_page,   0, "UI.show-galaxy-map-scale-circle",       UserString("OPTIONS_GALAXY_MAP_SCALE_CIRCLE"));
    BoolOption(current_page,   0, "UI.show-galaxy-map-zoom-slider",        UserString("OPTIONS_GALAXY_MAP_ZOOM_SLIDER"));
    BoolOption(current_page,   0, "UI.show-detection-range",               UserString("OPTIONS_GALAXY_MAP_DETECTION_RANGE"));
    IntOption(current_page,    0, "UI.detection-range-opacity",            UserString("OPTIONS_GALAXY_MAP_DETECTION_RANGE_OPACITY"));
    BoolOption(current_page,   0, "UI.map-right-click-popup-menu",         UserString("OPTIONS_GALAXY_MAP_POPUP"));
    BoolOption(current_page,   0, "UI.show-unexplored_system_overlay",     UserString("OPTIONS_UI_SYSTEM_UNEXPLORED_OVERLAY"));
    BoolOption(current_page,   0, "UI.hide-map-panels",                    UserString("OPTIONS_UI_HIDE_MAP_PANELS"));

    m_tabs->SetCurrentWnd(0);

    // Objects List Page
    current_page = CreatePage(UserString("OPTIONS_PAGE_OBJECTS_WINDOW"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_COLUMNS"));
    for (unsigned int i = 0; i < 12u; ++i) {
        std::string col_width_opt_name = "UI.objects-list-width-col-" + boost::lexical_cast<std::string>(i);
        if (!GetOptionsDB().OptionExists(col_width_opt_name))
            break;
        std::string col_opt_name = "UI.objects-list-info-col-" + boost::lexical_cast<std::string>(i);
        if (!GetOptionsDB().OptionExists(col_opt_name))
            break;
        std::string col_contents = GetOptionsDB().GetValueString(col_opt_name);
        const std::string& tx_contents = (col_contents.empty() ? "" : UserString(col_contents));

        IntOption(current_page, 0, col_width_opt_name,                      tx_contents);
    }

    m_tabs->SetCurrentWnd(0);

    // Colors tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_COLORS"));
    CreateSectionHeader(current_page, 0, UserString("OPTIONS_GENERAL_COLORS"));
    ColorOption(current_page, 0, "UI.text-color",          UserString("OPTIONS_TEXT_COLOR"));
    ColorOption(current_page, 0, "UI.default-link-color",  UserString("OPTIONS_DEFAULT_LINK_COLOR"));
    ColorOption(current_page, 0, "UI.rollover-link-color", UserString("OPTIONS_ROLLOVER_LINK_COLOR"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_WINDOW_COLORS"));
    ColorOption(current_page, 0, "UI.wnd-color",              UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 0, "UI.wnd-inner-border-color", UserString("OPTIONS_INNER_BORDER_COLOR"));
    ColorOption(current_page, 0, "UI.wnd-outer-border-color", UserString("OPTIONS_OUTER_BORDER_COLOR"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_CONTROL_COLORS"));
    ColorOption(current_page, 0, "UI.ctrl-color",               UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 0, "UI.ctrl-border-color",        UserString("OPTIONS_BORDER_COLOR"));
    ColorOption(current_page, 0, "UI.edit-hilite",              UserString("OPTIONS_HIGHLIGHT_COLOR"));
    ColorOption(current_page, 0, "UI.dropdownlist-arrow-color", UserString("OPTIONS_DROPLIST_ARROW_COLOR"));
    ColorOption(current_page, 0, "UI.state-button-color",       UserString("OPTIONS_STATE_BUTTON_COLOR"));
    ColorOption(current_page, 0, "UI.stat-increase-color",      UserString("OPTIONS_STAT_INCREASE_COLOR"));
    ColorOption(current_page, 0, "UI.stat-decrease-color",      UserString("OPTIONS_STAT_DECREASE_COLOR"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_COMBAT_COLORS"));
    ColorOption(current_page, 0, "UI.combat.summary.dead-color", UserString("OPTIONS_COMBAT_SUMMARY_DEAD_COLOR"));
    ColorOption(current_page, 0, "UI.combat.summary.wound-color", UserString("OPTIONS_COMBAT_SUMMARY_WOUND_COLOR"));
    ColorOption(current_page, 0, "UI.combat.summary.health-color", UserString("OPTIONS_COMBAT_SUMMARY_HEALTH_COLOR"));

    CreateSectionHeader(current_page, 0, UserString("OPTIONS_TECH_COLORS"));
    CreateSectionHeader(current_page, 1, UserString("OPTIONS_KNOWN_TECH_COLORS"));
    ColorOption(current_page, 1, "UI.known-tech",        UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 1, "UI.known-tech-border", UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_RESEARCHABLE_TECH_COLORS"));
    ColorOption(current_page, 1, "UI.researchable-tech",        UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 1, "UI.researchable-tech-border", UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_UNRESEARCHABLE_TECH_COLORS"));
    ColorOption(current_page, 1, "UI.unresearchable-tech",        UserString("OPTIONS_FILL_COLOR"));
    ColorOption(current_page, 1, "UI.unresearchable-tech-border", UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));

    CreateSectionHeader(current_page, 1, UserString("OPTIONS_TECH_PROGRESS_COLORS"));
    ColorOption(current_page, 1, "UI.tech-progress",            UserString("OPTIONS_PROGRESS_BAR_COLOR"));
    ColorOption(current_page, 1, "UI.tech-progress-background", UserString("OPTIONS_PROGRESS_BACKGROUND_COLOR"));
    m_tabs->SetCurrentWnd(0);

    // Ausosave settings tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_AUTOSAVE"));
    BoolOption(current_page, 0, "autosave.single-player", UserString("OPTIONS_SINGLEPLAYER"));
    BoolOption(current_page, 0, "autosave.multiplayer",   UserString("OPTIONS_MULTIPLAYER"));
    IntOption(current_page,  0, "autosave.turns",         UserString("OPTIONS_AUTOSAVE_TURNS_BETWEEN"));
    IntOption(current_page,  0, "autosave.limit",         UserString("OPTIONS_AUTOSAVE_LIMIT"));
    m_tabs->SetCurrentWnd(0);

    // Keyboard shortcuts tab
    HotkeysPage();

    // Directories tab
    current_page = CreatePage(UserString("OPTIONS_PAGE_DIRECTORIES"));
    DirectoryOption(current_page, 0, "resource-dir", UserString("OPTIONS_FOLDER_SETTINGS"), GetRootDataDir());  // GetRootDataDir() returns the default browse path when modifying this directory option.  the actual default directory (before modifying) is gotten from the specified option name "resource-dir"
    DirectoryOption(current_page, 0, "save-dir",     UserString("OPTIONS_FOLDER_SAVE"),     GetUserDataDir());
    m_tabs->SetCurrentWnd(0);

    // Misc
    current_page = CreatePage(UserString("OPTIONS_PAGE_MISC"));
    IntOption(current_page, 0, "effects-threads-ui",        UserString("OPTIONS_EFFECTS_THREADS_UI"));
    IntOption(current_page, 0, "effects-threads-server",    UserString("OPTIONS_EFFECTS_THREADS_SERVER"));
    IntOption(current_page, 0, "effects-threads-ai",        UserString("OPTIONS_EFFECTS_THREADS_AI"));
    BoolOption(current_page, 0, "auto-add-saved-designs",   UserString("OPTIONS_ADD_SAVED_DESIGNS"));
    BoolOption(current_page, 0, "binary-serialization",     UserString("OPTIONS_USE_BINARY_SERIALIZATION"));  // Consider changing to Enum to support more serialization formats
    BoolOption(current_page, 0, "verbose-logging",          UserString("OPTIONS_VERBOSE_LOGGING_DESC"));
    BoolOption(current_page, 0, "verbose-sitrep",           UserString("OPTIONS_VERBOSE_SITREP_DESC"));
    BoolOption(current_page, 0, "effect-accounting",        UserString("OPTIONS_EFFECT_ACCOUNTING"));
    m_tabs->SetCurrentWnd(0);

    DoLayout();

    // Connect the done and cancel button
    GG::Connect(m_done_button->LeftClickedSignal, &OptionsWnd::DoneClicked, this);
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
    GG::ListBox* page = new OptionsList();
    m_tabs->AddWnd(page, name);
    m_tabs->SetCurrentWnd(m_tabs->NumWnds() - 1);
    return page;
}

void OptionsWnd::CreateSectionHeader(GG::ListBox* page, int indentation_level, const std::string& name) {
    assert(0 <= indentation_level);
    GG::Label* heading_text = new CUILabel(name, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP);
    heading_text->SetFont(ClientUI::GetFont(ClientUI::Pts() * 4 / 3));
    GG::ListBox::Row* row = new OptionsListRow(ROW_WIDTH, heading_text->MinUsableSize().y + LAYOUT_MARGIN + 6,
                                               heading_text, indentation_level);
    page->Insert(row);
}

GG::StateButton* OptionsWnd::BoolOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    GG::StateButton* button = new CUIStateButton(text, GG::FORMAT_LEFT, boost::make_shared<CUICheckBoxRepresenter>());
    GG::ListBox::Row* row = new OptionsListRow(ROW_WIDTH, button->MinUsableSize().y + LAYOUT_MARGIN + 6,
                                               button, indentation_level);
    page->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetCheck(GetOptionsDB().Get<bool>(option_name));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(button->CheckedSignal, SetOptionFunctor<bool>(option_name));
    return button;
}

namespace {
    void HandleSetHotkeyOption(const std::string & hk_name, GG::Button* button) {
        std::pair<GG::Key, GG::Flags<GG::ModKey> > kp = KeyPressCatcher::GetKeypress();

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
    GG::Label* text_control = new CUILabel(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    GG::Button* button = new CUIButton(hk.PrettyPrint());

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, ROW_WIDTH, std::max(button->MinUsableSize().y, text_control->MinUsableSize().y),
                                        1, 2, 0, 5);
    layout->Add(text_control,   0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(button,         0, 1, GG::ALIGN_VCENTER | GG::ALIGN_RIGHT);

    GG::ListBox::Row* row = new OptionsListRow(ROW_WIDTH, std::max(button->MinUsableSize().y, text_control->MinUsableSize().y) + 6,
                                               layout, indentation_level);

    GG::Connect(button->LeftClickedSignal, boost::bind(HandleSetHotkeyOption, hotkey_name, button));
    GG::Connect(button->RightClickedSignal, boost::bind(HandleResetHotkeyOption, hotkey_name, button));

    page->Insert(row);
}

GG::Spin<int>* OptionsWnd::IntOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    GG::Label* text_control = new CUILabel(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    boost::shared_ptr<const ValidatorBase> validator = GetOptionsDB().GetValidator(option_name);
    GG::Spin<int>* spin = 0;
    int value = GetOptionsDB().Get<int>(option_name);
    if (boost::shared_ptr<const RangedValidator<int> > ranged_validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(validator))
        spin = new CUISpin<int>(value, 1, ranged_validator->m_min, ranged_validator->m_max, true);
    else if (boost::shared_ptr<const StepValidator<int> > step_validator = boost::dynamic_pointer_cast<const StepValidator<int> >(validator))
        spin = new CUISpin<int>(value, step_validator->m_step_size, -1000000, 1000000, true);
    else if (boost::shared_ptr<const RangedStepValidator<int> > ranged_step_validator = boost::dynamic_pointer_cast<const RangedStepValidator<int> >(validator))
        spin = new CUISpin<int>(value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);
    else if (boost::shared_ptr<const Validator<int> > int_validator = boost::dynamic_pointer_cast<const Validator<int> >(validator))
        spin = new CUISpin<int>(value, 1, -1000000, 1000000, true);
    if (!spin) {
        ErrorLogger() << "Unable to create IntOption spin";
        return 0;
    }
    spin->Resize(GG::Pt(SPIN_WIDTH, spin->MinUsableSize().y));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, ROW_WIDTH, spin->MinUsableSize().y, 1, 2, 0, 5);
    layout->Add(spin, 0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(text_control, 0, 1, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    layout->SetChildClippingMode(ClipToClient);

    GG::ListBox::Row* row = new OptionsListRow(ROW_WIDTH, spin->MinUsableSize().y, layout, indentation_level);
    page->Insert(row);

    spin->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    spin->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(spin->ValueChangedSignal, SetOptionFunctor<int>(option_name));
    return spin;
}

GG::Spin<double>* OptionsWnd::DoubleOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    GG::Label* text_control = new CUILabel(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    boost::shared_ptr<const ValidatorBase> validator = GetOptionsDB().GetValidator(option_name);
    GG::Spin<double>* spin = 0;
    double value = GetOptionsDB().Get<double>(option_name);
    if (boost::shared_ptr<const RangedValidator<double> > ranged_validator = boost::dynamic_pointer_cast<const RangedValidator<double> >(validator))
        spin = new CUISpin<double>(value, 1, ranged_validator->m_min, ranged_validator->m_max, true);
    else if (boost::shared_ptr<const StepValidator<double> > step_validator = boost::dynamic_pointer_cast<const StepValidator<double> >(validator))
        spin = new CUISpin<double>(value, step_validator->m_step_size, -1000000, 1000000, true);
    else if (boost::shared_ptr<const RangedStepValidator<double> > ranged_step_validator = boost::dynamic_pointer_cast<const RangedStepValidator<double> >(validator))
        spin = new CUISpin<double>(value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);
    else if (boost::shared_ptr<const Validator<double> > double_validator = boost::dynamic_pointer_cast<const Validator<double> >(validator))
        spin = new CUISpin<double>(value, 1, -1000000, 1000000, true);
    if (!spin) {
        ErrorLogger() << "Unable to create DoubleOption spin";
        return 0;
    }
    spin->Resize(GG::Pt(SPIN_WIDTH, spin->MinUsableSize().y));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, ROW_WIDTH, spin->MinUsableSize().y, 1, 2, 0, 5);
    layout->Add(spin, 0, 0, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->Add(text_control, 0, 1, GG::ALIGN_VCENTER | GG::ALIGN_LEFT);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    layout->SetChildClippingMode(ClipToClient);

    GG::ListBox::Row* row = new OptionsListRow(ROW_WIDTH, spin->MinUsableSize().y, layout, indentation_level);
    page->Insert(row);

    spin->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    spin->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(spin->ValueChangedSignal, SetOptionFunctor<double>(option_name));
    return spin;
}

void OptionsWnd::MusicVolumeOption(GG::ListBox* page, int indentation_level, SoundOptionsFeedback &fb) {
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::StateButton* button = new CUIStateButton(UserString("OPTIONS_MUSIC"), GG::FORMAT_LEFT, boost::make_shared<CUICheckBoxRepresenter>());
    button->Resize(button->MinUsableSize());
    button->SetCheck(GetOptionsDB().Get<bool>("UI.sound.music-enabled"));
    boost::shared_ptr<const RangedValidator<int> > validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(GetOptionsDB().GetValidator("UI.sound.music-volume"));
    assert(validator);
    GG::Slider<int>* slider = new CUISlider<int>(validator->m_min, validator->m_max, GG::HORIZONTAL);
    slider->SlideTo(GetOptionsDB().Get<int>("UI.sound.music-volume"));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, 5);
    layout->Add(button, 0, 0);
    layout->Add(slider, 0, 1);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(button->MinUsableSize().y, slider->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, indentation_level));
    page->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription("UI.sound.music-enabled")));
    slider->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    slider->SetBrowseText(UserString(GetOptionsDB().GetDescription("UI.sound.music-volume")));
    fb.SetMusicButton(button);
    GG::Connect(button->CheckedSignal, &OptionsWnd::SoundOptionsFeedback::MusicClicked, &fb);
    GG::Connect(slider->SlidSignal, &OptionsWnd::SoundOptionsFeedback::MusicVolumeSlid, &fb);
}

void OptionsWnd::VolumeOption(GG::ListBox* page, int indentation_level, const std::string& toggle_option_name,
                              const std::string& volume_option_name, const std::string& text,
                              bool toggle_value, SoundOptionsFeedback &fb)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::StateButton* button = new CUIStateButton(text, GG::FORMAT_LEFT, boost::make_shared<CUICheckBoxRepresenter>());
    button->Resize(button->MinUsableSize());
    button->SetCheck(toggle_value);
    boost::shared_ptr<const RangedValidator<int> > validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(GetOptionsDB().GetValidator(volume_option_name));
    assert(validator);
    GG::Slider<int>* slider = new CUISlider<int>(validator->m_min, validator->m_max, GG::HORIZONTAL);
    slider->SlideTo(GetOptionsDB().Get<int>(volume_option_name));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, 5);
    layout->Add(button, 0, 0);
    layout->Add(slider, 0, 1);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(button->MinUsableSize().y, slider->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, indentation_level));
    page->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(toggle_option_name)));
    slider->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    slider->SetBrowseText(UserString(GetOptionsDB().GetDescription(volume_option_name)));
    fb.SetEffectsButton(button);
    GG::Connect(button->CheckedSignal,        &OptionsWnd::SoundOptionsFeedback::SoundEffectsEnableClicked, &fb);
    GG::Connect(slider->SlidAndStoppedSignal, &OptionsWnd::SoundOptionsFeedback::UISoundsVolumeSlid, &fb);
}

void OptionsWnd::FileOptionImpl(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const fs::path& path,
                                const std::vector<std::pair<std::string, std::string> >& filters,
                                StringValidator string_validator, bool directory, bool relative_path)
{
    GG::Label* text_control = new CUILabel(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    GG::Edit* edit = new CUIEdit(GetOptionsDB().Get<std::string>(option_name));
    edit->Resize(GG::Pt(50*SPIN_WIDTH, edit->Height())); // won't resize within layout bigger than its initial size, so giving a big initial size here
    GG::Button* button = new CUIButton("...");

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, ROW_WIDTH, button->MinUsableSize().y,
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

    GG::ListBox::Row* row = new OptionsListRow(ROW_WIDTH, layout->Height() + 6, layout, indentation_level);
    page->Insert(row);

    edit->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    edit->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(edit->EditedSignal, SetOptionFunctor<std::string>(option_name, edit, string_validator));
    GG::Connect(button->LeftClickedSignal, BrowseForPathButtonFunctor(path, filters, edit, directory, relative_path));
    if (string_validator && !string_validator(edit->Text()))
        edit->SetTextColor(GG::CLR_RED);
}

void OptionsWnd::FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const fs::path& path,
                            StringValidator string_validator/* = 0*/)
{ FileOption(page, indentation_level, option_name, text, path, std::vector<std::pair<std::string, std::string> >(), string_validator); }

void OptionsWnd::FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const fs::path& path,
                            const std::pair<std::string, std::string>& filter, StringValidator string_validator/* = 0*/)
{ FileOption(page, indentation_level, option_name, text, path, std::vector<std::pair<std::string, std::string> >(1, filter), string_validator); }

void OptionsWnd::FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const fs::path& path,
                            const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator/* = 0*/)
{ FileOptionImpl(page, indentation_level, option_name, text, path, filters, string_validator, false, false); }

void OptionsWnd::SoundFileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    FileOption(page, indentation_level, option_name, text, ClientUI::SoundDir(), std::make_pair(UserString("OPTIONS_SOUND_FILE"),
               "*" + SOUND_FILE_SUFFIX), ValidSoundFile);
}

void OptionsWnd::DirectoryOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text,
                                 const fs::path& path)
{
    FileOptionImpl(page, indentation_level, option_name, text, path, std::vector<std::pair<std::string, std::string> >(),
                   ValidDirectory, true, false);
}

void OptionsWnd::ColorOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::Label* text_control = new CUILabel(text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    ColorSelector* color_selector = new ColorSelector(GetOptionsDB().Get<StreamableColor>(option_name).ToClr(),
                                                      GetOptionsDB().GetDefault<StreamableColor>(option_name).ToClr());
    color_selector->Resize(GG::Pt(color_selector->Width(), GG::Y(ClientUI::Pts() + 4)));
    color_selector->SetMaxSize(GG::Pt(color_selector->MaxSize().x, color_selector->Size().y));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2);
    layout->Add(text_control, 0, 0);
    layout->Add(color_selector, 0, 1, 1, 1, GG::ALIGN_VCENTER);
    layout->SetMinimumColumnWidth(1, COLOR_SELECTOR_WIDTH);
    layout->SetColumnStretch(0, 1.0);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(text_control->MinUsableSize().y, color_selector->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, indentation_level));
    page->Insert(row);
    color_selector->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    color_selector->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(color_selector->ColorChangedSignal, SetOptionFunctor<GG::Clr>(option_name));
}

void OptionsWnd::FontOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text) {
    FileOption(page, indentation_level, option_name, text, GetRootDataDir() / "default",
               std::make_pair<std::string, std::string>(std::string(option_name), "*" + FONT_FILE_SUFFIX),
               &ValidFontFile);
}

void OptionsWnd::ResolutionOption(GG::ListBox* page, int indentation_level) {
    boost::shared_ptr<const RangedValidator<int> > width_validator =
        boost::dynamic_pointer_cast<const RangedValidator<int> >(
            GetOptionsDB().GetValidator("app-width"));
    boost::shared_ptr<const RangedValidator<int> > height_validator =
        boost::dynamic_pointer_cast<const RangedValidator<int> >(
            GetOptionsDB().GetValidator("app-height"));
    boost::shared_ptr<const RangedValidator<int> > windowed_width_validator =
        boost::dynamic_pointer_cast<const RangedValidator<int> >(
            GetOptionsDB().GetValidator("app-width-windowed"));
    boost::shared_ptr<const RangedValidator<int> > windowed_height_validator =
        boost::dynamic_pointer_cast<const RangedValidator<int> >(
            GetOptionsDB().GetValidator("app-height-windowed"));
    boost::shared_ptr<const RangedValidator<int> > windowed_left_validator =
        boost::dynamic_pointer_cast<const RangedValidator<int> >(
            GetOptionsDB().GetValidator("app-left-windowed"));
    boost::shared_ptr<const RangedValidator<int> > windowed_top_validator =
        boost::dynamic_pointer_cast<const RangedValidator<int> >(
            GetOptionsDB().GetValidator("app-top-windowed"));

    // compile list of resolutions available on this system

    std::vector<std::string> resolutions = GG::GUI::GetGUI()->GetSupportedResolutions();

    // find text representation of current fullscreen resolution selection
    int colour_depth = GetOptionsDB().Get<int>("color-depth");
    int width = GetOptionsDB().Get<int>("app-width");
    int height = GetOptionsDB().Get<int>("app-height");
    std::string current_video_mode_str = boost::io::str(boost::format("%1% x %2% @ %3%") % width % height % colour_depth);

    // find which index in list, if any, represents current fullscreen resolution selection
    int current_resolution_index = -1, loop_res_index = 0;
    for (const std::string& resolution : resolutions) {
        if (resolution == current_video_mode_str)
            current_resolution_index = loop_res_index;
        ++loop_res_index;
    }


    // drop list and label
    GG::Label* drop_list_label = new CUILabel(UserString("OPTIONS_VIDEO_MODE"), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    drop_list_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    drop_list_label->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_LIST_DESCRIPTION"));

    GG::DropDownList* drop_list = new CUIDropDownList(6);
    drop_list->Resize(GG::Pt(drop_list->MinUsableSize().x, GG::Y(ClientUI::Pts() + 4)));
    drop_list->SetMaxSize(GG::Pt(drop_list->MaxSize().x, drop_list->Size().y));
    drop_list->SetStyle(GG::LIST_NOSORT);
    drop_list->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    drop_list->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_LIST_DESCRIPTION"));

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 2, 1, 0, LAYOUT_MARGIN);
    layout->Add(drop_list_label, 0, 0);
    layout->Add(drop_list, 1, 0, 1, 1, GG::ALIGN_VCENTER);

    GG::ListBox::Row* row = new GG::ListBox::Row();
    row->Resize(GG::Pt(ROW_WIDTH, drop_list_label->MinUsableSize().y + LAYOUT_MARGIN + drop_list->MaxSize().y + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, indentation_level));

    page->Insert(row);


    // selectable rows in video modes list box...
    for (const std::string& resolution : resolutions) {
        GG::ListBox::Row* video_mode_row = new CUISimpleDropDownListRow(resolution);
        video_mode_row->SetName(resolution);
        drop_list->Insert(video_mode_row);
    }

    if (drop_list->NumRows() >= 1 && current_resolution_index != -1)
        drop_list->Select(current_resolution_index);

    // fullscreen / windowed toggle
    BoolOption(page, indentation_level, "fullscreen",            UserString("OPTIONS_FULLSCREEN"));
    // Fake mode change is not possible without the opengl frame buffer extension
    if (GG::SDLGUI::GetGUI()->FramebuffersAvailable()) {
        BoolOption(page, indentation_level, "fake-mode-change",       UserString("OPTIONS_FAKE_MODE_CHANGE"));
    } else {
        GetOptionsDB().Set<bool>("fake-mode-change", false);
    }
    IntOption(page, indentation_level,  "fullscreen-monitor-id", UserString("OPTIONS_FULLSCREEN_MONITOR_ID"));


    // customizable windowed width and height
    GG::Label* windowed_spinner_label = new CUILabel(UserString("OPTIONS_VIDEO_MODE_WINDOWED"), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP, GG::INTERACTIVE);
    windowed_spinner_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    windowed_spinner_label->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_WINDOWED_SPINNERS_DESCRIPTION"));

    row = new GG::ListBox::Row();
    row->Resize(GG::Pt(ROW_WIDTH, windowed_spinner_label->MinUsableSize().y + LAYOUT_MARGIN + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), windowed_spinner_label, indentation_level));
    page->Insert(row);

    IntOption(page, indentation_level, "app-width-windowed",  UserString("OPTIONS_APP_WIDTH_WINDOWED"));
    IntOption(page, indentation_level, "app-height-windowed", UserString("OPTIONS_APP_HEIGHT_WINDOWED"));
    IntOption(page, indentation_level, "app-left-windowed",   UserString("OPTIONS_APP_LEFT_WINDOWED"));
    IntOption(page, indentation_level, "app-top-windowed",    UserString("OPTIONS_APP_TOP_WINDOWED"));

    // fps
    BoolOption(page, indentation_level, "show-fps", UserString("OPTIONS_SHOW_FPS"));

    //GG::StateButton* limit_FPS_button = BoolOption(page, indentation_level, "limit-fps", UserString("OPTIONS_LIMIT_FPS"));
    //GG::Spin<double>* max_fps_spin = 
    DoubleOption(page, indentation_level,  "max-fps",          UserString("OPTIONS_MAX_FPS"));
    //GG::Connect(limit_FPS_button->CheckedSignal, LimitFPSSetOptionFunctor(max_fps_spin));
    //limit_FPS_button->SetCheck(GetOptionsDB().Get<bool>("limit-fps"));
    //limit_FPS_button->CheckedSignal(limit_FPS_button->Checked());

    //GG::StateButton* limit_FPS_nofocus_button = BoolOption(page, indentation_level, "limit-fps-no-focus", UserString("OPTIONS_LIMIT_FPS_NO_FOCUS"));
    //GG::Spin<double>* max_fps_nofocus_spin = 
    DoubleOption(page, indentation_level,  "max-fps-no_focus", UserString("OPTIONS_MAX_FPS_NO_FOCUS"));
    //limit_FPS_nofocus_button->SetCheck(GetOptionsDB().Get<bool>("limit-fps-no-focus"));


    // apply button, sized to fit text
    GG::Button* apply_button = new CUIButton(UserString("OPTIONS_APPLY"));
    row = new OptionsListRow(ROW_WIDTH, apply_button->MinUsableSize().y + LAYOUT_MARGIN + 6,
                             apply_button, indentation_level);
    page->Insert(row);
    GG::Connect(apply_button->LeftClickedSignal, &HumanClientApp::Reinitialize, HumanClientApp::GetApp());

    GG::Connect(drop_list->SelChangedSignal, ResolutionDropListIndexSetOptionFunctor(drop_list));
}

void OptionsWnd::HotkeysPage()
{
    GG::ListBox* page = CreatePage(UserString("OPTIONS_PAGE_HOTKEYS"));
    for (const std::map<std::string, std::set<std::string>>::value_type& class_hotkeys : Hotkey::ClassifyHotkeys()) {
        CreateSectionHeader(page, 0, UserString(class_hotkeys.first));
        for (const std::string& hotkey : class_hotkeys.second)
            HotkeyOption(page, 0, hotkey);
    }
    m_tabs->SetCurrentWnd(0);
}

OptionsWnd::~OptionsWnd()
{}

void OptionsWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    if (key == GG::GGK_ESCAPE || key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) // Same behaviour as if "done" was pressed
        DoneClicked();
}

void OptionsWnd::DoneClicked()
{
    GetOptionsDB().Commit();
    m_done = true;
}

void OptionsWnd::SoundOptionsFeedback::SoundEffectsEnableClicked(bool checked) {
    if (checked) {
        try {
            Sound::GetSound().Enable();
            GetOptionsDB().Set("UI.sound.enabled", true);
            Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-click"), true);
        } catch (Sound::InitializationFailureException const &e) {
            SoundInitializationFailure(e);
        }
    } else {
        GetOptionsDB().Set("UI.sound.enabled", false);
        if (!GetOptionsDB().Get<bool>("UI.sound.music-enabled"))
            Sound::GetSound().Disable();
    }
}

void OptionsWnd::SoundOptionsFeedback::MusicClicked(bool checked) {
    if (checked) {
        try {
            Sound::GetSound().Enable();
            GetOptionsDB().Set("UI.sound.music-enabled", true);
            Sound::GetSound().PlayMusic(GetOptionsDB().Get<std::string>("UI.sound.bg-music"), -1);
        } catch (Sound::InitializationFailureException const &e) {
            SoundInitializationFailure(e);
        }
    } else {
        GetOptionsDB().Set("UI.sound.music-enabled", false);
        Sound::GetSound().StopMusic();
        if (!GetOptionsDB().Get<bool>("UI.sound.enabled"))
            Sound::GetSound().Disable();
    }
}

void OptionsWnd::SoundOptionsFeedback::MusicVolumeSlid(int pos, int low, int high) const {
    GetOptionsDB().Set("UI.sound.music-volume", pos);
    Sound::GetSound().SetMusicVolume(pos);
}

void OptionsWnd::SoundOptionsFeedback::UISoundsVolumeSlid(int pos, int low, int high) const {
    GetOptionsDB().Set("UI.sound.volume", pos);
    Sound::GetSound().SetUISoundsVolume(pos);
    Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-click"), true);
}

void OptionsWnd::SoundOptionsFeedback::SetMusicButton(GG::StateButton* button)
{ m_music_button = button; }

void OptionsWnd::SoundOptionsFeedback::SetEffectsButton(GG::StateButton* button)
{ m_effects_button = button; }

void OptionsWnd::SoundOptionsFeedback::SoundInitializationFailure(Sound::InitializationFailureException const &e) {
    GetOptionsDB().Set("UI.sound.enabled", false);
    GetOptionsDB().Set("UI.sound.music-enabled", false);
    if (m_effects_button)
        m_effects_button->SetCheck(false);
    if (m_music_button)
        m_music_button->SetCheck(false);
    ClientUI::MessageBox(UserString(e.what()), false);
}
