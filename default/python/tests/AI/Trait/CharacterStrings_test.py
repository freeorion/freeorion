from Trait.Character import Trait, Character, Aggression
from Trait.CharacterStrings import make_aggression_table


def test_make_aggression_table():
    """Check that the table adds the correct prefixes, suffixes, runs the postfix function,
    has the UNKNOWN value entry and is the correct length.
    """
    table = make_aggression_table("prefix", lambda x: "post_proc_%s" % x)

    character = Character([Aggression(0)])
    assert table[character] == "post_proc_prefix_BEGINNER"

    character = Character([Aggression(5)])
    assert table[character] == "post_proc_prefix_MANIACAL"

    character = Character([Trait()])
    assert table[character] == "post_proc_UNKNOWN_VALUE_SYMBOL"

    assert len(table) == 7


def test_make_aggression_table_no_post_func():
    """Check that the table doesn't run the post function if it doesn't exist"""
    table = make_aggression_table("prefix")

    character = Character([Aggression(0)])
    assert table[character] == "prefix_BEGINNER"
