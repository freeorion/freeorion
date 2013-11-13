#include "Fleet.h"

#include "Universe.h"
#include "Ship.h"
#include "System.h"
#include "Predicates.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/ScopedTimer.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"
#include "../universe/ShipDesign.h"

#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>

#include <cmath>

namespace {
    const double MAX_SHIP_SPEED = 500.0;        // max allowed speed of ship movement
    const double FLEET_MOVEMENT_EPSILON = 0.1;  // how close a fleet needs to be to a system to have arrived in the system

    bool SystemHasNoVisibleStarlanes(int system_id, int empire_id)
    { return !GetUniverse().SystemHasVisibleStarlanes(system_id, empire_id); }

}

// static(s)
const int Fleet::ETA_UNKNOWN =      (1 << 30);
const int Fleet::ETA_OUT_OF_RANGE = (1 << 30) - 1;
const int Fleet::ETA_NEVER =        (1 << 30) - 2;

Fleet::Fleet(const std::string& name, double x, double y, int owner) :
    UniverseObject(name, x, y),
    m_moving_to(INVALID_OBJECT_ID),
    m_prev_system(INVALID_OBJECT_ID),
    m_next_system(INVALID_OBJECT_ID),
    m_aggressive(true),
    m_travel_distance(0.0),
    m_arrived_this_turn(false),
    m_arrival_starlane(INVALID_OBJECT_ID)
{
    UniverseObject::Init();
    SetOwner(owner);
}

Fleet* Fleet::Clone(int empire_id) const {
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= VIS_BASIC_VISIBILITY && vis <= VIS_FULL_VISIBILITY))
        return 0;

    Fleet* retval = new Fleet();
    retval->Copy(TemporaryFromThis(), empire_id);
    return retval;
}

void Fleet::Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object == this)
        return;
    TemporaryPtr<const Fleet> copied_fleet = universe_object_ptr_cast<Fleet>(copied_object);
    if (!copied_fleet) {
        Logger().errorStream() << "Fleet::Copy passed an object that wasn't a Fleet";
        return;
    }

    int copied_object_id = copied_object->ID();
    Visibility vis = GetUniverse().GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    std::set<std::string> visible_specials = GetUniverse().GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_object, vis, visible_specials);

    if (vis == VIS_BASIC_VISIBILITY) {
        ShipIDSet vis_ship_ids = copied_fleet->VisibleContainedObjects(empire_id);
        // removal from old fleet and setting of ship's m_fleet_id is handled by the ship, as is setting of the ships fleetID
        this->m_ships.insert(vis_ship_ids.begin(), vis_ship_ids.end());
        Logger().debugStream() << "Fleet::Copy for fleet " << this->Name() << " ID(" << this->ID() << ") BASIC_VISIBILITY -- updating infor for "<<vis_ship_ids.size()<<" ships";
    }
    if (vis >= VIS_BASIC_VISIBILITY) {
        this->m_next_system =                   copied_fleet->m_next_system;
        this->m_prev_system =                   copied_fleet->m_prev_system;
        this->m_arrived_this_turn =             copied_fleet->m_arrived_this_turn;
        this->m_arrival_starlane =              copied_fleet->m_arrival_starlane;

        if (vis >= VIS_PARTIAL_VISIBILITY) {
            this->m_ships =                     copied_fleet->VisibleContainedObjects(empire_id);
            this->m_aggressive =                copied_fleet->m_aggressive;
            if (this->Unowned())
                this->m_name =                  copied_fleet->m_name;

            if (vis >= VIS_FULL_VISIBILITY) {
                this->m_moving_to =             copied_fleet->m_moving_to;
                this->m_travel_route =          copied_fleet->m_travel_route;
                this->m_travel_distance =       copied_fleet->m_travel_distance;

            } else {
                int             moving_to =         copied_fleet->m_next_system;
                std::list<int>  travel_route;
                double          travel_distance =   copied_fleet->m_travel_distance;

                const std::list<int>& copied_fleet_route = copied_fleet->m_travel_route;

                ShortenRouteToEndAtSystem(travel_route, moving_to);
                if (!travel_route.empty() && travel_route.front() != 0 && travel_route.size() != copied_fleet_route.size()) {
                    if (moving_to == copied_fleet->m_moving_to)
                        moving_to = travel_route.back();
                    try {
                        travel_distance -= GetUniverse().ShortestPath(travel_route.back(),
                                                                      copied_fleet_route.back()).second;
                    } catch (...) {
                        Logger().debugStream() << "Fleet::Copy couldn't find route to system(s):"
                                               << " travel route back: " << travel_route.back()
                                               << " or copied fleet route back: " << copied_fleet_route.back();
                    }
                }

                this->m_moving_to = moving_to;
                this->m_travel_route = travel_route;
                this->m_travel_distance = travel_distance;
            }
        }
    }
}

const std::string& Fleet::TypeName() const
{ return UserString("FLEET"); }

UniverseObjectType Fleet::ObjectType() const
{ return OBJ_FLEET; }

std::string Fleet::Dump() const {
    std::stringstream os;
    os << UniverseObject::Dump();
    os << ( m_aggressive ? " agressive" : " passive")
       << " cur system: " << SystemID()
       << " moving to: " << m_moving_to
       << " prev system: " << m_prev_system
       << " next system: " << m_next_system
       << " arrival lane: " << m_arrival_starlane
       << " ships: ";
    for (ShipIDSet::const_iterator it = m_ships.begin(); it != m_ships.end();) {
        int ship_id = *it;
        ++it;
        os << ship_id << (it == m_ships.end() ? "" : ", ");
    }
    return os.str();
}

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
        return UserString("FLEET");
}

const std::list<int>& Fleet::TravelRoute() const {
    CalculateRoute();
    //Logger().debugStream() << "fleet travel route: ";
    //for (std::list<int>::const_iterator it = m_travel_route.begin(); it != m_travel_route.end(); ++it)
    //    Logger().debugStream() << "... " << (*it)->Name();
    return m_travel_route;
}

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
        retval.push_back(MovePathNode(this->X(), this->Y(), true, ETA_NEVER, this->SystemID(), INVALID_OBJECT_ID, INVALID_OBJECT_ID));
        return retval;                                      // can't move => path is just this system with explanatory ETA
    }

    float fuel =       Fuel();
    float max_fuel =   MaxFuel();

    //Logger().debugStream() << "Fleet " << this->Name() << " movePath fuel: " << fuel << " sys id: " << this->SystemID();

    // determine all systems where fleet(s) can be resupplied if fuel runs out
    int owner = this->Owner();
    const Empire* empire = Empires().Lookup(owner);
    std::set<int> fleet_supplied_systems;
    std::set<int> unobstructed_systems;
    if (empire) {
        fleet_supplied_systems = empire->FleetSupplyableSystemIDs();
        unobstructed_systems = empire->SupplyUnobstructedSystems();
    }

    // determine if, given fuel available and supplyable systems, fleet will ever be able to move
    if (fuel < 1.0 &&
        this->SystemID() != INVALID_OBJECT_ID &&
        fleet_supplied_systems.find(this->SystemID()) == fleet_supplied_systems.end())
    {
        MovePathNode node(this->X(), this->Y(), true, ETA_OUT_OF_RANGE,
                          this->SystemID(),
                          INVALID_OBJECT_ID,
                          INVALID_OBJECT_ID);
        retval.push_back(node);
        return retval;      // can't move => path is just this system with explanatory ETA
    }

    // blockade debug logging
    //Logger().debugStream() << "Fleet::MovePath for fleet " << this->Name() << " ID(" << this->ID() <<") and route:";
    //for (std::list<int>::const_iterator route_it = route.begin(); route_it != route.end(); route_it++)
    //    Logger().debugStream() << "Fleet::MovePath ... " << *route_it;
    //Logger().debugStream() << "Fleet::MovePath END of Route ";

    // get iterator pointing to TemporaryPtr<System> on route that is the first after where this fleet is currently.
    // if this fleet is in a system, the iterator will point to the system after the current in the route
    // if this fleet is not in a system, the iterator will point to the first system in the route
    std::list<int>::const_iterator route_it = route.begin();
    if (*route_it == SystemID())
        ++route_it;     // first system in route is current system of this fleet.  skip to the next system
    if (route_it == route.end())
        return retval;  // current system of this fleet is the *only* system in the route.  path is empty.


    // get current, previous and next systems of fleet
    TemporaryPtr<const System> cur_system = GetSystem(this->SystemID());         // may be 0
    TemporaryPtr<const System> prev_system = GetSystem(this->PreviousSystemID());// may be 0 if this fleet is not moving or ordered to move
    TemporaryPtr<const System> next_system = GetSystem(*route_it);               // can't use this->NextSystemID() because this fleet may not be moving and may not have a next system. this might occur when a fleet is in a system, not ordered to move or ordered to move to a system, but a projected fleet move line is being calculated to a different system
    if (!next_system) {
        Logger().errorStream() << "Fleet::MovePath couldn't get next system with id " << *route_it << " for this fleet " << this->Name();
        return retval;
    }

    //Logger().debugStream() << "initial cur system: " << (cur_system ? cur_system->Name() : "(none)") <<
    //                          "  prev system: " << (prev_system ? prev_system->Name() : "(none)") <<
    //                          "  next system: " << (next_system ? next_system->Name() : "(none)");


    bool isPostBlockade=false;
    if (cur_system) {
        //Logger().debugStream() << "Fleet::MovePath starting in system "<< SystemID();
        if (flag_blockades && next_system->ID() != m_arrival_starlane && 
            (unobstructed_systems.find(cur_system->ID()) == unobstructed_systems.end())) 
        {
            //Logger().debugStream() << "Fleet::MovePath checking blockade from "<< cur_system->ID() << " to "<< next_system->ID();
            if (BlockadedAtSystem(cur_system->ID(), next_system->ID())){
                // blockade debug logging
                //Logger().debugStream() <<   "Fleet::MovePath finds system " <<cur_system->Name() << " (" <<cur_system->ID() <<
                //                            ") blockaded for fleet " << this->Name();
                isPostBlockade = true;
            } else {
                // blockade debug logging
                //Logger().debugStream() <<   "Fleet::MovePath finds system " << cur_system->Name() << " (" << cur_system->ID() <<
                //                            ") NOT blockaded for fleet " << this->Name();
            }
        }
    }
    // place initial position MovePathNode
    MovePathNode initial_pos(this->X(), this->Y(), false /* not an end of turn node */, 0 /* turns taken to reach position of node */,
                             (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                             (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                             (next_system ? next_system->ID() : INVALID_OBJECT_ID));
    retval.push_back(initial_pos);


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

        //Logger().debugStream() << " starting iteration";
        //if (cur_system)
        //    Logger().debugStream() << "     at system " << cur_system->Name() << " with id " << cur_system->ID();
        //else
        //    Logger().debugStream() << "     at (" << cur_x << ", " << cur_y << ")";


        // check if fuel limits movement or current system refuels passing fleet
        if (cur_system) {
            // check if current system has fuel supply available
            if (fleet_supplied_systems.find(cur_system->ID()) != fleet_supplied_systems.end()) {
                // current system has fuel supply.  replenish fleet's supply and don't restrict movement
                fuel = max_fuel;
                //Logger().debugStream() << " ... at system with fuel supply.  replenishing and continuing movement";

            } else {
                // current system has no fuel supply.  require fuel to proceed
                if (fuel >= 1.0) {
                    //Logger().debugStream() << " ... at system without fuel supply.  consuming unit of fuel to proceed";
                    fuel -= 1.0;

                } else {
                    //Logger().debugStream() << " ... at system without fuel supply.  have insufficient fuel to continue moving";
                    turns_taken = ETA_OUT_OF_RANGE;
                    break;
                }
            }
        }


        // find distance to next system along path from current position
        double dist_to_next_system = std::sqrt((next_x - cur_x)*(next_x - cur_x) + (next_y - cur_y)*(next_y - cur_y));
        //Logger().debugStream() << " ... dist to next system: " << dist_to_next_system;


        // move ship as far as it can go this turn, or to next system, whichever is closer, and deduct
        // distance travelled from distance travellable this turn
        if (turn_dist_remaining >= FLEET_MOVEMENT_EPSILON) {
            double dist_travelled_this_step = std::min(turn_dist_remaining, dist_to_next_system);

            //Logger().debugStream() << " ... fleet moving " << dist_travelled_this_step << " this iteration.  dist to next system: " << dist_to_next_system << " and turn_dist_remaining: " << turn_dist_remaining;

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
                cur_system = TemporaryPtr<System>();
            }
        }

        bool end_turn_at_cur_position = false;

        // check if fleet can move any further this turn
        if (turn_dist_remaining < FLEET_MOVEMENT_EPSILON) {
            //Logger().debugStream() << " ... fleet can't move further this turn.";
            turn_dist_remaining = 0.0;      // to prevent any possible precision-related errors
            end_turn_at_cur_position = true;
        }

        // check if current position is close enough to next system on route to qualify as at that system.
        if (dist_to_next_system < FLEET_MOVEMENT_EPSILON) {
            // close enough to be consider to be at next system.
            // set current position to be exactly at next system to avoid rounding issues
            cur_system = next_system;
            cur_x = cur_system->X();    // update positions to ensure no round-off-errors
            cur_y = cur_system->Y();

            //Logger().debugStream() << " ... arrived at system: " << cur_system->Name();


            bool clear_exit = cur_system->ID() == m_arrival_starlane; //just part of the test for the moment
            // attempt to get next system on route, to update next system.  if new current
            // system is the end of the route, abort.
            ++route_it;
            if (route_it != route.end()) {
                // update next system on route and distance to it from current position
                next_system = GetEmpireKnownSystem(*route_it, owner);
                if (next_system) {
                    //Logger().debugStream() << "Fleet::MovePath checking unrestriced lane travel";
                    clear_exit = clear_exit || (next_system && next_system->ID() == m_arrival_starlane) ||
                    (empire && empire->UnrestrictedLaneTravel(cur_system->ID(), next_system->ID()));
                }
            }
            if (flag_blockades && !clear_exit) {
                //Logger().debugStream() <<   "Fleet::MovePath checking blockades at system "<<cur_system->Name() << " ("<<cur_system->ID() <<
                //                            ") for fleet " << this->Name() <<" travelling to system "<< (*route_it);
                if (BlockadedAtSystem(cur_system->ID(), next_system->ID())) {
                    // blockade debug logging
                    //Logger().debugStream() <<   "Fleet::MovePath finds system "<<cur_system->Name() << " ("<<cur_system->ID() <<
                    //                            ") blockaded for fleet " << this->Name();
                    isPostBlockade = true;
                } else {
                    //Logger().debugStream() <<   "Fleet::MovePath finds system "<<cur_system->Name() << " ("<<cur_system->ID() <<
                    //                            ") NOT blockaded for fleet " << this->Name();
                }
            }

            if (route_it == route.end())
                break;

            if (!next_system) {
                Logger().errorStream() << "Fleet::MovePath couldn't get system with id " << *route_it;
                break;
            }
            next_x = next_system->X();
            next_y = next_system->Y();
        }

        // if new position is an obstructed system, must end turn here
        // on client side, if have stale info on cur_system it may appear blockaded even if not actually obstructed,
        // and so will force a stop in that situation
        if (cur_system && (unobstructed_systems.find(cur_system->ID()) == unobstructed_systems.end())) {
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
        //Logger().debugStream() << "Fleet::MovePath for fleet " << this->Name() << " id " << this->ID() << " adding node at sysID " <<
        //                        (cur_system ? cur_system->ID() : INVALID_OBJECT_ID) << " with post blockade status " << isPostBlockade <<
        //                        " and ETA " << turns_taken;

        // add MovePathNode for current position (end of turn position and/or system location)
        MovePathNode cur_pos(cur_x, cur_y, end_turn_at_cur_position, turns_taken,
                             (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                             (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                             (next_system ? next_system->ID() : INVALID_OBJECT_ID), isPostBlockade);
        retval.push_back(cur_pos);


        // if the turn ended at this position, increment the turns taken and
        // reset the distance remaining to be travelled during the current (now
        // next) turn for the next loop iteration
        if (end_turn_at_cur_position) {
            //Logger().debugStream() << " ... end of simulated turn " << turns_taken;
            ++turns_taken;
            turn_dist_remaining = this->Speed();
        }
    }


    // done looping.  may have exited due to reaching end of path, lack of fuel, or turns taken getting too big
    if (turns_taken == TOO_LONG)
        turns_taken = ETA_NEVER;
    // blockade debug logging
    //Logger().debugStream() << "Fleet::MovePath for fleet " << this->Name()<<" id "<<this->ID()<<" adding node at sysID "<<
    //                    (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID) << " with post blockade status " << isPostBlockade <<
    //                    " and ETA " << turns_taken;

    MovePathNode final_pos(cur_x, cur_y, true, turns_taken,
                           (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                           (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                           (next_system ? next_system->ID() : INVALID_OBJECT_ID), isPostBlockade);
    retval.push_back(final_pos);
    //Logger().debugStream() << "Fleet::MovePath for fleet " << this->Name()<<" id "<<this->ID()<<" is complete";

    return retval;
}

std::pair<int, int> Fleet::ETA() const
{ return ETA(MovePath()); }

std::pair<int, int> Fleet::ETA(const std::list<MovePathNode>& move_path) const {
    // check that path exists.  if empty, there was no valid route or some other problem prevented pathing
    if (move_path.empty())
        return std::make_pair(ETA_UNKNOWN, ETA_UNKNOWN);

    // check for single node in path.  return the single node's eta as both .first and .second (likely indicates that fleet couldn't move)
    if (move_path.size() == 1) {
        const MovePathNode& node = *move_path.begin();
        return std::make_pair(node.eta, node.eta);
    }

    // general case: there is a multi-node path.  return the ETA of the first object node, and the ETA of the last node
    int last_stop_eta = move_path.rbegin()->eta;
    int first_stop_eta = last_stop_eta;
    for (std::list<MovePathNode>::const_iterator it = ++(move_path.begin()); it != move_path.end(); ++it) {
        const MovePathNode& node = *it;
        if (node.object_id != INVALID_OBJECT_ID) {
            first_stop_eta = node.eta;
            break;
        }
    }

    return std::make_pair(last_stop_eta, first_stop_eta);
}

float Fleet::Fuel() const {
    if (NumShips() < 1)
        return 0.0f;

    // determine fuel available to fleet (fuel of the ship that has the least fuel in the fleet)
    float fuel = Meter::LARGE_VALUE;
    bool is_fleet_scrapped = true;
    for (const_iterator ship_it = begin(); ship_it != end(); ++ship_it) {
        TemporaryPtr<const Ship> ship = GetShip(*ship_it);
        if (!ship) {
            Logger().errorStream() << "Fleet::Fuel couldn't get ship with id " << *ship_it;
            continue;
        }
        const Meter* meter = ship->UniverseObject::GetMeter(METER_FUEL);
        if (!meter) {
            Logger().errorStream() << "Fleet::Fuel skipping ship with no fuel meter";
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
    for (const_iterator ship_it = begin(); ship_it != end(); ++ship_it) {
        TemporaryPtr<const Ship> ship = GetShip(*ship_it);
        if (!ship) {
            Logger().errorStream() << "Fleet::MaxFuel couldn't get ship with id " << *ship_it;
            continue;
        }
        const Meter* meter = ship->UniverseObject::GetMeter(METER_MAX_FUEL);
        if (!meter) {
            Logger().errorStream() << "Fleet::MaxFuel skipping ship with no max fuel meter";
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

bool Fleet::HasMonsters() const {
    for (Fleet::const_iterator it = begin(); it != end(); it++) {
        if (TemporaryPtr<const Ship> ship = GetShip(*it))
            if (ship->IsMonster())
                return true;
    }
    return false;
}

bool Fleet::HasArmedShips() const {
    for (Fleet::const_iterator it = begin(); it != end(); it++) {
        if (TemporaryPtr<const Ship> ship = GetShip(*it))
            if (ship->IsArmed())
                return true;
    }
    return false;
}

bool Fleet::HasColonyShips() const {
    for (Fleet::const_iterator it = begin(); it != end(); it++)
        if (TemporaryPtr<const Ship> ship = GetShip(*it))
            if (ship->CanColonize())
                if (const ShipDesign* design = ship->Design())
                    if (design->ColonyCapacity() > 0.0)
                        return true;
    return false;
}

bool Fleet::HasOutpostShips() const {
    for (Fleet::const_iterator it = begin(); it != end(); it++)
        if (TemporaryPtr<const Ship> ship = GetShip(*it))
            if (ship->CanColonize())
                if (const ShipDesign* design = ship->Design())
                    if (design->ColonyCapacity() == 0.0)
                        return true;
    return false;
}

bool Fleet::HasTroopShips() const {
    for (Fleet::const_iterator it = begin(); it != end(); it++)
        if (TemporaryPtr<const Ship> ship = GetShip(*it))
            if (ship->HasTroops())
                return true;
    return false;
}

bool Fleet::HasShipsOrderedScrapped() const {
    for (Fleet::const_iterator it = begin(); it != end(); it++)
        if (TemporaryPtr<const Ship> ship = GetShip(*it))
            if (ship->OrderedScrapped())
                return true;
    return false;
}

bool Fleet::HasShipsWithoutScrapOrders() const {
    for (Fleet::const_iterator it = begin(); it != end(); it++)
        if (TemporaryPtr<const Ship> ship = GetShip(*it))
            if (!ship->OrderedScrapped())
                return true;
    return false;
}

bool Fleet::Contains(int object_id) const
{ return m_ships.find(object_id) != m_ships.end(); }

std::vector<TemporaryPtr<UniverseObject> > Fleet::FindObjects() const {
    std::vector<TemporaryPtr<UniverseObject> > retval;
    // add ships in this fleet
    for (ShipIDSet::const_iterator it = m_ships.begin(); it != m_ships.end(); ++it)
        if (TemporaryPtr<UniverseObject> obj = GetUniverseObject(*it))
            retval.push_back(obj);
    return retval;
}

std::vector<int> Fleet::FindObjectIDs() const {
    std::vector<int> retval;
    // add ships in this fleet
    std::copy(m_ships.begin(), m_ships.end(), std::back_inserter(retval));
    return retval;
}

bool Fleet::UnknownRoute() const {
    return m_travel_route.size() == 1 && m_travel_route.front() == INVALID_OBJECT_ID;
}

TemporaryPtr<UniverseObject> Fleet::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(const_ptr_cast<Fleet>(static_ptr_cast<const Fleet>(TemporaryFromThis()))); }

void Fleet::SetRoute(const std::list<int>& route) {
    //Logger().debugStream() << "Fleet::SetRoute() ";
    if (route.empty())
        throw std::invalid_argument("Fleet::SetRoute() : Attempted to set an empty route.");

    if (UnknownRoute())
        throw std::invalid_argument("Fleet::SetRoute() : Attempted to set an unknown route.");

    if (m_prev_system != SystemID() && m_prev_system == route.front() && !CanChangeDirectionEnRoute())
        throw std::invalid_argument("Fleet::SetRoute() : Illegally attempted to change a fleet's direction while it was in transit.");

    m_travel_route = route;


    // calculate length of line segments between systems on route, and sum up to determine length of route between
    // systems on route.  (Might later add distance from fleet to first system on route to this to get the total
    // route length, or this may itself be the total route length if the fleet is at the first system on the route).
    m_travel_distance = 0.0;
    for (std::list<int>::const_iterator it = m_travel_route.begin(); it != m_travel_route.end(); ++it) {
        std::list<int>::const_iterator next_it = it;    ++next_it;
        //Logger().debugStream() << "Fleet::SetRoute() new route has system id " << *it;

        if (next_it == m_travel_route.end())
            break;  // current system is the last on the route, so don't need to add any additional distance.

        TemporaryPtr<const System> cur_sys = GetSystem(*it);
        if (!cur_sys) {
            Logger().errorStream() << "Fleet::SetRoute() couldn't get system with id " << *it;
            return;
        }

        TemporaryPtr<const System> next_sys = GetSystem(*next_it);
        if (!next_sys) {
            Logger().errorStream() << "Fleet::SetRoute() couldn't get system with id " << *next_it;
            return;
        }

        double dist_x = next_sys->X() - cur_sys->X();
        double dist_y = next_sys->Y() - cur_sys->Y();
        m_travel_distance += std::sqrt(dist_x*dist_x + dist_y*dist_y);
    }


    // if resetting to no movement while in a system
    if (SystemID() != INVALID_OBJECT_ID && SystemID() == m_travel_route.back()) {
        m_moving_to = INVALID_OBJECT_ID;
        m_next_system = INVALID_OBJECT_ID;
        //m_prev_system = INVALID_OBJECT_ID;

    } else {
        // if we're already moving, add in the distance from where we are to the first system in the route
        if (SystemID() != route.front()) {
            TemporaryPtr<const System> starting_system = GetSystem(route.front());
            if (!starting_system) {
                Logger().errorStream() << "Fleet::SetRoute couldn't get system with id " << route.front();
                return;
            }
            double dist_x = starting_system->X() - this->X();
            double dist_y = starting_system->Y() - this->Y();
            m_travel_distance += std::sqrt(dist_x*dist_x + dist_y*dist_y);
        }
        m_moving_to = m_travel_route.back();
        if (m_prev_system != SystemID() && m_prev_system == m_travel_route.front()) {
            m_prev_system = m_next_system;      // if already in transit and turning around, swap prev and next
        } else if (SystemID() == route.front()) {
            m_prev_system = SystemID();
        }
        std::list<int>::const_iterator it = m_travel_route.begin();
        m_next_system = m_prev_system == SystemID() ? (*++it) : (*it);
    }

    StateChangedSignal();
}

void Fleet::SetAggressive(bool aggressive/* = true*/) {
    if (aggressive == m_aggressive)
        return;
    m_aggressive = aggressive;
    StateChangedSignal();
}

void Fleet::AddShip(int ship_id) {
    std::vector<int> ship_ids;
    ship_ids.push_back(ship_id);
    AddShips(ship_ids);
}

void Fleet::AddShips(const std::vector<int>& ship_ids) {
    ScopedTimer timer("Fleet::AddShips for " + boost::lexical_cast<std::string>(ship_ids.size()) + " ship ids");
    // sort ships to be added by old fleet
    std::map<int, std::pair<std::vector<int>, std::vector<TemporaryPtr<Ship> > > > old_fleets_ships;
    for (std::vector<int>::const_iterator it = ship_ids.begin(); it != ship_ids.end(); ++it) {
        TemporaryPtr<Ship> ship = GetShip(*it);
        if (!ship) {
            Logger().errorStream() << "Fleet::AddShips couldn't find ship with id  " << *it;
            continue;
        }
        if (this->Contains(*it))
            continue;
        old_fleets_ships[ship->FleetID()].first.push_back(*it);
        old_fleets_ships[ship->FleetID()].second.push_back(ship);
    }

    bool changed = false;

    // move all the ships from each source fleet together
    for (std::map<int, std::pair<std::vector<int>, std::vector<TemporaryPtr<Ship> > > >::const_iterator
         old_fleet_it = old_fleets_ships.begin();
         old_fleet_it != old_fleets_ships.end(); ++old_fleet_it)
    {
        const std::vector<int>& ships_to_move_ids = old_fleet_it->second.first;
        const std::vector<TemporaryPtr<Ship> >& ships_to_move = old_fleet_it->second.second;
        int old_fleet_id = old_fleet_it->first;

        if (TemporaryPtr<Fleet> old_fleet = GetFleet(old_fleet_id))
            old_fleet->RemoveShips(ships_to_move_ids);

        // ensure ships are in same system as this fleet and set their fleet IDs
        int fleet_system_id = this->SystemID();
        for (std::vector<TemporaryPtr<Ship> >::const_iterator ship_it = ships_to_move.begin();
             ship_it != ships_to_move.end(); ++ship_it)
        {
            TemporaryPtr<Ship> ship = *ship_it;
            if (fleet_system_id != ship->SystemID())
                if (TemporaryPtr<System> system = GetSystem(fleet_system_id))
                    system->Insert(ship);
            ship->SetFleetID(this->ID());
        }
        // record this fleet's ships
        std::copy(ships_to_move_ids.begin(), ships_to_move_ids.end(), std::inserter(m_ships, m_ships.end()));
        changed = true;
    }
    if (changed)
        StateChangedSignal();
}

void Fleet::RemoveShip(int ship_id) {
    std::vector<int> ship_ids;
    ship_ids.push_back(ship_id);
    RemoveShips(ship_ids);
}

void Fleet::RemoveShips(const std::vector<int>& ship_ids) {
    ScopedTimer timer("Fleet::RemoveShips for " + boost::lexical_cast<std::string>(ship_ids.size()) + " ship ids");
    std::set<int> initial_m_ships = m_ships;
    for (std::vector<int>::const_iterator it = ship_ids.begin(); it != ship_ids.end(); ++it)
        m_ships.erase(*it);
    if (m_ships != initial_m_ships)
        StateChangedSignal();
}

void Fleet::SetSystem(int sys) {
    //Logger().debugStream() << "Fleet::SetSystem(int sys)";
    UniverseObject::SetSystem(sys);
    for (iterator it = begin(); it != end(); ++it)
        if (TemporaryPtr<UniverseObject> obj = GetUniverseObject(*it))
            obj->SetSystem(sys);
}

void Fleet::MoveTo(double x, double y) {
    //Logger().debugStream() << "Fleet::MoveTo(double x, double y)";
    // move fleet itself
    UniverseObject::MoveTo(x, y);
    // move ships in fleet
    for (iterator it = begin(); it != end(); ++it)
        if (TemporaryPtr<UniverseObject> obj = GetUniverseObject(*it))
            obj->UniverseObject::MoveTo(x, y);
}

void Fleet::SetNextAndPreviousSystems(int next, int prev) {
    m_prev_system = prev;
    m_next_system = next;
    m_arrival_starlane = prev; // see comment for ArrivalStarlane()
}

void Fleet::MovementPhase() {
    //Logger().debugStream() << "Fleet::MovementPhase this: " << this->Name() << " id: " << this->ID();

    TemporaryPtr<Fleet> fleet = dynamic_ptr_cast<Fleet>(TemporaryFromThis());
    if (fleet != this) {
        Logger().errorStream() << "Fleet::MovementPhase was passed a TemporaryPtr different from itself.";
        return;
    }

    Empire* empire = Empires().Lookup(fleet->Owner());
    std::set<int> supply_unobstructed_systems;
    if (empire)
        supply_unobstructed_systems.insert(empire->SupplyUnobstructedSystems().begin(), empire->SupplyUnobstructedSystems().end());

    // if owner of fleet can resupply ships at the location of this fleet, then
    // resupply all ships in this fleet
    if (empire && empire->SystemHasFleetSupply(fleet->SystemID())) {
        for (Fleet::const_iterator ship_it = fleet->begin(); ship_it != fleet->end(); ++ship_it) {
            if (TemporaryPtr<Ship> ship = GetShip(*ship_it)) {
                ship->Resupply();
                if (Meter* fuel_meter = ship->UniverseObject::GetMeter(METER_FUEL))
                    fuel_meter->BackPropegate();
            }
        }
    }

    TemporaryPtr<System> current_system = GetSystem(fleet->SystemID());
    TemporaryPtr<const System> initial_system = current_system;
    std::list<MovePathNode> move_path = fleet->MovePath();
    std::list<MovePathNode>::const_iterator it = move_path.begin();
    std::list<MovePathNode>::const_iterator next_it = it;
    if (next_it != move_path.end())
        ++next_it;

    //Logger().debugStream() << "Fleet::MovementPhase for Fleet "<< ID() << " with m_arrival_lane "<<m_arrival_starlane<< " at system ("<<SystemID()<<")";

    // is the ship stuck in a system for a whole turn?
    if (current_system) {
        ///update m_arrival_starlane if no blockade, if needed
        // blockade debug logging
        //Logger().debugStream() << "Fleet::MovementPhase checking blockade for Fleet "<< ID() << " with m_arrival_lane "<<m_arrival_starlane<< " at current_system " << current_system->Name() << "("<<SystemID()<<")";
        if (supply_unobstructed_systems.find(fleet->SystemID()) != supply_unobstructed_systems.end()) {
                //Logger().debugStream() << "Fleet::MovementPhase clearing m_arrival_starlane for Fleet "<< ID() << " at current_system " << current_system->Name() << "("<<SystemID()<<")";
                fleet->m_arrival_starlane = fleet->SystemID();//allows departure via any starlane
        }
        // in a system.  if there is no system after the current one in the
        // path, or the current and next nodes have the same system id, that
        // is an actual system,  or if blockaded from intended path, then won't be moving this turn.
        bool stopped = false;
        if (next_it == move_path.end()) {
            stopped = true;
        } else if (it->object_id != INVALID_OBJECT_ID && it->object_id == next_it->object_id) {
            stopped = true;
            //Logger().debugStream() << "Fleet::MovementPhase stopping due to doubled system at start of path";
        } else if (fleet->m_arrival_starlane != fleet->SystemID()) {
            int next_sys_id;
            if (next_it->object_id != INVALID_OBJECT_ID) {
                next_sys_id = next_it->object_id;
            } else {
                next_sys_id = next_it->lane_end_id;
            }
            stopped = fleet->BlockadedAtSystem(fleet->SystemID(), next_sys_id);
        }

        if (stopped) {
            // fuel regeneration for ships in stationary fleet
            if (fleet->FinalDestinationID() == INVALID_OBJECT_ID ||
                fleet->FinalDestinationID() == fleet->SystemID())
            {
                for (Fleet::const_iterator ship_it = fleet->begin(); ship_it != fleet->end(); ++ship_it) {
                    if (TemporaryPtr<Ship> ship = GetShip(*ship_it))
                        if (Meter* fuel_meter = ship->UniverseObject::GetMeter(METER_FUEL)) {
                            fuel_meter->AddToCurrent(0.1001f);
                            fuel_meter->BackPropegate();
                        }
                }
            }
            return;
        } else {
            fleet->m_arrival_starlane = fleet->SystemID();
            fleet->m_prev_system = fleet->SystemID();
        }
    }


    // if fleet not moving, nothing more to do.
    if (move_path.empty() || move_path.size() == 1) {
        //Logger().debugStream() << "Fleet::MovementPhase: Fleet move path is empty or has only one entry.  doing nothing";
        return;
    }

    //Logger().debugStream() << "Fleet::MovementPhase move path:";
    //for (std::list<MovePathNode>::const_iterator it = move_path.begin(); it != move_path.end(); ++it)
    //    Logger().debugStream() << "... (" << it->x << ", " << it->y << ") at object id: " << it->object_id << " eta: " << it->eta << (it->turn_end ? " (end of turn)" : " (during turn)");


    // move fleet in sequence to MovePathNodes it can reach this turn
    float fuel_consumed = 0.0f;
    for (it = move_path.begin(); it != move_path.end(); ++it) {
        next_it = it;   ++next_it;

        TemporaryPtr<System> system = GetSystem(it->object_id);

        //Logger().debugStream() << "... node " << (system ? system->Name() : "no system");

        // is this system the last node reached this turn?  either it's an end of turn node,
        // or there are no more nodes after this one on path
        bool node_is_next_stop = (it->turn_end || next_it == move_path.end());


        if (system) {
            // node is a system.  explore system for all owners of this fleet
            if (empire) {
                empire->AddExploredSystem(it->object_id);
                empire->RecordPendingLaneUpdate(it->object_id, fleet->m_prev_system);  // specifies the lane from it->object_id back to m_prev_system is available
            }

            fleet->m_prev_system = system->ID();               // passing a system, so update previous system of this fleet

            bool resupply_here = empire ? empire->SystemHasFleetSupply(system->ID()) : false;

            // if this system can provide supplies, reset consumed fuel and refuel ships
            if (resupply_here) {
                //Logger().debugStream() << " ... node has fuel supply.  consumed fuel for movement reset to 0 and fleet resupplied";
                fuel_consumed = 0.0;
                for (Fleet::const_iterator ship_it = fleet->begin(); ship_it != fleet->end(); ++ship_it) {
                    TemporaryPtr<Ship> ship = GetShip(*ship_it);
                    assert(ship);
                    ship->Resupply();
                }
            }


            if (node_is_next_stop) {                    // is system the last node reached this turn?
                system->Insert(fleet);                       // fleet ends turn at this node.  insert fleet into system
                current_system = system;
                // blockade debug logging
                //Logger().debugStream() << "Fleet::MovementPhase checking blockade for Fleet "<< ID() << " with m_arrival_lane "<<m_arrival_starlane<< " at next stop node system " << current_system->Name() << "("<<SystemID()<<")";
                if (supply_unobstructed_systems.find(fleet->SystemID()) != supply_unobstructed_systems.end()) {
                    fleet->m_arrival_starlane = fleet->SystemID();//allows departure via any starlane
                    //Logger().debugStream() << "Fleet::MovementPhase clearing m_arrival_starlane for Fleet "<< ID() << " at (next stop node) system " << system->Name() << "("<<system->ID()<<")";
                }
                //Logger().debugStream() << "... ... inserted fleet into system";
                break;
            } else {
                // fleet will continue past this system this turn.
                //Logger().debugStream() << "... ... moved fleet to system (not inserted)";
                fleet->m_arrival_starlane = fleet->m_prev_system;
                //Logger().debugStream() << "Fleet::MovementPhase setting m_arrival_starlane for Fleet "<< ID() << " to system just passed: " << system->Name() << "("<<system->ID()<<")";
                if (!resupply_here) {
                    fuel_consumed += 1.0;
                    //Logger().debugStream() << "... ... consuming 1 unit of fuel to continue moving.  total fuel consumed now: " << fuel_consumed;
                } else {
                    //Logger().debugStream() << "... ... not consuming fuel to depart resupply system";
                }
            }

        } else {
            // node is not a system.
            fleet->m_arrival_starlane = fleet->m_prev_system;
            //Logger().debugStream() << "Fleet::MovementPhase setting m_arrival_starlane for Fleet "<< ID() << " mid starlane from system ID " << m_prev_system;
            if (node_is_next_stop) {                    // node is not a system, but is it the last node reached this turn?
                fleet->MoveTo(it->x, it->y);                       // fleet ends turn at this node.  move fleet here
                //Logger().debugStream() << "... ... moved fleet to position";
                break;
            }
        }
    }


    //Logger().debugStream() << "Fleet::MovementPhase rest of move path:";
    //for (std::list<MovePathNode>::const_iterator it2 = it; it2 != move_path.end(); ++it2)
    //    Logger().debugStream() << "... (" << it2->x << ", " << it2->y << ") at object id: " << it2->object_id << " eta: " << it2->eta << (it2->turn_end ? " (end of turn)" : " (during turn)");

    // update next system
    if (fleet->m_moving_to != fleet->SystemID() && next_it != move_path.end() && it != move_path.end()) {
        // there is another system later on the path to aim for.  find it
        for (; next_it != move_path.end(); ++next_it) {
            if (GetSystem(next_it->object_id)) {
                //Logger().debugStream() << "___ setting m_next_system to " << next_it->object_id;
                fleet->m_next_system = next_it->object_id;
                break;
            }
        }

    } else {
        // no more systems on path
        fleet->m_arrived_this_turn = current_system != initial_system;
        fleet->m_moving_to = fleet->m_next_system = fleet->m_prev_system = INVALID_OBJECT_ID;
    }


    // consume fuel from ships in fleet
    if (fuel_consumed > 0.0) {
        for (Fleet::const_iterator ship_it = fleet->begin(); ship_it != fleet->end(); ++ship_it)
            if (TemporaryPtr<Ship> ship = GetShip(*ship_it))
                if (Meter* meter = ship->UniverseObject::GetMeter(METER_FUEL)) {
                    meter->AddToCurrent(-fuel_consumed);
                    meter->BackPropegate();
                }
    }
}

void Fleet::ResetTargetMaxUnpairedMeters() {
    UniverseObject::ResetTargetMaxUnpairedMeters();

    // give fleets base stealth very high, so that they can (almost?) never be
    // seen by empires that don't own them, unless their ships are seen and
    // that visibility is propegated to the fleet that contains the ships
    if (Meter* stealth = GetMeter(METER_STEALTH)) {
        stealth->ResetCurrent();
        stealth->AddToCurrent(2000.0);
    }
}

void Fleet::CalculateRoute() const {
    m_travel_distance = 0.0;
    m_travel_route.clear();

    //Logger().debugStream() << "Fleet::CalculateRoute";
    if (m_moving_to == INVALID_OBJECT_ID)
        return;

    if (m_prev_system != INVALID_OBJECT_ID && SystemID() == m_prev_system) {
        // if we haven't actually left yet, we have to move from whichever system we are at now

        if (!GetSystem(m_moving_to))
            return; // destination system doesn't exist or doesn't exist in known universe, so can't move to it.  leave route empty.

        std::pair<std::list<int>, double> path;
        try {
            path = GetUniverse().ShortestPath(m_prev_system, m_moving_to, this->Owner());
        } catch (...) {
            Logger().debugStream() << "Fleet::CalculateRoute couldn't find route to system(s):"
                                   << " fleet's previous: " << m_prev_system << " or moving to: " << m_moving_to;
        }
        m_travel_route = path.first;
        m_travel_distance = path.second;

        return;
    }


    int dest_system_id = m_moving_to;

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
    //        Logger().errorStream() << "Fleet::CalculateRoute found system with id " << m_next_system << " should be visible to this fleet's owner, but the system doesn't exist in the known universe!";
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
            path1 = GetUniverse().ShortestPath(m_next_system, dest_system_id, this->Owner());
        } catch (...) {
            Logger().debugStream() << "Fleet::CalculateRoute couldn't find route to system(s):"
                                   << " fleet's next: " << m_next_system << " or destination: " << dest_system_id;
        }
        const std::list<int>& sys_list1 = path1.first;
        if (sys_list1.empty()) {
            Logger().errorStream() << "Fleet::CalculateRoute got empty route from ShortestPath";
            return;
        }
        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(sys_list1.front());
        if (!obj) {
            Logger().errorStream() << "Fleet::CalculateRoute couldn't get path start object with id " << path1.first.front();
            return;
        }
        double dist_x = obj->X() - this->X();
        double dist_y = obj->Y() - this->Y();
        double dist1 = std::sqrt(dist_x*dist_x + dist_y*dist_y);

        std::pair<std::list<int>, double> path2;
        try {
            path2 = GetUniverse().ShortestPath(m_prev_system, dest_system_id, this->Owner());
        } catch (...) {
            Logger().debugStream() << "Fleet::CalculateRoute couldn't find route to system(s):"
                                   << " fleet's previous: " << m_prev_system << " or destination: " << dest_system_id;
        }
        const std::list<int>& sys_list2 = path2.first;
        if (sys_list2.empty()) {
            Logger().errorStream() << "Fleet::CalculateRoute got empty route from ShortestPath";
            return;
        }
        obj = GetUniverseObject(sys_list2.front());
        if (!obj) {
            Logger().errorStream() << "Fleet::CalculateRoute couldn't get path start object with id " << path2.first.front();
            return;
        }
        dist_x = obj->X() - this->X();
        dist_y = obj->Y() - this->Y();
        double dist2 = std::sqrt(dist_x*dist_x + dist_y*dist_y);

        // pick whichever path is quicker
        if (dist1 + path1.second < dist2 + path2.second) {
            m_travel_route = path1.first;
            m_travel_distance = dist1 + path1.second;
        } else {
            m_travel_route = path2.first;
            m_travel_distance = dist2 + path2.second;
        }

    } else {

        std::pair<std::list<int>, double> route;
        try {
            route = GetUniverse().ShortestPath(m_next_system, dest_system_id, this->Owner());
        } catch (...) {
            Logger().debugStream() << "Fleet::CalculateRoute couldn't find route to system(s):"
                                   << " fleet's next: " << m_next_system << " or destination: " << dest_system_id;
        }
        const std::list<int>& sys_list = route.first;
        if (sys_list.empty()) {
            Logger().errorStream() << "Fleet::CalculateRoute got empty route from ShortestPath";
            return;
        }
        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(sys_list.front());
        if (!obj) {
            Logger().errorStream() << "Fleet::CalculateRoute couldn't get path start object with id " << route.first.front();
            return;
        }
        double dist_x = obj->X() - this->X();
        double dist_y = obj->Y() - this->Y();
        double dist = std::sqrt(dist_x*dist_x + dist_y*dist_y);
        m_travel_route = route.first;
        m_travel_distance = dist + route.second;
    }
}

bool Fleet::BlockadedAtSystem(int start_system_id, int dest_system_id) const {
    /** If a newly arrived fleet joins a non-blockaded fleet of the same empire (perhaps should include allies?)
     * already at the system, the newly arrived fleet will not be blockaded.  Additionally, 
     * since fleets are blockade-checked at movement phase and also postcombat, the following tests mean
     * that post-compbat, this fleet will be blockaded iff it was blockaded pre-combat AND there are armed 
     * aggressive enemies surviving in system post-combat which can detect this fleet.  Fleets arriving at the
     * same time do not blockade each other. Unrestricted lane access (i.e, (fleet->ArrivalStarlane() == system->ID()) ) 
     * is used as a proxy for order of arrival -- if an enemy has unrestricted l*ane access and you don't, 
     * they must have arrived before you, or be in cahoots with someone who did. */

    if (m_arrival_starlane == start_system_id) {
        //Logger().debugStream() << "Fleet::BlockadedAtSystem fleet " << ID() << " has cleared blockade flag for system (" << start_system_id << ")";
        return false;
    }
    bool not_yet_in_system = SystemID() != start_system_id;

    // find which empires have blockading aggressive armed ships in system; fleets that just arrived do not 
    // blockade by themselves, but may reinforce a preexisting blockade, and may possibly contribute to detection
    TemporaryPtr<System> current_system = GetSystem(start_system_id);
    if (!current_system) {
        Logger().debugStream() << "Fleet::BlockadedAtSystem fleet " << ID() << " considering system (" << start_system_id << ") but can't retrieve system copy";
        return false;
    }

    EmpireManager& manager = Empires();
    const Empire* empire = manager.Lookup(this->Owner());
    if (empire) {  
        std::set<int> unobstructed_systems = empire->SupplyUnobstructedSystems();
        if (unobstructed_systems.find(start_system_id) != unobstructed_systems.end())
            return false;
        if (empire->UnrestrictedLaneTravel(start_system_id, dest_system_id)) {
            return false;
        } else {
            //Logger().debugStream() << "Fleet::BlockadedAtSystem fleet " << ID() << " considering travel from system (" << start_system_id << ") to system (" << dest_system_id << ")";
        }
    }

    float lowestShipStealth = 99999.9f;
    for (std::set<int>::const_iterator ship_it = this->ShipIDs().begin(); ship_it != this->ShipIDs().end(); ++ship_it) {
        TemporaryPtr<const Ship> ship = GetShip(*ship_it);
        if ((ship) && lowestShipStealth > ship->CurrentMeterValue(METER_STEALTH))
            lowestShipStealth = ship->CurrentMeterValue(METER_STEALTH);
    }

    std::vector<int> system_fleet_ids = current_system->FindObjectIDs<Fleet>();

    float monsterDetection = 0.0f;
    for (std::vector<int>::const_iterator fleet_it = system_fleet_ids.begin(); fleet_it != system_fleet_ids.end(); ++fleet_it) {
        TemporaryPtr<const Fleet> fleet = GetFleet(*fleet_it);
        if (!fleet->Unowned())
            continue;
        for (std::set<int>::const_iterator ship_it = fleet->ShipIDs().begin(); ship_it != fleet->ShipIDs().end(); ++ship_it) {
            if (TemporaryPtr<const Ship> ship = GetShip(*ship_it)){
                float curDetect = ship->CurrentMeterValue(METER_DETECTION);
                if (curDetect >= monsterDetection)
                    monsterDetection = curDetect;
            }
        }
    }

    bool canBeBlockaded = false;
    for (std::vector<int>::const_iterator fleet_it = system_fleet_ids.begin(); fleet_it != system_fleet_ids.end(); ++fleet_it) {
        TemporaryPtr<const Fleet> fleet = GetFleet(*fleet_it);
        if (!fleet || (fleet->NextSystemID() != INVALID_OBJECT_ID)) //fleets trying to leave this turn can't blockade pre-combat.
            continue;
        bool unrestricted = (fleet->m_arrival_starlane == start_system_id);
        if  (fleet->Owner() == this->Owner()) {
            if (unrestricted)  // perhaps should consider allies 
                return false;
            continue;
        }
        bool can_see;
        if (!fleet->Unowned()) {
            can_see = (manager.Lookup(fleet->Owner())->GetMeter("METER_DETECTION_STRENGTH")->Current() >= lowestShipStealth);
        } else {
            can_see = (monsterDetection >= lowestShipStealth);
        }
        bool at_war = Unowned() || fleet->Unowned() || Empires().GetDiplomaticStatus(this->Owner(), fleet->Owner()) == DIPLO_WAR ;
        bool aggressive = (fleet->Aggressive() || fleet->Unowned() );

        if (aggressive && fleet->HasArmedShips() && at_war && can_see && (unrestricted || not_yet_in_system))
            canBeBlockaded = true; // don't exit early here, because blockade may yet be thwarted by ownership & presence check above
    }
    if (canBeBlockaded) {
        return true;
    }
    return false;

} 

float Fleet::Speed() const {
    if (m_ships.empty())
        return 0.0f;

    bool isFleetScrapped = true;
    float retval = MAX_SHIP_SPEED;  // max speed no ship can go faster than
    for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
        if (TemporaryPtr<const Ship> ship = GetShip(*it)) {
            if (!ship->OrderedScrapped()) {
                if (ship->Speed() < retval)
                    retval = ship->Speed();
                isFleetScrapped = false;
            }
        }
    }

    if (isFleetScrapped)
        retval = 0.0f;

    return retval;
}

float Fleet::Damage() const {
    if (m_ships.empty())
        return 0.0f;

    bool isFleetScrapped = true;
    float retval = 0.0f;
    for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
        if (TemporaryPtr<const Ship> ship = GetShip(*it)) {
            if (!ship->OrderedScrapped()) {
                if (const ShipDesign* design = ship->Design()){
                    retval += design->Attack();
                }
                isFleetScrapped = false;
            }
        }
    }

    if (isFleetScrapped)
        retval = 0.0f;

    return retval;
}

float Fleet::Structure() const {
    if (m_ships.empty())
        return 0.0f;

    bool isFleetScrapped = true;
    float retval = 0.0f;
    for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
        if (TemporaryPtr<const Ship> ship = GetShip(*it)) {
            if (!ship->OrderedScrapped()) {
                retval += ship->CurrentMeterValue(METER_STRUCTURE);
                isFleetScrapped = false;
            }
        }
    }

    if (isFleetScrapped)
        retval = 0.0f;

    return retval;
}

float Fleet::Shields() const {
    if (m_ships.empty())
        return 0.0f;

    bool isFleetScrapped = true;
    float retval = 0.0f;
    for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
        if (TemporaryPtr<const Ship> ship = GetShip(*it)) {
            if (!ship->OrderedScrapped()) {
                retval += ship->CurrentMeterValue(METER_SHIELD);
                isFleetScrapped = false;
            }
        }
    }

    if (isFleetScrapped)
        retval = 0.0f;

    return retval;
}

void Fleet::ShortenRouteToEndAtSystem(std::list<int>& travel_route, int last_system) {
    std::list<int>::iterator visible_end_it;
    if (last_system != m_moving_to) {
        visible_end_it = std::find(m_travel_route.begin(), m_travel_route.end(), last_system);

        // if requested last system not in route, do nothing
        if (visible_end_it == m_travel_route.end())
            return;

        ++visible_end_it;

    } else {
        visible_end_it = m_travel_route.end();
    }

    // remove any extra systems from the route after the apparent destination
    std::list<int>::iterator end_it = std::find_if(m_travel_route.begin(), visible_end_it, boost::bind(&SystemHasNoVisibleStarlanes, _1, this->Owner()));
    std::copy(m_travel_route.begin(), end_it, std::back_inserter(travel_route));

    // If no Systems in a nonempty route are known reachable, default to just
    // showing the next system on the route.
    if (travel_route.empty() && !m_travel_route.empty())
        travel_route.push_back(*m_travel_route.begin());
}

std::string Fleet::GenerateFleetName(const std::vector<int>& ship_ids, int new_fleet_id) {
    // TODO: Change returned name based on passed ship designs.  eg. return "colony fleet" if
    // ships are colony ships, or "battle fleet" if ships are armed.
    if (new_fleet_id == INVALID_OBJECT_ID)
        return UserString("NEW_FLEET_NAME_NO_NUMBER");

    return boost::io::str(FlexibleFormat(UserString("NEW_FLEET_NAME")) % boost::lexical_cast<std::string>(new_fleet_id));
}

Fleet::ShipIDSet Fleet::VisibleContainedObjects(int empire_id) const {
    ShipIDSet retval;
    const Universe& universe = GetUniverse();
    for (ShipIDSet::const_iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
        int object_id = *it;
        if (universe.GetObjectVisibilityByEmpire(object_id, empire_id) >= VIS_BASIC_VISIBILITY)
            retval.insert(object_id);
    }
    return retval;
}
