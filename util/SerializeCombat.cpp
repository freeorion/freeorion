#include "Serialize.h"

#include "Serialize.ipp"
#include "SerializeUtil.h"

#include "../combat/CombatEvents.h"
#include "../combat/CombatLogManager.h"

#include <numeric>

namespace {
    DeclareThreadSafeLogger(combat_log);
}

template<typename Archive>
void serialize(Archive&, CombatEvent&, unsigned int const)
{}

BOOST_CLASS_EXPORT(CombatEvent)

template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatEvent&, const unsigned int);
template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatEvent&, const unsigned int);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatEvent&, const unsigned int);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatEvent&, const unsigned int);


template <typename Archive>
void serialize(Archive& ar, BoutBeginEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("bout", obj.bout);
}

BOOST_CLASS_EXPORT(BoutBeginEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, BoutBeginEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, BoutBeginEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, BoutBeginEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, BoutBeginEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, BoutEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("bout", obj.bout)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(BoutEvent, 4)
BOOST_CLASS_EXPORT(BoutEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, BoutEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, BoutEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, BoutEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, BoutEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, SimultaneousEvents& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(SimultaneousEvents, 4)
BOOST_CLASS_EXPORT(SimultaneousEvents)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SimultaneousEvents&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SimultaneousEvents&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SimultaneousEvents&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SimultaneousEvents&, unsigned int const);



namespace {
    template <typename Archive>
    void Serialize(Archive& ar, auto& container, const char* tag, bool old_non_string_format)
        requires std::is_same_v<int, std::decay_t<decltype(*container.begin())>>
    {
        if constexpr (Archive::is_loading::value) {
            if (old_non_string_format) {
                ar >> boost::serialization::make_nvp(tag, container);
            } else if constexpr (std::is_same_v<Archive, boost::archive::xml_iarchive>) {
                std::string str;
                ar >> boost::serialization::make_nvp(tag, str);
                FillIntContainer(container, str);
            } else {
                ar >> boost::serialization::make_nvp(tag, container);
            }
        } else {
            if constexpr (std::is_same_v<Archive, boost::archive::xml_oarchive>) {
                std::string str = ToString(container);
                ar << boost::serialization::make_nvp(tag, str);
            } else {
                ar << boost::serialization::make_nvp(tag, container);
            }
        }
    }


    std::string ToString(const auto& data)
        requires requires {
            data.size();
            data.begin();
            data.end();
            std::to_string(data.begin()->first);
            Meter::FromFloat(data.begin()->second.current_health);
            Meter::FromFloat(data.begin()->second.max_health);
        }
    {
        std::string retval;

        try {
            retval.reserve(data.size() * 3 * (int_digits + 1) + int_digits + 2); // space for count and all int value triples and gaps
        } catch(...) {}

        retval.append(std::to_string(data.size()));

        for (const auto& [id, state] : data) {
            retval.append(" ").append(std::to_string(id))
                  .append(" ").append(std::to_string(Meter::FromFloat(state.current_health)))
                  .append(" ").append(std::to_string(Meter::FromFloat(state.max_health)));
        }

        return retval;
    }

    // interprets text at \a next as int or unsigned int. skips initial whitespace.
    // fails if \a next or \a buffer_end are nullptr, if incrementing \a next reaches
    // \a buffer end before parsing succeeds, or if text at \a next (after whitespace)
    // is not interpretable as int or unsigned int as appropriate.
    // returns tuple of result value, bool indicating success/fail, and new next address
    template <typename IntOrUInt> requires std::is_same_v<int, IntOrUInt> || std::is_same_v<unsigned int, IntOrUInt>
    CONSTEXPR_FROM_CHARS std::tuple<IntOrUInt, bool, const char*>
        GetIntFromChars(const char* const buffer_end, const char* next, const IntOrUInt default_val)
    {
        // safety checks
        if (!next || !buffer_end)
            return {default_val, false, next};

        // skip whitespace
        while (next != buffer_end && *next == ' ')
            ++next;

        // safety check for end of buffer
        if (next == buffer_end)
            return {default_val, false, next};

        // parse string to int
        IntOrUInt result = default_val;
        auto [next_out, success] = FromChars(next, buffer_end, result);
        return {result, success, next_out};
    };


    CONSTEXPR_FROM_CHARS void FillCombatStates(auto& container, std::string_view buffer)
        requires requires { container.emplace(1, CombatParticipantState{}); }
    {
        if (buffer.empty() || !buffer.data())
            return;

        auto* next = buffer.data();
        const auto* const buffer_end = buffer.data() + buffer.size();
        unsigned int count = 0;
        bool success = false;

        std::tie(count, success, next) = GetIntFromChars<unsigned int>(buffer_end, next, 0u);
        if (!success)
            return;

        if constexpr (requires { container.reserve(count); }) {
            try {
                container.reserve(count);
            } catch (...) {}
        }

        const auto get_int_from_chars = [buffer_end](const char* next, const int default_val)
        { return GetIntFromChars<int>(buffer_end, next, default_val); };

        for (std::size_t idx = 0; idx < static_cast<std::size_t>(count) && next != buffer_end; ++idx) {
            int obj_id = INVALID_OBJECT_ID;
            int cur_int = Meter::DEFAULT_INT;
            int max_int = Meter::DEFAULT_INT;

            std::tie(obj_id, success, next) = get_int_from_chars(next, INVALID_OBJECT_ID);
            if (!success)
                break;
            std::tie(cur_int, success, next) = get_int_from_chars(next, Meter::DEFAULT_INT);
            if (!success)
                break;
            std::tie(max_int, success, next) = get_int_from_chars(next, Meter::DEFAULT_INT);
            if (!success)
                break;

            container.emplace(std::piecewise_construct,
                              std::forward_as_tuple(obj_id),
                              std::forward_as_tuple(Meter::FromInt(cur_int), Meter::FromInt(max_int)));
        }
    }

    template <typename Archive>
    void Serialize(Archive& ar, auto& states, const char* tag, bool old_non_string_format)
        requires std::is_same_v<int, std::decay_t<decltype(states.begin()->first)>> &&
                 std::is_same_v<CombatParticipantState, std::decay_t<decltype(states.begin()->second)>>
    {
        if constexpr (Archive::is_loading::value) {
            if (old_non_string_format) {
                std::map<int, CombatParticipantState> data;
                ar >> boost::serialization::make_nvp(tag, data);
                states.clear();
                states.insert(boost::container::ordered_unique_range, data.begin(), data.end());
            } else if constexpr (std::is_same_v<Archive, boost::archive::xml_iarchive>) {
                std::string str;
                ar >> boost::serialization::make_nvp(tag, str);
                FillCombatStates(states, str);
            } else {
                ar >> boost::serialization::make_nvp(tag, states);
            }
        } else {
            if constexpr (std::is_same_v<Archive, boost::archive::xml_oarchive>) {
                std::string str = ToString(states);
                ar << boost::serialization::make_nvp(tag, str);
            } else {
                ar << boost::serialization::make_nvp(tag, states);
            }
        }
    }
}


template <typename Archive>
void serialize(Archive& ar, InitialStealthEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    if constexpr (Archive::is_loading::value) {
        obj.empire_object_visibility.clear();
        bool old_format = version < 5;
        const char* xml_tag = old_format ? "empire_to_object_visibility" : nullptr;
        if (old_format)
            DebugLogger() << "Deserializing InitialStealthEvent old format version: " << version;
        Deserialize(ar, obj.empire_object_visibility, old_format, xml_tag);
    } else {
        Serialize(ar, obj.empire_object_visibility);
    }
}

BOOST_CLASS_VERSION(InitialStealthEvent, 5)
BOOST_CLASS_EXPORT(InitialStealthEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, InitialStealthEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, InitialStealthEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, InitialStealthEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, InitialStealthEvent&, unsigned int const);


namespace {
    constexpr auto not_null = [](const auto& p) noexcept -> bool { return p.get(); };
    constexpr auto to_size = [](const auto& c) noexcept { return c.size(); };
    constexpr auto extract_from_shptr = [](auto& sptr) -> auto& { return *sptr; };
}


namespace {
    [[nodiscard]] constexpr std::string_view VisToChar(Visibility vis) noexcept {
        switch (vis) {
        case Visibility::VIS_FULL_VISIBILITY:    return "f"; break;
        case Visibility::VIS_PARTIAL_VISIBILITY: return "p"; break;
        case Visibility::VIS_BASIC_VISIBILITY:   return "b"; break;
        default:                                 return "x";
        }
    }

    [[nodiscard]] constexpr Visibility CharToVis(char c) noexcept {
        switch (c) {
        case 'f': return Visibility::VIS_FULL_VISIBILITY;    break;
        case 'p': return Visibility::VIS_PARTIAL_VISIBILITY; break;
        case 'b': return Visibility::VIS_BASIC_VISIBILITY;   break;
        default:  return Visibility::VIS_NO_VISIBILITY;
        }
    }

    std::string ToString(const std::vector<StealthChangeEvent::StealthChangeEventDetail>& events) {
        std::string retval;
        try {
            static constexpr std::size_t one_event_buffer_sz = 4 * (int_digits + 1) + 4; // 4 ints + spaces, fighter flag, visibility flag, separator spaces
            retval.reserve(int_digits + 1 + events.size()*one_event_buffer_sz);          // count, spaces, events
        } catch (...) {}

        retval.append(std::to_string(events.size()));
        for (const auto& event : events) {
            retval.append("  ")
                  .append(std::to_string(event.attacker_id)).append(" ")
                  .append(std::to_string(event.target_id)).append(" ")
                  .append(std::to_string(event.attacker_empire_id)).append(" ")
                  .append(std::to_string(event.target_observer_empire_id)).append(" ")
                  .append(VisToChar(event.visibility))
                  .append(event.is_fighter_launch ? "l" : "n");
        }
        return retval;
    }

    template <typename Archive>
    void FillStealthChangeEventViaSharedPtrs(Archive& ar, std::vector<StealthChangeEvent::StealthChangeEventDetail>& events)
    {
        using boost::serialization::make_nvp;

        using StealthChangeEventDetailPtr = std::shared_ptr<StealthChangeEvent::StealthChangeEventDetail>;
        std::map<int, std::vector<StealthChangeEventDetailPtr>> sced_sptr_map;
        ar >> make_nvp("events", sced_sptr_map);

        auto sc_event_counts_rng = sced_sptr_map | range_values | range_transform(to_size);
        const std::size_t sc_event_count =
            std::accumulate(sc_event_counts_rng.begin(), sc_event_counts_rng.end(), std::size_t{0});

        events.clear();
        events.reserve(sc_event_count);

        for (auto& sced_sptr_vec : sced_sptr_map | range_values) {
            auto sptr_rng = sced_sptr_vec | range_filter(not_null) | range_transform(extract_from_shptr);
            events.insert(events.end(), sptr_rng.begin(), sptr_rng.end());
        }
    }

    std::tuple<StealthChangeEvent::StealthChangeEventDetail, const char*, bool>
    GetStealthChangeEventDetailFromChars(const char* next, const char* const buffer_end)
    {
        StealthChangeEvent::StealthChangeEventDetail retval;
        if (!next || !buffer_end)
            return {retval, next, false};

        const auto get_next_int = [buffer_end](const char* next, const int default_val) -> std::tuple<int, bool, const char*>
        { return GetIntFromChars<int>(buffer_end, next, default_val); };

        bool success = false;

        std::tie(retval.attacker_id, success, next) = get_next_int(next, INVALID_OBJECT_ID);
        if (!success)
            return {retval, next, false};
        std::tie(retval.target_id, success, next) = get_next_int(next, INVALID_OBJECT_ID);
        if (!success)
            return {retval, next, false};
        std::tie(retval.attacker_empire_id, success, next) = get_next_int(next, ALL_EMPIRES);
        if (!success)
            return {retval, next, false};
        std::tie(retval.target_observer_empire_id, success, next) = get_next_int(next, ALL_EMPIRES);
        if (!success)
            return {retval, next, false};

        // skip whitespace
        while (next != buffer_end && *next == ' ')
            ++next;
        // safety check for end of buffer
        if (next == buffer_end)
            return {retval, next, false};

        retval.visibility = CharToVis(*next);
        ++next;
        if (next == buffer_end)
            return {retval, next, false};

        retval.is_fighter_launch = (*next == 'l'); // anything besides 'l' treated as not a fighter launch
        if (*next != ' ')
            ++next;

        return {retval, next, true};
    }

    void FillStealthChangeEvent(std::vector<StealthChangeEvent::StealthChangeEventDetail>& events, std::string_view buffer) {
        using Detail = StealthChangeEvent::StealthChangeEventDetail;
        events.clear();

        const auto* const buffer_end = buffer.data() + buffer.size();
        const auto* next = buffer.data();

        unsigned int count_ui = 0;
        bool success = false;
        std::tie(next, success) = FromChars(next, buffer_end, count_ui);
        if (!success)
            return;
        std::size_t count = static_cast<std::size_t>(count_ui);

        try {
            events.reserve(count);
        } catch (...) {}

        for (std::size_t idx = 0; idx < count && next != buffer_end; ++idx) {
            StealthChangeEvent::StealthChangeEventDetail detail;
            std::tie(detail, next, success) = GetStealthChangeEventDetailFromChars(next, buffer_end);
            if (!success)
                return;
            events.push_back(std::move(detail));
        }

        static constexpr auto target_observer_id_less = [](const Detail& lhs, const Detail& rhs) noexcept
        { return lhs.target_observer_empire_id < rhs.target_observer_empire_id; };

        // order by observer empire id
        std::stable_sort(events.begin(), events.end(), target_observer_id_less);
    }
}

template <typename Archive>
void serialize(Archive& ar, StealthChangeEvent& obj, unsigned int const version)
{
    using boost::serialization::make_nvp;
    using boost::serialization::base_object;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("bout", obj.bout);

    if constexpr (Archive::is_loading::value) {
        if (version < 5) {
            FillStealthChangeEventViaSharedPtrs(ar, obj.events);
        } else if constexpr (std::is_same_v<Archive, boost::archive::xml_iarchive>) {
            std::string str;
            ar >> make_nvp("events", str);
            FillStealthChangeEvent(obj.events, str);
        } else {
            ar >> make_nvp("events", obj.events);
        }
    } else {
        if constexpr (std::is_same_v<Archive, boost::archive::xml_oarchive>) {
            std::string str = ToString(obj.events);
            ar << make_nvp("events", str);
        } else {
            ar << make_nvp("events", obj.events);
        }
    }
}

BOOST_CLASS_VERSION(StealthChangeEvent, 5)
BOOST_CLASS_EXPORT(StealthChangeEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, StealthChangeEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, StealthChangeEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, StealthChangeEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, StealthChangeEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, StealthChangeEvent::StealthChangeEventDetail& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar  & make_nvp("attacker_id", obj.attacker_id)
        & make_nvp("target_id", obj.target_id)
        & make_nvp("attacker_empire_id", obj.attacker_empire_id)
        & make_nvp("target_empire_id", obj.target_observer_empire_id)
        & make_nvp("visibility", obj.visibility);
    if (version >= 5)
        ar  & make_nvp("is_fighter_launch", obj.is_fighter_launch);
}

BOOST_CLASS_VERSION(StealthChangeEvent::StealthChangeEventDetail, 5)
BOOST_CLASS_EXPORT(StealthChangeEvent::StealthChangeEventDetail)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, StealthChangeEvent::StealthChangeEventDetail&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, StealthChangeEvent::StealthChangeEventDetail&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, StealthChangeEvent::StealthChangeEventDetail&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, StealthChangeEvent::StealthChangeEventDetail&, unsigned int const);


namespace {
    std::string ToString(const WeaponFireEvent& obj) {
        std::string retval;
        try {
            retval.reserve(7 * (int_digits + 1) + obj.weapon_name.size());
        } catch (...) {}
        retval.append(std::to_string(obj.attacker_id)).append(" ")
              .append(std::to_string(obj.target_id)).append(" ")
              .append(std::to_string(obj.attacker_owner_id)).append(" ")
              .append(std::to_string(obj.target_owner_id)).append(" ")
              .append(std::to_string(Meter::FromFloat(obj.power))).append(" ")
              .append(std::to_string(Meter::FromFloat(obj.shield))).append(" ")
              .append(std::to_string(Meter::FromFloat(obj.damage))).append(" ")
              .append(obj.weapon_name);
        return retval;
    }
    constexpr auto wfe_to_string = [](const WeaponFireEvent& wfe) { return ToString(wfe); };

    // fills \a obj from \a buffer. returns true/false indicating success/failure. 
    bool FillWeaponFireEvent(WeaponFireEvent& obj, std::string_view buffer) {
        const auto* const buffer_end = buffer.data() + buffer.size();
        const auto* next = buffer.data();
        bool success = false;

        const auto get_next_int = [buffer_end](const char* next, const int default_val)
        { return GetIntFromChars<int>(buffer_end, next, default_val); };

        std::tie(obj.attacker_id, success, next) = get_next_int(next, INVALID_OBJECT_ID);
        if (!success)
            return false;
        std::tie(obj.target_id, success, next) = get_next_int(next, INVALID_OBJECT_ID);
        if (!success)
            return false;
        std::tie(obj.attacker_owner_id, success, next) = get_next_int(next, ALL_EMPIRES);
        if (!success)
            return false;
        std::tie(obj.target_owner_id, success, next) = get_next_int(next, ALL_EMPIRES);
        if (!success)
            return false;

        int power_as_int = 0;
        std::tie(power_as_int, success, next) = get_next_int(next, 0);
        if (!success)
            return false;
        obj.power = Meter::FromInt(power_as_int);

        int shield_as_int = 0;
        std::tie(shield_as_int, success, next) = get_next_int(next, 0);
        if (!success)
            return false;
        obj.shield = Meter::FromInt(shield_as_int);

        int damage_as_int = 0;
        std::tie(damage_as_int, success, next) = get_next_int(next, 0);
        if (!success)
            return false;
        obj.damage = Meter::FromInt(damage_as_int);


        // skip whitespace
        while (next != buffer_end && *next == ' ')
            ++next;

        const auto next_offset = static_cast<std::size_t>(std::distance(buffer.data(), next));
        if (next_offset > buffer.size())
            return false;

        // extract weapon name and remove trailing whitespace
        auto weapon_name = buffer.substr(next_offset);
        const auto trim_pos = weapon_name.find_last_not_of(' ');
        if (trim_pos != weapon_name.npos)
            weapon_name.remove_suffix(weapon_name.size() - (trim_pos + 1));

        obj.weapon_name = weapon_name;

        return true;
    }

    static_assert([]() {
        std::string_view ts = "trailing spaces    ";
        auto trim_pos0 = ts.find_last_not_of(' ');
        if (trim_pos0 != ts.npos)
            ts.remove_suffix(ts.size() - (trim_pos0 + 1));

        std::string_view empty;
        auto trim_pos1 = empty.find_last_not_of(' ');
        if (trim_pos1 != empty.npos)
            empty.remove_suffix(empty.size() - (trim_pos1 + 1));

        return ts == "trailing spaces" && empty.empty();
    }());

    template <typename Archive>
    void Serialize(Archive& ar, WeaponFireEvent& obj, bool short_tags, bool include_bout_round)
    {
        using boost::serialization::make_nvp;
        if (include_bout_round) {
            int ignored = -1;
            ar  & make_nvp(short_tags ? "b" : "bout", ignored)
                & make_nvp(short_tags ? "r" : "round", ignored);
        }
        ar  & make_nvp(short_tags ? "a" : "attacker_id", obj.attacker_id)
            & make_nvp(short_tags ? "t" : "target_id", obj.target_id)
            & make_nvp(short_tags ? "w" : "weapon_name", obj.weapon_name)
            & make_nvp(short_tags ? "p" : "power", obj.power)
            & make_nvp(short_tags ? "s" : "shield", obj.shield)
            & make_nvp(short_tags ? "d" : "damage", obj.damage)
            & make_nvp(short_tags ? "to" : "target_owner_id", obj.target_owner_id)
            & make_nvp(short_tags ? "ao" : "attacker_owner_id", obj.attacker_owner_id);
    }
}

template <typename Archive>
void serialize(Archive& ar, WeaponFireEvent& obj, unsigned int const version)
{
    using boost::serialization::make_nvp;
    using boost::serialization::base_object;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    if constexpr (Archive::is_loading::value) {
        if (version < 6) {
            // versions 5 and earlier had round and bout
            // version 5 started using shortenend tags
            Serialize(ar, obj, version >= 5, true); 
        } else if constexpr (std::is_same_v<Archive, boost::archive::xml_iarchive>) {
            std::string str;
            ar >> make_nvp("info", str);
            FillWeaponFireEvent(obj, str);
        } else {
            Serialize(ar, obj, true, false); // 6 and later use short tags and exclude round and bout
        }
    } else {
        if constexpr (std::is_same_v<Archive, boost::archive::xml_oarchive>) {
            std::string str = ToString(obj);
            ar << make_nvp("info", str);
        } else {
            Serialize(ar, obj, true, false); // use short tags, and exclude round and bout for new encodings
        }
    }
}

BOOST_CLASS_VERSION(WeaponFireEvent, 6)
BOOST_CLASS_EXPORT(WeaponFireEvent)
template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, WeaponFireEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, WeaponFireEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, WeaponFireEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, WeaponFireEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, IncapacitationEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    if (version < 2) {
        ar & make_nvp("bout", obj.bout)
           & make_nvp("object_id", obj.object_id)
           & make_nvp("object_owner_id", obj.object_owner_id);
    } else {
        ar & make_nvp("b", obj.bout)
           & make_nvp("i", obj.object_id)
           & make_nvp("o", obj.object_owner_id);
    }
}

BOOST_CLASS_VERSION(IncapacitationEvent, 2)
BOOST_CLASS_EXPORT(IncapacitationEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, IncapacitationEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, IncapacitationEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, IncapacitationEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, IncapacitationEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, FightersAttackFightersEvent& obj, unsigned int const)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    ar & make_nvp("bout", obj.bout)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(FightersAttackFightersEvent, 4)
BOOST_CLASS_EXPORT(FightersAttackFightersEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, FightersAttackFightersEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, FightersAttackFightersEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, FightersAttackFightersEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, FightersAttackFightersEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, FighterLaunchEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    ar & make_nvp("bout", obj.bout)
       & make_nvp("fighter_owner_empire_id", obj.fighter_owner_empire_id)
       & make_nvp("launched_from_id", obj.launched_from_id)
       & make_nvp("number_launched", obj.number_launched);
}

BOOST_CLASS_EXPORT(FighterLaunchEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, FighterLaunchEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, FighterLaunchEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, FighterLaunchEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, FighterLaunchEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, FightersDestroyedEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    ar & make_nvp("bout", obj.bout)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(FightersDestroyedEvent, 4)
BOOST_CLASS_EXPORT(FightersDestroyedEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, FightersDestroyedEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, FightersDestroyedEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, FightersDestroyedEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, FightersDestroyedEvent&, unsigned int const);

namespace {
    std::vector<std::string> ToStrings(const WeaponsPlatformEvent& obj) {
        // WeaponsPlatformEvent contains a map from target_id to vector of WeaponFireEvent
        // WeaponFireEvent also contains a target_id
        // this stringification flattens that to a series of strings for each WeaponFireEvent
        auto fire_event_counts_rng = obj.events | range_values | range_transform(to_size);
        const std::size_t fire_event_count =
            std::accumulate(fire_event_counts_rng.begin(), fire_event_counts_rng.end(), std::size_t{0});

        std::vector<std::string> retval;
        try {
            retval.reserve(fire_event_count + 1); // + 1 for common "own" state
        } catch (...) {}

        auto& own_state_str = retval.emplace_back();
        own_state_str.reserve(2*int_digits + 1);
        own_state_str.append(std::to_string(obj.attacker_id)).append(" ")
                     .append(std::to_string(obj.attacker_owner_id));

        for (const auto& [target_id, weapon_fire_events] : obj.events) {
            auto wfe_str_rng = weapon_fire_events | range_transform(wfe_to_string);
            retval.insert(retval.end(), wfe_str_rng.begin(), wfe_str_rng.end());
        }

        return retval;
    }

    bool FillWeaponsPlatformBaseState(WeaponsPlatformEvent& obj, std::string_view buffer) {
        const auto* buffer_end = buffer.data() + buffer.size();
        const auto* next = buffer.data();
        bool success = false;

        std::tie(obj.attacker_id, success, next) = GetIntFromChars<int>(buffer_end, next, INVALID_OBJECT_ID);
        if (!success)
            return false;
        std::tie(obj.attacker_owner_id, success, next) = GetIntFromChars<int>(buffer_end, next, ALL_EMPIRES);
        return success;
    }

    void FillWeaponsPlatformEvent(WeaponsPlatformEvent& obj, const std::vector<std::string>& buffers) {
        if (buffers.empty())
            return;

        {
            // extract base state for platform event
            bool success = FillWeaponsPlatformBaseState(obj, buffers.front());
            if (!success)
                return;
        }

        for (const auto& wfe_buffer : buffers | range_drop(1)) {
            WeaponFireEvent wfe;
            bool success = FillWeaponFireEvent(wfe, wfe_buffer);
            if (!success) continue;
            const int target_id = wfe.target_id;
            obj.events[target_id].push_back(std::move(wfe));
        }
    }

    template <typename Archive>
    void FillWeaponsPlatformEventViaSharedPtrVecs(Archive& ar, WeaponsPlatformEvent& obj)
    {
        static_assert(Archive::is_loading::value);
        using boost::serialization::make_nvp;

        int ingored = 0;
        ar >> make_nvp("bout", ingored)
           >> make_nvp("attacker_id", obj.attacker_id)
           >> make_nvp("attacker_owner_id", obj.attacker_owner_id);

        using WeaponFireEventPtr = std::shared_ptr<WeaponFireEvent>;
        std::map<int, std::vector<WeaponFireEventPtr>> shared_wfes;

        ar >> make_nvp("events", shared_wfes);

        static_assert(std::is_same_v<std::decay_t<decltype(obj.events)>, std::map<int, std::vector<WeaponFireEvent>>>);


        obj.events.clear();
        for (auto& [target_id, shared_wfe_vec] : shared_wfes) {
            auto value_wfes_rng = shared_wfe_vec | range_filter(not_null) | range_transform(extract_from_shptr);
            static_assert(std::is_same_v<decltype(*value_wfes_rng.begin()), WeaponFireEvent&>);
            obj.events.emplace(std::piecewise_construct,
                               std::forward_as_tuple(target_id),
                               std::forward_as_tuple(std::make_move_iterator(value_wfes_rng.begin()),
                                                     std::make_move_iterator(value_wfes_rng.end())));
        }
    }

    template <typename Archive>
    void Serialize(Archive& ar, WeaponsPlatformEvent& obj, bool include_bout)
    {
        using boost::serialization::make_nvp;
        if (include_bout) {
            int ignored = -1;
            ar  & make_nvp("bout", ignored);
        }
        ar  & make_nvp("attacker_id", obj.attacker_id)
            & make_nvp("attacker_owner_id", obj.attacker_owner_id)
            & make_nvp("events", obj.events);
    }
}

template <typename Archive>
void serialize(Archive& ar, WeaponsPlatformEvent& obj, unsigned int const version)
{
    using boost::serialization::make_nvp;
    using boost::serialization::base_object;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    if constexpr (Archive::is_loading::value) {
        if (version < 5) {
            FillWeaponsPlatformEventViaSharedPtrVecs(ar, obj);
        } else if constexpr (std::is_same_v<Archive, boost::archive::xml_iarchive>) {
            std::vector<std::string> strs;
            ar >> make_nvp("WeaponEvents", strs);
            FillWeaponsPlatformEvent(obj, strs);
        } else {
            Serialize(ar, obj, version < 5); // format version < 5 had bout encoded
        }
    } else {
        if constexpr (std::is_same_v<Archive, boost::archive::xml_oarchive>) {
            std::vector<std::string> strs = ToStrings(obj);
            ar << make_nvp("WeaponEvents", strs);
        } else {
            Serialize(ar, obj, false); // new format omits bout
        }
    }
}

BOOST_CLASS_VERSION(WeaponsPlatformEvent, 5)
BOOST_CLASS_EXPORT(WeaponsPlatformEvent)

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, WeaponsPlatformEvent&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, WeaponsPlatformEvent&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, WeaponsPlatformEvent&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, WeaponsPlatformEvent&, unsigned int const);


template <typename Archive>
void serialize(Archive& ar, CombatParticipantState& obj, const unsigned int/* version*/)
{
    using namespace boost::serialization;

    ar & make_nvp("current_health", obj.current_health)
       & make_nvp("max_health", obj.max_health);
}


BOOST_CLASS_EXPORT(CombatLog)

template <typename Archive>
void serialize(Archive& ar, CombatLog& obj, const unsigned int version)
{
    using namespace boost::serialization;

    // CombatEvents are serialized only through
    // pointers to their base class.
    // Therefore we need to manually register their types
    // in the archive.
    ar.template register_type<WeaponFireEvent>();
    ar.template register_type<IncapacitationEvent>();
    ar.template register_type<BoutBeginEvent>();
    ar.template register_type<InitialStealthEvent>();
    ar.template register_type<StealthChangeEvent>();
    ar.template register_type<WeaponsPlatformEvent>();

    ar  & make_nvp("turn", obj.turn)
        & make_nvp("system_id", obj.system_id);

    Serialize(ar, obj.empire_ids, "empire_ids", version < 2);
    Serialize(ar, obj.object_ids, "object_ids", version < 2);
    Serialize(ar, obj.damaged_object_ids, "damaged_object_ids", version < 2);
    Serialize(ar, obj.destroyed_object_ids, "destroyed_object_ids", version < 2);

    if (obj.combat_events.size() > 1)
        TraceLogger() << "CombatLog::serialize turn " << obj.turn << "  combat at " << obj.system_id << "  combat events size: " << obj.combat_events.size();
    try {
        ar  & make_nvp("combat_events", obj.combat_events);
    } catch (const std::exception& e) {
        ErrorLogger() << "combat events serializing failed!: caught exception: " << e.what();
    }

    static_assert(std::is_same_v<std::pair<int, CombatParticipantState>, std::decay_t<decltype(*obj.participant_states.begin())>>);

    Serialize(ar, obj.participant_states, "participant_states", version < 3);
}


template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLog&, const unsigned int);
template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLog&, const unsigned int);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLog&, const unsigned int);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLog&, const unsigned int);


template <typename Archive>
void serialize(Archive& ar, CombatLogManager& obj, const unsigned int/* version*/)
{
    using namespace boost::serialization;

    std::map<int, CombatLog> logs;

    if constexpr (Archive::is_saving::value) {
        logs.insert(obj.m_logs.begin(), obj.m_logs.end());
        // TODO: filter logs by who should have access to them
    }

    ar  & make_nvp("logs", logs);

    if constexpr (Archive::is_loading::value) {
        int latest_log_id = 0;
        ar  & make_nvp("m_latest_log_id", latest_log_id);
        obj.m_latest_log_id.store(latest_log_id);
    } else {
        int latest_log_id = obj.m_latest_log_id.load();
        ar  & make_nvp("m_latest_log_id", latest_log_id);
    }

    if constexpr (Archive::is_loading::value) {
        // copy new logs, but don't erase old ones
        obj.m_logs.insert(std::make_move_iterator(logs.begin()), std::make_move_iterator(logs.end()));
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLogManager&, const unsigned int);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLogManager&, const unsigned int);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLogManager&, const unsigned int);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLogManager&, const unsigned int);


template <typename Archive>
void SerializeIncompleteLogs(Archive& ar, CombatLogManager& obj, const unsigned int/* version*/)
{
    using namespace boost::serialization;

    int latest_log_id = obj.m_latest_log_id.load();
    if constexpr (Archive::is_loading::value) {
        int old_latest_log_id = latest_log_id;
        ar  & make_nvp("m_latest_log_id", latest_log_id);
        obj.m_latest_log_id.store(latest_log_id);
        DebugLogger(combat_log) << "SerializeIncompleteLogs loaded latest log id: " << latest_log_id << " and had old latest log id: " << old_latest_log_id;

        // If the new m_latest_log_id is greater than the old one then add all
        // of the new ids to the incomplete log set.
        if (latest_log_id > old_latest_log_id)
            for (++old_latest_log_id; old_latest_log_id <= latest_log_id; ++old_latest_log_id)
                obj.m_incomplete_logs.insert(old_latest_log_id);

    } else {
        ar  & make_nvp("m_latest_log_id", latest_log_id);
        DebugLogger(combat_log) << "SerializeIncompleteLogs saved latest log id: " << latest_log_id;
    }
}

template void SerializeIncompleteLogs<freeorion_bin_oarchive>(freeorion_bin_oarchive&, CombatLogManager&, unsigned int const);
template void SerializeIncompleteLogs<freeorion_bin_iarchive>(freeorion_bin_iarchive&, CombatLogManager&, unsigned int const);
template void SerializeIncompleteLogs<freeorion_xml_oarchive>(freeorion_xml_oarchive&, CombatLogManager&, unsigned int const);
template void SerializeIncompleteLogs<freeorion_xml_iarchive>(freeorion_xml_iarchive&, CombatLogManager&, unsigned int const);

