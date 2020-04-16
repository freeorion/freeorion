import pytest
from savegame_codec import build_savegame_string, load_savegame_string
import aistate_interface
from savegame_codec._decoder import SaveDecompressException


def test_save_load_game(monkeypatch):
    state = {1: "2"}

    def get_aistate():
        return state

    monkeypatch.setattr(aistate_interface, "get_aistate", get_aistate)

    save_game = build_savegame_string().decode('utf-8')
    result = load_savegame_string(save_game)
    assert result == state


def test_decode_not_base64():
    with pytest.raises(SaveDecompressException, match="Fail to decode base64 savestate"):
        load_savegame_string("///")


def test_decode_not_gziped():
    with pytest.raises(SaveDecompressException, match="Fail to ungzip savestate"):
        load_savegame_string("aaaa")
