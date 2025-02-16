import re
from logging import error
from typing import IO


def _get_name_from_line(line):
    if "BuildingType " not in line or "BuildingsParser.cpp" not in line or "[trace]" not in line:
        return None
    body = line.rsplit(" : ", 2)[1]
    return body.removeprefix("BuildingType ")


_log_like = re.compile(r"\d{2}:\d{2}:\d{2}\.\d{4}.*")


def _is_regular_log(line):
    return _log_like.match(line)


def get_lines_with_dump(f: IO):
    iterator = iter(f)

    for line in iterator:
        if "Start parsing FOCS for BuildingTypes" in line:
            break
    for line in iterator:
        if "End parsing FOCS for BuildingTypes" in line:
            return
        yield line


def parse_buildings(f: IO):
    _result = []
    _current = []
    for line in get_lines_with_dump(f):
        next_section = _get_name_from_line(line)
        if next_section:
            if _current:
                _result.append((_current[0], "".join(_current[1])))
            _current = [next_section, []]
        else:
            if _is_regular_log(line):
                continue
            if _current:
                _current[1].append(line)

    if not _current:
        error("No data in log, check that game is run with 'parsing' set to 'trace'")
        exit(1)

    _result.append((_current[0], "".join(_current[1])))

    return _result
