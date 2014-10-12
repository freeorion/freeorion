#include "CombatEvent.h"

#include "CombatEvents.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include "../util/Serialize.ipp"
#include "../util/Serialize.h"
#include "../universe/Universe.h"

#include <sstream>

CombatEvent::CombatEvent() {}

template<typename Archive>
void CombatEvent::serialize ( Archive& ar, const unsigned int version ) {}

BOOST_CLASS_EXPORT_IMPLEMENT ( CombatEvent )

template
void CombatEvent::serialize<freeorion_bin_iarchive> ( freeorion_bin_iarchive& ar, const unsigned int version );
template
void CombatEvent::serialize<freeorion_bin_oarchive> ( freeorion_bin_oarchive& ar, const unsigned int version );
template
void CombatEvent::serialize<freeorion_xml_iarchive> ( freeorion_xml_iarchive& ar, const unsigned int version );
template
void CombatEvent::serialize<freeorion_xml_oarchive> ( freeorion_xml_oarchive& ar, const unsigned int version );
