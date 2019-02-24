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

#include <GG/Button.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/DrawUtil.h>
#include <GG/DropDownList.h>
#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/utf8/checked.h>
#include <GG/WndEvent.h>

#include <boost/cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
// boost::spirit::classic pulls in windows.h which in turn defines macro
// versions of min and max.  Defining NOMINMAX disables the creation of those
// macros
#define NOMINMAX
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
        FrontStringBegin(const std::shared_ptr<std::vector<std::string>>& strings) :
            m_strings(strings)
        {}

        const char* operator()() const {return m_strings->front().c_str();}

        std::shared_ptr<std::vector<std::string>> m_strings;
    };
    struct FrontStringEnd
    {
        FrontStringEnd(const std::shared_ptr<std::vector<std::string>>& strings) :
            m_strings(strings)
        {}

        const char* operator()() const {return m_strings->front().c_str() + m_strings->front().size();}

        std::shared_ptr<std::vector<std::string>> m_strings;
    };
    struct IndexedStringBegin
    {
        IndexedStringBegin(const std::shared_ptr<std::vector<std::string>>& strings) :
            m_strings(strings)
        {}

        const char* operator()() const {return (*m_strings)[Index::value].c_str();}

        std::shared_ptr<std::vector<std::string>> m_strings;
    };
    struct IndexedStringEnd
    {
        IndexedStringEnd(const std::shared_ptr<std::vector<std::string>>& strings) :
            m_strings(strings)
        {}

        const char* operator()() const {return (*m_strings)[Index::value].c_str() + (*m_strings)[Index::value].size();}

        std::shared_ptr<std::vector<std::string>> m_strings;
    };

    bool WindowsRoot(const std::string& root_name)
    { return root_name.size() == 2 && std::isalpha(root_name[0]) && root_name[1] == ':'; }

    bool Win32Paths()
    { return WindowsRoot(boost::filesystem::initial_path().root_name().string()); }

    const X H_SPACING(10);
    const Y V_SPACING(10);
}

namespace fs = boost::filesystem;

// static member definition(s)
fs::path FileDlg::s_working_dir = fs::current_path();
const X FileDlg::DEFAULT_WIDTH(500);
const Y FileDlg::DEFAULT_HEIGHT(450);


FileDlg::FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                 const std::shared_ptr<Font>& font, Clr color, Clr border_color, Clr text_color/* = CLR_BLACK*/) :
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
    m_save_str(GetStyleFactory()->Translate("Save")),
    m_open_str(GetStyleFactory()->Translate("Open")),
    m_curr_dir_text(nullptr),
    m_files_list(nullptr),
    m_files_edit(nullptr),
    m_filter_list(nullptr),
    m_ok_button(nullptr),
    m_cancel_button(nullptr),
    m_files_label(nullptr),
    m_file_types_label(nullptr),
    m_init_directory(directory),
    m_init_filename(filename)
{
    const auto& style = GetStyleFactory();

    if (m_save)
        multi = false;

    // finally, we can create the listbox with the files in it, sized to fill the available space
    m_files_list = style->NewListBox(m_border_color);
    m_files_list->SetStyle(LIST_NOSORT | (multi ? LIST_NONE : LIST_SINGLESEL));
}

void FileDlg::CompleteConstruction()
{
    const auto& style = GetStyleFactory();

    m_files_edit = style->NewEdit("", m_font, m_border_color, m_text_color);
    m_filter_list = style->NewDropDownList(3, m_border_color);
    m_filter_list->SetStyle(LIST_NOSORT);

    m_curr_dir_text = style->NewTextControl("", m_font, m_text_color, FORMAT_NOWRAP);
    m_files_label = style->NewTextControl(style->Translate("File(s):"), m_font, m_text_color, FORMAT_RIGHT | FORMAT_VCENTER);
    m_file_types_label = style->NewTextControl(style->Translate("Type(s):"), m_font, m_text_color, FORMAT_RIGHT | FORMAT_VCENTER);

    m_ok_button = style->NewButton(m_save ? m_save_str : m_open_str, m_font, m_color, m_text_color);
    m_cancel_button = style->NewButton(style->Translate("Cancel"), m_font, m_color, m_text_color);

    DoLayout();

    AttachChild(m_files_edit);
    AttachChild(m_filter_list);
    AttachChild(m_ok_button);
    AttachChild(m_cancel_button);
    AttachChild(m_files_list);
    AttachChild(m_curr_dir_text);
    AttachChild(m_files_label);
    AttachChild(m_file_types_label);

    if (!m_init_directory.empty()) {
#if defined(_WIN32)
        // convert UTF-8 file name to UTF-16
        boost::filesystem::path::string_type directory_native;
        utf8::utf8to16(m_init_directory.begin(), m_init_directory.end(), std::back_inserter(directory_native));
        fs::path dir_path = fs::system_complete(fs::path(directory_native));
#else
        fs::path dir_path = fs::system_complete(fs::path(m_init_directory));
#endif
        if (!fs::exists(dir_path))
            throw BadInitialDirectory("FileDlg::FileDlg() : Initial directory \"" + dir_path.string() + "\" does not exist.");
        SetWorkingDirectory(dir_path);
    }

    UpdateDirectoryText();
    PopulateFilters();
    UpdateList();

    m_ok_button->LeftClickedSignal.connect(
        boost::bind(&FileDlg::OkClicked, this));
    m_cancel_button->LeftClickedSignal.connect(
        boost::bind(&FileDlg::CancelClicked, this));
    m_files_list->SelRowsChangedSignal.connect(
        boost::bind(&FileDlg::FileSetChanged, this, _1));
    m_files_list->DoubleClickedRowSignal.connect(
        boost::bind(&FileDlg::FileDoubleClicked, this, _1, _2, _3));
    m_files_edit->EditedSignal.connect(
        boost::bind(&FileDlg::FilesEditChanged, this, _1));
    m_filter_list->SelChangedSignal.connect(
        boost::bind(&FileDlg::FilterChanged, this, _1));

    if (!m_init_filename.empty()) {
        fs::path filename_path = fs::system_complete(fs::path(m_init_filename));
        m_files_edit->SetText(filename_path.leaf().string());
    }
}

std::set<std::string> FileDlg::Result() const
{ return m_result; }

bool FileDlg::SelectDirectories() const
{ return m_select_directories; }

bool FileDlg::AppendMissingSaveExtension() const
{ return m_append_missing_save_extension; }

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

void FileDlg::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
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

void FileDlg::SetFileFilters(const std::vector<std::pair<std::string, std::string>>& filters)
{
    m_file_filters = filters;
    PopulateFilters();
    UpdateList();
}

const fs::path& FileDlg::WorkingDirectory()
{ return s_working_dir; }

const boost::filesystem::path FileDlg::StringToPath(const std::string& str) {
#if defined(_WIN32)
    // convert UTF-8 path string to UTF-16
    fs::path::string_type str_native;
    utf8::utf8to16(str.begin(), str.end(), std::back_inserter(str_native));
    return fs::path(str_native);
#else
    return fs::path(str);
#endif
}

void FileDlg::DoLayout()
{
    X button_width = Width() / 4 - H_SPACING;
    Y button_height = m_font->Height() + 2 * 5;

    m_curr_dir_text->MoveTo(GG::Pt(H_SPACING, V_SPACING / 2));

    m_files_list->MoveTo(Pt(H_SPACING, m_curr_dir_text->Height() + V_SPACING));
    m_files_list->Resize(Pt(Width() - 2 * H_SPACING,
                            Height() - (button_height + V_SPACING) * 2 - m_curr_dir_text->Height() - 2 * V_SPACING));

    // determine the space needed to display both text labels in the chosen font; use this to expand the edit as far as
    // possible
    X labels_width = std::max((m_files_label->MinUsableSize()).x,
                              (m_file_types_label->MinUsableSize()).x) + H_SPACING;

    m_files_label->MoveTo(Pt(X0, Height() - (button_height + V_SPACING) * 2));
    m_files_label->Resize(Pt(labels_width - H_SPACING / 2, button_height));

    m_file_types_label->MoveTo(Pt(X0, Height() - (button_height + V_SPACING) * 1));
    m_file_types_label->Resize(Pt(labels_width - H_SPACING / 2, button_height));

    m_files_edit->SizeMove(Pt(labels_width, Height() - (button_height + V_SPACING) * 2),
                           Pt(Width() - (button_width + 2 * H_SPACING), Height() - (button_height + 2 * V_SPACING)));

    m_filter_list->SizeMove(Pt(labels_width, Height() - (button_height + V_SPACING)),
                            Pt(Width() - (button_width + 2 * H_SPACING), Height() - V_SPACING));

    m_ok_button->MoveTo(Pt(Width() - (button_width + H_SPACING), Height() - (button_height + V_SPACING) * 2));
    m_ok_button->Resize(Pt(button_width, button_height));

    m_cancel_button->MoveTo(Pt(Width() - (button_width + H_SPACING), Height() - (button_height + V_SPACING)));
    m_cancel_button->Resize(Pt(button_width, button_height));
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

    const auto& style = GetStyleFactory();

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

#if defined (_WIN32)
            // convert UTF-16 path back to UTF-8 for storage
            boost::filesystem::path::string_type path_native = p.native();
            std::string path_string;
            utf8::utf16to8(path_native.begin(), path_native.end(), std::back_inserter(path_string));
            m_result.insert(path_string);
#else
            m_result.insert(p.string());
#endif
            // check to see if file already exists; if so, ask if it's ok to overwrite
            if (fs::exists(p)) {
                std::string msg_str = boost::str(boost::format(style->Translate("%1% exists.\nOk to overwrite it?")) % save_file);
                auto dlg =
                    style->NewThreeButtonDlg(X(300), Y(125), msg_str, m_font, m_color, m_border_color, m_color, m_text_color,
                                             2, style->Translate("Ok"), style->Translate("Cancel"));
                dlg->Run();
                results_valid = (dlg->Result() == 0);
            }
        }
    } else { // file open case
        if (files.empty() || (m_select_directories && double_click)) {
            OpenDirectory();
        } else { // ensure the file(s) are valid before returning them
            for (const std::string& file_name : files) {
#if defined(_WIN32)
                // convert UTF-8 file name to UTF-16
                boost::filesystem::path::string_type file_name_native;
                utf8::utf8to16(file_name.begin(), file_name.end(), std::back_inserter(file_name_native));
                fs::path p = s_working_dir / fs::path(file_name_native);
#else
                fs::path p = s_working_dir / fs::path(file_name);
#endif
                if (fs::exists(p)) {
                    bool p_is_directory = fs::is_directory(p);
                    if (!m_select_directories && p_is_directory) {
                        std::string msg_str = boost::str(boost::format(style->Translate("\"%1%\"\nis a directory.")) % file_name);
                        auto dlg =
                            style->NewThreeButtonDlg(X(300), Y(125), msg_str, m_font, m_color, m_border_color, m_color,
                                                     m_text_color, 1, style->Translate("Ok"));
                        dlg->Run();
                        results_valid = false;
                        break;
                    }
#if defined(_WIN32)
                    // convert UTF-16 path string to UTF-8
                    std::string temp;
                    boost::filesystem::path::string_type file_name_native = p.native();
                    utf8::utf16to8(file_name_native.begin(), file_name_native.end(), std::back_inserter(temp));
                    m_result.insert(temp);
#else
                    m_result.insert(p.string());
#endif
                    results_valid = true; // indicate validity only if at least one good file was found
                } else {
                    std::string msg_str = boost::str(boost::format(style->Translate("File \"%1%\"\ndoes not exist.")) % file_name);
                    auto dlg =
                        style->NewThreeButtonDlg(X(300), Y(125), msg_str, m_font, m_color, m_border_color, m_color,
                                                 m_text_color, 1, style->Translate("Ok"));
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
    for (const auto& file : files) {
        std::string filename = !(**file).empty() ? boost::polymorphic_downcast<TextControl*>((**file).at(0))->Text() : "";
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

void FileDlg::FileDoubleClicked(DropDownList::iterator it, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys)
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
        for (auto& file_filter : m_file_filters) {
            auto row = Wnd::Create<ListBox::Row>();
            row->push_back(GetStyleFactory()->NewTextControl(file_filter.first, m_font, m_text_color, FORMAT_NOWRAP));
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
    std::vector<rule<>> file_filters;

    auto filter_it = m_filter_list->CurrentItem();
    if (filter_it != m_filter_list->end()) {
        std::vector<std::string> filter_specs; // the filter specifications (e.g. "*.png")
        parse(m_file_filters[std::distance(m_filter_list->begin(), filter_it)].second.c_str(),
              *(!ch_p(',') >> (+(anychar_p - ','))[append(filter_specs)]),
              space_p);
        file_filters.resize(filter_specs.size());
        for (std::size_t i = 0; i < filter_specs.size(); ++i) {
            auto non_wildcards = std::make_shared<std::vector<std::string>>(); // the parts of the filter spec that are not wildcards
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
             !s_working_dir.branch_path().string().empty()) ||
            Win32Paths())
        {
            auto row = Wnd::Create<ListBox::Row>();
            row->push_back(GetStyleFactory()->NewTextControl("[..]", m_font, m_text_color, FORMAT_NOWRAP));
            m_files_list->Insert(row);
        }
        // current directory selector
        {
            auto row = Wnd::Create<ListBox::Row>();
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
        std::multimap<std::string, std::shared_ptr<ListBox::Row>> sorted_rows;
        for (fs::directory_iterator it(s_working_dir); it != end_it; ++it) {
            try {
                if (fs::exists(*it) && fs::is_directory(*it) && it->path().filename().native()[0] != '.') {
                    auto row = Wnd::Create<ListBox::Row>();

#if defined(_WIN32)
                    // convert UTF-16 path to UTF-8 for display
                    boost::filesystem::path::string_type file_name_native = it->path().filename().native();
                    std::string temp;
                    utf8::utf16to8(file_name_native.begin(), file_name_native.end(), std::back_inserter(temp));
                    std::string row_text = "[" + temp + "]";
#else
                    std::string row_text = "[" + it->path().filename().string() + "]";
#endif
                    row->push_back(GetStyleFactory()->NewTextControl(row_text, m_font, m_text_color, FORMAT_NOWRAP));
                    sorted_rows.insert({row_text, row});
                }
            } catch (const fs::filesystem_error&) {
            }
        }

        std::vector<std::shared_ptr<ListBox::Row>> rows;
        rows.reserve(sorted_rows.size());
        for (auto& row : sorted_rows)
        { rows.push_back(row.second); }
        m_files_list->Insert(rows);

        if (!m_select_directories) {
            sorted_rows.clear();
            for (fs::directory_iterator it(s_working_dir); it != end_it; ++it) {
                try {
                    if (fs::exists(*it) && !fs::is_directory(*it) && it->path().filename().native()[0] != '.') {
                        bool meets_filters = file_filters.empty();
                        for (std::size_t i = 0; i < file_filters.size() && !meets_filters; ++i) {
                            if (parse(it->path().filename().string().c_str(), file_filters[i]).full)
                                meets_filters = true;
                        }
                        if (meets_filters) {
                            auto row = Wnd::Create<ListBox::Row>();
                            row->push_back(GetStyleFactory()->NewTextControl(it->path().filename().string(), m_font, m_text_color, FORMAT_NOWRAP));
                            sorted_rows.insert({it->path().filename().string(), row});
                        }
                    }
                } catch (const fs::filesystem_error&) {
                }
            }
            for (const auto& row : sorted_rows) {
                m_files_list->Insert(row.second);
            }
        }
    } else {
        for (char c = 'C'; c <= 'Z'; ++c) {
            try {
                fs::path path(c + std::string(":"));
                if (fs::exists(path)) {
                    auto row = Wnd::Create<ListBox::Row>();
                    row->push_back(GetStyleFactory()->NewTextControl("[" + path.root_name().string() + "]", m_font, m_text_color, FORMAT_NOWRAP));
                    m_files_list->Insert(row);
                }
            } catch (const fs::filesystem_error&) {
            }
        }
    }
}

void FileDlg::UpdateDirectoryText()
{
#if defined(_WIN32)
    // convert UTF-16 path to UTF-8 for display
    boost::filesystem::path::string_type working_dir_native = s_working_dir.native();
    std::string str;
    utf8::utf16to8(working_dir_native.begin(), working_dir_native.end(), std::back_inserter(str));
#else
    std::string str = s_working_dir.string();
#endif
    m_curr_dir_text->SetText(str);
    while (m_curr_dir_text->Width() > Width() - 2 * H_SPACING) {
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
        m_curr_dir_text->SetText(str);
    }
    DoLayout();
}

void FileDlg::OpenDirectory()
{
    const auto& style = GetStyleFactory();

    // see if there is a directory selected; if so open the directory.
    // if more than one is selected, take the first one
    const ListBox::SelectionSet& sels = m_files_list->Selections();
    if (sels.empty())
        return;

    std::string directory;
    directory = !(***sels.begin()).empty() ? boost::polymorphic_downcast<TextControl*>((***sels.begin()).at(0))->Text() : "";

    if (directory.size() < 2 || directory[0] != '[')
        return;

    directory = directory.substr(1, directory.size() - 2); // strip off '[' and ']'

    if (directory == ".") {
        // remain in current directory
        UpdateList();

    } else if (directory == "..") {
        // move to parent directory of current directory
        if (s_working_dir.string() != s_working_dir.root_path().string() &&
            !s_working_dir.branch_path().string().empty())
        {
            // move to new directory
            SetWorkingDirectory(s_working_dir.branch_path());

        } else {
            // switch to drive selection mode
            m_in_win32_drive_selection = true;
            m_files_edit->Clear();
            FilesEditChanged(m_files_edit->Text());
            m_curr_dir_text->SetText("");
            DoLayout();
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
                if (e.code() == boost::system::errc::io_error) {
                    m_in_win32_drive_selection = true;
                    m_files_edit->Clear();
                    FilesEditChanged(m_files_edit->Text());
                    m_curr_dir_text->SetText("");
                    DoLayout();
                    UpdateList();
                    auto dlg =
                        GetStyleFactory()->NewThreeButtonDlg(X(175), Y(75),
                                                             style->Translate("Device is not ready."),
                                                             m_font, m_color,
                                                             m_border_color, m_color,
                                                             m_text_color, 1);
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
