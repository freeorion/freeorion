import freeOrionAIInterface as fo


# this name left with C naming style for compatibility with translation assistance procedures
def UserString(label, default=None):  # pylint: disable=invalid-name
    """
    A translation assistance tool is intended to search for this method to identify translatable strings.

    :param label: a UserString key
    :param default: a default value to return if there is a key error
    :return: a translated string for the label
    """

    table_string = fo.userString(label)

    if "ERROR: " + label in table_string:  # implement test for string lookup not found error
        return default or table_string
    else:
        return table_string


# this name left with C naming style for compatibility with translation assistance procedures
def UserStringList(label):  # pylint: disable=invalid-name
    """
    A translation assistance tool is intended to search for this method to identify translatable strings.

    :param label: a UserString key
    :return: a python list of translated strings from the UserString list identified by the label
    """

    return fo.userStringList(label)
