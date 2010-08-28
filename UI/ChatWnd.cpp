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
    const int PAD = 3;

    void AddOptions(OptionsDB& db)
    {}
    bool temp_bool = RegisterOptions(&AddOptions);
}

class MessageWndEdit : public CUIEdit
{
public:
    //! \name Structors //@{
    MessageWndEdit(GG::X x, GG::Y y, GG::X w);
    //@}

    //! \name Mutators //@{
    virtual void    GainingFocus();
    virtual void    LosingFocus();
    virtual void    KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);
    //@}

    mutable boost::signal<void ()>  TextEnteredSignal;  //!< emitted when user presses enter/return while entering text
    mutable boost::signal<void ()>  UpPressedSignal;
    mutable boost::signal<void ()>  DownPressedSignal;
    mutable boost::signal<void ()>  GainingFocusSignal;
    mutable boost::signal<void ()>  LosingFocusSignal;

private:
    void            AutoComplete();                     //!< Autocomplete current word
};

////////////////////
// MessageWndEdit //
////////////////////
MessageWndEdit::MessageWndEdit(GG::X x, GG::Y y, GG::X w) :
    CUIEdit(x, y, w, "")
{}

void MessageWndEdit::GainingFocus()
{
    GG::Edit::GainingFocus();
    GainingFocusSignal();
}

void MessageWndEdit::LosingFocus()
{
    GG::Edit::LosingFocus();
    LosingFocusSignal();
}

void MessageWndEdit::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    switch (key) {
    case GG::GGK_TAB:
        AutoComplete();
        break;
    case GG::GGK_RETURN:
    case GG::GGK_KP_ENTER:
        TextEnteredSignal();
        break;
    case GG::GGK_UP:
        UpPressedSignal();
        break;
    case GG::GGK_DOWN:
        DownPressedSignal();
        break;
    default:
        break;
    }
    Edit::KeyPress(key, key_code_point, mod_keys);
}

void MessageWndEdit::AutoComplete()
{
    std::string text = this->Text();
    std::pair<GG::CPSize, GG::CPSize> cursor_pos = this->CursorPosn();
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
        std::vector<System*> systems = GetUniverse().Objects().FindObjects<System>();
        for (unsigned int i = 0; i < systems.size(); ++i) {
            if (systems[i]->Name() != "")
                names.insert(systems[i]->Name());
        }

        if (names.find(partial_word) != names.end()) { // if there's an exact match, just add a space
            text.insert(Value(cursor_pos.first), " ");
            this->SetText(text);
            this->SelectRange(cursor_pos.first + 1, cursor_pos.first + 1);
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
            this->SetText(text);
            GG::CPSize move_cursor_to = cursor_pos.first + chars_to_add + (full_completion ? GG::CP1 : GG::CP0);
            this->SelectRange(move_cursor_to, move_cursor_to);
        }
    }
}


////////////////////
//   MessageWnd   //
////////////////////
MessageWnd::MessageWnd(GG::X x, GG::Y y, GG::X w, GG::Y h) :
    CUIWnd(UserString("MESSAGES_PANEL_TITLE"), x, y, w, h, GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | GG::RESIZABLE | CLOSABLE),
    m_display(0),
    m_edit(0),
    m_display_show_time(0),
    m_history(),
    m_history_position()
{
    m_display = new CUIMultiEdit(
        GG::X0, GG::Y0, ClientWidth(), ClientHeight(), "",
        GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY | GG::MULTI_TERMINAL_STYLE | GG::MULTI_INTEGRAL_HEIGHT);
    AttachChild(m_display);
    m_display->SetMaxLinesOfHistory(100);

    m_edit = new MessageWndEdit(GG::X0, GG::Y0, ClientWidth());
    AttachChild(m_edit);

    GG::Connect(m_edit->TextEnteredSignal,  &MessageWnd::MessageEntered,                this);
    GG::Connect(m_edit->UpPressedSignal,    &MessageWnd::MessageHistoryUpRequested,     this);
    GG::Connect(m_edit->DownPressedSignal,  &MessageWnd::MessageHistoryDownRequested,   this);
    GG::Connect(m_edit->GainingFocusSignal, TypingSignal);
    GG::Connect(m_edit->LosingFocusSignal,  DoneTypingSignal);

    m_history.push_front("");

    DoLayout();
}

void MessageWnd::DoLayout()
{
    m_display->SizeMove(GG::Pt(GG::X0, GG::Y0),
                        GG::Pt(ClientWidth(), ClientHeight() - GG::Y(PAD) - m_edit->Height()));
    m_edit->SizeMove(GG::Pt(GG::X0, ClientHeight() - m_edit->Height()),
                     GG::Pt(ClientWidth() - GG::X(CUIWnd::INNER_BORDER_ANGLE_OFFSET), ClientHeight()));
}

void MessageWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    const GG::Pt old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void MessageWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey>& mod_keys)
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt final_move(std::max(-ul.x, std::min(move.x, GG::GUI::GetGUI()->AppWidth() - 1 - lr.x)),
                      std::max(-ul.y, std::min(move.y, GG::GUI::GetGUI()->AppHeight() - 1 - lr.y)));
    GG::Wnd::LDrag(pt + final_move - move, final_move, mod_keys);
}

void MessageWnd::HandlePlayerChatMessage(const std::string& text, int sender_player_id, int recipient_player_id)
{
    const ClientApp* app = ClientApp::GetApp();
    if (!app) {
        Logger().errorStream() << "MessageWnd::HandlePlayerChatMessage couldn't get client app!";
        return;
    }

    const std::map<int, PlayerInfo>& players = app->Players();
    std::map<int, PlayerInfo>::const_iterator player_it = players.find(sender_player_id);
    if (player_it == players.end()) {
        Logger().errorStream() << "MessageWnd::HandlePlayerChatMessage couldn't message sending player with id: " << sender_player_id;
        return;
    }

    std::string sender_name = player_it->second.name;

    const Empire* sender_empire = app->GetPlayerEmpire(sender_player_id);
    if (player_it == players.end()) {
        Logger().errorStream() << "MessageWnd::HandlePlayerChatMessage couldn't message sending player's empire";
        return;
    }

    GG::Clr sender_colour = sender_empire->Color();

    std::string wrapped_text = RgbaTag(sender_colour) + sender_name + ": " + text + "</rgba>";

    *m_display += wrapped_text + "\n";
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id, int empire_id)
{
    std::string phase_str;
    switch (phase_id) {
    case Message::FLEET_MOVEMENT:
        phase_str = UserString("TURN_PROGRESS_PHASE_FLEET_MOVEMENT");
        break;
    case Message::COMBAT:
        phase_str = UserString("TURN_PROGRESS_PHASE_COMBAT");
        break;
    case Message::EMPIRE_PRODUCTION:
        phase_str = UserString("TURN_PROGRESS_PHASE_EMPIRE_GROWTH");
        break;
    case Message::WAITING_FOR_PLAYERS:
        phase_str = UserString("TURN_PROGRESS_PHASE_WAITING");
        break;
    case Message::PROCESSING_ORDERS:
        {
            const Empire* empire = Empires().Lookup(empire_id);
            if (!empire) {
                Logger().errorStream() << "MessageWnd::HandleTurnPhaseUpdate couldn't get empire with id " << empire_id;
                return;
            }
            const std::string& empire_name = empire->Name();
            const GG::Clr empire_clr = empire->Color();

            std::string wrapped_empire_name = RgbaTag(empire_clr) + empire_name + "</rgba>";

            phase_str = boost::io::str(FlexibleFormat(UserString("TURN_PROGRESS_PHASE_ORDERS")) % wrapped_empire_name);
        }
        break;
    case Message::COLONIZE_AND_SCRAP:
        phase_str = UserString("TURN_PROGRESS_COLONIZE_AND_SCRAP");
        break;
    case Message::DOWNLOADING:
        phase_str = UserString("TURN_PROGRESS_PHASE_DOWNLOADING");
        break;
    default:
        Logger().errorStream() << "MessageWnd::HandleTurnPhaseUpdate got unknown turn phase id";
        return;
        break;
    }

    *m_display += phase_str + "\n";
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::HandleGameStatusUpdate(const std::string& text)
{
    *m_display += text;
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::HandleLogMessage(const std::string& text)
{
    *m_display += text;
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::Clear()
{ m_display->Clear(); }

void MessageWnd::OpenForInput()
{
    GG::GUI::GetGUI()->SetFocusWnd(m_edit);
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::MessageEntered()
{
    // TODO: Check if message is chat, or some other command...?

    // send chat message
    std::string edit_text = m_edit->Text();
    if (edit_text != "") {
        if (m_history.size() == 1 || m_history[1] != edit_text) {
            m_history[0] = edit_text;
            m_history.push_front("");
        } else {
            m_history[0] = "";
        }
        while (12 < static_cast<int>(m_history.size()) + 1)
            m_history.pop_back();
        m_history_position = 0;
        HumanClientApp::GetApp()->Networking().SendMessage(GlobalChatMessage(HumanClientApp::GetApp()->PlayerID(), edit_text));
    }
    m_edit->Clear();
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::MessageHistoryUpRequested()
{
    if (m_history_position < static_cast<int>(m_history.size()) - 1) {
        m_history[m_history_position] = m_edit->Text();
        ++m_history_position;
        m_edit->SetText(m_history[m_history_position]);
    }
}

void MessageWnd::MessageHistoryDownRequested()
{
    if (0 < m_history_position) {
        m_history[m_history_position] = m_edit->Text();
        --m_history_position;
        m_edit->SetText(m_history[m_history_position]);
    }
}

