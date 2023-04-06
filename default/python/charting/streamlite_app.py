import os
import sys

import streamlit as st

from tabs.policies import plot_policy_adoptions
from tabs.stats import draw_plots

current_dir = os.path.dirname(__file__)
common = os.path.join(current_dir, "..")
assert os.path.exists(common)
sys.path.append(common)

from charting.log_parser.collect_data import get_ais_data  # noqa: E402, need to inject common dir before import


def setup_layout():
    st.set_page_config(layout="wide")


setup_layout()

plots_tab, policies_tab = st.tabs(["Stats", "Policies"])

with plots_tab:
    draw_plots(get_ais_data())

with policies_tab:
    plot_policy_adoptions(get_ais_data())
