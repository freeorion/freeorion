// -*- C++ -*-
#ifndef _Diplomacy_h_
#define _Diplomacy_h_

#include <string>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

class DiplomaticMessage {
public:
    enum DiplomaticMessageType {
        INVALID_DIPLOMATIC_MESSAGE_TYPE = -1,
        WAR_DECLARATION,
        PEACE_PROPOSAL,
        ACCEPT_PROPOSAL,
        CANCEL_PROPOSAL,
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

bool operator==(const DiplomaticMessage& lhs, const DiplomaticMessage& rhs);
bool operator!=(const DiplomaticMessage& lhs, const DiplomaticMessage& rhs);

DiplomaticMessage WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
DiplomaticMessage PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
DiplomaticMessage AcceptDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
DiplomaticMessage CancelDiplomaticMessage(int sender_empire_id, int recipient_empire_id);

#endif // _Diplomacy_h_
