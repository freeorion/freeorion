"""
character_strings_module.py contains tables of strings indexed by the AI character
"""

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
        if None not in self.table:
            table[None] = "UNKNOWN_VALUE_SYMBOL"
        self.post_process = post_process_func

    def __repr__(self):
        return str(self.table)

    def __call__(self, character):
        """Return the indexed and post-processed string. Get the key from the correct trait in the character."""
        elem = self.table[character.get_trait(self.trait_class).key]
        if self.post_process:
            elem = self.post_process(elem)
        print "CharacterTable returns ", elem
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
