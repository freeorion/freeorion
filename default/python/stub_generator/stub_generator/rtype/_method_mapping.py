from stub_generator.stub_generator.rtype._base_rtype_mapping import make_type

_method_map = {
    ("empire", "supplyProjections"): "Dict[SystemId, int]",
}


def update_method_rtype(class_: str, method_name: str, rtype: str) -> str:
    key = (class_, method_name)
    if key in _method_map:
        return _method_map[key]
    else:
        return make_type(rtype)
