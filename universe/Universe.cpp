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

#include <boost/tuple/tuple.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

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
    const double  OFFROAD_SLOWDOWN_FACTOR = 1000.0; // the factor by which non-starlane travel is slower than starlane travel

    DataTableMap& UniverseDataTables()
    {
        static DataTableMap map;
        if (map.empty()) {
            LoadDataTables("default/universe_tables.txt", map);
        }
        return map;
    }

    // use this to track the starlane-connected groups of systems, and the systems that make up each groups "corners".
    // we'll do a final pass to ensure that the entire galaxy is connected, at which point we'll connect the nearest corners of all groups
    struct ConnectedGroup
    {
        std::set<System*> systems;

        // "corner" systems, needed later in order to connect groups
        System* upper_left;
        System* upper_right;
        System* lower_right;
        System* lower_left;
    };

    // these return the System whose position is the farthest in a particular "corner" direction
    System* UpperLeft(System* s1, System *s2)  
    {
        double x_offset = s2->X() - s1->X();
        double y_offset = s2->Y() - s1->Y();
        return ((y_offset <= 0 && (x_offset <= 0 || x_offset < -y_offset)) || 
                (x_offset <= 0 && (y_offset <= 0 || y_offset < -x_offset))) ? s2 : s1;
    }
    System* UpperRight(System* s1, System *s2)
    {
        double x_offset = s2->X() - s1->X();
        double y_offset = s2->Y() - s1->Y();
        return ((y_offset <= 0 && (0 <= x_offset || -x_offset < -y_offset)) || 
                (0 <= x_offset && (y_offset <= 0 || y_offset < x_offset))) ? s2 : s1;
    }
    System* LowerRight(System* s1, System *s2)
    {
        double x_offset = s2->X() - s1->X();
        double y_offset = s2->Y() - s1->Y();
        return ((0 <= y_offset && (0 <= x_offset || -x_offset < y_offset)) || 
                (0 <= x_offset && (0 <= y_offset || -y_offset < x_offset))) ? s2 : s1;
    }
    System* LowerLeft(System* s1, System *s2)
    {
        double x_offset = s2->X() - s1->X();
        double y_offset = s2->Y() - s1->Y();
        return ((0 <= y_offset && (x_offset <= 0 || x_offset < y_offset)) || 
                (x_offset <= 0 && (0 <= y_offset || -y_offset < x_offset))) ? s2 : s1;
    }

    typedef boost::tuple<System*, System*, std::list<ConnectedGroup>::iterator, std::list<ConnectedGroup>::iterator> DistanceInfo;
    void InsertDistances(std::multimap<double, DistanceInfo>& distances, System* s1, 
                         std::list<ConnectedGroup>::iterator group1_it, std::list<ConnectedGroup>::iterator group2_it)
    {
        System* s2 = group2_it->upper_left;
        double x_dist = s2->X() - s1->X();
        double y_dist = s2->Y() - s1->Y();
        distances.insert(std::make_pair(x_dist * x_dist + y_dist * y_dist, DistanceInfo(s1, s2, group1_it, group2_it)));
        s2 = group2_it->upper_right;
        x_dist = s2->X() - s1->X();
        y_dist = s2->Y() - s1->Y();
        distances.insert(std::make_pair(x_dist * x_dist + y_dist * y_dist, DistanceInfo(s1, s2, group1_it, group2_it)));
        s2 = group2_it->lower_right;
        x_dist = s2->X() - s1->X();
        y_dist = s2->Y() - s1->Y();
        distances.insert(std::make_pair(x_dist * x_dist + y_dist * y_dist, DistanceInfo(s1, s2, group1_it, group2_it)));
        s2 = group2_it->lower_left;
        x_dist = s2->X() - s1->X();
        y_dist = s2->Y() - s1->Y();
        distances.insert(std::make_pair(x_dist * x_dist + y_dist * y_dist, DistanceInfo(s1, s2, group1_it, group2_it)));
    }

    // used to short-circuit the use of Dijkstra's algorithm for pathfinding when it finds the desired destination system
    struct PathFindingDijkstraVisitor : public boost::base_visitor<PathFindingDijkstraVisitor>
    {
        typedef boost::on_finish_vertex event_filter;

        struct FoundDestination {}; // exception type thrown when destination is found

        PathFindingDijkstraVisitor(int dest_system) : destination_system(dest_system) {}
        template <class Vertex, class Graph>
        void operator()(Vertex u, Graph& g)
        {
            if (u == destination_system)
                throw FoundDestination();
        }
        const int destination_system;
    };

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

    InitializeSystemGraph();
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

double Universe::LinearDistance(System* system1, System* system2) const
{
    return LinearDistance(system1->ID(), system2->ID());
}

double Universe::LinearDistance(int system1, int system2) const
{
    return m_system_distances.at(std::max(system1, system2)).at(std::min(system1, system2));
}

std::pair<std::list<System*>, double> Universe::ShortestPath(System* system1, System* system2) const
{
    return ShortestPath(system1->ID(), system2->ID());
}

std::pair<std::list<System*>, double> Universe::ShortestPath(int system1, int system2) const
{
    std::pair<std::list<System*>, double> retval;

    double linear_distance = LinearDistance(system1, system2);

    std::vector<int> predecessors(boost::num_vertices(m_system_graph));
    std::vector<double> distances(boost::num_vertices(m_system_graph));
    IndexPropertyMap index_map = boost::get(boost::vertex_index, m_system_graph);
    ConstEdgeWeightPropertyMap edge_weight_map = boost::get(boost::edge_weight, m_system_graph);
    try {
        boost::dijkstra_shortest_paths(m_system_graph, system1, &predecessors[0], &distances[0], edge_weight_map, index_map, 
                                    std::less<int>(), std::plus<int>(), std::numeric_limits<int>::max(), 0, 
                                    boost::make_dijkstra_visitor(PathFindingDijkstraVisitor(system2)));
    } catch (const PathFindingDijkstraVisitor::FoundDestination& fd) {
        // catching this just means that the destination was found, and so the algorithm was exited early, via exception
    }

    ConstSystemPointerPropertyMap pointer_property_map = boost::get(vertex_system_pointer_t(), m_system_graph);
    int current_system = system2;
    while (predecessors[current_system] != current_system) {
        retval.first.push_front(pointer_property_map[current_system]);
        retval.second += distances[current_system];
        current_system = predecessors[current_system];
    }

    // note that at this point retval.first will be empty if there was no starlane path from system1 to system2
    if (!retval.first.empty()) {
        retval.first.push_front(pointer_property_map[current_system]);
    }

    // if system2 is unreachable or it would be faster to travel "offroad", use the linear distance
    if (linear_distance * OFFROAD_SLOWDOWN_FACTOR < retval.second || retval.first.empty()) {
        retval.first.clear();
        retval.first.push_back(pointer_property_map[system1]);
        retval.first.push_back(pointer_property_map[system2]);
        retval.second = linear_distance;
    }

    return retval;
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
    GenerateStarlanes(LANES_AVERAGE, adjacency_grid);
    InitializeSystemGraph();
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

void Universe::GenerateStarlanes(StarlaneFreqency freq, const AdjacencyGrid& adjacency_grid)
{
    const double ADJACENCY_BOX_SIZE = UniverseWidth() / ADJACENCY_BOXES;

    std::vector<System*> sys_vec = FindObjects<System>();

    if (sys_vec.empty())
        throw std::runtime_error("Attempted to generate starlanes in an empty galaxy.");

    int MAX_LANES = UniverseDataTables()["MaxStarlanes"][0][freq];
    double MAX_LANE_LENGTH = static_cast<double>(UniverseDataTables()["MaxStarlaneLength"][0][0]);

    if (!MAX_LANES)
        return;

    SmallIntDistType lanes_dist = SmallIntDist(1, MAX_LANES);
    GaussianDistType lane_length_dist = GaussianDist(0.0, MAX_LANE_LENGTH / 3.0); // obviously, only the positive values generated by this dist should be used

    bool full_connectivity = true; // this may be optional later; for now, we should always do it
    std::list<ConnectedGroup> connected_groups;

    // generate the number of starlanes that each system should have
    std::map<System*, int> starlanes;
    for (unsigned int i = 0; i < sys_vec.size(); ++i) {
        starlanes[sys_vec[i]] = lanes_dist();
    }

    for (unsigned int i = 0; i < sys_vec.size(); ++i) {
        System* system = sys_vec[i];
        // subtract any starlanes that might have already been placed when processing other systems
        int lanes = starlanes[sys_vec[i]] - system->Starlanes();

        double system_x = system->X();
        double system_y = system->Y();
        while (0 < lanes) {
            double lane_length = -1.0;
            while (lane_length < 0.0) {
                lane_length = std::min(lane_length_dist(), MAX_LANE_LENGTH);
            }

            // look in the circle of adjacency grid boxes for a candidate.  If none is found, 
            // keep looking in circles of one grid-box-size smaller in radius each.  If still
            // nothing is found, look outside of lane_length until another system is found.
            bool placed = false;
            double current_lane_length = lane_length;
            bool first_chance = true;
            int attempts = 0;
            const int MAX_PLACEMENT_ATTEMPTS = 35;
            while (!placed && ++attempts < MAX_PLACEMENT_ATTEMPTS) {
                std::set<std::pair<unsigned int, unsigned int> > searched_grid_squares;
                // a simple heuristic for getting the approximate number of slices that will catch all the grid boxes 
                // in a cirle of the given grid-box radius
                const int SLICES = std::max(1, static_cast<int>(current_lane_length / ADJACENCY_BOX_SIZE * PI));
                const double SLICE_ANGLE = 2.0 * PI / SLICES;
                const int FIRST_SLICE = RandSmallInt(0, SLICES - 1); // this randomizes the direction in which we start looking
                for (int j = FIRST_SLICE; j < FIRST_SLICE + SLICES; ++j) {
                    double theta = SLICE_ANGLE * j;
                    unsigned int grid_x = static_cast<unsigned int>((system_x + current_lane_length * std::cos(theta)) / ADJACENCY_BOX_SIZE);
                    unsigned int grid_y = static_cast<unsigned int>((system_y + current_lane_length * std::sin(theta)) / ADJACENCY_BOX_SIZE);

                    if (grid_x < 0 || adjacency_grid.size() <= grid_x || grid_y < 0 || adjacency_grid.size() <= grid_y ||
                        searched_grid_squares.find(std::make_pair(grid_x, grid_y)) != searched_grid_squares.end())
                        continue;

                    searched_grid_squares.insert(std::make_pair(grid_x, grid_y));
                    const std::set<System*> grid_box = adjacency_grid[grid_x][grid_y];
                    if (!grid_box.empty()) {
                        std::set<System*>::const_iterator grid_box_it = grid_box.begin();
                        std::advance(grid_box_it, RandSmallInt(0, grid_box.size() - 1));
                        System* dest_system = *grid_box_it;
                        // don't place a starlane to yourself, or a system with enough starlanes already; but if we're 
                        // in second-chance mode, take any connection you can find
                        if (system != dest_system && (!first_chance || dest_system->Starlanes() < starlanes[dest_system])) {
                            system->AddStarlane(dest_system->ID());
                            dest_system->AddStarlane(system->ID());
                            placed = true;
                            --lanes;

                            if (full_connectivity) {
                                // record connectivity info; first, find out which group each system is already in, if any
                                std::list<ConnectedGroup>::iterator system_group_it, dest_system_group_it;
                                for (system_group_it = connected_groups.begin(); system_group_it != connected_groups.end(); ++system_group_it) {
                                    if (system_group_it->systems.find(system) != system_group_it->systems.end())
                                        break;
                                }
                                for (dest_system_group_it = connected_groups.begin(); dest_system_group_it != connected_groups.end(); ++dest_system_group_it) {
                                    if (dest_system_group_it->systems.find(dest_system) != dest_system_group_it->systems.end())
                                        break;
                                }

                                if (system_group_it == connected_groups.end() && dest_system_group_it == connected_groups.end()) {
                                    // niether belongs to a group, so create a new group
                                    connected_groups.push_back(ConnectedGroup());
                                    connected_groups.back().systems.insert(system);
                                    connected_groups.back().systems.insert(dest_system);
                                    connected_groups.back().upper_left = UpperLeft(system, dest_system);
                                    connected_groups.back().upper_right = UpperRight(system, dest_system);
                                    connected_groups.back().lower_right = LowerRight(system, dest_system);
                                    connected_groups.back().lower_left = LowerLeft(system, dest_system);
                                } else if (system_group_it != connected_groups.end() && dest_system_group_it != connected_groups.end() && system_group_it != dest_system_group_it) {
                                    std::string group1, group2, final_group;
                                    for (std::set<System*>::iterator sys_it = system_group_it->systems.begin(); sys_it != system_group_it->systems.end(); ++sys_it) {
                                        group1 += (*sys_it)->Name() + " ";
                                    }
                                    for (std::set<System*>::iterator sys_it = dest_system_group_it->systems.begin(); sys_it != dest_system_group_it->systems.end(); ++sys_it) {
                                        group2 += (*sys_it)->Name() + " ";
                                    }
                                    // each belongs to a group, so merge them
                                    system_group_it->systems.insert(dest_system_group_it->systems.begin(), dest_system_group_it->systems.end());
                                    for (std::set<System*>::iterator sys_it = system_group_it->systems.begin(); sys_it != system_group_it->systems.end(); ++sys_it) {
                                        final_group += (*sys_it)->Name() + " ";
                                    }
                                    system_group_it->upper_left = UpperLeft(system_group_it->upper_left, dest_system_group_it->upper_left);
                                    system_group_it->upper_right = UpperRight(system_group_it->upper_right, dest_system_group_it->upper_right);
                                    system_group_it->lower_right = LowerRight(system_group_it->lower_right, dest_system_group_it->lower_right);
                                    system_group_it->lower_left = LowerLeft(system_group_it->lower_left, dest_system_group_it->lower_left);
                                    connected_groups.erase(dest_system_group_it);
                                } else if (system_group_it != connected_groups.end()) {
                                    // add dest_system to system's group
                                    system_group_it->systems.insert(dest_system);
                                    system_group_it->upper_left = UpperLeft(system_group_it->upper_left, dest_system);
                                    system_group_it->upper_right = UpperRight(system_group_it->upper_right, dest_system);
                                    system_group_it->lower_right = LowerRight(system_group_it->lower_right, dest_system);
                                    system_group_it->lower_left = LowerLeft(system_group_it->lower_left, dest_system);
                                } else if (dest_system_group_it != connected_groups.end()) {
                                    // add system to dest_system's group
                                    dest_system_group_it->systems.insert(system);
                                    dest_system_group_it->upper_left = UpperLeft(dest_system_group_it->upper_left, system);
                                    dest_system_group_it->upper_right = UpperRight(dest_system_group_it->upper_right, system);
                                    dest_system_group_it->lower_right = LowerRight(dest_system_group_it->lower_right, system);
                                    dest_system_group_it->lower_left = LowerLeft(dest_system_group_it->lower_left, system);
                                }
                            }

                            break;
                        }
                    }
                }

                // bad luck; try again with a different length
                if (first_chance) {
                    current_lane_length -= ADJACENCY_BOX_SIZE;
                    // there's nothing suitable inside radius lane_length, so look outside of it now
                    if (current_lane_length < 0.0) {
                        current_lane_length = lane_length + ADJACENCY_BOX_SIZE;
                        first_chance = false;
                    }
                } else {
                    current_lane_length += ADJACENCY_BOX_SIZE;
                }
            }

            // give up on this lane after excessive attempts
            if (attempts == MAX_PLACEMENT_ATTEMPTS)
                --lanes;
        }
    }

    // warning: this algorithm is O(N^2), where N is the number of groups. 
    // normally, with a small number of groups, this is fine
    if (1 < connected_groups.size()) {
        // find the distances between the "corner" systems in all the groups
        std::multimap<double, DistanceInfo> distances;
        for (std::list<ConnectedGroup>::iterator it = connected_groups.begin(); it != connected_groups.end(); ++it) {
            for (std::list<ConnectedGroup>::iterator inner_it = it; inner_it != connected_groups.end(); ++inner_it) {
                if (it == inner_it)
                    continue;

                System* s1;
                s1 = it->upper_left;
                InsertDistances(distances, s1, it, inner_it);
                s1 = it->lower_left;
                InsertDistances(distances, s1, it, inner_it);
                s1 = it->lower_right;
                InsertDistances(distances, s1, it, inner_it);
                s1 = it->lower_left;
                InsertDistances(distances, s1, it, inner_it);
            }
        }

        std::set<int> groups_eliminated;
        for (unsigned int i = 0; i < connected_groups.size() - 1; ++i) {
            std::multimap<double, DistanceInfo>::iterator map_it = distances.begin();
            while (groups_eliminated.find(std::distance(connected_groups.begin(), map_it->second.get<2>())) != groups_eliminated.end() || 
                   groups_eliminated.find(std::distance(connected_groups.begin(), map_it->second.get<3>())) != groups_eliminated.end()) {\
               ++map_it;
            }
            DistanceInfo distance_info = map_it->second;
            std::list<ConnectedGroup>::iterator it1 = distance_info.get<2>();
            std::list<ConnectedGroup>::iterator it2 = distance_info.get<3>();
            System* system = distance_info.get<0>();
            System* dest_system = distance_info.get<1>();
            system->AddStarlane(dest_system->ID());
            dest_system->AddStarlane(system->ID());
            groups_eliminated.insert(std::distance(connected_groups.begin(), it2));
        }
    }
}

void Universe::InitializeSystemGraph()
{
    for (int i = static_cast<int>(boost::num_vertices(m_system_graph)) - 1; i >= 0; --i) {
        boost::clear_vertex(i, m_system_graph);
        boost::remove_vertex(i, m_system_graph);
    }

    std::vector<System*> systems = FindObjects<System>();
    m_system_distances.resize(systems.size());
    SystemPointerPropertyMap pointer_property_map = boost::get(vertex_system_pointer_t(), m_system_graph);

    EdgeWeightPropertyMap edge_weight_map = boost::get(boost::edge_weight, m_system_graph);
    typedef boost::graph_traits<SystemGraph>::edge_descriptor EdgeDescriptor;

    for (int i = 0; i < static_cast<int>(systems.size()); ++i) {
        // add a vertex to the graph for this system, and assign it a pointer for its System object
        boost::add_vertex(m_system_graph);
        System* system1 = systems[i];
        pointer_property_map[i] = system1;

        // add edges and edge weights
        for (System::lane_iterator it = system1->begin_lanes(); it != system1->end_lanes(); ++it) {
            std::pair<EdgeDescriptor, bool> add_edge_result = boost::add_edge(system1->ID(), it->first, m_system_graph);
            if (it->second) { // if this is a wormhole
                edge_weight_map[add_edge_result.first] = 0.0;
            } else if (add_edge_result.second) { // if this is a non-duplicate starlane
                UniverseObject* system2 = Object(it->first);
                double x_dist = system2->X() - system1->X();
                double y_dist = system2->Y() - system1->Y();
                edge_weight_map[add_edge_result.first] = std::sqrt(x_dist * x_dist + y_dist * y_dist);
            }
        }

        // define the straight-line system distances for this system
        m_system_distances[i].clear();
        for (int j = 0; j < i; ++j) {
            UniverseObject* system2 = Object(j);
            double x_dist = system2->X() - system1->X();
            double y_dist = system2->Y() - system1->Y();
            m_system_distances[i].push_back(std::sqrt(x_dist * x_dist + y_dist * y_dist));
        }
        m_system_distances[i].push_back(0.0);
    }
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


