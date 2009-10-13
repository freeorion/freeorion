#include "ChatWnd.h"

#include "CUIControls.h"
#include "../client/human/HumanClientApp.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../network/Message.h"
#include "../universe/Universe.h"
#include "../universe/System.h"
#include "../util/OptionsDB.h"
#include "../util/MultiplayerCommon.h"

#include <GG/GUI.h>


namespace {
    const int LAYOUT_MARGIN = 5;
    const GG::X CHAT_WIDTH(400);
    const GG::Y CHAT_HEIGHT(400);
    const GG::Y CHAT_EDIT_HEIGHT(30);

    void AddOptions(OptionsDB& db)
    {
        db.Add("UI.chat-hide-interval", "OPTIONS_DB_UI_CHAT_HIDE_INTERVAL",
               10, RangedValidator<int>(0, 3600));
        db.Add("UI.chat-edit-history",  "OPTIONS_DB_UI_CHAT_EDIT_HISTORY",
               50, RangedValidator<int>(0, 1000));
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

ChatWnd::ChatWnd() :
    Wnd(GG::X0, GG::Y0, GG::GUI::GetGUI()->AppWidth(), GG::GUI::GetGUI()->AppHeight(), GG::ONTOP),
    m_display(0),
    m_edit(new CUIEdit(GG::X(LAYOUT_MARGIN), GG::GUI::GetGUI()->AppHeight() - CHAT_EDIT_HEIGHT - LAYOUT_MARGIN, CHAT_WIDTH, "",
                       ClientUI::GetFont(), ClientUI::CtrlBorderColor(), ClientUI::TextColor(), ClientUI::CtrlColor())),
    m_display_show_time(0),
    m_history(),
    m_history_position()
{
    std::auto_ptr<CUITurnButton> turn_button(
        new CUITurnButton(GG::X0, GG::Y(LAYOUT_MARGIN), GG::X(100), ""));
    m_display = new GG::MultiEdit(
        GG::X(LAYOUT_MARGIN), turn_button->Height() + LAYOUT_MARGIN, CHAT_WIDTH, CHAT_HEIGHT, "", ClientUI::GetFont(), GG::CLR_ZERO,
        GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY | GG::MULTI_TERMINAL_STYLE | GG::MULTI_INTEGRAL_HEIGHT | GG::MULTI_NO_VSCROLL,
        ClientUI::TextColor(), GG::CLR_ZERO, GG::Flags<GG::WndFlag>());
    AttachChild(m_display);
    m_display->SetMaxLinesOfHistory(100);
    m_display->Hide();

    AttachChild(m_edit);
    m_edit->Hide();

    m_history.push_front("");
}

void ChatWnd::HandlePlayerChatMessage(const std::string& msg)
{
    *m_display += msg;
    m_display->Show();
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void ChatWnd::Clear()
{ m_display->Clear(); }

bool ChatWnd::OpenForInput()
{
    bool retval = false;
    if (!m_display->Visible() || !m_edit->Visible()) {
        m_display->Show();
        m_edit->Show();
        GG::GUI::GetGUI()->SetFocusWnd(m_edit);
        m_display_show_time = GG::GUI::GetGUI()->Ticks();
        retval = true;
    }
    return retval;
}

void ChatWnd::HideEdit()
{ m_edit->Hide(); }

void ChatWnd::Render()
{
    unsigned int interval = GetOptionsDB().Get<int>("UI.chat-hide-interval");
    if (!m_edit->Visible() && m_display_show_time && interval && 
        (interval < (GG::GUI::GetGUI()->Ticks() - m_display_show_time) / 1000)) {
        m_display->Hide();
        m_display_show_time = 0;
    }
}

void ChatWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    switch (key) {
    case GG::GGK_TAB: { // auto-complete current chat edit word
        if (m_edit->Visible()) {
            std::string text = m_edit->Text();
            std::pair<GG::CPSize, GG::CPSize> cursor_pos = m_edit->CursorPosn();
            if (cursor_pos.first == cursor_pos.second && 0 < cursor_pos.first && cursor_pos.first <= text.size()) {
                std::string::size_type word_start = text.substr(0, Value(cursor_pos.first)).find_last_of(" :");
                if (word_start == std::string::npos)
                    word_start = 0;
                else
                    ++word_start;
                std::string partial_word = text.substr(word_start, Value(cursor_pos.first - word_start));
                if (partial_word == "")
                    return;
                std::set<std::string> names;
                // add player and empire names
                for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
                    names.insert(it->second->Name());
                    names.insert(it->second->PlayerName());
                }
                // add system names
                std::vector<System*> systems = GetUniverse().FindObjects<System>();
                for (unsigned int i = 0; i < systems.size(); ++i) {
                    if (systems[i]->Name() != "")
                        names.insert(systems[i]->Name());
                }

                if (names.find(partial_word) != names.end()) { // if there's an exact match, just add a space
                    text.insert(Value(cursor_pos.first), " ");
                    m_edit->SetText(text);
                    m_edit->SelectRange(cursor_pos.first + 1, cursor_pos.first + 1);
                } else { // no exact match; look for possible completions
                    // find the range of strings in names that is at least partially matched by partial_word
                    std::set<std::string>::iterator lower_bound = names.lower_bound(partial_word);
                    std::set<std::string>::iterator upper_bound = lower_bound;
                    while (upper_bound != names.end() && upper_bound->find(partial_word) == 0)
                        ++upper_bound;
                    if (lower_bound == upper_bound)
                        return;

                    // find the common portion of the strings in (upper_bound, lower_bound)
                    unsigned int common_end = partial_word.size();
                    for ( ; common_end < lower_bound->size(); ++common_end) {
                        std::set<std::string>::iterator it = lower_bound;
                        char ch = (*it++)[common_end];
                        bool match = true;
                        for ( ; it != upper_bound; ++it) {
                            if ((*it)[common_end] != ch) {
                                match = false;
                                break;
                            }
                        }
                        if (!match)
                            break;
                    }
                    unsigned int chars_to_add = common_end - partial_word.size();
                    bool full_completion = common_end == lower_bound->size();
                    text.insert(Value(cursor_pos.first), lower_bound->substr(partial_word.size(), chars_to_add) + (full_completion ? " " : ""));
                    m_edit->SetText(text);
                    GG::CPSize move_cursor_to = cursor_pos.first + chars_to_add + (full_completion ? GG::CP1 : GG::CP0);
                    m_edit->SelectRange(move_cursor_to, move_cursor_to);
                }
            }
        }
        break;
    }
    case GG::GGK_RETURN:
    case GG::GGK_KP_ENTER: { // send chat message
        if (m_edit->Visible()) {
            std::string edit_text = m_edit->Text();
            if (edit_text != "") {
                if (m_history.size() == 1 || m_history[1] != edit_text) {
                    m_history[0] = edit_text;
                    m_history.push_front("");
                } else {
                    m_history[0] = "";
                }
                while (GetOptionsDB().Get<int>("UI.chat-edit-history") < static_cast<int>(m_history.size()) + 1)
                    m_history.pop_back();
                m_history_position = 0;
                HumanClientApp::GetApp()->Networking().SendMessage(GlobalChatMessage(HumanClientApp::GetApp()->PlayerID(), edit_text));
            }
            m_edit->Clear();
            m_edit->Hide();
            m_display_show_time = GG::GUI::GetGUI()->Ticks();
            MessageSentSignal();
        }
        break;
    }

    case GG::GGK_UP: {
        if (m_edit->Visible() && m_history_position < static_cast<int>(m_history.size()) - 1) {
            m_history[m_history_position] = m_edit->Text();
            ++m_history_position;
            m_edit->SetText(m_history[m_history_position]);
        }
        break;
    }

    case GG::GGK_DOWN: {
        if (m_edit->Visible() && 0 < m_history_position) {
            m_history[m_history_position] = m_edit->Text();
            --m_history_position;
            m_edit->SetText(m_history[m_history_position]);
        }
        break;
    }

    default:
        break;
    }
}

ChatWnd* GetChatWnd()
{
    static bool init = false;
    static std::auto_ptr<ChatWnd> retval(new ChatWnd);
    if (!init) {
        GG::GUI::GetGUI()->Register(retval.get());
        init = true;
    }
    return retval.get();
}
