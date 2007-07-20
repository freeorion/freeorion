#include "ClientFSMEvents.h"


MessageEventBase::MessageEventBase(Message& message)
{ swap(m_message, message); }
