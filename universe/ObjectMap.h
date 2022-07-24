#ifndef _Object_Map_h_
#define _Object_Map_h_


#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/any_range.hpp>
#include "ConstantsFwd.h"
#include "../util/Export.h"


struct UniverseObjectVisitor;

class Universe;
class UniverseObject;
class ResourceCenter;
class PopCenter;
class Ship;
class Fleet;
class Planet;
class System;
class Building;
class Field;

/** Contains a set of objects that make up a (known or complete) Universe. */
class FO_COMMON_API ObjectMap {
public:
    template <typename T>
    using container_type = std::map<int, std::shared_ptr<T>>;

    /** Copies contents of this ObjectMap to a new ObjectMap, which is
      * returned.  Copies are limited to only duplicate information that the
      * empire with id \a empire_id would know about the copied objects. */
    ObjectMap* Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const;

    /** Returns the number of objects of the specified class in this ObjectMap. */
    template <typename T = UniverseObject>
    std::size_t size() const;

    /** Returns true if this ObjectMap contains no objects */
    bool empty() const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <typename T = UniverseObject>
    [[nodiscard]] std::shared_ptr<const T> get(int id) const;
    template <typename T = UniverseObject>
    [[nodiscard]] const T* getRaw(int id) const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <typename T = UniverseObject>
    [[nodiscard]] std::shared_ptr<T> get(int id);
    template <typename T = UniverseObject>
    [[nodiscard]] T* getRaw(int id);

    using id_range = boost::any_range<int, boost::forward_traversal_tag>;

    /** Returns a vector containing the objects with ids in \a object_ids that
      * are of type T */
    template <typename T = UniverseObject>
    [[nodiscard]] std::vector<std::shared_ptr<const T>> find(const id_range& object_ids) const;
    template <typename T = UniverseObject>
    [[nodiscard]] std::vector<const T*> findRaw(const id_range& object_ids) const;

    /** Returns a vector containing the objects with ids in \a object_ids that
      * are of type T */
    template <typename T = UniverseObject>
    [[nodiscard]] std::vector<std::shared_ptr<T>> find(const id_range& object_ids);
    template <typename T = UniverseObject>
    [[nodiscard]] std::vector<T*> findRaw(const id_range& object_ids);

    /** Returns all the objects that match \a visitor */
    template <typename T = UniverseObject>
    [[nodiscard]] std::vector<std::shared_ptr<const T>> find(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects that match \a visitor */
    template <typename T = UniverseObject>
    [[nodiscard]] std::vector<std::shared_ptr<T>> find(const UniverseObjectVisitor& visitor);

    /** Returns IDs of all the objects that match \a visitor */
    template <typename T = UniverseObject>
    [[nodiscard]] std::vector<int> findIDs(const UniverseObjectVisitor& visitor) const;

    /** Returns how many objects match \a visitor */
    template <typename T = UniverseObject>
    int count(const UniverseObjectVisitor& visitor) const;

    /** Returns true iff any object matches \a visitor */
    template <typename T = UniverseObject>
    bool check_if_any(const UniverseObjectVisitor& visitor) const;

    /** Returns all the objects of type T */
    template <typename T = UniverseObject>
    [[nodiscard]] auto all() const
    {
        using DecayT = std::decay_t<T>;
        static constexpr auto tx = [](const typename container_type<DecayT>::mapped_type& p)
            -> typename container_type<const DecayT>::mapped_type
        { return std::const_pointer_cast<const DecayT>(p); };

        return Map<DecayT>() |
            boost::adaptors::map_values |
            boost::adaptors::transformed(tx);
    }

    template <typename T = UniverseObject>
    [[nodiscard]] auto allRaw() const
    {
        using DecayT = std::decay_t<T>;
        return std::as_const(Map<DecayT>()) |
            boost::adaptors::map_values |
            boost::adaptors::transformed(
                [](const auto& p) -> const DecayT* { return p.get(); }
        );
    }

    /** Returns all the objects of type T */
    template <typename T = UniverseObject>
    [[nodiscard]] auto all()
    {
        using DecayT = std::decay_t<T>;
        if constexpr (std::is_const_v<T>) {
            const auto& const_this = *this;
            return const_this.all<DecayT>();
        } else {
            return std::as_const(Map<DecayT>()) | boost::adaptors::map_values;
        }
    }

    /** Returns all the objects of type T */
    template <typename T = UniverseObject>
    [[nodiscard]] auto allRaw()
    {
        using DecayT = std::decay_t<T>;
        using OutT = std::conditional_t<std::is_const_v<T>, const DecayT*, DecayT*>;
        return Map<DecayT>() |
            boost::adaptors::map_values |
            boost::adaptors::transformed(
                [](const auto& p) -> OutT { return p.get(); });
    }

    /** Returns all the ids and objects of type T */
    template <typename T = UniverseObject>
    [[nodiscard]] auto allWithIDs() const
    {
        using DecayT = std::decay_t<T>;
        static const auto tx = [](const typename container_type<DecayT>::value_type& p)
            -> typename container_type<const DecayT>::value_type
        { return {p.first, std::const_pointer_cast<const DecayT>(p.second)}; };

        return std::as_const(Map<DecayT>()) | boost::adaptors::transformed(tx);
    }

    /** Returns all the ids and objects of type T */
    template <typename T, std::enable_if_t<std::is_const_v<T>>* = nullptr>
    [[nodiscard]] auto allWithIDs()
    {
        const auto& const_this = *this;
        return const_this.allWithIDs<T>();
    }

    /** Returns all the ids and objects of type T */
    template <typename T = UniverseObject, std::enable_if_t<!std::is_const_v<T>>* = nullptr>
    [[nodiscard]] const auto& allWithIDs()
    {
        using DecayT = std::decay_t<T>;
        return std::as_const(Map<DecayT>());
    }


    /** Returns the IDs of all objects not known to have been destroyed. */
    [[nodiscard]] std::vector<int> FindExistingObjectIDs() const;

    /** Returns highest used object ID in this ObjectMap */
    int HighestObjectID() const;

    [[nodiscard]] std::string Dump(unsigned short ntabs = 0) const;

    /**  */
    [[nodiscard]] std::shared_ptr<const UniverseObject> getExisting(int id) const;

    template <typename T = UniverseObject>
    [[nodiscard]] const auto& allExisting() const
    {
        using DecayT = std::decay_t<T>;
        if constexpr (std::is_same_v<DecayT, UniverseObject>)
            return m_existing_objects;
        else if constexpr (std::is_same_v<DecayT, ResourceCenter>)
            return m_existing_resource_centers;
        else if constexpr (std::is_same_v<DecayT, PopCenter>)
            return m_existing_pop_centers;
        else if constexpr (std::is_same_v<DecayT, Ship>)
            return m_existing_ships;
        else if constexpr (std::is_same_v<DecayT, Fleet>)
            return m_existing_fleets;
        else if constexpr (std::is_same_v<DecayT, Planet>)
            return m_existing_planets;
        else if constexpr (std::is_same_v<DecayT, System>)
            return m_existing_systems;
        else if constexpr (std::is_same_v<DecayT, Building>)
            return m_existing_buildings;
        else if constexpr (std::is_same_v<DecayT, Field>)
            return m_existing_fields;
        else {
            static_assert(std::is_same_v<DecayT, UniverseObject>, "invalid type for allExisting()");
            static const decltype(m_existing_objects) error_retval;
            return error_retval;
        }
    }

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
    void Copy(const ObjectMap& copied_map, const Universe& universe, int empire_id = ALL_EMPIRES);

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
    void CopyObject(std::shared_ptr<const UniverseObject> source, int empire_id,
                    const Universe& universe);

    /** Adds object \a obj to the map under its ID, if it is a valid object.
      * If there already was an object in the map with the id \a id then
      * that object will be removed. */
    template <typename T,
              typename std::enable_if_t<std::is_base_of_v<UniverseObject, T>>* = nullptr>
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
    ///** Swaps the contents of *this with \a rhs. */
    //void swap(ObjectMap& rhs);

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
    void insertCore(std::shared_ptr<Planet> item, int empire_id = ALL_EMPIRES);

    void CopyObjectsToSpecializedMaps();

    // returns const container of mutable T ... may need further adapting for fully const safe use
    template <typename T>
    const container_type<std::decay_t<T>>& Map() const
    {
        static_assert(!std::is_const_v<T>, "type for Map() should not be const");
        using DecayT = std::decay_t<T>;
        if constexpr (std::is_same_v<DecayT, UniverseObject>)
            return m_objects;
        else if constexpr (std::is_same_v<DecayT, ResourceCenter>)
            return m_resource_centers;
        else if constexpr (std::is_same_v<DecayT, PopCenter>)
            return m_pop_centers;
        else if constexpr (std::is_same_v<DecayT, Ship>)
            return m_ships;
        else if constexpr (std::is_same_v<DecayT, Fleet>)
            return m_fleets;
        else if constexpr (std::is_same_v<DecayT, Planet>)
            return m_planets;
        else if constexpr (std::is_same_v<DecayT, System>)
            return m_systems;
        else if constexpr (std::is_same_v<DecayT, Building>)
            return m_buildings;
        else if constexpr (std::is_same_v<DecayT, Field>)
            return m_fields;
        else {
            static_assert(std::is_same_v<DecayT, UniverseObject>, "invalid type for Map()");
            static const decltype(m_objects) error_retval;
            return error_retval;
        }
    }

    template <typename T>
    container_type<std::decay_t<T>>& Map()
    {
        static_assert(!std::is_const_v<T>, "type for Map() should not be const");
        using DecayT = std::decay_t<T>;
        if constexpr (std::is_same_v<DecayT, UniverseObject>)
            return m_objects;
        else if constexpr (std::is_same_v<DecayT, ResourceCenter>)
            return m_resource_centers;
        else if constexpr (std::is_same_v<DecayT, PopCenter>)
            return m_pop_centers;
        else if constexpr (std::is_same_v<DecayT, Ship>)
            return m_ships;
        else if constexpr (std::is_same_v<DecayT, Fleet>)
            return m_fleets;
        else if constexpr (std::is_same_v<DecayT, Planet>)
            return m_planets;
        else if constexpr (std::is_same_v<DecayT, System>)
            return m_systems;
        else if constexpr (std::is_same_v<DecayT, Building>)
            return m_buildings;
        else if constexpr (std::is_same_v<DecayT, Field>)
            return m_fields;
        else {
            static_assert(std::is_same_v<DecayT, UniverseObject>, "invalid type for Map()");
            static const decltype(m_objects) error_retval;
            return error_retval;
        }
    }

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
    auto it = Map<typename std::remove_const_t<T>>().find(id);
    return std::shared_ptr<const T>(
        it != Map<typename std::remove_const_t<T>>().end()
            ? it->second
            : nullptr);
}

template <typename T>
const T* ObjectMap::getRaw(int id) const
{
    auto it = Map<typename std::remove_const_t<T>>().find(id);
    return
        it != Map<typename std::remove_const_t<T>>().end()
            ? it->second.get()
            : nullptr;
}

template <typename T>
std::shared_ptr<T> ObjectMap::get(int id)
{
    auto it = Map<typename std::remove_const_t<T>>().find(id);
    return std::shared_ptr<T>(
        it != Map<typename std::remove_const_t<T>>().end()
            ? it->second
            : nullptr);
}

template <typename T>
T* ObjectMap::getRaw(int id)
{
    auto it = Map<typename std::remove_const_t<T>>().find(id);
    return
        it != Map<typename std::remove_const_t<T>>().end()
            ? it->second.get()
            : nullptr;
}

template <typename T>
std::vector<std::shared_ptr<const T>> ObjectMap::find(const id_range& object_ids) const
{
    std::vector<std::shared_ptr<const T>> retval;
    retval.reserve(boost::size(object_ids));
    typedef typename std::remove_const_t<T> mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.push_back(map_it->second);
    }
    return retval;
}

template <typename T>
std::vector<const T*> ObjectMap::findRaw(const id_range& object_ids) const
{
    std::vector<const T*> retval;
    retval.reserve(boost::size(object_ids));
    typedef typename std::remove_const_t<T> mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.push_back(map_it->second.get());
    }
    return retval;
}

template <typename T>
std::vector<std::shared_ptr<T>> ObjectMap::find(const id_range& object_ids)
{
    std::vector<std::shared_ptr<T>> retval;
    retval.reserve(boost::size(object_ids));
    typedef typename std::remove_const_t<T> mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.push_back(map_it->second);
    }
    return retval;
}

template <typename T>
std::vector<T*> ObjectMap::findRaw(const id_range& object_ids)
{
    std::vector<T*> retval;
    retval.reserve(boost::size(object_ids));
    typedef typename std::remove_const_t<T> mutableT;
    for (int object_id : object_ids) {
        auto map_it = Map<mutableT>().find(object_id);
        if (map_it != Map<mutableT>().end())
            retval.push_back(map_it->second.get());
    }
    return retval;
}

template <typename T>
std::vector<std::shared_ptr<const T>> ObjectMap::find(const UniverseObjectVisitor& visitor) const
{
    std::vector<std::shared_ptr<const T>> result;
    typedef typename std::remove_const_t<T> mutableT;
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
    typedef typename std::remove_const_t<T> mutableT;
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
    typedef typename std::remove_const_t<T> mutableT;
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
    typedef typename std::remove_const_t<T> mutableT;
    return std::count_if(Map<mutableT>(),
                         [&visitor](const auto& entry) { return entry.second->Accept(visitor); });
}

/** Returns true iff no objects match \a visitor */
template <typename T>
bool ObjectMap::check_if_any(const UniverseObjectVisitor& visitor) const
{
    typedef typename std::remove_const_t<T> mutableT;
    return std::any_of(Map<mutableT>().begin(), Map<mutableT>().end(),
                       [&visitor](const auto& entry) { return entry.second->Accept(visitor); });
}

template <typename T>
std::size_t ObjectMap::size() const
{ return Map<typename std::remove_const_t<T>>().size(); }

template <typename T,
          typename std::enable_if_t<std::is_base_of_v<UniverseObject, T>>*>
void ObjectMap::insert(std::shared_ptr<T> item, int empire_id)
{
    if (item)
        insertCore(std::move(item), empire_id);
}

#endif
