// -*- C++ -*-
#ifndef _Object_Map_h_
#define _Object_Map_h_

#include <map>
#include <vector>
#include <string>

#include <boost/serialization/access.hpp>

class Universe;
struct UniverseObjectVisitor;

class UniverseObject;
class ResourceCenter;
class PopCenter;
class Ship;
class Fleet;
class Planet;
class System;
class Building;
class Field;

extern const int ALL_EMPIRES;
extern const int INVALID_OBJECT_ID;

/** Contains a set of objects that make up a (known or complete) Universe. */
class ObjectMap {
public:
    /* templated iterators. These allow us to iterate over a specific type
     * of UniverseObject, expose const iterators without a soring a separate
     * map of const pointers, and expose iterators over only the values stored
     * in the map. TODO: consider whether the iterator over key/value pairs is
     * really needed, or if the iterator over values is adequate, or if the
     * iterator over key/value pairs should only be used privately to lookup
     * objects by id */
    template <class T = UniverseObject>
    struct iterator : std::map<int, T*>::const_iterator {
        iterator(const typename  std::map<int, T*>::const_iterator& base) :
            std::map<int, T*>::const_iterator(base)
        {}
    };

    template <class T = UniverseObject>
    struct const_iterator : std::map<int, T*>::const_iterator {
        const_iterator(const typename  std::map<int, T*>::const_iterator& base) :
            std::map<int, T*>::const_iterator(base)
        {}

        const std::pair<const int, const T*>& operator *()
        { return std::map<int, T*>::const_iterator::operator*(); }

        const std::pair<const int, const T*>* operator ->() {
            const std::pair<const int, T*>* temp = &(std::map<int, T*>::const_iterator::operator*());
            // HACK: Can't const cast pair<A,B>* to pair<A,const B>* and can't
            // return the address of a temporary, so just tell the compiler
            // they are equivalent.
            const std::pair<const int, const T*>* retval =
                static_cast<const std::pair<const int, const T*>*>(
                    static_cast<const void*>(temp));
            return retval;
        }
    };

    template <class T = UniverseObject>
    struct value_iterator : std::map<int, T*>::iterator {
        value_iterator(const typename  std::map<int, T*>::iterator& base) :
            std::map<int, T*>::iterator(base)
        {}

        T* operator *()
        { return std::map<int, T*>::iterator::operator*().second; }

        T* operator ->()
        { return std::map<int, T*>::iterator::operator*().second; }
    };

    template <class T = UniverseObject>
    struct const_value_iterator : std::map<int, T*>::const_iterator {
        const_value_iterator(typename std::map<int, T*>::const_iterator base) :
            std::map<int, T*>::const_iterator(base)
        {}

        const T* operator *() const
        { return std::map<int, T*>::const_iterator::operator*().second; }

        const T* operator ->() const
        { return std::map<int, T*>::const_iterator::operator*().second; }
    };

    /** \name Structors */ //@{
    ObjectMap();            ///< default ctor
    ~ObjectMap();           ///< dtor

    /** Copies contents of this ObjectMap to a new ObjectMap, which is
      * returned.  Copies are limited to only duplicate information that the
      * empire with id \a empire_id would know about the copied objects. */
    ObjectMap*              Clone(int empire_id = ALL_EMPIRES) const;
    //@}

    /** \name Accessors */ //@{
    /** Returns number of objects in this ObjectMap */
    int                     NumObjects() const;
    template <class T>
    int                     NumObjects() const;

    /** Returns true if this ObjectMap contains no objects */
    bool                    Empty() const;

    /** Returns a pointer to the universe object with ID number \a id, or 0 if
      * none exists */
    const UniverseObject*   Object(int id) const;

    /** Returns a pointer to the universe object with ID number \a id, or 0 if
      * none exists */
    UniverseObject*         Object(int id);

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns 0 if none exists or the object with ID \a id is not of
      * type T. */
    template <class T>
    const T*                Object(int id) const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns 0 if none exists or the object with ID \a id is not of
      * type T */
    template <class T>
    T*                      Object(int id);

    /** Returns a vector containing the objects with ids in \a object_ids */
    std::vector<const UniverseObject*>  FindObjects(const std::vector<int>& object_ids) const;

    /** Returns a vector containing the objects with ids in \a object_ids */
    std::vector<UniverseObject*>        FindObjects(const std::vector<int>& object_ids);

    /** Returns all the objects that match \a visitor */
    std::vector<const UniverseObject*>  FindObjects(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects that match \a visitor */
    std::vector<UniverseObject*>        FindObjects(const UniverseObjectVisitor& visitor);

    /** Returns all the objects of type T */
    template <class T>
    std::vector<const T*>   FindObjects() const;

    /** Returns all the objects of type T */
    template <class T>
    std::vector<T*>         FindObjects();

    /** Returns the IDs of all the objects that match \a visitor */
    std::vector<int>        FindObjectIDs(const UniverseObjectVisitor& visitor) const;

    /** Returns the IDs of all the objects of type T */
    template <class T>
    std::vector<int>        FindObjectIDs() const;

    /** Returns the IDs of all objects in this ObjectMap */
    std::vector<int>        FindObjectIDs() const;

    /** iterators */
    // these first 8 are primarily for convenience
    iterator<>              begin();
    iterator<>              end();
    const_iterator<>        begin() const;
    const_iterator<>        end() const;
    value_iterator<>        begin_values();
    value_iterator<>        end_values();
    const_value_iterator<>  begin_values() const;
    const_value_iterator<>  end_values() const;

    template <class T>
    iterator<T>             begin();
    template <class T>
    iterator<T>             end();
    template <class T>
    const_iterator<T>       begin() const;
    template <class T>
    const_iterator<T>       end() const;
    template <class T>
    value_iterator<T>       begin_values();
    template <class T>
    value_iterator<T>       end_values();
    template <class T>
    const_value_iterator<T> begin_values() const;
    template <class T>
    const_value_iterator<T> end_values() const;

    std::string         Dump() const;
    //@}

    /** \name Mutators */ //@{

    /** Copies the contents of the ObjectMap \a copied_map into this ObjectMap.
      * Each object in \a copied_map has information transferred to this map.
      * If there already is a version of an object in \a copied_map in this map
      * then information is copied onto this map's version of the object using
      * the UniverseObject::Copy function.  If there is no corresponding object
      * in this map, a new object is created using the UinverseObject::Clone
      * function.  The copied objects are complete copies if \a empire_id is
      * ALL_EMPIRES, but if another \a empire_id is specified, the copied
      * information is limited by passing \a empire_id to are limited to the
      * Copy or Clone functions of the copied UniverseObjects.  Any objects
      * in this ObjectMap that have no corresponding object in \a copied_map
      * are left unchanged. */
    void                Copy(const ObjectMap& copied_map, int empire_id = ALL_EMPIRES);

    /** Copies the passed \a object into this ObjectMap, overwriting any
      * existing information about that object or creating a new object in this
      * map as appropriate with UniverseObject::Copy or UniverseObject::Clone.
      * The object is fully copied if \a empire_id is ALL_EMPIRES, but if
      * another empire id is specified, then the copied information is limited
      * by passing the visibility of the object by the empire specified by
      * \a empire_id to Copy or Clone of the object.  The passed object is
      * unchanged. */
    void                CopyObject(const UniverseObject* source, int empire_id = ALL_EMPIRES);

    /** Copies the objects of the ObjectMap \a copied_map that are visible to
      * the empire with id \a empire_id into this ObjectMap.  Copied objects
      * are complete copies of all information in \a copied_map about objects
      * that are visible, and no information about not-visible objects is
      * copied.  Any existing objects in this ObjectMap that are not visible to
      * the empire with id \a empire_id are left unchanged.  If \a empire_id is
      * ALL_EMPIRES, then all objects in \a copied_map are copied completely
      * and this function acts just like ObjectMap::Copy .*/
    void                CompleteCopyVisible(const ObjectMap& copied_map, int empire_id = ALL_EMPIRES);

    /** Adds object \a obj to the map under id \a id if id is a valid object id
      * and obj is an object with that id set.  If there already was an object
      * in the map with the id \a id then that object is first removed, and
      * is returned, otherwise 0 is returned. This ObjectMap takes ownership
      * of the passed UniverseObject. The caller takes ownership of any
      * returned UniverseObject. */
    UniverseObject*     Insert(UniverseObject* obj);

    /** Removes object with id \a id from map, and returns that object, if
      * there was an object under that ID in the map.  If no such object
      * existed in the map, 0 is returned and nothing is removed. The caller
      * takes ownership of any returned UniverseObject. */
    UniverseObject*     Remove(int id);

    /** Removes object with id \a id from map, and deletes that object, if
      * there was an object under that ID in the map.  If no such object
      * existed in the map, nothing is done. */
    void                Delete(int id);

    /** Empties map and deletes all objects within. */
    void                Clear();

    /** Swaps the contents of *this with \a rhs. */
    void                swap(ObjectMap& rhs);
    //@}

private:
    void                CopyObjectsToSpecializedMaps();
    template <class T>
    const std::map<int, T*>& Map() const;
    template <class T>
    std::map<int, T*>& Map();

    template<class T>
    static void         ClearMap(std::map<int, T*>& map);
    template <class T>
    static void         TryInsertIntoMap(std::map<int, T*>& map, UniverseObject* item);
    template <class T>
    static void         EraseFromMap(std::map<int, T*>& map, int id);
    template <class T>
    static void         SwapMap(std::map<int, T*>& map, ObjectMap& rhs);

    std::map<int, UniverseObject*>          m_objects;
    std::map<int, ResourceCenter*>          m_resource_centers;
    std::map<int, PopCenter*>               m_pop_centers;
    std::map<int, Ship*>                    m_ships;
    std::map<int, Fleet*>                   m_fleets;
    std::map<int, Planet*>                  m_planets;
    std::map<int, System*>                  m_systems;
    std::map<int, Building*>                m_buildings;
    std::map<int, Field*>                   m_fields;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
#if (10 * __GNUC__ + __GNUC_MINOR__ > 33) && (!defined _UniverseObject_h_)
#  include "UniverseObject.h"
#endif

template <class T>
ObjectMap::iterator<T> ObjectMap::begin()
{ return const_value_iterator<T>(Map<T>() .begin()); }

template <class T>
ObjectMap::iterator<T> ObjectMap::end()
{ return const_value_iterator<T>(Map<T>().end()); }

template <class T>
ObjectMap::const_iterator<T> ObjectMap::begin() const
{ return const_value_iterator<T>(Map<T>().begin()); }

template <class T>
ObjectMap::const_iterator<T> ObjectMap::end() const
{ return const_value_iterator<T>(Map<T>().end()); }

template <class T>
ObjectMap::value_iterator<T> ObjectMap::begin_values()
{ return value_iterator<T>(Map<T>().begin()); }

template <class T>
ObjectMap::value_iterator<T> ObjectMap::end_values()
{ return value_iterator<T>(Map<T>().end()); }

template <class T>
ObjectMap::const_value_iterator<T> ObjectMap::begin_values() const
{ return const_value_iterator<T>(Map<T>().begin()); }

template <class T>
ObjectMap::const_value_iterator<T> ObjectMap::end_values() const
{ return const_value_iterator<T>(Map<T>().end()); }

template <class T>
const T* ObjectMap::Object(int id) const {
    const_iterator<T> it = Map<T>().find(id);
    return (it != Map<T>().end() ? it->second : 0);
}

template <class T>
T* ObjectMap::Object(int id) {
    typename std::map<int, T*>::iterator it = Map<T>().find(id);
    return (it != Map<T>().end() ? it->second : 0);
}

template <class T>
std::vector<const T*> ObjectMap::FindObjects() const {
    std::vector<const T*> result;
    for (const_value_iterator<T> it = begin_values<T>(); it != end_values<T>(); ++it)
        result.push_back(*it);
    return result;
}

template <class T>
std::vector<T*> ObjectMap::FindObjects() {
    std::vector<T*> result;
    for (value_iterator<T> it = begin_values<T>(); it != end_values<T>(); ++it)
        result.push_back(*it);
    return result;
}

template <class T>
std::vector<int> ObjectMap::FindObjectIDs() const {
    std::vector<int> result;
    for (const_iterator<T> it = begin<T>(); it != end<T>(); ++it)
        result.push_back(it->first);
    return result;
}

template <class T>
int ObjectMap::NumObjects() const {
    return Map<T>().size();
}

// template specializations

template <>
const std::map<int, UniverseObject*>&  ObjectMap::Map() const;

template <>
const std::map<int, ResourceCenter*>&  ObjectMap::Map() const;

template <>
const std::map<int, PopCenter*>&  ObjectMap::Map() const;

template <>
const std::map<int, Ship*>&  ObjectMap::Map() const;

template <>
const std::map<int, Fleet*>&  ObjectMap::Map() const;

template <>
const std::map<int, Planet*>&  ObjectMap::Map() const;

template <>
const std::map<int, System*>&  ObjectMap::Map() const;

template <>
const std::map<int, Building*>&  ObjectMap::Map() const;

template <>
const std::map<int, Field*>&  ObjectMap::Map() const;

#endif
