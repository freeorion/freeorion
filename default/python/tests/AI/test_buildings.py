import pytest
from buildings import Shipyard


def test_get_shipyard_by_calling_constructor():
    assert Shipyard.GEO == Shipyard("BLD_SHIPYARD_CON_GEOINT")


def test_invalid_shipyard_raises_value_error_if_passed_via_constructor():
    with pytest.raises(ValueError, match="'XXX' is not a valid Shipyard"):
        Shipyard("XXX")
