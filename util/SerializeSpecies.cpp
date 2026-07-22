#include "Serialize.h"
#include "Serialize.ipp"

#include "../universe/Species.h"

namespace {
    template <typename K, typename V>
    using flat_map = boost::container::flat_map<K, V, std::less<>>;
    template <typename V>
    using flat_set = boost::container::flat_set<V, std::less<>>;

    constexpr auto to_uid = [](const auto& i) noexcept { return UniverseObjectID{i}; };
    constexpr auto to_value_int = [](const auto& i) noexcept -> int { return Value(i); };

    auto ToIntFlatSet(const auto& in)
    { return in | range_transform(to_value_int) | range_to<flat_set<int>>(); }

    auto ToUniverseObjectIDFlatSet(const auto& in)
    { return in | range_transform(to_uid) | range_to<flat_set<UniverseObjectID>>(); }
}

template <typename Archive>
void serialize(Archive& ar, SpeciesManager& sm, unsigned int const version)
{
    // Don't need to send all the data about species, as this is derived from
    // content data files in scripting/species that should be available to any
    // client or server. Instead, just need to send the gamestate portion of
    // species: their homeworlds in the current game, and their opinions of
    // empires and of eachother
    if (Archive::is_loading::value && version < 1) {
        std::map<std::string, std::set<int>>                species_homeworlds;
        std::map<std::string, std::map<int, float>>         empire_opinions; // ignored
        std::map<std::string, std::map<std::string, float>> other_species_opinions; // ignored
        std::map<std::string, std::map<int, float>>         species_object_populations; // ignored
        std::map<std::string, std::map<std::string, int>>   species_ships_destroyed; // ingored

        ar  & BOOST_SERIALIZATION_NVP(species_homeworlds)
            & BOOST_SERIALIZATION_NVP(empire_opinions)
            & BOOST_SERIALIZATION_NVP(other_species_opinions)
            & BOOST_SERIALIZATION_NVP(species_object_populations)
            & BOOST_SERIALIZATION_NVP(species_ships_destroyed);

        {
            auto& sm_species_homeworlds = sm.GetSpeciesHomeworldsMap();
            sm_species_homeworlds.clear();
            for (auto& [key, vals] : species_homeworlds)
                sm_species_homeworlds.emplace(key, ToUniverseObjectIDFlatSet(vals));
        }

    } else {
        auto& sm_species_homeworlds = sm.GetSpeciesHomeworldsMap();
        auto& sm_species_empire_opinions = sm.GetSpeciesEmpireOpinionsMap();

        flat_map<std::string, flat_set<int>> species_homeworlds;
        flat_map<std::string, flat_map<int, std::pair<Meter, Meter>>> species_empire_opinions;
        if (Archive::is_saving::value) {
            for (const auto& [sp, uids] : sm_species_homeworlds)
                species_homeworlds.emplace(sp, ToIntFlatSet(uids));
            for (const auto& [sp, eid_opins] : sm_species_empire_opinions) {
                auto& int_meter_meter_map = species_empire_opinions[sp];
                for (const auto& [eid, opins] : eid_opins)
                    int_meter_meter_map.emplace(Value(eid), opins);
            }
        }

        auto& species_species_opinions = sm.GetSpeciesSpeciesOpinionsMap();
        auto& species_species_ships_destroyed = sm.SpeciesShipsDestroyed();

        ar  & BOOST_SERIALIZATION_NVP(species_homeworlds)
            & BOOST_SERIALIZATION_NVP(species_empire_opinions)
            & BOOST_SERIALIZATION_NVP(species_species_opinions)
            & BOOST_SERIALIZATION_NVP(species_species_ships_destroyed);

        if (Archive::is_loading::value) {
            sm_species_homeworlds.clear();
            for (auto& [sp, ids] : species_homeworlds.extract_sequence())
                sm_species_homeworlds.emplace(std::move(sp), ToUniverseObjectIDFlatSet(ids));
            sm_species_empire_opinions.clear();
            for (auto& [sp, id_opins] : species_empire_opinions.extract_sequence()) {
                auto& sm_eid_opins = sm_species_empire_opinions.emplace(std::piecewise_construct,
                                                                        std::forward_as_tuple(std::move(sp)),
                                                                        std::forward_as_tuple()).first->second;
                for (auto& [id, opins] : id_opins.extract_sequence())
                    sm_eid_opins.emplace(EmpireID{id}, std::move(opins));
            }
        }
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SpeciesManager&, unsigned int const);
