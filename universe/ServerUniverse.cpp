#include "ServerUniverse.h"
#include "UniverseObject.h"
#include "Fleet.h"
#include "Planet.h"
#include "XMLDoc.h"
#include "../server/ServerApp.h"
#include "Predicates.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>


namespace {
// These specify the minimum distance of the homeworlds from each other in terms
// of systems separating them. i.e. for a very small galaxy no homeworld should be 
// within the 3 nearest systems to any other homeworld
const int HOMEWORLD_PROXIMITY_LIMIT_V_SMALL_UNI  =   3;
const int HOMEWORLD_PROXIMITY_LIMIT_SMALL_UNI    =   4;
const int HOMEWORLD_PROXIMITY_LIMIT_MEDIUM_UNI   =   5;
const int HOMEWORLD_PROXIMITY_LIMIT_LARGE_UNI    =   5;
const int HOMEWORLD_PROXIMITY_LIMIT_V_LARGE_UNI  =   5;
const int HOMEWORLD_PROXIMITY_LIMIT_ENORMOUS_UNI =   6;

// only defined for 1 <= n <= 15
std::string RomanNumber(int n)
{
    std::string retval;
    switch (n) {
    case  1: retval = "I"; break;
    case  2: retval = "II"; break;
    case  3: retval = "III"; break;
    case  4: retval = "IV"; break;
    case  5: retval = "V"; break;
    case  6: retval = "VI"; break;
    case  7: retval = "VII"; break;
    case  8: retval = "VIII"; break;
    case  9: retval = "IX"; break;
    case 10: retval = "X"; break;
    case 11: retval = "XI"; break;
    case 12: retval = "XII"; break;
    case 13: retval = "XIII"; break;
    case 14: retval = "XIV"; break;
    case 15: retval = "XV"; break;
    default: retval = "ERROR"; break;
    }
    return retval;
}

void LoadPlanetNames(std::list<std::string>& names)
{
    std::ifstream ifs("default/starnames.txt");
    while (ifs) {
        std::string latest_name;
        std::getline(ifs, latest_name);
        if (latest_name != "")
            names.push_back(latest_name);
    }
}
}

ServerUniverse::ServerUniverse() : 
    m_last_allocated_id(-1)
{
}

void ServerUniverse::CreateUniverse(Shape shape, int size, int players, int ai_players)
{
   ServerApp* server_app = ServerApp::GetApp();
   server_app->Logger().debugStream() << "Creating universe with " << size << " stars and " << players << " players.";

   std::vector<int> homeworlds;

   // wipe out anything present in the object map
   for (ObjectMap::iterator itr = m_objects.begin(); itr != m_objects.end(); ++itr)
      delete itr->second;
   m_objects.clear();

   m_last_allocated_id = -1;

   // generate the stars
   switch (shape) {
#if 0
   case SPIRAL_2:
      GenerateSpiralGalaxy(2, size);
      break;
   case SPIRAL_3:
      GenerateSpiralGalaxy(3, size);
      break;
   case SPIRAL_4:
      GenerateSpiralGalaxy(4, size);
      break;
   case ELLIPTICAL:
      GenerateElipticalGalaxy(size);
      break;
#endif
   case IRREGULAR:
      GenerateIrregularGalaxy(size);
      break;
   default:
      server_app->Logger().errorStream() << "ServerUniverse::ServerUniverse : Unknown galaxy shape: "<< shape << ".  Using IRREGULAR as default.";
      GenerateIrregularGalaxy(size);
   }

#if 1
   GenerateHomeworlds(players, size, homeworlds);
#endif
#if 1
   PopulateSystems();
#endif
#if 1
   GenerateEmpires(players, ai_players, homeworlds);
#endif
}

void ServerUniverse::CreateUniverse(const std::string& map_file, int size, int players, int ai_players)
{
   // intialize the ID counter
   m_last_allocated_id = -1;   

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
            
      return UniverseObject::INVALID_OBJECT_ID;
   }

   int object_id;

   //Need to determine if object is a fleet before allocating ID
   if (dynamic_cast<Fleet*>(obj))
   {
      // Fleet ID may have already been allocated by
      // the client in the case of a fleet split order.
      // In this case the previously allocated ID is used.
      if (obj->ID() != UniverseObject::INVALID_OBJECT_ID)
      {
         if (m_objects.find(obj->ID()) != m_objects.end())
         {
            // This ID is already in use!  The object will not be added.
            ServerApp* server_app = ServerApp::GetApp();
            server_app->Logger().errorStream() << "ServerUniverse::Insert : Attempt to add fleet to object map failed because ID (" << obj->ID() << ") previously assigned to the fleet is already in use.";
            
            return UniverseObject::INVALID_OBJECT_ID;           
         }   
         m_objects[obj->ID()] = obj;
         return obj->ID();                  
      }
      else
      {
         ServerApp* server_app = ServerApp::GetApp();
         const std::set<int> obj_owners = obj->Owners();  // for a fleet there will only be 1 owner
      
         Empire* empire = (server_app->Empires()).Lookup(*(obj_owners.begin()));
         int empire_fleet_id_min = empire->FleetIDMin();
         int empire_fleet_id_max = empire->FleetIDMax();
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
         
         return UniverseObject::INVALID_OBJECT_ID;         
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
      if (object_id + 1 == UniverseObject::MIN_SHIP_ID)
      {
         object_id = 0;
      }
   }

   // No available ID's!
   ServerApp* server_app = ServerApp::GetApp();
   server_app->Logger().errorStream() << "ServerUniverse::Insert : Attempt to add object to object map failed because ID pool is exhausted.";
   
   return UniverseObject::INVALID_OBJECT_ID;         
}


UniverseObject* ServerUniverse::Remove(int id)
{
   UniverseObject* retval = 0;
   iterator it = m_objects.find(id);
   if (it != m_objects.end()) {
      retval = it->second;
      m_objects.erase(id);
      if (System* sys = dynamic_cast<System*>(Object(retval->SystemID())))
          sys->Remove(id);
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

void ServerUniverse::GenerateIrregularGalaxy(int stars)
{
    std::list<std::string> star_names;
    LoadPlanetNames(star_names);

    // generate star field
    for (int star_cnt = 0; star_cnt < stars; ++star_cnt) {
        // generate new star
        int star_name_idx = rand() % star_names.size();
        std::list<std::string>::iterator it = star_names.begin();
        std::advance(it, star_name_idx);
        std::string star_name(*it);
        star_names.erase(it);
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
        System* sys_temp = 0;

        while (!valid_position) {
            int x_coord = (int) (UNIVERSE_WIDTH * ((double)rand()/(double)RAND_MAX));
            int y_coord = (int) (UNIVERSE_WIDTH * ((double)rand()/(double)RAND_MAX));
         
            sys_temp = new System(star_type, num_orbits, star_name, x_coord, y_coord);       

#if 0
            int nearest_id = NearestSystem(*sys_temp);
            if (nearest_id == UniverseObject::INVALID_OBJECT_ID) {
                // occurs when no other systems have been created yet

                valid_position = true;
            } else {
                System* sys_nearest = dynamic_cast<System*>(Object(nearest_id));
                if (sys_nearest == NULL) {
                    ServerApp* server_app = ServerApp::GetApp();
                    server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Attempt to retrieve System object with ID " << Object(nearest_id) << " resulted in a null pointer.  Skipping distance check.";
                    valid_position = true;
                } else {
                    float sys_dist = sqrt(pow((sys_temp->X()-sys_nearest->X()),2) + pow(sys_temp->Y()-sys_nearest->Y(),2));
                    if (sys_dist > MIN_STAR_DISTANCE) {
                        // TODO: min star dist should probably be relative to galaxy size..

                        valid_position = true;
                    } else {
                        // position is no good, delete the object and try again
                        delete sys_temp;
                    }
                }
            }
#else
            valid_position = true;
#endif
        }

        // star is ready to go in the object map
        int new_sys_id = Insert(sys_temp);

        if (new_sys_id == UniverseObject::INVALID_OBJECT_ID) {
            ServerApp* server_app = ServerApp::GetApp();
            server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Attemp to insert system " << star_name << " into the object map failed.";
            server_app->Exit(1);  // TODO: should probably handle this error better... but will be difficult...
        }
    }
    // stars have all been created
}



void ServerUniverse::GenerateHomeworlds(int players, int stars, std::vector<int>& homeworlds)
{
   int safety = 0;

   // find the min range between homeworlds
   int prox_limit = 0;
   
   if (stars <= 50)
       prox_limit = HOMEWORLD_PROXIMITY_LIMIT_V_SMALL_UNI;
   else if (stars <= 100)
       prox_limit = HOMEWORLD_PROXIMITY_LIMIT_SMALL_UNI;
   else if (stars <= 150)
      prox_limit = HOMEWORLD_PROXIMITY_LIMIT_MEDIUM_UNI;
   else if (stars <= 200)
      prox_limit = HOMEWORLD_PROXIMITY_LIMIT_LARGE_UNI;
   else if (stars <= 250)
      prox_limit = HOMEWORLD_PROXIMITY_LIMIT_V_LARGE_UNI;
   else
      prox_limit = HOMEWORLD_PROXIMITY_LIMIT_ENORMOUS_UNI;

   // select homeworld systems, add planets appropriately
   homeworlds.clear();

   // get a vector of all the systems
   ObjectVec sys_obj_vec = FindObjects(IsSystem);

   assert(!sys_obj_vec.empty());
      
   while (static_cast<int>(homeworlds.size()) < players)
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

#if 0
      // next, need to verify that no systems within the proximity limit
      // have been selected already as homewords

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
      
      // check how loop was exited
      if (prox_cnt != prox_limit)
      {
         // homeworld detected
         continue;
      }
#endif
      // all checks have passed, this system will host a homeworld.
        
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

         // Add planet to universe map
         int planet_id = Insert(planet);
         if (planet_id == UniverseObject::INVALID_OBJECT_ID)
         {
            ServerApp* server_app = ServerApp::GetApp();
            server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Attemp to insert planet into the object map failed.";
            server_app->Exit(1);  // TODO: should probably handle this error better... but will be difficult...
         }

         // Add planet to system map
         system_temp->Insert(planet, orbit);

         // name the planet
         planet->Rename(system_temp->Name() + " " + RomanNumber(orbit + 1));

         if (orbit == home_orbit)
         {
            // add to the homeworlds list
            homeworlds.push_back(planet_id);

         }
      }  // end adding planets


      // It is possible, given the configuration (number of stars, number of players)
      // and particular geography generated, that it will be impossible to find suitable
      // home systems for all players. This would result in an infinite loop since the
      // system list is being searched and tested at random. The following is a half-assed
      // effort to bail out when this occurs.

      safety++;
      
      if (safety >= (stars*players*1.5))
      {
         // may not be possible to satisfy homeworld conditions
         // fail the setup..
             ServerApp* server_app = ServerApp::GetApp();
             server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Could not find enough eligible homeworld planets.";
             server_app->Exit(1);  // TODO: should probably handle this error better... but will be difficult...        
      }       
        
   }  // end adding homeworld systems

}


void ServerUniverse::PopulateSystems()
{
   // populate non-homeworld systems

   // get a vector of all the systems
   ObjectVec sys_obj_vec = FindObjects(IsSystem);

   assert(!sys_obj_vec.empty());

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
         
         // Add planet to universe map
         int planet_id = Insert(planet);
         if (planet_id == UniverseObject::INVALID_OBJECT_ID)
         {
            ServerApp* server_app = ServerApp::GetApp();
            server_app->Logger().errorStream() << "ServerUniverse::GenerateIrregularGalaxy : Attemp to insert planet into the object map failed.";
            server_app->Exit(1);  // TODO: should probably handle this error better... but will be difficult...
         }

         // Add planet to system map
         system->Insert(planet, planet_cnt);

         // name the planet
         planet->Rename(system->Name() + " " + RomanNumber(planet_cnt + 1));

      }  // end adding planets
   }

}

void ServerUniverse::GenerateEmpires(int players, int ai_players, std::vector<int>& homeworlds)
{
   // create empires and assign homeworlds, names, colors, and fleet ranges to them
   // for each empire
   
   int fleet_id_rng_size = (int)((UniverseObject::MAX_SHIP_ID - UniverseObject::MIN_SHIP_ID) / players);
   
   ServerApp* server_app = ServerApp::GetApp();
   ServerEmpireManager* empire_mgr = &(server_app->Empires());

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
      int empire_min_flt_id = UniverseObject::MIN_SHIP_ID + (fleet_id_rng_size * empire_cnt);
      int empire_max_flt_id = UniverseObject::MIN_SHIP_ID + (fleet_id_rng_size * (empire_cnt+1)) - 1;

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
      Empire* new_empire = empire_mgr->CreateEmpire(empire_name, *empire_color, home_planet_id, control);

      // store the the empire's valid fleet ID range
      new_empire->SetFleetIDs(empire_min_flt_id, empire_max_flt_id);

      // set ownership of home planet
      int empire_id = new_empire->EmpireID();

      Planet* home_planet = dynamic_cast<Planet*>(Object(homeworlds[empire_cnt]));
      home_planet->AddOwner(empire_id);

      ServerApp* server_app = ServerApp::GetApp();
      server_app->Logger().debugStream() << "Setting Planet " <<  home_planet->ID() << " to be home planet for Empire " << empire_id;

      // TODO: adding an owner to a planet should probably add that owner to the 
      //       system automatically...
      System* home_system = dynamic_cast<System*>(Object(home_planet->SystemID()));
      home_system->AddOwner(empire_id);

      // create population and industry on home planet
      home_planet->AdjustPop(20);
      home_planet->AdjustWorkforce(20);
      home_planet->AdjustIndustry(0.10);
      home_planet->AdjustDefBases(3);

#if 1
      // create the empire's initial ship designs
      ShipDesign* scout_design = new ShipDesign();
      scout_design->name = "Scout";
      scout_design->attack = 0;
      scout_design->defense = 0;
      scout_design->cost = 50;
      scout_design->colonize = false;
      scout_design->empire = empire_id;

      int scout_id = new_empire->AddShipDesign(*scout_design);

      delete scout_design;
      
      ShipDesign* colony_ship_design = new ShipDesign();
      colony_ship_design->name = "Colony Ship";
      colony_ship_design->attack = 0;
      colony_ship_design->defense = 0;
      colony_ship_design->cost = 250;
      colony_ship_design->colonize = true;
      colony_ship_design->empire = empire_id;

      int colony_id = new_empire->AddShipDesign(*colony_ship_design);

      delete colony_ship_design;

      ShipDesign* mark_1_design = new ShipDesign();
      mark_1_design->name = "Mark I";
      mark_1_design->attack = 2;
      mark_1_design->defense = 1;
      mark_1_design->cost = 100;
      mark_1_design->colonize = false;
      mark_1_design->empire = empire_id;

      new_empire->AddShipDesign(*mark_1_design);

      delete mark_1_design;

      
      // create the empire's starting fleet
      
      Fleet* home_fleet = new Fleet("Home Fleet", home_system->X(), home_system->Y(), empire_id);
      
      int fleet_id = Insert(home_fleet);
      home_system->Insert(home_fleet);
      new_empire->AddFleet(fleet_id);
    
      Ship* scout = new Ship(empire_id, scout_id);

      int ship_id = Insert(scout);
      home_fleet->AddShip(ship_id);

      scout = new Ship(empire_id, scout_id);

      ship_id = Insert(scout);
      home_fleet->AddShip(ship_id);

      Ship* colony_ship = new Ship(empire_id, colony_id);

      ship_id = Insert(colony_ship);
      home_fleet->AddShip(ship_id);
#endif
   }
}
