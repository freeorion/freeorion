from common.listeners import register_post_handler
from stub_generator import generate_stub


def inspect_universe_generation_interface(*args, **kwargs):
    import freeorion as fo
    tech = fo.getTech('LRN_NASCENT_AI')
    universe = fo.get_universe()
    empire = fo.get_empire(1)
    rules = fo.getGameRules()
    ship_hull = fo.getShipHull('SH_XENTRONIUM')
    species = fo.getSpecies('SP_ABADDONI')
    generate_stub(
        fo,
        instances=[
            fo.getFieldType('FLD_ION_STORM'),
            fo.getBuildingType('BLD_SHIPYARD_BASE'),
            ship_hull,
            ship_hull.slots,
            fo.getShipPart('SR_WEAPON_1_1'),
            fo.getSpecial('MODERATE_TECH_NATIVES_SPECIAL'),
            species,
            fo.diplomaticMessage(1, 2, fo.diplomaticMessageType.acceptPeaceProposal),
            rules,
            tech,
            tech.unlockedItems,
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
        dump=False,
    )


register_post_handler("create_universe", inspect_universe_generation_interface)
