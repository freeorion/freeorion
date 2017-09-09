from character.character_module import Trait, Character, Aggression
from character.character_strings_module import _make_aggression_based_function, get_trait_name_aggression


def test_make_aggression_based_function():
    """Check that the table adds the correct prefixes, suffixes, runs the postfix function,
    has the UNKNOWN value entry and is the correct length.
    """
    table = _make_aggression_based_function("prefix", lambda x: "post_proc_%s" % x)

    character = Character([Aggression(0)])
    assert table(character) == "post_proc_prefix_BEGINNER"

    character = Character([Aggression(5)])
    assert table(character) == "post_proc_prefix_MANIACAL"

    character = Character([Trait()])
    assert table(character) == "post_proc_UNKNOWN_VALUE_SYMBOL"


def test_make_aggression_based_function_no_post_func():
    """Check that the table doesn't run the post function if it doesn't exist"""
    table = _make_aggression_based_function("prefix")

    character = Character([Aggression(0)])
    assert table(character) == "prefix_BEGINNER"


class UnknownTrait(Trait):
    pass


def test_get_trait_name_aggression():
    """Check that asking for an unknown trait returns None"""

    character = Character([Aggression(1)])
    assert get_trait_name_aggression(character) == "UserString GSETUP_TURTLE"

    nonaggressive_character = Character([UnknownTrait()])
    assert get_trait_name_aggression(nonaggressive_character) == "UserString UNKNOWN_VALUE_SYMBOL"
