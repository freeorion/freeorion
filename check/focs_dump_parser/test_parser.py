from io import StringIO

from check.focs_dump_parser.parser import _get_name_from_line, parse_buildings

file_ = StringIO(
    """
00:00:34.002036 {0x00002980} [info] log : Logger.cpp:137 : Added logger named "conditions"
00:00:34.001036 {0x00002980} [trace] parsing : BuildingsParser.cpp:258 : Start parsing FOCS for BuildingTypes: 108
00:00:34.002036 {0x00002980} [debug] log : LoggerWithOptionsDB.cpp:108 : Configure log source "effects" from optionsDB using threshold debug
00:00:34.002036 {0x00002980} [trace] parsing : BuildingsParser.cpp:260 : BuildingType BLD_ABANDON_OUTPOST : 4243737
BuildingType
    name = "BLD_ABANDON_OUTPOST"
    description = "BLD_ABANDON_OUTPOST_DESC"
00:00:34.002036 {0x00002980} [trace] parsing : BuildingsParser.cpp:260 : BuildingType BLD_ART_BLACK_HOLE : 4128519
BuildingType
    name = "BLD_ART_BLACK_HOLE"
    description = "BLD_ART_BLACK_HOLE_DESC"
00:35:48.229253 {0x000031f0} [debug] ai : i18n.cpp:100 : Detected locale language: en
00:35:48.336232 {0x000031f0} [trace] parsing : BuildingsParser.cpp:263 : End parsing FOCS for BuildingTypes96
""".strip()
)


bld_abandon_outpost = """
BuildingType
    name = "BLD_ABANDON_OUTPOST"
    description = "BLD_ABANDON_OUTPOST_DESC"
""".lstrip()

bld_art_black_hole = """
BuildingType
    name = "BLD_ART_BLACK_HOLE"
    description = "BLD_ART_BLACK_HOLE_DESC"
""".lstrip()


def test_parse():
    assert parse_buildings(file_) == [
        ("BLD_ABANDON_OUTPOST", bld_abandon_outpost),
        ("BLD_ART_BLACK_HOLE", bld_art_black_hole),
    ]


def test_get_name_from_line():
    assert (
        _get_name_from_line(
            "00:00:34.002036 {0x00002980} [trace] parsing : BuildingsParser.cpp:260 : BuildingType BLD_ABANDON_OUTPOST : 4243737"
        )
        == "BLD_ABANDON_OUTPOST"
    )
    assert (
        _get_name_from_line(
            '00:00:34.002036 {0x00002980} [info] log : Logger.cpp:257 : Setting "conditions" logger threshold to "debug".'
        )
        is None
    )
