import cProfile
import os
import pstats
import time
import freeOrionAIInterface as fo
import StringIO
import FreeOrionAI as foAI


def profile(sort_by='cumulative'):
    """
    Profile function decorator.

    Each function execution will add new entry to file.

    Result stored in user directory in profile catalog. See lne in logs for exact position.
    """

    def argument_wrapper(function):
        def wrapper(*args, **kwargs):
            base_path = os.path.join(fo.getUserDataDir(), 'profiling')
            path = os.path.join(base_path, foAI.foAIstate.uid)

            pr = cProfile.Profile()
            pr.enable()
            start = time.clock()
            result = function(*args, **kwargs)
            end = time.clock()
            pr.disable()
            print "Profile %s tooks %f s, saved to %s" % (function.__name__, end - start, path)
            s = StringIO.StringIO()
            ps = pstats.Stats(pr, stream=s).strip_dirs().sort_stats(sort_by)
            ps.print_stats()

            if not os.path.exists(base_path):
                os.makedirs(base_path)
            with open(path, 'a') as f:
                f.write(s.getvalue())
                f.write('\n')

            return result

        return wrapper

    return argument_wrapper
