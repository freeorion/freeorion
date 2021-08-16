import pytest
from freeorion_tools import combine_ratings


@pytest.mark.parametrize(
    ("rating1", "raring2", "combined"),
    (
        (1, 1, pytest.approx(4.0)),
        (2, 1, pytest.approx(5.82, rel=1e-2)),
        (2, 2, pytest.approx(8.0)),
        (4, 2, pytest.approx(11.65, rel=1e-2)),
    ),
)
def test_two_rating_are_merged(rating1, raring2, combined):
    assert combine_ratings(rating1, raring2) == combined


def test_merge_3_ratings():
    assert combine_ratings(1, 1, 2) == pytest.approx(11.65, rel=1e-2)


@pytest.mark.parametrize(
    ("iterable"),
    (
        (1, 1, 2),
        iter((1, 1, 2)),
        [1, 1, 2],
    ),
)
def test_merge_3_ratings_as_collection(iterable):
    assert combine_ratings(iterable) == pytest.approx(11.65, rel=1e-2)


def test_merge_3_ratings_as_collection_and_args():
    assert combine_ratings([1, 1], 2) == pytest.approx(11.65, rel=1e-2)


def test_idempotency():
    assert combine_ratings(1, 1, 2) == combine_ratings(2, 1, 1) == combine_ratings(1, 2, 1)
