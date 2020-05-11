#ifndef _FleetPlan_h_
#define _FleetPlan_h_


#include <memory>
#include <string>
#include <vector>
#include "../universe/Condition.h"
#include "../util/Export.h"


//! Prepopulated Fleet, consisting of a name and list of ShipDesign names.
//!
//! FleetPlan%s are used for providing Fleets during universe generation or
//! during game events.
//! 
//! Fleet plans are loaded from the `default/scripting/starting_unlocks/fleets.yml`
//! file, the ShipDesign referenced are liste in `default/scripting/ship_designs`.
class FO_COMMON_API FleetPlan {
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

    auto Name() const -> const std::string&;

    auto ShipDesigns() const -> const std::vector<std::string>&
    { return m_ship_designs; }

protected:
    std::string              m_name;
    std::vector<std::string> m_ship_designs;
    bool                     m_name_in_stringtable;
};


//! Spawning instruction for Monster Fleets during universe generation.
class FO_COMMON_API MonsterFleetPlan : public FleetPlan {
public:
    MonsterFleetPlan(const std::string& fleet_name, const std::vector<std::string>& ship_design_names,
                     double spawn_rate = 1.0, int spawn_limit = 9999,
                     std::unique_ptr<Condition::Condition>&& location = nullptr,
                     bool lookup_name_userstring = false) :
        FleetPlan(fleet_name, ship_design_names, lookup_name_userstring),
        m_spawn_rate(spawn_rate),
        m_spawn_limit(spawn_limit),
        m_location(std::move(location))
    {}

    MonsterFleetPlan() :
        FleetPlan()
    {}

    auto SpawnRate() const -> auto
    { return m_spawn_rate; }

    auto SpawnLimit() const -> int
    { return m_spawn_limit; }

    auto Location() const -> const Condition::Condition*
    { return m_location.get(); }

protected:
    double                                      m_spawn_rate = 1.0;
    int                                         m_spawn_limit = 9999;
    // Use shared_ptr insead of unique_ptr because boost::python requires a deleter
    const std::shared_ptr<Condition::Condition> m_location;
};


#endif
