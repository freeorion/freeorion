#include "ServerUniverse.h"

#include "Fleet.h"
#include "Planet.h"
#include "Predicates.h"
#include "../util/Random.h"
#include "../server/ServerApp.h"
#include "XMLDoc.h"

#include <log4cpp/Appender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/FileAppender.hh>
#include <boost/lexical_cast.hpp>


namespace {
const double  MIN_SYSTEM_SEPARATION = 30.0; // in universe units [0.0, ClientUniverse::UNIVERSE_WIDTH]
const double  MIN_HOME_SYSTEM_SEPARATION = 200.0; // in universe units [0.0, ClientUniverse::UNIVERSE_WIDTH]
const int     ADJACENCY_BOXES = 25;
const double  ADJACENCY_BOX_SIZE = ClientUniverse::UNIVERSE_WIDTH / ADJACENCY_BOXES;

// "only" defined for 1 <= n <= 3999, as we can't
// display the symbol for 5000
std::string RomanNumber(unsigned int n)
{
    static const char N[] = "IVXLCDM??";
    string retval;
    int e = 3;
    int mod = 1000;
    for( ; e>=0 ; e--,mod/=10) {
	unsigned int m = (n/mod)%10;
	if (m%5 == 4) {
	    retval += N[e<<1];
	    ++m;
	    if (m == 10) {
		retval += N[(e<<1)+2];
		continue;
	    }
	}
	if (m >= 5) {
	    retval += N[(e<<1)+1];
	    m -= 5;
	}
	while (m) {
	    retval += N[e<<1];
	    m--;
	}
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
    // wipe out anything present in the object map
    for (ObjectMap::iterator itr = m_objects.begin(); itr != m_objects.end(); ++itr)
        delete itr->second;
    m_objects.clear();

    m_last_allocated_id = -1;

    ServerApp::GetApp()->Logger().debugStream() << "Creating universe with " << size << " stars and " << players << " players.";

    std::vector<int> homeworlds;

    // a grid of ADJACENCY_BOXES x ADJACENCY_BOXES boxes to hold the positions of the systems as they are generated, 
    // in order to ensure that they get spaced out properly
    AdjacencyGrid adjacency_grid(ADJACENCY_BOXES, std::vector<std::set<std::pair<double, double> > >(ADJACENCY_BOXES));

    // generate the stars
    switch (shape) {
#if 0
    case SPIRAL_2:
        GenerateSpiralGalaxy(2, size, adjacency_grid);
        break;
    case SPIRAL_3:
        GenerateSpiralGalaxy(3, size, adjacency_grid);
        break;
    case SPIRAL_4:
        GenerateSpiralGalaxy(4, size, adjacency_grid);
        break;
    case ELLIPTICAL:
        GenerateEllipticalGalaxy(size, adjacency_grid);
        break;
#endif
    case IRREGULAR:
        GenerateIrregularGalaxy(size, adjacency_grid);
        break;
    default:
        ServerApp::GetApp()->Logger().errorStream() << "ServerUniverse::ServerUniverse : Unknown galaxy shape: "<< shape << ".  Using IRREGULAR as default.";
        GenerateIrregularGalaxy(size, adjacency_grid);
    }

    PopulateSystems();
    GenerateHomeworlds(players + ai_players, size, homeworlds, adjacency_grid);
    GenerateEmpires(players, ai_players, homeworlds);
}

void ServerUniverse::CreateUniverse(const std::string& map_file, int size, int players, int ai_players)
{
    // intialize the ID counter
    m_last_allocated_id = -1;   

    // TODO
}

ServerUniverse::ServerUniverse(const GG::XMLElement& elem) : 
    ClientUniverse(elem),
    m_last_allocated_id(-1)
{
}

ServerUniverse::~ServerUniverse()
{
    // TODO
}

int ServerUniverse::Insert(UniverseObject* obj)
{
   int retval = UniverseObject::INVALID_OBJECT_ID;
#if 0 // this is turned off for now, since the functionality is not yet needed
   if (dynamic_cast<Fleet*>(obj)) {
       const std::set<int> owners = obj->Owners(); // for a fleet there will only be 1 owner
       Empire* empire = ServerApp::GetApp()->Empires().Lookup(*owners.begin());
       int empire_fleet_id_min = empire->FleetIDMin();
       int empire_fleet_id_max = empire->FleetIDMax();
       for (int i = empire_fleet_id_min; i < empire_fleet_id_max; ++i) {
           if (m_objects.find(i) == m_objects.end()) {
               m_objects[i] = obj;
               obj->SetID(i);
               retval = i;
               break;
           }
       }
   } else 
#endif
   if (obj) {
       if (m_last_allocated_id + 1 < UniverseObject::MIN_SHIP_ID) {
           m_objects[++m_last_allocated_id] = obj;
           obj->SetID(m_last_allocated_id);
           retval = m_last_allocated_id;
       } else { // we'll probably never execute this branch, considering how many IDs are available
           // find a hole in the assigned IDs in which to place the object
           int last_id_seen = UniverseObject::INVALID_OBJECT_ID;
           for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
               if (1 < it->first - last_id_seen) {
                   m_objects[last_id_seen + 1] = obj;
                   obj->SetID(last_id_seen + 1);
                   retval = last_id_seen + 1;
                   break;
               }
           }
       }
   }

   return retval;
}


UniverseObject* ServerUniverse::Remove(int id)
{
   UniverseObject* retval = 0;
   iterator it = m_objects.find(id);
   if (it != m_objects.end()) {
      retval = it->second;
      if (System* sys = dynamic_cast<System*>(Object(retval->SystemID())))
          sys->Remove(id);
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


void ServerUniverse::GenerateSpiralGalaxy(int arms, int stars, AdjacencyGrid& adjacency_grid)
{
   // TODO
}

void ServerUniverse::GenerateEllipticalGalaxy(int stars, AdjacencyGrid& adjacency_grid)
{
   // TODO
}

void ServerUniverse::GenerateIrregularGalaxy(int stars, AdjacencyGrid& adjacency_grid)
{
    std::list<std::string> star_names;
    LoadPlanetNames(star_names);

    SmallIntDistType star_type_gen = SmallIntDist(0, System::NUM_STARTYPES - 1);

    // generate star field
    for (int star_cnt = 0; star_cnt < stars; ++star_cnt) {
        // generate new star
        int star_name_idx = RandSmallInt(0, static_cast<int>(star_names.size()) - 1);
        std::list<std::string>::iterator it = star_names.begin();
        std::advance(it, star_name_idx);
        std::string star_name(*it);
        star_names.erase(it);
        System::StarType star_type = System::StarType(star_type_gen());
        int num_orbits = 5;
        double orbits_rand = RandZeroToOne();
        if (orbits_rand < 0.05) {
            num_orbits = 0;
        } else if (orbits_rand < 0.25) {
            num_orbits = 1;
        } else if (orbits_rand < 0.55) {
            num_orbits = 2;
        } else if (orbits_rand < 0.85) {
            num_orbits = 3;
        } else if (orbits_rand < 0.95) {
            num_orbits = 4;
        }
        // above 0.95 stays at the default 5

        bool placed = false;
        int attempts = 0;
        System* system = new System(star_type, num_orbits, star_name, UNIVERSE_WIDTH * RandZeroToOne(), UNIVERSE_WIDTH * RandZeroToOne());
        int new_sys_id = Insert(system);
        if (new_sys_id == UniverseObject::INVALID_OBJECT_ID) {
            throw std::runtime_error("ServerUniverse::GenerateIrregularGalaxy : Attemp to insert system " + 
                                     star_name + " into the object map failed.");
        }
        while (!placed && attempts < 10) {
            // look in the box into which this system falls, and those boxes immediately around that box; 
            // make sure this system doesn't get slapped down too close to or on top of any systems
            std::pair<unsigned int, unsigned int> grid_box(static_cast<unsigned int>(system->X() / ADJACENCY_BOX_SIZE), 
                                                           static_cast<unsigned int>(system->Y() / ADJACENCY_BOX_SIZE));
            std::set<std::pair<double, double> > neighbors(adjacency_grid[grid_box.first][grid_box.second]);
            if (0 < grid_box.first) {
                if (0 < grid_box.second) {
                    std::set<std::pair<double, double> >& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second - 1];
                    neighbors.insert(grid_square.begin(), grid_square.end());
                }
                std::set<std::pair<double, double> >& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second];
                neighbors.insert(grid_square.begin(), grid_square.end());
                if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
                    std::set<std::pair<double, double> >& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second + 1];
                    neighbors.insert(grid_square.begin(), grid_square.end());
                }
            }
            if (0 < grid_box.second) {
                std::set<std::pair<double, double> >& grid_square = adjacency_grid[grid_box.first][grid_box.second - 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
            if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
                std::set<std::pair<double, double> >& grid_square = adjacency_grid[grid_box.first][grid_box.second + 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
            if (grid_box.first < adjacency_grid.size() - 1) {
                if (0 < grid_box.second) {
                    std::set<std::pair<double, double> >& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second - 1];
                    neighbors.insert(grid_square.begin(), grid_square.end());
                }
                std::set<std::pair<double, double> >& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second];
                neighbors.insert(grid_square.begin(), grid_square.end());
                if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
                    std::set<std::pair<double, double> >& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second + 1];
                    neighbors.insert(grid_square.begin(), grid_square.end());
                }
            }

            std::pair<double, double> neighbor_centroid;
            bool too_close = false;
            double system_x = system->X();
            double system_y = system->Y();
            for (std::set<std::pair<double, double> >::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                double x_dist = system_x - it->first;
                double y_dist = system_y - it->second;
                if (x_dist * x_dist + y_dist * y_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION)
                    too_close = true;
                neighbor_centroid.first += it->first;
                neighbor_centroid.second += it->second;
            }

            if (too_close) {
                neighbor_centroid.first /= neighbors.size();
                neighbor_centroid.second /= neighbors.size();
                double move_x = system_x - neighbor_centroid.first;
                double move_y = system_y - neighbor_centroid.second;
                move_x = std::max(0.0, std::min(move_x + system_x, UNIVERSE_WIDTH));
                move_y = std::max(0.0, std::min(move_y + system_y, UNIVERSE_WIDTH));
                system->MoveTo(move_x, move_y);
            } else {
                placed = true;
            }

            if (placed || ++attempts == 20) {
                adjacency_grid[static_cast<int>(system->X() / ADJACENCY_BOX_SIZE)]
                    [static_cast<int>(system->Y() / ADJACENCY_BOX_SIZE)].insert(std::make_pair(system->X(), system->Y()));
            }
        }
    }
}



void ServerUniverse::GenerateHomeworlds(int players, int stars, std::vector<int>& homeworlds, AdjacencyGrid& adjacency_grid)
{
    homeworlds.clear();

    ObjectVec sys_vec = FindObjects(IsSystem);

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to generate homeworlds in an empty galaxy.");

    for (int i = 0; i < players; ++i) {
        int system_index = RandSmallInt(0, static_cast<int>(sys_vec.size()) - 1);

        System* system = dynamic_cast<System*>(sys_vec[system_index]);

        // make sure it has planets and it's not too close to the other homeworlds
        bool too_close = true;
        int attempts = 0;
        while ((!system->Orbits() || too_close) && ++attempts < 50) {
            too_close = false;
            system = dynamic_cast<System*>(sys_vec[system_index]);
            for (unsigned int j = 0; j < homeworlds.size(); ++j) {
                UniverseObject* existing_system = Object(Object(homeworlds[j])->SystemID());
                double x_dist = existing_system->X() - system->X();
                double y_dist = existing_system->Y() - system->Y();
                if (x_dist * x_dist + y_dist * y_dist < MIN_HOME_SYSTEM_SEPARATION * MIN_HOME_SYSTEM_SEPARATION) {
                    too_close = true;
                    break;
                }
            }
        }

        sys_vec.erase(sys_vec.begin() + system_index);

        // find a place to put the homeworld, and replace whatever planet is there already
        int home_orbit = RandSmallInt(0, system->Orbits() - 1);
        System::ObjectIDVec planet_IDs = system->FindObjectIDsInOrbit(home_orbit, IsPlanet);
        Planet* planet = new Planet(Planet::TERRAN, Planet::SZ_MEDIUM);
        planet->Rename(Object(planet_IDs.back())->Name());
        Delete(planet_IDs.back());
        int planet_id = Insert(planet);
        system->Insert(planet, home_orbit);
        homeworlds.push_back(planet_id);
    }
}


void ServerUniverse::PopulateSystems()
{
    ObjectVec sys_vec = FindObjects(IsSystem);

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to populate an empty galaxy.");

    SmallIntDistType planet_type_gen = SmallIntDist(0, Planet::MAX_PLANET_TYPE - 1);

    for (ObjectVec::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it) {
        System* system = dynamic_cast<System*>(*it);

        for (int orbit = 0; orbit < system->Orbits(); orbit++) {
            Planet::PlanetSize plt_size;       

            // Type is selected at random. Type only affects the planet's image in v0.1
            Planet::PlanetType plt_type = Planet::PlanetType(planet_type_gen());
            double size_rnd = RandZeroToOne();
            if (size_rnd < 0.10)
                plt_size = Planet::SZ_TINY;
            else if (size_rnd < 0.35)
                plt_size = Planet::SZ_SMALL;
            else if (size_rnd < 0.65)
                plt_size = Planet::SZ_MEDIUM;
            else if (size_rnd < 0.90)
                plt_size = Planet::SZ_LARGE;
            else
                plt_size = Planet::SZ_HUGE;

            Planet* planet = new Planet(plt_type, plt_size);

            Insert(planet); // add planet to universe map
            system->Insert(planet, orbit);  // add planet to system map
            planet->Rename(system->Name() + " " + RomanNumber(orbit + 1));
        }
    }
}

void ServerUniverse::GenerateEmpires(int players, int ai_players, std::vector<int>& homeworlds)
{
   // create empires and assign homeworlds, names, colors, and fleet ranges to
   // for each one

   const int FLEET_ID_RNG = (UniverseObject::MAX_SHIP_ID - UniverseObject::MIN_SHIP_ID) / players;

   for (int i = 0; i < players + ai_players; ++i) {
      // TODO: select name at random from default list
      std::string name = "Empire" + boost::lexical_cast<std::string>(i);

      // TODO: select color at random from default list
      GG::Clr color(RandZeroToOne(), RandZeroToOne(), RandZeroToOne(), 1.0);
  
      int min_fleet_id = UniverseObject::MIN_SHIP_ID + (FLEET_ID_RNG * i);
      int max_fleet_id = UniverseObject::MIN_SHIP_ID + (FLEET_ID_RNG * (i + 1)) - 1;
      int home_planet_id = homeworlds[i];

      // create new Empire object through empire manager
      Empire* empire = ServerApp::GetApp()->Empires().CreateEmpire(name, color, home_planet_id, i < players ? Empire::CONTROL_HUMAN : Empire::CONTROL_AI);
      empire->SetFleetIDs(min_fleet_id, max_fleet_id);

      // set ownership of home planet
      int empire_id = empire->EmpireID();
      Planet* home_planet = dynamic_cast<Planet*>(Object(homeworlds[i]));
      ServerApp::GetApp()->Logger().debugStream() << "Setting " << Object(home_planet->SystemID())->Name() << " (Planet #" <<  home_planet->ID() << 
          ") to be home system for Empire " << empire_id;
      home_planet->AddOwner(empire_id);
      // TODO: adding an owner to a planet should probably add that owner to the 
      //       system automatically...
      System* home_system = dynamic_cast<System*>(Object(home_planet->SystemID()));
      home_system->AddOwner(empire_id);

      // create population and industry on home planet
      home_planet->AdjustPop(20);
      home_planet->AdjustWorkforce(20);
      home_planet->AdjustIndustry(0.10);
      home_planet->AdjustDefBases(3);

      // create the empire's initial ship designs
      ShipDesign scout_design;
      scout_design.name = "Scout";
      scout_design.attack = 0;
      scout_design.defense = 0;
      scout_design.cost = 50;
      scout_design.colonize = false;
      scout_design.empire = empire_id;
      int scout_id = empire->AddShipDesign(scout_design);

      ShipDesign colony_ship_design;
      colony_ship_design.name = "Colony Ship";
      colony_ship_design.attack = 0;
      colony_ship_design.defense = 0;
      colony_ship_design.cost = 250;
      colony_ship_design.colonize = true;
      colony_ship_design.empire = empire_id;
      int colony_id = empire->AddShipDesign(colony_ship_design);

      ShipDesign mark_1_design;
      mark_1_design.name = "Mark I";
      mark_1_design.attack = 2;
      mark_1_design.defense = 1;
      mark_1_design.cost = 100;
      mark_1_design.colonize = false;
      mark_1_design.empire = empire_id;
      empire->AddShipDesign(mark_1_design);

      // create the empire's starting fleet
      Fleet* home_fleet = new Fleet("Home Fleet", home_system->X(), home_system->Y(), empire_id);
      int fleet_id = Insert(home_fleet);
      home_system->Insert(home_fleet);
      empire->AddFleet(fleet_id);

      int ship_id = Insert(new Ship(empire_id, scout_id));
      home_fleet->AddShip(ship_id);

      ship_id = Insert(new Ship(empire_id, scout_id));
      home_fleet->AddShip(ship_id);

      ship_id = Insert(new Ship(empire_id, colony_id));
      home_fleet->AddShip(ship_id);
   }
}
