import ast
import os
from argparse import ArgumentParser
from collections.abc import Iterator
from pathlib import Path
from time import monotonic


def _check_presence(path: Path, attributes: set[str]) -> Iterator[tuple[str, int, int]]:
    with path.open() as f:
        tree = ast.parse(f.read(), filename=path.name)

    for node in ast.walk(tree):
        candidates = (
            node.id if node.__class__.__name__ == "Name" else "",
            node.attr if node.__class__.__name__ == "Attribute" else "",
            node.name if node.__class__.__name__ in ("FunctionDef", "ClassDef") else "",
        )
        for candidate in candidates:
            if candidate in attributes:
                yield candidate, node.lineno, node.col_offset


def _resolve_folders(p: Path) -> Iterator[Path]:
    if p.is_file() and p.name.endswith(".py"):
        yield p
    elif p.is_dir():
        for base, _, files in os.walk(p):
            for f in files:
                if f.endswith(".py"):
                    yield Path(base) / f
    else:
        pass


def _get_time_string(start: float) -> str:
    seconds = monotonic() - start
    return f"{seconds:.2f} seconds"


def check_presence(*, raw_paths: list[Path], attrs: set[str], exclude: set[Path]) -> bool:
    start = monotonic()

    paths = [Path(f) for p in raw_paths for f in _resolve_folders(Path(p)) if p not in exclude]

    cases = []
    for p in paths:
        for attr, line, offset in _check_presence(p, attrs):
            cases.append(f"{p}:{line}:{offset}: {attr}")

    if cases:
        for c in cases:
            print(c)
        print(f"Checked {len(paths)}, found: {len(cases)} errors in {_get_time_string(start)}")
        return False
    else:
        print(f"Checked {len(paths)}, no errors found in {_get_time_string(start)}")
        return True


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--attr", action="append", help="An attribute you want to track, multiple keys allowed")
    parser.add_argument("--exclude", action="append", nargs="+", help="Exclude files")
    parser.add_argument("files", nargs="+", help="files to check")
    args = parser.parse_args()

    raw_paths = [Path(p) for p in args.files]
    exclude = {Path(e) for e in args.exclude} if args.exclude else set()

    attributes = set(args.attr)
    result = check_presence(raw_paths=raw_paths, attrs=attributes, exclude=exclude)
    if result is False:
        exit(1)
    else:
        exit(0)
