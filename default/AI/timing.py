import os
import freeOrionAIInterface as fo

TIMERS_DIR = 'timers'
DUMP_TO_FILE = os.path.isdir(TIMERS_DIR)


def make_header(*args):
    return ['%-8s  ' % x for x in args]

MAIN_HEADER = make_header('Turn', "PriorityAI",  "ExplorationAI",  "ColonisationAI",  "InvasionAI",  "MilitaryAI",
                          "Gen_Fleet_Orders", "Issue_Fleet_Orders", "ResearchAI", "ProductionAI",  "ResourcesAI",
                          "Cleanup")

BUCKET_HEADER = make_header('Turn', "Server_Processing",  "AI_Planning")

RESOURCE_GENERATE_HEADER = make_header('Turn', "TopResources",  "SetCapital",  "SetPlanets",  "SetAsteroids",
                                       "SetGiants",  "PrintResources")
RESOURCE_HEADER = make_header('Turn', "getPlanets",  "Filter",  "Priority",  "Shuffle",  "Targets",  "Loop")


class TimeLogger(object):
    def __init__(self, name, headers, write_log=False):
        self.name = name
        self.log_name = None
        self.headers = headers
        self.write_log = write_log

    def _write(self, text, mode):
        with open(self.log_name, mode) as f:
            f.write(text)
            f.write('\n')

    def init(self):
        if DUMP_TO_FILE:
            self.log_name = os.path.join(TIMERS_DIR, '%s-%02d.txt' % (self.name, fo.getEmpire().empireID - 1))
            self._write(''.join(self.headers) + '\n' + ''.join(['-' * (len(x) - 2) + '  ' for x in self.headers]), 'w')

    def add_time(self, turn, times):
        if self.write_log:
            for mod,  modTime in zip(self.headers,  times):
                print "%-*s%8d msec" % (len(max(self.headers, key=len)), mod,  1000 * modTime)

        if DUMP_TO_FILE:
            row = []
            for header, val in zip(self.headers, [turn] + [int(x * 1000) for x in times]):
                row.append('%*s  ' % (len(header) - 2, val))
            self._write(''.join(row), 'a')

main_timer = TimeLogger('timer', MAIN_HEADER, write_log=True)
bucket_timer = TimeLogger("timer_bucket", BUCKET_HEADER, write_log=False)
# resource_generate_timer = TimeLogger('resource_generate', RESOURCE_GENERATE_HEADER, write_log=True)
resource_timer = TimeLogger('resource', RESOURCE_HEADER, write_log=True)


def init_timers():
    main_timer.init()
    bucket_timer.init()
    resource_timer.init()
    # resource_generate_timer.start()  # currently disabled
