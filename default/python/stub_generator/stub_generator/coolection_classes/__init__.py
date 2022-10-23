from stub_generator.stub_generator.coolection_classes._collection_name_parser import make_type


def is_collection_type(type_name: str) -> bool:
    return make_type(type_name) is not None
