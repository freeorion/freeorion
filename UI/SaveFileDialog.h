// -*- C++ -*-
//SaveFileDialog.h
#ifndef _SaveFileDialog_h_
#define _SaveFileDialog_h_

#include <set>
#include <string>
#include <GG/DropDownList.h>
#include <GG/ListBox.h>
#include <GG/Layout.h>

#include "CUIWnd.h"

class SaveFileRow;
class SaveFileListBox;
struct PreviewInformation;

/** A dialog for choosing save files.
 * Shows some additional infoormation in the listing and more
 * in a preview section to the side.
 */
class SaveFileDialog : public CUIWnd {
public:
    /** \name Structors */ //@{
    /// Constructor for local browsing
    /// @param extension The extension to enforce on the file name
    /// @param load If set to true, only allow choosing existing files
    /// @param relative Return a relative file name
    SaveFileDialog(const std::string& extension, bool load = false);

    /// Contruct for getting the previews from the server
    SaveFileDialog(bool load = false);
    ~SaveFileDialog(); //!< dtor
    //@}

    /** \name Mutators */ //@{
    virtual void ModalInit(); //< Called when dialog is shown. Overrides
    virtual void KeyPress(GG::Key key, boost::uint32_t key_code_point,
                          GG::Flags<GG::ModKey> mod_keys);
    //@}

    /// Get the chosen save files full path
    std::string Result() const;

private:
    void Init();

    void Confirm();                         //!< when m_save_btn button is pressed
    void AskDelete();                       //!< when a file is trying to be deleted
    void DoubleClickRow(GG::ListBox::iterator row);
    void Cancel();                          //!< when m_load_btn button is pressed
    void SelectionChanged(const GG::ListBox::SelectionSet& files);      //!< When file selection changes.
    void UpdateDirectory(const std::string& newdir);                    //!< Change current directory
    void DirectoryDropdownSelect(GG::DropDownList::iterator selection); //!< On remote directory select

    void UpdatePreviewList();
    bool CheckChoiceValidity();                         //!< Disables confirm if filename invalid. Returns false if not valid.
    void FileNameEdited(const std::string& filename);   //!< Called when the filename changes
    void DirectoryEdited(const std::string& filename);  //!< Called when the directory text changes

    std::string GetDirPath() const;         //!< Gets the current directory path string clean of display decorations
    void        SetDirPath(const std::string& path); //!< Sets the shown directory path, applying decorations if applicable

    GG::Layout*         m_layout;           //!< The layout of the dialog

    SaveFileListBox*    m_file_list;        //!< The list of available saves
    GG::Edit*           m_name_edit;        //!< The file name edit control
    GG::Edit*           m_current_dir_edit; //!< The editor for the save directory
    GG::DropDownList*   m_remote_dir_dropdown; //!< Dropdown to select remote dir
    GG::Button*         m_confirm_btn;      //!< Button to confirm choice

    std::string         m_loaded_dir;       //!< The directory whose contents are currently shown
    std::string         m_extension;        //!< The save game file name extension
    bool                m_load_only;        //!< Whether we are loading
    bool                m_server_previews;  //!< If true, get the previews from the server

    /// Remove copy ctor, assign
    SaveFileDialog(const SaveFileDialog&);
    SaveFileDialog& operator=(const SaveFileDialog&);
};

#endif // _SaveFileDialog_h_
