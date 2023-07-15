"""
Check the files were changed and set GitHub output values
"""
import os
import sys
from argparse import ArgumentParser
from contextlib import contextmanager
from subprocess import getoutput
from typing import TextIO

from check_file_changed._detectors import detect_file_groups
from check_file_changed._workflow import Workflow, get_workflows


@contextmanager
def print_group(title: str):
    if os.environ.get("GITHUB_ACTIONS") == "true":
        print(f"::group::{title}")
    yield
    if os.environ.get("GITHUB_ACTIONS") == "true":
        print("::endgroup::")


def get_changed_files(change, base):
    show_change = ["git", "show", "-q", change]
    print("Change:", getoutput(show_change))  # pyright: ignore[reportGeneralTypeIssues]

    show_base = [
        "git",
        "show",
        "-q",
        base,
    ]
    print("Base:", getoutput(show_base))  # pyright: ignore[reportGeneralTypeIssues]

    cmd = ["git", "diff-tree", "--name-only", "-r", change, base]
    return getoutput(cmd).split("\n")  # pyright: ignore[reportGeneralTypeIssues]


def write_workflow_output(file: TextIO, workflows: set[Workflow]):
    for workflow in workflows:
        file.write(f"{workflow.name}=true")


def check_changed_files(*, change_ref, base_ref, dry_run):
    with print_group("Getting changed files"):
        changed_files = get_changed_files(change_ref, base_ref)
        print("\n".join(changed_files))
    changed_file_groups = detect_file_groups(changed_files)
    with print_group("Detected file Groups"):
        print("\n".join(x.name for x in changed_file_groups))

    affected_workflows = get_workflows(changed_file_groups)

    print("Changed workflows:", ", ".join(x.name for x in sorted(affected_workflows, key=lambda x: x.name)))

    if dry_run:
        write_workflow_output(sys.stdout, affected_workflows)
    else:
        output_path = os.environ["GITHUB_OUTPUT"]
        with open(output_path, "a") as f:
            write_workflow_output(f, affected_workflows)

    print("Done")


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--change-ref", default="HEAD")
    parser.add_argument("--base-ref", default="HEAD^1")
    parser.add_argument("--dry-run", action="store_true")

    args = parser.parse_args()
    check_changed_files(change_ref=args.change_ref, base_ref=args.base_ref, dry_run=args.dry_run)
