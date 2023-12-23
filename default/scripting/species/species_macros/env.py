# BROAD_EP and NARROW_EP are stub placeholders for applying the
# board/narrow environment tolerance description to a species.
from focs._effects import (
    Adequate,
    AsteroidsType,
    Barren,
    Desert,
    GasGiantType,
    Good,
    Hostile,
    Inferno,
    Ocean,
    Poor,
    Radiated,
    Swamp,
    Terran,
    Toxic,
    Tundra,
    Uninhabitable,
)
from macros.misc import DESCRIPTION_EFFECTSGROUP_MACRO

BROAD_EP = DESCRIPTION_EFFECTSGROUP_MACRO("BROAD_EP_DESC")

NARROW_EP = DESCRIPTION_EFFECTSGROUP_MACRO("NARROW_EP_DESC")

SWAMP_STANDARD_EP = {
    Swamp: Good,
    Toxic: Adequate,
    Inferno: Poor,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Poor,
    Ocean: Adequate,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TOXIC_STANDARD_EP = {
    Swamp: Adequate,
    Toxic: Good,
    Inferno: Adequate,
    Radiated: Poor,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Poor,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

INFERNO_STANDARD_EP = {
    Swamp: Poor,
    Toxic: Adequate,
    Inferno: Good,
    Radiated: Adequate,
    Barren: Poor,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

RADIATED_STANDARD_EP = {
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

BARREN_STANDARD_EP = {
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

TUNDRA_STANDARD_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Poor,
    Barren: Adequate,
    Tundra: Good,
    Desert: Adequate,
    Terran: Poor,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

DESERT_STANDARD_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Poor,
    Tundra: Adequate,
    Desert: Good,
    Terran: Adequate,
    Ocean: Poor,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TERRAN_STANDARD_EP = {
    Swamp: Poor,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Poor,
    Desert: Adequate,
    Terran: Good,
    Ocean: Adequate,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

OCEAN_STANDARD_EP = {
    Swamp: Adequate,
    Toxic: Poor,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Poor,
    Terran: Adequate,
    Ocean: Good,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

SWAMP_NARROW_EP = {
    Swamp: Good,
    Toxic: Poor,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Poor,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TOXIC_NARROW_EP = {
    Swamp: Poor,
    Toxic: Good,
    Inferno: Poor,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

INFERNO_NARROW_EP = {
    Swamp: Hostile,
    Toxic: Poor,
    Inferno: Good,
    Radiated: Poor,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

RADIATED_NARROW_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Poor,
    Radiated: Good,
    Barren: Poor,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

BARREN_NARROW_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Poor,
    Barren: Good,
    Tundra: Poor,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TUNDRA_NARROW_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Poor,
    Tundra: Good,
    Desert: Poor,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

DESERT_NARROW_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Poor,
    Desert: Good,
    Terran: Poor,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TERRAN_NARROW_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Poor,
    Terran: Good,
    Ocean: Poor,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

OCEAN_NARROW_EP = {
    Swamp: Poor,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Poor,
    Ocean: Good,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

SWAMP_BROAD_EP = {
    Swamp: Good,
    Toxic: Adequate,
    Inferno: Poor,
    Radiated: Poor,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Poor,
    Terran: Poor,
    Ocean: Adequate,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TOXIC_BROAD_EP = {
    Swamp: Adequate,
    Toxic: Good,
    Inferno: Adequate,
    Radiated: Poor,
    Barren: Poor,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Poor,
    Ocean: Poor,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

INFERNO_BROAD_EP = {
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

RADIATED_BROAD_EP = {
    Swamp: Poor,
    Toxic: Poor,
    Inferno: Adequate,
    Radiated: Good,
    Barren: Adequate,
    Tundra: Poor,
    Desert: Poor,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

BARREN_BROAD_EP = {
    Swamp: Hostile,
    Toxic: Poor,
    Inferno: Poor,
    Radiated: Adequate,
    Barren: Good,
    Tundra: Adequate,
    Desert: Poor,
    Terran: Poor,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TUNDRA_BROAD_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Poor,
    Radiated: Poor,
    Barren: Adequate,
    Tundra: Good,
    Desert: Adequate,
    Terran: Poor,
    Ocean: Poor,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

DESERT_BROAD_EP = {
    Swamp: Poor,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Poor,
    Barren: Poor,
    Tundra: Adequate,
    Desert: Good,
    Terran: Adequate,
    Ocean: Poor,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TERRAN_BROAD_EP = {
    Swamp: Poor,
    Toxic: Poor,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Poor,
    Tundra: Poor,
    Desert: Adequate,
    Terran: Good,
    Ocean: Adequate,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

OCEAN_BROAD_EP = {
    Swamp: Adequate,
    Toxic: Poor,
    Inferno: Poor,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Poor,
    Desert: Poor,
    Terran: Adequate,
    Ocean: Good,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

TOLERANT_EP = {
    Swamp: Good,
    Toxic: Good,
    Inferno: Good,
    Radiated: Good,
    Barren: Good,
    Tundra: Good,
    Desert: Good,
    Terran: Good,
    Ocean: Good,
    AsteroidsType: Uninhabitable,
    GasGiantType: Uninhabitable,
}

VERY_TOLERANT_EP = {
    Swamp: Good,
    Toxic: Good,
    Inferno: Good,
    Radiated: Good,
    Barren: Good,
    Tundra: Good,
    Desert: Good,
    Terran: Good,
    Ocean: Good,
    AsteroidsType: Adequate,
    GasGiantType: Adequate,
}

ASTEROIDAL_NARROW_EP = {
    Swamp: Uninhabitable,
    Toxic: Uninhabitable,
    Inferno: Uninhabitable,
    Radiated: Uninhabitable,
    Barren: Hostile,
    Tundra: Uninhabitable,
    Desert: Uninhabitable,
    Terran: Uninhabitable,
    Ocean: Uninhabitable,
    AsteroidsType: Adequate,
    GasGiantType: Uninhabitable,
}

ASTEROIDAL_STANDARD_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Poor,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Good,
    GasGiantType: Uninhabitable,
}

GASEOUS_NARROW_EP = {
    Swamp: Uninhabitable,
    Toxic: Uninhabitable,
    Inferno: Uninhabitable,
    Radiated: Uninhabitable,
    Barren: Uninhabitable,
    Tundra: Uninhabitable,
    Desert: Uninhabitable,
    Terran: Uninhabitable,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Adequate,
}

GASEOUS_STANDARD_EP = {
    Swamp: Hostile,
    Toxic: Hostile,
    Inferno: Hostile,
    Radiated: Hostile,
    Barren: Hostile,
    Tundra: Hostile,
    Desert: Hostile,
    Terran: Hostile,
    Ocean: Hostile,
    AsteroidsType: Uninhabitable,
    GasGiantType: Good,
}
