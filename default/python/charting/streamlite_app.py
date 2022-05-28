import os
import sys

import altair as alt
import pandas as pd
import streamlit as st

current_dir = os.path.dirname(__file__)
common = os.path.join(current_dir, "..", "common")
sys.path.append(common)


from collect_data import get_ais  # noqa: E402


def plot_param(ais, attribute):
    plot_data = {}
    f"Distribution of the {attribute}"

    for item in ais:

        name = item["player"]
        id_ = item["empire"]

        f"{id_}: {name}"
        user_turns = plot_data.setdefault(id_, {})
        for turn_info in item["turns"]:
            turn = turn_info["turn"]
            user_turns[turn] = turn_info[attribute]

    chart_data = pd.DataFrame(plot_data)

    st.altair_chart(
        alt.Chart(chart_data)
        .mark_line()
        .encode(x=alt.X("turn", timeUnit="int", title="turn"), y=alt.Y(attribute, timeUnit="int", title=attribute))
    )


def draw_plots(data):
    for param in ["PP"]:  # , "RP", "SHIP_CONT"]:
        plot_param(data, attribute=param)


data = get_ais()
draw_plots(data)
