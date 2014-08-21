/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

#include <GG/dialogs/FileDlg.h>

#include <GG/GUI.h>
#include <GG/Button.h>
#include <GG/Edit.h>
#include <GG/DropDownList.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/WndEvent.h>
#include <GG/utf8/checked.h>

#include <GG/dialogs/ThreeButtonDlg.h>

// HACK! MSVC #defines int64_t to be __int64, which breaks the code in boost's cstdint.hpp
#ifdef int64_t
#undef int64_t
#endif

#include <boost/cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_dynamic.hpp>
#include <boost/system/system_error.hpp>


using namespace GG;

namespace {
    using namespace boost::spirit::classic;

    // these functors are used by the if_p, while_p, and for_p parsers in UpdateList()
    struct LeadingWildcard
    {
        LeadingWildcard(const std::string& str) : m_value(!str.empty() && *str.begin() == '*') {}
        bool operator()() const {return m_value;}
        bool m_value;
    };
    struct TrailingWildcard
    {
        TrailingWildcard(const std::string& str) : m_value(!str.empty() && *str.rbegin() == '*') {}
        bool operator()() const {return m_value;}
        bool m_value;
    };

    struct Index
    {
        Index(int i = 0) : m_initial_value(i) {}
        void operator()() const {value = m_initial_value;}
        int m_initial_value;
        static int value;
    };
    int Index::value;
    struct IndexLess
    {
        IndexLess(int val) : m_value(val) {}
        bool operator()() const {return Index::value <  m_value;}
        int m_value;
    };
    struct IndexIncr
    {
        void operator()() const {++Index::value;}
    };

    struct FrontStringBegin
    {
        FrontStringBegin(const boost::shared_ptr<std::vector<std::string> >& strings) : m_strings(strings) {}
        const char* operator()() const {return m_strings->front().c_str();}
        boost::shared_ptr<std::vector<std::string> > m_strings;
    };
    struct FrontStringEnd
    {
        FrontStringEnd(const boost::shared_ptr<std::vector<std::string> >& strings) : m_strings(strings) {}
        const char* operator()() const {return m_strings->front().c_str() + m_strings->front().size();}
        boost::shared_ptr<std::vector<std::string> > m_strings;
    };
    struct IndexedStringBegin
    {
        IndexedStringBegin(const boost::shared_ptr<std::vector<std::string> >& strings) : m_strings(strings) {}
        const char* operator()() const {return (*m_strings)[Index::value].c_str();}
        boost::shared_ptr<std::vector<std::string> > m_strings;
    };
    struct IndexedStringEnd
    {
        IndexedStringEnd(const boost::shared_ptr<std::vector<std::string> >& strings) : m_strings(strings) {}
        const char* operator()() const {return (*m_strings)[Index::value].c_str() + (*m_strings)[Index::value].size();}
        boost::shared_ptr<std::vector<std::string> > m_strings;
    };

    bool WindowsRoot(const std::string& root_name)
    { return root_name.size() == 2 && std::isalpha(root_name[0]) && root_name[1] == ':'; }

    bool Win32Paths()
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    { return WindowsRoot(boost::filesystem::initial_path().root_name().string()); }
#else
    { return WindowsRoot(boost::filesystem::initial_path().root_name()); }
#endif

    const X H_SPACING(10);
    const Y V_SPACING(10);
}

namespace fs = boost::filesystem;

// static member definition(s)
fs::path FileDlg::s_working_dir = fs::current_path();
const X FileDlg::DEFAULT_WIDTH(500);
const Y FileDlg::DEFAULT_HEIGHT(450);


FileDlg::FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                 const boost::shared_ptr<Font>& font, Clr color, Clr border_color, Clr text_color/* = CLR_BLACK*/) : 
    Wnd((GUI::GetGUI()->AppWidth() - DEFAULT_WIDTH) / 2,
        (GUI::GetGUI()->AppHeight() - DEFAULT_HEIGHT) / 2,
        DEFAULT_WIDTH, DEFAULT_HEIGHT, INTERACTIVE | DRAGABLE | MODAL),
    m_color(color),
    m_border_color(border_color),
    m_text_color(text_color),
    m_font(font),
    m_save(save),
    m_select_directories(false),
    m_append_missing_save_extension(false),
    m_in_win32_drive_selection(false),
    m_save_str("Save"),
    m_open_str("Open"),
    m_cancel_str("Cancel"),
    m_malformed_filename_str("Invalid file name."),
    m_overwrite_prompt_str("%1% exists.\nOk to overwrite it?"),
    m_invalid_filename_str("\"%1%\"\nis an invalid file name."),
    m_filename_is_a_directory_str("\"%1%\"\nis a directory."),
    m_file_does_not_exist_str("File \"%1%\"\ndoes not exist."),
    m_device_is_not_ready_str("Device is not ready."),
    m_three_button_dlg_ok_str("Ok"),
    m_three_button_dlg_cancel_str("Cancel"),
    m_curr_dir_text(0),
    m_files_list(0),
    m_files_edit(0),
    m_filter_list(0),
    m_ok_button(0),
    m_cancel_button(0),
    m_files_label(0),
    m_file_types_label(0)
{
    CreateChildren(multi);
    Init(directory);

    if (!filename.empty()) {
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    fs::path filename_path = fs::system_complete(fs::path(filename));
    m_files_edit->SetText(filename_path.leaf().string());
#else
    fs::path filename_path = fs::complete(fs::path(filename));
    m_files_edit->SetText(filename_path.leaf());
#endif
    }
}

std::set<std::string> FileDlg::Result() const
{ return m_result; }

bool FileDlg::SelectDirectories() const
{ return m_select_directories; }

bool FileDlg::AppendMissingSaveExtension() const
{ return m_append_missing_save_extension; }

const std::string& FileDlg::FilesString() const
{ return m_files_label->Text(); }

const std::string& FileDlg::FileTypesString() const
{ return m_file_types_label->Text(); }

const std::string& FileDlg::SaveString() const
{ return m_save_str; }

const std::string& FileDlg::OpenString() const
{ return m_open_str; }

const std::string& FileDlg::CancelString() const
{ return m_cancel_str; }

const std::string& FileDlg::MalformedFilenameString() const
{ return m_malformed_filename_str; }

const std::string& FileDlg::OverwritePromptString() const
{ return m_overwrite_prompt_str; }

const std::string& FileDlg::InvalidFilenameString() const
{ return m_invalid_filename_str; }

const std::string& FileDlg::FilenameIsADirectoryString() const
{ return m_filename_is_a_directory_str; }

const std::string& FileDlg::FileDoesNotExistString() const
{ return m_file_does_not_exist_str; }

const std::string& FileDlg::DeviceIsNotReadyString() const
{ return m_device_is_not_ready_str; }

const std::string& FileDlg::ThreeButtonDlgOKString() const
{ return m_three_button_dlg_ok_str; }

const std::string& FileDlg::ThreeButtonDlgCancelString() const
{ return m_three_button_dlg_cancel_str; }

void FileDlg::Render()
{
    FlatRectangle(UpperLeft(), LowerRight(), m_color, m_border_color, 1);
    try {
        fs::directory_iterator test(s_working_dir);
    } catch (const fs::filesystem_error&) {
        // This ctor has been found to throw on Win32 when we attempt to iterate over a path into a drive that has just
        // been disconnected (e.g. a USB thumb drive).  In this case, we will just cancel the dialog.
        CancelClicked();
    }
}

void FileDlg::KeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (key == GGK_RETURN || key == GGK_KP_ENTER)
        OkHandler(false);
    else if (key == GGK_ESCAPE)
        CancelClicked();
}

void FileDlg::SelectDirectories(bool directories)
{
    if (!m_save) {
        bool refresh_list = directories != m_select_directories;
        m_select_directories = directories;
        if (refresh_list)
            UpdateList();
    }
}

void FileDlg::AppendMissingSaveExtension(bool append)
{ m_append_missing_save_extension = append; }

void FileDlg::SetFileFilters(const std::vector<std::pair<std::string, std::string> >& filters)
{
    m_file_filters = filters;
    PopulateFilters();
    UpdateList();
}

void FileDlg::SetFilesString(const std::string& str)
{
    m_files_label->SetText(str);
    PlaceLabelsAndEdits(Width() / 4 - H_SPACING, m_files_edit->Height());
}

void FileDlg::SetFileTypesString(const std::string& str)
{
    m_file_types_label->SetText(str);
    PlaceLabelsAndEdits(Width() / 4 - H_SPACING, m_files_edit->Height());
}

void FileDlg::SetSaveString(const std::string& str)
{
    bool set_button_text = m_ok_button->Text() == m_save_str;
    m_save_str = str;
    if (set_button_text)
        m_ok_button->SetText(m_save_str);
}

void FileDlg::SetOpenString(const std::string& str)
{
    bool set_button_text = m_ok_button->Text() == m_open_str;
    m_open_str = str;
    if (set_button_text)
        m_ok_button->SetText(m_open_str);
}

void FileDlg::SetCancelString(const std::string& str)
{
    m_cancel_str = str;
    m_cancel_button->SetText(m_cancel_str);
}

void FileDlg::SetMalformedFilenameString(const std::string& str)
{ m_malformed_filename_str = str; }

void FileDlg::SetOverwritePromptString(const std::string& str)
{ m_overwrite_prompt_str = str; }

void FileDlg::SetInvalidFilenameString(const std::string& str)
{ m_invalid_filename_str = str; }

void FileDlg::SetFilenameIsADirectoryString(const std::string& str)
{ m_filename_is_a_directory_str = str; }

void FileDlg::SetFileDoesNotExistString(const std::string& str)
{ m_file_does_not_exist_str = str; }

void FileDlg::SetDeviceIsNotReadyString(const std::string& str)
{ m_device_is_not_ready_str = str; }

void FileDlg::SetThreeButtonDlgOKString(const std::string& str)
{ m_three_button_dlg_ok_str = str; }

void FileDlg::SetThreeButtonDlgCancelString(const std::string& str)
{ m_three_button_dlg_cancel_str = str; }

const fs::path& FileDlg::WorkingDirectory()
{ return s_working_dir; }

void FileDlg::CreateChildren(bool multi)
{
    if (m_save)
        multi = false;

    const X USABLE_WIDTH = Width() - 4 * H_SPACING;
    const X BUTTON_WIDTH = USABLE_WIDTH / 4;

    boost::shared_ptr<StyleFactory> style = GetStyleFactory();

    m_files_edit = style->NewEdit("", m_font, m_border_color, m_text_color);
    m_filter_list = style->NewDropDownList(3, m_border_color);
    m_filter_list->SetStyle(LIST_NOSORT);
    m_files_edit->Resize(Pt(X(100), m_font->Height() + 2 * 5));
    m_files_edit->MoveTo(Pt());
    m_filter_list->Resize(Pt(X(100), m_font->Lineskip()));
    m_filter_list->MoveTo(Pt());

    const Y BUTTON_HEIGHT = m_files_edit->Height(); // use the edit's height for the buttons as well

    m_curr_dir_text = style->NewTextControl("", m_font, m_text_color, FORMAT_NOWRAP);
    m_files_label = style->NewTextControl(X0, Height() - (BUTTON_HEIGHT + V_SPACING) * 2, Width() - (3 * BUTTON_WIDTH + 3 * H_SPACING), BUTTON_HEIGHT, "File(s):", m_font, m_text_color, FORMAT_RIGHT | FORMAT_VCENTER);
    m_file_types_label = style->NewTextControl(X0, Height() - (BUTTON_HEIGHT + V_SPACING) * 1, Width() - (3 * BUTTON_WIDTH + 3 * H_SPACING), BUTTON_HEIGHT, "Type(s):", m_font, m_text_color, FORMAT_RIGHT | FORMAT_VCENTER);

    m_ok_button = style->NewButton(m_save ? m_save_str : m_open_str, m_font, m_color, m_text_color);
    m_cancel_button = style->NewButton(m_cancel_str, m_font, m_color, m_text_color);

    m_ok_button->Resize(Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_ok_button->MoveTo(Pt(Width() - (BUTTON_WIDTH + H_SPACING), Height() - (BUTTON_HEIGHT + V_SPACING) * 2));
    m_cancel_button->Resize(Pt(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_cancel_button->MoveTo(Pt(Width() - (BUTTON_WIDTH + H_SPACING), Height() - (BUTTON_HEIGHT + V_SPACING)));

    // finally, we can create the listbox with the files in it, sized to fill the available space
    m_files_list = style->NewListBox(m_border_color);
    m_files_list->SetStyle(LIST_NOSORT | (multi ? LIST_NONE : LIST_SINGLESEL));

    PlaceLabelsAndEdits(BUTTON_WIDTH, BUTTON_HEIGHT);
}

void FileDlg::PlaceLabelsAndEdits(X button_width, Y button_height)
{
    m_curr_dir_text->MoveTo(GG::Pt(H_SPACING, V_SPACING / 2));

    Y file_list_top = m_curr_dir_text->Height() + V_SPACING;
    m_files_list->Resize(Pt(Width() - 2 * H_SPACING,
                            Height() - (button_height + V_SPACING) * 2 - file_list_top - V_SPACING));
    m_files_list->MoveTo(Pt(H_SPACING, file_list_top));

    // determine the space needed to display both text labels in the chosen font; use this to expand the edit as far as
    // possible
    X labels_width = std::max(m_font->TextExtent(m_files_label->Text()).x, 
                              m_font->TextExtent(m_file_types_label->Text()).x) + H_SPACING;
    m_files_label->Resize(Pt(labels_width - H_SPACING / 2, m_files_label->Height()));
    m_file_types_label->Resize(Pt(labels_width - H_SPACING / 2, m_file_types_label->Height()));
    m_files_edit->SizeMove(Pt(labels_width, Height() - (button_height + V_SPACING) * 2),
                           Pt(Width() - (button_width + 2 * H_SPACING), Height() - (button_height + 2 * V_SPACING)));
    m_filter_list->SizeMove(Pt(labels_width, Height() - (button_height + V_SPACING)),
                            Pt(Width() - (button_width + 2 * H_SPACING), Height() - V_SPACING));
}

void FileDlg::Init(const std::string& directory)
{
    AttachChild(m_files_edit);
    AttachChild(m_filter_list);
    AttachChild(m_ok_button);
    AttachChild(m_cancel_button);
    AttachChild(m_files_list);
    AttachChild(m_curr_dir_text);
    AttachChild(m_files_label);
    AttachChild(m_file_types_label);

    if (directory != "") {
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    #if defined(_WIN32)
        // convert UTF-8 file name to UTF-16
        boost::filesystem::path::string_type directory_native;
        utf8::utf8to16(directory.begin(), directory.end(), std::back_inserter(directory_native));
        fs::path dir_path = fs::system_complete(fs::path(directory_native));
    #else
        fs::path dir_path = fs::system_complete(fs::path(directory));
    #endif
#else
        fs::path dir_path = fs::complete(fs::path(directory));
#endif
        if (!fs::exists(dir_path))
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
            throw BadInitialDirectory("FileDlg::Init() : Initial directory \"" + dir_path.string() + "\" does not exist.");
#else
            throw BadInitialDirectory("FileDlg::Init() : Initial directory \"" + dir_path.native_directory_string() + "\" does not exist.");
#endif
        SetWorkingDirectory(dir_path);
    }

    UpdateDirectoryText();
    PopulateFilters();
    UpdateList();
    ConnectSignals();
}

void FileDlg::ConnectSignals()
{
    Connect(m_ok_button->LeftClickedSignal,     &FileDlg::OkClicked,        this);
    Connect(m_cancel_button->LeftClickedSignal, &FileDlg::CancelClicked,    this);
    Connect(m_files_list->SelChangedSignal,     &FileDlg::FileSetChanged,   this);
    Connect(m_files_list->DoubleClickedSignal,  &FileDlg::FileDoubleClicked,this);
    Connect(m_files_edit->EditedSignal,         &FileDlg::FilesEditChanged, this);
    Connect(m_filter_list->SelChangedSignal,    &FileDlg::FilterChanged,    this);
}

void FileDlg::OkClicked()
{ OkHandler(false); }

void FileDlg::OkHandler(bool double_click)
{
    bool results_valid = false;

    // parse contents of edit control to determine file names
    m_result.clear();

    std::vector<std::string> files;
    parse(m_files_edit->Text().c_str(), (+anychar_p)[append(files)], space_p);
    std::sort(files.begin(), files.end());

    boost::shared_ptr<StyleFactory> style = GetStyleFactory();

    if (m_save) { // file save case
        if (m_ok_button->Text() != m_save_str) {
            OpenDirectory();
        } else if (files.size() == 1) {
            results_valid = true;
            std::string save_file = *files.begin();
            if (m_append_missing_save_extension &&
                m_file_filters.size() == 1 &&
                std::count(m_file_filters[0].second.begin(), m_file_filters[0].second.end(), '*') == 1 &&
                m_file_filters[0].second[0] == '*' &&
                !boost::algorithm::ends_with(save_file, m_file_filters[0].second.substr(1)))
            {
                save_file += m_file_filters[0].second.substr(1);
            }
#if defined(_WIN32)
            // convert UTF-8 file name to UTF-16
            boost::filesystem::path::string_type file_name_native;
            utf8::utf8to16(save_file.begin(), save_file.end(), std::back_inserter(file_name_native));
            fs::path p = s_working_dir / fs::path(file_name_native);
#else
            fs::path p = s_working_dir / fs::path(save_file);
#endif

#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    #if defined (_WIN32)
            // convert UTF-16 path back to UTF-8 for storage
            boost::filesystem::path::string_type path_native = p.native();
            std::string path_string;
            utf8::utf16to8(path_native.begin(), path_native.end(), std::back_inserter(path_string));
            m_result.insert(path_string);
    #else
            m_result.insert(p.string());
    #endif
#else
            m_result.insert(m_select_directories ? p.native_directory_string() : p.native_file_string());
#endif
            // check to see if file already exists; if so, ask if it's ok to overwrite
            if (fs::exists(p)) {
                std::string msg_str = boost::str(boost::format(m_overwrite_prompt_str) % save_file);
                boost::shared_ptr<ThreeButtonDlg> dlg(
                    style->NewThreeButtonDlg(X(300), Y(125), msg_str, m_font, m_color, m_border_color, m_color, m_text_color,
                                             2, m_three_button_dlg_ok_str, m_three_button_dlg_cancel_str));
                dlg->Run();
                results_valid = (dlg->Result() == 0);
            }
        }
    } else { // file open case
        if (files.empty() || (m_select_directories && double_click)) {
            OpenDirectory();
        } else { // ensure the file(s) are valid before returning them
            for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
#if defined(_WIN32)
                // convert UTF-8 file name to UTF-16
                const std::string& file_name = *it;
                boost::filesystem::path::string_type file_name_native;
                utf8::utf8to16(file_name.begin(), file_name.end(), std::back_inserter(file_name_native));
                fs::path p = s_working_dir / fs::path(file_name_native);
#else
                fs::path p = s_working_dir / fs::path(*it);
#endif
                if (fs::exists(p)) {
                    bool p_is_directory = fs::is_directory(p);
                    if (!m_select_directories && p_is_directory) {
                        std::string msg_str = boost::str(boost::format(m_filename_is_a_directory_str) % (*it));
                        boost::shared_ptr<ThreeButtonDlg> dlg(
                            style->NewThreeButtonDlg(X(300), Y(125), msg_str, m_font, m_color, m_border_color, m_color,
                                                     m_text_color, 1, m_three_button_dlg_ok_str));
                        dlg->Run();
                        results_valid = false;
                        break;
                    }
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    #if defined(_WIN32)
                    // convert UTF-16 path string to UTF-8
                    std::string temp;
                    boost::filesystem::path::string_type file_name_native = p.native();
                    utf8::utf16to8(file_name_native.begin(), file_name_native.end(), std::back_inserter(temp));
                    m_result.insert(temp);
    #else
                    m_result.insert(p.string());
    #endif
#else
                    m_result.insert(p_is_directory ? p.native_directory_string() : p.native_file_string());
#endif
                    results_valid = true; // indicate validity only if at least one good file was found
                } else {
                    std::string msg_str = boost::str(boost::format(m_file_does_not_exist_str) % (*it));
                    boost::shared_ptr<ThreeButtonDlg> dlg(
                        style->NewThreeButtonDlg(X(300), Y(125), msg_str, m_font, m_color, m_border_color, m_color,
                                                 m_text_color, 1, m_three_button_dlg_ok_str));
                    dlg->Run();
                    results_valid = false;
                    break;
                }
            }
        }
    }
    if (results_valid)
        m_done = true;
}

void FileDlg::CancelClicked()
{
    m_done = true;
    m_result.clear();
}

void FileDlg::FileSetChanged(const ListBox::SelectionSet& files)
{
    std::string all_files;
    bool dir_selected = false;
    for (ListBox::SelectionSet::const_iterator it = files.begin(); it != files.end(); ++it) {
        std::string filename = boost::polymorphic_downcast<TextControl*>((***it)[0])->Text();
        if (filename[0] != '[') {
            if (!all_files.empty())
                all_files += " ";
            all_files += filename;
        } else {
            if (m_select_directories) {
                if (!all_files.empty())
                    all_files += " ";
                all_files += filename.substr(1, filename.size() - 2);
            }
            dir_selected = true;
        }
    }
    *m_files_edit << all_files;
    if (m_save && !dir_selected && m_ok_button->Text() != m_save_str)
        m_ok_button->SetText(m_save_str);
    else if (m_save && dir_selected && m_ok_button->Text() == m_save_str)
        m_ok_button->SetText(m_open_str);
}

void FileDlg::FileDoubleClicked(DropDownList::iterator it)
{
    m_files_list->DeselectAll();
    m_files_list->SelectRow(it);
    FileSetChanged(m_files_list->Selections());
    OkHandler(true);
}

void FileDlg::FilesEditChanged(const std::string& str)
{
    if (m_save && m_ok_button->Text() != m_save_str)
        m_ok_button->SetText(m_save_str);
}

void FileDlg::FilterChanged(DropDownList::iterator it)
{ UpdateList(); }

void FileDlg::SetWorkingDirectory(const fs::path& p)
{
    m_files_edit->Clear();
    FilesEditChanged(m_files_edit->Text());
    s_working_dir = p;
    UpdateDirectoryText();
    UpdateList();
}

void FileDlg::PopulateFilters()
{
    m_filter_list->Clear();
    if (m_file_filters.empty()) {
        m_file_types_label->Disable();
        m_filter_list->Disable();
    } else {
        for (std::size_t i = 0; i < m_file_filters.size(); ++i) {
            ListBox::Row* row = new ListBox::Row();
            row->push_back(GetStyleFactory()->NewTextControl(m_file_filters[i].first, m_font, m_text_color, FORMAT_NOWRAP));
            m_filter_list->Insert(row);
        }
        m_filter_list->Select(m_filter_list->begin());
        m_filter_list->SelChangedSignal(m_filter_list->CurrentItem());
    }
}

void FileDlg::UpdateList()
{
    m_files_list->Clear();
    fs::directory_iterator end_it;

    // define a wildcard ('*') as any combination of characters
    rule<> wildcard = anychar_p;

    // define file filters based on the filter strings in the filter drop list
    std::vector<rule<> > file_filters;

    DropDownList::iterator it = m_filter_list->CurrentItem();
    if (it != m_filter_list->end()) {
        std::vector<std::string> filter_specs; // the filter specifications (e.g. "*.png")
        parse(m_file_filters[std::distance(m_filter_list->begin(), it)].second.c_str(), *(!ch_p(',') >> (+(anychar_p - ','))[append(filter_specs)]), space_p);
        file_filters.resize(filter_specs.size());
        for (std::size_t i = 0; i < filter_specs.size(); ++i) {
            boost::shared_ptr<std::vector<std::string> > non_wildcards(new std::vector<std::string>); // the parts of the filter spec that are not wildcards
            parse(filter_specs[i].c_str(), *(*ch_p('*') >> (+(anychar_p - '*'))[append(*non_wildcards)]));

            if (non_wildcards->empty()) {
                file_filters[i] = *anychar_p;
            } else {
                file_filters[i] = 
                    if_p (LeadingWildcard(filter_specs[i])) [
                        *(wildcard - f_str_p(FrontStringBegin(non_wildcards), FrontStringEnd(non_wildcards))) 
                        >> f_str_p(FrontStringBegin(non_wildcards), FrontStringEnd(non_wildcards))
                    ] .else_p [
                        f_str_p(FrontStringBegin(non_wildcards), FrontStringEnd(non_wildcards))
                    ] 
                    >> for_p (Index(1), IndexLess(static_cast<int>(non_wildcards->size()) - 1), IndexIncr()) [
                        *(wildcard - f_str_p(IndexedStringBegin(non_wildcards), IndexedStringEnd(non_wildcards)))
                        >> f_str_p(IndexedStringBegin(non_wildcards), IndexedStringEnd(non_wildcards))
                    ] 
                    >> if_p (TrailingWildcard(filter_specs[i])) [
                        *wildcard
                    ];
            }
        }
    }

    if (!m_in_win32_drive_selection) {
        // parent directory selector
        if ((s_working_dir.string() != s_working_dir.root_path().string() &&
             s_working_dir.branch_path().string() != "") ||
            Win32Paths())
        {
            ListBox::Row* row = new ListBox::Row();
            row->push_back(GetStyleFactory()->NewTextControl("[..]", m_font, m_text_color, FORMAT_NOWRAP));
            m_files_list->Insert(row);
        }
        // current directory selector
        {
            ListBox::Row* row = new ListBox::Row();
            row->push_back(GetStyleFactory()->NewTextControl("[.]", m_font, m_text_color, FORMAT_NOWRAP));
            m_files_list->Insert(row);
        }
        try {
            fs::directory_iterator test(s_working_dir);
        } catch (const fs::filesystem_error&) {
            // This ctor has been found to throw on Win32 when we attempt to
            // iterate over a path into a drive that has just been disconnected
            // (e.g. a USB thumb drive).  In this case, we will just cancel the
            // dialog.
            CancelClicked();
            return;
        }
        // contained directories
        std::multimap<std::string, ListBox::Row*> sorted_rows;
        for (fs::directory_iterator it(s_working_dir); it != end_it; ++it) {
            try {
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
                if (fs::exists(*it) && fs::is_directory(*it) && it->path().filename().native()[0] != '.') {
#else
                if (fs::exists(*it) && fs::is_directory(*it) && it->filename()[0] != '.') {
#endif
                    ListBox::Row* row = new ListBox::Row();

#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    #if defined(_WIN32)
                    // convert UTF-16 path to UTF-8 for display
                    boost::filesystem::path::string_type file_name_native = it->path().filename().native();
                    std::string temp;
                    utf8::utf16to8(file_name_native.begin(), file_name_native.end(), std::back_inserter(temp));
                    std::string row_text = "[" + temp + "]";
    #else
                    std::string row_text = "[" + it->path().filename().string() + "]";
    #endif
#else
                    std::string row_text = "[" + it->filename() + "]";
#endif
                    row->push_back(GetStyleFactory()->NewTextControl(row_text, m_font, m_text_color, FORMAT_NOWRAP));
                    sorted_rows.insert(std::make_pair(row_text, row));
                }
            } catch (const fs::filesystem_error&) {
            }
        }

        std::vector<ListBox::Row*> rows;
        rows.reserve(sorted_rows.size());
        for (std::multimap<std::string, ListBox::Row*>::const_iterator it = sorted_rows.begin();
             it != sorted_rows.end(); ++it)
        { rows.push_back(it->second); }
        m_files_list->Insert(rows, false);

        if (!m_select_directories) {
            sorted_rows.clear();
            for (fs::directory_iterator it(s_working_dir); it != end_it; ++it) {
                try {
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
                    if (fs::exists(*it) && !fs::is_directory(*it) && it->path().filename().native()[0] != '.') {
#else
                    if (fs::exists(*it) && !fs::is_directory(*it) && it->filename()[0] != '.') {
#endif
                        bool meets_filters = file_filters.empty();
                        for (std::size_t i = 0; i < file_filters.size() && !meets_filters; ++i) {
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
                            if (parse(it->path().filename().string().c_str(), file_filters[i]).full)
#else
                            if (parse(it->filename().c_str(), file_filters[i]).full)
#endif
                                meets_filters = true;
                        }
                        if (meets_filters) {
                            ListBox::Row* row = new ListBox::Row();
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
                            row->push_back(GetStyleFactory()->NewTextControl(it->path().filename().string(), m_font, m_text_color, FORMAT_NOWRAP));
                            sorted_rows.insert(std::make_pair(it->path().filename().string(), row));
#else
                            row->push_back(GetStyleFactory()->NewTextControl(it->filename(), m_font, m_text_color, FORMAT_NOWRAP));
                            sorted_rows.insert(std::make_pair(it->filename(), row));
#endif
                        }
                    }
                } catch (const fs::filesystem_error&) {
                }
            }
            for (std::multimap<std::string, ListBox::Row*>::const_iterator it = sorted_rows.begin(); it != sorted_rows.end(); ++it) {
                m_files_list->Insert(it->second);
            }
        }
    } else {
        for (char c = 'C'; c <= 'Z'; ++c) {
            try {
                fs::path path(c + std::string(":"));
                if (fs::exists(path)) {
                    ListBox::Row* row = new ListBox::Row();
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
                    row->push_back(GetStyleFactory()->NewTextControl("[" + path.root_name().string() + "]", m_font, m_text_color, FORMAT_NOWRAP));
#else
                    row->push_back(GetStyleFactory()->NewTextControl("[" + path.root_name() + "]", m_font, m_text_color, FORMAT_NOWRAP));
#endif
                    m_files_list->Insert(row);
                }
            } catch (const fs::filesystem_error&) {
            }
        }
    }
}

void FileDlg::UpdateDirectoryText()
{
#if defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
    #if defined(_WIN32)
    // convert UTF-16 path to UTF-8 for display
    boost::filesystem::path::string_type working_dir_native = s_working_dir.native();
    std::string str;
    utf8::utf16to8(working_dir_native.begin(), working_dir_native.end(), std::back_inserter(str));
    #else
    std::string str = s_working_dir.string();
    #endif
#else
    std::string str = s_working_dir.native_directory_string();
#endif
    const X H_SPACING(10);
    while (m_font->TextExtent(str).x > Width() - 2 * H_SPACING) {
        std::string::size_type slash_idx = str.find('/', 1);
        std::string::size_type backslash_idx = str.find('\\', 1);
        if (slash_idx != std::string::npos) {
            slash_idx = str.find_first_not_of('/', slash_idx);
            str = "..." + str.substr(slash_idx);
        } else if (backslash_idx != std::string::npos) {
            backslash_idx = str.find_first_not_of('\\', backslash_idx);
            str = "..." + str.substr(backslash_idx);
        } else {
            break;
        }
    }
    *m_curr_dir_text << str;
    PlaceLabelsAndEdits(Width() / 4 - H_SPACING, m_files_edit->Height());
}

void FileDlg::OpenDirectory()
{
    // see if there is a directory selected; if so open the directory.
    // if more than one is selected, take the first one
    const ListBox::SelectionSet& sels = m_files_list->Selections();
    if (sels.empty())
        return;

    std::string directory;
    directory = boost::polymorphic_downcast<TextControl*>((***sels.begin())[0])->Text();

    if (directory.size() < 2 || directory[0] != '[')
        return;

    directory = directory.substr(1, directory.size() - 2); // strip off '[' and ']'

    if (directory == ".") {
        // remain in current directory
        UpdateList();

    } else if (directory == "..") {
        // move to parent directory of current directory
        if (s_working_dir.string() != s_working_dir.root_path().string() &&
            s_working_dir.branch_path().string() != "")
        {
            // move to new directory
            SetWorkingDirectory(s_working_dir.branch_path());

        } else {
            // switch to drive selection mode
            m_in_win32_drive_selection = true;
            m_files_edit->Clear();
            FilesEditChanged(m_files_edit->Text());
            m_curr_dir_text->SetText("");
            PlaceLabelsAndEdits(Width() / 4 - H_SPACING, m_files_edit->Height());
            UpdateList();
        }

    } else {
        // move to contained directory, which may be a drive selection...
        if (!m_in_win32_drive_selection) {

#if defined(_WIN32)
            // convert UTF-8 file name to UTF-16
            boost::filesystem::path::string_type directory_native;
            utf8::utf8to16(directory.begin(), directory.end(), std::back_inserter(directory_native));
            SetWorkingDirectory(s_working_dir / fs::path(directory_native));
#else
            SetWorkingDirectory(s_working_dir / fs::path(directory));
#endif
        } else {
            m_in_win32_drive_selection = false;
            try {
                SetWorkingDirectory(fs::path(directory + "\\"));
            } catch (const fs::filesystem_error& e) {
                if (e.code() == boost::system::posix_error::io_error) {
                    m_in_win32_drive_selection = true;
                    m_files_edit->Clear();
                    FilesEditChanged(m_files_edit->Text());
                    m_curr_dir_text->SetText("");
                    PlaceLabelsAndEdits(Width() / 4 - H_SPACING, m_files_edit->Height());
                    UpdateList();
                    boost::shared_ptr<ThreeButtonDlg> dlg(
                        GetStyleFactory()->NewThreeButtonDlg(X(175), Y(75),
                                                             m_device_is_not_ready_str,
                                                             m_font, m_color,
                                                             m_border_color, m_color,
                                                             m_text_color, 1));
                    dlg->Run();
                } else {
                    throw;
                }
            }
        }
    }

    if (m_save && m_ok_button->Text() != m_save_str)
        m_ok_button->SetText(m_save_str);
}
