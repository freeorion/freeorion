from logging import warning

from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()

import freeorion as fo

from operator import itemgetter
from empires import home_system_layout


def home_system_team_core(home_systems, teams):
    """
    Choose core for teams
    Returns map from team to core home system
    """
    print("Teams: ", teams)
    # sort all home systems by distance
    home_systems_distances = {}
    for hs1 in home_systems:
        for hs2 in home_systems:
            if hs1 < hs2:
                dist = fo.jump_distance(hs1, hs2)
                home_systems_distances[(hs1, hs2)] = dist
    home_systems_sorted = sorted(home_systems_distances.items(), key=itemgetter(1), reverse=True)
    print("Home systems sorted: ", home_systems_sorted)

    result = {}
    if len(teams) == 0 or len(home_systems_sorted) == 0:
        pass
    elif len(teams) == 1:
        result[teams[0][0]] = home_systems_sorted[0][0][0]
    else:
        result[teams[0][0]] = home_systems_sorted[0][0][0]
        result[teams[1][0]] = home_systems_sorted[0][0][1]
        if len(teams) > 2:
            warning("Teamed placement poorly implemented for ", len(teams), " teams")
    return result


def place_teams_layout(layout, cores, placement_teams):
    """
    Place teams on home systems layout
    Returns map from home system to team
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
                choose_hs = set([hs])
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
                    choose_hs2 = set([hs])
                elif dist_to_enemy == max_dist_to_enemy:
                    choose_hs2.add(hs)
            choose_hs = choose_hs2

        if len(choose_hs) > 0:
            hs = choose_hs.pop()
            result[hs] = team
            left_home_systems.remove(hs)
    return result.items()


def place_teams(home_systems, systems, teams):
    """
    Place teams on home systems layout
    Returns map from home system to team
    """
    # choose team's core home system
    home_system_teams = home_system_team_core(home_systems, sorted(teams.items(), key=itemgetter(1), reverse=True))
    print("Home systems team core: ", home_system_teams)
    # choose order of placing teams exclude already placed cores
    placement_list = []
    for team in home_system_teams.keys():
        placement_list += [(i, team) for i in range(teams[team] - (1 if team in home_system_teams.keys() else 0))]
    placement_list.sort(reverse=True)
    placement_teams = [i[1] for i in placement_list]
    print("Placement teams: ", placement_teams)
    # calculate placement based on teams
    layout = home_system_layout(home_systems, systems)
    print("Home systems layout: ", layout)
    return place_teams_layout(layout, home_system_teams, placement_teams)
