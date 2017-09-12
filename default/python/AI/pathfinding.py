from heapq import heappush, heappop
from collections import namedtuple

import freeOrionAIInterface as fo


def find_path_with_resupply(start, target, fleet_id):
    path_information = namedtuple('path_information', ['distance', 'fuel', 'path'])
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    fleet = universe.getFleet(fleet_id)
    supplied_systems = set(fo.getEmpire().fleetSupplyableSystemIDs)

    # initialize data structures
    start_fuel = fleet.maxFuel if start in supplied_systems else fleet.fuel
    path_cache = {}
    queue = [(path_information(distance=0, fuel=start_fuel, path=tuple()), start)]

    while queue:
        # get next system u with path information
        (path_info, u) = heappop(queue)

        # did we reach the target?
        if u == target:
            return path_info

        # add information about how we reached here to the cache
        path_cache.setdefault(u, []).append(path_info)

        # check if we have enough fuel to travel to neighbors
        if path_info.fuel < 1:
            continue

        # add neighboring systems to the queue if the resulting path
        # is either shorter or offers more fuel than the other paths
        # which we already found to those systems
        for neighbor in universe.getImmediateNeighbors(u, empire_id):
            new_dist = path_info.distance + universe.linearDistance(u, neighbor)
            new_fuel = fleet.maxFuel if neighbor in supplied_systems else path_info.fuel - 1
            if all((new_dist < dist or new_fuel > fuel) for dist, fuel, _ in path_cache.get(neighbor, [])):
                heappush(queue, (path_information(new_dist, new_fuel, (path_info.path, u)), neighbor))

    # no path exists, not even if we refuel on the way
    return None


def run(s1, s2):
    universe = fo.getUniverse()
    fleets = universe.fleetIDs
    f = [fid for fid in universe.getSystem(126).fleetIDs][0]
    if not (s1 and s2):
        ss = universe.systemIDs
        s1 = ss[0]
        s2 = ss[1]
    print find_path_with_resupply(s1, s2, f)
