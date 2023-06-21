#include "ResourcePool.h"

#include <numeric>
#include <cassert>
#include "../universe/Enums.h"
#include "../universe/ObjectMap.h"
#include "../universe/Planet.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"

MeterType ResourceToMeter(ResourceType type) noexcept {
    switch (type) {
    case ResourceType::RE_INDUSTRY:  return MeterType::METER_INDUSTRY;
    case ResourceType::RE_RESEARCH:  return MeterType::METER_RESEARCH;
    case ResourceType::RE_INFLUENCE: return MeterType::METER_INFLUENCE;
    case ResourceType::RE_STOCKPILE: return MeterType::METER_STOCKPILE;
    default:
        return MeterType::INVALID_METER_TYPE;
        break;
    }
}

MeterType ResourceToTargetMeter(ResourceType type) noexcept {
    switch (type) {
    case ResourceType::RE_INDUSTRY:  return MeterType::METER_TARGET_INDUSTRY;
    case ResourceType::RE_RESEARCH:  return MeterType::METER_TARGET_RESEARCH;
    case ResourceType::RE_INFLUENCE: return MeterType::METER_TARGET_INFLUENCE;
    case ResourceType::RE_STOCKPILE: return MeterType::METER_MAX_STOCKPILE;
    default:
        return MeterType::INVALID_METER_TYPE;
        break;
    }
}

ResourceType MeterToResource(MeterType type) noexcept {
    switch (type) {
    case MeterType::METER_INDUSTRY:  return ResourceType::RE_INDUSTRY;
    case MeterType::METER_RESEARCH:  return ResourceType::RE_RESEARCH;
    case MeterType::METER_INFLUENCE: return ResourceType::RE_INFLUENCE;
    case MeterType::METER_STOCKPILE: return ResourceType::RE_STOCKPILE;
    default:
        return ResourceType::INVALID_RESOURCE_TYPE;
        break;
    }
}

//////////////////////////////////////////////////
// ResourcePool
//////////////////////////////////////////////////
float ResourcePool::TotalOutput() const {
    float retval = 0.0f;
    // transform_reduce
    for (const auto& entry : m_connected_object_groups_resource_output)
        retval += entry.second;
    return retval;
}

float ResourcePool::GroupOutput(int object_id) const {
    // find group containing specified object
    // find_if
    for (const auto& [group, output] : m_connected_object_groups_resource_output) {
        if (group.contains(object_id))
            return output;
    }

    // default return case:
    //DebugLogger() << "ResourcePool::GroupOutput passed unknown object id: " << object_id;
    return 0.0f;
}

float ResourcePool::TargetOutput() const {
    float retval = 0.0f;
    // transform_reduce
    for (const auto& entry : m_connected_object_groups_resource_target_output)
        retval += entry.second;
    return retval;
}

float ResourcePool::GroupTargetOutput(int object_id) const {
    // find group containing specified object
    // find_if
    for (const auto& entry : m_connected_object_groups_resource_target_output) {
        if (entry.first.contains(object_id))
            return entry.second;
    }

    // default return case:
    DebugLogger() << "ResourcePool::GroupTargetOutput passed unknown object id: " << object_id;
    return 0.0f;
}

float ResourcePool::TotalAvailable() const {
    float retval = m_stockpile;
    // transform_reduce
    for (const auto& entry : m_connected_object_groups_resource_output)
        retval += entry.second;
    return retval;
}

float ResourcePool::GroupAvailable(int object_id) const {
    TraceLogger() << "ResourcePool::GroupAvailable(" << object_id << ")";
    // available is production in this group
    return GroupOutput(object_id);
}

std::string ResourcePool::Dump() const {
    std::string retval{"ResourcePool type = "};
    retval.append(to_string(m_type)).append(" stockpile = ").append(std::to_string(m_stockpile))
          .append(" object_ids: ");
    for (int obj_id : m_object_ids)
        retval.append(std::to_string(obj_id)).append(", ");
    return retval;
}

void ResourcePool::SetObjects(std::vector<int> object_ids)
{ m_object_ids = std::move(object_ids); }

void ResourcePool::SetConnectedSupplyGroups(const std::set<std::set<int>>& connected_system_groups)
{ m_connected_system_groups = connected_system_groups; }

void ResourcePool::SetStockpile(float d) {
    DebugLogger() << "ResourcePool " << to_string(m_type) << " set to " << d;
    m_stockpile = d;
}

void ResourcePool::Update(const ObjectMap& objects) {
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
    std::map<int_flat_set, std::pair<boost::container::flat_set<const UniverseObject*>, int_flat_set>> system_groups_to_object_groups;


    // for every object, find if a connected system group contains the object's
    // system.  If a group does, place the object into that system group's set
    // of objects.  If no group contains the object, place the object in its own
    // single-object group.
    for (auto* obj : objects.findRaw<const UniverseObject>(m_object_ids)) {
        const int object_id = obj->ID();
        const int object_system_id = obj->SystemID();
        // can't generate resources when not in a system
        if (object_system_id == INVALID_OBJECT_ID)
            continue;

        // is object's system in a system group?
        int_flat_set object_system_group = [&]() -> int_flat_set {
            for (const auto& sys_group : m_connected_system_groups)
                if (sys_group.find(object_system_id) != sys_group.end())
                    return {boost::container::ordered_unique_range, sys_group.begin(), sys_group.end()};
            return {};
        }();

        // if object's system is not in a system group, add it as its
        // own entry in m_connected_object_groups_resource_output and m_connected_object_groups_resource_target_output
        // this will allow the object to use its own locally produced
        // resource when, for instance, distributing pp
        if (object_system_group.empty()) {
            // store object id in set, even though it is not likely actually a system
            object_system_group.insert(object_id);

            const auto* mmt = obj->GetMeter(meter_type);
            const auto* mtmt = obj->GetMeter(target_meter_type);

            m_connected_object_groups_resource_output[object_system_group] = mmt ? mmt->Current() : 0.0f;
            m_connected_object_groups_resource_target_output[std::move(object_system_group)] = mtmt ? mtmt->Current() : 0.0f;

        } else {
            // if resource center's system is in a system group, put system
            // and its id into object group for that system group
            auto& [objects, ids] = system_groups_to_object_groups.try_emplace(std::move(object_system_group)).first->second;
            objects.insert(obj);
            ids.insert(object_id);
        }
    }

    // sum the resource production for object groups, and store the total
    // group production, indexed by group of object ids
    for (const auto& [ignored, group_objects_and_ids] : system_groups_to_object_groups) {
        (void)ignored;
        auto& [object_group, object_ids] = group_objects_and_ids;

        float total_group_output =
            std::transform_reduce(object_group.begin(), object_group.end(), 0.0f, std::plus{},
                                  [meter_type](const UniverseObject* o) {
                                      if (const auto* m = o->GetMeter(meter_type))
                                          return m->Current();
                                      return 0.0f;
                                  });
        m_connected_object_groups_resource_output[object_ids] = total_group_output;

        float total_group_target_output =
            std::transform_reduce(object_group.begin(), object_group.end(), 0.0f, std::plus{},
                                  [target_meter_type](const UniverseObject* o) {
                                      if (const auto* m = o->GetMeter(target_meter_type))
                                          return m->Current();
                                      return 0.0f;
                                  });
        m_connected_object_groups_resource_target_output[std::move(object_ids)] = total_group_target_output;
    }

    ChangedSignal();
}
