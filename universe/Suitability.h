// -*- C++ -*-
#ifndef _Suitability_h_
#define _Suitability_h_

#include "Enums.h"
#include "TemporaryPtr.h"

#include "../util/Export.h"

#include <set>
#include <map>
#include <string>

class Empire;
class Planet;

FO_COMMON_API std::set<std::string> GetColonizerSpecies(Empire* empire, TemporaryPtr<const Planet> planet, bool all_species = true); ///< get a set of all the species from the given empire that can colonize this planet
FO_COMMON_API std::map<std::string, std::pair<PlanetEnvironment, float> > GetSuitabilitiesForSpecies(int empire_id, int planet_id, std::set<std::string> species_names); ///< get the species, suitability pairs for all relevant species of the given empire and planet
FO_COMMON_API std::string GetBestSuitable(std::map<std::string, std::pair<PlanetEnvironment, float> > suitabilities); ///< choose the best suitable species from the given set

#endif // _Suitability_h_
