#include "ObjectMap.h"

#include "Universe.h"
#include "UniverseObject.h"
#include "Enums.h"
#include "../util/AppInterface.h"

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
    for (ObjectMap::const_iterator it = copied_map.const_begin(); it != copied_map.const_end(); ++it)
        this->Copy(it->second, empire_id);
}

void ObjectMap::Copy(const UniverseObject* obj, int empire_id/* = ALL_EMPIRES*/) {
    if (!obj)
        return;
    int object_id = obj->ID();

    // can empire see object at all?  if not, skip copying object's info
    if (GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id) <= VIS_NO_VISIBILITY)
        return;

    if (UniverseObject* copy_to_object = this->Object(object_id)) {
        copy_to_object->Copy(obj, empire_id);           // there already is a version of this object present in this ObjectMap, so just update it
    } else {
        UniverseObject* clone = obj->Clone(empire_id);  // this object is not yet present in this ObjectMap, so add a new UniverseObject object for it
        this->Insert(object_id, clone);
    }
}

void ObjectMap::CompleteCopyVisible(const ObjectMap& copied_map, int empire_id/* = ALL_EMPIRES*/) {
    if (&copied_map == this)
        return;

    // loop through objects in copied map, copying or cloning each depending
    // on whether there already is a corresponding object in this map
    for (ObjectMap::const_iterator it = copied_map.const_begin(); it != copied_map.const_end(); ++it) {
        int object_id = it->first;

        // can empire see object at all?  if not, skip copying object's info
        if (GetUniverse().GetObjectVisibilityByEmpire(object_id, empire_id) <= VIS_NO_VISIBILITY)
            continue;

        // if object is at all visible, copy all information, not just info
        // appropriate for the actual visibility level.  this ensures that any
        // details previously learned about object will still be recorded in
        // copied-to ObjectMap
        this->Copy(it->second, ALL_EMPIRES);
   }
}

ObjectMap* ObjectMap::Clone(int empire_id) const {
    ObjectMap* retval = new ObjectMap();
    retval->Copy(*this, empire_id);
    return retval;
}

int ObjectMap::NumObjects() const
{ return static_cast<int>(m_objects.size()); }

bool ObjectMap::Empty() const
{ return m_objects.empty(); }

const UniverseObject* ObjectMap::Object(int id) const {
    const_iterator it = m_const_objects.find(id);
    return (it != m_const_objects.end() ? it->second : 0);
}

UniverseObject* ObjectMap::Object(int id) {
    iterator it = m_objects.find(id);
    return (it != m_objects.end() ? it->second : 0);
}

std::vector<const UniverseObject*> ObjectMap::FindObjects(const std::vector<int>& object_ids) const {
    std::vector<const UniverseObject*> retval;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
        if (const UniverseObject* obj = Object(*it))
            retval.push_back(obj);
        else
            Logger().errorStream() << "ObjectMap::FindObjects couldn't find object with id " << *it;
    return retval;
}

std::vector<UniverseObject*> ObjectMap::FindObjects(const std::vector<int>& object_ids) {
    std::vector<UniverseObject*> retval;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it)
        if (UniverseObject* obj = Object(*it))
            retval.push_back(obj);
        else
            Logger().errorStream() << "ObjectMap::FindObjects couldn't find object with id " << *it;
    return retval;
}

std::vector<const UniverseObject*> ObjectMap::FindObjects(const UniverseObjectVisitor& visitor) const {
    std::vector<const UniverseObject*> retval;
    for (const_iterator it = m_const_objects.begin(); it != m_const_objects.end(); ++it) {
        if (UniverseObject* obj = it->second->Accept(visitor))
            retval.push_back(obj);
    }
    return retval;
}

std::vector<UniverseObject*> ObjectMap::FindObjects(const UniverseObjectVisitor& visitor) {
    std::vector<UniverseObject*> retval;
    for (iterator it = m_objects.begin(); it != m_objects.end(); ++it) {
        if (UniverseObject* obj = it->second->Accept(visitor))
            retval.push_back(obj);
    }
    return retval;
}

std::vector<int> ObjectMap::FindObjectIDs(const UniverseObjectVisitor& visitor) const {
    std::vector<int> retval;
    for (const_iterator it = m_const_objects.begin(); it != m_const_objects.end(); ++it) {
        if (it->second->Accept(visitor))
            retval.push_back(it->first);
    }
    return retval;
}

std::vector<int> ObjectMap::FindObjectIDs() const {
    std::vector<int> retval;
    for (const_iterator it = m_const_objects.begin(); it != m_const_objects.end(); ++it)
        retval.push_back(it->first);
    return retval;
}

ObjectMap::iterator ObjectMap::begin()
{ return m_objects.begin(); }

ObjectMap::iterator ObjectMap::end()
{ return m_objects.end(); }

ObjectMap::const_iterator ObjectMap::const_begin() const
{ return m_const_objects.begin(); }

ObjectMap::const_iterator ObjectMap::const_end() const
{ return m_const_objects.end(); }

UniverseObject* ObjectMap::Insert(int id, UniverseObject* obj) {
    // safety checks...
    if (!obj || id == UniverseObject::INVALID_OBJECT_ID)
        return 0;

    if (obj->ID() != id) {
        Logger().errorStream() << "ObjectMap::Insert passed object and id that doesn't match the object's id";
        obj->SetID(id);
    }

    // check if an object is in the map already with specified id
    std::map<int, UniverseObject*>::iterator it = m_objects.find(id);
    if (it == m_objects.end()) {
        // no pre-existing object was stored under specified id, so just insert
        // the new object
        m_objects[id] = obj;
        m_const_objects[id] = obj;
        return 0;
    }

    // pre-existing object is present.  need to get it and store it first...
    UniverseObject* old_obj = it->second;

    // and update maps
    it->second = obj;
    m_const_objects[id] = obj;

    // and return old object for external handling
    return old_obj;
}

UniverseObject* ObjectMap::Remove(int id) {
    // search for object in objects maps
    std::map<int, UniverseObject*>::iterator it = m_objects.find(id);
    if (it == m_objects.end())
        return 0;

    // object found, so store pointer for later...
    UniverseObject* retval = it->second;

    // and erase from pointer maps
    m_objects.erase(it);
    m_const_objects.erase(id);

    return retval;
}

void ObjectMap::Delete(int id)
{ delete Remove(id); }

void ObjectMap::Clear() {
    for (iterator it = m_objects.begin(); it != m_objects.end(); ++it)
        delete it->second;
    m_objects.clear();
    m_const_objects.clear();
}

void ObjectMap::swap(ObjectMap& rhs) {
    m_objects.swap(rhs.m_objects);
    m_const_objects.swap(rhs.m_const_objects);
}

void ObjectMap::CopyObjectsToConstObjects() {
    // remove existing entries in const objects and replace with values from non-const objects
    m_const_objects.clear();
    m_const_objects.insert(m_objects.begin(), m_objects.end());
}

std::string ObjectMap::Dump() const {
    std::ostringstream os;
    os << "ObjectMap contains UniverseObjects: " << std::endl;
    for (ObjectMap::const_iterator it = const_begin(); it != const_end(); ++it) {
        os << it->second->Dump() << std::endl;
    }
    os << std::endl;
    return os.str();
}

