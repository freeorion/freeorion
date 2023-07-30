"""
Check the files were changed and set GitHub output values.
"""
import os
from argparse import ArgumentParser
from contextlib import contextmanager
from subprocess import check_output

from _detectors import detect_file_groups
from _workflow import Workflow, get_workflows


def is_github():
    return os.environ.get("GITHUB_ACTIONS") == "true"


@contextmanager
def print_group(title: str):
    if is_github():
        print(f"::group::{title}")
    else:
        print(title)
    yield
    print()
    if is_github():
        print("::endgroup::")


def get_changed_files(change, base):
    cmd = ["git", "diff-tree", "--name-only", "-r", change, base]
    output = check_output(cmd)
    return output.decode().split("\n")


def format_output(workflows: set[Workflow]):
    return [f"{workflow.name}=true" for workflow in workflows]


def check_changed_files(*, change_ref, base_ref):
    with print_group("Getting changed files"):
        changed_files = get_changed_files(change_ref, base_ref)
        print("\n".join(changed_files))

    with print_group("Detected file Groups:"):
        changed_file_groups = detect_file_groups(changed_files)
        print("\n".join(x.name for x in changed_file_groups))

    affected_workflows = get_workflows(changed_file_groups)

    print("Changed workflows:\n - %s" % "\n - ".join(x.name for x in sorted(affected_workflows, key=lambda x: x.name)))
    print()

    output_text = "\n".join(format_output(affected_workflows))

    print("Generated output:")
    print(output_text)

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
