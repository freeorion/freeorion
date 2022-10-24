from collections import Counter, OrderedDict, defaultdict

import streamlit as st
from collect_data import get_ais_data  # noqa: E402
from widgets import DIFF_ADD, DIFF_REMOVE, colored_span, span_with_hint, to_hex_color


def gather_policies_data(ais_data):
    data = OrderedDict()
    existing_empires = {}
    summary_stats = defaultdict(Counter)

    for empire in ais_data:
        empire_id = empire["empire_id"]
        existing_empires[empire_id] = empire["color"]
        for turn in empire["turns"]:
            all_added = turn.get("PolicyAdoption", [])
            summary_stats[empire_id].update(Counter(all_added))
            all_removed = turn.get("PolicyDeAdoption", [])

            summary_stats[empire_id].update({k: -v for k, v in Counter(all_removed).items()})

            # policies may be added or removed more than once per turn
            added_or_removed = {}
            for added in all_added:
                added_or_removed[added] = added_or_removed.get(added, 0) + 1
            for removed in all_removed:
                added_or_removed[removed] = added_or_removed.get(removed, 0) - 1
            added = sorted([x for x, value in added_or_removed.items() if value > 0])
            removed = sorted([x for x, value in added_or_removed.items() if value < 0])

            turn = turn["turn"]

            data.setdefault(turn, {})[empire_id] = (added, removed)

    return data, existing_empires, summary_stats


def norm_policy_name(name):
    name = name.split("PLC_", 1)[-1]
    return name.lower().capitalize()


def to_policy_span(text, color):
    return colored_span(span_with_hint(norm_policy_name(text), text), color)


def show_summary(existing_empires, summary_stats):
    st.header("Policies adopted")
    summary = []
    for empire_id, counts in summary_stats.items():
        counts = sorted([name for name, added in counts.items() if added])
        counts = ", ".join(to_policy_span(x, DIFF_ADD) for x in counts)
        empire_id = colored_span(empire_id, to_hex_color(existing_empires[empire_id]))
        summary.append(f"- {empire_id}: {counts}")

    st.markdown("\n".join(summary), unsafe_allow_html=True)


def show_policies_table(data, existing_empires):
    header = " | ".join(colored_span(eid, to_hex_color(color)) for eid, color in sorted(existing_empires.items()))
    segments = " | ".join("---" for x in range(len(existing_empires) + 1))
    separator = f"| {segments} |"

    table = [
        f"| turn | {header} |",
        separator,
    ]

    is_gap = False

    for key, empires in data.items():
        has_changed = [bool(k) or bool(v) for k, v in empires.values()]
        if any(has_changed):
            is_gap = False

        else:
            if is_gap:
                continue
            else:
                is_gap = True

        def to_diff(val):
            added, removed = val

            return ", ".join(
                [*[to_policy_span(x, DIFF_ADD) for x in added], *[to_policy_span(x, DIFF_REMOVE) for x in removed]]
            )

        if is_gap:
            inner_row = " | ".join(["...", *[to_diff(v) for k, v in sorted(empires.items())]])
        else:

            inner_row = " | ".join([str(key), *[to_diff(v) for k, v in sorted(empires.items())]])

        table.append(f"| {inner_row} |")
    st.markdown("\n".join(table), unsafe_allow_html=True)


def plot_policy_adoptions(ais_data):
    data, existing_empires, summary_stats = gather_policies_data(ais_data)
    st.header("Policies")
    if not data:
        return

    show_policies_table(data, existing_empires)
    show_summary(existing_empires, summary_stats)


plot_policy_adoptions(get_ais_data())
