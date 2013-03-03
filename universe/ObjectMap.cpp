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
    for (const_value_iterator<> it = copied_map.begin_values(); it != copied_map.end_values(); ++it)
        this->CopyObject(*it, empire_id);
}

void ObjectMap::CopyObject(const UniverseObject* source, int empire_id/* = ALL_EMPIRES*/) {
    if (!source)
        return;

    int source_id = source->ID();

    // can empire see object at all?  if not, skip copying object's info
    if (GetUniverse().GetObjectVisibilityByEmpire(source_id, empire_id) <= VIS_NO_VISIBILITY)
        return;

    if (UniverseObject* destination = this->Object(source_id)) {
        destination->Copy(source, empire_id); // there already is a version of this object present in this ObjectMap, so just update it
    } else {
        UniverseObject* clone = source->Clone();  // this object is not yet present in this ObjectMap, so add a new UniverseObject object for it
        Insert(clone);
    }
}

void ObjectMap::CompleteCopyVisible(const ObjectMap& copied_map, int empire_id/* = ALL_EMPIRES*/) {
    if (&copied_map == this)
        return;

    // loop through objects in copied map, copying or cloning each depending
    // on whether there already is a corresponding object in this map
    for (const_iterator<> it = copied_map.begin(); it != copied_map.end(); ++it) {
        int object_id = it->first;

        // can empire see object at all?  if not, skip copying object's info
        if (GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id) <= VIS_NO_VISIBILITY)
            continue;

        // if object is at all visible, copy all information, not just info
        // appropriate for the actual visibility level.  this ensures that any
        // details previously learned about object will still be recorded in
        // copied-to ObjectMap
        this->CopyObject(it->second, ALL_EMPIRES);
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

const UniverseObject* ObjectMap::Object(int id) const {
    std::map<int, UniverseObject*>::const_iterator it = m_objects.find(id);
    return (it != m_objects.end() ? it->second : 0);
}

UniverseObject* ObjectMap::Object(int id) {
    std::map<int, UniverseObject*>::iterator it = m_objects.find(id);
    return (it != m_objects.end() ? it->second : 0);
}

std::vector<const UniverseObject*> ObjectMap::FindObjects(const std::vector<int>& object_ids) const {
    std::vector<const UniverseObject*> result;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
        if (const UniverseObject* obj = Object(*it))
            result.push_back(obj);
        else
            Logger().errorStream() << "ObjectMap::FindObjects couldn't find object with id " << *it;
    return result;
}

std::vector<UniverseObject*> ObjectMap::FindObjects(const std::vector<int>& object_ids) {
    std::vector<UniverseObject*> result;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
        if (UniverseObject* obj = Object(*it))
            result.push_back(obj);
        else
            Logger().errorStream() << "ObjectMap::FindObjects couldn't find object with id " << *it;
    return result;
}

std::vector<const UniverseObject*> ObjectMap::FindObjects(const UniverseObjectVisitor& visitor) const {
    std::vector<const UniverseObject*> result;
    for (const_value_iterator<> it = begin_values(); it != end_values(); ++it) {
        if (UniverseObject* obj = it->Accept(visitor))
            result.push_back(obj);
    }
    return result;
}

std::vector<UniverseObject*> ObjectMap::FindObjects(const UniverseObjectVisitor& visitor) {
    std::vector<UniverseObject*> result;
    for (value_iterator<> it = begin_values(); it != end_values(); ++it) {
        if (UniverseObject* obj = it->Accept(visitor))
            result.push_back(obj);
    }
    return result;
}

std::vector<int> ObjectMap::FindObjectIDs(const UniverseObjectVisitor& visitor) const {
    std::vector<int> result;
    for (const_iterator<> it = begin(); it != end(); ++it) {
        if (it->second->Accept(visitor))
            result.push_back(it->first);
    }
    return result;
}

std::vector<int> ObjectMap::FindObjectIDs() const {
    std::vector<int> result;
    for (const_iterator<> it = begin(); it != end(); ++it)
        result.push_back(it->first);
    return result;
}

ObjectMap::iterator<> ObjectMap::begin()
{ return begin<UniverseObject>(); }

ObjectMap::iterator<> ObjectMap::end()
{ return end<UniverseObject>(); }

ObjectMap::const_iterator<> ObjectMap::begin() const
{ return begin<UniverseObject>(); }

ObjectMap::const_iterator<> ObjectMap::end() const
{ return end<UniverseObject>(); }

ObjectMap::value_iterator<> ObjectMap::begin_values()
{ return begin_values<UniverseObject>(); }

ObjectMap::value_iterator<> ObjectMap::end_values()
{ return end_values<UniverseObject>(); }

ObjectMap::const_value_iterator<> ObjectMap::begin_values() const
{ return begin_values<UniverseObject>(); }

ObjectMap::const_value_iterator<> ObjectMap::end_values() const
{ return end_values<UniverseObject>(); }

UniverseObject* ObjectMap::Insert(UniverseObject* item) {
    if (!item)
        return 0;

    FOR_EACH_SPECIALIZED_MAP(TryInsertIntoMap, item);

    // check if an object is in the map already with specified id
    std::map<int, UniverseObject*>::iterator it = m_objects.find(item->ID());
    if (it == m_objects.end()) {
        // no pre-existing object was stored under specified id,
        // so just insert the new object
        m_objects[item->ID()] = item;
        return 0;
    }
    else {
        UniverseObject* old_item = it->second;  // pre-existing object is present. Need to get it and store it first...
        it->second = item;                      // and update map...
        return old_item;                        // and return old object for external handling
    }
}

UniverseObject* ObjectMap::Remove(int id) {
    // search for object in objects maps
    std::map<int, UniverseObject*>::iterator it = m_objects.find(id);
    if (it == m_objects.end())
        return 0;
    Logger().debugStream() << "Object was removed: " << it->second->Dump();
    // object found, so store pointer for later...
    UniverseObject* result = it->second;

    // and erase from pointer maps
    m_objects.erase(it);
    FOR_EACH_SPECIALIZED_MAP(EraseFromMap, id);

    return result;
}

void ObjectMap::Delete(int id)
{ delete Remove(id); }

void ObjectMap::Clear() {
    for (value_iterator<> it = begin_values(); it != end_values(); ++it)
        delete *it;
    FOR_EACH_MAP(ClearMap);
}

void ObjectMap::swap(ObjectMap& rhs) {
    FOR_EACH_MAP(SwapMap, rhs);
}

void ObjectMap::CopyObjectsToSpecializedMaps() {
    FOR_EACH_SPECIALIZED_MAP(ClearMap);
    for (value_iterator<> it = begin_values(); it != end_values(); ++it)
        FOR_EACH_SPECIALIZED_MAP(TryInsertIntoMap, *it);
}

std::string ObjectMap::Dump() const {
    std::ostringstream dump_stream;
    dump_stream << "ObjectMap contains UniverseObjects: " << std::endl;
    for (const_value_iterator<> it = begin_values(); it != end_values(); ++it)
        dump_stream << it->Dump() << std::endl;
    dump_stream << std::endl;
    return dump_stream.str();
}

// Static helpers

template<class T>
void ObjectMap::EraseFromMap(std::map<int, T*>& map, int id)
{ map.erase(id); }

template<class T>
void ObjectMap::ClearMap(std::map<int, T*>& map)
{ map.clear(); }

template<class T>
void ObjectMap::SwapMap(std::map<int, T*>& map, ObjectMap& rhs)
{ map.swap(rhs.Map<T>()); }

template <class T>
void ObjectMap::TryInsertIntoMap(std::map<int, T*>& map, UniverseObject* item) {
    if (T* t_item = dynamic_cast<T*>(item))
        map[item->ID()] = t_item;
}

// template specializations

template <>
const std::map<int, UniverseObject*>&  ObjectMap::Map() const
{ return m_objects; }

template <>
const std::map<int, ResourceCenter*>&  ObjectMap::Map() const
{ return m_resource_centers; }

template <>
const std::map<int, PopCenter*>&  ObjectMap::Map() const
{ return m_pop_centers; }

template <>
const std::map<int, Ship*>&  ObjectMap::Map() const
{ return m_ships; }

template <>
const std::map<int, Fleet*>&  ObjectMap::Map() const
{ return m_fleets; }

template <>
const std::map<int, Planet*>&  ObjectMap::Map() const
{ return m_planets; }

template <>
const std::map<int, System*>&  ObjectMap::Map() const
{ return m_systems; }

template <>
const std::map<int, Building*>&  ObjectMap::Map() const
{ return m_buildings; }

template <>
const std::map<int, Field*>&  ObjectMap::Map() const
{ return m_fields; }

template <>
std::map<int, UniverseObject*>&  ObjectMap::Map()
{ return m_objects; }

template <>
std::map<int, ResourceCenter*>&  ObjectMap::Map()
{ return m_resource_centers; }

template <>
std::map<int, PopCenter*>&  ObjectMap::Map()
{ return m_pop_centers; }

template <>
std::map<int, Ship*>&  ObjectMap::Map()
{ return m_ships; }

template <>
std::map<int, Fleet*>&  ObjectMap::Map()
{ return m_fleets; }

template <>
std::map<int, Planet*>&  ObjectMap::Map()
{ return m_planets; }

template <>
std::map<int, System*>&  ObjectMap::Map()
{ return m_systems; }

template <>
std::map<int, Building*>&  ObjectMap::Map()
{ return m_buildings; }

template <>
std::map<int, Field*>&  ObjectMap::Map()
{ return m_fields; }
