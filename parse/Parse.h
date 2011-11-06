// -*- C++ -*-
#ifndef _Parse_h_
#define _Parse_h_

#include "../universe/Tech.h"

#include <boost/filesystem/path.hpp>


class Alignment;
class BuildingType;
class FleetPlan;
class HullType;
class MonsterFleetPlan;
class PartType;
class ShipDesign;
class Special;
class Species;

namespace parse {

    void init();

    bool buildings(const boost::filesystem::path& path, std::map<std::string, BuildingType*>& building_types);

    bool specials(const boost::filesystem::path& path, std::map<std::string, Special*>& specials_);

    bool species(const boost::filesystem::path& path, std::map<std::string, Species*>& species_);

    bool techs(const boost::filesystem::path& path,
               TechManager::TechContainer& techs_,
               std::map<std::string, TechCategory*>& tech_categories,
               std::set<std::string>& categories_seen);

    bool items(const boost::filesystem::path& path, std::vector<ItemSpec>& items_);

    bool ship_parts(const boost::filesystem::path& path, std::map<std::string, PartType*>& parts);

    bool ship_hulls(const boost::filesystem::path& path, std::map<std::string, HullType*>& hulls);

    bool ship_designs(const boost::filesystem::path& path, std::map<std::string, ShipDesign*>& designs);

    bool fleet_plans(const boost::filesystem::path& path, std::vector<FleetPlan*>& fleet_plans_);

    bool monster_fleet_plans(const boost::filesystem::path& path, std::vector<MonsterFleetPlan*>& monster_fleet_plans_);

    bool alignments(const boost::filesystem::path& path,
                    std::vector<Alignment>& alignments_,
                    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups);

}

#endif
