// -*- C++ -*-
#ifndef _OptionsWnd_h_
#define _OptionsWnd_h_

#ifndef _CUIWnd_h_
#include "CUIWnd.h"
#endif

#ifndef _OptionsDB_h_
#include "../util/OptionsDB.h"
#endif

#ifndef _CUISpin_h_
#include "CUISpin.h"
#endif

#include <utility>
#include <vector>


class ColorSelector;
class CUIStateButton;
namespace GG { class TabWnd; }

//! This is a dialog box that allows the user to control certain basic game parameters, such as sound and music
class OptionsWnd : public CUIWnd
{
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
    //!@}

private:
    typedef void (OptionsWnd::* VolumeSliderHandler)(int, int, int);

    void                BeginPage(const std::string& name);
    void                EndPage();
    void                BeginSection(const std::string& name);
    void                EndSection();
    CUIStateButton*     BoolOption(const std::string& option_name, const std::string& text);
    CUISpin<int>*       IntOption(const std::string& option_name, const std::string& text);
    CUISpin<double>*    DoubleOption(const std::string& option_name, const std::string& text);
    void                MusicVolumeOption();
    void                VolumeOption(const std::string& toggle_option_name, const std::string& volume_option_name, const std::string& text,
                                     VolumeSliderHandler volume_slider_handler, bool toggle_value);
    void                FileOptionImpl(const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator, bool directory, bool relative_path);
    void                FileOption(const std::string& option_name, const std::string& text, const boost::filesystem::path& path, StringValidator string_validator = 0);
    void                FileOption(const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::pair<std::string, std::string>& filter, StringValidator string_validator = 0);
    void                FileOption(const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator = 0);
    void                DirectoryOption(const std::string& option_name, const std::string& text, const boost::filesystem::path& path);
    void                SoundFileOption(const std::string& option_name, const std::string& text);
    void                ColorOption(const std::string& option_name, const std::string& text);
    void                FontOption(const std::string& option_name, const std::string& text);
    void                ResolutionOption();

    void                Init();
    void                DoneClicked();
    void                MusicClicked(bool checked);
    void                MusicVolumeSlid(int pos, int low, int high);
    void                UISoundsVolumeSlid(int pos, int low, int high);

    CUIListBox* m_current_option_list;
    int         m_indentation_level;
    GG::TabWnd* m_tabs;
    CUIButton*  m_done_button;
    int         m_num_wnds;
};

#endif // _OptionsWnd_h_
