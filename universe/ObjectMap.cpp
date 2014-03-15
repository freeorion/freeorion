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

/////////////////////////////////////////////
// class ObjectMap
/////////////////////////////////////////////
ObjectMap::ObjectMap()
{}

ObjectMap::~ObjectMap() {
    // Make sure to call ObjectMap::Clear() before destruction somewhere if
    // this ObjectMap contains any unique pointers to UniverseObject objects.
    // Otherwise, the pointed-to UniverseObjects will be leaked memory...
}

void ObjectMap::Copy(const ObjectMap& copied_map, int empire_id/* = ALL_EMPIRES*/) {
    if (&copied_map == this)
        return;

    // loop through objects in copied map, copying or cloning each depending
    // on whether there already is a corresponding object in this map
    for (const_iterator<> it = copied_map.const_begin(); it != copied_map.const_end(); ++it)
        this->CopyObject(*it, empire_id);
}

void ObjectMap::CopyForSerialize(const ObjectMap& copied_map) {
    if (&copied_map == this)
        return;

    // note: the following relies upon only m_objects actually getting serialized by ObjectMap::serialize
    m_objects.insert(copied_map.m_objects.begin(), copied_map.m_objects.end());
}

void ObjectMap::CopyObject(TemporaryPtr<const UniverseObject> source, int empire_id/* = ALL_EMPIRES*/) {
    if (!source)
        return;

    int source_id = source->ID();

    // can empire see object at all?  if not, skip copying object's info
    if (GetUniverse().GetObjectVisibilityByEmpire(source_id, empire_id) <= VIS_NO_VISIBILITY)
        return;
    
    if (TemporaryPtr<UniverseObject> destination = this->Object(source_id)) {
        destination->Copy(source, empire_id); // there already is a version of this object present in this ObjectMap, so just update it
    } else {
        Insert(source->Clone(), empire_id); // this object is not yet present in this ObjectMap, so add a new UniverseObject object for it
    }
}

ObjectMap* ObjectMap::Clone(int empire_id) const {
    ObjectMap* result = new ObjectMap();
    result->Copy(*this, empire_id);
    return result;
}

int ObjectMap::NumObjects() const
{ return static_cast<int>(m_objects.size()); }

bool ObjectMap::Empty() const
{ return m_objects.empty(); }

TemporaryPtr<const UniverseObject> ObjectMap::Object(int id) const
{ return Object<UniverseObject>(id); }

TemporaryPtr<UniverseObject> ObjectMap::Object(int id)
{ return Object<UniverseObject>(id); }

std::vector<TemporaryPtr<const UniverseObject> > ObjectMap::FindObjects(const std::vector<int>& object_ids) const {
    std::vector<TemporaryPtr<const UniverseObject> > result;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
        if (TemporaryPtr<const UniverseObject> obj = Object(*it))
            result.push_back(obj);
        else
            Logger().errorStream() << "ObjectMap::FindObjects couldn't find object with id " << *it;
    return result;
}

std::vector<TemporaryPtr<UniverseObject> > ObjectMap::FindObjects(const std::vector<int>& object_ids) {
    std::vector<TemporaryPtr<UniverseObject> > result;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
        if (TemporaryPtr<UniverseObject> obj = Object(*it))
            result.push_back(obj);
        else
            Logger().errorStream() << "ObjectMap::FindObjects couldn't find object with id " << *it;
    return result;
}

std::vector<TemporaryPtr<const UniverseObject> > ObjectMap::FindObjects(UniverseObjectVisitorRR& visitor) const {
    std::vector<TemporaryPtr<const UniverseObject> > result;
    for (const_iterator<> it = const_begin(); it != const_end(); ++it) {
        if (TemporaryPtr<const UniverseObject> obj = it->Accept(visitor))
            result.push_back(Object(obj->ID()));
    }
    return result;
}

std::vector<TemporaryPtr<UniverseObject> > ObjectMap::FindObjects(UniverseObjectVisitorRR& visitor) {
    std::vector<TemporaryPtr<UniverseObject> > result;
    for (iterator<> it = begin(); it != end(); ++it) {
        if (TemporaryPtr<UniverseObject> obj = it->Accept(visitor))
            result.push_back(Object(obj->ID()));
    }
    return result;
}

std::vector<int> ObjectMap::FindObjectIDs(UniverseObjectVisitorRR& visitor) const {
    std::vector<int> result;
    for (const_iterator<> it = const_begin(); it != const_end(); ++it) {
        if (it->Accept(visitor))
            result.push_back(it->ID());
    }
    return result;
}

std::vector<int> ObjectMap::FindObjectIDs() const {
    return FindObjectIDs<UniverseObject>();
}

ObjectMap::iterator<> ObjectMap::begin()
{ return begin<UniverseObject>(); }

ObjectMap::iterator<> ObjectMap::end()
{ return end<UniverseObject>(); }

ObjectMap::const_iterator<> ObjectMap::const_begin() const
{ return const_begin<UniverseObject>(); }

ObjectMap::const_iterator<> ObjectMap::const_end() const
{ return const_end<UniverseObject>(); }

void ObjectMap::Insert(TemporaryPtr<UniverseObject> item, int empire_id/* = ALL_EMPIRES*/) {
    FOR_EACH_MAP(TryInsertIntoMap, item);
    if (item &&
        GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id).find(item->ID()) ==
            GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id).end())
    {
        TemporaryPtr<UniverseObject> this_item = this->Object(item->ID());
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
            default:
                break;
        }
    }
}

TemporaryPtr<UniverseObject> ObjectMap::Remove(int id) {
    // search for object in objects map
    std::map<int, TemporaryPtr<UniverseObject> >::iterator it = m_objects.find(id);
    if (it == m_objects.end())
        return TemporaryPtr<UniverseObject>();
    //Logger().debugStream() << "Object was removed: " << it->second->Dump();
    // object found, so store pointer for later...
    TemporaryPtr<UniverseObject> result = it->second;
    // and erase from pointer maps
    m_objects.erase(it);
    FOR_EACH_SPECIALIZED_MAP(EraseFromMap, id);
    m_existing_objects.erase(id);
    m_existing_buildings.erase(id);
    m_existing_fields.erase(id);
    m_existing_fleets.erase(id);
    m_existing_ships.erase(id);
    m_existing_planets.erase(id);
    m_existing_pop_centers.erase(id);
    m_existing_resource_centers.erase(id);
    m_existing_systems.erase(id);
    return result;
}

void ObjectMap::Clear() {
    FOR_EACH_MAP(ClearMap);
}

void ObjectMap::swap(ObjectMap& rhs) {
    FOR_EACH_MAP(SwapMap, rhs);
}

std::vector<int> ObjectMap::FindExistingObjectIDs() const {
    std::vector<int> result;
    for (std::map< int, TemporaryPtr< UniverseObject > >::const_iterator it = m_existing_objects.begin();
         it != m_existing_objects.end(); ++it)
    { result.push_back(it->first); }
    return result;
}

void ObjectMap::UpdateCurrentDestroyedObjects(const std::set<int>& destroyed_object_ids) {
    m_existing_objects.clear();
    m_existing_buildings.clear();
    m_existing_fields.clear();
    m_existing_fleets.clear();
    m_existing_ships.clear();
    m_existing_planets.clear();
    m_existing_pop_centers.clear();
    m_existing_resource_centers.clear();
    m_existing_systems.clear();
    for (std::map<int, TemporaryPtr<UniverseObject> >::iterator it = m_objects.begin();
         it != m_objects.end(); ++it)
    {
        if (!it->second)
            continue;
        if (destroyed_object_ids.find(it->first) != destroyed_object_ids.end())
            continue;
        TemporaryPtr< UniverseObject > this_item = this->Object(it->first);
        m_existing_objects[it->first] = this_item;
        switch (it->second->ObjectType()) {
            case OBJ_BUILDING:
                m_existing_buildings[it->first] = this_item;
                break;
            case OBJ_FIELD:
                m_existing_fields[it->first] = this_item;
                break;
            case OBJ_FLEET:
                m_existing_fleets[it->first] = this_item;
                break;
            case OBJ_PLANET:
                m_existing_planets[it->first] = this_item;
                m_existing_pop_centers[it->first] = this_item;
                m_existing_resource_centers[it->first] = this_item;
                break;
            case OBJ_POP_CENTER:
                m_existing_pop_centers[it->first] = this_item;
                break;
            case OBJ_PROD_CENTER:
                m_existing_resource_centers[it->first] = this_item;
                break;
            case OBJ_SHIP:
                m_existing_ships[it->first] = this_item;
                break;
            case OBJ_SYSTEM:
                m_existing_systems[it->first] = this_item;
                break;
            default:
                break;
        }
    }
}

void ObjectMap::AuditContainment(const std::set<int>& destroyed_object_ids) {
    // determine all objects that some other object thinks contains them
    std::map<int, std::set<int> >   contained_objs;
    std::map<int, std::set<int> >   contained_planets;
    std::map<int, std::set<int> >   contained_buildings;
    std::map<int, std::set<int> >   contained_fleets;
    std::map<int, std::set<int> >   contained_ships;
    std::map<int, std::set<int> >   contained_fields;

    for (const_iterator<> it = const_begin (); it != const_end(); ++it) {
        TemporaryPtr<const UniverseObject> contained = *it;
        if (destroyed_object_ids.find(contained->ID()) != destroyed_object_ids.end())
            continue;

        int contained_id = contained->ID();
        int sys_id = contained->SystemID();
        int alt_id = contained->ContainerObjectID();    // planet or fleet id for a building or ship, or system id again for a fleet, field, or planet
        UniverseObjectType type = contained->ObjectType();
        if (type == OBJ_SYSTEM)
            continue;

        // store systems' contained objects
        if (this->Object(sys_id)) { // although this is expected to be a system, can't use Object<System> here due to CopyForSerialize not copying the type-specific objects info
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
        if (type == OBJ_BUILDING && this->Object(alt_id))
            contained_buildings[alt_id].insert(contained_id);

        // store fleets' contained ships
        if (type == OBJ_SHIP && this->Object(alt_id))
            contained_ships[alt_id].insert(contained_id);
    }

    // set contained objects of all possible containers
    for (iterator<> it = begin(); it != end(); ++it) {
        TemporaryPtr<UniverseObject> obj = *it;
        if (obj->ObjectType() == OBJ_SYSTEM) {
            TemporaryPtr<System> sys = boost::dynamic_pointer_cast<System>(obj);
            if (!sys)
                continue;
            sys->m_objects =    contained_objs[sys->ID()];
            sys->m_planets =    contained_planets[sys->ID()];
            sys->m_buildings =  contained_buildings[sys->ID()];
            sys->m_fleets =     contained_fleets[sys->ID()];
            sys->m_ships =      contained_ships[sys->ID()];
            sys->m_fields =     contained_fields[sys->ID()];
        } else if (obj->ObjectType() == OBJ_PLANET) {
            TemporaryPtr<Planet> plt = boost::dynamic_pointer_cast<Planet>(obj);
            if (!plt)
                continue;
            plt->m_buildings =  contained_buildings[plt->ID()];
        } else if (obj->ObjectType() == OBJ_FLEET) {
            TemporaryPtr<Fleet> flt = boost::dynamic_pointer_cast<Fleet>(obj);
            if (!flt)
                continue;
            flt->m_ships =      contained_ships[flt->ID()];
        }
    }
}

void ObjectMap::CopyObjectsToSpecializedMaps() {
    FOR_EACH_SPECIALIZED_MAP(ClearMap);
    for (std::map<int, TemporaryPtr<UniverseObject> >::iterator it = Map<UniverseObject>().begin();
         it != Map<UniverseObject>().end(); ++it)
    { FOR_EACH_SPECIALIZED_MAP(TryInsertIntoMap, it->second); }
}

std::string ObjectMap::Dump() const {
    std::ostringstream dump_stream;
    dump_stream << "ObjectMap contains UniverseObjects: " << std::endl;
    for (const_iterator<> it = const_begin(); it != const_end(); ++it)
        dump_stream << it->Dump() << std::endl;
    dump_stream << std::endl;
    return dump_stream.str();
}

TemporaryPtr<UniverseObject> ObjectMap::ExistingObject(int id) {
    std::map<int, TemporaryPtr<UniverseObject> >::iterator it = m_existing_objects.find(id);
    if (it != m_existing_objects.end())
        return it->second;
    return TemporaryPtr<UniverseObject>();
}

// Static helpers

template<class T>
void ObjectMap::EraseFromMap(std::map<int, TemporaryPtr<T> >& map, int id)
{ map.erase(id); }

template<class T>
void ObjectMap::ClearMap(std::map<int, TemporaryPtr<T> >& map)
{ map.clear(); }

template<class T>
void ObjectMap::SwapMap(std::map<int, TemporaryPtr<T> >& map, ObjectMap& rhs)
{ map.swap(rhs.Map<T>()); }

template <class T>
void ObjectMap::TryInsertIntoMap(std::map<int, TemporaryPtr<T> >& map, TemporaryPtr<UniverseObject> item) {
    if (dynamic_cast<T*>(item.get()))
        map[item->ID()] = boost::dynamic_pointer_cast<T, UniverseObject>(item);
}

// template specializations

template <>
const std::map<int, TemporaryPtr<UniverseObject> >&  ObjectMap::Map() const
{ return m_objects; }

template <>
const std::map<int, TemporaryPtr<ResourceCenter> >&  ObjectMap::Map() const
{ return m_resource_centers; }

template <>
const std::map<int, TemporaryPtr<PopCenter> >&  ObjectMap::Map() const
{ return m_pop_centers; }

template <>
const std::map<int, TemporaryPtr<Ship> >&  ObjectMap::Map() const
{ return m_ships; }

template <>
const std::map<int, TemporaryPtr<Fleet> >&  ObjectMap::Map() const
{ return m_fleets; }

template <>
const std::map<int, TemporaryPtr<Planet> >&  ObjectMap::Map() const
{ return m_planets; }

template <>
const std::map<int, TemporaryPtr<System> >&  ObjectMap::Map() const
{ return m_systems; }

template <>
const std::map<int, TemporaryPtr<Building> >&  ObjectMap::Map() const
{ return m_buildings; }

template <>
const std::map<int, TemporaryPtr<Field> >&  ObjectMap::Map() const
{ return m_fields; }

template <>
std::map<int, TemporaryPtr<UniverseObject> >&  ObjectMap::Map()
{ return m_objects; }

template <>
std::map<int, TemporaryPtr<ResourceCenter> >&  ObjectMap::Map()
{ return m_resource_centers; }

template <>
std::map<int, TemporaryPtr<PopCenter> >&  ObjectMap::Map()
{ return m_pop_centers; }

template <>
std::map<int, TemporaryPtr<Ship> >&  ObjectMap::Map()
{ return m_ships; }

template <>
std::map<int, TemporaryPtr<Fleet> >&  ObjectMap::Map()
{ return m_fleets; }

template <>
std::map<int, TemporaryPtr<Planet> >&  ObjectMap::Map()
{ return m_planets; }

template <>
std::map<int, TemporaryPtr<System> >&  ObjectMap::Map()
{ return m_systems; }

template <>
std::map<int, TemporaryPtr<Building> >&  ObjectMap::Map()
{ return m_buildings; }

template <>
std::map<int, TemporaryPtr<Field> >&  ObjectMap::Map()
{ return m_fields; }
