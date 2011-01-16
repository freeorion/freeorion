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

const std::vector<int>& ResourcePool::ResourceCenterIDs() const {
    return m_resource_center_ids;
}

int ResourcePool::StockpileSystemID() const {
    return m_stockpile_system_id;
}

double ResourcePool::Stockpile() const {
    return m_stockpile;
}

double ResourcePool::Production() const {
    double retval = 0.0;
    for (std::map<std::set<int>, double>::const_iterator it = m_supply_system_groups_resource_production.begin();
         it != m_supply_system_groups_resource_production.end(); ++it)
    {
        retval += it->second;
    }
    return retval;
}

double ResourcePool::GroupProduction(int system_id) const {
    // find group containing specified system
    for (std::map<std::set<int>, double>::const_iterator it = m_supply_system_groups_resource_production.begin();
         it != m_supply_system_groups_resource_production.end(); ++it)
    {
        const std::set<int>& group = it->first;
        if (group.find(system_id) != group.end())
            return it->second;
    }

    // default return case:
    Logger().debugStream() << "ResourcePool::GroupProduction passed unknown system id: " << system_id;
    return 0.0;
}

double ResourcePool::TotalAvailable() const {
    double retval = m_stockpile;
    for (std::map<std::set<int>, double>::const_iterator it = m_supply_system_groups_resource_production.begin(); it != m_supply_system_groups_resource_production.end(); ++it)
        retval += it->second;
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
    // available is stockpile + production in this group

    if (m_stockpile_system_id == UniverseObject::INVALID_OBJECT_ID)
        return GroupProduction(system_id);

    // need to find if stockpile system is in the requested system's group
    for (std::map<std::set<int>, double>::const_iterator it = m_supply_system_groups_resource_production.begin();
         it != m_supply_system_groups_resource_production.end(); ++it)
    {
        const std::set<int>& group = it->first;
        if (group.find(system_id) != group.end()) {
            // found group for requested system.  is stockpile also in this group?
            if (group.find(m_stockpile_system_id) != group.end())
                return it->second + m_stockpile;    // yes; add stockpile to production to return available
            else
                return it->second;                  // no; just return production as available
        }
    }

    // default return case:
    Logger().debugStream() << "ResourcePool::GroupAvailable passed unknown system id: " << system_id;
    return 0.0;
}

std::string ResourcePool::Dump() const {
    std::string retval = "ResourcePool type = " + boost::lexical_cast<std::string>(m_type) +
                         " stockpile = " + boost::lexical_cast<std::string>(m_stockpile) +
                         " stockpile_system_id = " + boost::lexical_cast<std::string>(m_stockpile_system_id) +
                         " resource_center_ids: ";
    for (std::vector<int>::const_iterator it = m_resource_center_ids.begin(); it != m_resource_center_ids.end(); ++it)
        retval += boost::lexical_cast<std::string>(*it) + ", ";
    return retval;
}

void ResourcePool::SetResourceCenters(const std::vector<int>& resource_center_ids) {
    m_resource_center_ids = resource_center_ids;
}

void ResourcePool::SetSystemSupplyGroups(const std::set<std::set<int> >& supply_system_groups) {
    m_supply_system_groups = supply_system_groups;
}

void ResourcePool::SetStockpileSystem(int stockpile_system_id) {
    m_stockpile_system_id = stockpile_system_id;
}

void ResourcePool::SetStockpile(double d) {
    m_stockpile = d;
}

void ResourcePool::Update() {
    //Logger().debugStream() << "ResourcePool::Update for type " << boost::lexical_cast<std::string>(m_type);
    // sum production from all ResourceCenters in each group, for resource point type appropriate for this pool
    MeterType meter_type = ResourceToMeter(m_type);

    if (INVALID_METER_TYPE == meter_type)
        Logger().errorStream() << "ResourcePool::Update() called when m_type can't be converted to a valid MeterType";


    // zero group production
    m_supply_system_groups_resource_production.clear();
    // add a (zero-valued, for now) entry in the map from sets to production
    // amounts for each group.  will accumulate actual production later
    for (std::set<std::set<int> >::const_iterator it = m_supply_system_groups.begin(); it != m_supply_system_groups.end(); ++it)
        m_supply_system_groups_resource_production[*it] = 0.0;


    // find supply system group for each resource center and add production to that group's tally
    for (std::vector<int>::const_iterator res_it = m_resource_center_ids.begin(); res_it != m_resource_center_ids.end(); ++res_it) {
        const UniverseObject* obj = GetObject(*res_it);
        if (!obj) {
            Logger().errorStream() << "ResourcePool::Update couldn't find object / resoure center with id " << *res_it;
            continue;
        }
        int res_center_system_id = obj->SystemID();


        for (std::map<std::set<int>, double>::iterator it = m_supply_system_groups_resource_production.begin();
             it != m_supply_system_groups_resource_production.end(); ++it)
        {
            const std::set<int>& group_ids = it->first;
            if (group_ids.find(res_center_system_id) != group_ids.end()) {
                it->second += obj->CurrentMeterValue(meter_type);
                break;
            }
        }
    }

    ChangedSignal();
}

//////////////////////////////////////////////////
// PopulationPool
//////////////////////////////////////////////////
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

void PopulationPool::SetPopCenters(const std::vector<int>& pop_center_ids) {
    if (m_pop_center_ids == pop_center_ids)
        return;
    m_pop_center_ids = pop_center_ids;
}

void PopulationPool::Update() {
    m_population = 0.0;
    double future_population = 0.0;
    // sum population from all PopCenters in this pool
    for (std::vector<int>::const_iterator it = m_pop_center_ids.begin(); it != m_pop_center_ids.end(); ++it) {
        if (const UniverseObject* obj = GetObject(*it)) {
            if (const PopCenter* center = dynamic_cast<const PopCenter*>(obj)) {
                m_population += center->CurrentMeterValue(METER_POPULATION);
                future_population += center->NextTurnCurrentMeterValue(METER_POPULATION);
            }
        }
    }
    m_growth = future_population - m_population;
    ChangedSignal();
}
