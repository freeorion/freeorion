import pytest

import TechsListsAI


def get_tech_lists():
    techs = []
    for attr in dir(TechsListsAI):
        val = getattr(TechsListsAI, attr)
        if isinstance(val, type) and issubclass(val, TechsListsAI.TechGroup):
            techs.append(val)
    return techs


@pytest.fixture(scope="module", params=get_tech_lists())
def tech_group_class(request):
    yield request.param


def test_tech_group_integrity(tech_group_class):
    errors = tech_group_class().get_errors()
    assert not errors
