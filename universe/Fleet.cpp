#include "Fleet.h"

#include "Ship.h"
#include "Predicates.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <boost/lexical_cast.hpp>

#include <cmath>

namespace {
    const double MAX_SHIP_SPEED = 500.0;            // max allowed speed of ship movement
    const double FLEET_MOVEMENT_EPSILON = 1.0e-1;   // how close a fleet needs to be to a system to have arrived in the system

    inline bool SystemNotReachable(System* system, int empire_id) {
        return !GetUniverse().SystemReachable(system->ID(), empire_id);
    }

}

// static(s)
const int Fleet::ETA_UNKNOWN = (1 << 30);
const int Fleet::ETA_OUT_OF_RANGE = (1 << 30) - 1;
const int Fleet::ETA_NEVER = (1 << 30) - 2;

Fleet::Fleet() : 
    UniverseObject(),
    m_moving_to(INVALID_OBJECT_ID),
    m_speed(0.0),
    m_prev_system(INVALID_OBJECT_ID),
    m_next_system(INVALID_OBJECT_ID),
    m_travel_distance(0.0)
{}

Fleet::Fleet(const std::string& name, double x, double y, int owner) :
    UniverseObject(name, x, y),
    m_moving_to(INVALID_OBJECT_ID),
    m_speed(0.0),
    m_prev_system(INVALID_OBJECT_ID),
    m_next_system(INVALID_OBJECT_ID),
    m_travel_distance(0.0)
{ AddOwner(owner); }

Fleet::const_iterator Fleet::begin() const
{
    return m_ships.begin();
}

Fleet::const_iterator Fleet::end() const
{
    return m_ships.end();
}

const std::set<int>& Fleet::ShipIDs() const
{
    return m_ships;
}

Visibility Fleet::GetVisibility(int empire_id) const
{
    if (Universe::ALL_OBJECTS_VISIBLE || empire_id == ALL_EMPIRES || OwnedBy(empire_id)) {
        return VIS_FULL_VISIBILITY;
    } else {
        // A fleet is visible to another player, iff
        // the previous system on the route or the next system on the route
        // is visible to the player.
        System * system;
        if ((system = GetUniverse().Object<System>(SystemID())) &&
            system->GetVisibility(empire_id) != VIS_NO_VISIBITY)
            return VIS_PARTIAL_VISIBILITY;
        if ((system = GetUniverse().Object<System>(NextSystemID())) &&
            system->GetVisibility(empire_id) != VIS_NO_VISIBITY)
            return VIS_PARTIAL_VISIBILITY;
        if ((system = GetUniverse().Object<System>(PreviousSystemID())) &&
            system->GetVisibility(empire_id) != VIS_NO_VISIBITY)
            return VIS_PARTIAL_VISIBILITY;
        return VIS_NO_VISIBITY;
    }
}

const std::string& Fleet::PublicName(int empire_id) const
{
    // Disclose real fleet name only to fleet owners. Rationale: a player might become suspicious if the incoming
    // foreign fleet is called "Decoy"
    if (Universe::ALL_OBJECTS_VISIBLE || empire_id == ALL_EMPIRES || OwnedBy(empire_id))
        return Name();
    else
        return UserString("FW_FOREIGN_FLEET");
}

const std::list<System*>& Fleet::TravelRoute() const
{
    CalculateRoute();
    //Logger().debugStream() << "fleet travel route: ";
    //for (std::list<System*>::const_iterator it = m_travel_route.begin(); it != m_travel_route.end(); ++it)
    //    Logger().debugStream() << "... " << (*it)->Name();
    return m_travel_route;
}

std::list<MovePathNode> Fleet::MovePath() const
{
    return MovePath(TravelRoute());
}

std::list<MovePathNode> Fleet::MovePath(const std::list<System*>& route) const
{
    std::list<MovePathNode> retval = std::list<MovePathNode>();

    if (route.empty())
        return retval;                                      // nowhere to go => empty path
    // if (route.size() == 1) do nothing special.  this fleet is probably on the starlane leading to
    //                        its final destination.  normal looping to read destination should work fine
    if (route.size() == 2 && route.front() == route.back())
        return retval;                                      // nowhere to go => empty path
    if (this->Speed() < FLEET_MOVEMENT_EPSILON) {
        retval.push_back(MovePathNode(this->X(), this->Y(), true, ETA_NEVER, this->SystemID(), UniverseObject::INVALID_OBJECT_ID, UniverseObject::INVALID_OBJECT_ID));
        return retval;                                      // can't move => path is just this system with explanitory ETA
    }

    double fuel =       Fuel();
    double max_fuel =   MaxFuel();

    //Logger().debugStream() << "Fleet " << this->Name() << " movePath fuel: " << fuel << " sys id: " << this->SystemID();

    // determine all systems where fleet(s) can be resupplied if fuel runs out
    std::set<int> fleet_supplied_systems;
    const std::set<int>& owners = this->Owners();
    for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it) {
        const Empire* empire = Empires().Lookup(*it);
        std::set<int> empire_fleet_supplied_systems;
        if (empire)
            empire_fleet_supplied_systems = empire->FleetSupplyableSystemIDs();
        fleet_supplied_systems.insert(empire_fleet_supplied_systems.begin(), empire_fleet_supplied_systems.end());
    }



    // determine if, given fuel available and supplyable systems, fleet will ever be able to move
    if (fuel < 1.0 && this->GetSystem() && fleet_supplied_systems.find(this->SystemID()) == fleet_supplied_systems.end()) {
        MovePathNode node(this->X(), this->Y(), true, ETA_OUT_OF_RANGE,
                          this->SystemID(),
                          UniverseObject::INVALID_OBJECT_ID,
                          UniverseObject::INVALID_OBJECT_ID);
        retval.push_back(node);
        return retval;      // can't move => path is just this system with explanitory ETA
    }


    const Universe& universe = GetUniverse();


    // get iterator pointing to System* on route that is the first after where this fleet is currently.
    // if this fleet is in a system, the iterator will point to the system after the current in the route
    // if this fleet is not in a system, the iterator will point to the first system in the route
    std::list<System*>::const_iterator route_it = route.begin();
    if ((*route_it)->ID() == SystemID())
        ++route_it;     // first system in route is current system of this fleet.  skip to the next system
    if (route_it == route.end())
        return retval;  // current system of this fleet is the *only* system in the route.  path is empty.


    // get current, previous and next systems of fleet
    const System* cur_system = this->GetSystem();                                   // may be NULL
    const System* prev_system = universe.Object<System>(this->PreviousSystemID());  // may be NULL if this fleet is not moving or ordered to move
    const System* next_system = *route_it;  // can't use this->NextSystemID() because this fleet may not be moving and may not have a next system. this might occur when a fleet is in a system, not ordered to move or ordered to move to a system, but a projected fleet move line is being calculated to a different system
    if (!next_system) {
        //Logger().errorStream() << "Fleet::MovePath couldn't get next system for this fleet " << this->Name();
        return retval;
    }

    //Logger().debugStream() << "initial cur system: " << (cur_system ? cur_system->Name() : "(none)") <<
    //                          "  prev system: " << (prev_system ? prev_system->Name() : "(none)") <<
    //                          "  next system: " << (next_system ? next_system->Name() : "(none)");



    // place initial position MovePathNode
    MovePathNode initial_pos(this->X(), this->Y(), false /* not an end of turn node */, 0 /* turns taken to reach position of node */,
                             (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                             (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                             (next_system ? next_system->ID() : INVALID_OBJECT_ID));
    retval.push_back(initial_pos);


    const int       TOO_LONG =              100;        // limit on turns to simulate.  99 turns max keeps ETA to two digits, making UI work better
    int             turns_taken =           1;
    double          turn_dist_remaining =   m_speed;    // additional distance that can be travelled in current turn of fleet movement being simulated
    double          cur_x =                 this->X();
    double          cur_y =                 this->Y();
    double          next_x =                next_system->X();
    double          next_y =                next_system->Y();

    // simulate fleet movement given known speed, starting position, fuel limit and systems on route
    // need to populate retval with MovePathNodes that indicate the correct position, whether this
    // fleet will end a turn at the node, the turns it will take to reach the node, and (when appliable)
    // the current (if at a system), previous and next system IDs at which the fleet will be.  the
    // previou and next system ids are needed to know what starlane a given node is located on, if any.
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


        // find distnace to next system along path from current position
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
                cur_system = NULL;
            }
        }


        // determine whether the fleet ends its turn at its new position.  if fleet can't move, it
        // must wait until next turn for another chance to move
        bool end_turn_at_cur_position = false;
        if (turn_dist_remaining < FLEET_MOVEMENT_EPSILON) {
            //Logger().debugStream() << " ... fleet can't move further this turn.";
            turn_dist_remaining = 0.0;      // to prevent any possible precision-related errors
            end_turn_at_cur_position = true;
        } else {
            //Logger().debugStream() << " ... fleet CAN move further this turn on next iteration.";
        }


        // check if current position is close enough to next system on route to qualify as at that system.
        if (dist_to_next_system < FLEET_MOVEMENT_EPSILON) {

            // close enough to be consider to be at next system.
            // set current position to be exactly at next system to avoid rounding issues
            cur_system = next_system;
            cur_x = cur_system->X();    // update positions to ensure no round-off-errors
            cur_y = cur_system->Y();

            //Logger().debugStream() << " ... arrived at system: " << cur_system->Name();


            // attempt to get next system on route, to update next system.  if new current
            // system is the end of the route, abort.
            ++route_it;
            if (route_it == route.end())
                break;

            // update next system on route and distance to it from current position
            next_system = *route_it;
            next_x = next_system->X();
            next_y = next_system->Y();
        }


        if (end_turn_at_cur_position && (turns_taken + 1 >= TOO_LONG)) {
            // exit loop before placing current node to simplfy post-loop processing: now all cases require a post-loop node to be added
            ++turns_taken;
            break;
        }

        // add MovePathNode for current position (end of turn position and/or system location)
        MovePathNode cur_pos(cur_x, cur_y, end_turn_at_cur_position, turns_taken,
                             (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                             (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                             (next_system ? next_system->ID() : INVALID_OBJECT_ID));
        retval.push_back(cur_pos);


        // if the turn ended at this position, increment the turns taken and reset the distance remaining
        // to be travelled during the current (now next) turn for the next loop iteration
        if (end_turn_at_cur_position) {
            //Logger().debugStream() << " ... end of simulated turn " << turns_taken;
            ++turns_taken;
            turn_dist_remaining = m_speed;
        }
    }


    // done looping.  may have exited due to reaching end of path, lack of fuel, or turns taken getting too big
    if (turns_taken == TOO_LONG)
        turns_taken = ETA_NEVER;

    MovePathNode final_pos(cur_x, cur_y, true, turns_taken,
                           (cur_system  ? cur_system->ID()  : INVALID_OBJECT_ID),
                           (prev_system ? prev_system->ID() : INVALID_OBJECT_ID),
                           (next_system ? next_system->ID() : INVALID_OBJECT_ID));
    retval.push_back(final_pos);

    return retval;
}

std::pair<int, int> Fleet::ETA() const
{
    return ETA(MovePath());
}

std::pair<int, int> Fleet::ETA(const std::list<MovePathNode>& move_path) const
{
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

double Fleet::Fuel() const
{
    if (NumShips() < 1)
        return 0.0;

    // determine fuel available to fleet (fuel of the ship that has the least fuel in the fleet)
    const Universe& universe = GetUniverse();
    double fuel = Meter::METER_MAX;
    for (const_iterator ship_it = begin(); ship_it != end(); ++ship_it) {
        const Ship* ship = universe.Object<Ship>(*ship_it);
        assert(ship);
        const Meter* meter = ship->GetMeter(METER_FUEL);
        assert(meter);
        fuel = std::min(fuel, meter->Current());
    }
    return fuel;
}

double Fleet::MaxFuel() const
{
    if (NumShips() < 1)
        return 0.0;

    // determine the maximum amount of fuel that can be stored by the ship in the fleet that
    // can store the least amount of fuel
    const Universe& universe = GetUniverse();
    double max_fuel = Meter::METER_MAX;
    for (const_iterator ship_it = begin(); ship_it != end(); ++ship_it) {
        const Ship* ship = universe.Object<Ship>(*ship_it);
        assert(ship);
        const Meter* meter = ship->GetMeter(METER_FUEL);
        assert(meter);
        max_fuel = std::min(max_fuel, meter->Max());
    }
    return max_fuel;
}

int Fleet::FinalDestinationID() const
{ return m_moving_to; }

System* Fleet::FinalDestination() const
{ return GetUniverse().Object<System>(m_moving_to); }

int Fleet::PreviousSystemID() const
{ return m_prev_system; }

int Fleet::NextSystemID() const
{ return m_next_system; }

double Fleet::Speed() const
{
    //Logger().debugStream() << "Fleet " << this->Name() << " has speed: " << m_speed;
    return m_speed;
}

bool Fleet::CanChangeDirectionEnRoute() const
{
    // TODO: enable this code when technologies or other factors to allow a fleet to turn around in mid-flight, without completing its current leg
    return false;
}

bool Fleet::HasArmedShips() const
{
    for (Fleet::const_iterator it = begin(); it != end(); it++) {   
        if (GetUniverse().Object<Ship>(*it)->IsArmed())
            return true;
    }
    return false;
}

bool Fleet::HasColonyShips() const
{
    for (Fleet::const_iterator it = begin(); it != end(); it++) {   
        if (GetUniverse().Object<Ship>(*it)->CanColonize())
            return true;
    }
    return false;}

int Fleet::NumShips() const
{
    return m_ships.size();
}

bool Fleet::Contains(int object_id) const
{
    return m_ships.find(object_id) != m_ships.end();
}

std::vector<UniverseObject*> Fleet::FindObjects() const
{
    Universe& universe = GetUniverse();
    std::vector<UniverseObject*> retval;
    // add ships in this fleet
    for (ShipIDSet::const_iterator it = m_ships.begin(); it != m_ships.end(); ++it)
        retval.push_back(universe.Object(*it));
    return retval;
}

std::vector<int> Fleet::FindObjectIDs() const
{
    std::vector<int> retval;
    // add ships in this fleet
    std::copy(m_ships.begin(), m_ships.end(), std::back_inserter(retval));
    return retval;
}

bool Fleet::UnknownRoute() const
{
    return m_travel_route.size() == 1 && m_travel_route.front()->ID() == UniverseObject::INVALID_OBJECT_ID;
}

UniverseObject* Fleet::Accept(const UniverseObjectVisitor& visitor) const
{
    return visitor.Visit(const_cast<Fleet* const>(this));
}

void Fleet::SetRoute(const std::list<System*>& route)
{
    if (route.empty())
        throw std::invalid_argument("Fleet::SetRoute() : Attempted to set an empty route.");

    if (UnknownRoute())
        throw std::invalid_argument("Fleet::SetRoute() : Attempted to set an unkown route.");

    if (m_prev_system != SystemID() && m_prev_system == route.front()->ID() && !CanChangeDirectionEnRoute())
        throw std::invalid_argument("Fleet::SetRoute() : Illegally attempted to change a fleet's direction while it was in transit.");

    m_travel_route = route;

    // calculate length of line segments between systems on route, and sum up to determine length of route between
    // systems on route.  (Might later add distance from fleet to first system on route to this to get the total
    // route length, or this may itself be the total route length if the fleet is at the first system on the route).
    m_travel_distance = 0.0;
    for (std::list<System*>::const_iterator it = m_travel_route.begin(); it != m_travel_route.end(); ++it) {
        std::list<System*>::const_iterator next_it = it;    ++next_it;

        if (next_it == m_travel_route.end())
            break;  // current system is the last on the route, so don't need to add any additional distance.

        const System* cur_sys = *it;
        const System* next_sys = *next_it;
        if (!cur_sys || !next_sys)
            throw std::invalid_argument("Fleet::SetRoute() : passed a null System pointer on route");

        double dist_x = next_sys->X() - cur_sys->X();
        double dist_y = next_sys->Y() - cur_sys->Y();
        m_travel_distance += std::sqrt(dist_x * dist_x + dist_y * dist_y);
    }


    // if resetting to no movement while in a system
    if (SystemID() != UniverseObject::INVALID_OBJECT_ID && SystemID() == m_travel_route.back()->ID()) {
        m_moving_to = UniverseObject::INVALID_OBJECT_ID;
        m_next_system = UniverseObject::INVALID_OBJECT_ID;
        m_prev_system = UniverseObject::INVALID_OBJECT_ID;
    } else {
        // if we're already moving, add in the distance from where we are to the first system in the route
        if (SystemID() != route.front()->ID()) {
            System* starting_system = route.front();
            double dist_x = starting_system->X() - X();
            double dist_y = starting_system->Y() - Y();
            m_travel_distance += std::sqrt(dist_x * dist_x + dist_y * dist_y);
        }
        m_moving_to = m_travel_route.back()->ID();
        if (m_prev_system != SystemID() && m_prev_system == m_travel_route.front()->ID()) {
            m_prev_system = m_next_system;      // if already in transit and turning around, swap prev and next
        } else if (SystemID() == route.front()->ID()) {
            m_prev_system = SystemID();
        }
        std::list<System*>::const_iterator it = m_travel_route.begin();
        m_next_system = m_prev_system == SystemID() ? (*++it)->ID() : (*it)->ID();
    }

    StateChangedSignal();
}

void Fleet::AddShips(const std::vector<int>& ships)
{
    for (unsigned int i = 0; i < ships.size(); ++i) {
        int ship_id = ships[i];

        if (this->Contains(ship_id)) {
            Logger().debugStream() << "Fleet::AddShip this fleet " << this->Name() << " already contained ship " << ship_id;
            continue;
        }

        if (Ship* s = GetUniverse().Object<Ship>(ship_id)) {
            if (System* system = GetSystem()) {
                system->Insert(s);
            } else {
                s->MoveTo(X(), Y());
                s->SetSystem(SystemID());
            }
            s->SetFleetID(ID());
            m_ships.insert(ship_id);
        } else {
            throw std::invalid_argument("Fleet::AddShips() : Attempted to add an id of a non-ship object to a fleet.");
        }
    }
    RecalculateFleetSpeed();
    StateChangedSignal();
}

void Fleet::AddShip(int ship_id)
{
    if (this->Contains(ship_id)) {
        Logger().debugStream() << "Fleet::AddShip this fleet " << this->Name() << " already contained ship " << ship_id;
        return;
    }

    Logger().debugStream() << "Fleet " << this->Name() << " adding ship: " << ship_id;
    if (Ship* s = GetUniverse().Object<Ship>(ship_id)) {
        if (System* system = GetSystem()) {
            system->Insert(s);
        } else {
            s->MoveTo(X(), Y());
            s->SetSystem(SystemID());
        }
        s->SetFleetID(ID());
        m_ships.insert(ship_id);
    } else {
        Logger().errorStream() << "Fleet::AddShip() : Attempted to add an id of a non-ship object to a fleet.";
    }
    RecalculateFleetSpeed(); // makes AddShip take Order(m_ships.size()) time - may need replacement
    StateChangedSignal();
}

std::vector<int> Fleet::RemoveShips(const std::vector<int>& ships)
{
    std::vector<int> retval;
    for (unsigned int i = 0; i < ships.size(); ++i) {
        bool found = m_ships.find(ships[i]) != m_ships.end();
        m_ships.erase(ships[i]);
        if (!found)
            retval.push_back(ships[i]);
    }
    RecalculateFleetSpeed();
    StateChangedSignal();
    return retval;
}

std::vector<int> Fleet::DeleteShips(const std::vector<int>& ships)
{
    std::vector<int> retval;
    for (unsigned int i = 0; i < ships.size(); ++i) {
        bool found = m_ships.find(ships[i]) != m_ships.end();
        m_ships.erase(ships[i]);
        if (!found) {
            retval.push_back(ships[i]);
        } else {
            GetUniverse().Delete(ships[i]);
        }
    }
    RecalculateFleetSpeed();
    StateChangedSignal();
    return retval;
}


bool Fleet::RemoveShip(int ship)
{
    bool retval = false;
    iterator it = m_ships.find(ship);
    if (it != m_ships.end()) {
        m_ships.erase(it);
        RecalculateFleetSpeed();
        StateChangedSignal();
        retval = true;
    }
    return retval;
}

void Fleet::SetSystem(int sys)
{
    //Logger().debugStream() << "Fleet::SetSystem(int sys)";
    UniverseObject::SetSystem(sys);
    for (iterator it = begin(); it != end(); ++it) {
        UniverseObject* obj = GetUniverse().Object(*it);
        assert(obj);
        obj->SetSystem(sys);
    }
}

void Fleet::MoveTo(double x, double y)
{
    //Logger().debugStream() << "Fleet::MoveTo(double x, double y)";
    // move fleet itself
    UniverseObject::MoveTo(x, y);
    // move ships in fleet
    for (iterator it = begin(); it != end(); ++it) {
        UniverseObject* obj = GetUniverse().Object(*it);
        assert(obj);
        obj->UniverseObject::MoveTo(x, y);
    }
}

void Fleet::SetNextAndPreviousSystems(int next, int prev)
{
    m_prev_system = prev;
    m_next_system = next;
}

void Fleet::MovementPhase()
{
    //Logger().debugStream() << "Fleet::MovementPhase this: " << this->Name() << " id: " << this->ID();

    std::list<MovePathNode> move_path = this->MovePath();

    System* current_system = GetSystem();
    Universe& universe = GetUniverse();


    // get systems at which this fleet can be resupplied
    std::set<int> fleet_supplied_systems;
    const std::set<int>& owners = Owners();
    for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it) {
        std::set<int> empire_fleet_supplied_systems;
        if (const Empire* empire = Empires().Lookup(*it))
            empire_fleet_supplied_systems = empire->FleetSupplyableSystemIDs();
        fleet_supplied_systems.insert(empire_fleet_supplied_systems.begin(), empire_fleet_supplied_systems.end());
    }
    //Logger().debugStream() << "Fleet Supplied Systems:";
    //for (std::set<int>::const_iterator it = fleet_supplied_systems.begin(); it != fleet_supplied_systems.end(); ++it)
    //    Logger().debugStream() << " ... " << *it << " is supplied";


    // resupply fleet if possible
    if (current_system) {
        //Logger().debugStream() << "Fleet current system: " << current_system->Name() << " id: " << current_system->ID();
        // resupply ships, if fleet is supplyable at this location
        if (fleet_supplied_systems.find(current_system->ID()) != fleet_supplied_systems.end()) {
            for (Fleet::const_iterator ship_it = this->begin(); ship_it != this->end(); ++ship_it) {
                Ship* ship = universe.Object<Ship>(*ship_it);
                assert(ship);
                ship->Resupply();
            }
        }
    }


    if (move_path.empty() || move_path.size() == 1) {
        //Logger().debugStream() << "Fleet::MovementPhase: Fleet move path is empty or has only one entry.  doing nothing";
        return;
    }

    //Logger().debugStream() << "Fleet::MovementPhase move path:";
    //for (std::list<MovePathNode>::const_iterator it = move_path.begin(); it != move_path.end(); ++it)
    //    Logger().debugStream() << "... (" << it->x << ", " << it->y << ") at object id: " << it->object_id << " eta: " << it->eta << (it->turn_end ? " (end of turn)" : " (during turn)");



    std::list<MovePathNode>::const_iterator it = move_path.begin();
    std::list<MovePathNode>::const_iterator next_it = it;   next_it++;
    // is the ship stuck in a system for a whole turn?
    if (current_system) {
        // in a system
        if (next_it == move_path.end()) {
            // there is no system after the current one in the path... so won't be moving
            return;
        }
        if (it->object_id != INVALID_OBJECT_ID && it->object_id == next_it->object_id) {
            // the current and next nodes have the same system id, that is an actual system.  thus won't be moving this turn.
            return;
        }
    }


    // move fleet in sequence to MovePathNodes it can reach this turn
    double fuel_consumed = 0.0;
    for (it = move_path.begin(); it != move_path.end(); ++it) {
        next_it = it;   ++next_it;

        System* system = universe.Object<System>(it->object_id);

        //Logger().debugStream() << "... node " << (system ? system->Name() : "no system");

        // is this system the last node reached this turn?  either it's an end of turn node,
        // or there are no more nodes after this one on path
        bool node_is_next_stop = (it->turn_end || next_it == move_path.end());


        if (system) {
            // node is a system.  explore system for all owners of this fleet
            for (std::set<int>::const_iterator owners_it = owners.begin(); owners_it != owners.end(); ++owners_it)
                if (Empire* empire = Empires().Lookup(*owners_it))
                    empire->AddExploredSystem(it->object_id);


            m_prev_system = system->ID();               // passing a system, so update previous system of this fleet


            bool resupply_here = (fleet_supplied_systems.find(system->ID()) != fleet_supplied_systems.end());

            // if this system can provide supplies, reset consumed fuel and refuel ships
            if (resupply_here) {
                //Logger().debugStream() << " ... node has fuel supply.  consumed fuel for movement reset to 0 and fleet resupplied";
                fuel_consumed = 0.0;
                for (Fleet::const_iterator ship_it = this->begin(); ship_it != this->end(); ++ship_it) {
                    Ship* ship = universe.Object<Ship>(*ship_it);
                    assert(ship);
                    ship->Resupply();
                }
            }


            if (node_is_next_stop) {                    // is system the last node reached this turn?
                system->Insert(this);                       // fleet ends turn at this node.  insert fleet into system
                current_system = system;
                //Logger().debugStream() << "... ... inserted fleet into system";
                break;
            } else {
                // fleet will continue past this system this turn.
                Logger().debugStream() << "... ... moved fleet to system (not inserted)";
                if (!resupply_here) {
                    fuel_consumed += 1.0;
                    //Logger().debugStream() << "... ... consuming 1 unit of fuel to continue moving.  total fuel consumed now: " << fuel_consumed;
                } else {
                    //Logger().debugStream() << "... ... not consuming fuel to depart resupply system";
                }
            }

        } else {
            // node is not a system.
            if (node_is_next_stop) {                    // node is not a system, but is it the last node reached this turn?
                MoveTo(it->x, it->y);                       // fleet ends turn at this node.  move fleet here
                //Logger().debugStream() << "... ... moved fleet to position";
                break;
            }
        }
    }


    //Logger().debugStream() << "Fleet::MovementPhase rest of move path:";
    //for (std::list<MovePathNode>::const_iterator it2 = it; it2 != move_path.end(); ++it2)
    //    Logger().debugStream() << "... (" << it2->x << ", " << it2->y << ") at object id: " << it2->object_id << " eta: " << it2->eta << (it2->turn_end ? " (end of turn)" : " (during turn)");

    // update next system
    if (m_moving_to != SystemID() && next_it != move_path.end() && it != move_path.end()) {
        // there is another system later on the path to aim for.  find it
        for (; next_it != move_path.end(); ++next_it) {
            if (universe.Object<System>(next_it->object_id)) {
                //Logger().debugStream() << "___ setting m_next_system to " << next_it->object_id;
                m_next_system = next_it->object_id;
                break;
            }
        }

    } else {
        // no more systems on path
        m_moving_to = m_next_system = m_prev_system = UniverseObject::INVALID_OBJECT_ID;
    }


    // consume fuel from ships in fleet
    if (fuel_consumed > 0.0) {
        for (const_iterator ship_it = begin(); ship_it != end(); ++ship_it) {
            Ship* ship = universe.Object<Ship>(*ship_it);
            assert(ship);
            Meter* meter = ship->GetMeter(METER_FUEL);
            assert(meter);
            meter->AdjustCurrent(-fuel_consumed);
        }
    }
}

Fleet::iterator Fleet::begin()
{
    return m_ships.begin();
}

Fleet::iterator Fleet::end()
{
    return m_ships.end();
}

void Fleet::PopGrowthProductionResearchPhase()
{
    // ensure that any newly opened or closed routes are taken into account
    m_travel_route.clear();
    CalculateRoute();
}

void Fleet::CalculateRoute() const
{
    //Logger().debugStream() << "Fleet::CalculateRoute";
    if (m_moving_to != INVALID_OBJECT_ID && m_travel_route.empty()) {
        m_travel_distance = 0.0;
        if (m_prev_system != UniverseObject::INVALID_OBJECT_ID && SystemID() == m_prev_system) { // if we haven't actually left yet, we have to move from whichever system we are at now
            std::pair<std::list<System*>, double> path = GetUniverse().ShortestPath(m_prev_system, m_moving_to, *Owners().begin());
            m_travel_route = path.first;
            m_travel_distance = path.second;
        } else { // if we're between systems, the shortest route may be through either one
            if (CanChangeDirectionEnRoute()) {
                std::pair<std::list<System*>, double> path1 = GetUniverse().ShortestPath(m_next_system, m_moving_to, *Owners().begin());
                std::pair<std::list<System*>, double> path2 = GetUniverse().ShortestPath(m_prev_system, m_moving_to, *Owners().begin());
                double dist_x = path1.first.front()->X() - X();
                double dist_y = path1.first.front()->Y() - Y();
                double dist1 = std::sqrt(dist_x * dist_x + dist_y * dist_y);
                dist_x = path2.first.front()->X() - X();
                dist_y = path2.first.front()->Y() - Y();
                double dist2 = std::sqrt(dist_x * dist_x + dist_y * dist_y);
                if (dist1 + path1.second < dist2 + path2.second) {
                    m_travel_route = path1.first;
                    m_travel_distance = dist1 + path1.second;
                } else {
                    m_travel_route = path2.first;
                    m_travel_distance = dist2 + path2.second;
                }
            } else {
                std::pair<std::list<System*>, double> route = GetUniverse().ShortestPath(m_next_system, m_moving_to, *Owners().begin());
                double dist_x = route.first.front()->X() - X();
                double dist_y = route.first.front()->Y() - Y();
                double dist = std::sqrt(dist_x * dist_x + dist_y * dist_y);
                m_travel_route = route.first;
                m_travel_distance = dist + route.second;
            }
        }
    }
}

void Fleet::RecalculateFleetSpeed()
{
    if (!(m_ships.empty())) {
        m_speed = MAX_SHIP_SPEED;  // max speed no ship can go faster than
        for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
            Ship* ship = GetUniverse().Object<Ship>(*it);
            if (ship) {
                if (ship->Speed() < m_speed)
                    m_speed = ship->Speed();
            }
        }
    } else {
        m_speed = 0.0;
    }
}

void Fleet::ShortenRouteToEndAtSystem(std::list<System*>& travel_route, int last_system)
{
    std::list<System*>::iterator visible_end_it;
    if (last_system != m_moving_to) {
        // The system the fleet will appear to be moving to it's actually it's final destination.  remove any
        // extra systems from the route after the apparent destination
        System* final_destination = GetUniverse().Object<System>(last_system);
        assert(std::find(m_travel_route.begin(), m_travel_route.end(), final_destination) != m_travel_route.end());
        visible_end_it = ++std::find(m_travel_route.begin(), m_travel_route.end(), final_destination);
    } else {
        visible_end_it = m_travel_route.end();
    }

    int fleet_owner = -1;
    const std::set<int>& owners = Owners();
    if (owners.size() == 1)
        fleet_owner = *(owners.begin());
    const Empire* empire = Empires().Lookup(fleet_owner);
    if (!empire)        // may occur for destroyed objects whose previous owner has since been eliminated
        fleet_owner = -1;

    std::list<System*>::iterator end_it = std::find_if(m_travel_route.begin(), visible_end_it, boost::bind(&SystemNotReachable, _1, fleet_owner));
    std::copy(m_travel_route.begin(), end_it, std::back_inserter(travel_route));
    // If no Systems in a nonempty route are known reachable, put a null pointer in the route as a sentinel indicating
    // that the route is unknown, but needs not be recomputed.
    if (travel_route.empty() && !m_travel_route.empty())
        travel_route.push_back(0);
}

std::string Fleet::GenerateFleetName(const std::vector<int>& ship_ids, int new_fleet_id) {
    // TODO: Change returned name based on passed ship designs.  eg. return "colony fleet" if
    // ships are colony ships, or "battle fleet" if ships are armed.
    if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID)
        return UserString("NEW_FLEET_NAME_NO_NUMBER");

    return boost::io::str(FlexibleFormat(UserString("NEW_FLEET_NAME")) % boost::lexical_cast<std::string>(new_fleet_id));
}

