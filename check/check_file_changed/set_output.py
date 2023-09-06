"""
Check the files were changed and set GitHub output values.
"""
import os
from argparse import ArgumentParser
from collections.abc import Collection
from contextlib import contextmanager
from subprocess import check_output

from _file_checker import WorkflowName, read


def is_github():
    return os.environ.get("GITHUB_ACTIONS") == "true"


@contextmanager
def print_group(title: str):
    if is_github():
        print(f"::group::{title}")
    else:
        print(f"{title}:")
    yield
    print()
    if is_github():
        print("::endgroup::")


def print_list(items: Collection[str]):
    for i in sorted(items):
        print(f" - {i}")


def get_changed_files(change, base) -> list[str]:
    cmd = ["git", "diff-tree", "--name-only", "-r", change, base]
    output = check_output(cmd)
    return output.decode().split("\n")


def format_output(workflows: set[WorkflowName]):
    return [f"{workflow.upper()}=true" for workflow in workflows]


def check_changed_files(*, change_ref, base_ref):
    file_checker = read()

    with print_group("Getting changed files"):
        changed_files = get_changed_files(change_ref, base_ref)
        print("\n".join(changed_files))

    with print_group("Detected file Groups:"):
        detected_groups = (file_checker.get_detected_group(path) for path in changed_files)
        detected_groups = {g for g in detected_groups if g}
        print_list(detected_groups)

    affected_workflows = file_checker.get_workflows(detected_groups)

    print("Changed workflows:")
    print_list(affected_workflows)
    print()

    output_text = "\n".join(format_output(affected_workflows))
    with print_group("Generated output"):
        print(output_text)
        print()

    if is_github():
        output_path = os.environ["GITHUB_OUTPUT"]
        with open(output_path, "a") as f:
            f.write(output_text)
    print("Done")


if __name__ == "__main__":
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--change-ref", default="HEAD", help="The latest commit of PR")
    parser.add_argument("--base-ref", default="HEAD^1", help="The latest commit of target branch (master)")

    args = parser.parse_args()
    check_changed_files(change_ref=args.change_ref, base_ref=args.base_ref)
