import os
import sys

import altair as alt
import pandas as pd
import streamlit as st

current_dir = os.path.dirname(__file__)
common = os.path.join(current_dir, "..")
assert os.path.exists(common)
sys.path.append(common)

from collect_data import get_ais_data  # noqa: E402


def to_hex_color(color):
    r, g, b = color["R"], color["G"], color["B"]
    return f"#{r:02X}{g:02X}{b:02X}"


def plot_for_attribute(ais_data, attribute):
    st.markdown(f"**Distribution of the {attribute}**")

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
            empire_dict[str(turn)] = value

    wide_format = pd.DataFrame(plot_data).rename_axis("turn").reset_index()
    long_format = wide_format.melt("turn", var_name="empire_id", value_name=attribute)

    turn_order = sorted(wide_format["turn"].values, key=int)

    st.altair_chart(
        alt.Chart(long_format)
        .mark_line()
        .encode(
            x=alt.X(
                "turn",
                sort=turn_order,
                axis=alt.Axis(
                    grid=True,
                ),
            ),
            y=alt.Y(f"{attribute}:Q"),
            color=alt.Color("empire_id:N", sort=empires),
        )
        .configure_range(category=alt.RangeScheme(theme_colors))
    )


def add_sidebar(ais_data):
    with st.sidebar:
        for item in ais_data:
            color = to_hex_color(item["color"])
            player = item["player_name"]
            empire_id = item["empire_id"]
            body = f"""- **{empire_id}**: <span style="color:{color}">{player}</span>"""
            st.markdown(body, unsafe_allow_html=True)


def draw_plots(ais_data):
    ais_data = sorted(ais_data, key=lambda x: int(x["empire_id"]))
    st.button("Extended legend", key="?", on_click=add_sidebar, args=(ais_data,))

    for param in ["PP", "RP", "SHIP_CONT"]:
        plot_for_attribute(ais_data, attribute=param)


draw_plots(get_ais_data())
