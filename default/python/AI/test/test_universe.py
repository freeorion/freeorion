import unittest
from copy import deepcopy

import freeOrionAIInterface as fo

from AIDependencies import INVALID_ID

from abstract_test_bases import PropertyTester, DumpTester
from utils import greater_or_equal, is_equal, is_not_equal, is_false


TYPE = PropertyTester.TYPE
READ_ONLY = PropertyTester.READ_ONLY
ADDITIONAL_TESTS = PropertyTester.ADDITIONAL_TESTS


class UniverseTester(unittest.TestCase):

    def test_universe(self):
        self.assertIsNotNone(fo.getUniverse())


class UniverseObjectTester(PropertyTester, DumpTester):
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


class ShipDesignTester(PropertyTester, DumpTester):
    class_to_test = fo.shipDesign
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "id": {
            TYPE: int,
        },
        "name": {
            TYPE: str,
        },
        "description": {
            TYPE: str,
        },
        "designedOnTurn": {
            TYPE: int,
        },
        "speed": {
            TYPE: float,
        },
        "structure": {
            TYPE: float,
        },
        "shields": {
            TYPE: float,
        },
        "fuel": {
            TYPE: float,
        },
        "detection": {
            TYPE: float,
        },
        "colonyCapacity": {
            TYPE: float,
        },
        "troopCapacity": {
            TYPE: float,
        },
        "stealth": {
            TYPE: float,
        },
        "industryGeneration": {
            TYPE: float,
        },
        "researchGeneration": {
            TYPE: float,
        },
        "tradeGeneration": {
            TYPE: float,
        },
        "defense": {
            TYPE: float,
        },
        "attack": {
            TYPE: float,
        },
        "canColonize": {
            TYPE: bool,
        },
        "canInvade": {
            TYPE: bool,
        },
        "isArmed": {
            TYPE: bool,
        },
        "hasFighters": {
            TYPE: bool,
        },
        "hasDirectWeapons": {
            TYPE: bool,
        },
        "isMonster": {
            TYPE: bool,
        },
        "costTimeLocationInvariant": {
            TYPE: bool,
        },
        "hull": {
            TYPE: str,
            ADDITIONAL_TESTS: [is_not_equal("")],
        },
        "hull_type": {
            TYPE: fo.hullType,
        },
        "parts": {
            TYPE: fo.StringVec,
        },
        "attackStats": {
            TYPE: fo.IntVec,
        },
    })

    def test_productionCost(self):
        for obj in self.objects_to_test:
            retval = obj.productionCost(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, float)
            self.assertGreater(retval, 0.0)

    def test_productionTime(self):
        for obj in self.objects_to_test:
            retval = obj.productionTime(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, int)
            self.assertGreater(retval, 0.0)

    def test_perTurnCost(self):
        for obj in self.objects_to_test:
            retval = obj.perTurnCost(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, float)
            self.assertGreater(retval, 0.0)
            cost = obj.productionCost(fo.empireID(), INVALID_ID)
            time = obj.productionTime(fo.empireID(), INVALID_ID)
            self.assertAlmostEquals(retval, cost/time, places=3)

    def test_productionLocationForEmpire(self):
        for obj in self.objects_to_test:
            retval = obj.productionLocationForEmpire(fo.empireID(), -1)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def test_validShipDesign(self):
        for obj in self.objects_to_test:
            retval = fo.validShipDesign(obj.hull, list(obj.parts))
            self.assertIsInstance(retval, bool)
            self.assertTrue(retval)

        retval = fo.validShipDesign("THIS_HULL_DOES_NOT_EXIST", ["", ""])
        self.assertIsInstance(retval, bool)
        self.assertFalse(retval)

    def setUp(self):
        universe = fo.getUniverse()
        fleet_ids = list(universe.fleetIDs)
        ship_ids = [ship_id for fid in fleet_ids
                    for ship_id in universe.getFleet(fid).shipIDs]
        self.objects_to_test = [fo.getShipDesign(universe.getShip(ship_id).designID) for ship_id in ship_ids]
        self.objects_to_test.append(fo.getPredefinedShipDesign("SD_COLONY_BASE"))
        assert self.objects_to_test


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


class HullTypeTester(PropertyTester):
    class_to_test = fo.hullType
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "name": {
            TYPE: str,
        },
        "numSlots": {
            TYPE: int,
        },
        "structure": {
            TYPE: float,
        },
        "stealth": {
            TYPE: float,
        },
        "fuel": {
            TYPE: float,
        },
        "speed": {
            TYPE: float,
        },
        "slots": {
            TYPE: fo.ShipSlotVec,
        },
        "costTimeLocationInvariant": {
            TYPE: bool,
        },
    })

    def test_numSlotsOfSlotType(self):
        for obj in self.objects_to_test:
            total_slots = 0
            for slot_type in (fo.shipSlotType.internal, fo.shipSlotType.external, fo.shipSlotType.core):
                retval = obj.numSlotsOfSlotType(slot_type)
                self.assertIsInstance(retval, int)
                self.assertGreaterEqual(retval, 0)
                total_slots += retval
            self.assertEquals(total_slots, obj.numSlots)

            with self.assertRaises(Exception):
                obj.numSlotsOfSlotType(2)
            with self.assertRaises(Exception):
                obj.numSlotsOfSlotType("")

    def test_productionCost(self):
        for obj in self.objects_to_test:
            retval = obj.productionCost(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, float)
            self.assertGreaterEqual(retval, 0.0)

    def test_productionTime(self):
        for obj in self.objects_to_test:
            retval = obj.productionTime(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, int)
            self.assertGreaterEqual(retval, 0)

    def test_hasTag(self):
        for obj in self.objects_to_test:
            retval = obj.hasTag("")
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def setUp(self):
        empire = fo.getEmpire()
        self.objects_to_test = [fo.getHullType(hull) for hull in empire.availableShipHulls]


class BuildingTester(UniverseObjectTester):
    class_to_test = fo.building
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "buildingTypeName": {
            TYPE: str,
        },
        "planetID": {
            TYPE: int,
        },
        "producedByEmpireID": {
            TYPE: int,
        },
        "orderedScrapped": {
            TYPE: bool,
            ADDITIONAL_TESTS: [is_false()]
        }
    })

    def setUp(self):
        universe = fo.getUniverse()
        self.objects_to_test = [universe.getBuilding(_id) for _id in universe.buildingIDs]


class BuildingTypeTester(PropertyTester, DumpTester):
    class_to_test = fo.buildingType
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "name": {
            TYPE: str,
        },
        "description": {
            TYPE: str,
        },
        "costTimeLocationInvariant": {
            TYPE: bool,
        },
    })

    def test_productionCost(self):
        for obj in self.objects_to_test:
            retval = obj.productionCost(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, float)
            self.assertGreaterEqual(retval, 0.0)

    def test_productionTime(self):
        for obj in self.objects_to_test:
            retval = obj.productionTime(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, int)
            self.assertGreaterEqual(retval, 0)

    def test_perTurnCost(self):
        for obj in self.objects_to_test:
            retval = obj.perTurnCost(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, float)
            self.assertGreaterEqual(retval, 0)

            cost = obj.productionCost(fo.empireID(), INVALID_ID)
            time = obj.productionTime(fo.empireID(), INVALID_ID)
            self.assertAlmostEquals(retval, cost/time, places=3)

    def test_captureResult(self):
        for obj in self.objects_to_test:
            retval = obj.captureResult(fo.empireID(), fo.empireID(), INVALID_ID, False)
            self.assertIsInstance(retval, fo.captureResult)

    def test_canBeProduced(self):
        for obj in self.objects_to_test:
            retval = obj.canBeProduced(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def test_canBeEnqueued(self):
        for obj in self.objects_to_test:
            retval = obj.canBeEnqueued(fo.empireID(), INVALID_ID)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

    def setUp(self):
        universe = fo.getUniverse()
        self.objects_to_test = [fo.getBuildingType(universe.getBuilding(_id).buildingTypeName)
                                for _id in universe.buildingIDs]


class ResourceCenterTester(PropertyTester, DumpTester):
    class_to_test = fo.resourceCenter
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "focus": {
            TYPE: str,
        },
        "turnsSinceFocusChange": {
            TYPE: int,
        },
        "availableFoci": {
            TYPE: fo.StringVec,
        },
    })

    def setUp(self):
        universe = fo.getUniverse()
        self.objects_to_test = [planet for planet in
                                [universe.getPlanet(planet_id) for planet_id in universe.planetIDs]
                                if planet.ownedBy(fo.empireID())]


class PopCenterTester(PropertyTester, DumpTester):
    class_to_test = fo.popCenter
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "speciesName": {
            TYPE: str,
        },
    })

    def setUp(self):
        universe = fo.getUniverse()
        self.objects_to_test = [planet for planet in
                                [universe.getPlanet(planet_id) for planet_id in universe.planetIDs]
                                if planet.ownedBy(fo.empireID())]


class PlanetTester(UniverseObjectTester):
    class_to_test = fo.planet
    properties = deepcopy(UniverseObjectTester.properties)
    properties.update({
        "size": {
            TYPE: fo.planetSize,
        },
        "type": {
            TYPE: fo.planetType,
        },
        "originalType": {
            TYPE: fo.planetType,
        },
        "distanceFromOriginalType": {
            TYPE: int,
        },
        "clockwiseNextPlanetType": {
            TYPE: fo.planetType,
        },
        "counterClockwiseNextPlanetType": {
            TYPE: fo.planetType,
        },
        "nextLargerPlanetSize": {
            TYPE: fo.planetSize,
        },
        "nextSmallerPlanetSize": {
            TYPE: fo.planetSize,
        },
        "OrbitalPeriod": {
            TYPE: float,
        },
        "InitialOrbitalPosition": {
            TYPE: float,
        },
        "RotationalPeriod": {
            TYPE: float,
        },
        "LastTurnAttackedByShip": {
            TYPE: int,
        },
        "LastTurnConquered": {
            TYPE: int,
        },
        "buildingIDs": {
            TYPE: fo.IntSet,
        },
        "habitableSize": {
            TYPE: int,
        },
    })

    def test_environmentForSpecies(self):
        species = "SP_HUMAN"
        species_obj = fo.getSpecies(species)
        for obj in self.objects_to_test:
            retval = obj.environmentForSpecies(species)
            self.assertIsInstance(retval, fo.planetEnvironment)
            self.assertEquals(retval, species_obj.getPlanetEnvironment(obj.type))
            with self.assertRaises(Exception):
                _ = obj.environmentForSpecies()

    def test_nextBetterPlanetTypeForSpecies(self):
        species = "SP_HUMAN"
        for obj in self.objects_to_test:
            retval = obj.nextBetterPlanetTypeForSpecies(species)
            self.assertIsInstance(retval, fo.planetType)
            with self.assertRaises(Exception):
                _ = obj.nextBetterPlanetTypeForSpecies()

    def test_OrbitalPositionOnTurn(self):
        for obj in self.objects_to_test:
            retval = obj.OrbitalPositionOnTurn(0)
            self.assertIsInstance(retval, float)
            with self.assertRaises(Exception):
                _ = obj.OrbitalPositionOnTurn()

    def setUp(self):
        universe = fo.getUniverse()
        self.objects_to_test = [universe.getPlanet(planet_id) for planet_id in universe.planetIDs]


class SystemTester(UniverseObjectTester):
    class_to_test = fo.system
    properties = deepcopy(UniverseObjectTester.properties)
    properties.update({
       "starType": {
           TYPE: fo.starType,
       },
        "numStarlanes": {
            TYPE: int,
        },
        "numWormholes": {
            TYPE: int,
        },
        "starlanesWormholes": {
            TYPE: fo.IntBoolMap,
        },
        "planetIDs": {
            TYPE: fo.IntSet,
        },
        "buildingIDs": {
            TYPE: fo.IntSet,
        },
        "fleetIDs": {
            TYPE: fo.IntSet,
        },
        "shipIDs": {
            TYPE: fo.IntSet,
        },
        "fieldIDs": {
            TYPE: fo.IntSet,
        },
        "lastTurnBattleHere": {
            TYPE: int,
        },
    })

    def test_HasStarlaneToSystemID(self):
        universe = fo.getUniverse()
        for obj in self.objects_to_test:
            # should not have connection to INVALID_ID
            retval = obj.HasStarlaneToSystemID(INVALID_ID)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

            # should have connection to its immediate neighbours
            for sys_id in universe.getImmediateNeighbors(obj.id, fo.empireID()):
                retval = obj.HasStarlaneToSystemID(sys_id)
                self.assertIsInstance(retval, bool)
                self.assertTrue(retval)

            # should accept only int
            with self.assertRaises(Exception):
                _ = obj.HasStarlaneToSystemID()
            with self.assertRaises(Exception):
                _ = obj.HasStarlaneToSystemID(0.2)
            with self.assertRaises(Exception):
                _ = obj.HasStarlaneToSystemID(1.0)
            with self.assertRaises(Exception):
                _ = obj.HasStarlaneToSystemID('1')

    def test_HasWormholeToSystemID(self):
        universe = fo.getUniverse()
        for obj in self.objects_to_test:
            # should not have any wormholes
            retval = obj.HasWormholeToSystemID(INVALID_ID)
            self.assertIsInstance(retval, bool)
            self.assertFalse(retval)

            for sys_id in universe.getImmediateNeighbors(obj.id, fo.empireID()):
                retval = obj.HasWormholeToSystemID(sys_id)
                self.assertIsInstance(retval, bool)
                self.assertFalse(retval)

            # should accept only int
            with self.assertRaises(Exception):
                _ = obj.HasWormholeToSystemID()
            with self.assertRaises(Exception):
                _ = obj.HasWormholeToSystemID(0.2)
            with self.assertRaises(Exception):
                _ = obj.HasWormholeToSystemID(1.0)
            with self.assertRaises(Exception):
                _ = obj.HasWormholeToSystemID('1')

    def test_contained_fleets(self):
        universe = fo.getUniverse()
        for obj in self.objects_to_test:
            for fleet_id in obj.fleetIDs:
                self.assertTrue(universe.getFleet(fleet_id).containedBy(obj.id))

    def test_contained_fields(self):
        universe = fo.getUniverse()
        for obj in self.objects_to_test:
            for field_id in obj.fieldIDs:
                self.assertTrue(universe.getField(field_id).containedBy(obj.id))

    def test_contained_planets(self):
        universe = fo.getUniverse()
        for obj in self.objects_to_test:
            for planet_id in obj.planetIDs:
                self.assertTrue(universe.getPlanet(planet_id).containedBy(obj.id))

    def test_contained_buildings(self):
        universe = fo.getUniverse()
        for obj in self.objects_to_test:
            for building_id in obj.buildingIDs:
                self.assertTrue(universe.getBuilding(building_id).containedBy(obj.id))

    def test_contained_ships(self):
        universe = fo.getUniverse()
        for obj in self.objects_to_test:
            for ship_id in obj.shipIDs:
                self.assertTrue(universe.getShip(ship_id).containedBy(obj.id))

    def setUp(self):
        universe = fo.getUniverse()
        self.objects_to_test = [universe.getSystem(system_id) for system_id in universe.systemIDs]

# TODO: Fields - but need to query an actual object, so need dedicated universe setup


class FieldTypeTester(PropertyTester, DumpTester):
    class_to_test = fo.fieldType
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "name": {
            TYPE: str,
        },
        "description": {
            TYPE: str,
        },
    })

    def setUp(self):
        self.objects_to_test = [fo.getFieldType("FLD_ION_STORM")]


class SpecialTester(PropertyTester, DumpTester):
    class_to_test = fo.special
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "name": {
            TYPE: str,
        },
        "description": {
            TYPE: str,
        },
        "spawnrate": {
            TYPE: float,
        },
        "spawnlimit": {
            TYPE: int,
        },
    })

    def test_initialCapacity(self):
        retval = fo.getSpecial("ANCIENT_RUINS_SPECIAL").initialCapacity(-1)
        self.assertIsInstance(retval, float)
        self.assertEquals(retval, 0.0)

    def setUp(self):
        self.objects_to_test = [fo.getSpecial("ANCIENT_RUINS_SPECIAL")]


class SpeciesTester(PropertyTester, DumpTester):
    class_to_test = fo.species
    properties = deepcopy(PropertyTester.properties)
    properties.update({
        "name": {
            TYPE: str,
        },
        "description": {
            TYPE: str,
        },
        "homeworlds": {
            TYPE: fo.IntSet,
        },
        "foci": {
            TYPE: fo.StringVec,
        },
        "preferredFocus": {
            TYPE: str,
        },
        "canColonize": {
            TYPE: bool,
        },
        "canProduceShips": {
            TYPE: bool,
        },
        "tags": {
            TYPE: fo.StringSet,
        },
    })

    def test_getPlanetEnvironment(self):
        species = fo.getSpecies("SP_HUMAN")
        retval = species.getPlanetEnvironment(fo.planetType.terran)
        self.assertIsInstance(retval, fo.planetEnvironment)
        self.assertEquals(retval, fo.planetEnvironment.good)

        retval = species.getPlanetEnvironment(fo.planetType.asteroids)
        self.assertIsInstance(retval, fo.planetEnvironment)
        self.assertEquals(retval, fo.planetEnvironment.uninhabitable)

        retval = species.getPlanetEnvironment(fo.planetType.toxic)
        self.assertIsInstance(retval, fo.planetEnvironment)
        self.assertEquals(retval, fo.planetEnvironment.hostile)

        with self.assertRaises(Exception):
            species.getPlanetEnvironment()

        with self.assertRaises(Exception):
            species.getPlanetEnvironment(int())

        with self.assertRaises(Exception):
            species.getPlanetEnvironment(str())

    def setUp(self):
        self.objects_to_test = [fo.getSpecies("SP_HUMAN")]


def load_tests(loader, tests, pattern):
    suite = unittest.TestSuite()
    test_classes = [UniverseTester, UniverseObjectTester, FleetTester, ShipTester, PartTypeTester,
                    HullTypeTester, BuildingTester, BuildingTypeTester, ResourceCenterTester,
                    PopCenterTester, PlanetTester, SystemTester,
                    ShipDesignTester, FieldTypeTester, SpecialTester, SpeciesTester]
    for test_class in test_classes:
        if issubclass(test_class, PropertyTester):
            # generate the tests from setup data
            test_class.generate_tests()
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)
    return suite
