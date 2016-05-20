import cProfile
import pstats
import time

import StringIO


def profile(sort_by='cumulative'):
    def argument_wrapper(function):
        def wrapper(*args, **kwargs):
            pr = cProfile.Profile()
            pr.enable()
            start = time.clock()
            result = function(*args, **kwargs)
            end = time.clock()
            pr.disable()
            print "Profile %s tooks %f s" % (function.__name__, end - start)
            print "-----"
            s = StringIO.StringIO()
            ps = pstats.Stats(pr, stream=s).sort_stats(sort_by)
            ps.print_stats()
            print s.getvalue()
            print "-----"
            return result

        return wrapper

    return argument_wrapper
