#include "Ship.h"

#include "../util/AppInterface.h"
#include "Fleet.h"
#include "ShipDesign.h"
#include "XMLDoc.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;
#include <stdexcept>

////////////////////////////////////////////////
// Ship
////////////////////////////////////////////////
Ship::Ship()
{
}

Ship::Ship(int empire_id, const std::string& design_name) :
    m_design_name(design_name)
{
   if (!GetShipDesign(empire_id, m_design_name))
      throw std::invalid_argument("Attempted to construct a Ship with an invalid design name");

   AddOwner(empire_id);
}

Ship::Ship(const GG::XMLElement& elem) : 
  UniverseObject(elem.Child("UniverseObject"))
{
    if (elem.Tag().find("Ship") == std::string::npos)
        throw std::invalid_argument("Attempted to construct a Ship from an XMLElement that had a tag other than \"Ship\"");

    try {
        m_fleet_id = lexical_cast<int>(elem.Child("m_fleet_id").Text());
        m_design_name = elem.Child("m_design_name").Text();
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in Ship::Ship(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
}

const ShipDesign* Ship::Design() const
{
    return GetShipDesign(*Owners().begin(), m_design_name);
}

int Ship::FleetID() const
{
    return m_fleet_id;
}

Fleet* Ship::GetFleet() const
{
    return m_fleet_id == INVALID_OBJECT_ID ? 0 : GetUniverse().Object<Fleet>(m_fleet_id);
}

UniverseObject::Visibility Ship::GetVisibility(int empire_id) const
{
    UniverseObject::Visibility vis = NO_VISIBILITY;

    if (empire_id == Universe::ALL_EMPIRES || OwnedBy(empire_id))
        vis = FULL_VISIBILITY;
    else
        vis = PARTIAL_VISIBILITY; // TODO: do something smarter here, such as a range check vs. owned systems and fleets

    // Ship is visible if its fleet is visible
    return FleetID() == INVALID_OBJECT_ID ? NO_VISIBILITY : (GetFleet()?GetFleet()->GetVisibility(empire_id):vis);
}

bool Ship::IsArmed() const
{
    return Design()->attack > 0;
}

GG::XMLElement Ship::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
    using GG::XMLElement;
    using boost::lexical_cast;
    using std::string;
    XMLElement retval("Ship" + boost::lexical_cast<std::string>(ID()));
    retval.AppendChild(UniverseObject::XMLEncode(empire_id));
    retval.AppendChild(XMLElement("m_fleet_id", lexical_cast<std::string>(m_fleet_id)));
    retval.AppendChild(XMLElement("m_design_name", m_design_name));
    return retval;
}
  	
void Ship::MovementPhase()
{
}

void Ship::PopGrowthProductionResearchPhase()
{
}
