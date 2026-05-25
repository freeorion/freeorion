#ifndef _Diplomacy_h_
#define _Diplomacy_h_

#include <compare>
#include <cstdint>
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

    constexpr DiplomaticMessage() noexcept = default;
    constexpr DiplomaticMessage(EmpireID sender_empire_id, EmpireID recipient_empire_id, Type type) noexcept :
        m_sender_empire(sender_empire_id),
        m_recipient_empire(recipient_empire_id),
        m_type(type)
    {}

    [[nodiscard]] constexpr auto GetType() const noexcept { return m_type; }
    [[nodiscard]] constexpr auto SenderEmpireID() const noexcept { return m_sender_empire; }
    [[nodiscard]] constexpr auto RecipientEmpireID() const noexcept { return m_recipient_empire; }

    [[nodiscard]] std::string    Dump() const;
    [[nodiscard]] bool           IsAllowed() const; ///< Tells if this dimplomatic message allowed by game rules

    constexpr auto operator<=>(const DiplomaticMessage&) const noexcept = default;

private:
    EmpireID m_sender_empire = ALL_EMPIRES;
    EmpireID m_recipient_empire = ALL_EMPIRES;
    Type     m_type = Type::INVALID;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

struct FO_COMMON_API DiplomaticStatusUpdateInfo {
    constexpr DiplomaticStatusUpdateInfo() noexcept = default;
    constexpr DiplomaticStatusUpdateInfo(EmpireID empire1_id_, EmpireID empire2_id_, DiplomaticStatus status) noexcept :
        empire1_id(empire1_id_),
        empire2_id(empire2_id_),
        diplo_status(status)
    {}
    constexpr auto operator<=>(const DiplomaticStatusUpdateInfo&) const noexcept = default;
    EmpireID            empire1_id = ALL_EMPIRES;
    EmpireID            empire2_id = ALL_EMPIRES;
    DiplomaticStatus    diplo_status = DiplomaticStatus::INVALID_DIPLOMATIC_STATUS;
};

[[nodiscard]] FO_COMMON_API DiplomaticMessage WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage AcceptPeaceDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage AlliesProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage AcceptAlliesDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage EndAllianceDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage CancelDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
[[nodiscard]] FO_COMMON_API DiplomaticMessage RejectProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);


#endif
