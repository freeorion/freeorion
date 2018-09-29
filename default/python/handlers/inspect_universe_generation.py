from common.listeners import register_post_handler
from stub_generator import inspect


def inspect_universe_generation_interface(*args, **kwargs):
    import freeorion as fo
    tech = fo.getTech('LRN_ARTIF_MINDS')
    universe = fo.get_universe()
    empire = fo.get_empire(1)
    rules = fo.getGameRules()
    hull_type = fo.getHullType('SH_XENTRONIUM')
    species = fo.getSpecies('SP_ABADDONI')
    inspect(
        fo,
        instances=[
            fo.getFieldType('FLD_ION_STORM'),
            fo.getBuildingType('BLD_SHIPYARD_BASE'),
            hull_type,
            hull_type.slots,
            fo.getPartType('SR_WEAPON_1_1'),
            fo.getSpecial('MODERATE_TECH_NATIVES_SPECIAL'),
            species,
            fo.diplomaticMessage(1, 2, fo.diplomaticMessageType.acceptPeaceProposal),
            rules,
            tech,
            tech.unlockedItems,
            rules.getRulesAsStrings,
            universe,
            universe.effectAccounting,
            universe.buildingIDs,
            fo.get_galaxy_setup_data(),
            empire,
            empire.colour,
            empire.productionQueue,
            empire.researchQueue,
        ],
        classes_to_ignore=(
            'IntBoolMap', 'IntDblMap', 'IntFltMap', 'IntIntMap', 'IntPairVec', 'IntSetSet',
            'MeterTypeAccountingInfoVecMap', 'MeterTypeMeterMap', 'MeterTypeStringPair', 'MonsterFleetPlan',
            'PairIntInt_IntMap', 'RuleValueStringStringPair', 'ShipPartMeterMap', 'VisibilityIntMap',
            'AccountingInfoVec', 'IntSet', 'StringSet', 'StringVec',
        ),
        path=".",
    )


register_post_handler("create_universe", inspect_universe_generation_interface)
