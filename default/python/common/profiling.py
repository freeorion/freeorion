from __future__ import print_function

import cProfile
import os
import pstats
import time
import StringIO


def profile(save_path, sort_by='cumulative'):
    """
    Profile function decorator.

    Each function execution will add new entry to file.

    Result stored in user directory in profile catalog. See lne in logs for exact position.

    :param save_path: path to save profile, for example:
        os.path.join(fo.getUserDataDir(), 'profiling', base_path, get_aistate().uid)
    :type save_path: str
    :param sort_by: sort stats https://docs.python.org/2/library/profile.html#pstats.Stats.sort_stats
    :type sort_by: str
    """

    def argument_wrapper(function):
        def wrapper(*args, **kwargs):
            pr = cProfile.Profile()
            pr.enable()
            start = time.clock()
            result = function(*args, **kwargs)
            end = time.clock()
            pr.disable()
            print("Profile %s tooks %f s, saved to %s" % (function.__name__, end - start, save_path))
            s = StringIO.StringIO()
            ps = pstats.Stats(pr, stream=s).strip_dirs().sort_stats(sort_by)
            ps.print_stats()

            base_path = os.path.dirname(save_path)
            if not os.path.exists(base_path):
                os.makedirs(base_path)
            with open(unicode(save_path, 'utf-8'), 'a') as f:
                f.write(s.getvalue())
                f.write('\n')

            return result

        return wrapper

    return argument_wrapper
