#include "SaveFileDialog.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "OptionsWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../network/Networking.h"
#include "../util/i18n.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/SaveGamePreviewUtils.h"
#include "../util/VarText.h"

#include <GG/Button.h>
#include <GG/Clr.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/DrawUtil.h>
#include <GG/utf8/checked.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/cast.hpp>
#include <boost/format.hpp>

#include <memory>
#include <string>

namespace fs = boost::filesystem;

using std::vector;
using std::string;

namespace {
    const GG::X SAVE_FILE_DIALOG_WIDTH ( 600 );
    const GG::Y SAVE_FILE_DIALOG_HEIGHT ( 400 );
    const GG::X SAVE_FILE_DIALOG_MIN_WIDTH ( 160 );
    const GG::Y SAVE_FILE_DIALOG_MIN_HEIGHT ( 100 );

    const std::string SAVE_FILE_WND_NAME = "save-load";

    const GG::X PROMT_WIDTH(200);
    const GG::Y PROMPT_HEIGHT(75);

    const double DEFAULT_STRETCH = 1.0;

    const GG::X SAVE_FILE_BUTTON_MARGIN ( 10 );
    const unsigned int SAVE_FILE_CELL_MARGIN = 2;
    const unsigned int ROW_MARGIN = 2;

    const std::string PATH_DELIM_BEGIN = "[";
    const std::string PATH_DELIM_END = "]";

    const std::string WIDE_AS = "wide-as";
    const std::string STRETCH = "stretch";

    const std::string SERVER_LABEL = "SERVER";

    const std::string VALID_PREVIEW_COLUMNS[] = {
        "player", "empire", "turn", "time", "file", "seed", "galaxy_age", "galaxy_size", "galaxy_shape",
        "monster_freq", "native_freq", "planet_freq", "specials_freq", "starlane_freq", "ai_aggression",
        "number_of_empires", "number_of_humans"
    };

    const unsigned int VALID_PREVIEW_COLUMN_COUNT = sizeof(VALID_PREVIEW_COLUMNS) / sizeof(std::string);

    const int WHEEL_INCREMENT = 80;

    // command-line options
    void AddOptions(OptionsDB& db) {
        // List the columns to show, separated by colons.
        // Valid: time, turn, player, empire, systems, seed, galaxy_age, galaxy_shape, planet_freq, native_freq, specials_freq, starlane_freq
        // These settings are not visible in the options panel; the defaults should be good for regular users.
        db.Add<std::vector<std::string>>("UI.save-file-dialog.columns", UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMNS"),
                                          StringToList("time,turn,player,empire,file"));
        db.Add<std::string>("UI.save-file-dialog.time." + WIDE_AS, UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMN_WIDE_AS"),
                            "YYYY-MM-DD");
        db.Add<std::string>("UI.save-file-dialog.turn." + WIDE_AS, UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMN_WIDE_AS"),
                            "9999");
        db.Add("UI.save-file-dialog.player." + STRETCH, UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMN_STRETCH"), 1.0);
        db.Add("UI.save-file-dialog.empire." + STRETCH, UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMN_STRETCH"), 1.0);
        db.Add("UI.save-file-dialog.file." + STRETCH, UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMN_STRETCH"), 2.0);
        db.Add<std::string>("UI.save-file-dialog.galaxy_size." + WIDE_AS, UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMN_WIDE_AS"), "9999");
        db.Add("UI.save-file-dialog.seed." + STRETCH, UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMN_STRETCH"), 0.75);

        db.Add("UI.save-file-dialog.default." + STRETCH, UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMN_STRETCH"), 1.0);

        // We give them a custom delay since the general one is a bit quick
        db.Add("UI.save-file-dialog.tooltip-delay", UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_TOOLTIP_DELAY"), 800);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    /// Creates a text control that support resizing and word wrap.
    GG::Label* CreateResizingText(const std::string& string, GG::X width)
    {
        // Calculate the extent manually to ensure the control stretches to full
        // width when possible.  Otherwise it would always word break.
        GG::Flags<GG::TextFormat> fmt = GG::FORMAT_NONE;
        std::vector<std::shared_ptr<GG::Font::TextElement>> text_elements =
            ClientUI::GetFont()->ExpensiveParseFromTextToTextElements(string, fmt);
        std::vector<GG::Font::LineData> lines = ClientUI::GetFont()->DetermineLines(string, fmt, width, text_elements);
        GG::Pt extent = ClientUI::GetFont()->TextExtent(lines);
        auto text = new CUILabel(string, text_elements,
                                       GG::FORMAT_WORDBREAK | GG::FORMAT_LEFT, GG::NO_WND_FLAGS,
                                       GG::X0, GG::Y0, extent.x, extent.y);
        text->ClipText(true);
        text->SetChildClippingMode(GG::Wnd::ClipToClient);
        return text;
    }

    std::string GenericPathString(const fs::path& path) {
#ifndef FREEORION_WIN32
        return path.generic_string();
#else
        fs::path::string_type native_string = path.generic_wstring();
        std::string retval;
        utf8::utf16to8(native_string.begin(), native_string.end(), std::back_inserter(retval));
        return retval;
#endif
    }

    bool Prompt(const std::string& question){
        std::shared_ptr<GG::Font> font = ClientUI::GetFont();
        GG::ThreeButtonDlg prompt(PROMT_WIDTH, PROMPT_HEIGHT, question, font,
                                  ClientUI::CtrlColor(), ClientUI::CtrlBorderColor(), ClientUI::CtrlColor(), ClientUI::TextColor(),
                                  std::size_t(2), UserString("YES"), UserString("CANCEL"), "");
        prompt.Run();
        return prompt.Result() == 0;
    }

    /// Returns true if list contains a row with the text str
    bool HasRow(const GG::DropDownList* list, const std::string& str){
        if (!list)
            return false;

        for (const auto& row : *list) {
            for (unsigned j = 0; j < row->size(); ++j) {
                const GG::Control* control = row->at(j);
                const GG::Label* text = dynamic_cast<const GG::Label*>(control);
                if (text) {
                    if (text->Text() == str || text->Text() + "/" == str) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
}

/** Describes how a column should be set up in the dialog */
class SaveFileColumn {
public:
    static std::shared_ptr<std::vector<SaveFileColumn>> GetColumns(GG::X max_width) {
        auto columns = std::make_shared<std::vector<SaveFileColumn>>();
        for (unsigned int i = 0; i < VALID_PREVIEW_COLUMN_COUNT; ++i)
            columns->push_back(GetColumn(VALID_PREVIEW_COLUMNS[i], max_width));
        return columns;
    }

    static GG::Control* TitleForColumn(const SaveFileColumn& column)
    {
        auto retval = new CUILabel(column.Title(), GG::FORMAT_LEFT);
        retval->Resize(GG::Pt(GG::X1, ClientUI::GetFont()->Height()));
        return retval;
    }

    static GG::Control* CellForColumn(const SaveFileColumn& column, const FullPreview& full, GG::X max_width)
    {
        GG::Clr color = ClientUI::TextColor();
        std::string value = ColumnInPreview(full, column.m_name);
        if (column.m_name == "empire")
            color = full.preview.main_player_empire_colour;

        GG::Flags<GG::TextFormat> format_flags = GG::FORMAT_LEFT;
        if (column.m_name == "turn")
            format_flags = GG::FORMAT_CENTER;

        GG::Label* retval = nullptr;

        if (column.m_fixed) {
            retval = new CUILabel(value, format_flags, GG::NO_WND_FLAGS,
                                  GG::X0, GG::Y0,
                                  column.FixedWidth(), ClientUI::GetFont()->Height());
        } else {
            retval = CreateResizingText(value, max_width);
        }

        retval->SetTextColor(color);
        return retval;
    }

    bool Fixed() const
    { return m_fixed; }

    GG::X FixedWidth() const
    { return m_fixed_width; }

    double Stretch() const
    { return m_stretch; }

    const std::string& Name() const
    { return m_name; }

private:
    static SaveFileColumn GetColumn(const std::string& name, GG::X max_width) {
        const std::string prefix = "UI.save-file-dialog.";
        std::string option = prefix + name + ".";
        OptionsDB&  db = GetOptionsDB();

        if (db.OptionExists(option + WIDE_AS)) {
            return SaveFileColumn(name, db.Get<std::string>(option + WIDE_AS), max_width);
        } else if (db.OptionExists(option + STRETCH)) {
            return SaveFileColumn(name, db.Get<double>(option + STRETCH), max_width);
        } else if (db.OptionExists(prefix + "default." + STRETCH)) {
            return SaveFileColumn(name, db.Get<double>(prefix + "default." + STRETCH), max_width);
        } else {
            return SaveFileColumn(name, DEFAULT_STRETCH, max_width);
        }
    }

    /// Creates a fixed width column
    SaveFileColumn(const std::string& name, const std::string& wide_as, GG::X max_width) :
        m_name(name),
        m_fixed(true),
        m_fixed_width(GG::X0),
        m_wide_as(wide_as),
        m_stretch(0.0)
    { m_fixed_width = ComputeFixedWidth(Title(), m_wide_as, max_width);}

    /// Creates a stretchy column
    SaveFileColumn(const std::string& name, double stretch, GG::X max_width) :
        m_name(name),
        m_fixed(false),
        m_fixed_width(GG::X0),
        m_wide_as(),
        m_stretch(stretch)
    { m_fixed_width = ComputeFixedWidth(Title(), m_wide_as, max_width);}

    std::string Title() const {
        if (m_name == "player") {
            return UserString("SAVE_PLAYER_TITLE");
        } else if( m_name == "empire") {
            return UserString("SAVE_EMPIRE_TITLE");
        } else if( m_name == "turn") {
            return UserString("SAVE_TURN_TITLE");
        } else if( m_name == "time") {
            return UserString("SAVE_TIME_TITLE");
        } else if( m_name == "file") {
            return UserString("SAVE_FILE_TITLE");
        } else if( m_name == "galaxy_size") {
            return UserString("SAVE_GALAXY_SIZE_TITLE");
        } else if( m_name == "seed") {
            return UserString("SAVE_SEED_TITLE");
        } else if( m_name == "galaxy_age") {
            return UserString("SAVE_GALAXY_AGE_TITLE");
        } else if( m_name == "monster_freq") {
            return UserString("SAVE_MONSTER_FREQ_TITLE");
        } else if( m_name == "native_freq") {
            return UserString("SAVE_NATIVE_FREQ_TITLE");
        } else if( m_name == "planet_freq") {
            return UserString("SAVE_PLANET_FREQ_TITLE");
        } else if( m_name == "specials_freq") {
            return UserString("SAVE_SPECIALS_FREQ_TITLE");
        } else if( m_name == "starlane_freq") {
            return UserString("SAVE_STARLANE_FREQ_TITLE");
        } else if( m_name == "galaxy_shape") {
            return UserString("SAVE_GALAXY_SHAPE_TITLE");
        } else if( m_name == "ai_aggression") {
            return UserString("SAVE_AI_AGGRESSION_TITLE");
        } else if( m_name == "number_of_empires") {
            return UserString("SAVE_NUMBER_EMPIRES_TITLE");
        } else if( m_name == "number_of_humans" ){
            return UserString("SAVE_NUMBER_HUMANS_TITLE");
        } else {
            ErrorLogger() << "SaveFileColumn::Title Error: no such preview field: " << m_name;
            return "???";
        }
    }

    static GG::X ComputeFixedWidth(const std::string& title, const std::string& wide_as, GG::X max_width) {
        std::shared_ptr<GG::Font> font = ClientUI::GetFont();
        // We need to maintain the fixed sizes since the base list box messes them
        std::vector<GG::Font::LineData> lines;
        GG::Flags<GG::TextFormat> fmt = GG::FORMAT_NONE;

        //TODO cache this resulting extent
        std::vector<std::shared_ptr<GG::Font::TextElement>> text_elements =
            font->ExpensiveParseFromTextToTextElements(wide_as, fmt);
        lines = font->DetermineLines(wide_as, fmt, max_width, text_elements);
        GG::Pt extent1 = font->TextExtent(lines);

        text_elements = font->ExpensiveParseFromTextToTextElements(title, fmt);
        lines = font->DetermineLines(title, fmt, max_width, text_elements);
        GG::Pt extent2 = font->TextExtent(lines);

        return std::max(extent1.x, extent2.x) + GG::X(SAVE_FILE_CELL_MARGIN);
    }

    /// The identifier of what data to show. Must be valid.
    std::string m_name;
    /// If true, column width is fixed to be the width of m_wide_as under the current font.
    /// If false, column stretches with factor m_stretch
    bool m_fixed;
    /// The wideset fixed width from wide_as or the title
    GG::X m_fixed_width;
    /// The string to be used in determining the width of the column
    std::string m_wide_as;
    /// The stretch of the column.
    double m_stretch;
};

/** A Specialized row for the save dialog list box. */
class SaveFileRow: public GG::ListBox::Row {
public:
    SaveFileRow() {}

    SaveFileRow(const std::shared_ptr<std::vector<SaveFileColumn>>& columns, const std::string& filename) :
        m_filename(filename),
        m_columns(columns),
        m_initialized(false)
    {
        SetName("SaveFileRow for \""+filename+"\"");
        SetChildClippingMode(ClipToClient);
        RequirePreRender();
    }

    virtual void Init()
    { m_initialized = true;}

    const std::string&  Filename() const
    { return m_filename; }

    void PreRender() override {
        if (!m_initialized)
            Init();
        GG::ListBox::Row::PreRender();
    }

    void Render() override {
        GG::FlatRectangle(ClientUpperLeft(),
                          ClientLowerRight() - GG::Pt(GG::X(SAVE_FILE_CELL_MARGIN), GG::Y0),
                          GG::CLR_ZERO, ClientUI::WndOuterBorderColor(), 1u);
    }

    /** Excludes border from the client area. */
    GG::Pt ClientUpperLeft() const override {
        return UpperLeft() + GG::Pt(GG::X(SAVE_FILE_CELL_MARGIN),
                                    GG::Y(SAVE_FILE_CELL_MARGIN));
    }

    /** Excludes border from the client area. */
    GG::Pt ClientLowerRight() const override {
        return LowerRight() - GG::Pt(GG::X(SAVE_FILE_CELL_MARGIN * 2),
                                     GG::Y(SAVE_FILE_CELL_MARGIN));
    }

    /** Forces the columns to column widths not defined by ListBox. Needs to be called after any
        interaction with the ListBox base class that sets the column widths back to those defined
        by SetColWidths().*/
    virtual void AdjustColumns() {
        GG::Layout* layout = GetLayout();
        if (!layout)
            return;
        for (unsigned int i = 0; i < m_columns->size(); ++i) {
            const SaveFileColumn& column = (*m_columns)[i];
            layout->SetColumnStretch ( i, column.Stretch() );
            layout->SetMinimumColumnWidth ( i, column.FixedWidth() ); // Considers header
        }
    }

protected:
    std::string m_filename;
    std::shared_ptr<std::vector<SaveFileColumn>> m_columns;
    bool m_initialized;
};

class SaveFileHeaderRow: public SaveFileRow {
public:
    SaveFileHeaderRow(const std::shared_ptr<std::vector<SaveFileColumn>>& columns) :
        SaveFileRow(columns, "")
    {
        SetMargin(ROW_MARGIN);

        for (const SaveFileColumn& column : *m_columns)
        { push_back(SaveFileColumn::TitleForColumn(column)); }
        AdjustColumns();
    }

    void Render() override
    {}
};

class SaveFileDirectoryRow: public SaveFileRow {
public:
    SaveFileDirectoryRow(const std::shared_ptr<std::vector<SaveFileColumn>>& columns, const std::string& directory) :
        SaveFileRow(columns, directory) {
        SetMargin(ROW_MARGIN);
    }

    void Init() override {
        SaveFileRow::Init();
        for (unsigned int i = 0; i < m_columns->size(); ++i) {
            if (i==0) {
                auto label = new CUILabel(PATH_DELIM_BEGIN + m_filename + PATH_DELIM_END,
                                               GG::FORMAT_NOWRAP | GG::FORMAT_LEFT);
                label->Resize(GG::Pt(DirectoryNameSize(), ClientUI::GetFont()->Height()));
                push_back(label);
            } else {
                // Dummy columns so that all rows have the same number of cols
                auto label = new CUILabel("", GG::FORMAT_NOWRAP);
                label->Resize(GG::Pt(GG::X0, ClientUI::GetFont()->Height()));
                push_back(label);
            }
        }

        AdjustColumns();

        GetLayout()->PreRender();
    }

    SortKeyType SortKey(std::size_t column) const override
    { return m_filename; }

    GG::X DirectoryNameSize() {
        GG::Layout* layout = GetLayout();
        if (!layout)
            return ClientUI::GetFont()->SpaceWidth() * 10;

        // Give the directory label at least all the room that the other columns demand anyway
        GG::X sum(0);
        for (const SaveFileColumn& column : *m_columns) {
            sum += column.FixedWidth();
        }
        return sum;
    }
};

class SaveFileFileRow: public SaveFileRow {
public:
    /// Creates a row for the given savefile
    SaveFileFileRow(const FullPreview& full,
                    const std::shared_ptr<std::vector<SaveFileColumn>>& visible_columns,
                    const std::shared_ptr<std::vector<SaveFileColumn>>& columns,
                    int tooltip_delay) :
        SaveFileRow(visible_columns, full.filename),
        m_all_columns(columns),
        m_full_preview(full)
    {
        SetMargin (ROW_MARGIN);
        SetBrowseModeTime(tooltip_delay);
    }

    void Init() override {
        SaveFileRow::Init();
        VarText browse_text(UserStringNop("SAVE_DIALOG_ROW_BROWSE_TEMPLATE"));

        for (const SaveFileColumn& column : *m_columns) {
            GG::Control* cfc = SaveFileColumn::CellForColumn(column, m_full_preview, ClientWidth());
            push_back(cfc);
        }
        for (const SaveFileColumn& column : *m_all_columns) {
            browse_text.AddVariable(column.Name(), ColumnInPreview(m_full_preview, column.Name(), false));
        }
        AdjustColumns();
        SetBrowseText(browse_text.GetText());
        GetLayout()->PreRender();
    }

    SortKeyType SortKey(std::size_t column) const override
    { return m_full_preview.preview.save_time; }

    private:
    /** All possible columns. */
    std::shared_ptr<std::vector<SaveFileColumn>> m_all_columns;

    const FullPreview m_full_preview;
};


class SaveFileListBox : public CUIListBox {
public:
    SaveFileListBox() :
        CUIListBox ()
    { }

    void Init() {
        m_columns = SaveFileColumn::GetColumns(ClientWidth());
        m_visible_columns = FilterColumns(m_columns);
        ManuallyManageColProps();
        NormalizeRowsOnInsert(false);
        SetNumCols(m_visible_columns->size());

        auto header_row = new SaveFileHeaderRow(m_visible_columns);
        SetColHeaders(header_row);
        for (unsigned int i = 0; i < m_visible_columns->size(); ++i) {
            const SaveFileColumn& column = (*m_visible_columns)[i];
            SetColStretch(i, column.Stretch());
            SetColWidth(i, column.FixedWidth());
        }

        SetSortCmp(&SaveFileListBox::DirectoryAwareCmp);
        SetVScrollWheelIncrement(WHEEL_INCREMENT);
    }

    void ResetColHeaders() {
        RemoveColHeaders();
        SetColHeaders(new SaveFileHeaderRow(m_visible_columns));
    }

    GG::Pt ListRowSize() const
    { return GG::Pt(Width() - RightMargin(), ListRowHeight()); }

    static GG::Y ListRowHeight()
    { return GG::Y(ClientUI::Pts() * 2); }

    /// Loads preview data on all save files in a directory specidifed by path.
    /// @param[in] path The path of the directory
    /// @param[in] extension File name extension to filter by
    void LoadSaveGamePreviews(const fs::path& path, const std::string& extension) {
        LoadDirectories(path);

        vector<FullPreview> previews;
        ::LoadSaveGamePreviews(path, extension, previews);
        LoadSaveGamePreviews(previews);
    }

    /// Loads preview data on all save files in a directory specidifed by path.
    /// @param [in] previews The preview data
    void LoadSaveGamePreviews(const std::vector<FullPreview>& previews) {
        int tooltip_delay = GetOptionsDB().Get<int>("UI.save-file-dialog.tooltip-delay");

        std::vector<Row*> rows;
        for (const FullPreview& preview : previews) {
            rows.push_back(new SaveFileFileRow(preview, m_visible_columns, m_columns, tooltip_delay));
        }

        // Insert rows enmasse to avoid per insertion vector sort costs.
        Insert(rows, false);
    }

    void LoadDirectories(const fs::path& path) {
        fs::directory_iterator end_it;
        if (path.has_parent_path() && path.parent_path() != path) {
            auto row = new SaveFileDirectoryRow(m_visible_columns, "..");
            Insert(row);
        }

        for (fs::directory_iterator it(path); it != end_it; ++it) {
            if (fs::is_directory(it->path())) {
                fs::path last_bit_of_path = it->path().filename();
                std::string utf8_dir_name = PathString(last_bit_of_path);
                DebugLogger() << "SaveFileDialog::LoadDirectories name: " << utf8_dir_name << " valid UTF-8: " << IsValidUTF8(utf8_dir_name);
                auto row = new SaveFileDirectoryRow(m_visible_columns, utf8_dir_name);
                Insert(row);

                //boost::filesystem::path::string_type path_native = last_bit_of_path.native();
                //std::string path_string;
                //utf8::utf16to8(path_native.begin(), path_native.end(), std::back_inserter(path_string));
                //DebugLogger() << "SaveFileDialog::LoadDirectories name: " << path_string << " valid UTF-8: " << IsValidUTF8(path_string);
                //Insert(new SaveFileDirectoryRow(path_string));
            }
        }
    }

    bool HasFile(const std::string& filename) {
        for (auto& row : *this) {
            SaveFileRow* srow = dynamic_cast<SaveFileRow*>(row);
            if (srow && srow->Filename() == filename)
                return true;
        }
        return false;
    }

private:
    std::shared_ptr<std::vector<SaveFileColumn>> m_columns;
    std::shared_ptr<std::vector<SaveFileColumn>> m_visible_columns;

    static std::shared_ptr<std::vector<SaveFileColumn>> FilterColumns(
        const std::shared_ptr<std::vector<SaveFileColumn>>& all_cols)
    {
        std::vector<std::string> names = GetOptionsDB().Get<std::vector<std::string>>("UI.save-file-dialog.columns");
        auto columns = std::make_shared<std::vector<SaveFileColumn>>();
        for (const std::string& column_name : names) {
            bool found_col = false;
            for (const SaveFileColumn& column : *all_cols) {
                if (column.Name() == column_name) {
                    columns->push_back(column);
                    found_col = true;
                    break;
                }
            }
            if (!found_col)
                ErrorLogger() << "SaveFileListBox::FilterColumns: Column not found: " << column_name;
        }
        DebugLogger() << "SaveFileDialog::FilterColumns: Visible columns: " << columns->size();
        return columns;
    }

    /// We want the timestamps to be sorted in ascending order to get the latest save
    /// first, but we want the directories to
    /// a) always be first
    /// b) be sorted alphabetically
    /// This custom comparer achieves these goals.
    static bool DirectoryAwareCmp(const Row& row1, const Row& row2, int column_int) {
        std::string key1(row1.SortKey(0));
        std::string key2(row2.SortKey(0));

        const bool row1_is_directory = dynamic_cast<const SaveFileDirectoryRow*>(&row1);
        const bool row2_is_directory = dynamic_cast<const SaveFileDirectoryRow*>(&row2);
        if (!row1_is_directory && !row2_is_directory) {
            return key1.compare(key2) <= 0;
        } else if ( row1_is_directory && row2_is_directory ) {
            // Directories always return directory name as sort key
            return key1.compare(key2) >= 0;
        } else {
            return ( !row1_is_directory && row2_is_directory );
        }
    }
};

SaveFileDialog::SaveFileDialog (const std::string& extension, bool load) :
    CUIWnd(UserString("GAME_MENU_SAVE_FILES"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE,
           SAVE_FILE_WND_NAME)
{
    m_extension = extension;
    m_load_only = load;
    m_server_previews = false;
    Init();
}

SaveFileDialog::SaveFileDialog (bool load) :
    CUIWnd(UserString("GAME_MENU_SAVE_FILES"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE,
           SAVE_FILE_WND_NAME)
{
    m_load_only = load;
    m_server_previews = true;
    m_extension = MP_SAVE_FILE_EXTENSION;
    Init();
}

void SaveFileDialog::Init() {
    ResetDefaultPosition();
    SetMinSize(GG::Pt(2*SAVE_FILE_DIALOG_MIN_WIDTH, 2*SAVE_FILE_DIALOG_MIN_HEIGHT));

    m_layout = GG::Wnd::Create<GG::Layout>(GG::X0, GG::Y0,
                              SAVE_FILE_DIALOG_WIDTH - LeftBorder() - RightBorder(),
                              SAVE_FILE_DIALOG_HEIGHT - TopBorder() - BottomBorder(), 4, 4);
    m_layout->SetCellMargin(SAVE_FILE_CELL_MARGIN);
    m_layout->SetBorderMargin(SAVE_FILE_CELL_MARGIN*2);

    m_file_list = new SaveFileListBox();
    m_file_list->SetStyle(GG::LIST_SINGLESEL | GG::LIST_SORTDESCENDING);

    m_confirm_btn = new CUIButton(UserString("OK"));
    auto cancel_btn = new CUIButton(UserString("CANCEL"));

    m_name_edit = new CUIEdit("");
    if (m_extension != MP_SAVE_FILE_EXTENSION && m_extension != SP_SAVE_FILE_EXTENSION) {
        std::string savefile_ext = HumanClientApp::GetApp()->SinglePlayerGame() ? SP_SAVE_FILE_EXTENSION : MP_SAVE_FILE_EXTENSION;
        DebugLogger() << "SaveFileDialog passed invalid extension " << m_extension << ", changing to " << savefile_ext;
        m_extension = savefile_ext;
    }

    auto filename_label = new CUILabel(UserString("SAVE_FILENAME"), GG::FORMAT_NOWRAP);
    auto directory_label = new CUILabel(UserString("SAVE_DIRECTORY"), GG::FORMAT_NOWRAP);
    //std::cout << "pathstrnig: " << PathString(GetSaveDir()) << std::endl;
    DebugLogger() << "pathstring: " << PathString(GetSaveDir());
    m_current_dir_edit = new CUIEdit(PathString(GetSaveDir()));

    m_layout->Add(directory_label, 0, 0);

    std::shared_ptr<GG::Font> font = ClientUI::GetFont();
    if (!m_server_previews) {
        m_layout->Add(m_current_dir_edit, 0, 1, 1, 3);

        auto delete_btn = new CUIButton(UserString("DELETE"));
        m_layout->Add(delete_btn, 2, 3);
        delete_btn->LeftClickedSignal.connect(
            boost::bind(&SaveFileDialog::AskDelete, this));

        m_layout->SetMinimumRowHeight(2, delete_btn->MinUsableSize().y + GG::Y(Value(SAVE_FILE_BUTTON_MARGIN)));
        m_layout->SetMinimumColumnWidth(2, m_confirm_btn->MinUsableSize().x + 2*SAVE_FILE_BUTTON_MARGIN);
        m_layout->SetMinimumColumnWidth(3, std::max( cancel_btn->MinUsableSize().x,
                                        delete_btn->MinUsableSize().x) + SAVE_FILE_BUTTON_MARGIN);
    } else {
        m_remote_dir_dropdown = new CUIDropDownList(6);
        m_layout->Add(m_current_dir_edit, 0, 1, 1, 1);
        m_layout->Add(m_remote_dir_dropdown, 0, 2 , 1, 2);
        GG::Flags<GG::TextFormat> fmt = GG::FORMAT_NONE;
        std::string server_label(SERVER_LABEL+SERVER_LABEL+SERVER_LABEL);
        std::vector<std::shared_ptr<GG::Font::TextElement>> text_elements =
            font->ExpensiveParseFromTextToTextElements(server_label, fmt);
        std::vector<GG::Font::LineData> lines =
            font->DetermineLines(server_label, fmt, ClientWidth(), text_elements);
        GG::X drop_width = font->TextExtent(lines).x;
        m_layout->SetMinimumColumnWidth(2, std::max(m_confirm_btn->MinUsableSize().x + 2*SAVE_FILE_BUTTON_MARGIN, drop_width/2));
        m_layout->SetMinimumColumnWidth(3, std::max(cancel_btn->MinUsableSize().x + SAVE_FILE_BUTTON_MARGIN, drop_width / 2));

        m_remote_dir_dropdown->SelChangedSignal.connect(
            boost::bind(&SaveFileDialog::DirectoryDropdownSelect, this, _1));
    }

    m_layout->Add(m_file_list,      1, 0, 1, 4);
    m_layout->Add(filename_label,   2, 0);
    m_layout->Add(m_name_edit,      3, 0, 1, 2);
    m_layout->Add(m_confirm_btn,    3, 2);
    m_layout->Add(cancel_btn,       3, 3);

    m_layout->SetMinimumRowHeight(0, m_current_dir_edit->MinUsableSize().y);
    m_layout->SetRowStretch      (1, 1.0 );
    GG::Flags<GG::TextFormat> fmt = GG::FORMAT_NONE;
    std::string cancel_text(cancel_btn->Text());
    std::vector<std::shared_ptr<GG::Font::TextElement>> text_elements =
    font->ExpensiveParseFromTextToTextElements(cancel_text, fmt);
    std::vector<GG::Font::LineData> lines = ClientUI::GetFont()->DetermineLines(
        cancel_text, fmt, GG::X(1 << 15), text_elements);
    GG::Pt extent = ClientUI::GetFont()->TextExtent(lines);
    m_layout->SetMinimumRowHeight(3, extent.y);

    std::string filename_label_text(filename_label->Text());
    text_elements = font->ExpensiveParseFromTextToTextElements(filename_label_text, fmt);
    lines = font->DetermineLines(filename_label_text, fmt, ClientWidth(), text_elements);
    GG::Pt extent1 = font->TextExtent(lines);

    std::string dir_label_text(directory_label->Text());
    text_elements = font->ExpensiveParseFromTextToTextElements(dir_label_text, fmt);
    lines = font->DetermineLines(dir_label_text, fmt, ClientWidth(), text_elements);
    GG::Pt extent2 = font->TextExtent(lines);

    m_layout->SetMinimumColumnWidth(0, std::max(extent1.x, extent2.x));
    m_layout->SetColumnStretch(1, 1.0);

    SetLayout(m_layout);

    m_confirm_btn->LeftClickedSignal.connect(
        boost::bind(&SaveFileDialog::Confirm, this));
    cancel_btn->LeftClickedSignal.connect(
        boost::bind(&SaveFileDialog::Cancel, this));
    m_file_list->SelRowsChangedSignal.connect(
        boost::bind(&SaveFileDialog::SelectionChanged, this, _1));
    m_file_list->DoubleClickedRowSignal.connect(
        boost::bind(&SaveFileDialog::DoubleClickRow, this, _1, _2, _3));
    m_name_edit->EditedSignal.connect(
        boost::bind(&SaveFileDialog::FileNameEdited, this, _1));
    m_current_dir_edit->EditedSignal.connect(
        boost::bind(&SaveFileDialog::DirectoryEdited, this, _1));

    if (!m_load_only) {
        m_name_edit->SetText(std::string("save-") + FilenameTimestamp() + m_extension);
        m_name_edit->SelectAll();
    }

    if (m_server_previews) {
        // Indicate to the user that they are browsing server saves
        SetDirPath("./");
    }

    UpdatePreviewList();
}

SaveFileDialog::~SaveFileDialog()
{}

GG::Rect SaveFileDialog::CalculatePosition() const {
    GG::Pt ul((GG::GUI::GetGUI()->AppWidth() - SAVE_FILE_DIALOG_WIDTH) / 2,
              (GG::GUI::GetGUI()->AppHeight() - SAVE_FILE_DIALOG_HEIGHT) / 2);
    GG::Pt wh(SAVE_FILE_DIALOG_WIDTH, SAVE_FILE_DIALOG_HEIGHT);
    return GG::Rect(ul, ul + wh);
}

void SaveFileDialog::ModalInit() {
    GG::Wnd::ModalInit();
    GG::GUI::GetGUI()->SetFocusWnd(m_name_edit);
}

void SaveFileDialog::KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys ) {
    // Return without filename
    if (key == GG::GGK_ESCAPE) {
        Cancel();
        return;
    }

    // Update list on enter if directory changed by hand
    if (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) {
        if (m_loaded_dir != GetDirPath()) {
            UpdatePreviewList();
        } else {
            if (GG::GUI::GetGUI()->FocusWnd() == m_name_edit) {
                Confirm();
            }
        }
    } else if (key == GG::GGK_DELETE) { // Delete would be better, but gets eaten by someone
        // Ask to delete selection on Delete, if valid and not editing text
        if (CheckChoiceValidity() &&
            GG::GUI::GetGUI()->FocusWnd() != m_name_edit &&
            GG::GUI::GetGUI()->FocusWnd() != m_current_dir_edit)
        {
            AskDelete();
        }
    } else {
        // The keypress may have changed our choice
        CheckChoiceValidity();
    }
}

void SaveFileDialog::Confirm() {
    DebugLogger() << "SaveFileDialog::Confirm: Confirming";

    if (!CheckChoiceValidity()) {
        DebugLogger() << "SaveFileDialog::Confirm: Invalid choice. abort.";
        return;
    }

    /// Check if we chose a directory
    std::string choice = m_name_edit->Text();
    if (choice.empty()) {
        DebugLogger() << "SaveFileDialog::Confirm: Returning no file.";
        CloseClicked();
        return;
    }

    fs::path choice_path = FilenameToPath(choice);
    DebugLogger() << "choice: " << choice << " valid utf-8: " << IsValidUTF8(choice);

    fs::path current_dir = FilenameToPath(GetDirPath());
    DebugLogger() << "current dir PathString: " << PathString(current_dir) << " valid utf-8: " << IsValidUTF8(PathString(current_dir));

    fs::path chosen_full_path = current_dir / choice_path;
    DebugLogger() << "chosen_full_path PathString: " << PathString(chosen_full_path) << " valid utf-8: " << IsValidUTF8(PathString(chosen_full_path));
    DebugLogger() << "chosen_full_path is directory? : " << fs::is_directory(chosen_full_path);


    if (fs::is_directory(chosen_full_path)) {
        DebugLogger() << "SaveFileDialog::Confirm: " << PathString(chosen_full_path) << " is a directory. Listing content.";
        UpdateDirectory(PathString(chosen_full_path));
        return;

    } else if (!m_load_only) {
        // append appropriate extension if invalid
        std::string chosen_ext = fs::path(chosen_full_path).extension().string();
        if (chosen_ext != m_extension) {
            choice += m_extension;
            chosen_full_path += m_extension;
            m_name_edit->SetText(m_name_edit->Text() + m_extension);
        }
        DebugLogger() << "SaveFileDialog::Confirm: File " << PathString(chosen_full_path) << " chosen.";
        // If not loading and file exists(and is regular file), ask to confirm override
        if (fs::is_regular_file(chosen_full_path)) {
            std::string question = str((FlexibleFormat(UserString("SAVE_REALLY_OVERRIDE")) % choice));
            if (!Prompt(question))
                return;
        } else if (fs::exists(chosen_full_path)) {
            ErrorLogger() << "SaveFileDialog::Confirm: Invalid status for file: " << Result();
            return;
        }
    }

    CloseClicked();
}

void SaveFileDialog::AskDelete() {
    if (m_server_previews)
        return;

    fs::path chosen(Result());
    if (fs::exists (chosen) && fs::is_regular_file (chosen)) {
        std::string filename = m_name_edit->Text();

        boost::format templ(UserString("SAVE_REALLY_DELETE"));

        std::string question = str(templ % filename);
        if (Prompt (question)) {
            fs::remove(chosen);
            // Move selection to next if any or previous, if any
            GG::ListBox::SelectionSet::const_iterator it = m_file_list->Selections().begin();
            if (it != m_file_list->Selections().end()) {
                GG::ListBox::iterator row_it = *it;
                GG::ListBox::iterator next(row_it);
                ++next;
                if (next != m_file_list->end()) {
                    m_file_list->SelectRow(next, true);
                } else if (row_it != m_file_list->begin()) {
                    GG::ListBox::iterator prev(row_it);
                    --prev;
                    m_file_list->SelectRow(next, true);
                }
                delete m_file_list->Erase(row_it);
            }
        }
    }
}

void SaveFileDialog::DoubleClickRow(GG::ListBox::iterator row, const GG::Pt& pt, const GG::Flags<GG::ModKey>& modkeys) {
    m_file_list->SelectRow(row);
    Confirm();
}

void SaveFileDialog::Cancel() {
    DebugLogger() << "SaveFileDialog::Cancel: Dialog Canceled";
    m_name_edit->SetText("");
    CloseClicked();
}

void SaveFileDialog::SelectionChanged(const GG::ListBox::SelectionSet& selections) {
    if ( selections.size() == 1 ) {
        GG::ListBox::Row* row = **selections.begin();
        SaveFileRow* save_row = boost::polymorphic_downcast<SaveFileRow*> ( row );
        m_name_edit -> SetText ( save_row->Filename() );
    } else {
        DebugLogger() << "SaveFileDialog::SelectionChanged: Unexpected selection size: " << selections.size();
    }
    CheckChoiceValidity();
}

void SaveFileDialog::UpdateDirectory(const std::string& newdir) {
    //std::cout << "SaveFileDialog::UpdateDirectory newdir: " << newdir << std::endl;
    SetDirPath(newdir);
    UpdatePreviewList();
}

void SaveFileDialog::DirectoryDropdownSelect(GG::DropDownList::iterator selection) {
    if (selection == m_remote_dir_dropdown->end())
        return;
    GG::DropDownList::Row& row = **selection;
    if (row.size() > 0) {
        GG::Label* control = dynamic_cast<GG::Label*>(row.at(0));
        if (control) {
            UpdateDirectory(control->Text());
        }
    }
}

void SaveFileDialog::UpdatePreviewList() {
    DebugLogger() << "SaveFileDialog::UpdatePreviewList";

    m_file_list->Clear();
    m_file_list->Init();

    // If no browsing, no reloading
    if (!m_server_previews) {
        m_file_list->LoadSaveGamePreviews(FilenameToPath(GetDirPath()), m_extension);
    } else {
        PreviewInformation preview_information;
        HumanClientApp::GetApp()->RequestSavePreviews(GetDirPath(), preview_information);
        m_file_list->LoadSaveGamePreviews(preview_information.previews);
        m_remote_dir_dropdown->Clear();
        SetDirPath(preview_information.folder);

        auto row = GG::Wnd::Create<GG::DropDownList::Row>();
        row->push_back(new CUILabel(SERVER_LABEL));
        m_remote_dir_dropdown->Insert(row);

        for (const std::string& subdir : preview_information.subdirectories) {
            auto row = GG::Wnd::Create<GG::DropDownList::Row>();
            if (subdir.find("/") == 0) {
                row->push_back(new CUILabel(SERVER_LABEL + subdir));
            } else if(subdir.find("./") == 0) {
                row->push_back(new CUILabel(SERVER_LABEL + subdir.substr(1)));
            } else {
                row->push_back(new CUILabel(SERVER_LABEL + "/" + subdir));
            }

            m_remote_dir_dropdown->Insert(row);
        }
    }

    // HACK: Sometimes the first row is not drawn without this
    m_file_list->BringRowIntoView(m_file_list->begin());

    // Remember which directory we are showing
    //m_loaded_dir = GetDirPath();

    CheckChoiceValidity();
}

bool SaveFileDialog::CheckChoiceValidity() {
    // Check folder validity
    if (!m_server_previews) {
        fs::path dir(FilenameToPath(GetDirPath()));
        if (fs::exists(dir) && fs::is_directory(dir)) {
            m_current_dir_edit->SetColor(ClientUI::TextColor());
        } else {
            m_current_dir_edit->SetColor(GG::CLR_RED);
        }
    } else {
        if (HasRow(m_remote_dir_dropdown, m_current_dir_edit->Text())) {
            m_current_dir_edit->SetColor(ClientUI::TextColor());
        } else {
            m_current_dir_edit->SetColor(GG::CLR_RED);
        }
    }

    // Check file name validity
    if (m_load_only) {
        if (!m_file_list->HasFile(m_name_edit->Text())) {
            m_confirm_btn->Disable();
            return false;
        } else {
            m_confirm_btn->Disable(false);
            return true;
        }
    }

    return true;
}

void SaveFileDialog::FileNameEdited(const std::string& filename)
{ CheckChoiceValidity(); }

void SaveFileDialog::DirectoryEdited(const string& filename)
{ CheckChoiceValidity(); }

std::string SaveFileDialog::GetDirPath() const {
    const std::string& path_edit_text = m_current_dir_edit->Text();

    //std::cout << "SaveFileDialog::GetDirPath text: " << path_edit_text << " valid UTF-8: " << utf8::is_valid(path_edit_text.begin(), path_edit_text.end()) << std::endl;
    //DebugLogger() << "SaveFileDialog::GetDirPath text: " << path_edit_text << " valid UTF-8: " << utf8::is_valid(path_edit_text.begin(), path_edit_text.end());
    if (!m_server_previews) {
        return path_edit_text;
    }

    std::string dir = path_edit_text;

    // We want to indicate at all times that the saves are on the server.
    if (dir.find(SERVER_LABEL) != 0) {
        if (dir.find("/") != 0)
            dir = "/" + dir;

        dir = SERVER_LABEL + dir;
    }

    // We should now be sure that the path is SERVER/whatever
    if (dir.length() < SERVER_LABEL.size()) {
        ErrorLogger() << "SaveFileDialog::GetDirPath: Error decorating directory for server: not long enough";
        return ".";
    } else {
        // Translate the server label into the standard relative path marker the server understands
        std::string retval = "." + dir.substr(SERVER_LABEL.size());
        //std::cout << "SaveFileDialog::GetDirPath retval: " << retval << " valid UTF-8: " << utf8::is_valid(retval.begin(), retval.end()) << std::endl;
        DebugLogger() << "SaveFileDialog::GetDirPath retval: " << retval << " valid UTF-8: " << utf8::is_valid(retval.begin(), retval.end());
        return retval;
    }
}

void SaveFileDialog::SetDirPath(const string& path) {
    std::string dirname = path;
    if (m_server_previews) {
        // convert the path string into the boost filesystem "generic" format
        // to allow scanning for delimeters like "/"
        dirname = GenericPathString(fs::path(path));
        // Indicate that the path is on the server
        if (path.find("./") == 0) {
            dirname = SERVER_LABEL + path.substr(1);
        } else if (path.find(SERVER_LABEL) == 0) {
            // Already has label. No need to change
        } else {
            dirname = SERVER_LABEL + "/" + path;
        }
    } else {
        // Normalize path
        fs::path path(dirname);
        if (fs::is_directory(path)) {
            path = fs::canonical(path);
            dirname = PathString(path);
        }
    }
    m_current_dir_edit->SetText(dirname);
}

std::string SaveFileDialog::Result() const {
    std::string choice = m_name_edit->Text();
    if (choice.empty()) {
        return "";
    }

    fs::path choice_path = FilenameToPath(choice);
    fs::path current_dir = FilenameToPath(GetDirPath());
    fs::path chosen_full_path = current_dir / choice_path;

    return PathString(chosen_full_path);
}
