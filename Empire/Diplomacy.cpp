#include "Diplomacy.h"

#include <boost/lexical_cast.hpp>

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

std::string DiplomaticMessage::Dump() const {
    std::string retval;
    retval += "Dimplomatic message from : " + boost::lexical_cast<std::string>(m_sender_empire) +
              " to: " + boost::lexical_cast<std::string>(m_recipient_empire) +
              " about: ";
    switch (m_type) {
    case WAR_DECLARATION:   retval += "War Declaration";    break;
    case PEACE_PROPOSAL:    retval += "Peace Proposal";     break;
    case ACCEPT_PROPOSAL:   retval += "Accept Proposal";    break;
    case CANCEL_PROPOSAL:   retval += "Cancel Proposal";    break;
    case INVALID_DIPLOMATIC_MESSAGE_TYPE:
    default:                retval += "Invalid / Unknown";  break;
    }
    return retval;
}

DiplomaticMessage WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::WAR_DECLARATION); }

DiplomaticMessage PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::PEACE_PROPOSAL); }

DiplomaticMessage AcceptDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::ACCEPT_PROPOSAL); }

DiplomaticMessage CancelDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::CANCEL_PROPOSAL); }

