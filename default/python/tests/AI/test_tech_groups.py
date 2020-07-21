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


def test_1(tech_group_class, caplog):
    tech_group_class()
    head_message = caplog.records and caplog.records[0].message or None

    assert not head_message
