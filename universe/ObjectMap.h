#ifndef _Object_Map_h_
#define _Object_Map_h_


#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>
#include <boost/range/any_range.hpp>
#include <boost/range/size.hpp>
#include "../util/Export.h"


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

    ObjectMap();
    ~ObjectMap();

    /** Copies contents of this ObjectMap to a new ObjectMap, which is
      * returned.  Copies are limited to only duplicate information that the
      * empire with id \a empire_id would know about the copied objects. */
    ObjectMap* Clone(int empire_id = ALL_EMPIRES) const;

    /** Returns the number of objects of the specified class in this ObjectMap. */
    template <typename T = UniverseObject>
    std::size_t size() const;

    /** Returns true if this ObjectMap contains no objects */
    bool empty() const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <typename T = UniverseObject>
    std::shared_ptr<const T> get(int id) const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <typename T = UniverseObject>
    std::shared_ptr<T> get(int id);

    using id_range = boost::any_range<int, boost::forward_traversal_tag>;

    /** Returns a vector containing the objects with ids in \a object_ids that
      * are of type T */
    template <typename T = UniverseObject>
    std::vector<std::shared_ptr<const T>> find(const id_range& object_ids) const;

    /** Returns a vector containing the objects with ids in \a object_ids that
      * are of type T */
    template <typename T = UniverseObject>
    std::vector<std::shared_ptr<T>> find(const id_range& object_ids);

    /** Returns all the objects that match \a visitor */
    template <typename T = UniverseObject>
    std::vector<std::shared_ptr<const T>> find(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects that match \a visitor */
    template <typename T = UniverseObject>
    std::vector<std::shared_ptr<T>> find(const UniverseObjectVisitor& visitor);

    /** Returns IDs of all the objects that match \a visitor */
    template <typename T = UniverseObject>
    std::vector<int> findIDs(const UniverseObjectVisitor& visitor) const;

    /** Returns how many objects match \a visitor */
    template <typename T = UniverseObject>
    int count(const UniverseObjectVisitor& visitor) const;

    /** Returns true iff any object matches \a visitor */
    template <typename T = UniverseObject>
    bool check_if_any(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects of type T */
    template <typename T = UniverseObject>
    boost::select_second_const_range<container_type<T>> all() const
    { return Map<T>() | boost::adaptors::map_values; }

    /** Returns all the objects of type T */
    template <typename T = UniverseObject>
    boost::select_second_mutable_range<container_type<T>> all()
    { return Map<T>() | boost::adaptors::map_values; }

    /** Returns the IDs of all objects not known to have been destroyed. */
    std::vector<int>        FindExistingObjectIDs() const;

    /** Returns highest used object ID in this ObjectMap */
    int                     HighestObjectID() const;

    std::string             Dump(unsigned short ntabs = 0) const;

    /**  */
    std::shared_ptr<const UniverseObject> ExistingObject(int id) const;

    const container_type<const UniverseObject>& ExistingObjects() const
    { return m_existing_objects; }
    const container_type<const UniverseObject>& ExistingResourceCenters() const
    { return m_existing_resource_centers; }
    const container_type<const UniverseObject>& ExistingPopCenters() const
    { return m_existing_pop_centers; }
    const container_type<const UniverseObject>& ExistingShips() const
    { return m_existing_ships; }
    const container_type<const UniverseObject>& ExistingFleets() const
    { return m_existing_fleets; }
    const container_type<const UniverseObject>& ExistingPlanets() const
    { return m_existing_planets; }
    const container_type<const UniverseObject>& ExistingSystems() const
    { return m_existing_systems; }
    const container_type<const UniverseObject>& ExistingBuildings() const
    { return m_existing_buildings; }
    const container_type<const UniverseObject>& ExistingFields() const
    { return m_existing_fields; }

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
    template <typename T,
              typename std::enable_if<std::is_base_of<UniverseObject, T>::value>::type* = nullptr>
    void insert(std::shared_ptr<T> item, int empire_id = ALL_EMPIRES);

    /** Removes object with id \a id from map, and returns that object, if
      * there was an object under that ID in the map.  If no such object
      * existed in the map, a null shared_ptr is returned and nothing is
      * removed. The ObjectMap will no longer share ownership of the
      * returned object. */
    std::shared_ptr<UniverseObject> erase(int id);

    /** Empties map, removing shared ownership by this map of all
      * previously contained objects. */
    void clear();

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

private:
    void insertCore(std::shared_ptr<UniverseObject> item, int empire_id = ALL_EMPIRES);

    void CopyObjectsToSpecializedMaps();

    template <typename T>
    const container_type<T>& Map() const;

    template <typename T>
    container_type<T>& Map();

    template <typename T>
    static void SwapMap(container_type<T>& map, ObjectMap& rhs);

    container_type<UniverseObject>  m_objects;
    container_type<ResourceCenter>  m_resource_centers;
    container_type<PopCenter>       m_pop_centers;
    container_type<Ship>            m_ships;
    container_type<Fleet>           m_fleets;
    container_type<Planet>          m_planets;
    container_type<System>          m_systems;
    container_type<Building>        m_buildings;
    container_type<Field>           m_fields;

    container_type<const UniverseObject>  m_existing_objects;
    container_type<const UniverseObject>  m_existing_resource_centers;
    container_type<const UniverseObject>  m_existing_pop_centers;
    container_type<const UniverseObject>  m_existing_ships;
    container_type<const UniverseObject>  m_existing_fleets;
    container_type<const UniverseObject>  m_existing_planets;
    container_type<const UniverseObject>  m_existing_systems;
    container_type<const UniverseObject>  m_existing_buildings;
    container_type<const UniverseObject>  m_existing_fields;

    template <typename Archive>
    friend void serialize(Archive&, ObjectMap&, unsigned int const);
};

template <typename T>
std::shared_ptr<const T> ObjectMap::get(int id) const
{
    auto it = Map<typename std::remove_const<T>::type>().find(id);
    return std::shared_ptr<const T>(
        it != Map<typename std::remove_const<T>::type>().end()
            ? it->second
            : nullptr);
}

template <typename T>
std::shared_ptr<T> ObjectMap::get(int id)
{
    auto it = Map<typename std::remove_const<T>::type>().find(id);
    return std::shared_ptr<T>(
        it != Map<typename std::remove_const<T>::type>().end()
            ? it->second
            : nullptr);
}

template <typename T>
std::vector<std::shared_ptr<const T>> ObjectMap::find(const id_range& object_ids) const
{
    std::vector<std::shared_ptr<const T>> retval;
    retval.reserve(boost::size(object_ids));
    typedef typename std::remove_const<T>::type mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.emplace_back(map_it->second);
    }
    return retval;
}

template <typename T>
std::vector<std::shared_ptr<T>> ObjectMap::find(const id_range& object_ids)
{
    std::vector<std::shared_ptr<T>> retval;
    retval.reserve(boost::size(object_ids));
    typedef typename std::remove_const<T>::type mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.emplace_back(map_it->second);
    }
    return retval;
}

template <typename T>
std::vector<std::shared_ptr<const T>> ObjectMap::find(const UniverseObjectVisitor& visitor) const
{
    std::vector<std::shared_ptr<const T>> result;
    typedef typename std::remove_const<T>::type mutableT;
    result.reserve(size<mutableT>());
    for ([[maybe_unused]] auto& [ignored_id, obj] : Map<mutableT>()) {
        (void)ignored_id; // suppress unused variable warning
        if (obj->Accept(visitor))
            result.push_back(obj);
    }
    return result;
}

template <typename T>
std::vector<std::shared_ptr<T>> ObjectMap::find(const UniverseObjectVisitor& visitor)
{
    std::vector<std::shared_ptr<T>> result;
    typedef typename std::remove_const<T>::type mutableT;
    result.reserve(size<mutableT>());
    for ([[maybe_unused]] auto& [ignored_id, obj] : Map<mutableT>()) {
        (void)ignored_id; // suppress unused variable warning
        if (obj->Accept(visitor))
            result.push_back(obj);
    }
    return result;
}

template <typename T>
std::vector<int> ObjectMap::findIDs(const UniverseObjectVisitor& visitor) const
{
    std::vector<int> result;
    typedef typename std::remove_const<T>::type mutableT;
    result.reserve(size<mutableT>());
    for (const auto& [id, obj] : Map<mutableT>()) {
        if (obj->Accept(visitor))
            result.push_back(id);
    }
    return result;
}

template <typename T>
int ObjectMap::count(const UniverseObjectVisitor& visitor) const
{
    typedef typename std::remove_const<T>::type mutableT;
    // TODO: use std::count_if when switching to C++17
    return boost::range::count_if(Map<mutableT>(),
                                  [&visitor](const auto& entry) { return entry.second->Accept(visitor); });
    /*
    int retval = 0;
    for (const auto& entry : Map<mutableT>())
        retval += (entry.second->Accept(visitor) ? 1 : 0);
    return retval;
    */
}

/** Returns true iff no objects match \a visitor */
template <typename T>
bool ObjectMap::check_if_any(const UniverseObjectVisitor& visitor) const
{
    typedef typename std::remove_const<T>::type mutableT;
    return std::any_of(Map<mutableT>().begin(), Map<mutableT>().end(),
                       [&visitor](const auto& entry) { return entry.second->Accept(visitor); });
}

template <typename T>
std::size_t ObjectMap::size() const
{ return Map<typename std::remove_const<T>::type>().size(); }

template <typename T,
          typename std::enable_if<std::is_base_of<UniverseObject, T>::value>::type*>
void ObjectMap::insert(std::shared_ptr<T> item, int empire_id)
{
    if (!item)
        return;
    insertCore(std::move(item), empire_id);
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
