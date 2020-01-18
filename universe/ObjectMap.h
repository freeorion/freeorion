#ifndef _Object_Map_h_
#define _Object_Map_h_


#include <boost/serialization/access.hpp>

#include "../util/Export.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <type_traits>


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


/** Contains a set of objects that make up a (known or complete) Universe. */
class FO_COMMON_API ObjectMap {
public:
    template <typename T>
    using container_type = std::map<int, std::shared_ptr<T>>;

    template <typename T>
    struct iterator : private container_type<T>::iterator {
        iterator(const typename container_type<T>::iterator& base, ObjectMap& owner) :
            container_type<T>::iterator(base),
            m_owner(owner)
        { Refresh(); }

        std::shared_ptr<T> operator *() const
        { return m_current_ptr; }

        // The result of this operator is not intended to be stored, so it's safe to
        // return a reference to an instance variable that's going to be soon overwritten.
        std::shared_ptr<T>& operator ->() const
        { return m_current_ptr; }

        iterator& operator ++() {
            container_type<T>::iterator::operator++();
            Refresh();
            return *this;
        }

        iterator operator ++(int) {
            iterator result = iterator(container_type<T>::iterator::operator++(0), m_owner);
            Refresh();
            return result;
        }

        iterator& operator --() {
            container_type<T>::iterator::operator--();
            Refresh();
            return *this;
        }

        iterator operator --(int) {
            iterator result = iterator(container_type<T>::iterator::operator--(0), m_owner);
            Refresh();
            return result;
        }

        bool operator ==(const iterator& other) const
        { return (typename container_type<T>::iterator(*this) == other); }

        bool operator !=(const iterator& other) const
        { return (typename container_type<T>::iterator(*this) != other);}

    private:
        mutable std::shared_ptr<T> m_current_ptr;
        ObjectMap& m_owner;

        // We always want m_current_ptr to be pointing to our parent iterator's
        // current item, if it is a valid object. Otherwise, we just want to
        // return a "null" pointer.  We assume that we are dealing with valid
        // iterators in the range [begin(), end()].
        void Refresh() const {
            if (typename container_type<T>::iterator(*this) == (m_owner.Map<T>().end())) {
                m_current_ptr = nullptr;
            } else {
                m_current_ptr = std::shared_ptr<T>(container_type<T>::iterator::operator*().second);
            }
        }
    };

    template <typename T>
    struct const_iterator : private container_type<T>::const_iterator {
        const_iterator(const typename container_type<T>::const_iterator& base,
                       const ObjectMap& owner) :
            container_type<T>::const_iterator(base),
            m_owner(owner)
        { Refresh(); }

        std::shared_ptr<const T> operator *() const
        { return m_current_ptr; }

        // The result of this operator is not intended to be stored, so it's safe to
        // return a reference to an instance variable that's going to be soon overwritten.
        std::shared_ptr<const T>& operator ->() const
        { return m_current_ptr; }

        const_iterator& operator ++() {
            container_type<T>::const_iterator::operator++();
            Refresh();
            return *this;
        }

        const_iterator operator ++(int) {
            const_iterator result = container_type<T>::const_iterator::operator++(0);
            Refresh();
            return result;
        }

        const_iterator& operator --() {
            container_type<T>::const_iterator::operator--();
            Refresh();
            return *this;
        }

        const_iterator operator --(int) {
            const_iterator result = container_type<T>::const_iterator::operator--(0);
            Refresh();
            return result;
        }

        bool operator ==(const const_iterator& other) const
        { return (typename container_type<T>::const_iterator(*this) == other); }

        bool operator !=(const const_iterator& other) const
        { return (typename container_type<T>::const_iterator(*this) != other); }

    private:
        // See iterator for comments.
        mutable std::shared_ptr<const T> m_current_ptr;
        const ObjectMap& m_owner;

        // We always want m_current_ptr to be pointing to our parent iterator's current item, if it is a valid object.
        // Otherwise, we just want to return a "null" pointer.  We assume that we are dealing with valid iterators in
        // the range [begin(), end()].
        void Refresh() const {
            if (typename container_type<T>::const_iterator(*this) == (m_owner.Map<T>().end())) {
                m_current_ptr = nullptr;
            } else {
                m_current_ptr = std::shared_ptr<T>(container_type<T>::const_iterator::operator*().second);
            }
        }
    };

    /** \name Structors */ //@{
    ObjectMap();
    ~ObjectMap();

    /** Copies contents of this ObjectMap to a new ObjectMap, which is
      * returned.  Copies are limited to only duplicate information that the
      * empire with id \a empire_id would know about the copied objects. */
    ObjectMap* Clone(int empire_id = ALL_EMPIRES) const;
    //@}

    /** \name Accessors */ //@{
    /** Returns number of objects in this ObjectMap */
    int NumObjects() const;

    /** Returns the number of objects of the specified class in this ObjectMap. */
    template <class T>
    int NumObjects() const;

    /** Returns true if this ObjectMap contains no objects */
    bool Empty() const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <class T = UniverseObject>
    std::shared_ptr<const T> Object(int id) const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <class T = UniverseObject>
    std::shared_ptr<T> Object(int id);

    /** Returns a vector containing the objects with ids in \a object_ids that
      * are of type T */
    template <class T = UniverseObject>
    std::vector<std::shared_ptr<const T>> FindObjects(const std::vector<int>& object_ids) const;

    template <class T = UniverseObject>
    std::vector<std::shared_ptr<const T>> FindObjects(const std::set<int>& object_ids) const;

    /** Returns a vector containing the objects with ids in \a object_ids that
      * are of type T */
    template <class T = UniverseObject>
    std::vector<std::shared_ptr<T>> FindObjects(const std::vector<int>& object_ids);

    template <class T = UniverseObject>
    std::vector<std::shared_ptr<T>> FindObjects(const std::set<int>& object_ids);

    /** Returns all the objects that match \a visitor */
    std::vector<std::shared_ptr<const UniverseObject>> FindObjects(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects that match \a visitor */
    std::vector<std::shared_ptr<UniverseObject>> FindObjects(const UniverseObjectVisitor& visitor);

    /** Returns all the objects of type T */
    template <class T = UniverseObject>
    std::vector<std::shared_ptr<const T>> FindObjects() const;

    /** Returns all the objects of type T */
    template <class T = UniverseObject>
    std::vector<std::shared_ptr<T>> FindObjects();

    /** Returns the IDs of all the objects that match \a visitor */
    std::vector<int>        FindObjectIDs(const UniverseObjectVisitor& visitor) const;

    /** Returns the IDs of all the objects of type T */
    template <class T>
    std::vector<int>        FindObjectIDs() const;

    /** Returns the IDs of all objects not known to have been destroyed. */
    std::vector<int>        FindExistingObjectIDs() const;

    /** Returns the IDs of all objects in this ObjectMap */
    std::vector<int>        FindObjectIDs() const;

    /** Returns highest used object ID in this ObjectMap */
    int                     HighestObjectID() const;

    /** iterators */
    template <class T = UniverseObject>
    iterator<T>             begin();
    template <class T = UniverseObject>
    iterator<T>             end();
    template <class T = UniverseObject>
    const_iterator<T>       begin() const;
    template <class T = UniverseObject>
    const_iterator<T>       end() const;

    std::string             Dump(unsigned short ntabs = 0) const;

    /**  */
    std::shared_ptr<UniverseObject> ExistingObject(int id);
    const container_type<UniverseObject>& ExistingObjects()
    { return m_existing_objects; }
    const container_type<UniverseObject>& ExistingResourceCenters()
    { return m_existing_resource_centers; }
    const container_type<UniverseObject>& ExistingPopCenters()
    { return m_existing_pop_centers; }
    const container_type<UniverseObject>& ExistingShips()
    { return m_existing_ships; }
    const container_type<UniverseObject>& ExistingFleets()
    { return m_existing_fleets; }
    const container_type<UniverseObject>& ExistingPlanets()
    { return m_existing_planets; }
    const container_type<UniverseObject>& ExistingSystems()
    { return m_existing_systems; }
    const container_type<UniverseObject>& ExistingBuildings()
    { return m_existing_buildings; }
    const container_type<UniverseObject>& ExistingFields()
    { return m_existing_fields; }

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
    void Copy(const ObjectMap& copied_map, int empire_id = ALL_EMPIRES);

    /** Copies the contents of the ObjectMap \a copied_map into this ObjectMap, in
     * preparation for serializing this ObjectMap.  The normal object-by-object 
     * CopyObject process is bypassed and only m_objects is copied, in a direct fashion. */
    void CopyForSerialize(const ObjectMap& copied_map);

    /** Copies the passed \a object into this ObjectMap, overwriting any
      * existing information about that object or creating a new object in this
      * map as appropriate with UniverseObject::Copy or UniverseObject::Clone.
      * The object is fully copied if \a empire_id is ALL_EMPIRES, but if
      * another empire id is specified, then the copied information is limited
      * by passing the visibility of the object by the empire specified by
      * \a empire_id to Copy or Clone of the object.  The passed object is
      * unchanged. */
    void CopyObject(std::shared_ptr<const UniverseObject> source, int empire_id = ALL_EMPIRES);

    /** Adds object \a obj to the map under its ID, if it is a valid object.
      * If there already was an object in the map with the id \a id then
      * that object will be removed. */
    template <class T>
    void Insert(std::shared_ptr<T> obj, int empire_id = ALL_EMPIRES);

    /** Removes object with id \a id from map, and returns that object, if
      * there was an object under that ID in the map.  If no such object
      * existed in the map, a null shared_ptr is returned and nothing is
      * removed. The ObjectMap will no longer share ownership of the
      * returned object. */
    std::shared_ptr<UniverseObject> Remove(int id);

    /** Empties map, removing shared ownership by this map of all
      * previously contained objects. */
    void Clear();

    /** Swaps the contents of *this with \a rhs. */
    void swap(ObjectMap& rhs);

    /** */
    void UpdateCurrentDestroyedObjects(const std::set<int>& destroyed_object_ids);

    /** Recalculates contained objects for all objects in this ObjectMap based
      * on what other objects exist in this ObjectMap. Useful to eliminate
      * cases where there are inconsistencies between whan an object thinks it
      * contains, and what other objects think they are contained by the first
      * object. */
    void AuditContainment(const std::set<int>& destroyed_object_ids);
    //@}

private:
    void InsertCore(std::shared_ptr<UniverseObject> item, int empire_id = ALL_EMPIRES);

    void CopyObjectsToSpecializedMaps();

    template <class T>
    const container_type<T>& Map() const;

    template <class T>
    container_type<T>& Map();

    template <class T>
    static void SwapMap(container_type<T>& map, ObjectMap& rhs);

    container_type<UniverseObject> m_objects;
    container_type<ResourceCenter> m_resource_centers;
    container_type<PopCenter> m_pop_centers;
    container_type<Ship> m_ships;
    container_type<Fleet> m_fleets;
    container_type<Planet> m_planets;
    container_type<System> m_systems;
    container_type<Building> m_buildings;
    container_type<Field> m_fields;

    container_type<UniverseObject> m_existing_objects;
    container_type<UniverseObject> m_existing_resource_centers;
    container_type<UniverseObject> m_existing_pop_centers;
    container_type<UniverseObject> m_existing_ships;
    container_type<UniverseObject> m_existing_fleets;
    container_type<UniverseObject> m_existing_planets;
    container_type<UniverseObject> m_existing_systems;
    container_type<UniverseObject> m_existing_buildings;
    container_type<UniverseObject> m_existing_fields;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

template <class T>
ObjectMap::iterator<T> ObjectMap::begin()
{ return iterator<T>(Map<typename std::remove_const<T>::type>().begin(), *this); }

template <class T>
ObjectMap::iterator<T> ObjectMap::end()
{ return iterator<T>(Map<typename std::remove_const<T>::type>().end(), *this); }

template <class T>
ObjectMap::const_iterator<T> ObjectMap::begin() const
{ return const_iterator<T>(Map<typename std::remove_const<T>::type>().begin(), *this); }

template <class T>
ObjectMap::const_iterator<T> ObjectMap::end() const
{ return const_iterator<T>(Map<typename std::remove_const<T>::type>().end(), *this); }

template <class T>
std::shared_ptr<const T> ObjectMap::Object(int id) const {
    auto it = Map<typename std::remove_const<T>::type>().find(id);
    return std::shared_ptr<const T>(
        it != Map<typename std::remove_const<T>::type>().end()
            ? it->second
            : nullptr);
}

template <class T>
std::shared_ptr<T> ObjectMap::Object(int id) {
    auto it = Map<typename std::remove_const<T>::type>().find(id);
    return std::shared_ptr<T>(
        it != Map<typename std::remove_const<T>::type>().end()
            ? it->second
            : nullptr);
}

template <class T>
std::vector<std::shared_ptr<const T>> ObjectMap::FindObjects() const {
    std::vector<std::shared_ptr<const T>> result;
    for (const auto& entry : Map<T>())
        result.push_back(entry.second);
    return result;
}

template <class T>
std::vector<std::shared_ptr<T>> ObjectMap::FindObjects() {
    std::vector<std::shared_ptr<T>> result;
    for (const auto& entry : Map<T>())
        result.push_back(entry.second);
    return result;
}

template <class T>
std::vector<int> ObjectMap::FindObjectIDs() const {
    std::vector<int> result;
    for (const auto& entry : Map<T>())
        result.push_back(entry.first);
    return result;
}

template <class T>
std::vector<std::shared_ptr<const T>> ObjectMap::FindObjects(const std::vector<int>& object_ids) const {
    std::vector<std::shared_ptr<const T>> retval;
    typedef typename std::remove_const<T>::type mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.push_back(std::shared_ptr<const T>(map_it->second));
    }
    return retval;
}

template <class T>
std::vector<std::shared_ptr<const T>> ObjectMap::FindObjects(const std::set<int>& object_ids) const {
    std::vector<std::shared_ptr<const T>> retval;
    typedef typename std::remove_const<T>::type mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.push_back(std::shared_ptr<const T>(map_it->second));
    }
    return retval;
}

template <class T>
std::vector<std::shared_ptr<T>> ObjectMap::FindObjects(const std::vector<int>& object_ids) {
    std::vector<std::shared_ptr<T>> retval;
    typedef typename std::remove_const<T>::type mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.push_back(std::shared_ptr<T>(map_it->second));
    }
    return retval;
}

template <class T>
std::vector<std::shared_ptr<T>> ObjectMap::FindObjects(const std::set<int>& object_ids) {
    std::vector<std::shared_ptr<T>> retval;
    typedef typename std::remove_const<T>::type mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.push_back(std::shared_ptr<T>(map_it->second));
    }
    return retval;
}

template <class T>
int ObjectMap::NumObjects() const
{ return Map<typename std::remove_const<T>::type>().size(); }

template <class T>
void ObjectMap::Insert(std::shared_ptr<T> item, int empire_id /* = ALL_EMPIRES */) {
    if (!item)
        return;
    InsertCore(std::dynamic_pointer_cast<UniverseObject>(item), empire_id);
}

// template specializations

template <>
FO_COMMON_API const ObjectMap::container_type<UniverseObject>& ObjectMap::Map() const;

template <>
FO_COMMON_API const ObjectMap::container_type<ResourceCenter>& ObjectMap::Map() const;

template <>
FO_COMMON_API const ObjectMap::container_type<PopCenter>& ObjectMap::Map() const;

template <>
FO_COMMON_API const ObjectMap::container_type<Ship>& ObjectMap::Map() const;

template <>
FO_COMMON_API const ObjectMap::container_type<Fleet>& ObjectMap::Map() const;

template <>
FO_COMMON_API const ObjectMap::container_type<Planet>& ObjectMap::Map() const;

template <>
FO_COMMON_API const ObjectMap::container_type<System>& ObjectMap::Map() const;

template <>
FO_COMMON_API const ObjectMap::container_type<Building>& ObjectMap::Map() const;

template <>
FO_COMMON_API const ObjectMap::container_type<Field>& ObjectMap::Map() const;

template <>
FO_COMMON_API ObjectMap::container_type<UniverseObject>& ObjectMap::Map();

template <>
FO_COMMON_API ObjectMap::container_type<ResourceCenter>& ObjectMap::Map();

template <>
FO_COMMON_API ObjectMap::container_type<PopCenter>& ObjectMap::Map();

template <>
FO_COMMON_API ObjectMap::container_type<Ship>& ObjectMap::Map();

template <>
FO_COMMON_API ObjectMap::container_type<Fleet>& ObjectMap::Map();

template <>
FO_COMMON_API ObjectMap::container_type<Planet>& ObjectMap::Map();

template <>
FO_COMMON_API ObjectMap::container_type<System>& ObjectMap::Map();

template <>
FO_COMMON_API ObjectMap::container_type<Building>& ObjectMap::Map();

template <>
FO_COMMON_API ObjectMap::container_type<Field>& ObjectMap::Map();

#endif
