import os
import sys

import altair as alt
import pandas as pd
import streamlit as st
from widgets import colored_span, to_hex_color

current_dir = os.path.dirname(__file__)
common = os.path.join(current_dir, "..")
assert os.path.exists(common)
sys.path.append(common)

from collect_data import get_ais_data  # noqa: E402


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
    show_legend(ais_data)
    plot_params = ["PP", "RP", "SHIP_CONT"]
    for param in plot_params:
        plot_for_attribute(ais_data, attribute=param)


def setup_layout():
    st.set_page_config(layout="wide")


setup_layout()
draw_plots(get_ais_data())
