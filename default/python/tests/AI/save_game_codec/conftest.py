import aistate_interface
import pytest
from savegame_codec import build_savegame_string


@pytest.fixture()
def pack_object(monkeypatch):
    def do_decoding(obj):
        def get_aistate():
            return obj

        monkeypatch.setattr(aistate_interface, "get_aistate", get_aistate)
        return build_savegame_string().decode("utf-8")

    return do_decoding
