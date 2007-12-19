#include "ShipDesign.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

////////////////////////////////////////////////
// PartType
////////////////////////////////////////////////
PartType::PartType() :
    m_name("generic part type")
{}

////////////////////////////////////////////////
// HullType
////////////////////////////////////////////////
HullType::HullType() :
    m_name("generic hull type")
{}

////////////////////////////////////////////////
// Free Function
////////////////////////////////////////////////
const ShipDesign* GetShipDesign(int ship_design_id)
{
    return GetUniverse().GetShipDesign(ship_design_id);
}

////////////////////////////////////////////////
// ShipDesign
////////////////////////////////////////////////
ShipDesign::ShipDesign() :
    m_id(UniverseObject::INVALID_OBJECT_ID),
    m_name(""),
    m_designed_by_empire_id(-1),
    m_designed_on_turn(UniverseObject::INVALID_OBJECT_AGE),
    m_hull(""),
    m_parts(),
    m_graphic(""),
    m_3D_model("")
{}

ShipDesign::ShipDesign(int id, std::string name, int designed_by_empire_id, int designed_on_turn, 
                       std::string hull, std::vector<std::string> parts, std::string graphic,
                       std::string model) :
    m_id(id),
    m_name(name),
    m_designed_by_empire_id(designed_by_empire_id),
    m_designed_on_turn(designed_on_turn),
    m_hull(hull),
    m_parts(parts),
    m_graphic(graphic),
    m_3D_model(model)
{}

std::string ShipDesign::Name() const
{
    return m_name;
}
    
int ShipDesign::DesignedByEmpire() const
{
    return m_designed_by_empire_id;
}

void ShipDesign::SetID(int id)
{
    // TODO: figure out if this requires a bunch of adjustments to ships using this design
    m_id = id;
}

void ShipDesign::Rename(const std::string& name)
{
    m_name = name;
}

std::string ShipDesign::Graphic() const
{
    if (m_name == "Scout")
        return "misc/scout1.png";
    if (m_name == "Colony Ship")
        return "misc/colony1.png";
    if (m_name == "Mark I")
        return "misc/mark1.png";
    if (m_name == "Mark II")
        return "misc/mark2.png";
    if (m_name == "Mark III")
        return "misc/mark3.png";
    if (m_name == "Mark IV")
        return "misc/mark4.png";
    return "";
}

std::string ShipDesign::Description() const
{
    if (m_name == "Scout")
        return "Small and cheap unarmed vessel designed for recon and exploration.";
    if (m_name == "Colony Ship")
        return "Huge unarmed vessel capable of delivering millions of citizens safely to new colony sites.";
    if (m_name == "Mark I")
        return "Affordable armed patrol frigate.";
    if (m_name == "Mark II")
        return "Cruiser with storng defensive and offensive capabilities.";
    if (m_name == "Mark III")
        return "Advanced cruiser with heavy weaponry and armor to do the dirty work.";
    if (m_name == "Mark IV")
        return "Massive state-of-art warship armed and protected with the latest technolgy. Priced accordingly.";
    return "A nonspecific ship";
}

double ShipDesign::Defense() const
{
    if (m_name == "Mark I")
        return 1.0;
    if (m_name == "Mark II")
        return 2.0;
    if (m_name == "Mark III")
        return 3.0;
    if (m_name == "Mark IV")
        return 5.0;
    return 1.0;
}

double ShipDesign::Speed() const
{
    if (m_name == "Scout")
        return 80.0;
    if (m_name == "Colony Ship")
        return 50.0;
    if (m_name == "Mark I")
        return 50.0;
    if (m_name == "Mark II")
        return 40.0;
    if (m_name == "Mark III")
        return 30.0;
    if (m_name == "Mark IV")
        return 25.0;
    return 50.0;
}

double ShipDesign::Attack() const
{
    if (m_name == "Mark I")
        return 2.0;
    if (m_name == "Mark II")
        return 5.0;
    if (m_name == "Mark III")
        return 10.0;
    if (m_name == "Mark IV")
        return 15.0;
    return 0.0;
}

bool ShipDesign::Colonize() const
{
    if (m_name == "Colony Ship")
        return true;    
    return false;
}

double ShipDesign::Cost() const
{
    if (m_name == "Scout")
        return 10.0;
    if (m_name == "Colony Ship")
        return 50.0;
    if (m_name == "Mark I")
        return 20.0;
    if (m_name == "Mark II")
        return 40.0;
    if (m_name == "Mark III")
        return 60.0;
    if (m_name == "Mark IV")
        return 80.0;
    return 0;
}
