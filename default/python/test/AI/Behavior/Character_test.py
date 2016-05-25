import pytest
from Behavior.Character import Behavior, Character

class LeftBehavior(Behavior):
    """A test behavior"""

class RightBehavior(Behavior):
    """A test Behavior that injects values to be found by combiners"""

    def may_explore_system(self, monster_threat):  # pylint: disable=no-self-use,unused-argument
        """Always reject. 1 parameter"""
        return False

    def may_maximize_research(self):  # pylint: disable=no-self-use,unused-argument
        """Always reject. 0 parameters"""
        return False

    def preferred_research_cutoff(self, alternatives):  # pylint: disable=no-self-use,unused-argument
        """Pick 1st"""
        return alternatives[1]

    def max_number_colonies(self):  # pylint: disable=no-self-use,unused-argument
        """Use for min combiner"""
        return 100

    def military_safety_factor(self):  # pylint: disable=no-self-use,unused-argument
        """Use for max combiner"""
        return 10

class OtherBehavior(Behavior):
    """A test behavior"""


left_behavior = LeftBehavior()
right_behavior = RightBehavior()
other_behavior = OtherBehavior()

rejection_character = Character([left_behavior, right_behavior])
permissive_character = Character([left_behavior, other_behavior])

class TestCharacter(object):
    """Test the Character class which combines behaviors
    Each combiner test checks that the combiner generates the expected output.
    """

    def test_get_behavior(self):
        assert rejection_character.get_behavior(LeftBehavior) == left_behavior
        assert rejection_character.get_behavior(RightBehavior) == right_behavior
        assert rejection_character.get_behavior(OtherBehavior) != left_behavior
        assert rejection_character.get_behavior(OtherBehavior) != right_behavior
        assert isinstance(rejection_character.get_behavior(OtherBehavior), Behavior)

    def test_all_combiner(self):
        assert permissive_character.may_maximize_research()
        assert not rejection_character.may_maximize_research()

    def test_all_combiner_one_parameter(self):
        assert permissive_character.may_explore_system(0)
        assert not rejection_character.may_explore_system(0)

    def test_min_combiner(self):
        assert permissive_character.max_number_colonies() == 1000000
        assert rejection_character.max_number_colonies() == 100

    def test_max_combiner(self):
        assert permissive_character.military_safety_factor() == 0
        assert rejection_character.military_safety_factor() == 10

    def test_preference_combiner(self):
        assert permissive_character.preferred_research_cutoff([10, 11, 12]) == None
        assert rejection_character.preferred_research_cutoff([10, 11, 12]) == 11

    def test_character_must_be_composed_of_behaviors(self):
        with pytest.raises(TypeError):
            not_a_behavior = int(1)
            bad_character = Character([LeftBehavior, not_a_behavior])
