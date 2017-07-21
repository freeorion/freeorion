#include "CombatEvent.h"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include "../util/Serialize.ipp"
#include "../util/Serialize.h"
#include "../util/Logger.h"

#include <sstream>

CombatEvent::CombatEvent() {}

boost::optional<int> CombatEvent::PrincipalFaction(int viewing_empire_id) const {
    ErrorLogger() << "A combat logger expected this event to be associated with a faction: "
                  << this->DebugString();
    return boost::optional<int>();
}

template<typename Archive>
void CombatEvent::serialize(Archive& ar, const unsigned int version) {}

BOOST_CLASS_EXPORT_IMPLEMENT(CombatEvent)

template
void CombatEvent::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);

template
void CombatEvent::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);

template
void CombatEvent::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

template
void CombatEvent::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);
