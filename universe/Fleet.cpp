#include "Fleet.h"

#include "Pathfinder.h"
#include "ShipDesign.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObjectVisitor.h"
#include "Universe.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../Empire/Supply.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"
#include "../util/ScopedTimer.h"
#include "../util/i18n.h"

namespace {
    constexpr bool ALLOW_ALLIED_SUPPLY = true;

    const std::set<int> EMPTY_SET;
    constexpr double MAX_SHIP_SPEED = 500.0;        // max allowed speed of ship movement

    void MoveFleetWithShips(Fleet& fleet, double x, double y, ObjectMap& objects) {
        fleet.MoveTo(x, y);
        for (auto* ship : objects.findRaw<Ship>(fleet.ShipIDs()))
            ship->MoveTo(x, y);
    }

    void InsertFleetWithShips(Fleet& fleet, System& system, ObjectMap& objects, int current_turn) {
        system.Insert(fleet.shared_from_this(), System::NO_ORBIT, current_turn, objects);
        for (auto* ship : objects.findRaw<Ship>(fleet.ShipIDs()))
            system.Insert(ship, System::NO_ORBIT, current_turn, objects);
    }

    /** Return \p full_route terminates at \p last_system or before the first
      * system not known to the \p empire_id. If \a last_system is INVALID_OBJECT_ID,
      * returns an empty route. */
    std::vector<int> TruncateRouteToEndAtSystem(const std::vector<int>& full_route,
                                                const Universe& universe, int last_system)
    {
        if (full_route.empty() || (last_system == INVALID_OBJECT_ID))
            return {};

        auto visible_end_it = full_route.cend();
        if (last_system != full_route.back()) {
            visible_end_it = std::find(full_route.begin(), full_route.end(), last_system);

            // if requested last system not in route, do nothing
            if (visible_end_it == full_route.end())
                return {};

            ++visible_end_it;
        }

        // Remove any extra systems from the route after the apparent destination.
        //
        // It is enforced on the server, in the visibility calculations, that an
        // owning empire knows about:
        // a) the system containing an owned fleet
        // b) the starlane on which an owned fleet is travelling
        // c) both systems terminating a starlane on which an owned fleet is travelling.
        auto has_no_visible_starlanes = [&universe](int system_id)
        { return !universe.GetPathfinder()->SystemHasVisibleStarlanes(system_id, universe.Objects()); };
        auto end_it = std::find_if(full_route.begin(), visible_end_it, has_no_visible_starlanes);

        return {full_route.begin(), end_it};
    }
}


Fleet::Fleet(std::string name, double x, double y, int owner_id, int creation_turn) :
    UniverseObject{UniverseObjectType::OBJ_FLEET, std::move(name), owner_id, creation_turn}
{ UniverseObject::Init(); }

std::shared_ptr<UniverseObject> Fleet::Clone(const Universe& universe, int empire_id) const {
    Visibility vis = universe.GetObjectVisibilityByEmpire(this->ID(), empire_id);

    if (!(vis >= Visibility::VIS_BASIC_VISIBILITY && vis <= Visibility::VIS_FULL_VISIBILITY))
        return nullptr;

    auto retval = std::make_shared<Fleet>();
    retval->Copy(*this, universe, empire_id);
    return retval;
}

void Fleet::Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id) {
    if (&copied_object == this)
        return;
    if (copied_object.ObjectType() != UniverseObjectType::OBJ_FLEET) {
        ErrorLogger() << "Fleet::Copy passed an object that wasn't a Fleet";
        return;
    }

    Copy(static_cast<const Fleet&>(copied_object), universe, empire_id);
}

void Fleet::Copy(const Fleet& copied_fleet, const Universe& universe, int empire_id) {
    if (&copied_fleet == this)
        return;

    const int copied_object_id = copied_fleet.ID();
    const Visibility vis = universe.GetObjectVisibilityByEmpire(copied_object_id, empire_id);
    const auto visible_specials = universe.GetObjectVisibleSpecialsByEmpire(copied_object_id, empire_id);

    UniverseObject::Copy(copied_fleet, vis, visible_specials, universe);

    if (vis >= Visibility::VIS_BASIC_VISIBILITY) {
        m_ships =               copied_fleet.VisibleContainedObjectIDs(empire_id, universe.GetEmpireObjectVisibility());

        m_next_system =         ((universe.EmpireKnownObjects(empire_id).getRaw<System>(copied_fleet.m_next_system))
                                    ? copied_fleet.m_next_system : INVALID_OBJECT_ID);
        m_prev_system =         ((universe.EmpireKnownObjects(empire_id).getRaw<System>(copied_fleet.m_prev_system))
                                    ? copied_fleet.m_prev_system : INVALID_OBJECT_ID);
        m_arrived_this_turn =   copied_fleet.m_arrived_this_turn;
        m_arrival_starlane =    copied_fleet.m_arrival_starlane;

        if (vis >= Visibility::VIS_PARTIAL_VISIBILITY) {
            m_aggression =      copied_fleet.m_aggression;
            if (Unowned())
                m_name =        copied_fleet.m_name;

            // Truncate the travel route to only systems known to empire_id
            int moving_to = (vis >= Visibility::VIS_FULL_VISIBILITY
                             ? (!copied_fleet.m_travel_route.empty()
                                ? copied_fleet.m_travel_route.back()
                                : INVALID_OBJECT_ID)
                             : m_next_system);

            m_travel_route = TruncateRouteToEndAtSystem(copied_fleet.m_travel_route, universe, moving_to);


            if (vis >= Visibility::VIS_FULL_VISIBILITY) {
                m_ordered_given_to_empire_id =  copied_fleet.m_ordered_given_to_empire_id;
                m_last_turn_move_ordered =      copied_fleet.m_last_turn_move_ordered;
            }
        }
    }
}

bool Fleet::HostileToEmpire(int empire_id, const EmpireManager& empires) const {
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
           empires.GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

std::string Fleet::Dump(uint8_t ntabs) const {
    std::string retval = UniverseObject::Dump(ntabs);
    retval.reserve(2048);
    retval.append(" aggression: ").append(to_string(m_aggression))
          .append(" cur system: ").append(std::to_string(SystemID()))
          .append(" moving to: ").append(std::to_string(FinalDestinationID()))
          .append(" prev system: ").append(std::to_string(m_prev_system))
          .append(" next system: ").append(std::to_string(m_next_system))
          .append(" arrival lane: ").append(std::to_string(m_arrival_starlane))
          .append(" ships: ");
    for (auto it = m_ships.begin(); it != m_ships.end();) {
        int ship_id = *it;
        ++it;
        retval.append(std::to_string(ship_id)).append(it == m_ships.end() ? "" : ", ");
    }
    return retval;
}

bool Fleet::Contains(int object_id) const
{ return object_id != INVALID_OBJECT_ID && m_ships.contains(object_id); }

bool Fleet::ContainedBy(int object_id) const
{ return object_id != INVALID_OBJECT_ID && this->SystemID() == object_id; }

const std::string& Fleet::PublicName(int empire_id, const Universe& universe) const {
    // Disclose real fleet name only to fleet owners.
    if (empire_id == ALL_EMPIRES || OwnedBy(empire_id)) // TODO: GameRule for all objets visible
        return Name();
    else if (!Unowned())
        return UserString("FW_FOREIGN_FLEET");
    else if (Unowned() && HasMonsters(universe))
        return UserString("MONSTERS");
    else if (Unowned() && GetVisibility(empire_id, universe) > Visibility::VIS_NO_VISIBILITY)
        return UserString("FW_ROGUE_FLEET");
    else
        return UserString("OBJ_FLEET");
}

int Fleet::MaxShipAgeInTurns(const ObjectMap& objects, int current_turn) const {
    if (m_ships.empty())
        return INVALID_OBJECT_AGE;

    bool fleet_is_scrapped = true;
    int retval = 0;
    for (const auto* ship : objects.findRaw<Ship>(m_ships)) {
        if (!ship || ship->OrderedScrapped())
            continue;
        if (ship->AgeInTurns(current_turn) > retval)
            retval = ship->AgeInTurns(current_turn);
        fleet_is_scrapped = false;
    }

    if (fleet_is_scrapped)
        retval = 0;

    return retval;
}

std::vector<MovePathNode> Fleet::MovePath(bool flag_blockades, const ScriptingContext& context) const
{ return MovePath(TravelRoute(), flag_blockades, context); }

std::vector<MovePathNode> Fleet::MovePath(const std::vector<int>& route, bool flag_blockades,
                                          const ScriptingContext& context) const
{
    std::vector<MovePathNode> retval;

    if (route.empty())
        return retval; // nowhere to go => empty path
    // if (route.size() == 1) do nothing special.  this fleet is probably on the starlane leading to
    //                        its final destination.  normal looping to read destination should work fine
    if (route.size() == 2 && route.front() == route.back())
        return retval; // nowhere to go => empty path
    if (this->Speed(context.ContextObjects()) < FLEET_MOVEMENT_EPSILON) { // TODO: cache speed and reuse later in function?
        retval.emplace_back(this->X(), this->Y(), true, ETA_NEVER,
                            this->SystemID(),
                            INVALID_OBJECT_ID,
                            INVALID_OBJECT_ID,
                            false);
        return retval; // can't move => path is just this system with explanatory ETA
    }

    float fuel = Fuel(context.ContextObjects());
    float max_fuel = MaxFuel(context.ContextObjects());

    auto RouteNums = [route]() {
        std::string retval;
        retval.reserve(route.size() * 8);
        for (int waypoint : route)
            retval.append(std::to_string(waypoint)).append(" ");
        return retval;
    };
    TraceLogger() << "Fleet::MovePath for Fleet " << this->Name() << " (" << this->ID()
                  << ") fuel: " << fuel << " at sys id: " << this->SystemID() << "  route: "
                  << RouteNums();

    // determine all systems where fleet(s) can be resupplied if fuel runs out
    auto empire = context.GetEmpire(this->Owner());
    const auto fleet_supplied_systems = context.supply.FleetSupplyableSystemIDs(
        this->Owner(), ALLOW_ALLIED_SUPPLY, context);
    auto& unobstructed_systems = empire ? empire->SupplyUnobstructedSystems() : EMPTY_SET;

    // determine if, given fuel available and supplyable systems, fleet will ever be able to move
    if (fuel < 1.0f &&
        this->SystemID() != INVALID_OBJECT_ID &&
        std::none_of(fleet_supplied_systems.begin(), fleet_supplied_systems.end(),
                     [sys_id{this->SystemID()}] (const auto fss) { return fss == sys_id; }))
    {
        // no fuel and out of supply => can't move => path is just this system with explanatory ETA
        retval.emplace_back(this->X(), this->Y(), true, ETA_OUT_OF_RANGE,
                            this->SystemID(),
                            INVALID_OBJECT_ID,
                            INVALID_OBJECT_ID);
        return retval;
    }


    // get iterator pointing to std::shared_ptr<System> on route that is the
    // first after where this fleet is currently. if this fleet is in a system,
    // the iterator will point to the system after the current in the route
    // if this fleet is not in a system, the iterator will point to the first
    // system in the route
    auto route_it = route.begin();
    if (*route_it == SystemID())
        ++route_it;     // first system in route is current system of this fleet.  skip to the next system
    if (route_it == route.end())
        return retval;  // current system of this fleet is the *only* system in the route.  path is empty.


    // get current, previous and next systems of fleet
    auto cur_system = context.ContextObjects().getRaw<System>(this->SystemID());          // may be 0
    auto prev_system = context.ContextObjects().getRaw<System>(this->PreviousSystemID()); // may be 0 if this fleet is not moving or ordered to move
    auto next_system = context.ContextObjects().getRaw<System>(*route_it);                // can't use this->NextSystemID() because this fleet may not be moving and may not have a next system. this might occur when a fleet is in a system, not ordered to move or ordered to move to a system, but a projected fleet move line is being calculated to a different system
    if (!next_system) {
        ErrorLogger() << "Fleet::MovePath couldn't get next system with id " << *route_it << " for fleet " << this->Name() << "(" << this->ID() << ")";
        return retval;
    }

    TraceLogger() << "Initial cur system: " << (cur_system ? cur_system->Name() : "(none)") << "(" << (cur_system ? cur_system->ID() : -1) << ")"
                  << "  prev system: " << (prev_system ? prev_system->Name() : "(none)") << "(" << (prev_system ? prev_system->ID() : -1) << ")"
                  << "  next system: " << (next_system ? next_system->Name() : "(none)") << "(" << (next_system ? next_system->ID() : -1) << ")";


    bool is_post_blockade = false;
    if (cur_system) {
        //DebugLogger() << "Fleet::MovePath starting in system "<< SystemID();
        if (flag_blockades) {
            if (BlockadedAtSystem(cur_system->ID(), next_system->ID(), context)) {
                // blockade debug logging
                TraceLogger() << "Fleet::MovePath checking blockade from "<< cur_system->ID() << " to "<< next_system->ID();
                TraceLogger() << "Fleet::MovePath finds system " << cur_system->Name() << " (" <<cur_system->ID()
                              << ") blockaded for fleet " << this->Name();
                is_post_blockade = true;
            } else {
                // blockade debug logging, but only for the more complex situations
                if (next_system->ID() != m_arrival_starlane && !unobstructed_systems.contains(cur_system->ID())) {
                    TraceLogger() << "Fleet::MovePath checking blockade from "<< cur_system->ID() << " to " << next_system->ID();
                    TraceLogger() << "Fleet::MovePath finds system " << cur_system->Name() << " (" << cur_system->ID()
                                  << ") NOT blockaded for fleet " << this->Name();
                }
            }
        }
    }
    // place initial position MovePathNode
    retval.reserve(route.size()*3); // rough guesstimate
    retval.emplace_back(this->X(), this->Y(), false, 0,
                        (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                        (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                        (next_system ? next_system->ID() : INVALID_OBJECT_ID),
                        false);


    static constexpr int TOO_LONG = 100; // limit on turns to simulate.  99 turns max keeps ETA to two digits, making UI work better
    int    turns_taken =         1;
    double turn_dist_remaining = this->Speed(context.ContextObjects()); // additional distance that can be travelled in current turn of fleet movement being simulated
    double cur_x =               this->X();
    double cur_y =               this->Y();
    double next_x =              next_system->X();
    double next_y =              next_system->Y();

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

        if (cur_system)
            TraceLogger() << "Starting iteration at system " << cur_system->Name() << " (" << cur_system->ID() << ")";
        else
            TraceLogger() << "Starting iteration at (" << cur_x << ", " << cur_y << ")";

        // Make sure that there actually still is a starlane between the two systems
        // we are between
        const System* const prev_or_cur = cur_system ? cur_system : prev_system ? prev_system : nullptr;
        if (prev_or_cur && !prev_or_cur->HasStarlaneTo(next_system->ID())) {
            DebugLogger() << "Fleet::MovePath for Fleet " << this->Name() << " (" << this->ID()
                            << ") No starlane connection between systems " << prev_or_cur->Name() << "(" << prev_or_cur->ID()
                            << ")  and " << next_system->Name() << "(" << next_system->ID()
                            << "). Abandoning the rest of the route. Route was: " << RouteNums();
            return retval;
        } else if (!prev_or_cur) {
            ErrorLogger() << "Fleet::MovePath: No previous or current system!?";
        }


        // check if fuel limits movement or current system refuels passing fleet
        if (cur_system) {
            // check if current system has fuel supply available
            if (std::any_of(fleet_supplied_systems.begin(), fleet_supplied_systems.end(),
                            [csid{cur_system->ID()}](const auto fss) { return csid == fss; }))
            {
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

            TraceLogger() << " ... arrived at system: " << cur_system->Name();

            bool clear_exit = cur_system->ID() == m_arrival_starlane; //just part of the test for the moment
            // attempt to get next system on route, to update next system.  if new current
            // system is the end of the route, abort.
            ++route_it;
            if (route_it != route.end()) {
                // update next system on route and distance to it from current position
                next_system = context.ContextObjects().getRaw<System>(*route_it); // TODO: filter by known objects for this->Owner()
                if (next_system) {
                    TraceLogger() << "Fleet::MovePath checking unrestriced lane travel from Sys("
                                  <<  cur_system->ID() << ") to Sys(" << (next_system && next_system->ID()) << ")";
                    clear_exit = clear_exit
                        || next_system->ID() == m_arrival_starlane
                        || (empire && empire->PreservedLaneTravel(cur_system->ID(), next_system->ID()));
                }
            }
            if (flag_blockades && !clear_exit) {
                TraceLogger() << "Fleet::MovePath checking blockades at system " << cur_system->Name()
                              << " (" << cur_system->ID() << ") for fleet " << this->Name()
                              << " travelling to system " << (*route_it);
                if (cur_system && next_system && BlockadedAtSystem(cur_system->ID(), next_system->ID(), context))
                {
                    // blockade debug logging
                    TraceLogger() << "Fleet::MovePath finds system " << cur_system->Name() << " (" << cur_system->ID()
                                  << ") blockaded for fleet " << this->Name();
                    is_post_blockade = true;
                } else {
                    TraceLogger() << "Fleet::MovePath finds system " << cur_system->Name() << " (" << cur_system->ID()
                                  << ") NOT blockaded for fleet " << this->Name();
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
        if (cur_system && !unobstructed_systems.contains(cur_system->ID())) {
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
        TraceLogger() << "Fleet::MovePath for fleet " << this->Name() << " id " << this->ID() << " adding node at sysID " 
                      << (cur_system ? cur_system->ID() : INVALID_OBJECT_ID) << " with post blockade status " 
                      << is_post_blockade << " and ETA " << turns_taken;

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
            turn_dist_remaining = this->Speed(context.ContextObjects());
        }
    }


    // done looping.  may have exited due to reaching end of path, lack of fuel, or turns taken getting too big
    if (turns_taken == TOO_LONG)
        turns_taken = ETA_NEVER;
    // blockade debug logging
    TraceLogger() << "Fleet::MovePath for fleet " << this->Name()<<" id "<<this->ID()<<" adding node at sysID "
                  << (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID) << " with post blockade status "
                  << is_post_blockade << " and ETA " << turns_taken;

    retval.emplace_back(cur_x, cur_y, true, turns_taken,
                        (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                        (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                        (next_system ? next_system->ID() : INVALID_OBJECT_ID),
                        is_post_blockade);
    TraceLogger() << "Fleet::MovePath for fleet " << this->Name() << "(" << this->ID() << ") is complete";

    return retval;
}

std::pair<int, int> Fleet::ETA(const ScriptingContext& context) const
{ return ETA(MovePath(false, context)); }

std::pair<int, int> Fleet::ETA(const std::vector<MovePathNode>& move_path) const {
    // check that path exists.  if empty, there was no valid route or some other problem prevented pathing
    if (move_path.empty())
        return {ETA_UNKNOWN, ETA_UNKNOWN};

    // check for single node in path.  return the single node's eta as both .first and .second (likely indicates that fleet couldn't move)
    if (move_path.size() == 1) {
        const MovePathNode& node = move_path.front();
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

float Fleet::Fuel(const ObjectMap& objects) const {
    if (NumShips() < 1)
        return 0.0f;

    // determine fuel available to fleet (fuel of the ship that has the least fuel in the fleet)
    float fuel = Meter::LARGE_VALUE;
    bool is_fleet_scrapped = true;

    for (auto& ship : objects.find<const Ship>(m_ships)) {
        const Meter* meter = ship->UniverseObject::GetMeter(MeterType::METER_FUEL);
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

float Fleet::MaxFuel(const ObjectMap& objects) const {
    if (NumShips() < 1)
        return 0.0f;

    // determine the maximum amount of fuel that can be stored by the ship in the fleet that
    // can store the least amount of fuel
    float max_fuel = Meter::LARGE_VALUE;
    bool is_fleet_scrapped = true;

    for (auto& ship : objects.find<const Ship>(m_ships)) {
        const Meter* meter = ship->UniverseObject::GetMeter(MeterType::METER_MAX_FUEL);
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
    if (m_travel_route.empty())
        return INVALID_OBJECT_ID;
    else
        return m_travel_route.back();
}

int Fleet::PreviousToFinalDestinationID() const {
    if (m_travel_route.empty())
        return INVALID_OBJECT_ID;
    else if (m_travel_route.size() == 1)
        return this->PreviousSystemID();
    else
        return *std::prev(std::prev(m_travel_route.end()));
}

namespace {
    template <typename ShipPredicate, typename IDContainer>
    bool HasXShips(const ShipPredicate& pred, const IDContainer& ship_ids, const ObjectMap& objects) {
        // Searching for each Ship one at a time is possibly faster than find(ship_ids),
        // because an early exit avoids searching the remaining ids.
        return std::any_of(ship_ids.begin(), ship_ids.end(),
                           [&pred, &objects](const int ship_id) {
                               const auto ship = objects.getRaw<const Ship>(ship_id);
                               return ship && pred(ship);
                           });
    }
}

bool Fleet::CanDamageShips(const ScriptingContext& context, float target_shields) const {
    auto isX = [target_shields, &context](const Ship* ship)
    { return ship->CanDamageShips(context, target_shields); };
    return HasXShips(isX, m_ships, context.ContextObjects());
}

bool Fleet::CanDestroyFighters(const ScriptingContext& context) const {
    auto isX = [&context](const Ship* ship)
    { return ship->CanDestroyFighters(context); };
    return HasXShips(isX, m_ships, context.ContextObjects());
}

bool Fleet::HasMonsters(const Universe& u) const {
    auto isX = [&u](const Ship* ship){ return ship->IsMonster(u); };
    return HasXShips(isX, m_ships, u.Objects());
}

bool Fleet::HasArmedShips(const ScriptingContext& context) const {
    auto isX = [&context](const Ship* ship){ return ship->IsArmed(context); };
    return HasXShips(isX, m_ships, context.ContextObjects());
}

bool Fleet::HasFighterShips(const Universe& u) const {
    auto isX = [&u](const Ship* ship){ return ship->HasFighters(u); };
    return HasXShips(isX, m_ships, u.Objects());
}

bool Fleet::HasColonyShips(const Universe& universe) const {
    auto isX = [&universe](const Ship* ship) {
        if (const auto design = universe.GetShipDesign(ship->DesignID()))
            if (design->ColonyCapacity() > 0.0f)
                return true;
        return false;
    };
    return HasXShips(isX, m_ships, universe.Objects());
}

bool Fleet::HasOutpostShips(const Universe& universe) const {
    auto isX = [&universe](const Ship* ship) {
        if (const auto design = universe.GetShipDesign(ship->DesignID()))
            if (design->CanColonize() && design->ColonyCapacity() == 0.0f)
                return true;
        return false;
    };
    return HasXShips(isX, m_ships, universe.Objects());
}

bool Fleet::HasTroopShips(const Universe& u) const {
    auto isX = [&u](const Ship* ship){ return ship->HasTroops(u); };
    return HasXShips(isX, m_ships, u.Objects());
}

bool Fleet::HasShipsOrderedScrapped(const Universe& u) const {
    auto isX = [](const Ship* ship){ return ship->OrderedScrapped(); };
    return HasXShips(isX, m_ships, u.Objects());
}

bool Fleet::HasShipsWithoutScrapOrders(const Universe& u) const {
    auto isX = [](const Ship* ship){ return !ship->OrderedScrapped(); };
    return HasXShips(isX, m_ships, u.Objects());
}

float Fleet::ResourceOutput(ResourceType type, const ObjectMap& objects) const {
    float output = 0.0f;
    if (NumShips() < 1)
        return output;
    MeterType meter_type = ResourceToMeter(type);
    if (meter_type == MeterType::INVALID_METER_TYPE)
        return output;

    // determine resource output of each ship in this fleet
    for (auto& ship : objects.find<const Ship>(m_ships))
        output += ship->GetMeter(meter_type)->Current();

    return output;
}

bool Fleet::UnknownRoute() const
{ return m_travel_route.size() == 1 && m_travel_route.front() == INVALID_OBJECT_ID; }

std::shared_ptr<UniverseObject> Fleet::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Fleet>(std::static_pointer_cast<const Fleet>(shared_from_this()))); }

void Fleet::SetRoute(std::vector<int> route, const ObjectMap& objects) {
    if (route.empty()) {
        if (SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "Fleet::SetRoute() : Attempted to change fleet " << this->Name()
                          << " (" << this->ID() << ") route to empty while not in a system.";
            return;
        }
    } else if (m_prev_system != SystemID() &&
               m_prev_system == route.front() &&
               !CanChangeDirectionEnRoute())
    {
        ErrorLogger() << "Fleet::SetRoute() : Illegally attempted to change fleet " << this->Name()
                      << " (" << this->ID() << ") direction while in transit.";
        return;
    }

    m_travel_route = std::move(route);

    TraceLogger() << "Fleet::SetRoute: " << this->Name() << " (" << this->ID() << ")  input: " << [&]() {
        std::stringstream ss;
        for (int id : m_travel_route)
            if (const auto obj = objects.getRaw<UniverseObject>(id))
                ss << obj->Name() << " (" << id << ")  ";
        return ss.str();
    }();

    if (m_travel_route.size() == 1 && this->SystemID() == m_travel_route.front()) {
        // Fleet was ordered to move to the system where it is currently,
        // without an intermediate stop
        m_travel_route.clear();
        m_prev_system = m_next_system = INVALID_OBJECT_ID;

    } else if (!m_travel_route.empty()) {
        // Fleet was given a route to follow...
        if (m_prev_system != SystemID() && m_prev_system == m_travel_route.front()) {
            // Fleet was ordered to return to its previous system directly
            m_prev_system = m_next_system;
        } else if (SystemID() == m_travel_route.front()) {
            // Fleet was ordered to follow a route that starts at its current system
            m_prev_system = SystemID();
        }

        auto it = m_travel_route.begin();
        if (m_prev_system == SystemID() && m_travel_route.size() > 1) {
            m_next_system = *++it;
        } else {
            m_next_system = *it;
        }

    } else if (m_travel_route.empty() && this->SystemID() != INVALID_OBJECT_ID) {
        // route is empty and fleet is in a system. no movement needed.
        // ensure next and previous systems are reset
        m_prev_system = m_next_system = INVALID_OBJECT_ID;

    } else {    // m_travel_route.empty() && this->SystemID() == INVALID_OBJECT_ID
        if (m_next_system != INVALID_OBJECT_ID) {
            ErrorLogger() << "Fleet::SetRoute fleet " << this->Name() << " has empty route but fleet is not in a system. Resetting route to end at next system: " << m_next_system;
            m_travel_route.push_back(m_next_system);
        } else {
            ErrorLogger() << "Fleet::SetRoute fleet " << this->Name() << " has empty route but fleet is not in a system, and has no next system set.";
        }
    }

    TraceLogger() << "Fleet::SetRoute: " << this->Name() << " (" << this->ID() << ")  final: " << [&]() {
        std::stringstream ss;
        for (int id : m_travel_route)
            if (const auto obj = objects.getRaw<UniverseObject>(id))
                ss << obj->Name() << " (" << id << ")  ";
        return ss.str();
    }();

    StateChangedSignal();
}

void Fleet::SetAggression(FleetAggression aggression) {
    if (m_aggression == aggression)
        return;
    m_aggression = aggression;
    StateChangedSignal();
}

void Fleet::AddShips(const std::vector<int>& ship_ids) {
    auto old_ships_size = m_ships.size();
    m_ships.insert(ship_ids.begin(), ship_ids.end());
    if (old_ships_size != m_ships.size())
        StateChangedSignal();
}

void Fleet::RemoveShips(const std::vector<int>& ship_ids) {
    auto old_ships_size = m_ships.size();
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

void Fleet::MovementPhase(ScriptingContext& context) {
    auto empire = context.GetEmpire(Owner());
    std::set<int> supply_unobstructed_systems;
    if (empire)
        supply_unobstructed_systems.insert(empire->SupplyUnobstructedSystems().begin(),
                                           empire->SupplyUnobstructedSystems().end());

    auto& objects = context.ContextObjects();
    const auto& universe = context.ContextUniverse();
    const auto& supply = context.supply;

    auto ships = objects.findRaw<Ship>(m_ships);

    // if owner of fleet can resupply ships at the location of this fleet, then
    // resupply all ships in this fleet
    if (supply.SystemHasFleetSupply(SystemID(), Owner(), ALLOW_ALLIED_SUPPLY,
                                    context.diplo_statuses))
    {
        for (auto& ship : ships)
            ship->Resupply(context.current_turn);
    }

    auto current_system = objects.getRaw<System>(SystemID());
    auto initial_system = current_system;
    auto move_path = MovePath(false, context);

    if (!move_path.empty()) {
        DebugLogger() << "Fleet::MovementPhase " << this->Name() << " (" << this->ID()
                      << ")  route:" << [&]() {
            std::string ss;
            ss.reserve(this->TravelRoute().size() * 32); // guesstimate
            for (auto sys_id : this->TravelRoute()) {
                if (auto sys = objects.getRaw<const System>(sys_id))
                    ss.append("  ").append(sys->Name()).append(" (")
                      .append(std::to_string(sys_id)).append(")");
                else
                    ss.append("  (?) (").append(std::to_string(sys_id)).append(")");
            }
            return ss;
        }()
                      << "   move path:" << [&]() {
            std::string ss;
            ss.reserve(move_path.size() * 32); // guesstimate
            for (const auto& node : move_path) {
                if (auto sys = context.ContextObjects().getRaw<const System>(node.object_id))
                    ss.append("  ").append(sys->Name()).append(" (")
                      .append(std::to_string(node.object_id)).append(")");
                else
                    ss.append("  (-)");
            }
            return ss;
        }();
    } else {
        // enforce m_next_system and m_prev_system being INVALID_OBJECT_ID when
        // move path is empty. bug was reported where m_next_system was somehow
        // left with a system ID in it, which was never reset, and lead to
        // supply propagation issues
        m_next_system = m_prev_system = INVALID_OBJECT_ID;
    }

    // If the move path cannot lead to the destination,
    // make the route go as far as it can
    if (!move_path.empty() && !m_travel_route.empty() &&
         move_path.back().object_id != m_travel_route.back())
    {
        const int back_id = move_path.back().object_id;
        auto shortened_route = TruncateRouteToEndAtSystem(m_travel_route, universe, back_id);
        try {
            SetRoute(std::move(shortened_route), objects);
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception in Fleet MovementPhase shorentning route: " << e.what();
        }
        move_path = MovePath(false, context);
    }

    auto it = move_path.begin();
    auto next_it = it;
    if (next_it != move_path.end())
        ++next_it;


    // is the fleet stuck in a system for a whole turn?
    if (current_system) {
        // update m_arrival_starlane if no blockade, if needed
        if (supply_unobstructed_systems.contains(SystemID()))
            m_arrival_starlane = SystemID();// allows departure via any starlane

        // in a system.  if either:
        // - there is no system after the current one in the path
        // - the current and next nodes are the same and are system IDs or actual systems (not empty space)
        // - this fleet is blockaded from its intended path
        // then this fleet won't be moving further this turn
        bool stopped = false;

        if (next_it == move_path.end()) {
            // at end of path
            stopped = true;

        } else if (it->object_id != INVALID_OBJECT_ID && it->object_id == next_it->object_id) {
            // arriving at system
            stopped = true;

        } else if (m_arrival_starlane != SystemID()) {
            // blockaded
            int next_sys_id;
            if (next_it->object_id != INVALID_OBJECT_ID) {
                next_sys_id = next_it->object_id;
            } else {
                next_sys_id = next_it->lane_end_id;
            }
            stopped = BlockadedAtSystem(SystemID(), next_sys_id, context);
        }

        if (stopped)
            return;

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


    // if fleet not moving, nothing more to do.
    if (move_path.empty()) {
        m_next_system = m_prev_system = INVALID_OBJECT_ID;
        return;
    }


    // move fleet in sequence to MovePathNodes it can reach this turn
    float fuel_consumed = 0.0f;
    for (it = move_path.begin(); it != move_path.end(); ++it) {
        next_it = it;   ++next_it;

        auto system = objects.getRaw<System>(it->object_id);

        // is this system the last node reached this turn?  either it's an end of turn node,
        // or there are no more nodes after this one on path
        bool node_is_next_stop = (it->turn_end || next_it == move_path.end());


        if (system) {
            // node is a system.  explore system for all owners of this fleet
            if (empire) {
                empire->AddExploredSystem(it->object_id, context.current_turn, context.ContextObjects());
                empire->RecordPendingLaneUpdate(it->object_id, m_prev_system, context.ContextObjects()); // specifies the lane from it->object_id back to m_prev_system is available
            }

            m_prev_system = system->ID(); // passing a system, so update previous system of this fleet

            // reached a system, so remove it from the route
            if (m_travel_route.front() == system->ID())
                m_travel_route.erase(m_travel_route.begin());

            bool resupply_here = supply.SystemHasFleetSupply(
                system->ID(), this->Owner(), ALLOW_ALLIED_SUPPLY, context.diplo_statuses);

            // if this system can provide supplies, reset consumed fuel and refuel ships
            if (resupply_here) {
                //DebugLogger() << " ... node has fuel supply.  consumed fuel for movement reset to 0 and fleet resupplied";
                fuel_consumed = 0.0f;
                for (auto& ship : ships)
                    ship->Resupply(context.current_turn);
            }


            // is system the last node reached this turn?
            if (node_is_next_stop) {
                // fleet ends turn at this node.  insert fleet and ships into system
                InsertFleetWithShips(*this, *system, objects, context.current_turn);

                current_system = system;

                if (supply_unobstructed_systems.contains(SystemID()))
                    m_arrival_starlane = SystemID(); // allows departure via any starlane

                // Add current system to the start of any existing route for next turn
                if (!m_travel_route.empty() && m_travel_route.front() != SystemID())
                    m_travel_route.insert(m_travel_route.begin(), SystemID());

                break;

            } else {
                // fleet will continue past this system this turn.
                m_arrival_starlane = m_prev_system;
                if (!resupply_here)
                    fuel_consumed += 1.0f;
            }

        } else {
            // node is not a system.
            m_arrival_starlane = m_prev_system;
            if (node_is_next_stop) { // node is not a system, but is it the last node reached this turn?
                MoveFleetWithShips(*this, it->x, it->y, objects);
                break;
            }
        }
    }


    // update next system
    if (!m_travel_route.empty() && next_it != move_path.end() && it != move_path.end()) {
        // there is another system later on the path to aim for.  find it
        for (; next_it != move_path.end(); ++next_it) {
            if (objects.get<System>(next_it->object_id)) {
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
            if (Meter* meter = ship->UniverseObject::GetMeter(MeterType::METER_FUEL)) {
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
    if (Meter* stealth = GetMeter(MeterType::METER_STEALTH)) {
        stealth->ResetCurrent();
        stealth->AddToCurrent(2000.0f);
    }
}

void Fleet::CalculateRouteTo(int target_system_id, const Universe& universe) {
    const ObjectMap& objects = universe.Objects();

    //DebugLogger() << "Fleet::CalculateRoute";
    if (target_system_id == INVALID_OBJECT_ID) {
        try {
            ClearRoute(objects);
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception in Fleet CalculateRouteTo: " << e.what();
        }
        return;
    }

    if (m_prev_system != INVALID_OBJECT_ID && SystemID() == m_prev_system) {
        // if we haven't actually left yet, we have to move from whichever system we are at now

        if (!objects.get<System>(target_system_id)) {
            // destination system doesn't exist or doesn't exist in known universe, so can't move to it.  leave route empty.
            try {
                ClearRoute(objects);
            } catch (const std::exception& e) {
                ErrorLogger() << "Caught exception in Fleet CalculateRouteTo: " << e.what();
            }
            return;
        }

        std::pair<std::vector<int>, double> path;
        try {
            path = universe.GetPathfinder()->ShortestPath(m_prev_system, target_system_id, this->Owner(), objects);
        } catch (...) {
            DebugLogger() << "Fleet::CalculateRoute couldn't find route to system(s):"
                          << " fleet's previous: " << m_prev_system << " or moving to: " << target_system_id;
        }
        try {
            SetRoute(std::move(path.first), objects);
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception in Fleet CalculateRouteTo: " << e.what();
        }
        return;
    }

    const int dest_system_id = target_system_id;

    // if we're between systems, the shortest route may be through either one
    if (this->CanChangeDirectionEnRoute()) {
        std::pair<std::vector<int>, double> path1;
        try {
            path1 = universe.GetPathfinder()->ShortestPath(m_next_system, dest_system_id, this->Owner(), objects);
        } catch (...) {
            DebugLogger() << "Fleet::CalculateRoute couldn't find route to system(s):"
                          << " fleet's next: " << m_next_system << " or destination: " << dest_system_id;
        }
        auto& sys_list1 = path1.first;
        if (sys_list1.empty()) {
            ErrorLogger() << "Fleet::CalculateRoute got empty route from ShortestPath";
            return;
        }
        auto obj = objects.getRaw(sys_list1.front());
        if (!obj) {
            ErrorLogger() << "Fleet::CalculateRoute couldn't get path start object with id " << path1.first.front();
            return;
        }
        double dist_x = obj->X() - this->X();
        double dist_y = obj->Y() - this->Y();
        const double dist1 = std::sqrt(dist_x*dist_x + dist_y*dist_y);

        std::pair<std::vector<int>, double> path2;
        try {
            path2 = universe.GetPathfinder()->ShortestPath(m_prev_system, dest_system_id, this->Owner(), objects);
        } catch (...) {
            DebugLogger() << "Fleet::CalculateRoute couldn't find route to system(s):"
                          << " fleet's previous: " << m_prev_system << " or destination: " << dest_system_id;
        }
        auto& sys_list2 = path2.first;
        if (sys_list2.empty()) {
            ErrorLogger() << "Fleet::CalculateRoute got empty route from ShortestPath";
            return;
        }
        obj = objects.getRaw(sys_list2.front());
        if (!obj) {
            ErrorLogger() << "Fleet::CalculateRoute couldn't get path start object with id " << path2.first.front();
            return;
        }
        dist_x = obj->X() - this->X();
        dist_y = obj->Y() - this->Y();
        const double dist2 = std::sqrt(dist_x*dist_x + dist_y*dist_y);

        try {
            // pick whichever path is quicker
            if (dist1 + path1.second < dist2 + path2.second)
                SetRoute(std::move(path1.first), objects);
            else
                SetRoute(std::move(path2.first), objects);
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception in Fleet CalculateRouteTo: " << e.what();
        }

    } else {
        // Cannot change direction. Must go to the end of the current starlane
        std::pair<std::vector<int>, double> path;
        try {
            path = universe.GetPathfinder()->ShortestPath(m_next_system, dest_system_id, this->Owner(), objects);
        } catch (...) {
            DebugLogger() << "Fleet::CalculateRoute couldn't find route to system(s):"
                          << " fleet's next: " << m_next_system << " or destination: " << dest_system_id;
        }
        try {
            SetRoute(std::move(path).first, objects);
        } catch (const std::exception& e) {
            ErrorLogger() << "Caught exception in Fleet CalculateRouteTo: " << e.what();
        }
    }
}

bool Fleet::Blockaded(const ScriptingContext& context) const {
    auto system = context.ContextObjects().get<System>(this->SystemID());

    if (!system)
        return false;

    if (m_next_system != INVALID_OBJECT_ID)
        return BlockadedAtSystem(SystemID(), m_next_system, context);

    for (const auto& target_system : system->StarlanesWormholes()) {
        if (BlockadedAtSystem(this->SystemID(), target_system.first, context))
            return true;
    }

    return false;
}

bool Fleet::BlockadedAtSystem(int start_system_id, int dest_system_id,
                              const ScriptingContext& context) const
{
    /** If a newly arrived fleet joins a non-blockaded fleet of the same empire
      * (perhaps should include allies?) already at the system, the newly
      * arrived fleet will not be blockaded.  Additionally, since fleets are
      * blockade-checked at movement phase and also postcombat, the following
      * tests mean that post-compbat, this fleet will be blockaded iff it was
      * blockaded pre-combat AND there are armed aggressive enemies surviving in
      * system post-combat which can detect this fleet.  Fleets arriving at the
      * same time do not blockade each other. Unrestricted lane access (i.e,
      * (fleet->ArrivalStarlane() == system->ID()) ) is used as a proxy for
      * order of arrival -- if an enemy has unrestricted lane access and you
      * don't, they must have arrived before you, or be in cahoots with someone
      * who did. */
    const ObjectMap& objects = context.ContextObjects();

    if (m_arrival_starlane == start_system_id) {
        //DebugLogger() << "Fleet::BlockadedAtSystem fleet " << ID() << " has cleared blockade flag for system (" << start_system_id << ")";
        return false;
    }
    bool not_yet_in_system = SystemID() != start_system_id;

    if (!not_yet_in_system && m_arrival_starlane == dest_system_id)
        return false;

    // find which empires have blockading aggressive armed ships in system;
    // fleets that just arrived do not blockade by themselves, but may
    // reinforce a preexisting blockade, and may possibly contribute to detection
    auto current_system = objects.get<System>(start_system_id);
    if (!current_system) {
        DebugLogger() << "Fleet::BlockadedAtSystem fleet " << ID() << " considering system (" << start_system_id
                      << ") but can't retrieve system copy";
        return false;
    }

    auto empire = context.GetEmpire(this->Owner());
    if (empire) {
        if (empire->SupplyUnobstructedSystems().contains(start_system_id))
            return false;
        if (empire->PreservedLaneTravel(start_system_id, dest_system_id)) {
            return false;
        } else {
            TraceLogger() << "Fleet::BlockadedAtSystem fleet " << ID() << " considering travel from system (" << start_system_id << ") to system (" << dest_system_id << ")";
        }
    }

    float lowest_ship_stealth = 99999.9f; // arbitrary large number. actual stealth of ships should be less than this...
    for (auto& ship : objects.find<const Ship>(this->ShipIDs())) {
        float ship_stealth = ship->GetMeter(MeterType::METER_STEALTH)->Current();
        if (lowest_ship_stealth > ship_stealth)
            lowest_ship_stealth = ship_stealth;
    }

    float monster_detection = 0.0f;
    auto fleets = objects.find<const Fleet>(current_system->FleetIDs());
    for (auto& fleet : fleets) {
        if (!fleet->Unowned())
            continue;

        for (auto& ship : objects.find<const Ship>(fleet->ShipIDs())) {
            float cur_detection = ship->GetMeter(MeterType::METER_DETECTION)->Current();
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
            can_see = (context.GetEmpire(fleet->Owner())->GetMeter("METER_DETECTION_STRENGTH")->Current() >= lowest_ship_stealth);
        else
            can_see = (monster_detection >= lowest_ship_stealth);
        if (!can_see)
            continue;

        bool at_war = Unowned() || fleet->Unowned() ||
                      context.ContextDiploStatus(this->Owner(), fleet->Owner()) == DiplomaticStatus::DIPLO_WAR;
        if (!at_war)
            continue;
        bool obstructive = (fleet->Obstructive() || fleet->Unowned());
        if (!obstructive)
            continue;
        // Newly created ships/monsters are not allowed to block other fleet movement since they have not even
        // potentially gone through a combat round at the present location.  Potential sources for such new ships are
        // monsters created via Effect and Ships/fleets newly constructed by empires.  We check ship ages not fleet
        // ageas since fleets can be created/destroyed as purely organizational matters.  Since these checks are
        // pertinent just during those stages of turn processing immediately following turn number advancement,
        // whereas the new ships were created just prior to turn advamcenemt, we require age greater than 1.
        if (fleet->MaxShipAgeInTurns(objects, context.current_turn) <= 1)
            continue;
        // These are the most costly checks.  Do them last
        if (!fleet->CanDamageShips(context))
            continue;

        // don't exit early here, because blockade may yet be thwarted by ownership & presence check above
        can_be_blockaded = true;
    }

    return can_be_blockaded;
}

float Fleet::Speed(const ObjectMap& objects) const {
    if (m_ships.empty())
        return 0.0f;

    bool fleet_is_scrapped = true;
    float retval = MAX_SHIP_SPEED;  // max speed no ship can go faster than
    for (const auto& ship : objects.find<Ship>(m_ships)) {
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

float Fleet::Damage(const Universe& universe) const {
    if (m_ships.empty())
        return 0.0f;

    bool fleet_is_scrapped = true;
    float retval = 0.0f;
    for (const auto& ship : universe.Objects().find<Ship>(m_ships)) {
        if (!ship || ship->OrderedScrapped())
            continue;
        if (const auto design = universe.GetShipDesign(ship->DesignID()))
            retval += design->Attack();
        fleet_is_scrapped = false;
    }

    if (fleet_is_scrapped)
        retval = 0.0f;

    return retval;
}

float Fleet::Structure(const ObjectMap& objects) const {
    if (m_ships.empty())
        return 0.0f;

    bool fleet_is_scrapped = true;
    float retval = 0.0f;
    for (const auto& ship : objects.find<Ship>(m_ships)) {
        if (!ship || ship->OrderedScrapped())
            continue;
        retval += ship->GetMeter(MeterType::METER_STRUCTURE)->Current();
        fleet_is_scrapped = false;
    }

    if (fleet_is_scrapped)
        retval = 0.0f;

    return retval;
}

float Fleet::Shields(const ObjectMap& objects) const {
    if (m_ships.empty())
        return 0.0f;

    bool fleet_is_scrapped = true;
    float retval = 0.0f;
    for (const auto& ship : objects.find<Ship>(m_ships)) {
        if (!ship || ship->OrderedScrapped())
            continue;
        retval += ship->GetMeter(MeterType::METER_SHIELD)->Current();
        fleet_is_scrapped = false;
    }

    if (fleet_is_scrapped)
        retval = 0.0f;

    return retval;
}

std::string Fleet::GenerateFleetName(const ScriptingContext& context) {
    // TODO: Change returned name based on passed ship designs.  eg. return "colony fleet" if
    // ships are colony ships, or "battle fleet" if ships are armed.
    if (ID() == INVALID_OBJECT_ID)
        return UserString("NEW_FLEET_NAME_NO_NUMBER");

    const Universe& u{context.ContextUniverse()};
    const SpeciesManager& sm{context.species};
    const ObjectMap& objects = u.Objects();
    auto ships = objects.find<Ship>(m_ships);

    // todo: rewrite with a lambda and store in a const string& to avoid copies...
    std::string fleet_name_key = UserStringNop("NEW_FLEET_NAME");

    auto IsCombatShip = [&context, &u](const auto& ship) {
        return ship.IsArmed(context) || ship.HasFighters(u) ||
               ship.CanHaveTroops(u) || ship.CanBombard(u);
    };

    if (std::all_of(ships.begin(), ships.end(), [&u](const auto& ship){ return ship->IsMonster(u); }))
        fleet_name_key = UserStringNop("NEW_MONSTER_FLEET_NAME");
    else if (std::all_of(ships.begin(), ships.end(), [&u, &sm](const auto& ship){ return ship->CanColonize(u, sm); }))
        fleet_name_key = UserStringNop("NEW_COLONY_FLEET_NAME");
    else if (std::all_of(ships.begin(), ships.end(), [&IsCombatShip](const auto& ship){ return !IsCombatShip(*ship); }))
        fleet_name_key = UserStringNop("NEW_RECON_FLEET_NAME");
    else if (std::all_of(ships.begin(), ships.end(), [&u](const auto& ship){ return ship->CanHaveTroops(u); }))
        fleet_name_key = UserStringNop("NEW_TROOP_FLEET_NAME");
    else if (std::all_of(ships.begin(), ships.end(), [&u](const auto& ship){ return ship->CanBombard(u); }))
        fleet_name_key = UserStringNop("NEW_BOMBARD_FLEET_NAME");
    else if (std::all_of(ships.begin(), ships.end(), [&IsCombatShip](const auto& ship){ return IsCombatShip(*ship); }))
        fleet_name_key = UserStringNop("NEW_BATTLE_FLEET_NAME");

    return boost::io::str(FlexibleFormat(UserString(fleet_name_key)) % ID());
}

void Fleet::SetGiveToEmpire(int empire_id) {
    if (empire_id == m_ordered_given_to_empire_id) return;
    m_ordered_given_to_empire_id = empire_id;
    StateChangedSignal();
}

void Fleet::ClearGiveToEmpire()
{ SetGiveToEmpire(ALL_EMPIRES); }

void Fleet::SetMoveOrderedTurn(int turn)
{ m_last_turn_move_ordered = turn; }
