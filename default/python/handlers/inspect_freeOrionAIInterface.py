from common.listeners import register_pre_handler
from handlers.shared_instances_code import (
    classes_to_exclude_from_ai,
    get_common_instances,
    get_item_with_location,
)
from stub_generator import generate_stub


def collect_ai_instances():
    import freeOrionAIInterface as fo

    yield from get_common_instances()

    empire_of_first_ai = fo.getEmpire(2)

    prod_queue = empire_of_first_ai.productionQueue
    fo.issueEnqueueShipProductionOrder(list(empire_of_first_ai.availableShipDesigns)[0], empire_of_first_ai.capitalID)

    yield prod_queue
    yield prod_queue[0]

    research_queue = empire_of_first_ai.researchQueue
    fo.issueEnqueueTechOrder("SHP_WEAPON_1_2", -1)
    yield research_queue
    yield research_queue[0]


def inspect_ai_interface():
    # Put all related imports inside
    import freeOrionAIInterface as fo

    instances = list(get_item_with_location(collect_ai_instances()))

    generate_stub(
        fo,
        instances=instances,
        classes_to_ignore=classes_to_exclude_from_ai,
        path="AI",
    )
    exit(1)  # exit game to main menu, gameplay may be broken due to invasive instance generation.


register_pre_handler("generateOrders", inspect_ai_interface)
