import cProfile
import pstats
from collections.abc import Callable
from functools import wraps
from io import StringIO
from logging import debug


def profile(func: Callable):
    """
    Decorator to profile a function.

    This is debug tool, so its usage should not be committed. Decorate wit it, profile code, remove usage.
    """

    @wraps(func)
    def wrapper(*args, **kwargs):
        pr = cProfile.Profile()
        pr.enable()
        retval = func(*args, **kwargs)
        pr.disable()
        s = StringIO()
        sortby = "cumulative"
        ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
        ps.print_stats()
        debug(f"Profile stats for {func.__name__}")
        debug(s.getvalue())
        return retval

    return wrapper
