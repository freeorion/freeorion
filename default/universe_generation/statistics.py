import foUniverseGenerator as fo
import planets


species_summary = {species: 0 for species in fo.get_native_species()}
specials_summary = {special: 0 for special in fo.get_all_specials()}


def log_planet_count_dist(sys_list):
    planet_count_dist = {}
    for system in sys_list:
        planet_count = 0
        for planet in fo.sys_get_planets(system):
            if fo.planet_get_size(planet) in planets.planet_sizes:
                planet_count += 1
        planet_count_dist.setdefault(planet_count, [0])[0] += 1
    print "Planet Count Distribution: planets_in_system | num_systems"
    for planet_count, sys_count in planet_count_dist.items():
        print "\t\t\t%2d  | %5d" % (planet_count, sys_count[0])


def log_planet_type_summary(sys_list):
    planet_type_summary = {k: 0 for k in planets.planet_types}
    for system in sys_list:
        for planet in fo.sys_get_planets(system):
            planet_type_summary[fo.planet_get_type(planet)] += 1
    planet_total = sum(planet_type_summary.values())
    print "Planet Type Summary for a total of %d placed planets" % planet_total
    for planet_type, planet_count in planet_type_summary.items():
        print "%-12s %.1f%%" % (planet_type.name, 100.0 * planet_count / planet_total)


def log_species_summary():
    print "Natives Placement Summary"
    for species in species_summary:
        if species_summary[species] > 0:
            print "... settled natives", species, "with", species_summary[species], "planets"


def log_specials_summary():
    print "Special Placement Summary"
    for special in specials_summary:
        if specials_summary[special] > 0:
            print "... placed special", special, specials_summary[special], "times"
