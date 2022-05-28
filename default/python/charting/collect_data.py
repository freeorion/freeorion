from file_reader import return_file_list
from my_parser import parse_log
from state_machine import process_state


def get_ais():
    file_list = return_file_list()
    return [process_state(parse_log(file_)) for file_ in file_list]
