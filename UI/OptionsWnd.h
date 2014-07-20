// -*- C++ -*-
#ifndef _OptionsWnd_h_
#define _OptionsWnd_h_

#include "CUIWnd.h"
#include "CUISpin.h"

#include <utility>
#include <vector>


class CUIStateButton;
namespace GG { class TabWnd; }

//! This is a dialog box that allows the user to control certain basic game parameters, such as sound and music
class OptionsWnd : public CUIWnd {
public:
    typedef bool (*StringValidator)(const std::string& file);

    //! \name Structors
    //!@{
    OptionsWnd();  //!< default ctor
    ~OptionsWnd(); //!< dtor
    //!@}

    //! \name Mutators
    //!@{
    virtual void KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);

    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //!@}

private:
    typedef void (OptionsWnd::* VolumeSliderHandler)(int, int, int);

    void                DoLayout(void);

    CUIListBox*         CreatePage(const std::string& name);
    void                CreateSectionHeader(CUIListBox* page, int indentation_level, const std::string& name);
    void                HotkeysPage();
    CUIStateButton*     BoolOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    CUISpin<int>*       IntOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    CUISpin<double>*    DoubleOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    void                HotkeyOption(CUIListBox* page, int indentation_level, const std::string& hotkey_name);
    void                MusicVolumeOption(CUIListBox* page, int indentation_level);
    void                VolumeOption(CUIListBox* page, int indentation_level, const std::string& toggle_option_name, const std::string& volume_option_name, const std::string& text,
                                     VolumeSliderHandler volume_slider_handler, bool toggle_value);
    void                FileOptionImpl(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator, bool directory, bool relative_path);
    void                FileOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path, StringValidator string_validator = 0);
    void                FileOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::pair<std::string, std::string>& filter, StringValidator string_validator = 0);
    void                FileOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator = 0);
    void                DirectoryOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path);
    void                SoundFileOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    void                ColorOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    void                FontOption(CUIListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    void                ResolutionOption(CUIListBox* page, int indentation_level);

    void                DoneClicked();
    void                MusicClicked(bool checked);
    void                MusicVolumeSlid(int pos, int low, int high);
    void                UISoundsVolumeSlid(int pos, int low, int high);

    GG::TabWnd* m_tabs;
    CUIButton*  m_done_button;
};

#endif // _OptionsWnd_h_
