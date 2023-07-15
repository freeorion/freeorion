# pyright: reportMissingImports=false
import pytest
from check.check_file_changed._detectors import FileGroup, detect_file_groups, registered_detectors


def flat_examples():
    examples = []
    for detector in registered_detectors:
        for path in detector.examples():
            examples.append(pytest.param(detector, path, id=f"{detector.file_type().name}-{path}"))
    return examples


@pytest.mark.parametrize(
    ("detector", "example"),
    flat_examples(),
)
def test_classes_could_define_their_examples(detector, example):
    assert detect_file_groups([example]) == {detector.file_type()}


def test_number_of_checkers_matches_enums():
    assert {x.file_type() for x in registered_detectors} == set(FileGroup)


@pytest.mark.parametrize(
    ("detector"),
    [pytest.param(detector, id=detector.file_type().name) for detector in registered_detectors],
)
def test_all_checkers_has_examples(detector):
    examples = detector.examples()
    assert len(examples) > 0
