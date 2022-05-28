"""
# My first app
Here's our first attempt at using data to create a table:
"""

import altair as alt
import pandas as pd
import streamlit as st
from collect_data import get_ais


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

    alt.Chart(chart_data).mark_line().encode(x="x", y="f(x)")


def draw_plots():
    for param in ["PP", "RP", "SHIP_CONT"]:
        plot_param(get_ais(), attribute=param)


st.button("Draw", "l", "Show diagrams", draw_plots)
