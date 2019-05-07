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


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    test_classes = [GalaxySetupDataTester]
    for test_class in test_classes:
        if issubclass(test_class, PropertyTester):
            # generate the tests from setup data
            test_class.generate_tests()
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    return suite
