#include "Ship.h"

#include "../util/AppInterface.h"
#include "Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "Predicates.h"
#include "ShipDesign.h"


Ship::Ship() :
    m_design_id(INVALID_OBJECT_ID),
    m_fleet_id(INVALID_OBJECT_ID)
{}

Ship::Ship(int empire_id, int design_id) :
    m_design_id(design_id),
    m_fleet_id(INVALID_OBJECT_ID)
{
    if (!GetShipDesign(design_id))
        throw std::invalid_argument("Attempted to construct a Ship with an invalid design id");

    AddOwner(empire_id);

    InsertMeter(METER_FUEL, Meter());
    InsertMeter(METER_SHIELD, Meter());
    InsertMeter(METER_DETECTION, Meter());
    InsertMeter(METER_STEALTH, Meter());
    InsertMeter(METER_HEALTH, Meter());

    const std::vector<std::string>& part_names = Design()->Parts();
    for (std::size_t i = 0; i < part_names.size(); ++i) {
        if (part_names[i] != "") {
            const PartType* part = GetPartType(part_names[i]);
            assert(part);
            if (part->Class() == PC_FIGHTERS) {
                std::pair<std::size_t, std::size_t>& part_fighters =
                    m_fighters[part_names[i]];
                ++part_fighters.first;
                part_fighters.second += boost::get<FighterStats>(part->Stats()).m_capacity;
            } else if (part->Class() == PC_MISSILES) {
                std::pair<std::size_t, std::size_t>& part_missiles =
                    m_missiles[part_names[i]];
                ++part_missiles.first;
                part_missiles.second += boost::get<LRStats>(part->Stats()).m_capacity;
            }
        }
    }
}

const ShipDesign* Ship::Design() const {
    return GetShipDesign(m_design_id);
}

int Ship::DesignID() const {
    return m_design_id;
}

int Ship::FleetID() const {
    return m_fleet_id;
}

Fleet* Ship::GetFleet() const {
    return m_fleet_id == INVALID_OBJECT_ID ? 0 : GetUniverse().Object<Fleet>(m_fleet_id);
}

Visibility Ship::GetVisibility(int empire_id) const {
    Visibility vis = VIS_NO_VISIBITY;

    if (Universe::ALL_OBJECTS_VISIBLE || empire_id == ALL_EMPIRES || OwnedBy(empire_id))
        vis = VIS_FULL_VISIBILITY;

    // Ship is visible if its fleet is visible
    Visibility retval = FleetID() == INVALID_OBJECT_ID ? VIS_NO_VISIBITY : (GetFleet() ? GetFleet()->GetVisibility(empire_id) : vis);
    return retval;
}

bool Ship::IsArmed() const {
    return Design()->IsArmed();
}

bool Ship::CanColonize() const {
    return Design()->CanColonize();
}

double Ship::Speed() const {
    return Design()->Speed();
}

const Ship::ConsumablesMap& Ship::Fighters() const {
    return m_fighters;
}

const Ship::ConsumablesMap& Ship::Missiles() const {
    return m_missiles;
}

const std::string& Ship::PublicName(int empire_id) const {
    // Disclose real ship name only to fleet owners. Rationale: a player who doesn't know the design for a particular
    // ship can easily guess it if the ship's name is "Scout"
    if (Universe::ALL_OBJECTS_VISIBLE || empire_id == ALL_EMPIRES || OwnedBy(empire_id))
        return Name();
    else
        return UserString("FW_FOREIGN_SHIP");
}

UniverseObject* Ship::Accept(const UniverseObjectVisitor& visitor) const {
    return visitor.Visit(const_cast<Ship* const>(this));
}

double Ship::ProjectedCurrentMeter(MeterType type) const {
    return UniverseObject::ProjectedCurrentMeter(type);
}

void Ship::SetFleetID(int fleet_id)
{
    m_fleet_id = fleet_id;
    StateChangedSignal();
}

void Ship::Resupply()
{
    Meter* meter = GetMeter(METER_FUEL);
    assert(meter);
    meter->SetCurrent(meter->Max());
    for (ConsumablesMap::iterator it = m_fighters.begin();
         it != m_fighters.end();
         ++it) {
        it->second.second =
            it->second.first *
            boost::get<FighterStats>(GetPartType(it->first)->Stats()).m_capacity;
    }
    for (ConsumablesMap::iterator it = m_missiles.begin();
         it != m_missiles.end();
         ++it) {
        it->second.second =
            it->second.first *
            boost::get<LRStats>(GetPartType(it->first)->Stats()).m_capacity;
    }
}

void Ship::AddFighters(const std::string& part_name, std::size_t n)
{
    assert(m_fighters[part_name].second + n <=
           m_fighters[part_name].first *
           boost::get<FighterStats>(GetPartType(part_name)->Stats()).m_capacity);
    m_fighters[part_name].second += n;
}

void Ship::RemoveFighters(const std::string& part_name, std::size_t n)
{
    assert(m_fighters[part_name].second < n);
    m_fighters[part_name].second -= n;
}

void Ship::RemoveMissiles(const std::string& part_name, std::size_t n)
{
    assert(m_missiles[part_name].second < n);
    m_missiles[part_name].second -= n;
}

void Ship::MoveTo(double x, double y)
{
    UniverseObject::MoveTo(x, y);

    // if ship is being moved away from its fleet, remove from the fleet.  otherwise, keep ship in fleet.
    if (Fleet* fleet = GetFleet()) {
        Logger().debugStream() << "Ship::MoveTo removing " << this->ID() << " from fleet " << fleet->Name();
        fleet->RemoveShip(this->ID());
    }
}

void Ship::MovementPhase() {
    // Fleet::MovementPhase moves ships within fleet around and deals with ship fuel consumption
}

void Ship::PopGrowthProductionResearchPhase() {
}
