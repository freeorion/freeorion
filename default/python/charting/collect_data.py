import streamlit as st
from extract_data import process_state
from file_reader import return_file_list
from log_tokenizer import tokenize_log


def get_ais_data():
    timestamp, file_list = return_file_list()
    return _get_ais_data(timestamp, file_list)


@st.cache
def _get_ais_data(_cache_key, file_list: tuple):
    data = (process_state(tokenize_log(file_)) for file_ in file_list)
    return sorted(data, key=lambda x: int(x["empire_id"]))
