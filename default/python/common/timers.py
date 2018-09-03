from __future__ import print_function

from time import time


class Timer(object):
    def __init__(self, timer_name):
        """
        Creates timer. Timer name is name that will be in logs.
        """
        self.timer_name = timer_name
        self.start_time = None
        self.end_time = None
        self.section_name = None
        self.timers = []

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

    def _print_timer_table(self, time_table):
        """
        Print a header and a footer and fill it with the timing results from time_table.
        """
        # TODO: Convert to use table code from here /python/common/print_utils.py#L116

        if not time_table:
            print("NOT Time Table")
            return
        max_header = max(len(x[0]) for x in time_table)
        line_max_size = max_header + 14
        print()
        print('Timing for %s:' % self.timer_name)
        print('=' * line_max_size)

        for name, val in time_table:
            print("%-*s %8d msec" % (max_header, name, val))

        print('-' * line_max_size)
        print(("Total: %8d msec" % sum(x[1] for x in time_table)).rjust(line_max_size))

    def print_flat(self):
        """
        Output result.
        """
        self._print_timer_table(self.timers)

    def print_aggregate(self):
        """
        Print aggregated results for each section.
        """
        accumulated_times = {}
        ordered_names = []
        for name, val in self.timers:
            if name not in accumulated_times:
                ordered_names.append(name)
                accumulated_times[name] = 0
            accumulated_times[name] += val
        time_table = [(name, accumulated_times[name]) for name in ordered_names]

        self._print_timer_table(time_table)

    def print_statistics(self):
        """
        Print number of times, average, std, and max statistics for each section.
        """
        # TODO: If conversion to python 3 happens, use statistics.pstdev() to compute statistics.
        number_samples = {}
        means = {}
        stds = {}
        maxes = {}
        ordered_names = []
        for name, val in self.timers:
            if name not in ordered_names:
                ordered_names.append(name)
                means[name] = 0
                stds[name] = 0
                number_samples[name] = 0
                maxes[name] = val
            means[name] += val
            number_samples[name] += 1
            maxes[name] = max(val, maxes[name])
        for name in ordered_names:
            means[name] = means[name] / number_samples[name]
        for name, val in self.timers:
            stds[name] += (val - means[name]) ** 2.0
        for name in ordered_names:
            stds[name] = (stds[name] / number_samples[name]) ** 0.5

        max_header = max(len(x) for x in ordered_names)
        line_max_size = max_header + 70
        print()
        print('Timing statistics for %s:' % self.timer_name)
        print('=' * line_max_size)

        for name in ordered_names:
            print(("{:<{name_width}} num: {:>6d}, mean: {:>6.2f} msec, std: {:>6.2f} msec, max: {:>6.2f} msec").
                  format(name, number_samples[name], means[name], stds[name], maxes[name], name_width=max_header))

        print('=' * line_max_size)

    def stop_and_print(self):
        """
        Stop timer, output flat result.
        """
        self.stop()
        self.print_flat()

    def clear_data(self):
        """
        Clear timer data.
        """
        self.timers = []
