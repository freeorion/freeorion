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
#include <GG/DrawUtil.h>
#include <GG/utf8/checked.h>
#include <GG/dialogs/ThreeButtonDlg.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/cast.hpp>
#include <boost/format.hpp>

#include <string>

namespace fs = boost::filesystem;

using std::vector;
using std::string;

namespace {
    const GG::X SAVE_FILE_DIALOG_WIDTH ( 600 );
    const GG::Y SAVE_FILE_DIALOG_HEIGHT ( 400 );
    const GG::X SAVE_FILE_DIALOG_MIN_WIDTH ( 160 );
    const GG::Y SAVE_FILE_DIALOG_MIN_HEIGHT ( 100 );

    const GG::X PROMT_WIDTH ( 200 );
    const GG::Y PROMPT_HEIGHT ( 75 );

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
        db.Add<std::string>("UI.save-file-dialog.columns", UserStringNop("OPTIONS_DB_UI_SAVE_DIALOG_COLUMNS"),
                            "time,turn,player,empire,file");
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
    CUILabel* CreateResizingText(const std::string& string)
    {
        // Calculate the extent manually to ensure the control stretches to full
        // width when possible.  Otherwise it would always word break.
        GG::Pt extent = ClientUI::GetFont()->TextExtent(string);
        CUILabel* text = new CUILabel(string, GG::FORMAT_WORDBREAK | GG::FORMAT_LEFT);
        text->Resize(GG::Pt(extent.x, extent.y));
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
        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
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

        for (unsigned i = 0; i < list->NumRows(); ++i) {
            const GG::DropDownList::Row& row = list->GetRow(i);
            for (unsigned j = 0; j < row.size(); ++j) {
                const GG::Control* control = row.at(j);
                const CUILabel* text = dynamic_cast<const CUILabel*>(control);
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
    static std::vector<SaveFileColumn> GetColumns(){
        std::vector<SaveFileColumn> columns;
        for (unsigned int i = 0; i < VALID_PREVIEW_COLUMN_COUNT; ++i)
            columns.push_back(GetColumn(VALID_PREVIEW_COLUMNS[i]));
        return columns;
    }

    static GG::Control* TitleForColumn(const SaveFileColumn& column)
    {
        CUILabel* retval = new CUILabel(column.Title(), GG::FORMAT_LEFT);
        retval->Resize(GG::Pt(GG::X1, ClientUI::GetFont()->Height()));
        return retval;
    }

    static GG::Control* CellForColumn(const SaveFileColumn& column, const FullPreview& full)
    {
        GG::Clr color = ClientUI::TextColor();
        std::string value = ColumnInPreview(full, column.m_name);
        if (column.m_name == "empire")
            color = full.preview.main_player_empire_colour;

        GG::Flags<GG::TextFormat> format_flags = GG::FORMAT_LEFT;
        if (column.m_name == "turn")
            format_flags = GG::FORMAT_CENTER;

        CUILabel* retval = 0;

        if (column.m_fixed) {
            retval = new CUILabel(value, format_flags);
            retval->Resize(GG::Pt(column.FixedWidth(), ClientUI::GetFont()->Height()));
        } else {
            retval = CreateResizingText(value);
        }

        retval->SetTextColor(color);
        return retval;
    }

    bool Fixed() const
    { return m_fixed; }

    GG::X FixedWidth() const {
        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        // We need to maintain the fixed sizes since the base list box messes them
        return std::max(font->TextExtent(m_wide_as).x, font->TextExtent(Title()).x)
            + GG::X(SAVE_FILE_CELL_MARGIN);
    }

    double Stretch() const
    { return m_stretch; }

    const std::string& Name() const
    { return m_name; }

private:
    static SaveFileColumn GetColumn(const std::string& name) {
        const std::string prefix = "UI.save-file-dialog.";
        std::string option = prefix + name + ".";
        OptionsDB&  db = GetOptionsDB();

        if (db.OptionExists(option + WIDE_AS)) {
            return SaveFileColumn(name, db.Get<std::string>(option + WIDE_AS));
        } else if (db.OptionExists(option + STRETCH)) {
            return SaveFileColumn(name, db.Get<double>(option + STRETCH));
        } else if (db.OptionExists(prefix + "default." + STRETCH)) {
            return SaveFileColumn(name, db.Get<double>(prefix + "default." + STRETCH));
        } else {
            return SaveFileColumn(name, DEFAULT_STRETCH);
        }
    }

    /// Creates a fixed width column
    SaveFileColumn(const std::string& name, const std::string& wide_as) :
        m_name(name),
        m_fixed(true),
        m_wide_as(wide_as),
        m_stretch(0.0)
    {}

    /// Creates a stretchy column
    SaveFileColumn(const std::string& name, double stretch) :
        m_name(name),
        m_fixed(false),
        m_wide_as(),
        m_stretch(stretch)
    {}

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
            Logger().errorStream() << "SaveFileColumn::Title Error: no such preview field: " << m_name;
            return "???";
        }
    }

    /// The identifier of what data to show. Must be valid.
    std::string m_name;
    /// If true, column width is fixed to be the width of m_wide_as under the current font.
    /// If false, column stretches with factor m_stretch
    bool m_fixed;
    /// The string to be used in determining the width of the column
    std::string m_wide_as;
    /// The stretch of the column.
    double m_stretch;
};

/** A Specialized row for the save dialog list box. */
class SaveFileRow: public GG::ListBox::Row {
public:
    /// What sort of a row
    enum RowType {
        HEADER, PREVIEW, DIRECTORY
    };

    /// Creates a header row
    SaveFileRow(const std::vector<SaveFileColumn>& columns) :
        m_type(HEADER)
    {
        SetMargin(ROW_MARGIN);

        for (std::vector<SaveFileColumn>::const_iterator it = columns.begin(); it != columns.end(); ++it)
        { push_back(SaveFileColumn::TitleForColumn(*it)); }
        AdjustColumns(columns);
    }

    /// Creates a row for the given savefile
    SaveFileRow(const FullPreview& full, const std::vector<SaveFileColumn>& visible_columns,
                const std::vector<SaveFileColumn>& columns, int tooltip_delay) :
        m_type(PREVIEW)
    {
        SetMargin (ROW_MARGIN);
        this->m_filename = full.filename;

        VarText browse_text("SAVE_DIALOG_ROW_BROWSE_TEMPLATE");

        for (std::vector<SaveFileColumn>::const_iterator it = visible_columns.begin();
             it != visible_columns.end(); ++it)
        {
            push_back(SaveFileColumn::CellForColumn(*it, full));
        }
        for (std::vector<SaveFileColumn>::const_iterator it = columns.begin();
             it != columns.end(); ++it)
        {
            browse_text.AddVariable(it->Name(), ColumnInPreview(full, it->Name(), false));
        }
        AdjustColumns(visible_columns);
        SetBrowseModeTime(tooltip_delay);
        SetBrowseText(browse_text.GetText());
    }

    /// Creates a directory row
    SaveFileRow(const std::string& directory) :
        m_filename(directory),
        m_type (DIRECTORY)
    {
        SetMargin(ROW_MARGIN);

        push_back(new CUILabel(PATH_DELIM_BEGIN + directory + PATH_DELIM_END, GG::FORMAT_NOWRAP));
        GetLayout()->SetColumnStretch(0, 1.0);
    }

    const std::string&  Filename() const
    { return m_filename; }

    const RowType       Type() const
    { return m_type; }

    // Overrides
    virtual void        Render() {
        if (m_type != HEADER) {
            GG::FlatRectangle(ClientUpperLeft(),
                              ClientLowerRight() - GG::Pt(GG::X(SAVE_FILE_CELL_MARGIN), GG::Y0),
                              GG::CLR_ZERO, ClientUI::WndOuterBorderColor(), 1u);
        }
    }

    /** Excludes border from the client area. */
    virtual GG::Pt      ClientUpperLeft() const {
        return UpperLeft() + GG::Pt(GG::X(SAVE_FILE_CELL_MARGIN),
                                    GG::Y(SAVE_FILE_CELL_MARGIN));
    }

    /** Excludes border from the client area. */
    virtual GG::Pt      ClientLowerRight() const {
        return LowerRight() - GG::Pt(GG::X(SAVE_FILE_CELL_MARGIN * 2),
                                     GG::Y(SAVE_FILE_CELL_MARGIN));
    }

    void AdjustColumns(const std::vector<SaveFileColumn>& columns) {
        if (Type() != DIRECTORY) {
            GG::Layout* layout = GetLayout();
            for (unsigned int i = 0; i < columns.size(); ++i){
                const SaveFileColumn& column = columns[i];
                layout->SetColumnStretch ( i, column.Stretch() );
                layout->SetMinimumColumnWidth ( i, column.FixedWidth() ); // Considers header
            }
        } else {
            // Give the directory label at least all the room that the other columns demand anyway
            GG::Layout* layout = GetLayout();
            GG::X sum(0);
            for (unsigned int i = 0; i < columns.size(); ++i) {
                const SaveFileColumn& column = columns[i];
                sum += column.FixedWidth();
            }
            layout->SetMinimumColumnWidth (0, sum);
        }
    }

private:

    std::string m_filename;
    RowType     m_type;
};

class SaveFileListBox : public CUIListBox {
public:
    SaveFileListBox() :
        CUIListBox ()
    {
        m_columns = SaveFileColumn::GetColumns();
        m_visible_columns = FilterColumns();
        SetNumCols(m_visible_columns.size());
        for (unsigned int i = 0; i < m_visible_columns.size(); ++i)
        { SetColWidth(i, GG::X1); }
        LockColWidths();
        SetColHeaders(new SaveFileRow(m_visible_columns));
        SetSortCmp(&SaveFileListBox::DirectoryAwareCmp);
        SetVScrollWheelIncrement(WHEEL_INCREMENT);
    }

    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove(ul, lr);
        if (old_size != Size())
            RefreshRowSizes();
    }

    void RefreshRowSizes() {
        ResetColHeaders();
        const GG::Pt row_size = ListRowSize();
        ColHeaders().Resize(row_size);
        dynamic_cast<SaveFileRow*>(&ColHeaders())->AdjustColumns(m_visible_columns);
        for (GG::ListBox::iterator it = begin(); it != end(); ++it) {
            SaveFileRow* row = dynamic_cast<SaveFileRow*>(*it);
            if (row)
                row->AdjustColumns(m_visible_columns);
            (*it)->Resize(row_size);
        }
    }

    void ResetColHeaders() {
        RemoveColHeaders();
        SetColHeaders(new SaveFileRow(m_visible_columns));
    }

    GG::Pt ListRowSize() const
    { return GG::Pt(Width() - RightMargin(), ListRowHeight()); }

    static GG::Y ListRowHeight()
    { return GG::Y(ClientUI::Pts() * 2); }

    /// Loads preview data on all save files in a directory specidifed by path.
    /// @param [in] path The path of the directory
    /// @param [out] previews The preview datas indexed by file names
    void LoadSaveGamePreviews(const fs::path& path, const std::string& extension) {
        LoadDirectories(path);

        vector<FullPreview> previews;
        ::LoadSaveGamePreviews(path, extension, previews);
        LoadSaveGamePreviews(previews);
    }

    /// Loads preview data on all save files in a directory specidifed by path.
    /// @param [in] path The path of the directory
    /// @param [out] previews The preview datas indexed by file names
    void LoadSaveGamePreviews(const std::vector<FullPreview>& previews) {
        int tooltip_delay = GetOptionsDB().Get<int>("UI.save-file-dialog.tooltip-delay");
        for (vector<FullPreview>::const_iterator it = previews.begin(); it != previews.end(); ++it)
            Insert(new SaveFileRow(*it, m_visible_columns, m_columns, tooltip_delay));
    }

    void LoadDirectories(const fs::path& path) {
        fs::directory_iterator end_it;
        if (path.has_parent_path() && path.parent_path() != path)
            Insert(new SaveFileRow(".."));

        for (fs::directory_iterator it(path); it != end_it; ++it) {
            if (fs::is_directory(it->path()))
                Insert(new SaveFileRow(PathString(it->path().filename())));
        }
    }

    bool HasFile(const std::string& filename) {
        for (GG::ListBox::iterator it = begin(); it != end(); ++it) {
            SaveFileRow* row = dynamic_cast<SaveFileRow*>(*it);
            if (row && row->Filename() == filename)
                    return true;
        }
        return false;
    }

private:
    std::vector<SaveFileColumn> m_columns;
    std::vector<SaveFileColumn> m_visible_columns;

    std::vector<SaveFileColumn> FilterColumns() {
        std::string names_string = GetOptionsDB().Get<std::string>("UI.save-file-dialog.columns");
        std::vector<std::string> names;
        std::vector< SaveFileColumn > columns;
        boost::split(names, names_string,boost::is_any_of(","));
        for (std::vector<std::string>::const_iterator it = names.begin();
             it != names.end(); ++it)
        {
            for (std::vector<SaveFileColumn>::const_iterator jt = m_columns.begin();
                 jt != m_columns.end(); ++jt)
            {
                if (jt->Name() == *it) {
                    columns.push_back(*jt);
                    break;
                }
            }
            Logger().errorStream() << "SaveFileListBox::FilterColumns: Column not found: " << *it;
        }
        Logger().debugStream() << "SaveFileDialog::FilterColumns: Visible columns: " << columns.size();
        return columns;
    }

    static bool IsDelimitedWith(const std::string& str, const std::string& begin,
                                const std::string& end)
    {
        return str.find(begin) == 0 && str.find(end) == str.size() - end.size();
    }

    /// We want the timestamps to be sorted in ascending order to get the latest save
    /// first, but we want the directories to
    /// a) always be first
    /// b) be sorted alphabetically, ignoring the PATH_DELIM_BEGIN and PATH_DELIM_END they are wrapped with
    /// This custom comparer achieves these goals.
    static bool DirectoryAwareCmp(const Row& row1, const Row& row2, int column_int) {
        unsigned column = 0;
        if (column_int >= 0) {
            column = column_int;
        }
        std::string key1;
        std::string key2;
        // If column is other than the first one,
        // use the first one for columns that don't have that many,
        // since they should be directories.
        if (row1.size() < column) {
            key1 = row1.SortKey(column);
        } else {
            key1 = row1.SortKey(0);
        }
        if (row2.size() < column) {
            key2 = row2.SortKey(column);
        } else {
            key2 = row2.SortKey(0);
        }
        const int total_delim_size = PATH_DELIM_BEGIN.size() + PATH_DELIM_END.size();
        const bool row1_is_directory = IsDelimitedWith(key1, PATH_DELIM_BEGIN, PATH_DELIM_END);
        const bool row2_is_directory = IsDelimitedWith(key2, PATH_DELIM_BEGIN, PATH_DELIM_END);
        if (!row1_is_directory && !row2_is_directory) {
            return key1.compare(key2) <= 0;
        } else if ( row1_is_directory && row2_is_directory ) {
            // Clean out the delimiters
            std::string true_key1 = key1.substr(PATH_DELIM_BEGIN.size(), key1.size() - total_delim_size );
            std::string true_key2 = key2.substr(PATH_DELIM_BEGIN.size(), key2.size() - total_delim_size );
            // Compare directories alphabetically ascending, while the timestamp comparisons are descending
            return true_key1.compare(true_key2) >= 0;
        } else if ( row1_is_directory && !row2_is_directory ){
            return false;
        } else {
            return true;
        }
    }
};

SaveFileDialog::SaveFileDialog (const std::string& extension, bool load) :
    CUIWnd(UserString("GAME_MENU_SAVE_FILES"),
           (GG::GUI::GetGUI()->AppWidth() - SAVE_FILE_DIALOG_WIDTH) / 2,
           (GG::GUI::GetGUI()->AppHeight() - SAVE_FILE_DIALOG_HEIGHT) / 2,
           SAVE_FILE_DIALOG_WIDTH, SAVE_FILE_DIALOG_HEIGHT,
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE)
{
    m_extension = extension;
    m_load_only = load;
    m_server_previews = false;
    Init();
}

SaveFileDialog::SaveFileDialog (bool load) :
    CUIWnd(UserString("GAME_MENU_SAVE_FILES"),
           (GG::GUI::GetGUI()->AppWidth() - SAVE_FILE_DIALOG_WIDTH) / 2,
           (GG::GUI::GetGUI()->AppHeight() - SAVE_FILE_DIALOG_HEIGHT) / 2,
           SAVE_FILE_DIALOG_WIDTH, SAVE_FILE_DIALOG_HEIGHT,
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE)
{
    m_load_only = load;
    m_server_previews = true;
    m_extension = MP_SAVE_FILE_EXTENSION;
    Init();
}

void SaveFileDialog::Init() {
    SetMinSize(GG::Pt(2*SAVE_FILE_DIALOG_MIN_WIDTH, 2*SAVE_FILE_DIALOG_MIN_HEIGHT));

    m_layout = new GG::Layout(GG::X0, GG::Y0,
                              SAVE_FILE_DIALOG_WIDTH - LeftBorder() - RightBorder(),
                              SAVE_FILE_DIALOG_HEIGHT - TopBorder() - BottomBorder(), 4, 4);
    m_layout->SetCellMargin(SAVE_FILE_CELL_MARGIN);
    m_layout->SetBorderMargin(SAVE_FILE_CELL_MARGIN*2);

    m_file_list = new SaveFileListBox();
    m_file_list->SetStyle(GG::LIST_SINGLESEL | GG::LIST_SORTDESCENDING);

    m_confirm_btn = new CUIButton(UserString("OK"));
    CUIButton* cancel_btn = new CUIButton(UserString("CANCEL"));

    m_name_edit = new CUIEdit("");
    CUILabel* filename_label = new CUILabel(UserString("SAVE_FILENAME"), GG::FORMAT_NOWRAP);
    CUILabel* directory_label = new CUILabel(UserString("SAVE_DIRECTORY"), GG::FORMAT_NOWRAP);
    m_current_dir_edit = new CUIEdit(PathString(GetSaveDir()));

    m_layout->Add(directory_label, 0, 0);

    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    if (!m_server_previews) {
        m_layout->Add(m_current_dir_edit, 0, 1, 1, 3);

        CUIButton* delete_btn = new CUIButton(UserString("DELETE"));
        m_layout->Add(delete_btn, 2, 3);
        GG::Connect(delete_btn->LeftClickedSignal, &SaveFileDialog::AskDelete, this);

        m_layout->SetMinimumRowHeight(2, delete_btn->MinUsableSize().y + GG::Y(Value(SAVE_FILE_BUTTON_MARGIN)));
        m_layout->SetMinimumColumnWidth(2, m_confirm_btn->MinUsableSize().x + 2*SAVE_FILE_BUTTON_MARGIN);
        m_layout->SetMinimumColumnWidth(3, std::max( cancel_btn->MinUsableSize().x,
                                        delete_btn->MinUsableSize().x) + SAVE_FILE_BUTTON_MARGIN);
    } else {
        m_remote_dir_dropdown = new CUIDropDownList(SAVE_FILE_DIALOG_HEIGHT/2);
        m_layout->Add(m_current_dir_edit, 0, 1, 1, 1);
        m_layout->Add(m_remote_dir_dropdown, 0, 2 , 1, 2);
        GG::X drop_width = font->TextExtent(SERVER_LABEL+SERVER_LABEL+SERVER_LABEL).x;
        m_layout->SetMinimumColumnWidth(2, std::max(m_confirm_btn->MinUsableSize().x + 2*SAVE_FILE_BUTTON_MARGIN, drop_width/2));
        m_layout->SetMinimumColumnWidth(3, std::max(cancel_btn->MinUsableSize().x + SAVE_FILE_BUTTON_MARGIN, drop_width / 2));

        GG::Connect(m_remote_dir_dropdown->SelChangedSignal,   &SaveFileDialog::DirectoryDropdownSelect, this );
    }

    m_layout->Add(m_file_list,      1, 0, 1, 4);
    m_layout->Add(filename_label,   2, 0);
    m_layout->Add(m_name_edit,      3, 0, 1, 2);
    m_layout->Add(m_confirm_btn,    3, 2);
    m_layout->Add(cancel_btn,       3, 3);

    m_layout->SetMinimumRowHeight(0, m_current_dir_edit->MinUsableSize().y);
    m_layout->SetRowStretch      (1, 1.0 );
    m_layout->SetMinimumRowHeight(3, font->TextExtent(cancel_btn->Text()).y);

    m_layout->SetMinimumColumnWidth(0, std::max(font->TextExtent(filename_label->Text()).x,
                                                font->TextExtent(directory_label->Text()).x));
    m_layout->SetColumnStretch(1, 1.0);

    SetLayout(m_layout);

    GG::Connect(m_confirm_btn->LeftClickedSignal,               &SaveFileDialog::Confirm,           this);
    GG::Connect(cancel_btn->LeftClickedSignal,                  &SaveFileDialog::Cancel,            this);
    GG::Connect(m_file_list->SelChangedSignal,                  &SaveFileDialog::SelectionChanged,  this);
    GG::Connect(m_file_list->DoubleClickedSignal,               &SaveFileDialog::DoubleClickRow,    this);
    GG::Connect(m_name_edit->EditedSignal,                      &SaveFileDialog::FileNameEdited,    this);
    GG::Connect(m_current_dir_edit->EditedSignal,               &SaveFileDialog::DirectoryEdited,   this);
    GG::Connect(HumanClientApp::GetApp()->FocusChangedSignal,   &SaveFileDialog::UpdatePreviewList, this);

    if (!m_load_only) {
        m_name_edit->SetText(std::string("save-") + FilenameTimestamp());
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

void SaveFileDialog::ModalInit() {
    GG::Wnd::ModalInit();
    GG::GUI::GetGUI()->SetFocusWnd(m_name_edit);
}

void SaveFileDialog::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys ) {
    // Return without filename
    if (key == GG::GGK_ESCAPE) {
        Cancel();
        return;
    }

    // Update list on enter if directory changed by hand
    if (key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) {
        if (m_loaded_dir != GetDirPath() ) {
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
    Logger().debugStream() << "SaveFileDialog::Confirm: Confirming";

    if (!CheckChoiceValidity()) {
        Logger().debugStream() << "SaveFileDialog::Confirm: Invalid choice. abort.";
        return;
    }

    /// Check if we chose a directory
    std::string choice = m_name_edit->Text();
    fs::path current_dir(GetDirPath());
    if (!choice.empty()) {
        fs::path chosen = current_dir / choice;
        if (fs::is_directory(chosen)) {
            Logger().debugStream() << "SaveFileDialog::Confirm: " << chosen << " is a directory. Listing content.";
            UpdateDirectory(PathString(chosen));
            return;
        } else {
            Logger().debugStream() << "SaveFileDialog::Confirm: File " << chosen << " chosen.";
            // If not loading and file exists, ask to confirm override
            // Use Result() to ensure we are using the extension.
            if ( !m_load_only && fs::exists(fs::path(Result())) ) {
                boost::format templ( UserString("SAVE_REALLY_OVERRIDE") );
                std::string question = (templ % choice).str();
                if(!Prompt(question)){
                    return;
                }
            }
        }
    } else {
        Logger().debugStream() << "SaveFileDialog::Confirm: Returning no file.";
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
            GG::ListBox::SelectionSet::iterator it = m_file_list->Selections().begin();
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

void SaveFileDialog::DoubleClickRow(GG::ListBox::iterator row) {
    m_file_list->SelectRow ( row );
    Confirm();
}

void SaveFileDialog::Cancel() {
    Logger().debugStream() << "SaveFileDialog::Cancel: Dialog Canceled";
    m_name_edit->SetText ( "" );
    CloseClicked();
}

void SaveFileDialog::SelectionChanged(const GG::ListBox::SelectionSet& selections) {
    if ( selections.size() == 1 ) {
        GG::ListBox::Row* row = **selections.begin();
        SaveFileRow* save_row = boost::polymorphic_downcast<SaveFileRow*> ( row );
        m_name_edit -> SetText ( save_row->Filename() );
    } else {
        Logger().debugStream() << "SaveFileDialog::SelectionChanged: Unexpected selection size: " << selections.size();
    }
    CheckChoiceValidity();
}

void SaveFileDialog::UpdateDirectory(const std::string& newdir) {
    SetDirPath(newdir);
    UpdatePreviewList();
}

void SaveFileDialog::DirectoryDropdownSelect(GG::DropDownList::iterator selection) {
    GG::DropDownList::Row& row = **selection;
    if (row.size() > 0) {
        CUILabel* control = dynamic_cast<CUILabel*>(row[0]);
        if (control) {
            UpdateDirectory(control->Text());
        }
    }
}

void SaveFileDialog::UpdatePreviewList() {
    Logger().debugStream() << "SaveFileDialog::UpdatePreviewList";

    m_file_list->Clear();
    // Needed because there is a bug in ListBox, where the headers
    // never resize to less wide
    m_file_list->ResetColHeaders();
    // If no browsing, no reloading
    if (!m_server_previews) {
        m_file_list->LoadSaveGamePreviews(GetDirPath(), m_extension);
    } else {
        PreviewInformation preview_information;
        HumanClientApp::GetApp()->RequestSavePreviews(GetDirPath(), preview_information);
        m_file_list->LoadSaveGamePreviews(preview_information.previews);
        m_remote_dir_dropdown->Clear();
        SetDirPath(preview_information.folder);

        GG::DropDownList::Row* row = new GG::DropDownList::Row();
        row->push_back(new CUILabel(SERVER_LABEL));
        m_remote_dir_dropdown->Insert(row);

        for (std::vector<std::string>::const_iterator
             it = preview_information.subdirectories.begin();
             it != preview_information.subdirectories.end(); ++it)
        {
            GG::DropDownList::Row* row = new GG::DropDownList::Row();
            if (it->find("/") == 0) {
                row->push_back(new CUILabel(SERVER_LABEL + *it));
            } else if(it->find("./") == 0) {
                row->push_back(new CUILabel(SERVER_LABEL + it->substr(1)));
            } else {
                row->push_back(new CUILabel(SERVER_LABEL + "/" + *it));
            }

            m_remote_dir_dropdown->Insert(row);
        }
    }

    // Forces the width to recompute
    m_file_list->RefreshRowSizes();
    // HACK: Sometimes the first row is not drawn without this
    m_file_list->BringRowIntoView(m_file_list->begin());

    // Remember which directory we are showing
    m_loaded_dir = GetDirPath();

    CheckChoiceValidity();
}

bool SaveFileDialog::CheckChoiceValidity() {
    // Check folder validity
    if ( !m_server_previews ) {
        fs::path dir ( GetDirPath() );
        if (fs::exists (dir) && fs::is_directory (dir)) {
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
    if ( m_load_only ) {
        if ( !m_file_list->HasFile ( m_name_edit->Text() ) ) {
            m_confirm_btn->Disable();
            return false;
        } else {
            m_confirm_btn->Disable ( false );
            return true;
        }
    }else{
        return true;
    }
}

void SaveFileDialog::FileNameEdited(const std::string& filename)
{ CheckChoiceValidity(); }

void SaveFileDialog::DirectoryEdited(const string& filename)
{ CheckChoiceValidity(); }

std::string SaveFileDialog::GetDirPath() const{
    std::string dir = m_current_dir_edit->Text();

    if (m_server_previews) {
        // We want to indicate at all times that the saves are on the server.
        if (dir.find(SERVER_LABEL) != 0) {
            if (dir.find("/") != 0)
                dir = "/" + dir;

            dir = SERVER_LABEL + dir;
        }

        // We should now be sure that the path is SERVER/whatever
        if (dir.length() < SERVER_LABEL.size()) {
            Logger().errorStream() << "SaveFileDialog::GetDirPath: Error decorating directory for server: not long enough";
            return ".";
        } else {
            // Translate the server label into the standard relative path marker the server understands
            return std::string(".") + dir.substr(SERVER_LABEL.size());
        }
    } else {
        return dir;
    }
}

void SaveFileDialog::SetDirPath(const string& path) {
    std::string dirname = path;
    if (m_server_previews) {
        // Change the format into the generic one
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
    std::string filename = m_name_edit->Text();
    if ( filename.empty() ) {
        return "";
    } else {
        // Ensure the file has an extension
        std::string::size_type end = filename.length();
        std::size_t ext_length = m_extension.length();
        if ( filename.length() < ext_length || filename.substr ( end-ext_length, ext_length ) != m_extension ) {
            filename.append ( m_extension );
        }
        // Convert to a path to create a generic string representation.
        fs::path current_dir ( GetDirPath() );
        return ( current_dir / filename ).generic_string();
    }
}
