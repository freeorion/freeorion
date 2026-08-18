#include "Diplomacy.h"

#include "../universe/ConstantsFwd.h"
#include "../util/GameRules.h"
#include "../util/GameRuleRanks.h"
#include "../util/i18n.h"

namespace {
    void AddRules(GameRules& rules) {
        // determine if diplomacy allowed
        rules.Add<std::string>(UserStringNop("RULE_DIPLOMACY"), UserStringNop("RULE_DIPLOMACY_DESC"),
                               GameRuleCategories::GameRuleCategory::MULTIPLAYER,
                               UserStringNop("RULE_DIPLOMACY_ALLOWED_FOR_ALL"),
                               true,
                               GameRuleRanks::RULE_DIPLOMACY_RANK,
                               DiscreteValidator<std::string>(std::array{
                                   "RULE_DIPLOMACY_ALLOWED_FOR_ALL",
                                   "RULE_DIPLOMACY_FORBIDDEN_FOR_ALL"
                               }));
    }
    bool temp_bool = RegisterGameRules(&AddRules);
}

std::string DiplomaticMessage::Dump() const {
    std::string retval;
    retval += "Dimplomatic message from : " + to_string(m_sender_empire) +
              " to: " + to_string(m_recipient_empire) + " about: ";
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


DiplomaticMessage WarDeclarationDiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::WAR_DECLARATION); }

DiplomaticMessage PeaceProposalDiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::PEACE_PROPOSAL); }

DiplomaticMessage AcceptPeaceDiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::ACCEPT_PEACE_PROPOSAL); }

DiplomaticMessage AlliesProposalDiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::ALLIES_PROPOSAL); }

DiplomaticMessage AcceptAlliesDiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::ACCEPT_ALLIES_PROPOSAL); }

DiplomaticMessage EndAllianceDiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::END_ALLIANCE_DECLARATION); }

DiplomaticMessage CancelDiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::CANCEL_PROPOSAL); }

DiplomaticMessage RejectProposalDiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id)
{ return DiplomaticMessage(sender_empire_id, recipient_empire_id, DiplomaticMessage::Type::REJECT_PROPOSAL); }
