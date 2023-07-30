from enum import Enum, auto

from _detectors import FileGroup


class Workflow(Enum):
    BUILD_WINDOWS_WITH_CMAKE = auto()
    BUILD_ANDROID = auto()
    BUILD_MACOS = auto()
    BUILD_UBUNTU = auto()
    BUILD_WINDOWS_WITH_MSVS = auto()
    LINT_PY_FOCS = auto()
    LINT_CODEQL = auto()
    LINT_PYTHON = auto()
    LINT_STRING_TABLES = auto()


workflow_mapping = {
    Workflow.BUILD_WINDOWS_WITH_CMAKE: {
        FileGroup.FOCS_PY,
        FileGroup.CPP,
        FileGroup.CMAKE,
        FileGroup.GODOT,
        FileGroup.WORKFLOWS,
    },
    Workflow.BUILD_ANDROID: {
        FileGroup.CPP,
        FileGroup.WORKFLOWS,
    },
    Workflow.BUILD_MACOS: {
        FileGroup.CPP,
        FileGroup.WORKFLOWS,
    },
    Workflow.BUILD_UBUNTU: {
        FileGroup.CPP,
        FileGroup.WORKFLOWS,
    },
    Workflow.BUILD_WINDOWS_WITH_MSVS: {
        FileGroup.CPP,
        FileGroup.WORKFLOWS,
        FileGroup.VISUAL_STUDIO,
    },
    Workflow.LINT_CODEQL: {
        FileGroup.CPP,
        FileGroup.WORKFLOWS,
    },
    Workflow.LINT_PY_FOCS: {
        FileGroup.FOCS_PY,
        FileGroup.PYPROJECT_TOML,
        FileGroup.PYTHON_DEV_REQUIREMENTS,
        FileGroup.WORKFLOWS,
    },
    Workflow.LINT_PYTHON: {
        FileGroup.FOCS_PY,
        FileGroup.PYPROJECT_TOML,
        FileGroup.PYTHON_DEV_REQUIREMENTS,
        FileGroup.WORKFLOWS,
        FileGroup.PYTHON,
        FileGroup.WORKFLOWS,
    },
    Workflow.LINT_STRING_TABLES: {
        FileGroup.STRINGTABLES,
        FileGroup.WORKFLOWS,
    },
}


def get_workflows(base_sections: set[FileGroup]) -> set[Workflow]:
    return {workflow for workflow, items in workflow_mapping.items() if items.intersection(base_sections)}
