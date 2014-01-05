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

void ObjectMap::CompleteCopyVisible(const ObjectMap& copied_map, int empire_id/* = ALL_EMPIRES*/) {
    if (&copied_map == this)
        return;

    // loop through objects in copied map, copying or cloning each depending
    // on whether there already is a corresponding object in this map
    for (const_iterator<> it = copied_map.const_begin(); it != copied_map.const_end(); ++it) {
        int object_id = it->ID();

        // can empire see object at all?  if not, skip copying object's info
        if (GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id) <= VIS_NO_VISIBILITY)
            continue;

        // if object is at all visible, copy all information, not just info
        // appropriate for the actual visibility level.  this ensures that any
        // details previously learned about object will still be recorded in
        // copied-to ObjectMap
        this->CopyObject(*it, ALL_EMPIRES);
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

TemporaryPtr<const UniverseObject> ObjectMap::Object(int id) const {
    return Object<UniverseObject>(id);
}

TemporaryPtr<UniverseObject> ObjectMap::Object(int id) {
    return Object<UniverseObject>(id);
}

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

std::vector<TemporaryPtr<const UniverseObject> > ObjectMap::FindObjects(const UniverseObjectVisitor& visitor) const {
    std::vector<TemporaryPtr<const UniverseObject> > result;
    for (const_iterator<> it = const_begin(); it != const_end(); ++it) {
        if (TemporaryPtr<UniverseObject> obj = it->Accept(visitor))
            result.push_back(Object(obj->ID()));
    }
    return result;
}

std::vector<TemporaryPtr<UniverseObject> > ObjectMap::FindObjects(const UniverseObjectVisitor& visitor) {
    std::vector<TemporaryPtr<UniverseObject> > result;
    for (iterator<> it = begin(); it != end(); ++it) {
        if (TemporaryPtr<UniverseObject> obj = it->Accept(visitor))
            result.push_back(Object(obj->ID()));
    }
    return result;
}

std::vector<int> ObjectMap::FindObjectIDs(const UniverseObjectVisitor& visitor) const {
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

void ObjectMap::Insert(boost::shared_ptr<UniverseObject> item, int empire_id/* = ALL_EMPIRES*/) {
    FOR_EACH_MAP(TryInsertIntoMap, item);
    if (item && GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id).find(item->ID()) == GetUniverse().EmpireKnownDestroyedObjectIDs(empire_id).end()) {
        TemporaryPtr< UniverseObject > this_item = this->Object(item->ID());
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

boost::shared_ptr<UniverseObject> ObjectMap::Remove(int id) {
    // search for object in objects map
    std::map<int, boost::shared_ptr<UniverseObject> >::iterator it = m_objects.find(id);
    if (it == m_objects.end())
        return boost::shared_ptr<UniverseObject>();
    //Logger().debugStream() << "Object was removed: " << it->second->Dump();
    // object found, so store pointer for later...
    boost::shared_ptr<UniverseObject> result = it->second;
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
    for (std::map< int, TemporaryPtr< UniverseObject > >::const_iterator it = m_existing_objects.begin(); it != m_existing_objects.end(); ++it)
        result.push_back(it->first);
    return result;
}

void ObjectMap::UpdateCurrentDestroyedObjects(const std::set<int> destroyed_object_ids) {
    m_existing_objects.clear();
    m_existing_buildings.clear();
    m_existing_fields.clear();
    m_existing_fleets.clear();
    m_existing_ships.clear();
    m_existing_planets.clear();
    m_existing_pop_centers.clear();
    m_existing_resource_centers.clear();
    m_existing_systems.clear();
    for ( std::map< int, boost::shared_ptr< UniverseObject > >::iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
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

void ObjectMap::CopyObjectsToSpecializedMaps() {
    FOR_EACH_SPECIALIZED_MAP(ClearMap);
    for (std::map<int, boost::shared_ptr<UniverseObject> >::iterator it = Map<UniverseObject>().begin();
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

// Static helpers

template<class T>
void ObjectMap::EraseFromMap(std::map<int, boost::shared_ptr<T> >& map, int id)
{ map.erase(id); }

template<class T>
void ObjectMap::ClearMap(std::map<int, boost::shared_ptr<T> >& map)
{ map.clear(); }

template<class T>
void ObjectMap::SwapMap(std::map<int, boost::shared_ptr<T> >& map, ObjectMap& rhs)
{ map.swap(rhs.Map<T>()); }

template <class T>
void ObjectMap::TryInsertIntoMap(std::map<int, boost::shared_ptr<T> >& map, boost::shared_ptr<UniverseObject> item) {
    if (T* t_item = dynamic_cast<T*>(item.get()))
        map[item->ID()] = boost::dynamic_pointer_cast<T, UniverseObject>(item);
}

// template specializations

template <>
const std::map<int, boost::shared_ptr<UniverseObject> >&  ObjectMap::Map() const
{ return m_objects; }

template <>
const std::map<int, boost::shared_ptr<ResourceCenter> >&  ObjectMap::Map() const
{ return m_resource_centers; }

template <>
const std::map<int, boost::shared_ptr<PopCenter> >&  ObjectMap::Map() const
{ return m_pop_centers; }

template <>
const std::map<int, boost::shared_ptr<Ship> >&  ObjectMap::Map() const
{ return m_ships; }

template <>
const std::map<int, boost::shared_ptr<Fleet> >&  ObjectMap::Map() const
{ return m_fleets; }

template <>
const std::map<int, boost::shared_ptr<Planet> >&  ObjectMap::Map() const
{ return m_planets; }

template <>
const std::map<int, boost::shared_ptr<System> >&  ObjectMap::Map() const
{ return m_systems; }

template <>
const std::map<int, boost::shared_ptr<Building> >&  ObjectMap::Map() const
{ return m_buildings; }

template <>
const std::map<int, boost::shared_ptr<Field> >&  ObjectMap::Map() const
{ return m_fields; }

template <>
std::map<int, boost::shared_ptr<UniverseObject> >&  ObjectMap::Map()
{ return m_objects; }

template <>
std::map<int, boost::shared_ptr<ResourceCenter> >&  ObjectMap::Map()
{ return m_resource_centers; }

template <>
std::map<int, boost::shared_ptr<PopCenter> >&  ObjectMap::Map()
{ return m_pop_centers; }

template <>
std::map<int, boost::shared_ptr<Ship> >&  ObjectMap::Map()
{ return m_ships; }

template <>
std::map<int, boost::shared_ptr<Fleet> >&  ObjectMap::Map()
{ return m_fleets; }

template <>
std::map<int, boost::shared_ptr<Planet> >&  ObjectMap::Map()
{ return m_planets; }

template <>
std::map<int, boost::shared_ptr<System> >&  ObjectMap::Map()
{ return m_systems; }

template <>
std::map<int, boost::shared_ptr<Building> >&  ObjectMap::Map()
{ return m_buildings; }

template <>
std::map<int, boost::shared_ptr<Field> >&  ObjectMap::Map()
{ return m_fields; }
