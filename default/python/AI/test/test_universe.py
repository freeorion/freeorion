import unittest
from copy import deepcopy

import freeOrionAIInterface as fo

from AIDependencies import INVALID_ID


def greater_or_equal(arg):
    return unittest.TestCase.assertGreaterEqual, arg


def is_equal(arg):
    return unittest.TestCase.assertGreaterEqual, arg


def is_false():
    return (unittest.TestCase.assertFalse,)


TYPE = "type"
READ_ONLY = "read_only"
ADDITIONAL_TESTS = "additional_tests"


class UniverseTester(unittest.TestCase):

    def test_universe(self):
        self.assertIsNotNone(fo.getUniverse())


class PropertyTester(unittest.TestCase):
    # hack: we don't want unittest to run this abstract test base.
    # Therefore, we override this classes run function to do nothing.
    class_to_test = None
    properties = {}

    def __init__(self, *args, **kwargs):
        super(PropertyTester, self).__init__(*args, **kwargs)
        if self.__class__ == PropertyTester:
            self.run = lambda self, *args, **kwargs: None

    @classmethod
    def generate_tests(cls):
        for property in cls.properties:
            setattr(cls, "test_property_%s" % property,
                    PropertyTester._test_case_generator(property, cls.properties[property]))

    @staticmethod
    def _test_case_generator(property, kwargs):
        def _test_case(self):
            for obj in self.objects_to_test:
                self.__test_property(property, obj, **kwargs)
        return _test_case

    def __test_property(self, property_to_test, obj, type=None, read_only=None, additional_tests=[]):
        self.assertIn(property_to_test, dir(obj))
        property_to_test = getattr(self.class_to_test, property_to_test)
        return_value = property_to_test.fget(obj)
        if type is not None:
            self.assertIsInstance(return_value, type)
        else:
            self.assertIsNone(return_value)

        if read_only is True:
            self.assertIsNone(property_to_test.fset)

        if read_only is False:
            property_to_test.fset(obj, type())
            self.assertEqual(property_to_test.fget(obj), type())
            property_to_test.fset(obj, return_value)
            self.assertEqual(property_to_test.fget(obj), return_value)

        for test in additional_tests:
            test[0](self, return_value, *test[1:])


class UniverseObjectTester(PropertyTester):
    # hack: we don't want unittest to run this abstract test base.
    # Therefore, we override this classes run function to do nothing.
    class_to_test = fo.universeObject
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "id": {
            TYPE: int,
            READ_ONLY: True,
        },
        "name": {
            TYPE: str,
            READ_ONLY: True,
        },
        "x": {
            TYPE: float,
            READ_ONLY: True,
        },
        "y": {
            TYPE: float,
            READ_ONLY: True,
        },
        "systemID": {
            TYPE: int,
            READ_ONLY: True,
        },
        "unowned": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "owner": {
            TYPE: int,
            READ_ONLY: True,
        },
        "creationTurn": {
            TYPE: int,
            READ_ONLY: True,
        },
        "ageInTurns": {
            TYPE: int,
            READ_ONLY: True,
        },
        "containedObjects": {
            TYPE: fo.IntSet,
            READ_ONLY: True,
        },
        "containerObject": {
            TYPE: int,
            READ_ONLY: True,
        },
        "tags": {
            TYPE: fo.StringSet,
            READ_ONLY: True,
        },
        "meters": {
            TYPE: fo.MeterTypeMeterMap,
            READ_ONLY: True,
        },
    })

    def setUp(self):
        universe = fo.getUniverse()
        object_ids = list(universe.allObjectIDs)
        self.objects_to_test = map(universe.getObject, object_ids)


class FleetTester(UniverseObjectTester):
    class_to_test = fo.fleet
    properties = deepcopy(UniverseObjectTester.properties)
    properties.update({
        "fuel": {
            TYPE: float,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "maxFuel": {
            TYPE: float,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "finalDestinationID": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "previousSystemID": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "nextSystemID": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "aggressive": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "speed": {
            TYPE: float,
            READ_ONLY: True,
        },
        "hasMonsters": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "hasFighterShips": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "hasColonyShips": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "hasOutpostShips": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "hasTroopShips": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "numShips": {
            TYPE: int,
            READ_ONLY: True,
        },
        "empty": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [is_false()],
        },
        "shipIDs": {
            TYPE: fo.IntSet,
            READ_ONLY: True,
        },
    })

    def setUp(self):
        universe = fo.getUniverse()
        fleet_ids = list(universe.fleetIDs)
        self.objects_to_test = map(universe.getFleet, fleet_ids)
        assert all(isinstance(obj, self.class_to_test) for obj in self.objects_to_test)


class ShipTester(UniverseObjectTester):
    class_to_test = fo.ship
    properties = deepcopy(UniverseObjectTester.properties)
    properties.update({
        "design": {
            TYPE: fo.shipDesign,
            READ_ONLY: True,
        },
        "designID": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "fleetID": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "producedByEmpireID": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [greater_or_equal(INVALID_ID)],
        },
        "arrivedOnTurn": {
            TYPE: int,
            READ_ONLY: True,
        },
        "lastResuppliedOnTurn": {
            TYPE: int,
            READ_ONLY: True,
        },
        "lastTurnActiveInCombat": {
            TYPE: int,
            READ_ONLY: True,
        },
        "isMonster": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "isArmed": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "hasFighters": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "canColonize": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "canInvade": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "canBombard": {
            TYPE: bool,
            READ_ONLY: True,
        },
        "speciesName": {
            TYPE: str,
            READ_ONLY: True,
        },
        "speed": {
            TYPE: float,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "colonyCapacity": {
            TYPE: float,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "troopCapacity": {
            TYPE: float,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "orderedScrapped": {
            TYPE: bool,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [is_false()],
        },
        "orderedColonizePlanet": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "orderedInvadePlanet": {
            TYPE: int,
            READ_ONLY: True,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "partMeters": {
            TYPE: fo.ShipPartMeterMap,
            READ_ONLY: True,
        },
    })

    def setUp(self):
        universe = fo.getUniverse()
        fleet_ids = list(universe.fleetIDs)
        self.objects_to_test = []
        for fid in fleet_ids:
            self.objects_to_test.extend(map(universe.getShip, universe.getFleet(fid).shipIDs))


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    test_classes = [UniverseObjectTester, UniverseTester, FleetTester, ShipTester]
    for test_class in test_classes:
        if issubclass(test_class, PropertyTester):
            # generate the tests from setup data
            test_class.generate_tests()
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    return suite
