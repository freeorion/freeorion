import os
import sys
from pathlib import Path
from shutil import rmtree

from check.focs_dump_parser._tools import get_base_dump_folder, get_branch
from check.focs_dump_parser.make_diff import report_diff
from check.focs_dump_parser.parser import parse_buildings


def is_windows():
    return sys.platform.startswith("win")


def get_log_file():
    if is_windows():
        app_data = Path(os.getenv("APPDATA"))
        return app_data / "FreeOrion/AI_1.log"
    else:
        raise NotImplementedError("Where logs are located?")


def main():
    branch = get_branch()

    dump_folder = get_base_dump_folder() / branch
    rmtree(dump_folder, ignore_errors=True)
    dump_folder.mkdir(parents=True, exist_ok=True)

    with get_log_file().open() as f:
        all_buildings = parse_buildings(f)

    for name, content in all_buildings:
        with open(dump_folder / f"{name}.txt", "w") as f:
            f.write(content)

    stats = f"Buildings parsed: {len(all_buildings)}"

    if branch != "master":
        diff_stats = report_diff()
    else:
        diff_stats = {}

    print(stats)
    for k, v in diff_stats.items():
        print(f"{k}({len(v)}): {','.join(v)}")


if __name__ == "__main__":
    main()
