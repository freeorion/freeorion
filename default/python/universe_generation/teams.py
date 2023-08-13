from logging import debug, warning

from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()

import freeorion as fo
from operator import itemgetter

from empires import home_system_layout
from util import unique_product


def home_system_team_core(home_systems: list[int], teams: list[tuple[int, int]]) -> dict[int, int]:
    """
    Choose core for teams which is a list of pairs team id and count of empires in the team.
    Returns map from team to core home system.
    """
    if not teams:
        return {}
    debug("Teams: %s", teams)
    # sort all home systems by distance
    home_systems_distances = {}
    for hs1, hs2 in unique_product(home_systems, home_systems):
        dist = fo.jump_distance(hs1, hs2)
        home_systems_distances[(hs1, hs2)] = dist
    home_systems_sorted = sorted(home_systems_distances.items(), key=itemgetter(1), reverse=True)
    debug("Home systems sorted: %s", home_systems_sorted)

    result = {}
    if not home_systems_sorted:
        pass
    elif len(teams) == 1:
        first_team = teams[0][0]
        first_of_most_distant_systems = home_systems_sorted[0][0][0]
        result[first_team] = first_of_most_distant_systems
    else:
        first_team = teams[0][0]
        first_of_most_distant_systems = home_systems_sorted[0][0][0]
        second_team = teams[1][0]
        second_of_most_distant_systems = home_systems_sorted[0][0][1]
        result[first_team] = first_of_most_distant_systems
        result[second_team] = second_of_most_distant_systems
        if len(teams) > 2:
            warning("Teamed placement poorly implemented for %d teams", len(teams))
    return result


def place_teams_layout(  # noqa: C901
    layout: dict[int, list[int]], cores: dict[int, int], placement_teams: list[int]
) -> dict[int, int]:
    """
    Place teams on home systems layout.
    Returns map from home system to team.
    """
    # set team cores
    left_home_systems = set(layout.keys())
    result = {}
    for team, hs in cores.items():
        result[hs] = team
        left_home_systems.remove(hs)
    # for each team search for home system
    for team in placement_teams:
        # search such home system
        # 1. with maximum same team neighbors
        choose_hs = set()
        neighbors_count = None
        for hs in left_home_systems:
            cnt = len([n for n in layout[hs] if result.get(n, -1) == team])
            if neighbors_count is None or cnt > neighbors_count:
                choose_hs = {hs}
                neighbors_count = cnt
            elif cnt == neighbors_count:
                choose_hs.add(hs)

        # 2. with maximum jump distance from other teams
        if len(choose_hs) > 1:
            choose_hs2 = set()
            max_dist_to_enemy = None
            for hs in choose_hs:
                dist_to_enemy = None
                for hs2, team2 in result.items():
                    if team2 != team:
                        dist = fo.jump_distance(hs, hs2)
                        if dist_to_enemy is None or dist < dist_to_enemy:
                            dist_to_enemy = dist
                if max_dist_to_enemy is None or dist_to_enemy > max_dist_to_enemy:
                    max_dist_to_enemy = dist_to_enemy
                    choose_hs2 = {hs}
                elif dist_to_enemy == max_dist_to_enemy:
                    choose_hs2.add(hs)
            choose_hs = choose_hs2

        if choose_hs:
            hs = choose_hs.pop()
            result[hs] = team
            left_home_systems.remove(hs)
    return result.items()


def place_teams(home_systems: list[int], systems: list[int], teams: dict[int, int]) -> dict[int, int]:
    """
    Place teams on home systems layout.
    Returns map from home system to team.
    """
    # choose team's core home system
    home_system_teams = home_system_team_core(home_systems, sorted(teams.items(), key=itemgetter(1), reverse=True))
    debug("Home systems team core: %s", home_system_teams)
    # choose order of placing teams exclude already placed cores
    placement_list = []
    for team in home_system_teams.keys():
        placement_list += [(i, team) for i in range(teams[team] - (1 if team in home_system_teams.keys() else 0))]
    placement_list.sort(reverse=True)
    placement_teams = [i[1] for i in placement_list]
    debug("Placement teams: %s", placement_teams)
    # calculate placement based on teams
    layout = home_system_layout(home_systems, systems)
    debug("Home systems layout: %s", layout)
    return place_teams_layout(layout, home_system_teams, placement_teams)
