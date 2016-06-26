class State(object):
    """
    This class represent state for current turn.
    It contains variables that renewed each turn.
    Before each turn `clean` method should be called at the beginning of each turn.

    Each variable should have a getter that returns value or deepcopy of value if it is immutable
    and one or couple setters (if need to change inner collections in collection).

    This behaviour is similar for `freeorion_tools` but all to change state from external location.
    """

    def __init__(self):
        self.__have_gas_giant = False
        self.__have_asteroids = False
        self.__have_ruins = False
        self.__have_nest = False
        self.__have_computronium = False
        self.__best_pilot_rating = 1e-8
        self.__medium_pilot_rating = 1e-8

    def cleanup(self):
        self.__init__()

    @property
    def have_gas_giant(self):
        return self.__have_gas_giant

    def set_have_gas_giant(self):
        self.__have_gas_giant = True

    @property
    def have_asteroids(self):
        return self.__have_asteroids

    def set_have_asteroids(self):
        self.__have_asteroids = True

    @property
    def have_ruins(self):
        return self.__have_ruins

    def set_have_ruins(self):
        self.__have_ruins = True

    @property
    def have_nest(self):
        return self.__have_nest

    def set_have_nest(self):
        self.__have_nest = True

    @property
    def have_computronium(self):
        return self.__have_computronium

    def set_have_computronium(self):
        self.__have_computronium = True

    @property
    def best_pilot_rating(self):
        return self.__best_pilot_rating

    def set_best_pilot_rating(self, value):
        self.__best_pilot_rating = value

    @property
    def medium_pilot_rating(self):
        return self.__medium_pilot_rating

    def set_medium_pilot_rating(self, value):
        self.__medium_pilot_rating = value


state = State()
