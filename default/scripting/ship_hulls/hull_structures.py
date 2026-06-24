# Base values for specific hulls for sharing values
# Note: _BASE_STRUCTURE are a workaround for getting structure directly from hulls
#      _GROWTH_STRUCTURE and _BASE_STRUCTURE are before base values before structure scaling


class HullConfig:
    BASE_STRUCTURE: int
    GROWTH_STRUCTURE: int
    GROWTH_TURNS: int

    def __init__(self, base: int, growth: int, turns: int):
        self.BASE_STRUCTURE = base
        self.GROWTH_STRUCTURE = growth
        self.GROWTH_TURNS = turns


HULL_STRUCTURES: dict[str, HullConfig] = {
    "SH_BIOADAPTIVE": HullConfig(10, 30, 60),
    "SH_ENDOMORPHIC": HullConfig(5, 15, 30),
    "SH_ENDOSYMBIOTIC": HullConfig(5, 15, 30),
    "SH_ORGANIC": HullConfig(3, 3, 25),
    "SH_PROTOPLASMIC": HullConfig(5, 25, 50),
    "SH_RAVENOUS": HullConfig(10, 36, 48),
    "SH_SENTIENT": HullConfig(12, 60, 60),
    "SH_SYMBIOTIC": HullConfig(4, 16, 40),
}
