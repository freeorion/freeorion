#ifndef _OptionsWnd_h_
#define _OptionsWnd_h_

#include <utility>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <GG/GGFwd.h>

#include "CUIWnd.h"


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

protected:
    virtual GG::Rect CalculatePosition() const;

private:

    class SoundOptionsFeedback {
    public:
        SoundOptionsFeedback() :
            m_effects_button(0),
            m_music_button(0)
        {}

        void SetEffectsButton(GG::StateButton * const button);
        void SetMusicButton(GG::StateButton * const button);

        void SoundEffectsEnableClicked(bool checked);
        void MusicClicked(bool checked);
        void MusicVolumeSlid(int pos, int low, int high) const;
        void UISoundsVolumeSlid(int pos, int low, int high) const;
        void SoundInitializationFailure(std::runtime_error const &e);

    private:
        GG::StateButton* m_effects_button;
        GG::StateButton* m_music_button;
    };

    void                DoLayout();

    GG::ListBox*        CreatePage(const std::string& name);
    void                CreateSectionHeader(GG::ListBox* page, int indentation_level, const std::string& name);
    void                HotkeysPage();
    GG::StateButton*    BoolOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    GG::Spin<int>*      IntOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    GG::Spin<double>*   DoubleOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    void                HotkeyOption(GG::ListBox* page, int indentation_level, const std::string& hotkey_name);
    void                MusicVolumeOption(GG::ListBox* page, int indentation_level, SoundOptionsFeedback &fb);
    void                VolumeOption(GG::ListBox* page, int indentation_level, const std::string& toggle_option_name,
                                     const std::string& volume_option_name, const std::string& text, bool toggle_value,
                                     SoundOptionsFeedback &fb);
    void                FileOptionImpl(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator, bool directory, bool relative_path);
    void                FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path, StringValidator string_validator = 0);
    void                FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::pair<std::string, std::string>& filter, StringValidator string_validator = 0);
    void                FileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path, const std::vector<std::pair<std::string, std::string> >& filters, StringValidator string_validator = 0);
    void                DirectoryOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text, const boost::filesystem::path& path);
    void                SoundFileOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    void                ColorOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    void                FontOption(GG::ListBox* page, int indentation_level, const std::string& option_name, const std::string& text);
    void                ResolutionOption(GG::ListBox* page, int indentation_level);

    void                DoneClicked();

    GG::TabWnd* m_tabs;
    GG::Button* m_done_button;

    // Enable and disable the sound when audio options are changed.
    SoundOptionsFeedback m_sound_feedback;

};

#endif // _OptionsWnd_h_
