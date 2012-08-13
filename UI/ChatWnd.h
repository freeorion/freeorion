// -*- C++ -*-
#ifndef _ChatWnd_h_
#define _ChatWnd_h_

#include "CUIWnd.h"
#include "CUIControls.h"
#include "../network/Message.h"

#include <deque>
#include <string>


namespace GG { class MultiEdit; }
class MessageWndEdit;

class MessageWnd : public CUIWnd {
public:
    //! \name Structors //@{
    MessageWnd(GG::X x, GG::Y y, GG::X w, GG::Y h);
    //@}

    //! \name Mutators //@{
    void            HandlePlayerChatMessage(const std::string& text, int sender_player_id, int recipient_player_id);
    void            HandlePlayerStatusUpdate(Message::PlayerStatus player_status, int about_player_id);
    void            HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id);
    void            HandleGameStatusUpdate(const std::string& text);
    void            HandleLogMessage(const std::string& text);
    void            Clear();
    void            OpenForInput();

    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    //@}

    mutable boost::signal<void ()> TypingSignal;        // emitted when the edit gains focus.  keyboard accelerators elsehwere should be disabled
    mutable boost::signal<void ()> DoneTypingSignal;    // emitted when the edit loses focus.  not necessary when a message is sent

private:
    void            DoLayout();
    void            MessageEntered();
    void            MessageHistoryUpRequested();
    void            MessageHistoryDownRequested();

    GG::MultiEdit*          m_display;
    MessageWndEdit*         m_edit;
    int                     m_display_show_time;
    std::deque<std::string> m_history;
    int                     m_history_position;
};

#endif
