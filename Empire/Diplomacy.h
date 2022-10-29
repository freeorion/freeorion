#ifndef _Diplomacy_h_
#define _Diplomacy_h_

#include <string>
#include <boost/serialization/access.hpp>
#include "../util/Enum.h"
#include "../util/Export.h"
#include "../universe/ConstantsFwd.h"


//! diplomatic statuses
FO_ENUM(
    (DiplomaticStatus),
    ((INVALID_DIPLOMATIC_STATUS, -1))
    ((DIPLO_WAR))
    ((DIPLO_PEACE))
    ((DIPLO_ALLIED))
    ((NUM_DIPLO_STATUSES))
)


class FO_COMMON_API DiplomaticMessage {
public:
    enum class Type : int8_t {
        INVALID = -1,
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
    DiplomaticMessage(int sender_empire_id, int recipient_empire_id, Type type);

    [[nodiscard]] auto GetType() const noexcept { return m_type; }

    [[nodiscard]] int           SenderEmpireID() const noexcept { return m_sender_empire; }
    [[nodiscard]] int           RecipientEmpireID() const noexcept { return m_recipient_empire; }
    [[nodiscard]] std::string   Dump() const;
    [[nodiscard]] bool          IsAllowed() const; ///< Tells if this dimplomatic message allowed by game rules

private:
    int  m_sender_empire = ALL_EMPIRES;
    int  m_recipient_empire = ALL_EMPIRES;
    Type m_type = Type::INVALID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

struct FO_COMMON_API DiplomaticStatusUpdateInfo {
    DiplomaticStatusUpdateInfo();
    DiplomaticStatusUpdateInfo(int empire1_id_, int empire2_id_, DiplomaticStatus status);
    int                 empire1_id = ALL_EMPIRES;
    int                 empire2_id = ALL_EMPIRES;
    DiplomaticStatus    diplo_status = DiplomaticStatus::INVALID_DIPLOMATIC_STATUS;
};

bool operator==(const DiplomaticMessage& lhs, const DiplomaticMessage& rhs);
bool operator!=(const DiplomaticMessage& lhs, const DiplomaticMessage& rhs);

[[nodiscard]] FO_COMMON_API DiplomaticMessage WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage AcceptPeaceDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage AlliesProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage AcceptAlliesDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage EndAllianceDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage CancelDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage RejectProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);


#endif
