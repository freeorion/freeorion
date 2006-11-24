#include "ShipDesign.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

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
    speed(50.0),
    colonize(false),
    attack(0),
    defense(0),
    graphic("")
{
}

const ShipDesign* GetShipDesign(int empire_id, const std::string& name)
{
    Empire* empire = Empires().Lookup(empire_id);
    return empire->GetShipDesign(name);
}
