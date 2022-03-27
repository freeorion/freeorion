import os
from pathlib import Path

excluded_paths = tuple(
    os.path.join(*x)
    for x in (
        ("python", "AI"),
        ("python", "universe_generation"),
        ("python", "turn_events"),
        ("python", "common", "configure_logging.py"),
        ("python", "common", "charting"),
        ("python", "tests", "fixtures"),
        ("python", "tests", "htmlcov"),
        ("python", "auth"),
        ("python", "chat"),
        (".pyi",),
        (".txt",),
        (".pytest_cache",),
        (".md",),
        (".coverage",),
        (".ini",),
    )
)


def pytest_ignore_collect(path: Path):
    """
    Filter out modules that imports C++ bindings.

    With pytest --doctest-modules we try to import every module in the python folder.
    We create a list of exclusions to manually control files that should not be imported.

    Each exclusion is a suffix of the path. If path is directory all path in it will be e excluded.

    Keep in mind, that we have similar folder in python and tests,
    for example python/AI and python/tests/AI
    """
    if str(path).endswith(excluded_paths):
        return True
