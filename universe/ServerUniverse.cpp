#include "ServerUniverse.h"
#include "UniverseObject.h"
#include "Fleet.h"
#include "../GG/XML/XMLDoc.h"
#include "../server/ServerApp.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>


ServerUniverse::ServerUniverse()
{
   // intialize the ID counter
   m_last_allocated_id = 0;   

}

ServerUniverse::ServerUniverse(Shape shape, int stars, int players, int ai_players)
{
   std::vector<int> homeworlds;

   // intialize the ID counter
   m_last_allocated_id = 0;   

   // generate the galaxy and get a list of the homeworlds
   switch (shape) {
   case SPIRAL_2:
      generate_spiral_galaxy(2, stars, players, homeworlds);
      break;
   case SPIRAL_3:
      generate_spiral_galaxy(3, stars, players, homeworlds);
      break;
   case SPIRAL_4:
      generate_spiral_galaxy(4, stars, players, homeworlds);
      break;
   case ELLIPTICAL:
      generate_eliptical_galaxy(stars, players, homeworlds);
      break;
   case IRREGULAR:
      generate_irregular_galaxy(stars, players, homeworlds);
      break;
   default:
      // unknown shape, use irregular as default
      ServerApp* server_app = ServerApp::GetApp();
      server_app->Logger().errorStream() << "ServerUniverse::ServerUniverse : Unknown shapez: "<< shape << ".  Using IRREGULAR as default.";
      generate_irregular_galaxy(stars, players, homeworlds);
   }

   // create empires and assign homeworlds, names, colors, and fleet ranges to them
   // for each empire
      // select name at random from default list
      // select color at randome from default list
      // select fleet ID range, based on loop itr
      // select homeworld, based on loop itr
      // create new Empire object through empire manager
      // set ownership of home planet
      // create population and industry on home planet
      // create starting fleet
   
}

ServerUniverse::ServerUniverse(const std::string& map_file, int stars, int players, int ai_players)
{
   // intialize the ID counter
   m_last_allocated_id = 0;   

   // TODO
}

ServerUniverse::ServerUniverse(const GG::XMLElement& elem) : 
   ClientUniverse(elem)
{
}

ServerUniverse::~ServerUniverse()
{
   // TODO
}

int ServerUniverse::Insert(UniverseObject* obj)
{
   if (obj == NULL)
   {
      ServerApp* server_app = ServerApp::GetApp();
      server_app->Logger().errorStream() << "ServerUniverse::Insert : Attempt to add object to object map failed because the object pointer is NULL.";
            
      return INVALID_OBJECT_ID;
   }

   int object_id;

   //Need to determine if object is a fleet before allocating ID
   if (dynamic_cast<Fleet*>(obj))
   {
      // Fleet ID may have already been allocated by
      // the client in the case of a fleet split order.
      // In this case the previously allocated ID is used.
      if (obj->ID() != INVALID_OBJECT_ID)
      {
         if (m_objects.find(obj->ID()) != m_objects.end())
         {
            // This ID is already in use!  The object will not be added.
            ServerApp* server_app = ServerApp::GetApp();
            server_app->Logger().errorStream() << "ServerUniverse::Insert : Attempt to add fleet to object map failed because ID (" << obj->ID() << ") previously assigned to the fleet is already in use.";
            
            return INVALID_OBJECT_ID;           
         }   
         m_objects[obj->ID()] = obj;
         return obj->ID();                  
      }
      else
      {
         ServerApp* server_app = ServerApp::GetApp();
         const std::set<int> obj_owners = obj->Owners();  // for a fleet there will only be 1 owner
      
         Empire* empire = (server_app->Empires()).Lookup(*(obj_owners.begin()));
         int empire_fleet_id_min = MIN_SHIP_ID;  // NEED TO FIX!!!!!!!
         int empire_fleet_id_max = MAX_SHIP_ID;  // NEED TO FIX!!!!!!!
         for (object_id = empire_fleet_id_min; object_id <= empire_fleet_id_max; object_id++)
         {
            if (m_objects.find(object_id) == m_objects.end())
            {
               // ID is unused, store object here
               m_objects[object_id] = obj;
               obj->SetID(object_id);
               return object_id;
            }
         }
         
         // No available ID's!
         server_app->Logger().errorStream() << "ServerUniverse::Insert : Attempt to add fleet to object map failed because ID pool is exhausted.";
         
         return INVALID_OBJECT_ID;         
      }
   }
   
   // Object is not a fleet.  Search the general pool starting after the last 
   // allocated ID.
   object_id = m_last_allocated_id + 1;
   for (; object_id != m_last_allocated_id; object_id++)
   {
      if (m_objects.find(object_id) == m_objects.end())
      {
         // ID is unused, store object here
         m_objects[object_id] = obj;
         m_last_allocated_id = object_id;
         // ID must be stored into the object as well
         obj->SetID(object_id);
         return object_id;
      }
      
      // need to roll the counter over if it hits the ID ceiling
      if (object_id+1 == MIN_SHIP_ID)
      {
         object_id = 0;
      }
   }

   // No available ID's!
   ServerApp* server_app = ServerApp::GetApp();
   server_app->Logger().errorStream() << "ServerUniverse::Insert : Attempt to add object to object map failed because ID pool is exhausted.";
   
   return INVALID_OBJECT_ID;         
}


UniverseObject* ServerUniverse::Remove(int id)
{
   UniverseObject* retval = 0;
   iterator it = m_objects.find(id);
   if (it != m_objects.end()) {
      retval = it->second;
      m_objects.erase(id);
   }
   return retval;
}
   
bool ServerUniverse::Delete(int id)
{
   UniverseObject* obj = Remove(id);
   delete obj;
   return obj;
}
   
UniverseObject* ServerUniverse::Object(int id)
{
   iterator it = m_objects.find(id);
   return (it != m_objects.end() ? it->second : 0);
}

void ServerUniverse::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}
   
void ServerUniverse::PopGrowthProductionResearch(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}


void ServerUniverse::generate_spiral_galaxy(int arms, int stars, int players, std::vector<int>& homeworlds)
{
   // TODO
}

void ServerUniverse::generate_eliptical_galaxy(int stars, int players, std::vector<int>& homeworlds)
{
   // TODO
}

void ServerUniverse::generate_irregular_galaxy(int stars, int players, std::vector<int>& homeworlds)
{

   // generate star field
      // generate new star
      // check distance to nearest existing star is acceptable
      // add system to object map
   // select homeworld systems, add planets appropriately
   // add all other planets

}
