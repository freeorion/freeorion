#include "ServerUniverse.h"
#include "UniverseObject.h"
#include "Fleet.h"
#include "Planet.h"
#include "../GG/XML/XMLDoc.h"
#include "../server/ServerApp.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>


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

   // generate the stars
   switch (shape) {
   case SPIRAL_2:
      GenerateSpiralGalaxy(2, stars);
      break;
   case SPIRAL_3:
      GenerateSpiralGalaxy(3, stars);
      break;
   case SPIRAL_4:
      GenerateSpiralGalaxy(4, stars);
      break;
   case ELLIPTICAL:
      GenerateElipticalGalaxy(stars);
      break;
   case IRREGULAR:
      GenerateIrregularGalaxy(stars);
      break;
   default:
      // unknown shape, use irregular as default
      ServerApp* server_app = ServerApp::GetApp();
      server_app->Logger().errorStream() << "ServerUniverse::ServerUniverse : Unknown galaxy shape: "<< shape << ".  Using IRREGULAR as default.";
      GenerateIrregularGalaxy(stars);
   }

   // set up the homeworld systems
   GenerateHomeworlds(players, stars, homeworlds);

   // populate the rest of the planets
   PopulateSystems();

   // create the empires
   GenerateEmpires(players, ai_players, homeworlds);
     
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


void ServerUniverse::GenerateSpiralGalaxy(int arms, int stars)
{
   // TODO
}

void ServerUniverse::GenerateElipticalGalaxy(int stars)
{
   // TODO
}

// predicate for FindOjbects pred itr
bool ServerIsSystem(UniverseObject* obj) { return dynamic_cast<System*>(obj); }


void ServerUniverse::GenerateIrregularGalaxy(int stars)
{
   // TODO: split this function up!  Probably should only generate the systems. 
   //       separate functions can create the homeworlds and other planets and 
   //       can then reduce duplicated code between the various shaped galaxy
   //       generators


   // generate star field
   for (int star_cnt = 0; star_cnt < stars; star_cnt++)
   {
      // generate new star
      std::string star_name("System");  // TODO: read name from default list
      char star_num[3];
      sprintf(star_num, "%i", star_cnt);
      star_name.append(star_num);
      System::StarType star_type = (System::StarType) (System::NUM_STARTYPES * ((double)rand()/(double)RAND_MAX));
      int num_orbits = 5;
      float orbits_rand = (float) ((double)rand()/(double)RAND_MAX);
      if (orbits_rand < 0.05)
      {
         num_orbits = 0;
      }
      else if (orbits_rand < 0.25)
      {
         num_orbits = 1;
      }
      else if (orbits_rand < 0.55)
      {
         num_orbits = 2;
      }
      else if (orbits_rand < 0.85)
      {
         num_orbits = 3;
      }
      else if (orbits_rand < 0.95)
      {
         num_orbits = 4;
      }
      // above 0.95 stays at the default 5
      
      
      bool valid_position = false;
      System* sys_temp = NULL;
            
      while (!valid_position)
      {
         int x_coord = (int) (UNIVERSE_X_SIZE * ((double)rand()/(double)RAND_MAX));
         int y_coord = (int) (UNIVERSE_Y_SIZE * ((double)rand()/(double)RAND_MAX));
         
         sys_temp = new System(star_type, num_orbits, star_name, x_coord, y_coord);       

         int nearest_id = NearestSystem(*sys_temp);
         if (nearest_id == INVALID_OBJECT_ID)
         {
            // occurs when no other systems have been created yet

            valid_position = true;
         }
         else
         {
            System* sys_nearest = dynamic_cast<System*>(Object(nearest_id));
            if (sys_nearest == NULL)
            {
               ServerApp* server_app = ServerApp::GetApp();
               server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Attempt to retrieve System object with ID " << Object(nearest_id) << " resulted in a null pointer.  Skipping distance check.";
               valid_position = true;
            }
            else 
            {
               float sys_dist = sqrt(pow((sys_temp->X()-sys_nearest->X()),2) + pow(sys_temp->Y()-sys_nearest->Y(),2));
               if (sys_dist > MIN_STAR_DISTANCE)  
               {
                  // TODO: min star dist should probably be relative to galaxy size..

                  valid_position = true;
               }
               else
               {
                  // position is no good, delete the object and try again
                  delete sys_temp;
               }
            }
         }
         
      }
      
      // star is ready to go in the object map
      int new_sys_id = Insert(sys_temp);
      
      if (new_sys_id == INVALID_OBJECT_ID)
      {
         ServerApp* server_app = ServerApp::GetApp();
         server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Attemp to insert system " << star_name << " into the object map failed.";
         server_app->Exit(1);  // TODO: should probably handle this error better... but will be difficult...
      }

   }
  
   // stars have all been created
         
}



void ServerUniverse::GenerateHomeworlds(int players, int stars, std::vector<int>& homeworlds)
{
   // select homeworld systems, add planets appropriately
   homeworlds.clear();

   // get a vector of all the systems
   ObjectVec sys_obj_vec = FindObjects(ServerIsSystem);

   while ((int) homeworlds.size() < players)
   {
      // select a system at random
      int system_index = (int) (sys_obj_vec.size() * ((double)rand()/(double)RAND_MAX));
      System* system_temp = dynamic_cast<System*>(sys_obj_vec[system_index]);

      // make sure it has planets
      if (system_temp->Orbits() == 0)
      {
         continue;
      }
      
      // make sure it is not already in the homeworlds list 
      // (only homeworlds will have planets at this point)
      if (system_temp->begin() != system_temp->end())
      {
         continue;
      }         

      // make sure it is not too close to an existing homeworld
      int prox_limit = 0;

      switch(stars) {
      case 50:
         prox_limit = HOMEWORLD_PROXIMITY_LIMIT_V_SMALL_UNI;
         break;
      case 100:
         prox_limit = HOMEWORLD_PROXIMITY_LIMIT_SMALL_UNI;
         break;
      case 150:
         prox_limit = HOMEWORLD_PROXIMITY_LIMIT_MEDIUM_UNI;
         break;
      case 200:
         prox_limit = HOMEWORLD_PROXIMITY_LIMIT_LARGE_UNI;
         break;
      case 250:
         prox_limit = HOMEWORLD_PROXIMITY_LIMIT_V_LARGE_UNI;
         break;
      case 300:
         prox_limit = HOMEWORLD_PROXIMITY_LIMIT_ENORMOUS_UNI;
         break;
      default:
         ServerApp* server_app = ServerApp::GetApp();
         server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Unknown galaxy size (" << stars << " stars).  Using 5 as default homeworld proximity limit.";
         prox_limit = 5;
      }

      // check nearest system
      int nearest_id = NearestSystem(*system_temp);
      System* sys_nearest = dynamic_cast<System*>(Object(nearest_id));
      
      if (sys_nearest->begin() != sys_nearest->end())
      {
         continue;
      }
        
      int prox_cnt = 1;
      for (; prox_cnt < prox_limit; prox_cnt++)
      {
         nearest_id = NearestSystem(*system_temp, *sys_nearest);
         sys_nearest = dynamic_cast<System*>(Object(nearest_id));
            
         if (sys_nearest->begin() != sys_nearest->end())
         {
            break;
         }
      }
      
      // check if loop was exited
      if (prox_cnt != prox_limit)
      {
         // homeworld detected
         continue;
      }

        
      // select an orbit at random.  Note: ultimately this orbit number may not be
      // the one that the system object places this planet at. It is still good to
      // randomize the order in which the homeworlds are added to the system's map.
      int home_orbit = (int) (system_temp->Orbits() * ((double)rand()/(double)RAND_MAX));

      for (int orbit = 0; orbit < system_temp->Orbits(); orbit++)
      {
         Planet::PlanetSize plt_size;       

         // Type is selected at random. Type only affects the planet's image in v0.1
         Planet::PlanetType plt_type = (Planet::PlanetType) (Planet::MAX_PLANET_TYPE * ((double)rand()/(double)RAND_MAX));

         if (orbit == home_orbit)
         {
            // create a sufficiently sized planet for a homeworld
            plt_size = Planet::SZ_LARGE;
            
         }
         else
         {
            float size_rnd = (float) (100 * (double)rand()/(double)RAND_MAX);
            if (size_rnd < 10)
            {
               plt_size = Planet::SZ_TINY;
            }
            else if (size_rnd < 35)
            {
               plt_size = Planet::SZ_SMALL;
            }
            else if (size_rnd < 65)
            {
               plt_size = Planet::SZ_MEDIUM;
            }
            else if (size_rnd < 90)
            {
               plt_size = Planet::SZ_LARGE;
            }
            else
            {
               plt_size = Planet::SZ_HUGE;
            }

         }

         Planet* planet = new Planet(plt_type, plt_size);
         
         
         // Add planet to system map
         system_temp->Insert(planet);
         planet->SetSystemID(system_temp->ID());

         // Add planet to universe map
         int planet_id = Insert(planet);
         if (planet_id == INVALID_OBJECT_ID)
         {
            ServerApp* server_app = ServerApp::GetApp();
            server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Attemp to insert planet into the object map failed.";
            server_app->Exit(1);  // TODO: should probably handle this error better... but will be difficult...
         }

         if (orbit == home_orbit)
         {
            // add to the homeworlds list
            homeworlds.push_back(planet_id);

         }
      }  // end adding planets
   }  // end adding homeworld systems

}


void ServerUniverse::PopulateSystems()
{
   // populate non-homeworld systems

   // get a vector of all the systems
   ObjectVec sys_obj_vec = FindObjects(ServerIsSystem);

   for (ObjectVec::iterator sys_itr = sys_obj_vec.begin(); sys_itr != sys_obj_vec.end(); sys_itr++)
   {
      System* system = dynamic_cast<System*>(*sys_itr);
   
      // check whether this system has already been populated
      if (system->begin() != system->end())
      {
         continue;
      }

      if (system->Orbits() == 0)
      {
         // no planets...
         continue;
      }
      
      for (int planet_cnt = 0; planet_cnt < system->Orbits(); planet_cnt++)
      {
         Planet::PlanetSize plt_size;       

         // Type is selected at random. Type only affects the planet's image in v0.1
         Planet::PlanetType plt_type = (Planet::PlanetType) (Planet::MAX_PLANET_TYPE * ((double)rand()/(double)RAND_MAX));
         float size_rnd = (float) (100 * (double)rand()/(double)RAND_MAX);
         if (size_rnd < 10)
         {
            plt_size = Planet::SZ_TINY;
         }
         else if (size_rnd < 35)
         {
            plt_size = Planet::SZ_SMALL;
         }
         else if (size_rnd < 65)
         {
            plt_size = Planet::SZ_MEDIUM;
         }
         else if (size_rnd < 90)
         {
            plt_size = Planet::SZ_LARGE;
         }
         else
         {
            plt_size = Planet::SZ_HUGE;
         }
         
         Planet* planet = new Planet(plt_type, plt_size);
         
         // Add planet to system map
         system->Insert(planet);
         planet->SetSystemID(system->ID());

         // Add planet to universe map
         int planet_id = Insert(planet);
         if (planet_id == INVALID_OBJECT_ID)
         {
            ServerApp* server_app = ServerApp::GetApp();
            server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Attemp to insert planet into the object map failed.";
            server_app->Exit(1);  // TODO: should probably handle this error better... but will be difficult...
         }

      }  // end adding planets
   }

}


void ServerUniverse::GenerateEmpires(int players, int ai_players, std::vector<int>& homeworlds)
{
   // create empires and assign homeworlds, names, colors, and fleet ranges to them
   // for each empire
   
   int fleet_id_rng_size =(int) ((MAX_SHIP_ID - MIN_SHIP_ID)/players);
   
   ServerApp* server_app = ServerApp::GetApp();
   ServerEmpireManager empire_mgr = server_app->Empires();

   for (int empire_cnt = 0; empire_cnt < players; empire_cnt++)
   {
      // TODO: select name at random from default list
      std::string empire_name("Empire");
      char empire_num[3];
      sprintf(empire_num, "%i", empire_cnt);
      empire_name.append(empire_num);


      // TODO: select color at random from default list
      GG::Clr* empire_color = new GG::Clr(((double)rand()/(double)RAND_MAX), ((double)rand()/(double)RAND_MAX), ((double)rand()/(double)RAND_MAX), (double) 0);
  
      // select fleet ID range, based on loop itr
      int empire_min_flt_id = MIN_SHIP_ID + (fleet_id_rng_size * empire_cnt);
      int empire_max_flt_id = MIN_SHIP_ID + (fleet_id_rng_size * (empire_cnt+1)) - 1;

      // select homeworld, based on loop itr
      int home_planet_id = homeworlds[empire_cnt];

      // determine control status
      Empire::ControlStatus control;

      if (empire_cnt < ai_players)
      {
         control = Empire::CONTROL_AI;
      }
      else
      {
         control = Empire::CONTROL_HUMAN;
      }

      // create new Empire object through empire manager
      Empire* new_empire = empire_mgr.CreateEmpire(empire_name, *empire_color, home_planet_id, control);
      
      // TODO: need to input the empire's ship ID range somehow

      // set ownership of home planet
      int empire_id = new_empire->EmpireID();

      Planet* home_planet = dynamic_cast<Planet*>(Object(homeworlds[empire_cnt]));
      home_planet->AddOwner(empire_id);

      // TODO: adding an owner to a planet should probably add that owner to the 
      //       system automatically...
      System* home_system = dynamic_cast<System*>(Object(home_planet->SystemID()));
      home_system->AddOwner(empire_id);

      // create population and industry on home planet
      home_planet->AdjustPop(20);
      home_planet->AdjustIndustry(0.10);  // NEED TO ADD THIS FUNCTION!!!
      home_planet->AdjustDefBases(3);

      // TODO: create starting fleet. Will need to add the default ship types to the empire
      //       then create 2 scouts and 1 colony ship...
   }
}
