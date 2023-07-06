# This module should be imported with *, that means we should not import other things in it, otherwise things that should be imported from other modules will be imported from here.

def Species(
    *,
    name: str,
    description: str,
    gameplay_description: str,
    tags: list[str],
    foci: list,
    defaultfocus: str,
    effectsgroups: list,
    graphic: str,
    environments: dict = {},
    likes: list[str] = [],
    dislikes: list[str] = [],
    can_produce_ships: bool = False,
    playable: bool = False,
    can_colonize: bool = False,
    native: bool = False,
    annexation_condition=None,
): ...
