from pathlib import Path
from subprocess import getoutput


def get_branch():
    return getoutput(["git", "rev-parse", "--abbrev-ref", "HEAD"]).replace("/", "-")


def get_main_folder() -> Path:
    return Path(__file__).parent.parent.parent


def get_base_dump_folder():
    return Path(__file__).parent / "dumps"
