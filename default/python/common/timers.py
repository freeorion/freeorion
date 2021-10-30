from time import time


class Timer:
    def __init__(self, timer_name):
        """
        Creates timer. Timer name is name that will be in logs.
        """
        self.timer_name = timer_name
        self.start_time = None
        self.end_time = None
        self.section_name = None
        self.timers = []

    def stop(self, section_name=""):
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
        print("Timing for %s:" % self.timer_name)
        print("=" * line_max_size)

        for name, val in time_table:
            print("%-*s %8d msec" % (max_header, name, val))

        print("-" * line_max_size)
        print(("Total: %8d msec" % sum(x[1] for x in time_table)).rjust(line_max_size))

    def stop_print_and_clear(self):
        """
        Stop timer, output result and clear state.
        """
        self.stop()
        self._print_timer_table(self.timers)
        self._clear_data()

    def _clear_data(self):
        """
        Clear timer data.
        """
        self.timers = []
