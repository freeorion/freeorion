#include "Fleet.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include "../util/AppInterface.h"
#include "System.h"

#include <stdexcept>

namespace {
    const double SHIP_SPEED = 50.0; // "reasonable" speed --can cross galaxy in 20 turns (v0.2 only !!!!)
}


Fleet::Fleet() : 
   UniverseObject(),
   m_moving_to(INVALID_OBJECT_ID),
   m_prev_system(INVALID_OBJECT_ID),
   m_next_system(INVALID_OBJECT_ID),
   m_travel_distance(0.0)
{
}

Fleet::Fleet(const std::string& name, double x, double y, int owner) :
   UniverseObject(name, x, y),
   m_moving_to(INVALID_OBJECT_ID),
   m_prev_system(INVALID_OBJECT_ID),
   m_next_system(INVALID_OBJECT_ID),
   m_travel_distance(0.0)
{
   AddOwner(owner);
}

Fleet::Fleet(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject"))
{
    using GG::XMLElement;

    if (elem.Tag().find("Fleet") == std::string::npos )
        throw std::invalid_argument("Attempted to construct a Fleet from an XMLElement that had a tag other than \"Fleet\"");

    try {
        m_ships = GG::ContainerFromString<ShipIDSet>(elem.Child("m_ships").Text());
        m_moving_to = lexical_cast<int>(elem.Child("m_moving_to").Text());
        m_prev_system = lexical_cast<int>(elem.Child("m_prev_system").Text());
        m_next_system = lexical_cast<int>(elem.Child("m_next_system").Text());
    } catch (const boost::bad_lexical_cast& e) {
        Logger().debugStream() << "Caught boost::bad_lexical_cast in Fleet::Fleet(); bad XMLElement was:";
        std::stringstream osstream;
        elem.WriteElement(osstream);
        Logger().debugStream() << "\n" << osstream.str();
        throw;
    }
}

UniverseObject::Visibility Fleet::GetVisibility(int empire_id) const
{
    Empire* empire = 0;
    if (empire_id == Universe::ALL_EMPIRES || OwnedBy(empire_id))
        return FULL_VISIBILITY;
    else
        return PARTIAL_VISIBILITY; // TODO: do something smarter here, such as a range check vs. owned systems and fleets
}


GG::XMLElement Fleet::XMLEncode(int empire_id/* = Universe::ALL_EMPIRES*/) const
{
   // Fleets are either visible or not, so there is no 
   // difference between the full and partial visibilty
   // encodings for this class
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   XMLElement retval("Fleet" + boost::lexical_cast<std::string>(ID()));
   retval.AppendChild(UniverseObject::XMLEncode(empire_id));
   retval.AppendChild(XMLElement("m_ships", GG::StringFromContainer<ShipIDSet>(m_ships)));
   retval.AppendChild(XMLElement("m_moving_to", lexical_cast<std::string>(m_moving_to)));
   retval.AppendChild(XMLElement("m_prev_system", lexical_cast<std::string>(m_prev_system)));
   retval.AppendChild(XMLElement("m_next_system", lexical_cast<std::string>(m_next_system)));
   return retval;
}

const std::list<System*>& Fleet::TravelRoute() const
{
    CalculateRoute();
    return m_travel_route;
}

std::pair<int, int> Fleet::ETA() const
{
    std::pair<int, int> retval;
    CalculateRoute();
    retval.first = static_cast<int>(std::ceil(m_travel_distance / SHIP_SPEED));
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
            retval.second = static_cast<int>(std::ceil(std::sqrt(x * x + y * y) / SHIP_SPEED));
        }
    }
    return retval;
}

System* Fleet::FinalDestination() const
{
    return dynamic_cast<System*>(GetUniverse().Object(m_moving_to));
}

bool Fleet::CanChangeDirectionInRoute() const
{
    // TODO: check tech levels or game options to allow this
    return false;
}

bool Fleet::HasArmedShips() const
{
    for (Fleet::const_iterator it = begin(); it != end(); it++) {   
        if (dynamic_cast<Ship*>(GetUniverse().Object(*it))->IsArmed())
            return true;
    }
    return false;
}

void Fleet::SetRoute(const std::list<System*>& route, double distance)
{
    if (route.empty())
        throw std::invalid_argument("Fleet::SetRoute() : Attempted to set an empty route.");

    if (m_prev_system != SystemID() && m_prev_system == route.front()->ID() && !CanChangeDirectionInRoute())
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

    StateChangedSignal()();
}

void Fleet::AddShips(const std::vector<int>& ships)
{
    for (unsigned int i = 0; i < ships.size(); ++i) {
        if (Ship* s = dynamic_cast<Ship*>(GetUniverse().Object(ships[i]))) {
            s->SetFleetID(ID());
            m_ships.insert(ships[i]);
        } else {
            throw std::invalid_argument("Fleet::AddShips() : Attempted to add an id of a non-ship object to a fleet.");
        }
    }
    StateChangedSignal()();
}

void Fleet::AddShip(const int ship_id)
{
    if (Ship* s = dynamic_cast<Ship*>(GetUniverse().Object(ship_id))) {
        s->SetFleetID(ID());
        m_ships.insert(ship_id);
    } else {
        throw std::invalid_argument("Fleet::AddShip() : Attempted to add an id of a non-ship object to a fleet.");
    }
    StateChangedSignal()();
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
   StateChangedSignal()();
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
   StateChangedSignal()();
   return retval;
}


bool Fleet::RemoveShip(int ship)
{
    bool retval = false;
    iterator it = m_ships.find(ship);
    if (it != m_ships.end()) {
        m_ships.erase(it);
        StateChangedSignal()();
        retval = true;
    }
    return retval;
}

void Fleet::MovementPhase()
{
    if (m_moving_to != INVALID_OBJECT_ID) {
        if (m_travel_route.empty())
            CalculateRoute();

        Logger().debugStream() << "Fleet::MovementPhase() : travel route is:";
        for (std::list<System*>::iterator it = m_travel_route.begin(); it != m_travel_route.end(); ++it) {
            Logger().debugStream() << (*it)->Name();
        }
        if (System* current_system = GetSystem()) {
            Logger().debugStream() << "current_system is " << current_system->Name();
            if (current_system == m_travel_route.front()) {
                m_travel_route.pop_front();
                Logger().debugStream() << "popping " << current_system->Name() << " from front of list";
            }
            current_system->Remove(ID());
        }

        System* next_system = m_travel_route.front();
        double movement_left = SHIP_SPEED;
        Logger().debugStream() << "Fleet \"" << Name() << "\" is moving to system \"" << GetUniverse().Object(m_moving_to)->Name() << "\" with " << movement_left << " movement";
        while (movement_left) {
            Logger().debugStream() << "  top of while(): moving to #" << next_system->ID() << "\"" << next_system->Name() 
                << "\"; movement_left=" << movement_left << " m_prev_system=" << m_prev_system << " m_next_system=" << m_next_system;
            double direction_x = next_system->X() - X();
            double direction_y = next_system->Y() - Y();
            double distance = std::sqrt(direction_x * direction_x + direction_y * direction_y);
            if (distance <= movement_left) {
                Logger().debugStream() << "    distance=" << distance << " <= movement_left=" << movement_left << "; popping " << m_travel_route.front()->Name() << " from the front of the list";
                m_travel_route.pop_front();
                if (m_travel_route.empty()) {
                    Logger().debugStream() << "    m_travel_route is empty";
                    MoveTo(next_system->X(), next_system->Y());
                    next_system->Insert(this);
                    movement_left = 0.0;
                    m_moving_to = m_prev_system = m_next_system = INVALID_OBJECT_ID;
                    m_travel_route.clear();
                    m_travel_distance = 0.0;
                } else {
                    Logger().debugStream() << "    m_travel_route is not empty; continuing...";
                    MoveTo(next_system->X(), next_system->Y());
                    next_system = m_travel_route.front();
                    movement_left -= distance;
                    m_prev_system = m_next_system;
                    m_next_system = m_travel_route.front()->ID();
                    m_travel_distance -= distance;
                    // if we're "at" this system (within some epsilon), insert this fleet into the system
                    if (std::abs(movement_left) < 1.0e-5) {
                        Logger().debugStream() << "    fleet is close enough to non-destination system" << next_system->Name() << "; inserting it...";
                        next_system->Insert(this);
                        movement_left = 0.0;
                    }
                }

                // explore new system
                Empire* empire = Empires().Lookup(*Owners().begin()); // assumes one owner empire per fleet
                empire->AddExploredSystem(m_travel_route.empty() ? SystemID() : m_prev_system);
                Logger().debugStream() << "    Added explored system #" << next_system->ID() << " to empire " << *Owners().begin();
            } else {
                Logger().debugStream() << "    distance=" << distance << " > movement_left=" << movement_left << "; we're done";
                Move(direction_x / distance * movement_left, direction_y / distance * movement_left);
                m_travel_distance -= distance;
                movement_left = 0.0;
            }
        }
        Logger().debugStream() << "[End of Fleet::MovementPhase()] SystemID()=" << SystemID() 
            << " ; movement_left=" << movement_left << " m_prev_system=" << m_prev_system << " m_next_system=" << m_next_system << "\n";
    }
}

void Fleet::PopGrowthProductionResearchPhase()
{
}

void Fleet::CalculateRoute() const
{
    if (m_moving_to != INVALID_OBJECT_ID && m_travel_route.empty()) {
        m_travel_distance = 0.0;
        if (SystemID() == m_prev_system) { // if we haven't actually left yet, we have to move from whichever system we are at now
            std::pair<std::list<System*>, double> path = GetUniverse().ShortestPath(m_prev_system, m_moving_to);
            m_travel_route = path.first;
            m_travel_distance = path.second;
        } else { // if we're between systems, the shortest route may be through either one
            std::pair<std::list<System*>, double> path1 = GetUniverse().ShortestPath(m_next_system, m_moving_to);
            std::pair<std::list<System*>, double> path2 = GetUniverse().ShortestPath(m_prev_system, m_moving_to);
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
        }
    }
}
