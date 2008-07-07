#include "OptionsWnd.h"

#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Directories.h"

#include "ClientUI.h"
#include "CUISpin.h"
#include "Sound.h"

#include <OgreRoot.h>
#include <OgreRenderSystem.h>

#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/TabWnd.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/cerrno.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/spirit.hpp>


namespace fs = boost::filesystem;

namespace {
    const int PAGE_WIDTH = 400;
    const int PAGE_HEIGHT = 450;
    const int INDENTATION = 20;
    const int ROW_WIDTH = PAGE_WIDTH - 4 - 14 - 5;
    const int COLOR_SELECTOR_WIDTH = 75;
    const int SPIN_WIDTH = 65;
    const int LAYOUT_MARGIN = 3;

    const std::string STRINGTABLE_FILE_SUFFIX = "_stringtable.txt";
    const std::string MUSIC_FILE_SUFFIX = ".ogg";
    const std::string SOUND_FILE_SUFFIX = ".wav";
    const std::string FONT_SUFFIX = ".ttf";

    class PlaceholderWnd : public GG::Wnd
    {
    public:
        PlaceholderWnd(int w, int h) : Wnd(0, 0, w, h, GG::Flags<GG::WndFlag>()) {}
        virtual void Render() {}
    };

    class RowContentsWnd : public GG::Control
    {
    public:
        RowContentsWnd(int w, int h, Wnd* contents, int indentation_level) :
            Control(0, 0, w, h, GG::CLICKABLE)
            {
                assert(contents);
                if (!indentation_level) {
                    GG::Layout* layout = new GG::Layout(0, 0, w, h, 1, 1);
                    layout->Add(contents, 0, 0);
                    SetLayout(layout);
                } else {
                    GG::Layout* layout = new GG::Layout(0, 0, w, h, 1, 2);
                    layout->SetMinimumColumnWidth(0, indentation_level * INDENTATION);
                    layout->SetColumnStretch(1, 1.0);
                    layout->Add(new PlaceholderWnd(1, 1), 0, 0);
                    layout->Add(contents, 0, 1);
                    SetLayout(layout);
                }
            }
        virtual void Render() {}
    };

    struct BrowseForPathButtonFunctor
    {
        BrowseForPathButtonFunctor(const fs::path& path, const std::vector<std::pair<std::string, std::string> >& filters, CUIEdit* edit, bool directory) :
            m_path(path), m_filters(filters), m_edit(edit), m_directory(directory) {}

        void operator()()
            {
                try {
                    FileDlg dlg(m_path.native_directory_string(), m_edit->WindowText(), false, false, m_filters);
                    if (m_directory)
                        dlg.SelectDirectories(true);
                    dlg.Run();
                    if (!dlg.Result().empty()) {
                        fs::path path = m_directory ? fs::complete(*dlg.Result().begin()) : RelativePath(m_path, fs::path(*dlg.Result().begin()));
                        *m_edit << path.native_file_string();
                    }
                } catch (const FileDlg::BadInitialDirectory& e) {
                    ClientUI::MessageBox(e.what(), true);
                }
            }

        fs::path m_path;
        std::vector<std::pair<std::string, std::string> > m_filters;
        CUIEdit* m_edit;
        bool m_directory;
    };

    bool ValidStringtableFile(const std::string& file)
    {
        return boost::algorithm::ends_with(file, STRINGTABLE_FILE_SUFFIX) &&
            fs::exists(GetSettingsDir() / file) && !fs::is_directory(GetSettingsDir() / file);
    }

    bool ValidMusicFile(const std::string& file)
    {
        return boost::algorithm::ends_with(file, MUSIC_FILE_SUFFIX) &&
            fs::exists(ClientUI::SoundDir() / file) && !fs::is_directory(ClientUI::SoundDir() / file);
    }

    bool ValidSoundFile(const std::string& file)
    {
        return boost::algorithm::ends_with(file, SOUND_FILE_SUFFIX) &&
            fs::exists(ClientUI::SoundDir() / file) && !fs::is_directory(ClientUI::SoundDir() / file);
    }

    bool ValidDirectory(const std::string& file)
    {
        // putting this in try-catch block prevents crash with error output along the lines of:
        // main() caught exception(std::exception): boost::filesystem::path: invalid name ":" in path: ":\FreeOrion\default"
        try {
            fs::path path = fs::path(file);
            return fs::exists(path) && fs::is_directory(path);
        } catch (std::exception ex) {
            return false;
        }        
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
                    m_edit->SetTextColor(GG::CLR_RED);
                } else {
                    m_edit->SetTextColor(ClientUI::TextColor());
                    GetOptionsDB().Set<std::string>(m_option_name, str);
                }
            }
        const std::string m_option_name;
        CUIEdit* m_edit;
        OptionsWnd::StringValidator m_string_validator;
    };

    struct DropListIndexSetOptionFunctor
    {
        DropListIndexSetOptionFunctor(const std::string& option_name, CUIDropDownList* drop_list) :
            m_option_name(option_name), m_drop_list(drop_list) {}
        void operator()(GG::DropDownList::iterator it)
            {
                assert(it != m_drop_list->end());
                GetOptionsDB().Set<std::string>(m_option_name, (*it)->WindowText());
            }
        const std::string m_option_name;
        CUIDropDownList* m_drop_list;
    };

    struct ResolutionDropListIndexSetOptionFunctor
    {
        ResolutionDropListIndexSetOptionFunctor(CUIDropDownList* drop_list, CUISpin<int>* width_spin, CUISpin<int>* height_spin, CUISpin<int>* color_depth_spin, CUIStateButton* fullscreen_button) :
            m_drop_list(drop_list), m_width_spin(width_spin), m_height_spin(height_spin), m_color_depth_spin(color_depth_spin), m_fullscreen_button(fullscreen_button)
            {
                const GG::ListBox::Row* row = *m_drop_list->CurrentItem();
                if (row && row->WindowText() != UserString("OPTIONS_VIDEO_MODE_LIST_CUSTOM_OPTION")) {
                    m_width_spin->Disable(true);
                    m_height_spin->Disable(true);
                    m_color_depth_spin->Disable(true);
                    if (m_fullscreen_button)
                        m_fullscreen_button->Disable(true);
                }
            }
        void operator()(GG::ListBox::iterator it)
            {
                const GG::ListBox::Row* row = *it;
                assert(row);
                if (row->WindowText() == UserString("OPTIONS_VIDEO_MODE_LIST_CUSTOM_OPTION")) {
                    m_width_spin->Disable(false);
                    m_height_spin->Disable(false);
                    m_color_depth_spin->Disable(false);
                    if (m_fullscreen_button)
                        m_fullscreen_button->Disable(false);
                    GetOptionsDB().Set<int>("app-width", m_width_spin->Value());
                    GetOptionsDB().Set<int>("app-height", m_height_spin->Value());
                    GetOptionsDB().Set<int>("color-depth", m_color_depth_spin->Value());
                    if (m_fullscreen_button)
                        GetOptionsDB().Set<bool>("fullscreen", m_fullscreen_button->Checked());
                } else {
                    int w, h, bpp;
                    using namespace boost::spirit;
                    rule<> resolution_p = int_p[assign_a(w)] >> str_p(" x ") >> int_p[assign_a(h)] >> str_p(" @ ") >> int_p[assign_a(bpp)];
                    parse(row->WindowText().c_str(), resolution_p);
                    GetOptionsDB().Set<int>("app-width", w);
                    GetOptionsDB().Set<int>("app-height", h);
                    GetOptionsDB().Set<int>("color-depth", bpp);
                    if (m_fullscreen_button)
                        GetOptionsDB().Set<bool>("fullscreen", true);
                    m_width_spin->Disable(true);
                    m_height_spin->Disable(true);
                    m_color_depth_spin->Disable(true);
                    if (m_fullscreen_button)
                        m_fullscreen_button->Disable(true);
                }
            }
        CUIDropDownList* m_drop_list;
        CUISpin<int>* m_width_spin;
        CUISpin<int>* m_height_spin;
        CUISpin<int>* m_color_depth_spin;
        CUIStateButton* m_fullscreen_button;
    };
    struct LimitFPSSetOptionFunctor
    {
        LimitFPSSetOptionFunctor(CUISpin<double>* max_fps_spin) :
            m_max_fps_spin(max_fps_spin)
        {}
        void operator()(bool b)
        {
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
           PAGE_WIDTH + 20, PAGE_HEIGHT + 70, GG::CLICKABLE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE),
    m_current_option_list(0),
    m_indentation_level(0),
    m_tabs(0),
    m_done_button(0),
    m_num_wnds(0)
{
    SetMaxSize(GG::Pt(PAGE_WIDTH + 20, MaxSize().y));
    SetMinSize(GG::Pt(PAGE_WIDTH + 20, PAGE_HEIGHT + 70));
    m_done_button = new CUIButton(15, PAGE_HEIGHT + 17, 75, UserString("DONE"));
    m_tabs = new GG::TabWnd(5, 2, PAGE_WIDTH, PAGE_HEIGHT + 20, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::WndColor(), ClientUI::TextColor(), GG::TAB_BAR_DETACHED);
    Init();
}

void OptionsWnd::BeginPage(const std::string& name)
{
    m_current_option_list = new CUIListBox(0, 0, 1, 1);
    m_current_option_list->SetColor(GG::CLR_ZERO);
    m_current_option_list->SetStyle(GG::LIST_NOSORT | GG::LIST_NOSEL);
    m_tabs->AddWnd(m_current_option_list, name);
    m_tabs->SetCurrentWnd(m_num_wnds++);
}

void OptionsWnd::EndPage()
{
    assert(m_current_option_list);
    m_current_option_list = 0;
    m_tabs->SetCurrentWnd(0);
}

void OptionsWnd::BeginSection(const std::string& name)
{
    assert(m_current_option_list);
    assert(0 <= m_indentation_level);
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* heading_text = new GG::TextControl(0, 0, name, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts() * 4 / 3), ClientUI::TextColor(), GG::FORMAT_LEFT);
    row->Resize(GG::Pt(ROW_WIDTH, heading_text->MinUsableSize().y + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), heading_text, m_indentation_level));
    m_current_option_list->Insert(row);
    ++m_indentation_level;
}

void OptionsWnd::EndSection()
{
    assert(m_current_option_list);
    assert(0 < m_indentation_level);
    --m_indentation_level;
}

CUIStateButton* OptionsWnd::BoolOption(const std::string& option_name, const std::string& text)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    CUIStateButton* button = new CUIStateButton(0, 0, 1, 1, text, GG::FORMAT_LEFT);
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

CUISpin<int>* OptionsWnd::IntOption(const std::string& option_name, const std::string& text)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(0, 0, text, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::CLICKABLE);
    boost::shared_ptr<const ValidatorBase> validator = GetOptionsDB().GetValidator(option_name);
    CUISpin<int>* spin = 0;
    int value = GetOptionsDB().Get<int>(option_name);
    if (boost::shared_ptr<const RangedValidator<int> > ranged_validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(validator))
        spin = new CUISpin<int>(0, 0, 1, value, 1, ranged_validator->m_min, ranged_validator->m_max, true);
    else if (boost::shared_ptr<const StepValidator<int> > step_validator = boost::dynamic_pointer_cast<const StepValidator<int> >(validator))
        spin = new CUISpin<int>(0, 0, 1, value, step_validator->m_step_size, -1000000, 1000000, true);
    else if (boost::shared_ptr<const RangedStepValidator<int> > ranged_step_validator = boost::dynamic_pointer_cast<const RangedStepValidator<int> >(validator))
        spin = new CUISpin<int>(0, 0, 1, value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);
    else if (boost::shared_ptr<const Validator<int> > int_validator = boost::dynamic_pointer_cast<const Validator<int> >(validator))
        spin = new CUISpin<int>(0, 0, 1, value, 1, -1000000, 1000000, true);
    assert(spin);
    spin->SetMaxSize(GG::Pt(spin->MaxSize().x, spin->Size().y));
    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 1, 2, 0, 5);
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

CUISpin<double>* OptionsWnd::DoubleOption(const std::string& option_name, const std::string& text)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(0, 0, text, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::CLICKABLE);
    boost::shared_ptr<const ValidatorBase> validator = GetOptionsDB().GetValidator(option_name);
    CUISpin<double>* spin = 0;
    double value = GetOptionsDB().Get<double>(option_name);
    if (boost::shared_ptr<const RangedValidator<double> > ranged_validator = boost::dynamic_pointer_cast<const RangedValidator<double> >(validator))
        spin = new CUISpin<double>(0, 0, 1, value, 1, ranged_validator->m_min, ranged_validator->m_max, true);
    else if (boost::shared_ptr<const StepValidator<double> > step_validator = boost::dynamic_pointer_cast<const StepValidator<double> >(validator))
        spin = new CUISpin<double>(0, 0, 1, value, step_validator->m_step_size, -1000000, 1000000, true);
    else if (boost::shared_ptr<const RangedStepValidator<double> > ranged_step_validator = boost::dynamic_pointer_cast<const RangedStepValidator<double> >(validator))
        spin = new CUISpin<double>(0, 0, 1, value, ranged_step_validator->m_step_size, ranged_step_validator->m_min, ranged_step_validator->m_max, true);
    else if (boost::shared_ptr<const Validator<double> > double_validator = boost::dynamic_pointer_cast<const Validator<double> >(validator))
        spin = new CUISpin<double>(0, 0, 1, value, 1, -1000000, 1000000, true);
    assert(spin);
    spin->SetMaxSize(GG::Pt(spin->MaxSize().x, spin->Size().y));
    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 1, 2, 0, 5);
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

void OptionsWnd::MusicVolumeOption()
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    CUIStateButton* button = new CUIStateButton(0, 0, 1, 1, UserString("OPTIONS_MUSIC"), GG::FORMAT_LEFT);
    button->Resize(button->MinUsableSize());
    button->SetCheck(!GetOptionsDB().Get<bool>("music-off"));
    boost::shared_ptr<const RangedValidator<int> > validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(GetOptionsDB().GetValidator("music-volume"));
    assert(validator);
    CUISlider* slider = new CUISlider(0, 0, 1, 14, validator->m_min, validator->m_max, GG::HORIZONTAL);
    slider->SlideTo(GetOptionsDB().Get<int>("music-volume"));
    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 1, 2, 0, 5);
    layout->Add(button, 0, 0);
    layout->Add(slider, 0, 1);
    row->Resize(GG::Pt(ROW_WIDTH, std::max(button->MinUsableSize().y, slider->MinUsableSize().y) + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    button->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    button->SetBrowseText(UserString(GetOptionsDB().GetDescription("music-off")));
    slider->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    slider->SetBrowseText(UserString(GetOptionsDB().GetDescription("music-volume")));
    GG::Connect(button->CheckedSignal, &OptionsWnd::MusicClicked, this);
    GG::Connect(slider->SlidSignal, &OptionsWnd::MusicVolumeSlid, this);
}

void OptionsWnd::VolumeOption(const std::string& toggle_option_name, const std::string& volume_option_name, const std::string& text,
                              VolumeSliderHandler volume_slider_handler, bool toggle_value)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    CUIStateButton* button = new CUIStateButton(0, 0, 1, 1, text, GG::FORMAT_LEFT);
    button->Resize(button->MinUsableSize());
    button->SetCheck(toggle_value);
    boost::shared_ptr<const RangedValidator<int> > validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(GetOptionsDB().GetValidator(volume_option_name));
    assert(validator);
    CUISlider* slider = new CUISlider(0, 0, 1, 14, validator->m_min, validator->m_max, GG::HORIZONTAL);
    slider->SlideTo(GetOptionsDB().Get<int>(volume_option_name));
    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 1, 2, 0, 5);
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

void OptionsWnd::FileOptionImpl(const std::string& option_name, const std::string& text, const fs::path& path, const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator, bool directory)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(0, 0, text, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::CLICKABLE);
    CUIEdit* edit = new CUIEdit(0, 0, 1, GetOptionsDB().Get<std::string>(option_name));
    edit->SetMaxSize(GG::Pt(edit->MaxSize().x, edit->Size().y));
    CUIButton* button = new CUIButton(0, 0, 1, "...");
    button->SetMinSize(GG::Pt(button->MinUsableSize().x + 8, button->Height()));
    button->SetMaxSize(GG::Pt(button->MaxSize().x, button->Height()));
    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 2, 2, 0, LAYOUT_MARGIN);
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
    GG::Connect(button->ClickedSignal, BrowseForPathButtonFunctor(path, filters, edit, directory));
    if (string_validator && !string_validator(edit->WindowText()))
        edit->SetTextColor(GG::CLR_RED);
}

void OptionsWnd::FileOption(const std::string& option_name, const std::string& text, const fs::path& path, StringValidator string_validator/* = 0*/)
{
    FileOption(option_name, text, path, std::vector<std::pair<std::string, std::string> >(), string_validator);
}

void OptionsWnd::FileOption(const std::string& option_name, const std::string& text, const fs::path& path, const std::pair<std::string, std::string>& filter, StringValidator string_validator/* = 0*/)
{
    FileOption(option_name, text, path, std::vector<std::pair<std::string, std::string> >(1, filter), string_validator);
}

void OptionsWnd::FileOption(const std::string& option_name, const std::string& text, const fs::path& path, const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator/* = 0*/)
{
    FileOptionImpl(option_name, text, path, filters, string_validator, false);
}

void OptionsWnd::SoundFileOption(const std::string& option_name, const std::string& text)
{
    FileOption(option_name, text, ClientUI::SoundDir(), std::make_pair(UserString("OPTIONS_SOUND_FILE"), "*" + SOUND_FILE_SUFFIX), ValidSoundFile);
}

void OptionsWnd::DirectoryOption(const std::string& option_name, const std::string& text, const boost::filesystem::path& path)
{
    FileOptionImpl(option_name, text, path, std::vector<std::pair<std::string, std::string> >(), ValidDirectory, true);
}

void OptionsWnd::ColorOption(const std::string& option_name, const std::string& text)
{
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(0, 0, text, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::CLICKABLE);
    ColorSelector* color_selector = new ColorSelector(0, 0, 1, ClientUI::Pts() + 4, GetOptionsDB().Get<StreamableColor>(option_name).ToClr());
    color_selector->SetMaxSize(GG::Pt(color_selector->MaxSize().x, color_selector->Size().y));
    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 1, 2);
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

void OptionsWnd::FontOption(const std::string& option_name, const std::string& text)
{
    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;
    const int DROPLIST_DROP_HEIGHT = DROPLIST_HEIGHT * 5;
    GG::ListBox::Row* row = new GG::ListBox::Row();
    GG::TextControl* text_control = new GG::TextControl(0, 0, text, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::CLICKABLE);
    CUIDropDownList* drop_list = new CUIDropDownList(0, 0, 1, DROPLIST_HEIGHT, DROPLIST_DROP_HEIGHT);
    drop_list->SetStyle(GG::LIST_NOSORT);
    std::set<std::string> filenames;
    fs::directory_iterator end_it;
    for (fs::directory_iterator it(GetSettingsDir()); it != end_it; ++it) {
        try {
            if (fs::exists(*it)) {
                std::string filename = it->leaf();
                if (boost::algorithm::ends_with(filename, FONT_SUFFIX))
                    filenames.insert(filename);
            }
        } catch (const fs::filesystem_error& e) {
            // ignore files for which permission is denied, and rethrow other exceptions
            if (e.system_error() != EACCES)
                throw;
        }
    }
    drop_list->SetMaxSize(GG::Pt(drop_list->MaxSize().x, drop_list->Size().y));
    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 2, 1, 0, LAYOUT_MARGIN);
    layout->Add(text_control, 0, 0);
    layout->Add(drop_list, 1, 0, 1, 1, GG::ALIGN_VCENTER);
    row->Resize(GG::Pt(ROW_WIDTH, text_control->MinUsableSize().y + LAYOUT_MARGIN + drop_list->MaxSize().y + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    std::string current_font = GetOptionsDB().Get<std::string>(option_name);
    assert(boost::algorithm::ends_with(current_font, FONT_SUFFIX));
    int index = -1;
    for (std::set<std::string>::const_iterator it = filenames.begin(); it != filenames.end(); ++it) {
        GG::ListBox::Row* font_row = new CUISimpleDropDownListRow(boost::algorithm::erase_last_copy(*it, FONT_SUFFIX));
        font_row->SetText(*it);
        drop_list->Insert(font_row);
        if (*it == current_font)
            index = drop_list->NumRows() - 1;
    }
    if (index != -1)
        drop_list->Select(index);
    drop_list->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    drop_list->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    text_control->SetBrowseText(UserString(GetOptionsDB().GetDescription(option_name)));
    GG::Connect(drop_list->SelChangedSignal, DropListIndexSetOptionFunctor(option_name, drop_list));
}

void OptionsWnd::ResolutionOption()
{
    // Retrieve (and if necessary generate) the fullscreen resolutions.
    std::vector<std::string> resolutions;
    boost::shared_ptr<const RangedValidator<int> > width_validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(GetOptionsDB().GetValidator("app-width"));
    boost::shared_ptr<const RangedValidator<int> > height_validator = boost::dynamic_pointer_cast<const RangedValidator<int> >(GetOptionsDB().GetValidator("app-height"));
    std::string current_resolution_string;
    int current_resolution_index = -1;
    Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();
    assert(render_system);
    Ogre::ConfigOptionMap& current_renderer_options = render_system->getConfigOptions();
    Ogre::ConfigOptionMap::iterator end_it = current_renderer_options.end();
    for (Ogre::ConfigOptionMap::iterator it = current_renderer_options.begin(); it != end_it; ++it) {
        if (it->first == "Video Mode") {
            current_resolution_string = it->second.currentValue;
            for (unsigned int i = 0; i < it->second.possibleValues.size(); ++i) {
                resolutions.push_back(it->second.possibleValues[i]);
                if (resolutions.back().substr(0, current_resolution_string.size()) == current_resolution_string)
                    current_resolution_index = resolutions.size() - 1;
                if (resolutions.back().find_first_of("@") == std::string::npos)
                    resolutions.back() += " @ 32";
            }
        }
    } 

    // create controls
    GG::ListBox::Row* row = new GG::ListBox::Row();
    const int DROPLIST_HEIGHT = ClientUI::Pts() + 4;
    const int DROPLIST_DROP_HEIGHT = DROPLIST_HEIGHT * 10;
    GG::TextControl* drop_list_text_control = new GG::TextControl(0, 0, UserString("OPTIONS_VIDEO_MODE"), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT, GG::CLICKABLE);
    CUIDropDownList* drop_list = new CUIDropDownList(0, 0, 1, DROPLIST_HEIGHT, DROPLIST_DROP_HEIGHT);
    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 2, 1, 0, LAYOUT_MARGIN);
    drop_list->SetMaxSize(GG::Pt(drop_list->MaxSize().x, drop_list->Size().y));
    layout->Add(drop_list_text_control, 0, 0);
    layout->Add(drop_list, 1, 0, 1, 1, GG::ALIGN_VCENTER);
    row->Resize(GG::Pt(ROW_WIDTH, drop_list_text_control->MinUsableSize().y + LAYOUT_MARGIN + drop_list->MaxSize().y + 6));
    row->push_back(new RowContentsWnd(row->Width(), row->Height(), layout, m_indentation_level));
    m_current_option_list->Insert(row);
    drop_list->SetStyle(GG::LIST_NOSORT);
    GG::ListBox::Row* font_row = new CUISimpleDropDownListRow(UserString("OPTIONS_VIDEO_MODE_LIST_CUSTOM_OPTION"));
    font_row->SetText(UserString("OPTIONS_VIDEO_MODE_LIST_CUSTOM_OPTION"));
    drop_list->Insert(font_row);
    for (std::vector<std::string>::const_iterator it = resolutions.begin(); it != resolutions.end(); ++it) {
        font_row = new CUISimpleDropDownListRow(*it);
        font_row->SetText(*it);
        drop_list->Insert(font_row);
    }
    if (current_resolution_index != -1 && GetOptionsDB().Get<bool>("fullscreen"))
        drop_list->Select(current_resolution_index + 1);
    else
        drop_list->Select(0);
    drop_list->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    drop_list->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_LIST_DESCRIPTION"));
    drop_list_text_control->SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
    drop_list_text_control->SetBrowseText(UserString("OPTIONS_VIDEO_MODE_LIST_DESCRIPTION"));

    CUISpin<int>* width_spin = IntOption("app-width", UserString("OPTIONS_APP_WIDTH"));
    CUISpin<int>* height_spin = IntOption("app-height", UserString("OPTIONS_APP_HEIGHT"));
    CUISpin<int>* color_depth_spin = IntOption("color-depth", UserString("OPTIONS_COLOR_DEPTH"));
    BoolOption("show-fps", UserString("OPTIONS_SHOW_FPS"));
    CUIStateButton* fullscreen_button = 0;
#ifndef FREEORION_WIN32
    fullscreen_button = BoolOption("fullscreen", UserString("OPTIONS_FULLSCREEN"));
#endif
    CUIStateButton* limit_FPS_button = BoolOption("limit-fps", UserString("OPTIONS_LIMIT_FPS"));
    CUISpin<double>* max_fps_spin = DoubleOption("max-fps", UserString("OPTIONS_MAX_FPS"));
    GG::Connect(limit_FPS_button->CheckedSignal, LimitFPSSetOptionFunctor(max_fps_spin));
    limit_FPS_button->SetCheck(GetOptionsDB().Get<bool>("limit-fps"));

    GG::Connect(drop_list->SelChangedSignal,
                ResolutionDropListIndexSetOptionFunctor(drop_list, width_spin, height_spin, color_depth_spin, fullscreen_button));
}

void OptionsWnd::Init()
{
    bool UI_sound_enabled = GetOptionsDB().Get<bool>("UI.sound.enabled");

    Sound::TempUISoundDisabler sound_disabler;

    GG::Layout* layout = new GG::Layout(0, 0, 1, 1, 2, 2, LAYOUT_MARGIN, LAYOUT_MARGIN);
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
    FileOption("bg-music", UserString("OPTIONS_BACKGROUND_MUSIC"), ClientUI::SoundDir(),
               std::make_pair(UserString("OPTIONS_MUSIC_FILE"), "*" + MUSIC_FILE_SUFFIX),
               ValidMusicFile);
    EndSection();
    BeginSection(UserString("OPTIONS_SOUNDS"));
    BeginSection(UserString("OPTIONS_UI_SOUNDS"));
    SoundFileOption("UI.sound.alert", UserString("OPTIONS_SOUND_ALERT"));
    SoundFileOption("UI.sound.text-typing", UserString("OPTIONS_SOUND_TYPING"));
    EndSection();
    BeginSection(UserString("OPTIONS_SOUND_WINDOW"));
    SoundFileOption("UI.sound.window-close", UserString("OPTIONS_SOUND_CLOSE"));
    SoundFileOption("UI.sound.window-maximize", UserString("OPTIONS_SOUND_MAXIMIZE"));
    SoundFileOption("UI.sound.window-minimize", UserString("OPTIONS_SOUND_MINIMIZE"));
    SoundFileOption("UI.sound.sidepanel-open", UserString("OPTIONS_SOUND_SIDEPANEL"));
    EndSection();
    BeginSection(UserString("OPTIONS_SOUND_LIST"));
    SoundFileOption("UI.sound.item-drop", UserString("OPTIONS_SOUND_DROP"));
    SoundFileOption("UI.sound.list-pulldown", UserString("OPTIONS_SOUND_PULLDOWN"));
    SoundFileOption("UI.sound.list-select", UserString("OPTIONS_SOUND_SELECT"));
    EndSection();
    BeginSection(UserString("OPTIONS_SOUND_BUTTON"));
    SoundFileOption("UI.sound.button-click", UserString("OPTIONS_SOUND_CLICK"));
    SoundFileOption("UI.sound.button-rollover", UserString("OPTIONS_SOUND_ROLLOVER"));
    SoundFileOption("UI.sound.fleet-button-click", UserString("OPTIONS_SOUND_FLEET_CLICK"));
    SoundFileOption("UI.sound.fleet-button-rollover", UserString("OPTIONS_SOUND_FLEET_ROLLOVER"));
    SoundFileOption("UI.sound.system-icon-rollover", UserString("OPTIONS_SOUND_SYSTEM_ROLLOVER"));
    SoundFileOption("UI.sound.turn-button-click", UserString("OPTIONS_SOUND_TURN"));
    SoundFileOption("UI.sound.planet-button-click", UserString("OPTIONS_SOUND_PLANET"));
    EndSection();
    BeginSection(UserString("OPTIONS_SOUND_FOCUS"));
    SoundFileOption("UI.sound.balanced-focus", UserString("OPTIONS_SOUND_BALANCED"));
    SoundFileOption("UI.sound.farming-focus", UserString("OPTIONS_SOUND_FARMING"));
    SoundFileOption("UI.sound.industry-focus", UserString("OPTIONS_SOUND_INDUSTRY"));
    SoundFileOption("UI.sound.mining-focus", UserString("OPTIONS_SOUND_MINING"));
    SoundFileOption("UI.sound.research-focus", UserString("OPTIONS_SOUND_RESEARCH"));
    EndSection();
    EndSection();
    EndPage();

    // UI settings tab
    BeginPage(UserString("OPTIONS_PAGE_UI"));
    BeginSection(UserString("OPTIONS_MISC_UI"));
    BoolOption("UI.fleet-autoselect", UserString("OPTIONS_AUTOSELECT_FLEET"));
    BoolOption("UI.multiple-fleet-windows", UserString("OPTIONS_MULTIPLE_FLEET_WNDS"));
    BoolOption("UI.window-quickclose", UserString("OPTIONS_QUICK_CLOSE_WNDS"));
    FileOption("stringtable-filename", UserString("OPTIONS_LANGUAGE"), GetSettingsDir(), std::make_pair(UserString("OPTIONS_LANGUAGE_FILE"), "*" + STRINGTABLE_FILE_SUFFIX), &ValidStringtableFile);
    IntOption("UI.tooltip-delay", UserString("OPTIONS_TOOLTIP_DELAY"));
    EndSection();
    BeginSection(UserString("OPTIONS_FONTS"));
    FontOption("UI.font", UserString("OPTIONS_FONT_TEXT"));
    FontOption("UI.title-font", UserString("OPTIONS_FONT_TITLE"));
    EndSection();
    BeginSection(UserString("OPTIONS_FONT_SIZES"));
    IntOption("UI.font-size", UserString("OPTIONS_FONT_TEXT"));
    IntOption("UI.title-font-size", UserString("OPTIONS_FONT_TITLE"));
    EndSection();
    BeginSection(UserString("OPTIONS_TECH_SPACING"));
    DoubleOption("UI.tech-layout-horz-spacing", UserString("OPTIONS_HORIZONTAL"));
    DoubleOption("UI.tech-layout-vert-spacing", UserString("OPTIONS_VERTICAL"));
    EndSection();
    BeginSection(UserString("OPTIONS_CHAT"));
    IntOption("UI.chat-edit-history", UserString("OPTIONS_CHAT_HISTORY"));
    IntOption("UI.chat-hide-interval", UserString("OPTIONS_CHAT_HIDE"));
    EndSection();
    BeginSection(UserString("OPTIONS_GALAXY_MAP"));
    BoolOption("UI.galaxy-gas-background", UserString("OPTIONS_GALAXY_MAP_GAS"));
    BoolOption("UI.optimized-system-rendering", UserString("OPTIONS_OPTIMIZED_SYSTEM_RENDERING"));
    EndPage();

    // Colors tab
    BeginPage(UserString("OPTIONS_PAGE_COLORS"));
    BeginSection(UserString("OPTIONS_GENERAL_COLORS"));
    ColorOption("UI.text-color", UserString("OPTIONS_TEXT_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_WINDOW_COLORS"));
    ColorOption("UI.ctrl-color", UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.ctrl-border-color", UserString("OPTIONS_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_CONTROL_COLORS"));
    ColorOption("UI.wnd-color", UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.wnd-border-color", UserString("OPTIONS_BORDER_COLOR"));
    ColorOption("UI.wnd-inner-border-color", UserString("OPTIONS_INNER_BORDER_COLOR"));
    ColorOption("UI.wnd-outer-border-color", UserString("OPTIONS_OUTER_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_EDIT_COLORS"));
    ColorOption("UI.edit-hilite", UserString("OPTIONS_HIGHLIGHT_COLOR"));
    ColorOption("UI.edit-interior", UserString("OPTIONS_INTERIOR_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_TECH_COLORS"));
    BeginSection(UserString("OPTIONS_KNOWN_TECH_COLORS"));
    ColorOption("UI.known-tech", UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.known-tech-border", UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_RESEARCHABLE_TECH_COLORS"));
    ColorOption("UI.researchable-tech", UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.researchable-tech-border", UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_UNRESEARCHABLE_TECH_COLORS"));
    ColorOption("UI.unresearchable-tech", UserString("OPTIONS_FILL_COLOR"));
    ColorOption("UI.unresearchable-tech-border", UserString("OPTIONS_TEXT_AND_BORDER_COLOR"));
    EndSection();
    BeginSection(UserString("OPTIONS_TECH_PROGRESS_COLORS"));
    ColorOption("UI.tech-progress", UserString("OPTIONS_PROGRESS_BAR_COLOR"));
    ColorOption("UI.tech-progress-background", UserString("OPTIONS_PROGRESS_BACKGROUND_COLOR"));
    EndSection();
    EndSection();
    EndPage();

    // combat settings tab
    BeginPage(UserString("OPTIONS_PAGE_COMBAT"));
    BoolOption("combat.enable-glow", UserString("OPTIONS_COMBAT_ENABLE_GLOW"));
    BoolOption("combat.filled-selection", UserString("OPTIONS_COMBAT_FILLED_SELECTION"));
    EndPage();

    // Misc. settings tab
    BeginPage(UserString("OPTIONS_PAGE_AUTOSAVE"));
    BoolOption("autosave.single-player", UserString("OPTIONS_SINGLEPLAYER"));
    BoolOption("autosave.multiplayer", UserString("OPTIONS_MULTIPLAYER"));
    IntOption("autosave.saves", UserString("OPTIONS_AUTOSAVE_TO_KEEP"));
    IntOption("autosave.turns", UserString("OPTIONS_AUTOSAVE_TURNS_BETWEEN"));
    EndPage();

    // Directories tab
    BeginPage(UserString("OPTIONS_PAGE_DIRECTORIES"));
    DirectoryOption("settings-dir", UserString("OPTIONS_FOLDER_SETTINGS"), GetGlobalDir());
    DirectoryOption("save-dir", UserString("OPTIONS_FOLDER_SAVE"), GetGlobalDir());
    EndPage();

    // Connect the done and cancel button
    GG::Connect(m_done_button->ClickedSignal, &OptionsWnd::DoneClicked, this);
}

OptionsWnd::~OptionsWnd()
{}

void OptionsWnd::KeyPress (GG::Key key, GG::Flags<GG::ModKey> mod_keys)
{
    if (key == GG::GGK_ESCAPE || key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) // Same behaviour as if "done" was pressed
        DoneClicked();
}

void OptionsWnd::DoneClicked()
{
    // Save the changes:
    fs::ofstream ofs(GetConfigPath());
    GetOptionsDB().GetXML().WriteDoc(ofs);
    m_done = true;
}

void OptionsWnd::MusicClicked(bool checked)
{
    if (!checked)
    {
        GetOptionsDB().Set("music-off", true);
        Sound::GetSound().StopMusic();
    }
    else
    {
        GetOptionsDB().Set("music-off", false);
        Sound::GetSound().PlayMusic(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("bg-music"), -1);
    }
}

void OptionsWnd::MusicVolumeSlid(int pos, int low, int high)
{
    GetOptionsDB().Set("music-volume", pos);
    Sound::GetSound().SetMusicVolume(pos);
}

void OptionsWnd::UISoundsVolumeSlid(int pos, int low, int high)
{
    GetOptionsDB().Set("UI.sound.volume", pos);
    Sound::GetSound().SetUISoundsVolume(pos);
    Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.button-click"), true);
}
