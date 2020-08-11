#include "ResourcePool.h"

#include <cassert>
#include <boost/lexical_cast.hpp>
#include "../universe/Enums.h"
#include "../universe/ObjectMap.h"
#include "../universe/Planet.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"

MeterType ResourceToMeter(ResourceType type) {
    switch (type) {
    case ResourceType::RE_INDUSTRY:  return MeterType::METER_INDUSTRY;
    case ResourceType::RE_RESEARCH:  return MeterType::METER_RESEARCH;
    case ResourceType::RE_INFLUENCE: return MeterType::METER_INFLUENCE;
    case ResourceType::RE_STOCKPILE: return MeterType::METER_STOCKPILE;
    default:
        assert(0);
        return MeterType::INVALID_METER_TYPE;
        break;
    }
}

MeterType ResourceToTargetMeter(ResourceType type) {
    switch (type) {
    case ResourceType::RE_INDUSTRY:  return MeterType::METER_TARGET_INDUSTRY;
    case ResourceType::RE_RESEARCH:  return MeterType::METER_TARGET_RESEARCH;
    case ResourceType::RE_INFLUENCE: return MeterType::METER_TARGET_INFLUENCE;
    case ResourceType::RE_STOCKPILE: return MeterType::METER_MAX_STOCKPILE;
    default:
        assert(0);
        return MeterType::INVALID_METER_TYPE;
        break;
    }
}

ResourceType MeterToResource(MeterType type) {
    switch (type) {
    case MeterType::METER_INDUSTRY:  return ResourceType::RE_INDUSTRY;
    case MeterType::METER_RESEARCH:  return ResourceType::RE_RESEARCH;
    case MeterType::METER_INFLUENCE: return ResourceType::RE_INFLUENCE;
    case MeterType::METER_STOCKPILE: return ResourceType::RE_STOCKPILE;
    default:
        assert(0);
        return ResourceType::INVALID_RESOURCE_TYPE;
        break;
    }
}

//////////////////////////////////////////////////
// ResourcePool
//////////////////////////////////////////////////
ResourcePool::ResourcePool() :
    m_type(ResourceType::INVALID_RESOURCE_TYPE)
{}

ResourcePool::ResourcePool(ResourceType type) :
    m_type(type)
{}

const std::vector<int>& ResourcePool::ObjectIDs() const
{ return m_object_ids; }

float ResourcePool::Stockpile() const
{ return m_stockpile; }

float ResourcePool::TotalOutput() const {
    float retval = 0.0f;
    for (const auto& entry : m_connected_object_groups_resource_output)
    { retval += entry.second; }
    return retval;
}

std::map<std::set<int>, float> ResourcePool::Output() const
{ return m_connected_object_groups_resource_output; }

float ResourcePool::GroupOutput(int object_id) const {
    // find group containing specified object
    for (const auto& entry : m_connected_object_groups_resource_output) {
        if (entry.first.count(object_id))
            return entry.second;
    }

    // default return case:
    //DebugLogger() << "ResourcePool::GroupOutput passed unknown object id: " << object_id;
    return 0.0f;
}

float ResourcePool::TargetOutput() const {
    float retval = 0.0f;
    for (const auto& entry : m_connected_object_groups_resource_target_output)
    { retval += entry.second; }
    return retval;
}

float ResourcePool::GroupTargetOutput(int object_id) const {
    // find group containing specified object
    for (const auto& entry : m_connected_object_groups_resource_target_output) {
        if (entry.first.count(object_id))
            return entry.second;
    }

    // default return case:
    DebugLogger() << "ResourcePool::GroupTargetOutput passed unknown object id: " << object_id;
    return 0.0f;
}

float ResourcePool::TotalAvailable() const {
    float retval = m_stockpile;
    for (const auto& entry : m_connected_object_groups_resource_output)
    { retval += entry.second; }
    return retval;
}

std::map<std::set<int>, float> ResourcePool::Available() const {
    auto retval = m_connected_object_groups_resource_output;
    return retval;
}

float ResourcePool::GroupAvailable(int object_id) const {
    TraceLogger() << "ResourcePool::GroupAvailable(" << object_id << ")";
    // available is production in this group
    return GroupOutput(object_id);
}

std::string ResourcePool::Dump() const {
    std::string retval = "ResourcePool type = " + boost::lexical_cast<std::string>(m_type) +
                         " stockpile = " + std::to_string(m_stockpile) +
                         " object_ids: ";
    for (int obj_id : m_object_ids)
        retval += std::to_string(obj_id) + ", ";
    return retval;
}

void ResourcePool::SetObjects(const std::vector<int>& object_ids)
{ m_object_ids = object_ids; }

void ResourcePool::SetConnectedSupplyGroups(const std::set<std::set<int>>& connected_system_groups)
{ m_connected_system_groups = connected_system_groups; }

void ResourcePool::SetStockpile(float d)
{
    DebugLogger() << "ResourcePool " << boost::lexical_cast<std::string>(m_type) << " set to " << d;
    m_stockpile = d;
}

void ResourcePool::Update() {
    //DebugLogger() << "ResourcePool::Update for type " << m_type;
    // sum production from all ResourceCenters in each group, for resource point type appropriate for this pool
    MeterType meter_type = ResourceToMeter(m_type);
    MeterType target_meter_type = ResourceToTargetMeter(m_type);

    if (MeterType::INVALID_METER_TYPE == meter_type || MeterType::INVALID_METER_TYPE == target_meter_type)
        ErrorLogger() << "ResourcePool::Update() called when m_type can't be converted to a valid MeterType";

    // zero to start...
    m_connected_object_groups_resource_output.clear();
    m_connected_object_groups_resource_target_output.clear();

    // temporary storage: indexed by group of systems, which objects
    // are located in that system group?
    std::map<std::set<int>, std::set<std::shared_ptr<const UniverseObject>>>
        system_groups_to_object_groups;


    // for every object, find if a connected system group contains the object's
    // system.  If a group does, place the object into that system group's set
    // of objects.  If no group contains the object, place the object in its own
    // single-object group.
    for (auto& obj : Objects().find<const UniverseObject>(m_object_ids)) {
        int object_id = obj->ID();
        int object_system_id = obj->SystemID();
        // can't generate resources when not in a system
        if (object_system_id == INVALID_OBJECT_ID)
            continue;

        // is object's system in a system group?
        std::set<int> object_system_group;
        for (const auto& sys_group : m_connected_system_groups) {
            if (sys_group.count(object_system_id)) {
                object_system_group = sys_group;
                break;
            }
        }

        // if object's system is not in a system group, add it as its
        // own entry in m_connected_object_groups_resource_output and m_connected_object_groups_resource_target_output
        // this will allow the object to use its own locally produced
        // resource when, for instance, distributing pp
        if (object_system_group.empty()) {
            object_system_group.insert(object_id);  // just use this already-available set to store the object id, even though it is not likely actually a system

            const auto* mmt = obj->GetMeter(meter_type);
            m_connected_object_groups_resource_output[object_system_group] = mmt ? mmt->Current() : 0.0f;

            const auto* mtmt = obj->GetMeter(target_meter_type);
            m_connected_object_groups_resource_target_output[object_system_group] = mtmt ? mtmt->Current() : 0.0f;

            continue;
        }

        // if resource center's system is in a system group, record which system
        // group that is for later
        system_groups_to_object_groups[object_system_group].insert(obj);
    }

    // sum the resource production for object groups, and store the total
    // group production, indexed by group of object ids
    for (auto& entry : system_groups_to_object_groups) {
        const auto& object_group = entry.second;
        std::set<int> object_group_ids;
        float total_group_output = 0.0f;
        float total_group_target_output = 0.0f;
        for (auto& obj : object_group) {
            if (const auto* m = obj->GetMeter(meter_type))
                total_group_output += m->Current();
            if (const auto* m = obj->GetMeter(target_meter_type))
                total_group_target_output += m->Current();
            object_group_ids.insert(obj->ID());
        }
        m_connected_object_groups_resource_output[object_group_ids] = total_group_output;
        m_connected_object_groups_resource_target_output[object_group_ids] = total_group_target_output;
    }

    ChangedSignal();
}
