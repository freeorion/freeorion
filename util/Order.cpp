#include "Order.h"

#include <fstream>
#include <vector>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "AppInterface.h"
#include "i18n.h"
#include "Logger.h"
#include "OrderSet.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Pathfinder.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Ship.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"


/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
std::shared_ptr<Empire> Order::GetValidatedEmpire(ScriptingContext& context) const {
    auto empire = context.GetEmpire(EmpireID());
    if (!empire)
        throw std::runtime_error("Invalid empire ID specified for order.");
    return empire;
}

void Order::Execute(ScriptingContext& context) const {
    if (m_executed)
        return;
    m_executed = true;

    ExecuteImpl(context);
}

bool Order::Undo(ScriptingContext& context) const {
    auto undone =  UndoImpl(context);
    if (undone)
        m_executed = false;
    return undone;
}

namespace {
    const std::string EMPTY_STRING;
    const std::string& ExecutedTag(const Order* order) {
        if (order && !order->Executed())
            return UserString("ORDER_UNEXECUTED");
        return EMPTY_STRING;
    }
}

////////////////////////////////////////////////
// RenameOrder
////////////////////////////////////////////////
RenameOrder::RenameOrder(int empire, int object, const std::string& name,
                         const ScriptingContext& context) :
    Order(empire),
    m_object(object),
    m_name(name)
{
    if (!Check(empire, object, name, context)) {
        m_object = INVALID_OBJECT_ID;
        return;
    }
}

std::string RenameOrder::Dump() const
{ return boost::io::str(FlexibleFormat(UserString("ORDER_RENAME")) % m_object % m_name) + ExecutedTag(this); }

bool RenameOrder::Check(int empire, int object, const std::string& new_name,
                        const ScriptingContext& context)
{
    // disallow the name "", since that denotes an unknown object
    if (new_name.empty()) {
        ErrorLogger() << "RenameOrder::Check() : passed an empty new_name.";
        return false;
    }

    auto obj = context.ContextObjects().get(object);

    if (!obj) {
        ErrorLogger() << "RenameOrder::Check() : passed an invalid object.";
        return false;
    }

    // verify that empire specified in order owns specified object
    if (!obj->OwnedBy(empire)) {
        ErrorLogger() << "RenameOrder::Check() : Object " << object << " is"
                      << " not owned by empire " << empire << ".";
        return false;
    }

    if (obj->Name() == new_name) {
        ErrorLogger() << "RenameOrder::Check() : Object " << object
                      << " should renamed to the same name.";
        return false;
    }

    return true;
}

void RenameOrder::ExecuteImpl(ScriptingContext& context) const {
    if (!Check(EmpireID(), m_object, m_name, context))
        return;

    GetValidatedEmpire(context);

    auto obj = context.ContextObjects().get(m_object);

    obj->Rename(m_name);
}

////////////////////////////////////////////////
// CreateFleetOrder
////////////////////////////////////////////////
NewFleetOrder::NewFleetOrder(int empire, std::string fleet_name,
                             std::vector<int> ship_ids,
                             const ScriptingContext& context,
                             bool aggressive, bool passive, bool defensive) :
    NewFleetOrder(empire, std::move(fleet_name), std::move(ship_ids),
                  aggressive ? FleetAggression::FLEET_AGGRESSIVE :
                  defensive ? FleetAggression::FLEET_DEFENSIVE :
                  passive ? FleetAggression::FLEET_PASSIVE :
                  FleetAggression::FLEET_OBSTRUCTIVE, 
                  context)
{}

NewFleetOrder::NewFleetOrder(int empire, std::string fleet_name,
                             std::vector<int> ship_ids, FleetAggression aggression,
                             const ScriptingContext& context) :
    Order(empire),
    m_fleet_name(std::move(fleet_name)),
    m_fleet_id(INVALID_OBJECT_ID),
    m_ship_ids(std::move(ship_ids)),
    m_aggression(aggression)
{
    if (!Check(empire, m_fleet_name, m_ship_ids, m_aggression, context))
        return;
}

bool NewFleetOrder::Aggressive() const
{ return m_aggression == FleetAggression::FLEET_AGGRESSIVE; }

std::string NewFleetOrder::Dump() const {
    const std::string& aggression_text =
        m_aggression == FleetAggression::FLEET_AGGRESSIVE ? UserString("FLEET_AGGRESSIVE") :
        m_aggression == FleetAggression::FLEET_OBSTRUCTIVE ? UserString("FLEET_OBSTRUCTIVE") :
        m_aggression == FleetAggression::FLEET_DEFENSIVE ? UserString("FLEET_DEFENSIVE") :
        m_aggression == FleetAggression::FLEET_PASSIVE ? UserString("FLEET_PASSIVE") :
        UserString("INVALID_FLEET_AGGRESSION");

    return boost::io::str(FlexibleFormat(UserString("ORDER_FLEET_NEW"))
                          % m_fleet_name
                          % std::to_string(m_ship_ids.size())
                          % aggression_text)
        + ExecutedTag(this);
}

bool NewFleetOrder::Check(int empire, const std::string& fleet_name, const std::vector<int>& ship_ids,
                          FleetAggression aggression, const ScriptingContext& context)
{
    if (ship_ids.empty()) {
        ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name << ") without ships";
        return false;
    }

    int system_id = INVALID_OBJECT_ID;

    std::set<int> arrival_starlane_ids;
    for (const auto& ship : context.ContextObjects().find<Ship>(ship_ids)) {
        if (!ship) {
            ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                          << ") with an invalid ship";
            return false;
        }

        auto ship_fleet = context.ContextObjects().get<Fleet>(ship->FleetID());
        if (!ship_fleet)
            continue;   // OK
        auto arr_lane = ship_fleet->ArrivalStarlane();
        if (arr_lane == INVALID_OBJECT_ID) // newly produced fleets have invalid object ID as arrival lane for one turn. this lets them be merged with fleets that have had their arrival lane set to their system
            arr_lane = ship_fleet->SystemID();
        arrival_starlane_ids.insert(arr_lane);
    }
    if (arrival_starlane_ids.size() > 1) {
        ErrorLogger() << "Empire " << empire << " attempted to create a new fleet with ships from multiple arrival starlanes";
        return false;
    }


    for (const auto& ship : context.ContextObjects().find<Ship>(ship_ids)) {
        // verify that empire is not trying to take ships from somebody else's fleet
        if (!ship->OwnedBy(empire)) {
            ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                          << ") with ships from another's (" << ship->Owner() << ") fleet.";
            return false;
        }
        if (ship->SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                          << ") with ship (" << ship->ID() << ") not in a system";
            return false;
        }

        if (system_id == INVALID_OBJECT_ID)
            system_id = ship->SystemID();

        if (ship->SystemID() != system_id) {
            ErrorLogger() << "Empire " << empire << " attempted to make a new fleet (" << fleet_name
                          << ") from ship (" << ship->ID() << ") in the wrong system (" << ship->SystemID()
                          << " not " << system_id << ")";
            return false;
        }
    }

    if (system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                      << ") outside a system";
        return false;
    }
    auto system = context.ContextObjects().get<System>(system_id);
    if (!system) {
        ErrorLogger() << "Empire " << empire << " attempted to create a new fleet (" << fleet_name
                      << ") in a nonexistant system (" << system_id << ")";
        return false;
    }

    return true;
}

void NewFleetOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_fleet_name, m_ship_ids, m_aggression, context))
        return;

    Universe& u = context.ContextUniverse();
    ObjectMap& o = context.ContextObjects();
    auto empire_ids = context.EmpireIDs();

    u.InhibitUniverseObjectSignals(true);

    // validate specified ships
    auto validated_ships = o.find<Ship>(m_ship_ids);

    int system_id = validated_ships[0]->SystemID();
    auto system = o.get<System>(system_id);

    std::shared_ptr<Fleet> fleet;
    if (m_fleet_id == INVALID_OBJECT_ID) {
        // create fleet
        fleet = u.InsertNew<Fleet>(m_fleet_name, system->X(), system->Y(),
                                   EmpireID(), context.current_turn);
        m_fleet_id = fleet->ID();
    } else {
        fleet = u.InsertByEmpireWithID<Fleet>(EmpireID(), m_fleet_id, m_fleet_name,
                                              system->X(), system->Y(), EmpireID(),
                                              context.current_turn);
    }

    if (!fleet) {
        ErrorLogger() << "Unable to create fleet.";
        return;
    }

    fleet->GetMeter(MeterType::METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);
    fleet->SetAggression(m_aggression);

    // an ID is provided to ensure consistancy between server and client universes
    u.SetEmpireObjectVisibility(EmpireID(), fleet->ID(), Visibility::VIS_FULL_VISIBILITY);

    system->Insert(fleet);

    // new fleet will get same m_arrival_starlane as fleet of the first ship in the list.
    auto first_ship{validated_ships[0]};
    auto first_fleet = o.get<Fleet>(first_ship->FleetID());
    if (first_fleet)
        fleet->SetArrivalStarlane(first_fleet->ArrivalStarlane());

    std::unordered_set<std::shared_ptr<Fleet>> modified_fleets;
    int ordered_moved_turn = BEFORE_FIRST_TURN;
    // remove ships from old fleet(s) and add to new
    for (auto& ship : validated_ships) {
        if (auto old_fleet = o.get<Fleet>(ship->FleetID())) {
            ordered_moved_turn = std::max(ordered_moved_turn, old_fleet->LastTurnMoveOrdered());
            old_fleet->RemoveShips({ship->ID()});
            modified_fleets.insert(std::move(old_fleet));
        }
        ship->SetFleetID(fleet->ID());
    }
    fleet->AddShips(m_ship_ids);
    fleet->SetMoveOrderedTurn(ordered_moved_turn);

    if (m_fleet_name.empty())
        fleet->Rename(fleet->GenerateFleetName(context));

    u.InhibitUniverseObjectSignals(false);

    std::vector<std::shared_ptr<Fleet>> created_fleets{fleet};
    system->FleetsInsertedSignal(created_fleets);
    system->StateChangedSignal();

    // Signal changed state of modified fleets and remove any empty fleets.
    for (auto& modified_fleet : modified_fleets) {
        if (!modified_fleet->Empty()) {
            modified_fleet->StateChangedSignal();
        } else {
            if (auto modified_fleet_system = o.get<System>(modified_fleet->SystemID()))
                modified_fleet_system->Remove(modified_fleet->ID());

            u.Destroy(modified_fleet->ID(), empire_ids);
        }
    }
}

////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder(int empire_id, int fleet_id, int dest_system_id,
                               bool append, const ScriptingContext& context) :
    Order(empire_id),
    m_fleet(fleet_id),
    m_dest_system(dest_system_id),
    m_append(append)
{
    if (!Check(empire_id, fleet_id, dest_system_id, append, context))
        return;

    auto fleet = context.ContextObjects().get<Fleet>(FleetID()); // TODO: Get from passed-in stuff

    int start_system = fleet->SystemID();
    if (start_system == INVALID_OBJECT_ID)
        start_system = fleet->NextSystemID();
    if (append && !fleet->TravelRoute().empty())
        start_system = fleet->TravelRoute().back();

    auto short_path = context.ContextUniverse().GetPathfinder()->ShortestPath(
        start_system, m_dest_system, EmpireID(), context.ContextObjects());
    if (short_path.first.empty()) {
        ErrorLogger() << "FleetMoveOrder generated empty shortest path between system " << start_system
                      << " and " << m_dest_system << " for empire " << EmpireID() << " with fleet " << fleet_id;
        return;
    }

    // if in a system now, don't include it in the route
    if (short_path.first.front() == fleet->SystemID()) {
        DebugLogger() << "FleetMoveOrder removing fleet " << fleet_id
                      << " current system location " << fleet->SystemID()
                      << " from shortest path to system " << m_dest_system;
        short_path.first.erase(short_path.first.begin()); // pop_front();
    }

    m_route = std::move(short_path.first);

    // ensure a zero-length (invalid) route is not requested / sent to a fleet
    if (m_route.empty())
        m_route.push_back(start_system);
}

std::string FleetMoveOrder::Dump() const {
    return UserString("ORDER_FLEET_MOVE");
}

bool FleetMoveOrder::Check(int empire_id, int fleet_id, int dest_system_id,
                           bool append, const ScriptingContext& context)
{
    auto fleet = context.ContextObjects().get<Fleet>(fleet_id);
    if (!fleet) {
        ErrorLogger() << "Empire with id " << empire_id << " ordered fleet with id " << fleet_id << " to move, but no such fleet exists";
        return false;
    }

    if (!fleet->OwnedBy(empire_id) ) {
        ErrorLogger() << "Empire with id " << empire_id << " order to move but does not own fleet with id " << fleet_id;
        return false;
    }

    const auto& known_objs{AppEmpireID() == ALL_EMPIRES ?
        context.ContextUniverse().EmpireKnownObjects(empire_id) : context.ContextObjects()};
    auto dest_system = known_objs.get<System>(dest_system_id);
    if (!dest_system) {
        ErrorLogger() << "Empire with id " << empire_id << " ordered fleet to move to system with id " << dest_system_id << " but no such system is known to that empire";
        return false;
    }

    return true;
}

void FleetMoveOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_fleet, m_dest_system, m_append, context))
        return;

    // convert list of ids to list of System
    auto fleet = context.ContextObjects().get<Fleet>(FleetID());
    if (!fleet) {
        ErrorLogger() << "FleetMoveOrder::ExecuteImpl couldn't get fleet with ID: " << m_fleet;
        return;
    }
    using RouteListT = std::remove_const_t<std::remove_reference_t<decltype(fleet->TravelRoute())>>;
    RouteListT fleet_travel_route{m_append ? fleet->TravelRoute() : RouteListT{}};

    if (m_append && !fleet_travel_route.empty()) {
        DebugLogger() << "FleetMoveOrder::ExecuteImpl appending initial" << [&]() {
            std::stringstream ss;
            for (int waypoint : fleet_travel_route)
                ss << " " << waypoint;
            return ss.str();
        }() << "  with" << [&]() {
            std::stringstream ss;
            for (int waypoint : m_route)
                ss << " " << waypoint;
            return ss.str();
        }();
        fleet_travel_route.pop_back(); // remove last item as it should be the first in the appended route
    }

    std::copy(m_route.begin(), m_route.end(), std::back_inserter(fleet_travel_route));
    DebugLogger() << [&fleet, &fleet_travel_route]() {
        std::stringstream ss;
        ss << "FleetMoveOrder::ExecuteImpl Setting route of fleet " << fleet->ID() << " at system " << fleet->SystemID() << " to: ";
        if (fleet_travel_route.empty())
            return std::string("[empty route]");
        for (int waypoint : fleet_travel_route)
            ss << " " << std::to_string(waypoint);
        return ss.str();
    }();

    if (!fleet_travel_route.empty() && fleet_travel_route.front() == fleet->SystemID()) {
        DebugLogger() << "FleetMoveOrder::ExecuteImpl given route that starts with fleet " << fleet->ID()
                      << "'s current system (" << fleet_travel_route.front() << "); removing it";
        fleet_travel_route.erase(fleet_travel_route.begin()); // pop_front();
    }

    // check destination validity: disallow movement that's out of range
    auto eta = fleet->ETA(fleet->MovePath(fleet_travel_route, false, context));
    if (eta.first == Fleet::ETA_NEVER || eta.first == Fleet::ETA_OUT_OF_RANGE) {
        DebugLogger() << "FleetMoveOrder::ExecuteImpl rejected out of range move order";
        return;
    }

    try {
        fleet->SetRoute(fleet_travel_route, context.ContextObjects());
        fleet->SetMoveOrderedTurn(context.current_turn);
        // todo: set last turn ordered moved
    } catch (const std::exception& e) {
        ErrorLogger() << "Caught exception setting fleet route while executing fleet move order: " << e.what();
    }
}

////////////////////////////////////////////////
// FleetTransferOrder
////////////////////////////////////////////////
FleetTransferOrder::FleetTransferOrder(int empire, int dest_fleet, const std::vector<int>& ships,
                                       const ScriptingContext& context) :
    Order(empire),
    m_dest_fleet(dest_fleet),
    m_add_ships(ships)
{
    if (!Check(empire, dest_fleet, ships, context))
        ErrorLogger() << "FleetTransferOrder constructor found problem...";
}

std::string FleetTransferOrder::Dump() const
{ return UserString("ORDER_FLEET_TRANSFER"); }

bool FleetTransferOrder::Check(int empire_id, int dest_fleet_id, const std::vector<int>& ship_ids,
                               const ScriptingContext& context)
{
    const ObjectMap& objects{context.ContextObjects()};

    auto fleet = objects.get<Fleet>(dest_fleet_id);
    if (!fleet) {
        ErrorLogger() << "Empire attempted to move ships to a nonexistant fleet";
        return false;
    }
    // check that destination fleet is owned by empire
    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueFleetTransferOrder : passed fleet_id "<< dest_fleet_id << " of fleet not owned by player";
        return false;
    }

    if (fleet->SystemID() == INVALID_OBJECT_ID) {
        ErrorLogger() << "IssueFleetTransferOrder : new fleet is not in a system";
        return false;
    }

    bool invalid_ships{false};

    for (const auto& ship : objects.find<Ship>(ship_ids)) {
        if (!ship) {
            ErrorLogger() << "IssueFleetTransferOrder : passed an invalid ship_id";
            invalid_ships = true;
            break;
        }

        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship_id of ship not owned by player";
            invalid_ships = true;
            break;
        }

        if (ship->SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueFleetTransferOrder : ship is not in a system";
            invalid_ships = true;
            break;
        }

        if (ship->SystemID() != fleet->SystemID()) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship is not in the same system as the target fleet";
            invalid_ships = true;
            break;
        }

        if (ship->FleetID() == dest_fleet_id) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship that is already in the target fleet";
            invalid_ships = true;
            break;
        }

        if (auto original_fleet = context.ContextObjects().get<Fleet>(ship->FleetID())) {
            auto ship_old_fleet_arr_lane = original_fleet->ArrivalStarlane();
            if (ship_old_fleet_arr_lane == INVALID_OBJECT_ID) // deal with quirky case where new fleets have no arrival lane for one turn
                ship_old_fleet_arr_lane = original_fleet->SystemID();
            auto new_fleet_arr_lane = fleet->ArrivalStarlane();
            if (new_fleet_arr_lane == INVALID_OBJECT_ID)      // same quirky case can affect both source and destination fleets
                new_fleet_arr_lane = fleet->SystemID();

            if (ship_old_fleet_arr_lane != new_fleet_arr_lane) {
                ErrorLogger() << "IssueFleetTransferOrder : passed ship " << ship->ID()
                              << " that is in a fleet " << original_fleet->ID()
                              << " that has a different arrival starlane " << ship_old_fleet_arr_lane
                              << " than the destination fleet " << fleet->ID()
                              << " with arrival starlane " << new_fleet_arr_lane;
                invalid_ships = true;
                break;
            }
        }
    }

    return !invalid_ships;
}

void FleetTransferOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_dest_fleet, m_add_ships, context))
        return;

    // look up the destination fleet
    auto target_fleet = context.ContextObjects().get<Fleet>(m_dest_fleet);

    // check that all ships are in the same system
    auto ships = context.ContextObjects().find<Ship>(m_add_ships);

    context.ContextUniverse().InhibitUniverseObjectSignals(true);

    // remove from old fleet(s)
    std::set<Fleet*> modified_fleets;
    for (auto& ship : ships) {
        if (auto source_fleet = context.ContextObjects().getRaw<Fleet>(ship->FleetID())) {
            source_fleet->RemoveShips({ship->ID()});
            modified_fleets.insert(source_fleet);
        }
        ship->SetFleetID(target_fleet->ID());
    }

    // add to new fleet
    std::vector<int> validated_ship_ids;
    validated_ship_ids.reserve(m_add_ships.size());
    for (const auto& ship : ships)
        validated_ship_ids.push_back(ship->ID());

    target_fleet->AddShips(validated_ship_ids);

    context.ContextUniverse().InhibitUniverseObjectSignals(false);

    // signal change to fleet states
    modified_fleets.insert(target_fleet.get());
    auto empire_ids = context.EmpireIDs();

    for (auto* modified_fleet : modified_fleets) {
        if (!modified_fleet) {
            continue;
        } else if (!modified_fleet->Empty()) {
            modified_fleet->StateChangedSignal();
        } else {
            if (auto system = context.ContextObjects().getRaw<System>(modified_fleet->SystemID()))
                system->Remove(modified_fleet->ID());

            context.ContextUniverse().Destroy(modified_fleet->ID(), empire_ids);
        }
    }
}

////////////////////////////////////////////////
// ColonizeOrder
////////////////////////////////////////////////
ColonizeOrder::ColonizeOrder(int empire, int ship, int planet, const ScriptingContext& context) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{
    if (!Check(empire, ship, planet, context))
        return;
}

std::string ColonizeOrder::Dump() const
{ return UserString("ORDER_COLONIZE"); }

bool ColonizeOrder::Check(int empire_id, int ship_id, int planet_id,
                          const ScriptingContext& context)
{
    const Universe& u = context.ContextUniverse();
    const ObjectMap& o = context.ContextObjects();
    const SpeciesManager& sm = context.species;

    auto ship = o.get<Ship>(ship_id);
    if (!ship) {
        ErrorLogger() << "ColonizeOrder::Check() : empire " << empire_id
                      << " passed an invalid ship_id: " << ship_id;
        return false;
    }
    auto fleet = o.get<Fleet>(ship->FleetID());
    if (!fleet) {
        ErrorLogger() << "ColonizeOrder::Check() : empire " << empire_id
                      << " passed ship (" << ship_id << ") with an invalid fleet_id: " << ship->FleetID();
        return false;
    }

    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "ColonizeOrder::Check() : empire does not own fleet of passed ship";
        return 0;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "ColonizeOrder::Check() : got ship that isn't owned by the order-issuing empire";
        return false;
    }

    if (!ship->CanColonize(u, sm)) { // verifies that species exists and can colonize and that ship can colonize
        ErrorLogger() << "ColonizeOrder::Check() : got ship that can't colonize";
        return false;
    }

    auto planet = o.get<Planet>(planet_id);
    float colonist_capacity = ship->ColonyCapacity(u);
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::Check() : couldn't get planet with id " << planet_id;
        return false;
    }
    if (planet->GetMeter(MeterType::METER_POPULATION)->Initial() > 0.0f) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that already has population";
        return false;
    }
    if (!planet->Unowned() && planet->Owner() != empire_id) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that owned by another empire";
        return false;
    }
    if (planet->OwnedBy(empire_id) && colonist_capacity == 0.0f) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that is already owned by empire and colony ship with zero capcity";
        return false;
    }
    if (u.GetObjectVisibilityByEmpire(planet_id, empire_id) < Visibility::VIS_PARTIAL_VISIBILITY) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that empire has insufficient visibility of";
        return false;
    }
    if (colonist_capacity > 0.0f && planet->EnvironmentForSpecies(ship->SpeciesName()) < PlanetEnvironment::PE_HOSTILE) {
        ErrorLogger() << "ColonizeOrder::Check() : nonzero colonist capacity, " << colonist_capacity
                      << ", and planet " << planet->Name() << " of type, " << planet->Type() << ", that ship's species, "
                      << ship->SpeciesName() << ", can't colonize";
        return false;
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "ColonizeOrder::Check() : given id of ship not in a system";
        return false;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "ColonizeOrder::Check() : given ids of ship and planet not in the same system";
        return false;
    }
    if (planet->IsAboutToBeColonized()) {
        ErrorLogger() << "ColonizeOrder::Check() : given id planet that is already being colonized";
        return false;
    }

    return true;
}

void ColonizeOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_ship, m_planet, context))
        return;

    ObjectMap& objects{context.ContextObjects()};
    auto ship = objects.get<Ship>(m_ship);
    auto planet = objects.get<Planet>(m_planet);

    planet->SetIsAboutToBeColonized(true);
    ship->SetColonizePlanet(m_planet);

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool ColonizeOrder::UndoImpl(ScriptingContext& context) const {
    ObjectMap& objects{context.ContextObjects()};

    auto planet = objects.get<Planet>(m_planet);
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }
    if (!planet->IsAboutToBeColonized()) {
        ErrorLogger() << "ColonizeOrder::UndoImpl planet is not about to be colonized...";
        return false;
    }

    auto ship = objects.get<Ship>(m_ship);
    if (!ship) {
        ErrorLogger() << "ColonizeOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedColonizePlanet() != m_planet) {
        ErrorLogger() << "ColonizeOrder::UndoImpl ship is not about to colonize planet";
        return false;
    }

    planet->SetIsAboutToBeColonized(false);
    ship->ClearColonizePlanet();

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// InvadeOrder
////////////////////////////////////////////////
InvadeOrder::InvadeOrder(int empire, int ship, int planet, const ScriptingContext& context) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{
    if (!Check(empire, ship, planet, context))
        return;
}

std::string InvadeOrder::Dump() const {
    return UserString("ORDER_INVADE");
}

bool InvadeOrder::Check(int empire_id, int ship_id, int planet_id, const ScriptingContext& context) {
    const Universe& u = context.ContextUniverse();
    const ObjectMap& o = context.ContextObjects();

    // make sure ship_id is a ship...
    auto ship = o.get<Ship>(ship_id);
    if (!ship) {
        ErrorLogger() << "IssueInvadeOrder : passed an invalid ship_id";
        return false;
    }

    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueInvadeOrder : empire does not own passed ship";
        return false;
    }
    if (!ship->HasTroops(u)) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl got ship that can't invade";
        return false;
    }

    // get fleet of ship
    auto fleet = o.get<Fleet>(ship->FleetID());
    if (!fleet) {
        ErrorLogger() << "IssueInvadeOrder : ship with passed ship_id has invalid fleet_id";
        return false;
    }

    // make sure player owns ship and its fleet
    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueInvadeOrder : empire does not own fleet of passed ship";
        return false;
    }

    auto planet = o.get<Planet>(planet_id);
    if (!planet) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl couldn't get planet with id " << planet_id;
        return false;
    }

    if (ship->SystemID() != planet->SystemID()) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return false;
    }

    if (u.GetObjectVisibilityByEmpire(planet_id, empire_id) < Visibility::VIS_BASIC_VISIBILITY) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet that empire reportedly has insufficient visibility of, but will be allowed to proceed pending investigation";
        return false;
    }

    if (planet->OwnedBy(empire_id)) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet that is already owned by the order-issuing empire";
        return false;
    }

    if (planet->Unowned() && planet->GetMeter(MeterType::METER_POPULATION)->Initial() == 0.0f) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given unpopulated planet";
        return false;
    }

    if (planet->GetMeter(MeterType::METER_SHIELD)->Initial() > 0.0f) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet with shield > 0";
        return false;
    }

    if (!planet->Unowned() && context.ContextDiploStatus(planet->Owner(), empire_id) !=
                              DiplomaticStatus::DIPLO_WAR)
    {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet owned by an empire not at war with order-issuing empire";
        return false;
    }

    return true;
}

void InvadeOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_ship, m_planet, context))
        return;

    ObjectMap& objects{context.ContextObjects()};
    auto ship = objects.get<Ship>(m_ship);
    auto planet = objects.get<Planet>(m_planet);

    // note: multiple ships, from same or different empires, can invade the same planet on the same turn
    DebugLogger() << "InvadeOrder::ExecuteImpl set for ship " << m_ship << " "
                  << ship->Name() << " to invade planet " << m_planet << " " << planet->Name();
    planet->SetIsAboutToBeInvaded(true);
    ship->SetInvadePlanet(m_planet);

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool InvadeOrder::UndoImpl(ScriptingContext& context) const {
    ObjectMap& objects{context.ContextObjects()};

    auto planet = objects.get<Planet>(m_planet);
    if (!planet) {
        ErrorLogger() << "InvadeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    auto ship = objects.get<Ship>(m_ship);
    if (!ship) {
        ErrorLogger() << "InvadeOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedInvadePlanet() != m_planet) {
        ErrorLogger() << "InvadeOrder::UndoImpl ship is not about to invade planet";
        return false;
    }

    planet->SetIsAboutToBeInvaded(false);
    ship->ClearInvadePlanet();

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// BombardOrder
////////////////////////////////////////////////
BombardOrder::BombardOrder(int empire, int ship, int planet, const ScriptingContext& context) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{
    if(!Check(empire, ship, planet, context))
        return;
}

std::string BombardOrder::Dump() const {
    return UserString("ORDER_BOMBARD");
}

bool BombardOrder::Check(int empire_id, int ship_id, int planet_id,
                         const ScriptingContext& context)
{
    const Universe& universe = context.ContextUniverse();
    const ObjectMap& objects = context.ContextObjects();

    auto ship = objects.get<Ship>(ship_id);
    if (!ship) {
        ErrorLogger() << "BombardOrder::ExecuteImpl couldn't get ship with id " << ship_id;
        return false;
    }
    if (!ship->CanBombard(universe)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl got ship that can't bombard";
        return false;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl got ship that isn't owned by the order-issuing empire";
        return false;
    }

    auto planet = objects.get<Planet>(planet_id);
    if (!planet) {
        ErrorLogger() << "BombardOrder::ExecuteImpl couldn't get planet with id " << planet_id;
        return false;
    }
    if (planet->OwnedBy(empire_id)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet that is already owned by the order-issuing empire";
        return false;
    }
    if (!planet->Unowned() && context.ContextDiploStatus(planet->Owner(), empire_id) != DiplomaticStatus::DIPLO_WAR) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet owned by an empire not at war with order-issuing empire";
        return false;
    }
    if (universe.GetObjectVisibilityByEmpire(planet_id, empire_id) < Visibility::VIS_BASIC_VISIBILITY) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet that empire reportedly has insufficient visibility of, but will be allowed to proceed pending investigation";
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given id of ship not in a system";
        return false;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return false;
    }

    return true;
}

void BombardOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_ship, m_planet, context))
        return;

    ObjectMap& objects{context.ContextObjects()};
    auto ship = objects.get<Ship>(m_ship);
    auto planet = objects.get<Planet>(m_planet);

    // note: multiple ships, from same or different empires, can bombard the same planet on the same turn
    DebugLogger() << "BombardOrder::ExecuteImpl set for ship " << m_ship << " "
                  << ship->Name() << " to bombard planet " << m_planet << " "
                  << planet->Name();
    planet->SetIsAboutToBeBombarded(true);
    ship->SetBombardPlanet(m_planet);

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool BombardOrder::UndoImpl(ScriptingContext& context) const {
    ObjectMap& objects{context.ContextObjects()};

    auto planet = objects.get<Planet>(m_planet);
    if (!planet) {
        ErrorLogger() << "BombardOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    auto ship = objects.get<Ship>(m_ship);
    if (!ship) {
        ErrorLogger() << "BombardOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedBombardPlanet() != m_planet) {
        ErrorLogger() << "BombardOrder::UndoImpl ship is not about to bombard planet";
        return false;
    }

    planet->SetIsAboutToBeBombarded(false);
    ship->ClearBombardPlanet();

    if (auto fleet = objects.get<Fleet>(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// ChangeFocusOrder
////////////////////////////////////////////////
ChangeFocusOrder::ChangeFocusOrder(int empire, int planet, std::string focus,
                                   const ScriptingContext& context) :
    Order(empire),
    m_planet(planet),
    m_focus(focus)
{
    if (!Check(empire, planet, focus, context))
        return;
}

std::string ChangeFocusOrder::Dump() const {
    return UserString("ORDER_FOCUS_CHANGE");
}

bool ChangeFocusOrder::Check(int empire_id, int planet_id, const std::string& focus,
                             const ScriptingContext& context) {
    auto planet = context.ContextObjects().get<Planet>(planet_id);

    if (!planet) {
        ErrorLogger() << "Illegal planet id specified in change planet focus order.";
        return false;
    }

    if (!planet->OwnedBy(empire_id)) {
        ErrorLogger() << "Empire attempted to issue change planet focus to another's planet.";
        return false;
    }

    if constexpr (false) {    // todo: verify that focus is valid for specified planet
        ErrorLogger() << "IssueChangeFocusOrder : invalid focus specified";
        return false;
    }

    return true;
}

void ChangeFocusOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_planet, m_focus, context))
        return;

    auto planet = context.ContextObjects().get<Planet>(m_planet);

    planet->SetFocus(m_focus);
}

////////////////////////////////////////////////
// PolicyOrder
////////////////////////////////////////////////
PolicyOrder::PolicyOrder(int empire, std::string name, std::string category, bool adopt, int slot) :
    Order(empire),
    m_policy_name(std::move(name)),
    m_category(std::move(category)),
    m_slot(slot),
    m_adopt(adopt)
{}

std::string PolicyOrder::Dump() const
{ return m_adopt ? UserString("ORDER_POLICY_ADOPT") : UserString("ORDER_POLICY_ABANDON"); }

void PolicyOrder::ExecuteImpl(ScriptingContext& context) const {
    auto empire = GetValidatedEmpire(context);
    if (m_adopt) {
        DebugLogger() << "PolicyOrder adopt " << m_policy_name << " in category " << m_category
                      << " in slot " << m_slot;
        empire->AdoptPolicy(m_policy_name, m_category, context, m_adopt, m_slot);
    } else if (!m_revert) {
        DebugLogger() << "PolicyOrder revoke " << m_policy_name << " from category " << m_category
                      << " in slot " << m_slot;
        empire->AdoptPolicy(m_policy_name, m_category, context, m_adopt, m_slot);
    } else {
        empire->RevertPolicies();
    }
}

////////////////////////////////////////////////
// ResearchQueueOrder
////////////////////////////////////////////////
ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name) :
    Order(empire),
    m_tech_name(tech_name),
    m_remove(true)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name, int position) :
    Order(empire),
    m_tech_name(tech_name),
    m_position(position)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name, bool pause, float dummy) :
    Order(empire),
    m_tech_name(tech_name),
    m_pause(pause ? PAUSE : RESUME)
{}

std::string ResearchQueueOrder::Dump() const
{ return UserString("ORDER_RESEARCH"); }

void ResearchQueueOrder::ExecuteImpl(ScriptingContext& context) const {
    auto empire = GetValidatedEmpire(context);

    if (m_remove) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: removing from queue tech: " << m_tech_name;
        empire->RemoveTechFromQueue(m_tech_name);
    } else if (m_pause == PAUSE) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: pausing tech: " << m_tech_name;
        empire->PauseResearch(m_tech_name);
    } else if (m_pause == RESUME) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: unpausing tech: " << m_tech_name;
        empire->ResumeResearch(m_tech_name);
    } else if (m_position != INVALID_INDEX) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: adding tech to queue: " << m_tech_name;
        empire->PlaceTechInQueue(m_tech_name, m_position);
    } else {
        ErrorLogger() << "ResearchQueueOrder::ExecuteImpl: Malformed";
    }
}

////////////////////////////////////////////////
// ProductionQueueOrder
////////////////////////////////////////////////
ProductionQueueOrder::ProductionQueueOrder(ProdQueueOrderAction action, int empire,
                                           const ProductionQueue::ProductionItem& item, // TODO: pass by value and move?
                                           int number, int location, int pos) :
    Order(empire),
    m_item(item),
    m_location(location),
    m_new_quantity(number),
    m_new_index(pos),
    m_uuid(boost::uuids::random_generator()()),
    m_action(action)
{
    if (action != ProdQueueOrderAction::PLACE_IN_QUEUE)
        ErrorLogger() << "ProductionQueueOrder called with parameters for placing in queue but with another action";
}

ProductionQueueOrder::ProductionQueueOrder(ProdQueueOrderAction action, int empire,
                                           boost::uuids::uuid uuid, int num1, int num2) :
    Order(empire),
    m_uuid(uuid),
    m_action(action)
{
    switch(m_action) {
    case ProdQueueOrderAction::REMOVE_FROM_QUEUE:
        break;
    case ProdQueueOrderAction::SPLIT_INCOMPLETE:
    case ProdQueueOrderAction::DUPLICATE_ITEM:
        m_uuid2 = boost::uuids::random_generator()();
        break;
    case ProdQueueOrderAction::SET_QUANTITY_AND_BLOCK_SIZE:
        m_new_quantity = num1;
        m_new_blocksize = num2;
        break;
    case ProdQueueOrderAction::SET_QUANTITY:
        m_new_quantity = num1;
        break;
    case ProdQueueOrderAction::MOVE_ITEM_TO_INDEX:
        m_new_index = num1;
        break;
    case ProdQueueOrderAction::SET_RALLY_POINT:
        m_rally_point_id = num1;
        break;
    case ProdQueueOrderAction::PAUSE_PRODUCTION:
    case ProdQueueOrderAction::RESUME_PRODUCTION:
    case ProdQueueOrderAction::ALLOW_STOCKPILE_USE:
    case ProdQueueOrderAction::DISALLOW_STOCKPILE_USE:
        break;
    default:
        ErrorLogger() << "ProductionQueueOrder given unrecognized action!";
    }
}

std::string ProductionQueueOrder::Dump() const
{ return UserString("ORDER_PRODUCTION"); }

void ProductionQueueOrder::ExecuteImpl(ScriptingContext& context) const {
    try {
        auto empire = GetValidatedEmpire(context);

        switch(m_action) {
        case ProdQueueOrderAction::PLACE_IN_QUEUE: {
            if (m_item.build_type == BuildType::BT_BUILDING ||
                m_item.build_type == BuildType::BT_SHIP ||
                m_item.build_type == BuildType::BT_STOCKPILE)
            {
                DebugLogger() << "ProductionQueueOrder place in queue: " << m_item.Dump()
                              << "  at index: " << m_new_index;
                empire->PlaceProductionOnQueue(m_item, m_uuid, m_new_quantity, 1, m_location, m_new_index);
            } else {
                ErrorLogger() << "ProductionQueueOrder tried to place invalid build type in queue!";
            }
            break;
        }
        case ProdQueueOrderAction::REMOVE_FROM_QUEUE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to remove invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder removing item at index: " << idx;
                empire->RemoveProductionFromQueue(idx);
            }
            break;
        }
        case ProdQueueOrderAction::SPLIT_INCOMPLETE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to split invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder splitting incomplete from item";
                empire->SplitIncompleteProductionItem(idx, m_uuid2);
            }
            break;
        }
        case ProdQueueOrderAction::DUPLICATE_ITEM: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to duplicate invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder duplicating item";
                empire->DuplicateProductionItem(idx, m_uuid2);
            }
            break;
        }
        case ProdQueueOrderAction::SET_QUANTITY_AND_BLOCK_SIZE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to set quantity and blocksize of invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder setting quantity and block size";
                empire->SetProductionQuantityAndBlocksize(idx, m_new_quantity, m_new_blocksize);
            }
            break;
        }
        case ProdQueueOrderAction::SET_QUANTITY: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to set quantity of invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder setting quantity " << m_new_quantity;
                empire->SetProductionQuantity(idx, m_new_quantity);
            }
            break;
        }
        case ProdQueueOrderAction::MOVE_ITEM_TO_INDEX: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to move invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder moving to index " << m_new_index;
                empire->MoveProductionWithinQueue(idx, m_new_index);
            }
            break;
        }
        case ProdQueueOrderAction::SET_RALLY_POINT: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to set rally point of invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder setting rally point to " << m_rally_point_id;
                empire->SetProductionRallyPoint(idx, m_rally_point_id);
            }
            break;
        }
        case ProdQueueOrderAction::PAUSE_PRODUCTION: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to pause invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder pausing";
                empire->PauseProduction(idx);
            }
            break;
        }
        case ProdQueueOrderAction::RESUME_PRODUCTION: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to resume invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder resuming";
                empire->ResumeProduction(idx);
            }
            break;
        }
        case ProdQueueOrderAction::ALLOW_STOCKPILE_USE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to allow stockpiling on invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder allowing stockpile";
                empire->AllowUseImperialPP(idx, true);
            }
            break;
        }
        case ProdQueueOrderAction::DISALLOW_STOCKPILE_USE: {
            auto idx = empire->GetProductionQueue().IndexOfUUID(m_uuid);
            if (idx == -1) {
                ErrorLogger() << "ProductionQueueOrder asked to disallow stockpiling on invalid UUID: " << boost::uuids::to_string(m_uuid);
            } else {
                DebugLogger() << "ProductionQueueOrder disallowing stockpile";
                empire->AllowUseImperialPP(idx, false);
            }
            break;
        }
        default:
            ErrorLogger() << "ProductionQueueOrder::ExecuteImpl got invalid action";
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "Production order execution threw exception: " << e.what();
        throw;
    }
}

////////////////////////////////////////////////
// ShipDesignOrder
////////////////////////////////////////////////
ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id_to_remember) :
    Order(empire),
    m_design_id(existing_design_id_to_remember)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int design_id_to_erase, bool dummy) :
    Order(empire),
    m_design_id(design_id_to_erase),
    m_delete_design_from_empire(true)
{}

ShipDesignOrder::ShipDesignOrder(int empire, const ShipDesign& ship_design) :
    Order(empire),
    m_design_id(INVALID_DESIGN_ID),
    m_uuid(ship_design.UUID()),
    m_create_new_design(true),
    m_name(ship_design.Name(false)),
    m_description(ship_design.Description(false)),
    m_designed_on_turn(ship_design.DesignedOnTurn()),
    m_hull(ship_design.Hull()),
    m_parts(ship_design.Parts()),
    m_is_monster(ship_design.IsMonster()),
    m_icon(ship_design.Icon()),
    m_3D_model(ship_design.Model()),
    m_name_desc_in_stringtable(ship_design.LookupInStringtable())
{}

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id, const std::string& new_name/* = ""*/, const std::string& new_description/* = ""*/) :
    Order(empire),
    m_design_id(existing_design_id),
    m_update_name_or_description(true),
    m_name(new_name),
    m_description(new_description)
{}

std::string ShipDesignOrder::Dump() const
{ return UserString("ORDER_SHIP_DESIGN"); }

void ShipDesignOrder::ExecuteImpl(ScriptingContext& context) const {
    auto empire = GetValidatedEmpire(context);

    Universe& universe = context.ContextUniverse();

    if (m_delete_design_from_empire) {
        // player is ordering empire to forget about a particular design
        if (!empire->ShipDesignKept(m_design_id)) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to remove a ShipDesign id = " << m_design_id
                          << " that the empire wasn't remembering";
            return;
        }
        empire->RemoveShipDesign(m_design_id);

    } else if (m_create_new_design) {
        // check if a design with this ID already exists
        if (const auto existing = universe.GetShipDesign(m_design_id)) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to create a new ShipDesign with an id, " << m_design_id
                          << " of an already-existing ShipDesign " << existing->Name();

            return;
        }

        ShipDesign* new_ship_design = nullptr;
        try {
            new_ship_design = new ShipDesign(std::invalid_argument(""), m_name, m_description,
                                             m_designed_on_turn, EmpireID(), m_hull, m_parts,
                                             m_icon, m_3D_model, m_name_desc_in_stringtable,
                                             m_is_monster, m_uuid);
        } catch (const std::exception& e) {
            ErrorLogger() << "Couldn't create ship design: " << e.what();
            return;
        }

        if (m_design_id == INVALID_DESIGN_ID) {
            // On the client create a new design id
            universe.InsertShipDesign(new_ship_design);
            m_design_id = new_ship_design->ID();
            DebugLogger() << "ShipDesignOrder::ExecuteImpl Create new ship design ID " << m_design_id;
        } else {
            // On the server use the design id passed from the client
            if (!universe.InsertShipDesignID(new_ship_design, EmpireID(), m_design_id)) {
                ErrorLogger() << "Couldn't insert ship design by ID " << m_design_id;
                return;
            }
        }

        universe.SetEmpireKnowledgeOfShipDesign(m_design_id, EmpireID());
        empire->AddShipDesign(m_design_id, universe);

    } else if (m_update_name_or_description) {
        // player is ordering empire to rename a design
        const std::set<int>& empire_known_design_ids = universe.EmpireKnownShipDesignIDs(EmpireID());
        auto design_it = empire_known_design_ids.find(m_design_id);
        if (design_it == empire_known_design_ids.end()) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to rename/redescribe a ShipDesign id = " << m_design_id
                          << " that this empire hasn't seen";
            return;
        }
        const ShipDesign* design = universe.GetShipDesign(*design_it);
        if (!design) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to rename/redescribe a ShipDesign id = " << m_design_id
                          << " that doesn't exist (but this empire has seen it)!";
            return;
        }
        if (design->DesignedByEmpire() != EmpireID()) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to rename/redescribe a ShipDesign id = " << m_design_id
                          << " that isn't owned by this empire!";
            return;
        }
        universe.RenameShipDesign(m_design_id, m_name, m_description);

    } else {
        // player is ordering empire to retain a particular design, so that is can
        // be used to construct ships by that empire.

        // TODO: consider removing this order, so that an empire needs to use
        // espionage or influence to gain access to a ship design made by another
        // player

        // check if empire is already remembering the design
        if (empire->ShipDesignKept(m_design_id)) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to remember a ShipDesign id = " << m_design_id
                          << " that was already being remembered";
            return;
        }

        // check if the empire can see any objects that have this design (thus enabling it to be copied)
        const std::set<int>& empire_known_design_ids = universe.EmpireKnownShipDesignIDs(EmpireID());
        if (empire_known_design_ids.count(m_design_id)) {
            empire->AddShipDesign(m_design_id, universe);
        } else {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to remember a ShipDesign id = " << m_design_id
                          << " that this empire hasn't seen";
            return;
        }
    }
}

////////////////////////////////////////////////
// ScrapOrder
////////////////////////////////////////////////
ScrapOrder::ScrapOrder(int empire, int object_id, const ScriptingContext& context) :
    Order(empire),
    m_object_id(object_id)
{
    if (!Check(empire, object_id, context))
        return;
}

std::string ScrapOrder::Dump() const
{ return UserString("ORDER_SCRAP"); }

bool ScrapOrder::Check(int empire_id, int object_id, const ScriptingContext& context) {
    auto obj = context.ContextObjects().get(object_id);

    if (!obj) {
        ErrorLogger() << "IssueScrapOrder : passed an invalid object_id";
        return false;
    }

    if (!obj->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueScrapOrder : passed object_id of object not owned by player";
        return false;
    }

    if (obj->ObjectType() != UniverseObjectType::OBJ_SHIP && obj->ObjectType() != UniverseObjectType::OBJ_BUILDING) {
        ErrorLogger() << "ScrapOrder::Check : passed object that is not a ship or building";
        return false;
    }

    auto ship = context.ContextObjects().get<Ship>(object_id);
    if (ship && ship->SystemID() == INVALID_OBJECT_ID)
        ErrorLogger() << "ScrapOrder::Check : can scrap a traveling ship";

    return true;
}

void ScrapOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_object_id, context))
        return;

    ObjectMap& objects{context.ContextObjects()};

    if (auto ship = objects.get<Ship>(m_object_id)) {
        ship->SetOrderedScrapped(true);
    } else if (auto building = objects.get<Building>(m_object_id)) {
        building->SetOrderedScrapped(true);
    }
}

bool ScrapOrder::UndoImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);
    int empire_id = EmpireID();

    ObjectMap& objects{context.ContextObjects()};

    if (auto ship = objects.get<Ship>(m_object_id)) {
        if (ship->OwnedBy(empire_id))
            ship->SetOrderedScrapped(false);
    } else if (auto building = objects.get<Building>(m_object_id)) {
        if (building->OwnedBy(empire_id))
            building->SetOrderedScrapped(false);
    } else {
        return false;
    }
    return true;
}

////////////////////////////////////////////////
// AggressiveOrder
////////////////////////////////////////////////
AggressiveOrder::AggressiveOrder(int empire, int object_id, FleetAggression aggression,
                                 const ScriptingContext& context) :
    Order(empire),
    m_object_id(object_id),
    m_aggression(aggression)
{
    if (!Check(empire, object_id, m_aggression, context))
        return;
}

std::string AggressiveOrder::Dump() const
{ return UserString("ORDER_FLEET_AGGRESSION"); }

bool AggressiveOrder::Check(int empire_id, int object_id, FleetAggression aggression,
                            const ScriptingContext& context)
{
    const ObjectMap& objects{context.ContextObjects()};

    auto fleet = objects.get<Fleet>(object_id);
    if (!fleet) {
        ErrorLogger() << "IssueAggressionOrder : no fleet with passed id";
        return false;
    }

    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueAggressionOrder : passed object_id of object not owned by player";
        return false;
    }

    return true;
}

void AggressiveOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_object_id, m_aggression, context))
        return;

    if (auto fleet = context.ContextObjects().get<Fleet>(m_object_id))
        fleet->SetAggression(m_aggression);
    else
        ErrorLogger() << "AggressiveOrder::ExecuteImpl couldn't find fleet with id " << m_object_id;
}

/////////////////////////////////////////////////////
// GiveObjectToEmpireOrder
/////////////////////////////////////////////////////
GiveObjectToEmpireOrder::GiveObjectToEmpireOrder(int empire, int object_id, int recipient,
                                                 const ScriptingContext& context) :
    Order(empire),
    m_object_id(object_id),
    m_recipient_empire_id(recipient)
{
    if (!Check(empire, object_id, recipient, context))
        return;
}

std::string GiveObjectToEmpireOrder::Dump() const
{ return UserString("ORDER_GIVE_TO_EMPIRE"); }

bool GiveObjectToEmpireOrder::Check(int empire_id, int object_id, int recipient_empire_id,
                                    const ScriptingContext& context)
{
    if (!context.GetEmpire(recipient_empire_id)) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : given invalid recipient empire id";
        return false;
    }

    auto dip = context.ContextDiploStatus(empire_id, recipient_empire_id);
    if (dip < DiplomaticStatus::DIPLO_PEACE) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : attempting to give to empire not at peace";
        return false;
    }

    const ObjectMap& objects{context.ContextObjects()};

    auto obj = objects.get(object_id);
    if (!obj) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed invalid object id";
        return false;
    }

    if (!obj->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed object not owned by player";
        return false;
    }

    auto system = objects.get<System>(obj->SystemID());
    if (!system) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : couldn't get system of object";
        return false;
    }

    if (obj->ObjectType() != UniverseObjectType::OBJ_FLEET &&
        obj->ObjectType() != UniverseObjectType::OBJ_PLANET)
    {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed object that is not a fleet or planet";
        return false;
    }

    auto system_objects = objects.find<const UniverseObject>(system->ObjectIDs());
    if (!std::any_of(system_objects.begin(), system_objects.end(),
                     [recipient_empire_id](const auto& o){ return o->Owner() == recipient_empire_id; }))
    {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : recipient empire has nothing in system";
        return false;
    }

    return true;
}

void GiveObjectToEmpireOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);

    if (!Check(EmpireID(), m_object_id, m_recipient_empire_id, context))
        return;

    if (auto fleet = context.ContextObjects().get<Fleet>(m_object_id))
        fleet->SetGiveToEmpire(m_recipient_empire_id);
    else if (auto planet = context.ContextObjects().get<Planet>(m_object_id))
        planet->SetGiveToEmpire(m_recipient_empire_id);
}

bool GiveObjectToEmpireOrder::UndoImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);
    int empire_id = EmpireID();

    ObjectMap& objects{context.ContextObjects()};

    if (auto fleet = objects.get<Fleet>(m_object_id)) {
        if (fleet->OwnedBy(empire_id)) {
            fleet->ClearGiveToEmpire();
            return true;
        }
    } else if (auto planet = objects.get<Planet>(m_object_id)) {
        if (planet->OwnedBy(empire_id)) {
            planet->ClearGiveToEmpire();
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////
// ForgetOrder
////////////////////////////////////////////////
ForgetOrder::ForgetOrder(int empire, int object_id) :
    Order(empire),
    m_object_id(object_id)
{}

std::string ForgetOrder::Dump() const
{ return UserString("ORDER_FORGET"); }

void ForgetOrder::ExecuteImpl(ScriptingContext& context) const {
    GetValidatedEmpire(context);
    int empire_id = EmpireID();

    DebugLogger() << "ForgetOrder::ExecuteImpl empire: " << empire_id
                  << " for object: " << m_object_id;

    context.ContextUniverse().ForgetKnownObject(empire_id, m_object_id);
}
