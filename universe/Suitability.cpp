#include "Suitability.h"

#include "Building.h"
#include "Fleet.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Planet.h"
#include "Predicates.h"
#include "Species.h"
#include "System.h"
#include "Field.h"
#include "Universe.h"
#include "UniverseObject.h"
#include "Condition.h"
#include "Suitability.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"
#include "../util/Random.h"
#include "../util/Logger.h"
#include "../util/MultiplayerCommon.h"

std::string GetBestSuitable(std::map<std::string, std::pair<PlanetEnvironment, float> > suitabilities)
{
    float best_suitable = -100000000000000.0f;
    std::string best_species;
    for (std::map<std::string, std::pair<PlanetEnvironment, float> >::const_iterator it = suitabilities.begin(); it != suitabilities.end(); it++)
    {
        std::string species_name = it->first;
        float suitability = it->second.second;
        if (suitability > best_suitable) {
            best_suitable = suitability;
            best_species = species_name;
        }
    }
    return best_species;
}

std::map<std::string, std::pair<PlanetEnvironment, float> > GetSuitabilitiesForSpecies(int empire_id, int planet_id, std::set<std::string> species_names)
{
    TemporaryPtr<Planet> planet = GetPlanet(planet_id);
    std::map<std::string, std::pair<PlanetEnvironment, float> > population_counts;

    // Back original values up to restore later
    std::string original_planet_species = planet->SpeciesName();
    int original_owner_id = planet->Owner();
    float orig_initial_target_pop = planet->GetMeter(METER_TARGET_POPULATION)->Initial();

    // We don't want to notify universe about what we are about to change
    GetUniverse().InhibitUniverseObjectSignals(true);

    // Set owner to allow computing planet meters with this empire
    planet->SetOwner(empire_id);

    for (std::set<std::string>::const_iterator it = species_names.begin();
         it != species_names.end(); it++)
    {
        std::string species_name = *it;

        const Species* species = GetSpecies(species_name);
        if (!species)
            continue;

        PlanetEnvironment planet_environment = species->GetPlanetEnvironment(planet->Type());
        population_counts[species_name].first = planet_environment;
        population_counts[species_name].second = 0.0f;

        if (planet_environment != PE_UNINHABITABLE) {
            // Setting the planet's species allows all of its meters to reflect
            // species (and empire) properties, such as environment type
            // preferences and techs.
            // @see also: MapWnd::UpdateMeterEstimates()
            planet->SetSpecies(species_name);
            planet->GetMeter(METER_TARGET_POPULATION)->Set(0.0, 0.0);

            // Compute the meter values, for newly set species
            GetUniverse().UpdateMeterEstimates(planet_id);

            // Store the value we're interested in
            population_counts[species_name].second = planet->CurrentMeterValue(METER_TARGET_POPULATION);
        }
    }

    // Restore planet's original values
    planet->SetSpecies(original_planet_species);
    planet->SetOwner(original_owner_id);
    planet->GetMeter(METER_TARGET_POPULATION)->Set(orig_initial_target_pop, orig_initial_target_pop);

    // Restore meters
    GetUniverse().UpdateMeterEstimates(planet_id);

    // Restore signalling
    GetUniverse().InhibitUniverseObjectSignals(false);

    return population_counts;
}

std::set<std::string> GetColonizerSpecies(Empire* empire, TemporaryPtr<const Planet> planet, bool all_species /* = true */)
{
    std::set<std::string> species_names;

    if (all_species) {
        const SpeciesManager& species_manager = GetSpeciesManager();
        for (SpeciesManager::iterator it = species_manager.begin(); it != species_manager.end(); ++it)
        {
            if (it->second && (it->second->Tags().find("CTRL_ALWAYS_REPORT") != it->second->Tags().end()))
                    species_names.insert(it->first);
        }
    }

    const std::vector<int> pop_center_ids = empire->GetPopulationPool().PopCenterIDs();
    for (std::vector<int>::const_iterator it = pop_center_ids.begin(); it != pop_center_ids.end(); it++) {
        TemporaryPtr<const UniverseObject> obj = GetUniverseObject(*it);
        TemporaryPtr<const PopCenter> pc = boost::dynamic_pointer_cast<const PopCenter>(obj);
        if (!pc)
            continue;

        const std::string& species_name = pc->SpeciesName();
        if (species_name.empty())
            continue;

        const Species* species = GetSpecies(species_name);
        if (!species)
            continue;

        const PlanetEnvironment planet_environment = species->GetPlanetEnvironment(planet->Type());
        if (planet_environment == PE_UNINHABITABLE)
            continue;

        // Exclude species that can't colonize UNLESS they
        // are already here (aka: it's their home planet). Showing them on
        // their own planet allows comparison vs other races, which might
        // be better suited to this planet.
        if (species->CanColonize() || species_name == planet->SpeciesName())
            species_names.insert(species_name);
    }
    return species_names;
}
