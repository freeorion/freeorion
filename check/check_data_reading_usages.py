"""
Script to check presence of reading game constants.
"""
from pathlib import Path

from check.check_presense import check_presence

if __name__ == "__main__":
    base = Path(__file__).parent.parent

    paths = [
        base / "default" / "python" / "AI",
    ]
    exclude = {
        base / "default" / "python" / "AI" / "freeorion_tools" / "_freeorion_tools.py",
    }
    attrs = {
        "get_named_real",
        "get_named_int",
        "namedRealDefined",
        "namedIntDefined",
        "getNamedReal",
        "getNamedInt",
    }
    result = check_presence(raw_paths=paths, attrs=attrs, exclude=exclude)

    if result is False:
        exit(1)
    else:
        exit(0)
