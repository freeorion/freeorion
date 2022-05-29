import re

from dump_interface import DumpKey, DumpType


def parse_empire(value):
    """
    2 Name: Binding_2_pid_2_AI_1_RIdx_4_Aggressive Turn: 8
    :param value:
    :return:
    """

    match = re.match(r"(?P<id>\d+) Name: (?P<name>[a-zA-Z0-9_]+) Turn: \d+", value)
    keys = match.groupdict()

    return (
        int(keys["id"]),
        keys["name"],
    )


def parse_color(value):
    return tuple(int(chunk) for chunk in value.strip().split())


def start_turn(key, type_, value, data):
    data["turns"].append({})


def add_attribute(key, type_, value, data):
    info = data["turns"][-1]
    info[key.name] = _as_type(type_, value)


def _as_type(type_: DumpType, value):
    if type_ is DumpType.int:
        return int(value)
    else:
        return value


def start_first_turn(key, type_, value, data):
    empire_id, name = parse_empire(value)
    start_turn(key, type_, value, data)
    data["empire"] = empire_id
    data["player"] = name


def set_color(key, type_, value, data):
    data["color"] = parse_color(value)


def parse_output(value):
    """
    turn: 3 RP: 5.0 PP: 16.0
    """
    match = re.match(r"turn:\s+(?P<turn>\d+) RP: (?P<rp>[0-9.]+) PP: (?P<pp>[0-9.]+)", value)
    keys = match.groupdict()
    return (
        int(keys["turn"]),
        float(keys["rp"]),
        float(keys["pp"]),
    )


def process_output(key, type_, value, data):
    """
    data = {"PP": [], "RP": [], "RP_Ratio": [], "ShipCount": [], "turnsP": [], "turnPP": [], "PP + 2RP": []}
    details = {"color": {1, 1, 1, 1}, "name": "", "species": ""}
    :param key:
    :param type_:
    :param value:
    :param data:
    :return:
    """

    (
        turn,
        rp,
        pp,
    ) = parse_output(value)
    info = data["turns"][-1]
    info["turn"] = turn
    info["PP"] = pp
    info["RP"] = rp
    info["RP_Ratio"] = rp / pp
    info["PP + 2RP"] = pp + 2 * rp


def skip(key, type_, value, data):
    pass


states = {
    "new": {DumpKey.EmpireID: (start_first_turn, "first_turn")},
    "first_turn": {
        DumpKey.EmpireColors: (set_color, "turn"),
    },
    "turn": {
        DumpKey.EmpireID: (start_turn, "turn"),
        DumpKey.EmpireColors: (skip, "turn"),
        DumpKey.SHIP_CONT: (add_attribute, "turn"),
        DumpKey.Output: (process_output, "turn"),
    },
}


def process_state(lines):
    data = {"turns": []}

    state = "new"
    for key, type_, value in lines:
        handlers = states[state]
        if key in handlers:
            handler, state = handlers[key]
            handler(key, type_, value, data)
        else:
            pass
    return data
