from logging import error, warning
from textwrap import fill
from typing import Iterable, List, Set

from common.print_utils import Table, Text
from stub_generator.interface_inspector import ClassInfo, EnumInfo, InstanceInfo
from stub_generator.parse_docs import Docs
from stub_generator.stub_generator.base_generator import BaseGenerator


def _get_property_return_type_by_name(attr_name: str) -> str:
    """
    Match property of unknown type.
    """
    property_map = {
        "id": "ObjectId",
        "systemID": "SystemId",
        "systemIDs": "SystemId",
        "name": "str",
        "empireID": "EmpireId",
        "description": "str",
        "speciesName": "SpeciesName",
        "capitalID": "PlaneId",
        "owner": "EmpireId",
        "designedOnTurn": "Turn",
    }
    return property_map.get(attr_name, "")


def _update_property_return_type(attr_name: str, rtype: str):
    """
    Match property of known type.
    """
    if rtype.startswith("<type"):
        rtype = rtype[7:-2]
    else:
        rtype = rtype.split(".")[-1].strip("'>")

    property_map = {
        ("shipIDs", "IntSet"): "Set[ShipId]",
        ("shipIDs", "IntVec"): "Sequence[ShipId]",
        ("buildingIDs", "IntSet"): "Set[BuildingId]",
        ("buildingIDs", "IntVec"): "Sequence[BuildingId]",
        ("planetIDs", "IntSet"): "Set[PlanetId]",
        ("planetIDs", "IntVec"): "Sequence[PlanetId]",
        ("fleetIDs", "IntVec"): "Sequence[FleetId]",
        ("fleetIDs", "IntSet"): "Set[FleetId]",
        ("systemIDs", "IntVec"): "Sequence[SystemId]",
        ("empireID", "int"): "EmpireId",
        ("capitalID", "int"): "PlanetId",
        ("locationID", "int"): "PlanetId",
        ("owner", "int"): "EmpireId",
        ("speciesName", "str"): "SpeciesName",
        ("designedOnTurn", "int"): "Turn",
        ("buildingTypeName", "str"): "BuildingName",
    }
    return property_map.get((attr_name, rtype), rtype)


def _update_method_return_type(class_: str, method_name: str, rtype: str) -> str:
    method_map = {("empire", "supplyProjections"): "Dict[SystemId, int]"}
    key = (class_, method_name)
    rtype = method_map.get(key, rtype)

    if rtype in ("VisibilityIntMap", "IntIntMap"):
        return "Dict[int, int]"
    else:
        return rtype


def _handle_class(info: ClassInfo):
    assert not info.doc, "Got docs need to handle it"
    parents = [x for x in info.parents if x != "object"]

    result = []
    if parents:
        result.append("class %s(%s):" % (info.name, ", ".join(info.parents)))
    else:
        result.append("class %s:" % info.name)

    properties = []
    instance_methods = []
    for attr_name, attr in sorted(info.attributes.items()):
        if attr["type"] == "<class 'property'>":
            rtype = attr.get("rtype", "")
            if not rtype:
                rtype = _get_property_return_type_by_name(attr_name)
            else:
                rtype = _update_property_return_type(attr_name, rtype)
            properties.append((attr_name, rtype))
        elif attr["type"] in ("<class 'Boost.Python.function'>", "<class 'function'>"):
            instance_methods.append(attr["routine"])
        else:
            warning("Skipping '%s' (%s): %s" % ((info.name), attr["type"], attr))

    for property_name, rtype in properties:
        if not rtype:
            return_annotation = ""
        else:
            return_annotation = " -> %s" % rtype

        if property_name == "class":
            result.append("    # cant define it via python in that way")
            result.append("    # @property")
            result.append("    # def %s(self)%s: ..." % (property_name, return_annotation))
        else:
            result.append("    @property")
            result.append("    def %s(self)%s: ..." % (property_name, return_annotation))

    for routine_name, routine_docs in instance_methods:
        docs = Docs(routine_docs, 2, is_class=True)
        # TODO: Subclass map-like classes from dict (or custom class) rather than this hack
        rtype = _update_method_return_type(info.name, routine_name, docs.rtype)

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
                "    def %s(%s)%s:%s%s"
                % (routine_name, next(docs.get_argument_strings()), return_annotation, doc_string, end)
            )
        else:
            for arg_string in arg_strings:
                result.append("    @overload\n    def %s(%s)%s:%s" % (routine_name, arg_string, return_annotation, end))

            result.append("    def %s(*args)%s:%s%s" % (routine_name, return_annotation, doc_string, end))

    if not (properties or instance_methods):
        result[-1] += " ..."
    result.append("")
    yield "\n".join(result)


def _report_classes_without_instances(classes_map: Iterable[str], instance_names, classes_to_ignore: Set[str]):
    missed_instances = instance_names.symmetric_difference(classes_map).difference(classes_to_ignore)

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
    table.print_table(warning)


class ClassGenerator(BaseGenerator):
    def __init__(
        self,
        classes: List[ClassInfo],
        instances: List[InstanceInfo],
        classes_to_ignore: Set[str],
        enums: List[EnumInfo],
    ):
        super().__init__()
        self.classes_to_ignore = classes_to_ignore
        self.instances = instances
        self.classes = classes
        self.enums = enums
        self._process()

    def _process(self):
        # exclude technical Map classes that are prefixed with map_indexing_suite_ classes
        classes = [x for x in self.classes if not x.name.startswith("map_indexing_suite_")]
        classes_map = {x.name: x for x in classes}

        _report_classes_without_instances(
            classes_map.keys(), {instance.class_name for instance in self.instances}, self.classes_to_ignore
        )

        enums_names = {x.name for x in self.enums}

        # enrich class data with the instance data
        # class properties does not provide any useful info, so we use instance to find return type of the properties
        for instance in self.instances:
            if instance.class_name in enums_names:
                warning("skipping enum instance: %s" % instance.class_name)
                continue
            try:
                class_attrs = classes_map[instance.class_name].attributes
            except KeyError:
                error(
                    "Instance class was not found in classes generated by C++ interface,"
                    " please check instance at %s with class: %s"
                    % (
                        instance["location"],
                        (instance.class_name),
                    )
                )
                continue

            for attribute_name, class_attibute in class_attrs.items():
                inst = instance.attributes.get(attribute_name)
                if not inst:
                    continue
                type_ = class_attibute["type"]

                if type_ in ("<class 'Boost.Python.function'>", "<class 'function'>"):
                    continue

                if type_ == "<class 'property'>":
                    assert class_attibute["getter"] is None  # if we will have docs here, handle them
                    class_attibute["rtype"] = inst["type"][8:-2]  # "<class 'str'>" - > str TODO extract to function
                    continue
                error(
                    "Unknown class attribute type: '%s' for %s.%s: %s"
                    % (type_, instance.class_name, attribute_name, class_attibute)
                )
        classes = sorted(
            classes, key=lambda class_: (len(class_.parents), class_.parents and class_.parents[0] or "", class_.name)
        )  # put classes with no parents on first place

        for cls in classes:
            self.body.extend(_handle_class(cls))
