from abc import ABC, abstractmethod
from enum import Enum, auto
from pathlib import PurePath


class FileGroup(Enum):
    FOCS_PY = auto()
    PYPROJECT_TOML = auto()
    PYTHON_DEV_REQUIREMENTS = auto()
    CPP = auto()
    CMAKE = auto()
    WORKFLOWS = auto()


class _Detector(ABC):
    @abstractmethod
    def accept(self, path: PurePath) -> bool:
        ...

    @abstractmethod
    def file_type(self) -> FileGroup:
        ...

    @abstractmethod
    def examples(self) -> list[str]:
        """
        A list of matching paths in unix style.

        This serves as documentation and tests.  Should not be empty.
        """

    def __hash__(self):
        return hash(self.file_type())

    def __eq__(self, other):
        return self.file_type() == other.file_type()

    def __repr__(self):
        return f"{self.__class__.__name__}"


class DetectorPyFocs(_Detector):
    def examples(self) -> list[str]:
        return [
            "default/scripting/species/SP_BANFORO.focs.py",
        ]

    def file_type(self) -> FileGroup:
        return FileGroup.FOCS_PY

    def accept(self, path: PurePath) -> bool:
        folder = PurePath("default", "scripting")
        return path.is_relative_to(folder) and path.name.endswith(".py")


class DetectorPyProjectToml(_Detector):
    def examples(self) -> list[str]:
        return ["pyproject.toml"]

    def file_type(self) -> FileGroup:
        return FileGroup.PYPROJECT_TOML

    def accept(self, path: PurePath) -> bool:
        return path.name == "pyproject.toml"


class DetectorPythonDevRequirements(_Detector):
    def examples(self) -> list[str]:
        return ["default/python/requirements-dev.txt"]

    def file_type(self) -> FileGroup:
        return FileGroup.PYTHON_DEV_REQUIREMENTS

    def accept(self, path: PurePath) -> bool:
        folder = PurePath("default", "python")
        return path.is_relative_to(folder) and path.name.endswith("requirements-dev.txt")


class DetectorCPP(_Detector):
    def examples(self) -> list[str]:
        return [
            "parse/CommonParamsParser.cpp",
            "parse/ConditionParser.h",
        ]

    def file_type(self) -> FileGroup:
        return FileGroup.CPP

    def accept(self, path: PurePath) -> bool:
        return path.name.endswith((".cpp", ".h"))


class DetectorCmake(_Detector):
    def examples(self) -> list[str]:
        return ["cmake/FreeOrionVersion.cmake.in", "cmake/FFindVorbis.cmake"]

    def file_type(self) -> FileGroup:
        return FileGroup.CMAKE

    def accept(self, path: PurePath) -> bool:
        return path.is_relative_to(PurePath("cmake"))


class DetectorWorkflows(_Detector):
    def examples(self) -> list[str]:
        return [".github/workflows/project-labeler.yml"]

    def file_type(self) -> FileGroup:
        return FileGroup.WORKFLOWS

    def accept(self, path: PurePath) -> bool:
        return path.is_relative_to(PurePath(".github", "workflows"))


# Todo generate automatically based on classes
registered_detectors: set[_Detector] = {
    DetectorPyFocs(),
    DetectorPyProjectToml(),
    DetectorPythonDevRequirements(),
    DetectorCmake(),
    DetectorCPP(),
    DetectorWorkflows(),
}


def detect_file_groups(file_list) -> set[FileGroup]:
    detectors: set[_Detector] = registered_detectors.copy()
    file_types_found: set[_Detector] = set()

    for file_ in file_list:
        if not detectors:
            break
        file_path = PurePath(file_)

        for detector in detectors:
            if detector.accept(file_path):
                file_types_found.add(detector)

        detectors = detectors - file_types_found
    return {x.file_type() for x in file_types_found}