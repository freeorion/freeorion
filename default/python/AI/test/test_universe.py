import unittest

import freeOrionAIInterface as fo

from AIDependencies import INVALID_ID


TYPE = "type"
READ_ONLY = "read_only"
ADDITIONAL_TESTS = "additional_tests"


class UniverseTester(unittest.TestCase):

    def test_universe(self):
        self.assertIsNotNone(fo.getUniverse())


class PropertyTester(unittest.TestCase):
    # hack: we don't want unittest to run this abstract test base.
    # Therefore, we override this classes run function to do nothing.
    def __init__(self, *args, **kwargs):
        super(PropertyTester, self).__init__(*args, **kwargs)
        if self.__class__ == PropertyTester:
            self.run = lambda self, *args, **kwargs: None

    def test_properties(self):
        self.assertTrue(self.properties)
        self.assertTrue(self.objects_to_test)
        for property, kwargs in self.properties.items():
            for obj in self.objects_to_test:
                self.__test_property(property, obj, **kwargs)

    def __test_property(self, property_to_test, obj, type=None, read_only=None, additional_tests=[]):
        return_value = property_to_test.fget(obj)
        if type is not None:
            self.assertIsInstance(return_value, type)
        else:
            self.assertIsNone(return_value)

        if read_only is True:
            self.assertIsNone(property_to_test.fset)

        if read_only is False:
            property_to_test.fset(obj, type())
            self.assertIsEqual(property_to_test.fget(obj), type())
            property_to_test.fset(obj, return_value)
            self.assertIsEqual(property_to_test.fget(obj), return_value)

        for test in additional_tests:
            test[0](return_value, *test[1:])


class UniverseObjectTester(PropertyTester):
    # hack: we don't want unittest to run this abstract test base.
    # Therefore, we override this classes run function to do nothing.
    def __init__(self, *args, **kwargs):
        super(UniverseObjectTester, self).__init__(*args, **kwargs)
        if self.__class__ == UniverseObjectTester:
            self.run = lambda self, *args, **kwargs: None

    def setUp(self):
        self.properties = {
            fo.universeObject.id: {
                TYPE: int,
                READ_ONLY: True,
            },
            fo.universeObject.name: {
                TYPE: str,
                READ_ONLY: True,
            },
            fo.universeObject.x: {
                TYPE: float,
                READ_ONLY: True,
            },
            fo.universeObject.y: {
                TYPE: float,
                READ_ONLY: True,
            },
            fo.universeObject.systemID: {
                TYPE: int,
                READ_ONLY: True,
            },
            fo.universeObject.unowned: {
                TYPE: bool,
                READ_ONLY: True,
            },
            fo.universeObject.owner: {
                TYPE: int,
                READ_ONLY: True,
            },
            fo.universeObject.creationTurn: {
                TYPE: int,
                READ_ONLY: True,
            },
            fo.universeObject.ageInTurns: {
                TYPE: int,
                READ_ONLY: True,
            },
            fo.universeObject.containedObjects: {
                TYPE: fo.IntSet,
                READ_ONLY: True,
            },
            fo.universeObject.containerObject: {
                TYPE: int,
                READ_ONLY: True,
            },
            fo.universeObject.tags: {
                TYPE: fo.StringSet,
                READ_ONLY: True,
            },
            fo.universeObject.meters: {
                TYPE: fo.MeterTypeMeterMap,
                READ_ONLY: True,
            },
        }


class FleetTester(UniverseObjectTester):

    def setUp(self):
        super(FleetTester, self).setUp()
        universe = fo.getUniverse()
        fleet_ids = list(universe.fleetIDs)
        self.objects_to_test = map(universe.getFleet, fleet_ids)

        def greater_or_equal(x):
            return (self.assertGreaterEqual, x)

        def is_equal(x):
            return (self.assertEqual, x)

        self.properties.update({
            fo.fleet.fuel: {
                TYPE: float,
                READ_ONLY: True,
                ADDITIONAL_TESTS: [greater_or_equal(0)],
            },
            fo.fleet.maxFuel: {
                TYPE: float,
                READ_ONLY: True,
                ADDITIONAL_TESTS: [greater_or_equal(0)],
            },
            fo.fleet.finalDestinationID: {
                TYPE: int,
                READ_ONLY: True,
                ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
            },
            fo.fleet.previousSystemID: {
                TYPE: int,
                READ_ONLY: True,
                ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
            },
            fo.fleet.nextSystemID: {
                TYPE: int,
                READ_ONLY: True,
                ADDITIONAL_TESTS: [is_equal(INVALID_ID)],
            },
            fo.fleet.aggressive: {
                TYPE: bool,
                READ_ONLY: True,
            },
            fo.fleet.speed: {
                TYPE: float,
                READ_ONLY: True,
            },
            fo.fleet.hasMonsters: {
                TYPE: bool,
                READ_ONLY: True,
            },
            fo.fleet.hasFighterShips: {
                TYPE: bool,
                READ_ONLY: True,
            },
            fo.fleet.hasColonyShips: {
                TYPE: bool,
                READ_ONLY: True,
            },
            fo.fleet.hasOutpostShips: {
                TYPE: bool,
                READ_ONLY: True,
            },
            fo.fleet.hasTroopShips: {
                TYPE: bool,
                READ_ONLY: True,
            },
            fo.fleet.numShips: {
                TYPE: int,
                READ_ONLY: True,
            },
            fo.fleet.empty: {
                TYPE: int,
                READ_ONLY: True,
                ADDITIONAL_TESTS: [(self.assertFalse,)],
            },
            fo.fleet.shipIDs: {
                TYPE: fo.IntSet,
                READ_ONLY: False,  # FIXME: Intentional failure of unittest!
            },
        })
