#include "Serialize.h"

#include "Logger.h"
#include "Serialize.ipp"

#include "../universe/IDAllocator.h"
#include "../universe/Building.h"
#include "../universe/Enums.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/Planet.h"
#include "../universe/ShipDesign.h"
#include "../universe/Species.h"
#include "../universe/System.h"
#include "../universe/Field.h"
#include "../universe/Universe.h"
#include "ScopedTimer.h"
#include "AppInterface.h"
#include "OptionsDB.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>

#if __has_include(<charconv>)
#include <charconv>
#else
#include <stdio.h>
#endif

namespace {
    template <typename T, std::size_t N>
    constexpr std::size_t ArrSize(std::array<T, N>)
    { return N; }
}

BOOST_CLASS_EXPORT(Field)
BOOST_CLASS_EXPORT(Universe)
BOOST_CLASS_VERSION(Universe, 2)

template <typename Archive>
void serialize(Archive& ar, PopCenter& p, unsigned int const version)
{
    ar  & boost::serialization::make_nvp("m_species_name", p.m_species_name);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, PopCenter&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, PopCenter&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, PopCenter&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, PopCenter&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, ResourceCenter& rs, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_focus", rs.m_focus)
        & make_nvp("m_last_turn_focus_changed", rs.m_last_turn_focus_changed)
        & make_nvp("m_focus_turn_initial", rs.m_focus_turn_initial)
        & make_nvp("m_last_turn_focus_changed_turn_initial", rs.m_last_turn_focus_changed_turn_initial);
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, ResourceCenter&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, ResourceCenter&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, ResourceCenter&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, ResourceCenter&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, ObjectMap& objmap, unsigned int const version)
{
    ar & boost::serialization::make_nvp("m_objects", objmap.m_objects);

    // If loading from the archive, propagate the changes to the specialized maps.
    if constexpr (Archive::is_loading::value)
        objmap.CopyObjectsToSpecializedMaps();
}

template <typename Archive>
void serialize(Archive& ar, Universe& u, unsigned int const version)
{
    using namespace boost::serialization;

    std::unique_ptr<ObjectMap>                objects_ptr = std::make_unique<ObjectMap>();
    ObjectMap& objects =                     *objects_ptr;
    std::set<int>                             destroyed_object_ids;
    Universe::EmpireObjectMap                 empire_latest_known_objects;
    Universe::EmpireObjectVisibilityMap       empire_object_visibility;
    Universe::EmpireObjectVisibilityTurnMap   empire_object_visibility_turns;
    Universe::ObjectKnowledgeMap              empire_known_destroyed_object_ids;
    Universe::ObjectKnowledgeMap              empire_stale_knowledge_object_ids;
    Universe::ShipDesignMap                   ship_designs;

    ar.template register_type<System>();

    static const std::string serializing_label = (Archive::is_loading::value ? "deserializing" : "serializing");

    SectionedScopedTimer timer("Universe " + serializing_label);

    if constexpr (Archive::is_saving::value) {
        DebugLogger() << "Universe::serialize : Getting gamestate data";
        timer.EnterSection("collecting data");
        u.GetObjectsToSerialize(              objects,                            GlobalSerializationEncodingForEmpire());
        u.GetDestroyedObjectsToSerialize(     destroyed_object_ids,               GlobalSerializationEncodingForEmpire());
        u.GetEmpireKnownObjectsToSerialize(   empire_latest_known_objects,        GlobalSerializationEncodingForEmpire());
        u.GetEmpireObjectVisibilityMap(       empire_object_visibility,           GlobalSerializationEncodingForEmpire());
        u.GetEmpireObjectVisibilityTurnMap(   empire_object_visibility_turns,     GlobalSerializationEncodingForEmpire());
        u.GetEmpireKnownDestroyedObjects(     empire_known_destroyed_object_ids,  GlobalSerializationEncodingForEmpire());
        u.GetEmpireStaleKnowledgeObjects(     empire_stale_knowledge_object_ids,  GlobalSerializationEncodingForEmpire());
        u.GetShipDesignsToSerialize(          ship_designs,                       GlobalSerializationEncodingForEmpire());
        timer.EnterSection("");
    }

    if constexpr (Archive::is_loading::value) {
        // clean up any existing dynamically allocated contents before replacing
        // containers with deserialized data.
        u.Clear();
    }

    ar  & make_nvp("m_universe_width", u.m_universe_width);
    DebugLogger() << "Universe::serialize : " << serializing_label << " universe width: " << u.m_universe_width;

    timer.EnterSection("designs");
    ar  & make_nvp("ship_designs", ship_designs);
    if constexpr (Archive::is_loading::value)
        u.m_ship_designs.swap(ship_designs);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << ship_designs.size() << " ship designs";

    ar  & make_nvp("m_empire_known_ship_design_ids", u.m_empire_known_ship_design_ids);

    timer.EnterSection("visibility / known destroyed or stale");
    ar  & make_nvp("empire_object_visibility", empire_object_visibility);
    ar  & make_nvp("empire_object_visibility_turns", empire_object_visibility_turns);
    if constexpr (Archive::is_loading::value) {
        u.m_empire_object_visibility.swap(empire_object_visibility);
        u.m_empire_object_visibility_turns.swap(empire_object_visibility_turns);
    }


    const auto copy_map = [](const auto& from, auto& to) {
        for (auto& [id, ids] : from)
            to.emplace(std::piecewise_construct,
                       std::forward_as_tuple(id),
                       std::forward_as_tuple(ids.begin(), ids.end()));
    };

    if constexpr (Archive::is_loading::value) {
        u.m_empire_known_destroyed_object_ids.clear();
        u.m_empire_stale_knowledge_object_ids.clear();
        if (version < 2) {
            std::map<int, std::set<int>> known_map;
            std::map<int, std::set<int>> stale_map;
            ar >> make_nvp("empire_known_destroyed_object_ids", known_map);
            ar >> make_nvp("empire_stale_knowledge_object_ids", stale_map);
            copy_map(known_map, u.m_empire_known_destroyed_object_ids);
            copy_map(stale_map, u.m_empire_stale_knowledge_object_ids);

        } else {
            std::map<int, std::vector<int>> known_map;
            std::map<int, std::vector<int>> stale_map;
            ar >> make_nvp("empire_known_destroyed_object_ids", known_map);
            ar >> make_nvp("empire_stale_knowledge_object_ids", stale_map);
            copy_map(known_map, u.m_empire_known_destroyed_object_ids);
            copy_map(stale_map, u.m_empire_stale_knowledge_object_ids);
        }

    } else { // saving
        std::map<int, std::vector<int>> known_map;
        std::map<int, std::vector<int>> stale_map;
        copy_map(empire_known_destroyed_object_ids, known_map);
        copy_map(empire_stale_knowledge_object_ids, stale_map);
        ar << make_nvp("empire_known_destroyed_object_ids", known_map);
        ar << make_nvp("empire_stale_knowledge_object_ids", stale_map);
    }
    timer.EnterSection("");

    DebugLogger() << "Universe::serialize : " << serializing_label
                  << " empire object visibility for " << empire_object_visibility.size() << ", "
                  << empire_object_visibility_turns.size() << ", "
                  << empire_known_destroyed_object_ids.size() << ", "
                  << empire_stale_knowledge_object_ids.size() <<  " empires";


    timer.EnterSection("objects");
    ar  & make_nvp("objects", objects);
    if constexpr (Archive::is_loading::value) {
        u.m_objects.swap(objects_ptr);

        // use the Universe u's flag to enable/disable StateChangedSignal for these UniverseObject
        for (auto* obj : u.m_objects->allRaw())
            obj->SetSignalCombiner(u);
    }
    timer.EnterSection("");
    DebugLogger() << "Universe::" << serializing_label << " " << u.m_objects->size() << " objects";

    timer.EnterSection("destroyed ids");
    ar  & make_nvp("destroyed_object_ids", destroyed_object_ids);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << destroyed_object_ids.size() << " destroyed object ids";
    if constexpr (Archive::is_loading::value) {
        u.m_destroyed_object_ids.clear();
        u.m_destroyed_object_ids.insert(destroyed_object_ids.begin(), destroyed_object_ids.end());
        u.m_objects->UpdateCurrentDestroyedObjects(u.m_destroyed_object_ids);
    }

    timer.EnterSection("latest known objects");
    ar  & make_nvp("empire_latest_known_objects", empire_latest_known_objects);
    DebugLogger() << "Universe::serialize : " << serializing_label << " empire known objects for " << empire_latest_known_objects.size() << " empires";
    if constexpr (Archive::is_loading::value) {
        u.m_empire_latest_known_objects.swap(empire_latest_known_objects);
    }

    timer.EnterSection("id allocator");
    if (version >= 1) {
        DebugLogger() << "Universe::serialize : " << serializing_label << " id allocator version = " << version;
        u.m_object_id_allocator->SerializeForEmpire(ar, version, GlobalSerializationEncodingForEmpire());
        u.m_design_id_allocator->SerializeForEmpire(ar, version, GlobalSerializationEncodingForEmpire());
    } else {
        if constexpr (Archive::is_loading::value) {
            int dummy_last_allocated_object_id;
            int dummy_last_allocated_design_id;
            DebugLogger() << "Universe::serialize : " << serializing_label << " legacy last allocated ids version = " << version;
            ar  & boost::serialization::make_nvp("m_last_allocated_object_id", dummy_last_allocated_object_id);
            DebugLogger() << "Universe::serialize : " << serializing_label << " legacy last allocated ids2";
            ar  & boost::serialization::make_nvp("m_last_allocated_design_id", dummy_last_allocated_design_id);

            DebugLogger() << "Universe::serialize : " << serializing_label << " legacy id allocator";
            // For legacy loads pre-dating the use of the IDAllocator the server
            // allocators need to be initialized with a list of the empires.
            std::vector<int> allocating_empire_ids;
            allocating_empire_ids.reserve(u.m_empire_latest_known_objects.size());
            std::transform(u.m_empire_latest_known_objects.begin(), u.m_empire_latest_known_objects.end(),
                           std::back_inserter(allocating_empire_ids),
                           [](const auto& ii) { return ii.first; });

            u.ResetAllIDAllocation(allocating_empire_ids);
        }
    }

    timer.EnterSection("stats");
    if (Archive::is_saving::value && GlobalSerializationEncodingForEmpire() != ALL_EMPIRES &&
        (!GetOptionsDB().Get<bool>("network.server.publish-statistics")))
    {
        std::map<std::string, std::map<int, std::map<int, double>>> dummy_stat_records;
        ar  & boost::serialization::make_nvp("m_stat_records", dummy_stat_records);
    } else {
        ar  & make_nvp("m_stat_records", u.m_stat_records);
        DebugLogger() << "Universe::serialize : " << serializing_label << " " << u.m_stat_records.size() << " types of statistic";
    }
    timer.EnterSection("");

    if constexpr (Archive::is_loading::value) {
        DebugLogger() << "Universe::serialize : updating empires' latest known object destruction states";
        // update known destroyed objects state in each empire's latest known
        // objects.
        for (auto& elko : u.m_empire_latest_known_objects) {
            auto destroyed_ids_it = u.m_empire_known_destroyed_object_ids.find(elko.first);
            if (destroyed_ids_it != u.m_empire_known_destroyed_object_ids.end())
                elko.second.UpdateCurrentDestroyedObjects(destroyed_ids_it->second);
        }
    }
    DebugLogger() << "Universe " << serializing_label << " done";
}



namespace {
    std::size_t ToChars(std::size_t num, char* buffer, char* buffer_end) {
#if defined(__cpp_lib_to_chars)
        auto result_ptr = std::to_chars(buffer, buffer_end, num).ptr;
        return std::distance(buffer, result_ptr);
#else
        std::size_t buffer_sz = std::distance(buffer, buffer_end);
        auto temp = std::to_string(num);
        auto out_sz = std::min(buffer_sz, temp.size());
        std::copy_n(temp.begin(), out_sz, buffer);
        return out_sz;
#endif
    }

    constexpr std::size_t num_meters_possible{static_cast<std::size_t>(MeterType::NUM_METER_TYPES)};
    constexpr std::size_t single_meter_text_size{ArrSize(Meter::ToCharsArrayT())};

    constexpr std::array<std::string_view, num_meters_possible + 2> tags{
        "inv",
        "POP", "IND", "RES", "INF", "CON", "STB",
        "CAP", "SEC",
        "FUL", "SHD", "STR", "DEF", "SUP", "STO", "TRP",
        "pop", "ind", "res", "inf", "con", "stb",
        "cap", "sec",
        "ful", "shd", "str", "def", "sup", "sto", "trp",
        "reb", "siz", "slt", "det", "spd",
        "num"};
    static_assert([]() -> bool {
        for (const auto& tag : tags)
            if (tag.size() != 3)
                return false;
        return true;
    }());

    constexpr std::string_view MeterTypeTag(MeterType mt) {
        using mt_under = std::underlying_type_t<MeterType>;
        static_assert(std::is_same_v<mt_under, int8_t>);
        static_assert(static_cast<mt_under>(MeterType::INVALID_METER_TYPE) == -1);
        auto mt_offset = static_cast<std::size_t>(MeterType(static_cast<mt_under>(mt) + 1));
        return tags.at(mt_offset);
    }
    static_assert(MeterTypeTag(MeterType::INVALID_METER_TYPE) == "inv");
    static_assert(MeterTypeTag(MeterType::METER_DEFENSE) == "def");
    static_assert(MeterTypeTag(MeterType::NUM_METER_TYPES) == "num");

    constexpr MeterType MeterTypeFromTag(std::string_view sv) {
        using mt_under = std::underlying_type_t<MeterType>;

        for (std::size_t idx = 0; idx < tags.size(); ++idx) {
            if (tags[idx] == sv)
                return MeterType(static_cast<mt_under>(idx) - 1);
        }
        return MeterType::INVALID_METER_TYPE;
    }
    static_assert(MeterTypeFromTag("inv") == MeterType::INVALID_METER_TYPE);
    static_assert(MeterTypeFromTag("RES") == MeterType::METER_TARGET_RESEARCH);
    static_assert(MeterTypeFromTag("num") == MeterType::NUM_METER_TYPES);


    /** Write text representation of meter type, current, and initial value.
      * Return number of consumed chars. */
    std::size_t ToChars(const UniverseObject::MeterMap::value_type& val, char* const buffer, char* const buffer_end) {
        const auto& [type, m] = val;

        if (std::distance(buffer, buffer_end) < 10)
            return 0;
        std::copy_n(MeterTypeTag(type).data(), 3, buffer); // tags should all be 3 chars
        auto result_ptr = buffer + 3;

        *result_ptr++ = ' ';
        result_ptr += m.ToChars(result_ptr, buffer_end);
        return std::distance(buffer, result_ptr);
    }


    template <typename Archive>
    void Serialize(Archive& ar, UniverseObject::MeterMap& meters, unsigned int const version)
    { ar & boost::serialization::make_nvp("m_meters", meters); }

    template <>
    void Serialize(boost::archive::xml_oarchive& ar, UniverseObject::MeterMap& meters,
                   unsigned int const)
    {
        using namespace boost::serialization;

        static constexpr std::size_t buffer_size = num_meters_possible * single_meter_text_size;
        static_assert(buffer_size > 100);

        std::array<std::string::value_type, buffer_size> buffer{};
        char* buffer_next = buffer.data();
        char* buffer_end = buffer.data() + buffer.size();

        // store number of meters
        buffer_next += ToChars(meters.size(), buffer_next, buffer_end);

        // store each meter as a triple of (metertype, current, initial)
        for (const auto& mt_meter : meters) {
            *buffer_next++ = ' ';
            buffer_next += ToChars(mt_meter, buffer_next, buffer_end);
        }

        std::string s{buffer.data()};
        ar << make_nvp("meters", s);
    }

    template <>
    void Serialize(boost::archive::xml_iarchive& ar, UniverseObject::MeterMap& meters,
                   unsigned int const version)
    {
        using namespace boost::serialization;

        static constexpr std::size_t buffer_size = num_meters_possible * single_meter_text_size;

        if (version < 4) {
            ar >> make_nvp("m_meters", meters);
            return;
        }

        std::string buffer;
        buffer.reserve(buffer_size);
        ar >> make_nvp("meters", buffer);

        unsigned int count = 0U;
        const char* const buffer_end = buffer.c_str() + buffer.size();

#if defined(__cpp_lib_to_chars)
        auto result = std::from_chars(buffer.c_str(), buffer_end, count);
        count = std::min(count, static_cast<unsigned int>(num_meters_possible));
        if (result.ec != std::errc())
            return;
        auto next{result.ptr};
#else
        int chars_consumed = 0;
        const char* next = buffer.data();
        auto matched = sscanf(next, "%u%n", &count, &chars_consumed);
        if (matched < 1)
            return;

        count = std::min(count, static_cast<unsigned int>(num_meters_possible));
        next += chars_consumed;
#endif
        while (std::distance(next, buffer_end) > 0 && *next == ' ')
            ++next;

        for (unsigned int idx = 0; idx < count; ++idx) {
            if (std::distance(next, buffer_end) < 7) // 7 is enough for "POP 0 0" or similar
                return;
            auto mt = MeterTypeFromTag(std::string_view(next, 3));
            next += 3;
            while (std::distance(next, buffer_end) > 0 && *next == ' ')
                ++next;

            Meter meter;
            auto consumed = meter.SetFromChars(std::string_view(next, std::distance(next, buffer_end)));
            if (consumed < 1)
                return;

            meters.emplace(mt, std::move(meter));

            next += consumed;
            while (std::distance(next, buffer_end) > 0 && *next == ' ')
                ++next;
        }
    }
}

BOOST_CLASS_EXPORT(UniverseObject)
BOOST_CLASS_VERSION(UniverseObject, 4)

template <typename Archive>
void serialize(Archive& ar, UniverseObject& o, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_id", o.m_id)
        & make_nvp("m_name", o.m_name)
        & make_nvp("m_x", o.m_x)
        & make_nvp("m_y", o.m_y)
        & make_nvp("m_owner_empire_id", o.m_owner_empire_id)
        & make_nvp("m_system_id", o.m_system_id);
    if (version < 3) {
        std::map<std::string, std::pair<int, float>> specials_map;
        ar  & make_nvp("m_specials", specials_map);
        o.m_specials.reserve(specials_map.size());
        o.m_specials.insert(specials_map.begin(), specials_map.end());
    } else {
        ar  & make_nvp("m_specials", o.m_specials);
    }
    if (version < 2) {
        std::map<MeterType, Meter> meter_map;
        ar  & make_nvp("m_meters", meter_map);
        o.m_meters.reserve(meter_map.size());
        o.m_meters.insert(meter_map.begin(), meter_map.end());
    } else {
        Serialize(ar, o.m_meters, version);
    }
    ar  & make_nvp("m_created_on_turn", o.m_created_on_turn);
}


template <typename Archive>
void load_construct_data(Archive& ar, System* obj, unsigned int const version)
{ ::new(obj)System(); }

template <typename Archive>
void serialize(Archive& ar, System& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_star", obj.m_star)
        & make_nvp("m_orbits", obj.m_orbits)
        & make_nvp("m_objects", obj.m_objects)
        & make_nvp("m_planets", obj.m_planets)
        & make_nvp("m_buildings", obj.m_buildings)
        & make_nvp("m_fleets", obj.m_fleets)
        & make_nvp("m_ships", obj.m_ships)
        & make_nvp("m_fields", obj.m_fields)
        & make_nvp("m_starlanes_wormholes", obj.m_starlanes_wormholes)
        & make_nvp("m_last_turn_battle_here", obj.m_last_turn_battle_here);
    if constexpr (Archive::is_loading::value)
        obj.m_system_id = obj.ID(); // override old value that was stored differently previously...
}

BOOST_CLASS_EXPORT(System)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, System&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, System&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, System&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, System&, unsigned int const);


template <typename Archive>
void load_construct_data(Archive& ar, Field* obj, unsigned int const version)
{ ::new(obj)Field(); }

template <typename Archive>
void serialize(Archive& ar, Field& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_type_name", obj.m_type_name);
}


template <typename Archive>
void load_construct_data(Archive& ar, Planet* obj, unsigned int const version)
{ ::new(obj)Planet(); }

template <typename Archive>
void serialize(Archive& ar, Planet& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("PopCenter", base_object<PopCenter>(obj))
        & make_nvp("ResourceCenter", base_object<ResourceCenter>(obj))
        & make_nvp("m_type", obj.m_type)
        & make_nvp("m_original_type", obj.m_original_type)
        & make_nvp("m_size", obj.m_size)
        & make_nvp("m_orbital_period", obj.m_orbital_period)
        & make_nvp("m_initial_orbital_position", obj.m_initial_orbital_position)
        & make_nvp("m_rotational_period", obj.m_rotational_period)
        & make_nvp("m_axial_tilt", obj.m_axial_tilt)
        & make_nvp("m_buildings", obj.m_buildings);
    if (version < 2) {
        // if deserializing an old save, default to standard default never-colonized turn
        obj.m_turn_last_colonized = INVALID_GAME_TURN;
        if (!obj.SpeciesName().empty()) // but if a planet has a species, it must have been colonized, so default to the previous turn
            obj.m_turn_last_colonized = CurrentTurn() - 1;
    } else {
        ar   & make_nvp("m_turn_last_colonized", obj.m_turn_last_colonized);
    }
    if (version < 1) {
        bool dummy = false;
        ar   & boost::serialization::make_nvp("m_just_conquered", dummy);
    } else {
        ar   & make_nvp("m_turn_last_conquered", obj.m_turn_last_conquered);
    }
    ar  & make_nvp("m_is_about_to_be_colonized", obj.m_is_about_to_be_colonized)
        & make_nvp("m_is_about_to_be_invaded", obj.m_is_about_to_be_invaded)
        & make_nvp("m_is_about_to_be_bombarded", obj.m_is_about_to_be_bombarded)
        & make_nvp("m_ordered_given_to_empire_id", obj.m_ordered_given_to_empire_id)
        & make_nvp("m_last_turn_attacked_by_ship", obj.m_last_turn_attacked_by_ship);
}

BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_VERSION(Planet, 2)


template <typename Archive>
void load_construct_data(Archive& ar, Building* obj, unsigned int const version)
{ ::new(obj)Building(); }

template <typename Archive>
void serialize(Archive& ar, Building& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_building_type", obj.m_building_type)
        & make_nvp("m_planet_id", obj.m_planet_id)
        & make_nvp("m_ordered_scrapped", obj.m_ordered_scrapped)
        & make_nvp("m_produced_by_empire_id", obj.m_produced_by_empire_id);
}

BOOST_CLASS_EXPORT(Building)


template <typename Archive>
void load_construct_data(Archive& ar, Fleet* obj, unsigned int const version)
{ ::new(obj)Fleet(); }

template <typename Archive>
void serialize(Archive& ar, Fleet& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_ships", obj.m_ships)
        & make_nvp("m_prev_system", obj.m_prev_system)
        & make_nvp("m_next_system", obj.m_next_system);

    if (Archive::is_loading::value && version < 5) {
        bool aggressive = false;
        ar  & make_nvp("m_aggressive", aggressive);
        obj.m_aggression = aggressive ? FleetAggression::FLEET_AGGRESSIVE : FleetAggression::FLEET_DEFENSIVE;

    } else {
        ar  & make_nvp("m_aggression", obj.m_aggression);
    }

    ar  & make_nvp("m_ordered_given_to_empire_id", obj.m_ordered_given_to_empire_id);
    if (version < 6) {
        std::list<int> travel_route;
        ar & make_nvp("m_travel_route", travel_route);
        obj.m_travel_route = std::vector(travel_route.begin(), travel_route.end());
    } else {
        ar & make_nvp("m_travel_route", obj.m_travel_route);
    }
    if (version < 3) {
        double dummy_travel_distance;
        ar & boost::serialization::make_nvp("m_travel_distance", dummy_travel_distance);
    }
    if (version >= 4) {
        ar & boost::serialization::make_nvp("m_last_turn_move_ordered", obj.m_last_turn_move_ordered);
    }
    ar  & make_nvp("m_arrived_this_turn", obj.m_arrived_this_turn)
        & make_nvp("m_arrival_starlane", obj.m_arrival_starlane);
}

BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_VERSION(Fleet, 6)


template <typename Archive>
void serialize(Archive& ar, Ship& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_design_id", obj.m_design_id)
        & make_nvp("m_fleet_id", obj.m_fleet_id)
        & make_nvp("m_ordered_scrapped", obj.m_ordered_scrapped)
        & make_nvp("m_ordered_colonize_planet_id", obj.m_ordered_colonize_planet_id)
        & make_nvp("m_ordered_invade_planet_id", obj.m_ordered_invade_planet_id)
        & make_nvp("m_ordered_bombard_planet_id", obj.m_ordered_bombard_planet_id)
        & make_nvp("m_part_meters", obj.m_part_meters)
        & make_nvp("m_species_name", obj.m_species_name)
        & make_nvp("m_produced_by_empire_id", obj.m_produced_by_empire_id)
        & make_nvp("m_arrived_on_turn", obj.m_arrived_on_turn);
    if (version >= 1) {
        ar  & make_nvp("m_last_turn_active_in_combat", obj.m_last_turn_active_in_combat);
        if (version >= 2) {
            ar  & make_nvp("m_last_resupplied_on_turn", obj.m_last_resupplied_on_turn);
        }
    }
}

BOOST_CLASS_EXPORT(Ship)
BOOST_CLASS_VERSION(Ship, 2)


template <typename Archive>
void serialize(Archive& ar, ShipDesign& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_id", obj.m_id)
        & make_nvp("m_name", obj.m_name);

    TraceLogger() << "ship design serialize version: " << version << " : " << (Archive::is_saving::value ? "saving" : "loading");

    if (version >= 1) {
        // Serialization of m_uuid as a primitive doesn't work as expected from
        // the documentation.  This workaround instead serializes a string
        // representation.
        if constexpr (Archive::is_saving::value) {
            auto string_uuid = boost::uuids::to_string(obj.m_uuid);
            ar & make_nvp("string_uuid", string_uuid);
        } else {
            std::string string_uuid;
            ar & make_nvp("string_uuid", string_uuid);
            try {
                obj.m_uuid = boost::lexical_cast<boost::uuids::uuid>(string_uuid);
            } catch (const boost::bad_lexical_cast&) {
                obj.m_uuid = boost::uuids::nil_generator()();
            }
        }
    } else if constexpr (Archive::is_loading::value) {
        obj.m_uuid = boost::uuids::nil_generator()();
    }

    ar  & make_nvp("m_description", obj.m_description)
        & make_nvp("m_designed_on_turn", obj.m_designed_on_turn);
    if (version >= 2)
        ar  & make_nvp("m_designed_by_empire", obj.m_designed_by_empire);
    ar  & make_nvp("m_hull", obj.m_hull)
        & make_nvp("m_parts", obj.m_parts)
        & make_nvp("m_is_monster", obj.m_is_monster)
        & make_nvp("m_icon", obj.m_icon)
        & make_nvp("m_3D_model", obj.m_3D_model)
        & make_nvp("m_name_desc_in_stringtable", obj.m_name_desc_in_stringtable);
    if constexpr (Archive::is_loading::value) {
        obj.ForceValidDesignOrThrow(boost::none, true);
        obj.BuildStatCaches();
    }
}

BOOST_CLASS_EXPORT(ShipDesign)
BOOST_CLASS_VERSION(ShipDesign, 2)


template <typename Archive>
void serialize(Archive& ar, SpeciesManager& sm, unsigned int const version)
{
    // Don't need to send all the data about species, as this is derived from
    // content data files in scripting/species that should be available to any
    // client or server.  Instead, just need to send the gamestate portion of
    // species: their homeworlds in the current game, and their opinions of
    // empires and eachother

    std::map<std::string, std::set<int>>                species_homeworlds;
    std::map<std::string, std::map<int, float>>         empire_opinions;
    std::map<std::string, std::map<std::string, float>> other_species_opinions;
    std::map<std::string, std::map<int, float>>         species_object_populations;
    std::map<std::string, std::map<std::string, int>>   species_ships_destroyed;

    if constexpr (Archive::is_saving::value) {
        species_homeworlds = sm.GetSpeciesHomeworldsMap(GlobalSerializationEncodingForEmpire());
        empire_opinions = sm.GetSpeciesEmpireOpinionsMap(GlobalSerializationEncodingForEmpire());
        other_species_opinions = sm.GetSpeciesSpeciesOpinionsMap(GlobalSerializationEncodingForEmpire());
        species_object_populations = sm.SpeciesObjectPopulations(GlobalSerializationEncodingForEmpire());
        species_ships_destroyed = sm.SpeciesShipsDestroyed(GlobalSerializationEncodingForEmpire());
    }

    ar  & BOOST_SERIALIZATION_NVP(species_homeworlds)
        & BOOST_SERIALIZATION_NVP(empire_opinions)
        & BOOST_SERIALIZATION_NVP(other_species_opinions)
        & BOOST_SERIALIZATION_NVP(species_object_populations)
        & BOOST_SERIALIZATION_NVP(species_ships_destroyed);

    if constexpr (Archive::is_loading::value) {
        sm.SetSpeciesHomeworlds(std::move(species_homeworlds));
        sm.SetSpeciesEmpireOpinions(std::move(empire_opinions));
        sm.SetSpeciesSpeciesOpinions(std::move(other_species_opinions));
        sm.SetSpeciesObjectPopulations(std::move(species_object_populations));
        sm.SetSpeciesShipsDestroyed(std::move(species_ships_destroyed));
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SpeciesManager&, unsigned int const);

template <typename Archive>
void Serialize(Archive& oa, const Universe& universe)
{ oa << BOOST_SERIALIZATION_NVP(universe); }
template FO_COMMON_API void Serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& oa, const Universe& universe);
template FO_COMMON_API void Serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& oa, const Universe& universe);

template <typename Archive>
void Serialize(Archive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects)
{ oa << BOOST_SERIALIZATION_NVP(objects); }
template void Serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects);
template void Serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& oa, const std::map<int, std::shared_ptr<UniverseObject>>& objects);

template <typename Archive>
void Deserialize(Archive& ia, Universe& universe)
{ ia >> BOOST_SERIALIZATION_NVP(universe); }
template FO_COMMON_API void Deserialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ia, Universe& universe);
template FO_COMMON_API void Deserialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ia, Universe& universe);

template <typename Archive>
void Deserialize(Archive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects)
{ ia >> BOOST_SERIALIZATION_NVP(objects); }
template void Deserialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects);
template void Deserialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ia, std::map<int, std::shared_ptr<UniverseObject>>& objects);

namespace boost::serialization {
    template<class Archive, class Key, class Value>
    void save(Archive& ar, const flat_map<Key, Value>& m, const unsigned int)
    { stl::save_collection<Archive, flat_map<Key, Value>>(ar, m); }

    template<class Archive, class Key, class Value>
    void load(Archive& ar, flat_map<Key, Value>& m, const unsigned int)
    { load_map_collection(ar, m); }

    template<class Archive, class Key, class Value>
    void serialize(Archive& ar, flat_map<Key, Value>& m, const unsigned int file_version)
    { split_free(ar, m, file_version); }

    // Note: I tried loading the internal vector of a flat_map instead of
    //       loading it as a map and constructing elements on the stack.
    //       The result was similar or slightly slower than the stack loader.
}
