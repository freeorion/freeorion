#include "Fleet.h"
#include "XMLDoc.h"

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#ifdef FREEORION_BUILD_SERVER
#include "../server/ServerApp.h"
#endif

#include <stdexcept>


Fleet::Fleet() : 
   UniverseObject(),
   m_moving_to(-1)
{
   // TODO
}

Fleet::Fleet(const GG::XMLElement& elem) : 
   UniverseObject(elem.Child("UniverseObject"))
{
   using GG::XMLElement;

   if (elem.Tag() != "Fleet")
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
#ifdef FREEORION_BUILD_SERVER
   ServerApp* server_app = ServerApp::GetApp();
   Empire* empire = (server_app->Empires()).Lookup(empire_id);
   
   if ((empire->HasFleet(ID())) || (empire->HasVisibleFleet(ID())))
   {
      return FULL_VISIBILITY;
   }
#endif   
   return NO_VISIBILITY;
}


GG::XMLElement Fleet::XMLEncode() const
{
   using GG::XMLElement;
   using boost::lexical_cast;

   XMLElement element("Fleet");

   element.AppendChild( UniverseObject::XMLEncode() );

   XMLElement ships("m_ships");
   for(const_iterator itr=begin(); itr != end(); itr++)
   {
      XMLElement ship("ship");
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

   XMLElement element("Fleet");

   element.AppendChild( UniverseObject::XMLEncode() );

   XMLElement ships("m_ships");
   for(const_iterator itr=begin(); itr != end(); itr++)
   {
      XMLElement ship("ship");
      ship.SetAttribute( "value", lexical_cast<std::string>(*itr) );
      ships.AppendChild(ship);
   }
   element.AppendChild(ships);

   XMLElement moving_to("m_moving_to");
   moving_to.SetAttribute( "value", lexical_cast<std::string>(m_moving_to) );
   element.AppendChild(moving_to);

   return element;
}

void Fleet::SetMoveOrders(int id)
{
   // TODO
}

void Fleet::AddShips(const std::vector<int>& ships)
{
   for (unsigned int i = 0; i < ships.size(); ++i) {
      m_ships.insert(ships[i]);

      // TODO: store fleet id into the ship objects
   }
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
#ifdef FREEORION_BUILD_SERVER
         ServerApp::GetApp()->Universe().Delete(ships[i]);
#endif
      }
   }
   return retval;
}

void Fleet::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void Fleet::PopGrowthProductionResearchPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}



