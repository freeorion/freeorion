#ifndef _SaveFileDialog_h_
#define _SaveFileDialog_h_

#include <set>
#include <string>
#include <GG/DropDownList.h>
#include <GG/ListBox.h>
#include <GG/Layout.h>

#include "CUIWnd.h"

#include <boost/filesystem/path.hpp>

class SaveFileRow;
class SaveFileListBox;
struct PreviewInformation;

/** A dialog for choosing save files.
 * Shows some additional infoormation in the listing and more
 * in a preview section to the side.
 */
class SaveFileDialog : public CUIWnd {
public:
    enum class Purpose : bool {Save, Load};
    enum class SaveType : bool {SinglePlayer, MultiPlayer};

    SaveFileDialog(const Purpose purpose = Purpose::Load, const SaveType type = SaveType::SinglePlayer);

    void CompleteConstruction() override;
    ~SaveFileDialog() = default;

    void ModalInit() override;

    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    /** Update the previews with \p preview_info*/
    void SetPreviewList(PreviewInformation&& preview_info);

    /// Get the chosen save files full path
    [[nodiscard]] boost::filesystem::path ResultPath() const;
    [[nodiscard]] std::string ResultString() const;

protected:
    GG::Rect CalculatePosition() const override;

private:
    void Init();

    void Confirm();                         //!< when m_save_btn button is pressed
    void AskDelete();                       //!< when a file is trying to be deleted
    void DoubleClickRow(GG::ListBox::iterator row, GG::Pt pt, GG::Flags<GG::ModKey> mod);
    void Cancel();                          //!< when m_load_btn button is pressed
    void SelectionChanged(const GG::ListBox::SelectionSet& files);  //!< When file selection changes.
    void UpdateDirectory(std::string newdir);                       //!< Change current directory

    /** Either directly update from the local save directory, or request the
        server for save preview information*/
    void UpdatePreviewList();
    /** Update the preview list from a local save directory*/
    void SetPreviewList(const boost::filesystem::path& path);
    /** Update the previews with preview info set by \p setup_preview_info*/
    void SetPreviewListCore(const std::function<void ()>& setup_preview_info);

    bool CheckChoiceValidity();                         //!< Disables confirm if filename invalid. Returns false if not valid.
    void FileNameEdited(const std::string& filename);   //!< Called when the filename changes
    void DirectoryEdited(const std::string& filename);  //!< Called when the directory text changes

    std::string GetDirPath() const;                     //!< Gets the current directory path string clean of display decorations
    void        SetDirPath(std::string path);           //!< Sets the shown directory path, applying decorations if applicable

    std::shared_ptr<GG::Layout>         m_layout;           //!< The layout of the dialog;

    std::shared_ptr<SaveFileListBox>    m_file_list;        //!< The list of available saves
    std::shared_ptr<GG::Edit>           m_name_edit;        //!< The file name edit control;
    std::shared_ptr<GG::Edit>           m_current_dir_edit; //!< The editor for the save directory;
    std::shared_ptr<GG::Button>         m_confirm_btn;      //!< Button to confirm choice;

    std::string         m_loaded_dir;       //!< The directory whose contents are currently shown
    std::string         m_extension;        //!< The save game file name extension
    bool                m_load_only;        //!< Whether we are loading
    bool                m_server_previews;  //!< If true, get the previews from the server

    /// Remove copy ctor, assign
    SaveFileDialog(const SaveFileDialog&);
    SaveFileDialog& operator=(const SaveFileDialog&);
};


#endif
