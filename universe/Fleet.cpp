#include "Fleet.h"

#include "Predicates.h"
#include "ShipDesign.h"
#include "Ship.h"
#include "System.h"
#include "Pathfinder.h"
#include "Universe.h"
#include "Enums.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/ScopedTimer.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Supply.h"

#include <boost/bind.hpp>
#include <boost/timer.hpp>

namespace {
    const bool ALLOW_ALLIED_SUPPLY = true;

    const std::set<int> EMPTY_SET;
    const double MAX_SHIP_SPEED = 500.0;        // max allowed speed of ship movement
    const double FLEET_MOVEMENT_EPSILON = 0.1;  // how close a fleet needs to be to a system to have arrived in the system

    bool SystemHasNoVisibleStarlanes(int system_id, int empire_id)
    { return !GetPathfinder()->SystemHasVisibleStarlanes(system_id, empire_id); }

    void MoveFleetWithShips(Fleet& fleet, double x, double y){
        fleet.MoveTo(x, y);

        for (auto& ship : Objects().FindObjects<Ship>(fleet.ShipIDs())) {
            ship->MoveTo(x, y);
        }
    }

    void InsertFleetWithShips(Fleet& fleet, std::shared_ptr<System>& system){
        system->Insert(fleet.shared_from_this());

        for (auto& ship : Objects().FindObjects<Ship>(fleet.ShipIDs())) {
            system->Insert(ship);
        }
    }

    /** Return \p full_route terminates at \p last_system or before the first
        system not known to the \p empire_id. */
    std::list<int> TruncateRouteToEndAtSystem(const std::list<int>& full_route, int empire_id, int last_system) {

        if (full_route.empty() || (last_system == INVALID_OBJECT_ID))
            return std::list<int>();

        auto visible_end_it = full_route.cend();
        if (last_system != full_route.back()) {
            visible_end_it = std::find(full_route.begin(), full_route.end(), last_system);

            // if requested last system not in route, do nothing
            if (visible_end_it == full_route.end())
                return std::list<int>();

            ++visible_end_it;
        }

        // Remove any extra systems from the route after the apparent destination.
        // SystemHasNoVisibleStarlanes determines if empire_id knows about the
        // system and/or its starlanes.  It is enforced on the server in the
        // visibility calculations that an owning empire knows about a) the
        // system containing a fleet, b) the starlane on which a fleet is travelling
        // and c) both systems terminating a starlane on which a fleet is travelling.
        auto end_it = std::find_if(full_route.begin(), visible_end_it,
                                   boost::bind(&SystemHasNoVisibleStarlanes, _1, empire_id));

        std::list<int> truncated_route;
        std::copy(full_route.begin(), end_it, std::back_inserter(truncated_route));

        return truncated_route;
    }
}

// static(s)
const int Fleet::ETA_UNKNOWN =      (1 << 30);
const int Fleet::ETA_OUT_OF_RANGE = (1 << 30) - 1;
const int Fleet::ETA_NEVER =        (1 << 30) - 2;

Fleet::Fleet(const std::string& name, double x, double y, int owner) :
    UniverseObject(name, x, y)
{
    UniverseObject::Init();
    SetOwner(owner);
}

Fleet* Fleet::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return nullptr;

    Fleet* retval = new Fleet();
    retval->Copy(shared_from_this(), empire_id);
    return retval;
}

void Fleet::Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object.get() == this)
        return;
    auto copied_fleet = std::dynamic_pointer_cast<const Fleet>(copied_object);
    if (!copied_fleet) {
        ErrorLogger() << "Fleet::Copy passed an object that wasn't a Fleet";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    auto visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);

    if (vis >= VIS_BASIC_VISIBILITY) {
        m_ships =                         copied_fleet->VisibleContainedObjectIDs(empire_id);

        m_next_system = ((GetEmpireKnownSystem(copied_fleet->m_next_system, empire_id))
                         ? copied_fleet->m_next_system : INVALID_OBJECT_ID);
        m_prev_system = ((GetEmpireKnownSystem(copied_fleet->m_prev_system, empire_id))
                         ? copied_fleet->m_prev_system : INVALID_OBJECT_ID);
        m_arrived_this_turn =             copied_fleet->m_arrived_this_turn;
        m_arrival_starlane =              copied_fleet->m_arrival_starlane;

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            m_aggressive =                copied_fleet->m_aggressive;
            if (Unowned())
                m_name =                  copied_fleet->m_name;

            // Truncate the travel route to only systems known to empire_id
            int moving_to = (vis >= VIS_FULL_VISIBILITY
                             ? (!copied_fleet->m_travel_route.empty()
                                ? copied_fleet->m_travel_route.back()
                                : INVALID_OBJECT_ID)
                             : m_next_system);

            m_travel_route = TruncateRouteToEndAtSystem(copied_fleet->m_travel_route, empire_id, moving_to);


            if (vis >= VIS_FULL_VISIBILITY)
                m_ordered_given_to_empire_id =copied_fleet->m_ordered_given_to_empire_id;
        }
    }
}

bool Fleet::HostileToEmpire(int empire_id) const
{
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
           Empires().GetDiplomaticStatus(Owner(), empire_id) == DIPLO_WAR;
}

UniverseObjectType Fleet::ObjectType() const
{ return OBJ_FLEET; }

std::string Fleet::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << ( m_aggressive ? " agressive" : " passive")
       << " cur system: " << SystemID()
       << " moving to: " << FinalDestinationID()
       << " prev system: " << m_prev_system
       << " next system: " << m_next_system
       << " arrival lane: " << m_arrival_starlane
       << " ships: ";
    for (auto it = m_ships.begin(); it != m_ships.end();) {
        int ship_id = *it;
        ++it;
        os << ship_id << (it == m_ships.end() ? "" : ", ");
    }
    return os.str();
}

int Fleet::ContainerObjectID() const
{ return this->SystemID(); }

const std::set<int>& Fleet::ContainedObjectIDs() const
{ return m_ships; }

bool Fleet::Contains(int object_id) const
{ return object_id != INVALID_OBJECT_ID && m_ships.count(object_id); }

bool Fleet::ContainedBy(int object_id) const
{ return object_id != INVALID_OBJECT_ID && this->SystemID() == object_id; }

const std::string& Fleet::PublicName(int empire_id) const {
    // Disclose real fleet name only to fleet owners.
    if (GetUniverse().AllObjectsVisible() || empire_id == ALL_EMPIRES || OwnedBy(empire_id))
        return Name();
    else if (!Unowned())
        return UserString("FW_FOREIGN_FLEET");
    else if (Unowned() && HasMonsters())
        return UserString("MONSTERS");
    else if (Unowned() && GetVisibility(empire_id) > VIS_NO_VISIBILITY)
        return UserString("FW_ROGUE_FLEET");
    else
        return UserString("OBJ_FLEET");
}

int Fleet::MaxShipAgeInTurns() const {
    if (m_ships.empty())
        return INVALID_OBJECT_AGE;

    bool fleet_is_scrapped = true;
    int retval = 0;
    for (int ship_id : m_ships) {
        auto ship = GetShip(ship_id);
        if (!ship || ship->OrderedScrapped())
            continue;
        if (ship->AgeInTurns() > retval)
            retval = ship->AgeInTurns();
        fleet_is_scrapped = false;
    }

    if (fleet_is_scrapped)
        retval = 0;

    return retval;
}

const std::list<int>& Fleet::TravelRoute() const
{ return m_travel_route; }

std::list<MovePathNode> Fleet::MovePath(bool flag_blockades /*= false*/) const
{ return MovePath(TravelRoute(), flag_blockades); }

std::list<MovePathNode> Fleet::MovePath(const std::list<int>& route, bool flag_blockades /*= false*/) const {
    std::list<MovePathNode> retval;

    if (route.empty())
        return retval;                                      // nowhere to go => empty path
    // if (route.size() == 1) do nothing special.  this fleet is probably on the starlane leading to
    //                        its final destination.  normal looping to read destination should work fine
    if (route.size() == 2 && route.front() == route.back())
        return retval;                                      // nowhere to go => empty path
    if (this->Speed() < FLEET_MOVEMENT_EPSILON) {
        retval.emplace_back(this->X(), this->Y(), true, ETA_NEVER,
                            this->SystemID(),
                            INVALID_OBJECT_ID,
                            INVALID_OBJECT_ID,
                            false);
        return retval;                                      // can't move => path is just this system with explanatory ETA
    }

    float fuel =       Fuel();
    float max_fuel =   MaxFuel();

    //DebugLogger() << "Fleet " << this->Name() << " movePath fuel: " << fuel << " sys id: " << this->SystemID();

    // determine all systems where fleet(s) can be resupplied if fuel runs out
    const Empire* empire = GetEmpire(this->Owner());
    auto fleet_supplied_systems = GetSupplyManager().FleetSupplyableSystemIDs(this->Owner(), ALLOW_ALLIED_SUPPLY);
    auto& unobstructed_systems = empire ? empire->SupplyUnobstructedSystems() : EMPTY_SET;

    // determine if, given fuel available and supplyable systems, fleet will ever be able to move
    if (fuel < 1.0f &&
        this->SystemID() != INVALID_OBJECT_ID &&
        !fleet_supplied_systems.count(this->SystemID()))
    {
        retval.emplace_back(this->X(), this->Y(), true, ETA_OUT_OF_RANGE,
                            this->SystemID(),
                            INVALID_OBJECT_ID,
                            INVALID_OBJECT_ID);
        return retval;      // can't move => path is just this system with explanatory ETA
    }

    // blockade debug logging
    //DebugLogger() << "Fleet::MovePath for fleet " << this->Name() << " ID(" << this->ID() <<") and route:";
    //for (int waypoint : route)
    //    DebugLogger() << "Fleet::MovePath ... " << waypoint;
    //DebugLogger() << "Fleet::MovePath END of Route ";

    // get iterator pointing to std::shared_ptr<System> on route that is the first after where this fleet is currently.
    // if this fleet is in a system, the iterator will point to the system after the current in the route
    // if this fleet is not in a system, the iterator will point to the first system in the route
    auto route_it = route.begin();
    if (*route_it == SystemID())
        ++route_it;     // first system in route is current system of this fleet.  skip to the next system
    if (route_it == route.end())
        return retval;  // current system of this fleet is the *only* system in the route.  path is empty.


    // get current, previous and next systems of fleet
    auto cur_system = GetSystem(this->SystemID());          // may be 0
    auto prev_system = GetSystem(this->PreviousSystemID()); // may be 0 if this fleet is not moving or ordered to move
    auto next_system = GetSystem(*route_it);                // can't use this->NextSystemID() because this fleet may not be moving and may not have a next system. this might occur when a fleet is in a system, not ordered to move or ordered to move to a system, but a projected fleet move line is being calculated to a different system
    if (!next_system) {
        ErrorLogger() << "Fleet::MovePath couldn't get next system with id " << *route_it << " for this fleet " << this->Name();
        return retval;
    }

    //DebugLogger() << "initial cur system: " << (cur_system ? cur_system->Name() : "(none)") <<
    //                          "  prev system: " << (prev_system ? prev_system->Name() : "(none)") <<
    //                          "  next system: " << (next_system ? next_system->Name() : "(none)");


    bool is_post_blockade = false;
    if (cur_system) {
        //DebugLogger() << "Fleet::MovePath starting in system "<< SystemID();
        if (flag_blockades && next_system->ID() != m_arrival_starlane &&
            !unobstructed_systems.count(cur_system->ID()))
        {
            //DebugLogger() << "Fleet::MovePath checking blockade from "<< cur_system->ID() << " to "<< next_system->ID();
            if (BlockadedAtSystem(cur_system->ID(), next_system->ID())){
                // blockade debug logging
                //DebugLogger() <<   "Fleet::MovePath finds system " <<cur_system->Name() << " (" <<cur_system->ID() <<
                //                            ") blockaded for fleet " << this->Name();
                is_post_blockade = true;
            } else {
                // blockade debug logging
                //DebugLogger() <<   "Fleet::MovePath finds system " << cur_system->Name() << " (" << cur_system->ID() <<
                //                            ") NOT blockaded for fleet " << this->Name();
            }
        }
    }
    // place initial position MovePathNode
    retval.emplace_back(this->X(), this->Y(), false, 0,
                        (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                        (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                        (next_system ? next_system->ID() : INVALID_OBJECT_ID),
                        false);


    const int       TOO_LONG =              100;            // limit on turns to simulate.  99 turns max keeps ETA to two digits, making UI work better
    int             turns_taken =           1;
    double          turn_dist_remaining =   this->Speed();  // additional distance that can be travelled in current turn of fleet movement being simulated
    double          cur_x =                 this->X();
    double          cur_y =                 this->Y();
    double          next_x =                next_system->X();
    double          next_y =                next_system->Y();

    // simulate fleet movement given known speed, starting position, fuel limit and systems on route
    // need to populate retval with MovePathNodes that indicate the correct position, whether this
    // fleet will end a turn at the node, the turns it will take to reach the node, and (when applicable)
    // the current (if at a system), previous and next system IDs at which the fleet will be.  the
    // previous and next system ids are needed to know what starlane a given node is located on, if any.
    // nodes at systems don't need previous system ids to be valid, but should have next system ids
    // valid so that when rendering starlanes using the returned move path, lines departing a system
    // can be drawn on the correct side of the system icon

    while (turns_taken < TOO_LONG) {
        // each loop iteration moves the current position to the next location of interest along the move
        // path, and then adds a node at that position.

        //DebugLogger() << " starting iteration";
        //if (cur_system)
        //    DebugLogger() << "     at system " << cur_system->Name() << " with id " << cur_system->ID();
        //else
        //    DebugLogger() << "     at (" << cur_x << ", " << cur_y << ")";

        // Make sure that there actually still is a starlane between the two systems
        // we are between
        std::shared_ptr<const System> prev_or_cur;
        if (cur_system) {
            prev_or_cur = cur_system;
        } else if (prev_system) {
            prev_or_cur = prev_system;
        } else {
            ErrorLogger() << "Fleet::MovePath: No previous or current system!?";
        }
        if (prev_or_cur) {
            if (!prev_or_cur->HasStarlaneTo(next_system->ID())) {
                DebugLogger() << "Fleet::MovePath: No starlane connection between systems " << prev_or_cur->ID() << " and " << next_system->ID()
                                       << ". Abandoning the rest of the route.";
                return retval;
            }
        }


        // check if fuel limits movement or current system refuels passing fleet
        if (cur_system) {
            // check if current system has fuel supply available
            if (fleet_supplied_systems.count(cur_system->ID())) {
                // current system has fuel supply.  replenish fleet's supply and don't restrict movement
                fuel = max_fuel;
                //DebugLogger() << " ... at system with fuel supply.  replenishing and continuing movement";

            } else {
                // current system has no fuel supply.  require fuel to proceed
                if (fuel >= 1.0) {
                    //DebugLogger() << " ... at system without fuel supply.  consuming unit of fuel to proceed";
                    fuel -= 1.0;

                } else {
                    //DebugLogger() << " ... at system without fuel supply.  have insufficient fuel to continue moving";
                    turns_taken = ETA_OUT_OF_RANGE;
                    break;
                }
            }
        }

        // find distance to next system along path from current position
        double dist_to_next_system = std::sqrt((next_x - cur_x)*(next_x - cur_x) + (next_y - cur_y)*(next_y - cur_y));
        //DebugLogger() << " ... dist to next system: " << dist_to_next_system;


        // move ship as far as it can go this turn, or to next system, whichever is closer, and deduct
        // distance travelled from distance travellable this turn
        if (turn_dist_remaining >= FLEET_MOVEMENT_EPSILON) {
            double dist_travelled_this_step = std::min(turn_dist_remaining, dist_to_next_system);

            //DebugLogger() << " ... fleet moving " << dist_travelled_this_step << " this iteration.  dist to next system: " << dist_to_next_system << " and turn_dist_remaining: " << turn_dist_remaining;

            double x_dist = next_x - cur_x;
            double y_dist = next_y - cur_y;
            // dist_to_next_system = std::sqrt(x_dist * x_dist + y_dist * y_dist);  // should already equal this distance, so don't need to recalculate
            double unit_vec_x = x_dist / dist_to_next_system;
            double unit_vec_y = y_dist / dist_to_next_system;

            cur_x += unit_vec_x*dist_travelled_this_step;
            cur_y += unit_vec_y*dist_travelled_this_step;

            turn_dist_remaining -= dist_travelled_this_step;
            dist_to_next_system -= dist_travelled_this_step;

            // if moved away any distance from a system, are no longer in that system
            if (cur_system && dist_travelled_this_step >= FLEET_MOVEMENT_EPSILON) {
                prev_system = cur_system;
                cur_system = nullptr;
            }
        }

        bool end_turn_at_cur_position = false;

        // check if fleet can move any further this turn
        if (turn_dist_remaining < FLEET_MOVEMENT_EPSILON) {
            //DebugLogger() << " ... fleet can't move further this turn.";
            turn_dist_remaining = 0.0;      // to prevent any possible precision-related errors
            end_turn_at_cur_position = true;
        }

        // check if current position is close enough to next system on route to qualify as at that system.
        if (dist_to_next_system < FLEET_MOVEMENT_EPSILON) {
            // close enough to be consider to be at next system.
            // set current position to be exactly at next system to avoid rounding issues
            cur_system = next_system;
            if (!cur_system) {
                ErrorLogger() << "Fleet::MovePath got null next system!";
                break;
            }
            cur_x = cur_system->X();    // update positions to ensure no round-off-errors
            cur_y = cur_system->Y();

            //DebugLogger() << " ... arrived at system: " << cur_system->Name();

            bool clear_exit = cur_system->ID() == m_arrival_starlane; //just part of the test for the moment
            // attempt to get next system on route, to update next system.  if new current
            // system is the end of the route, abort.
            ++route_it;
            if (route_it != route.end()) {
                // update next system on route and distance to it from current position
                next_system = GetEmpireKnownSystem(*route_it, this->Owner());
                if (next_system) {
                    //DebugLogger() << "Fleet::MovePath checking unrestriced lane travel";
                    clear_exit = clear_exit || next_system->ID() == m_arrival_starlane ||
                    (empire && empire->PreservedLaneTravel(cur_system->ID(), next_system->ID()));
                }
            }
            if (flag_blockades && !clear_exit) {
                //DebugLogger() << "Fleet::MovePath checking blockades at system " << cur_system->Name() << " (" << cur_system->ID() <<
                //                 ") for fleet " << this->Name() << " travelling to system " << (*route_it);
                if (cur_system && next_system && BlockadedAtSystem(cur_system->ID(), next_system->ID())) {
                    // blockade debug logging
                    //DebugLogger() << "Fleet::MovePath finds system " << cur_system->Name() << " (" << cur_system->ID() <<
                    //                 ") blockaded for fleet " << this->Name();
                    is_post_blockade = true;
                } else {
                    //DebugLogger() << "Fleet::MovePath finds system " << cur_system->Name() << " (" << cur_system->ID() <<
                    //                 ") NOT blockaded for fleet " << this->Name();
                }
            }

            if (route_it == route.end() || !cur_system)
                break;

            if (!next_system) {
                ErrorLogger() << "Fleet::MovePath couldn't get system with id " << *route_it;
                break;
            }
            next_x = next_system->X();
            next_y = next_system->Y();
        }

        // if new position is an obstructed system, must end turn here
        // on client side, if have stale info on cur_system it may appear blockaded even if not actually obstructed,
        // and so will force a stop in that situation
        if (cur_system && !unobstructed_systems.count(cur_system->ID())) {
            turn_dist_remaining = 0.0;
            end_turn_at_cur_position = true;
        }

        // if turn done and turns taken is enough, abort simulation
        if (end_turn_at_cur_position && (turns_taken + 1 >= TOO_LONG)) {
            // exit loop before placing current node to simplify post-loop processing: now all cases require a post-loop node to be added
            ++turns_taken;
            break;
        }

        // blockade debug logging
        //DebugLogger() << "Fleet::MovePath for fleet " << this->Name() << " id " << this->ID() << " adding node at sysID " <<
        //                        (cur_system ? cur_system->ID() : INVALID_OBJECT_ID) << " with post blockade status " << is_post_blockade <<
        //                        " and ETA " << turns_taken;

        // add MovePathNode for current position (end of turn position and/or system location)
        retval.emplace_back(cur_x, cur_y, end_turn_at_cur_position, turns_taken,
                            (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                            (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                            (next_system ? next_system->ID() : INVALID_OBJECT_ID),
                            is_post_blockade);


        // if the turn ended at this position, increment the turns taken and
        // reset the distance remaining to be travelled during the current (now
        // next) turn for the next loop iteration
        if (end_turn_at_cur_position) {
            //DebugLogger() << " ... end of simulated turn " << turns_taken;
            ++turns_taken;
            turn_dist_remaining = this->Speed();
        }
    }


    // done looping.  may have exited due to reaching end of path, lack of fuel, or turns taken getting too big
    if (turns_taken == TOO_LONG)
        turns_taken = ETA_NEVER;
    // blockade debug logging
    //DebugLogger() << "Fleet::MovePath for fleet " << this->Name()<<" id "<<this->ID()<<" adding node at sysID "<<
    //                    (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID) << " with post blockade status " << is_post_blockade <<
    //                    " and ETA " << turns_taken;

    retval.emplace_back(cur_x, cur_y, true, turns_taken,
                        (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                        (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                        (next_system ? next_system->ID() : INVALID_OBJECT_ID),
                        is_post_blockade);
    //DebugLogger() << "Fleet::MovePath for fleet " << this->Name()<<" id "<<this->ID()<<" is complete";

    return retval;
}

std::pair<int, int> Fleet::ETA() const
{ return ETA(MovePath()); }

std::pair<int, int> Fleet::ETA(const std::list<MovePathNode>& move_path) const {
    // check that path exists.  if empty, there was no valid route or some other problem prevented pathing
    if (move_path.empty())
        return {ETA_UNKNOWN, ETA_UNKNOWN};

    // check for single node in path.  return the single node's eta as both .first and .second (likely indicates that fleet couldn't move)
    if (move_path.size() == 1) {
        const MovePathNode& node = *move_path.begin();
        return {node.eta, node.eta};
    }

    // general case: there is a multi-node path.  return the ETA of the first object node, and the ETA of the last node
    int last_stop_eta = move_path.rbegin()->eta;
    int first_stop_eta = last_stop_eta;
    for (auto it = ++(move_path.begin()); it != move_path.end(); ++it) {
        const MovePathNode& node = *it;
        if (node.object_id != INVALID_OBJECT_ID) {
            first_stop_eta = node.eta;
            break;
        }
    }

    return {last_stop_eta, first_stop_eta};
}

float Fleet::Fuel() const {
    if (NumShips() < 1)
        return 0.0f;

    // determine fuel available to fleet (fuel of the ship that has the least fuel in the fleet)
    float fuel = Meter::LARGE_VALUE;
    bool is_fleet_scrapped = true;

    for (auto& ship : Objects().FindObjects<const Ship>(m_ships)) {
        const Meter* meter = ship->UniverseObject::GetMeter(METER_FUEL);
        if (!meter) {
            ErrorLogger() << "Fleet::Fuel skipping ship with no fuel meter";
            continue;
        }
        if (!ship->OrderedScrapped()) {
            fuel = std::min(fuel, meter->Current());
            is_fleet_scrapped = false;
        } 
    }
    if (is_fleet_scrapped) {
        fuel = 0.0f;
    }
    return fuel;
}

float Fleet::MaxFuel() const {
    if (NumShips() < 1)
        return 0.0f;

    // determine the maximum amount of fuel that can be stored by the ship in the fleet that
    // can store the least amount of fuel
    float max_fuel = Meter::LARGE_VALUE;
    bool is_fleet_scrapped = true;

    for (auto& ship : Objects().FindObjects<const Ship>(m_ships)) {
        const Meter* meter = ship->UniverseObject::GetMeter(METER_MAX_FUEL);
        if (!meter) {
            ErrorLogger() << "Fleet::MaxFuel skipping ship with no max fuel meter";
            continue;
        }
        if (!ship->OrderedScrapped()) {
            max_fuel = std::min(max_fuel, meter->Current());
            is_fleet_scrapped = false;
        }
    }
    if (is_fleet_scrapped) {
        max_fuel = 0.0f;
    }
    return max_fuel;
}

int Fleet::FinalDestinationID() const {
    if (m_travel_route.empty()) {
        return INVALID_OBJECT_ID;
    } else {
        return m_travel_route.back();
    }
} 

namespace {
    bool HasXShips(const std::function<bool(const std::shared_ptr<const Ship>&)>& pred,
                   const std::set<int>& ship_ids)
    {
        // Searching for each Ship one at a time is faster than
        // FindObjects(ship_ids), because an early exit avoids searching the
        // remaining ids.
        return std::any_of(
            ship_ids.begin(), ship_ids.end(),
            [&pred](const int ship_id) {
                const auto& ship = Objects().Object<const Ship>(ship_id);
                if (!ship) {
                    WarnLogger() << "Object map is missing ship with expected id " << ship_id;
                    return false;
                }
                return pred(ship);
            });
    }
}

bool Fleet::HasMonsters() const {
    auto isX = [](const std::shared_ptr<const Ship>& ship){ return ship->IsMonster(); };
    return HasXShips(isX, m_ships);
}

bool Fleet::HasArmedShips() const {
    auto isX = [](const std::shared_ptr<const Ship>& ship){ return ship->IsArmed(); };
    return HasXShips(isX, m_ships);
}

bool Fleet::HasFighterShips() const {
    auto isX = [](const std::shared_ptr<const Ship>& ship){ return ship->HasFighters(); };
    return HasXShips(isX, m_ships);
}

bool Fleet::HasColonyShips() const {
    auto isX = [](const std::shared_ptr<const Ship>& ship) {
        if (ship->CanColonize())
            if (const auto design = ship->Design())
                if (design->ColonyCapacity() > 0.0f)
                    return true;
        return false;
    };
    return HasXShips(isX, m_ships);
}

bool Fleet::HasOutpostShips() const {
    auto isX = [](const std::shared_ptr<const Ship>& ship) {
        if (ship->CanColonize())
            if (const auto design = ship->Design())
                if (design->ColonyCapacity() == 0.0f)
                    return true;
        return false;
    };
    return HasXShips(isX, m_ships);
}

bool Fleet::HasTroopShips() const {
    auto isX = [](const std::shared_ptr<const Ship>& ship){ return ship->HasTroops(); };
    return HasXShips(isX, m_ships);
}

bool Fleet::HasShipsOrderedScrapped() const {
    auto isX = [](const std::shared_ptr<const Ship>& ship){ return ship->OrderedScrapped(); };
    return HasXShips(isX, m_ships);
}

bool Fleet::HasShipsWithoutScrapOrders() const {
    auto isX = [](const std::shared_ptr<const Ship>& ship){ return !ship->OrderedScrapped(); };
    return HasXShips(isX, m_ships);
}

float Fleet::ResourceOutput(ResourceType type) const {
    float output = 0.0f;
    if (NumShips() < 1)
        return output;
    MeterType meter_type = ResourceToMeter(type);
    if (meter_type == INVALID_METER_TYPE)
        return output;

    // determine resource output of each ship in this fleet
    for (auto& ship : Objects().FindObjects<const Ship>(m_ships)) {
        output += ship->CurrentMeterValue(meter_type);
    }
    return output;
}

bool Fleet::UnknownRoute() const
{ return m_travel_route.size() == 1 && m_travel_route.front() == INVALID_OBJECT_ID; }

std::shared_ptr<UniverseObject> Fleet::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Fleet>(std::static_pointer_cast<const Fleet>(shared_from_this()))); }

void Fleet::SetRoute(const std::list<int>& route) {
    //DebugLogger() << "Fleet::SetRoute() ";

    if (UnknownRoute())
        throw std::invalid_argument("Fleet::SetRoute() : Attempted to set an unknown route.");

    if (m_prev_system != SystemID() && m_prev_system == route.front() && !CanChangeDirectionEnRoute())
        throw std::invalid_argument("Fleet::SetRoute() : Illegally attempted to change a fleet's direction while it was in transit.");

    m_travel_route = route;

    // Moving to where we are is not moving at all
    if (m_travel_route.size() == 1 && this->SystemID() == m_travel_route.front()) {
        m_travel_route.clear();
        m_next_system = INVALID_OBJECT_ID;
    }

    if (!m_travel_route.empty()) {
        if (m_prev_system != SystemID() && m_prev_system == m_travel_route.front()) {
            m_prev_system = m_next_system;      // if already in transit and turning around, swap prev and next
        } else if (SystemID() == route.front()) {
            m_prev_system = SystemID();
        }
        auto it = m_travel_route.begin();
        m_next_system = m_prev_system == SystemID() && m_travel_route.size() > 1 ? (*++it) : (*it);
    }

    StateChangedSignal();
}

void Fleet::SetAggressive(bool aggressive/* = true*/) {
    if (aggressive == m_aggressive)
        return;
    m_aggressive = aggressive;
    StateChangedSignal();
}

void Fleet::AddShips(const std::vector<int>& ship_ids) {
    size_t old_ships_size = m_ships.size();
    std::copy(ship_ids.begin(), ship_ids.end(), std::inserter(m_ships, m_ships.end()));
    if (old_ships_size != m_ships.size())
        StateChangedSignal();
}

void Fleet::RemoveShips(const std::vector<int>& ship_ids) {
    size_t old_ships_size = m_ships.size();
    for (int ship_id : ship_ids)
        m_ships.erase(ship_id);
    if (old_ships_size != m_ships.size())
        StateChangedSignal();
}

void Fleet::SetNextAndPreviousSystems(int next, int prev) {
    m_prev_system = prev;
    m_next_system = next;
    m_arrival_starlane = prev; // see comment for ArrivalStarlane()
}

void Fleet::MovementPhase() {
    //DebugLogger() << "Fleet::MovementPhase this: " << this->Name() << " id: " << this->ID();

    Empire* empire = GetEmpire(Owner());
    std::set<int> supply_unobstructed_systems;
    if (empire)
        supply_unobstructed_systems.insert(empire->SupplyUnobstructedSystems().begin(),
                                           empire->SupplyUnobstructedSystems().end());

    std::vector<std::shared_ptr<Ship>> ships = Objects().FindObjects<Ship>(m_ships);

    // if owner of fleet can resupply ships at the location of this fleet, then
    // resupply all ships in this fleet
    if (GetSupplyManager().SystemHasFleetSupply(SystemID(), Owner(), ALLOW_ALLIED_SUPPLY)) {
        for (auto& ship : ships) {
            ship->Resupply();
        }
    }

    auto current_system = GetSystem(SystemID());
    auto initial_system = current_system;
    auto move_path = MovePath();

    // If the move path cannot lead to our destination,
    // make our route take us as far as it can
    if (!move_path.empty() && !m_travel_route.empty() &&
         move_path.back().object_id != m_travel_route.back())
    {
        auto shortened_route = TruncateRouteToEndAtSystem(m_travel_route, Owner(), move_path.back().object_id);
        SetRoute(shortened_route);
        move_path = MovePath();
    }

    auto it = move_path.begin();
    auto next_it = it;
    if (next_it != move_path.end())
        ++next_it;


    // is the fleet stuck in a system for a whole turn?
    if (current_system) {
        ///update m_arrival_starlane if no blockade, if needed
        if (supply_unobstructed_systems.count(SystemID())) {
            m_arrival_starlane = SystemID();//allows departure via any starlane
        }

        // in a system.  if there is no system after the current one in the
        // path, or the current and next nodes have the same system id, that
        // is an actual system, or if blockaded from intended path, then won't
        // be moving this turn.
        bool stopped = false;
        if (next_it == move_path.end()) {
            stopped = true;

        } else if (it->object_id != INVALID_OBJECT_ID && it->object_id == next_it->object_id) {
            stopped = true;

        } else if (m_arrival_starlane != SystemID()) {
            int next_sys_id;
            if (next_it->object_id != INVALID_OBJECT_ID) {
                next_sys_id = next_it->object_id;
            } else {
                next_sys_id = next_it->lane_end_id;
            }
            stopped = BlockadedAtSystem(SystemID(), next_sys_id);
        }

        if (stopped) {
            //// fuel regeneration for ships in stationary fleet
            //if (FinalDestinationID() == INVALID_OBJECT_ID ||
            //    FinalDestinationID() == SystemID())
            //{
            //    for (auto& ship : ships) {
            //        if (Meter* fuel_meter = ship->UniverseObject::GetMeter(METER_FUEL)) {
            //            fuel_meter->AddToCurrent(0.1001f);  // .0001 to prevent rounding down
            //            fuel_meter->BackPropagate();
            //        }
            //    }
            //}
            return;

        } else {
            // record previous system on fleet's path, and the starlane along
            // which it will arrive at the next system (for blockading purposes)
            m_arrival_starlane = SystemID();
            m_prev_system = SystemID();

            // remove fleet and ships from system they are departing
            current_system->Remove(ID());
            SetSystem(INVALID_OBJECT_ID);
            for (auto& ship : ships) {
                current_system->Remove(ship->ID());
                ship->SetSystem(INVALID_OBJECT_ID);
            }
        }
    }


    // if fleet not moving, nothing more to do.
    if (move_path.empty() || move_path.size() == 1) {
        return;
    }


    // move fleet in sequence to MovePathNodes it can reach this turn
    float fuel_consumed = 0.0f;
    for (it = move_path.begin(); it != move_path.end(); ++it) {
        next_it = it;   ++next_it;

        auto system = GetSystem(it->object_id);

        // is this system the last node reached this turn?  either it's an end of turn node,
        // or there are no more nodes after this one on path
        bool node_is_next_stop = (it->turn_end || next_it == move_path.end());


        if (system) {
            // node is a system.  explore system for all owners of this fleet
            if (empire) {
                empire->AddExploredSystem(it->object_id);
                empire->RecordPendingLaneUpdate(it->object_id, m_prev_system);  // specifies the lane from it->object_id back to m_prev_system is available
            }

            m_prev_system = system->ID();               // passing a system, so update previous system of this fleet

            // reached a system, so remove it from the route
            if (m_travel_route.front() == system->ID()) {
                m_travel_route.erase(m_travel_route.begin());
            } else {
                std::stringstream ss;
                for (const auto& loc : m_travel_route) {
                    auto route_system = GetSystem(loc);
                    ss << (route_system ? route_system->Name() : "invalid id") << "("<< loc << ") ";
                }

                std::stringstream ss_path_back;
                try {
                    const auto path_back_to_expected = GetPathfinder()->ShortestPath(
                        system->ID(), m_travel_route.front(), ALL_EMPIRES);
                    for (const auto& loc : path_back_to_expected.first) {
                        auto route_system = GetSystem(loc);
                        ss_path_back << (route_system ? route_system->Name() : "invalid id") << "("<< loc << ") ";
                    }
                } catch (const std::out_of_range&) {
                    ErrorLogger() << "Couldn't find route from current location back to expected path";
                }

                ErrorLogger() << "Fleet::MovementPhase: Encountered an unexpected system on route."
                              << "  Fleet is " << Name() << "(" << ID() << ")"
                              << " Expected system is " << system->Name() << "(" << system->ID() << ")"
                              << " but front of route is " << m_travel_route.front()
                              << ". Whole route is [" << ss.str() <<"]"
                              << ". Path back to expected path is [" << ss_path_back.str() << "]";
                // TODO: Notify the suer with a sitrep?
            }

            bool resupply_here = GetSupplyManager().SystemHasFleetSupply(system->ID(), this->Owner(), ALLOW_ALLIED_SUPPLY);

            // if this system can provide supplies, reset consumed fuel and refuel ships
            if (resupply_here) {
                //DebugLogger() << " ... node has fuel supply.  consumed fuel for movement reset to 0 and fleet resupplied";
                fuel_consumed = 0.0f;
                for (auto& ship : ships) {
                    ship->Resupply();
                }
            }


            // is system the last node reached this turn?
            if (node_is_next_stop) {
                // fleet ends turn at this node.  insert fleet and ships into system
                InsertFleetWithShips(*this, system);

                current_system = system;

                if (supply_unobstructed_systems.count(SystemID()))
                    m_arrival_starlane = SystemID();//allows departure via any starlane

                // Add current system to the start of any existing route for next turn
                if (!m_travel_route.empty() && m_travel_route.front() != SystemID())
                    m_travel_route.push_front(SystemID());

                break;

            } else {
                // fleet will continue past this system this turn.
                m_arrival_starlane = m_prev_system;
                if (!resupply_here) {
                    fuel_consumed += 1.0f;
                }
            }

        } else {
            // node is not a system.
            m_arrival_starlane = m_prev_system;
            if (node_is_next_stop) {            // node is not a system, but is it the last node reached this turn?
                MoveFleetWithShips(*this, it->x, it->y);
                break;
            }
        }
    }


    // update next system
    if (!m_travel_route.empty() && next_it != move_path.end() && it != move_path.end()) {
        // there is another system later on the path to aim for.  find it
        for (; next_it != move_path.end(); ++next_it) {
            if (GetSystem(next_it->object_id)) {
                //DebugLogger() << "___ setting m_next_system to " << next_it->object_id;
                m_next_system = next_it->object_id;
                break;
            }
        }

    } else {
        // no more systems on path
        m_arrived_this_turn = current_system != initial_system;
        m_next_system = m_prev_system = INVALID_OBJECT_ID;
    }


    // consume fuel from ships in fleet
    if (fuel_consumed > 0.0f) {
        for (auto& ship : ships) {
            if (Meter* meter = ship->UniverseObject::GetMeter(METER_FUEL)) {
                meter->AddToCurrent(-fuel_consumed);
                meter->BackPropagate();
            }
        }
    }
}

void Fleet::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    // give fleets base stealth very high, so that they can (almost?) never be
    // seen by empires that don't own them, unless their ships are seen and
    // that visibility is propagated to the fleet that contains the ships
    if (Meter* stealth = GetMeter(METER_STEALTH)) {
        stealth->ResetCurrent();
        stealth->AddToCurrent(2000.0f);
    }
}

void Fleet::CalculateRouteTo(int target_system_id) {
    std::list<int> route;

    //DebugLogger() << "Fleet::CalculateRoute";
    if (target_system_id == INVALID_OBJECT_ID) {
        SetRoute(route);
        return; 
    }

    if (m_prev_system != INVALID_OBJECT_ID && SystemID() == m_prev_system) {
        // if we haven't actually left yet, we have to move from whichever system we are at now

        if (!GetSystem(target_system_id)) {
            // destination system doesn't exist or doesn't exist in known universe, so can't move to it.  leave route empty.
            SetRoute(route);
            return;
        }

        std::pair<std::list<int>, double> path;
        try {
            path = GetPathfinder()->ShortestPath(m_prev_system, target_system_id, this->Owner());
        } catch (...) {
            DebugLogger() << "Fleet::CalculateRoute couldn't find route to system(s):"
                          << " fleet's previous: " << m_prev_system << " or moving to: " << target_system_id;
        }
        SetRoute(path.first);
        return;
    }


    int dest_system_id = target_system_id;

    // Geoff: commenting out the early exit code of the owner of a fleet doesn't
    // have visibility of the destination system, since this was preventing the
    // human client's attempts to find routes for enemy fleets, for which the
    // player's client doesn't know which systems are visible, and since
    // visibility of a system on the current turn isn't necessary to plot
    // route to it now that empire's can remember systems they've seen on
    // previous turns.
    //if (universe.GetObjectVisibilityByEmpire(dest_system_id, this->Owner()) <= VIS_NO_VISIBILITY) {
    //    // destination system isn't visible to this fleet's owner, so the fleet can't move to it
    //
    //    // check if system to which fleet is moving is visible to the fleet's owner.  this should always be true, but just in case...
    //    if (universe.GetObjectVisibilityByEmpire(m_next_system, this->Owner()) <= VIS_NO_VISIBILITY)
    //        return; // next system also isn't visible; leave route empty.
    //
    //    // safety check: ensure supposedly visible object actually exists in known universe.
    //    if (!GetSystem(m_next_system)) {
    //        ErrorLogger() << "Fleet::CalculateRoute found system with id " << m_next_system << " should be visible to this fleet's owner, but the system doesn't exist in the known universe!";
    //        return; // abort if object doesn't exist in known universe... can't path to it if it's not there, even if it's considered visible for some reason...
    //    }
    //
    //    // next system is visible, so move to that instead of ordered destination (m_moving_to)
    //    dest_system_id = m_next_system;
    //}

    // if we're between systems, the shortest route may be through either one
    if (this->CanChangeDirectionEnRoute()) {
        std::pair<std::list<int>, double> path1;
        try {
            path1 = GetPathfinder()->ShortestPath(m_next_system, dest_system_id, this->Owner());
        } catch (...) {
            DebugLogger() << "Fleet::CalculateRoute couldn't find route to system(s):"
                          << " fleet's next: " << m_next_system << " or destination: " << dest_system_id;
        }
        auto& sys_list1 = path1.first;
        if (sys_list1.empty()) {
            ErrorLogger() << "Fleet::CalculateRoute got empty route from ShortestPath";
            return;
        }
        auto obj = GetUniverseObject(sys_list1.front());
        if (!obj) {
            ErrorLogger() << "Fleet::CalculateRoute couldn't get path start object with id " << path1.first.front();
            return;
        }
        double dist_x = obj->X() - this->X();
        double dist_y = obj->Y() - this->Y();
        double dist1 = std::sqrt(dist_x*dist_x + dist_y*dist_y);

        std::pair<std::list<int>, double> path2;
        try {
            path2 = GetPathfinder()->ShortestPath(m_prev_system, dest_system_id, this->Owner());
        } catch (...) {
            DebugLogger() << "Fleet::CalculateRoute couldn't find route to system(s):"
                          << " fleet's previous: " << m_prev_system << " or destination: " << dest_system_id;
        }
        auto& sys_list2 = path2.first;
        if (sys_list2.empty()) {
            ErrorLogger() << "Fleet::CalculateRoute got empty route from ShortestPath";
            return;
        }
        obj = GetUniverseObject(sys_list2.front());
        if (!obj) {
            ErrorLogger() << "Fleet::CalculateRoute couldn't get path start object with id " << path2.first.front();
            return;
        }
        dist_x = obj->X() - this->X();
        dist_y = obj->Y() - this->Y();
        double dist2 = std::sqrt(dist_x*dist_x + dist_y*dist_y);

        // pick whichever path is quicker
        if (dist1 + path1.second < dist2 + path2.second) {
            SetRoute(path1.first);
        } else {
            SetRoute(path2.first);
        }

    } else {
        // Cannot change direction. Must go to the end of the current starlane
        std::pair<std::list<int>, double> path;
        try {
            path = GetPathfinder()->ShortestPath(m_next_system, dest_system_id, this->Owner());
        } catch (...) {
            DebugLogger() << "Fleet::CalculateRoute couldn't find route to system(s):"
                          << " fleet's next: " << m_next_system << " or destination: " << dest_system_id;
        }
        SetRoute(path.first);
    }
}

bool Fleet::Blockaded() const {
    auto system = GetSystem(this->SystemID());

    if (!system)
        return false;

    if (m_next_system != INVALID_OBJECT_ID)
        return BlockadedAtSystem(SystemID(), m_next_system);

    for (const auto& target_system : system->StarlanesWormholes()) {
        if (BlockadedAtSystem(this->SystemID(), target_system.first))
            return true;
    }

    return false;
}

bool Fleet::BlockadedAtSystem(int start_system_id, int dest_system_id) const {
    /** If a newly arrived fleet joins a non-blockaded fleet of the same empire
      * (perhaps should include allies?) already at the system, the newly
      * arrived fleet will not be blockaded.  Additionally, since fleets are
      * blockade-checked at movement phase and also postcombat, the following
      * tests mean that post-compbat, this fleet will be blockaded iff it was
      * blockaded pre-combat AND there are armed aggressive enemies surviving in
      * system post-combat which can detect this fleet.  Fleets arriving at the
      * same time do not blockade each other. Unrestricted lane access (i.e,
      * (fleet->ArrivalStarlane() == system->ID()) ) is used as a proxy for
      * order of arrival -- if an enemy has unrestricted l*ane access and you
      * don't, they must have arrived before you, or be in cahoots with someone
      * who did. */

    if (m_arrival_starlane == start_system_id) {
        //DebugLogger() << "Fleet::BlockadedAtSystem fleet " << ID() << " has cleared blockade flag for system (" << start_system_id << ")";
        return false;
    }
    bool not_yet_in_system = SystemID() != start_system_id;

    // find which empires have blockading aggressive armed ships in system;
    // fleets that just arrived do not blockade by themselves, but may
    // reinforce a preexisting blockade, and may possibly contribute to detection
    auto current_system = GetSystem(start_system_id);
    if (!current_system) {
        DebugLogger() << "Fleet::BlockadedAtSystem fleet " << ID() << " considering system (" << start_system_id << ") but can't retrieve system copy";
        return false;
    }

    auto empire = GetEmpire(this->Owner());
    if (empire) {
        auto unobstructed_systems = empire->SupplyUnobstructedSystems();
        if (unobstructed_systems.count(start_system_id))
            return false;
        if (empire->PreservedLaneTravel(start_system_id, dest_system_id)) {
            return false;
        } else {
            //DebugLogger() << "Fleet::BlockadedAtSystem fleet " << ID() << " considering travel from system (" << start_system_id << ") to system (" << dest_system_id << ")";
        }
    }

    float lowest_ship_stealth = 99999.9f; // arbitrary large number. actual stealth of ships should be less than this...
    for (auto& ship : Objects().FindObjects<const Ship>(this->ShipIDs())) {
        if (lowest_ship_stealth > ship->CurrentMeterValue(METER_STEALTH))
            lowest_ship_stealth = ship->CurrentMeterValue(METER_STEALTH);
    }

    float monster_detection = 0.0f;
    auto fleets = Objects().FindObjects<const Fleet>(current_system->FleetIDs());
    for (auto& fleet : fleets) {
        if (!fleet->Unowned())
            continue;

        for (auto& ship : Objects().FindObjects<const Ship>(fleet->ShipIDs())) {
            float cur_detection = ship->CurrentMeterValue(METER_DETECTION);
            if (cur_detection >= monster_detection)
                monster_detection = cur_detection;
        }
    }

    bool can_be_blockaded = false;
    for (auto& fleet : fleets) {
        if (fleet->NextSystemID() != INVALID_OBJECT_ID) //fleets trying to leave this turn can't blockade pre-combat.
            continue;
        bool unrestricted = (fleet->m_arrival_starlane == start_system_id);
        if  (fleet->Owner() == this->Owner()) {
            if (unrestricted)  // perhaps should consider allies 
                return false;
            continue;
        }
        if (!unrestricted && !not_yet_in_system)
            continue;

        bool can_see;
        if (!fleet->Unowned())
            can_see = (GetEmpire(fleet->Owner())->GetMeter("METER_DETECTION_STRENGTH")->Current() >= lowest_ship_stealth);
        else
            can_see = (monster_detection >= lowest_ship_stealth);
        if (!can_see)
            continue;

        bool at_war = Unowned() || fleet->Unowned() ||
                      Empires().GetDiplomaticStatus(this->Owner(), fleet->Owner()) == DIPLO_WAR;
        if (!at_war)
            continue;
        bool aggressive = (fleet->Aggressive() || fleet->Unowned());
        if (!aggressive)
            continue;
        // Newly created ships/monsters are not allowed to block other fleet movement since they have not even
        // potentially gone through a combat round at the present location.  Potential sources for such new ships are
        // monsters created via Effect and Ships/fleets newly constructed by empires.  We check ship ages not fleet
        // ageas since fleets can be created/destroyed as purely organizational matters.  Since these checks are
        // pertinent just during those stages of turn processing immediately following turn number advancement,
        // whereas the new ships were created just prior to turn advamcenemt, we require age greater than 1.
        // 
        if (fleet->MaxShipAgeInTurns() <= 1)
            continue;
        // These are the most costly checks.  Do them last
        if (!fleet->HasArmedShips() && !fleet->HasFighterShips())
            continue;

        // don't exit early here, because blockade may yet be thwarted by ownership & presence check above
        can_be_blockaded = true;

    }

    return can_be_blockaded;
}

float Fleet::Speed() const {
    if (m_ships.empty())
        return 0.0f;

    bool fleet_is_scrapped = true;
    float retval = MAX_SHIP_SPEED;  // max speed no ship can go faster than
    for (int ship_id : m_ships) {
        auto ship = GetShip(ship_id);
        if (!ship || ship->OrderedScrapped())
            continue;
        if (ship->Speed() < retval)
            retval = ship->Speed();
        fleet_is_scrapped = false;
    }

    if (fleet_is_scrapped)
        retval = 0.0f;

    return retval;
}

float Fleet::Damage() const {
    if (m_ships.empty())
        return 0.0f;

    bool fleet_is_scrapped = true;
    float retval = 0.0f;
    for (int ship_id : m_ships) {
        if (auto ship = GetShip(ship_id)) {
            if (!ship->OrderedScrapped()) {
                if (const auto design = ship->Design()){
                    retval += design->Attack();
                }
                fleet_is_scrapped = false;
            }
        }
    }

    if (fleet_is_scrapped)
        retval = 0.0f;

    return retval;
}

float Fleet::Structure() const {
    if (m_ships.empty())
        return 0.0f;

    bool fleet_is_scrapped = true;
    float retval = 0.0f;
    for (int ship_id : m_ships) {
        if (auto ship = GetShip(ship_id)) {
            if (!ship->OrderedScrapped()) {
                retval += ship->CurrentMeterValue(METER_STRUCTURE);
                fleet_is_scrapped = false;
            }
        }
    }

    if (fleet_is_scrapped)
        retval = 0.0f;

    return retval;
}

float Fleet::Shields() const {
    if (m_ships.empty())
        return 0.0f;

    bool fleet_is_scrapped = true;
    float retval = 0.0f;
    for (int ship_id : m_ships) {
        if (auto ship = GetShip(ship_id)) {
            if (!ship->OrderedScrapped()) {
                retval += ship->CurrentMeterValue(METER_SHIELD);
                fleet_is_scrapped = false;
            }
        }
    }

    if (fleet_is_scrapped)
        retval = 0.0f;

    return retval;
}

std::string Fleet::GenerateFleetName() {
    // TODO: Change returned name based on passed ship designs.  eg. return "colony fleet" if
    // ships are colony ships, or "battle fleet" if ships are armed.
    if (ID() == INVALID_OBJECT_ID)
        return UserString("NEW_FLEET_NAME_NO_NUMBER");

    std::vector<std::shared_ptr<const Ship>> ships;
    for (int ship_id : m_ships) {
        if (auto ship = GetShip(ship_id)) {
            ships.push_back(ship);
        }
    }

    auto it = ships.begin();

    // TODO C++11 replace all of these loops with std::all_of
    bool all_monster(true);
    while (it != ships.end()) {
        if (!(*it)->IsMonster()) {
            all_monster = false;
            break;
        }
        ++it;
    }

    if (all_monster)
        return boost::io::str(FlexibleFormat(UserString("NEW_MONSTER_FLEET_NAME")) % ID());

    bool all_colony(true);
    it = ships.begin();
    while (it != ships.end()) {
        if (!(*it)->CanColonize()) {
            all_colony = false;
            break;
        }
        ++it;
    }

    if (all_colony)
        return boost::io::str(FlexibleFormat(UserString("NEW_COLONY_FLEET_NAME")) % ID());

    bool all_non_coms(true);
    it = ships.begin();
    while (it != ships.end()) {
        if ((*it)->IsArmed() || (*it)->HasFighters() || (*it)->CanHaveTroops() || (*it)->CanBombard()) {
            all_non_coms = false;
            break;
        }
        ++it;
    }

    if (all_non_coms)
        return boost::io::str(FlexibleFormat(UserString("NEW_RECON_FLEET_NAME")) % ID());

    bool all_troop(true);
    it = ships.begin();
    while (it != ships.end()) {
        if (!(*it)->CanHaveTroops()) {
            all_troop = false;
            break;
        }
        ++it;
    }

    if (all_troop)
        return boost::io::str(FlexibleFormat(UserString("NEW_TROOP_FLEET_NAME")) % ID());

    bool all_bombard(true);
    it = ships.begin();
    while (it != ships.end()) {
        if (!(*it)->CanBombard()) {
            all_bombard = false;
            break;
        }
        ++it;
    }

    if (all_bombard)
        return boost::io::str(FlexibleFormat(UserString("NEW_BOMBARD_FLEET_NAME")) % ID());

    bool mixed_combat(true);
    it = ships.begin();
    while (it != ships.end()) {
        if (!((*it)->IsArmed() || (*it)->HasFighters() || (*it)->CanHaveTroops() || (*it)->CanBombard())) {
            mixed_combat = false;
            break;
        }
        ++it;
    }

    if (mixed_combat)
        return boost::io::str(FlexibleFormat(UserString("NEW_BATTLE_FLEET_NAME")) % ID());


    return boost::io::str(FlexibleFormat(UserString("NEW_FLEET_NAME")) % ID());
}

void Fleet::SetGiveToEmpire(int empire_id) {
    if (empire_id == m_ordered_given_to_empire_id) return;
    m_ordered_given_to_empire_id = empire_id;
    StateChangedSignal();
}

void Fleet::ClearGiveToEmpire()
{ SetGiveToEmpire(ALL_EMPIRES); }
