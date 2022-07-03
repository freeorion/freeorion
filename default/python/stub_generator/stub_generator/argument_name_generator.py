from logging import error

normalization_dict = {
    "empire": "empire_object",
    "int": "number",
    "str": "string",
    "float": "floating_number",
    "object": "obj",
    "IntBoolMap": "int_bool_map",
    "IntDblMap": "int_dbl_map",
    "universeObject": "base_object",
    "meterType": "meter_type",
    "bool": "boolean",
    "StringVec": "string_list",
    "shipDesign": "ship_design",
    "universe": "universe_object",
    "researchQueue": "research_queue",
    "resPoolMap": "res_pool",
    "productionQueue": "production_queue",
    "diplomaticMessage": "diplomatic_message",
    "ship": "ship_object",
    "species": "species_object",
    "planetType": "planet_type",
    "system": "system_object",
    "tech": "tech_object",
    "list": "item_list",
    "planet": "planet_object",
    "shipPart": "ship_part",
    "resPool": "res_pool",
    "researchQueueElement": "research_queue_element",
    "shipSlotType": "ship_slot_type",
    "IntIntMap": "int_int_map",
    "IntPairVec": "int_pair_list",
    "IntSet": "int_set",
    "IntSetSet": "int_set_set",
    "IntVec": "int_list",
    "IntVisibilityMap": "int_visibility_map",
    "UnlockableItemVec": "unlockable_item_vec",
    "StringSet": "string_set",
    "StringSet2": "string_set",
    "VisibilityIntMap": "visibility_int_map",
    "buildingType": "buildingType",
    "productionQueueElement": "production_queue_element",
    "resourceType": "resource_type",
    "fleetAggression": "fleet_aggression",
    "buildType": "build_type",
    "field": "field",
    "shipHull": "ship_hull",
    "PlanetSize": "planet_size",
    "planetSize": "planet_size",
    "unlockableItemType": "unlockable_item_type",
    "dict": "dictionary",
    "StarType": "star_type",
    "starType": "star_type",
    "UnlockableItemType": "unlocable_item_type",
    "FleetPlan": "fleet_plan",
    "MeterTypeMeterMap": "meter_type_meter_map",
    "PairIntInt_IntMap": "pair_int_int_int_map",
    "MonsterFleetPlan": "monster_fleet_plan",
    "ShipPartMeterMap": "ship_part_meter_map",
    "ShipSlotVec": "ship_slot_vec",
    "special": "special",
    "IntFltMap": "int_flt_map",
    "ruleType": "rule_type",
    "influenceQueueElement": "influence_queue_element",
}


def report_once_and_register(argument_type):
    error("Can't find proper name for: %s, please add it to name mapping \n" % argument_type)
    normalization_dict[argument_type] = "arg"


def normalize_name(tp):
    argument_type, provided_name = tp
    if not provided_name.startswith("arg"):
        return provided_name
    if argument_type not in normalization_dict:
        report_once_and_register(argument_type)
    return normalization_dict[argument_type]
