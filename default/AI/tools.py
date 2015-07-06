import freeOrionAIInterface as fo  # pylint: disable=import-error


def dict_from_map(thismap):
    return {el.key(): el.data() for el in thismap}


def UserString(label, default=None):  # this name left with C naming style for compatibility with translation assistance procedures  #pylint: disable=invalid-name
    """ A translation assistance tool is intended to search for this method to identify translatable strings."""
    table_string = fo.userString(label)

    if "ERROR: " + label in table_string:  # implement test for string lookup not found error
        return default or table_string
    else:
        return table_string