#ifndef _Object_Map_h_
#define _Object_Map_h_

#include <array>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <boost/range/adaptor/map.hpp>
#include "ConstantsFwd.h"
#include "UniverseObjectVisitor.h"
#include "../util/Export.h"

class Universe;
class UniverseObject;
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
    [[nodiscard]] ObjectMap* Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const;

    /** Returns the number of objects of the specified class in this ObjectMap. */
    template <typename T = UniverseObject>
    [[nodiscard]] std::size_t size() const { return Map<typename std::decay_t<T>>().size(); }

    /** Returns true if this ObjectMap contains no objects */
    [[nodiscard]] bool empty() const noexcept { return m_objects.empty(); };

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <typename T = UniverseObject>
    [[nodiscard]] std::shared_ptr<const std::decay_t<T>> get(int id) const;
    template <typename T = UniverseObject>
    [[nodiscard]] const std::decay_t<T>* getRaw(int id) const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <typename T = UniverseObject>
    [[nodiscard]] std::shared_ptr<std::decay_t<T>> get(int id);
    template <typename T = UniverseObject>
    [[nodiscard]] std::decay_t<T>* getRaw(int id);

    /** Returns a vector containing the objects that match \a pred when applied as a visitor or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] std::vector<const std::decay_t<T>*> findRaw(Pred pred) const;

    /** Returns a vector containing the objects that match \a pred when applied as a visitor or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] std::vector<std::decay_t<T>*> findRaw(Pred pred);

    /** Returns all the objects that match \a pred when applied as a visitor or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] std::vector<std::shared_ptr<const std::decay_t<T>>> find(Pred pred) const;

    /** Returns all the objects that match \a pred when applied as a visitor or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] std::vector<std::shared_ptr<std::decay_t<T>>> find(Pred pred);

    /** Returns IDs of all the objects that match \a pred when applied as a visitor or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] std::vector<int> findIDs(Pred pred) const;

    /** Returns how many objects match \a pred when applied as a visitor or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] int count(Pred pred) const;

    /** Returns true iff any object matches \a pred when applied as a visitor or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] bool check_if_any(Pred pred) const;

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
    template <typename T> requires (std::is_const_v<T>)
    [[nodiscard]] auto allWithIDs()
    {
        const auto& const_this = *this;
        return const_this.allWithIDs<T>();
    }

    /** Returns all the ids and objects of type T */
    template <typename T = UniverseObject> requires (!std::is_const_v<T>)
    [[nodiscard]] const auto& allWithIDs() noexcept
    {
        using DecayT = std::decay_t<T>;
        return std::as_const(Map<DecayT>());
    }


    /** Returns the IDs of all objects not known to have been destroyed. */
    [[nodiscard]] std::vector<int> FindExistingObjectIDs() const;

    /** Returns highest used object ID in this ObjectMap */
    [[nodiscard]] int HighestObjectID() const;

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const;

    /**  */
    [[nodiscard]] std::shared_ptr<const UniverseObject> getExisting(int id) const;

    template <typename T = UniverseObject>
    [[nodiscard]] const auto& allExisting() const noexcept
    {
        using DecayT = std::decay_t<T>;
        if constexpr (std::is_same_v<DecayT, UniverseObject>)
            return m_existing_objects;
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
            static const decltype(m_existing_objects) error_retval{};
            return error_retval;
        }
    }

    template <typename T = UniverseObject>
    [[nodiscard]] const auto& allExistingRaw() const noexcept
    {
        using DecayT = std::decay_t<T>;
        if constexpr (std::is_same_v<DecayT, UniverseObject>)
            return m_existing_object_vec;
        else if constexpr (std::is_same_v<DecayT, Ship>)
            return m_existing_ship_vec;
        else if constexpr (std::is_same_v<DecayT, Fleet>)
            return m_existing_fleet_vec;
        else if constexpr (std::is_same_v<DecayT, Planet>)
            return m_existing_planet_vec;
        else if constexpr (std::is_same_v<DecayT, System>)
            return m_existing_system_vec;
        else if constexpr (std::is_same_v<DecayT, Building>)
            return m_existing_building_vec;
        else if constexpr (std::is_same_v<DecayT, Field>)
            return m_existing_field_vec;
        else {
            static_assert(std::is_same_v<DecayT, UniverseObject>, "invalid type for allExistingRaw()");
            static const decltype(m_existing_object_vec) error_retval;
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

    /** Adds object \a obj to the map under its ID. If there already was an object
      * in the map with the id \a id then that object will be replaced. If destroyed
      * is false, then \a obj is also added to this ObjectMap's internal lists of
      * existing (ie. not destroyed) objects, which are returned by the Existing*
      * functions. */
    template <typename T> requires (std::is_base_of_v<UniverseObject, T>)
    void insert(std::shared_ptr<T> obj, bool destroyed)
    { insertCore(std::move(obj), destroyed); }

    /** Removes object with id \a id from map, and returns that object, if
      * there was an object under that ID in the map.  If no such object
      * existed in the map, a null shared_ptr is returned and nothing is
      * removed. The ObjectMap will no longer share ownership of the
      * returned object. */
    std::shared_ptr<UniverseObject> erase(int id);

    /** Empties map, removing shared ownership by this map of all
      * previously contained objects. */
    void clear();

    /** */
    void UpdateCurrentDestroyedObjects(const std::unordered_set<int>& destroyed_object_ids);

    /** Recalculates contained objects for all objects in this ObjectMap based
      * on what other objects exist in this ObjectMap. Useful to eliminate
      * cases where there are inconsistencies between whan an object thinks it
      * contains, and what other objects think they are contained by the first
      * object. */
    void AuditContainment(const std::unordered_set<int>& destroyed_object_ids);

private:
    void insertCore(std::shared_ptr<UniverseObject> obj, bool destroyed);
    void insertCore(std::shared_ptr<Planet> obj, bool destroyed);

    void CopyObjectsToSpecializedMaps();

    // returns const container of mutable T ... may need further adapting for fully const safe use
    template <typename T>
    [[nodiscard]] const container_type<std::decay_t<T>>& Map() const noexcept
    {
        static_assert(!std::is_const_v<T>, "type for Map() should not be const");
        using DecayT = std::decay_t<T>;
        if constexpr (std::is_same_v<DecayT, UniverseObject>)
            return m_objects;
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
    [[nodiscard]] container_type<std::decay_t<T>>& Map() noexcept
    {
        static_assert(!std::is_const_v<T>, "type for Map() should not be const");
        using DecayT = std::decay_t<T>;
        if constexpr (std::is_same_v<DecayT, UniverseObject>)
            return m_objects;
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

    template <typename T, typename Pred>
    [[nodiscard]] static constexpr std::array<bool, 11> CheckTypes();

    container_type<UniverseObject>  m_objects;
    container_type<Ship>            m_ships;
    container_type<Fleet>           m_fleets;
    container_type<Planet>          m_planets;
    container_type<System>          m_systems;
    container_type<Building>        m_buildings;
    container_type<Field>           m_fields;

    container_type<const UniverseObject> m_existing_objects;
    container_type<const UniverseObject> m_existing_ships;
    container_type<const UniverseObject> m_existing_fleets;
    container_type<const UniverseObject> m_existing_planets;
    container_type<const UniverseObject> m_existing_systems;
    container_type<const UniverseObject> m_existing_buildings;
    container_type<const UniverseObject> m_existing_fields;

    std::vector<const UniverseObject*> m_existing_object_vec;
    std::vector<const UniverseObject*> m_existing_ship_vec;
    std::vector<const UniverseObject*> m_existing_fleet_vec;
    std::vector<const UniverseObject*> m_existing_planet_vec;
    std::vector<const UniverseObject*> m_existing_system_vec;
    std::vector<const UniverseObject*> m_existing_building_vec;
    std::vector<const UniverseObject*> m_existing_field_vec;

    template <typename Archive>
    friend void serialize(Archive&, ObjectMap&, unsigned int const);
};

template <typename T>
std::shared_ptr<const std::decay_t<T>> ObjectMap::get(int id) const
{
    using DecayT = std::decay_t<T>;
    auto it = Map<DecayT>().find(id);
    return it != Map<DecayT>().end() ? it->second : std::shared_ptr<const DecayT>();
}

template <typename T>
const std::decay_t<T>* ObjectMap::getRaw(int id) const
{
    using DecayT = std::decay_t<T>;
    auto it = Map<DecayT>().find(id);
    return it != Map<DecayT>().end() ? it->second.get() : nullptr;
}

template <typename T>
std::shared_ptr<std::decay_t<T>> ObjectMap::get(int id)
{
    using DecayT = std::decay_t<T>;
    auto it = Map<DecayT>().find(id);
    return it != Map<DecayT>().end() ? it->second : std::shared_ptr<DecayT>();
}

template <typename T>
std::decay_t<T>* ObjectMap::getRaw(int id)
{
    using DecayT = std::decay_t<T>;
    auto it = Map<DecayT>().find(id);
    return it != Map<DecayT>().end() ? it->second.get() : nullptr;
}

namespace {
    template <typename T>
    concept int_iterable = std::is_same_v<typename T::value_type, int> && requires(T t) { t.begin(); t.end(); };

    static_assert(!int_iterable<int>);
    static_assert(!int_iterable<std::array<float, 5>>);
    static_assert(int_iterable<std::vector<int>>);
}

/** Checks whether and how a predicate can be applied to select objects from the ObjectMap.
  * T is a UniverseObject-like type, eg. Planet or Ship. Pred is the predicate type.
  * Pred may be a function that returns bool and that can be called by passing a
  * const T&, T*, shared_ptr<const T>, pair<const int, shared_ptr<T>>, or similar.
  * Pred may also be a UniverseObjectVisitor.
  * Pred may also be an iterable range of int that specifes IDs of the UniverseObjects
  * to select. */
template <typename T, typename Pred>
constexpr std::array<bool, 11> ObjectMap::CheckTypes()
{
    using DecayT = std::decay_t<T>;
    using DecayPred = std::decay_t<Pred>;
    using ContainerT = container_type<DecayT>;
    using EntryT = typename ContainerT::value_type;
    static_assert(std::is_same_v<std::pair<const int, std::shared_ptr<DecayT>>, EntryT>);
    using ConstEntryT = std::pair<const int, std::shared_ptr<const DecayT>>;
    static_assert(std::is_convertible_v<EntryT, ConstEntryT>);


    constexpr bool is_int_range = int_iterable<Pred>;


    constexpr bool is_visitor = std::is_convertible_v<DecayPred, UniverseObjectVisitor>;


    constexpr bool invokable_on_raw_const_object =
        std::is_invocable_r_v<bool, DecayPred, const DecayT*>;
    constexpr bool invokable_on_raw_mutable_object =
        std::is_invocable_r_v<bool, DecayPred, DecayT*>;
    // accepting const objects only is OK, or accepting const or mutable objects, but not only mutable objects
    static_assert(invokable_on_raw_const_object ||
                    (!invokable_on_raw_const_object && !invokable_on_raw_mutable_object),
                    "predicate may not modify ObjectMap contents");


    static_assert(std::is_convertible_v<std::shared_ptr<DecayT>, std::shared_ptr<const DecayT>>);

    constexpr bool invokable_on_shared_const_object =
        std::is_invocable_r_v<bool, DecayPred, const std::shared_ptr<const DecayT>&>;
    constexpr bool invokable_on_shared_mutable_object =
        std::is_invocable_r_v<bool, DecayPred, const std::shared_ptr<DecayT>&>;
    static_assert(invokable_on_shared_const_object ||
                    (!invokable_on_shared_const_object && !invokable_on_shared_mutable_object),
                    "predicate may not modify ObjectMap contents");


    constexpr bool invokable_on_const_entry =
        std::is_invocable_r_v<bool, DecayPred, const ConstEntryT&>;
    constexpr bool invokable_on_mutable_entry =
        std::is_invocable_r_v<bool, DecayPred, const EntryT&>;
    static_assert(invokable_on_const_entry ||
                    (!invokable_on_const_entry && !invokable_on_mutable_entry),
                    "predicate may not modify ObjectMap contents");


    constexpr bool invokable_on_const_reference =
        std::is_invocable_r_v<bool, DecayPred, const DecayT&>;
    constexpr bool invokable_on_mutable_reference =
        std::is_invocable_r_v<bool, DecayPred, DecayT&>;
    static_assert(invokable_on_const_reference ||
                    (!invokable_on_const_reference && !invokable_on_mutable_reference),
                    "predicate may not modify ObjectMap contents");

    constexpr bool invokable =
        invokable_on_raw_const_object || invokable_on_raw_mutable_object ||
        invokable_on_shared_const_object || invokable_on_shared_mutable_object ||
        invokable_on_const_entry || invokable_on_mutable_entry ||
        invokable_on_const_reference || invokable_on_mutable_reference;

    return std::array<bool, 11>{
        invokable_on_raw_const_object, invokable_on_raw_mutable_object,
        invokable_on_shared_const_object, invokable_on_shared_mutable_object,
        invokable_on_const_entry, invokable_on_mutable_entry,
        invokable_on_const_reference, invokable_on_mutable_reference,
        invokable, is_visitor, is_int_range};
}

template <typename T, typename Pred>
std::vector<const std::decay_t<T>*> ObjectMap::findRaw(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool is_visitor = invoke_flags[9];
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    std::vector<const DecayT*> result;
    if constexpr (!is_int_range)
        result.reserve(size<DecayT>());
    else
        result.reserve(std::size(pred));

    if constexpr (is_int_range) {
        // TODO: special case for sorted range of int?
        for (int object_id : pred) {
            auto map_it = Map<DecayT>().find(object_id);
            if (map_it != Map<DecayT>().end())
                result.push_back(map_it->second.get());
        }

    } else if constexpr (is_visitor) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (obj->Accept(pred))
                result.push_back(obj.get());

    } else if constexpr (invokable_on_raw_const_object) {
        for (const auto& [id, obj] : Map<DecayT>()) {
            const DecayT* obj_raw = obj.get();
            if (pred(obj_raw))
                result.push_back(obj_raw);
        }

    } else if constexpr (invokable_on_shared_const_object) {
        for (const auto& [id, obj] : Map<DecayT>()) {
            if (pred(obj))
                result.push_back(obj.get());
        }

    } else if constexpr (invokable_on_const_entry) {
        for (const auto& id_obj : Map<DecayT>())
            if (pred(id_obj))
                result.push_back(id_obj.second.get());

    } else if constexpr (invokable_on_const_reference) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(*obj))
                result.push_back(obj.get());

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred>
std::vector<std::decay_t<T>*> ObjectMap::findRaw(Pred pred)
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool is_visitor = invoke_flags[9];
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    std::vector<DecayT*> result;
    if constexpr (!is_int_range)
        result.reserve(size<DecayT>());
    else
        result.reserve(std::size(pred));

    if constexpr (is_int_range) {
        for (int object_id : pred) {
            auto map_it = Map<DecayT>().find(object_id);
            if (map_it != Map<DecayT>().end())
                result.push_back(map_it->second.get());
        }

    } else if constexpr (is_visitor) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (obj->Accept(pred))
                result.push_back(obj.get());

    } else if constexpr (invokable_on_raw_const_object) {
        for (const auto& [id, obj] : Map<DecayT>()) {
            DecayT* obj_raw = obj.get();
            if (pred(obj_raw))
                result.push_back(obj_raw);
        }

    } else if constexpr (invokable_on_shared_const_object) {
        for (const auto& [id, obj] : Map<DecayT>()) {
            if (pred(obj))
                result.push_back(obj.get());
        }

    } else if constexpr (invokable_on_const_entry) {
        for (const auto& id_obj : Map<DecayT>())
            if (pred(id_obj))
                result.push_back(id_obj.second.get());

    } else if constexpr (invokable_on_const_reference) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(*obj))
                result.push_back(obj.get());

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred>
std::vector<std::shared_ptr<const std::decay_t<T>>> ObjectMap::find(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool is_visitor = invoke_flags[9];
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    std::vector<std::shared_ptr<const DecayT>> result;
    if constexpr (!is_int_range)
        result.reserve(size<DecayT>());
    else
        result.reserve(std::size(pred));

    if constexpr (is_int_range) {
        for (int object_id : pred) {
            auto map_it = Map<DecayT>().find(object_id);
            if (map_it != Map<DecayT>().end())
                result.push_back(map_it->second);
        }

    } else if constexpr (is_visitor) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (obj->Accept(pred))
                result.push_back(obj);

    } else if constexpr (invokable_on_raw_const_object) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(obj.get()))
                result.push_back(obj);

    } else if constexpr (invokable_on_shared_const_object) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(obj))
                result.push_back(obj);

    } else if constexpr (invokable_on_const_entry) {
        for (const auto& id_obj : Map<DecayT>())
            if (pred(id_obj))
                result.push_back(id_obj.second);

    } else if constexpr (invokable_on_const_reference) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(*obj))
                result.push_back(obj);

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred>
std::vector<std::shared_ptr<std::decay_t<T>>> ObjectMap::find(Pred pred)
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool is_visitor = invoke_flags[9];
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    std::vector<std::shared_ptr<DecayT>> result;
    if constexpr (!is_int_range)
        result.reserve(size<DecayT>());
    else
        result.reserve(std::size(pred));

    if constexpr (is_int_range) {
        for (int object_id : pred) {
            auto map_it = Map<DecayT>().find(object_id);
            if (map_it != Map<DecayT>().end())
                result.push_back(map_it->second);
        }

    } else if constexpr (is_visitor) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (obj->Accept(pred))
                result.push_back(obj);

    } else if constexpr (invokable_on_raw_const_object) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(obj.get()))
                result.push_back(obj);

    } else if constexpr (invokable_on_shared_const_object) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(obj))
                result.push_back(obj);

    } else if constexpr (invokable_on_const_entry) {
        for (const auto& id_obj : Map<DecayT>())
            if (pred(id_obj))
                result.push_back(id_obj.second);

    } else if constexpr (invokable_on_const_reference) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(*obj))
                result.push_back(obj);

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred>
std::vector<int> ObjectMap::findIDs(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool is_visitor = invoke_flags[9];
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    std::vector<int> result;
    if constexpr (!is_int_range)
        result.reserve(size<DecayT>());
    else
        result.reserve(std::size(pred));

    if constexpr (is_int_range) {
        std::copy_if(pred.begin(), pred.end(), std::back_inserter(result),
                     [this](int id) { return Map<DecayT>().contains(id); });

    } else if constexpr (is_visitor) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (obj->Accept(pred))
                result.push_back(id);

    } else if constexpr (invokable_on_raw_const_object) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(obj.get()))
                result.push_back(id);

    } else if constexpr (invokable_on_shared_const_object) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(obj))
                result.push_back(id);

    } else if constexpr (invokable_on_const_entry) {
        for (const auto& id_obj : Map<DecayT>())
            if (pred(id_obj))
                result.push_back(id_obj.first);

    } else if constexpr (invokable_on_const_reference) {
        for (const auto& [id, obj] : Map<DecayT>())
            if (pred(*obj))
                result.push_back(id);

    } else {
        constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred>
int ObjectMap::count(Pred pred) const
{
    constexpr auto invoke_flags = CheckTypes<T, Pred>();
    constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    constexpr bool invokable_on_const_entry = invoke_flags[4];
    constexpr bool invokable_on_const_reference = invoke_flags[6];
    constexpr bool invokable = invoke_flags[8];
    constexpr bool is_visitor = invoke_flags[9];
    constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;
    using ContainerT = std::decay_t<decltype(Map<DecayT>())>;
    using EntryT = typename ContainerT::value_type;

    if constexpr (is_int_range) {
        return std::count_if(pred.begin(), pred.end(),
                             [this](const int id) { return Map<DecayT>().contains(id); });

    } else if constexpr (is_visitor) {
        return std::count_if(Map<DecayT>().begin(), Map<DecayT>().end(),
                             [visitor{pred}](const EntryT& o) { return o.second->Accept(visitor); });

    } else if constexpr (invokable_on_raw_const_object) {
        return std::count_if(Map<DecayT>().begin(), Map<DecayT>().end(),
                             [obj_pred{pred}](const EntryT& o) { return obj_pred(o.second.get()); });

    } else if constexpr (invokable_on_shared_const_object) {
        return std::count_if(Map<DecayT>().begin(), Map<DecayT>().end(),
                             [obj_pred{pred}](const EntryT& o) { return obj_pred(o.second); });

    } else if constexpr (invokable_on_const_entry) {
        return std::count_if(Map<DecayT>().begin(), Map<DecayT>().end(),
                             [entry_pred{pred}](const EntryT& o) { return entry_pred(o); });

    } else if constexpr (invokable_on_const_reference) {
        return std::count_if(Map<DecayT>().begin(), Map<DecayT>().end(),
                             [ref_pred{pred}](const EntryT& o) { return ref_pred(*o.second); });

    } else {
        static_assert(invokable, "Don't know how to handle predicate");
        return false;
    }
}

template <typename T, typename Pred>
bool ObjectMap::check_if_any(Pred pred) const
{
    constexpr auto invoke_flags = CheckTypes<T, Pred>();
    constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    constexpr bool invokable_on_const_entry = invoke_flags[4];
    constexpr bool invokable_on_const_reference = invoke_flags[6];
    constexpr bool is_visitor = invoke_flags[9];
    constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;
    using ContainerT = std::decay_t<decltype(Map<DecayT>())>;
    using EntryT = typename ContainerT::value_type;

    if constexpr (is_int_range) {
        return std::any_of(pred.begin(), pred.end(),
                           [this](int id) { return Map<DecayT>().contains(id); });

    } else if constexpr (is_visitor) {
        return std::any_of(Map<DecayT>().begin(), Map<DecayT>().end(),
                           [visitor{pred}](const EntryT& o) { return o.second->Accept(visitor); });

    } else if constexpr (invokable_on_raw_const_object) {
        return std::any_of(Map<DecayT>().begin(), Map<DecayT>().end(),
                           [obj_pred{pred}](const EntryT& o) { return obj_pred(o.second.get()); });

    } else if constexpr (invokable_on_shared_const_object) {
        return std::any_of(Map<DecayT>().begin(), Map<DecayT>().end(),
                           [obj_pred{pred}](const EntryT& o) { return obj_pred(o.second); });

    } else if constexpr (invokable_on_const_entry) {
        return std::any_of(Map<DecayT>().begin(), Map<DecayT>().end(),
                           [entry_pred{pred}](const EntryT& o) { return entry_pred(o); });

    } else if constexpr (invokable_on_const_reference) {
        return std::any_of(Map<DecayT>().begin(), Map<DecayT>().end(),
                           [ref_pred{pred}](const EntryT& o) { return ref_pred(*o.second); });

    } else {
        constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
        return false;
    }
}


#endif
