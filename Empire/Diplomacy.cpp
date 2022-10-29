#include "Diplomacy.h"

#include "../universe/ConstantsFwd.h"
#include "../util/GameRules.h"
#include "../util/i18n.h"

namespace {
    void AddRules(GameRules& rules) {
        // determine if diplomacy allowed
        rules.Add<std::string>(UserStringNop("RULE_DIPLOMACY"), UserStringNop("RULE_DIPLOMACY_DESC"),
                               UserStringNop("MULTIPLAYER"), UserStringNop("RULE_DIPLOMACY_ALLOWED_FOR_ALL"),
                               true,
                               DiscreteValidator<std::string>(std::array{
                                   "RULE_DIPLOMACY_ALLOWED_FOR_ALL",
                                   "RULE_DIPLOMACY_FORBIDDEN_FOR_ALL"
                               }));
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}

DiplomaticMessage::DiplomaticMessage() :
    m_sender_empire(ALL_EMPIRES),
    m_recipient_empire(ALL_EMPIRES),
    m_type(Type::INVALID)
{}

DiplomaticMessage::DiplomaticMessage(int sender_empire_id, int recipient_empire_id, Type type) :
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
    case Type::WAR_DECLARATION:           retval += "War Declaration";            break;
    case Type::PEACE_PROPOSAL:            retval += "Peace Proposal";             break;
    case Type::ACCEPT_PEACE_PROPOSAL:     retval += "Accept Peace Proposal";      break;
    case Type::ALLIES_PROPOSAL:           retval += "Allies Proposal";            break;
    case Type::ACCEPT_ALLIES_PROPOSAL:    retval += "Accept Allies Proposal";     break;
    case Type::END_ALLIANCE_DECLARATION:  retval += "End Alliance Declaration";   break;
    case Type::CANCEL_PROPOSAL:           retval += "Cancel Proposal";            break;
    case Type::REJECT_PROPOSAL:           retval += "Reject Proposal";            break;
    case Type::INVALID:
    default:                              retval += "Invalid / Unknown";          break;
    }
    return retval;
}

bool DiplomaticMessage::IsAllowed() const {
    return GetGameRules().Get<std::string>("RULE_DIPLOMACY") !=
        UserStringNop("RULE_DIPLOMACY_FORBIDDEN_FOR_ALL");
}


DiplomaticStatusUpdateInfo::DiplomaticStatusUpdateInfo() :
    empire1_id(ALL_EMPIRES),
    empire2_id(ALL_EMPIRES),
    diplo_status(DiplomaticStatus::INVALID_DIPLOMATIC_STATUS)
{}

DiplomaticStatusUpdateInfo::DiplomaticStatusUpdateInfo(int empire1_id_, int empire2_id_, DiplomaticStatus status) :
    empire1_id(empire1_id_),
    empire2_id(empire2_id_),
    diplo_status(status)
{}


DiplomaticMessage WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::WAR_DECLARATION); }

DiplomaticMessage PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::PEACE_PROPOSAL); }

DiplomaticMessage AcceptPeaceDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::ACCEPT_PEACE_PROPOSAL); }

DiplomaticMessage AlliesProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::ALLIES_PROPOSAL); }

DiplomaticMessage AcceptAlliesDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::ACCEPT_ALLIES_PROPOSAL); }

DiplomaticMessage EndAllianceDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::END_ALLIANCE_DECLARATION); }

DiplomaticMessage CancelDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::CANCEL_PROPOSAL); }

DiplomaticMessage RejectProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::REJECT_PROPOSAL); }
