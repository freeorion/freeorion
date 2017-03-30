#include "ClientFSMEvents.h"


MessageEventBase::MessageEventBase(MessagePacket& message)
{ swap(m_message, message); }
