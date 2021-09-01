from common.option_tools import TIMERS_USE_TIMERS, check_bool, get_option_dict
from common.timers import Timer

# setup module
options = get_option_dict()

try:
    USE_TIMERS = check_bool(options[TIMERS_USE_TIMERS])
except KeyError:
    USE_TIMERS = False


class DummyTimer:
    """
    Dummy timer to be used if timers are disabled.
    """

    def __init__(self, *args, **kwargs):
        pass

    def stop(self, *args, **kwargs):
        pass

    def start(self, *args, **kwargs):
        pass

    def stop_print_and_clear(self):
        pass


AITimer = Timer if USE_TIMERS else DummyTimer
