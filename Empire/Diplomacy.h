// -*- C++ -*-
#ifndef _Diplomacy_h_
#define _Diplomacy_h_

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

class DiplomaticMessage {
public:
    enum DiplomaticMessageType {
        INVALID_DIPLOMATIC_MESSAGE_TYPE = -1,
        WAR_DECLARATION,
        PEACE_PROPOSAL,
        PEACE_ACCEPT
    };

    DiplomaticMessage();
    DiplomaticMessage(int sender_empire_id, int recipient_empire_id, DiplomaticMessageType type);

private:
    int                     m_sender_empire;
    int                     m_recipient_empire;
    DiplomaticMessageType   m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class WarDeclarationDiplomaticMessage : public DiplomaticMessage {
public:
    WarDeclarationDiplomaticMessage();
    WarDeclarationDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class PeaceProposalDiplomaticMessage : public DiplomaticMessage {
public:
    PeaceProposalDiplomaticMessage();
    PeaceProposalDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class PeaceAcceptanceDiplomaticMessage : public DiplomaticMessage {
public:
    PeaceAcceptanceDiplomaticMessage();
    PeaceAcceptanceDiplomaticMessage(int sender_empire_id, int recipient_empire_id);
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif // _Diplomacy_h_
