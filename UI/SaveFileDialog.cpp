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
#include "../util/Serialize.h"

#include <GG/Button.h>
#include <GG/Clr.h>
#include <GG/DrawUtil.h>
#include <GG/utf8/checked.h>
#include <GG/dialogs/FileDlg.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cast.hpp>
#include <fstream>

namespace fs = boost::filesystem;

namespace {
    const GG::X SAVE_FILE_DIALOG_WIDTH ( 600 );
    const GG::Y SAVE_FILE_DIALOG_HEIGHT ( 400 );
    const GG::X SAVE_FILE_DIALOG_MIN_WIDTH ( 160 );
    const GG::Y SAVE_FILE_DIALOG_MIN_HEIGHT ( 100 );

    const GG::X WIDE_COLUMN_WIDTH ( 198 );

    const GG::X SAVE_FILE_BUTTON_MARGIN ( 10 );
    const unsigned int SAVE_FILE_CELL_MARGIN = 2;
    const unsigned int ROW_MARGIN = 2;
    const std::string UNABLE_TO_OPEN_FILE ( "Unable to open file" );

    /// Splits time and date on separate lines for an ISO datetime string
    std::string split_time ( const std::string& time ) {
        std::string result = time;
        std::string::size_type pos = result.find ( "T" );
        if ( pos != std::string::npos ) {
            result.replace ( pos, 1, "\n" );
        }
        return result;
    }

    /// Populates a SaveGamePreviewData from a given file
    /// returns true on success, false if preview data could not be found
    bool LoadSaveGamePreviewData ( const fs::path& path, SaveGamePreviewData& preview ) {
        if ( !fs::exists ( path ) ) {
            Logger().debugStream() << "LoadSaveGamePreviewData: Save file note found: " << path.string();
            return false;
        }

        fs::ifstream ifs ( path, std::ios_base::binary );

        if ( !ifs )
            throw std::runtime_error ( UNABLE_TO_OPEN_FILE );
        try {
            freeorion_iarchive ia ( ifs );
            Logger().debugStream() << "LoadSaveGamePreviewData: Loading preview from:" << path.string();
            ia >> BOOST_SERIALIZATION_NVP ( preview );
        } catch ( const std::exception& e ) {
            Logger().errorStream() << "LoadSaveGamePreviewData: Failed to read preview of " << path.string() << " because: " << e.what();
            return false;
        }
        if ( preview.Valid() ) {
            Logger().debugStream() << "LoadSaveGamePreviewData: Successfully loaded preview from:" << path.string();
            return true;
        } else {
            Logger().debugStream() << "LoadSaveGamePreviewData: Passing save file with no preview: " << path.string();
            return false;
        }
    }

    /// Creates a text control that support resizing and word wrap.
    GG::TextControl* CreateResizingText(const std::string& string,
                                        const boost::shared_ptr<GG::Font>& font,
                                        const GG::Clr& color)
    {
        // Calculate the extent manually to ensure the control stretches to full
        // width when possible.  Otherwise it would always word break.
        GG::Pt extent = font->TextExtent ( string );
        GG::TextControl* text = new GG::TextControl ( GG::X0, GG::Y0, extent.x, extent.y,
                                                      string, font, color,
                                                      GG::FORMAT_WORDBREAK | GG::FORMAT_LEFT );
        text->ClipText ( true );
        text->SetChildClippingMode ( GG::Wnd::ClipToClient );
        return text;
    }
}

/** A Specialized row for the save dialog list box. */
class SaveFileRow: public GG::ListBox::Row {
public:
    /// What sort of a row
    enum RowType {
        HEADER, PREVIEW
    };

    /// Creates a header row
    SaveFileRow() :
        m_type ( HEADER )
    {
        SetMargin ( ROW_MARGIN );
        CalculateColumnWidths();

        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        GG::Clr head_clr = ClientUI::TextColor();

        push_back ( new GG::TextControl ( GG::X0, GG::Y0, UserString ( "SAVE_TIME_TITLE" ),     font, head_clr ) );
        push_back ( new GG::TextControl ( GG::X0, GG::Y0, UserString ( "SAVE_TURN_TITLE" ),     font, head_clr ) );
        push_back ( new GG::TextControl ( GG::X0, GG::Y0, UserString ( "SAVE_PLAYER_TITLE" ),   font, head_clr ) );
        push_back ( new GG::TextControl ( GG::X0, GG::Y0, UserString ( "SAVE_EMPIRE_TITLE" ),   font, head_clr ) );
        push_back ( new GG::TextControl ( GG::X0, GG::Y0, UserString ( "SAVE_FILE_TITLE" ),     font, head_clr ) );
    }

    /// Creates a row for the given savefile
    SaveFileRow ( const std::string& filename, const SaveGamePreviewData& preview ) :
        m_type ( PREVIEW )
    {
        SetMargin ( ROW_MARGIN );
        this->m_filename = filename;
        CalculateColumnWidths();

        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        GG::Clr item_clr = ClientUI::TextColor();

        std::string save_time = split_time ( preview.save_time );


        push_back ( new GG::TextControl ( GG::X0, GG::Y0, m_time_column_width, font->Height(),
                                          save_time, font, item_clr,
                                          GG::FORMAT_LEFT ) );
        push_back ( new GG::TextControl ( GG::X0, GG::Y0, m_turn_column_width, font->Height(),
                                          std::string ( " " ) + boost::lexical_cast<std::string> ( preview.current_turn ), font, item_clr,
                                          GG::FORMAT_LEFT ) );

        push_back ( CreateResizingText(preview.main_player_name, font, item_clr) );
        push_back ( CreateResizingText(preview.main_player_empire_name, font, preview.main_player_empire_colour) );
        push_back ( CreateResizingText(filename, font, item_clr) );
    }

    const std::string&  Filename() const {
        return m_filename;
    }

    const RowType       Type() const {
        return m_type;
    }

    // Overrides
    virtual void        Render() {
        if ( m_type != HEADER ) {
            GG::FlatRectangle ( ClientUpperLeft(), ClientLowerRight() - GG::Pt ( GG::X ( SAVE_FILE_CELL_MARGIN ), GG::Y0 ),
                                GG::CLR_ZERO, ClientUI::WndOuterBorderColor(), 1u );
        }
    }

    /** Excludes border from the client area. */
    virtual GG::Pt      ClientUpperLeft() const {
        return UpperLeft() + GG::Pt ( GG::X ( SAVE_FILE_CELL_MARGIN ), GG::Y ( SAVE_FILE_CELL_MARGIN ) );
    }

    /** Excludes border from the client area. */
    virtual GG::Pt      ClientLowerRight() const {
        return LowerRight() - GG::Pt ( GG::X ( SAVE_FILE_CELL_MARGIN*2 ), GG::Y ( SAVE_FILE_CELL_MARGIN ) );
    }

    virtual void        SizeMove ( const GG::Pt& ul, const GG::Pt& lr ) {
        if ( GetLayout() ) {
            GG::Layout* layout = GetLayout();
            int cols = layout->Columns();
            if ( cols > 0 ) {
                layout->SetColumnStretch ( 0, 0 );
                layout->SetMinimumColumnWidth ( 0, m_time_column_width );
            }
            if ( cols > 1 ) {
                layout->SetColumnStretch ( 1, 0 );
                layout->SetMinimumColumnWidth ( 1, m_turn_column_width );
            }
            if ( cols > 2 ) {
                layout->SetColumnStretch ( 2, 1.0 );
            }
            if ( cols > 3 ) {
                layout->SetColumnStretch ( 3, 1.0 );
            }
            if ( cols > 4 ) {
                layout->SetColumnStretch ( 4, 1.0 );
            }
        }
        GG::ListBox::Row::SizeMove ( ul, lr );
    }

    private:
    void                CalculateColumnWidths() {
        boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
        // We need to maintain the fixed sizes since the base list box messes them
        m_time_column_width = font->TextExtent ( "YYYY-MM-DD\nHH:MM.ss" ).x + GG::X ( SAVE_FILE_CELL_MARGIN );
        m_turn_column_width = font->TextExtent ( "9999" ).x + GG::X ( SAVE_FILE_CELL_MARGIN );
    }

    std::string m_filename;
    GG::X m_time_column_width;
    GG::X m_turn_column_width;
    RowType m_type;
};

class SaveFileListBox : public CUIListBox {
public:
    SaveFileListBox() :
        CUIListBox ( GG::X0, GG::Y0, GG::X1, GG::Y1 )
    {
        SetNumCols ( 5 );
        SetColWidth ( 0, GG::X0 );
        SetColWidth ( 1, GG::X0 );
        SetColWidth ( 2, GG::X0 );
        SetColWidth ( 3, GG::X0 );
        SetColWidth ( 4, GG::X0 );
        LockColWidths();
    }

    virtual void SizeMove ( const GG::Pt& ul, const GG::Pt& lr ) {
        const GG::Pt old_size = Size();
        CUIListBox::SizeMove ( ul, lr );
        if ( old_size != Size() ) {
            RefreshRowSizes();
        }
    }

    void RefreshRowSizes() {
        const GG::Pt row_size = ListRowSize();
        ColHeaders().Resize ( row_size );
        for ( GG::ListBox::iterator it = begin(); it != end(); ++it ) {
            ( *it )->Resize ( row_size );
        }
    }

    GG::Pt ListRowSize() const {
        return GG::Pt ( Width() - RightMargin(), ListRowHeight() );
    }

    static GG::Y ListRowHeight() {
        return GG::Y ( ClientUI::Pts() * 2 );
    }

    /// Loads preview data on all save files in a directory specidifed by path.
    /// @param [in] path The path of the directory
    /// @param [out] previews The preview datas indexed by file names
    void LoadSaveGamePreviews ( const fs::path& path, const std::string& extension ) {
        SaveGamePreviewData data;
        fs::directory_iterator end_it;

        if ( !fs::exists ( path ) ) {
            Logger().errorStream() << "SaveFileListBox::LoadSaveGamePreviews: Save Game directory \"" << path << "\" not found";
            return;
        }
        if ( !fs::is_directory ( path ) ) {
            Logger().errorStream() << "SaveFileListBox::LoadSaveGamePreviews: Save Game directory \"" << path << "\" was not a directory";
            return;
        }

        for ( fs::directory_iterator it ( path ); it != end_it; ++it ) {
            try {
                std::string filename = PathString ( it->path().filename() );
                if ( it->path().filename().extension() == extension ) {
                    if ( LoadSaveGamePreviewData ( *it, data ) ) {
                        // Add preview entry to list
                        Insert ( new SaveFileRow ( filename, data ) );
                    }
                }
            } catch ( const std::exception& e ) {
                Logger().errorStream() << "SaveFileListBox::LoadSaveGamePreviews: Failed loading preview from " << it->path() << " because: " << e.what();
            }
        }
    }

    bool HasFile ( const std::string& filename ) {
        for ( GG::ListBox::iterator it = begin(); it != end(); ++it ) {
            SaveFileRow* row = dynamic_cast<SaveFileRow*> ( *it );
            if ( row ) {
                if ( row->Filename() == filename ) {
                    return true;
                }
            }
        }
        return false;
    }
};

SaveFileDialog::SaveFileDialog (const std::string& extension, bool load ) :
CUIWnd ( UserString ( "GAME_MENU_SAVE_FILES" ),
         ( GG::GUI::GetGUI()->AppWidth() - SAVE_FILE_DIALOG_WIDTH ) / 2,
         ( GG::GUI::GetGUI()->AppHeight() - SAVE_FILE_DIALOG_HEIGHT ) / 2,
         SAVE_FILE_DIALOG_WIDTH,
         SAVE_FILE_DIALOG_HEIGHT,
         GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL | GG::RESIZABLE ) {
    m_extension = extension;
    m_load_only = load;
    boost::shared_ptr<GG::Font> font = ClientUI::GetFont();
    SetMinSize ( GG::Pt ( 2*SAVE_FILE_DIALOG_MIN_WIDTH, 2*SAVE_FILE_DIALOG_MIN_HEIGHT ) );

    m_layout = new GG::Layout ( GG::X0, GG::Y0,
                                SAVE_FILE_DIALOG_WIDTH - LeftBorder() - RightBorder(),
                                SAVE_FILE_DIALOG_HEIGHT - TopBorder() - BottomBorder(), 3, 4 );
    m_layout->SetCellMargin ( SAVE_FILE_CELL_MARGIN );
    m_layout->SetBorderMargin ( SAVE_FILE_CELL_MARGIN*2 );

    m_file_list = new SaveFileListBox();
    m_file_list->SetStyle ( GG::LIST_SINGLESEL | GG::LIST_SORTDESCENDING );
    m_file_list->SetColHeaders ( new SaveFileRow() );


    m_confirm_btn = new CUIButton ( UserString ( "OK" ) );
    CUIButton* cancel_btn = new CUIButton ( UserString ( "CANCEL" ) );

    m_name_edit = new CUIEdit ( GG::X0, GG::Y0, GG::X1, "", font );
    GG::TextControl* filename_label = new GG::TextControl ( GG::X0, GG::Y0, UserString ( "SAVE_FILENAME" ), font, ClientUI::TextColor() );
    GG::TextControl* directory_label = new GG::TextControl ( GG::X0, GG::Y0, UserString ( "SAVE_DIRECTORY" ), font, ClientUI::TextColor() );
    m_current_dir_edit = new CUIEdit ( GG::X0, GG::Y0, GG::X1, PathString ( GetSaveDir() ), font );
    m_browse_dir_btn = new CUIButton ( "..." );

    m_layout->Add ( m_file_list,    0, 0, 1, 4 );
    m_layout->Add ( filename_label, 1, 0, 1, 1 );
    m_layout->Add ( m_name_edit,    1, 1, 1, 2 );
    m_layout->Add ( m_confirm_btn,    1, 3 );
    m_layout->Add ( directory_label,2, 0 );
    m_layout->Add ( m_current_dir_edit,  2, 1 );
    m_layout->Add ( m_browse_dir_btn,    2, 2 );
    m_layout->Add ( cancel_btn,          2, 3 );


    m_layout->SetRowStretch ( 0, 1.0 );
    m_layout->SetMinimumRowHeight ( 1, font->TextExtent ( m_confirm_btn->Text() ).y );
    m_layout->SetMinimumRowHeight ( 2, font->TextExtent ( cancel_btn->Text() ).y );

    m_layout->SetMinimumColumnWidth ( 0, std::max ( font->TextExtent ( filename_label->Text() ).x,
                                      font->TextExtent ( directory_label->Text() ).x ) );
    m_layout->SetColumnStretch ( 1, 1.0 );
    m_layout->SetMinimumColumnWidth ( 2, m_browse_dir_btn->MinUsableSize().x +
                                      SAVE_FILE_BUTTON_MARGIN*2 );
    m_layout->SetMinimumColumnWidth ( 3, std::max ( m_confirm_btn->MinUsableSize().x,
                                      cancel_btn->MinUsableSize().x ) +
                                      SAVE_FILE_BUTTON_MARGIN );

    SetLayout ( m_layout );

    GG::Connect ( m_confirm_btn->LeftClickedSignal,      &SaveFileDialog::Confirm,           this );
    GG::Connect ( cancel_btn->LeftClickedSignal,         &SaveFileDialog::Cancel,            this );
    GG::Connect ( m_file_list->SelChangedSignal,         &SaveFileDialog::SelectionChanged,  this );
    GG::Connect ( m_file_list->DoubleClickedSignal,      &SaveFileDialog::DoubleClickRow,    this );
    GG::Connect ( m_browse_dir_btn->LeftClickedSignal,   &SaveFileDialog::BrowseDirectories, this );
    GG::Connect ( m_name_edit->EditedSignal,             &SaveFileDialog::FileNameEdited,    this );

    UpdatePreviewList();
    
    if(!m_load_only){
        m_name_edit->SetText(std::string("save-") + FilenameTimestamp());
        m_name_edit->SelectAll();
    }
}

SaveFileDialog::~SaveFileDialog()
{}

void SaveFileDialog::ModalInit() {
    GG::Wnd::ModalInit();
    GG::GUI::GetGUI()->SetFocusWnd(m_name_edit);
}

void SaveFileDialog::KeyPress ( GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys ) {
    // Return without filename
    if ( key == GG::GGK_ESCAPE || key == GG::GGK_F10 ) {
        Cancel();
        return;
    }

    // Update list on enter if directory changed by hand
    if ( key == GG::GGK_RETURN ) {
        if ( m_loaded_dir != m_current_dir_edit->Text() ) {
            UpdatePreviewList();
        }else{
            if(GG::GUI::GetGUI()->FocusWnd() == m_name_edit){
                Confirm();
            }
        }
    } else {
        // The keypress may have changed our choice
        CheckChoiceValidity();
    }


}

void SaveFileDialog::Confirm() {
    Logger().debugStream() << "SaveFileDialog::Confirm: Confirming";
    
    if(!CheckChoiceValidity()){
        Logger().debugStream() << "SaveFileDialog::Confirm: Invalid choice. abort.";
        return;
    }

    /// Check if we chose a directory
    std::string choice = m_name_edit->Text();
    fs::path current_dir ( m_current_dir_edit->Text() );
    if ( !choice.empty() ) {
        fs::path chosen = current_dir / choice;
        if ( fs::is_directory ( chosen ) ) {
            Logger().debugStream() << "SaveFileDialog::Confirm: " << chosen << " is a directory. Listing content.";
            UpdateDirectory ( PathString ( chosen ) );
            return;
        } else {
            Logger().debugStream() << "SaveFileDialog::Confirm: File " << chosen << " chosen.";
        }
    } else {
        Logger().debugStream() << "SaveFileDialog::Confirm: Returning no file.";
    }

    CloseClicked();
}

void SaveFileDialog::DoubleClickRow ( GG::ListBox::iterator row ) {
    m_file_list->SelectRow ( row );
    Confirm();
}

void SaveFileDialog::Cancel() {
    Logger().debugStream() << "SaveFileDialog::Cancel: Dialog Canceled";

    m_name_edit->SetText ( "" );

    CloseClicked();
}

void SaveFileDialog::SelectionChanged ( const GG::ListBox::SelectionSet& selections ) {
    if ( selections.size() == 1 ) {
        GG::ListBox::Row* row = **selections.begin();
        SaveFileRow* save_row = boost::polymorphic_downcast<SaveFileRow*> ( row );
        m_name_edit -> SetText ( save_row->Filename() );
    } else {
        Logger().debugStream() << "SaveFileDialog::SelectionChanged: Unexpected selection size: " << selections.size();
    }
    CheckChoiceValidity();
}

void SaveFileDialog::BrowseDirectories() {
    std::vector<std::pair<std::string, std::string> > dummy;
    FileDlg dlg ( m_current_dir_edit->Text(), "", false, false, dummy );
    dlg.SelectDirectories ( true );
    dlg.Run();
    if ( !dlg.Result().empty() ) {
        // Normalize the path by converting it into a path and back
        fs::path choice ( *dlg.Result().begin() );
        UpdateDirectory ( PathString ( fs::canonical ( choice ) ) );
    }
}

void SaveFileDialog::UpdateDirectory ( const std::string& newdir ) {
    m_current_dir_edit->SetText ( newdir );
    UpdatePreviewList();
}

void SaveFileDialog::UpdatePreviewList() {
    Logger().debugStream() << "SaveFileDialog::UpdatePreviewList";

    m_file_list->Clear();
    // Needed because there is a bug in ListBox, where the headers
    // never resize to less wide
    m_file_list->RemoveColHeaders();
    m_file_list->SetColHeaders ( new SaveFileRow() );
    m_file_list->LoadSaveGamePreviews ( m_current_dir_edit->Text(), m_extension );

    // Forces the width to recompute
    m_file_list->RefreshRowSizes();
    // HACK: Sometimes the first row is not drawn without this
    m_file_list->BringRowIntoView ( m_file_list->begin() );

    // Remember which directory we are showing
    m_loaded_dir = m_current_dir_edit->Text();

    CheckChoiceValidity();
}

bool SaveFileDialog::CheckChoiceValidity() {
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

void SaveFileDialog::FileNameEdited ( const std::string& filename ) {
    CheckChoiceValidity();
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
        fs::path current_dir ( m_current_dir_edit->Text() );
        return PathString ( ( current_dir / filename ).string() );
    }
}
