from functools import wraps
from typing import Callable


class InitializerLock:
    """
    Class that ensure that it's properly initialized before the first use of data.
    """

    def __init__(self, name):
        self.__initialized = False
        self.__name = name

    def lock(self):
        self.__initialized = False

    def unlock(self):
        self.__initialized = True

    def __call__(self, function: Callable):
        @wraps(function)
        def wrapper(*args, **kwargs):
            if self.__initialized:
                return function(*args, **kwargs)
            else:
                raise ValueError(
                    f"Call of '{function.__name__}' is forbidden before {self.__class__.__name__}('{self.__name}') is initialized"
                )

        return wrapper
