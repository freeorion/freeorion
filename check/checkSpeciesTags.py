#!/usr/bin/env python3
import re
from copy import copy
from pathlib import Path
from sys import argv

re_tags = re.compile(r"^ *tags *= *\[")
re_start_effects = re.compile(r"^ *effectsgroups *= *\[")
re_start_foci = re.compile(r"^ *foci *= *\[")
re_end_list = re.compile(r"^ *\]")
re_macro = re.compile(r"^ *\[\[([A-Z_]+)\]\]")
issues_found = False
# Commented out are not used by the AI, most likely they never will.
# For tags that are implemented as species effects, only the AI
# is using the tags.
skills_to_check = [
    "INDUSTRY",
    "RESEARCH",
    "INFLUENCE",
    # "STOCKPILE",
    "POPULATION",
    "HAPPINESS",
    "SUPPLY",
    "OFFENSE_TROOPS",
    # "DEFENSE_TROOPS",
    "WEAPONS",
    "DETECTION",
    "STEALTH",
    "FUEL",
]
foci_to_check = [
    "INDUSTRY",
    "RESEARCH",
    "INFLUENCE",
]


def check_skill(name):
    if name.startswith(("AVERAGE", "PEDIA", "HAEMAESTHETIC", "INFINITE")):
        return ""
    for skill in skills_to_check:
        if name.endswith(skill):
            return skill
    return ""


def check_species(file):  # noqa: C901
    def issue(text):
        global issues_found
        issues_found = True
        print(f"{file.name}: {text}")

    effects = False
    foci = False
    tags = set()
    foci_list = set(foci_to_check)
    with open(file, encoding="utf-8") as f:
        for line in f.readlines():
            if re_tags.match(line):
                strings = line.split('"')
                for i in range(1, len(strings), 2):
                    if check_skill(strings[i]):
                        tags.add(strings[i])
                alltags = copy(tags)
            if re_start_effects.match(line):
                effects = True
            if re_start_foci.match(line):
                foci = True
            if effects or foci:
                r = re_macro.match(line)
                if r:
                    value = r.groups()[0]
                    if effects:
                        skill = check_skill(value)
                        if skill:
                            if value in tags:
                                tags.remove(value)
                            else:
                                issue("%s in effectgroups, missing in tags" % value)
                    else:
                        focus = value.split("_")[1]
                        if focus in foci_list:
                            foci_list.remove(focus)
                            if ("NO_" + focus) in alltags:
                                issue(f"{value}, but not tag NO_{focus}")
                elif re_end_list.match(line):
                    effects = False
                    foci = False
    for focus in foci_list:
        if ("NO_" + focus) not in alltags:
            issue(f"Not HAS_{focus}_FOCUS, NO_{focus} missing in tags")
    if tags:
        for tag in tags:
            issue("%s in tags, missing in effectgroups" % tag)


if __name__ == "__main__":
    if len(argv) != 2:
        print("Usage: %s <species file|directory contains species files>" % argv[0])
        exit(1)
    path = Path(argv[1])
    if path.is_dir():
        for f in path.iterdir():
            if f.name.startswith("SP_") and f.name.endswith(".txt"):
                check_species(f)
    else:
        check_species(path)
    exit(int(issues_found))
