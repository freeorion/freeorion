"""
character_strings_module.py produces strings and lists of strings based on
Character for communicating with players and devs.

There are two types of public functions: functions that provide a readable name
of the level/type of a trait and functions that provide a list of strings to be
used as responses in the UI.

The functions of the type get_trait_name_X(character) return a user string
describing the nature of the trait X if the character has that trait or None if
the character does not have that trait.

For example:
get_trait_name_aggression(Character([Aggression(1), EmpireID(0,0)]])) returns "Turtle"
while
get_trait_name_aggression(Character([EmpireID(0,0)])) returns None

The functions of the type possible_X(character) return a list of strings based
on character suitable for the UI.

For example:
possible_capitals(Character([Aggression(0)])) returns ['Royal', 'Imperial'].
"""
from logging import debug

import character as character_package
import freeOrionAIInterface as fo  # pylint: disable=import-error


class _CharacterTableFunction(object):
    """A table indexed by a particular trait of a Character that is used like a
    function.

    When called this object will use the appropriate trait to lookup and
    possibly post process a line in the table.

    """

    def __init__(self, trait_class, table, post_process_func=None):
        """Store trait type, table and post processing function."""
        self.trait_class = trait_class
        self.table = table
        self.post_process = post_process_func

    def __repr__(self):
        return str(self.table)

    def __call__(self, character):
        """Return the indexed and post-processed string. Get the key from the correct trait in the character."""
        trait = character.get_trait(self.trait_class)
        if trait is None and None not in self.table:
            return None
        elem = self.table[trait.key] if trait is not None else self.table[None]
        if self.post_process:
            elem = self.post_process(elem)
        debug("CharacterTable returns %s", elem)
        return elem


_aggression_label_suffix = {fo.aggression.beginner: "_BEGINNER",
                            fo.aggression.turtle: "_TURTLE",
                            fo.aggression.cautious: "_CAUTIOUS",
                            fo.aggression.typical: "_TYPICAL",
                            fo.aggression.aggressive: "_AGGRESSIVE",
                            fo.aggression.maniacal: "_MANIACAL"}


def _make_aggression_based_function(prefix, post_process_func=None):
    """Make an aggression _CharacterTableFunction"""
    table = {key: "%s%s" % (prefix, suffix) for (key, suffix) in _aggression_label_suffix.items()}
    table[None] = "UNKNOWN_VALUE_SYMBOL"
    return _CharacterTableFunction(character_package.character_module.Aggression, table, post_process_func)


# Human readable trait name functions for debugging
get_trait_name_aggression = _make_aggression_based_function("GSETUP", fo.userString)

# Lists of strings for the UI based on character
possible_capitals = _make_aggression_based_function("AI_CAPITOL_NAMES", fo.userStringList)
possible_greetings = _make_aggression_based_function("AI_FIRST_TURN_GREETING_LIST", fo.userStringList)
