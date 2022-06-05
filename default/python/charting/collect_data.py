from extract_data import process_state
from file_reader import return_file_list
from log_tokenizer import tokenize_log


def get_ais_data():
    file_list = return_file_list()
    return [process_state(tokenize_log(file_)) for file_ in file_list]
