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

namespace {
    const double MAX_SHIP_SPEED = 500.0; // max allowed speed of ship movement

    inline bool SystemNotReachable(System* system, int empire_id) {
        return !GetUniverse().SystemReachable(system->ID(), empire_id);
    }
}

// static(s)
const int Fleet::ETA_NEVER = -1;
const int Fleet::ETA_UNKNOWN = -2;

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
{ return m_ships.begin(); }

Fleet::const_iterator Fleet::end() const
{ return m_ships.end(); }

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
    return m_travel_route;
}

std::list<MovePathNode> Fleet::MovePath() const
{
    return MovePath(TravelRoute());
}

std::list<MovePathNode> Fleet::MovePath(const std::list<System*>& route) const
{
    std::list<MovePathNode> retval = std::list<MovePathNode>();

    if (route.empty()) return retval;                                       // nowhere to go => empty path
    if (route.size() == 2 && route.front() == route.back()) return retval;  // nowhere to go => empty path
    if (this->Speed() < 1.0e-5) return retval;                              // unable to move (epsilon taken from Fleet::MovementPhase())

    MovePathNode cur_pos(this->X(), this->Y(), true, 0);    // initialize to initial position of fleet.

    int running_eta = 1;    // turns to each node
    double this_turn_travel_dist_left = this->Speed();
    
    // simulate moving fleet to each stop along path in sequence, remembering nodes at each stop and 
    // each time a turn's travel distance runs out
    for(std::list<System*>::const_iterator next_sys_it = route.begin(); next_sys_it != route.end(); ++next_sys_it) {

        const System* next_sys = *next_sys_it;

        if (GetSystem() && GetSystem() == next_sys) continue;   // don't add system fleet starts in to the path


        // get direction and distance to next system
        double x_dist = next_sys->X() - cur_pos.x;
        double y_dist = next_sys->Y() - cur_pos.y;
        double next_sys_dist = std::sqrt(x_dist * x_dist + y_dist * y_dist);
        double unit_vec_x = x_dist / next_sys_dist;
        double unit_vec_y = y_dist / next_sys_dist;
        
        
        // take turn-travel-distance steps until next system is reached
        while (next_sys_dist > 1.0e-5) {   // epsilon taken from Fleet::MovementPhase()

            if (next_sys_dist > this_turn_travel_dist_left) {
                // system is further away than the fleet can travel this turn -> take the rest of this turn's movement

                // set current position to position after this turn's movement
                double step_vec_x = unit_vec_x * this_turn_travel_dist_left;
                double step_vec_y = unit_vec_y * this_turn_travel_dist_left;
                cur_pos.x += step_vec_x;
                cur_pos.y += step_vec_y;
                cur_pos.turn_end = true;    // new position is at the end of a turn's movement
                cur_pos.eta = running_eta;

                // add new node to list
                retval.push_back(cur_pos);

                // update distance to next system to account for this turn's movement
                next_sys_dist -= this_turn_travel_dist_left;

                // next turn, can move a full turn's movement
                this_turn_travel_dist_left = this->Speed();

                ++running_eta;

            } else {
                // system can be reached this turn

                // set current position to system's position
                cur_pos.x = next_sys->X();
                cur_pos.y = next_sys->Y();
                cur_pos.turn_end = false;
                cur_pos.eta = running_eta;

                // add new node to list
                retval.push_back(cur_pos);

                // update distance that can still be travelled this turn
                this_turn_travel_dist_left -= next_sys_dist;

                // have arrived at system ensure inner loop ends and next system is fetched
                next_sys_dist = 0.0;

                // TO DO: Account for fuel use.  If fleet has insufficient fuel, it can't move towards the next system
            }
        }
    }
    return retval;
}

std::pair<int, int> Fleet::ETA() const
{
    // if route is unknown, ETA is unknown
    if (UnknownRoute())
        return std::make_pair(ETA_UNKNOWN, ETA_UNKNOWN);

    // if next system is an invalid system id, the fleet isn't moving, so ETA is 0
    if (m_next_system == UniverseObject::INVALID_OBJECT_ID)
        return std::make_pair(0, 0);

    // if fleet can't move, ETA is never
    if (m_speed <= 0.0)
        return std::make_pair(ETA_NEVER, ETA_NEVER);

    // otherwise, need to do calcuations to determine ETA
    std::pair<int, int> retval(ETA_NEVER, ETA_NEVER);

    CalculateRoute();

    // determine fuel available to fleet (fuel of the ship that has the least fuel in the fleet)
    const Universe& universe = GetUniverse();
    double fuel = Meter::METER_MAX;
    for (const_iterator ship_it = begin(); ship_it != end(); ++ship_it) {
        const Ship* ship = universe.Object<Ship>(*ship_it);
        assert(ship);
        fuel = std::min(fuel, ship->MeterPoints(METER_FUEL));
    }

    //// if fleet has no fuel, it can't move
    //if (fuel < 1.0)
    //    return std::make_pair(ETA_NEVER, ETA_NEVER);

    // if fuel doesn't limit speed, ETA is just distance / speed
    retval.first = static_cast<int>(std::ceil(m_travel_distance / m_speed));
    if (retval.first > 500)
        retval.first = ETA_NEVER;   // avoid overly-large ETA numbers

    if (!m_travel_route.empty()) {
        std::list<System*>::iterator next_system_it = m_travel_route.begin();
        if (SystemID() == m_travel_route.front()->ID())
            ++next_system_it;
        assert(next_system_it != m_travel_route.end());
        System* next = *next_system_it;
        if (next == m_travel_route.back()) {
            retval.second = retval.first;
        } else {
            double x = next->X() - X();
            double y = next->Y() - Y();
            retval.second = static_cast<int>(std::ceil(std::sqrt(x * x + y * y) / m_speed));
        }
    }

    return retval;
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
{ return m_speed; }

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
            if (Fleet* old_fleet = s->GetFleet()) {
                old_fleet->RemoveShip(ships[i]);
            }
            s->SetFleetID(ID());
            s->MoveTo(X(), Y());
            if (System* system = GetSystem()) {
                system->Insert(s);
            } else {
                s->SetSystem(SystemID());
            }
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
    if (Ship* s = GetUniverse().Object<Ship>(ship_id)) {
        if (Fleet* old_fleet = s->GetFleet()) {
            old_fleet->RemoveShip(ship_id);
        }
        s->SetFleetID(ID());
        s->MoveTo(X(), Y());
        if (System* system = GetSystem()) {
            system->Insert(s);
        } else {
            s->SetSystem(SystemID());
        }
        m_ships.insert(ship_id);
    } else {
        throw std::invalid_argument("Fleet::AddShip() : Attempted to add an id of a non-ship object to a fleet.");
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
    UniverseObject::SetSystem(sys);
    for (iterator it = begin(); it != end(); ++it) {
        UniverseObject* obj = GetUniverse().Object(*it);
        assert(obj);
        obj->SetSystem(sys);
    }
}

void Fleet::MovementPhase()
{
    if (m_moving_to != INVALID_OBJECT_ID) {
        if (m_travel_route.empty())
            CalculateRoute();

        if (System* current_system = GetSystem()) {
            if (current_system == m_travel_route.front())
                m_travel_route.pop_front();
            current_system->Remove(ID());
            SetSystem(INVALID_OBJECT_ID);
            for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
                current_system->Remove(*it);
                Ship* ship = GetUniverse().Object<Ship>(*it);
                if (ship) ship->SetSystem(INVALID_OBJECT_ID);
            }
        }

        System* next_system = m_travel_route.front();
        double movement_left = m_speed;
        while (movement_left) {
            double direction_x = next_system->X() - X();
            double direction_y = next_system->Y() - Y();
            double distance = std::sqrt(direction_x * direction_x + direction_y * direction_y);
            if (distance <= movement_left) {
                m_travel_route.pop_front();
                if (m_travel_route.empty()) {
                    MoveTo(next_system->X(), next_system->Y());
                    next_system->Insert(this);
                    for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
                        UniverseObject* ship = GetUniverse().Object(*it);
                        ship->MoveTo(X(), Y());
                        next_system->Insert(ship);
                    }
                    movement_left = 0.0;
                    m_moving_to = m_prev_system = m_next_system = INVALID_OBJECT_ID;
                    m_travel_route.clear();
                    m_travel_distance = 0.0;
                } else {
                    MoveTo(next_system->X(), next_system->Y());
                    next_system = m_travel_route.front();
                    movement_left -= distance;
                    m_prev_system = m_next_system;
                    m_next_system = m_travel_route.front()->ID();
                    m_travel_distance -= distance;
                    // if we're "at" this system (within some epsilon), insert this fleet into the system
                    if (std::abs(movement_left) < 1.0e-5) {
                        next_system->Insert(this);
                        for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
                            UniverseObject* ship = GetUniverse().Object(*it);
                            ship->MoveTo(X(), Y());
                            next_system->Insert(ship);
                        }
                        movement_left = 0.0;
                    } else {
                        for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
                            UniverseObject* ship = GetUniverse().Object(*it);
                            ship->MoveTo(X(), Y());
                        }
                    }
                }

                // explore new system
                Empire* empire = Empires().Lookup(*Owners().begin()); // assumes one owner empire per fleet
                empire->AddExploredSystem(m_travel_route.empty() ? SystemID() : m_prev_system);
            } else {
                Move(direction_x / distance * movement_left, direction_y / distance * movement_left);
                for (ShipIDSet::iterator it = m_ships.begin(); it != m_ships.end(); ++it) {
                    UniverseObject* ship = GetUniverse().Object(*it);
                    ship->MoveTo(X(), Y());
                }
                m_travel_distance -= distance;
                movement_left = 0.0;
            }
        }
    }
}

Fleet::iterator Fleet::begin()
{ return m_ships.begin(); }

Fleet::iterator Fleet::end()
{ return m_ships.end(); }

void Fleet::PopGrowthProductionResearchPhase()
{
    // ensure that any newly opened or closed routes are taken into account
    m_travel_route.clear();
    CalculateRoute();
}

void Fleet::CalculateRoute() const
{
    if (m_moving_to != INVALID_OBJECT_ID && m_travel_route.empty()) {
        m_travel_distance = 0.0;
        if (SystemID() == m_prev_system) { // if we haven't actually left yet, we have to move from whichever system we are at now
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
