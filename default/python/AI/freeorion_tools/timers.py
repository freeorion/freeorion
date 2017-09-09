import os
import freeOrionAIInterface as fo
from common.timers import Timer
from common.option_tools import get_option_dict, check_bool, DEFAULT_SUB_DIR
from common.option_tools import TIMERS_TO_FILE, TIMERS_USE_TIMERS, TIMERS_DUMP_FOLDER
import sys

# setup module
options = get_option_dict()

try:
    USE_TIMERS = check_bool(options[TIMERS_USE_TIMERS])
    DUMP_TO_FILE = check_bool(options[TIMERS_TO_FILE])
    TIMERS_DIR = os.path.join(fo.getUserDataDir(), DEFAULT_SUB_DIR, options[TIMERS_DUMP_FOLDER])
except KeyError:
    USE_TIMERS = False


def make_header(*args):
    return ['%-8s ' % x for x in args]


def _get_timers_dir():
    try:
        if os.path.isdir(fo.getUserDataDir()) and not os.path.isdir(TIMERS_DIR):
            os.makedirs(TIMERS_DIR)
    except OSError:
        sys.stderr.write("AI Config Error: could not create path %s\n" % TIMERS_DIR)
        return False

    return TIMERS_DIR


class DummyTimer(object):
    """
    Dummy timer to be used if timers are disabled.
    """
    def __init__(self, *args, **kwargs):
        pass

    def stop(self, *args, **kwargs):
        pass

    def start(self, *args, **kwargs):
        pass

    def stop_and_print(self):
        pass

    def stop_print_and_clear(self):
        pass

    def clear_data(self):
        pass

    def print_flat(self):
        pass

    def print_aggregate(self):
        pass

    def print_statistics(self):
        pass


class AILogTimer(Timer):
    """A Timer with a FO AI engine dependent extension that logs timer results to a file each turn.
    """
    def __init__(self, timer_name, write_log=False):
        """
        Creates timer. Timer name is name that will be in logs header and part of filename if write_log=True
        If write_log true and timers logging enabled (DUMP_TO_FILE=True) save timer info to file.

        """
        Timer.__init__(self, timer_name)
        self.headers = None
        self.write_log = write_log
        self.log_name = None

    def _write(self, text):
        if not _get_timers_dir():
            return
        if not self.log_name:
            empaire_id = fo.getEmpire().empireID - 1
            self.log_name = os.path.join(_get_timers_dir(), '%s-%02d.txt' % (self.timer_name, empaire_id))
            mode = 'w'  # empty file
        else:
            mode = 'a'
        with open(unicode(self.log_name, 'utf-8'), mode) as f:
            f.write(text)
            f.write('\n')

    def stop_print_and_clear(self):
        """
        Stop timer, output result, and clear timers.
        If dumping to file, if headers are not match to prev, new header line will be added.
        """
        Timer.stop_and_print(self)

        if self.write_log and DUMP_TO_FILE:
            turn = fo.currentTurn()
            headers = make_header('Turn', *[x[0] for x in self.timers])
            if self.headers != headers:
                self._write(''.join(headers) + '\n' + ''.join(['-' * (len(x) - 2) + '  ' for x in headers]))
                self.headers = headers

            row = []
            for header, val in zip(self.headers, [turn] + [x[1] for x in self.timers]):
                row.append('%*s ' % (len(header) - 2, int(val)))
            self._write(''.join(row))


AITimer = AILogTimer if USE_TIMERS else DummyTimer
