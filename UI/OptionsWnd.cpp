//OptionsWnd.cpp

#include "OptionsWnd.h"

#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"

#include "ClientUI.h"
#include "GGApp.h"
#include "CUITabbedPages.h"
#include "CUISpin.h"
#include "dialogs/GGThreeButtonDlg.h"

#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>

namespace {
    bool temp_header_bool = RecordHeaderFile(OptionsWndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");

	const int PAGE_VERT_MARGIN = 10;
	const int PAGE_ROW_HEIGHT = 30;
	const int PAGE_HORZ_MARGIN = 10;
	const int PAGE_HORZ_OFFSET = 15;
	const int PAGE_WIDTH = 300;
	const int PAGE_HEIGHT = 450;

    const int DROPLIST_HEIGHT = ClientUI::PTS + 4;
    const int DROPLIST_DROP_HEIGHT = DROPLIST_HEIGHT * 5;

	bool OptionPathToNativePath(std::string& path)
	{
		boost::filesystem::path value_path;
		try {
			value_path = boost::filesystem::path(path);
		} catch (const boost::filesystem::filesystem_error& e) {
			return false;
		}
		path = value_path.native_file_string();
		return true;
	}

	bool NativePathToOptionPath(std::string& path, bool folder)
	{
		boost::filesystem::path value_path;
		try {
			value_path = boost::filesystem::path(path, boost::filesystem::native);
		} catch (const boost::filesystem::filesystem_error& e) {
			return false;
		}
		path = value_path.string();
		if (folder) {
			if (path.empty())
				path = "./";
			else if (*path.rbegin() != '/')
				path += '/';
		}
		return true;
	}

    template <class T>
    struct SetOptionFunctor
    {
        SetOptionFunctor(const std::string& key_) : key(key_) {}
        void operator()(const T& value) { GetOptionsDB().Set(key, value); }
        const std::string key;
    };

    template <>
    struct SetOptionFunctor<GG::Clr>
    {
        SetOptionFunctor(const std::string& key_) : key(key_) {}
        void operator()(const GG::Clr& clr) { GetOptionsDB().Set<StreamableColor>(key, clr); }
        const std::string key;
    };

    struct SetFileStringFunctor
    {
        SetFileStringFunctor(const std::string& key_, bool folder_ = false) :
            key(key_),
            folder(folder_)
        {}
        void operator()(const std::string& value)
        {
            std::string path = value;
            if (NativePathToOptionPath(path, folder))
                GetOptionsDB().Set<std::string>(key, path);
        }
        const std::string key;
        const bool folder;
    };
}

#ifdef WIN32
	#define SLASH	'\\'
	#define CURRENT_DIR ".\\"
#else
	#define SLASH	'/'
	#define CURRENT_DIR "./"
#endif


OptionsWnd::BrowseForFileFunctor::BrowseForFileFunctor(const std::string& key_, const std::string& dir_key_,
                                                       const std::string& file_type_,
                                                       const std::string& file_extension_, CUIEdit* edit_,
                                                       OptionsWnd* wnd_) :
    key(key_),
    dir_key(dir_key_),
    file_type(file_type_),
    file_extension(file_extension_),
    edit(edit_),
    wnd(wnd_)
{}

void OptionsWnd::BrowseForFileFunctor::operator()()
{
    wnd->Browse(key, dir_key, file_type, file_extension, edit);
}

OptionsWnd::BrowseForSoundFileFunctor::BrowseForSoundFileFunctor(const std::string& key_, CUIEdit* edit_, OptionsWnd* wnd_) :
    key(key_),
    edit(edit_),
    wnd(wnd_)
{}

void OptionsWnd::BrowseForSoundFileFunctor::operator()()
{
    wnd->Browse(key, "sound-dir", "OPTIONS_SOUND_FILE", ".wav", edit);
}

OptionsWnd::OptionsWnd():
    CUI_Wnd(UserString("OPTIONS_TITLE"),
            (GG::App::GetApp()->AppWidth() - (PAGE_WIDTH + 20)) / 2,
            (GG::App::GetApp()->AppHeight() - (PAGE_HEIGHT + 70)) / 2,
            PAGE_WIDTH + 20, PAGE_HEIGHT + 70, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_end_with_done(false)
{
    m_done_btn = new CUIButton(20, PAGE_HEIGHT + 35, 75, UserString("DONE"));
	m_tabs = new CUITabbedPages(10, 20, PAGE_WIDTH, PAGE_HEIGHT + 20);
    Init();
}

class PageWnd : public GG::Wnd
{
public:
	PageWnd(int x, int y, int w, int h) : GG::Wnd(x, y, w, h, 0) {};
};

void OptionsWnd::AddSoundControls(int x, int y, const std::string& userString, GG::Wnd* pageWnd, const std::string& optionName, CUIEdit*& editControl, CUIButton*& btn, bool connect_file_browser/* = true*/)
{
	GG::TextControl* textControl = new GG::TextControl(x, y + 2, UserString(userString), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	pageWnd->AttachChild(textControl);
	btn = new CUIButton(260, y, textControl->Height(), "...");
	btn->MoveTo(PAGE_WIDTH - btn->Width(), y);
	pageWnd->AttachChild(btn);
	x += textControl->Width() + 5;
	std::string path(GetOptionsDB().Get<std::string>(optionName));
	if (!OptionPathToNativePath(path))
		path = "";
	editControl = new CUIEdit(x, y - 4, PAGE_WIDTH - btn->Width() - x - 5, 26, path);
	pageWnd->AttachChild(editControl);
    if (connect_file_browser)
        GG::Connect(btn->ClickedSignal, BrowseForSoundFileFunctor(optionName, editControl, this));
	GG::Connect(editControl->EditedSignal, SetFileStringFunctor(optionName));
}

void OptionsWnd::AddColorControls(int x, int y, const std::string& userString, GG::Wnd* pageWnd, const std::string& optionName, ColorSelector*& comboColor)
{
	GG::TextControl* textControl = new GG::TextControl(x, y + 2, UserString(userString), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	pageWnd->AttachChild(textControl);
    const int WIDTH = 75;
	comboColor = new ColorSelector(PAGE_WIDTH - WIDTH, y, WIDTH, ClientUI::PTS + 4,
                                   GetOptionsDB().Get<StreamableColor>(optionName).ToClr());
	pageWnd->AttachChild(comboColor);
	GG::Connect(comboColor->ColorChangedSignal, SetOptionFunctor<GG::Clr>(optionName));
}

void OptionsWnd::AddFolderControls(int x, int y, const std::string& userString, GG::Wnd* pageWnd, const std::string& optionName, CUIEdit*& editControl)
{
	GG::TextControl* textControl = new GG::TextControl(x, y + 2, UserString(userString), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	pageWnd->AttachChild(textControl);
	x += textControl->Width() + 5;
	std::string path(GetOptionsDB().Get<std::string>(optionName));
	if (!OptionPathToNativePath(path))
		path = "";
	else if (!path.empty() && (*path.rbegin() != SLASH))
		path += SLASH;
	editControl = new CUIEdit(x, y - 4, PAGE_WIDTH - x, 26, path);
	pageWnd->AttachChild(editControl);
}
void OptionsWnd::FillFontCombo(CUIDropDownList* combo, const std::string& option_name)
{
	std::string s = GetOptionsDB().Get<std::string>(option_name);
	std::string::size_type pos = s.find_last_of('.');
	if (pos != std::string::npos)
		s.erase(pos);
	FillCombo(combo, m_fonts, s);
}

void OptionsWnd::AddFontControls(int x, int y, const std::string& userString, GG::Wnd* pageWnd, CUIDropDownList*& combo,
                                 const std::string& optionName, const std::string& userStringSize, CUISpin<int>*& spin)
{
	GG::TextControl* textControl = new GG::TextControl(x, y + 3, UserString(userString), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	pageWnd->AttachChild(textControl);
	x += textControl->Width() + 5;
	combo = new CUIDropDownList(x, y, 200, DROPLIST_HEIGHT, DROPLIST_DROP_HEIGHT);
	combo->SetStyle(GG::LB_NOSORT);
	FillFontCombo(combo, optionName);
	pageWnd->AttachChild(combo);
	textControl = new GG::TextControl(x, y + 3, UserString(userStringSize), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	textControl->MoveTo(PAGE_WIDTH - 55 - textControl->Width(), y + 3);
	combo->Resize(PAGE_WIDTH - 60 - textControl->Width() - x, combo->Height());
	pageWnd->AttachChild(textControl);
    spin = new CUISpin<int>(PAGE_WIDTH - 50, y - 1, 50, GetOptionsDB().Get<int>(optionName + "-size"), 1, 6, 72, true);
	pageWnd->AttachChild(spin);
}

void OptionsWnd::Init()
{
    TempUISoundDisabler sound_disabler;

	FillLists();

	GG::Wnd* control_page = new PageWnd(0, 0, PAGE_WIDTH, PAGE_HEIGHT);
	GG::Wnd* ui_page = new PageWnd(0, 0, PAGE_WIDTH, PAGE_HEIGHT);
	GG::Wnd* sound_page = new PageWnd(0, 0, PAGE_WIDTH, PAGE_HEIGHT);
	GG::Wnd* sound_page2 = new PageWnd(0, 0, PAGE_WIDTH, PAGE_HEIGHT);
	GG::Wnd* colors_page = new PageWnd(0, 0, PAGE_WIDTH, PAGE_HEIGHT);
	GG::Wnd* colors_page2 = new PageWnd(0, 0, PAGE_WIDTH, PAGE_HEIGHT);
	GG::Wnd* folders_page = new PageWnd(0, 0, PAGE_WIDTH, PAGE_HEIGHT);

    AttachChild(m_done_btn);
	AttachChild(m_tabs);

	GG::TextControl* textControl;
	CUIStateButton* stateBtn;
	CUISlider* sliderControl;
	CUISpin<int>* spinControlInt;
	CUISpin<double>* spinControlDouble;
	CUIButton* btn;
	CUIEdit* editControl;
	CUIDropDownList* comboControl;
	ColorSelector* comboColor;
	std::string s;
	char c[128];

	// Fill up the Control Options tab page:
	/*******************************************
	[x] Autoselect Fleet
	[x] Multiple Fleet Windows
	[x] Quick close windows
	Autosave
		[x] Single Player [x] Multiplayer
		Autosaves to keep [number]^
		Turns between saves [number]^
	*******************************************/
	// [x] Autoselect Fleet
	int y = PAGE_VERT_MARGIN;
	int x = PAGE_HORZ_MARGIN;
	stateBtn = new CUIStateButton(x, y, 75, 20, UserString("OPTIONS_AUTOSELECT_FLEET"), GG::TF_LEFT);
	stateBtn->SetCheck(GetOptionsDB().Get<bool>("UI.fleet-autoselect"));
	GG::Connect(stateBtn->CheckedSignal, SetOptionFunctor<bool>("UI.fleet-autoselect"));
	control_page->AttachChild(stateBtn);
	// [x] Multiple Fleet Windows
	y += PAGE_ROW_HEIGHT;
	stateBtn = new CUIStateButton(x, y, 75, 20, UserString("OPTIONS_MULTIPLE_FLEET_WNDS"), GG::TF_LEFT);
	stateBtn->SetCheck(GetOptionsDB().Get<bool>("UI.multiple-fleet-windows"));
	GG::Connect(stateBtn->CheckedSignal, SetOptionFunctor<bool>("UI.multiple-fleet-windows"));
	control_page->AttachChild(stateBtn);
	// [x] Quick close windows
	y += PAGE_ROW_HEIGHT;
	stateBtn = new CUIStateButton(x, y, 75, 20, UserString("OPTIONS_QUICK_CLOSE_WNDS"), GG::TF_LEFT);
	stateBtn->SetCheck(GetOptionsDB().Get<bool>("UI.window-quickclose"));
	GG::Connect(stateBtn->CheckedSignal, SetOptionFunctor<bool>("UI.window-quickclose"));
	control_page->AttachChild(stateBtn);
	// Autosave
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(x, y, UserString("OPTIONS_AUTOSAVE"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	control_page->AttachChild(textControl);
	// [x] Single Player [x] Multiplayer
	y += PAGE_ROW_HEIGHT - 5;
	x += PAGE_HORZ_OFFSET;
	stateBtn = new CUIStateButton(x, y, 75, 20, UserString("OPTIONS_SINGLEPLAYER"), GG::TF_LEFT);
	stateBtn->SetCheck(GetOptionsDB().Get<bool>("autosave.single-player"));
	GG::Connect(stateBtn->CheckedSignal, SetOptionFunctor<bool>("autosave.single-player"));
	control_page->AttachChild(stateBtn);
	stateBtn = new CUIStateButton(x + 100, y, 75, 20, UserString("OPTIONS_MULTIPLAYER"), GG::TF_LEFT);
	stateBtn->SetCheck(GetOptionsDB().Get<bool>("autosave.multiplayer"));
	GG::Connect(stateBtn->CheckedSignal, SetOptionFunctor<bool>("autosave.multiplayer"));
	control_page->AttachChild(stateBtn);
	// Autosaves to keep [number]^
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(x, y, UserString("OPTIONS_AUTOSAVE_TO_KEEP"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	control_page->AttachChild(textControl);
    spinControlInt = new CUISpin<int>(190, y - 5, 75, GetOptionsDB().Get<int>("autosave.saves"), 1, 1, 100, true);
	GG::Connect(spinControlInt->ValueChangedSignal, SetOptionFunctor<int>("autosave.saves"));
	control_page->AttachChild(spinControlInt);
	// Turns between saves [number]^
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(x, y, UserString("OPTIONS_AUTOSAVE_TURNS_BETWEEN"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	control_page->AttachChild(textControl);
    spinControlInt = new CUISpin<int>(190, y - 5, 75, GetOptionsDB().Get<int>("autosave.turns"), 1, 1, 100, true);
	GG::Connect(spinControlInt->ValueChangedSignal, SetOptionFunctor<int>("autosave.turns"));
	control_page->AttachChild(spinControlInt);

	// Fill up the UI Options tab page:
	/*******************************************
	*Language [file]
	Fonts
		*Text [font] *Size [size]^
		*Title [font] *Size [size]^
	Tech Spacing
		*Horz [space]^ *Vert[space]^
	Tooltip Delay (ms.) [time]^
	*Resolution [list]
	*Color Depth [list]
	Chat
		History [count]^
		Hide Interval [Interval]^
	*******************************************/
	// *Language [file]
	y = PAGE_VERT_MARGIN;
	x = PAGE_HORZ_MARGIN;
	textControl = new GG::TextControl(x, y + 2, UserString("OPTIONS_LANGUAGE"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
	btn = new CUIButton(260, y, textControl->Height(), "...");
	btn->MoveTo(PAGE_WIDTH - btn->Width(), y);
	ui_page->AttachChild(btn);
	GG::Connect(btn->ClickedSignal, BrowseForFileFunctor("stringtable-filename", "settings-dir", "OPTIONS_LANGUAGE_FILE", "stringtable.txt", m_language_edit, this));
	x += textControl->Width() + 5;
	s = GetOptionsDB().Get<std::string>("stringtable-filename");
	if (!OptionPathToNativePath(s))
		s = "";
	editControl = new CUIEdit(x, y - 4, PAGE_WIDTH - btn->Width() - x - 5, 26, s);
	ui_page->AttachChild(editControl);
	GG::Connect(editControl->EditedSignal, SetFileStringFunctor("stringtable-filename"));
	m_language_edit = editControl;
	// Fonts
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN;
	textControl = new GG::TextControl(x, y, UserString("OPTIONS_FONTS"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
	// *Text [font] *Size [size]^
	y += PAGE_ROW_HEIGHT - 5;
	x += PAGE_HORZ_OFFSET;
	AddFontControls(x, y, "OPTIONS_FONT_TEXT", ui_page, m_comboTextFont, "UI.font", "OPTIONS_FONT_SIZE", spinControlInt);
	GG::Connect(m_comboTextFont->SelChangedSignal, &OptionsWnd::TextFont, this);
	GG::Connect(spinControlInt->ValueChangedSignal, SetOptionFunctor<int>("UI.font-size"));
	// *Title [font] *Size [size]^
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN + PAGE_HORZ_OFFSET;
	AddFontControls(x, y, "OPTIONS_FONT_TITLE", ui_page, m_comboTitleFont, "UI.title-font", "OPTIONS_FONT_SIZE", spinControlInt);
	GG::Connect(m_comboTitleFont->SelChangedSignal, &OptionsWnd::TitleFont, this);
	GG::Connect(spinControlInt->ValueChangedSignal, SetOptionFunctor<int>("UI.title-font-size"));
	// Tech Spacing
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN;
	textControl = new GG::TextControl(x, y, UserString("OPTIONS_TECH_SPACING"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
	// *Horz [space]^ *Vert[space]^
	y += PAGE_ROW_HEIGHT - 5;
	x = PAGE_HORZ_MARGIN + PAGE_HORZ_OFFSET;
	textControl = new GG::TextControl(x, y + 4, UserString("OPTIONS_HORIZONTAL"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
	x += textControl->Width() + 5;
    spinControlDouble = new CUISpin<double>(x, y, 60, GetOptionsDB().Get<double>("UI.tech-layout-horz-spacing"), 0.05, 0.1, 10.0, true);
	GG::Connect(spinControlDouble->ValueChangedSignal, SetOptionFunctor<double>("UI.tech-layout-horz-spacing"));
	ui_page->AttachChild(spinControlDouble);
	x += 65;
	textControl = new GG::TextControl(x, y + 4, UserString("OPTIONS_VERTICAL"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
	textControl->MoveTo(PAGE_WIDTH - 65 - textControl->Width(), y + 4);
    spinControlDouble = new CUISpin<double>(PAGE_WIDTH - 60, y, 60, GetOptionsDB().Get<double>("UI.tech-layout-vert-spacing"), 0.05, 0.1, 10.0, true);
	GG::Connect(spinControlDouble->ValueChangedSignal, SetOptionFunctor<double>("UI.tech-layout-vert-spacing"));
	ui_page->AttachChild(spinControlDouble);
	// Tooltip Delay (ms.) [time]^
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN;
	textControl = new GG::TextControl(x, y + 4, UserString("OPTIONS_TOOLTIP_DELAY"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
    spinControlInt = new CUISpin<int>(PAGE_WIDTH - 110, y, 110, GetOptionsDB().Get<int>("UI.tooltip-delay"), 50, 0, 3000, true);
	GG::Connect(spinControlInt->ValueChangedSignal, SetOptionFunctor<int>("UI.tooltip-delay"));
	ui_page->AttachChild(spinControlInt);
	// *Resolution [list]
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN;
	textControl = new GG::TextControl(x, y + 3, UserString("OPTIONS_RESOLUTION"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
	comboControl = new CUIDropDownList(PAGE_WIDTH - 110, y, 110, DROPLIST_HEIGHT, DROPLIST_DROP_HEIGHT);
	comboControl->SetStyle(GG::LB_NOSORT);
	sprintf(c, "%d x %d", GetOptionsDB().Get<int>("app-width"), GetOptionsDB().Get<int>("app-height"));
	FillCombo(comboControl, m_resolutions, c);
	ui_page->AttachChild(comboControl);
	GG::Connect(comboControl->SelChangedSignal, &OptionsWnd::Resolution, this);
	// *Color Depth [list]
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN;
	textControl = new GG::TextControl(x, y + 3, UserString("OPTIONS_COLOR_DEPTH"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
	comboControl = new CUIDropDownList(PAGE_WIDTH - 110, y, 110, DROPLIST_HEIGHT, DROPLIST_DROP_HEIGHT);
	comboControl->SetStyle(GG::LB_NOSORT);
	sprintf(c, "%d", GetOptionsDB().Get<int>("color-depth"));
	FillCombo(comboControl, m_colorDepth, c);
	ui_page->AttachChild(comboControl);
	GG::Connect(comboControl->SelChangedSignal, &OptionsWnd::ColorDepth, this);
	// Chat
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN;
	textControl = new GG::TextControl(x, y, UserString("OPTIONS_CHAT"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
	// History [count]^
	y += PAGE_ROW_HEIGHT - 5;
	x = PAGE_HORZ_MARGIN + PAGE_HORZ_OFFSET;
	textControl = new GG::TextControl(x, y + 4, UserString("OPTIONS_CHAT_HISTORY"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
    spinControlInt = new CUISpin<int>(PAGE_WIDTH - 110, y, 110, GetOptionsDB().Get<int>("UI.chat-edit-history"), 10, 0, 1000, true);
	GG::Connect(spinControlInt->ValueChangedSignal, SetOptionFunctor<int>("UI.chat-edit-history"));
	ui_page->AttachChild(spinControlInt);
	// Hide Interval [Interval]^
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN + PAGE_HORZ_OFFSET;
	textControl = new GG::TextControl(x, y + 4, UserString("OPTIONS_CHAT_HIDE"), ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR);
	ui_page->AttachChild(textControl);
    spinControlInt = new CUISpin<int>(PAGE_WIDTH - 110, y, 110, GetOptionsDB().Get<int>("UI.chat-hide-interval"), 10, 0, 3600, true);
	GG::Connect(spinControlInt->ValueChangedSignal, SetOptionFunctor<int>("UI.chat-hide-interval"));
	ui_page->AttachChild(spinControlInt);

	// Fill up the Sound Options (1) tab page:
	/*******************************************
	Music + volume
	Effects + volume
	BKG Music Track [music]
	Window 
		Close [sound] 
		Maximize [sound] 
		Minimize [sound]
	Button
		Click [sound] 
		Rollover [sound]
	Fleet button
		Click [sound] 
		Rollover [sound]
	*******************************************/
	//	The Music check box and slider
	y = PAGE_VERT_MARGIN;
	x = PAGE_HORZ_MARGIN;
    stateBtn = new CUIStateButton(x, y, 75, 20, UserString("OPTIONS_MUSIC"), GG::TF_LEFT);
	stateBtn->SetCheck(!GetOptionsDB().Get<bool>("music-off"));
    GG::Connect(stateBtn->CheckedSignal, &OptionsWnd::MusicClicked, this);
	sound_page->AttachChild(stateBtn);
    sliderControl = new CUISlider(PAGE_WIDTH - 150, y, 150, 14, 0, 255, CUISlider::HORIZONTAL);
    sliderControl->SlideTo(GetOptionsDB().Get<int>("music-volume"));
    GG::Connect(sliderControl->SlidSignal, &OptionsWnd::MusicVolumeSlid, this);
	sound_page->AttachChild(sliderControl);
	//	The Sound Effects check box and slider
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN;
    stateBtn = new CUIStateButton(x, y, 75, 20, UserString("OPTIONS_UI_SOUNDS"), GG::TF_LEFT);
	stateBtn->SetCheck(GetOptionsDB().Get<bool>("UI.sound.enabled"));
    GG::Connect(stateBtn->CheckedSignal, &OptionsWnd::UIEffectsClicked, this);
	sound_page->AttachChild(stateBtn);
    sliderControl = new CUISlider(PAGE_WIDTH - 150, 40, 150, 14, 0, 255, CUISlider::HORIZONTAL);
    sliderControl->SlideTo(GetOptionsDB().Get<int>("UI.sound.volume"));
    GG::Connect(sliderControl->SlidAndStoppedSignal, &OptionsWnd::UISoundsVolumeSlid, this);
	sound_page->AttachChild(sliderControl);
	// BKG Music Track [music]
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN;
	AddSoundControls(x, y, "OPTIONS_BACKGROUND_MUSIC", sound_page, "bg-music", m_music_edit, btn, false);
	GG::Connect(btn->ClickedSignal, BrowseForFileFunctor("bg-music", "sound-dir", "OPTIONS_MUSIC_FILE", ".ogg", m_music_edit, this));
	// Window
	y += PAGE_ROW_HEIGHT;
	x = PAGE_HORZ_MARGIN + PAGE_HORZ_OFFSET;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_SOUND_WINDOW"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	sound_page->AttachChild(textControl);
	// Close [sound] 
	y += PAGE_ROW_HEIGHT - 5;
	AddSoundControls(x, y, "OPTIONS_SOUND_CLOSE", sound_page, "UI.sound.window-close", m_close_edit, btn);
	// Maximize [sound] 
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_MAXIMIZE", sound_page, "UI.sound.window-maximize", m_maximize_edit, btn);
	// Minimize [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_MINIMIZE", sound_page, "UI.sound.window-minimize", m_minimize_edit, btn);
	// Button
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_SOUND_BUTTON"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	sound_page->AttachChild(textControl);
	// Click [sound] 
	y += PAGE_ROW_HEIGHT - 5;
	AddSoundControls(x, y, "OPTIONS_SOUND_CLICK", sound_page, "UI.sound.button-click", m_clickButton_edit, btn);
	// Rollover [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_ROLLOVER", sound_page, "UI.sound.button-rollover", m_rolloverButton_edit, btn);
	// Fleet
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_SOUND_FLEET"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	sound_page->AttachChild(textControl);
	// Click [sound] 
	y += PAGE_ROW_HEIGHT - 5;
	AddSoundControls(x, y, "OPTIONS_SOUND_CLICK", sound_page, "UI.sound.fleet-button-click", m_clickFleet_edit, btn);
	// Rollover [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_ROLLOVER", sound_page, "UI.sound.fleet-button-rollover", m_rolloverFleet_edit, btn);

	// Fill up the Sound Options (2) tab page:
	/*******************************************
	Alert [sound]
	Text Typing [sound]
	Turn Button [sound]
	SidePanel Open [sound]
	Planet Button [sound]
	Focus
		Balanced [sound]
		Farming [sound]
		Industry [sound]
		Mining [sound]
		Research [sound]
	List
		Drop [sound] 
		Pulldown [sound] 
		Select [sound]
	*******************************************/
	y = PAGE_VERT_MARGIN;
	x = PAGE_HORZ_MARGIN + PAGE_HORZ_OFFSET;
	// Alert [sound]
	AddSoundControls(PAGE_HORZ_MARGIN, y, "OPTIONS_SOUND_ALERT", sound_page2, "UI.sound.alert", m_alert_edit, btn);
	// Text typing [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(PAGE_HORZ_MARGIN, y, "OPTIONS_SOUND_TYPING", sound_page2, "UI.sound.text-typing", m_typing_edit, btn);
	// Turn Button [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(PAGE_HORZ_MARGIN, y, "OPTIONS_SOUND_TURN", sound_page2, "UI.sound.turn-button-click", m_turn_edit, btn);
	// SidePanel Open [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(PAGE_HORZ_MARGIN, y, "OPTIONS_SOUND_SIDEPANEL", sound_page2, "UI.sound.sidepanel-open", m_sidePanel_edit, btn);
	// Planet Button [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(PAGE_HORZ_MARGIN, y, "OPTIONS_SOUND_PLANET", sound_page2, "UI.sound.planet-button-click", m_planet_edit, btn);
	// Focus
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_SOUND_FOCUS"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	sound_page2->AttachChild(textControl);
	// Balanced [sound]
	y += PAGE_ROW_HEIGHT - 5;
	AddSoundControls(x, y, "OPTIONS_SOUND_BALANCED", sound_page2, "UI.sound.balanced-focus", m_balanced_edit, btn);
	// Farming [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_FARMING", sound_page2, "UI.sound.farming-focus", m_farming_edit, btn);
	// Industry [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_INDUSTRY", sound_page2, "UI.sound.industry-focus", m_industry_edit, btn);
	// Mining [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_MINING", sound_page2, "UI.sound.mining-focus", m_mining_edit, btn);
	// Research [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_RESEARCH", sound_page2, "UI.sound.research-focus", m_research_edit, btn);
	// List
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_SOUND_LIST"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	sound_page2->AttachChild(textControl);
	// Drop [sound] 
	y += PAGE_ROW_HEIGHT - 5;
	AddSoundControls(x, y, "OPTIONS_SOUND_DROP", sound_page2, "UI.sound.item-drop", m_drop_edit, btn);
	// Pulldown [sound] 
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_PULLDOWN", sound_page2, "UI.sound.list-pulldown", m_pulldown_edit, btn);
	// Select [sound]
	y += PAGE_ROW_HEIGHT;
	AddSoundControls(x, y, "OPTIONS_SOUND_SELECT", sound_page2, "UI.sound.list-select", m_select_edit, btn);

	// Fill up the Controls Color Options tab page:
	/*******************************************
	General
		Color [color] 
		Border [color]
		Text [color]
	Edit
		Highlight [color] 
		Interior [color]
	Window 
		Color [color] 
		Border [color]
		Inner Border [color] 
		Outer border [color]
	*******************************************/
	y = PAGE_VERT_MARGIN;
	x = PAGE_HORZ_MARGIN + PAGE_HORZ_OFFSET;
	// General
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_COLOR_GENERAL"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	colors_page->AttachChild(textControl);
	// Color [color]
	y += PAGE_ROW_HEIGHT - 5;
	AddColorControls(x, y, "OPTIONS_COLOR_COLOR", colors_page, "UI.ctrl-color", comboColor);
	// Border [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_BORDER", colors_page, "UI.ctrl-border-color", comboColor);
	// Text [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_TEXT", colors_page, "UI.text-color", comboColor);
	// Edit
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_COLOR_EDIT"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	colors_page->AttachChild(textControl);
	// Highlight [color] 
	y += PAGE_ROW_HEIGHT - 5;
	AddColorControls(x, y, "OPTIONS_COLOR_HIGHLIGHT", colors_page, "UI.edit-hilite", comboColor);
	// Interior [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_INTERIOR", colors_page, "UI.edit-interior", comboColor);
	// Window
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_COLOR_WINDOW"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	colors_page->AttachChild(textControl);
	// Color [color]
	y += PAGE_ROW_HEIGHT - 5;
	AddColorControls(x, y, "OPTIONS_COLOR_COLOR", colors_page, "UI.wnd-color", comboColor);
	// Border [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_BORDER", colors_page, "UI.wnd-border-color", comboColor);
	// Inner Border [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_INNER_BORDER", colors_page, "UI.wnd-inner-border-color", comboColor);
	// Outer Border [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_OUTER_BORDER", colors_page, "UI.wnd-outer-border-color", comboColor);

	// Fill up the Game Color Options tab page:
	/*******************************************
	Known Tech 
		Color [color] 
		border [color]
	Researchable Tech 
		Color [color] 
		border [color]
	Unresearchable Tech 
		Color [color] 
		border [color]
	Tech Progress 
		Color [color] 
		background [color]
	*******************************************/
	y = PAGE_VERT_MARGIN;
	x = PAGE_HORZ_MARGIN + PAGE_HORZ_OFFSET;
	// Known Tech
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_COLOR_TECH_KNOWN"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	colors_page2->AttachChild(textControl);
	// Color [color]
	y += PAGE_ROW_HEIGHT - 5;
	AddColorControls(x, y, "OPTIONS_COLOR_COLOR", colors_page2, "UI.known-tech", comboColor);
	// Border [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_BORDER", colors_page2, "UI.known-tech-border", comboColor);
	// Researchable Tech
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_COLOR_TECH_RESEARCHABLE"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	colors_page2->AttachChild(textControl);
	// Color [color]
	y += PAGE_ROW_HEIGHT - 5;
	AddColorControls(x, y, "OPTIONS_COLOR_COLOR", colors_page2, "UI.researchable-tech", comboColor);
	// Border [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_BORDER", colors_page2, "UI.researchable-tech-border", comboColor);
	// Unresearchable Tech
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_COLOR_TECH_UNRESEARCHABLE"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	colors_page2->AttachChild(textControl);
	// Color [color]
	y += PAGE_ROW_HEIGHT - 5;
	AddColorControls(x, y, "OPTIONS_COLOR_COLOR", colors_page2, "UI.unresearchable-tech", comboColor);
	// Border [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_BORDER", colors_page2, "UI.unresearchable-tech-border", comboColor);
	// Tech Progress
	y += PAGE_ROW_HEIGHT;
	textControl = new GG::TextControl(PAGE_HORZ_MARGIN, y, UserString("OPTIONS_COLOR_TECH_PROGRESS"), ClientUI::FONT, ClientUI::SIDE_PANEL_PLANET_NAME_PTS, ClientUI::TEXT_COLOR);
	colors_page2->AttachChild(textControl);
	// Color [color]
	y += PAGE_ROW_HEIGHT - 5;
	AddColorControls(x, y, "OPTIONS_COLOR_PROGRESS", colors_page2, "UI.tech-progress", comboColor);
	// Background [color]
	y += PAGE_ROW_HEIGHT;
	AddColorControls(x, y, "OPTIONS_COLOR_PROGRESS_BACKGROUND", colors_page2, "UI.tech-progress-background", comboColor);

	// Fill up the Folders Options tab page:
	/*******************************************
	Settings Directory [folder]
	Art Directory [folder]
	Save Directory [folder]
	Sounds Directory [folder]
	*******************************************/
	y = PAGE_VERT_MARGIN;
	x = PAGE_HORZ_MARGIN;
	// Settings Directory [folder]
	AddFolderControls(x, y, "OPTIONS_FOLDER_SETTINGS", folders_page, "settings-dir", m_settings_dir_edit);
	GG::Connect(m_settings_dir_edit->FocusUpdateSignal, &OptionsWnd::SettingsDirFocusUpdate, this);
	// Art Directory [folder]
	y += PAGE_ROW_HEIGHT;
	AddFolderControls(x, y, "OPTIONS_FOLDER_ART", folders_page, "art-dir", m_art_dir_edit);
	GG::Connect(m_art_dir_edit->FocusUpdateSignal, &OptionsWnd::ArtDirFocusUpdate, this);
	// Save Directory [folder]
	y += PAGE_ROW_HEIGHT;
	AddFolderControls(x, y, "OPTIONS_FOLDER_SAVE", folders_page, "save-dir", m_save_dir_edit);
	GG::Connect(m_save_dir_edit->FocusUpdateSignal, &OptionsWnd::SaveDirFocusUpdate, this);
	// Sounds Directory [folder]
	y += PAGE_ROW_HEIGHT;
	AddFolderControls(x, y, "OPTIONS_FOLDER_SOUNDS", folders_page, "sound-dir", m_sound_dir_edit);
	GG::Connect(m_sound_dir_edit->FocusUpdateSignal, &OptionsWnd::SoundDirFocusUpdate, this);
	y += PAGE_ROW_HEIGHT;

	// Add the pages to the tabbed pages control
	m_tabs->AddPage(control_page, UserString("OPTIONS_PAGE_CONTROL"));
	m_tabs->AddPage(ui_page, UserString("OPTIONS_PAGE_UI"));
	m_tabs->AddPage(sound_page, UserString("OPTIONS_PAGE_SOUND"));
	m_tabs->AddPage(sound_page2, UserString("OPTIONS_PAGE_SOUND_2"));
	m_tabs->AddPage(colors_page, UserString("OPTIONS_PAGE_COLOR_CONTROLS"));
	m_tabs->AddPage(colors_page2, UserString("OPTIONS_PAGE_COLOR_GAME"));
	m_tabs->AddPage(folders_page, UserString("OPTIONS_PAGE_FOLDERS"));

	// Connect the done and cancel button
    GG::Connect(m_done_btn->ClickedSignal, &OptionsWnd::DoneClicked, this);
}

OptionsWnd::~OptionsWnd()
{
}

void OptionsWnd::Keypress (GG::Key key, Uint32 key_mods)
{
	if (key == GG::GGK_ESCAPE || key == GG::GGK_RETURN || key == GG::GGK_KP_ENTER) // Same behaviour as if "done" was pressed
		DoneClicked();
}

void OptionsWnd::DoneClicked()
{
	// Save the changes:
    std::ofstream ofs("default/config.xml");
    GetOptionsDB().GetXML().WriteDoc(ofs);
    m_done = true;
}

void OptionsWnd::MusicClicked(bool checked)
{
	if (!checked)
	{
		GetOptionsDB().Set("music-off", true);
		HumanClientApp::GetApp()->StopMusic();
	}
    else
	{
		GetOptionsDB().Set("music-off", false);
        HumanClientApp::GetApp()->PlayMusic(ClientUI::SOUND_DIR + GetOptionsDB().Get<std::string>("bg-music"), -1);
	}
}

void OptionsWnd::UIEffectsClicked(bool checked)
{
    GetOptionsDB().Set("UI.sound.enabled", checked);
}

void OptionsWnd::MusicVolumeSlid(int pos, int low, int high)
{
    GetOptionsDB().Set("music-volume", pos);
    HumanClientApp::GetApp()->SetMusicVolume(pos);
}

void OptionsWnd::UISoundsVolumeSlid(int pos, int low, int high)
{
    GetOptionsDB().Set("UI.sound.volume", pos);
    HumanClientApp::GetApp()->SetUISoundsVolume(pos);
    HumanClientApp::GetApp()->PlaySound(ClientUI::SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.button-click"));
}

void OptionsWnd::TextFont(int selection)
{
	GetOptionsDB().Set<std::string>("UI.font", m_fonts[selection] + ".ttf");
}

void OptionsWnd::TitleFont(int selection)
{
	GetOptionsDB().Set<std::string>("UI.title-font", m_fonts[selection] + ".ttf");
}

void OptionsWnd::Resolution(int selection)
{
	int width, height;
	switch (selection)
	{
	case 0:
		width = 640;
		height = 480;
		break;
	case 1:
		width = 800;
		height = 600;
		break;
	case 2:
		width = 1024;
		height = 768;
		break;
	case 3:
		width = 1280;
		height = 1024;
		break;
	case 4:
		width = 1600;
		height = 1200;
		break;
	default:
		return;
	}
	GetOptionsDB().Set<int>("app-width", width);
	GetOptionsDB().Set<int>("app-height", height);
}

void OptionsWnd::ColorDepth(int selection)
{
	if ((selection < 0) || (selection >= (int)m_colorDepth.size()))
		return;
	int depth = atoi(m_colorDepth[selection].c_str());
	GetOptionsDB().Set<int>("color-depth", depth);
}

void OptionsWnd::UpdateFileOption(const std::string& option_name, std::string file_name, bool folder /* = false */)
{
	if (NativePathToOptionPath(file_name, folder))
		GetOptionsDB().Set<std::string>(option_name, file_name);
}

bool OptionsWnd::TestFolder(std::string& value)
{
	if (value.empty()) {
		value = CURRENT_DIR;
		return true;
	}

	boost::filesystem::path value_path, dir;
	try {
		value_path = boost::filesystem::path(value, boost::filesystem::native);
	} catch (const boost::filesystem::filesystem_error& e) {
		GG::ThreeButtonDlg dlg(300, 125, "\"" + value + "\"\nis an invalid file name.", ClientUI::FONT, ClientUI::PTS + 2, ClientUI::WND_COLOR, ClientUI::WND_BORDER_COLOR, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR, 1);
		dlg.Run();
		return false;
	}

	dir = boost::filesystem::system_complete(value_path);
	if (!boost::filesystem::exists(dir)) {
		GG::ThreeButtonDlg dlg(300, 125, "\"" + value + "\"\ndoes not exist.", ClientUI::FONT, ClientUI::PTS + 2, ClientUI::WND_COLOR, ClientUI::WND_BORDER_COLOR, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR, 1);
		dlg.Run();
		return false;
	}
	if (!boost::filesystem::is_directory(dir)) {
		GG::ThreeButtonDlg dlg(300, 125, "\"" + value + "\"\nisn't a directory.", ClientUI::FONT, ClientUI::PTS + 2, ClientUI::WND_COLOR, ClientUI::WND_BORDER_COLOR, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR, 1);
		dlg.Run();
		return false;
	}
	value = value_path.native_file_string() + SLASH;

	return true;
}

void OptionsWnd::SettingsDirFocusUpdate(const std::string& value)
{
	// Make sure this is a valid folder:
	std::string folder(value);
	if (!TestFolder(folder))
		return;
	UpdateFileOption("settings-dir", folder, true);
	if (folder != value)
		m_settings_dir_edit->SetText(folder);
}

void OptionsWnd::ArtDirFocusUpdate(const std::string& value)
{
	// Make sure this is a valid folder:
	std::string folder(value);
	if (!TestFolder(folder))
		return;
	UpdateFileOption("art-dir", folder, true);
	if (folder != value)
		m_art_dir_edit->SetText(folder);
}

void OptionsWnd::SaveDirFocusUpdate(const std::string& value)
{
	// Make sure this is a valid folder:
	std::string folder(value);
	if (!TestFolder(folder))
		return;
	UpdateFileOption("save-dir", folder, true);
	if (folder != value)
		m_save_dir_edit->SetText(folder);
}

void OptionsWnd::SoundDirFocusUpdate(const std::string& value)
{
	// Make sure this is a valid folder:
	std::string folder(value);
	if (!TestFolder(folder))
		return;
	UpdateFileOption("sound-dir", folder, true);
	if (folder != value)
		m_sound_dir_edit->SetText(folder);
}

void OptionsWnd::Browse(const std::string& optionName, const std::string& optionDir, const std::string& userString, const std::string& extension, CUIEdit* editControl)
{
	std::vector<std::pair<std::string, std::string> > file_types;
    file_types.push_back(std::pair<std::string, std::string>(UserString(userString), "*" + extension));
    file_types.push_back(std::pair<std::string, std::string>(UserString("OPTIONS_ANY_FILE"), "*.*"));

	std::string filename = BrowseForFile(GetOptionsDB().Get<std::string>(optionDir), GetOptionsDB().Get<std::string>(optionName), file_types);
	if (filename.empty())
		return;

	std::string option_filename = filename;
	if (!NativePathToOptionPath(option_filename, false)) {
		GG::ThreeButtonDlg dlg(300, 125, "\"" + filename + "\"\nIs invalid.", ClientUI::FONT, ClientUI::PTS + 2, ClientUI::WND_COLOR, ClientUI::WND_BORDER_COLOR, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR, 1);
		dlg.Run();
	} else {
		GetOptionsDB().Set<std::string>(optionName, option_filename);
		editControl->SetText(filename);
	}
}

std::string OptionsWnd::BrowseForFile(const std::string& directory, const std::string& aFile, const std::vector<std::pair<std::string, std::string> >& file_types)
{
    try {
		boost::filesystem::path settings = boost::filesystem::system_complete(boost::filesystem::path(directory));
		boost::filesystem::path file = settings / boost::filesystem::path(aFile, boost::filesystem::native);
		file.normalize();
		FileDlg dlg(file.branch_path().string(), file.leaf(), false, false, file_types);
        dlg.Run();
        if (dlg.Result().empty())
			return "";

		std::string filename = *dlg.Result().begin();
		if (!filename.empty()) {
			// Make it into a path relative to the settings-dir
			boost::filesystem::path newFile(filename, boost::filesystem::native);
			boost::filesystem::path::iterator iSettings, iNewFile = newFile.begin();
			boost::filesystem::path relative;
			for (iSettings = settings.begin(); (iSettings != settings.end()) && (iNewFile != newFile.end()); iSettings++, iNewFile++)
			{
				if ((*iSettings) != (*iNewFile))
					break;
			}
			if (iNewFile == newFile.begin())
				relative = newFile;
			else {
				for (; iSettings != settings.end(); iSettings++)
					relative /= "..";
				for (; iNewFile != newFile.end(); iNewFile++)
					relative /= (*iNewFile);
			}
			return relative.native_directory_string();
		}
    } catch (const FileDlg::InitialDirectoryDoesNotExistException& e) {
        ClientUI::MessageBox(e.Message(), true);
    }
	return "";
}

void OptionsWnd::FillCombo(CUIDropDownList* combo, const std::vector<std::string>& values, const std::string& currentValue)
{
	combo->Clear();
	bool selected = false;
	for (unsigned int i = 0; i < values.size(); i++)
	{
		int index = combo->Insert(new CUISimpleDropDownListRow(values[i]), i);
		if (values[i] == currentValue)
		{
			combo->Select(index);
			selected = true;
		}
	}
	if (!selected && !values.empty())
		combo->Select(0);
}

void OptionsWnd::FillLists()
{
	// Fill up the fonts list
	FillFontList();

	// Fill up the resolution list
	m_resolutions.clear();
	m_resolutions.push_back("1024 x 768");
	m_resolutions.push_back("1280 x 1024");
	m_resolutions.push_back("1600 x 1200");

	// Fill up the color-depth list
	m_colorDepth.clear();
	m_colorDepth.push_back("16");
	m_colorDepth.push_back("32");
}

void OptionsWnd::FillFontList()
{
	m_fonts.clear();
	try {
		boost::filesystem::path font_folder = boost::filesystem::initial_path();
		boost::filesystem::directory_iterator end_it;
		for (boost::filesystem::directory_iterator it(font_folder); it != end_it; it++)
		{
			if (boost::filesystem::is_directory(*it))
				continue;
			if (boost::filesystem::extension(*it) != ".ttf")
				continue;
			std::string s(it->leaf());
			s.erase(s.size() - 4);
			m_fonts.push_back(s);
		}
	} catch (boost::filesystem::filesystem_error& e) {
		m_fonts.clear();
	}
}
