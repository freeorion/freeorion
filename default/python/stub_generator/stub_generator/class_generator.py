from logging import error, warning
from textwrap import fill
from typing import Any

from common.print_utils import Table, Text
from stub_generator.interface_inspector import ClassInfo, EnumInfo, InstanceInfo
from stub_generator.parse_docs import Docs
from stub_generator.stub_generator.collection_classes import is_collection_type, make_type
from stub_generator.stub_generator.rtype import update_method_rtype, update_property_rtype

# Types that are instantiated by Python code.
instantiated_classes = {
    "diplomaticMessage",
}


def _get_attribute(attr_name: str, attr: Any) -> str:
    if attr["type"] == "<class 'property'>":
        rtype = attr.get("rtype", "")
        rtype = update_property_rtype(attr_name, rtype)
        return f"    {attr_name}: {rtype}"
    else:
        msg = f"Not supported yet: {attr_name}: {attr}"
        raise ValueError(msg)


def process_instantiated_class(info: ClassInfo):
    if info.parents:
        raise ValueError("Instanceable class does not support parenting")

    return "\n".join(
        [
            f"class {info.name}(NamedTuple):",
            *[_get_attribute(attr_name, attr) for attr_name, attr in sorted(info.attributes.items())],
        ]
    )


def _handle_class(info: ClassInfo) -> str:  # noqa: C901
    assert not info.doc, "Got docs need to handle it"

    if info.name in instantiated_classes:
        return process_instantiated_class(info)

    parents = [x for x in info.parents if x != "object"]

    result = []
    if parents:
        result.append(f"class {info.name}({', '.join(info.parents)}):")
    else:
        result.append("class %s:" % info.name)

    properties = []
    instance_methods = []
    for attr_name, attr in sorted(info.attributes.items()):
        if attr["type"] == "<class 'property'>":
            rtype = attr.get("rtype", "")
            rtype = update_property_rtype(attr_name, rtype)
            properties.append((attr_name, rtype))
        elif attr["type"] in ("<class 'Boost.Python.function'>", "<class 'function'>"):
            instance_methods.append(attr["routine"])
        else:
            warning("Skipping '{}' ({}): {}".format((info.name), attr["type"], attr))

    for property_name, rtype in properties:
        if not rtype:
            return_annotation = ""
        else:
            return_annotation = " -> %s" % rtype

        if property_name == "class":
            result.append("    # cant define it via python in that way")
            result.append("    # @property")
            result.append(f"    # def {property_name}(self){return_annotation}: ...")
        else:
            result.append("    @property")
            result.append(f"    def {property_name}(self){return_annotation}: ...")

    for routine_name, routine_docs in instance_methods:
        docs = Docs(routine_docs, 2, is_class=True)
        # TODO: Subclass map-like classes from dict (or custom class) rather than this hack
        rtype = update_method_rtype(routine_name, docs.rtype)

        doc_string = docs.get_doc_string()
        if doc_string:
            doc_string = "\n" + doc_string
            end = ""
        else:
            end = " ..."
        return_annotation = " -> %s" % rtype if rtype else ""
        arg_strings = list(docs.get_argument_strings())
        if len(arg_strings) == 1:
            result.append(
                f"    def {routine_name}({next(docs.get_argument_strings())}){return_annotation}:{doc_string}{end}"
            )
        else:
            for arg_string in arg_strings:
                result.append(f"    @overload\n    def {routine_name}({arg_string}){return_annotation}:{end}")

            result.append(f"    def {routine_name}(*args){return_annotation}:{doc_string}{end}")

    if not (properties or instance_methods):
        result[-1] += " ..."
    return "\n".join(result)


def _report_classes_without_instances(classes_map: set[str], instance_names: set[str], classes_to_ignore: set[str]):
    missed_instances = classes_map.difference(instance_names).difference(classes_to_ignore)

    if not missed_instances:
        return

    warning("")
    warning(
        fill(
            "In order to get more information about the classes in API"
            " we need to process an instances of classes."
            " Classes mentioned bellow does not have instances so their specs are not full."
            " Please provide instances or add them to ignored,"
            " check generate_stub usage in the"
            " freeorion/default/python/handlers folder.",
            width=60,
        )
    )

    table = Table(Text("classes without instances"))

    for inst in sorted(missed_instances, key=str.lower):
        table.add_row(inst)
    warning(table)


def generate_classes(
    raw_classes: list[ClassInfo],
    instances: list[InstanceInfo],
    classes_to_ignore: set[str],
    enums: list[EnumInfo],
):
    # exclude technical Map classes that are prefixed with map_indexing_suite_ classes
    raw_classes = [x for x in raw_classes if not x.name.startswith("map_indexing_suite_")]

    # exclude collection classes
    collection_classes = {info.name for info in raw_classes if is_collection_type(info.name)}
    classes_map = {x.name: x for x in raw_classes if x.name not in collection_classes}
    _report_classes_without_instances(
        set(classes_map), {instance.class_name for instance in instances}, classes_to_ignore
    )

    enums_names = {x.name for x in enums}

    # enrich class data with the instance data
    # class properties does not provide any useful info, so we use instance to find return type of the properties
    for instance in instances:
        if instance.class_name in enums_names:
            warning("skipping enum instance: %s" % instance.class_name)
            continue
        try:
            class_attrs = classes_map[instance.class_name].attributes
        except KeyError:
            error(
                "Instance class was not found in classes generated by C++ interface,"
                " please check instance at %s with class: %s, representing: %s",
                instance.location,
                instance.class_name,
                make_type(instance.class_name),
            )
            continue
        for member_name, class_member in class_attrs.items():
            update_class_member_rtype_from_instance(member_name, instance, class_member)

    classes = sorted(
        classes_map.values(),
        key=lambda class_: (len(class_.parents), class_.parents and class_.parents[0] or "", class_.name),
    )  # put classes with no parents on first place

    for cls in classes:
        yield _handle_class(cls)


def update_class_member_rtype_from_instance(member_name, instance, class_member):
    inst_member = instance.attributes.get(member_name)

    if not inst_member:
        return
    type_ = class_member["type"]

    if type_ in ("<class 'Boost.Python.function'>", "<class 'function'>"):
        return

    if type_ == "<class 'property'>":
        assert class_member["getter"] is None  # if we will have docs here, handle them
        class_member["rtype"] = inst_member["type"][8:-2]  # "<class 'str'>" - > str
        return

    error("Unknown class attribute type: '%s' for %s.%s: %s", type_, instance.class_name, member_name, class_member)
