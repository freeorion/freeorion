"""
Check which files in the repo are covered.

It will help to have visibility over all files in the repo.
"""
from argparse import ArgumentParser
from pathlib import PurePath
from subprocess import check_output

from _detectors import detect_file, registered_detectors


def list_all_files():
    cmd = ["git", "ls-tree", "--full-tree", "--name-only", "-r", "HEAD"]
    return check_output(cmd).decode().split("\n")


def check_coverage():
    all_files = list_all_files()

    missed_count = 0

    for path in all_files:
        result = detect_file(PurePath(path), registered_detectors)
        if result is None:
            missed_count += 1
            print(path)
    print(f"Missed statements: {missed_count} from {len(all_files)}")


if __name__ == "__main__":
    parser = ArgumentParser(description=__doc__)
    args = parser.parse_args()
    check_coverage()
