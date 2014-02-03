// -*- C++ -*-
#ifndef _Object_Map_h_
#define _Object_Map_h_

#include <map>
#include <vector>
#include <set>
#include <string>

#include <boost/smart_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/type_traits/remove_const.hpp>

#include "../util/Export.h"
#include "TemporaryPtr.h"

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

FO_COMMON_API extern const int ALL_EMPIRES;
FO_COMMON_API extern const int TEMPORARY_OBJECT_ID;
FO_COMMON_API extern const int INVALID_OBJECT_ID;

/** Contains a set of objects that make up a (known or complete) Universe. */
class FO_COMMON_API ObjectMap {
public:

    template <class T = UniverseObject>
    struct iterator : private std::map<int, boost::shared_ptr<T> >::iterator {
        iterator(const typename std::map<int, boost::shared_ptr<T> >::iterator& base, ObjectMap& owner) :
            std::map<int, boost::shared_ptr<T> >::iterator(base),
            m_owner(owner)
        { Refresh(); }

        TemporaryPtr<T> operator *() const
        { return m_current_ptr; }

        // The result of this operator is not intended to be stored, so it's safe to
        // return a reference to an instance variable that's going to be soon overwritten.
        TemporaryPtr<T>& operator ->() const
        { return m_current_ptr; }

        iterator& operator ++() {
            std::map<int, boost::shared_ptr<T> >::iterator::operator++();
            Refresh();
            return *this;
        }

        iterator operator ++(int) {
            iterator result = iterator(std::map<int, boost::shared_ptr<T> >::iterator::operator++(0), m_owner);
            Refresh();
            return result;
        }

        iterator& operator --() {
            std::map<int, boost::shared_ptr<T> >::iterator::operator--();
            Refresh();
            return *this;
        }

        iterator operator --(int) {
            iterator result = iterator(std::map<int, boost::shared_ptr<T> >::iterator::operator--(0), m_owner);
            Refresh();
            return result;
        }

        bool operator ==(const iterator& other) const
        { return std::map<int, boost::shared_ptr<T> >::iterator::operator ==(other); }

        bool operator !=(const iterator& other) const
        { return std::map<int, boost::shared_ptr<T> >::iterator::operator !=(other); }

    private:
        // 
        mutable TemporaryPtr<T> m_current_ptr;
        ObjectMap& m_owner;

        // We always want m_current_ptr to be pointing to our parent iterator's
        // current item, if it is a valid object. Otherwise, we just want to
        // return a "null" pointer.  We assume that we are dealing with valid
        // iterators in the range [begin(), end()].
        void Refresh() const {
            if (std::map<int, boost::shared_ptr<T> >::iterator::operator ==(m_owner.Map<T>().end())) {
                m_current_ptr = TemporaryPtr<T>();
            } else {
                m_current_ptr = TemporaryPtr<T>(std::map<int, boost::shared_ptr<T> >::iterator::operator*().second);
            }
        }
    };

    template <class T = UniverseObject>
    struct const_iterator : private std::map<int, boost::shared_ptr<T> >::const_iterator {
        const_iterator(const typename std::map<int, boost::shared_ptr<T> >::const_iterator& base, const ObjectMap& owner) :
            std::map<int, boost::shared_ptr<T> >::const_iterator(base),
            m_owner(owner)
        { Refresh(); }

        TemporaryPtr<const T> operator *() const
        { return m_current_ptr; }
        
        // The result of this operator is not intended to be stored, so it's safe to
        // return a reference to an instance variable that's going to be soon overwritten.
        TemporaryPtr<const T>& operator ->() const
        { return m_current_ptr; }

        const_iterator& operator ++() {
            std::map<int, boost::shared_ptr<T> >::const_iterator::operator++();
            Refresh();
            return *this;
        }

        const_iterator operator ++(int) {
            const_iterator result = std::map<int, boost::shared_ptr<T> >::const_iterator::operator++(0);
            Refresh();
            return result;
        }

        const_iterator& operator --() {
            std::map<int, boost::shared_ptr<T> >::const_iterator::operator--();
            Refresh();
            return *this;
        }

        const_iterator operator --(int) {
            const_iterator result = std::map<int, boost::shared_ptr<T> >::const_iterator::operator--(0);
            Refresh();
            return result;
        }

        bool operator ==(const const_iterator& other) const
        { return std::map<int, boost::shared_ptr<T> >::const_iterator::operator ==(other); }

        bool operator !=(const const_iterator& other) const
        { return std::map<int, boost::shared_ptr<T> >::const_iterator::operator !=(other); }

    private:
        // See iterator for comments.
        mutable TemporaryPtr<const T> m_current_ptr;
        const ObjectMap& m_owner;

        // We always want m_current_ptr to be pointing to our parent iterator's current item, if it is a valid object.
        // Otherwise, we just want to return a "null" pointer.  We assume that we are dealing with valid iterators in
        // the range [begin(), end()].
        void Refresh() const {
            if (std::map<int, boost::shared_ptr<T> >::const_iterator::operator ==(m_owner.Map<T>().end())) {
                m_current_ptr = TemporaryPtr<T>();
            } else {
                m_current_ptr = TemporaryPtr<T>(std::map<int, boost::shared_ptr<T> >::const_iterator::operator*().second);
            }
        }
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

    /** Returns the number of objects of the specified class in this ObjectMap. */
    template <class T>
    int                     NumObjects() const;

    /** Returns true if this ObjectMap contains no objects */
    bool                    Empty() const;

    /** Returns a pointer to the universe object with ID number \a id,
      * or a null TemporaryPtr if none exists */
    TemporaryPtr<const UniverseObject>  Object(int id) const;

    /** Returns a pointer to the universe object with ID number \a id,
      * or a null TemporaryPtr if none exists */
    TemporaryPtr<UniverseObject>        Object(int id);

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null TemporaryPtr if none exists or the object with
      * ID \a id is not of type T. */
    template <class T>
    TemporaryPtr<const T>               Object(int id) const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a nullTemporaryPtr if none exists or the object with
      * ID \a id is not of type T. */
    template <class T>
    TemporaryPtr<T>                     Object(int id);

    /** Returns a vector containing the objects with ids in \a object_ids */
    std::vector<TemporaryPtr<const UniverseObject> >    FindObjects(const std::vector<int>& object_ids) const;

    /** Returns a vector containing the objects with ids in \a object_ids */
    std::vector<TemporaryPtr<UniverseObject> >          FindObjects(const std::vector<int>& object_ids);

    /** Returns a vector containing the objects with ids in \a object_ids that
      * are of type T */
    template <class T>
    std::vector<TemporaryPtr<const T> >    FindObjects(const std::vector<int>& object_ids) const;

    template <class T>
    std::vector<TemporaryPtr<const T> >    FindObjects(const std::set<int>& object_ids) const;

    /** Returns a vector containing the objects with ids in \a object_ids that
      * are of type T */
    template <class T>
    std::vector<TemporaryPtr<T> >          FindObjects(const std::vector<int>& object_ids);

    template <class T>
    std::vector<TemporaryPtr<T> >          FindObjects(const std::set<int>& object_ids);

    /** Returns all the objects that match \a visitor */
    std::vector<TemporaryPtr<const UniverseObject> >    FindObjects(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects that match \a visitor */
    std::vector<TemporaryPtr<UniverseObject> >          FindObjects(const UniverseObjectVisitor& visitor);

    /** Returns all the objects of type T */
    template <class T>
    std::vector<TemporaryPtr<const T> >     FindObjects() const;

    /** Returns all the objects of type T */
    template <class T>
    std::vector<TemporaryPtr<T> >           FindObjects();

    /** Returns the IDs of all the objects that match \a visitor */
    std::vector<int>        FindObjectIDs(const UniverseObjectVisitor& visitor) const;

    /** Returns the IDs of all the objects of type T */
    template <class T>
    std::vector<int>        FindObjectIDs() const;

    /** Returns the IDs of all objects not known to have been destroyed. */
    std::vector<int>        FindExistingObjectIDs() const;

    /** Returns the IDs of all objects in this ObjectMap */
    std::vector<int>        FindObjectIDs() const;

    /** iterators */
    // these first 4 are primarily for convenience
    iterator<>              begin();
    iterator<>              end();
    const_iterator<>        const_begin() const;
    const_iterator<>        const_end() const;

    template <class T>
    iterator<T>             begin();
    template <class T>
    iterator<T>             end();
    template <class T>
    const_iterator<T>       const_begin() const;
    template <class T>
    const_iterator<T>       const_end() const;

    std::string             Dump() const;

    /**  */
    TemporaryPtr<UniverseObject> ExistingObject(int id);
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingObjectsBegin()
    { return m_existing_objects.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingObjectsEnd()
    { return m_existing_objects.end(); }
    int NumExistingObjects()
    {return m_existing_objects.size(); }

    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingResourceCentersBegin()
    { return m_existing_resource_centers.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingResourceCentersEnd()
    { return m_existing_resource_centers.end(); }
    int NumExistingResourceCenters()
    {return m_existing_resource_centers.size(); }

    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingPopCentersBegin()
    { return m_existing_pop_centers.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingPopCentersEnd()
    { return m_existing_pop_centers.end(); }
    int NumExistingPopCenters()
    {return m_existing_pop_centers.size(); }

    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingShipsBegin()
    { return m_existing_ships.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingShipsEnd()
    { return m_existing_ships.end(); }
    int NumExistingShips()
    {return m_existing_ships.size(); }

    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingFleetsBegin()
    { return m_existing_fleets.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingFleetsEnd()
    { return m_existing_fleets.end(); }
    int NumExistingFleets()
    {return m_existing_fleets.size(); }

    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingPlanetsBegin()
    { return m_existing_planets.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingPlanetsEnd()
    { return m_existing_planets.end(); }
    int NumExistingPlanets()
    {return m_existing_planets.size(); }

    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingSystemsBegin()
    { return m_existing_systems.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingSystemsEnd()
    { return m_existing_systems.end(); }
    int NumExistingSystems()
    {return m_existing_systems.size(); }

    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingBuildingsBegin()
    { return m_existing_buildings.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingBuildingsEnd()
    { return m_existing_buildings.end(); }
    int NumExistingBuildings()
    {return m_existing_buildings.size(); }

    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingFieldsBegin()
    { return m_existing_fields.begin(); }
    std::map<int, TemporaryPtr<UniverseObject> >::iterator ExistingFieldsEnd()
    { return m_existing_fields.end(); }
    int NumExistingFields()
    {return m_existing_fields.size(); }

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

    /** Copies the contents of the ObjectMap \a copied_map into this ObjectMap, in
     * preparation for serializing this ObjectMap.  The normal object-by-object 
     * CopyObject process is bypassed and only m_objects is copied, in a direct fashion. */
    void                CopyForSerialize(const ObjectMap& copied_map);

    /** Copies the passed \a object into this ObjectMap, overwriting any
      * existing information about that object or creating a new object in this
      * map as appropriate with UniverseObject::Copy or UniverseObject::Clone.
      * The object is fully copied if \a empire_id is ALL_EMPIRES, but if
      * another empire id is specified, then the copied information is limited
      * by passing the visibility of the object by the empire specified by
      * \a empire_id to Copy or Clone of the object.  The passed object is
      * unchanged. */
    void                CopyObject(TemporaryPtr<const UniverseObject> source, int empire_id = ALL_EMPIRES);

    /** Adds object \a obj to the map under its ID, if it is a valid object.
      * If there already was an object in the map with the id \a id then
      * that object will be removed.  A TemporaryPtr to the new object is
      * returned. */
    template <class T>
    TemporaryPtr<T>     Insert(T* obj, int empire_id = ALL_EMPIRES);

    /** Adds object \a obj to the map under its ID, if it is a valid object. */
    template <class T>
    void                Insert(TemporaryPtr<T> obj, int empire_id = ALL_EMPIRES);

    /** Removes object with id \a id from map, and returns that object, if
      * there was an object under that ID in the map.  If no such object
      * existed in the map, a null shared_ptr is returned and nothing is
      * removed. The ObjectMap will no longer share ownership of the
      * returned object. */
    boost::shared_ptr<UniverseObject>     Remove(int id);

    /** Empties map, removing shared ownership by this map of all
      * previously contained objects. */
    void                Clear();

    /** Swaps the contents of *this with \a rhs. */
    void                swap(ObjectMap& rhs);

    /** */
    void                UpdateCurrentDestroyedObjects(std::set<int> destroyed_object_ids);

    /** Recalculates contained objects for all objects in this ObjectMap based
      * on what other objects exist in this ObjectMap. Useful to eliminate
      * cases where there are inconsistencies between whan an object thinks it
      * contains, and what other objects think they are contained by the first
      * object. */
    void                AuditContainment();
    //@}

private:
    void                Insert(boost::shared_ptr<UniverseObject> item, int empire_id = ALL_EMPIRES);
    void                CopyObjectsToSpecializedMaps();
    template <class T>
    const std::map<int, boost::shared_ptr<T> >& Map() const;
    template <class T>
    std::map<int, boost::shared_ptr<T> >& Map();

    template<class T>
    static void         ClearMap(std::map<int, boost::shared_ptr<T> >& map);
    template <class T>
    static void         TryInsertIntoMap(std::map<int, boost::shared_ptr<T> >& map, boost::shared_ptr<UniverseObject> item);
    template <class T>
    static void         EraseFromMap(std::map<int, boost::shared_ptr<T> >& map, int id);
    template <class T>
    static void         SwapMap(std::map<int, boost::shared_ptr<T> >& map, ObjectMap& rhs);

    std::map<int, boost::shared_ptr<UniverseObject> >           m_objects;
    std::map<int, boost::shared_ptr<ResourceCenter> >           m_resource_centers;
    std::map<int, boost::shared_ptr<PopCenter> >                m_pop_centers;
    std::map<int, boost::shared_ptr<Ship> >                     m_ships;
    std::map<int, boost::shared_ptr<Fleet> >                    m_fleets;
    std::map<int, boost::shared_ptr<Planet> >                   m_planets;
    std::map<int, boost::shared_ptr<System> >                   m_systems;
    std::map<int, boost::shared_ptr<Building> >                 m_buildings;
    std::map<int, boost::shared_ptr<Field> >                    m_fields;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_objects;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_resource_centers;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_pop_centers;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_ships;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_fleets;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_planets;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_systems;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_buildings;
    std::map<int, TemporaryPtr<UniverseObject> >                m_existing_fields;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

template <class T>
ObjectMap::iterator<T> ObjectMap::begin()
{ return iterator<T>(Map<typename boost::remove_const<T>::type>().begin(), *this); }

template <class T>
ObjectMap::iterator<T> ObjectMap::end()
{ return iterator<T>(Map<typename boost::remove_const<T>::type>().end(), *this); }

template <class T>
ObjectMap::const_iterator<T> ObjectMap::const_begin() const
{ return const_iterator<T>(Map<typename boost::remove_const<T>::type>().begin(), *this); }

template <class T>
ObjectMap::const_iterator<T> ObjectMap::const_end() const
{ return const_iterator<T>(Map<typename boost::remove_const<T>::type>().end(), *this); }

template <class T>
TemporaryPtr<const T> ObjectMap::Object(int id) const {
    typename std::map<int, boost::shared_ptr<typename boost::remove_const<T>::type> >::const_iterator it =
        Map<typename boost::remove_const<T>::type>().find(id);
    return TemporaryPtr<const T>(
        it != Map<typename boost::remove_const<T>::type>().end()
            ? it->second
            : boost::shared_ptr<const T>());
}

template <class T>
TemporaryPtr<T> ObjectMap::Object(int id) {
    typename std::map<int, boost::shared_ptr<typename boost::remove_const<T>::type> >::iterator it =
        Map<typename boost::remove_const<T>::type>().find(id);
    return TemporaryPtr<T>(
        it != Map<typename boost::remove_const<T>::type>().end()
            ? it->second
            : boost::shared_ptr<typename boost::remove_const<T>::type>()
        );
}

template <class T>
std::vector<TemporaryPtr<const T> > ObjectMap::FindObjects() const {
    std::vector<TemporaryPtr<const T> > result;
    for (const_iterator<T> it = const_begin<T>(); it != const_end<T>(); ++it)
        result.push_back(*it);
    return result;
}

template <class T>
std::vector<TemporaryPtr<T> > ObjectMap::FindObjects() {
    std::vector<TemporaryPtr<T> > result;
    for (iterator<T> it = begin<T>(); it != end<T>(); ++it)
        result.push_back(*it);
    return result;
}

template <class T>
std::vector<int> ObjectMap::FindObjectIDs() const {
    std::vector<int> result;
    for (typename std::map<int, boost::shared_ptr<T> >::const_iterator
         it = Map<typename boost::remove_const<T>::type>().begin();
         it != Map<typename boost::remove_const<T>::type>().end(); ++it)
    { result.push_back(it->first); }
    return result;
}

template <class T>
std::vector<TemporaryPtr<const T> > ObjectMap::FindObjects(const std::vector<int>& object_ids) const {
    std::vector<TemporaryPtr<const T> > retval;
    typedef typename boost::remove_const<T>::type mutableT;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it) {
        typename std::map<int, boost::shared_ptr<mutableT> >::const_iterator map_it = Map<mutableT>().find(*it);
        if (map_it != Map<mutableT>().end())
            retval.push_back(TemporaryPtr<const T>(map_it->second));
    }
    return retval;
}

template <class T>
std::vector<TemporaryPtr<const T> > ObjectMap::FindObjects(const std::set<int>& object_ids) const {
    std::vector<TemporaryPtr<const T> > retval;
    typedef typename boost::remove_const<T>::type mutableT;
    for (std::set<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it) {
        typename std::map<int, boost::shared_ptr<mutableT> >::const_iterator map_it = Map<mutableT>().find(*it);
        if (map_it != Map<mutableT>().end())
            retval.push_back(TemporaryPtr<const T>(map_it->second));
    }
    return retval;
}

template <class T>
std::vector<TemporaryPtr<T> > ObjectMap::FindObjects(const std::vector<int>& object_ids) {
    std::vector<TemporaryPtr<T> > retval;
    typedef typename boost::remove_const<T>::type mutableT;
    for (std::vector<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it) {
        typename std::map<int, boost::shared_ptr<mutableT> >::const_iterator map_it = Map<mutableT>().find(*it);
        if (map_it != Map<mutableT>().end())
            retval.push_back(TemporaryPtr<T>(map_it->second));
    }
    return retval;
}

template <class T>
std::vector<TemporaryPtr<T> > ObjectMap::FindObjects(const std::set<int>& object_ids) {
    std::vector<TemporaryPtr<T> > retval;
    typedef typename boost::remove_const<T>::type mutableT;
    for (std::set<int>::const_iterator it = object_ids.begin(); it != object_ids.end(); ++it) {
        typename std::map<int, boost::shared_ptr<mutableT> >::const_iterator map_it = Map<mutableT>().find(*it);
        if (map_it != Map<mutableT>().end())
            retval.push_back(TemporaryPtr<T>(map_it->second));
    }
    return retval;
}

template <class T>
int ObjectMap::NumObjects() const
{ return Map<typename boost::remove_const<T>::type>().size(); }

template <class T>
TemporaryPtr<T> ObjectMap::Insert(T* item, int empire_id /* = ALL_EMPIRES */) {
    if (!item)
        return TemporaryPtr<T>();

    boost::shared_ptr<T> shared_item(item);
    Insert(shared_item, empire_id);
    return TemporaryPtr<T>(shared_item);
}

template <class T>
void ObjectMap::Insert(TemporaryPtr<T> item, int empire_id /* = ALL_EMPIRES */) {
    if (!item)
        return;
    Insert(boost::dynamic_pointer_cast<UniverseObject>(item.m_ptr), empire_id);
}

// template specializations

template <>
const std::map<int, boost::shared_ptr<UniverseObject> >&  ObjectMap::Map() const;

template <>
const std::map<int, boost::shared_ptr<ResourceCenter> >&  ObjectMap::Map() const;

template <>
const std::map<int, boost::shared_ptr<PopCenter> >&  ObjectMap::Map() const;

template <>
const std::map<int, boost::shared_ptr<Ship> >&  ObjectMap::Map() const;

template <>
const std::map<int, boost::shared_ptr<Fleet> >&  ObjectMap::Map() const;

template <>
const std::map<int, boost::shared_ptr<Planet> >&  ObjectMap::Map() const;

template <>
const std::map<int, boost::shared_ptr<System> >&  ObjectMap::Map() const;

template <>
const std::map<int, boost::shared_ptr<Building> >&  ObjectMap::Map() const;

template <>
const std::map<int, boost::shared_ptr<Field> >&  ObjectMap::Map() const;

template <>
std::map<int, boost::shared_ptr<UniverseObject> >&  ObjectMap::Map();

template <>
std::map<int, boost::shared_ptr<ResourceCenter> >&  ObjectMap::Map();

template <>
std::map<int, boost::shared_ptr<PopCenter> >&  ObjectMap::Map();

template <>
std::map<int, boost::shared_ptr<Ship> >&  ObjectMap::Map();

template <>
std::map<int, boost::shared_ptr<Fleet> >&  ObjectMap::Map();

template <>
std::map<int, boost::shared_ptr<Planet> >&  ObjectMap::Map();

template <>
std::map<int, boost::shared_ptr<System> >&  ObjectMap::Map();

template <>
std::map<int, boost::shared_ptr<Building> >&  ObjectMap::Map();

template <>
std::map<int, boost::shared_ptr<Field> >&  ObjectMap::Map();

#endif
