#include "Fleet.h"

#include "Ship.h"
#include "Predicates.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"
#include "../Empire/Empire.h"
#include "../Empire/EmpireManager.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdexcept>
#include <cmath>

#include <boost/format.hpp>

namespace {
    const double MAX_SHIP_SPEED = 500.0;            // max allowed speed of ship movement
    const double FLEET_MOVEMENT_EPSILON = 1.0e-1;   // how close a fleet needs to be to a system to have arrived in the system

    inline bool SystemNotReachable(System* system, int empire_id) {
        return !GetUniverse().SystemReachable(system->ID(), empire_id);
    }

    void ResupplyFleetIfPossible(Fleet* fleet) {
        if (fleet->SystemID() == UniverseObject::INVALID_OBJECT_ID)
            return;
        Universe& universe = GetUniverse();

        // get all supplyable systems
        std::set<int> fleet_supplied_systems;
        const std::set<int>& owners = fleet->Owners();
        for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it) {
            std::set<int> empire_fleet_supplied_systems;
            if (const Empire* empire = Empires().Lookup(*it))
                empire_fleet_supplied_systems = empire->FleetSupplyableSystemIDs();
            fleet_supplied_systems.insert(empire_fleet_supplied_systems.begin(), empire_fleet_supplied_systems.end());
        }

        // give fuel to ships, if fleet is supplyable at this location
        if (fleet_supplied_systems.find(fleet->SystemID()) != fleet_supplied_systems.end()) {
            for (Fleet::const_iterator ship_it = fleet->begin(); ship_it != fleet->end(); ++ship_it) {
                Ship* ship = universe.Object<Ship>(*ship_it);
                assert(ship);
                Meter* meter = ship->GetMeter(METER_FUEL);
                assert(meter);
                meter->SetCurrent(meter->Max());
            }
        }
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

UniverseObject::Visibility Fleet::GetVisibility(int empire_id) const
{
    if (Universe::ALL_OBJECTS_VISIBLE || empire_id == ALL_EMPIRES || OwnedBy(empire_id)) {
        return FULL_VISIBILITY;
    } else {
        // A fleet is visible to another player, iff
        // the previous system on the route or the next system on the route
        // is visible to the player.
        System * system;
        if ((system = GetUniverse().Object<System>(SystemID())) &&
            system->GetVisibility(empire_id) != NO_VISIBILITY)
            return PARTIAL_VISIBILITY;
        if ((system = GetUniverse().Object<System>(NextSystemID())) &&
            system->GetVisibility(empire_id) != NO_VISIBILITY)
            return PARTIAL_VISIBILITY;
        if ((system = GetUniverse().Object<System>(PreviousSystemID())) &&
            system->GetVisibility(empire_id) != NO_VISIBILITY)
            return PARTIAL_VISIBILITY;
        return NO_VISIBILITY;
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
    if (route.size() == 2 && route.front() == route.back())
        return retval;                                      // nowhere to go => empty path
    if (this->Speed() < FLEET_MOVEMENT_EPSILON) {
        retval.push_back(MovePathNode(this->X(), this->Y(), true, ETA_NEVER, this->SystemID()));
        return retval;                                      // can't move => path is just this system with explanitory ETA
    }

    double fuel = Fuel();
    double max_fuel = MaxFuel();

    //Logger().debugStream() << "Fleet " << this->Name() << "MovePath fuel: " << fuel << " sys id: " << this->SystemID();

    // determine all systems where fleet(s) can be resupplied if fuel runs out
    std::set<int> fleet_supplied_systems;
    const std::set<int>& owners = this->Owners();
    //Logger().debugStream()<< " ... owners.size: " << owners.size();
    for (std::set<int>::const_iterator it = owners.begin(); it != owners.end(); ++it) {
        //Logger().debugStream() << " ... ... owner: " << *it;
        const Empire* empire = Empires().Lookup(*it);
        std::set<int> empire_fleet_supplied_systems;
        if (empire)
            empire_fleet_supplied_systems = empire->FleetSupplyableSystemIDs();
        //Logger().debugStream() << " ... ... supplied systems size: " << empire_fleet_supplied_systems.size();
        fleet_supplied_systems.insert(empire_fleet_supplied_systems.begin(), empire_fleet_supplied_systems.end());
    }

    //Logger().debugStream() << " ... all supplyable systems: ";
    //for (std::set<int>::const_iterator it = fleet_supplied_systems.begin(); it != fleet_supplied_systems.end(); ++it)
    //    Logger().debugStream() << "sys id: " << *it;


    // determine if, given fuel available and supplyable systems, fleet will ever be able to move
    if (fuel < 1.0 && this->GetSystem() && fleet_supplied_systems.find(this->SystemID()) == fleet_supplied_systems.end()) {
        retval.push_back(MovePathNode(this->X(), this->Y(), true, ETA_OUT_OF_RANGE, this->SystemID()));
        return retval;                                      // can't move => path is just this system with explanitory ETA
    }


    // node for initial position of fleet
    MovePathNode cur_pos(this->X(), this->Y(), true, 0, this->SystemID());


    // get current system of fleet, if it is in a system, and next system reached in path
    const System* cur_system = 0;
    std::list<System*>::const_iterator route_it = route.begin();
    if ((*route_it)->ID() == SystemID()) {
        cur_system = *route_it;
        ++route_it;
    }
    if (route_it == route.end())
        return retval;
    const System* next_system = *route_it;

    MovePathNode next_sys_pos(next_system->X(), next_system->Y(), false, -1, next_system->SystemID());


    double dist_to_next_system = std::sqrt((next_sys_pos.x - cur_pos.x)*(next_sys_pos.x - cur_pos.x) + (next_sys_pos.y - cur_pos.y)*(next_sys_pos.y - cur_pos.y));
    double turn_dist_remaining = m_speed;                           // additional distance that can be travelled in current turn of fleet movement being simulated
    bool new_turn = true;                                           // does / should the next update iteration be the start of a new turn?


    // count turns to get to destination by simulating movement steps, accounting for fuel needs and resupply

    const int TOO_LONG = 200;                                       // limit on turns to simulate
    int turns_taken = 0;

    while (turns_taken < TOO_LONG) {
        cur_pos.eta = turns_taken;
        cur_pos.turn_end = new_turn;
        if (!cur_system)
            cur_pos.object_id = INVALID_OBJECT_ID;

        //if (cur_system) Logger().debugStream() << " ... at system: " << cur_system->Name();

        // check for arrival at next system on path
        if (dist_to_next_system < FLEET_MOVEMENT_EPSILON) {
            // update current system and position, and next system and position
            cur_system = next_system;

            cur_pos.x = next_system->X();
            cur_pos.y = next_system->Y();
            // cur_pos.turn_end                     // set previously; don't want to modify here
            // cur_pos.turns_taken                  // set previously; don't want to modify here
            cur_pos.object_id = next_system->ID();

            ++route_it;
            if (route_it == route.end()) {
                cur_pos.turn_end = true;
                retval.push_back(cur_pos);
                break;
            } else {
                retval.push_back(cur_pos);
            }

            next_system = *route_it;
            next_sys_pos.x = next_system->X();
            next_sys_pos.y = next_system->Y();
            next_sys_pos.eta = ETA_UNKNOWN;         // will be set later
            next_sys_pos.turn_end = false;          // may be set later
            next_sys_pos.object_id = next_system->ID();

            dist_to_next_system = std::sqrt((next_sys_pos.x - cur_pos.x)*(next_sys_pos.x - cur_pos.x) + (next_sys_pos.y - cur_pos.y)*(next_sys_pos.y - cur_pos.y));
        } else {
            retval.push_back(cur_pos);
        }

        //Logger().debugStream() << " ... dist to next system: " << dist_to_next_system;

        // if this iteration is the start of a new simulated turn, distance to be travelled this turn is reset and turns taken incremented
        if (new_turn) {
            ++turns_taken;
            turn_dist_remaining = m_speed;
            //Logger().debugStream() << " ... new turn!  turns taken now: " << turns_taken << " turn dist remaining: " << turn_dist_remaining;
        }


        // if this iteration starts at a system, the fleet must have fuel to start the next starlane jump.  if the fleet
        // doesn't have enough fuel to jump, it may be able to be resupplied by spending a turn in this sytem
        if (cur_system) {
            if (fuel >= 1.0) {
                //Logger().debugStream() << " ... have " << fuel << " fuel, deducting 1.  next turn should not be a new turn";
                fuel -= 1.0;                // can start next jump this turn.  turn_dist_remaining is unchanged
                new_turn = false;
            } else {
                //Logger().debugStream() << " ... have " << fuel << " fuel.  not moving this turn";
                turn_dist_remaining = 0.0;  // can't progress this turn due to lack of fuel

                // if a new turn started this update, the fleet has a chance to get fuel by waiting for a full turn in current system
                if (new_turn) {
                    //Logger().debugStream() << " ... new turn this update, so can refuel?";
                    // determine if current system is one where fuel can be supplied
                    std::set<int>::const_iterator it = fleet_supplied_systems.find(cur_system->ID());
                    if (it != fleet_supplied_systems.end()) {
                        //Logger().debugStream() << " ... fleet can be refueled here!";
                        // fleet supply is available, so give it
                        fuel = max_fuel;
                        // turn_dist_remaining is zero, so new_turn will remain true; must wait a full turn without moving to refuel
                    } else {
                        //Logger().debugStream() << " ... can't refuel, and have no fuel, so route goes out of range.  aborting";
                        // started a new turn with insufficient fuel to move, and can't get any more fuel by waiting, so can never get to destination.
                        turns_taken = ETA_OUT_OF_RANGE;
                        break;
                    }
                }
            }
        }


        // move ship as far as it can go this turn, or to next system, whichever is closer, and deduct distance travelled from distance
        // travellable this turn
        if (turn_dist_remaining >= FLEET_MOVEMENT_EPSILON) {
            double dist_travelled_this_step = std::min(turn_dist_remaining, dist_to_next_system);

            //Logger().debugStream() << " ... fleet moving " << dist_travelled_this_step << " this iteration.  dist to next system: " << dist_to_next_system << " and turn_dist_remaining: " << turn_dist_remaining;

            double x_dist = next_sys_pos.x - cur_pos.x;
            double y_dist = next_sys_pos.y - cur_pos.y;
            // dist_to_next_system = std::sqrt(x_dist * x_dist + y_dist * y_dist);  // should already equal this distance, so don't need to recalculate
            double unit_vec_x = x_dist / dist_to_next_system;
            double unit_vec_y = y_dist / dist_to_next_system;

            cur_pos.x += unit_vec_x*dist_travelled_this_step;
            cur_pos.y += unit_vec_y*dist_travelled_this_step;

            turn_dist_remaining -= dist_travelled_this_step;
            dist_to_next_system -= dist_travelled_this_step;

            if (dist_travelled_this_step >= FLEET_MOVEMENT_EPSILON && dist_to_next_system >= FLEET_MOVEMENT_EPSILON)
                cur_system = 0;
        }


        // if fleet can't move, must wait until next turn for another chance to move
        if (turn_dist_remaining < FLEET_MOVEMENT_EPSILON) {
            //Logger().debugStream() << " ... fleet can't move further this turn.  next iteration should be a new turn";
            turn_dist_remaining = 0.0;  // to prevent any possible precision-related errors
            new_turn = true;
        } else {
            //Logger().debugStream() << " ... fleet CAN move further this turn on next iteration.";
            new_turn = false;
        }
    }

    if (turns_taken >= TOO_LONG) {
        //Logger().debugStream() << " ... fleet path took too long or went out of range.";
        if (turns_taken == TOO_LONG)
            turns_taken = ETA_NEVER;
        cur_pos.eta = turns_taken;
        cur_pos.turn_end = true;
        retval.push_back(cur_pos);
    }

    //Logger().debugStream() << "MovePath size: " << retval.size();
    //for (std::list<MovePathNode>::const_iterator it = retval.begin(); it != retval.end(); ++it)
    //    Logger().debugStream() << " .. (" << it->x << ", " << it->y << ")  eta: " << it->eta << "  end turn?: " << boost::lexical_cast<std::string>(it->turn_end) <<
    //                              "  object id: " << it->object_id;
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
        //Logger().debugStream() << "ship " << ship->Name() << " has fuel: " << meter->Current();
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

void Fleet::SetRoute(const std::list<System*>& route, double distance)
{
    if (route.empty())
        throw std::invalid_argument("Fleet::SetRoute() : Attempted to set an empty route.");

    if (UnknownRoute())
        throw std::invalid_argument("Fleet::SetRoute() : Attempted to set an unkown route.");

    if (m_prev_system != SystemID() && m_prev_system == route.front()->ID() && !CanChangeDirectionEnRoute())
        throw std::invalid_argument("Fleet::SetRoute() : Illegally attempted to change a fleet's direction while it was in transit.");

    m_travel_route = route;
    m_travel_distance = distance;

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
            m_prev_system = m_next_system; // if already in transit and turning around, swap prev and next
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
        if (Ship* s = GetUniverse().Object<Ship>(ships[i])) {
            if (System* system = GetSystem()) {
                system->Insert(s);
            } else {
                s->MoveTo(X(), Y());
                s->SetSystem(SystemID());
            }
            s->SetFleetID(ID());
            m_ships.insert(ships[i]);
        } else {
            throw std::invalid_argument("Fleet::AddShips() : Attempted to add an id of a non-ship object to a fleet.");
        }
    }
    RecalculateFleetSpeed();
    StateChangedSignal();
}

void Fleet::AddShip(const int ship_id)
{
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

void Fleet::MovementPhase()
{
    //Logger().debugStream() << "Fleet::MovementPhase this: " << this->Name() << " id: " << this->ID();

    std::list<MovePathNode> move_path = this->MovePath();

    if (move_path.empty()) {
        ResupplyFleetIfPossible(this);
        //Logger().debugStream() << "Fleet::MovementPhase: Fleet has empty move path.  aborting.";
        return;
    }

    //Logger().debugStream() << "Fleet::MovementPhase move path:";
    //for (std::list<MovePathNode>::const_iterator it = move_path.begin(); it != move_path.end(); ++it)
    //    Logger().debugStream() << "... (" << it->x << ", " << it->y << ") at object id: " << it->object_id << " eta: " << it->eta << (it->turn_end ? " (end of turn)" : " (during turn)");


    System* current_system = GetSystem();
    int cur_sys_id = SystemID();
    Universe& universe = GetUniverse();


    if (move_path.size() == 1) {
        //Logger().debugStream() << "Fleet::MovementPhase: Fleet move path has only one entry.  doing nothing";
        ResupplyFleetIfPossible(this);  // just in case MovePath messed up...
        return;
    }


    std::list<MovePathNode>::const_iterator it = move_path.begin();
    std::list<MovePathNode>::const_iterator next_it = it;   next_it++;
    // is the ship stuck in a system for a whole turn?
    if (current_system && (                                             // in a system
            next_it == move_path.end() ||                               // there is no system after the current one in the path... so won't be moving
            next_it->eta > 1 || (                                       // more than one turn to reach next node.  (shouldn't be possible, but just in case...)
                it->object_id != UniverseObject::INVALID_OBJECT_ID &&   // the current and next nodes have the same system id, that is an actual system.  thus won't be moving this turn.
                it->object_id == next_it->object_id
            )
        ))
    {
        // ship won't move this turn.
        ResupplyFleetIfPossible(this);
        return;
    }

    Empire* empire = 0;
    if (!Owners().empty())
        empire = Empires().Lookup(*Owners().begin()); // assumes one owner empire per fleet

    double fuel_consumed = 0.0;

    for (it = move_path.begin(); it != move_path.end(); ++it) {
        next_it = it;   ++next_it;

        System* system = universe.Object<System>(it->object_id);

        //Logger().debugStream() << "... node " << (system ? system->Name() : "no system");

        // is this system the last node reached this turn?  either it's an end of turn node that's not the
        // starting node, or there are no more nodes after this one on path
        bool node_is_next_stop = it != move_path.begin() && (it->turn_end || next_it == move_path.end());
        //if (node_is_next_stop) Logger().debugStream() << "... ...is next stop";

        if (system) {                               // is node a system?
            if (empire)
                empire->AddExploredSystem(it->object_id);   // node is a system.  explore it

            m_prev_system = system->ID();               // passing a system, so update previous system of this fleet


            if (node_is_next_stop) {                    // is system the last node reached this turn?
                system->Insert(this);                       // fleet ends turn at this node.  insert fleet into system
                //Logger().debugStream() << "... ... inserted fleet into system";
                break;
            } else {
                fuel_consumed += 1.0;                       // fleet will continue past this system this turn.  fuel is consumed to do so.
                //Logger().debugStream() << "... ...fuel consumed";
            }
        } else {
            if (node_is_next_stop) {                    // node is not a system, but is it the last node reached this turn?
                MoveTo(it->x, it->y);                       // fleet ends turn at this node.  move fleet here
                //Logger().debugStream() << "... ... moved fleet to position";

                // find next system
                break;
            }
        }
    }

    cur_sys_id = SystemID();

    //Logger().debugStream() << "Fleet::MovementPhase rest of move path:";
    //for (std::list<MovePathNode>::const_iterator it2 = it; it2 != move_path.end(); ++it2)
    //    Logger().debugStream() << "... (" << it2->x << ", " << it2->y << ") at object id: " << it2->object_id << " eta: " << it2->eta << (it2->turn_end ? " (end of turn)" : " (during turn)");

    // update next system
    if (m_moving_to != cur_sys_id && next_it != move_path.end() && it != move_path.end()) {
        //Logger().debugStream() << "scanning rest of path...";
        // find next system on path
        for (; next_it != move_path.end(); ++next_it) {
            if (universe.Object<System>(next_it->object_id)) {
                //Logger().debugStream() << "___ setting m_next_system to " << next_it->object_id;
                m_next_system = next_it->object_id;
                break;
            }
        }
    } else {
        m_moving_to = m_next_system = m_prev_system = UniverseObject::INVALID_OBJECT_ID;
    }


    // consume fuel
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

