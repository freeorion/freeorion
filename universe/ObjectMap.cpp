#include "ObjectMap.h"

#include "Building.h"
#include "Field.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"
#include "Universe.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"

class Fighter;

#define FOR_EACH_SPECIALIZED_MAP(f, ...)  { f<UniverseObjectType::OBJ_SHIP>(m_ships, ##__VA_ARGS__);                   \
                                            f<UniverseObjectType::OBJ_FLEET>(m_fleets, ##__VA_ARGS__);                 \
                                            f<UniverseObjectType::OBJ_PLANET>(m_planets, ##__VA_ARGS__);               \
                                            f<UniverseObjectType::OBJ_SYSTEM>(m_systems, ##__VA_ARGS__);               \
                                            f<UniverseObjectType::OBJ_BUILDING>(m_buildings, ##__VA_ARGS__);           \
                                            f<UniverseObjectType::OBJ_FIELD>(m_fields, ##__VA_ARGS__); }

#define FOR_EACH_MAP(f, ...)              { f(m_objects, ##__VA_ARGS__);                   \
                                            FOR_EACH_SPECIALIZED_MAP(f, ##__VA_ARGS__); }

#define FOR_EACH_EXISTING_MAP(f, ...)     { f(m_existing_objects, ##__VA_ARGS__);                                               \
                                            f<UniverseObjectType::OBJ_SHIP>(m_existing_ships, ##__VA_ARGS__);                   \
                                            f<UniverseObjectType::OBJ_FLEET>(m_existing_fleets, ##__VA_ARGS__);                 \
                                            f<UniverseObjectType::OBJ_PLANET>(m_existing_planets, ##__VA_ARGS__);               \
                                            f<UniverseObjectType::OBJ_SYSTEM>(m_existing_systems, ##__VA_ARGS__);               \
                                            f<UniverseObjectType::OBJ_BUILDING>(m_existing_buildings, ##__VA_ARGS__);           \
                                            f<UniverseObjectType::OBJ_FIELD>(m_existing_fields, ##__VA_ARGS__); }

#define FOR_EACH_EXISTING_VEC(f, ...)     { f(m_existing_object_vec, ##__VA_ARGS__);                                               \
                                            f<UniverseObjectType::OBJ_SHIP>(m_existing_ship_vec, ##__VA_ARGS__);                   \
                                            f<UniverseObjectType::OBJ_FLEET>(m_existing_fleet_vec, ##__VA_ARGS__);                 \
                                            f<UniverseObjectType::OBJ_PLANET>(m_existing_planet_vec, ##__VA_ARGS__);               \
                                            f<UniverseObjectType::OBJ_SYSTEM>(m_existing_system_vec, ##__VA_ARGS__);               \
                                            f<UniverseObjectType::OBJ_BUILDING>(m_existing_building_vec, ##__VA_ARGS__);           \
                                            f<UniverseObjectType::OBJ_FIELD>(m_existing_field_vec, ##__VA_ARGS__); }


namespace {
    template <UniverseObjectType ignored = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE,
              typename T>
    void ClearContainer(ObjectMap::container_type<T>& container)
    { container.clear(); }

    template <UniverseObjectType ignored = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE,
              typename T>
    void ClearContainer(std::vector<T>& container)
    { container.clear(); }

    // UniverseObject class type to enum UniverseObjectType
    template <typename UniverseObject_t>
    constexpr UniverseObjectType UniverseObjectClassTypeToEnum() {
        static_assert(std::is_base_of_v<UniverseObject, UniverseObject_t>);

        if constexpr (std::is_same_v<UniverseObject_t, Building>)            return UniverseObjectType::OBJ_BUILDING;
        else if constexpr (std::is_same_v<UniverseObject_t, Ship>)           return UniverseObjectType::OBJ_SHIP;
        else if constexpr (std::is_same_v<UniverseObject_t, Fleet>)          return UniverseObjectType::OBJ_FLEET;
        else if constexpr (std::is_same_v<UniverseObject_t, Planet>)         return UniverseObjectType::OBJ_PLANET;
        else if constexpr (std::is_same_v<UniverseObject_t, System>)         return UniverseObjectType::OBJ_SYSTEM;
        else if constexpr (std::is_same_v<UniverseObject_t, Field>)          return UniverseObjectType::OBJ_FIELD;
        else if constexpr (std::is_same_v<UniverseObject_t, Fighter>)        return UniverseObjectType::OBJ_FIGHTER;
        else                                                                 return UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE;
    }
    template <typename T>
    inline constexpr auto uot_enum_v = UniverseObjectClassTypeToEnum<T>();


    // filtering and selective inserting into maps by UniverseObject derived class type
    template <UniverseObjectType obj_type_filter = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE,
              typename MappedObjectType, typename PtrType>
    bool TryInsertIntoMap(ObjectMap::container_type<MappedObjectType>& map, PtrType&& item)
    {
        using PtrTypeBare = std::remove_const_t<std::remove_reference_t<PtrType>>;
        static_assert(std::is_same_v<PtrTypeBare, std::shared_ptr<UniverseObject>>);
        using PtrElementType = typename PtrTypeBare::element_type; // should be UniverseObject
        static_assert(std::is_same_v<PtrElementType, UniverseObject>);

        using MapTypeBare = std::remove_const_t<MappedObjectType>; // eg. Planet, UniverseObject
        static_assert(std::is_base_of_v<UniverseObject, MapTypeBare>);

        if (!item)
            return false;

        // check dynamic type filter on the passed in pointer
        if constexpr (obj_type_filter == UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE) {
            // any pointed-to object type is acceptable, as long as it is convertable into
            // the MappedObjectType and thus insertable into the input map

        } else {
            // only the specified object type is acceptable
            // all pointers should be shared_ptr<UniverseObject> but the pointed-to
            // objects should be a concrete type (Planet, System, Building, etc.)
            if (item->ObjectType() != obj_type_filter)
                return false;
        }


        // check whether the passed-in pointer can be inserted into the passed-in map
        if constexpr (std::is_same_v<UniverseObject, MapTypeBare>) {
            // passed a pointer and a map of of the same type: UniverseObject
            map.insert_or_assign(item->ID(), std::forward<PtrType>(item));
            return true;

        } else if constexpr (std::is_base_of_v<UniverseObject, MapTypeBare>) {
            // passed a UniverseObject pointer and a map of derived type. need to check the
            // actual type of the pointer before inserting in the map
            if (item->ObjectType() == uot_enum_v<MapTypeBare>) {
                auto ID = item->ID();
                map.insert_or_assign(ID, std::static_pointer_cast<MappedObjectType>(std::forward<PtrType>(item)));
                return true;
            }
            return false;

        } else {
            // shouldn't be possible to instantiate with this block enabled, but if so,
            // try to generate some useful compiler error messages to indicate what
            // type MappedObjectType is
            using GenerateCompileError = typename MappedObjectType::not_a_member_zi23tg;
            return false;
        }
    }

    template <UniverseObjectType ignored = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE,
              typename T>
    void EraseFromMap(ObjectMap::container_type<T>& map, int id)
    { map.erase(id); }

    template <UniverseObjectType obj_type_filter = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE,
              typename VectorValueType>
    bool TryInsertIntoVec(std::vector<VectorValueType*>& vec, const std::shared_ptr<UniverseObject>& item)
    {
        using VecTypeBare = std::remove_const_t<VectorValueType>; // eg. Planet, UniverseObject
        static_assert(std::is_base_of_v<UniverseObject, VecTypeBare>);

        if (!item)
            return false;

        // check dynamic type filter on the passed in pointer
        if constexpr (obj_type_filter == UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE) {
            // any pointed-to object type is acceptable, as long as it is convertable into
            // the VectorValueType and thus insertable into the input map

        } else {
            // only the specified object type is acceptable
            // all pointers should be shared_ptr<UniverseObject> but the pointed-to
            // objects should be a concrete type (Planet, System, Building, etc.)
            if (item->ObjectType() != obj_type_filter)
                return false;
        }


        // check whether the passed-in pointer can be inserted into the passed-in map
        if constexpr (std::is_same_v<UniverseObject, VecTypeBare>) {
            // passed a pointer and a vector of of the same type: UniverseObject
            const UniverseObject* obj = item.get();
            if (std::any_of(vec.begin(), vec.end(), [obj](const auto* v) { return v == obj; }))
                return false;
            vec.push_back(obj);
            return true;

        } else if constexpr (std::is_base_of_v<UniverseObject, VecTypeBare>) {
            // passed a UniverseObject pointer and a vector of derived type. need to check the
            // actual type of the pointer and cast before inserting in the vector
            if (item->ObjectType() != uot_enum_v<VecTypeBare>)
                return false;
            const auto obj = static_cast<VecTypeBare*>(item.get());
            if (std::any_of(vec.begin(), vec.end(), [obj](const auto* v) { return v == obj; }))
                return false;
            vec.push_back(obj);
            return true;

        } else {
            // shouldn't be possible to instantiate with this block enabled, but if so,
            // try to generate some useful compiler error messages to indicate what
            // type VecObjectType is
            using GenerateCompileError = typename VectorValueType::not_a_member_zi23tg;
            return false;
        }
    }

    template <UniverseObjectType obj_type_filter = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE,
              typename VectorValueType>
    bool TryInsertIntoVec(std::vector<VectorValueType*>& vec, const UniverseObject* obj)
    {
        using VecTypeBare = std::remove_const_t<VectorValueType>; // eg. Planet, UniverseObject
        static_assert(std::is_base_of_v<UniverseObject, VecTypeBare>);

        if (!obj)
            return false;

        // check dynamic type filter on the passed in pointer
        if constexpr (obj_type_filter == UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE) {
            // any pointed-to object type is acceptable, as long as it is convertable into
            // the VectorValueType and thus insertable into the input map

        } else {
            // only the specified object type is acceptable
            // all pointers should be shared_ptr<UniverseObject> but the pointed-to
            // objects should be a concrete type (Planet, System, Building, etc.)
            if (obj->ObjectType() != obj_type_filter)
                return false;
        }


        // check whether the passed-in pointer can be inserted into the passed-in map
        if constexpr (std::is_same_v<UniverseObject, VecTypeBare>) {
            // passed a pointer and a vector of of the same type: UniverseObject
            if (std::any_of(vec.begin(), vec.end(), [obj](const auto* v) { return v == obj; }))
                return false;
            vec.push_back(obj);
            return true;

        } else if constexpr (std::is_base_of_v<UniverseObject, VecTypeBare>) {
            // passed a UniverseObject pointer and a vector of derived type. need to check the
            // actual type of the pointer and cast before inserting in the vector
            if (obj->ObjectType() != uot_enum_v<VecTypeBare>)
                return false;
            const auto vec_obj = static_cast<VecTypeBare*>(obj);
            if (std::any_of(vec.begin(), vec.end(), [vec_obj](const auto* v) { return v == vec_obj; }))
                return false;
            vec.push_back(vec_obj);
            return true;

        } else {
            // shouldn't be possible to instantiate with this block enabled, but if so,
            // try to generate some useful compiler error messages to indicate what
            // type VecObjectType is
            using GenerateCompileError = typename VectorValueType::not_a_member_zi23tg;
            return false;
        }
    }


    template <UniverseObjectType ignored = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE,
              typename T>
    void EraseFromVec(std::vector<const T*>& vec, int id)
    {
        const auto begin = vec.begin();
        const auto end = vec.end();
        auto it = std::find_if(begin, end, [id](const auto* v) -> bool { return v->ID() == id; });
        if (it != end)
            vec.erase(it);
    }

    template <UniverseObjectType ignored = UniverseObjectType::INVALID_UNIVERSE_OBJECT_TYPE,
              typename T>
    void EraseFromVec(std::vector<const T*>& vec, const T* o)
    {
        const auto begin = vec.begin();
        const auto end = vec.end();
        auto it = std::find(begin, end, o);
        if (it != end)
            vec.erase(it);
    }
}

namespace {
    using const_mut_planet_range_t = decltype(std::declval<ObjectMap>().all<Planet>());
    using const_mut_planet_t = decltype(std::declval<const_mut_planet_range_t>().front());
    static_assert(std::is_same_v<const_mut_planet_t, const std::shared_ptr<Planet>&>);

    using const_const_planet_range_t = decltype(std::declval<const ObjectMap>().all<Planet>());
    using const_const_planet_t = decltype(std::declval<const_const_planet_range_t>().front());
    static_assert(std::is_same_v<const_const_planet_t, std::shared_ptr<const Planet>>);

    using const_const_planet_range_t2 = decltype(std::declval<ObjectMap>().all<const Planet>());
    using const_const_planet_t2 = decltype(std::declval<const_const_planet_range_t2>().front());
    static_assert(std::is_same_v<const_const_planet_t2, std::shared_ptr<const Planet>>);

    using const_mut_planet_raw_range_t = decltype(std::declval<ObjectMap>().allRaw<Planet>());
    using const_mut_planet_raw_t = decltype(std::declval<const_mut_planet_raw_range_t>().front());
    static_assert(std::is_same_v<const_mut_planet_raw_t, Planet*>);

    using const_const_planet_raw_range_t = decltype(std::declval<const ObjectMap>().allRaw<Planet>());
    using const_const_planet_raw_t = decltype(std::declval<const_const_planet_raw_range_t>().front());
    static_assert(std::is_same_v<const_const_planet_raw_t, const Planet*>);

    using const_const_planet_raw_range_t2 = decltype(std::declval<ObjectMap>().allRaw<const Planet>());
    using const_const_planet_raw_t2 = decltype(std::declval<const_const_planet_raw_range_t2>().front());
    static_assert(std::is_same_v<const_const_planet_raw_t2, const Planet*>);
}

/////////////////////////////////////////////
// class ObjectMap
/////////////////////////////////////////////
void ObjectMap::Copy(const ObjectMap& copied_map, const Universe& universe, int empire_id) {
    if (&copied_map == this)
        return;

    // loop through objects in copied map, copying or cloning each depending
    // on whether there already is a corresponding object in this map
    for (const auto& obj : copied_map.all())
        this->CopyObject(obj, empire_id, universe);
}

void ObjectMap::CopyForSerialize(const ObjectMap& copied_map) {
    if (&copied_map == this)
        return;

    // note: the following relies upon only m_objects actually getting serialized by ObjectMap::serialize
    m_objects.insert(copied_map.m_objects.begin(), copied_map.m_objects.end());
}

void ObjectMap::CopyObject(std::shared_ptr<const UniverseObject> source,
                           int empire_id, const Universe& universe)
{
    if (!source)
        return;

    int source_id = source->ID();

    // can empire see object at all?  if not, skip copying object's info
    if (universe.GetObjectVisibilityByEmpire(source_id, empire_id) <= Visibility::VIS_NO_VISIBILITY)
        return;

    if (auto destination = this->get(source_id)) {
        destination->Copy(*source, universe, empire_id); // there already is a version of this object present in this ObjectMap, so just update it
    } else {
        bool destroyed = universe.DestroyedObjectIds().contains(source_id);
        // this object is not yet present in this ObjectMap, so add a new UniverseObject object for it
        if (source->ObjectType() == UniverseObjectType::OBJ_PLANET) {
            auto plt = static_cast<const Planet*>(source.get());
            insertCore(std::static_pointer_cast<Planet>(plt->Clone(universe)), destroyed);
        } else {
            insertCore(source->Clone(universe), destroyed);
        }
    }
}

ObjectMap* ObjectMap::Clone(const Universe& universe, int empire_id) const {
    auto result = std::make_unique<ObjectMap>();
    result->Copy(*this, universe, empire_id);
    return result.release();
}

int ObjectMap::HighestObjectID() const {
    if (m_objects.empty())
        return INVALID_OBJECT_ID;
    return m_objects.rbegin()->first;
}

void ObjectMap::insertCore(std::shared_ptr<UniverseObject> obj, bool destroyed) {
    if (!obj)
        return;
    if (obj->ObjectType() == UniverseObjectType::OBJ_PLANET) {
        insertCore(std::static_pointer_cast<Planet>(obj), destroyed);
        return;
    }
    const auto ID = obj->ID();

    // can't use FOR_EACH_EXISTING_MAP with TryInsertIntoMap as all the existing map types store UniverseObject pointers
    FOR_EACH_SPECIALIZED_MAP(TryInsertIntoMap, obj);
    if (!destroyed) {
        const auto obj_raw = obj.get();
        FOR_EACH_EXISTING_VEC(TryInsertIntoVec, obj_raw)
        FOR_EACH_EXISTING_MAP(TryInsertIntoMap, obj)

        bool already_there = m_existing_objects.contains(ID);

        m_existing_objects[ID] = obj;
        if (!already_there)
            m_existing_object_vec.push_back(obj.get());
    }
    m_objects[ID] = std::move(obj);
}

void ObjectMap::insertCore(std::shared_ptr<Planet> obj, bool destroyed) {
    if (!obj)
        return;
    const auto ID = obj->ID();

    m_planets.insert_or_assign(ID, obj);
    if (!destroyed) {
        m_existing_planets.insert_or_assign(ID, obj);
        m_existing_objects.insert_or_assign(ID, obj);

        m_existing_object_vec.push_back(obj.get());
        m_existing_planet_vec.push_back(obj.get());
    }
    m_objects[ID] = std::move(obj);
}

std::shared_ptr<UniverseObject> ObjectMap::erase(int id) {
    // search for object in objects map
    auto it = m_objects.find(id);
    if (it == m_objects.end())
        return nullptr;
    //DebugLogger() << "Object was removed: " << it->second->Dump();
    // object found, so store pointer for later...
    auto result = it->second;
    const auto raw_result = result.get();
    // and erase from pointer maps
    FOR_EACH_EXISTING_VEC(EraseFromVec, raw_result);
    FOR_EACH_MAP(EraseFromMap, id);
    FOR_EACH_EXISTING_MAP(EraseFromMap, id);
    return result;
}

void ObjectMap::clear() {
    FOR_EACH_EXISTING_VEC(ClearContainer);
    FOR_EACH_MAP(ClearContainer);
    FOR_EACH_EXISTING_MAP(ClearContainer);
}

std::vector<int> ObjectMap::FindExistingObjectIDs() const {
    std::vector<int> result;
    result.reserve(m_existing_objects.size());
    for ([[maybe_unused]] auto& [id, ignored_obj] : m_existing_objects) {
        (void)ignored_obj;
        result.push_back(id);
    }
    return result;
}

void ObjectMap::UpdateCurrentDestroyedObjects(const std::unordered_set<int>& destroyed_object_ids) {
    FOR_EACH_EXISTING_VEC(ClearContainer);
    FOR_EACH_EXISTING_MAP(ClearContainer);
    for (const auto& [ID, obj] : m_objects) {
        if (!obj || destroyed_object_ids.contains(ID))
            continue;
        const auto obj_raw = obj.get();
        FOR_EACH_EXISTING_VEC(TryInsertIntoVec, obj_raw);
        FOR_EACH_EXISTING_MAP(TryInsertIntoMap, obj);
    }
}

void ObjectMap::AuditContainment(const std::unordered_set<int>& destroyed_object_ids) {
    // determine all objects that some other object thinks contains them
    std::map<int, std::set<int>> contained_objs;
    std::map<int, std::set<int>> contained_planets;
    std::map<int, std::set<int>> contained_buildings;
    std::map<int, std::set<int>> contained_fleets;
    std::map<int, std::set<int>> contained_ships;
    std::map<int, std::set<int>> contained_fields;

    for (const auto* contained : allRaw()) {
        if (destroyed_object_ids.contains(contained->ID()))
            continue;

        const int contained_id = contained->ID();
        const int sys_id = contained->SystemID();
        const int alt_id = contained->ContainerObjectID(); // planet or fleet id for a building or ship, or system id again for a fleet, field, or planet
        const UniverseObjectType type = contained->ObjectType();
        if (type == UniverseObjectType::OBJ_SYSTEM)
            continue;

        // store systems' contained objects
        if (this->getRaw(sys_id)) { // although this is expected to be a system, can't use Object<System> here due to CopyForSerialize not copying the type-specific objects info
            contained_objs[sys_id].insert(contained_id);

            if (type == UniverseObjectType::OBJ_PLANET)
                contained_planets[sys_id].insert(contained_id);
            else if (type == UniverseObjectType::OBJ_BUILDING)
                contained_buildings[sys_id].insert(contained_id);
            else if (type == UniverseObjectType::OBJ_FLEET)
                contained_fleets[sys_id].insert(contained_id);
            else if (type == UniverseObjectType::OBJ_SHIP)
                contained_ships[sys_id].insert(contained_id);
            else if (type == UniverseObjectType::OBJ_FIELD)
                contained_fields[sys_id].insert(contained_id);
        }

        // store planets' contained buildings
        if (type == UniverseObjectType::OBJ_BUILDING && this->getRaw(alt_id))
            contained_buildings[alt_id].insert(contained_id);

        // store fleets' contained ships
        if (type == UniverseObjectType::OBJ_SHIP && this->getRaw(alt_id))
            contained_ships[alt_id].insert(contained_id);
    }

    auto to_flat_set = [](const auto& container)
    { return UniverseObject::IDSet(container.begin(), container.end()); };

    // set contained objects of all possible containers
    for (auto* obj : allRaw()) {
        const int ID = obj->ID();
        const auto TYPE = obj->ObjectType();
        if (TYPE == UniverseObjectType::OBJ_SYSTEM) {
            auto sys = static_cast<System*>(obj);
            sys->m_objects =   to_flat_set(contained_objs[ID]);
            sys->m_planets =   to_flat_set( contained_planets[ID]);
            sys->m_buildings = to_flat_set(contained_buildings[ID]);
            sys->m_fleets =    to_flat_set(contained_fleets[ID]);
            sys->m_ships =     to_flat_set(contained_ships[ID]);
            sys->m_fields =    to_flat_set(contained_fields[ID]);

        } else if (TYPE == UniverseObjectType::OBJ_PLANET) {
            auto plt = static_cast<Planet*>(obj);
            plt->m_buildings = to_flat_set(contained_buildings[ID]);

        } else if (TYPE == UniverseObjectType::OBJ_FLEET) {
            auto flt = static_cast<Fleet*>(obj);
            flt->m_ships =     to_flat_set(contained_ships[ID]);
        }
    }
}

void ObjectMap::CopyObjectsToSpecializedMaps() {
    FOR_EACH_SPECIALIZED_MAP(ClearContainer);
    for (const auto& entry : Map<UniverseObject>())
    { FOR_EACH_SPECIALIZED_MAP(TryInsertIntoMap, entry.second); }
}

std::string ObjectMap::Dump(uint8_t ntabs) const {
    std::ostringstream dump_stream;
    dump_stream << "ObjectMap contains UniverseObjects: \n";
    for (const auto& obj : all())
        dump_stream << obj->Dump(ntabs) << "\n";
    dump_stream << "\n";
    return dump_stream.str();
}

std::shared_ptr<const UniverseObject> ObjectMap::getExisting(int id) const {
    auto it = m_existing_objects.find(id);
    if (it != m_existing_objects.end())
        return it->second;
    return nullptr;
}


