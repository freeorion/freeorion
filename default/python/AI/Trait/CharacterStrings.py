"""
CharacterStrings.py contains tables of strings indexed by the AI character
"""

import Trait.Character
import freeOrionAIInterface as fo  # pylint: disable=import-error


class CharacterTable(object):
    """A table indexed by a particular behavior of a Character."""

    def __init__(self, behavior_class, table, post_process_func=None):
        """Store behavior type, table and post processing function."""
        self.behavior_class = behavior_class
        self.table = table
        if None not in self.table:
            table[None] = "UNKNOWN_VALUE_SYMBOL"
        self.post_process = post_process_func

    def __repr__(self):
        return str(self.table)

    def __len__(self):
        return len(self.table)

    def __getitem__(self, character):
        """Return the indexed and post-processed string. Get the key from the correct behavior in the character."""
        elem = self.table[character.get_behavior(self.behavior_class).key]
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


def make_aggression_table(prefix, post_process_func=None):
    """Make an aggression CharacterTable"""
    table = {key: "%s%s" % (prefix, suffix) for (key, suffix) in _aggression_label_suffix.items()}
    table[None] = "UNKNOWN_VALUE_SYMBOL"
    return CharacterTable(Trait.Character.Aggression, table, post_process_func)

aggression_name = make_aggression_table("GSETUP", fo.userString)
aggression_capitals = make_aggression_table("AI_CAPITOL_NAMES", fo.userStringList)
aggression_greetings = make_aggression_table("AI_FIRST_TURN_GREETING_LIST", fo.userStringList)
