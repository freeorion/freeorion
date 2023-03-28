#include "Serialize.h"
#include "Serialize.ipp"

#include "../universe/Species.h"

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

        sm.SetSpeciesHomeworlds(std::move(species_homeworlds));

    } else {
        auto& species_homeworlds = sm.GetSpeciesHomeworldsMap();
        auto& species_empire_opinions = sm.GetSpeciesEmpireOpinionsMap();
        auto& species_species_opinions = sm.GetSpeciesSpeciesOpinionsMap();
        auto& species_species_ships_destroyed = sm.SpeciesShipsDestroyed();

        ar  & BOOST_SERIALIZATION_NVP(species_homeworlds)
            & BOOST_SERIALIZATION_NVP(species_empire_opinions)
            & BOOST_SERIALIZATION_NVP(species_species_opinions)
            & BOOST_SERIALIZATION_NVP(species_species_ships_destroyed);
    }
}

template void serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, SpeciesManager&, unsigned int const);
template void serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, SpeciesManager&, unsigned int const);
