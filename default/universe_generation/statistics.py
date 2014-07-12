import foUniverseGenerator as fo
import planets


def log_planet_count_dist(sys_list):
    planet_count_dist = {}
    for system in sys_list:
        planet_count = 0
        for planet in fo.sys_get_planets(system):
            if fo.planet_get_size(planet) in planets.planet_sizes:
                planet_count += 1
        planet_count_dist.setdefault(planet_count, [0])[0] += 1
    print "\n Planet Count Distribution: planets_in_system | num_systems"
    for planet_count, sys_count in planet_count_dist.items():
        print "\t\t\t%2d  | %5d" % (planet_count, sys_count[0])
    print


def log_planet_type_summary(sys_list):
    planet_type_summary = {k: 0 for k in planets.planet_types}
    for system in sys_list:
        for planet in fo.sys_get_planets(system):
            planet_type_summary[fo.planet_get_type(planet)] += 1
    planet_total = sum(planet_type_summary.values())
    print "\nPlanet Type Summary for a total of %d placed planets" % planet_total
    for planet_type, planet_count in planet_type_summary.items():
        print "%-12s %.1f%%" % (planet_type.name, 100.0 * planet_count / planet_total)
    print
