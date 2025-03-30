import difflib
import os
import re
from collections import defaultdict
from pathlib import Path

from check.focs_dump_parser._tools import get_base_dump_folder, get_branch, get_main_folder

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

        a_relative_name = (path_a / (name + ".txt")).relative_to(get_main_folder()).as_posix()
        b_relative_name = (path_b / (name + ".txt")).relative_to(get_main_folder()).as_posix()

        if a == b == []:
            stats["both empty"].append(name)
        if a == b:
            stats["same"].append(name)
        elif content_a == []:
            stats["no pair for the file"].append(f"{name} is missed in branch {a_branch}")
        elif content_b == []:
            stats["no pair for for file"].append(f"{name} is missed in branch {b_branch}")

        else:
            stats["diff"].append(name)
            for line in difflib.unified_diff(a, b, a_relative_name, b_relative_name):
                print(line, end="")

    print(clean_species)
    print(f"Finish comparing {len(all_files)} files")
    return stats


if __name__ == "__main__":
    report_diff()
