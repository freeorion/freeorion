#ifndef _Object_Map_h_
#define _Object_Map_h_

#include <array>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <boost/container/flat_set.hpp>
#include <boost/unordered/unordered_set.hpp>
#include "ConstantsFwd.h"
#include "../util/Export.h"
#include "../util/ranges.h"

class Universe;
class UniverseObject;
class Ship;
class Fleet;
class Planet;
class System;
class Building;
class Field;

#if !defined(CONSTEXPR_VEC)
#  if defined(__cpp_lib_constexpr_vector)
#    define CONSTEXPR_VEC constexpr
#  else
#    define CONSTEXPR_VEC
#  endif
#endif

/** Contains a set of objects that make up a (known or complete) Universe. */
class FO_COMMON_API ObjectMap {
public:
    template <typename T = UniverseObject>
    using container_type = std::map<int, std::shared_ptr<T>>;


    /** Copies contents of this ObjectMap to a new ObjectMap, which is
      * returned.  Copies are limited to only duplicate information that the
      * empire with id \a empire_id would know about the copied objects. */
    [[nodiscard]] std::unique_ptr<ObjectMap> Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const;

    /** Returns the number of objects of the specified class in this ObjectMap. */
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] std::size_t size() const noexcept
    {
        static_assert(noexcept(Map<typename std::decay_t<T>, only_existing>().size()));
        return Map<typename std::decay_t<T>, only_existing>().size();
    }

    /** Returns true if this ObjectMap contains no objects */
    [[nodiscard]] bool empty() const noexcept { return m_objects.empty(); };

    /** Returns pointer to the object of type T with ID number \a id.
      * Returns a nullptr if none exists or the object with
      * ID \a id is not of type T. */
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] std::shared_ptr<const std::decay_t<T>> get(int id) const;
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] const std::decay_t<T>* getRaw(int id) const;

    /** Returns pointer to an object of type T that matches predicate \a pred.
      * returns nullptr if none exists. */
    template <typename T = UniverseObject, typename Pred, bool only_existing = false>
    [[nodiscard]] std::shared_ptr<const std::decay_t<T>> get(Pred pred) const;
    template <typename T = UniverseObject, typename Pred, bool only_existing = false>
    [[nodiscard]] const std::decay_t<T>* getRaw(Pred pred) const;

    /** Returns a pointer to the object of type T with ID number \a id.
      * Returns a null std::shared_ptr if none exists or the object with
      * ID \a id is not of type T. */
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] std::shared_ptr<std::decay_t<T>> get(int id);
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] std::decay_t<T>* getRaw(int id);

    /** Returns a vector containing the objects that match \a pred when applied as a visitor or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred, bool only_existing = false>
    [[nodiscard]] std::vector<const std::decay_t<T>*> findRaw(Pred pred) const;
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] std::vector<std::decay_t<T>*> findRaw(Pred pred);
    template <typename T = UniverseObject, typename Pred>
    [[nodiscard]] auto findExistingRaw(Pred pred) const
    { return findRaw<T, Pred, true>(pred); }
    template <typename T = UniverseObject, typename Pred, bool only_existing = false>
    [[nodiscard]] std::vector<std::shared_ptr<const std::decay_t<T>>> find(Pred pred) const;
    template <typename T = UniverseObject, typename Pred, bool only_existing = false>
    [[nodiscard]] std::vector<std::shared_ptr<std::decay_t<T>>> find(Pred pred);

    /** Returns IDs of all the objects that match \a pred when applied as a visitor
      * or predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred, bool only_existing = false>
    [[nodiscard]] std::vector<int> findIDs(Pred pred) const;

    /** Returns how many objects match \a pred when applied as a visitor or predicate
      * filter or range of object ids */
    template <typename T = UniverseObject, typename Pred, bool only_existing = false>
    [[nodiscard]] int count(Pred pred) const;

    /** Returns true iff any object matches \a pred when applied as a visitor or
      * predicate filter or range of object ids */
    template <typename T = UniverseObject, typename Pred, bool only_existing = false>
    [[nodiscard]] bool check_if_any(Pred pred) const;
    template <typename T = UniverseObject, typename Pred, typename IDs, bool only_existing = false>
#if !defined(FREEORION_ANDROID)
        requires requires(IDs ids) { ids.begin(); ids.end(); {*ids.begin()} -> std::convertible_to<int>; }
#else
        requires requires(IDs ids) { ids.begin(); ids.end(); {static_cast<int>(*ids.begin())}; }
#endif
    [[nodiscard]] bool check_if_any(Pred pred, IDs&& ids) const;

    /** Returns all the objects of type T (maybe shared) ptr to const */
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] decltype(auto) all() const
    {
        using DecayT = std::decay_t<T>;

        if constexpr (only_existing) {
            return ExistingMap<DecayT>() | range_values;

        } else {
            // convert ptr_to_mutable& to ptr_to_const
            static constexpr auto const_ptr_tx = [](const typename container_type<DecayT>::mapped_type& p)
                -> typename container_type<const DecayT>::mapped_type
            { return std::const_pointer_cast<const DecayT>(p); };

            auto rng = Map<DecayT, false>() | range_values | range_transform(const_ptr_tx);
            using retval_value_t = decltype(rng.front());
            static_assert(!std::is_reference_v<retval_value_t>); // would like to return a refence to a pointer-to-const, but not possible if adding const using const_pointer_cast
            static_assert(!std::is_const_v<retval_value_t>); // mutable pointer to const

            return rng;
        }
    }

    /** Returns all the objects of type T as raw ptr to const */
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] auto allRaw() const
    {
        static_assert(!only_existing, "haven't implemented  allRaw() const  for only_existing = true ...");

        using DecayT = std::decay_t<T>;
        static constexpr auto const_raw_ptr_tx = [](const auto& p) -> const DecayT* { return p.get(); };
        auto rng = Map<DecayT, only_existing>() | range_values | range_transform(const_raw_ptr_tx);
        using retval_value_t = decltype(rng.front());
        static_assert(std::is_same_v<retval_value_t, const DecayT*>);

        return rng;
    }

    /** Returns all the objects of type T as (maybe const&) shared ptr to (mutable or const, depending on T) */
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] decltype(auto) all()
    {
        using DecayT = std::decay_t<T>;

        if constexpr (std::is_const_v<T> || only_existing) {
            const auto& const_this = *this;

            auto rng = const_this.all<DecayT, only_existing>();
            using retval_value_t = decltype(rng.front()); 
            static_assert((!std::is_const_v<std::remove_reference_t<retval_value_t>> && !std::is_reference_v<retval_value_t>) || // values stored as shared_ptr<mutable T> so returning shared_ptr<const T> requires a cast rather than returning a const&
                          (std::is_const_v<std::remove_reference_t<retval_value_t>> && std::is_reference_v<retval_value_t>)); 
            return rng;

        } else {
            static constexpr const auto make_const =
                [](std::shared_ptr<DecayT>& p) -> const std::shared_ptr<DecayT>& { return p; };
            return Map<DecayT, false>() | range_values | range_transform(make_const);
        }
    }

    /** Returns all the objects of type T as raw ptr to mutable */
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] auto allRaw()
    {
        static_assert(!only_existing, "haven't implemented  allRaw() (mutable)  for only_existing = true ...");

        using DecayT = std::decay_t<T>;
        using OutT = std::conditional_t<std::is_const_v<T>, const DecayT*, DecayT*>;
        static constexpr auto raw_ptr_tx = [](const auto& p) -> OutT { return p.get(); };
        return Map<DecayT, only_existing>() | range_values | range_transform(raw_ptr_tx);
    }

    /** Returns all the ids and objects of type T as pair<int, const ptr to const> */
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] auto allWithIDs() const
    {
        using DecayT = std::decay_t<T>;
        static constexpr auto tx = [](const typename container_type<DecayT>::value_type& p)
            -> typename container_type<const DecayT>::value_type
        { return {p.first, std::const_pointer_cast<const DecayT>(p.second)}; };

        return std::as_const(Map<DecayT, only_existing>()) | range_transform(tx);
    }

    /** Returns all the ids and objects of type T */
    template <typename T, bool only_existing = false> requires (std::is_const_v<T>)
    [[nodiscard]] const auto& allWithIDs()
    { return std::as_const(*this).allWithIDs<T, only_existing>(); }

    /** Returns all the ids and objects of type T */
    template <typename T = UniverseObject, bool only_existing = false> requires (!std::is_const_v<T>)
    [[nodiscard]] const auto& allWithIDs() noexcept
    {
        using DecayT = std::decay_t<T>;
        return std::as_const(Map<DecayT, only_existing>());
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
    { return Map<T, true>(); }

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
            static CONSTEXPR_VEC const decltype(m_existing_object_vec) error_retval;
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

    template <typename T, typename Pred>
    [[nodiscard]] static constexpr std::array<bool, 12> CheckTypes();

private:
    void insertCore(std::shared_ptr<UniverseObject> obj, bool destroyed);

    void CopyObjectsToSpecializedMaps();

    // returns const container of mutable T ... may need further adapting for fully const safe use
    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] const auto& Map() const noexcept
    {
        static_assert(!std::is_const_v<T>, "type for Map() should not be const");
        using DecayT = std::decay_t<T>;

        if constexpr (!only_existing) {
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

        } else {
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
                static_assert(std::is_same_v<DecayT, UniverseObject>, "invalid type for Map()");
                static const decltype(m_objects) error_retval;
                return error_retval;
            }
        }
    }

    template <typename T = UniverseObject, bool only_existing = false>
    [[nodiscard]] auto& Map() noexcept
    {
        static_assert(!std::is_const_v<T>, "type for Map() should not be const");
        using DecayT = std::decay_t<T>;

        if constexpr (!only_existing) {
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
                return m_objects;
            }
        } else {
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
                static_assert(std::is_same_v<DecayT, UniverseObject>, "invalid type for Map()");
                return m_existing_objects;
            }
        }
    }

    template <typename T = UniverseObject>
    [[nodiscard]] const auto& ExistingMap() const noexcept
    { return Map<T, true>(); }

    template <typename T = UniverseObject>
    [[nodiscard]] auto& ExistingMap() noexcept
    { return Map<T, true>(); }

    template <typename T = UniverseObject>
    [[nodiscard]] const auto& ExistingVec() const noexcept
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
            static_assert(std::is_same_v<DecayT, UniverseObject>, "invalid type for ExistingVec()");
            static const decltype(m_existing_object_vec) error_retval;
            return error_retval;
        }
    }

    template <typename T = UniverseObject>
    [[nodiscard]] auto& ExistingVec() noexcept
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
            static_assert(std::is_same_v<DecayT, UniverseObject>, "invalid type for ExistingVec()");
            return m_existing_object_vec;
        }
    }

    // calls \a func on all maps containing objects
    template <typename Function>
    void ApplyToCoreMaps(Function func, bool include_nonspecialized = true);

    // calls \a func on all "existing" maps containing objects
    template <typename Function>
    void ApplyToExistingMaps(Function func, bool include_nonspecialized = true);

    // calls \a func on all "existing" vectors of objects
    template <typename Function>
    void ApplyToExistingVecs(Function func, bool include_nonspecialized = true);

    // inserts \a obj into the map / vec for existing objects of type of ObjectType
    template <typename ObjectType = ::UniverseObject>
    void TypedInsertExisting(auto ID, std::shared_ptr<ObjectType> obj);

    // dispatches to the appropriate TypedInsertExisting for the dynamic object type in \a obj
    void AutoTypedInsertExisting(auto ID, auto&& obj);

    // inserts \a obj into the map / vecs for the type ObjectType
    template <typename ObjectType = ::UniverseObject>
    void TypedInsert(auto ID, auto destroyed, std::shared_ptr<ObjectType> obj);

    // dispatches to the appropriate TypedInsert for the dynamic object type in \a obj
    void AutoTypedInsert(auto ID, auto destroyed, auto&& obj);

    container_type<UniverseObject>  m_objects;
    container_type<Ship>            m_ships;
    container_type<Fleet>           m_fleets;
    container_type<Planet>          m_planets;
    container_type<System>          m_systems;
    container_type<Building>        m_buildings;
    container_type<Field>           m_fields;

    container_type<const UniverseObject> m_existing_objects;
    container_type<const Ship>           m_existing_ships;
    container_type<const Fleet>          m_existing_fleets;
    container_type<const Planet>         m_existing_planets;
    container_type<const System>         m_existing_systems;
    container_type<const Building>       m_existing_buildings;
    container_type<const Field>          m_existing_fields;

    std::vector<const UniverseObject*> m_existing_object_vec;
    std::vector<const Ship*>           m_existing_ship_vec;
    std::vector<const Fleet*>          m_existing_fleet_vec;
    std::vector<const Planet*>         m_existing_planet_vec;
    std::vector<const System*>         m_existing_system_vec;
    std::vector<const Building*>       m_existing_building_vec;
    std::vector<const Field*>          m_existing_field_vec;

    template <typename Archive>
    friend void serialize(Archive&, ObjectMap&, unsigned int const);
};

namespace ObjectMapPredicateTypeTraits{
    template <typename T>
    concept int_iterable = std::is_same_v<std::decay_t<typename T::value_type>, int> && requires(T t) { t.begin(); t.end(); };

    template <class T>
    concept is_set = requires(T t) { []<typename ...Args>(std::set<Args...>&){}(t); };

    template <class T>
    concept is_multiset = requires(T t) { []<typename ...Args>(std::multiset<Args...>&){}(t); };

    template <class T>
    concept is_flat_set = requires(T t) { []<typename ...Args>(boost::container::flat_set<Args...>&){}(t); };

    template <typename T>
    concept is_sorted = is_set<T> || is_multiset<T> || is_flat_set<T>;

    template <class C>
    concept is_unordered_set = requires(C c) { []<typename ...Args>(std::unordered_set<Args...>&){}(c); };

    template <class C>
    concept is_boost_unordered_set = requires(C c) { []<typename ...Args>(boost::unordered::unordered_set<Args...>&){}(c); };

    template <class T>
    concept is_unique_set = is_set<T> || is_flat_set<T> || is_unordered_set<T> || is_boost_unordered_set<T>;
}

/** Checks whether and how a predicate can be applied to select objects from the ObjectMap.
  * T is a UniverseObject-like type, eg. Planet or Ship. Pred is the predicate type.
  * Pred may be a function that returns bool and that can be called by passing a
  * const T&, T*, shared_ptr<const T>, pair<const int, shared_ptr<T>>, or similar.
  * Pred may also be an iterable range of int that specifes IDs of the UniverseObjects
  * to select. */
template <typename T, typename Pred>
constexpr std::array<bool, 12> ObjectMap::CheckTypes() // TODO: check if it's a Condition
{
    using DecayT = std::decay_t<T>;
    static_assert(std::is_base_of_v<UniverseObject, DecayT>);
    using DecayPred = std::decay_t<Pred>;
    using ContainerT = container_type<DecayT>;
    using EntryT = typename ContainerT::value_type;
    static_assert(std::is_same_v<std::pair<const int, std::shared_ptr<DecayT>>, EntryT>);
    using ConstEntryT = std::pair<const int, std::shared_ptr<const DecayT>>;
    static_assert(std::is_convertible_v<EntryT, ConstEntryT>);

    using namespace ObjectMapPredicateTypeTraits;

    constexpr bool is_int_range = int_iterable<Pred>;


    constexpr bool is_visitor = false; // legacy cruft


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

    constexpr bool invokable_on_int = std::is_invocable_r_v<bool, DecayPred, int>;

    constexpr bool invokable =
        invokable_on_raw_const_object || invokable_on_raw_mutable_object ||
        invokable_on_shared_const_object || invokable_on_shared_mutable_object ||
        invokable_on_const_entry || invokable_on_mutable_entry ||
        invokable_on_const_reference || invokable_on_mutable_reference ||
        invokable_on_int;

    return std::array<bool, 12>{
        invokable_on_raw_const_object, invokable_on_raw_mutable_object,
        invokable_on_shared_const_object, invokable_on_shared_mutable_object,
        invokable_on_const_entry, invokable_on_mutable_entry,
        invokable_on_const_reference, invokable_on_mutable_reference,
        invokable, is_visitor, is_int_range, invokable_on_int};
}

template <typename T, bool only_existing>
std::shared_ptr<const std::decay_t<T>> ObjectMap::get(int id) const
{
    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};
    const auto it = map.find(id);
    return it != map.end() ? it->second : std::shared_ptr<const DecayT>();
}

template <typename T, bool only_existing>
const std::decay_t<T>* ObjectMap::getRaw(int id) const
{
    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};
    auto it = map.find(id);
    return it != map.end() ? it->second.get() : nullptr;
}

template <typename T, typename Pred, bool only_existing>
std::shared_ptr<const std::decay_t<T>> ObjectMap::get(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    // static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];
    static_assert(!is_int_range, "use findRaw to get multiple objects from IDs");

    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};

    if constexpr (invokable_on_raw_const_object) {
        auto rng = map | range_values;
        auto it = range_find_if(rng, [&pred](const auto& obj) { return pred(obj.get()); });
        return (it != rng.end()) ? *it : nullptr;

    } else if constexpr (invokable_on_shared_const_object) {
        auto rng = map | range_values;
        auto it = range_find_if(rng, pred);
        return (it != rng.end()) ? *it : nullptr;

    } else if constexpr (invokable_on_const_entry) {
        auto it = std::find_if(map.begin(), map.end(), pred);
        return (it != map.end()) ? it->second : nullptr;

    } else if constexpr (invokable_on_const_reference) {
        auto rng = map | range_values;
        auto it = range_find_if(rng, [&pred](const auto& id_obj) { return pred(*id_obj->second); });
        return (it != rng.end()) ? it->second : nullptr;

    } else if constexpr (invokable_on_int) {
        auto it = std::find_if(map.begin(), map.end(), [&pred](const auto& id_obj) { return pred(id_obj->first); });
        return (it != map.end()) ? it->second : nullptr;

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
        return nullptr;
    }
}

template <typename T, typename Pred, bool only_existing>
const std::decay_t<T>* ObjectMap::getRaw(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    //static constexpr bool is_visitor = invoke_flags[9]; // lrgacy cruft
    static constexpr bool is_int_range = invoke_flags[10];
    static_assert(!is_int_range, "use findRaw to get multiple objects from IDs");

    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};
    static constexpr auto raw_ptr_tx = [](const auto& p) -> const DecayT* { return p.get(); };
    static constexpr auto ref_tx = [](const auto& p) -> const DecayT& { return *p; };

    if constexpr (invokable_on_raw_const_object) {
        auto it = range_find_if(map | range_values | range_transform(raw_ptr_tx), pred);
        return (it != map.end()) ? *it : nullptr;

    } else if constexpr (invokable_on_shared_const_object) {
        auto it = range_find_if(map | range_values, pred);
        return (it != map.end()) ? it->get() : nullptr;

    } else if constexpr (invokable_on_const_entry) {
        auto it = std::find_if(map.begin(), map.end(), pred);
        return (it != map.end()) ? it->second.get() : nullptr;

    } else if constexpr (invokable_on_const_reference) {
        auto it = range_find_if(map | range_values | range_transform(ref_tx), pred);
        return (it != map.end()) ? &*it : nullptr;

    } else if constexpr (invokable_on_int) {
        auto it = range_find_if(map, [pred](const auto& id_obj) { return pred(id_obj.first); });
        return (it != map.end()) ? it->second.get() : nullptr;

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
        return nullptr;
    }
}

template <typename T, bool only_existing>
std::shared_ptr<std::decay_t<T>> ObjectMap::get(int id)
{
    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};
    auto it = map.find(id);
    return it != map.end() ? it->second : std::shared_ptr<DecayT>();
}

template <typename T, bool only_existing>
std::decay_t<T>* ObjectMap::getRaw(int id)
{
    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};
    auto it = map.find(id);
    return it != map.end() ? it->second.get() : nullptr;
}

template <typename T, typename Pred, bool only_existing>
std::vector<const std::decay_t<T>*> ObjectMap::findRaw(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    //static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    std::vector<const DecayT*> result;
    if constexpr (!is_int_range)
        result.reserve(size<DecayT>());
    else
        result.reserve(std::size(pred));

    auto& map{Map<DecayT, only_existing>()};

    static constexpr auto not_null = [](const auto& p) noexcept(noexcept(bool(p))) -> bool { return p; };
    static constexpr auto get_rawptr = [](const auto& p) noexcept(noexcept(p.get())) -> auto* { return p.get(); };

    if constexpr (is_int_range) {
        // TODO: special case for sorted range of int?
        const auto find_in_map = [&map](auto id) -> const DecayT* {
            auto map_it = map.find(id);
            return (map_it != map.end()) ? map_it->second.get() : nullptr;
        };
#if (USING_STD_RANGES)
        auto rng = pred | range_transform(find_in_map);
#else
        // avoids error: no type named 'type' in 'boost::range_iterator<const std::span<const int>>'
        auto rng = boost::make_iterator_range(pred.begin(), pred.end()) | range_transform(find_in_map);
#endif

        range_copy_if(rng, std::back_inserter(result), not_null);
        return result;

    } else if constexpr (invokable_on_raw_const_object) {
        auto rng = map | range_values | range_transform(get_rawptr);
        result.reserve(map.size());
        range_copy_if(rng, std::back_inserter(result), pred);
        return result;

    } else if constexpr (invokable_on_shared_const_object) {
        auto rng = map | range_values | range_filter(pred) | range_transform(get_rawptr);
        result.reserve(map.size());
        range_copy(rng, std::back_inserter(result));
        return result;

    } else if constexpr (invokable_on_const_entry) {
        auto rng = map | range_filter(pred) | range_values | range_transform(get_rawptr);
        result.reserve(map.size());
        range_copy(rng, std::back_inserter(result));
        return result;

    } else if constexpr (invokable_on_const_reference) {
        static constexpr auto to_cref = [](const auto& id_ptr) -> const auto& { return *(id_ptr.second); };
        static constexpr auto to_address = [](const auto& r) { return &r; };
        auto rng = map | range_transform(to_cref) | range_filter(pred) | range_transform(to_address);
        result.reserve(map.size());
        range_copy(rng, std::back_inserter(result));
        return result;

    } else if constexpr (invokable_on_int) {
        auto rng = map | range_filter([pred](const auto& id_ptr) { return pred(id_ptr.first); })
                       | range_values | range_transform(get_rawptr);
        result.reserve(map.size());
        range_copy(rng, std::back_inserter(result));
        return result;

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(is_int_range || invokable, "Don't know how to handle predicate");
        return result;
    }
}

template <typename T, typename Pred>
std::vector<std::decay_t<T>*> ObjectMap::findRaw(Pred pred)
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    //static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    std::vector<DecayT*> result;
    if constexpr (!is_int_range)
        result.reserve(size<DecayT>());
    else
        result.reserve(std::size(pred));

    auto& map{Map<DecayT, false>()};

    if constexpr (is_int_range) {
        for (int object_id : pred) {
            auto map_it = map.find(object_id);
            if (map_it != map.end())
                result.push_back(map_it->second.get());
        }

    } else if constexpr (invokable_on_raw_const_object) {
        for (auto& [id, obj] : map) {
            DecayT* obj_raw = obj.get();
            if (pred(std::as_const(obj_raw)))
                result.push_back(obj_raw);
        }

    } else if constexpr (invokable_on_shared_const_object) {
        for (auto& [id, obj] : map) {
            if (pred(std::as_const(obj)))
                result.push_back(obj.get());
        }

    } else if constexpr (invokable_on_const_entry) {
        for (auto& id_obj : map)
            if (pred(std::as_const(id_obj)))
                result.push_back(id_obj.second.get());

    } else if constexpr (invokable_on_const_reference) {
        for (auto& [id, obj] : map)
            if (pred(std::as_const(*obj)))
                result.push_back(obj.get());

    } else if constexpr (invokable_on_int) {
        for (auto& [id, obj] : map)
            if (pred(id))
                result.push_back(obj.get());

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred, bool only_existing>
std::vector<std::shared_ptr<const std::decay_t<T>>> ObjectMap::find(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    //static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    auto& map{Map<DecayT, only_existing>()};

    std::vector<std::shared_ptr<const DecayT>> result;
    if constexpr (!is_int_range)
        result.reserve(map.size());
    else
        result.reserve(std::size(pred));

    if constexpr (is_int_range) {
        for (int object_id : pred) {
            auto map_it = map.find(object_id);
            if (map_it != map.end())
                result.push_back(map_it->second);
        }

    } else if constexpr (invokable_on_raw_const_object) {
        for (const auto& [id, obj] : map)
            if (pred(obj.get()))
                result.push_back(obj);

    } else if constexpr (invokable_on_shared_const_object) {
        for (const auto& [id, obj] : map)
            if (pred(obj))
                result.push_back(obj);

    } else if constexpr (invokable_on_const_entry) {
        for (const auto& id_obj : map)
            if (pred(id_obj))
                result.push_back(id_obj.second);

    } else if constexpr (invokable_on_const_reference) {
        for (const auto& [id, obj] : map)
            if (pred(*obj))
                result.push_back(obj);

    } else if constexpr (invokable_on_int) {
        for (const auto& [id, obj] : map)
            if (pred(id))
                result.push_back(obj);

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred, bool only_existing>
std::vector<std::shared_ptr<std::decay_t<T>>> ObjectMap::find(Pred pred)
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    //static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    auto& map{Map<DecayT, only_existing>()};

    std::vector<std::shared_ptr<DecayT>> result;
    if constexpr (!is_int_range)
        result.reserve(map.size());
    else
        result.reserve(std::size(pred));

    if constexpr (is_int_range) {
        for (int object_id : pred) {
            auto map_it = map.find(object_id);
            if (map_it != map.end())
                result.push_back(map_it->second);
        }

    } else if constexpr (invokable_on_raw_const_object) {
        for (const auto& obj : map | range_values)
            if (pred(obj.get()))
                result.push_back(obj);

    } else if constexpr (invokable_on_shared_const_object) {
        for (const auto& obj : map | range_values)
            if (pred(obj))
                result.push_back(obj);

    } else if constexpr (invokable_on_const_entry) {
        for (const auto& id_obj : map)
            if (pred(id_obj))
                result.push_back(id_obj.second);

    } else if constexpr (invokable_on_const_reference) {
        for (const auto& obj : map | range_values)
            if (pred(*obj))
                result.push_back(obj);

    } else if constexpr (invokable_on_int) {
        for (const auto& [id, obj] : map)
            if (pred(id))
                result.push_back(obj);

    } else {
        static constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred, bool only_existing>
std::vector<int> ObjectMap::findIDs(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    //static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;

    auto& map{Map<DecayT, only_existing>()};

    std::vector<int> result;
    if constexpr (!is_int_range)
        result.reserve(map.size());
    else
        result.reserve(std::size(pred));

    if constexpr (is_int_range) {
        std::copy_if(pred.begin(), pred.end(), std::back_inserter(result),
                     [&map](int id) { return map.contains(id); });

    } else if constexpr (invokable_on_raw_const_object) {
        for (const auto& [id, obj] : map)
            if (pred(obj.get()))
                result.push_back(id);

    } else if constexpr (invokable_on_shared_const_object) {
        for (const auto& [id, obj] : map)
            if (pred(obj))
                result.push_back(id);

    } else if constexpr (invokable_on_const_entry) {
        for (const auto& id_obj : map)
            if (pred(id_obj))
                result.push_back(id_obj.first);

    } else if constexpr (invokable_on_const_reference) {
        for (const auto& [id, obj] : map)
            if (pred(*obj))
                result.push_back(id);

    } else if constexpr (invokable_on_int) {
        for (const auto id : map | range_keys)
            if (pred(id))
                result.push_back(id);

    } else {
        constexpr bool invokable = invoke_flags[8];
        static_assert(invokable, "Don't know how to handle predicate");
    }

    return result;
}

template <typename T, typename Pred, bool only_existing>
int ObjectMap::count(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    static constexpr bool invokable = invoke_flags[8];
    //static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};
    using ContainerT = std::decay_t<decltype(map)>;
    using EntryT = typename ContainerT::value_type;


    if constexpr (is_int_range) {
        return std::count_if(pred.begin(), pred.end(),
                             [&map](int id) { return map.contains(id); });

    } else if constexpr (invokable_on_raw_const_object) {
        return std::count_if(map.begin(), map.end(),
                             [obj_pred{pred}](const EntryT& o) { return obj_pred(o.second.get()); });

    } else if constexpr (invokable_on_shared_const_object) {
        return std::count_if(map.begin(), map.end(),
                             [obj_pred{pred}](const EntryT& o) { return obj_pred(o.second); });

    } else if constexpr (invokable_on_const_entry) {
        return std::count_if(map.begin(), map.end(),
                             [entry_pred{pred}](const EntryT& o) { return entry_pred(o); });

    } else if constexpr (invokable_on_const_reference) {
        return std::count_if(map.begin(), map.end(),
                             [ref_pred{pred}](const EntryT& o) { return ref_pred(*o.second); });

    } else if constexpr (invokable_on_int) {
        return std::count_if(map.begin(), map.end(),
                             [int_pred{pred}](const EntryT& o) { return int_pred(o.first); });

    } else {
        static_assert(invokable, "Don't know how to handle predicate");
        return false;
    }
}

template <typename T, typename Pred, bool only_existing>
bool ObjectMap::check_if_any(Pred pred) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    // static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];

    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};
    using ContainerT = std::decay_t<decltype(map)>;
    using EntryT = typename ContainerT::value_type;

    if constexpr (is_int_range) {
        return range_any_of(pred, [&map](int id) { return map.contains(id); });

    } else {
        const auto test_pred = [&pred](const EntryT& obj) {
            if constexpr (invokable_on_raw_const_object)
                return pred(obj.second.get());
            else if constexpr (invokable_on_shared_const_object)
                return pred(obj.second);
            else if constexpr (invokable_on_const_entry)
                return pred(obj);
            else if constexpr (invokable_on_const_reference)
                return pred(obj.second);
            else if constexpr (invokable_on_int)
                return pred(obj.first);

            else {
                constexpr bool invokable = invoke_flags[8];
                static_assert(invokable, "Don't know how to handle predicate");
                return false;
            }
        };

        return range_any_of(map, test_pred);
    }
}

template <typename T, typename Pred, typename IDs, bool only_existing>
#if !defined(FREEORION_ANDROID)
    requires requires(IDs ids) { ids.begin(); ids.end(); {*ids.begin()} -> std::convertible_to<int>; }
#else
    requires requires(IDs ids) { ids.begin(); ids.end(); {static_cast<int>(*ids.begin())}; }
#endif
bool ObjectMap::check_if_any(Pred pred, IDs&& ids) const
{
    static constexpr auto invoke_flags = CheckTypes<T, Pred>();
    static constexpr bool invokable_on_raw_const_object = invoke_flags[0];
    static constexpr bool invokable_on_shared_const_object = invoke_flags[2];
    static constexpr bool invokable_on_const_entry = invoke_flags[4];
    static constexpr bool invokable_on_const_reference = invoke_flags[6];
    static constexpr bool invokable_on_int = invoke_flags[11];
    //static constexpr bool is_visitor = invoke_flags[9]; // legacy cruft
    static constexpr bool is_int_range = invoke_flags[10];
    static_assert(!is_int_range, "check_if_any passed two int ranges. Don't know what to do with this...");

    using DecayT = std::decay_t<T>;
    auto& map{Map<DecayT, only_existing>()};

    const auto map_lookup = [&map](const int id) { return map.find(id); };
    const auto rng = ids | range_transform(map_lookup);

    const auto test_pred = [&pred, end_it{map.end()}](const auto it) {
        if constexpr (invokable_on_raw_const_object)
            return it != end_it && pred(it->second.get());
        else if constexpr (invokable_on_shared_const_object)
            return it != end_it && pred(it->second);
        else if constexpr (invokable_on_const_entry)
            return it != end_it && pred(*it);
        else if constexpr (invokable_on_const_reference)
            return it != end_it && pred(*it->second);
        else if constexpr (invokable_on_int)
            return it != end_it && pred(it->first);

        else {
            constexpr bool invokable = invoke_flags[8];
            static_assert(invokable, "Don't know how to handle predicate");
            return false;
        }
        return false;
    };

    return range_any_of(rng, test_pred);
}


#endif
