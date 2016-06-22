"""
Character represents the personalization of the AI.

Character is composed of orthogonal elements called Behavior.  Behavior
elements are orthogonal, which means that they do no interact and can be
freely mixed and matched to create a Character.  Orthogonality means
that they can be tested independently and combined with simple
combiners.

Character creates big changes in the behavior of the AI that are visible
to the player as personality.  At key points the AI checks the character
for permission 'may_<do something>' or preference of a series of
alternative actions 'prefer_<somethings>'.  Preference could be expanded
to look at all of the options and remove those that the behavior
forbids.

Character should be invoked by other modules at key behavioral points to
provide direction not optimization.  For example which empire/species do
I love/hate enough to attack.

The gated behaviors should be big user discernable changes, not small
coefficient optimizations better handled with a local optimization
routine. If there are setup options to the behavior they should fall
into 2 (on/off) or 3 settings.  For example,
deceitful/typical/trustworthy.  The idea is not to create deep, subtle,
nuanced personalities, but writ large Shakespearean characters that draw the
player into the narrative structure of the particular game that they are playing.

Each AI will get the mandatory behavior (probably Difficulty/Challenge)
and a selection of the optional behaviors.

The initial implementation was to pull aggression and empire.id modulo 2
or 3 optional portions of the code into two behaviors: Aggression and
EmpireID.  These are merely refactoring of existing code and are not
examples of ideal implementation.

Aggression implements two different behaviors as one: difficulty and
aggression.  It creates too many decision points not clearly related to
either concept.  Aggression should be broken into two behaviors:
difficulty/challenge and aggression.

EmpireID selects based on empire.id with is an implementation detail and
not a "visible" game mechanic.  EmpireID is also used to select between
coefficient options, which look like programmer experiments.  EmpireID
should be removed as a behavior.

Some ideas for future behavior modules are:
challenge/difficulty    -- to replace the difficulty related portions of
                           aggression.

research bent -- models an obsession/focus/repulsion by a particular
area of research. i.e. Shields are for grues.

nemesis/single-minded -- model a focus on a single opponent, perhaps
the first one to attack the AI.  This plays well with CharacterStrings
which can be used to inform all players who care that "I will hunt the
Dominion to the ends of the galaxy."  Humans and eventually AIs could
use this to modify their risk assessment of this AI.

deceitful/trustworthy -- Only useful after treaties are implemented to
determine if this AI will abide by the treaty

friendly and loyal -- Even without treaties this AI will assist their
friends.

vengeful -- An eye for eye.  AI generally counter attacks until things
are made right or even.

risk averse/gambler -- How much overkill does the AI require?  How risky
a research strategy will it attempt?  How many redundant buildings does
the AI build?

warlike/peaceful -- Will the AI prefer colonization to war?

cooperative/loner -- Once cooperative treaties (joint attack, sharing
system) are implemented, will the AI use them?

taciturn/chatty -- How often does the AI chat?

genocidal -- Will the AI research and use Concentration Camps/bombard
weapons/planet and star destroying weapons?  Is the AI horrified by the
use of such weapons?  Will the AI band together with other like minded
empires to protect the galaxy?

per playable species -- There is no need for per species behavior.  Each
playable species can be assigned some additional mandatory behavior(s).
For example the Trith and a mandatory Genocidal behavior.  Perhaps add
a probabilty distribution of behavior components to the FOCS description
of a playable species.
"""

import abc
from collections import defaultdict

import freeOrionAIInterface as fo  # pylint: disable=import-error


class Behavior(object):
    """An abstract class representing a type of behavior of the AI.

    Behaviors give the AI personality along some dimension.
    Behaviors do not help the AI make optimal decisions, they determine whether
    certain actions are permissible or preferable.

    The initial behavior is aggression which models both aggression and level of difficulty.
    """
    __metaclass__ = abc.ABCMeta

    def __repr__(self):
        return "Behavior"

    # @abc.abstractproperty
    @property
    def key(self):  # pylint: disable=no-self-use,unused-argument
        """Return an key for look up tables."""
        return None

    def may_explore_system(self, monster_threat):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to explore system given monster threat."""
        return True

    def may_surge_industry(self, totalPP, totalRP):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to surge industry."""
        return True

    def may_maximize_research(self):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to maximize research."""
        return True

    def preferred_research_cutoff(self, alternatives):  # pylint: disable=no-self-use,unused-argument
        """Return preferred research cutoff."""
        return None

    def max_number_colonies(self):  # pylint: disable=no-self-use,unused-argument
        """Return maximum allowed number of colonies"""
        return 1000000

    def may_invade(self):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to invade."""
        return True

    def may_invade_with_bases(self):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to invade with bases."""
        return True

    def invasion_priority_scaling(self):  # pylint: disable=no-self-use,unused-argument
        return 1.0

    def military_priority_scaling(self):  # pylint: disable=no-self-use,unused-argument
        return 1.0

    def preferred_colonization_portion(self, alternatives):  # pylint: disable=no-self-use,unused-argument
        """Select a fraction less than 1"""
        return None

    def preferred_outpost_portion(self, alternatives):  # pylint: disable=no-self-use,unused-argument
        """Select a fraction less than 1"""
        return None

    def preferred_building_ratio(self, alternatives):  # pylint: disable=no-self-use,unused-argument
        """Select a fraction less than 1 at maximum ratio of PP for buildings"""
        return None

    def preferred_discount_multiplier(self, alternatives):  # pylint: disable=no-self-use,unused-argument
        """Select a discount multiplier for use in evaluate planet"""
        return None

    def max_defense_portion(self):  # pylint: disable=no-self-use,unused-argument
        """Return maximum fraction of production for defense"""
        return 1.0

    def check_orbital_production(self):  # pylint: disable=no-self-use,unused-argument
        """Return if orbital production is checked this turn"""
        return False

    def target_number_of_orbitals(self):  # pylint: disable=no-self-use,unused-argument
        """Return target number of orbitals"""
        return 0

    def may_build_building(self, building):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to build ''building''"""
        return True

    def may_produce_troops(self):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to produce troop ships"""
        return True

    def military_safety_factor(self):  # pylint: disable=no-self-use,unused-argument
        """Return military safety factor"""
        return 0.0

    def may_dither_focus_to_gain_research(self):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to trade production at a loss for research"""
        return True

    def may_research_heavily(self):  # pylint: disable=no-self-use,unused-argument
        """Return true if allowed to target research/industry > 1.5"""
        return True

    def may_travel_beyond_supply(self, distance):  # pylint: disable=no-self-use,unused-argument
        """Return True if able to travel distance hops beyond empire supply"""
        return True

    def get_research_index(self):  # pylint: disable=no-self-use,unused-argument
        """Deprecated"""
        return None

    def may_research_xeno_genetics_variances(self):  # pylint: disable=no-self-use,unused-argument
        """Return True if able to vary xeno genetics research"""
        return True

    def prefer_research_defensive(self):  # pylint: disable=no-self-use,unused-argument
        """Return True if should prefer defensive tech research"""
        return True

    def prefer_research_low_aggression(self):  # pylint: disable=no-self-use,unused-argument
        """Return True if should prefer less aggressive tech research"""
        return True

    def may_research_tech(self, tech):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to research ''tech''"""
        return True

    def may_research_tech_classic(self, tech):  # pylint: disable=no-self-use,unused-argument
        """Return True if permitted to research ''tech'' in the classic algorithm"""
        return True


class Character(Behavior):
    """
    A collection of behaviours.

    For each query that Behavior supports a Character queries
    all of its own behaviors to determine if an action is permissible or prefered.
    """

    def __init__(self, behaviors):
        self.behaviors = behaviors
        if not all([isinstance(x, Behavior) for x in behaviors]):
            raise TypeError("All behaviors must be sub-classes of Behavior")

    @property
    def key(self):
        return super.key

    def get_behavior(self, type_of_behavior):
        """Return the requested behavior or None"""
        behavior = [x for x in self.behaviors if isinstance(x, type_of_behavior)]
        return behavior[0] if behavior else Behavior()


# Complete the Character class by adding all of the combiners to combine the outputs of the
# individual behaviors.  Character tries to combine results in the way most limiting to the AI

def _make_single_function_combiner(funcnamei, f_combo):
    """Make a combiner that collects the results of funcname from each behavior
    and applies f_combo to the results"""
    def func(self, *args, **kwargs):
        """Apply funcnamei to each behavior and combine them with ''f_combo''"""
        return f_combo([getattr(x, funcnamei)(*args, **kwargs) for x in self.behaviors])
    return func

# Create combiners for behaviors that all must be true
for funcname in ["may_explore_system", "may_surge_industry", "may_maximize_research", "may_invade",
                 "may-invade_with_bases", "may_build_building", "may_produce_troops", "may_dither_focus_to_gain_research",
                 "may_research_heavily", "may_travel_beyond_supply", "may_research_xeno_genetics_variances",
                 "prefer_research_defensive", "prefer_research_low_aggression", "may_research_tech",
                 "may_research_tech_classic"]:
    setattr(Character, funcname, _make_single_function_combiner(funcname, all))

# Create combiners for behaviors that take min result
for funcname in ["max_number_colonies", "invasion_priority_scaling", "military_priority_scaling", "max_defense_portion"]:
    setattr(Character, funcname, _make_single_function_combiner(funcname, min))

# Create combiners for behaviors that take max result
for funcname in ["target_number_of_orbitals", "military_safety_factor", "get_research_index"]:
    setattr(Character, funcname, _make_single_function_combiner(funcname, max))

# Create combiners for behaviors that take any result
for funcname in ["check_orbital_production"]:
    setattr(Character, funcname, _make_single_function_combiner(funcname, any))


def _make_most_preferred_combiner(funcnamei):
    """Make acombiner that runs the preference function for each behavior and
    returns the result most preferred by all the behaviors."""
    def _most_preferred(self, alternatives):
        """Applies funcnamei from each behavior to the alternatives and return the most preferred."""
        prefs = [y for y in [getattr(x, funcnamei)(alternatives) for x in self.behaviors] if y is not None]
        if not prefs:
            return None
        if len(prefs) == 1:
            return prefs[0]
        counts = defaultdict(int)
        for pref in prefs:
            counts[pref] += 1
        return max(counts)
    return _most_preferred

# Create combiners for behaviors deal with preference
for funcname in ["preferred_research_cutoff", "preferred_colonization_portion",
                 "preferred_outpost_portion", "preferred_building_ratio", "preferred_discount_multiplier"]:
    setattr(Character, funcname, _make_most_preferred_combiner(funcname))


