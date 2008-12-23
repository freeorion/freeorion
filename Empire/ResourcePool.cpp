#include "ResourcePool.h"

#include "../universe/Planet.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

//////////////////////////////////////////////////
// ResourcePool
//////////////////////////////////////////////////
ResourcePool::ResourcePool() :
    m_stockpile_system_id(UniverseObject::INVALID_OBJECT_ID),
    m_stockpile(0.0),
    m_type(INVALID_RESOURCE_TYPE)
{}

ResourcePool::ResourcePool(ResourceType type) :
    m_stockpile_system_id(UniverseObject::INVALID_OBJECT_ID),
    m_stockpile(0.0),
    m_type(type)
{}

const std::vector<ResourceCenter*>& ResourcePool::ResourceCenters() const {
    return m_resource_centers;
}

int ResourcePool::StockpileSystemID() const {
    return m_stockpile_system_id;
}

double ResourcePool::Stockpile() const {
    Logger().debugStream() << "ResourcePool::Stockpile returning " << m_stockpile;
    return m_stockpile;
}

double ResourcePool::Production() const {
    double retval = 0.0;
    for (std::map<std::set<int>, double>::const_iterator it = m_supply_system_groups_resource_production.begin(); it != m_supply_system_groups_resource_production.end(); ++it)
        retval += it->second;
    Logger().debugStream() << "ResourcePool::Production returning " << retval;
    return retval;
}

double ResourcePool::Production(int object_id) const {
    const UniverseObject* obj = GetUniverse().Object(object_id);
    if (!obj) {
        Logger().errorStream() << "ResourcePool::Production asked to return resource production for a nonexistant object with id " << object_id;
        // consider throwing exception instead...?
        return 0.0;
    }
    const ResourceCenter* res = dynamic_cast<const ResourceCenter*>(obj);
    if (!res) {
        Logger().errorStream() << "ResourcePool::Production asked to return resource production for an object that is not a ResourceCenter";
        // exception here as well?
        return 0.0;
    }

    // ensure this pool contains the passed resource center
    std::map<const ResourceCenter*, const UniverseObject*>::const_iterator it = m_resource_center_objs.find(res);
    if (it != m_resource_center_objs.end()) {
        MeterType meter_type = ResourceToMeter(m_type);
        return res->ProjectedMeterPoints(meter_type);
    }

    Logger().debugStream() << "ResourcePool::Production asked for the resource production of a ResourceCenter that is not in this pool";
    // thow here also?
    return 0.0;
}

double ResourcePool::GroupProduction(int system_id) const
{
    Logger().debugStream() << "ResourcePool::GroupProdeuction(" << system_id << ")";
    const UniverseObject* sys = GetUniverse().Object(system_id);
    if (!sys) {
        Logger().errorStream() << "ResourcePool::GroupProduction asked to return resource production for a nonexistant object with id " << system_id;
        // consider throwing exception instead...?
        return 0.0;
    }

    for (std::map<std::set<int>, double>::const_iterator it = m_supply_system_groups_resource_production.begin(); it != m_supply_system_groups_resource_production.end(); ++it) {
        const std::set<int>& group = it->first;
        if (group.find(system_id) != group.end())
            return it->second;
    }

    // default return case:
    Logger().debugStream() << "ResourcePool::GroupProduction asked to return resource production for the group of systems containing an object that is not contained within any systems in groups in this ResourcePool";
    return 0.0;
}

double ResourcePool::TotalAvailable() const {
    double retval = m_stockpile;
    for (std::map<std::set<int>, double>::const_iterator it = m_supply_system_groups_resource_production.begin(); it != m_supply_system_groups_resource_production.end(); ++it)
        retval += it->second;
    Logger().debugStream() << "ResourcePool::Available returning " << retval;
    return retval;
}

std::map<std::set<int>, double> ResourcePool::Available() const {
    std::map<std::set<int>, double> retval = m_supply_system_groups_resource_production;

    if (UniverseObject::INVALID_OBJECT_ID == m_stockpile_system_id)
        return retval;  // early exit for no stockpile

    // find group that contains the stockpile, and add the stockpile to that group's production to give its availability
    for (std::map<std::set<int>, double>::iterator map_it = retval.begin(); map_it != retval.end(); ++map_it) {
        const std::set<int>& group = map_it->first;
        if (group.find(m_stockpile_system_id) != group.end()) {
            map_it->second += m_stockpile;
            break;  // assuming stockpile is on only one group
        }
    }

    return retval;
}

double ResourcePool::GroupAvailable(int system_id) const
{
    Logger().debugStream() << "ResourcePool::GroupAvailable(" << system_id << ")";
    // available is stockpile + production in this group.  If system_id is the id of the stockpile
    // system, then add this pool's stockpile to the production of this group and return.
    if (m_stockpile_system_id == system_id)
        return GroupProduction(system_id) + m_stockpile;

    // system_id isn't the stockpile system, so need to do more work...

    // verify system_id is a valid object
    const UniverseObject* sys = GetUniverse().Object(system_id);
    if (!sys) {
        Logger().errorStream() << "ResourcePool::GroupAvailable asked to return available resource for a nonexistant object with id " << system_id;
        // consider throwing exception instead...?
        return 0.0;
    }

    // find group that contains passed system_id
    for (std::map<std::set<int>, double>::const_iterator it = m_supply_system_groups_resource_production.begin(); it != m_supply_system_groups_resource_production.end(); ++it) {
        const std::set<int>& group = it->first;
        if (group.find(system_id) != group.end()) {
            // found group containing system with id system_id.

            // available resource depends on whether this group also contains the stockpile system
            if (group.find(m_stockpile_system_id) != group.end())
                return it->second + m_stockpile;    // available for this group is group's production + stockpile
            else
                return it->second;                  // available for this group is just group's production.  stockpile is held elsewhere
        }
    }

    // default return case:
    Logger().debugStream() << "ResourcePool::GroupAvailable asked to return resource production for the group of systems containing an object that is not contained within any systems in groups in this ResourcePool";
    return 0.0;
}

void ResourcePool::SetResourceCenters(const std::vector<ResourceCenter*>& resource_center_vec) {
    Logger().debugStream() << "ResourcePool::SetResourceCenters for type " << boost::lexical_cast<std::string>(m_type);
    m_resource_centers = resource_center_vec;
    m_resource_center_objs.clear();
    for (std::vector<ResourceCenter*>::const_iterator it = m_resource_centers.begin(); it != m_resource_centers.end(); ++it) {
        ResourceCenter* rc = *it;
        const UniverseObject* obj = dynamic_cast<const UniverseObject*>(rc);
        if (!obj) {
            Logger().errorStream() << "ResourcePool::SetResourceCenters couldn't cast a ResourceCenter to a UniverseObject";
            continue;
        }
        Logger().debugStream() << "... adding object: " << obj->Name() << " (" << obj->ID() << ")";
        m_resource_center_objs[rc] = obj;
    }
}

void ResourcePool::SetSystemSupplyGroups(const std::set<std::set<int> >& supply_system_groups) {
    Logger().debugStream() << "ResourcePool::SetSystemSupplyGroups for type " << boost::lexical_cast<std::string>(m_type);
    m_supply_system_groups_resource_production.clear();         // get rid of old data
    // add a (zero-valued, for now) entry in the map from sets to production amounts for each group.  will accumulate actual production later
    for (std::set<std::set<int> >::const_iterator it = supply_system_groups.begin(); it != supply_system_groups.end(); ++it) {
        Logger().debugStream() << "... group: ";
        for (std::set<int>::const_iterator set_it = it->begin(); set_it != it->end(); ++set_it)
            Logger().debugStream() << "... ... " << *set_it;
        m_supply_system_groups_resource_production[*it] = 0.0;
    }
}

void ResourcePool::SetStockpileSystem(int stockpile_system_id) {
    m_stockpile_system_id = stockpile_system_id;
}

void ResourcePool::SetStockpile(double d) {
    m_stockpile = d;
}

void ResourcePool::Update() {
    Logger().debugStream() << "ResourcePool::Update for type " << boost::lexical_cast<std::string>(m_type);
    // sum production from all ResourceCenters in each group, for resource point type appropriate for this pool
    MeterType meter_type = ResourceToMeter(m_type);

    // for every group...
    for (std::map<std::set<int>, double>::iterator it = m_supply_system_groups_resource_production.begin(); it != m_supply_system_groups_resource_production.end(); ++it) {
        // sum production from from all ResourceCenters in this pool that are located in a system in this group

        it->second = 0.0;                               // zero production for this pool before beginning accumulation
        const std::set<int>& group_ids = it->first;     // extract the set of systems that this group comprises
        double& group_production = it->second;          // to be updated later...

        // loop through systems in group, checking each resource center to see if it is located therein
        for (std::set<int>::const_iterator group_it = group_ids.begin(); group_it != group_ids.end(); ++group_it) {
            int system_in_group_id = *group_it;         // get system to check resource centers against

            //// DEBUG
            //const UniverseObject* system = GetUniverse().Object(system_in_group_id);
            //if (!system)
            //    Logger().errorStream() << "ResourcePool::Update tried to get a system that doesn't exist but is listed in a supply system group";
            //Logger().debugStream() << "... system: " << system->Name() << "(" << system_in_group_id << ")";
            //// END DEBUG

            // check all ResourceCenters in this pool to see if any are in this system.
            for (std::map<const ResourceCenter*, const UniverseObject*>::const_iterator obj_it = m_resource_center_objs.begin(); obj_it != m_resource_center_objs.end(); ++obj_it) {
                if (obj_it->second->SystemID() == system_in_group_id) {
                    // add this ResourceCenter's production to the group pool
                    const ResourceCenter* rc = obj_it->first;
                    group_production += rc->ProjectedMeterPoints(meter_type);
                    Logger().debugStream() << "... ... contributes: " << rc->ProjectedMeterPoints(meter_type);
                    break;  // assuming ResourceCenter are located in only one system
                }
            }
        }
        Logger().debugStream() << "... group production: " << group_production;
    }

    ChangedSignal();
}

//////////////////////////////////////////////////
// PopulationPool
//////////////////////////////////////////////////
namespace {
    bool PopCenterLess(const PopCenter* elem1, const PopCenter* elem2) {
        return elem1->GetMeter(METER_POPULATION)->Current() < elem2->GetMeter(METER_POPULATION)->Current();
    }
}

PopulationPool::PopulationPool() :
    m_population(0.0),
    m_growth(0.0)
{}

double PopulationPool::Population() const {
    return m_population;
}

double PopulationPool::Growth() const {
    return m_growth;
}

void PopulationPool::SetPopCenters(const std::vector<PopCenter*>& pop_center_vec) {
    m_pop_centers = pop_center_vec;
    std::sort(m_pop_centers.begin(), m_pop_centers.end(), &PopCenterLess);  // this ordering ensures higher population PopCenters get first priority for food distribution
}

void PopulationPool::Update() {
    m_population = 0.0;
    double m_future_population = 0.0;
    // sum population from all PopCenters in this pool
    for (std::vector<PopCenter*>::const_iterator it = PopCenters().begin(); it != PopCenters().end(); ++it) {
        const PopCenter* center = (*it);
        const Meter* meter = center->GetMeter(METER_POPULATION);
        m_population += meter->InitialCurrent();
        m_future_population += meter->Current();
    }
    m_growth = m_future_population - m_population;
    ChangedSignal();
}
