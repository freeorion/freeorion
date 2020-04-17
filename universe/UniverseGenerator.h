#ifndef _UniverseGenerator_h_
#define _UniverseGenerator_h_

#include "Universe.h"

struct PlayerSetupData;

/** Set active meter current values equal to target/max meter current
 * values.  Useful when creating new object after applying effects. */
void SetActiveMetersToTargetMaxCurrentValues(ObjectMap& object_map);

/** Set the population of unowned planets to a random fraction of
 * their target values. */
void SetNativePopulationValues(ObjectMap& object_map);

/** Creates starlanes and adds them systems already generated. */
void GenerateStarlanes(int max_jumps_between_systems, int max_starlane_length);

/** Sets empire homeworld
 * This includes setting ownership, capital, species,
 * preferred environment (planet type) for the species */
bool SetEmpireHomeworld(Empire* empire, int planet_id, std::string species_name);

/** Creates Empires objects for each entry in \a player_setup_data with
 * empire ids equal to the corresponding map keys (so that the calling code
 * can know which empire belongs to which player). */
void InitEmpires(const std::map<int, PlayerSetupData>& player_setup_data);

#endif // _UniverseGenerator_h_
