import unittest
from copy import deepcopy

import freeOrionAIInterface as fo

from abstract_test_bases import PropertyTester
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

    def test_dump(self):
        for obj in self.objects_to_test:
            retval = obj.dump()
            self.assertIsInstance(retval, str)
            self.assertTrue(retval)


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
