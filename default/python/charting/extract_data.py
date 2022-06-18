from common.statistic_interface import StatKey


def start_turn(key, value, data):
    data["turns"].append({})


def add_attribute(key: StatKey, value, data):
    info = data["turns"][-1]
    if key.is_multi():
        info.setdefault(key.name, []).append(value)
    else:
        info[key.name] = value


def start_first_turn(key, value, data):
    start_turn(key, value, data)
    data["empire_id"] = value["empire_id"]
    data["player_name"] = value["name"]


def set_color(key, value, data):
    data["color"] = value


def process_output(key, value, data):
    info = data["turns"][-1]
    info["turn"] = value["turn"]
    info["PP"] = value["PP"]
    info["RP"] = value["RP"]


def skip(key, value, data):
    pass


DEFAULT = object()

states = {
    "new": {StatKey.EmpireID: (start_first_turn, "first_turn")},
    "first_turn": {
        StatKey.EmpireColors: (set_color, "turn"),
    },
    "turn": {
        StatKey.EmpireID: (start_turn, "turn"),
        StatKey.EmpireColors: (skip, "turn"),
        DEFAULT: (add_attribute, "turn"),
        StatKey.Output: (process_output, "turn"),
    },
}


def process_state(lines):
    data = {"turns": []}

    state = "new"
    for key, value in lines:
        handlers = states[state]
        if key in handlers:
            handler, state = handlers[key]
        else:
            handler, state = handlers[DEFAULT]
        handler(key, key.value.deserialize(value), data)
    return data
