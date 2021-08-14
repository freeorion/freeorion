from common.listeners import register_post_handler
from handlers.shared_instances_code import (
    classes_to_exclude_from_universe,
    get_common_instances,
    get_item_with_location,
)
from stub_generator import generate_stub


def inspect_universe_generation_interface(*args, **kwargs):
    import freeorion as fo

    universe = fo.get_universe()

    # this field should be visible to AI
    empire_of_first_ai = fo.get_empire(2)  # first AI
    fo.create_field_in_system("FLD_NEBULA_1", 100, universe.getPlanet(empire_of_first_ai.capitalID).systemID)
    instances = list(get_item_with_location(get_common_instances()))

    generate_stub(
        fo,
        instances=instances,
        classes_to_ignore=classes_to_exclude_from_universe,
        path=".",
    )


register_post_handler("create_universe", inspect_universe_generation_interface)
