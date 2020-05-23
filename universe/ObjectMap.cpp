#include "ObjectMap.h"

#include "Universe.h"
#include "UniverseObject.h"
#include "Ship.h"
#include "Fleet.h"
#include "Planet.h"
#include "System.h"
#include "Building.h"
#include "Field.h"
#include "Enums.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"


#define FOR_EACH_SPECIALIZED_MAP(f, ...)  { f(m_resource_centers, ##__VA_ARGS__);   \
                                            f(m_pop_centers, ##__VA_ARGS__);        \
                                            f(m_ships, ##__VA_ARGS__);              \
                                            f(m_fleets, ##__VA_ARGS__);             \
                                            f(m_planets, ##__VA_ARGS__);            \
                                            f(m_systems, ##__VA_ARGS__);            \
                                            f(m_buildings, ##__VA_ARGS__);          \
                                            f(m_fields, ##__VA_ARGS__); }

#define FOR_EACH_MAP(f, ...)              { f(m_objects, ##__VA_ARGS__);            \
                                            FOR_EACH_SPECIALIZED_MAP(f, ##__VA_ARGS__); }

#define FOR_EACH_EXISTING_MAP(f, ...)     { f(m_existing_objects, ##__VA_ARGS__);          \
                                            f(m_existing_resource_centers, ##__VA_ARGS__); \
                                            f(m_existing_pop_centers, ##__VA_ARGS__);      \
                                            f(m_existing_ships, ##__VA_ARGS__);            \
                                            f(m_existing_fleets, ##__VA_ARGS__);           \
                                            f(m_existing_planets, ##__VA_ARGS__);          \
                                            f(m_existing_systems, ##__VA_ARGS__);          \
                                            f(m_existing_buildings, ##__VA_ARGS__);        \
                                            f(m_existing_fields, ##__VA_ARGS__); }


namespace {
    template <typename T>
    static void ClearMap(ObjectMap::container_type<T>& map)
    { map.clear(); }

    template <typename T>
    static void TryInsertIntoMap(ObjectMap::container_type<T>& map, std::shared_ptr<UniverseObject> item)
    {
        if (dynamic_cast<T*>(item.get()))
            map[item->ID()] = std::dynamic_pointer_cast<T, UniverseObject>(item);
    }

    template <typename T>
    void EraseFromMap(ObjectMap::container_type<T>& map, int id)
    { map.erase(id); }
}


/////////////////////////////////////////////
// class ObjectMap
/////////////////////////////////////////////
ObjectMap::ObjectMap()
{}

ObjectMap::~ObjectMap()
{}

void ObjectMap::Copy(const ObjectMap& copied_map, int empire_id/* = ALL_EMPIRES*/) {
    if (&copied_map == this)
        return;

    // loop through objects in copied map, copying or cloning each depending
    // on whether there already is a corresponding object in this map
    for (const auto& obj : copied_map.all())
        this->CopyObject(obj, empire_id);
}

void ObjectMap::CopyForSerialize(const ObjectMap& copied_map) {
    if (&copied_map == this)
        return;

    // note: the following relies upon only m_objects actually getting serialized by ObjectMap::serialize
    m_objects.insert(copied_map.m_objects.begin(), copied_map.m_objects.end());
}

void ObjectMap::CopyObject(std::shared_ptr<const UniverseObject> source, int empire_id/* = ALL_EMPIRES*/) {
    if (!source)
        return;

    int source_id = source->ID();

    // can empire see object at all?  if not, skip copying object's info
    if (GetUniverse().GetObjectVisibilityByEmpire(source_id, empire_id) <= VIS_NO_VISIBILITY)
        return;

    if (auto destination = this->get(source_id)) {
        destination->Copy(source, empire_id); // there already is a version of this object present in this ObjectMap, so just update it
    } else {
        insertCore(std::shared_ptr<UniverseObject>(source->Clone()), empire_id); // this object is not yet present in this ObjectMap, so add a new UniverseObject object for it
    }
}

ObjectMap* ObjectMap::Clone(int empire_id) const {
    ObjectMap* result = new ObjectMap();
    result->Copy(*this, empire_id);
    return result;
}

bool ObjectMap::empty() const
{ return m_objects.empty(); }

int ObjectMap::HighestObjectID() const {
    if (m_objects.empty())
        return INVALID_OBJECT_ID;
    return m_objects.rbegin()->first;
}

void ObjectMap::insertCore(std::shared_ptr<UniverseObject> item, int empire_id/* = ALL_EMPIRES*/) {
    FOR_EACH_MAP(TryInsertIntoMap, item);
    if (item &&
        !GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id).count(item->ID()))
    {
        auto this_item = this->get(item->ID());
        m_existing_objects[item->ID()] = this_item;
        switch (item->ObjectType()) {
            case OBJ_BUILDING:
                m_existing_buildings[item->ID()] = this_item;
                break;
            case OBJ_FIELD:
                m_existing_fields[item->ID()] = this_item;
                break;
            case OBJ_FLEET:
                m_existing_fleets[item->ID()] = this_item;
                break;
            case OBJ_PLANET:
                m_existing_planets[item->ID()] = this_item;
                m_existing_pop_centers[item->ID()] = this_item;
                m_existing_resource_centers[item->ID()] = this_item;
                break;
            case OBJ_POP_CENTER:
                m_existing_pop_centers[item->ID()] = this_item;
                break;
            case OBJ_PROD_CENTER:
                m_existing_resource_centers[item->ID()] = this_item;
                break;
            case OBJ_SHIP:
                m_existing_ships[item->ID()] = this_item;
                break;
            case OBJ_SYSTEM:
                m_existing_systems[item->ID()] = this_item;
                break;
            case OBJ_FIGHTER:
            default:
                break;
        }
    }
}

std::shared_ptr<UniverseObject> ObjectMap::erase(int id) {
    // search for object in objects map
    auto it = m_objects.find(id);
    if (it == m_objects.end())
        return nullptr;
    //DebugLogger() << "Object was removed: " << it->second->Dump();
    // object found, so store pointer for later...
    auto result = it->second;
    // and erase from pointer maps
    m_objects.erase(it);
    FOR_EACH_SPECIALIZED_MAP(EraseFromMap, id);
    FOR_EACH_EXISTING_MAP(EraseFromMap, id);
    return result;
}

void ObjectMap::clear() {
    FOR_EACH_MAP(ClearMap);
    FOR_EACH_EXISTING_MAP(ClearMap);
}

void ObjectMap::swap(ObjectMap& rhs) {
    // SwapMap uses ObjectMap::Map<T> but this function isn't available for the existing maps,
    // so the FOR_EACH_EXISTING_MAP macro doesn't work with with SwapMap
    // and it is instead necessary to write them out explicitly.
    m_existing_objects.swap(rhs.m_existing_objects);
    m_existing_buildings.swap(rhs.m_existing_buildings);
    m_existing_fields.swap(rhs.m_existing_fields);
    m_existing_fleets.swap(rhs.m_existing_fleets);
    m_existing_ships.swap(rhs.m_existing_ships);
    m_existing_planets.swap(rhs.m_existing_planets);
    m_existing_pop_centers.swap(rhs.m_existing_pop_centers);
    m_existing_resource_centers.swap(rhs.m_existing_resource_centers);
    m_existing_systems.swap(rhs.m_existing_systems);
    FOR_EACH_MAP(SwapMap, rhs);
}

std::vector<int> ObjectMap::FindExistingObjectIDs() const {
    std::vector<int> result;
    for (const auto& entry : m_existing_objects)
    { result.push_back(entry.first); }
    return result;
}

void ObjectMap::UpdateCurrentDestroyedObjects(const std::set<int>& destroyed_object_ids) {
    FOR_EACH_EXISTING_MAP(ClearMap);
    for (const auto& entry : m_objects) {
        if (!entry.second)
            continue;
        if (destroyed_object_ids.count(entry.first))
            continue;
        auto this_item = this->get(entry.first);
        m_existing_objects[entry.first] = this_item;
        switch (entry.second->ObjectType()) {
            case OBJ_BUILDING:
                m_existing_buildings[entry.first] = this_item;
                break;
            case OBJ_FIELD:
                m_existing_fields[entry.first] = this_item;
                break;
            case OBJ_FLEET:
                m_existing_fleets[entry.first] = this_item;
                break;
            case OBJ_PLANET:
                m_existing_planets[entry.first] = this_item;
                m_existing_pop_centers[entry.first] = this_item;
                m_existing_resource_centers[entry.first] = this_item;
                break;
            case OBJ_POP_CENTER:
                m_existing_pop_centers[entry.first] = this_item;
                break;
            case OBJ_PROD_CENTER:
                m_existing_resource_centers[entry.first] = this_item;
                break;
            case OBJ_SHIP:
                m_existing_ships[entry.first] = this_item;
                break;
            case OBJ_SYSTEM:
                m_existing_systems[entry.first] = this_item;
                break;
            case OBJ_FIGHTER:
            default:
                break;
        }
    }
}

void ObjectMap::AuditContainment(const std::set<int>& destroyed_object_ids) {
    // determine all objects that some other object thinks contains them
    std::map<int, std::set<int>> contained_objs;
    std::map<int, std::set<int>> contained_planets;
    std::map<int, std::set<int>> contained_buildings;
    std::map<int, std::set<int>> contained_fleets;
    std::map<int, std::set<int>> contained_ships;
    std::map<int, std::set<int>> contained_fields;

    for (const auto& contained : all()) {
        if (destroyed_object_ids.count(contained->ID()))
            continue;

        int contained_id = contained->ID();
        int sys_id = contained->SystemID();
        int alt_id = contained->ContainerObjectID();    // planet or fleet id for a building or ship, or system id again for a fleet, field, or planet
        UniverseObjectType type = contained->ObjectType();
        if (type == OBJ_SYSTEM)
            continue;

        // store systems' contained objects
        if (this->get(sys_id)) { // although this is expected to be a system, can't use Object<System> here due to CopyForSerialize not copying the type-specific objects info
            contained_objs[sys_id].insert(contained_id);

            if (type == OBJ_PLANET)
                contained_planets[sys_id].insert(contained_id);
            else if (type == OBJ_BUILDING)
                contained_buildings[sys_id].insert(contained_id);
            else if (type == OBJ_FLEET)
                contained_fleets[sys_id].insert(contained_id);
            else if (type == OBJ_SHIP)
                contained_ships[sys_id].insert(contained_id);
            else if (type == OBJ_FIELD)
                contained_fields[sys_id].insert(contained_id);
        }

        // store planets' contained buildings
        if (type == OBJ_BUILDING && this->get(alt_id))
            contained_buildings[alt_id].insert(contained_id);

        // store fleets' contained ships
        if (type == OBJ_SHIP && this->get(alt_id))
            contained_ships[alt_id].insert(contained_id);
    }

    // set contained objects of all possible containers
    for (const auto& obj : all()) {
        if (obj->ObjectType() == OBJ_SYSTEM) {
            auto sys = std::dynamic_pointer_cast<System>(obj);
            if (!sys)
                continue;
            sys->m_objects =    contained_objs[sys->ID()];
            sys->m_planets =    contained_planets[sys->ID()];
            sys->m_buildings =  contained_buildings[sys->ID()];
            sys->m_fleets =     contained_fleets[sys->ID()];
            sys->m_ships =      contained_ships[sys->ID()];
            sys->m_fields =     contained_fields[sys->ID()];

        } else if (obj->ObjectType() == OBJ_PLANET) {
            auto plt = std::dynamic_pointer_cast<Planet>(obj);
            if (!plt)
                continue;
            plt->m_buildings =  contained_buildings[plt->ID()];

        } else if (obj->ObjectType() == OBJ_FLEET) {
            auto flt = std::dynamic_pointer_cast<Fleet>(obj);
            if (!flt)
                continue;
            flt->m_ships =      contained_ships[flt->ID()];
        }
    }
}

void ObjectMap::CopyObjectsToSpecializedMaps() {
    FOR_EACH_SPECIALIZED_MAP(ClearMap);
    for (const auto& entry : Map<UniverseObject>())
    { FOR_EACH_SPECIALIZED_MAP(TryInsertIntoMap, entry.second); }
}

std::string ObjectMap::Dump(unsigned short ntabs) const {
    std::ostringstream dump_stream;
    dump_stream << "ObjectMap contains UniverseObjects: " << std::endl;
    for (const auto& obj : all())
        dump_stream << obj->Dump(ntabs) << std::endl;
    dump_stream << std::endl;
    return dump_stream.str();
}

std::shared_ptr<const UniverseObject> ObjectMap::ExistingObject(int id) const {
    auto it = m_existing_objects.find(id);
    if (it != m_existing_objects.end())
        return it->second;
    return nullptr;
}

// Static helpers

template <typename T>
void ObjectMap::SwapMap(ObjectMap::container_type<T>& map, ObjectMap& rhs)
{ map.swap(rhs.Map<T>()); }

// template specializations

template <>
const ObjectMap::container_type<UniverseObject>& ObjectMap::Map() const
{ return m_objects; }

template <>
const ObjectMap::container_type<ResourceCenter>& ObjectMap::Map() const
{ return m_resource_centers; }

template <>
const ObjectMap::container_type<PopCenter>& ObjectMap::Map() const
{ return m_pop_centers; }

template <>
const ObjectMap::container_type<Ship>& ObjectMap::Map() const
{ return m_ships; }

template <>
const ObjectMap::container_type<Fleet>& ObjectMap::Map() const
{ return m_fleets; }

template <>
const ObjectMap::container_type<Planet>& ObjectMap::Map() const
{ return m_planets; }

template <>
const ObjectMap::container_type<System>& ObjectMap::Map() const
{ return m_systems; }

template <>
const ObjectMap::container_type<Building>& ObjectMap::Map() const
{ return m_buildings; }

template <>
const ObjectMap::container_type<Field>& ObjectMap::Map() const
{ return m_fields; }

template <>
ObjectMap::container_type<UniverseObject>& ObjectMap::Map()
{ return m_objects; }

template <>
ObjectMap::container_type<ResourceCenter>& ObjectMap::Map()
{ return m_resource_centers; }

template <>
ObjectMap::container_type<PopCenter>& ObjectMap::Map()
{ return m_pop_centers; }

template <>
ObjectMap::container_type<Ship>& ObjectMap::Map()
{ return m_ships; }

template <>
ObjectMap::container_type<Fleet>& ObjectMap::Map()
{ return m_fleets; }

template <>
ObjectMap::container_type<Planet>& ObjectMap::Map()
{ return m_planets; }

template <>
ObjectMap::container_type<System>& ObjectMap::Map()
{ return m_systems; }

template <>
ObjectMap::container_type<Building>& ObjectMap::Map()
{ return m_buildings; }

template <>
ObjectMap::container_type<Field>& ObjectMap::Map()
{ return m_fields; }
