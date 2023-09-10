"""
Check which files in the repo are covered.

It will help to have visibility over all files in the repo.
"""
from argparse import ArgumentParser
from subprocess import check_output

from _file_checker import FileChecker, read


def list_all_files():
    cmd = ["git", "ls-tree", "--full-tree", "--name-only", "-r", "HEAD"]
    return check_output(cmd).decode().strip().split("\n")


def check_patterns(config: FileChecker):
    unused_patterns = _get_unused_patterns(config)

    if not unused_patterns:
        print("All patterns are in use")
        return

    print(f"Found {len(unused_patterns)} unused patterns")
    for group, pattern in unused_patterns:
        print(f" - {group:<20}: {pattern}")


def _get_unused_patterns(config):
    unused_patterns = []
    for group in config.groups:
        for pattern, count in group.matched.items():
            if count == 0:
                unused_patterns.append((group.name, pattern))
    return unused_patterns


def check_coverage():
    file_checker = read()

    all_files = list_all_files()

    missed_count = 0
    missed_files = []

    for path in all_files:
        result = file_checker.get_detected_group(path)
        if result:
            workflows = ", ".join(sorted(file_checker.get_workflows([result]))).upper()
            print(f"  {path:<60}  {result.upper():<20} {workflows}")
        else:
            missed_count += 1
            missed_files.append(path)
    print(f"Missed files: {missed_count} from {len(all_files)}")
    for f in missed_files:
        print(" ", f)

    check_patterns(file_checker)


if __name__ == "__main__":
    parser = ArgumentParser(description=__doc__)
    args = parser.parse_args()
    check_coverage()
