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

class CUITabbedPages;
class ColorSelector;

//! This is a dialog box that allows the user to control certain basic game parameters, such as sound and music
class OptionsWnd : public CUIWnd
{
public:
    //! \name Structors
    //!@{
    OptionsWnd();  //!< default ctor
    ~OptionsWnd(); //!< dtor
    //!@}

    //! \name Mutators
    //!@{
    virtual void KeyPress (GG::Key key, Uint32 key_mods);
    //!@}

    bool m_end_with_done;    //!< determines whether or not we ended the dialog with OK or not

private:
    struct BrowseForFileFunctor
    {
        BrowseForFileFunctor(const std::string& key_, const std::string& dir_key_, const std::string& file_type_,
                             const std::string& file_extension_, CUIEdit* edit_, OptionsWnd* wnd_);
        void operator()();
        const std::string key;
        const std::string dir_key;
        const std::string file_type;
        const std::string file_extension;
        CUIEdit* const edit;
        OptionsWnd* const wnd;
    };

    struct BrowseForSoundFileFunctor
    {
        BrowseForSoundFileFunctor(const std::string& key_, CUIEdit* edit_, OptionsWnd* wnd_);
        void operator()();
        const std::string key;
        CUIEdit* const edit;
        OptionsWnd* const wnd;
    };

    void Init();
	static std::string BrowseForFile(const std::string& directory, const std::string& file, const std::vector<std::pair<std::string, std::string> >& file_types);
	static void FillCombo(CUIDropDownList* combo, const std::vector<std::string>& values, const std::string& currentValue);
	void FillLists();
	void FillFontList();
	void FillFontCombo(CUIDropDownList* combo, const std::string& option_name);
	void Browse(const std::string& optionName, const std::string& optionDir, const std::string& userString, const std::string& extension, CUIEdit* editControl);
	void BrowseSoundFile(const std::string& optionName, CUIEdit* editControl);
	void AddSoundControls(int x, int y, const std::string& userString, GG::Wnd* pageWnd, const std::string& optionName, CUIEdit*& editControl, CUIButton*& btn, bool connect_file_browser = true);
	void AddColorControls(int x, int y, const std::string& userString, GG::Wnd* pageWnd, const std::string& optionName, ColorSelector*& comboColor);
	void AddFolderControls(int x, int y, const std::string& userString, GG::Wnd* pageWnd, const std::string& optionName, CUIEdit*& editControl);
	void AddFontControls(int x, int y, const std::string& userString, GG::Wnd* pageWnd, CUIDropDownList*& combo, const std::string& optionName, const std::string& userStringSize, CUISpin<int>*& spin);
	bool TestFolder(std::string& value);
	void UpdateFileOption(const std::string& option_name, std::string file_name, bool folder = false);

	// General button handlers
    void DoneClicked();

	// UI Page handlers
	void TextFont(int selection);
	void TitleFont(int selection);
	void Resolution(int selection);
	void ColorDepth(int selection);

	// Sound Page (1) handlers
	void MusicClicked(bool checked);
	void UIEffectsClicked(bool checked);
    void MusicVolumeSlid(int pos, int low, int high);
    void UISoundsVolumeSlid(int pos, int low, int high);

	// Folders page handlers
	void SettingsDirFocusUpdate(const std::string& value);
	void ArtDirFocusUpdate(const std::string& value);
	void SaveDirFocusUpdate(const std::string& value);
	void SoundDirFocusUpdate(const std::string& value);

	std::vector<std::string> m_fonts;
	std::vector<std::string> m_resolutions;
	std::vector<std::string> m_colorDepth;

	CUITabbedPages*		m_tabs;	//!< Tabbed pages control
    CUIButton*			m_done_btn;    //!< Done button

	// UI page members
	CUIEdit*			m_language_edit;
	CUIDropDownList*	m_comboTextFont;
	CUIDropDownList*	m_comboTitleFont;
	CUIEdit*			m_music_edit;
	CUIEdit*			m_alert_edit;
	CUIEdit*			m_typing_edit;
	CUIEdit*			m_turn_edit;
	CUIEdit*			m_sidePanel_edit;
	CUIEdit*			m_planet_edit;
	CUIEdit*			m_close_edit;
	CUIEdit*			m_maximize_edit;
	CUIEdit*			m_minimize_edit;
	CUIEdit*			m_balanced_edit;
	CUIEdit*			m_industry_edit;
	CUIEdit*			m_farming_edit;
	CUIEdit*			m_mining_edit;
	CUIEdit*			m_research_edit;
	CUIEdit*			m_clickButton_edit;
	CUIEdit*			m_rolloverButton_edit;
	CUIEdit*			m_clickFleet_edit;
	CUIEdit*			m_rolloverFleet_edit;
	CUIEdit*			m_drop_edit;
	CUIEdit*			m_pulldown_edit;
	CUIEdit*			m_select_edit;
	CUIEdit*			m_art_dir_edit;
	CUIEdit*			m_save_dir_edit;
	CUIEdit*			m_font_dir_edit;
	CUIEdit*			m_sound_dir_edit;
	CUIEdit*			m_settings_dir_edit;

    friend struct BrowseForFileFunctor;
};

inline std::string OptionsWndRevision()
{return "$Id$";}

#endif // _OptionsWnd_h_
