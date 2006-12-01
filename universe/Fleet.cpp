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

    inline bool SystemNotReachable(System* system)
    { return !GetUniverse().SystemReachable(system->ID(), Universe::s_encoding_empire); }
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

std::pair<int, int> Fleet::ETA() const
{
    std::pair<int, int> retval;
    if (UnknownRoute()) {
        retval.first = ETA_UNKNOWN;
    } else {
        CalculateRoute();
        if (m_speed > 0.0) {
            retval.first = static_cast<int>(std::ceil(m_travel_distance / m_speed));

            if (!m_travel_route.empty()) {
                std::list<System*>::iterator next_system_it = m_travel_route.begin();
                if (SystemID() == m_travel_route.front()->ID())
                    ++next_system_it;
                System* next = *next_system_it;
                if (next == m_travel_route.back()) {
                    retval.second = retval.first;
                } else {
                    double x = next->X() - X();
                    double y = next->Y() - Y();
                    retval.second = static_cast<int>(std::ceil(std::sqrt(x * x + y * y) / m_speed));
                }
            }
        } else {
            retval.first = ETA_NEVER;
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
{ return m_ships.size(); }

bool Fleet::ContainsShip(int id) const
{ return m_ships.find(id) != m_ships.end(); }

bool Fleet::UnknownRoute() const
{ return m_travel_route.size() == 1 && m_travel_route.front() == 0; }

UniverseObject* Fleet::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(const_cast<Fleet* const>(this)); }

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

void Fleet::GetVisibleRoute(std::list<System*>& travel_route, int moving_to)
{
    std::list<System*>::iterator visible_end_it;
    if (moving_to != m_moving_to) {
        System* final_destination = GetUniverse().Object<System>(moving_to);
        assert(std::find(m_travel_route.begin(), m_travel_route.end(), final_destination) != m_travel_route.end());
        visible_end_it = ++std::find(m_travel_route.begin(), m_travel_route.end(), final_destination);
    } else {
        visible_end_it = m_travel_route.end();
    }
    std::list<System*>::iterator end_it = std::find_if(m_travel_route.begin(), visible_end_it, boost::bind(&SystemNotReachable, _1));
    std::copy(m_travel_route.begin(), end_it, std::back_inserter(travel_route));
    // If no Systems in a nonempty route are known reachable, put a null pointer in the route as a sentinel indicating
    // that the route is unknown, but needs not be recomputed.
    if (travel_route.empty() && !m_travel_route.empty())
        travel_route.push_back(0);
}
