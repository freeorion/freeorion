import difflib
import os
import re
from collections import defaultdict
from pathlib import Path

from check.focs_dump_parser._tools import get_base_dump_folder, get_branch

_negative_number = re.compile(r"\(-\(?\d+\.?\d*\)\)")


def read_file(path: Path):
    if not path.exists():
        return []
    else:
        with path.open() as f:
            return f.readlines()


def prettify_dump(text):
    text = text.replace("(0 = LocalCandidate.TurnsSinceFocusChange)", "(LocalCandidate.TurnsSinceFocusChange = 0)")

    def replacer(match):
        return match.group(0).replace("(", "").replace(")", "")

    return _negative_number.sub(replacer, text)


def report_diff():
    base = get_base_dump_folder()

    a_branch = "master"

    b_branch = get_branch()

    path_a = base / a_branch
    path_b = base / b_branch

    files_a = os.listdir(path_a)
    files_b = os.listdir(path_b)

    all_files = {*files_a, *files_b}

    clean_species = []

    stats = defaultdict(list)

    for name in all_files:
        content_a = read_file(path_a / name)
        content_b = read_file(path_b / name)

        name = name.removesuffix(".txt")

        a = [prettify_dump(x) for x in content_a]
        b = [prettify_dump(x) for x in content_b]

        if a == b == []:
            stats["both empty"].append(name)
        if a == b:
            stats["same"].append(name)
        elif content_a == []:
            stats["file mismatch"].append(f"{name}.{a_branch} is missed")
        elif content_b == []:
            stats["file mismatch"].append(f"{name}.{b_branch} is missed")

        else:
            stats["diff"].append(name)
            for line in difflib.unified_diff(a, b, f"{name}.{a_branch}", f"{name}.{b_branch}"):
                print(line, end="")

    print(clean_species)
    print(f"Finish comparing {len(all_files)} files")
    return stats


if __name__ == "__main__":
    report_diff()
