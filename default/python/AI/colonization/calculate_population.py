import freeOrionAIInterface as fo

import AIDependencies
from AIDependencies import (
    POP_CONST_MOD_MAP,
    POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES,
    POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES,
)
from colonization.colony_score import debug_rating
from freeorion_tools import get_species_population, tech_is_complete

active_growth_specials = {}


def calc_max_pop(planet, species, detail):  # noqa: C901
    planet_size = planet.habitableSize
    planet_env = species.getPlanetEnvironment(planet.type)
    if planet_env == fo.planetEnvironment.uninhabitable:
        detail.append("Uninhabitable.")
        return 0

    for bldg_id in planet.buildingIDs:
        building = fo.getUniverse().getBuilding(bldg_id)
        if not building:
            continue
        if building.buildingTypeName == "BLD_GATEWAY_VOID":
            detail.append("Gateway to the void: Uninhabitable.")
            return 0

    if planet.speciesName in AIDependencies.SPECIES_FIXED_POPULATION:
        return AIDependencies.SPECIES_FIXED_POPULATION[planet.speciesName]

    tag_list = list(species.tags) if species else []
    pop_tag_mod = get_species_population(species.name)
    if planet.type == fo.planetType.gasGiant and "GASEOUS" in tag_list:
        gaseous_adjustment = AIDependencies.GASEOUS_POP_FACTOR
        detail.append("GASEOUS adjustment: %.2f" % gaseous_adjustment)
    else:
        gaseous_adjustment = 1.0

    planet_specials = set(planet.specials)

    base_pop_modified_by_species = 0
    base_pop_not_modified_by_species = 0
    pop_const_mod = 0

    # first, account for the environment
    environment_mod = POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES["environment_bonus"][planet_env]
    detail.append("Base environment: %d" % environment_mod)
    base_pop_modified_by_species += environment_mod

    # find all applicable modifiers
    for tech in POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES:
        if tech != "environment_bonus" and tech_is_complete(tech):
            base_pop_modified_by_species += POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES[tech][planet_env]
            detail.append("%s_PSM_early(%d)" % (tech, POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES[tech][planet_env]))

    for tech in POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES:
        if tech_is_complete(tech):
            base_pop_not_modified_by_species += POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES[tech][planet_env]
            detail.append("%s_PSM_late(%d)" % (tech, POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES[tech][planet_env]))

    for tech in POP_CONST_MOD_MAP:
        if tech_is_complete(tech):
            pop_const_mod += POP_CONST_MOD_MAP[tech][planet_env]
            detail.append("%s_PCM(%d)" % (tech, POP_CONST_MOD_MAP[tech][planet_env]))

    for _special in planet_specials.intersection(AIDependencies.POP_FIXED_MOD_SPECIALS):
        if AIDependencies.not_affect_by_special(_special, species.name):
            continue
        this_mod = AIDependencies.POP_FIXED_MOD_SPECIALS[_special]
        detail.append("%s_PCM(%d)" % (_special, this_mod))
        pop_const_mod += this_mod

    for _special in planet_specials.intersection(AIDependencies.POP_PROPORTIONAL_MOD_SPECIALS):
        this_mod = AIDependencies.POP_PROPORTIONAL_MOD_SPECIALS[_special]
        detail.append(f"{_special} (maxPop{this_mod:+.1f})")
        base_pop_not_modified_by_species += this_mod

    gaia = AIDependencies.GAIA_SPECIAL
    if gaia in planet_specials and not AIDependencies.not_affect_by_special(gaia, species):
        base_pop_not_modified_by_species += 3
        detail.append("Gaia_PSM_late(3)")

    if "SELF_SUSTAINING" in tag_list:
        if planet_env == fo.planetEnvironment.good:
            base_pop_not_modified_by_species += 3
            detail.append("SelfSustaining_PSM_late(3)")

    applicable_boosts = set()
    for this_tag in [tag for tag in tag_list if tag in AIDependencies.metabolismBoostMap]:
        metab_boosts = AIDependencies.metabolismBoostMap.get(this_tag, [])
        for key in active_growth_specials.keys():
            if len(active_growth_specials[key]) > 0 and key in metab_boosts:
                applicable_boosts.add(key)
                detail.append("%s boost active" % key)
        for boost in metab_boosts:
            if boost in planet_specials:
                applicable_boosts.add(boost)
                detail.append("%s boost present" % boost)

    n_boosts = len(applicable_boosts)
    if n_boosts:
        base_pop_not_modified_by_species += n_boosts
        detail.append("boosts_PSM(%d from %s)" % (n_boosts, applicable_boosts))

    if planet.id in species.homeworlds:
        detail.append("Homeworld (2)")
        base_pop_not_modified_by_species += 2

    if AIDependencies.TAG_LIGHT_SENSITIVE in tag_list:
        star_type = fo.getUniverse().getSystem(planet.systemID).starType
        star_pop_mod = AIDependencies.POP_MOD_LIGHTSENSITIVE_STAR_MAP.get(star_type, 0)
        base_pop_not_modified_by_species += star_pop_mod
        detail.append("Lightsensitive Star Bonus_PSM_late(%.1f)" % star_pop_mod)

    def max_pop_size():
        species_effect = (pop_tag_mod - 1) * abs(base_pop_modified_by_species)
        gaseous_effect = (gaseous_adjustment - 1) * abs(base_pop_modified_by_species)
        base_pop = base_pop_not_modified_by_species + base_pop_modified_by_species + species_effect + gaseous_effect
        return planet_size * base_pop + pop_const_mod

    target_pop = max_pop_size()
    if "PHOTOTROPHIC" in tag_list and target_pop > 0:
        star_type = fo.getUniverse().getSystem(planet.systemID).starType
        star_pop_mod = AIDependencies.POP_MOD_PHOTOTROPHIC_STAR_MAP.get(star_type, 0)
        base_pop_not_modified_by_species += star_pop_mod
        detail.append("Phototropic Star Bonus_PSM_late(%0.1f)" % star_pop_mod)
        target_pop = max_pop_size()

    detail.append("max_pop = base + size*[psm_early + species_mod*abs(psm_early) + psm_late]")
    detail.append(
        "        = %.2f + %d * [%.2f + %.2f*abs(%.2f) + %.2f]"
        % (
            pop_const_mod,
            planet_size,
            base_pop_modified_by_species,
            (pop_tag_mod + gaseous_adjustment - 2),
            base_pop_modified_by_species,
            base_pop_not_modified_by_species,
        )
    )
    detail.append("        = %.2f" % target_pop)
    # new rating code passes a dummy to this function and only adds the result to detail
    debug_rating(f"Details from calc_max_pop: {detail}")
    return target_pop
