from freeorion_tools import get_ai_tag_grade
from pytest import mark


@mark.parametrize(
    "tags,tags_type",
    (
        (["GREAT_WEAPONS"], "WEAPONS"),
        (["GREAT_WEAPONS"], "weapons"),
        (["GOOD_HULLS", "GREAT_WEAPONS"], "WEAPONS"),
        (["GREAT_WEAPONS", "GOOD_WEAPONS"], "WEAPONS"),
    ),
)
def test_tags_search_by_tag_type_returns_grade(tags, tags_type):
    assert "GREAT" == get_ai_tag_grade(tags, tags_type)


def test_tags_search_by_tag_type_with_underscore_returns_first_grade():
    assert "GREAT" == get_ai_tag_grade(["GREAT_WEAPONS_TEST"], "WEAPONS_TEST")


def test_tags_search_by_not_matching_tag_type_returns_empty_string():
    assert "" == get_ai_tag_grade(["GOOD_HULL"], "WEAPONS")


def test_very_bad_tag_parsing_find_correct_modifier():
    assert "VERY_BAD" == get_ai_tag_grade(["VERY_BAD_WEAPONS"], "WEAPONS")


@mark.parametrize(
    "tags,tags_type",
    (
        (["GREAT_WEAPONS"], "WEAPON"),
        (["GREAT_WEAPONS"], "EAPONS"),
        (["GREAT_WEAPONS"], "EAPON"),
    ),
)
def test_tags_search_by_substring_tag_type_returns_empty_string(tags, tags_type):
    assert "" == get_ai_tag_grade(tags, tags_type)


@mark.parametrize(
    "tags,tags_type",
    (
        (["great_weapons"], "weapons"),
        (["great_weapons"], "WEAPONS"),
    ),
)
def test_lowercase_tags_search_by_tag_type_returns_empty_string(tags, tags_type):
    assert "" == get_ai_tag_grade(tags, tags_type)
