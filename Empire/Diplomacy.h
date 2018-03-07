#ifndef _Diplomacy_h_
#define _Diplomacy_h_

#include <string>
#include <boost/serialization/access.hpp>

#include "../universe/Enums.h"
#include "../util/Export.h"

class FO_COMMON_API DiplomaticMessage {
public:
    enum DiplomaticMessageType {
        INVALID_DIPLOMATIC_MESSAGE_TYPE = -1,
        WAR_DECLARATION,
        PEACE_PROPOSAL,
        ACCEPT_PEACE_PROPOSAL,
        ALLIES_PROPOSAL,
        ACCEPT_ALLIES_PROPOSAL,
        END_ALLIANCE_DECLARATION,
        CANCEL_PROPOSAL,
        REJECT_PROPOSAL
    };

    DiplomaticMessage();
    DiplomaticMessage(int sender_empire_id, int recipient_empire_id, DiplomaticMessageType type);

    DiplomaticMessageType   GetType() const { return m_type; }
    int                     SenderEmpireID() const { return m_sender_empire; }
    int                     RecipientEmpireID() const { return m_recipient_empire; }
    std::string             Dump() const;

private:
    int                     m_sender_empire;
    int                     m_recipient_empire;
    DiplomaticMessageType   m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

struct FO_COMMON_API DiplomaticStatusUpdateInfo {
    DiplomaticStatusUpdateInfo();
    DiplomaticStatusUpdateInfo(int empire1_id_, int empire2_id_, DiplomaticStatus status);
    int                 empire1_id;
    int                 empire2_id;
    DiplomaticStatus    diplo_status;
};

bool operator==(const DiplomaticMessage& lhs, const DiplomaticMessage& rhs);
bool operator!=(const DiplomaticMessage& lhs, const DiplomaticMessage& rhs);

FO_COMMON_API DiplomaticMessage WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
FO_COMMON_API DiplomaticMessage PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
FO_COMMON_API DiplomaticMessage AcceptPeaceDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
FO_COMMON_API DiplomaticMessage AlliesProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
FO_COMMON_API DiplomaticMessage AcceptAlliesDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
FO_COMMON_API DiplomaticMessage EndAllianceDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
FO_COMMON_API DiplomaticMessage CancelDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
FO_COMMON_API DiplomaticMessage RejectProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);

#endif // _Diplomacy_h_
