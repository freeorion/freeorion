import freeorion as fo
from common.print_utils import Float, Sequence, Table, Text


import natives
import planets
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
    planet_size_dist = {size: 0 for size in planets.planet_sizes}
    for system in sys_list:
        planet_count = 0
        for planet in fo.sys_get_planets(system):
            this_size = fo.planet_get_size(planet)
            if this_size in planets.planet_sizes:
                planet_count += 1
                planet_size_dist[this_size] += 1
        planet_count_dist.setdefault(planet_count, [0])[0] += 1
    planet_tally = sum(planet_size_dist.values())

    count_distribution_table = Table(
        [Text('planets in system'), Text('num of systems'), Float('% of systems', precession=1)],
        table_name='Planet Count Distribution'
    )
    for planet_count, sys_count in planet_count_dist.items():
        count_distribution_table.add_row((planet_count, sys_count[0], 100.0 * sys_count[0] / len(sys_list)))
    print count_distribution_table
    print

    size_distribution = Table(
        [Text('size'), Text('count'), Float('% of planets', precession=1)],
        table_name='Planet Size Distribution'
    )
    for planet_size, planet_count in sorted(planet_size_dist.items()):
        size_distribution.add_row((planet_size, planet_count, 100.0 * planet_count / planet_tally))
    print size_distribution
    print


def log_planet_type_summary(sys_list):
    planet_type_summary_table = {k: 0 for k in planets.planet_types}
    for system in sys_list:
        for planet in fo.sys_get_planets(system):
            planet_type_summary_table[fo.planet_get_type(planet)] += 1
    planet_total = sum(planet_type_summary_table.values())

    type_summary_table = Table(
        [Text('planet type', align='<'), Float('% of planets', precession=1)],
        table_name='Planet Type Summary for a total of %d placed planets' % planet_total
    )

    for planet_type, planet_count in sorted(planet_type_summary_table.items()):
        type_summary_table.add_row((planet_type.name, 100.0 * planet_count / planet_total))
    print type_summary_table
    print


def log_species_summary(native_freq):
    num_empires = sum(empire_species.values())
    num_species = len(fo.get_playable_species())
    exp_count = num_empires // num_species

    species_summary_table = Table(
        [Text('species'), Text('count'), Float('%', precession=1)],
        table_name=('Empire Starting Species Summary\n'
                    'Approximately %d to %d empires expected per species\n'
                    '%d Empires and %d playable species') % (max(0, exp_count - 1), exp_count + 1,
                                                             num_empires, num_species)
    )

    for species, count in sorted(empire_species.items()):
        species_summary_table.add_row((species, count, 100.0 * count / num_empires))
    print species_summary_table
    print

    native_chance = universe_tables.NATIVE_FREQUENCY[native_freq]
    # as the value in the universe table is higher for a lower frequency, we have to invert it
    # a value of 0 means no natives, in this case return immediately
    if native_chance <= 0:
        return
    native_table = Table(
        [
            Text('settled natives'),
            Text('on planets'),
            Float('total', precession=1),
            Float('actual', precession=1),
            Float('expected', precession=1),
            Sequence('planet types')
        ],
        table_name="Natives Placement Summary (native frequency: %.1f%%)" % (100 * native_chance)
    )

    native_potential_planet_total = sum(potential_native_planet_summary.values())
    for species in sorted(species_summary):
        if species_summary[species] > 0:
            settleable_planets = 0
            expectation_tally = 0.0
            for p_type in natives.planet_types_for_natives[species]:
                settleable_planets += potential_native_planet_summary[p_type]
                expectation_tally += (
                        native_chance *
                        100.0 *
                        potential_native_planet_summary[p_type] /
                        (1E-10 + len(natives.natives_for_planet_type[p_type]))
                )
            expectation = expectation_tally / (1E-10 + settleable_planets)
            native_table.add_row(
                [species,
                 species_summary[species], 100.0 * species_summary[species] / (1E-10 + native_potential_planet_total),
                 100.0 * species_summary[species] / (1E-10 + settleable_planets),
                 expectation,
                 [str(p_t) for p_t in natives.planet_types_for_natives[species]]]
            )

    print native_table
    print

    native_settled_planet_total = sum(settled_native_planet_summary.values())
    type_summary_table = Table(
        [Text('planet type'), Float('potential (% of tot)', precession=1),
         Float('settled (% of potential)', precession=1)],
        table_name=("Planet Type Summary for Native Planets (native frequency: %.1f%%)\n"
                    "Totals: native_potential_planet_total: %s; native_settled_planet_total %s"
                    ) % (100 * native_chance, native_potential_planet_total, native_settled_planet_total)
    )

    for planet_type, planet_count in sorted(potential_native_planet_summary.items()):
        settled_planet_count = settled_native_planet_summary.get(planet_type, 0)
        potential_percent = 100.0 * planet_count / (1E-10 + native_potential_planet_total)
        settled_percent = 100.0 * settled_planet_count / (1E-10 + planet_count)
        type_summary_table.add_row((planet_type.name, potential_percent, settled_percent))
    print type_summary_table
    print


def log_monsters_summary(monster_freq):
    monster_place_table = Table([Text('monster'), Text('count')], table_name='Monster placement')
    for monster, counter in sorted(monsters_summary):
        monster_place_table.add_row([monster, counter])
    print monster_place_table
    print

    monster_chance = universe_tables.MONSTER_FREQUENCY[monster_freq]
    monster_table = Table(
        [Text('monster'), Float('chance'), Text('attempts'),
         Text('number placed'), Text('number valid sys locs'), Text('number valid nest planet locs')],
        table_name=("Space Monsters Placement Summary\n"
                    "Tracked Monster and Nest Summary (base monster freq: %4.1f%%)" % (100 * monster_chance))
    )
    for monster in sorted(tracked_monsters_summary):
        monster_table.add_row(
            (monster, tracked_monsters_chance[monster],
             tracked_monsters_tries[monster], tracked_monsters_summary[monster],
             tracked_monsters_location_summary[monster], tracked_nest_location_summary[monster])
        )
    print monster_table
    print


def log_specials_summary():
    special_placement_count_table = Table(
        [Text('special'), Text('times placed')],
        table_name="Special Placement Summary"
    )
    for special in sorted(specials_summary):
        special_placement_count_table.add_row([special, specials_summary[special]])
    print special_placement_count_table
    print

    special_placement = Table(
        [Text('count'), Text('tally'), Float('% of objects', precession=1)],
        table_name="Specials Count(Repeat) Distribution"
    )
    objects_tally = sum(specials_repeat_dist.values())
    for number, tally in specials_repeat_dist.items():
        special_placement.add_row((number, tally, 100.0 * tally / (1E-10 + objects_tally)))
    print special_placement
    print


def log_systems():
    universe = fo.get_universe()

    systems_table = Table(
        [Text('id'), Text('name'), Sequence('planets'), Sequence('connections'), Text('star')],
        table_name='System summary')
    for sid in fo.get_systems():
        system = universe.getSystem(sid)
        systems_table.add_row([
            sid, system.name, fo.sys_get_planets(sid), fo.sys_get_starlanes(sid), system.starType.name
        ])

    # Printing too much info at once will lead to truncation of text
    for line in systems_table.get_table().split('\n'):
        print line


def log_planets():
    universe = fo.get_universe()

    planets_table = Table(
        [Text('id'), Text('name'), Text('system'), Text('type'),
         Sequence('specials'), Text('species'), Sequence('buildings')],
        table_name='Planets summary')
    # group planets by system
    for sid in fo.get_systems():
        for pid in fo.sys_get_planets(sid):
            planet = universe.getPlanet(pid)

            planet_type = fo.planet_get_type(pid).name
            planet_size = fo.planet_get_size(pid).name
            if planet_type != planet_size:
                planet_type = '%s %s' % (planet_type, planet_size)

            buildings = [universe.getBuilding(x).name for x in planet.buildingIDs]
            planets_table.add_row([
                pid, planet.name, planet.systemID, planet_type, list(planet.specials), planet.speciesName, buildings
            ])

    # Printing too much info at once will lead to truncation of text
    for line in planets_table.get_table().split('\n'):
        print line
