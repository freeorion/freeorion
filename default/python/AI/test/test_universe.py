import unittest
from copy import deepcopy

import freeOrionAIInterface as fo

from AIDependencies import INVALID_ID

from abstract_test_bases import PropertyTester
from utils import greater_or_equal, is_equal, is_false


TYPE = PropertyTester.TYPE
READ_ONLY = PropertyTester.READ_ONLY
ADDITIONAL_TESTS = PropertyTester.ADDITIONAL_TESTS


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

    def test_dump_takes_no_args(self):
        for obj in self.objects_to_test:
            for arg in (int(), float(), str(), None):
                self.assertRaises(Exception, obj.dump, arg)

    def test_ownedBy_owner(self):
        for obj in self.objects_to_test:
            retval = obj.ownedBy(obj.owner)
            self.assertIsInstance(retval, bool)
            if obj.owner != INVALID_ID:
                self.assertTrue(retval)
            else:
                self.assertFalse(retval)

    def test_ownedBy_invalid_id(self):
        for obj in self.objects_to_test:
            retval = obj.ownedBy(INVALID_ID)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def test_ownedBy_not_owner(self):
        for obj in self.objects_to_test:
            retval = obj.ownedBy(obj.owner + 1)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def test_ownedBy_invalid_args(self):
        for obj in self.objects_to_test:
            self.assertRaises(Exception, obj.ownedBy)
            for arg in (float(), str(), None):
                self.assertRaises(Exception, obj.ownedBy, arg)

    def test_hasSpecial_invalid_args(self):
        for obj in self.objects_to_test:
            for arg in (int(), float(), None):
                self.assertRaises(Exception, obj.hasSpecial, arg)

    def test_hasSpecial_no_args(self):
        for obj in self.objects_to_test:
            self.assertRaises(Exception, obj.hasSpecial)

    def test_hasSpecial_empty_string(self):
        for obj in self.objects_to_test:
            retval = obj.hasSpecial("")
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def test_hasSpecial_contained_special(self):
        for obj in self.objects_to_test:
            for special in obj.specials:
                retval = obj.hasSpecial(special)
                self.assertIsInstance(retval, bool)
                self.assertTrue(retval)

    def test_contains_no_args(self):
        for obj in self.objects_to_test:
            self.assertRaises(Exception, obj.contains)

    def test_contains_invalid_args(self):
        for obj in self.objects_to_test:
            for arg in (float(), str(), None):
                self.assertRaises(Exception, obj.contains, arg)

    def test_contains_invalid_id(self):
        for obj in self.objects_to_test:
            retval = obj.contains(INVALID_ID)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def test_contains_contained_objects(self):
        for obj in self.objects_to_test:
            for contained_obj in obj.containedObjects:
                retval = obj.contains(contained_obj)
                self.assertIsInstance(retval, bool)
                self.assertTrue(retval)

    def test_contains_not_contained_object(self):
        universe = fo.getUniverse()
        for obj in self.objects_to_test:
            for universe_object in universe.allObjectIDs:
                if universe_object in obj.containedObjects:
                    continue
                retval = obj.contains(universe_object)
                self.assertIsInstance(retval, bool)
                self.assertFalse(retval)

    def test_containedBy_no_args(self):
        for obj in self.objects_to_test:
            self.assertRaises(Exception, obj.containedBy)

    def test_containedBy_invalid_args(self):
        for obj in self.objects_to_test:
            for arg in (float(), str(), None):
                self.assertRaises(Exception, obj.containedBy, arg)

    def test_containedBy_invalid_id(self):
        for obj in self.objects_to_test:
            retval = obj.containedBy(INVALID_ID)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def test_containedBy_container(self):
        for obj in self.objects_to_test:
            retval = obj.containedBy(obj.containerObject)
            self.assertIsInstance(retval, bool)
            if obj.containerObject == INVALID_ID:
                self.assertFalse(retval)
            else:
                self.assertTrue(retval)

    def test_containedBy_not_container(self):
        for obj in self.objects_to_test:
            retval = obj.containedBy(obj.containerObject + 1)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)


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
