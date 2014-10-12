#include "CombatEvents.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include "../util/Serialize.ipp"
#include "../util/Serialize.h"
#include "../universe/Universe.h"

#include <sstream>


//////////////////////////////////////////
///////// BoutBeginEvent////////////////////
//////////////////////////////////////////

BoutBeginEvent::BoutBeginEvent () :
bout ( -1 )
{}

BoutBeginEvent::BoutBeginEvent ( int bout_ ) :
bout ( bout_ )
{}

std::string BoutBeginEvent::DebugString() const {
    std::stringstream ss;
    ss << "Bout " << bout << " begins.";
    return ss.str();
}

template <class Archive>
void BoutBeginEvent::serialize ( Archive& ar, const unsigned int version ) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP ( bout );
}

BOOST_CLASS_EXPORT ( BoutBeginEvent )

template
void BoutBeginEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version );
template
void BoutBeginEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version );
template
void BoutBeginEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version );
template
void BoutBeginEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version );

//////////////////////////////////////////
///////// Attack Event////////////////////
//////////////////////////////////////////

AttackEvent::AttackEvent() :
bout ( -1 ),
     round ( -1 ),
     attacker_id ( INVALID_OBJECT_ID ),
     target_id ( INVALID_OBJECT_ID ),
     damage ( 0.0f )
{}



AttackEvent::AttackEvent ( int bout_, int round_, int attacker_id_, int target_id_, float damage_ ) :
bout ( bout_ ),
round ( round_ ),
attacker_id ( attacker_id_ ),
target_id ( target_id_ ),
damage ( damage_ )
{}

std::string AttackEvent::DebugString() const {
    std::stringstream ss;
    ss << "rnd: " << round << " : "
       << attacker_id << " -> " << target_id << " : "
       << damage;
    return ss.str();
}

template <class Archive>
void AttackEvent::serialize ( Archive& ar, const unsigned int version ) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP ( bout )
    & BOOST_SERIALIZATION_NVP ( round )
    & BOOST_SERIALIZATION_NVP ( attacker_id )
    & BOOST_SERIALIZATION_NVP ( target_id )
    & BOOST_SERIALIZATION_NVP ( damage );
    if ( version < 3 ) {
        int target_destroyed = 0;
        ar & BOOST_SERIALIZATION_NVP ( target_destroyed );
    }
}

BOOST_CLASS_VERSION ( AttackEvent, 3 )
BOOST_CLASS_EXPORT ( AttackEvent )

template
void AttackEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version );
template
void AttackEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version );
template
void AttackEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version );
template
void AttackEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version );

//////////////////////////////////////////
///////// IncapacitationEvent////////////////////
//////////////////////////////////////////

IncapacitationEvent::IncapacitationEvent () :
bout ( -1 ),
     object_id ( INVALID_OBJECT_ID )
{}

IncapacitationEvent::IncapacitationEvent ( int bout_, int object_id_ ) :
bout ( bout_ ),
object_id ( object_id_ )
{}

std::string IncapacitationEvent::DebugString() const {
    std::stringstream ss;
    ss << "Incapacitation of " << object_id << " at bout " << bout;
    return ss.str();
}

template <class Archive>
void IncapacitationEvent::serialize ( Archive& ar, const unsigned int version ) {
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatEvent);
    ar & BOOST_SERIALIZATION_NVP ( bout )
    & BOOST_SERIALIZATION_NVP ( object_id );
}

BOOST_CLASS_EXPORT ( IncapacitationEvent )

template
void IncapacitationEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version );
template
void IncapacitationEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version );
template
void IncapacitationEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version );
template
void IncapacitationEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version );
