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

    empires = sorted(item["player"] for item in ais)

    for item in ais:
        name = item["player"]

        empire_dict = plot_data.setdefault(name, {})
        for turn_info in item["turns"]:
            turn = turn_info["turn"]
            value = turn_info[attribute]
            empire_dict[str(turn)] = value

    chart_data = pd.DataFrame(plot_data).rename_axis("turn").reset_index()

    turn_order = sorted(chart_data["turn"].values, key=int)

    st.altair_chart(
        alt.Chart(chart_data)
        .transform_fold(empires, as_=["empire", attribute])
        .mark_line()
        .encode(
            x=alt.X(
                "turn",
                sort=turn_order,
                axis=alt.Axis(
                    grid=True,
                ),
            ),
            y=f"{attribute}:Q",
            color=alt.Color("empire:N", sort=empires),
        )
    )


def configure_colors(data):
    colors = sorted((item["player"], item["color"]) for item in data)
    theme_colors = [f"#{r:02X}{g:02X}{b:02X}" for _, (r, g, b, a) in colors]

    def empire_theme():
        return {"config": {"range": {"category": {"scheme": theme_colors}}}}

    alt.themes.register("empire_theme", empire_theme)
    alt.themes.enable("empire_theme")


def draw_plots(data):
    configure_colors(data)

    for param in ["PP", "RP", "SHIP_CONT"]:
        plot_param(data, attribute=param)


draw_plots(get_ais())
