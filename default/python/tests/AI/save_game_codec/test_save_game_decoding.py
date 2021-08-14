from collections import OrderedDict

import pytest
from ColonisationAI import OrbitalColonizationManager
from EnumsAI import MissionType
from savegame_codec import CanNotSaveGameException, load_savegame_string
from savegame_codec._decoder import SaveDecompressException


@pytest.fixture()
def patch_orb_col_management(monkeypatch):
    def eq(self, other):
        self_tuple = self._colonization_plans, self.num_enqueued_bases
        other_tuple = other._colonization_plans, other.num_enqueued_bases
        return self_tuple == other_tuple

    monkeypatch.setattr(OrbitalColonizationManager, "__eq__", eq)


@pytest.fixture(
    params=[
        True,
        False,
        None,
        1,
        1.0,
        tuple(),
        tuple("abc"),
        ("1", ("2", ("3",))),
        "hello 'world'",
        [],
        [1, 2, 3],
        {},
        {1: 2, 3: 4},
        OrderedDict({}),
        OrderedDict({1: 2}),
        set(),
        {1, 2, 3},
        MissionType.COLONISATION,
        OrbitalColonizationManager(),
        [[[OrbitalColonizationManager(), MissionType.COLONISATION, 1], "2", 3.0, {1: 2}]],
        {1: {2: {3: OrbitalColonizationManager()}}},
    ]
)
def serialized_object(request, pack_object):
    obj = request.param
    yield obj, pack_object(obj)


def test_inherited_trusted_object_is_not_encoded(pack_object):
    class Inherited(OrbitalColonizationManager):
        pass

    with pytest.raises(CanNotSaveGameException, match="Class .*.Inherited is not trusted"):
        pack_object(Inherited())


@pytest.mark.parametrize("obj", ['"', "\\", "\n", "\t"])
def test_string_with_sepcial_caracters_encoded_and_decoded_back(obj, pack_object):
    encoded = pack_object(obj)
    assert obj == load_savegame_string(encoded)


@pytest.mark.usefixtures("patch_orb_col_management")
def test_string_decode_to_object(serialized_object):
    obj, serialized_string = serialized_object
    assert obj == load_savegame_string(serialized_string)


@pytest.mark.usefixtures("patch_orb_col_management")
def test_bytes_decode_to_object(serialized_object):
    obj, serialized_bytes = serialized_object
    assert obj == load_savegame_string(serialized_bytes.encode("utf-8"))


def test_not_base64_decode_raises_error():
    with pytest.raises(SaveDecompressException, match="Fail to decode base64 savestate"):
        load_savegame_string("///")


def test_not_gziped_decode_raises_error():
    with pytest.raises(SaveDecompressException, match="Fail to decompress savestate"):
        load_savegame_string("aaaa")


def test_not_unicode_decode_raises_error():
    with pytest.raises(SaveDecompressException, match="Fail to decode base64 savestate"):
        load_savegame_string("шишка")
