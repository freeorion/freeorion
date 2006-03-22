#include "ShipDesign.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../util/XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() : 
    empire(-1),
    name(""),
    description(""),
    cost(10000000.0),
    speed(1.0),
    colonize(false),
    attack(0),
    defense(0),
    graphic("")
{
}

ShipDesign::ShipDesign(const XMLElement& elem)
{
    if (elem.Tag() != "ShipDesign" )
        throw std::invalid_argument("Attempted to construct a ShipDesign from an XMLElement that had a tag other than \"ShipDesign\"");

    empire = lexical_cast<int>(elem.Child("empire").Text());
    name = elem.Child("name").Text();
    description = elem.Child("description").Text();
    cost = lexical_cast<double>(elem.Child("cost").Text());
    speed = lexical_cast<double>(elem.Child("speed").Text());
    colonize = lexical_cast<bool>(elem.Child("colonize").Text());

    /////////////////////////////////////////////////////////////////////////////
    // V0.3 ONLY!!!!
    attack = lexical_cast<int>(elem.Child("attack").Text());
    defense = lexical_cast<int>(elem.Child("defense").Text());
    graphic = elem.Child("graphic").Text();
    // V0.3 ONLY!!!!
    /////////////////////////////////////////////////////////////////////////////
}

XMLElement ShipDesign::XMLEncode() const
{
    using boost::lexical_cast;

    XMLElement retval("ShipDesign");
    retval.AppendChild(XMLElement("empire", lexical_cast<std::string>(empire)));
    retval.AppendChild(XMLElement("name", name));
    retval.AppendChild(XMLElement("description", description));
    retval.AppendChild(XMLElement("cost", lexical_cast<std::string>(cost)));
    retval.AppendChild(XMLElement("speed", lexical_cast<std::string>(speed)));
    retval.AppendChild(XMLElement("colonize", lexical_cast<std::string>(colonize)));
    /////////////////////////////////////////////////////////////////////////////
    // V0.3 ONLY!!!!
    retval.AppendChild(XMLElement("attack", lexical_cast<std::string>(attack)));
    retval.AppendChild(XMLElement("defense", lexical_cast<std::string>(defense)));
    retval.AppendChild(XMLElement("graphic", graphic));
    // V0.3 ONLY!!!!
    /////////////////////////////////////////////////////////////////////////////
    return retval;
}

const ShipDesign* GetShipDesign(int empire_id, const std::string& name)
{
    Empire* empire = Empires().Lookup(empire_id);
    return empire->GetShipDesign(name);
}
