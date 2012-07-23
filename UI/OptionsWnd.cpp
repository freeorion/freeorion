#include "OptionsWnd.h"

#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Directories.h"

#include "ClientUI.h"
#include "CUISpin.h"
#include "CUISlider.h"
#include "Sound.h"

#include <OgreRoot.h>
#include <OgreRenderSystem.h>

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
    const GG::Y PAGE_HEIGHT(450);
    const GG::X INDENTATION(20);
    const GG::X ROW_WIDTH(PAGE_WIDTH - 4 - 14 - 5);
    const GG::X COLOR_SELECTOR_WIDTH(75);
    const GG::X SPIN_WIDTH(65);
    const int LAYOUT_MARGIN = 3;

    const std::string STRINGTABLE_FILE_SUFFIX = "_stringtable.txt";
    const std::string MUSIC_FILE_SUFFIX = ".ogg";
    const std::string SOUND_FILE_SUFFIX = ".wav";
    const std::string FONT_FILE_SUFFIX = ".ttf";

    class PlaceholderWnd : public GG::Wnd {
    public:
        PlaceholderWnd(GG::X w, GG::Y h) : Wnd(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()) {}
        virtual void Render() {}
    };

    class RowContentsWnd : public GG::Control {
    public:
        RowContentsWnd(GG::X w, GG::Y h, Wnd* contents, int indentation_level) :
            Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE)
            {
                assert(contents);
                if (!indentation_level) {
                    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, w, h, 1, 1);
                    layout->Add(contents, 0, 0);
                    SetLayout(layout);
                } else {
                    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, w, h, 1, 2);
                    layout->SetMinimumColumnWidth(0, indentation_level * INDENTATION);
                    layout->SetColumnStretch(1, 1.0);
                    layout->Add(new PlaceholderWnd(GG::X1, GG::Y1), 0, 0);
                    layout->Add(contents, 0, 1);
                    SetLayout(layout);
                }
            }
        virtual void Render() {}
    };

    struct BrowseForPathButtonFunctor {
        BrowseForPathButtonFunctor(const fs::path& path, const std::vector<std::pair<std::string, std::string> >& filters,
                                   CUIEdit* edit, bool directory, bool return_relative_path) :
            m_path(path), m_filters(filters), m_edit(edit), m_directory(directory), m_return_relative_path(return_relative_path) {}

        void operator()() {
            try {
                FileDlg dlg(m_path.string(), m_edit->Text(), false, false, m_filters);
                if (m_directory)
                    dlg.SelectDirectories(true);
                dlg.Run();
                if (!dlg.Result().empty()) {
                    fs::path path = m_return_relative_path ?
                        RelativePath(m_path, fs::path(*dlg.Result().begin())) :
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
                    fs::absolute(*dlg.Result().begin());
#else
                    fs::complete(*dlg.Result().begin());
#endif
                    *m_edit << path.string();
                    m_edit->EditedSignal(m_edit->Text());
                }
            } catch (const FileDlg::BadInitialDirectory& e) {
                ClientUI::MessageBox(e.what(), true);
            }
        }

        fs::path                                            m_path;
        std::vector<std::pair<std::string, std::string> >   m_filters;
        CUIEdit*                                            m_edit;
        bool                                                m_directory;
        bool                                                m_return_relative_path;
    };

    bool ValidStringtableFile(const std::string& file) {
        // putting this in try-catch block prevents crash with error output along the lines of:
        // main() caught exception(std::exception): boost::filesystem::path: invalid name ":" in path: ":\FreeOrion\default"
        try {
#if defined(_WIN32)
            // convert UTF-8 file name to UTF-16
            fs::path::string_type file_name_native;
            utf8::utf8to16(file.begin(), file.end(), std::back_inserter(file_name_native));
            fs::path path = fs::path(file_name_native);
#else
            fs::path path = fs::path(file);
#endif
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
#if defined(_WIN32)
            // convert UTF-8 file name to UTF-16
            fs::path::string_type file_name_native;
            utf8::utf8to16(file.begin(), file.end(), std::back_inserter(file_name_native));
            fs::path path = fs::path(file_name_native);
#else
            fs::path path = fs::path(file);
#endif
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
#if defined(_WIN32)
            // convert UTF-8 file name to UTF-16
            fs::path::string_type file_name_native;
            utf8::utf8to16(file.begin(), file.end(), std::back_inserter(file_name_native));
            fs::path path = fs::path(file_name_native);
#else
            fs::path path = fs::path(file);
#endif
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
#if defined(_WIN32)
            // convert UTF-8 file name to UTF-16
            fs::path::string_type file_name_native;
            utf8::utf8to16(file.begin(), file.end(), std::back_inserter(file_name_native));
            fs::path path = fs::path(file_name_native);
#else
            fs::path path = fs::path(file);
#endif
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
#if defined(_WIN32)
            // convert UTF-8 file name to UTF-16
            fs::path::string_type file_name_native;
            utf8::utf8to16(file.begin(), file.end(), std::back_inserter(file_name_native));
            fs::path path = fs::path(file_name_native);
#else
            fs::path path = fs::path(file);
#endif
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
        SetOptionFunctor(const std::string& option_name, CUIEdit* edit = 0, OptionsWnd::StringValidator string_validator = 0) :
            m_option_name(option_name), m_edit(edit), m_string_validator(string_validator)
        { assert(bool(m_edit) == bool(m_string_validator)); }
        void operator()(const std::string& str)
        {
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
        CUIEdit* m_edit;
        OptionsWnd::StringValidator m_string_validator;
    };

    struct DropListIndexSetOptionFunctor {
        DropListIndexSetOptionFunctor(const std::string& option_name, CUIDropDownList* drop_list) :
            m_option_name(option_name), m_drop_list(drop_list) {}
        void operator()(GG::DropDownList::iterator it) {
            assert(it != m_drop_list->end());
            GetOptionsDB().Set<std::string>(m_option_name, (*it)->Name());
        }
        const std::string m_option_name;
        CUIDropDownList* m_drop_list;
    };

    struct ResolutionDropListIndexSetOptionFunctor {
        ResolutionDropListIndexSetOptionFunctor(CUIDropDownList* drop_list) :
            m_drop_list(drop_list)
        {}

        void operator()(GG::ListBox::iterator it) {
            const GG::ListBox::Row* row = *it;
            if (!row) {
                Logger().errorStream() << "ResolutionDropListIndexSetOptionFunctor couldn't get row from passed ListBox iterator";
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

        CUIDropDownList* m_drop_list;
    };

    struct LimitFPSSetOptionFunctor {
        LimitFPSSetOptionFunctor(CUISpin<double>* max_fps_spin) :
            m_max_fps_spin(max_fps_spin)
        {}
        void operator()(bool b) {
            Logger().debugStream() << "LimitFPSSetOptionFunction: bool: " << b;
            m_max_fps_spin->Disable(!b);
        }
        CUISpin<double>* m_max_fps_spin;
    };
}

OptionsWnd::OptionsWnd():
    CUIWnd(UserString("OPTIONS_TITLE"),
           (GG::GUI::GetGUI()->AppWidth() - (PAGE_WIDTH + 20)) / 2,
           (GG::GUI::GetGUI()->AppHeight() - (PAGE_HEIGHT + 70)) / 2,
           PAGE_WIDTH + 20, PAGE_HEIGHT + 70, GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE),
    m_current_option_list(0),
    m_indentation_level(0),
    m_tabs(0),
    m_done_button(0),
    m_num_wnds(0)
{
    SetMaxSize(GG::Pt(PAGE_WIDTH + 20, MaxSize().y));
    SetMinSize(GG::Pt(PAGE_WIDTH + 20, PAGE_HEIGHT + 70));
    m_done_button = new CUIButton(GG::X(15), PAGE_HEIGHT + 17, GG::X(75), UserString("DONE"));
    m_tabs = new GG::TabWnd(GG::X(5), GG::Y(2), PAGE_WIDTH, PAGE_HEIGHT + 20, ClientUI::GetFont(), ClientUI::WndColor(), ClientUI::TextColor(), GG::TAB_BAR_DETACHED);
    Init();
}

void OptionsWnd::BeginPage(const std::string& name) {
    m_current_option_list = new CUIListBox(GG::X0, GG::Y0, GG::X1, GG::Y1);
    m_current_option_list->SetColor(GG::CLR_ZERO);
    m_current_option_list->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
    m_tabs->AddWnd(m_current_option_list, name);
    m_tabs->SetCurrentWnd(m_num_wnds++);
}

void OptionsWnd::EndPage() {
    assert(m_current_option_list);
    m_current_option_list = 0;
    m_tabs->SetCurrentWnd(0);
}

void OptionsWnd::BeginSection(const std::string& name) {
    assert(m_current_option_list);
    assert(0 <= m_indentation_level);
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* heading_text = new GG::TextControl(GG::X0, GG::Y0, name, ClientUI::GetFont(ClientUI::Pts() * 4 / 3), ClientUI::TextColor(), GG::FORMAT_LEFT);
    row->Resize(GG::Pt(ROW_WIDTH, heading_text->MinUsableSize().y + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), heading_text, m_indentation_level));
    m_current_option_list->Insert(row);
    ++m_indentation_level;
}

void OptionsWnd::EndSection() {
    assert(m_current_option_list);
    assert(0 < m_indentation_level);
    --m_indentation_level;
}

CUIStateButton* OptionsWnd::BoolOption(const std::string& option_name, const std::string& text) {
    GG::ListBox::Row* row = new GG::ListBox::Row();
    CUIStateButton* button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, text, GG::FORMAT_LEFT);
    button->Resize(button->MinUsableSize());
    row->Resize(GG::Pt(ROW_WIDTH, button->MinUsableSize().y + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), button, m_indentation_level));
    m_current_option_list->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetCheck(GetOptionsDB().Get<bool>(option_name));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(button->CheckedSignal, SetOptionFunctor<bool>(option_name));
    return button;
}

CUISpin<int>* OptionsWnd::IntOption(const std::string& option_name, const std::string& text) {
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(GG::X0, GG::Y0, text, ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::INTERACTIVE);
    boost::shared_ptr<const ValidatorBase> validator = GetOptionsDB().GetValidator(option_name);
    CUISpin<int>* spin = 0;
    int value = GetOptionsDB().Get<int>(option_name);
    if (boost::shared_ptr<const RangedValidator<int> > ranged_validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(validator))
        spin = new CUISpin<int>(GG::X0, GG::Y0, GG::X1, value, 1, ranged_validator->m_min, ranged_validator->m_max, true);
    else if (boost::shared_ptr<const StepValidator<int> > step_validator = boost::dynamic_pointer_cast<const StepValidator<int> >(validator))
        spin = new CUISpin<int>(GG::X0, GG::Y0, GG::X1, value, step_validator->m_step_size, -1000000, 1000000, true);
    else if (boost::shared_ptr<const RangedStepValidator<int> > ranged_step_validator = boost::dynamic_pointer_cast<const RangedStepValidator<int> >(validator))
        spin = new CUISpin<int>(GG::X0, GG::Y0, GG::X1, value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);
    else if (boost::shared_ptr<const Validator<int> > int_validator = boost::dynamic_pointer_cast<const Validator<int> >(validator))
        spin = new CUISpin<int>(GG::X0, GG::Y0, GG::X1, value, 1, -1000000, 1000000, true);
    assert(spin);
    spin->SetMaxSize(GG::Pt(spin->MaxSize().x, spin->Size().y));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, 5);
    layout->Add(spin, 0, 0);
    layout->Add(text_control, 0, 1);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(spin->MinUsableSize().y, text_control->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    spin->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    spin->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(spin->ValueChangedSignal, SetOptionFunctor<int>(option_name));
    return spin;
}

CUISpin<double>* OptionsWnd::DoubleOption(const std::string& option_name, const std::string& text) {
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(GG::X0, GG::Y0, text, ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::INTERACTIVE);
    boost::shared_ptr<const ValidatorBase> validator = GetOptionsDB().GetValidator(option_name);
    CUISpin<double>* spin = 0;
    double value = GetOptionsDB().Get<double>(option_name);
    if (boost::shared_ptr<const RangedValidator<double> > ranged_validator = boost::dynamic_pointer_cast<const RangedValidator<double> >(validator))
        spin = new CUISpin<double>(GG::X0, GG::Y0, GG::X1, value, 1, ranged_validator->m_min, ranged_validator->m_max, true);
    else if (boost::shared_ptr<const StepValidator<double> > step_validator = boost::dynamic_pointer_cast<const StepValidator<double> >(validator))
        spin = new CUISpin<double>(GG::X0, GG::Y0, GG::X1, value, step_validator->m_step_size, -1000000, 1000000, true);
    else if (boost::shared_ptr<const RangedStepValidator<double> > ranged_step_validator = boost::dynamic_pointer_cast<const RangedStepValidator<double> >(validator))
        spin = new CUISpin<double>(GG::X0, GG::Y0, GG::X1, value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);
    else if (boost::shared_ptr<const Validator<double> > double_validator = boost::dynamic_pointer_cast<const Validator<double> >(validator))
        spin = new CUISpin<double>(GG::X0, GG::Y0, GG::X1, value, 1, -1000000, 1000000, true);
    assert(spin);
    spin->SetMaxSize(GG::Pt(spin->MaxSize().x, spin->Size().y));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, 5);
    layout->Add(spin, 0, 0);
    layout->Add(text_control, 0, 1);
    layout->SetMinimumColumnWidth(0, SPIN_WIDTH);
    layout->SetColumnStretch(1, 1.0);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(spin->MinUsableSize().y, text_control->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    spin->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    spin->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(spin->ValueChangedSignal, SetOptionFunctor<double>(option_name));
    return spin;
}

void OptionsWnd::MusicVolumeOption() {
    GG::ListBox::Row* row = new GG::ListBox::Row();
    CUIStateButton* button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, UserString("OPTIONS_MUSIC"), GG::FORMAT_LEFT);
    button->Resize(button->MinUsableSize());
    button->SetCheck(GetOptionsDB().Get<bool>("UI.sound.music-enabled"));
    boost::shared_ptr<const RangedValidator<int> > validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(GetOptionsDB().GetValidator("UI.sound.music-volume"));
    assert(validator);
    CUISlider<int>* slider = new CUISlider<int>(GG::X0, GG::Y0, GG::X1, GG::Y(14), validator->m_min, validator->m_max, GG::HORIZONTAL);
    slider->SlideTo(GetOptionsDB().Get<int>("UI.sound.music-volume"));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, 5);
    layout->Add(button, 0, 0);
    layout->Add(slider, 0, 1);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(button->MinUsableSize().y, slider->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription("UI.sound.music-enabled")));
    slider->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    slider->SetBrowseText(UserString(GetOptionsDB().GetDescription("UI.sound.music-volume")));
    GG::Connect(button->CheckedSignal, &OptionsWnd::MusicClicked, this);
    GG::Connect(slider->SlidSignal, &OptionsWnd::MusicVolumeSlid, this);
}

void OptionsWnd::VolumeOption(const std::string& toggle_option_name, const std::string& volume_option_name, const std::string& text,
                              VolumeSliderHandler volume_slider_handler, bool toggle_value)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    CUIStateButton* button = new CUIStateButton(GG::X0, GG::Y0, GG::X1, GG::Y1, text, GG::FORMAT_LEFT);
    button->Resize(button->MinUsableSize());
    button->SetCheck(toggle_value);
    boost::shared_ptr<const RangedValidator<int> > validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(GetOptionsDB().GetValidator(volume_option_name));
    assert(validator);
    CUISlider<int>* slider = new CUISlider<int>(GG::X0, GG::Y0, GG::X1, GG::Y(14), validator->m_min, validator->m_max, GG::HORIZONTAL);
    slider->SlideTo(GetOptionsDB().Get<int>(volume_option_name));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2, 0, 5);
    layout->Add(button, 0, 0);
    layout->Add(slider, 0, 1);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(button->MinUsableSize().y, slider->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(toggle_option_name)));
    slider->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    slider->SetBrowseText(UserString(GetOptionsDB().GetDescription(volume_option_name)));
    GG::Connect(button->CheckedSignal, SetOptionFunctor<bool>(toggle_option_name));
    GG::Connect(slider->SlidAndStoppedSignal, volume_slider_handler, this);
}

void OptionsWnd::FileOptionImpl(const std::string& option_name, const std::string& text, const fs::path& path,
                                const std::vector<std::pair<std::string, std::string> >& filters,
                                StringValidator string_validator, bool directory, bool relative_path)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(GG::X0, GG::Y0, text, ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::INTERACTIVE);
    CUIEdit* edit = new CUIEdit(GG::X0, GG::Y0, GG::X1, GetOptionsDB().Get<std::string>(option_name));
    edit->SetMaxSize(GG::Pt(edit->MaxSize().x, edit->Size().y));
    CUIButton* button = new CUIButton(GG::X0, GG::Y0, GG::X1, "...");
    button->SetMinSize(GG::Pt(button->MinUsableSize().x + 8, button->Height()));
    button->SetMaxSize(GG::Pt(button->MaxSize().x, button->Height()));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 2, 2, 0, LAYOUT_MARGIN);
    layout->Add(text_control, 0, 0, 1, 2);
    layout->Add(edit, 1, 0, 1, 1, GG::ALIGN_VCENTER);
    layout->Add(button, 1, 1, 1, 1, GG::ALIGN_VCENTER);
    layout->SetMinimumColumnWidth(1, button->Width());
    layout->SetColumnStretch(0, 1.0);
    row->Resize(GG::Pt(ROW_WIDTH, text_control->MinUsableSize().y + LAYOUT_MARGIN + std::max(edit->MinUsableSize().y, button->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    edit->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    edit->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(edit->EditedSignal, SetOptionFunctor<std::string>(option_name, edit, string_validator));
    GG::Connect(button->ClickedSignal, BrowseForPathButtonFunctor(path, filters, edit, directory, relative_path));
    if (string_validator && !string_validator(edit->Text()))
        edit->SetTextColor(GG::CLR_RED);
}

void OptionsWnd::FileOption(const std::string& option_name, const std::string& text, const fs::path& path,
                            StringValidator string_validator/* = 0*/)
{ FileOption(option_name, text, path, std::vector<std::pair<std::string, std::string> >(), string_validator); }

void OptionsWnd::FileOption(const std::string& option_name, const std::string& text, const fs::path& path,
                            const std::pair<std::string, std::string>& filter, StringValidator string_validator/* = 0*/)
{ FileOption(option_name, text, path, std::vector<std::pair<std::string, std::string> >(1, filter), string_validator); }

void OptionsWnd::FileOption(const std::string& option_name, const std::string& text, const fs::path& path,
                            const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator/* = 0*/)
{ FileOptionImpl(option_name, text, path, filters, string_validator, false, false); }

void OptionsWnd::SoundFileOption(const std::string& option_name, const std::string& text) {
    FileOption(option_name, text, ClientUI::SoundDir(), std::make_pair(UserString("OPTIONS_SOUND_FILE"),
               "*" + SOUND_FILE_SUFFIX), ValidSoundFile);
}

void OptionsWnd::DirectoryOption(const std::string& option_name, const std::string& text,
                                 const fs::path& path)
{
    FileOptionImpl(option_name, text, path, std::vector<std::pair<std::string, std::string> >(),
                   ValidDirectory, true, false);
}

void OptionsWnd::ColorOption(const std::string& option_name, const std::string& text) {
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(GG::X0, GG::Y0, text, ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::INTERACTIVE);
    ColorSelector* color_selector = new ColorSelector(GG::X0, GG::Y0, GG::X1, GG::Y(ClientUI::Pts() + 4),
                                                      GetOptionsDB().Get<StreamableColor>(option_name).ToClr(),
                                                      GetOptionsDB().GetDefault<StreamableColor>(option_name).ToClr());
    color_selector->SetMaxSize(GG::Pt(color_selector->MaxSize().x, color_selector->Size().y));
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 1, 2);
    layout->Add(text_control, 0, 0);
    layout->Add(color_selector, 0, 1, 1, 1, GG::ALIGN_VCENTER);
    layout->SetMinimumColumnWidth(1, COLOR_SELECTOR_WIDTH);
    layout->SetColumnStretch(0, 1.0);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(text_control->MinUsableSize().y, color_selector->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    color_selector->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    color_selector->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(color_selector->ColorChangedSignal, SetOptionFunctor<GG::Clr>(option_name));
}

void OptionsWnd::FontOption(const std::string& option_name, const std::string& text) {
    FileOption(option_name, text, GetRootDataDir() / "default",
               std::make_pair<std::string, std::string>(option_name, "*" + FONT_FILE_SUFFIX),
               &ValidFontFile);
}

void OptionsWnd::ResolutionOption() {
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

    Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();
    if (!render_system) {
        Logger().errorStream() << "OptionsWnd::ResolutionOption couldn't get render system!";
        return;
    }


    // compile list of resolutions available on this system
    std::vector<std::string> resolutions;
    Ogre::ConfigOptionMap& renderer_options = render_system->getConfigOptions();

    for (Ogre::ConfigOptionMap::iterator it = renderer_options.begin(); it != renderer_options.end(); ++it) {
        // only concerned with video mode options
        if (it->first != "Video Mode")
            continue;

        for (unsigned int i = 0; i < it->second.possibleValues.size(); ++i) {
            resolutions.push_back(it->second.possibleValues[i]);
            if (resolutions.back().find_first_of("@") == std::string::npos)
                resolutions.back() += " @ 32";
        }
    }


    // find text representation of current fullscreen resolution selection
    int colour_depth = GetOptionsDB().Get<int>("color-depth");
    int width = GetOptionsDB().Get<int>("app-width");
    int height = GetOptionsDB().Get<int>("app-height");
    std::string current_video_mode_str = boost::io::str(boost::format("%1% x %2% @ %3%") % width % height % colour_depth);

    // find which index in list, if any, represents current fullscreen resolution selection
    int current_resolution_index = -1, loop_res_index = 0;
    for (std::vector<std::string>::const_iterator res_it = resolutions.begin(); res_it != resolutions.end(); ++res_it, ++loop_res_index) {
        if (*res_it == current_video_mode_str)
            current_resolution_index = loop_res_index;
    }


    // create controls
    const GG::Y DROPLIST_HEIGHT = GG::Y(ClientUI::Pts() + 4);
    const GG::Y DROPLIST_DROP_HEIGHT = DROPLIST_HEIGHT * 10;


    // drop list and label
    GG::TextControl* drop_list_label =
        new GG::TextControl(GG::X0, GG::Y0, UserString("OPTIONS_VIDEO_MODE"), ClientUI::GetFont(),
                            ClientUI::TextColor(), GG::FORMAT_LEFT, GG::INTERACTIVE);
    drop_list_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    drop_list_label->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_LIST_DESCRIPTION"));

    CUIDropDownList* drop_list =
        new CUIDropDownList(GG::X0, GG::Y0, GG::X1, DROPLIST_HEIGHT, DROPLIST_DROP_HEIGHT);
    drop_list->SetMaxSize(GG::Pt(drop_list->MaxSize().x, drop_list->Size().y));
    drop_list->SetStyle(GG::LIST_NOSORT);
    drop_list->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    drop_list->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_LIST_DESCRIPTION"));

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 2, 1, 0, LAYOUT_MARGIN);
    layout->Add(drop_list_label, 0, 0);
    layout->Add(drop_list, 1, 0, 1, 1, GG::ALIGN_VCENTER);

    GG::ListBox::Row* row = new GG::ListBox::Row();
    row->Resize(GG::Pt(ROW_WIDTH, drop_list_label->MinUsableSize().y + LAYOUT_MARGIN + drop_list->MaxSize().y + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));

    m_current_option_list->Insert(row);


    // selectable rows in video modes list box...
    for (std::vector<std::string>::const_iterator it = resolutions.begin(); it != resolutions.end(); ++it) {
        GG::ListBox::Row* video_mode_row = new CUISimpleDropDownListRow(*it);
        video_mode_row->SetName(*it);
        drop_list->Insert(video_mode_row);
    }

    if (drop_list->NumRows() >= 1 && current_resolution_index != -1)
        drop_list->Select(current_resolution_index);


    // customizable windowed width and height
    GG::TextControl* windowed_spinner_label =
        new GG::TextControl(GG::X0, GG::Y0, UserString("OPTIONS_VIDEO_MODE_WINDOWED"), ClientUI::GetFont(),
                            ClientUI::TextColor(), GG::FORMAT_LEFT, GG::INTERACTIVE);
    windowed_spinner_label->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    windowed_spinner_label->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_WINDOWED_SPINNERS_DESCRIPTION"));

    row = new GG::ListBox::Row();
    row->Resize(GG::Pt(ROW_WIDTH, windowed_spinner_label->MinUsableSize().y + LAYOUT_MARGIN + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), windowed_spinner_label, m_indentation_level));
    m_current_option_list->Insert(row);

    IntOption("app-width-windowed", UserString("OPTIONS_APP_WIDTH_WINDOWED"));
    IntOption("app-height-windowed", UserString("OPTIONS_APP_HEIGHT_WINDOWED"));


    // fullscreen / windowed toggle
    BoolOption("fullscreen", UserString("OPTIONS_FULLSCREEN"));


    // fps
    BoolOption("show-fps", UserString("OPTIONS_SHOW_FPS"));

    CUIStateButton* limit_FPS_button = BoolOption("limit-fps", UserString("OPTIONS_LIMIT_FPS"));
    CUISpin<double>* max_fps_spin = DoubleOption("max-fps", UserString("OPTIONS_MAX_FPS"));
    GG::Connect(limit_FPS_button->CheckedSignal, LimitFPSSetOptionFunctor(max_fps_spin));
    limit_FPS_button->SetCheck(GetOptionsDB().Get<bool>("limit-fps"));
    limit_FPS_button->CheckedSignal(limit_FPS_button->Checked());


    // apply button, sized to fit text
    std::string apply_button_text = UserString("OPTIONS_APPLY");
    GG::X button_width = ClientUI::GetFont()->TextExtent(apply_button_text).x + GG::X(LAYOUT_MARGIN);
    GG::Button* apply_button = new CUIButton(GG::X(LAYOUT_MARGIN), GG::Y(LAYOUT_MARGIN), GG::X(20), apply_button_text, ClientUI::GetFont());
    row = new GG::ListBox::Row();
    row->Resize(GG::Pt(ROW_WIDTH, apply_button->MinUsableSize().y + LAYOUT_MARGIN + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), apply_button, m_indentation_level));
    m_current_option_list->Insert(row);


    GG::Connect(apply_button->ClickedSignal, &HumanClientApp::Reinitialize, HumanClientApp::GetApp());

    GG::Connect(drop_list->SelChangedSignal, ResolutionDropListIndexSetOptionFunctor(drop_list));
}

void OptionsWnd::Init() {
    bool UI_sound_enabled = GetOptionsDB().Get<bool>("UI.sound.enabled");

    Sound::TempUISoundDisabler sound_disabler;

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, GG::X1, GG::Y1, 2, 2, LAYOUT_MARGIN, LAYOUT_MARGIN);
    layout->SetMinimumColumnWidth(0, m_done_button->Width() + LAYOUT_MARGIN);
    layout->SetColumnStretch(1, 1.0);
    layout->SetRowStretch(0, 1.0);
    layout->SetMinimumRowHeight(1, m_done_button->Height() + LAYOUT_MARGIN);
    layout->Add(m_tabs, 0, 0, 1, 2);
    layout->Add(m_done_button, 1, 0);
    SetLayout(layout);

    // Video settings tab
    BeginPage(UserString("OPTIONS_PAGE_VIDEO"));
    ResolutionOption();
    EndPage();

    // Audio settings tab
    BeginPage(UserString("OPTIONS_PAGE_AUDIO"));
    BeginSection(UserString("OPTIONS_VOLUME_AND_MUSIC"));
    MusicVolumeOption();
    VolumeOption("UI.sound.enabled", "UI.sound.volume", UserString("OPTIONS_UI_SOUNDS"), &OptionsWnd::UISoundsVolumeSlid, UI_sound_enabled);
    FileOption("UI.sound.bg-music", UserString("OPTIONS_BACKGROUND_MUSIC"), ClientUI::SoundDir(),
               std::make_pair(UserString("OPTIONS_MUSIC_FILE"), "*" + MUSIC_FILE_SUFFIX),
               ValidMusicFile);
    EndSection();
    BeginSection(UserString("OPTIONS_SOUNDS"));
    BeginSection(UserString("OPTIONS_UI_SOUNDS"));
    SoundFileOption("UI.sound.alert",           UserString("OPTIONS_SOUND_ALERT"));
    SoundFileOption("UI.sound.text-typing",     UserString("OPTIONS_SOUND_TYPING"));
    EndSection();
    BeginSection(UserString("OPTIONS_SOUND_WINDOW"));
    SoundFileOption("UI.sound.window-close",    UserString("OPTIONS_SOUND_CLOSE"));
    SoundFileOption("UI.sound.window-maximize", UserString("OPTIONS_SOUND_MAXIMIZE"));
    SoundFileOption("UI.sound.window-minimize", UserString("OPTIONS_SOUND_MINIMIZE"));
    SoundFileOption("UI.sound.sidepanel-open",  UserString("OPTIONS_SOUND_SIDEPANEL"));
    EndSection();
    BeginSection(UserString("OPTIONS_SOUND_LIST"));
    SoundFileOption("UI.sound.item-drop",       UserString("OPTIONS_SOUND_DROP"));
    SoundFileOption("UI.sound.list-pulldown",   UserString("OPTIONS_SOUND_PULLDOWN"));
    SoundFileOption("UI.sound.list-select",     UserString("OPTIONS_SOUND_SELECT"));
    EndSection();
    BeginSection(UserString("OPTIONS_SOUND_BUTTON"));
    SoundFileOption("UI.sound.button-click",            UserString("OPTIONS_SOUND_CLICK"));
    SoundFileOption("UI.sound.button-rollover",         UserString("OPTIONS_SOUND_ROLLOVER"));
    SoundFileOption("UI.sound.fleet-button-click",      UserString("OPTIONS_SOUND_FLEET_CLICK"));
    SoundFileOption("UI.sound.fleet-button-rollover",   UserString("OPTIONS_SOUND_FLEET_ROLLOVER"));
    SoundFileOption("UI.sound.system-icon-rollover",    UserString("OPTIONS_SOUND_SYSTEM_ROLLOVER"));
    SoundFileOption("UI.sound.turn-button-click",       UserString("OPTIONS_SOUND_TURN"));
    SoundFileOption("UI.sound.planet-button-click",     UserString("OPTIONS_SOUND_PLANET"));
    EndSection();
    BeginSection(UserString("OPTIONS_SOUND_FOCUS"));
    SoundFileOption("UI.sound.balanced-focus",  UserString("OPTIONS_SOUND_BALANCED"));
    SoundFileOption("UI.sound.farming-focus",   UserString("OPTIONS_SOUND_FARMING"));
    SoundFileOption("UI.sound.industry-focus",  UserString("OPTIONS_SOUND_INDUSTRY"));
    SoundFileOption("UI.sound.mining-focus",    UserString("OPTIONS_SOUND_MINING"));
    SoundFileOption("UI.sound.research-focus",  UserString("OPTIONS_SOUND_RESEARCH"));
    EndSection();
    EndSection();
    EndPage();

    // UI settings tab
    BeginPage(UserString("OPTIONS_PAGE_UI"));
    BeginSection(UserString("OPTIONS_MISC_UI"));
    BoolOption("UI.fleet-autoselect",           UserString("OPTIONS_AUTOSELECT_FLEET"));
    BoolOption("UI.multiple-fleet-windows",     UserString("OPTIONS_MULTIPLE_FLEET_WNDS"));
    BoolOption("UI.window-quickclose",          UserString("OPTIONS_QUICK_CLOSE_WNDS"));
    BoolOption("UI.sidepanel-planet-shown",     UserString("OPTIONS_SHOW_SIDEPANEL_PLANETS"));
    FileOption("stringtable-filename",          UserString("OPTIONS_LANGUAGE"),     GetRootDataDir() / "default", std::make_pair(UserString("OPTIONS_LANGUAGE_FILE"), "*" + STRINGTABLE_FILE_SUFFIX), &ValidStringtableFile);
    IntOption("UI.tooltip-delay",               UserString("OPTIONS_TOOLTIP_DELAY"));
    EndSection();
    BeginSection(UserString("OPTIONS_FONTS"));
    FontOption("UI.font",                       UserString("OPTIONS_FONT_TEXT"));
    FontOption("UI.title-font",                 UserString("OPTIONS_FONT_TITLE"));
    EndSection();
    BeginSection(UserString("OPTIONS_FONT_SIZES"));
    IntOption("UI.font-size",                   UserString("OPTIONS_FONT_TEXT"));
    IntOption("UI.title-font-size",             UserString("OPTIONS_FONT_TITLE"));
    EndSection();
    BeginSection(UserString("OPTIONS_TECH_SPACING"));
    DoubleOption("UI.tech-layout-horz-spacing", UserString("OPTIONS_HORIZONTAL"));
    DoubleOption("UI.tech-layout-vert-spacing", UserString("OPTIONS_VERTICAL"));
    EndSection();
    BeginSection(UserString("OPTIONS_DESCRIPTIONS"));
    BoolOption("UI.autogenerated-effects-descriptions", UserString("OPTIONS_AUTO_EFFECT_DESC"));
    BoolOption("verbose-logging",                       UserString("OPTIONS_VERBOSE_LOGGING_DESC"));
    BoolOption("verbose-sitrep",                        UserString("OPTIONS_VERBOSE_SITREP_DESC"));
    EndSection();
    EndPage();

    // Galaxy Map Page
    BeginPage(UserString("OPTIONS_GALAXY_MAP"));
    BeginSection(UserString("OPTIONS_SYSTEM_ICONS"));
    IntOption("UI.system-icon-size",                    UserString("OPTIONS_UI_SYSTEM_ICON_SIZE"));
    BoolOption("UI.system-circles",                     UserString("OPTIONS_UI_SYSTEM_CIRCLES"));
    DoubleOption("UI.system-circle-size",               UserString("OPTIONS_UI_SYSTEM_CIRCLE_SIZE"));
    DoubleOption("UI.system-selection-indicator-size",  UserString("OPTIONS_UI_SYSTEM_SELECTION_INDICATOR_SIZE"));
    IntOption("UI.system-selection-indicator-fps",      UserString("OPTIONS_UI_SYSTEM_SELECTION_INDICATOR_FPS"));
    IntOption("UI.system-tiny-icon-size-threshold",     UserString("OPTIONS_UI_SYSTEM_TINY_ICON_SIZE_THRESHOLD"));
    ColorOption("UI.system-name-unowned-color",         UserString("OPTIONS_UI_SYSTEM_NAME_UNOWNED_COLOR"));
    BoolOption("UI.system-fog-of-war",                  UserString("OPTIONS_UI_SYSTEM_FOG"));
    DoubleOption("UI.system-fog-of-war-spacing",        UserString("OPTIONS_UI_SYSTEM_FOG_SPACING"));
    BoolOption("UI.optimized-system-rendering",         UserString("OPTIONS_OPTIMIZED_SYSTEM_RENDERING"));
    EndSection();
    BeginSection(UserString("OPTIONS_FLEET_ICONS"));
    DoubleOption("UI.tiny-fleet-button-minimum-zoom",   UserString("OPTIONS_UI_TINY_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption("UI.small-fleet-button-minimum-zoom",  UserString("OPTIONS_UI_SMALL_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption("UI.medium-fleet-button-minimum-zoom", UserString("OPTIONS_UI_MEDIUM_FLEET_BUTTON_MIN_ZOOM"));
    DoubleOption("UI.fleet-selection-indicator-size",   UserString("OPTIONS_UI_FLEET_SELECTION_INDICATOR_SIZE"));
    EndSection();
    BeginSection(UserString("OPTIONS_STARLANES"));
    DoubleOption("UI.starlane-thickness",               UserString("OPTIONS_STARLANE_THICKNESS"));
    BoolOption("UI.resource-starlane-colouring",        UserString("OPTIONS_RESOURCE_STARLANE_COLOURING"));
    BoolOption("UI.fleet-supply-lines",                 UserString("OPTIONS_FLEET_SUPPLY_LINES"));
    DoubleOption("UI.fleet-supply-line-width",          UserString("OPTIONS_FLEET_SUPPLY_LINE_WIDTH"));
    IntOption("UI.fleet-supply-line-dot-spacing",       UserString("OPTIONS_FLEET_SUPPLY_LINE_DOT_SPACING"));
    DoubleOption("UI.fleet-supply-line-dot-rate",       UserString("OPTIONS_FLEET_SUPPLY_LINE_DOT_RATE"));
    ColorOption("UI.unowned-starlane-colour",           UserString("OPTIONS_UNOWNED_STARLANE_COLOUR"));
    EndSection();
    BeginSection(UserString("OPTIONS_GALAXY_MAP_GENERAL"));
    BoolOption("UI.galaxy-gas-background",              UserString("OPTIONS_GALAXY_MAP_GAS"));
    BoolOption("UI.galaxy-starfields",                  UserString("OPTIONS_GALAXY_MAP_STARFIELDS"));
    BoolOption("UI.show-galaxy-map-scale",              UserString("OPTIONS_GALAXY_MAP_SCALE_LINE"));
    BoolOption("UI.show-galaxy-map-zoom-slider",        UserString("OPTIONS_GALAXY_MAP_ZOOM_SLIDER"));
    BoolOption("UI.show-detection-range",               UserString("OPTIONS_GALAXY_MAP_DETECTION_RANGE"));
    EndSection();
    EndPage();

    // Colors tab
    BeginPage(UserString("OPTIONS_PAGE_COLORS"));
    BeginSection(UserString("OPTIONS_GENERAL_COLORS"));
    ColorOption("UI.text-color",                    UserString("OPTIONS_TEXT_COLOR"));
    ColorOption("UI.default-link-color",            UserString("OPTIONS_DEFAULT_LINK_COLOR"));
    ColorOption("UI.rollover-link-color",           UserString("OPTIONS_ROLLOVER_LINK_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_WINDOW_COLORS"));
    ColorOption("UI.wnd-color",                     UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.wnd-inner-border-color",        UserString("OPTIONS_INNER_BORDER_COLOR"));
    ColorOption("UI.wnd-outer-border-color",        UserString("OPTIONS_OUTER_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_CONTROL_COLORS"));
    ColorOption("UI.ctrl-color",                    UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.ctrl-border-color",             UserString("OPTIONS_BORDER_COLOR"));
    ColorOption("UI.edit-hilite",                   UserString("OPTIONS_HIGHLIGHT_COLOR"));
    ColorOption("UI.dropdownlist-arrow-color",      UserString("OPTIONS_DROPLIST_ARROW_COLOR"));
    ColorOption("UI.state-button-color",            UserString("OPTIONS_STATE_BUTTON_COLOR"));
    ColorOption("UI.stat-increase-color",           UserString("OPTIONS_STAT_INCREASE_COLOR"));
    ColorOption("UI.stat-decrease-color",           UserString("OPTIONS_STAT_DECREASE_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_TECH_COLORS"));
    BeginSection(UserString("OPTIONS_KNOWN_TECH_COLORS"));
    ColorOption("UI.known-tech",                    UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.known-tech-border",             UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_RESEARCHABLE_TECH_COLORS"));
    ColorOption("UI.researchable-tech",             UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.researchable-tech-border",      UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_UNRESEARCHABLE_TECH_COLORS"));
    ColorOption("UI.unresearchable-tech",           UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.unresearchable-tech-border",    UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_TECH_PROGRESS_COLORS"));
    ColorOption("UI.tech-progress",                 UserString("OPTIONS_PROGRESS_BAR_COLOR"));
    ColorOption("UI.tech-progress-background",      UserString("OPTIONS_PROGRESS_BACKGROUND_COLOR"));
    EndSection();
    EndSection();
    EndPage();

    // combat settings tab
    BeginPage(UserString("OPTIONS_PAGE_COMBAT"));
    BoolOption("combat.enable-glow",                UserString("OPTIONS_COMBAT_ENABLE_GLOW"));
    BoolOption("combat.enable-skybox",              UserString("OPTIONS_COMBAT_ENABLE_SKYBOX"));
    BoolOption("combat.enable-lens-flare",          UserString("OPTIONS_COMBAT_ENABLE_LENS_FLARES"));
    BoolOption("combat.filled-selection",           UserString("OPTIONS_COMBAT_FILLED_SELECTION"));
    EndPage();

    // Ausosave settings tab
    BeginPage(UserString("OPTIONS_PAGE_AUTOSAVE"));
    BoolOption("autosave.single-player",            UserString("OPTIONS_SINGLEPLAYER"));
    BoolOption("autosave.multiplayer",              UserString("OPTIONS_MULTIPLAYER"));
    IntOption("autosave.turns",                     UserString("OPTIONS_AUTOSAVE_TURNS_BETWEEN"));
    EndPage();

    // Directories tab
    BeginPage(UserString("OPTIONS_PAGE_DIRECTORIES"));
    DirectoryOption("resource-dir",                 UserString("OPTIONS_FOLDER_SETTINGS"),  GetRootDataDir());  // GetRootDataDir() returns the default browse path when modifying this directory option.  the actual default directory (before modifying) is gotten from the specified option name "resource-dir"
    DirectoryOption("save-dir",                     UserString("OPTIONS_FOLDER_SAVE"),      GetUserDir());
    EndPage();

    // Connect the done and cancel button
    GG::Connect(m_done_button->ClickedSignal, &OptionsWnd::DoneClicked, this);
}

OptionsWnd::~OptionsWnd()
{}

void OptionsWnd::KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    if (key == GG::GGK_ESCAPE || key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) // Same behaviour as if "done" was pressed
        DoneClicked();
}

void OptionsWnd::DoneClicked()
{
    // Save the changes:
    {
        fs::ofstream ofs(GetConfigPath());
        if (ofs) {
            GetOptionsDB().GetXML().WriteDoc(ofs);
        } else {
            std::cerr << UserString("UNABLE_TO_WRITE_CONFIG_XML") << std::endl;
            std::cerr << GetConfigPath().string() << std::endl;
            Logger().errorStream() << UserString("UNABLE_TO_WRITE_CONFIG_XML");
            Logger().errorStream() << GetConfigPath().string();
        }
    }

    m_done = true;
}

void OptionsWnd::MusicClicked(bool checked)
{
    if (checked) {
        GetOptionsDB().Set("UI.sound.music-enabled", true);
        Sound::GetSound().PlayMusic(GetOptionsDB().Get<std::string>("UI.sound.bg-music"), -1);
    } else {
        GetOptionsDB().Set("UI.sound.music-enabled", false);
        Sound::GetSound().StopMusic();
    }
}

void OptionsWnd::MusicVolumeSlid(int pos, int low, int high)
{
    GetOptionsDB().Set("UI.sound.music-volume", pos);
    Sound::GetSound().SetMusicVolume(pos);
}

void OptionsWnd::UISoundsVolumeSlid(int pos, int low, int high)
{
    GetOptionsDB().Set("UI.sound.volume", pos);
    Sound::GetSound().SetUISoundsVolume(pos);
    Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-click"), true);
}
