// -*- C++ -*-
#ifndef _UniverseGenerator_h_
#define _UniverseGenerator_h_

#include "Universe.h"
#include "../util/MultiplayerCommon.h"



struct PlayerSetupData;


typedef std::vector<std::vector<std::set<TemporaryPtr<System> > > > AdjacencyGrid;

// Class representing a position on the galaxy map, used
// to store the positions at which systems shall be created
struct SystemPosition {
    double x;
    double y;
    
    SystemPosition(double pos_x, double pos_y) :
        x(pos_x),
        y(pos_y)
    {}
    
    bool operator == (const SystemPosition &p)
    { return ((x == p.x) && (y == p.y)); }
};

/** A combination of names of ShipDesign that can be put together to make a
 * fleet of ships, and a name for such a fleet, loaded from starting_fleets.txt
 * ShipDesign names refer to designs listed in premade_ship_designs.txt.
 * Useful for saving or specifying prearranged combinations of prearranged
 * ShipDesigns to automatically put together, such as during universe creation.*/
class FleetPlan {
public:
    FleetPlan(const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
              bool lookup_name_userstring = false) :
        m_name(fleet_name),
        m_ship_designs(ship_design_names),
        m_name_in_stringtable(lookup_name_userstring)
    {}
    FleetPlan() :
        m_name(""),
        m_ship_designs(),
        m_name_in_stringtable(false)
    {}
    virtual ~FleetPlan() {};
    const std::string&              Name() const;
    const std::vector<std::string>& ShipDesigns() const { return m_ship_designs; }
protected:
    std::string                     m_name;
    std::vector<std::string>        m_ship_designs;
    bool                            m_name_in_stringtable;
};

/** The combination of a FleetPlan and spawning instructions for start of game
 * monsters. */
class FO_COMMON_API MonsterFleetPlan : public FleetPlan {
public:
    MonsterFleetPlan(const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
                     double spawn_rate = 1.0, int spawn_limit = 9999, const Condition::ConditionBase* location = 0,
                     bool lookup_name_userstring = false) :
        FleetPlan(fleet_name, ship_design_names, lookup_name_userstring),
        m_spawn_rate(spawn_rate),
        m_spawn_limit(spawn_limit),
        m_location(location)
    {}
    MonsterFleetPlan() :
        FleetPlan(),
        m_spawn_rate(1.0),
        m_spawn_limit(9999),
        m_location(0)
    {}
    virtual ~MonsterFleetPlan()
    { delete m_location; }
    double                          SpawnRate() const   { return m_spawn_rate; }
    int                             SpawnLimit() const  { return m_spawn_limit; }
    const Condition::ConditionBase* Location() const    { return m_location; }
protected:
    double                          m_spawn_rate;
    int                             m_spawn_limit;
    const Condition::ConditionBase* m_location;
};


// Calculates typical universe width based on number of systems
// A 150 star universe should be 1000 units across
double CalcTypicalUniverseWidth(int size);

// Helper functions that calculate system positions for various
// predefined galaxy shapes
void SpiralGalaxyCalcPositions(std::vector<SystemPosition>& positions,
                               unsigned int arms, unsigned int stars, double width, double height);

void EllipticalGalaxyCalcPositions(std::vector<SystemPosition>& positions,
                                   unsigned int stars, double width, double height);

void ClusterGalaxyCalcPositions(std::vector<SystemPosition>& positions, unsigned int clusters,
                                unsigned int stars, double width, double height);

void RingGalaxyCalcPositions(std::vector<SystemPosition>& positions, unsigned int stars,
                             double width, double height);

void IrregularGalaxyPositions(std::vector<SystemPosition>& positions, unsigned int stars,
                              double width, double height);

/** Generates planets for all systems that have empty object maps (ie those
 * that aren't homeworld systems).*/
void PopulateSystems(Universe& universe, GalaxySetupOption density);

/** Adds start-of-game specials to objects. */
void AddStartingSpecials(Universe& universe, GalaxySetupOption specials_freq);

/** Adds non-empire-affiliated native populations to planets. */
void GenerateNatives(Universe& universe, GalaxySetupOption freq);

/** Adds space monsters to systems. */
void GenerateSpaceMonsters(Universe& universe, GalaxySetupOption freq);

/** Creates starlanes and adds them systems already generated. */
void GenerateStarlanes(Universe& universe, GalaxySetupOption freq, const AdjacencyGrid& adjacency_grid);

/** Picks systems to host homeworlds, generates planets for them, stores
 * the ID's of the homeworld planets into the homeworld vector. */
void GenerateHomeworlds(Universe& universe, int players, std::vector<int>& homeworld_planet_ids);

/** Names the planets in each system, based on the system's name. */
void NamePlanets(Universe& universe);

/** Creates some initial fields in universe. */
void GenerateFields(Universe& universe, GalaxySetupOption freq);

/** Creates Empires objects for each entry in \a player_setup_data with
 * empire id equal to the specified player ids (so that the calling code
 * can know which empire belongs to which player).  Homeworlds are
 * associated with the empires, and starting buildings and fleets are
 * created, and empire starting ship designs are created and added. */
void GenerateEmpires(Universe& universe,  std::vector<int>& homeworld_planet_ids,
                     const std::map<int, PlayerSetupData>& player_setup_data);

/** Generates systems and planets, assigns homeworlds and populates them
 * with people, industry and bases, and places starting fleets.  Uses
 * predefined galaxy shapes.
 * WILL BE REMOVED!!! Currently kept because the new Python universe
 * generator interface is not yet able to replace this function completely */
void CreateUniverse(Universe& universe,
                    int size,                          Shape shape,
                    GalaxySetupOption age,             GalaxySetupOption starlane_freq,
                    GalaxySetupOption planet_density,  GalaxySetupOption specials_freq,
                    GalaxySetupOption monster_freq,    GalaxySetupOption native_freq,
                    const std::vector<SystemPosition>& positions,
                    const std::map<int, PlayerSetupData>& player_setup_data);


#endif // _UniverseGenerator_h_