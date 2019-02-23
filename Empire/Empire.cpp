#include "Empire.h"

#include "../util/i18n.h"
#include "../util/Random.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"
#include "../util/SitRepEntry.h"
#include "../universe/Building.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Tech.h"
#include "../universe/UniverseObject.h"
#include "EmpireManager.h"
#include "Supply.h"

#include <unordered_set>


namespace {
    const float EPSILON = 0.01f;
    const std::string EMPTY_STRING;
}


////////////
// Empire //
////////////
Empire::Empire() :
    m_authenticated(false),
    m_research_queue(m_id),
    m_production_queue(m_id)
{ Init(); }

Empire::Empire(const std::string& name, const std::string& player_name,
               int empire_id, const GG::Clr& color, bool authenticated) :
    m_id(empire_id),
    m_name(name),
    m_player_name(player_name),
    m_authenticated(authenticated),
    m_color(color),
    m_research_queue(m_id),
    m_production_queue(m_id)
{
    DebugLogger() << "Empire::Empire(" << name << ", " << player_name << ", " << empire_id << ", colour)";
    Init();
}

void Empire::Init() {
    m_resource_pools[RE_RESEARCH] = std::make_shared<ResourcePool>(RE_RESEARCH);
    m_resource_pools[RE_INDUSTRY] = std::make_shared<ResourcePool>(RE_INDUSTRY);
    m_resource_pools[RE_TRADE] = std::make_shared<ResourcePool>(RE_TRADE);

    m_eliminated = false;

    m_meters[UserStringNop("METER_DETECTION_STRENGTH")];
    m_meters[UserStringNop("METER_BUILDING_COST_FACTOR")];
    m_meters[UserStringNop("METER_SHIP_COST_FACTOR")];
    m_meters[UserStringNop("METER_TECH_COST_FACTOR")];
}

Empire::~Empire()
{ ClearSitRep(); }

const std::string& Empire::Name() const
{ return m_name; }

const std::string& Empire::PlayerName() const
{ return m_player_name; }

bool Empire::IsAuthenticated() const
{ return m_authenticated; }

int Empire::EmpireID() const
{ return m_id; }

const GG::Clr& Empire::Color() const
{ return m_color; }

int Empire::CapitalID() const
{ return m_capital_id; }

int Empire::SourceID() const {
    auto good_source = Source();
    return good_source ? good_source->ID() : INVALID_OBJECT_ID;
}

std::shared_ptr<const UniverseObject> Empire::Source() const {
    if (m_eliminated)
        return nullptr;

    // Use the current source if valid
    auto valid_current_source = GetUniverseObject(m_source_id);
    if (valid_current_source && valid_current_source->OwnedBy(m_id))
        return valid_current_source;

    // Try the capital
    auto capital_as_source = GetUniverseObject(m_capital_id);
    if (capital_as_source && capital_as_source->OwnedBy(m_id)) {
        m_source_id = m_capital_id;
        return capital_as_source;
    }

    // Find any object owned by the empire
    // TODO determine if ExistingObjects() is faster and acceptable
    for (const auto& obj_it : Objects()) {
        if (obj_it->OwnedBy(m_id)) {
            m_source_id = obj_it->ID();
            return (obj_it);
        }
    }

    m_source_id = INVALID_OBJECT_ID;
    return nullptr;
}

std::string Empire::Dump() const {
    std::string retval = "Empire name: " + m_name +
                         " ID: " + std::to_string(m_id) +
                         " Capital ID: " + std::to_string(m_capital_id);
    retval += " meters:\n";
    for (const auto& meter : m_meters) {
        retval += UserString(meter.first) + ": " +
                  std::to_string(meter.second.Initial()) + "\n";
    }
    return retval;
}

void Empire::SetCapitalID(int id) {
    m_capital_id = INVALID_OBJECT_ID;
    m_source_id = INVALID_OBJECT_ID;

    if (id == INVALID_OBJECT_ID)
        return;

    // Verify that the capital exists and is owned by the empire
    auto possible_capital = Objects().ExistingObject(id);
    if (possible_capital && possible_capital->OwnedBy(m_id))
        m_capital_id = id;

    auto possible_source = GetUniverseObject(id);
    if (possible_source && possible_source->OwnedBy(m_id))
        m_source_id = id;
}

Meter* Empire::GetMeter(const std::string& name) {
    auto it = m_meters.find(name);
    if (it != m_meters.end())
        return &(it->second);
    else
        return nullptr;
}

const Meter* Empire::GetMeter(const std::string& name) const {
    auto it = m_meters.find(name);
    if (it != m_meters.end())
        return &(it->second);
    else
        return nullptr;
}

void Empire::BackPropagateMeters() {
    for (auto& meter : m_meters)
        meter.second.BackPropagate();
}

bool Empire::ResearchableTech(const std::string& name) const {
    const Tech* tech = GetTech(name);
    if (!tech)
        return false;
    for (const auto& prereq : tech->Prerequisites()) {
        if (!m_techs.count(prereq))
            return false;
    }
    return true;
}

bool Empire::HasResearchedPrereqAndUnresearchedPrereq(const std::string& name) const {
    const Tech* tech = GetTech(name);
    if (!tech)
        return false;
    bool one_unresearched = false;
    bool one_researched = false;
    for (const auto& prereq : tech->Prerequisites()) {
        if (m_techs.count(prereq))
            one_researched = true;
        else
            one_unresearched = true;
    }
    return one_unresearched && one_researched;
}

const ResearchQueue& Empire::GetResearchQueue() const
{ return m_research_queue; }

float Empire::ResearchProgress(const std::string& name) const {
    auto it = m_research_progress.find(name);
    if (it == m_research_progress.end())
        return 0.0f;
    const Tech* tech = GetTech(it->first);
    if (!tech)
        return 0.0f;
    float tech_cost = tech->ResearchCost(m_id);
    return it->second * tech_cost;
}

const std::map<std::string, int>& Empire::ResearchedTechs() const
{ return m_techs; }

bool Empire::TechResearched(const std::string& name) const
{ return m_techs.count(name); }

TechStatus Empire::GetTechStatus(const std::string& name) const {
    if (TechResearched(name)) return TS_COMPLETE;
    if (ResearchableTech(name)) return TS_RESEARCHABLE;
    if (HasResearchedPrereqAndUnresearchedPrereq(name)) return TS_HAS_RESEARCHED_PREREQ;
    return TS_UNRESEARCHABLE;
}

const std::string& Empire::TopPriorityEnqueuedTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    auto it = m_research_queue.begin();
    const std::string& tech = it->name;
    return tech;
}

const std::string& Empire::MostExpensiveEnqueuedTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    float biggest_cost = -99999.9f; // arbitrary small number

    const ResearchQueue::Element* best_elem = nullptr;

    for (const auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech)
            continue;
        float tech_cost = tech->ResearchCost(m_id);
        if (tech_cost > biggest_cost) {
            biggest_cost = tech_cost;
            best_elem = &elem;
        }
    }

    if (best_elem)
        return best_elem->name;
    return EMPTY_STRING;
}

const std::string& Empire::LeastExpensiveEnqueuedTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    float smallest_cost = 999999.9f; // arbitrary large number

    const ResearchQueue::Element* best_elem = nullptr;

    for (const auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech)
            continue;
        float tech_cost = tech->ResearchCost(m_id);
        if (tech_cost < smallest_cost) {
            smallest_cost = tech_cost;
            best_elem = &elem;
        }
    }

    if (best_elem)
        return best_elem->name;
    return EMPTY_STRING;
}

const std::string& Empire::MostRPSpentEnqueuedTech() const {
    float most_spent = -999999.9f;  // arbitrary small number
    const std::map<std::string, float>::value_type* best_progress = nullptr;

    for (const auto& progress : m_research_progress) {
        const auto& tech_name = progress.first;
        if (!m_research_queue.InQueue(tech_name))
            continue;
        float rp_spent = progress.second;
        if (rp_spent > most_spent) {
            best_progress = &progress;
            most_spent = rp_spent;
        }
    }

    if (best_progress)
        return best_progress->first;
    return EMPTY_STRING;
}

const std::string& Empire::MostRPCostLeftEnqueuedTech() const {
    float most_left = -999999.9f;  // arbitrary small number
    const std::map<std::string, float>::value_type* best_progress = nullptr;

    for (const auto& progress : m_research_progress) {
        const auto& tech_name = progress.first;
        const Tech* tech = GetTech(tech_name);
        if (!tech)
            continue;

        if (!m_research_queue.InQueue(tech_name))
            continue;

        float rp_spent = progress.second;
        float rp_total_cost = tech->ResearchCost(m_id);
        float rp_left = std::max(0.0f, rp_total_cost - rp_spent);

        if (rp_left > most_left) {
            best_progress = &progress;
            most_left = rp_left;
        }
    }

    if (best_progress)
        return best_progress->first;
    return EMPTY_STRING;
}

const std::string& Empire::TopPriorityResearchableTech() const {
    if (m_research_queue.empty())
        return EMPTY_STRING;
    for (const auto& elem : m_research_queue) {
        if (this->ResearchableTech(elem.name))
            return elem.name;
    }
    return EMPTY_STRING;
}

const std::string& Empire::MostExpensiveResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::LeastExpensiveResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::MostRPSpentResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::string& Empire::MostRPCostLeftResearchableTech() const {
    return EMPTY_STRING;    // TODO: IMPLEMENT THIS
}

const std::set<std::string>& Empire::AvailableBuildingTypes() const
{ return m_available_building_types; }

bool Empire::BuildingTypeAvailable(const std::string& name) const
{ return m_available_building_types.count(name); }

const std::set<int>& Empire::ShipDesigns() const
{ return m_known_ship_designs; }

std::set<int> Empire::AvailableShipDesigns() const {
    // create new map containing all ship designs that are available
    std::set<int> retval;
    for (int design_id : m_known_ship_designs) {
        if (ShipDesignAvailable(design_id))
            retval.insert(design_id);
    }
    return retval;
}

bool Empire::ShipDesignAvailable(int ship_design_id) const {
    const ShipDesign* design = GetShipDesign(ship_design_id);
    return design ? ShipDesignAvailable(*design) : false;
}

bool Empire::ShipDesignAvailable(const ShipDesign& design) const {
    if (!design.Producible()) return false;

    // design is kept, but still need to verify that it is buildable at this time.  Part or hull tech
    // requirements might prevent it from being built.
    for (const auto& name : design.Parts()) {
        if (name.empty())
            continue;   // empty slot can't be unavailable
        if (!ShipPartAvailable(name))
            return false;
    }
    if (!ShipHullAvailable(design.Hull()))
        return false;

    // if there are no reasons the design isn't available, then by default it is available
    return true;
}

bool Empire::ShipDesignKept(int ship_design_id) const
{ return m_known_ship_designs.count(ship_design_id); }

const std::set<std::string>& Empire::AvailableShipParts() const
{ return m_available_part_types; }

bool Empire::ShipPartAvailable(const std::string& name) const
{ return m_available_part_types.count(name); }

const std::set<std::string>& Empire::AvailableShipHulls() const
{ return m_available_hull_types; }

bool Empire::ShipHullAvailable(const std::string& name) const
{ return m_available_hull_types.count(name); }

const ProductionQueue& Empire::GetProductionQueue() const
{ return m_production_queue; }

float Empire::ProductionStatus(int i) const {
    if (0 > i || i >= static_cast<int>(m_production_queue.size()))
        return -1.0f;
    float item_progress = m_production_queue[i].progress;
    float item_cost;
    int item_time;
    std::tie(item_cost, item_time) = this->ProductionCostAndTime(m_production_queue[i]);
    return item_progress * item_cost * m_production_queue[i].blocksize;
}

std::pair<float, int> Empire::ProductionCostAndTime(const ProductionQueue::Element& element) const
{ return ProductionCostAndTime(element.item, element.location); }

std::pair<float, int> Empire::ProductionCostAndTime(const ProductionQueue::ProductionItem& item,
                                                    int location_id) const
{
    if (item.build_type == BT_BUILDING) {
        const BuildingType* type = GetBuildingType(item.name);
        if (!type)
            return std::make_pair(-1.0, -1);
        return std::make_pair(type->ProductionCost(m_id, location_id),
                              type->ProductionTime(m_id, location_id));
    } else if (item.build_type == BT_SHIP) {
        const ShipDesign* design = GetShipDesign(item.design_id);
        if (design)
            return std::make_pair(design->ProductionCost(m_id, location_id),
                                  design->ProductionTime(m_id, location_id));
        return std::make_pair(-1.0, -1);
    } else if (item.build_type == BT_STOCKPILE) {
        return std::make_pair(1.0, 1);
    }
    ErrorLogger() << "Empire::ProductionCostAndTime was passed a ProductionItem with an invalid BuildType";
    return std::make_pair(-1.0, -1);
}

bool Empire::HasExploredSystem(int ID) const
{ return m_explored_systems.count(ID); }

bool Empire::ProducibleItem(BuildType build_type, int location_id) const {
    if (build_type == BT_SHIP)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_SHIP with no further parameters, but ship designs are tracked by number");

    if (build_type == BT_BUILDING)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_BUILDING with no further parameters, but buildings are tracked by name");

    if (location_id == INVALID_OBJECT_ID)
        return false;

    // must own the production location...
    auto location = GetUniverseObject(location_id);
    if (!location) {
        WarnLogger() << "Empire::ProducibleItem for BT_STOCKPILE unable to get location object with id " << location_id;
        return false;
    }

    if (!location->OwnedBy(m_id))
        return false;

    if (!std::dynamic_pointer_cast<const ResourceCenter>(location))
        return false;

    if (build_type == BT_STOCKPILE) {
        return true;

    } else {
        ErrorLogger() << "Empire::ProducibleItem was passed an invalid BuildType";
        return false;
    }

}

bool Empire::ProducibleItem(BuildType build_type, const std::string& name, int location) const {
    // special case to check for ships being passed with names, not design ids
    if (build_type == BT_SHIP)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_SHIP with a name, but ship designs are tracked by number");

    if (build_type == BT_STOCKPILE)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_STOCKPILE with a name, but the stockpile does not need an identification");

    if (build_type == BT_BUILDING && !BuildingTypeAvailable(name))
        return false;

    const auto* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    auto build_location = GetUniverseObject(location);
    if (!build_location)
        return false;

    if (build_type == BT_BUILDING) {
        // specified location must be a valid production location for that building type
        return building_type->ProductionLocation(m_id, location);

    } else {
        ErrorLogger() << "Empire::ProducibleItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::ProducibleItem(BuildType build_type, int design_id, int location) const {
    // special case to check for buildings being passed with ids, not names
    if (build_type == BT_BUILDING)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_BUILDING with a design id number, but buildings are tracked by name");

    if (build_type == BT_STOCKPILE)
        throw std::invalid_argument("Empire::ProducibleItem was passed BuildType BT_STOCKPILE with a design id, but the stockpile does not need an identification");

    if (build_type == BT_SHIP && !ShipDesignAvailable(design_id))
        return false;

    // design must be known to this empire
    const ShipDesign* ship_design = GetShipDesign(design_id);
    if (!ship_design || !ship_design->Producible())
        return false;

    auto build_location = GetUniverseObject(location);
    if (!build_location) return false;

    if (build_type == BT_SHIP) {
        // specified location must be a valid production location for this design
        return ship_design->ProductionLocation(m_id, location);

    } else {
        ErrorLogger() << "Empire::ProducibleItem was passed an invalid BuildType";
        return false;
    }
}

bool Empire::ProducibleItem(const ProductionQueue::ProductionItem& item, int location) const {
    if (item.build_type == BT_BUILDING)
        return ProducibleItem(item.build_type, item.name, location);
    else if (item.build_type == BT_SHIP)
        return ProducibleItem(item.build_type, item.design_id, location);
    else if (item.build_type == BT_STOCKPILE)
        return ProducibleItem(item.build_type, location);
    else
        throw std::invalid_argument("Empire::ProducibleItem was passed a ProductionItem with an invalid BuildType");
    return false;
}

bool Empire::EnqueuableItem(BuildType build_type, const std::string& name, int location) const {
    if (build_type != BT_BUILDING)
        return false;

    const auto* building_type = GetBuildingType(name);
    if (!building_type || !building_type->Producible())
        return false;

    auto build_location = GetUniverseObject(location);
    if (!build_location)
        return false;

    // specified location must be a valid production location for that building type
    return building_type->EnqueueLocation(m_id, location);
}

bool Empire::EnqueuableItem(const ProductionQueue::ProductionItem& item, int location) const {
    if (item.build_type == BT_BUILDING)
        return EnqueuableItem(item.build_type, item.name, location);
    else if (item.build_type == BT_SHIP)    // ships don't have a distinction between enqueuable and producible
        return ProducibleItem(item.build_type, item.design_id, location);
    else if (item.build_type == BT_STOCKPILE) // stockpile does not have a distinction between enqueuable and producible
        return ProducibleItem(item.build_type, location);
    else
        throw std::invalid_argument("Empire::ProducibleItem was passed a ProductionItem with an invalid BuildType");
    return false;
}


int Empire::NumSitRepEntries(int turn/* = INVALID_GAME_TURN*/) const {
    if (turn == INVALID_GAME_TURN)
        return m_sitrep_entries.size();
    int count = 0;
    for (const SitRepEntry& sitrep : m_sitrep_entries)
        if (sitrep.GetTurn() == turn)
            count++;
    return count;
}

bool Empire::Eliminated() const {
    return m_eliminated;
}

void Empire::Eliminate() {
    m_eliminated = true;

    for (auto& entry : Empires())
        entry.second->AddSitRepEntry(CreateEmpireEliminatedSitRep(EmpireID()));

    // some Empire data not cleared when eliminating since it might be useful
    // to remember later, and having it doesn't hurt anything (as opposed to
    // the production queue that might actually cause some problems if left
    // uncleared after elimination

    m_capital_id = INVALID_OBJECT_ID;
    // m_techs
    m_research_queue.clear();
    m_research_progress.clear();
    m_production_queue.clear();
    // m_available_building_types;
    // m_available_part_types;
    // m_available_hull_types;
    // m_explored_systems;
    // m_known_ship_designs;
    m_sitrep_entries.clear();
    for (auto& entry : m_resource_pools)
        entry.second->SetObjects(std::vector<int>());
    m_population_pool.SetPopCenters(std::vector<int>());

    // m_ship_names_used;
    m_supply_system_ranges.clear();
    m_supply_unobstructed_systems.clear();
}

bool Empire::Won() const {
    return !m_victories.empty();
}

void Empire::Win(const std::string& reason) {
    if (m_victories.insert(reason).second) {
        for (auto& entry : Empires()) {
            entry.second->AddSitRepEntry(CreateVictorySitRep(reason, EmpireID()));
        }
    }
}

void Empire::UpdateSystemSupplyRanges(const std::set<int>& known_objects) {
    //std::cout << "Empire::UpdateSystemSupplyRanges() for empire " << this->Name() << std::endl;
    m_supply_system_ranges.clear();

    // as of this writing, only planets can generate supply propagation
    std::vector<std::shared_ptr<const UniverseObject>> owned_planets;
    for (int object_id : known_objects) {
        if (auto planet = GetPlanet(object_id))
            if (planet->OwnedBy(this->EmpireID()))
                owned_planets.push_back(planet);
    }

    //std::cout << "... empire owns " << owned_planets.size() << " planets" << std::endl;
    for (auto& obj : owned_planets) {
        //std::cout << "... considering owned planet: " << obj->Name() << std::endl;

        // ensure object is within a system, from which it can distribute supplies
        int system_id = obj->SystemID();
        if (system_id == INVALID_OBJECT_ID)
            continue;   // TODO: consider future special case if current object is itself a system

        // check if object has a supply meter
        if (obj->GetMeter(METER_SUPPLY)) {
            // get resource supply range for next turn for this object
            float supply_range = obj->InitialMeterValue(METER_SUPPLY);

            // if this object can provide more supply range than the best previously checked object in this system, record its range as the new best for the system
            auto system_it = m_supply_system_ranges.find(system_id);  // try to find a previous entry for this system's supply range
            if (system_it == m_supply_system_ranges.end() || supply_range > system_it->second) {// if there is no previous entry, or the previous entry is shorter than the new one, add or replace the entry
                //std::cout << " ... object " << obj->Name() << " has resource supply range: " << resource_supply_range << std::endl;
                m_supply_system_ranges[system_id] = supply_range;
            }
        }
    }
}

void Empire::UpdateSystemSupplyRanges() {
    const Universe& universe = GetUniverse();
    const ObjectMap& empire_known_objects = EmpireKnownObjects(this->EmpireID());

    // get ids of objects partially or better visible to this empire.
    std::vector<int> known_objects_vec = empire_known_objects.FindObjectIDs();
    const std::set<int>& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());

    std::set<int> known_objects_set;

    // exclude objects known to have been destroyed (or rather, include ones that aren't known by this empire to be destroyed)
    for (int object_id : known_objects_vec)
        if (!known_destroyed_objects.count(object_id))
            known_objects_set.insert(object_id);
    UpdateSystemSupplyRanges(known_objects_set);
}

void Empire::UpdateUnobstructedFleets() {
    const std::set<int>& known_destroyed_objects =
        GetUniverse().EmpireKnownDestroyedObjectIDs(this->EmpireID());

    for (int system_id : m_supply_unobstructed_systems) {
        auto system = GetSystem(system_id);
        if (!system)
            continue;

        for (auto& fleet : Objects().FindObjects<Fleet>(system->FleetIDs())) {
            if (known_destroyed_objects.count(fleet->ID()))
                continue;
            if (fleet->OwnedBy(m_id))
                fleet->SetArrivalStarlane(system_id);
        }
    }
}

void Empire::UpdateSupplyUnobstructedSystems(bool precombat /*=false*/) {
    Universe& universe = GetUniverse();

    // get ids of systems partially or better visible to this empire.
    // TODO: make a UniverseObjectVisitor for objects visible to an empire at a specified visibility or greater
    std::vector<int> known_systems_vec = EmpireKnownObjects(this->EmpireID()).FindObjectIDs<System>();
    const std::set<int>& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());

    std::set<int> known_systems_set;

    // exclude systems known to have been destroyed (or rather, include ones that aren't known to be destroyed)
    for (int system_id : known_systems_vec)
        if (!known_destroyed_objects.count(system_id))
            known_systems_set.insert(system_id);
    UpdateSupplyUnobstructedSystems(known_systems_set, precombat);
}

void Empire::UpdateSupplyUnobstructedSystems(const std::set<int>& known_systems, bool precombat /*=false*/) {
    //DebugLogger() << "UpdateSupplyUnobstructedSystems for empire " << m_id;
    m_supply_unobstructed_systems.clear();

    // get systems with historically at least partial visibility
    std::set<int> systems_with_at_least_partial_visibility_at_some_point;
    for (int system_id : known_systems) {
        const auto& vis_turns = GetUniverse().GetObjectVisibilityTurnMapByEmpire(system_id, m_id);
        if (vis_turns.count(VIS_PARTIAL_VISIBILITY))
            systems_with_at_least_partial_visibility_at_some_point.insert(system_id);
    }

    // get all fleets, or just those visible to this client's empire
    const auto& known_destroyed_objects = GetUniverse().EmpireKnownDestroyedObjectIDs(this->EmpireID());

    // get empire supply ranges
    std::map<int, std::map<int, float>> empire_system_supply_ranges;
    for (const auto& entry : Empires()) {
        const Empire* empire = entry.second;
        empire_system_supply_ranges[entry.first] = empire->SystemSupplyRanges();
    }

    // find systems that contain fleets that can either maintain supply or block supply.
    // to affect supply in either manner, a fleet must be armed & aggressive, & must be not
    // trying to depart the systme.  Qualifying enemy fleets will blockade if no friendly fleets
    // are present, or if the friendly fleets were already blockade-restricted and the enemy
    // fleets were not (meaning that the enemy fleets were continuing an existing blockade)
    // Friendly fleets can preserve available starlane accesss even if they are trying to leave the system

    // Unrestricted lane access (i.e, (fleet->ArrivalStarlane() == system->ID()) ) is used as a proxy for
    // order of arrival -- if an enemy has unrestricted lane access and you don't, they must have arrived
    // before you, or be in cahoots with someone who did.
    std::set<int> systems_containing_friendly_fleets;
    std::set<int> systems_with_lane_preserving_fleets;
    std::set<int> unrestricted_friendly_systems;
    std::set<int> systems_containing_obstructing_objects;
    std::set<int> unrestricted_obstruction_systems;
    for (auto& fleet : GetUniverse().Objects().FindObjects<Fleet>()) {
        int system_id = fleet->SystemID();
        if (system_id == INVALID_OBJECT_ID) {
            continue;   // not in a system, so can't affect system obstruction
        } else if (known_destroyed_objects.count(fleet->ID())) {
            continue; //known to be destroyed so can't affect supply, important just in case being updated on client side
        }

        //DebugLogger() << "Fleet " << fleet->ID() << " is in system " << system_id << " with next system " << fleet->NextSystemID() << " and is owned by " << fleet->Owner() << " armed: " << fleet->HasArmedShips() << " and agressive: " << fleet->Aggressive();
        if ((fleet->HasArmedShips() || fleet->HasFighterShips()) && fleet->Aggressive()) {
            if (fleet->OwnedBy(m_id)) {
                if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                    systems_containing_friendly_fleets.insert(system_id);
                    if (fleet->ArrivalStarlane() == system_id)
                        unrestricted_friendly_systems.insert(system_id);
                    else
                        systems_with_lane_preserving_fleets.insert(system_id);
                }
            } else if (fleet->NextSystemID() == INVALID_OBJECT_ID || fleet->NextSystemID() == fleet->SystemID()) {
                int fleet_owner = fleet->Owner();
                bool fleet_at_war = fleet_owner == ALL_EMPIRES || Empires().GetDiplomaticStatus(m_id, fleet_owner) == DIPLO_WAR;
                // newly created ships are not allowed to block supply since they have not even potentially gone
                // through a combat round at the present location.  Potential sources for such new ships are monsters
                // created via Effect.  (Ships/fleets constructed by empires are currently created at a later stage of
                // turn processing, but even if such were moved forward they should be similarly restricted.)  For
                // checks after combat and prior to turn advancement, we check against age zero here.  For checks
                // after turn advancement but prior to combat we check against age 1.  Because the
                // fleets themselves may be created and/or destroyed purely as organizational matters, we check ship
                // age not fleet age.
                int cutoff_age = precombat ? 1 : 0;
                if (fleet_at_war && fleet->MaxShipAgeInTurns() > cutoff_age) {
                    systems_containing_obstructing_objects.insert(system_id);
                    if (fleet->ArrivalStarlane() == system_id)
                        unrestricted_obstruction_systems.insert(system_id);
                }
            }
        }
    }

    //std::stringstream ss;
    //for (int obj_id : systems_containing_obstructing_objects)
    //{ ss << obj_id << ", "; }
    //DebugLogger() << "systems with obstructing objects for empire " << m_id << " : " << ss.str();


    // check each potential supplyable system for whether it can propagate supply.
    for (int sys_id : known_systems) {
        //DebugLogger() << "deciding unobstructedness for system " << sys_id;

        // has empire ever seen this system with partial or better visibility?
        if (!systems_with_at_least_partial_visibility_at_some_point.count(sys_id))
            continue;

        // if system is explored, then whether it can propagate supply depends
        // on what friendly / enemy ships and planets are in the system

        if (unrestricted_friendly_systems.count(sys_id))
            // if there are unrestricted friendly ships, supply can propagate
            m_supply_unobstructed_systems.insert(sys_id);
        else if (systems_containing_friendly_fleets.count(sys_id)) {
            if (!unrestricted_obstruction_systems.count(sys_id))
                // if there are (previously) restricted friendly ships, and no unrestricted enemy fleets, supply can propagate
                m_supply_unobstructed_systems.insert(sys_id);
        } else if (!systems_containing_obstructing_objects.count(sys_id))
            m_supply_unobstructed_systems.insert(sys_id);
        else if (!systems_with_lane_preserving_fleets.count(sys_id)) {
            // otherwise, if system contains no friendly fleets capable of
            // maintaining lane access but does contain an unfriendly fleet,
            // so it is obstructed, so isn't included in the unobstructed
            // systems set.  Furthermore, this empire's available system exit
            // lanes for this system are cleared
            if (!m_preserved_system_exit_lanes[sys_id].empty()) {
                //DebugLogger() << "Empire::UpdateSupplyUnobstructedSystems clearing available lanes for system ("<<sys_id<<"); available lanes were:";
                //for (int system_id : m_preserved_system_exit_lanes[sys_id])
                //    DebugLogger() << "...... "<< system_id;
                m_preserved_system_exit_lanes[sys_id].clear();
            }
        }
    }
}

void Empire::RecordPendingLaneUpdate(int start_system_id, int dest_system_id) {
    if (!m_supply_unobstructed_systems.count(start_system_id))
        m_pending_system_exit_lanes[start_system_id].insert(dest_system_id);
    else { // if the system is unobstructed, mark all its lanes as avilable
        auto system = GetSystem(start_system_id);
        for (const auto& lane : system->StarlanesWormholes()) {
            m_pending_system_exit_lanes[start_system_id].insert(lane.first); // will add both starlanes and wormholes
        }
    }
}

void Empire::UpdatePreservedLanes() {
    for (auto& system : m_pending_system_exit_lanes) {
        m_preserved_system_exit_lanes[system.first].insert(system.second.begin(), system.second.end());
        system.second.clear();
    }
    m_pending_system_exit_lanes.clear(); // TODO: consider: not really necessary, & may be more efficient to not clear.
}

const std::map<int, float>& Empire::SystemSupplyRanges() const
{ return m_supply_system_ranges; }

const std::set<int>& Empire::SupplyUnobstructedSystems() const
{ return m_supply_unobstructed_systems; }

const bool Empire::PreservedLaneTravel(int start_system_id, int dest_system_id) const {
    auto find_it = m_preserved_system_exit_lanes.find(start_system_id);
    return find_it != m_preserved_system_exit_lanes.end()
            && find_it->second.count(dest_system_id);
}

const std::set<int>& Empire::ExploredSystems() const
{ return m_explored_systems; }

const std::map<int, std::set<int>> Empire::KnownStarlanes() const {
    // compile starlanes leading into or out of each system
    std::map<int, std::set<int>> retval;

    const Universe& universe = GetUniverse();

    const std::set<int>& known_destroyed_objects = universe.EmpireKnownDestroyedObjectIDs(this->EmpireID());
    for (auto sys_it = Objects().const_begin<System>();
         sys_it != Objects().const_end<System>(); ++sys_it)
    {
        int start_id = sys_it->ID();

        // exclude lanes starting at systems known to be destroyed
        if (known_destroyed_objects.count(start_id))
            continue;

        for (const auto& lane : sys_it->StarlanesWormholes()) {
            if (lane.second || known_destroyed_objects.count(lane.second))
                continue;   // is a wormhole, not a starlane, or is connected to a known destroyed system
            int end_id = lane.first;
            retval[start_id].insert(end_id);
            retval[end_id].insert(start_id);
        }
    }

    return retval;
}

const std::map<int, std::set<int>> Empire::VisibleStarlanes() const {
    std::map<int, std::set<int>> retval;   // compile starlanes leading into or out of each system

    const Universe& universe = GetUniverse();
    const ObjectMap& objects = universe.Objects();

    for (auto sys_it = objects.const_begin<System>();
         sys_it != objects.const_end<System>(); ++sys_it)
    {
        int start_id = sys_it->ID();

        // is system visible to this empire?
        if (universe.GetObjectVisibilityByEmpire(start_id, m_id) <= VIS_NO_VISIBILITY)
            continue;

        // get system's visible lanes for this empire
        for (auto& lane : sys_it->VisibleStarlanesWormholes(m_id)) {
            if (lane.second)
                continue;   // is a wormhole, not a starlane
            int end_id = lane.first;
            retval[start_id].insert(end_id);
            retval[end_id].insert(start_id);
        }
    }

    return retval;
}

Empire::SitRepItr Empire::SitRepBegin() const
{ return m_sitrep_entries.begin(); }

Empire::SitRepItr Empire::SitRepEnd() const
{ return m_sitrep_entries.end(); }

float Empire::ProductionPoints() const
{ return GetResourcePool(RE_INDUSTRY)->TotalOutput(); }

const std::shared_ptr<ResourcePool> Empire::GetResourcePool(ResourceType resource_type) const {
    auto it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        return nullptr;
    return it->second;
}

float Empire::ResourceStockpile(ResourceType type) const {
    auto it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceStockpile passed invalid ResourceType");
    return it->second->Stockpile();
}

float Empire::ResourceOutput(ResourceType type) const {
    auto it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceOutput passed invalid ResourceType");
    return it->second->TotalOutput();
}

float Empire::ResourceAvailable(ResourceType type) const {
    auto it = m_resource_pools.find(type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::ResourceAvailable passed invalid ResourceType");
    return it->second->TotalAvailable();
}

const PopulationPool& Empire::GetPopulationPool() const
{ return m_population_pool; }

float Empire::Population() const
{ return m_population_pool.Population(); }

void Empire::SetResourceStockpile(ResourceType resource_type, float stockpile) {
    auto it = m_resource_pools.find(resource_type);
    if (it == m_resource_pools.end())
        throw std::invalid_argument("Empire::SetResourceStockpile passed invalid ResourceType");
    return it->second->SetStockpile(stockpile);
}

void Empire::PlaceTechInQueue(const std::string& name, int pos/* = -1*/) {
    if (name.empty() || TechResearched(name) || m_techs.count(name))
        return;
    const Tech* tech = GetTech(name);
    if (!tech || !tech->Researchable())
        return;

    auto it = m_research_queue.find(name);

    if (pos < 0 || static_cast<int>(m_research_queue.size()) <= pos) {
        // default to putting at end
        bool paused = false;
        if (it != m_research_queue.end()) {
            paused = it->paused;
            m_research_queue.erase(it);
        }
        m_research_queue.push_back(name, paused);
    } else {
        // put at requested position
        if (it < m_research_queue.begin() + pos)
            --pos;
        bool paused = false;
        if (it != m_research_queue.end()) {
            paused = it->paused;
            m_research_queue.erase(it);
        }
        m_research_queue.insert(m_research_queue.begin() + pos, name, paused);
    }
}

void Empire::RemoveTechFromQueue(const std::string& name) {
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        m_research_queue.erase(it);
}

void Empire::PauseResearch(const std::string& name) {
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        it->paused = true;
}

void Empire::ResumeResearch(const std::string& name){
    auto it = m_research_queue.find(name);
    if (it != m_research_queue.end())
        it->paused = false;
}

void Empire::SetTechResearchProgress(const std::string& name, float progress) {
    const Tech* tech = GetTech(name);
    if (!tech) {
        ErrorLogger() << "Empire::SetTechResearchProgress no such tech as: " << name;
        return;
    }
    if (TechResearched(name))
        return; // can't affect already-researched tech

    // set progress
    float clamped_progress = std::min(1.0f, std::max(0.0f, progress));
    m_research_progress[name] = clamped_progress;

    // if tech is complete, ensure it is on the queue, so it will be researched next turn
    if (clamped_progress >= tech->ResearchCost(m_id) &&
            !m_research_queue.InQueue(name))
        m_research_queue.push_back(name);

    // don't just give tech to empire, as another effect might reduce its progress before end of turn
}

const unsigned int MAX_PROD_QUEUE_SIZE = 500;

void Empire::PlaceProductionOnQueue(BuildType build_type, const std::string& name, int number,
                                    int blocksize, int location, int pos/* = -1*/)
{
    if (!EnqueuableItem(build_type, name, location)) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Attempted to place non-enqueuable item in queue: build_type: "
                      << boost::lexical_cast<std::string>(build_type) << "  name: " << name << "  location: " << location;
        return;
    }

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Maximum queue size reached. Aborting enqueue";
        return;
    }

    if (!ProducibleItem(build_type, name, location)) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Placed a non-buildable item in queue: build_type: "
                      << boost::lexical_cast<std::string>(build_type) << "  name: " << name << "  location: " << location;
        return;
    }

    ProductionQueue::Element build(build_type, name, m_id, number, number, blocksize, location);

    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos)
        m_production_queue.push_back(build);
    else
        m_production_queue.insert(m_production_queue.begin() + pos, build);
}

void Empire::PlaceProductionOnQueue(BuildType build_type, BuildType dummy, int number,
                                    int blocksize, int location, int pos/* = -1*/)
{
    // no distinction between enqueuable and producible...

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Maximum queue size reached. Aborting enqueue";
        return;
    }

    if (!ProducibleItem(build_type, location)) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Placed a non-buildable item in queue: build_type: "
                      << boost::lexical_cast<std::string>(build_type) << "  location: " << location;
        return;
    }

    const bool paused = false;
    const bool allowed_imperial_stockpile_use = false;
    const std::string name = "PROJECT_BT_STOCKPILE";
    ProductionQueue::Element build(build_type, name, m_id, number, number, blocksize,
                                   location, paused, allowed_imperial_stockpile_use);

    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos)
        m_production_queue.push_back(build);
    else
        m_production_queue.insert(m_production_queue.begin() + pos, build);
}

void Empire::PlaceProductionOnQueue(BuildType build_type, int design_id, int number,
                                    int blocksize, int location, int pos/* = -1*/)
{
    // ship designs don't have a distinction between enqueuable and producible...

    if (m_production_queue.size() >= MAX_PROD_QUEUE_SIZE) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Maximum queue size reached. Aborting enqueue";
        return;
    }

    if (!ProducibleItem(build_type, design_id, location)) {
        ErrorLogger() << "Empire::PlaceProductionOnQueue() : Placed a non-buildable item in queue: build_type: "
                      << boost::lexical_cast<std::string>(build_type) << "  design_id: " << design_id << "  location: " << location;
        return;
    }

    ProductionQueue::Element elem(build_type, design_id, m_id, number, number, blocksize, location);
    if (pos < 0 || static_cast<int>(m_production_queue.size()) <= pos)
        m_production_queue.push_back(elem);
    else
        m_production_queue.insert(m_production_queue.begin() + pos, elem);
}

void Empire::PlaceProductionOnQueue(const ProductionQueue::ProductionItem& item, int number,
                                    int blocksize, int location, int pos/* = -1*/)
{
    if (item.build_type == BT_BUILDING)
        PlaceProductionOnQueue(item.build_type, item.name, number, blocksize, location, pos);
    else if (item.build_type == BT_SHIP)
        PlaceProductionOnQueue(item.build_type, item.design_id, number, blocksize, location, pos);
    else if (item.build_type == BT_STOCKPILE)
        PlaceProductionOnQueue(item.build_type, item.build_type, number, blocksize, location, pos);
    else
        throw std::invalid_argument("Empire::PlaceProductionOnQueue was passed a ProductionQueue::ProductionItem with an invalid BuildType");
}

void Empire::SetProductionQuantityAndBlocksize(int index, int quantity, int blocksize) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    DebugLogger() << "Empire::SetProductionQuantityAndBlocksize() called for item "<< m_production_queue[index].item.name << "with new quant " << quantity << " and new blocksize " << blocksize;
    if (quantity < 1)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BT_BUILDING && ((1 < quantity) || ( 1 < blocksize) ))
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    //int original_blocksize = m_production_queue[index].blocksize;
    blocksize = std::max(1, blocksize);
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
    m_production_queue[index].blocksize = blocksize;
    //std::cout << "original block size: " << original_blocksize << "  new blocksize: " << blocksize << "  memory blocksize: " << m_production_queue[index].blocksize_memory << std::endl;
    if (blocksize <= m_production_queue[index].blocksize_memory) {
        // if reducing block size, progress on retained portion is unchanged.
        // if increasing block size, progress is proportionally reduced, unless undoing a recent reduction in block size
        m_production_queue[index].progress = m_production_queue[index].progress_memory;
    } else {
        m_production_queue[index].progress = m_production_queue[index].progress_memory * m_production_queue[index].blocksize_memory / blocksize;
    }
}

void Empire::SplitIncompleteProductionItem(int index) {
    DebugLogger() << "Empire::SplitIncompleteProductionItem() called for index " << index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SplitIncompleteProductionItem() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (m_production_queue[index].item.build_type == BT_BUILDING)
        throw std::runtime_error("Empire::SplitIncompleteProductionItem() : Attempted to split a production item that is not a ship.");

    ProductionQueue::Element& elem = m_production_queue[index];

    // if "splitting" an item with just 1 remaining, do nothing
    if (elem.remaining <= 1)
        return;

    // add duplicate
    int new_item_quantity = elem.remaining - 1;
    elem.remaining = 1; // reduce remaining on specified to 1
    PlaceProductionOnQueue(elem.item, new_item_quantity, elem.blocksize, elem.location, index + 1);
}

void Empire::DuplicateProductionItem(int index) {
    DebugLogger() << "Empire::DuplicateProductionItem() called for index " << index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::DuplicateProductionItem() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");

    auto& elem = m_production_queue[index];
    PlaceProductionOnQueue(elem.item, elem.remaining, elem.blocksize, elem.location, index + 1);
}

void Empire::SetProductionRallyPoint(int index, int rally_point_id) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    m_production_queue[index].rally_point_id = rally_point_id;
}

void Empire::SetProductionQuantity(int index, int quantity) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to adjust the quantity of items to be built in a nonexistent production queue item.");
    if (quantity < 1)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to set the quantity of a build run to a value less than zero.");
    if (m_production_queue[index].item.build_type == BT_BUILDING && 1 < quantity)
        throw std::runtime_error("Empire::SetProductionQuantity() : Attempted to build more than one instance of a building in the same build run.");
    int original_quantity = m_production_queue[index].remaining;
    m_production_queue[index].remaining = quantity;
    m_production_queue[index].ordered += quantity - original_quantity;
}

void Empire::MoveProductionWithinQueue(int index, int new_index) {
    if (index < new_index)
        --new_index;
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index ||
        new_index < 0 || static_cast<int>(m_production_queue.size()) <= new_index)
    {
        DebugLogger() << "Empire::MoveProductionWithinQueue index: " << index << "  new index: "
                               << new_index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted to move a production queue item to or from an invalid index.";
        return;
    }
    auto build = m_production_queue[index];
    m_production_queue.erase(index);
    m_production_queue.insert(m_production_queue.begin() + new_index, build);
}

void Empire::RemoveProductionFromQueue(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::RemoveProductionFromQueue index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted to delete a production queue item with an invalid index.";
        return;
    }
    m_production_queue.erase(index);
}

void Empire::PauseProduction(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::PauseProduction index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted pause a production queue item with an invalid index.";
        return;
    }
    m_production_queue[index].paused = true;
}

void Empire::ResumeProduction(int index) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::ResumeProduction index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted resume a production queue item with an invalid index.";
        return;
    }
    m_production_queue[index].paused = false;
}

void Empire::AllowUseImperialPP(int index, bool allow /*=true*/) {
    if (index < 0 || static_cast<int>(m_production_queue.size()) <= index) {
        DebugLogger() << "Empire::AllowUseImperialPP index: " << index << "  queue size: " << m_production_queue.size();
        ErrorLogger() << "Attempted allow/disallow use of the imperial PP stockpile for a production queue item with an invalid index.";
        return;
    }
    DebugLogger() << "Empire::AllowUseImperialPP allow: " << allow << "  index: " << index << "  queue size: " << m_production_queue.size();
    m_production_queue[index].allowed_imperial_stockpile_use = allow;
}

void Empire::ConquerProductionQueueItemsAtLocation(int location_id, int empire_id) {
    if (location_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire::ConquerProductionQueueItemsAtLocation: tried to conquer build items located at an invalid location";
        return;
    }

    DebugLogger() << "Empire::ConquerProductionQueueItemsAtLocation: conquering items located at "
                           << location_id << " to empire " << empire_id;

    Empire* to_empire = GetEmpire(empire_id);    // may be null
    if (!to_empire && empire_id != ALL_EMPIRES) {
        ErrorLogger() << "Couldn't get empire with id " << empire_id;
        return;
    }


    for (auto& entry : Empires()) {
        int from_empire_id = entry.first;
        if (from_empire_id == empire_id) continue;    // skip this empire; can't capture one's own ProductionItems

        Empire* from_empire = entry.second;
        ProductionQueue& queue = from_empire->m_production_queue;

        for (auto queue_it = queue.begin(); queue_it != queue.end(); ) {
            auto elem = *queue_it;
            if (elem.location != location_id) {
                ++queue_it;
                continue; // skip projects with wrong location
            }

            ProductionQueue::ProductionItem item = elem.item;

            if (item.build_type == BT_BUILDING) {
                std::string name = item.name;
                const BuildingType* type = GetBuildingType(name);
                if (!type) {
                    ErrorLogger() << "ConquerProductionQueueItemsAtLocation couldn't get building with name " << name;
                    continue;
                }

                CaptureResult result = type->GetCaptureResult(from_empire_id, empire_id, location_id, true);

                if (result == CR_DESTROY) {
                    // item removed from current queue, NOT added to conquerer's queue
                    queue_it = queue.erase(queue_it);

                } else if (result == CR_CAPTURE) {
                    if (to_empire) {
                        // item removed from current queue, added to conquerer's queue
                        ProductionQueue::Element new_elem(item, empire_id, elem.ordered, elem.remaining,
                                                          1, location_id);
                        new_elem.progress = elem.progress;
                        to_empire->m_production_queue.push_back(new_elem);

                        queue_it = queue.erase(queue_it);
                    } else {
                        // else do nothing; no empire can't capure things
                        ++queue_it;
                    }

                } else if (result == INVALID_CAPTURE_RESULT) {
                    ErrorLogger() << "Empire::ConquerBuildsAtLocationFromEmpire: BuildingType had an invalid CaptureResult";
                } else {
                    ++queue_it;
                }
                // otherwise do nothing: item left on current queue, conquerer gets nothing
            } else {
                ++queue_it;
            }

            // TODO: other types of build item...
        }
    }
}

void Empire::AddTech(const std::string& name) {
    const Tech* tech = GetTech(name);
    if (!tech) {
        ErrorLogger() << "Empire::AddTech given and invalid tech: " << name;
        return;
    }

    if (!m_techs.count(name))
        AddSitRepEntry(CreateTechResearchedSitRep(name));

    for (const ItemSpec& item : tech->UnlockedItems())
        UnlockItem(item);  // potential infinite if a tech (in)directly unlocks itself?

    if (!m_techs.count(name))
        m_techs[name] = CurrentTurn();
}

void Empire::UnlockItem(const ItemSpec& item) {
    switch (item.type) {
    case UIT_BUILDING:
        AddBuildingType(item.name);
        break;
    case UIT_SHIP_PART:
        AddPartType(item.name);
        break;
    case UIT_SHIP_HULL:
        AddHullType(item.name);
        break;
    case UIT_SHIP_DESIGN:
        AddShipDesign(GetPredefinedShipDesignManager().GetDesignID(item.name));
        break;
    case UIT_TECH:
        AddTech(item.name);
        break;
    default:
        ErrorLogger() << "Empire::UnlockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::AddBuildingType(const std::string& name) {
    const BuildingType* building_type = GetBuildingType(name);
    if (!building_type) {
        ErrorLogger() << "Empire::AddBuildingType given an invalid building type name: " << name;
        return;
    }
    if (!building_type->Producible())
        return;
    if (m_available_building_types.count(name))
        return;
    m_available_building_types.insert(name);
    AddSitRepEntry(CreateBuildingTypeUnlockedSitRep(name));
}

void Empire::AddPartType(const std::string& name) {
    const PartType* part_type = GetPartType(name);
    if (!part_type) {
        ErrorLogger() << "Empire::AddPartType given an invalid part type name: " << name;
        return;
    }
    if (!part_type->Producible())
        return;
    m_available_part_types.insert(name);
    AddSitRepEntry(CreateShipPartUnlockedSitRep(name));
}

void Empire::AddHullType(const std::string& name) {
    const HullType* hull_type = GetHullType(name);
    if (!hull_type) {
        ErrorLogger() << "Empire::AddHullType given an invalid hull type name: " << name;
        return;
    }
    if (!hull_type->Producible())
        return;
    m_available_hull_types.insert(name);
    AddSitRepEntry(CreateShipHullUnlockedSitRep(name));
}

void Empire::AddExploredSystem(int ID) {
    if (GetSystem(ID))
        m_explored_systems.insert(ID);
    else
        ErrorLogger() << "Empire::AddExploredSystem given an invalid system id: " << ID;
}

std::string Empire::NewShipName() {
    static std::vector<std::string> ship_names = UserStringList("SHIP_NAMES");
    if (ship_names.empty())
        ship_names.push_back(UserString("OBJ_SHIP"));

    // select name randomly from list
    int ship_name_idx = RandSmallInt(0, static_cast<int>(ship_names.size()) - 1);
    std::string retval = ship_names[ship_name_idx];
    int times_name_used = ++m_ship_names_used[retval];
    if (1 < times_name_used)
        retval += " " + RomanNumber(times_name_used);
    return retval;
}

void Empire::AddShipDesign(int ship_design_id, int next_design_id) {
    /* Check if design id is valid.  That is, check that it corresponds to an
     * existing shipdesign in the universe.  On clients, this means that this
     * empire knows about this ship design and the server consequently sent the
     * design to this player.  On the server, all existing ship designs will be
     * valid, so this just adds this design's id to those that this empire will
     * retain as one of it's ship designs, which are those displayed in the GUI
     * list of available designs for human players, and */
    if (ship_design_id == next_design_id)
        return;

    const ShipDesign* ship_design = GetUniverse().GetShipDesign(ship_design_id);
    if (ship_design) {  // don't check if design is producible; adding a ship design is useful for more than just producing it
        // design is valid, so just add the id to empire's set of ids that it knows about
        if (!m_known_ship_designs.count(ship_design_id)) {
            m_known_ship_designs.insert(ship_design_id);

            ShipDesignsChangedSignal();

            TraceLogger() << "AddShipDesign::  " << ship_design->Name() << " (" << ship_design_id
                          << ") to empire #" << EmpireID();
        }
    } else {
        // design in not valid
        ErrorLogger() << "Empire::AddShipDesign(int ship_design_id) was passed a design id that this empire doesn't know about, or that doesn't exist";
    }
}

int Empire::AddShipDesign(ShipDesign* ship_design) {
    Universe& universe = GetUniverse();
    /* check if there already exists this same design in the universe.  On clients, this checks whether this empire
       knows of this exact design and is trying to re-add it.  On the server, this checks whether this exact design
       exists at all yet */
    for (Universe::ship_design_iterator it = universe.beginShipDesigns(); it != universe.endShipDesigns(); ++it) {
        if (ship_design == it->second) {
            // ship design is already present in universe.  just need to add it to the empire's set of ship designs
            int ship_design_id = it->first;
            AddShipDesign(ship_design_id);
            return ship_design_id;
        }
    }

    bool success = universe.InsertShipDesign(ship_design);

    if (!success) {
        ErrorLogger() << "Empire::AddShipDesign Unable to add new design to universe";
        return INVALID_OBJECT_ID;
    }

    auto new_design_id = ship_design->ID();
    AddShipDesign(new_design_id);

    return new_design_id;
}

void Empire::RemoveShipDesign(int ship_design_id) {
    if (m_known_ship_designs.count(ship_design_id)) {
        m_known_ship_designs.erase(ship_design_id);
        ShipDesignsChangedSignal();
    } else {
        DebugLogger() << "Empire::RemoveShipDesign: this empire did not have design with id " << ship_design_id;
    }
}

void Empire::AddSitRepEntry(const SitRepEntry& entry)
{ m_sitrep_entries.push_back(entry); }

void Empire::RemoveTech(const std::string& name)
{ m_techs.erase(name); }

void Empire::LockItem(const ItemSpec& item) {
    switch (item.type) {
    case UIT_BUILDING:
        RemoveBuildingType(item.name);
        break;
    case UIT_SHIP_PART:
        RemovePartType(item.name);
        break;
    case UIT_SHIP_HULL:
        RemoveHullType(item.name);
        break;
    case UIT_SHIP_DESIGN:
        RemoveShipDesign(GetPredefinedShipDesignManager().GetDesignID(item.name));
        break;
    case UIT_TECH:
        RemoveTech(item.name);
        break;
    default:
        ErrorLogger() << "Empire::LockItem : passed ItemSpec with unrecognized UnlockableItemType";
    }
}

void Empire::RemoveBuildingType(const std::string& name) {
    if (!m_available_building_types.count(name))
        DebugLogger() << "Empire::RemoveBuildingType asked to remove building type " << name << " that was no available to this empire";
    m_available_building_types.erase(name);
}

void Empire::RemovePartType(const std::string& name) {
    auto it = m_available_part_types.find(name);
    if (it == m_available_part_types.end())
        DebugLogger() << "Empire::RemovePartType asked to remove part type " << name << " that was no available to this empire";
    m_available_part_types.erase(name);
}

void Empire::RemoveHullType(const std::string& name) {
    auto it = m_available_hull_types.find(name);
    if (it == m_available_hull_types.end())
        DebugLogger() << "Empire::RemoveHullType asked to remove hull type " << name << " that was no available to this empire";
    m_available_hull_types.erase(name);
}

void Empire::ClearSitRep()
{ m_sitrep_entries.clear(); }

namespace {
    // remove nonexistant / invalid techs from queue
    void SanitizeResearchQueue(ResearchQueue& queue) {
        bool done = false;
        while (!done) {
            auto it = queue.begin();
            while (true) {
                if (it == queue.end()) {
                    done = true;        // got all the way through the queue without finding an invalid tech
                    break;
                } else if (!GetTech(it->name)) {
                    DebugLogger() << "SanitizeResearchQueue for empire " << queue.EmpireID() << " removed invalid tech: " << it->name;
                    queue.erase(it);    // remove invalid tech, end inner loop without marking as finished
                    break;
                } else {
                    ++it;               // check next element
                }
            }
        }
    }
}

void Empire::CheckResearchProgress() {
    SanitizeResearchQueue(m_research_queue);

    float spent_rp{0.0f};
    float total_rp_available = m_resource_pools[RE_RESEARCH]->TotalAvailable();

    // process items on queue
    std::vector<std::string> to_erase;
    for (auto& elem : m_research_queue) {
        const Tech* tech = GetTech(elem.name);
        if (!tech) {
            ErrorLogger() << "Empire::CheckResearchProgress couldn't find tech on queue, even after sanitizing!";
            continue;
        }
        float& progress = m_research_progress[elem.name];
        float tech_cost = tech->ResearchCost(m_id);
        progress += elem.allocated_rp / std::max(EPSILON, tech_cost);
        spent_rp += elem.allocated_rp;
        if (tech->ResearchCost(m_id) - EPSILON <= progress * tech_cost)
            AddTech(elem.name);
        if (GetTechStatus(elem.name) == TS_COMPLETE) {
            m_research_progress.erase(elem.name);
            to_erase.push_back(elem.name);
        }
    }

    //DebugLogger() << m_research_queue.Dump();
    float rp_left_to_spend = total_rp_available - spent_rp;
    //DebugLogger() << "leftover RP: " << rp_left_to_spend;
    // auto-allocate any excess RP left over after player-specified queued techs

    // if there are left over RPs, any tech on the queue presumably can't
    // have RP allocated to it
    std::unordered_set<std::string> techs_not_suitable_for_auto_allocation;
    for (auto& elem : m_research_queue)
        techs_not_suitable_for_auto_allocation.insert(elem.name);

    // for all available and suitable techs, store ordered by cost to complete
    std::multimap<double, std::string> costs_to_complete_available_unpaused_techs;
    for (const auto& tech : GetTechManager()) {
        const std::string& tech_name = tech->Name();
        if (techs_not_suitable_for_auto_allocation.count(tech_name) > 0)
            continue;
        if (this->GetTechStatus(tech_name) != TS_RESEARCHABLE)
            continue;
        if (!tech->Researchable())
            continue;
        double progress = this->ResearchProgress(tech_name);
        double total_cost = tech->ResearchCost(m_id);
        if (progress >= total_cost)
            continue;
        costs_to_complete_available_unpaused_techs.emplace(total_cost - progress, tech_name);
    }

    // in order of minimum additional cost to complete, allocate RP to
    // techs up to available RP and per-turn limits
    for (auto const& cost_tech : costs_to_complete_available_unpaused_techs) {
        if (rp_left_to_spend <= EPSILON)
            break;

        const Tech* tech = GetTech(cost_tech.second);
        if (!tech)
            continue;

        //DebugLogger() << "extra tech: " << cost_tech.second << " needs: " << cost_tech.first << " more RP to finish";

        float RPs_per_turn_limit = tech->PerTurnCost(m_id);
        float tech_total_cost = tech->ResearchCost(m_id);
        float progress_fraction = m_research_progress[cost_tech.second];

        float progress_fraction_left = 1.0f - progress_fraction;
        float max_progress_per_turn = RPs_per_turn_limit / tech_total_cost;
        float progress_possible_with_available_rp = rp_left_to_spend / tech_total_cost;

        //DebugLogger() << "... progress left: " << progress_fraction_left
        //              << " max per turn: " << max_progress_per_turn
        //              << " progress possible with available rp: " << progress_possible_with_available_rp;

        float progress_increase = std::min(
            progress_fraction_left,
            std::min(max_progress_per_turn, progress_possible_with_available_rp));

        float consumed_rp = progress_increase * tech_total_cost;

        m_research_progress[cost_tech.second] += progress_increase;
        rp_left_to_spend -= consumed_rp;

        if (tech->ResearchCost(m_id) - EPSILON <= m_research_progress[cost_tech.second] * tech_total_cost)
            AddTech(cost_tech.second);

        //DebugLogger() << "... allocated: " << consumed_rp << " to increase progress by: " << progress_increase;
    }

    // remove completed items from queue (after consuming extra RP, as that
    // determination uses the contents of the queue as input)
    for (const std::string& tech_name : to_erase) {
        auto temp_it = m_research_queue.find(tech_name);
        if (temp_it != m_research_queue.end())
            m_research_queue.erase(temp_it);
    }

    // can uncomment following line when / if research stockpiling is enabled...
    // m_resource_pools[RE_RESEARCH]->SetStockpile(m_resource_pools[RE_RESEARCH]->TotalAvailable() - m_research_queue.TotalRPsSpent());
}

void Empire::CheckProductionProgress() {
    DebugLogger() << "========Empire::CheckProductionProgress=======";
    // following commented line should be redundant, as previous call to
    // UpdateResourcePools should have generated necessary info
    // m_production_queue.Update();

    Universe& universe = GetUniverse();

    std::map<int, std::vector<std::shared_ptr<Ship>>> system_new_ships;
    std::map<int, int> new_ship_rally_point_ids;

    // preprocess the queue to get all the costs and times of all items
    // at every location at which they are being produced,
    // before doing any generation of new objects or other modifications
    // of the gamestate. this will ensure that the cost of items doesn't
    // change while the queue is being processed, so that if there is
    // sufficent PP to complete an object at the start of a turn,
    // items above it on the queue getting finished don't increase the
    // cost and result in it not being finished that turn.
    std::map<std::pair<ProductionQueue::ProductionItem, int>, std::pair<float, int>>
        queue_item_costs_and_times;
    for (auto& elem : m_production_queue) {
        // for items that don't depend on location, only store cost/time once
        int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
        auto key = std::make_pair(elem.item, location_id);

        if (!queue_item_costs_and_times.count(key))
            queue_item_costs_and_times[key] = ProductionCostAndTime(elem);
    }

    //for (auto& entry : queue_item_costs_and_times)
    //{ DebugLogger() << entry.first.first.design_id << " : " << entry.second.first; }


    // go through queue, updating production progress.  If a production item is
    // completed, create the produced object or take whatever other action is
    // appropriate, and record that queue item as complete, so it can be erased
    // from the queue
    std::vector<int> to_erase;
    for (unsigned int i = 0; i < m_production_queue.size(); ++i) {
        auto& elem = m_production_queue[i];
        float item_cost;
        int build_turns;

        // for items that don't depend on location, only store cost/time once
        int location_id = (elem.item.CostIsProductionLocationInvariant() ? INVALID_OBJECT_ID : elem.location);
        std::pair<ProductionQueue::ProductionItem, int> key(elem.item, location_id);

        std::tie(item_cost, build_turns) = queue_item_costs_and_times[key];
        if (item_cost < 0.01f || build_turns < 1) {
            ErrorLogger() << "Empire::CheckProductionProgress got strang cost/time: " << item_cost << " / " << build_turns;
            break;
        }

        item_cost *= elem.blocksize;

        DebugLogger() << "elem: " << elem.Dump();
        DebugLogger() << "   allocated: " << elem.allocated_pp;
        DebugLogger() << "   initial progress: " << elem.progress;

        elem.progress += elem.allocated_pp / std::max(EPSILON, item_cost);  // add progress for allocated PP to queue item
        elem.progress_memory = elem.progress;
        elem.blocksize_memory = elem.blocksize;

        DebugLogger() << "   updated progress: " << elem.progress;
        DebugLogger() << " ";

        std::string build_description;
        switch (elem.item.build_type) {
            case BT_BUILDING: {
                build_description = "BuildingType " + elem.item.name;
                break;
            }
            case BT_SHIP: {
                build_description = "Ships(s) with design id " + std::to_string(elem.item.design_id);
                break;
            }
            case BT_STOCKPILE: {
                build_description = "Stockpile PP transfer";
                break;
            }
            default:
                build_description = "unknown build type";
        }

        auto build_location = GetUniverseObject(elem.location);
        if (!build_location || (elem.item.build_type == BT_BUILDING && build_location->ObjectType() != OBJ_PLANET)) {
            ErrorLogger() << "Couldn't get valid build location for completed " << build_description;
            continue;
        }
        auto system = GetSystem(build_location->SystemID());
        // TODO: account for shipyards and/or other ship production
        // sites that are in interstellar space, if needed
        if (!system) {
            ErrorLogger() << "Empire::CheckProductionProgress couldn't get system for producing new " << build_description;
            continue;
        }

        // check location condition before each item is created, so
        // that items being produced can prevent subsequent
        // completions on the same turn from going through
        if (!this->ProducibleItem(elem.item, elem.location)) {
            DebugLogger() << "Location test failed for " << build_description << " at location " << build_location->Name();
            continue;
        }


        // only if accumulated PP is sufficient, the item can be completed
        if (item_cost - EPSILON > elem.progress*item_cost)
            continue;


        // only if consumed resources are available, then item can be completd
        bool consumption_impossible = false;
        std::map<std::string, std::map<int, float>> sc = elem.item.CompletionSpecialConsumption(elem.location);
        for (auto& special_type : sc) {
            if (consumption_impossible)
                break;
            for (auto& special_meter : special_type.second) {
                auto obj = GetUniverseObject(special_meter.first);
                float capacity = obj ? obj->SpecialCapacity(special_type.first) : 0.0f;
                if (capacity < special_meter.second * elem.blocksize) {
                    consumption_impossible = true;
                    break;
                }
            }
        }
        auto mc = elem.item.CompletionMeterConsumption(elem.location);
        for (auto& meter_type : mc) {
            if (consumption_impossible)
                break;
            for (auto& object_meter : meter_type.second) {
                auto obj = GetUniverseObject(object_meter.first);
                const Meter* meter = obj ? obj->GetMeter(meter_type.first) : nullptr;
                if (!meter || meter->Current() < object_meter.second * elem.blocksize) {
                    consumption_impossible = true;
                    break;
                }
            }
        }
        if (consumption_impossible)
            continue;


        // deduct progress for complete item from accumulated progress, so that next
        // repetition can continue accumulating PP, but don't set progress to 0, as
        // this way overflow progress / PP allocated this turn can be used for the
        // next repetition of the item.
        elem.progress -= 1.0f;
        if (elem.progress < 0.0f) {
            if (elem.progress < -1e-3)
                ErrorLogger() << "Somehow got negative progress (" << elem.progress
                              << ") after deducting progress for completed item...";
            elem.progress = 0.0f;
        }

        elem.progress_memory = elem.progress;
        DebugLogger() << "Completed an item: " << elem.item.name;


        // consume the item's special and meter consumption
        for (auto& special_type : sc) {
            for (auto& special_meter : special_type.second) {
                auto obj = GetUniverseObject(special_meter.first);
                if (!obj)
                    continue;
                if (!obj->HasSpecial(special_type.first))
                    continue;
                float cur_capacity = obj->SpecialCapacity(special_type.first);
                float new_capacity = std::max(0.0f, cur_capacity - special_meter.second * elem.blocksize);
                obj->SetSpecialCapacity(special_type.first, new_capacity);
            }
        }
        for (auto& meter_type : mc) {
            for (const auto& object_meter : meter_type.second) {
                auto obj = GetUniverseObject(object_meter.first);
                if (!obj)
                    continue;
                Meter*meter = obj->GetMeter(meter_type.first);
                if (!meter)
                    continue;
                float cur_meter = meter->Current();
                float new_meter = cur_meter - object_meter.second * elem.blocksize;
                meter->SetCurrent(new_meter);
                meter->BackPropagate();
            }
        }


        // create actual thing(s) being produced
        switch (elem.item.build_type) {
        case BT_BUILDING: {
            auto planet = GetPlanet(elem.location);

            // create new building
            auto building = universe.InsertNew<Building>(m_id, elem.item.name, m_id);
            planet->AddBuilding(building->ID());
            building->SetPlanetID(planet->ID());
            system->Insert(building);

            // record building production in empire stats
            if (m_building_types_produced.count(elem.item.name))
                m_building_types_produced[elem.item.name]++;
            else
                m_building_types_produced[elem.item.name] = 1;

            AddSitRepEntry(CreateBuildingBuiltSitRep(building->ID(), planet->ID()));
            DebugLogger() << "New Building created on turn: " << CurrentTurn();
            break;
        }

        case BT_SHIP: {
            if (elem.blocksize < 1)
                break;   // nothing to do!

            // get species for this ship.  use popcenter species if build
            // location is a popcenter, or use ship species if build
            // location is a ship, or use empire capital species if there
            // is a valid capital, or otherwise ???
            // TODO: Add more fallbacks if necessary
            std::string species_name;
            if (auto location_pop_center = std::dynamic_pointer_cast<const PopCenter>(build_location))
                species_name = location_pop_center->SpeciesName();
            else if (auto location_ship = std::dynamic_pointer_cast<const Ship>(build_location))
                species_name = location_ship->SpeciesName();
            else if (auto capital_planet = GetPlanet(this->CapitalID()))
                species_name = capital_planet->SpeciesName();
            // else give up...
            if (species_name.empty()) {
                // only really a problem for colony ships, which need to have a species to function
                const auto* design = GetShipDesign(elem.item.design_id);
                if (!design) {
                    ErrorLogger() << "Couldn't get ShipDesign with id: " << elem.item.design_id;
                    break;
                }
                if (design->CanColonize()) {
                    ErrorLogger() << "Couldn't get species in order to make colony ship!";
                    break;
                }
            }

            std::shared_ptr<Ship> ship;

            for (int count = 0; count < elem.blocksize; count++) {
                // create ship
                ship = universe.InsertNew<Ship>(m_id, elem.item.design_id, species_name, m_id);
                system->Insert(ship);

                // record ship production in empire stats
                if (m_ship_designs_produced.count(elem.item.design_id))
                    m_ship_designs_produced[elem.item.design_id]++;
                else
                    m_ship_designs_produced[elem.item.design_id] = 1;
                if (m_species_ships_produced.count(species_name))
                    m_species_ships_produced[species_name]++;
                else
                    m_species_ships_produced[species_name] = 1;


                // set active meters that have associated max meters to an
                // initial very large value, so that when the active meters are
                // later clamped, they will equal the max meter after effects
                // have been applied, letting new ships start with maxed
                // everything that is traced with an associated max meter.
                ship->SetShipMetersToMax();
                ship->BackPropagateMeters();

                ship->Rename(NewShipName());

                // store ships to put into fleets later
                system_new_ships[system->ID()].push_back(ship);

                // store ship rally points
                if (elem.rally_point_id != INVALID_OBJECT_ID)
                    new_ship_rally_point_ids[ship->ID()] = elem.rally_point_id;
            }
            // add sitrep
            if (elem.blocksize == 1) {
                AddSitRepEntry(CreateShipBuiltSitRep(ship->ID(), system->ID(), ship->DesignID()));
                DebugLogger() << "New Ship, id " << ship->ID() << ", created on turn: " << ship->CreationTurn();
            } else {
                AddSitRepEntry(CreateShipBlockBuiltSitRep(system->ID(), ship->DesignID(), elem.blocksize));
                DebugLogger() << "New block of "<< elem.blocksize << " ships created on turn: " << ship->CreationTurn();
            }
            break;
        }

        case BT_STOCKPILE: {
            DebugLogger() << "Finished a transfer to stockpile";
            break;
        }

        default:
            ErrorLogger() << "Build item of unknown build type finished on production queue.";
            break;
        }

        if (!--m_production_queue[i].remaining) {   // decrement number of remaining items to be produced in current queue element
            to_erase.push_back(i);                  // remember completed element so that it can be removed from queue
            DebugLogger() << "Marking completed production queue item to be removed from queue";
        }
    }

    // create fleets for new ships and put ships into fleets
    for (auto& entry : system_new_ships) {
        auto system = GetSystem(entry.first);
        if (!system) {
            ErrorLogger() << "Couldn't get system with id " << entry.first << " for creating new fleets for newly produced ships";
            continue;
        }

        auto& new_ships = entry.second;
        if (new_ships.empty())
            continue;

        // group ships into fleets by rally point and design
        std::map<int, std::map<int, std::vector<std::shared_ptr<Ship>>>>
            new_ships_by_rally_point_id_and_design_id;
        for (auto& ship : new_ships) {
            int rally_point_id = INVALID_OBJECT_ID;

            auto rally_it = new_ship_rally_point_ids.find(ship->ID());
            if (rally_it != new_ship_rally_point_ids.end())
                rally_point_id = rally_it->second;

            new_ships_by_rally_point_id_and_design_id[rally_point_id][ship->DesignID()].push_back(ship);
        }

        // create fleets for ships with the same rally point, grouped by
        // ship design
        // Do not group unarmed ships with no troops (i.e. scouts and
        // colony ships).
        for (auto& rally_ships : new_ships_by_rally_point_id_and_design_id) {
            int rally_point_id = rally_ships.first;
            auto& new_ships_by_design = rally_ships.second;

            for (auto& ships_by_design : new_ships_by_design) {
                std::vector<int> ship_ids;

                auto& ships = ships_by_design.second;
                if (ships.empty())
                    continue;

                // create a single fleet for combat ships and individual
                // fleets for non-combat ships
                bool individual_fleets = !((*ships.begin())->IsArmed()
                                           || (*ships.begin())->HasFighters()
                                           || (*ships.begin())->CanHaveTroops()
                                           || (*ships.begin())->CanBombard());

                std::vector<std::shared_ptr<Fleet>> fleets;
                std::shared_ptr<Fleet> fleet;

                if (!individual_fleets) {
                    fleet = universe.InsertNew<Fleet>("", system->X(), system->Y(), m_id);

                    system->Insert(fleet);
                    // set prev system to prevent conflicts with CalculateRouteTo used for
                    // rally points below, but leave next system as INVALID_OBJECT_ID so
                    // fleet won't necessarily be disqualified from making blockades if it
                    // is left stationary
                    fleet->SetNextAndPreviousSystems(INVALID_OBJECT_ID, system->ID());
                    // set invalid arrival starlane so that fleet won't necessarily be free from blockades
                    fleet->SetArrivalStarlane(INVALID_OBJECT_ID);

                    fleets.push_back(fleet);
                }

                for (auto& ship : ships) {
                    if (individual_fleets) {
                        fleet = universe.InsertNew<Fleet>("", system->X(), system->Y(), m_id);

                        system->Insert(fleet);
                        // set prev system to prevent conflicts with CalculateRouteTo used for
                        // rally points below, but leave next system as INVALID_OBJECT_ID so
                        // fleet won't necessarily be disqualified from making blockades if it
                        // is left stationary
                        fleet->SetNextAndPreviousSystems(INVALID_OBJECT_ID, system->ID());
                        // set invalid arrival starlane so that fleet won't necessarily be free from blockades
                        fleet->SetArrivalStarlane(INVALID_OBJECT_ID);

                        fleets.push_back(fleet);
                    }
                    ship_ids.push_back(ship->ID());
                    fleet->AddShips({ship->ID()});
                    ship->SetFleetID(fleet->ID());
                }

                for (auto& next_fleet : fleets) {
                    // rename fleet, given its id and the ship that is in it
                    next_fleet->Rename(next_fleet->GenerateFleetName());
                    next_fleet->SetAggressive(next_fleet->HasArmedShips() || next_fleet->HasFighterShips());

                    if (rally_point_id != INVALID_OBJECT_ID) {
                        if (GetSystem(rally_point_id)) {
                            next_fleet->CalculateRouteTo(rally_point_id);
                        } else if (auto rally_obj = GetUniverseObject(rally_point_id)) {
                            if (GetSystem(rally_obj->SystemID()))
                                next_fleet->CalculateRouteTo(rally_obj->SystemID());
                        } else {
                            ErrorLogger() << "Unable to find system to route to with rally point id: " << rally_point_id;
                        }
                    }

                    DebugLogger() << "New Fleet \"" << next_fleet->Name()
                                  <<"\" created on turn: " << next_fleet->CreationTurn();
                }
            }
        }
    }

    // removed completed items from queue
    for (auto it = to_erase.rbegin(); it != to_erase.rend(); ++it)
        m_production_queue.erase(*it);

    // update stockpile
    SetResourceStockpile(RE_INDUSTRY, m_production_queue.ExpectedNewStockpileAmount());
}

void Empire::CheckTradeSocialProgress()
{ m_resource_pools[RE_TRADE]->SetStockpile(m_resource_pools[RE_TRADE]->TotalAvailable()); }

void Empire::SetColor(const GG::Clr& color)
{ m_color = color; }

void Empire::SetName(const std::string& name)
{ m_name = name; }

void Empire::SetPlayerName(const std::string& player_name)
{ m_player_name = player_name; }

void Empire::InitResourcePools() {
    // get this empire's owned resource centers and ships (which can both produce resources)
    std::vector<int> res_centers;
    res_centers.reserve(Objects().ExistingResourceCenters().size());
    for (const auto& entry : Objects().ExistingResourceCenters()) {
        if (!entry.second->OwnedBy(m_id))
            continue;
        res_centers.push_back(entry.first);
    }
    for (const auto& entry : Objects().ExistingShips()) {
        if (!entry.second->OwnedBy(m_id))
            continue;
        res_centers.push_back(entry.first);
    }
    m_resource_pools[RE_RESEARCH]->SetObjects(res_centers);
    m_resource_pools[RE_INDUSTRY]->SetObjects(res_centers);
    m_resource_pools[RE_TRADE]->SetObjects(res_centers);

    // get this empire's owned population centers
    std::vector<int> pop_centers;
    pop_centers.reserve(Objects().ExistingPopCenters().size());
    for (const auto& entry : Objects().ExistingPopCenters()) {
        if (entry.second->OwnedBy(m_id))
            pop_centers.push_back(entry.first);
    }
    m_population_pool.SetPopCenters(pop_centers);


    // inform the blockadeable resource pools about systems that can share
    m_resource_pools[RE_INDUSTRY]->SetConnectedSupplyGroups(GetSupplyManager().ResourceSupplyGroups(m_id));

    // set non-blockadeable resource pools to share resources between all systems
    std::set<std::set<int>> sets_set;
    std::set<int> all_systems_set;
    for (const auto& entry : Objects().ExistingSystems()) {
        all_systems_set.insert(entry.first);
    }
    sets_set.insert(all_systems_set);
    m_resource_pools[RE_RESEARCH]->SetConnectedSupplyGroups(sets_set);
    m_resource_pools[RE_TRADE]->SetConnectedSupplyGroups(sets_set);
}

void Empire::UpdateResourcePools() {
    // updating queues, allocated_rp, distribution and growth each update their
    // respective pools, (as well as the ways in which the resources are used,
    // which needs to be done simultaneously to keep things consistent)
    UpdateResearchQueue();
    UpdateProductionQueue();
    UpdateTradeSpending();
    UpdatePopulationGrowth();
}

void Empire::UpdateResearchQueue() {
    m_resource_pools[RE_RESEARCH]->Update();
    m_research_queue.Update(m_resource_pools[RE_RESEARCH]->TotalAvailable(), m_research_progress);
    m_resource_pools[RE_RESEARCH]->ChangedSignal();
}

void Empire::UpdateProductionQueue() {
    DebugLogger() << "========= Production Update for empire: " << EmpireID() << " ========";

    m_resource_pools[RE_INDUSTRY]->Update();
    m_production_queue.Update();
    m_resource_pools[RE_INDUSTRY]->ChangedSignal();
}

void Empire::UpdateTradeSpending() {
    m_resource_pools[RE_TRADE]->Update(); // recalculate total trade production
    m_resource_pools[RE_TRADE]->ChangedSignal();
}

void Empire::UpdatePopulationGrowth()
{ m_population_pool.Update(); }

void Empire::ResetMeters() {
    for (auto& entry : m_meters) {
        entry.second.ResetCurrent();
    }
}

void Empire::UpdateOwnedObjectCounters() {
    // ships of each species and design
    m_species_ships_owned.clear();
    m_ship_designs_owned.clear();
    for (const auto& entry : Objects().ExistingShips()) {
        if (!entry.second->OwnedBy(this->EmpireID()))
            continue;
        auto ship = std::dynamic_pointer_cast<const Ship>(entry.second);
        if (!ship)
            continue;
        if (!ship->SpeciesName().empty())
            m_species_ships_owned[ship->SpeciesName()]++;
        m_ship_designs_owned[ship->DesignID()]++;
    }

    // update ship part counts
    m_ship_part_types_owned.clear();
    m_ship_part_class_owned.clear();
    for (const auto& design_count : m_ship_designs_owned) {
        const ShipDesign* design = GetShipDesign(design_count.first);
        if (!design)
            continue;

        // update count of PartTypes
        for (const auto& part_type : design->PartTypeCount())
            m_ship_part_types_owned[part_type.first] += part_type.second * design_count.second;

        // update count of ShipPartClasses
        for (const auto& part_class : design->PartClassCount())
            m_ship_part_class_owned[part_class.first] += part_class.second * design_count.second;
    }

    // colonies of each species, and unspecified outposts
    m_species_colonies_owned.clear();
    m_outposts_owned = 0;
    for (const auto& entry : Objects().ExistingPlanets()) {
        if (!entry.second->OwnedBy(this->EmpireID()))
            continue;
        auto planet = std::dynamic_pointer_cast<const Planet>(entry.second);
        if (!planet)
            continue;
        if (planet->SpeciesName().empty())
            m_outposts_owned++;
        else
            m_species_colonies_owned[planet->SpeciesName()]++;
    }

    // buildings of each type
    m_building_types_owned.clear();
    for (const auto& entry : Objects().ExistingBuildings()) {
        if (!entry.second->OwnedBy(this->EmpireID()))
            continue;
        auto building = std::dynamic_pointer_cast<const Building>(entry.second);
        if (!building)
            continue;
        m_building_types_owned[building->BuildingTypeName()]++;
    }
}

void Empire::SetAuthenticated(bool authenticated /*= true*/)
{ m_authenticated = authenticated; }

int Empire::TotalShipsOwned() const {
    // sum up counts for each ship design owned by this empire
    // (not using species ship counts, as an empire could potentially own a
    //  ship that has no species...)
    int counter = 0;
    for (const auto& entry : m_ship_designs_owned)
    { counter += entry.second; }
    return counter;
}

int Empire::TotalShipPartsOwned() const {
    // sum counts of all ship parts owned by this empire
    int retval = 0;

    for (const auto& part_class : m_ship_part_class_owned)
        retval += part_class.second;

    return retval;
}

int Empire::TotalBuildingsOwned() const {
    // sum up counts for each building type owned by this empire
    int counter = 0;
    for (const auto& entry : m_building_types_owned)
    { counter += entry.second; }
    return counter;
}
