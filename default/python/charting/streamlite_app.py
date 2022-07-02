import os
import sys
from collections import Counter, OrderedDict, defaultdict

import altair as alt
import pandas as pd
import streamlit as st

current_dir = os.path.dirname(__file__)
common = os.path.join(current_dir, "..")
assert os.path.exists(common)
sys.path.append(common)

from collect_data import get_ais_data  # noqa: E402

DIFF_ADD = "#36CF70"
DIFF_REMOVE = "#A32929"

ANCHOR_ADOPTED_SUMMARY = "policies-adopted"
ANCHOR_POLICIES = "policies"


def to_hex_color(color):
    r, g, b = color["R"], color["G"], color["B"]
    return f"#{r:02X}{g:02X}{b:02X}"


def span_with_hint(text, hint):
    return f'<span title="{hint}">{text}</span>'


def norm_policy_name(name):
    name = name.split("PLC_", 1)[-1]
    return name.lower().capitalize()


def to_policy_span(text, color):
    return colored_span(span_with_hint(norm_policy_name(text), text), color)


def plot_for_attribute(ais_data, attribute):
    st.header(f"Distribution of the {attribute}", anchor=attribute)

    source, empires, theme_colors = get_source(ais_data, attribute)

    selection = alt.selection_multi(fields=["empire_id"], bind="legend")

    chart = (
        alt.Chart(source)
        .mark_line(interpolate="basis")
        .encode(
            x=alt.X("turn", axis=alt.Axis(grid=True)),
            y=f"{attribute}:Q",
            color=alt.Color("empire_id:N", sort=empires),
            opacity=alt.condition(selection, alt.value(1), alt.value(0.2)),
        )
        .configure_range(category=alt.RangeScheme(theme_colors))
        .add_selection(selection)
        .interactive()
    )

    st.altair_chart(chart, use_container_width=True)


def get_source(ais_data, attribute):
    empires = [str(item["empire_id"]) for item in ais_data]
    colors = sorted((item["empire_id"], item["color"]) for item in ais_data)
    theme_colors = [to_hex_color(color) for _, color in colors]
    plot_data = {}
    for item in ais_data:
        empire_id = str(item["empire_id"])

        empire_dict = plot_data.setdefault(empire_id, {})
        for turn_info in item["turns"]:
            turn = turn_info["turn"]
            value = turn_info[attribute]
            empire_dict[turn] = value
    wide_format = pd.DataFrame(plot_data).rename_axis("turn").reset_index()
    long_format = wide_format.melt("turn", var_name="empire_id", value_name=attribute)
    return long_format, empires, theme_colors


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

            really_removed = []

            for removed in all_removed:
                if removed in all_added:
                    all_added.remove(removed)
                else:
                    really_removed.append(removed)

            added = sorted(all_added)
            removed = sorted(really_removed)

            turn = turn["turn"]

            data.setdefault(turn, {})[empire_id] = (added, removed)

    return data, existing_empires, summary_stats


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


def show_summary(existing_empires, summary_stats):
    st.header("Policies adopted", anchor=ANCHOR_ADOPTED_SUMMARY)
    summary = []
    for empire_id, counts in summary_stats.items():
        counts = sorted([name for name, added in counts.items() if added])
        counts = ", ".join(to_policy_span(x, DIFF_ADD) for x in counts)
        empire_id = colored_span(empire_id, to_hex_color(existing_empires[empire_id]))
        summary.append(f"- {empire_id}: {counts}")

    st.markdown("\n".join(summary), unsafe_allow_html=True)


def plot_policy_adoptions(ais_data):
    data, existing_empires, summary_stats = gather_policies_data(ais_data)
    st.header("Policies")
    if not data:
        return

    show_policies_table(data, existing_empires)
    show_summary(existing_empires, summary_stats)


def colored_span(text, hex_color):
    return f'<span style="color:{hex_color}">{text}</span>'


def add_links_to_anchors(*anchors):
    result = []
    for anchor in anchors:
        norm_name = anchor.replace("-", " ").replace("_", " ").lower().capitalize()
        result.append(f"[{norm_name}](#{anchor})")
    text = f"{' | ' .join(result)}"
    st.markdown(text, unsafe_allow_html=True)


def show_legend(ais_data):
    with st.expander("Legend", expanded=False):
        body = []
        for item in ais_data:
            empire_id = item["empire_id"]
            player = item["player_name"]
            span = colored_span(player, to_hex_color(item["color"]))
            body.append(f"""- **{empire_id}**: {span}""")
        st.markdown("\n".join(body), unsafe_allow_html=True)


def draw_plots(ais_data):
    ais_data = sorted(ais_data, key=lambda x: int(x["empire_id"]))
    show_legend(ais_data)

    plot_params = ["PP", "RP", "SHIP_CONT"]
    add_links_to_anchors(ANCHOR_POLICIES, ANCHOR_ADOPTED_SUMMARY, *plot_params)

    plot_policy_adoptions(ais_data)

    for param in plot_params:
        plot_for_attribute(ais_data, attribute=param)


def setup_layout():
    st.set_page_config(layout="wide")


setup_layout()
draw_plots(get_ais_data())
