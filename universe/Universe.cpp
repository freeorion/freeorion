#include "Universe.h"

#include "../util/AppInterface.h"
#include "Fleet.h"
#include "../util/DataTable.h"
#include "Planet.h"
#include "Predicates.h"
#include "../util/Random.h"
#include "../Empire/ServerEmpireManager.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"
#include "XMLDoc.h"

#ifdef FREEORION_BUILD_HUMAN
#include "../client/human/HumanClientApp.h"
#endif

#ifdef FREEORION_BUILD_SERVER
#include "../server/ServerApp.h"
#endif

#include <stdexcept>
#include <cmath>

namespace {
    // for UniverseObject factory
    UniverseObject* NewFleet(const GG::XMLElement& elem)  {return new Fleet(elem);}
    UniverseObject* NewPlanet(const GG::XMLElement& elem) {return new Planet(elem);}
    UniverseObject* NewShip(const GG::XMLElement& elem)   {return new Ship(elem);}
    UniverseObject* NewSystem(const GG::XMLElement& elem) {return new System(elem);}

    const double  MIN_SYSTEM_SEPARATION = 30.0; // in universe units [0.0, s_universe_width]
    const double  MIN_HOME_SYSTEM_SEPARATION = 200.0; // in universe units [0.0, s_universe_width]
    const int     ADJACENCY_BOXES = 25;
    const double  PI = 3.141592;
    const int     MAX_SYSTEM_ORBITS = 10;   // maximum slots where planets can be, in v0.2
    SmallIntDistType g_hundred_dist = SmallIntDist(1, 100); // a linear distribution [1, 100] used in most universe generation

    DataTableMap& UniverseDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            LoadDataTables("default/universe_tables.txt", map);
        }
        return map;
    }

    // "only" defined for 1 <= n <= 3999, as we can't
    // display the symbol for 5000
    std::string RomanNumber(unsigned int n)
    {
        static const char N[] = "IVXLCDM??";
        std::string retval;
        int e = 3;
        int mod = 1000;
        for (; 0 <= e; e--, mod /= 10) {
            unsigned int m = (n / mod) % 10;
            if (m % 5 == 4) {
                retval += N[e << 1];
                ++m;
                if (m == 10) {
                    retval += N[(e << 1) + 2];
                    continue;
                }
            }
            if (m >= 5) {
                retval += N[(e << 1) + 1];
                m -= 5;
            }
            while (m) {
                retval += N[e << 1];
                --m;
            }
        }
        return retval;
    }

    void LoadSystemNames(std::list<std::string>& names)
    {
        std::ifstream ifs("default/starnames.txt");
        while (ifs) {
            std::string latest_name;
            std::getline(ifs, latest_name);
            if (latest_name != "")
            names.push_back(latest_name);
        }
    }

    void SpiralGalaxyCalcPositions(std::vector<std::pair<double, double> > &positions, unsigned int arms, unsigned int stars, double width, double height)
    {
        double arm_offset     = RandDouble(0.0,2.0*PI);
        double arm_angle      = 2.0*PI / arms;
        double arm_spread     = 0.3 * PI / arms;
        double arm_length     = 1.5 * PI;
        double center         = 0.25;
        double x,y;

        unsigned int i, j, attempts;

        GaussianDistType  random_gaussian = GaussianDist(0.0,arm_spread);
        SmallIntDistType  random_arm      = SmallIntDist(0  ,arms);
        DoubleDistType    random_degree   = DoubleDist  (0.0,2.0*PI);
        DoubleDistType    random_radius   = DoubleDist  (0.0,  1.0);

        for (i = 0, attempts = 0; i < stars && attempts < 100; i++, ++attempts)
        {
            double radius = random_radius();

            if (radius < center) {
                double angle = random_degree();
                x = radius * cos( arm_offset + angle );
                y = radius * sin( arm_offset + angle );
            } else {
                double arm    = (double)random_arm() * arm_angle;
                double angle  = random_gaussian();

                x = radius * cos( arm_offset + arm + angle + radius * arm_length );
                y = radius * sin( arm_offset + arm + angle + radius * arm_length );
            }

            x = (x + 1) * width / 2.0;
            y = (y + 1) * height / 2.0;

            if (x < 0 || width <= x || y < 0 || height <= y)
                continue;

            // ensure all system have a min separation to each other (search isn't opimized, not worth the effort)
            for (j = 0; j < positions.size(); ++j) {
                if ((positions[j].first - x) * (positions[j].first - x) + (positions[j].second - y) * (positions[j].second - y)
                    < MIN_SYSTEM_SEPARATION*MIN_SYSTEM_SEPARATION)
                break;
            }
            if (j < positions.size()) {
                --i;
                continue;
            }

            attempts = 0;
            positions.push_back(std::pair<double,double>(x, y));
        }
    }

    void EllipticalGalaxyCalcPositions(std::vector<std::pair<double,double> > &positions, unsigned int stars, double width, double height)
    {
        const double ellipse_width_vs_height = RandDouble(0.4, 0.6);
        const double rotation = RandDouble(0.0, PI),
            rotation_sin = std::sin(rotation),
            rotation_cos = std::cos(rotation);
        const unsigned int fail_attempts = 100;
        const double gap_constant = .95;
        const double gap_size = 1.0 - gap_constant * gap_constant * gap_constant;

        // Random number generators.
        DoubleDistType radius_dist = DoubleDist(0.0, gap_constant);
        DoubleDistType random_angle  = DoubleDist(0.0, 2.0 * PI);

        // Used to give up when failing to place a star too often.
        unsigned int attempts = 0;

        // For each attempt to place a star...
        for (unsigned int i = 0; i < stars && attempts < fail_attempts; ++i, ++attempts){
            double radius = radius_dist();
            // Adjust for bigger density near center and create gap.
            radius = radius * radius * radius + gap_size;
            double angle  = random_angle();

            // Rotate for individual angle and apply elliptical shape.
            double x1 = radius * std::cos(angle);
            double y1 = radius * std::sin(angle) * ellipse_width_vs_height;

            // Rotate for ellipse angle.
            double x = x1 * rotation_cos - y1 * rotation_sin;
            double y = x1 * rotation_sin + y1 * rotation_cos;

            // Move from [-1.0, 1.0] universe coordinates.
            x = (x + 1.0) * width / 2.0;
            y = (y + 1.0) * height / 2.0;

            // Discard stars that are outside boundaries (due to possible rounding errors).
            if (x < 0 || x >= width || y < 0 || y >= height)
                continue;

            // See if new star is too close to any existing star.
            unsigned int j = 0;
            for (; j < positions.size(); ++j){
                if ((positions[j].first - x) * (positions[j].first - x) +
                    (positions[j].second - y) * (positions[j].second - y)
                    < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION)
                    break;
            }
            // If so, we try again.
            if (j < positions.size()){
                --i;
                continue;
            }

            // Note that attempts is reset for every star.
            attempts = 0;

            // Add the new star location.
            positions.push_back(std::pair<double,double>(x, y));
        }
    }

    void ClusterGalaxyCalcPositions(std::vector<std::pair<double,double> > &positions, unsigned int clusters, unsigned int stars, double width, double height)
    {
        //propability of systems which don't belong to a cluster
        const double system_noise = 0.15;
        double ellipse_width_vs_height = RandDouble(0.2,0.5);
        // first innermost pair hold cluster position, second innermost pair stores help values for cluster rotation (sin,cos)
        std::vector<std::pair<std::pair<double,double>,std::pair<double,double> > > clusters_position;
        unsigned int i,j,attempts;

        DoubleDistType    random_zero_to_one = DoubleDist  (0.0,  1.0);
        DoubleDistType    random_angle  = DoubleDist  (0.0,2.0*PI);

        for (i=0,attempts=0;i<clusters && attempts<100;i++,attempts++)
        {
            // prevent cluster position near borders (and on border)
            double x=((random_zero_to_one()*2.0-1.0) /(clusters+1.0))*clusters,
                y=((random_zero_to_one()*2.0-1.0) /(clusters+1.0))*clusters;


            // ensure all clusters have a min separation to each other (search isn't opimized, not worth the effort)
            for (j=0;j<clusters_position.size();j++)
                if ((clusters_position[j].first.first - x)*(clusters_position[j].first.first - x)+ (clusters_position[j].first.second - y)*(clusters_position[j].first.second - y)
                    < (2.0/clusters))
                    break;
            if (j<clusters_position.size())
            {
                i--;
                continue;
            }

            attempts=0;
            double rotation = RandDouble(0.0,PI);
            clusters_position.push_back(std::pair<std::pair<double,double>,std::pair<double,double> >(std::pair<double,double>(x,y),std::pair<double,double>(sin(rotation),cos(rotation))));
        }

        for (i=0,attempts=0; i < stars && attempts<100; i++,attempts++ )
        {
            double x,y;
            if (random_zero_to_one()<system_noise)
            {
                x = random_zero_to_one() * 2.0 - 1.0;
                y = random_zero_to_one() * 2.0 - 1.0;
            }
            else
            {
                short  cluster = i%clusters_position.size();
                double radius  = random_zero_to_one();
                double angle   = random_angle();
                double x1,y1;

                x1 = radius * cos(angle);
                y1 = radius * sin(angle)*ellipse_width_vs_height;

                x = x1*clusters_position[cluster].second.second + y1*clusters_position[cluster].second.first;
                y =-x1*clusters_position[cluster].second.first  + y1*clusters_position[cluster].second.second;

                x = x/sqrt((double)clusters) + clusters_position[cluster].first.first;
                y = y/sqrt((double)clusters) + clusters_position[cluster].first.second;
            }
            x = (x+1)*width /2.0;
            y = (y+1)*height/2.0;

            if (x<0 || width<=x || y<0 || height<=y)
                continue;

            // ensure all system have a min separation to each other (search isn't opimized, not worth the effort)
            for (j=0;j<positions.size();j++)
                if ((positions[j].first - x)*(positions[j].first - x)+ (positions[j].second - y)*(positions[j].second - y)
                    < MIN_SYSTEM_SEPARATION*MIN_SYSTEM_SEPARATION)
                    break;
            if (j<positions.size())
            {
                i--;
                continue;
            }

            attempts=0;
            positions.push_back(std::pair<double,double>(x,y));
        }
    }

    System* GenerateSystem(Universe &universe, Universe::Age age, double x, double y)
    {
        const std::vector<int>& base_star_type_dist = UniverseDataTables()["BaseStarTypeDist"][0];
        const std::vector<std::vector<int> >& universe_age_mod_to_star_type_dist = UniverseDataTables()["UniverseAgeModToStarTypeDist"];

        static std::list<std::string> star_names;
        if (star_names.empty())
            LoadSystemNames(star_names);

        // generate new star
        int star_name_idx = RandSmallInt(0, static_cast<int>(star_names.size()) - 1);
        std::list<std::string>::iterator it = star_names.begin();
        std::advance(it, star_name_idx);
        std::string star_name(*it);
        star_names.erase(it);
        int num_orbits = 10; // this is fixed in the v0.2 DD

        // make a series of "rolls" (1-100) for each planet size, and take the highest modified roll
        int idx = 0;
        int max_roll = 0;
        for (unsigned int i = 0; i < System::NUM_STARTYPES; ++i) {
            int roll = g_hundred_dist() + universe_age_mod_to_star_type_dist[age][i] + base_star_type_dist[i];
            if (max_roll < roll) {
                max_roll = roll;
                idx = i;
            }
        }
        System* system = new System(System::StarType(idx), MAX_SYSTEM_ORBITS, star_name, x, y);

        int new_system_id = universe.Insert(system);
        if (new_system_id == UniverseObject::INVALID_OBJECT_ID) {
            throw std::runtime_error("Universe::GenerateIrregularGalaxy() : Attempt to insert system " +
                                    star_name + " into the object map failed.");
        }

        return system;
    }

    void GenerateStarField(Universe &universe, const std::vector<std::pair<double, double> > &positions, Universe::AdjacencyGrid& adjacency_grid, 
                           double adjacency_box_size, Universe::Age age = Universe::AGE_MATURE)
    {
        // generate star field
        for (unsigned int star_cnt = 0; star_cnt < positions.size(); ++star_cnt) {
            System* system = GenerateSystem(universe, age, positions[star_cnt].first, positions[star_cnt].second);
            adjacency_grid[static_cast<int>(system->X() / adjacency_box_size)]
                [static_cast<int>(system->Y() / adjacency_box_size)].insert(system);
        }
    }

    void GetNeighbors(double x, double y, const Universe::AdjacencyGrid& adjacency_grid, std::set<System*>& neighbors)
    {
        const double ADJACENCY_BOX_SIZE = Universe::UniverseWidth() / ADJACENCY_BOXES;
        std::pair<unsigned int, unsigned int> grid_box(static_cast<unsigned int>(x / ADJACENCY_BOX_SIZE),
                                                       static_cast<unsigned int>(y / ADJACENCY_BOX_SIZE));

        // look in the box into which this system falls, and those boxes immediately around that box
        neighbors = adjacency_grid[grid_box.first][grid_box.second];

        if (0 < grid_box.first) {
            if (0 < grid_box.second) {
                const std::set<System*>& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second - 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
            const std::set<System*>& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second];
            neighbors.insert(grid_square.begin(), grid_square.end());
            if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
                const std::set<System*>& grid_square = adjacency_grid[grid_box.first - 1][grid_box.second + 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
        }
        if (0 < grid_box.second) {
            const std::set<System*>& grid_square = adjacency_grid[grid_box.first][grid_box.second - 1];
            neighbors.insert(grid_square.begin(), grid_square.end());
        }
        if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
            const std::set<System*>& grid_square = adjacency_grid[grid_box.first][grid_box.second + 1];
            neighbors.insert(grid_square.begin(), grid_square.end());
        }

        if (grid_box.first < adjacency_grid.size() - 1) {
            if (0 < grid_box.second) {
                const std::set<System*>& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second - 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
            const std::set<System*>& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second];
            neighbors.insert(grid_square.begin(), grid_square.end());
            if (grid_box.second < adjacency_grid[grid_box.first].size() - 1) {
                const std::set<System*>& grid_square = adjacency_grid[grid_box.first + 1][grid_box.second + 1];
                neighbors.insert(grid_square.begin(), grid_square.end());
            }
        }
    }
}

// static(s)
double Universe::s_universe_width = 1000.0;

Universe::Universe()
{
    m_factory.AddGenerator("Fleet", &NewFleet);
    m_factory.AddGenerator("Planet", &NewPlanet);
    m_factory.AddGenerator("Ship", &NewShip);
    m_factory.AddGenerator("System", &NewSystem);
    m_last_allocated_id = -1;
}

Universe::Universe(const GG::XMLElement& elem)
{
    m_factory.AddGenerator("Fleet", &NewFleet);
    m_factory.AddGenerator("Planet", &NewPlanet);
    m_factory.AddGenerator("Ship", &NewShip);
    m_factory.AddGenerator("System", &NewSystem);

    SetUniverse(elem);
}

Universe::~Universe()
{
    for (ObjectMap::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
        delete it->second;
}

void Universe::SetUniverse(const GG::XMLElement& elem)
{
    using GG::XMLElement;

    if (elem.Tag() != "Universe")
        throw std::invalid_argument("Attempted to construct a Universe from an XMLElement that had a tag other than \"Universe\"");

    // wipe out anything present in the object map
    for (ObjectMap::iterator itr= m_objects.begin(); itr != m_objects.end(); ++itr)
        delete itr->second;
    m_objects.clear();

    s_universe_width = boost::lexical_cast<double>(elem.Child("s_universe_width").Text());

    for (int i = 0; i < elem.Child("m_objects").NumChildren(); ++i) {
        if (UniverseObject* obj = m_factory.GenerateObject(elem.Child("m_objects").Child(i))) {
            m_objects[obj->ID()] = obj;
        }
    }

    m_last_allocated_id = boost::lexical_cast<int>(elem.Child("m_last_allocated_id").Text());
}

const UniverseObject* Universe::Object(int id) const
{
    const_iterator it = m_objects.find(id);
    return (it != m_objects.end() ? it->second : 0);
}

UniverseObject* Universe::Object(int id)
{
    iterator it = m_objects.find(id);
    return (it != m_objects.end() ? it->second : 0);
}

GG::XMLElement Universe::XMLEncode() const
{
    GG::XMLElement retval("Universe");
    GG::XMLElement temp("m_objects");

    retval.AppendChild(GG::XMLElement("s_universe_width", boost::lexical_cast<std::string>(s_universe_width)));

    for (const_iterator it = begin(); it != end(); ++it)
        temp.AppendChild(it->second->XMLEncode());
    retval.AppendChild(temp);

    retval.AppendChild(GG::XMLElement("m_last_allocated_id", boost::lexical_cast<std::string>(m_last_allocated_id)));

    return retval;
}

GG::XMLElement Universe::XMLEncode(int empire_id) const
{
   using GG::XMLElement;

   XMLElement element("Universe");
   XMLElement object_map("m_objects");

   element.AppendChild(XMLElement("s_universe_width", boost::lexical_cast<std::string>(s_universe_width)));

   for (const_iterator itr = begin(); itr != end(); ++itr)
   {
       // determine visibility
       UniverseObject::Visibility vis = (*itr).second->Visible(empire_id);
       XMLElement univ_object("UniverseObject");
       XMLElement univ_element;

       if (vis == UniverseObject::FULL_VISIBILITY) {
           univ_element = (*itr).second->XMLEncode( );
       } else if (vis == UniverseObject::PARTIAL_VISIBILITY) {
           univ_element = (*itr).second->XMLEncode( empire_id );
       }

       //univ_element.AppendChild( univ_object );
       object_map.AppendChild( univ_element );

       // for NO_VISIBILITY no element is added
   }
   element.AppendChild(object_map);

   element.AppendChild(GG::XMLElement("m_last_allocated_id", boost::lexical_cast<std::string>(m_last_allocated_id)));

   return element;
}

/********************************************************************
 Methods for universe creation and object creation -- merged in from
ServerUniverse
**********************************************************************/

void Universe::CreateUniverse(Shape shape, int size, int players, int ai_players)
{
    // wipe out anything present in the object map
    for (ObjectMap::iterator itr = m_objects.begin(); itr != m_objects.end(); ++itr)
        delete itr->second;
    m_objects.clear();

    m_last_allocated_id = -1;

    Logger().debugStream() << "Creating universe with " << size << " stars and " << players << " players.";

    std::vector<int> homeworlds;

    // a grid of ADJACENCY_BOXES x ADJACENCY_BOXES boxes to hold the positions of the systems as they are generated,
    // in order to ensure that they get spaced out properly
    AdjacencyGrid adjacency_grid(ADJACENCY_BOXES, std::vector<std::set<System*> >(ADJACENCY_BOXES));

    s_universe_width = size * 1000.0 / 150.0; // chosen so that the width of a medium galaxy is 1000.0

    // generate the stars
    switch (shape) {
    case SPIRAL_2:
        GenerateSpiralGalaxy(2, size, adjacency_grid);
        break;
    case SPIRAL_3:
        GenerateSpiralGalaxy(3, size, adjacency_grid);
        break;
    case SPIRAL_4:
        GenerateSpiralGalaxy(4, size, adjacency_grid);
        break;
    case CLUSTER:
        GenerateClusterGalaxy(size, adjacency_grid);
        break;
    case ELLIPTICAL:
        GenerateEllipticalGalaxy(size, adjacency_grid);
        break;
    case IRREGULAR:
        GenerateIrregularGalaxy(size, adjacency_grid);
        break;
    default:
        Logger().errorStream() << "Universe::Universe : Unknown galaxy shape: "<< shape << ".  Using IRREGULAR as default.";
        GenerateIrregularGalaxy(size, adjacency_grid);
    }

    PopulateSystems(PD_AVERAGE);
    GenerateStarlanes(LANES_AVERAGE, LANES_AVERAGE, LANES_AVERAGE, adjacency_grid);
    GenerateHomeworlds(players + ai_players, homeworlds);
    GenerateEmpires(players + ai_players, homeworlds);
}

void Universe::CreateUniverse(const std::string& map_file, int size, int players, int ai_players)
{
    // intialize the ID counter
    m_last_allocated_id = -1;

    // TODO
}

int Universe::Insert(UniverseObject* obj)
{
   int retval = UniverseObject::INVALID_OBJECT_ID;
   if (obj) {
       if (m_last_allocated_id + 1 < UniverseObject::MAX_ID) {
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

bool Universe::InsertID(UniverseObject* obj, int id )
{
   bool retval = false;

   if (obj) {
       if ( id < UniverseObject::MAX_ID) {
           m_objects[id] = obj;
           obj->SetID(id);
           retval = true;
       }
   }
   return retval;
}

UniverseObject* Universe::Remove(int id)
{
   UniverseObject* retval = 0;
   iterator it = m_objects.find(id);
   if (it != m_objects.end()) {
      retval = it->second;
      if (System* sys = retval->GetSystem())
          sys->Remove(id);
      m_objects.erase(id);
   }
   return retval;
}

bool Universe::Delete(int id)
{
  UniverseObject* obj = Remove(id);
  if (obj)
    UniverseObjectDeleteSignal()(obj);
  delete obj;
  return obj;
}

void Universe::MovementPhase(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void Universe::PopGrowthProductionResearch(std::vector<SitRepEntry>& sit_reps)
{
   // TODO
}

void Universe::GenerateSpiralGalaxy(int arms, int stars, AdjacencyGrid& adjacency_grid)
{
    std::vector<std::pair<double,double> > positions;

    SpiralGalaxyCalcPositions(positions,arms,stars,s_universe_width,s_universe_width);
    GenerateStarField(*this,positions,adjacency_grid,s_universe_width/ADJACENCY_BOXES);
}

void Universe::GenerateEllipticalGalaxy(int stars, AdjacencyGrid& adjacency_grid)
{
    std::vector<std::pair<double,double> > positions;

    EllipticalGalaxyCalcPositions(positions,stars,s_universe_width,s_universe_width);
    GenerateStarField(*this,positions,adjacency_grid,s_universe_width/ADJACENCY_BOXES);
}

void Universe::GenerateClusterGalaxy(int stars, AdjacencyGrid& adjacency_grid)
{
    std::vector<std::pair<double,double> > positions;

    ClusterGalaxyCalcPositions(positions,5,stars,s_universe_width,s_universe_width);
    GenerateStarField(*this,positions,adjacency_grid,s_universe_width/ADJACENCY_BOXES);
}

void Universe::GenerateIrregularGalaxy(int stars, AdjacencyGrid& adjacency_grid)
{
    std::list<std::string> star_names;
    LoadSystemNames(star_names);

    SmallIntDistType star_type_gen = SmallIntDist(0, System::NUM_STARTYPES - 1);

    // generate star field
    for (int star_cnt = 0; star_cnt < stars; ++star_cnt) {
        // generate new star
        System* system = GenerateSystem(*this, AGE_MATURE, (s_universe_width - 0.1) * RandZeroToOne(), (s_universe_width - 0.1) * RandZeroToOne());

        const double ADJACENCY_BOX_SIZE = UniverseWidth() / ADJACENCY_BOXES;

        bool placed = false;
        int attempts_left = 25;
        while (!placed && attempts_left--) {
            // make sure this system doesn't get slapped down too close to or on top of any systems
            std::set<System*> neighbors;
            double x = (s_universe_width - 0.1) * RandZeroToOne(),
                y = (s_universe_width - 0.1) * RandZeroToOne();
            GetNeighbors(x, y, adjacency_grid, neighbors);

            bool too_close = false;
            for (std::set<System*>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
                double x_dist = x - (*it)->X();
                double y_dist = y - (*it)->Y();
                if (x_dist * x_dist + y_dist * y_dist < MIN_SYSTEM_SEPARATION * MIN_SYSTEM_SEPARATION) {
                    too_close = true;
                    break;
                }
            }

            if (!too_close) {
                System* system = GenerateSystem(*this, AGE_MATURE, x, y);
                adjacency_grid[static_cast<int>(system->X() / ADJACENCY_BOX_SIZE)]
                    [static_cast<int>(system->Y() / ADJACENCY_BOX_SIZE)].insert(system);
                placed = true;
            }
        }

        if (!attempts_left)
            Delete(system->ID());
    }
}

void Universe::GenerateStarlanes(StarlaneFreqency short_freq, StarlaneFreqency medium_freq, StarlaneFreqency long_freq,
                                 const AdjacencyGrid& adjacency_grid)
{
    const double ADJACENCY_BOX_SIZE = UniverseWidth() / ADJACENCY_BOXES;

    std::vector<System*> sys_vec = FindObjects<System>();

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to generate starlanes in an empty galaxy.");

    const std::vector<int>& max_starlanes = UniverseDataTables()["MaxStarlanes"][0];
    std::vector<int> starlane_ranges = UniverseDataTables()["StarlaneRanges"][0];
    for (unsigned int i = 0; i < starlane_ranges.size(); ++i) {
        starlane_ranges[i] /= ADJACENCY_BOX_SIZE;
    }

    SmallIntDistType short_dist = SmallIntDist(0, max_starlanes[short_freq]);
    SmallIntDistType medium_dist = SmallIntDist(0, max_starlanes[medium_freq]);
    SmallIntDistType long_dist = SmallIntDist(0, max_starlanes[long_freq]);
    SmallIntDistType short_range_dist = SmallIntDist(-starlane_ranges[0], starlane_ranges[0]);
    SmallIntDistType medium_range_dist = SmallIntDist(-starlane_ranges[1], starlane_ranges[1]);
    SmallIntDistType long_range_dist = SmallIntDist(-starlane_ranges[2], starlane_ranges[2]);

    for (std::vector<System*>::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it) {
        System* system = *it;
        int grid_x = static_cast<int>(system->X() / ADJACENCY_BOX_SIZE);
        int grid_y = static_cast<int>(system->Y() / ADJACENCY_BOX_SIZE);
        int short_lanes = short_dist();
        int medium_lanes = medium_dist();
        int long_lanes = long_dist();

        Logger().debugStream() << "System #" << system->ID() << " needs " << short_lanes << " short lanes, " 
            << medium_lanes << " medium lanes, and " << long_lanes << " long lanes; generating...\n";

        int attempts = 0;
        while ((long_lanes || medium_lanes || short_lanes) && attempts++ < 10) {
            Logger().debugStream() << "  top of while(): still need " << short_lanes << " short lanes, " 
                << medium_lanes << " medium lanes, and " << long_lanes << " long lanes\n";
            int x_offset;
            int y_offset;
            if (long_lanes) {
                x_offset = long_range_dist();
                y_offset = long_range_dist();
                Logger().debugStream() << "  rolled long lane with offset (" << x_offset << ", " << y_offset << ")\n";
            } else if (medium_lanes) {
                x_offset = medium_range_dist();
                y_offset = medium_range_dist();
                Logger().debugStream() << "  rolled medium lane with offset (" << x_offset << ", " << y_offset << ")\n";
            } else if (short_lanes) {
                x_offset = short_range_dist();
                y_offset = short_range_dist();
                Logger().debugStream() << "  rolled short lane with offset (" << x_offset << ", " << y_offset << ")\n";
            }

            if (grid_x + x_offset < 0 || ADJACENCY_BOXES <= grid_x + x_offset ||
                grid_y + y_offset < 0 || ADJACENCY_BOXES <= grid_y + y_offset)
            {Logger().debugStream() << "  *** position plus offset (" << (grid_x + x_offset) << ", " << (y_offset + grid_y) << ") is out of bounds! ***\n";
                continue;
            }

            const std::set<System*>& dest_grid_square = adjacency_grid[grid_x + x_offset][grid_y + y_offset];
            if (dest_grid_square.empty())
                continue;

            if (x_offset <= starlane_ranges[0] && y_offset <= starlane_ranges[0]) { // the destination falls in the short range
                if (!short_lanes)
                    continue;
                std::set<System*>::const_iterator system_it = dest_grid_square.begin();
                std::advance(system_it, RandSmallInt(0, dest_grid_square.size() - 1));
                System* dest_system = *system_it;
                if (system != dest_system) {
                    dest_system->AddStarlane(system->ID());
                    system->AddStarlane(dest_system->ID());
                    --short_lanes;
                }
            } else if (x_offset <= starlane_ranges[1] && y_offset <= starlane_ranges[1]) { // the destination falls in the medium range
                if (!medium_lanes)
                    continue;
                std::set<System*>::const_iterator system_it = dest_grid_square.begin();
                std::advance(system_it, RandSmallInt(0, dest_grid_square.size() - 1));
                System* dest_system = *system_it;
                if (system != dest_system) {
                    dest_system->AddStarlane(system->ID());
                    system->AddStarlane(dest_system->ID());
                    --medium_lanes;
                }
            } else { // the destination falls in the long range
                if (!long_lanes)
                    continue;
                std::set<System*>::const_iterator system_it = dest_grid_square.begin();
                std::advance(system_it, RandSmallInt(0, dest_grid_square.size() - 1));
                System* dest_system = *system_it;
                if (system != dest_system) {
                    dest_system->AddStarlane(system->ID());
                    system->AddStarlane(dest_system->ID());
                    --long_lanes;
                }
            }
        }
    }

    // TODO: make another pass to ensure every system has at least one starlane
}

void Universe::GenerateHomeworlds(int players, std::vector<int>& homeworlds)
{
    homeworlds.clear();

    std::vector<System*> sys_vec = FindObjects<System>();

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to generate homeworlds in an empty galaxy.");

    for (int i = 0; i < players; ++i) {
        int system_index;
        System* system;

        // make sure it has planets and it's not too close to the other homeworlds
        bool too_close = true;
        int attempts = 0;
        do {
            too_close = false;
            system_index = RandSmallInt(0, static_cast<int>(sys_vec.size()) - 1);
            system = sys_vec[system_index];
            for (unsigned int j = 0; j < homeworlds.size(); ++j) {
                System* existing_system = Object(homeworlds[j])->GetSystem();
                double x_dist = existing_system->X() - system->X();
                double y_dist = existing_system->Y() - system->Y();
                if (x_dist * x_dist + y_dist * y_dist < MIN_HOME_SYSTEM_SEPARATION * MIN_HOME_SYSTEM_SEPARATION) {
                    too_close = true;
                    break;
                }
            }
        } while ((!system->Orbits() || system->FindObjectIDs<Planet>().empty() || too_close) && ++attempts < 50);

        sys_vec.erase(sys_vec.begin() + system_index);

        // find a place to put the homeworld, and replace whatever planet is there already
        int planet_id, home_orbit;
        std::string planet_name;

        if (system->Orbits() > 0) {
            System::ObjectIDVec planet_IDs;
            while (planet_IDs.empty()) {
                home_orbit = RandSmallInt(0, system->Orbits() - 1);
                planet_IDs = system->FindObjectIDsInOrbit<Planet>(home_orbit);
            }
            planet_name = Object(planet_IDs.back())->Name();
            Delete(planet_IDs.back());
        } else {
            home_orbit = 0;
            planet_name = system->Name() + " " + RomanNumber(home_orbit + 1);
        }

        Planet* planet = new Planet(Planet::PT_TERRAN, Planet::SZ_LARGE);
        planet_id = Insert(planet);
        planet->Rename(planet_name);
        system->Insert(planet, home_orbit);

        homeworlds.push_back(planet_id);
    }
}

void Universe::PopulateSystems(Universe::PlanetDensity density)
{
    std::vector<System*> sys_vec = FindObjects<System>();

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to populate an empty galaxy.");

    const std::vector<std::vector<int> >& density_mod_to_planet_size_dist = UniverseDataTables()["DensityModToPlanetSizeDist"];
    const std::vector<std::vector<int> >& star_color_mod_to_planet_size_dist = UniverseDataTables()["StarColorModToPlanetSizeDist"];
    const std::vector<std::vector<int> >& slot_mod_to_planet_size_dist = UniverseDataTables()["SlotModToPlanetSizeDist"];
    const std::vector<std::vector<int> >& planet_size_mod_to_planet_type_dist = UniverseDataTables()["PlanetSizeModToPlanetTypeDist"];
    const std::vector<std::vector<int> >& slot_mod_to_planet_type_dist = UniverseDataTables()["SlotModToPlanetTypeDist"];
    const std::vector<std::vector<int> >& star_color_mod_to_planet_type_dist = UniverseDataTables()["StarColorModToPlanetTypeDist"];

    SmallIntDistType hundred_dist = SmallIntDist(1, 100);

    for (std::vector<System*>::iterator it = sys_vec.begin(); it != sys_vec.end(); ++it) {
        System* system = *it;

        int num_planets_in_system = 0;     // the number of slots in this system that were determined to contain planets
        for (int orbit = 0; orbit < system->Orbits(); orbit++) {
            // make a series of "rolls" (1-100) for each planet size, and take the highest modified roll
            int idx = 0;
            int max_roll = 0;
            for (unsigned int i = 0; i < Planet::MAX_PLANET_SIZE; ++i) {
                int roll = hundred_dist() + star_color_mod_to_planet_size_dist[system->Star()][i] + slot_mod_to_planet_size_dist[orbit][i]
                    + density_mod_to_planet_size_dist[density][i];
                if (max_roll < roll) {
                    max_roll = roll;
                    idx = i;
                }
            }
            Planet::PlanetSize planet_size = Planet::PlanetSize(idx);

            if (planet_size == Planet::SZ_NOWORLD)
                continue;
            else
                ++num_planets_in_system;

            if (planet_size == Planet::SZ_ASTEROIDS) {
                idx = Planet::PT_ASTEROIDS;
            } else if (planet_size == Planet::SZ_GASGIANT) {
                idx = Planet::PT_GASGIANT;
            } else {
                // make another series of modified rolls for planet type
                for (unsigned int i = 0; i < Planet::MAX_PLANET_TYPE; ++i) {
                    int roll = hundred_dist() + planet_size_mod_to_planet_type_dist[planet_size][i] + slot_mod_to_planet_type_dist[orbit][i] + 
                        star_color_mod_to_planet_type_dist[system->Star()][i];
                    if (max_roll < roll) {
                        max_roll = roll;
                        idx = i;
                    }
                }
            }
            Planet::PlanetType planet_type = Planet::PlanetType(idx);

            Planet* planet = new Planet(planet_type, planet_size);

            Insert(planet); // add planet to universe map
            system->Insert(planet, orbit);  // add planet to system map
            planet->Rename(system->Name() + " " + RomanNumber(num_planets_in_system));
        }
    }
}

void Universe::GenerateEmpires(int players, std::vector<int>& homeworlds)
{
#ifdef FREEORION_BUILD_SERVER
   // create empires and assign homeworlds, names, colors, and fleet ranges to
   // for each one

   const std::map<int, PlayerInfo>& player_info = ServerApp::GetApp()->NetworkCore().Players();
   int i = 0;
   for (std::map<int, PlayerInfo>::const_iterator it = player_info.begin(); it != player_info.end(); ++it, ++i) {
      // TODO: select name at random from default list
      std::string name = "Empire" + boost::lexical_cast<std::string>(i);

      // TODO: select color at random from default list
      GG::Clr color(RandZeroToOne(), RandZeroToOne(), RandZeroToOne(), 1.0);

      int home_planet_id = homeworlds[i];

      // create new Empire object through empire manager
      Empire* empire =
          dynamic_cast<ServerEmpireManager*>(&Empires())->CreateEmpire(it->first, name, color, home_planet_id, Empire::CONTROL_HUMAN);

      // set ownership of home planet
      int empire_id = empire->EmpireID();
      Planet* home_planet = dynamic_cast<Planet*>(Object(homeworlds[i]));
      Logger().debugStream() << "Setting " << home_planet->GetSystem()->Name() << " (Planet #" <<  home_planet->ID() <<
          ") to be home system for Empire " << empire_id;
      home_planet->AddOwner(empire_id);

      // TODO: adding an owner to a planet should probably add that owner to the
      //       system automatically...
      System* home_system = home_planet->GetSystem();
      home_system->AddOwner(empire_id);

      // create population and industry on home planet
      home_planet->AdjustPop(20);
      home_planet->SetWorkforce(20);
      home_planet->SetMaxWorkforce( home_planet->MaxPop() );
      home_planet->AdjustIndustry(0.10);
      home_planet->AdjustDefBases(3);

      // create the empire's initial ship designs
      // for now, the order that these are created need to match
      // the enums for ship designs in ships.h
      ShipDesign scout_design;
      scout_design.name = "Scout";
      scout_design.attack = 0;
      scout_design.defense = 1;
      scout_design.cost = 50;
      scout_design.colonize = false;
      scout_design.empire = empire_id;
      int scout_id = empire->AddShipDesign(scout_design);

      ShipDesign colony_ship_design;
      colony_ship_design.name = "Colony Ship";
      colony_ship_design.attack = 0;
      colony_ship_design.defense = 1;
      colony_ship_design.cost = 250;
      colony_ship_design.colonize = true;
      colony_ship_design.empire = empire_id;
      int colony_id = empire->AddShipDesign(colony_ship_design);

      ShipDesign design;
      design.name = "Mark I";
      design.attack = 2;
      design.defense = 1;
      design.cost = 100;
      design.colonize = false;
      design.empire = empire_id;
      empire->AddShipDesign(design);

      design.name = "Mark II";
      design.attack = 5;
      design.defense = 2;
      design.cost = 200;
      design.colonize = false;
      design.empire = empire_id;
      empire->AddShipDesign(design);

      design.name = "Mark III";
      design.attack = 10;
      design.defense = 3;
      design.cost = 375;
      design.colonize = false;
      design.empire = empire_id;
      empire->AddShipDesign(design);

      design.name = "Mark IV";
      design.attack = 15;
      design.defense = 5;
      design.cost = 700;
      design.colonize = false;
      design.empire = empire_id;
      empire->AddShipDesign(design);

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
#endif
}

int Universe::GenerateObjectID( )
{
  return( ++m_last_allocated_id );
}


