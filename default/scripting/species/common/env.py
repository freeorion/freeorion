from common.misc import DESCRIPTION_EFFECTSGROUP_MACRO

# BROAD_EP and NARROW_EP are stub placeholders for applying the
# board/narrow environment tolerance description to a species.
BROAD_EP = DESCRIPTION_EFFECTSGROUP_MACRO("BROAD_EP_DESC")

NARROW_EP = DESCRIPTION_EFFECTSGROUP_MACRO("NARROW_EP_DESC")

INFERNO_BROAD_EP = {
    "environments": {
        Swamp: Poor,
        Toxic: Adequate,
        Inferno: Good,
        Radiated: Adequate,
        Barren: Poor,
        Tundra: Poor,
        Desert: Hostile,
        Terran: Hostile,
        Ocean: Poor,
        AsteroidsType: Uninhabitable,
        GasGiantType: Uninhabitable,
    }
}

RADIATED_STANDARD_EP = {
    "environments": {
        Swamp: Hostile,
        Toxic: Poor,
        Inferno: Adequate,
        Radiated: Good,
        Barren: Adequate,
        Tundra: Poor,
        Desert: Hostile,
        Terran: Hostile,
        Ocean: Hostile,
        AsteroidsType: Uninhabitable,
        GasGiantType: Uninhabitable,
    }
}

BARREN_STANDARD_EP = {
    "environments": {
        Swamp: Hostile,
        Toxic: Hostile,
        Inferno: Poor,
        Radiated: Adequate,
        Barren: Good,
        Tundra: Adequate,
        Desert: Poor,
        Terran: Hostile,
        Ocean: Hostile,
        AsteroidsType: Uninhabitable,
        GasGiantType: Uninhabitable,
    }
}
