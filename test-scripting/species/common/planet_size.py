# HUGE_PLANET
# '''EffectsGroup
#         scope = Source
#         activation = And [
#             Planet
#             Turn high = 0
#         ]
#         effects = SetPlanetSize planetsize = Huge
# '''
try:
    from focs._effects import EffectsGroup, IsSource, Large, Planet, SetPlanetSize, Turn
except ModuleNotFoundError:
    pass


LARGE_PLANET = [
    EffectsGroup(scope=IsSource, activation=Planet() & Turn(high=0), effects=SetPlanetSize(planetsize=Large))
]

# MEDIUM_PLANET
# '''EffectsGroup
#         scope = Source
#         activation = And [
#             Planet
#             Turn high = 0
#         ]
#         effects = SetPlanetSize planetsize = Medium
# '''

# SMALL_PLANET
# '''EffectsGroup
#         scope = Source
#         activation = And [
#             Planet
#             Turn high = 0
#         ]
#         effects = SetPlanetSize planetsize = Small
# '''

# NOT_HUGE_PLANET
# '''EffectsGroup
#         scope = Source
#         activation = And [
#             Planet
#             Turn high = 0
#             Planet size = Huge
#         ]
#         effects = SetPlanetSize planetsize = Large
# '''

# NOT_LARGE_PLANET
# '''EffectsGroup
#         scope = Source
#         activation = And [
#             Planet
#             Turn high = 0
#             Not Planet size = Tiny
#             Not Planet size = Medium
#         ]
#         effects = SetPlanetSize planetsize = Small
# '''
