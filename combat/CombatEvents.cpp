#include "CombatEvents.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include "../util/Serialize.ipp"
#include "../util/Serialize.h"
#include "../universe/Universe.h"

#include <sstream>


//////////////////////////////////////////
///////// BoutBeginEvent//////////////////
//////////////////////////////////////////
BoutBeginEvent::BoutBeginEvent() :
    bout(-1)
{}

BoutBeginEvent::BoutBeginEvent(int bout_) :
    bout(bout_)
{}

std::string BoutBeginEvent::DebugString() const {
    std::stringstream ss;
    ss << "Bout " << bout << " begins.";
    return ss.str();
}

template <class Archive>
void BoutBeginEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout);
}

BOOST_CLASS_EXPORT(BoutBeginEvent)

template
void BoutBeginEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void BoutBeginEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void BoutBeginEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void BoutBeginEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

//////////////////////////////////////////
///////// Attack Event////////////////////
//////////////////////////////////////////
AttackEvent::AttackEvent() :
    bout(-1),
    round(-1),
    attacker_id(INVALID_OBJECT_ID),
    target_id(INVALID_OBJECT_ID),
    damage(0.0f)
{}

AttackEvent::AttackEvent(int bout_, int round_, int attacker_id_, int target_id_, float damage_, int attacker_owner_id_) :
    bout(bout_),
    round(round_),
    attacker_id(attacker_id_),
    target_id( target_id_),
    damage(damage_),
    attacker_owner_id(attacker_owner_id_)
{}

std::string AttackEvent::DebugString() const {
    std::stringstream ss;
    ss << "rnd: " << round << " : "
       << attacker_id << " -> " << target_id << " : "
       << damage << "   attacker owner: " << attacker_owner_id;
    return ss.str();
}

template <class Archive>
void AttackEvent::serialize(Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(round)
       & BOOST_SERIALIZATION_NVP(attacker_id)
       & BOOST_SERIALIZATION_NVP(target_id)
       & BOOST_SERIALIZATION_NVP(damage)
       & BOOST_SERIALIZATION_NVP(attacker_owner_id);

    if (version < 3) {
        int target_destroyed = 0;
        ar & BOOST_SERIALIZATION_NVP (target_destroyed);
    }
}

BOOST_CLASS_VERSION(AttackEvent, 4)
BOOST_CLASS_EXPORT(AttackEvent)

template
void AttackEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void AttackEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void AttackEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void AttackEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

//////////////////////////////////////////
///////// IncapacitationEvent/////////////
//////////////////////////////////////////
IncapacitationEvent::IncapacitationEvent() :
    bout(-1),
    object_id(INVALID_OBJECT_ID)
{}

IncapacitationEvent::IncapacitationEvent(int bout_, int object_id_, int object_owner_id_) :
    bout(bout_),
    object_id(object_id_),
    object_owner_id(object_owner_id_)
{}

std::string IncapacitationEvent::DebugString() const {
    std::stringstream ss;
    ss << "incapacitation of " << object_id << " owned by " << object_owner_id << " at bout " << bout;
    return ss.str();
}

template <class Archive>
void IncapacitationEvent::serialize (Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(object_id)
       & BOOST_SERIALIZATION_NVP(object_owner_id);
}

BOOST_CLASS_EXPORT(IncapacitationEvent)

template
void IncapacitationEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void IncapacitationEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void IncapacitationEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void IncapacitationEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

//////////////////////////////////////////
///////// FighterAttackedEvent ////////
//////////////////////////////////////////
FighterAttackedEvent::FighterAttackedEvent() :
    bout(-1),
    round(-1),
    attacker_owner_empire_id(ALL_EMPIRES),
    attacked_by_object_id(INVALID_OBJECT_ID),
    attacked_owner_id(ALL_EMPIRES)
{}

FighterAttackedEvent::FighterAttackedEvent(int bout_, int round_, int attacked_by_object_id_,
                                                 int attacker_owner_empire_id_, int attacked_owner_id_) :
    bout(bout_),
    round(round_),
    attacker_owner_empire_id(attacker_owner_empire_id_),
    attacked_by_object_id(attacked_by_object_id_),
    attacked_owner_id(attacked_owner_id_)
{}

std::string FighterAttackedEvent::DebugString() const {
    std::stringstream ss;
    ss << "rnd: " << round << " : "
       << attacked_by_object_id << " -> (Fighter of Empire " << attacked_owner_id << ")"
       << " by object owned by empire " << attacker_owner_empire_id
       << " at bout " << bout << " round " << round;
    return ss.str();
}

template <class Archive>
void FighterAttackedEvent::serialize (Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(round)
       & BOOST_SERIALIZATION_NVP(attacker_owner_empire_id)
       & BOOST_SERIALIZATION_NVP(attacked_by_object_id)
       & BOOST_SERIALIZATION_NVP(attacked_owner_id);
}

BOOST_CLASS_EXPORT(FighterAttackedEvent)

template
void FighterAttackedEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void FighterAttackedEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void FighterAttackedEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void FighterAttackedEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

//////////////////////////////////////////
/////////// FighterLaunchEvent ///////////
//////////////////////////////////////////
FighterLaunchEvent::FighterLaunchEvent() :
    bout(-1),
    fighter_owner_empire_id(ALL_EMPIRES),
    launched_from_id(INVALID_OBJECT_ID),
    number_launched(0)
{}

FighterLaunchEvent::FighterLaunchEvent(int bout_, int launched_from_id_, int fighter_owner_empire_id_, int number_launched_) :
    bout(bout_),
    fighter_owner_empire_id(fighter_owner_empire_id_),
    launched_from_id(launched_from_id_),
    number_launched(number_launched_)
{}

std::string FighterLaunchEvent::DebugString() const {
    std::stringstream ss;
    ss << "launch from object " << launched_from_id
       << " of " << number_launched
       << " fighter(s) of empire " << fighter_owner_empire_id
       << " at bout " << bout;
    return ss.str();
}

template <class Archive>
void FighterLaunchEvent::serialize (Archive& ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP(bout)
       & BOOST_SERIALIZATION_NVP(fighter_owner_empire_id)
       & BOOST_SERIALIZATION_NVP(launched_from_id)
       & BOOST_SERIALIZATION_NVP(number_launched);
}

BOOST_CLASS_EXPORT(FighterLaunchEvent)

template
void FighterLaunchEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void FighterLaunchEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void FighterLaunchEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);

template
void FighterLaunchEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);




