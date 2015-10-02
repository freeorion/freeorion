import freeorion as fo
import planets
import natives
import universe_tables


species_summary = {species: 0 for species in fo.get_native_species()}
empire_species = {species: 0 for species in fo.get_playable_species()}
potential_native_planet_summary = {planet_type: 0 for planet_type in planets.planet_types}
settled_native_planet_summary = {planet_type: 0 for planet_type in planets.planet_types}
monsters_summary = []
tracked_monsters_chance = {}
tracked_monsters_tries = {}
tracked_monsters_summary = {}
tracked_monsters_location_summary = {}
tracked_nest_location_summary = {}
specials_summary = {special: 0 for special in fo.get_all_specials()}
specials_repeat_dist = {count: 0 for count in [0, 1, 2, 3, 4]}

def log_planet_count_dist(sys_list):
    planet_count_dist = {}
    planet_size_dist = {size : 0 for size in planets.planet_sizes}
    for system in sys_list:
        planet_count = 0
        for planet in fo.sys_get_planets(system):
            this_size = fo.planet_get_size(planet)
            if this_size in planets.planet_sizes:
                planet_count += 1
                planet_size_dist[this_size] += 1
        planet_count_dist.setdefault(planet_count, [0])[0] += 1
    planet_tally = sum(planet_size_dist.values())
    print "Planet Count Distribution: planets_in_system | num_systems | % of systems"
    for planet_count, sys_count in planet_count_dist.items():
        print "\t\t\t%2d  | %5d | %4.1f%%" % (planet_count, sys_count[0], 100.0 * sys_count[0] / len(sys_list))
    print
    print "Planet Size Distribution: size | count | % of planets"
    for planet_size, planet_count in planet_size_dist.items():
        print "\t\t%-12s | %5d | %4.1f%%" % (planet_size, planet_count, 100.0 * planet_count / planet_tally)


def log_planet_type_summary(sys_list):
    planet_type_summary = {k: 0 for k in planets.planet_types}
    for system in sys_list:
        for planet in fo.sys_get_planets(system):
            planet_type_summary[fo.planet_get_type(planet)] += 1
    planet_total = sum(planet_type_summary.values())
    print "Planet Type Summary for a total of %d placed planets" % planet_total
    for planet_type, planet_count in planet_type_summary.items():
        print "%-12s %4.1f%%" % (planet_type.name, 100.0 * planet_count / planet_total)


def log_species_summary(native_freq):
    num_empires = sum(empire_species.values())
    num_species = len(fo.get_playable_species())
    exp_count = num_empires // num_species
    print "Empire Starting Species Summary for %d Empires and %d playable species" % (num_empires, num_species)
    print "Approximately %d to %d empires expected per species" % (max(0, exp_count - 1), exp_count + 1)
    print "%-16s :   # --     %%" % ("species")
    for species, count in empire_species.items():
        if count:
            print "%-16s : %3d -- %5.1f%%" % (species, count, 100.0 * count / num_empires)
    print
    native_chance = universe_tables.NATIVE_FREQUENCY[native_freq]
    # as the value in the universe table is higher for a lower frequency, we have to invert it
    # a value of 0 means no natives, in this case return immediately
    if native_chance <= 0:
        return
    print "Natives Placement Summary (native frequency: %.1f%%)" % (100 * native_chance)
    native_potential_planet_total = sum(potential_native_planet_summary.values())
    for species in species_summary:
        if species_summary[species] > 0:
            settleable_planets = 0
            expectation_tally = 0.0
            for p_type in natives.planet_types_for_natives[species]:
                settleable_planets += potential_native_planet_summary[p_type]
                expectation_tally += native_chance * 100.0 * potential_native_planet_summary[p_type] / (1E-10 + len(natives.natives_for_planet_type[p_type]))
            expectation = expectation_tally / (1E-10 + settleable_planets)
            print "Settled natives %18s on %3d planets -- %5.1f%% of total and %5.1f%% vs %5.1f%% (actual vs expected) of %s planets" % \
                (species, species_summary[species], 100.0 * species_summary[species] / (1E-10 + native_potential_planet_total),
                 100.0 * species_summary[species] / (1E-10 + settleable_planets), expectation, [str(p_t) for p_t in natives.planet_types_for_natives[species]])
    print
    native_settled_planet_total = sum(settled_native_planet_summary.values())
    print "Planet Type Summary for Native Planets (native frequency: %.1f%%)" % (100 * native_chance)
    print "%19s : %-s" % ("Potential (% of tot)", "Settled (% of potential)")
    print "%-13s %5d : %5d" % ("Totals", native_potential_planet_total, native_settled_planet_total)
    for planet_type, planet_count in potential_native_planet_summary.items():
        settled_planet_count = settled_native_planet_summary.get(planet_type, 0)
        potential_percent = 100.0 * planet_count / (1E-10 + native_potential_planet_total)
        settled_percent = 100.0 * settled_planet_count / (1E-10 + planet_count)
        print "%-12s %5.1f%% : %5.1f%%" % (planet_type.name, potential_percent, settled_percent)

def log_monsters_summary(monster_freq):
    print "Space Monsters Placement Summary"
    for monster, counter in monsters_summary:
        if counter > 0:
            print "Placed space monster", monster, counter, "times"
    print
    monster_chance = universe_tables.MONSTER_FREQUENCY[monster_freq]
    # the following loop depends on name mapping done in monsters.py
    print "Tracked Monster and Nest Summary (base monster freq: %4.1f%%)" % (100 * monster_chance)
    print "%-18s | %8s | %8s | %8s | %12s | %s" %\
          ("Monster", "chance", "# tried", "# placed", "# valid sys locs", "# valid nest planet locs")
    for monster in tracked_monsters_summary:
        print "%-18s |  %5.2f   |   %4d   |   %4d   |       %4d       | %4d" % (monster, tracked_monsters_chance[monster],
            tracked_monsters_tries[monster], tracked_monsters_summary[monster],
            tracked_monsters_location_summary[monster], tracked_nest_location_summary[monster])

def log_specials_summary():
    print "Special Placement Summary"
    for special in specials_summary:
        if specials_summary[special] > 0:
            print "Placed special", special, specials_summary[special], "times"
    print
    print "Specials Count(Repeat) Distribution:"
    print "\t count  | tally | % of objects"
    objects_tally = sum(specials_repeat_dist.values())
    for number, tally in specials_repeat_dist.items():
        print "\t\t%3d | %5d | %4.1f%%" % (number, tally, 100.0 * tally / (1E-10 + objects_tally))
