import unittest
from copy import deepcopy

import freeOrionAIInterface as fo

from abstract_test_bases import PropertyTester
from utils import greater_or_equal, is_not_equal

TYPE = PropertyTester.TYPE
READ_ONLY = PropertyTester.READ_ONLY
ADDITIONAL_TESTS = PropertyTester.ADDITIONAL_TESTS


class GalaxySetupDataTester(PropertyTester):
    class_to_test = fo.GalaxySetupData
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "seed": {
            TYPE: str,
        },
        "size": {
            TYPE: int,
            ADDITIONAL_TESTS: [greater_or_equal(1)],
        },
        "shape": {
            TYPE: fo.galaxyShape,
            ADDITIONAL_TESTS: [is_not_equal(fo.galaxyShape.invalid)],
        },
        "age": {
            TYPE: fo.galaxySetupOption,
            ADDITIONAL_TESTS: [is_not_equal(fo.galaxyShape.invalid)],
        },
        "starlaneFrequency": {
            TYPE: fo.galaxySetupOption,
            ADDITIONAL_TESTS: [is_not_equal(fo.galaxyShape.invalid)],
        },
        "planetDensity": {
            TYPE: fo.galaxySetupOption,
            ADDITIONAL_TESTS: [is_not_equal(fo.galaxyShape.invalid)],
        },
        "specialsFrequency": {
            TYPE: fo.galaxySetupOption,
            ADDITIONAL_TESTS: [is_not_equal(fo.galaxyShape.invalid)],
        },
        "monsterFrequency": {
            TYPE: fo.galaxySetupOption,
            ADDITIONAL_TESTS: [is_not_equal(fo.galaxyShape.invalid)],
        },
        "nativeFrequency": {
            TYPE: fo.galaxySetupOption,
            ADDITIONAL_TESTS: [is_not_equal(fo.galaxyShape.invalid)],
        },
        "maxAIAggression": {
            TYPE: fo.aggression,
            ADDITIONAL_TESTS: [is_not_equal(fo.aggression.invalid)],
        },
        "gameUID": {
            TYPE: str,
            READ_ONLY: False,
        },
    })

    def setUp(self):
        self.objects_to_test = [fo.getGalaxySetupData()]


class GameRuleTester(unittest.TestCase):

    def test_gamerules(self):
        self.assertIsNotNone(fo.getGameRules())


# TODO: Expose and test GameRules::Type and associated functions
class GameRulesFuncTester(PropertyTester):
    class_to_test = fo.GameRules
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "empty": {
            TYPE: bool,
        },
    })

    def test_getRulesAsStrings(self):
        retval = self.rules.getRulesAsStrings()
        self.assertIsNotNone(retval)
        self.assertIsInstance(retval, dict)

    def test_ruleExists(self):
        for rule in self.rule_names:
            retval = self.rules.ruleExists(rule)
            self.assertIsInstance(retval, bool)
            self.assertTrue(retval)

        retval = self.rules.ruleExists(self.nonexisting_rule)
        self.assertIsInstance(retval, bool)
        self.assertFalse(retval)

        with self.assertRaises(Exception):
            self.rules.ruleExists(2)  # should only take str

    def test_getDescription(self):
        for rule in self.rule_names:
            retval = self.rules.getDescription(rule)
            self.assertIsInstance(retval, str)

        with self.assertRaises(RuntimeError):
            self.rules.getDescription(self.nonexisting_rule)

    def setUp(self):
        self.rules = fo.getGameRules()
        self.objects_to_test = [self.rules]
        self.rule_names = []
        self.nonexisting_rule = "THIS_RULE_SHOULD_NOT_EXIST_2308213"
        for rule in self.rules.getRulesAsStrings():
            self.rule_names.append(rule)


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    test_classes = [GalaxySetupDataTester, GameRuleTester, GameRulesFuncTester]
    for test_class in test_classes:
        if issubclass(test_class, PropertyTester):
            # generate the tests from setup data
            test_class.generate_tests()
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    return suite
