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

    using const_planet_raw_ptr_t0 = decltype(std::declval<const ObjectMap>().getRaw<Planet>(INVALID_OBJECT_ID));
    static_assert(std::is_same_v<const_planet_raw_ptr_t0, const Planet*>);
    using const_planet_raw_ptr_t1 = decltype(std::declval<const ObjectMap>().getRaw<Planet>([](auto) { return true; }));
    static_assert(std::is_same_v<const_planet_raw_ptr_t1, const Planet*>);
    using const_planet_raw_ptr_t2 = decltype(std::declval<const ObjectMap>().getRaw<Planet>([](const auto&) { return true; }));
    static_assert(std::is_same_v<const_planet_raw_ptr_t2, const Planet*>);
    using const_planet_raw_ptr_t3 = decltype(std::declval<ObjectMap>().getRaw<Planet>([](const std::pair<int, const Planet>&) { return true; }));
    static_assert(std::is_same_v<const_planet_raw_ptr_t3, const Planet*>);
    using const_planet_raw_ptr_t4 = decltype(std::declval<ObjectMap>().getRaw<Planet>([](const std::pair<int, Planet>&) { return true; }));
    static_assert(std::is_same_v<const_planet_raw_ptr_t4, const Planet*>);
    using const_planet_raw_ptr_t5 = decltype(std::declval<ObjectMap>().getRaw<Planet>([](const std::shared_ptr<const Planet>&) { return true; }));
    static_assert(std::is_same_v<const_planet_raw_ptr_t5, const Planet*>);
}

namespace ObjectMapPredicateTypeTraits {
    static_assert(!int_iterable<int>);
    static_assert(!int_iterable<std::array<float, 5>>);
    static_assert(int_iterable<std::array<const int, 42>>);
    static_assert(int_iterable<boost::container::flat_set<int, std::greater<>>>);
    static_assert(int_iterable<std::vector<int>>);

    static_assert(is_sorted<std::set<int>>);
    static_assert(is_sorted<std::set<std::string, std::less<>>>);
    static_assert(is_sorted<std::multiset<int>>);
    static_assert(is_sorted<boost::container::flat_set<int>>);
    static_assert(!is_sorted<std::unordered_map<int, float>>);
    static_assert(!is_sorted<std::vector<float>>);
    static_assert(!is_sorted<float>);

    static_assert(is_unordered_set<std::unordered_set<std::vector<float>>>);
    static_assert(is_boost_unordered_set<boost::unordered::unordered_set<std::vector<float>>>);

    static_assert(is_unique_set<std::set<int, std::less<>>>);
    static_assert(is_unique_set<std::set<std::string>>);
    static_assert(!is_unique_set<std::multiset<int, std::less<>>>);
    static_assert(is_unique_set<boost::container::flat_set<int>>);
    static_assert(!is_unique_set<std::unordered_map<int, float>>);
    static_assert(is_unique_set<std::unordered_set<float, std::equal_to<>>>);
    static_assert(!is_unique_set<std::vector<float>>);
    static_assert(!is_unique_set<std::unique_ptr<std::string>>);

    /*
    invokable_on_raw_const_object, invokable_on_raw_mutable_object,
    invokable_on_shared_const_object, invokable_on_shared_mutable_object,
    invokable_on_const_entry, invokable_on_mutable_entry,
    invokable_on_const_reference, invokable_on_mutable_reference,
    invokable, is_visitor, is_int_range, invokable_on_int
    */
    constexpr auto fsiv = ObjectMap::CheckTypes<Ship, std::vector<int>>();
    static_assert(fsiv == std::array{false, false, false, false, false, false, false, false, false, false, true, false});

    constexpr auto fcuifs = ObjectMap::CheckTypes<UniverseObject, boost::container::flat_set<int>>();
    static_assert(fcuifs == std::array{false, false, false, false, false, false, false, false, false, false, true, false});

    constexpr auto ship_p_lambda = [](const Ship*) -> bool { return false; };
    constexpr auto lspbicsp = ObjectMap::CheckTypes<Ship, decltype(ship_p_lambda)>();
    static_assert(lspbicsp == std::array{true, true, false, false, false, false, false, false, true, false, false, false});

    constexpr auto int_lambda = [](const int) -> bool { return false; };
    constexpr auto libi = ObjectMap::CheckTypes<Ship, decltype(int_lambda)>();
    static_assert(libi == std::array{false, false, false, false, false, false, false, false, true, false, false, true});
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

    const int source_id = source->ID();

    // can empire see object at all?  if not, skip copying object's info
    if (empire_id != ALL_EMPIRES &&
        universe.GetObjectVisibilityByEmpire(source_id, empire_id) <= Visibility::VIS_NO_VISIBILITY)
    { return; }

    if (auto destination = this->get(source_id)) {
        destination->Copy(*source, universe, empire_id); // there already is a version of this object present in this ObjectMap, so just update it
    } else {
        bool destroyed = universe.DestroyedObjectIds().contains(source_id);
        // this object is not yet present in this ObjectMap, so add a new UniverseObject object for it
        insertCore(source->Clone(universe), destroyed);
    }
}

std::unique_ptr<ObjectMap> ObjectMap::Clone(const Universe& universe, int empire_id) const {
    auto result = std::make_unique<ObjectMap>();
    result->Copy(*this, universe, empire_id);
    return result;
}

int ObjectMap::HighestObjectID() const {
    if (m_objects.empty())
        return INVALID_OBJECT_ID;
    return m_objects.rbegin()->first;
}

template <typename Function>
void ObjectMap::ApplyToExistingMaps(Function func, bool include_nonspecialized)
{
    static_assert(requires (Function func) { func(m_existing_objects); });

    if (include_nonspecialized)
        func(m_existing_objects);
    func(m_existing_ships);
    func(m_existing_fleets);
    func(m_existing_planets);
    func(m_existing_systems);
    func(m_existing_buildings);
    func(m_existing_fields);
}

template <typename Function>
void ObjectMap::ApplyToCoreMaps(Function func, bool include_nonspecialized)
{
    static_assert(requires (Function func) { func(m_objects); });
    static_assert(requires (Function func) { func(m_ships); });
    static_assert(requires (Function func) { func(m_fleets); });
    static_assert(requires (Function func) { func(m_planets); });
    static_assert(requires (Function func) { func(m_systems); });
    static_assert(requires (Function func) { func(m_buildings); });
    static_assert(requires (Function func) { func(m_fields); });

    if (include_nonspecialized)
        func(m_objects);
    func(m_ships);
    func(m_fleets);
    func(m_planets);
    func(m_systems);
    func(m_buildings);
    func(m_fields);
}

template <typename Function>
void ObjectMap::ApplyToExistingVecs(Function func, bool include_nonspecialized)
{
    static_assert(requires (Function func) { func(m_existing_object_vec); });

    if (include_nonspecialized)
        func(m_existing_object_vec);
    func(m_existing_ship_vec);
    func(m_existing_fleet_vec);
    func(m_existing_planet_vec);
    func(m_existing_system_vec);
    func(m_existing_building_vec);
    func(m_existing_field_vec);
}

namespace {
    // wraps static_pointer_cast
    template <typename ObjectType>
    auto SCast(auto&& obj)
    { return std::static_pointer_cast<ObjectType>(std::forward<decltype(obj)>(obj)); };
}

template <typename ObjectType>
void ObjectMap::TypedInsertExisting(auto ID, std::shared_ptr<ObjectType> obj) {
    using OT = std::decay_t<ObjectType>;

    auto& evec = ExistingVec<OT>();
    const auto* raw_obj = obj.get();
    if (std::find(evec.begin(), evec.end(), raw_obj) == evec.end()) // avoid inserting duplicates
        evec.push_back(raw_obj);

    //std::cout << obj->ID() << " -> " << ID << " (" << to_string(raw_obj->ObjectType()) << ") "
    //          << " as " << typeid(ObjectType).name() << std::endl;

    auto& emap = ExistingMap<OT>();
    emap.insert_or_assign(ID, std::move(obj));
}

void ObjectMap::AutoTypedInsertExisting(auto ID, auto&& obj) {
    if (!obj)
        return;
    switch (obj->ObjectType()) {
    case UniverseObjectType::OBJ_BUILDING:  TypedInsertExisting(ID, SCast<Building>(std::forward<decltype(obj)>(obj)));   break;
    case UniverseObjectType::OBJ_SHIP:      TypedInsertExisting(ID, SCast<Ship>(std::forward<decltype(obj)>(obj)));       break;
    case UniverseObjectType::OBJ_FLEET:     TypedInsertExisting(ID, SCast<Fleet>(std::forward<decltype(obj)>(obj)));      break;
    case UniverseObjectType::OBJ_PLANET:    TypedInsertExisting(ID, SCast<Planet>(std::forward<decltype(obj)>(obj)));     break;
    case UniverseObjectType::OBJ_SYSTEM:    TypedInsertExisting(ID, SCast<System>(std::forward<decltype(obj)>(obj)));     break;
    case UniverseObjectType::OBJ_FIELD:     TypedInsertExisting(ID, SCast<Field>(std::forward<decltype(obj)>(obj)));      break;
    case UniverseObjectType::OBJ_FIGHTER:
    default: break;
    }
}

template <typename ObjectType>
void ObjectMap::TypedInsert(auto ID, auto destroyed, std::shared_ptr<ObjectType> obj) {
    if (!obj) {
        //std::cout << "null -> " << ID << " as " << typeid(ObjectType).name() << std::endl;
        return;
    }
    //std::cout << obj->ID() << " -> " << ID << " (" << (destroyed ? "destroyed" : "existing")
    //    << " (" << to_string(obj->ObjectType()) << ") as " << typeid(ObjectType).name() << std::endl;

    if (!destroyed)
        TypedInsertExisting(ID, obj);
    Map<std::decay_t<ObjectType>>().insert_or_assign(ID, std::move(obj));
}

void ObjectMap::AutoTypedInsert(auto ID, auto destroyed, auto&& obj) {
    if (!obj)
        return;
    switch (obj->ObjectType()) {
    case UniverseObjectType::OBJ_BUILDING:  TypedInsert(ID, destroyed, SCast<Building>(std::forward<decltype(obj)>(obj)));   break;
    case UniverseObjectType::OBJ_SHIP:      TypedInsert(ID, destroyed, SCast<Ship>(std::forward<decltype(obj)>(obj)));       break;
    case UniverseObjectType::OBJ_FLEET:     TypedInsert(ID, destroyed, SCast<Fleet>(std::forward<decltype(obj)>(obj)));      break;
    case UniverseObjectType::OBJ_PLANET:    TypedInsert(ID, destroyed, SCast<Planet>(std::forward<decltype(obj)>(obj)));     break;
    case UniverseObjectType::OBJ_SYSTEM:    TypedInsert(ID, destroyed, SCast<System>(std::forward<decltype(obj)>(obj)));     break;
    case UniverseObjectType::OBJ_FIELD:     TypedInsert(ID, destroyed, SCast<Field>(std::forward<decltype(obj)>(obj)));      break;
    case UniverseObjectType::OBJ_FIGHTER:
    default: break;
    }
}

void ObjectMap::insertCore(std::shared_ptr<UniverseObject> obj, bool destroyed) {
    if (!obj)
        return;
    const auto ID = obj->ID();

    TypedInsert(ID, destroyed, obj);
    AutoTypedInsert(ID, destroyed, std::move(obj));
}

std::shared_ptr<UniverseObject> ObjectMap::erase(int id) {
    // search for object in objects map
    auto it = m_objects.find(id);
    if (it == m_objects.end())
        return nullptr;

    // object found, so store pointer to keep alive and to return
    auto result{std::move(it->second)};

    // remove from existing and core vectors and maps...
    auto erase_from_vec = [o{result.get()}](auto& vec) {
        const auto it = std::find(vec.begin(), vec.end(), o);
        if (it != vec.end()) // cannot pass end() to vector::erase
            vec.erase(it);
    };
    ApplyToExistingVecs(erase_from_vec);
    ApplyToExistingMaps([id](auto& map) { map.erase(id); });
    ApplyToCoreMaps([id](auto& map) { map.erase(id); });

    return result;
}

void ObjectMap::clear() {
    ApplyToExistingVecs([](auto& vec) { vec.clear(); });
    ApplyToExistingMaps([](auto& map) { map.clear(); });
    ApplyToCoreMaps([](auto& map) { map.clear(); });
}

std::vector<int> ObjectMap::FindExistingObjectIDs() const {
    auto key_rng = m_existing_objects | range_keys;
    return {key_rng.begin(), key_rng.end()};
}

void ObjectMap::UpdateCurrentDestroyedObjects(const std::unordered_set<int>& destroyed_object_ids) {
    ApplyToExistingVecs([](auto& vec) { vec.clear(); });
    ApplyToExistingMaps([](auto& map) { map.clear(); });

    for (const auto& [ID, obj] : m_objects) {
        if (!destroyed_object_ids.contains(ID)) {
            TypedInsertExisting<UniverseObject>(ID, obj);
            AutoTypedInsertExisting(ID, obj);
        }
    }

    //std::cout << "updated existing" << std::endl;
    //ApplyToExistingMaps([](auto& map) { std::cout << "map " << typeid(map).name() << " sz: " << map.size() << std::endl; }, true);
}

void ObjectMap::AuditContainment(const std::unordered_set<int>& destroyed_object_ids) {
    // determine all objects that some other object thinks contains them
    std::map<int, std::set<int>> contained_objs; // TODO: use boost flat / unordered sets?
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
    //std::cout << "starting existing map/vec clears" << std::endl;
    ApplyToExistingMaps([](auto& map) { map.clear(); }, false);
    //std::cout << "starting auto typed inserts" << std::endl;
    static constexpr bool treat_as_destroyed = true;
    for (const auto& [ID, obj] : Map<UniverseObject>())
        AutoTypedInsert(ID, treat_as_destroyed, obj);

    //std::cout << "copied to maps/vecs" << std::endl;
    //ApplyToExistingMaps([](auto& map) { std::cout << "map " << typeid(map).name() << " sz: " << map.size() << std::endl; }, true);
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


