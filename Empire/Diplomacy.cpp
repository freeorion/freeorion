#include "Diplomacy.h"

FO_COMMON_API extern const int ALL_EMPIRES;

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

bool operator==(const DiplomaticMessage& lhs, const DiplomaticMessage& rhs) {
    return lhs.RecipientEmpireID() == rhs.RecipientEmpireID() &&
           lhs.SenderEmpireID() == rhs.SenderEmpireID() &&
           lhs.GetType() == rhs.GetType();
}

bool operator!=(const DiplomaticMessage& lhs, const DiplomaticMessage& rhs)
{ return !(lhs == rhs); }

std::string DiplomaticMessage::Dump() const {
    std::string retval;
    retval += "Dimplomatic message from : " + std::to_string(m_sender_empire) +
              " to: " + std::to_string(m_recipient_empire) + " about: ";
    switch (m_type) {
    case WAR_DECLARATION:           retval += "War Declaration";            break;
    case PEACE_PROPOSAL:            retval += "Peace Proposal";             break;
    case ACCEPT_PEACE_PROPOSAL:     retval += "Accept Peace Proposal";      break;
    case ALLIES_PROPOSAL:           retval += "Allies Proposal";            break;
    case ACCEPT_ALLIES_PROPOSAL:    retval += "Accept Allies Proposal";     break;
    case END_ALLIANCE_DECLARATION:  retval += "End Alliance Declaration";   break;
    case CANCEL_PROPOSAL:           retval += "Cancel Proposal";            break;
    case REJECT_PROPOSAL:           retval += "Reject Proposal";            break;
    case INVALID_DIPLOMATIC_MESSAGE_TYPE:
    default:                        retval += "Invalid / Unknown";          break;
    }
    return retval;
}


DiplomaticStatusUpdateInfo::DiplomaticStatusUpdateInfo() :
    empire1_id(ALL_EMPIRES),
    empire2_id(ALL_EMPIRES),
    diplo_status(INVALID_DIPLOMATIC_STATUS)
{}

DiplomaticStatusUpdateInfo::DiplomaticStatusUpdateInfo(int empire1_id_, int empire2_id_, DiplomaticStatus status) :
    empire1_id(empire1_id_),
    empire2_id(empire2_id_),
    diplo_status(status)
{}


DiplomaticMessage WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::WAR_DECLARATION); }

DiplomaticMessage PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::PEACE_PROPOSAL); }

DiplomaticMessage AcceptPeaceDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::ACCEPT_PEACE_PROPOSAL); }

DiplomaticMessage AlliesProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::ALLIES_PROPOSAL); }

DiplomaticMessage AcceptAlliesDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::ACCEPT_ALLIES_PROPOSAL); }

DiplomaticMessage EndAllianceDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::END_ALLIANCE_DECLARATION); }

DiplomaticMessage CancelDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::CANCEL_PROPOSAL); }

DiplomaticMessage RejectProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::REJECT_PROPOSAL); }

