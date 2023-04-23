"""
Script to set GitHub env [1] for optimising further steps.

Variables written here could be referenced with ${{ env.SHOULD_UPLOAD_BINARIES }}
This script is written in Python to avoid using the shell script on Windows.
To make it work, this script should be called after the actions/checkout.


[1] https://docs.github.com/en/actions/using-workflows/workflow-commands-for-github-actions#example-of-writing-an-environment-variable-to-github_env
"""

import os
import sys
from argparse import ArgumentParser
from typing import Callable


def set_cmake_windows_build_flags() -> dict:
    is_pull_request = os.environ["GITHUB_EVENT_NAME"] == "pull_request"
    is_push = os.environ["GITHUB_EVENT_NAME"] == "pull"
    current_branch = os.environ["GITHUB_REF_NAME"]
    push_to_master_master = is_push and current_branch == "master"
    push_to_weekly_build = is_push and current_branch == "weekly-test-builds"
    should_build = is_pull_request or push_to_master_master or push_to_weekly_build

    return {
        "SHOULD_UPLOAD_WEEKLY_BUILD": "true" if push_to_weekly_build else "",
        "SHOULD_UPLOAD_BINARIES": "true" if push_to_master_master else "",
        "SHOULD_BUILD": should_build,
    }


_group_setter = {"CMAKE_WINDOWS_BUILD": set_cmake_windows_build_flags}

_flag_group = {
    "SHOULD_UPLOAD_WEEKLY_BUILD": "CMAKE_WINDOWS_BUILD",
    "SHOULD_UPLOAD_BINARIES": "CMAKE_WINDOWS_BUILD",
    "SHOULD_BUILD": "CMAKE_WINDOWS_BUILD",
}


def get_flag_setter(name) -> Callable[[], dict]:
    """
    Get a setter for a flag.

    To resolve flag dependencies we use two level dispatching, first flag to group,
    and second group to setter, that will handle full flag group.
    """

    if name not in _flag_group:
        allowed = "\n".join(_flag_group)
        sys.stderr.write(f"Unknown flag: {name}, please use one of supported flags:\n{allowed}")
        exit(1)
    group_name = _flag_group[name]
    return _group_setter[group_name]


def get_flags(flag_names) -> dict:
    # fail on missed flag
    groups = {get_flag_setter(flag) for flag in flag_names}

    result_flags = {}

    for name in groups:
        result_group_flags = _group_setter[name]()
        result_flags.update(result_group_flags)
    return result_flags


def write_flags(flag_values):
    env_file = os.environ["GITHUB_ENV"]
    print("Setting env flags, you could access them as '${{env.<flag_name>}}`")
    with open(env_file, "a") as myfile:
        for name, value in flag_values.items():
            if value:
                print(f" - '{name}': '{value}'")
                myfile.write(f"{name}={value}")


def set_flags(flag_names):
    flags_values = get_flags(flag_names)
    write_flags(flags_values)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("flags", nargs="+")
    args = parser.parse_args()
    flags = args.flags
    set_flags(flags)
