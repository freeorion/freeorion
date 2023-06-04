_basic_types = {str, int, float, bool}


def _is_new_type(val):
    """
    NewType is a function on Python3.9, in Python3.10 is changed to class.
    """
    return str(val).startswith("<function NewType.")


def get_name_for_mapping(annotation) -> str:
    if _is_new_type(annotation) or annotation in _basic_types:
        return annotation.__name__
    elif isinstance(annotation, str):
        return annotation
    else:
        args = [get_name_for_mapping(x) for x in annotation.__args__]
        return f"{annotation.__name__}[{', '.join(args)}]"
