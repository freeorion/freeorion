#include "ChatWnd.h"

#include "CUIControls.h"
#include "PlayerListWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../network/Message.h"
#include "../universe/Universe.h"
#include "../universe/System.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/Tech.h"
#include "../universe/Building.h"
#include "../util/OptionsDB.h"
#include "../util/MultiplayerCommon.h"

#include <boost/algorithm/string.hpp>
#include <GG/GUI.h>


class MessageWndEdit : public CUIEdit {
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
    void            FindGameWords();                    //!< Finds all game words for autocomplete
    void            AutoComplete();                     //!< Autocomplete current word
    bool            CompleteWord(const std::set<std::string>& names, const std::string& partial_word,         //!< AutoComplete helper function
                                const std::pair<GG::CPSize, const GG::CPSize>& cursor_pos, std::string& text);

    // Set for autocomplete game words
    std::set<std::string> m_gameWords;

    // Repeated tabs variables
     std::vector<std::string> autoCompleteChoices;
     unsigned int m_repeatedTabCount;
     std::string m_lastLineRead;
     std::string m_lastGameWord;
};

////////////////////
// MessageWndEdit //
////////////////////
MessageWndEdit::MessageWndEdit(GG::X x, GG::Y y, GG::X w) :
    CUIEdit(x, y, w, ""), m_lastGameWord(""), m_lastLineRead(""), m_repeatedTabCount(0)
{}

void MessageWndEdit::GainingFocus() {
    GG::Edit::GainingFocus();
    GainingFocusSignal();
}

void MessageWndEdit::LosingFocus() {
    GG::Edit::LosingFocus();
    LosingFocusSignal();
}

void MessageWndEdit::KeyPress(GG::Key key, boost::uint32_t key_code_point,
                              GG::Flags<GG::ModKey> mod_keys)
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

void MessageWndEdit::FindGameWords() {
     // add player and empire names
    for (EmpireManager::const_iterator it = Empires().begin(); it != Empires().end(); ++it) {
        m_gameWords.insert(it->second->Name());
        m_gameWords.insert(it->second->PlayerName());
    }
    // add system names
    std::vector<System*> systems = GetUniverse().Objects().FindObjects<System>();
    for (unsigned int i = 0; i < systems.size(); ++i) {
        if (systems[i]->Name() != "")
            m_gameWords.insert(systems[i]->Name());
    }
     // add ship names
    std::vector<Ship*> ships = GetUniverse().Objects().FindObjects<Ship>();
    for (unsigned int i = 0; i < ships.size(); ++i) {
        if (ships[i]->Name() != "")
            m_gameWords.insert(ships[i]->Name());
    }
     // add ship design names
    for (PredefinedShipDesignManager::iterator it = GetPredefinedShipDesignManager().begin();
         it != GetPredefinedShipDesignManager().end(); ++it)
    {
        if (it->second->Name() != "")
            m_gameWords.insert(UserString(it->second->Name()));
    }
     // add specials names
    std::vector<std::string> specials =  SpecialNames();
    for (unsigned int i = 0; i < specials.size(); ++i) {
        if (specials[i] != "")
            m_gameWords.insert(UserString(specials[i]));
    }
     // add species names
    for (SpeciesManager::iterator it = GetSpeciesManager().begin();
         it != GetSpeciesManager().end(); ++it)
    {
        if (it->second->Name() != "")
            m_gameWords.insert(UserString(it->second->Name()));
    }
     // add techs names
    std::vector<std::string> techs = GetTechManager().TechNames();
    for (unsigned int i = 0; i < techs.size(); ++i) {
        if (techs[i] != "")
            m_gameWords.insert(UserString(techs[i]));
    }
    // add building type names
    for (BuildingTypeManager::iterator it = GetBuildingTypeManager().begin();
         it != GetBuildingTypeManager().end(); ++it)
    {
        if (it->second->Name() != "")
            m_gameWords.insert(UserString(it->second->Name()));
    }
    // add ship hulls
    for (PredefinedShipDesignManager::iterator it = GetPredefinedShipDesignManager().begin();
         it != GetPredefinedShipDesignManager().end(); ++it)
    {
        if (it->second->Hull() != "")
            m_gameWords.insert(UserString(it->second->Hull()));
    }
    // add ship parts
    for (PredefinedShipDesignManager::iterator it = GetPredefinedShipDesignManager().begin();
         it != GetPredefinedShipDesignManager().end(); ++it)
    {
        const std::vector<std::string>& parts = it->second->Parts();
        for (std::vector<std::string>::const_iterator it1 = parts.begin(); it1 != parts.end(); ++it1) {
            if (*it1 != "")
                m_gameWords.insert(UserString(*it1));
        }
    }
 }

void MessageWndEdit::AutoComplete() {
    std::string fullLine = this->Text();

    // Check for repeated tab
    // if current line is same as the last read line
    if (m_lastLineRead != "" && boost::equals(fullLine, m_lastLineRead)){
        if (m_repeatedTabCount >= autoCompleteChoices.size())
            m_repeatedTabCount = 0;

        std::string nextWord = autoCompleteChoices.at(m_repeatedTabCount);

        if (nextWord != "") {
             // Remove the old choice from the line
            // and replace it with the next choice
            fullLine = fullLine.substr(0, fullLine.size() - (m_lastGameWord.size() + 1));
            fullLine.insert(fullLine.size(), nextWord + " ");
            this->SetText(fullLine);
            GG::CPSize move_cursor_to = fullLine.size() + GG::CP1;
            this->SelectRange(move_cursor_to, move_cursor_to);
            m_lastGameWord = nextWord;
            m_lastLineRead = this->Text();
        }
        ++m_repeatedTabCount;
    }
    else {
        bool exactMatch = false;

        std::pair<GG::CPSize, GG::CPSize> cursor_pos = this->CursorPosn();
        if (cursor_pos.first == cursor_pos.second && 0 < cursor_pos.first && cursor_pos.first <= fullLine.size()) {
            std::string::size_type word_start = fullLine.substr(0, Value(cursor_pos.first)).find_last_of(" :");
            if (word_start == std::string::npos)
                word_start = 0;
            else
                ++word_start;
            std::string partial_word = fullLine.substr(word_start, Value(cursor_pos.first - word_start));
            if (partial_word == "")
                return;

            // Find game words to try an autocomplete
            FindGameWords();

            // See if word is an exact match with a game word
            for (std::set<std::string>::const_iterator it = m_gameWords.begin(); it != m_gameWords.end(); ++it) {
                if (boost::iequals(*it, partial_word)) { // if there's an exact match, just add a space
                    fullLine.insert(Value(cursor_pos.first), " ");
                    this->SetText(fullLine);
                    this->SelectRange(cursor_pos.first + 1, cursor_pos.first + 1);
                    exactMatch = true;
                    break;
                }
            }
            // If not an exact match try to complete the word
            if (!exactMatch) 
                CompleteWord(m_gameWords, partial_word, cursor_pos, fullLine);
        }
    }
}

bool MessageWndEdit::CompleteWord(const std::set<std::string>& names,
                                  const std::string& partial_word,
                                  const std::pair<GG::CPSize,
                                  const GG::CPSize>& cursor_pos,
                                  std::string& fullLine)
{
    // clear repeated tab variables
    autoCompleteChoices.clear();
    m_repeatedTabCount = 0;

    bool partialWordMatch = false;
    std::string gameWord;

    // Check if the partial_word is contained in any game words
    for (std::set<std::string>::const_iterator it = names.begin(); it != names.end(); ++it) {
        std::string tempGameWord = *it;

        if (tempGameWord.size() >= partial_word.size()) {
            std::string gameWordPartial = tempGameWord.substr(0, partial_word.size());

            if (boost::iequals(gameWordPartial, partial_word)) {
                if (gameWordPartial != "") {
                    // Add all possible word choices for repeated tab
                    autoCompleteChoices.push_back(tempGameWord);
                    partialWordMatch = true;
                }
            }
        }
    }
  
    if (!partialWordMatch)
        return false;

    // Grab first autocomplete choice
    gameWord = autoCompleteChoices.at(m_repeatedTabCount++);
    m_lastGameWord = gameWord;

    // Remove the partial_word from the line
    // and replace it with the properly formated game word
    fullLine = fullLine.substr(0, fullLine.size() - partial_word.size());
    fullLine.insert(fullLine.size(), gameWord + " ");
    this->SetText(fullLine);
    m_lastLineRead = this->Text();
    GG::CPSize move_cursor_to = fullLine.size() + GG::CP1;
    this->SelectRange(move_cursor_to, move_cursor_to);
    return true;
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
    m_display->SetMaxLinesOfHistory(100); // executing this line seems to cause crashes in MultiEdit when adding more lines to the control than the history limit

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

void MessageWnd::DoLayout() {
    const GG::Y PAD(3);
    m_display->SizeMove(GG::Pt(GG::X0, GG::Y0),
                        GG::Pt(ClientWidth(), ClientHeight() - PAD - m_edit->Height()));
    m_edit->SizeMove(GG::Pt(GG::X0, ClientHeight() - m_edit->Height()),
                     GG::Pt(ClientWidth() - GG::X(CUIWnd::INNER_BORDER_ANGLE_OFFSET), ClientHeight()));
}

void MessageWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        DoLayout();
}

void MessageWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey>& mod_keys) {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt final_move(std::max(-ul.x, std::min(move.x, GG::GUI::GetGUI()->AppWidth() - 1 - lr.x)),
                      std::max(-ul.y, std::min(move.y, GG::GUI::GetGUI()->AppHeight() - 1 - lr.y)));
    GG::Wnd::LDrag(pt + final_move - move, final_move, mod_keys);
}

void MessageWnd::HandlePlayerChatMessage(const std::string& text, int sender_player_id, int recipient_player_id) {
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

    GG::Clr sender_colour(ClientUI::TextColor());
    if (const Empire* sender_empire = app->GetPlayerEmpire(sender_player_id))
        sender_colour = sender_empire->Color();

    std::string wrapped_text = RgbaTag(sender_colour) + sender_name + ": " + text + "</rgba>";

    *m_display += wrapped_text + "\n";
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::HandlePlayerStatusUpdate(Message::PlayerStatus player_status, int about_player_id)
{}

void MessageWnd::HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id) {
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
        phase_str = UserString("TURN_PROGRESS_PHASE_ORDERS");
        break;
    case Message::COLONIZE_AND_SCRAP:
        phase_str = UserString("TURN_PROGRESS_COLONIZE_AND_SCRAP");
        break;
    case Message::DOWNLOADING:
        phase_str = UserString("TURN_PROGRESS_PHASE_DOWNLOADING");
        break;
    case Message::LOADING_GAME:
        phase_str = UserString("TURN_PROGRESS_PHASE_LOADING_GAME");
        break;
    case Message::GENERATING_UNIVERSE:
        phase_str = UserString("TURN_PROGRESS_PHASE_GENERATING_UNIVERSE");
        break;
    case Message::STARTING_AIS:
        phase_str = UserString("TURN_PROGRESS_STARTING_AIS");
        break;
    default:
        Logger().errorStream() << "MessageWnd::HandleTurnPhaseUpdate got unknown turn phase id";
        return;
        break;
    }

    *m_display += phase_str + "\n";
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::HandleGameStatusUpdate(const std::string& text) {
    *m_display += text;
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::HandleLogMessage(const std::string& text) {
    *m_display += text;
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

void MessageWnd::Clear()
{ m_display->Clear(); }

void MessageWnd::OpenForInput() {
    GG::GUI::GetGUI()->SetFocusWnd(m_edit);
    m_display_show_time = GG::GUI::GetGUI()->Ticks();
}

namespace {
    void SendChatMessage(const std::string& text, std::set<int> recipients) {
        ClientNetworking& net = HumanClientApp::GetApp()->Networking();
        int sender_id = HumanClientApp::GetApp()->PlayerID();

        if (recipients.empty()) {
            net.SendMessage(GlobalChatMessage(sender_id, text));
        } else {
            recipients.insert(sender_id);   // ensure recipient sees own sent message
            for (std::set<int>::const_iterator it = recipients.begin(); it != recipients.end(); ++it)
                net.SendMessage(SingleRecipientChatMessage(sender_id, *it, text));
        }
    }

    void HandleTextCommand(const std::string& text) {
        if (text.size() < 2)
            return;

        // extract command and parameters substrings
        std::string command, params;
        std::string::size_type space_pos = text.find_first_of(' ');
        command = boost::trim_copy(text.substr(1, space_pos));
        if (command.empty())
            return;
        if (space_pos != std::string::npos)
            params = boost::trim_copy(text.substr(space_pos, std::string::npos));

        // execute command matching understood syntax
        if (boost::iequals(command, "zoom") && !params.empty()) {
            ClientUI* client_ui = ClientUI::GetClientUI();
            client_ui->ZoomToObject(params) || client_ui->ZoomToContent(params, true);   // params came from chat, so will be localized, so should be reverse looked up to find internal name from human-readable name for zooming to content
        }
    }
}

void MessageWnd::MessageEntered() {
    std::string trimmed_text = boost::trim_copy(m_edit->Text());
    if (trimmed_text.empty())
        return;

    m_display_show_time = GG::GUI::GetGUI()->Ticks();

    // update history
    if (m_history.size() == 1 || m_history[1] != trimmed_text) {
        m_history[0] = trimmed_text;
        m_history.push_front("");
    } else {
        m_history[0] = "";
    }
    while (12 < static_cast<int>(m_history.size()) + 1)
        m_history.pop_back();
    m_history_position = 0;

    // if message starts with / treat it as a command
    if (trimmed_text[0] == '/') {
        HandleTextCommand(trimmed_text);
    } else {
        // otherwise, treat message as chat and send to recipients
        std::set<int> recipients;
        if (PlayerListWnd* player_list_wnd = ClientUI::GetClientUI()->GetPlayerListWnd())
            recipients = player_list_wnd->SelectedPlayerIDs();
        SendChatMessage(trimmed_text, recipients);
    }

    m_edit->Clear();
}

void MessageWnd::MessageHistoryUpRequested() {
    if (m_history_position < static_cast<int>(m_history.size()) - 1) {
        m_history[m_history_position] = m_edit->Text();
        ++m_history_position;
        m_edit->SetText(m_history[m_history_position]);
    }
}

void MessageWnd::MessageHistoryDownRequested() {
    if (0 < m_history_position) {
        m_history[m_history_position] = m_edit->Text();
        --m_history_position;
        m_edit->SetText(m_history[m_history_position]);
    }
}

