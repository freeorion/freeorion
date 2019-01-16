#ifndef _ChatWnd_h_
#define _ChatWnd_h_

#include <deque>
#include <string>
#include <GG/GGFwd.h>

#include "CUIWnd.h"
#include "CUIControls.h"
#include "../network/Message.h"


class MessageWndEdit;

class MessageWnd : public CUIWnd {
public:
    MessageWnd(GG::Flags<GG::WndFlag> flags, const std::string& config_name = "");
    void CompleteConstruction() override;
    //@}

    //! \name Mutators //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;

    void PreRender() override;

    void            HandlePlayerChatMessage(const std::string& text,
                                            const std::string& player_name,
                                            GG::Clr text_color,
                                            const boost::posix_time::ptime& timestamp,
                                            int recipient_player_id);
    void            HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id, bool prefixed = false);
    void            HandleGameStatusUpdate(const std::string& text);
    void            HandleLogMessage(const std::string& text);
    void            Clear();
    void            OpenForInput();
    //@}

    /** emitted when the edit gains focus.  Keyboard accelerators elsewhere
        should be disabled */
    mutable boost::signals2::signal<void ()> TypingSignal;
    /** emitted when the edit loses focus.  not necessary when a message
        is sent */
    mutable boost::signals2::signal<void ()> DoneTypingSignal;
    mutable boost::signals2::signal<void ()> ClosingSignal;

private:
    void CloseClicked() override;

    void            DoLayout();
    void            MessageEntered();
    void            MessageHistoryUpRequested();
    void            MessageHistoryDownRequested();

    std::shared_ptr<GG::MultiEdit>          m_display;
    std::shared_ptr<MessageWndEdit>         m_edit;
    int                     m_display_show_time;
    std::deque<std::string> m_history;
    int                     m_history_position;
};

#endif
