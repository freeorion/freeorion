#ifndef _OptionsWnd_h_
#define _OptionsWnd_h_

#include <boost/filesystem/path.hpp>
#include <GG/GGFwd.h>

#include "CUIWnd.h"
#include "Sound.h"

#include <functional>
#include <utility>
#include <vector>


//! This is a dialog box that allows the user to control certain basic game parameters, such as sound and music
class OptionsWnd : public CUIWnd {
public:
    OptionsWnd(bool is_game_running_);
    void CompleteConstruction() override;

    void KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;
    void SizeMove(GG::Pt ul, GG::Pt lr) override;

protected:
    GG::Rect CalculatePosition() const override;

private:
    /**SoundOptionsFeedback enables immediate player feedback when
       sound options are changed.*/
    class SoundOptionsFeedback {
    public:
        SoundOptionsFeedback() {}

        /** Stores a pointer to the sound effects check box.*/
        void SetEffectsButton(std::shared_ptr<GG::StateButton> button);
        /** Stores pointer to music enable check box.*/
        void SetMusicButton(std::shared_ptr<GG::StateButton> button);

        /**Enables/disables sound effects when the sound effects check
           box is clicked.*/
        void SoundEffectsEnableClicked(bool checked);
        /**Enables/disables background music when the music check
           box is clicked.*/
        void MusicClicked(bool checked);
        /** Adjusts the music volume.*/
        void MusicVolumeSlid(int pos, int low, int high) const;
        /** Adjusts the effects volume.*/
        void UISoundsVolumeSlid(int pos, int low, int high) const;
        /** Handles a sound initialization failure by setting sound
            effects and music enable check boxes to disabled and
            informing the player with a popup message box.*/
        void SoundInitializationFailure(Sound::InitializationFailureException const &e);

    private:
        std::shared_ptr<GG::StateButton> m_effects_button;
        std::shared_ptr<GG::StateButton> m_music_button;
    };

    void                DoLayout();

    GG::ListBox*        CreatePage(std::string name);
    void                CreateSectionHeader(GG::ListBox* page, int indentation_level,
                                            std::string name, std::string tooltip = "");
    void                HotkeysPage();
    GG::StateButton*    BoolOption(GG::ListBox* page, int indentation_level, std::string option_name, std::string text);
    GG::Spin<int>*      IntOption(GG::ListBox* page, int indentation_level, std::string option_name, std::string text);
    GG::Spin<double>*   DoubleOption(GG::ListBox* page, int indentation_level, std::string option_name, std::string text);
    void                HotkeyOption(GG::ListBox* page, int indentation_level, std::string hotkey_name);
    void                MusicVolumeOption(GG::ListBox* page, int indentation_level, SoundOptionsFeedback& fb);
    void                VolumeOption(GG::ListBox* page, int indentation_level, std::string_view toggle_option_name,
                                     std::string_view volume_option_name, std::string text, bool toggle_value,
                                     SoundOptionsFeedback &fb);
    void                PathDisplay(GG::ListBox* page, int indentation_level, std::string text,
                                    boost::filesystem::path path);
    void                FileOptionImpl(GG::ListBox* page, int indentation_level, std::string option_name,
                                       std::string text, boost::filesystem::path path,
                                       std::vector<std::pair<std::string, std::string>> filters,
                                       std::function<bool (const std::string&)> string_validator,
                                       bool directory, bool relative_path, bool disabled);
    void                FileOption(GG::ListBox* page, int indentation_level, std::string option_name,
                                   std::string text, boost::filesystem::path path,
                                   std::function<bool (const std::string&)> string_validator = nullptr);
    void                FileOption(GG::ListBox* page, int indentation_level, std::string option_name,
                                   std::string text, boost::filesystem::path path,
                                   std::pair<std::string, std::string> filter,
                                   std::function<bool (const std::string&)> string_validator = nullptr);
    void                FileOption(GG::ListBox* page, int indentation_level, std::string option_name,
                                   std::string text, boost::filesystem::path path,
                                   std::vector<std::pair<std::string, std::string>> filters,
                                   std::function<bool (const std::string&)> string_validator = nullptr);
    void                DirectoryOption(GG::ListBox* page, int indentation_level, std::string option_name,
                                        std::string text, boost::filesystem::path path, bool disabled = false);
    void                SoundFileOption(GG::ListBox* page, int indentation_level,
                                        std::string option_name, std::string text);
    void                ColorOption(GG::ListBox* page, int indentation_level,
                                    std::string option_name, std::string text);
    void                FontOption(GG::ListBox* page, int indentation_level,
                                   std::string option_name, std::string text);
    void                ResolutionOption(GG::ListBox* page, int indentation_level);

    void                DoneClicked();

    bool                        is_game_running;
    std::shared_ptr<GG::TabWnd> m_tabs;
    std::shared_ptr<GG::Button> m_done_button;
    SoundOptionsFeedback        m_sound_feedback;   // Enable and disable the sound when audio options are changed.
};


#endif
