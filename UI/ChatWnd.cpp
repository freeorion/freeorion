#include "ChatWnd.h"

#include "CUIControls.h"
#include "PlayerListWnd.h"
#include "../client/human/HumanClientApp.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../network/Message.h"
#include "../universe/Universe.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Special.h"
#include "../universe/Species.h"
#include "../universe/Tech.h"
#include "../universe/Building.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/MultiplayerCommon.h"

#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/classic.hpp>
#include <GG/GUI.h>


namespace {
    const std::string OPEN_TAG = "[[";
    const std::string CLOSE_TAG = "]]";

    /** Converts (first, last) to a string, looks it up as a key in the
      * stringtable, then then appends this to the end of a string. */
    struct UserStringSubstituteAndAppend {
        UserStringSubstituteAndAppend(std::string& str) :
            m_str(str)
        {}

        void operator()(const char* first, const char* last) const {
            std::string token(first, last);
            if (token.empty() || !UserStringExists(token))
                return;
            m_str += UserString(token);
        }

        std::string&    m_str;
    };

    // sticks a sequence of characters onto the end of a string
    struct StringAppend {
        StringAppend(std::string& str) :
            m_str(str)
        {}

        void operator()(const char* first, const char* last) const
        { m_str += std::string(first, last); }

        std::string&    m_str;
    };


    std::string StringtableTextSubstitute(const std::string& input) {
        std::string retval;

        // set up parser
        using namespace boost::spirit::classic;
        rule<> token = *(anychar_p - space_p - CLOSE_TAG.c_str());
        rule<> var = OPEN_TAG.c_str() >> token[UserStringSubstituteAndAppend(retval)] >> CLOSE_TAG.c_str();
        rule<> non_var = anychar_p - OPEN_TAG.c_str();

        // parse and substitute variables
        try {
            parse(input.c_str(), *(non_var[StringAppend(retval)] | var));
        } catch (const std::exception&) {
            ErrorLogger() << "StringtableTextSubstitute caught exception when parsing input: " << input;
        }

        return retval;
    }
}

class MessageWndEdit : public CUIEdit {
public:
    //! \name Structors //@{
    MessageWndEdit();
    //@}

    //! \name Mutators //@{
    virtual void    KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);
    //@}

    /** emitted when user presses enter/return while entering text */
    mutable boost::signals2::signal<void ()> TextEnteredSignal;
    mutable boost::signals2::signal<void ()> UpPressedSignal;
    mutable boost::signals2::signal<void ()> DownPressedSignal;

private:
    void            FindGameWords();                    //!< Finds all game words for autocomplete
    void            AutoComplete();                     //!< Autocomplete current word

    /** AutoComplete helper function */
    bool            CompleteWord(const std::set<std::string>& names, const std::string& partial_word,
                                 const std::pair<GG::CPSize, const GG::CPSize>& cursor_pos,
                                 std::string& text);

    // Set for autocomplete game words
    std::set<std::string>       m_game_words;

    // Repeated tabs variables
     std::vector<std::string>   m_auto_complete_choices;
     unsigned int               m_repeated_tab_count;
     std::string                m_last_line_read;
     std::string                m_last_game_word;
};

////////////////////
// MessageWndEdit //
////////////////////
MessageWndEdit::MessageWndEdit() :
    CUIEdit(""),
    m_auto_complete_choices(),
    m_repeated_tab_count(0),
    m_last_line_read(),
    m_last_game_word()
{}

void MessageWndEdit::KeyPress(GG::Key key, boost::uint32_t key_code_point,
                              GG::Flags<GG::ModKey> mod_keys)
{
    switch (key) {
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
    CUIEdit::KeyPress(key, key_code_point, mod_keys);
}

void MessageWndEdit::FindGameWords() {
     // add player and empire names
    for (std::map<int, Empire*>::value_type& entry : Empires()) {
        m_game_words.insert(entry.second->Name());
        m_game_words.insert(entry.second->PlayerName());
    }
    // add system names
    for (TemporaryPtr<System> system : GetUniverse().Objects().FindObjects<System>()) {
        if (system->Name() != "")
            m_game_words.insert(system->Name());
    }
     // add ship names
    for (TemporaryPtr<Ship> ship : GetUniverse().Objects().FindObjects<Ship>()) {
        if (ship->Name() != "")
            m_game_words.insert(ship->Name());
    }
     // add ship design names

    for (const std::map<std::string, ShipDesign*>::value_type& entry : GetPredefinedShipDesignManager()) {
        if (entry.second->Name() != "")
            m_game_words.insert(UserString(entry.second->Name()));
    }
     // add specials names
    for (const std::string& special_name : SpecialNames()) {
        if (special_name != "")
            m_game_words.insert(UserString(special_name));
    }
     // add species names
    for (const std::map<std::string, Species*>::value_type& entry : GetSpeciesManager()) {
        if (entry.second->Name() != "")
            m_game_words.insert(UserString(entry.second->Name()));
    }
     // add techs names
    for (const std::string& tech_name : GetTechManager().TechNames()) {
        if (tech_name != "")
            m_game_words.insert(UserString(tech_name));
    }
    // add building type names
    for (const std::map<std::string, BuildingType*>::value_type& entry : GetBuildingTypeManager()) {
        if (entry.second->Name() != "")
            m_game_words.insert(UserString(entry.second->Name()));
    }
    // add ship hulls
    for (const std::map<std::string, ShipDesign*>::value_type& entry : GetPredefinedShipDesignManager()) {
        if (entry.second->Hull() != "")
            m_game_words.insert(UserString(entry.second->Hull()));
    }
    // add ship parts
    for (const std::map<std::string, ShipDesign*>::value_type& entry : GetPredefinedShipDesignManager()) {
        for (const std::string& part_name : entry.second->Parts()) {
            if (part_name != "")
                m_game_words.insert(UserString(part_name));
        }
    }
 }

void MessageWndEdit::AutoComplete() {
    std::string full_line = this->Text();

    // Check for repeated tab
    // if current line is same as the last read line
    if (m_last_line_read != "" && boost::equals(full_line, m_last_line_read)){
        if (m_repeated_tab_count >= m_auto_complete_choices.size())
            m_repeated_tab_count = 0;

        std::string next_word = m_auto_complete_choices.at(m_repeated_tab_count);

        if (next_word != "") {
            // Remove the old choice from the line
            // and replace it with the next choice
            full_line = full_line.substr(0, full_line.size() - (m_last_game_word.size() + 1));
            full_line.insert(full_line.size(), next_word + " ");
            this->SetText(full_line);
            GG::CPSize move_cursor_to = full_line.size() + GG::CP1;
            this->SelectRange(move_cursor_to, move_cursor_to);
            m_last_game_word = next_word;
            m_last_line_read = this->Text();
        }
        ++m_repeated_tab_count;
    }
    else {
        bool exact_match = false;

        std::pair<GG::CPSize, GG::CPSize> cursor_pos = this->CursorPosn();
        if (cursor_pos.first == cursor_pos.second && 0 < cursor_pos.first && cursor_pos.first <= full_line.size()) {
            std::string::size_type word_start = full_line.substr(0, Value(cursor_pos.first)).find_last_of(" :");
            if (word_start == std::string::npos)
                word_start = 0;
            else
                ++word_start;
            std::string partial_word = full_line.substr(word_start, Value(cursor_pos.first - word_start));
            if (partial_word == "")
                return;

            // Find game words to try an autocomplete
            FindGameWords();

            // See if word is an exact match with a game word
            for (const std::string& word : m_game_words) {
                if (boost::iequals(word, partial_word)) { // if there's an exact match, just add a space
                    full_line.insert(Value(cursor_pos.first), " ");
                    this->SetText(full_line);
                    this->SelectRange(cursor_pos.first + 1, cursor_pos.first + 1);
                    exact_match = true;
                    break;
                }
            }
            // If not an exact match try to complete the word
            if (!exact_match) 
                CompleteWord(m_game_words, partial_word, cursor_pos, full_line);
        }
    }
}

bool MessageWndEdit::CompleteWord(const std::set<std::string>& names, const std::string& partial_word,
                                  const std::pair<GG::CPSize, const GG::CPSize>& cursor_pos,
                                  std::string& full_line)
{
    // clear repeated tab variables
    m_auto_complete_choices.clear();
    m_repeated_tab_count = 0;

    bool partial_word_match = false;
    std::string game_word;

    // Check if the partial_word is contained in any game words
    for (const std::string& temp_game_word : names) {
        if (temp_game_word.size() >= partial_word.size()) {
            std::string game_word_partial = temp_game_word.substr(0, partial_word.size());

            if (boost::iequals(game_word_partial, partial_word)) {
                if (game_word_partial != "") {
                    // Add all possible word choices for repeated tab
                    m_auto_complete_choices.push_back(temp_game_word);
                    partial_word_match = true;
                }
            }
        }
    }
  
    if (!partial_word_match)
        return false;

    // Grab first autocomplete choice
    game_word = m_auto_complete_choices.at(m_repeated_tab_count++);
    m_last_game_word = game_word;

    // Remove the partial_word from the line
    // and replace it with the properly formated game word
    full_line = full_line.substr(0, full_line.size() - partial_word.size());
    full_line.insert(full_line.size(), game_word + " ");
    this->SetText(full_line);
    m_last_line_read = this->Text();
    GG::CPSize move_cursor_to = full_line.size() + GG::CP1;
    this->SelectRange(move_cursor_to, move_cursor_to);
    return true;
}

////////////////////
//   MessageWnd   //
////////////////////
MessageWnd::MessageWnd(const std::string& config_name) :
    CUIWnd(UserString("MESSAGES_PANEL_TITLE"),
           GG::INTERACTIVE | GG::DRAGABLE | GG::ONTOP | GG::RESIZABLE | CLOSABLE | PINABLE,
           config_name),
    m_display(0),
    m_edit(0),
    m_display_show_time(0),
    m_history(),
    m_history_position()
{
    m_display = new CUIMultiEdit("", GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY | GG::MULTI_TERMINAL_STYLE | GG::MULTI_INTEGRAL_HEIGHT);
    AttachChild(m_display);
    m_display->SetMaxLinesOfHistory(100); // executing this line seems to cause crashes in MultiEdit when adding more lines to the control than the history limit

    m_edit = new MessageWndEdit();
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
                        GG::Pt(ClientWidth(), ClientHeight() - PAD - m_edit->MinUsableSize().y));
    m_edit->SizeMove(GG::Pt(GG::X0, ClientHeight() - m_edit->MinUsableSize().y),
                     GG::Pt(ClientWidth() - GG::X(CUIWnd::INNER_BORDER_ANGLE_OFFSET), ClientHeight()));
}

void MessageWnd::CloseClicked()
{ ClosingSignal(); }

void MessageWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    const GG::Pt old_size = Size();
    CUIWnd::SizeMove(ul, lr);
    if (old_size != Size())
        RequirePreRender();
}

void MessageWnd::PreRender() {
    GG::Wnd::PreRender();
    DoLayout();
}

void MessageWnd::HandlePlayerChatMessage(const std::string& text, int sender_player_id, int recipient_player_id) {
    const ClientApp* app = ClientApp::GetApp();
    if (!app) {
        ErrorLogger() << "MessageWnd::HandlePlayerChatMessage couldn't get client app!";
        return;
    }

    const std::map<int, PlayerInfo>& players = app->Players();
    std::map<int, PlayerInfo>::const_iterator player_it = players.find(sender_player_id);
    if (player_it == players.end()) {
        ErrorLogger() << "MessageWnd::HandlePlayerChatMessage couldn't message sending player with id: " << sender_player_id;
        return;
    }

    const std::string& sender_name = player_it->second.name;
    const int& sender_empire_id = player_it->second.empire_id;

    GG::Clr sender_colour(ClientUI::TextColor());
    if (const Empire* sender_empire = GetEmpire(sender_empire_id))
        sender_colour = sender_empire->Color();

    std::string filtered_message = StringtableTextSubstitute(text);
    std::string wrapped_text = RgbaTag(sender_colour) + sender_name + ": " + filtered_message + "</rgba>";

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
        ErrorLogger() << "MessageWnd::HandleTurnPhaseUpdate got unknown turn phase id";
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
            for (int recipient_id : recipients)
                net.SendMessage(SingleRecipientChatMessage(sender_id, recipient_id, text));
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

        ClientUI* client_ui = ClientUI::GetClientUI();
        if (!client_ui)
            return;

        // execute command matching understood syntax
        if (boost::iequals(command, "zoom") && !params.empty()) {
            client_ui->ZoomToObject(params) || client_ui->ZoomToContent(params, true);   // params came from chat, so will be localized, so should be reverse looked up to find internal name from human-readable name for zooming to content
        } else if (boost::iequals(command, "pedia")) {
            if (params.empty())
                client_ui->ZoomToEncyclopediaEntry(UserStringNop("ENC_INDEX"));
            else
                client_ui->ZoomToContent(params, true);
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
        m_edit->SelectRange(m_edit->Length(), m_edit->Length());    // put cursor at end of historical input
    }
}

void MessageWnd::MessageHistoryDownRequested() {
    if (0 < m_history_position) {
        m_history[m_history_position] = m_edit->Text();
        --m_history_position;
        m_edit->SetText(m_history[m_history_position]);
        m_edit->SelectRange(m_edit->Length(), m_edit->Length());    // put cursor at end of historical input
    }
}

