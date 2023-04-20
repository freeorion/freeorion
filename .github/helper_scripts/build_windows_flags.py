"""
Script to set GitHub env [1] for optimising further steps.

Variables written here could be referenced with ${{ env.SHOULD_UPLOAD_BINARIES }}
This script is written in Python to avoid using the shell script on Windows.
To make it work, this script should be called after the actions/checkout.


[1] https://docs.github.com/en/actions/using-workflows/workflow-commands-for-github-actions#example-of-writing-an-environment-variable-to-github_env
"""

import os


def get_flags() -> dict:
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


def write_flags(flags):
    env_file = os.environ["GITHUB_ENV"]
    with open(env_file, "a") as myfile:
        for name, value in flags.items():
            if value:
                myfile.write(f"{name}={value}")


def set_flags():
    flags = get_flags()
    write_flags(flags)


if __name__ == "__main__":
    set_flags()
