import pytest
from pytest import fixture
from savegame_codec import build_savegame_string, load_savegame_string
import aistate_interface
from savegame_codec._decoder import SaveDecompressException


@fixture
def state():
    return {1: "2"}


@fixture
def save_string(monkeypatch, state):
    def get_aistate():
        return state

    monkeypatch.setattr(aistate_interface, "get_aistate", get_aistate)
    return build_savegame_string().decode('utf-8')


def test_save_load_game_from_string(save_string, state):
    result = load_savegame_string(save_string)
    assert result == state


def test_save_load_game_from_bytes(save_string, state):
    result = load_savegame_string(save_string.encode("utf-8"))
    assert result == state


def test_decode_not_base64():
    with pytest.raises(SaveDecompressException, match="Fail to decode base64 savestate"):
        load_savegame_string("///")


def test_decode_not_gziped():
    with pytest.raises(SaveDecompressException, match="Fail to decompress savestate"):
        load_savegame_string("aaaa")


def test_decode_not_ascii():
    with pytest.raises(SaveDecompressException, match="Fail to decode base64 savestate"):
        load_savegame_string("шишка")
