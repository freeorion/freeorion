from macros.misc import DESCRIPTION_EFFECTSGROUP_MACRO

# BROAD_EP and NARROW_EP are stub placeholders for applying the
# board/narrow environment tolerance description to a species.

try:
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
except ModuleNotFoundError:
    pass

BROAD_EP = DESCRIPTION_EFFECTSGROUP_MACRO("BROAD_EP_DESC")

# NARROW_EP
# '''[[DESCRIPTION_EFFECTSGROUP_MACRO(NARROW_EP_DESC)]]'''

# SWAMP_STANDARD_EP
# '''environments = [
#       type = Swamp        environment = Good
#       type = Toxic        environment = Adequate
#       type = Inferno      environment = Poor
#       type = Radiated     environment = Hostile
#       type = Barren       environment = Hostile
#       type = Tundra       environment = Hostile
#       type = Desert       environment = Hostile
#       type = Terran       environment = Poor
#       type = Ocean        environment = Adequate
#       type = Asteroids    environment = Uninhabitable
#       type = GasGiant     environment = Uninhabitable
#     ]'''

# TOXIC_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Adequate
#         type = Toxic        environment = Good
#         type = Inferno      environment = Adequate
#         type = Radiated     environment = Poor
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Hostile
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Poor
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# INFERNO_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Poor
#         type = Toxic        environment = Adequate
#         type = Inferno      environment = Good
#         type = Radiated     environment = Adequate
#         type = Barren       environment = Poor
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Hostile
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# RADIATED_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Poor
#         type = Inferno      environment = Adequate
#         type = Radiated     environment = Good
#         type = Barren       environment = Adequate
#         type = Tundra       environment = Poor
#         type = Desert       environment = Hostile
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# BARREN_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Poor
#         type = Radiated     environment = Adequate
#         type = Barren       environment = Good
#         type = Tundra       environment = Adequate
#         type = Desert       environment = Poor
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# TUNDRA_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Poor
#         type = Barren       environment = Adequate
#         type = Tundra       environment = Good
#         type = Desert       environment = Adequate
#         type = Terran       environment = Poor
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# DESERT_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Poor
#         type = Tundra       environment = Adequate
#         type = Desert       environment = Good
#         type = Terran       environment = Adequate
#         type = Ocean        environment = Poor
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# TERRAN_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Poor
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Poor
#         type = Desert       environment = Adequate
#         type = Terran       environment = Good
#         type = Ocean        environment = Adequate
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# OCEAN_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Adequate
#         type = Toxic        environment = Poor
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Poor
#         type = Terran       environment = Adequate
#         type = Ocean        environment = Good
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# SWAMP_NARROW_EP
# '''environments = [
#       type = Swamp        environment = Good
#       type = Toxic        environment = Poor
#       type = Inferno      environment = Hostile
#       type = Radiated     environment = Hostile
#       type = Barren       environment = Hostile
#       type = Tundra       environment = Hostile
#       type = Desert       environment = Hostile
#       type = Terran       environment = Hostile
#       type = Ocean        environment = Poor
#       type = Asteroids    environment = Uninhabitable
#       type = GasGiant     environment = Uninhabitable
#     ]'''

# TOXIC_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Poor
#         type = Toxic        environment = Good
#         type = Inferno      environment = Poor
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Hostile
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# INFERNO_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Poor
#         type = Inferno      environment = Good
#         type = Radiated     environment = Poor
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Hostile
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# RADIATED_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Poor
#         type = Radiated     environment = Good
#         type = Barren       environment = Poor
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Hostile
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# BARREN_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Poor
#         type = Barren       environment = Good
#         type = Tundra       environment = Poor
#         type = Desert       environment = Hostile
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# TUNDRA_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Poor
#         type = Tundra       environment = Good
#         type = Desert       environment = Poor
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# DESERT_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Poor
#         type = Desert       environment = Good
#         type = Terran       environment = Poor
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# TERRAN_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Poor
#         type = Terran       environment = Good
#         type = Ocean        environment = Poor
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# OCEAN_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Poor
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Hostile
#         type = Terran       environment = Poor
#         type = Ocean        environment = Good
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# SWAMP_BROAD_EP
# '''environments = [
#       type = Swamp        environment = Good
#       type = Toxic        environment = Adequate
#       type = Inferno      environment = Poor
#       type = Radiated     environment = Poor
#       type = Barren       environment = Hostile
#       type = Tundra       environment = Hostile
#       type = Desert       environment = Poor
#       type = Terran       environment = Poor
#       type = Ocean        environment = Adequate
#       type = Asteroids    environment = Uninhabitable
#       type = GasGiant     environment = Uninhabitable
#     ]'''

# TOXIC_BROAD_EP
# '''environments = [
#         type = Swamp        environment = Adequate
#         type = Toxic        environment = Good
#         type = Inferno      environment = Adequate
#         type = Radiated     environment = Poor
#         type = Barren       environment = Poor
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Hostile
#         type = Terran       environment = Poor
#         type = Ocean        environment = Poor
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

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

# RADIATED_BROAD_EP
# '''environments = [
#         type = Swamp        environment = Poor
#         type = Toxic        environment = Poor
#         type = Inferno      environment = Adequate
#         type = Radiated     environment = Good
#         type = Barren       environment = Adequate
#         type = Tundra       environment = Poor
#         type = Desert       environment = Poor
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# BARREN_BROAD_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Poor
#         type = Inferno      environment = Poor
#         type = Radiated     environment = Adequate
#         type = Barren       environment = Good
#         type = Tundra       environment = Adequate
#         type = Desert       environment = Poor
#         type = Terran       environment = Poor
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# TUNDRA_BROAD_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Poor
#         type = Radiated     environment = Poor
#         type = Barren       environment = Adequate
#         type = Tundra       environment = Good
#         type = Desert       environment = Adequate
#         type = Terran       environment = Poor
#         type = Ocean        environment = Poor
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# DESERT_BROAD_EP
# '''environments = [
#         type = Swamp        environment = Poor
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Poor
#         type = Barren       environment = Poor
#         type = Tundra       environment = Adequate
#         type = Desert       environment = Good
#         type = Terran       environment = Adequate
#         type = Ocean        environment = Poor
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# TERRAN_BROAD_EP
# '''environments = [
#         type = Swamp        environment = Poor
#         type = Toxic        environment = Poor
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Poor
#         type = Tundra       environment = Poor
#         type = Desert       environment = Adequate
#         type = Terran       environment = Good
#         type = Ocean        environment = Adequate
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# OCEAN_BROAD_EP
# '''environments = [
#         type = Swamp        environment = Adequate
#         type = Toxic        environment = Poor
#         type = Inferno      environment = Poor
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Poor
#         type = Desert       environment = Poor
#         type = Terran       environment = Adequate
#         type = Ocean        environment = Good
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# TOLERANT_EP
# '''environments = [
#         type = Swamp        environment = Good
#         type = Toxic        environment = Good
#         type = Inferno      environment = Good
#         type = Radiated     environment = Good
#         type = Barren       environment = Good
#         type = Tundra       environment = Good
#         type = Desert       environment = Good
#         type = Terran       environment = Good
#         type = Ocean        environment = Good
#         type = Asteroids    environment = Uninhabitable
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# VERY_TOLERANT_EP
# '''environments = [
#         type = Swamp        environment = Good
#         type = Toxic        environment = Good
#         type = Inferno      environment = Good
#         type = Radiated     environment = Good
#         type = Barren       environment = Good
#         type = Tundra       environment = Good
#         type = Desert       environment = Good
#         type = Terran       environment = Good
#         type = Ocean        environment = Good
#         type = Asteroids    environment = Adequate
#         type = GasGiant     environment = Adequate
#     ]'''

# ASTEROIDAL_NARROW_EP
# '''environments = [
#         type = Swamp        environment = Uninhabitable
#         type = Toxic        environment = Uninhabitable
#         type = Inferno      environment = Uninhabitable
#         type = Radiated     environment = Uninhabitable
#         type = Barren       environment = Hostile
#         type = Tundra       environment = Uninhabitable
#         type = Desert       environment = Uninhabitable
#         type = Terran       environment = Uninhabitable
#         type = Ocean        environment = Uninhabitable
#         type = Asteroids    environment = Adequate
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# ASTEROIDAL_STANDARD_EP
# '''environments = [
#         type = Swamp        environment = Hostile
#         type = Toxic        environment = Hostile
#         type = Inferno      environment = Hostile
#         type = Radiated     environment = Hostile
#         type = Barren       environment = Poor
#         type = Tundra       environment = Hostile
#         type = Desert       environment = Hostile
#         type = Terran       environment = Hostile
#         type = Ocean        environment = Hostile
#         type = Asteroids    environment = Good
#         type = GasGiant     environment = Uninhabitable
#     ]'''

# GASEOUS_NARROW_EP
# '''environments = [
#       type = Swamp        environment = Uninhabitable
#       type = Toxic        environment = Uninhabitable
#       type = Inferno      environment = Uninhabitable
#       type = Radiated     environment = Uninhabitable
#       type = Barren       environment = Uninhabitable
#       type = Tundra       environment = Uninhabitable
#       type = Desert       environment = Uninhabitable
#       type = Terran       environment = Uninhabitable
#       type = Ocean        environment = Hostile
#       type = Asteroids    environment = Uninhabitable
#       type = GasGiant     environment = Adequate
#     ]'''

# GASEOUS_STANDARD_EP
# '''environments = [
#       type = Swamp        environment = Hostile
#       type = Toxic        environment = Hostile
#       type = Inferno      environment = Hostile
#       type = Radiated     environment = Hostile
#       type = Barren       environment = Hostile
#       type = Tundra       environment = Hostile
#       type = Desert       environment = Hostile
#       type = Terran       environment = Hostile
#       type = Ocean        environment = Hostile
#       type = Asteroids    environment = Uninhabitable
#       type = GasGiant     environment = Good
#     ]'''
