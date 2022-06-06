from dump_interface import DumpKey


def start_turn(key, value, data):
    data["turns"].append({})


def add_attribute(key, value, data):
    info = data["turns"][-1]
    info[key.name] = key.value.deserialize(value)


def start_first_turn(key, value, data):
    start_turn(key, value, data)
    data["empire"] = value["empire_id"]
    data["player"] = value["name"]


def set_color(key, value, data):
    data["color"] = value[:-1]


def process_output(key, value, data):
    info = data["turns"][-1]
    info["turn"] = value["turn"]
    info["PP"] = value["PP"]
    info["RP"] = value["RP"]


def skip(key, value, data):
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
    for key, value in lines:
        handlers = states[state]
        if key in handlers:
            handler, state = handlers[key]
            handler(key, key.value.deserialize(value), data)
        else:
            pass
    return data
