#ifndef _ChatWnd_h_
#define _ChatWnd_h_

#include <deque>
#include <string>
#include <GG/GGFwd.h>

#include "CUIWnd.h"
#include "CUIControls.h"
#include "../network/Message.h"


class MessageWndEdit;

class MessageWnd final : public CUIWnd {
public:
    MessageWnd(GG::Flags<GG::WndFlag> flags, std::string_view config_name = "");
    void CompleteConstruction() override;

    std::string GetText() const;

    void SizeMove(GG::Pt ul, GG::Pt lr) override;

    void PreRender() override;

    void HandlePlayerChatMessage(const std::string& text, const std::string& player_name,
                                 GG::Clr player_name_color, const boost::posix_time::ptime& timestamp,
                                 int recipient_player_id, bool pm);
    void HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id, bool prefixed = false);
    void HandleGameStatusUpdate(const std::string& text);
    void HandleLogMessage(const std::string& text);
    void HandleDiplomaticStatusChange(int empire1_id, int empire2_id);
    void Clear();
    void OpenForInput();
    void SetChatText(std::string chat_text);

    /** emitted when the edit gains focus.  Keyboard accelerators elsewhere
        should be disabled */
    mutable boost::signals2::signal<void ()> TypingSignal;
    /** emitted when the edit loses focus.  not necessary when a message
        is sent */
    mutable boost::signals2::signal<void ()> DoneTypingSignal;
    mutable boost::signals2::signal<void ()> ClosingSignal;

private:
    void CloseClicked() override;
    void LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDrag(GG::Pt pt, GG::Pt move, GG::Flags<GG::ModKey> mod_keys) override;

    void DoLayout();
    void HandleTextCommand(const std::string& text);
    void MessageEntered();
    void MessageHistoryUpRequested();
    void MessageHistoryDownRequested();

    std::shared_ptr<GG::MultiEdit>     m_display;
    std::shared_ptr<MessageWndEdit>    m_edit;
    int                                m_display_show_time = 0;
    std::deque<std::string>            m_history;
    int                                m_history_position = 0;
    boost::signals2::scoped_connection m_diplo_status_connection;
};

#endif
