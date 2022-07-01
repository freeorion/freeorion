from logging import debug

from common.statistic_interface import StatKey
from freeorion_tools.statistics.statistic import Stats


class StatsWrapper:
    def __init__(self):
        self._stats_writer = Stats(writer=debug)

    def _write(self, key, val):
        self._stats_writer.write(key, val)

    def ship_count(self, ship_count: int):
        self._write(StatKey.SHIP_CONT, ship_count)

    def capital(self, planet_id, planet_name, species):
        self._write(
            StatKey.CapitalID,
            {"capital_planet_id": planet_id, "capital_planet_name": planet_name, "capital_species": species},
        )

    def empire(self, id_, name, turn):
        return self._write(StatKey.EmpireID, {"empire_id": id_, "name": name, "turn": turn})

    def empire_color(self, r, g, b, a):
        return self._write(
            StatKey.EmpireColors,
            {
                "R": r,
                "G": g,
                "B": b,
                "A": a,
            },
        )

    def output(self, turn, rp, pp):
        self._write(StatKey.Output, {"turn": turn, "RP": rp, "PP": pp})

    def adopt_policy(self, name):
        self._write(StatKey.PolicyAdoption, name)

    def deadopt_policy(self, name):
        self._write(StatKey.PolicyDeAdoption, name)


stats = StatsWrapper()
