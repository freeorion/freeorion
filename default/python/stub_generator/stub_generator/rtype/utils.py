_basic_types = {str, int, float, bool}


def _is_new_type(val):
    """
    NewType is a function on Python3.9, in Python3.10 is changed to class.
    """
    return "NewType" in str(val)


def get_name_for_mapping(property_class) -> str:
    if _is_new_type(property_class) or property_class in _basic_types:
        return property_class.__name__
    if isinstance(property_class, str):
        return property_class
    else:
        _, _, rtype = str(property_class).rpartition(".")
        return rtype
