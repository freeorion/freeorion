#include "Fleet.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include "../util/AppInterface.h"
#include "System.h"

#include <stdexcept>

namespace {
const double SHIP_SPEED = 50.0; // "reasonable" speed --can cross galaxy in 20 turns (v0.1 only !!!!)
}


Fleet::Fleet() : 
   UniverseObject(),
   m_moving_to(INVALID_OBJECT_ID)
{
   // TODO
}

Fleet::Fleet(const std::string& name, double x, double y, int owner) :
  UniverseObject(name, x, y),
  m_moving_to(INVALID_OBJECT_ID)
{
   AddOwner(owner);
}

Fleet::Fleet(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject"))
{
   using GG::XMLElement;

   if (elem.Tag().find( "Fleet" ) == std::string::npos )
      throw std::invalid_argument("Attempted to construct a Fleet from an XMLElement that had a tag other than \"Fleet\"");

   XMLElement ships = elem.Child("m_ships");
   for(int i=0; i<ships.NumChildren(); i++)
   {
      m_ships.insert(  lexical_cast<int> (ships.Child(i).Attribute("value") ) );
   }

   m_moving_to = lexical_cast<int> ( elem.Child("m_moving_to").Attribute("value") );
   
}

UniverseObject::Visibility Fleet::Visible(int empire_id) const
{
   // if the fleet is visible it will be listed in the empire's
   // visible fleet list

   Empire* empire = Empires().Lookup(empire_id);
   
   if ( empire && (empire->HasFleet(ID())) || (empire->HasVisibleFleet(ID())))
   {
      return FULL_VISIBILITY;
   }
 
   return NO_VISIBILITY;
}


GG::XMLElement Fleet::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   string ship_name( "Fleet" );
   ship_name += boost::lexical_cast<std::string>( ID()  );
   XMLElement element( ship_name );

   element.AppendChild( UniverseObject::XMLEncode() );

   XMLElement ships("m_ships");
   for(const_iterator itr=begin(); itr != end(); itr++)
   {
       string name( "ship" );
       name += boost::lexical_cast<std::string>( *itr  );
       XMLElement ship( name );

       ship.SetAttribute( "value", lexical_cast<std::string>(*itr) );
       ships.AppendChild(ship);
   }
   element.AppendChild(ships);

   XMLElement moving_to("m_moving_to");
   moving_to.SetAttribute( "value", lexical_cast<std::string>(m_moving_to) );
   element.AppendChild(moving_to);

   return element;
}


GG::XMLElement Fleet::XMLEncode(int empire_id) const
{
   // Fleets are either visible or not, so there is no 
   // difference between the full and partial visibilty
   // XMLEncodes for this class

   using GG::XMLElement;
   using boost::lexical_cast;
   using std::string;

   string ship_name( "Fleet" );
   ship_name += boost::lexical_cast<std::string>( ID()  );
   XMLElement element( ship_name );

   element.AppendChild( UniverseObject::XMLEncode() );

   XMLElement ships("m_ships");
   for(const_iterator itr=begin(); itr != end(); itr++)
   {
       string name( "ship" );
       name += boost::lexical_cast<std::string>( *itr  );
       XMLElement ship( name );

       ship.SetAttribute( "value", lexical_cast<std::string>(*itr) );
       ships.AppendChild(ship);
   }
   element.AppendChild(ships);

   XMLElement moving_to("m_moving_to");
   moving_to.SetAttribute( "value", lexical_cast<std::string>(m_moving_to) );
   element.AppendChild(moving_to);

   return element;
}

int Fleet::ETA() const
{
    int retval = 0;
    if (System* dest = Destination()) {
        double x = dest->X() - X();
        double y = dest->Y() - Y();
        retval = static_cast<int>(std::ceil(std::sqrt(x * x + y * y) / SHIP_SPEED));
    }
    return retval;
}

System* Fleet::Destination() const
{
    return dynamic_cast<System*>(GetUniverse().Object(m_moving_to));
}

void Fleet::SetDestination(int id)
{
    m_moving_to = id;
    StateChangedSignal()();
}

void Fleet::AddShips(const std::vector<int>& ships)
{
   for (unsigned int i = 0; i < ships.size(); ++i) {
      m_ships.insert(ships[i]);

      // TODO: store fleet id into the ship objects
   }
   StateChangedSignal()();
}

void Fleet::AddShip(const int ship_id)
{
   m_ships.insert(ship_id);

   Universe& universe = GetUniverse();
   
   Ship* ship = dynamic_cast<Ship*>(universe.Object(ship_id));
   if (ship == NULL)
   {
      throw std::invalid_argument("Attempted to add a ship to a fleet, but object was missing.");
      return;
   }
   m_ships.insert(ship_id);
   ship->SetFleetID(ID());

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

void Fleet::MovementPhase( )
{
    if (System* destination_system = Destination()) {
        if (System* current_system = GetSystem()) {
            current_system->Remove(ID());
        }

        double direction_x = destination_system->X() - X();
        double direction_y = destination_system->Y() - Y();
        double distance = std::sqrt(direction_x * direction_x + direction_y * direction_y);
        if (distance < SHIP_SPEED) {
            destination_system->Insert(this);
            SetDestination(INVALID_OBJECT_ID);
            // TODO : explore new system
        } else {
            Move(direction_x / distance * SHIP_SPEED, direction_y / distance * SHIP_SPEED);
        }
    }
}

void Fleet::PopGrowthProductionResearchPhase()
{
}



