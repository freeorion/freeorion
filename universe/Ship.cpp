#include "Ship.h"

#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "Fleet.h"
#include "Predicates.h"
#include "ShipDesign.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

namespace {
    void GrowFuelMeter(Meter* fuel_meter) {
        assert(fuel_meter);
        fuel_meter->AdjustCurrent(0.1001);
    }

    /** returns true iff one of the empires with the indiated ids can provide
      * fleet supply directly or has resource connections to the system with
      * the id \a system_id 
      * in short: decides whether a fleet gets resupplied at the indicated
      *           system*/
    bool FleetOrResourceSupplyableAtSystemByAnyOfEmpiresWithIDs(int system_id, const std::set<int>& owner_ids) {
        for (std::set<int>::const_iterator it = owner_ids.begin(); it != owner_ids.end(); ++it)
            if (const Empire* empire = Empires().Lookup(*it))
                if (empire->FleetOrResourceSupplyableAtSystem(system_id))
                    return true;
        return false;
    }
}

Ship::Ship() :
    m_design_id(INVALID_OBJECT_ID),
    m_fleet_id(INVALID_OBJECT_ID),
    m_ordered_scrapped(false)
{}

Ship::Ship(int empire_id, int design_id) :
    m_design_id(design_id),
    m_fleet_id(INVALID_OBJECT_ID),
    m_ordered_scrapped(false)
{
    if (!GetShipDesign(design_id))
        throw std::invalid_argument("Attempted to construct a Ship with an invalid design id");

    AddOwner(empire_id);

    UniverseObject::Init();

    InsertMeter(METER_FUEL,         Meter());
    InsertMeter(METER_SHIELD,       Meter());
    InsertMeter(METER_DETECTION,    Meter());
    InsertMeter(METER_HEALTH,       Meter());

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

Ship* Ship::Clone(int empire_id) const
{
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return 0;

    Ship* retval = new Ship();
    retval->Copy(this, empire_id);
    return retval;
}

void Ship::Copy(const UniverseObject* copied_object, int empire_id)
{
    if (copied_object == this)
        return;
    const Ship* copied_ship = universe_object_cast<Ship*>(copied_object);
    if (!copied_ship) {
        Logger().errorStream() << "Ship::Copy passed an object that wasn't a Ship";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis);

    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_fleet_id =                  copied_ship->m_fleet_id;

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            this->m_design_id =             copied_ship->m_design_id;
            this->m_fighters =              copied_ship->m_fighters;
            this->m_missiles =              copied_ship->m_missiles;

            if (vis >= VIS_FULL_VISIBILITY) {
                this->m_ordered_scrapped =  copied_ship->m_ordered_scrapped;
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

bool Ship::IsArmed() const {
    const ShipDesign* design = Design();
    if (design)
        return design->IsArmed();
    else
        return false;
}

bool Ship::CanColonize() const {
    const ShipDesign* design = Design();
    if (design)
        return design->CanColonize();
    else
        return false;
}

double Ship::Speed() const {
    const ShipDesign* design = Design();
    if (design)
        return design->StarlaneSpeed();
    else
        return false;
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
    const ObjectMap& objects = GetMainObjectMap();
    const Meter* original_meter = GetMeter(type);
    if (!original_meter) {
        Logger().errorStream() << "Ship::ProjectedeCurrentMeter couldn't get meter of type " << boost::lexical_cast<std::string>(type);
        return 0.0;
    }
    Meter meter = Meter(*original_meter);
    const Fleet* fleet = objects.Object<Fleet>(this->FleetID());
    if (!fleet) {
        Logger().errorStream() << "Ship::ProjectedeCurrentMeter couldn't get fleet with id " << this->FleetID();
        return 0.0;
    }

    switch (type) {
    case METER_FUEL:
        if (FleetOrResourceSupplyableAtSystemByAnyOfEmpiresWithIDs(fleet->SystemID(), fleet->Owners())) {
            // fleets at systems where they can be supplied are fully resupplied
            meter.SetCurrent(meter.Max());
        } else if (fleet->FinalDestinationID() == UniverseObject::INVALID_OBJECT_ID ||
                   fleet->FinalDestinationID() == fleet->SystemID())
        {
            // fleets that are stationary regenerate fuel slowly
            GrowFuelMeter(&meter);
        }
        meter.Clamp();
        return meter.Current();
        break;
    default:
        return UniverseObject::ProjectedCurrentMeter(type);
    }
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
    if (Fleet* fleet = GetObject<Fleet>(this->FleetID())) {
        //Logger().debugStream() << "Ship::MoveTo removing " << this->ID() << " from fleet " << fleet->Name();
        fleet->RemoveShip(this->ID());
    }
}

void Ship::SetOrderedScrapped(bool b)
{
    bool initial_status = m_ordered_scrapped;
    if (b == initial_status) return;
    m_ordered_scrapped = b;
    StateChangedSignal();
}