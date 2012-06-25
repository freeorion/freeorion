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

WarDeclarationDiplomaticMessage::WarDeclarationDiplomaticMessage() :
    DiplomaticMessage(ALL_EMPIRES, ALL_EMPIRES, WAR_DECLARATION)
{}

WarDeclarationDiplomaticMessage::WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id) :
    DiplomaticMessage(sender_empire_id, recipient_empire_id, WAR_DECLARATION)
{}

PeaceProposalDiplomaticMessage::PeaceProposalDiplomaticMessage() :
    DiplomaticMessage(ALL_EMPIRES, ALL_EMPIRES, PEACE_PROPOSAL)
{}

PeaceProposalDiplomaticMessage::PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id) :
    DiplomaticMessage(sender_empire_id, recipient_empire_id, PEACE_PROPOSAL)
{}

PeaceAcceptanceDiplomaticMessage::PeaceAcceptanceDiplomaticMessage() :
    DiplomaticMessage(ALL_EMPIRES, ALL_EMPIRES, PEACE_ACCEPT)
{}

PeaceAcceptanceDiplomaticMessage::PeaceAcceptanceDiplomaticMessage(int sender_empire_id, int recipient_empire_id) :
    DiplomaticMessage(sender_empire_id, recipient_empire_id, PEACE_ACCEPT)
{}
