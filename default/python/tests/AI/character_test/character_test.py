import pytest
from character.character_module import Trait, Character


class LeftTrait(Trait):
    """A test trait"""


class RightTrait(Trait):
    """A test Trait that injects values to be found by combiners"""

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


class OtherTrait(Trait):
    """A test trait"""


left_trait = LeftTrait()
right_trait = RightTrait()
other_trait = OtherTrait()

rejection_character = Character([left_trait, right_trait])
permissive_character = Character([left_trait, other_trait])


class TestCharacter(object):
    """Test the Character class which combines traits
    Each combiner test checks that the combiner generates the expected output.
    """

    def test_get_trait(self):
        assert rejection_character.get_trait(LeftTrait) == left_trait
        assert rejection_character.get_trait(RightTrait) == right_trait
        assert rejection_character.get_trait(OtherTrait) != left_trait
        assert rejection_character.get_trait(OtherTrait) != right_trait
        assert rejection_character.get_trait(OtherTrait) is None

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
        assert permissive_character.preferred_research_cutoff([10, 11, 12]) is None
        assert rejection_character.preferred_research_cutoff([10, 11, 12]) == 11

    def test_character_must_be_composed_of_traits(self):
        with pytest.raises(TypeError, message='Expect TypeError', match='All traits must be sub-classes of Trait'):
            not_a_trait = int(1)
            Character([LeftTrait, not_a_trait])
