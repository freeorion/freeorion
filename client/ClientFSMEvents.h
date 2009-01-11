// -*- C++ -*-
#ifndef _ClientFSMEvents_h_
#define _ClientFSMEvents_h_

#include "../network/Message.h"

#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/statechart/event.hpp>


// Non-Message events
struct Disconnection : boost::statechart::event<Disconnection> {};


//  Message events
/** The base class for all state machine events that are based on Messages. */
struct MessageEventBase
{
    MessageEventBase(Message& message); ///< Basic ctor.
    Message m_message;
};

// Define Boost.Preprocessor list of all Message events
#define MESSAGE_EVENTS                             \
    (HostMPGame)                                   \
        (HostSPGame)                               \
        (JoinGame)                                 \
        (LobbyUpdate)                              \
        (LobbyChat)                                \
        (LobbyHostAbort)                           \
        (LobbyNonHostExit)                         \
        (SaveGame)                                 \
        (GameStart)                                \
        (TurnUpdate)                               \
        (TurnProgress)                             \
        (CombatStart)                              \
        (CombatRoundUpdate)                        \
        (CombatEnd)                                \
        (PlayerChat)                               \
        (VictoryDefeat)                            \
        (PlayerEliminated)                         \
        (EndGame)


#define DECLARE_MESSAGE_EVENT(r, data, name)                            \
    struct name :                                                       \
        boost::statechart::event<name>,                                 \
        MessageEventBase                                                \
    {                                                                   \
        name(Message& message) : MessageEventBase(message) {}           \
    };

BOOST_PP_SEQ_FOR_EACH(DECLARE_MESSAGE_EVENT, _, MESSAGE_EVENTS)

#undef DECLARE_MESSAGE_EVENT

#endif
