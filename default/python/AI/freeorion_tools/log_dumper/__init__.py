from logging import debug

from common.dump_interface import DumpKey
from freeorion_tools.log_dumper.dumper import Dumper, DumpValue


class DumpWrapper:
    def __init__(self):
        self._dumper = Dumper(writer=debug)

    def _dump(self, key, val):
        self._dumper.dump(key, val)

    def ship_count(self, ship_count: int):
        self._dump(DumpKey.SHIP_CONT, ship_count)

    def capital(self, planet_id, planet_name, species):

        self._dump(
            DumpKey.CapitalID,
            {"capital_planet_id": planet_id, "capital_planet_name": planet_name, "capital_species": species},
        )

    def empire(self, id_, name, turn):

        return self._dump(DumpKey.EmpireID, {"empire_id": id_, "name": name, "turn": turn})

    def empire_color(self, R, G, B, A):
        return self._dump(DumpKey.EmpireColors, (R, G, B, A))

    def output(self, turn, rp, pp):
        self._dump(DumpKey.Output, f"turn: {turn:4d} RP: {rp:.1f} PP: {pp:.1f}")


dump = DumpWrapper()
