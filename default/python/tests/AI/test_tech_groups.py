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


@pytest.fixture()
def tech_list():
    class TestGroup(TechsListsAI.TechGroup):
        def __init__(self):
            super().__init__()
            self.economy.extend(
                [
                    "ECO_1",
                    "ECO_2",
                ]
            )
            self.weapon.extend(
                [
                    "WEP_1",
                    "WEP_2",
                ]
            )
            self.defense.extend(
                [
                    "DEF_1",
                    "DEF_2",
                ]
            )
            self.hull.extend(
                [
                    "HULL_1",
                    "HULL_2",
                ]
            )
            self.enqueue()

    return TestGroup()


def test_adding_rest_adds_them_in_order(tech_list):
    tech_list.enqueue()
    assert not tech_list.get_errors()

    expected = [
        "ECO_1",
        "WEP_1",
        "DEF_1",
        "HULL_1",
        "ECO_2",
        "WEP_2",
        "DEF_2",
        "HULL_2",
    ]
    assert expected == tech_list.get_techs()
