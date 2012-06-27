#include "Diplomacy.h"

extern const int ALL_EMPIRES;

DiplomaticMessage::DiplomaticMessage() :
    m_sender_empire(ALL_EMPIRES),
    m_recipient_empire(ALL_EMPIRES),
    m_type(INVALID_DIPLOMATIC_MESSAGE_TYPE)
{}

DiplomaticMessage::DiplomaticMessage(int sender_empire_id, int recipient_empire_id, DiplomaticMessageType type) :
    m_sender_empire(sender_empire_id),
    m_recipient_empire(recipient_empire_id),
    m_type(type)
{}

DiplomaticMessage WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::WAR_DECLARATION); }

DiplomaticMessage PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::PEACE_PROPOSAL); }

DiplomaticMessage AcceptDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::ACCEPT_PROPOSAL); }

DiplomaticMessage CancelDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::CANCEL_PROPOSAL); }

