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
#include "../universe/System.h"
#include "../universe/Field.h"
#include "../universe/Universe.h"
#include "ScopedTimer.h"
#include "AppInterface.h"
#include "OptionsDB.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>

#include <numeric>
#if __has_include(<charconv>)
#include <charconv>
#else
#include <cstdio>
#endif
#include <type_traits>

namespace {
    // <concepts> library not fully implemented in XCode 13.2
    template <class T>
    concept integral = std::is_integral_v<T>;
}

BOOST_CLASS_EXPORT(Field)
BOOST_CLASS_EXPORT(Universe)
BOOST_CLASS_VERSION(Universe, 3)


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

    ObjectMap                                 objects;
    std::set<int>                             destroyed_object_ids;
    Universe::EmpireObjectMap                 empire_latest_known_objects;
    Universe::EmpireObjectVisibilityMap       empire_object_visibility;
    Universe::EmpireObjectVisibilityTurnMap   empire_object_visibility_turns;
    Universe::ObjectKnowledgeMap              empire_known_destroyed_object_ids;
    Universe::ObjectKnowledgeMap              empire_stale_knowledge_object_ids;
    Universe::ShipDesignMap                   ship_designs_scratch;

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
    }

    const auto& designs_to_serialize = [&timer, &ship_designs_scratch, &u]() {
        if constexpr(Archive::is_saving::value) {
            // when saving, get a reference to either the full universe ship designs map or
            // a filtered subset to be encoded for a particular empire. this call may
            // fill ship_designs_scratch and return it, or just retern a universe internal
            // map of ShipDesign
            const auto& retval = u.GetShipDesignsToSerialize(ship_designs_scratch,
                                                             GlobalSerializationEncodingForEmpire());
            timer.EnterSection("");
            return retval;
        } else {
            (void)timer; // silence unused capture warning
            (void)u;
            return ship_designs_scratch;
        }
    }();

    if constexpr (Archive::is_loading::value) {
        // clean up any existing dynamically allocated contents before replacing
        // containers with deserialized data.
        u.Clear();
    }

    ar  & make_nvp("m_universe_width", u.m_universe_width);
    DebugLogger() << "Universe::serialize : " << serializing_label << " universe width: " << u.m_universe_width;

    timer.EnterSection("designs");
    if constexpr (Archive::is_loading::value) {
        if (version >= 3) {
            ar >> make_nvp("ship_designs", ship_designs_scratch);

        } else {
            std::map<int, ShipDesign*> design_ptrs;
            ar  & make_nvp("ship_designs", design_ptrs);
            for (auto& [id, ptr] : design_ptrs) {
                ship_designs_scratch.emplace(id, std::move(*ptr));
                delete ptr;
            }
        }

        u.m_ship_designs.swap(ship_designs_scratch);
        DebugLogger() << "Universe::deserialized : " << serializing_label << " " << u.m_ship_designs.size() << " ship designs";

    } else { // saving
        ar << make_nvp("ship_designs", designs_to_serialize);
        DebugLogger() << "Universe::serialized : " << serializing_label << " " << designs_to_serialize.size() << " ship designs";
    }

    ar  & make_nvp("m_empire_known_ship_design_ids", u.m_empire_known_ship_design_ids);

    timer.EnterSection("visibility / known destroyed or stale");
    ar  & make_nvp("empire_object_visibility", empire_object_visibility);
    ar  & make_nvp("empire_object_visibility_turns", empire_object_visibility_turns);
    if constexpr (Archive::is_loading::value) {
        u.m_empire_object_visibility.swap(empire_object_visibility);
        u.m_empire_object_visibility_turns.swap(empire_object_visibility_turns);
    }


    static constexpr auto copy_map = [](const auto& from, auto& to) {
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
        u.m_objects = std::move(objects);

        // use the Universe u's flag to enable/disable StateChangedSignal for these UniverseObject
        for (auto* obj : u.m_objects.allRaw())
            obj->SetSignalCombiner(u);
    }
    timer.EnterSection("");
    DebugLogger() << "Universe::" << serializing_label << " " << u.m_objects.size() << " objects";

    timer.EnterSection("destroyed ids");
    ar  & make_nvp("destroyed_object_ids", destroyed_object_ids);
    DebugLogger() << "Universe::serialize : " << serializing_label << " " << destroyed_object_ids.size() << " destroyed object ids";
    if constexpr (Archive::is_loading::value) {
        u.m_destroyed_object_ids.clear();
        u.m_destroyed_object_ids.insert(destroyed_object_ids.begin(), destroyed_object_ids.end());
        u.m_objects.UpdateCurrentDestroyedObjects(u.m_destroyed_object_ids);
    }

    timer.EnterSection("latest known objects");
    ar  & make_nvp("empire_latest_known_objects", empire_latest_known_objects);
    DebugLogger() << "Universe::serialize : " << serializing_label << " empire known objects for " << empire_latest_known_objects.size() << " empires";
    if constexpr (Archive::is_loading::value)
        u.m_empire_latest_known_objects.swap(empire_latest_known_objects);


    timer.EnterSection("id allocator");
    if (version >= 1) {
        DebugLogger() << "Universe::serialize : " << serializing_label << " id allocator version = " << version;
        u.m_object_id_allocator->SerializeForEmpire(ar, version, GlobalSerializationEncodingForEmpire());
        u.m_design_id_allocator->SerializeForEmpire(ar, version, GlobalSerializationEncodingForEmpire());
    } else {
        if constexpr (Archive::is_loading::value) {
            int dummy_last_allocated_object_id = INVALID_OBJECT_ID;
            int dummy_last_allocated_design_id = INVALID_DESIGN_ID;
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
    static constexpr auto objects_mem_size = [](const ObjectMap& o) {
        const auto& all_objs = o.allWithIDs<UniverseObject>();
        return std::transform_reduce(all_objs.begin(), all_objs.end(), 0u, std::plus<>{},
                                     [](const auto& id_obj) { return sizeof(id_obj) + id_obj.second->SizeInMemory(); });
    };

    const auto& eo = u.m_empire_latest_known_objects;
    const auto empire_objects_mem_sz =
        std::transform_reduce(eo.begin(), eo.end(), 0u, std::plus<>{},
                              [](auto& o) { return objects_mem_size(o.second) + sizeof(o); });

    DebugLogger() << "Universe " << serializing_label << " done. UniverseObjects take at least: "
                  << objects_mem_size(u.m_objects)/1024 << " kB and empire known objects at least: "
                  << empire_objects_mem_sz/1024 << " kB";
    DebugLogger() << "Universe other stuff takes at least: " << u.SizeInMemory()/1024u << " kB";
}



namespace {
#if defined(__cpp_lib_to_chars) && defined(__cpp_lib_constexpr_charconv)
    constexpr
#endif
    auto ToChars(integral auto num, char* buffer, char* buffer_end) {
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
    constexpr std::size_t single_meter_text_size{std::size(Meter::ToCharsArrayT())};

    constexpr std::array<std::string_view, num_meters_possible + 2> tags{
        "inv", // invalid
        "POP", "IND", "RES", "INF", "CON", "STB", // target/max meters
        "CAP", "SEC",
        "FUL", "SHD", "STR", "DEF", "SUP", "STO", "TRP",
        "pop", "ind", "res", "inf", "con", "stb", // paired active/current meters
        "cap", "sec",
        "ful", "shd", "str", "def", "sup", "sto", "trp", // unpaired meters
        "reb", "siz", "slt", "det", "spd",
        "num"}; // num meter types
    static_assert(std::all_of(tags.begin(), tags.end(), [](const auto tag) { return tag.size() == 3; }));

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
        static_assert(std::is_signed_v<mt_under>);

        for (std::size_t idx = 0; idx < tags.size(); ++idx) {
            if (tags[idx] == sv)
                return MeterType(static_cast<mt_under>(idx) - 1); // tag at index 0 corresponds to INVALID_METER_TYPE = -1
        }
        return MeterType::INVALID_METER_TYPE;
    }
    static_assert(MeterTypeFromTag("inv") == MeterType::INVALID_METER_TYPE);
    static_assert(MeterTypeFromTag("RES") == MeterType::METER_TARGET_RESEARCH);
    static_assert(MeterTypeFromTag("num") == MeterType::NUM_METER_TYPES);


    /** Write text representation of meter type, current, and initial value.
      * Return number of written chars. */
    inline auto ToChars(MeterType type, const Meter& m, char* const buffer, char* const buffer_end) {
        using retval_t = decltype(std::distance(buffer, buffer_end));

        if (std::distance(buffer, buffer_end) < 10)
            return retval_t{0};

        std::copy_n(MeterTypeTag(type).data(), 3, buffer); // tags should all be 3 chars
        auto result_ptr = buffer + 3;

        *result_ptr++ = ' ';
        result_ptr += m.ToChars(result_ptr, buffer_end); // Meter::ToChars is currently not constexpr, making this whole function not constexpr
        return std::distance(buffer, result_ptr);
    }

    inline auto ToChars(const UniverseObject::MeterMap::value_type& val, char* const buffer, char* const buffer_end)
    { return ToChars(val.first, val.second, buffer, buffer_end); }

    constexpr bool have_to_chars_lib =
#if defined(__cpp_lib_to_chars)
        true;
#else
        false;
#endif

    template <integral T>
    constexpr const auto* GetFormatString() {
        if constexpr(std::is_unsigned_v<T>)
            return "%u%n";
        else if constexpr(std::is_signed_v<T>)
            return "%d%n";
        else
            return "";
    }

    // returns { next unconsumed char*, true/false did the parse succeed }
    // parsed value returned in result
    inline auto FromChars(const char* start, const char* end, integral auto& val_out) -> std::pair<const char*, bool>
    {
        if constexpr(have_to_chars_lib) {
            const auto result = std::from_chars(start, end, val_out);
            return {result.ptr, result.ec == std::errc()};

        } else {
            int chars_consumed = 0;
            using val_out_t = std::decay_t<decltype(val_out)>;
            constexpr auto val_format_str = GetFormatString<val_out_t>();
            const auto matched = sscanf(start, val_format_str, &val_out, &chars_consumed);
            return {start + chars_consumed, matched >= 1};
        }
    }


    template <typename Archive>
    void Serialize(Archive& ar, UniverseObject::MeterMap& meters, unsigned int const version)
    { ar & boost::serialization::make_nvp("m_meters", meters); }

    template <>
    void Serialize(boost::archive::xml_oarchive& ar, UniverseObject::MeterMap& meters, unsigned int const)
    {
        static constexpr std::size_t buffer_size = num_meters_possible * single_meter_text_size;
        static_assert(buffer_size > 100);

        std::array<std::string::value_type, buffer_size> buffer{};
        auto* buffer_next = buffer.data();
        auto* buffer_end = buffer.data() + buffer.size();

        // store number of meters
        buffer_next += ToChars(meters.size(), buffer_next, buffer_end);

        // store each meter as a triple of (metertype, current, initial)
        for (const auto& mt_meter : meters) {
            *buffer_next++ = ' ';
            buffer_next += ToChars(mt_meter, buffer_next, buffer_end);
        }

        std::string s{buffer.data()};
        ar << boost::serialization::make_nvp("meters", s);
    }

    template <>
    void Serialize(boost::archive::xml_iarchive& ar, UniverseObject::MeterMap& meters,
                   unsigned int const version)
    {
        static constexpr std::size_t buffer_size = num_meters_possible * single_meter_text_size;

        if (version < 4) {
            // old format used the default serialization for maps
            ar >> boost::serialization::make_nvp("m_meters", meters);
            return;
        }

        // interpret custom string representation of meters

        std::string buffer;
        buffer.reserve(buffer_size);
        ar >> boost::serialization::make_nvp("meters", buffer);

        unsigned int count = 0U;
        const char* const buffer_end = buffer.c_str() + buffer.size();

        auto [next, success] = FromChars(buffer.c_str(), buffer_end, count);
        if (!success)
            return;
        count = std::min<unsigned int>(count, num_meters_possible);
        meters.reserve(count);

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
            const auto consumed = meter.SetFromChars(std::string_view(next, std::distance(next, buffer_end)));
            if (consumed < 1)
                return;

            meters.insert_or_assign(mt, std::move(meter));

            next += consumed;
            while (std::distance(next, buffer_end) > 0 && *next == ' ')
                ++next;
        }
    }


    template <typename Archive>
    void Serialize(Archive& ar, Ship::PartMeterMap& meters, unsigned int const version)
    { ar & boost::serialization::make_nvp("m_part_meters", meters); }

    template <>
    void Serialize(boost::archive::xml_oarchive& ar, Ship::PartMeterMap& meters, unsigned int const)
    {
        // need enough space to store meter representations and part meter name strings.

        // size of just the Meter representation, without part names
        const auto meter_text_size = meters.size() * (single_meter_text_size + 2);

        // size of part names. sum of lengths of all part names to store will be an overestimate since the
        // part name doesn't need to be stored once per meter, but just once per part type that has meters
        const auto part_names_size = std::transform_reduce(meters.begin(), meters.end(), 0u, std::plus{},
                                                           [](const auto& mt_name) { return mt_name.first.first.size(); });

        std::vector<std::string::value_type> buffer(meter_text_size + part_names_size + 4, // 4 extra for safety padding
                                                    std::string::value_type{0});

        auto* buffer_next = buffer.data();
        auto* buffer_end = buffer.data() + buffer.size();

        // store number of meters
        buffer_next += ToChars(meters.size(), buffer_next, buffer_end);


        // store part name, count of meters, then each meter as triple of metertype, current, initial
        const auto end_it = meters.end();
        auto first_of_part_it = meters.begin();

        while (first_of_part_it != end_it) {
            *buffer_next++ = ' ';

            // store part name
            const auto& part_name = first_of_part_it->first.first;
            const auto name_sz = part_name.size();
            std::copy_n(part_name.c_str(), name_sz, buffer_next);
            buffer_next += name_sz;
            *buffer_next++ = ' ';

            // find range of meters for current part
            const auto first_of_next_part_it = std::find_if_not(first_of_part_it, end_it,
                                                                [&part_name](const auto& pn_mt_m) noexcept -> bool
                                                                { return pn_mt_m.first.first == part_name; });
            const auto part_meter_count = std::distance(first_of_part_it, first_of_next_part_it);

            // store number of meters for part
            buffer_next += ToChars(part_meter_count, buffer_next, buffer_end);

            // store each meter type, current, and initial values for part meters
            for (auto m_it{first_of_part_it}; m_it != first_of_next_part_it; ++m_it) {
                *buffer_next++ = ' ';
                buffer_next += ToChars(m_it->first.second, m_it->second, buffer_next, buffer_end);
            }

            first_of_part_it = first_of_next_part_it;
        }


        std::string s{buffer.data()};
        ar << boost::serialization::make_nvp("part_meters", s);
    }

    template <>
    void Serialize(boost::archive::xml_iarchive& ar, Ship::PartMeterMap& meters,
                   unsigned int const version)
    {
        static constexpr std::size_t buffer_capacity = num_meters_possible * (single_meter_text_size + 50); // guesstimate. number of of part meters and sizes of part type names is scriptable

        if (version < 3) {
            std::vector<std::pair<std::pair<MeterType, std::string>, Meter>> scratch;
            scratch.reserve(20); // guesstimate
            ar >> boost::serialization::make_nvp("m_part_meters", scratch);
            // reorder to match PartMeterMap sorting (first by name, then by MeterType)
            std::sort(scratch.begin(), scratch.end(),
                      [](const auto& lhs, const auto& rhs) {
                          return (lhs.first.second < rhs.first.second) || (
                              (lhs.first.second == rhs.first.second) && rhs.first.first < rhs.first.first);
                      });
            for (auto& [mt_pn, meter] : scratch) {
                meters.emplace(std::piecewise_construct,
                               std::forward_as_tuple(std::move(mt_pn.second), std::move(mt_pn.first)),
                               std::forward_as_tuple(std::move(meter)));
            }
            return;
        }

        // interpret custom string representation of part meters

        std::string buffer;
        buffer.reserve(buffer_capacity);
        ar >> boost::serialization::make_nvp("part_meters", buffer);

        // buffer should contain a text representation of a series of part meters, formatted like
        // 10 FT_BAY_KRILL 2 CAP 4000 4000 cap 4000 4000 FT_HANGAR_KRILL 4 CAP 0 0 SEC 6000 6000 cap 0 0 sec 6000 6000 SR_WEAPON_1_1 4 CAP 3000 3000 SEC 1000 1000 cap 3000 3000 sec 1000 1000
        // where
        // 10 = total number of part meters
        // FT_BAY_KRILL = name of first part with meters
        // 2 = number of meters for first part
        // CAP = type of meter (Max Capacity)
        // 4000 4000 = current and initial values of meter

        // get total number of meters
        unsigned int total_meter_count = 0U;
        const auto* const buffer_end = buffer.c_str() + buffer.size();

        auto [next, success] = FromChars(buffer.c_str(), buffer_end, total_meter_count);
        if (!success)
            return;
        meters.reserve(meters.size() + static_cast<std::size_t>(total_meter_count));

        // loop over meters
        unsigned int extracted_meters = 0U;
        while (next != buffer_end && extracted_meters < total_meter_count) {
            // skip whitespace
            while (std::distance(next, buffer_end) > 0 && *next == ' ')
                ++next;

            // get part name
            auto part_name_end_it = std::find(next, buffer_end, ' ');
            std::string part_name{next, part_name_end_it};
            if (part_name.empty())
                return;
            next = part_name_end_it;

            // skip whitespace
            while (std::distance(next, buffer_end) > 0 && *next == ' ')
                ++next;

            // get part meter count
            unsigned int part_meter_count = 0;
            std::tie(next, success) = FromChars(next, buffer_end, part_meter_count);
            if (!success || next == buffer_end)
                return;

            for (decltype(part_meter_count) part_idx = 0U; part_idx < part_meter_count; ++part_idx) {
                // skip whitespace
                while (std::distance(next, buffer_end) > 0 && *next == ' ')
                    ++next;

                // get part meter
                if (std::distance(next, buffer_end) < 7) // 7 is enough for "POP 0 0" or similar
                    return;
                const auto mt = MeterTypeFromTag(std::string_view(next, 3));
                next += 3;

                // skip whitespace
                while (std::distance(next, buffer_end) > 0 && *next == ' ')
                    ++next;

                // get meter values
                Meter meter;
                const std::string_view meter_view(next, std::distance(next, buffer_end));
                const auto consumed = meter.SetFromChars(meter_view);
                if (consumed < 1)
                    return;
                next += consumed;

                // store meter
                static_assert(std::is_same_v<Ship::PartMeterMap::key_type, std::pair<std::string, MeterType>>);
                meters.emplace(std::piecewise_construct,
                               std::forward_as_tuple(part_name, mt),
                               std::forward_as_tuple(std::move(meter)));
            }
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
    Serialize(ar, o.m_meters, version);
    ar  & make_nvp("m_created_on_turn", o.m_created_on_turn);
}

namespace {
    template <integral T>
    consteval std::size_t Digits(T t) {
        if constexpr (std::is_same_v<T, bool>) {
            return 5u; // for "false"

        } else {
            std::size_t retval = 1u;
            if constexpr (std::is_signed_v<T>)
                retval += (t < 0); // for '-' character

            while (t/10 != 0) {
                retval += 1;
                t /= 10;
            }
            return retval;
        }
    }
    static_assert(Digits(0) == 1);
    static_assert(Digits(1) == 1);
    static_assert(Digits(-1) == 2);
    static_assert(Digits(-12089) == 6);
    static_assert(Digits(12089) == 5);
    constexpr auto digits_id_max = Digits(std::numeric_limits<UniverseObject::IDSet::value_type>::max());
    constexpr auto digits_id_min = Digits(std::numeric_limits<UniverseObject::IDSet::value_type>::min());
    constexpr auto digits_id = std::max(digits_id_max, digits_id_min);
    using flat_set_size_t = boost::container::flat_set<UniverseObject::IDSet::value_type>::size_type;
    constexpr auto digits_size_t = Digits(std::numeric_limits<flat_set_size_t>::max());


    template <typename Archive>
    void Serialize(Archive& ar, const char* name, boost::container::flat_set<int32_t>& fs)
    { ar & boost::serialization::make_nvp(name, fs); }

    template <>
    void Serialize(boost::archive::xml_oarchive& ar, const char* name, boost::container::flat_set<int32_t>& fs)
    {
        std::string buffer;
        // space for a size of container value, for each ID number, a space pads between each, and a bit extra
        const auto space_needed = (digits_id + 1)*fs.size() + digits_size_t + 1 + 3;
        buffer.reserve(space_needed);

        // small buffer for each number to be written as text before appending to main buffer
        static constexpr auto num_buf_sz = std::max(digits_id, digits_size_t) + 2;
        std::array<std::string::value_type, num_buf_sz> sz_buf{};
        auto written_chars = ToChars(fs.size(), sz_buf.data(), sz_buf.data() + sz_buf.size());
        buffer.append(sz_buf.data(), written_chars);

        for (auto i : fs) {
            // small buffer for each number to be written as text before appending to main buffer
            std::array<std::string::value_type, num_buf_sz> number_buf{" "};
            written_chars = ToChars(i, number_buf.data() + 1, number_buf.data() + number_buf.size());
            buffer.append(number_buf.data(), written_chars + 1);
        }

        ar << boost::serialization::make_nvp(name, buffer);
    }

    template <>
    void Serialize(boost::archive::xml_iarchive& ar, const char* name, boost::container::flat_set<int32_t>& fs)
    {
        std::string buffer;
        ar >> boost::serialization::make_nvp(name, buffer);

        unsigned int count = 0U;
        const auto* const buffer_end = buffer.c_str() + buffer.size();

        auto [next, success] = FromChars(buffer.c_str(), buffer_end, count);
        if (!success)
            return;

        fs.reserve(fs.size() + static_cast<std::size_t>(count));
        using ID_t = std::decay_t<decltype(fs)>::value_type;
        std::vector<ID_t> maybe_unsorted_buffer;
        maybe_unsorted_buffer.reserve(count);

        // parse count ID_t values
        for (decltype(count) idx = 0u; idx < count; ++idx) {
            // advance to next bit of non-whitespace
            while (std::distance(next, buffer_end) > 0 && *next == ' ')
                ++next;

            ID_t id = INVALID_OBJECT_ID;
            std::tie(next, success) = FromChars(next, buffer_end, id);
            if (!success)
                return;

            if (id != INVALID_OBJECT_ID)
                maybe_unsorted_buffer.push_back(id);
        }
        std::sort(maybe_unsorted_buffer.begin(), maybe_unsorted_buffer.end());
        auto unique_it = std::unique(maybe_unsorted_buffer.begin(), maybe_unsorted_buffer.end());
        fs.insert(boost::container::ordered_unique_range, maybe_unsorted_buffer.begin(), unique_it);
    }

    template <typename Archive>
    void DeserializeSetIntoFlatSet(Archive& ar, const char* name, boost::container::flat_set<int>& c)
    {
        static_assert(Archive::is_loading::value);
        std::set<int> temp;
        ar >> boost::serialization::make_nvp(name, temp);
        c.clear();
        c.insert(boost::container::ordered_unique_range, temp.begin(), temp.end());
    }
}

template <typename Archive>
void load_construct_data(Archive& ar, System* obj, unsigned int const version)
{ ::new(obj)System(); }

template <typename Archive>
void serialize(Archive& ar, System& obj, unsigned int const version)
{
    using namespace boost::serialization;
    using namespace boost::container;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj))
        & make_nvp("m_star", obj.m_star)
        & make_nvp("m_orbits", obj.m_orbits);

    using SV_IDSet_pair = std::pair<std::string_view, UniverseObject::IDSet&>;
    std::array<SV_IDSet_pair, 6> id_sets{{
        {"m_objects", obj.m_objects}, {"m_planets", obj.m_planets}, {"m_buildings", obj.m_buildings},
        {"m_fleets", obj.m_fleets}, {"m_ships", obj.m_ships}, {"m_fields", obj.m_fields}}};
    auto serialize_flat_set = [&ar, version](SV_IDSet_pair& name_ids) {
        if constexpr (Archive::is_loading::value) {
            if (version < 1)
                DeserializeSetIntoFlatSet(ar, name_ids.first.data(), name_ids.second);
            else
                Serialize(ar, name_ids.first.data(), name_ids.second);
        } else {
            (void)version;
            Serialize(ar, name_ids.first.data(), name_ids.second);
        }
    };
    std::for_each(id_sets.begin(), id_sets.end(), serialize_flat_set);

    if (Archive::is_loading::value && version < 2) {
        obj.m_starlanes.clear();
        std::map<int, bool> lanes_wormholes;
        ar  & make_nvp("m_starlanes_wormholes", lanes_wormholes);
        std::transform(lanes_wormholes.begin(), lanes_wormholes.end(),
                       std::inserter(obj.m_starlanes, obj.m_starlanes.end()),
                       [](const auto& id_w) { return id_w.first; });
    } else {
        Serialize(ar, "m_starlanes", obj.m_starlanes);
    }

    ar  & make_nvp("m_last_turn_battle_here", obj.m_last_turn_battle_here);
    if constexpr (Archive::is_loading::value)
        obj.m_system_id = obj.ID(); // override old value that was stored differently previously...
}

BOOST_CLASS_EXPORT(System)
BOOST_CLASS_VERSION(System, 2)

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

namespace {
    // backwards compatability
    struct PopCenter { std::string m_species_name; };

    template <typename Archive>
    void serialize(Archive& ar, PopCenter& pop, unsigned int const version)
    { ar & boost::serialization::make_nvp("m_species_name", pop.m_species_name); }

    struct ResourceCenter {
        std::string m_focus;
        int         m_last_turn_focus_changed = INVALID_GAME_TURN;
        std::string m_focus_turn_initial;
        int         m_last_turn_focus_changed_turn_initial = INVALID_GAME_TURN;
    };

    template <typename Archive>
    void serialize(Archive& ar, ResourceCenter& rs, unsigned int const version)
    {
        using namespace boost::serialization;

        ar  & make_nvp("m_focus", rs.m_focus)
            & make_nvp("m_last_turn_focus_changed", rs.m_last_turn_focus_changed)
            & make_nvp("m_focus_turn_initial", rs.m_focus_turn_initial)
            & make_nvp("m_last_turn_focus_changed_turn_initial", rs.m_last_turn_focus_changed_turn_initial);
    }
}

template <typename Archive>
void serialize(Archive& ar, Planet& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj));
    if constexpr (Archive::is_loading::value) {
        if (version < 3) {
            PopCenter pop;
            ar  & make_nvp("PopCenter", pop);
            obj.m_species_name = std::move(pop.m_species_name);
        } else {
            ar  & make_nvp("m_species_name", obj.m_species_name);
        }

        if (version < 4) {
            ResourceCenter res;
            ar  & make_nvp("ResourceCenter", res);
            obj.m_focus = std::move(res.m_focus);
            obj.m_last_turn_focus_changed = res.m_last_turn_focus_changed;
            obj.m_focus_turn_initial = std::move(res.m_focus_turn_initial);
            obj.m_last_turn_focus_changed_turn_initial = res.m_last_turn_focus_changed_turn_initial;
        } else {
            ar  & make_nvp("m_focus", obj.m_focus)
                & make_nvp("m_last_turn_focus_changed", obj.m_last_turn_focus_changed)
                & make_nvp("m_focus_turn_initial", obj.m_focus_turn_initial)
                & make_nvp("m_last_turn_focus_changed_turn_initial", obj.m_last_turn_focus_changed_turn_initial);
        }

    } else {
        ar  & make_nvp("m_species_name", obj.m_species_name);

        ar  & make_nvp("m_focus", obj.m_focus)
            & make_nvp("m_last_turn_focus_changed", obj.m_last_turn_focus_changed)
            & make_nvp("m_focus_turn_initial", obj.m_focus_turn_initial)
            & make_nvp("m_last_turn_focus_changed_turn_initial", obj.m_last_turn_focus_changed_turn_initial);
    }

    ar  & make_nvp("m_type", obj.m_type)
        & make_nvp("m_original_type", obj.m_original_type)
        & make_nvp("m_size", obj.m_size)
        & make_nvp("m_orbital_period", obj.m_orbital_period)
        & make_nvp("m_initial_orbital_position", obj.m_initial_orbital_position)
        & make_nvp("m_rotational_period", obj.m_rotational_period)
        & make_nvp("m_axial_tilt", obj.m_axial_tilt);
    if constexpr (Archive::is_loading::value) {
        if (version < 5)
            DeserializeSetIntoFlatSet(ar, "m_buildings", obj.m_buildings);
        else
            Serialize(ar, "m_buildings", obj.m_buildings);
    } else {
        Serialize(ar, "m_buildings", obj.m_buildings);
    }
    if (Archive::is_loading::value && version < 6) {
        obj.m_turn_last_annexed = INVALID_GAME_TURN;
        obj.m_ordered_annexed_by_empire_id = ALL_EMPIRES;
    } else {
        ar  & make_nvp("m_turn_last_annexed", obj.m_turn_last_annexed)
            & make_nvp("m_ordered_annexed_by_empire_id", obj.m_ordered_annexed_by_empire_id);
    }
    ar  & make_nvp("m_turn_last_colonized", obj.m_turn_last_colonized);
    ar  & make_nvp("m_turn_last_conquered", obj.m_turn_last_conquered);
    ar  & make_nvp("m_is_about_to_be_colonized", obj.m_is_about_to_be_colonized)
        & make_nvp("m_is_about_to_be_invaded", obj.m_is_about_to_be_invaded)
        & make_nvp("m_is_about_to_be_bombarded", obj.m_is_about_to_be_bombarded)
        & make_nvp("m_ordered_given_to_empire_id", obj.m_ordered_given_to_empire_id)
        & make_nvp("m_last_turn_attacked_by_ship", obj.m_last_turn_attacked_by_ship);
    if (Archive::is_loading::value && version < 7) {
        obj.m_owner_before_last_conquered = obj.Owner();
    } else {
        ar  & make_nvp("m_owner_before_last_conquered", obj.m_owner_before_last_conquered);
    }
    if (Archive::is_loading::value && version < 8) {
        obj.m_last_invaded_by_empire_id = ALL_EMPIRES;
    } else {
        ar  & make_nvp("m_last_invaded_by_empire_id", obj.m_last_invaded_by_empire_id);
    }
    if (Archive::is_loading::value && version < 9) {
        obj.m_last_colonized_by_empire_id = ALL_EMPIRES;
    } else {
        ar  & make_nvp("m_last_colonized_by_empire_id", obj.m_last_colonized_by_empire_id);
    }
    if (Archive::is_loading::value && version < 10) {
        obj.m_last_annexed_by_empire_id = ALL_EMPIRES;
    } else {
        ar  & make_nvp("m_last_annexed_by_empire_id", obj.m_last_annexed_by_empire_id);
    }
}

BOOST_CLASS_EXPORT(Planet)
BOOST_CLASS_VERSION(Planet, 10)


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

    ar  & make_nvp("UniverseObject", base_object<UniverseObject>(obj));

    if constexpr (Archive::is_loading::value) {
        if (version < 7)
            DeserializeSetIntoFlatSet(ar, "m_ships", obj.m_ships);
        else
            Serialize(ar, "m_ships", obj.m_ships);
    } else {
        Serialize(ar, "m_ships", obj.m_ships);
    }

    ar  & make_nvp("m_prev_system", obj.m_prev_system)
        & make_nvp("m_next_system", obj.m_next_system);

    ar  & make_nvp("m_aggression", obj.m_aggression);

    ar  & make_nvp("m_ordered_given_to_empire_id", obj.m_ordered_given_to_empire_id);
    if (version < 6) {
        std::list<int> travel_route;
        ar & make_nvp("m_travel_route", travel_route);
        obj.m_travel_route = std::vector(travel_route.begin(), travel_route.end());
    } else {
        ar & make_nvp("m_travel_route", obj.m_travel_route);
    }
    ar & boost::serialization::make_nvp("m_last_turn_move_ordered", obj.m_last_turn_move_ordered);
    ar  & make_nvp("m_arrived_this_turn", obj.m_arrived_this_turn)
        & make_nvp("m_arrival_starlane", obj.m_arrival_starlane);
}

BOOST_CLASS_EXPORT(Fleet)
BOOST_CLASS_VERSION(Fleet, 7)


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
        & make_nvp("m_ordered_bombard_planet_id", obj.m_ordered_bombard_planet_id);
    Serialize(ar, obj.m_part_meters, version);
    ar  & make_nvp("m_species_name", obj.m_species_name)
        & make_nvp("m_produced_by_empire_id", obj.m_produced_by_empire_id)
        & make_nvp("m_arrived_on_turn", obj.m_arrived_on_turn);
    ar  & make_nvp("m_last_turn_active_in_combat", obj.m_last_turn_active_in_combat);
    ar  & make_nvp("m_last_resupplied_on_turn", obj.m_last_resupplied_on_turn);
}

BOOST_CLASS_EXPORT(Ship)
BOOST_CLASS_VERSION(Ship, 3)


template <typename Archive>
void serialize(Archive& ar, ShipDesign& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("m_id", obj.m_id)
        & make_nvp("m_name", obj.m_name);

    TraceLogger() << "ship design serialize version: " << version << " : " << (Archive::is_saving::value ? "saving" : "loading");

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

    ar  & make_nvp("m_description", obj.m_description)
        & make_nvp("m_designed_on_turn", obj.m_designed_on_turn);
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

