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


def test_tech_group_integrity(tech_group_class, caplog):
    tech_group_class()
    messages = [record.message for record in caplog.records]
    assert not messages
