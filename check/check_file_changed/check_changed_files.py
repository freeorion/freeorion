"""
Check the files were changed and set GitHub output values
"""
import os
import sys
from argparse import ArgumentParser
from subprocess import getoutput
from typing import TextIO

from check.check_file_changed._detectors import detect_file_groups
from check.check_file_changed._workflow import Workflow, get_workflows


def get_changed_files(change, base):
    return getoutput(["git", "diff-tree", "--name-only", "-r", change, base]).split("\n")


def write_workflow_output(file: TextIO, workflows: set[Workflow]):
    for workflow in workflows:
        file.write(f"{workflow.name}=true")


def check_changed_files(*, change_ref, base_ref, dry_run):
    changed_files = get_changed_files(change_ref, base_ref)
    changed_file_groups = detect_file_groups(changed_files)
    affected_workflows = get_workflows(changed_file_groups)

    print("Changed workflows:", ", ".join(x.name for x in sorted(affected_workflows)))

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

    args = parser.parse_args(["--base-ref", "50771b6ea5a6753ebe514b936996954582a937df", "--dry-run"])
    check_changed_files(change_ref=args.change_ref, base_ref=args.base_ref, dry_run=args.dry_run)
