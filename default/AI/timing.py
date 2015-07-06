import os
import freeOrionAIInterface as fo
from time import time

TIMERS_DIR = 'timers'
DUMP_TO_FILE = os.path.isdir(TIMERS_DIR)


def make_header(*args):
    return ['%-8s  ' % x for x in args]


class Timer(object):
    def __init__(self, timer_name, write_log=False):
        """
        Creates timer. Timer name is name that will be in logs header and part of filename if write_log=True
        If write_log true and timers logging enabled (DUMP_TO_FILE=True) save timer info to file.

        """
        self.timer_name = timer_name
        self.start_time = None
        self.section_name = None
        self.timers = []
        self.write_log = write_log
        self.headers = None
        self.log_name = None

    def stop(self, section_name=''):
        """
        Stop timer if running. Specify section_name if want to override its name.
        """
        if self.start_time:
            self.end_time = time()
            section_name = section_name or self.section_name
            self.timers.append((section_name, (self.end_time - self.start_time) * 1000.0))
        self.start_time = None
        self.section_name = None

    def start(self, section_name):
        """
        Stop prev timer if present and starts new.
        """
        self.stop()
        self.section_name = section_name
        self.start_time = time()

    def _write(self, text):
        if not self.log_name:
            empaire_id = fo.getEmpire().empireID - 1
            self.log_name = os.path.join(TIMERS_DIR, '%s-%02d.txt' % (self.timer_name, empaire_id))
            mode = 'w'  # empty file
        else:
            mode = 'a'
        with open(self.log_name, mode) as f:
            f.write(text)
            f.write('\n')

    def end(self):
        """
        Stop timer, output result, clear checks.
        If dumping to file, if headers are not match to prev, new header line will be added.
        """
        turn = fo.currentTurn()
        self.stop()
        if not self.timers:
            return
        max_header = max(len(x[0]) for x in self.timers)
        line_max_size = max_header + 14
        print
        print ('Timing for %s:' % self.timer_name)
        print '=' * line_max_size
        for name, val in self.timers:
            print "%-*s %8d msec" % (max_header, name, val)
        print '-' * line_max_size
        print ("Total: %8d msec" % sum(x[1] for x in self.timers)).rjust(line_max_size)

        if self.write_log and DUMP_TO_FILE:
            headers = make_header('Turn', *[x[0] for x in self.timers])
            if self.headers != headers:
                self._write(''.join(headers) + '\n' + ''.join(['-' * (len(x) - 2) + '  ' for x in headers]))
                self.headers = headers

            row = []
            for header, val in zip(self.headers, [turn] + [x[1] for x in self.timers]):
                row.append('%*s  ' % (len(header) - 2, int(val)))
            self._write(''.join(row))
        self.timers = []  # clear times
