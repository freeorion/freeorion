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

    def __test_property(self, property_to_test, obj, type=None, read_only=True, additional_tests=[]):
        self.assertIn(property_to_test, dir(obj))
        property_to_test = getattr(self.class_to_test, property_to_test)
        return_value = property_to_test.fget(obj)
        if type is not None:
            self.assertIsInstance(return_value, type)
        else:
            self.assertIsNone(return_value)

        if read_only:
            self.assertIsNone(property_to_test.fset)
        else:
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
        },
        "name": {
            TYPE: str,
        },
        "x": {
            TYPE: float,
        },
        "y": {
            TYPE: float,
        },
        "systemID": {
            TYPE: int,
        },
        "unowned": {
            TYPE: bool,
        },
        "owner": {
            TYPE: int,
        },
        "creationTurn": {
            TYPE: int,
        },
        "ageInTurns": {
            TYPE: int,
        },
        "containedObjects": {
            TYPE: fo.IntSet,
        },
        "containerObject": {
            TYPE: int,
        },
        "tags": {
            TYPE: fo.StringSet,
        },
        "meters": {
            TYPE: fo.MeterTypeMeterMap,
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
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "maxFuel": {
            TYPE: float,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "finalDestinationID": {
            TYPE: int,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "previousSystemID": {
            TYPE: int,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "nextSystemID": {
            TYPE: int,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "aggressive": {
            TYPE: bool,
        },
        "speed": {
            TYPE: float,
        },
        "hasMonsters": {
            TYPE: bool,
        },
        "hasFighterShips": {
            TYPE: bool,
        },
        "hasColonyShips": {
            TYPE: bool,
        },
        "hasOutpostShips": {
            TYPE: bool,
        },
        "hasTroopShips": {
            TYPE: bool,
        },
        "numShips": {
            TYPE: int,
        },
        "empty": {
            TYPE: int,
            ADDITIONAL_TESTS: [is_false()],
        },
        "shipIDs": {
            TYPE: fo.IntSet,
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
        },
        "designID": {
            TYPE: int,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "fleetID": {
            TYPE: int,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "producedByEmpireID": {
            TYPE: int,
            ADDITIONAL_TESTS: [greater_or_equal(INVALID_ID)],
        },
        "arrivedOnTurn": {
            TYPE: int,
        },
        "lastResuppliedOnTurn": {
            TYPE: int,
        },
        "lastTurnActiveInCombat": {
            TYPE: int,
        },
        "isMonster": {
            TYPE: bool,
        },
        "isArmed": {
            TYPE: bool,
        },
        "hasFighters": {
            TYPE: bool,
        },
        "canColonize": {
            TYPE: bool,
        },
        "canInvade": {
            TYPE: bool,
        },
        "canBombard": {
            TYPE: bool,
        },
        "speciesName": {
            TYPE: str,
        },
        "speed": {
            TYPE: float,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "colonyCapacity": {
            TYPE: float,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "troopCapacity": {
            TYPE: float,
            ADDITIONAL_TESTS: [greater_or_equal(0)],
        },
        "orderedScrapped": {
            TYPE: bool,
            ADDITIONAL_TESTS: [is_false()],
        },
        "orderedColonizePlanet": {
            TYPE: int,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "orderedInvadePlanet": {
            TYPE: int,
            ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
        },
        "partMeters": {
            TYPE: fo.ShipPartMeterMap,
        },
    })

    def setUp(self):
        universe = fo.getUniverse()
        fleet_ids = list(universe.fleetIDs)
        self.objects_to_test = []
        for fid in fleet_ids:
            self.objects_to_test.extend(map(universe.getShip, universe.getFleet(fid).shipIDs))


class PartTypeTester(PropertyTester):
    class_to_test = fo.partType
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "name": {
            TYPE: str,
        },
        "partClass": {
            TYPE: int,
        },
        "capacity": {
            TYPE: float,
        },
        "secondaryStat": {
            TYPE: float,
        },
        "mountableSlotTypes": {
            TYPE: fo.ShipSlotVec,
        },
        "costTimeLocationInvariant": {
            TYPE: bool,
        },
    })

    def setUp(self):
        universe = fo.getUniverse()
        ships = map(universe.getShip, universe.shipIDs)
        self.objects_to_test = []
        for ship in ships:
            design = ship.design
            self.objects_to_test.extend([fo.getPartType(part) for part in design.parts if part])


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    test_classes = [UniverseObjectTester, UniverseTester, FleetTester, ShipTester, PartTypeTester]
    for test_class in test_classes:
        if issubclass(test_class, PropertyTester):
            # generate the tests from setup data
            test_class.generate_tests()
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    return suite
