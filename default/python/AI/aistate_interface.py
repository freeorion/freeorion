"""
This module provides an interface for a quasi-singleton AIstate instance.

The AIstate must be initialized after module imports using either
   * create_new_aistate
   * load_aistate
This should happen only once per session and be triggered by the game client.

Access to the AIstate is provided via get_aistate().

Example usage:
    from aistate_interface import get_aistate()
    get_aistate().get_all_fleet_missions()

"""
_aistate = None


def create_new_aistate(*args, **kwargs):
    import AIstate
    global _aistate
    _aistate = AIstate.AIstate(*args, **kwargs)
    return _aistate


def load_aistate(savegame_string):
    import savegame_codec
    global _aistate
    _aistate = savegame_codec.load_savegame_string(savegame_string)
    return _aistate


def get_aistate():
    """
    :rtype: AIstate.AIstate
    """
    return _aistate
