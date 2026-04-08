#include "Serialize.h"

#include "Serialize.ipp"
#include "../combat/CombatEvents.h"
#include "../combat/CombatLogManager.h"


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
    // <concepts> library not fully implemented in XCode 13.2
    template <class T>
    concept integral = std::is_integral_v<T>;

    template <integral T>
    constexpr const auto* GetFormatString() {
        if constexpr (std::is_same_v<T, unsigned int>)
            return "%u%n";
        else if constexpr (std::is_same_v<T, int>)
            return "%d%n";
    }

    constexpr bool have_to_chars_lib =
#if defined(__cpp_lib_to_chars)
        true;
#else
        false;
#endif

#if defined(__cpp_lib_to_chars) && defined(__cpp_lib_constexpr_charconv)
    constexpr
#endif
    std::size_t ToChars(integral auto num, char* buffer, char* buffer_end) {
        if constexpr (have_to_chars_lib) {
            const auto result_ptr = std::to_chars(buffer, buffer_end, num).ptr;
            return static_cast<std::size_t>(std::distance(buffer, result_ptr));
        } else {
            std::size_t buffer_sz = std::distance(buffer, buffer_end);
            auto temp = std::to_string(num);
            auto out_sz = std::min(buffer_sz, temp.size());
            std::copy_n(temp.begin(), out_sz, buffer);
            return out_sz;
        }
    }

    // returns { next unconsumed char*, true/false did the parse succeed }
    // parsed value returned in \a val_out
    auto FromChars(const char* start, const char* end, integral auto& val_out) -> std::pair<const char*, bool> {
        if constexpr (have_to_chars_lib) {
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


    consteval std::size_t Pow(std::size_t base, std::size_t exp) noexcept {
        std::size_t retval = 1;
        while (exp--)
            retval *= base;
        return retval;
    }

    constexpr int int_max = std::numeric_limits<int>::max();
    constexpr uint8_t int_digits = 11; // digits in base 11 of -2147483648 = -2^31
    static_assert(Pow(10, int_digits + 1) > static_cast<std::size_t>(int_max)); // biggest possible int should fit in buffer
    constexpr std::size_t ovt_buffer_size = 4*(int_digits + 1); // space for "-2147483648 -2147483648 -2147483648 -2147483648 "
    std::string ToString(const auto& data)
        requires requires { data.size(); } && std::is_same_v<int, std::decay_t<decltype(*data.begin())>>
    {
        std::string retval;

        try {
            retval.reserve(data.size() * (int_digits + 1) + int_digits + 2); // space for count and all values and gaps
        } catch(...) {}

        retval.append(std::to_string(data.size()));

        for (const auto& v : data)
            retval.append(" ").append(std::to_string(v));

        return retval;
    }

    void FillIntContainer(auto& container, std::string_view buffer)
        requires requires { container.push_back(1); } || requires { container.insert(1); }
    {
        if (buffer.empty())
            return;

        const auto* const buffer_end = buffer.data() + buffer.size();

        unsigned int count = 0;
        auto [next, success] = FromChars(buffer.data(), buffer_end, count);
        if (!success)
            return;

        if constexpr (requires { container.reserve(count); }) {
            try {
                container.reserve(count);
            } catch (...) {}
        }

        const auto get_int_from_chars = [buffer_end](const char* next, const int default_val) -> std::tuple<int, bool, const char*> {
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
            int result = default_val;
            auto [next_out, success] = FromChars(next, buffer_end, result);
            return {result, success, next_out};
        };


        for (std::size_t idx = 0; idx < static_cast<std::size_t>(count) && next != buffer_end; ++idx) {
            int num = 0;

            std::tie(num, success, next) = get_int_from_chars(next, INVALID_OBJECT_ID);
            if (!success)
                break;

            if constexpr (requires { container.push_back(num); })
                container.push_back(num);
            else if constexpr (requires { container.insert(num); })
                container.insert(num);
        }
    }

    template <typename Archive>
    void Serialize(Archive& ar, auto& container, const char* tag, bool old_non_string_format)
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


template <typename Archive>
void serialize(Archive& ar, StealthChangeEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));
    ar & make_nvp("bout", obj.bout)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(StealthChangeEvent, 4)
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
        & make_nvp("target_empire_id", obj.target_empire_id)
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


template <typename Archive>
void serialize(Archive& ar, WeaponFireEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    if (version < 5) {
        ar & make_nvp("bout", obj.bout)
           & make_nvp("round", obj.round)
           & make_nvp("attacker_id", obj.attacker_id)
           & make_nvp("target_id", obj.target_id)
           & make_nvp("weapon_name", obj.weapon_name)
           & make_nvp("power", obj.power)
           & make_nvp("shield", obj.shield)
           & make_nvp("damage", obj.damage)
           & make_nvp("target_owner_id", obj.target_owner_id)
           & make_nvp("attacker_owner_id", obj.attacker_owner_id);
    } else {
        ar & make_nvp("b", obj.bout)
           & make_nvp("r", obj.round)
           & make_nvp("a", obj.attacker_id)
           & make_nvp("t", obj.target_id)
           & make_nvp("w", obj.weapon_name)
           & make_nvp("p", obj.power)
           & make_nvp("s", obj.shield)
           & make_nvp("d", obj.damage)
           & make_nvp("to", obj.target_owner_id)
           & make_nvp("ao", obj.attacker_owner_id);
    }
}

BOOST_CLASS_VERSION(WeaponFireEvent, 5)
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


template <typename Archive>
void serialize(Archive& ar, WeaponsPlatformEvent& obj, unsigned int const version)
{
    using namespace boost::serialization;

    ar & make_nvp("CombatEvent", base_object<CombatEvent>(obj));

    ar & make_nvp("bout", obj.bout)
       & make_nvp("attacker_id", obj.attacker_id)
       & make_nvp("attacker_owner_id", obj.attacker_owner_id)
       & make_nvp("events", obj.events);
}

BOOST_CLASS_VERSION(WeaponsPlatformEvent, 4)
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

    ar & make_nvp("participant_states", obj.participant_states);
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

