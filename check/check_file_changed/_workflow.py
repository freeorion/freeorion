from enum import Enum, auto

from check_file_changed._detectors import FileGroup


class Workflow(Enum):
    BUILD_WINDOWS_WITH_CMAKE = auto()


workflow_mapping = {
    Workflow.BUILD_WINDOWS_WITH_CMAKE: {
        FileGroup.FOCS_PY,
        FileGroup.CPP,
        FileGroup.CMAKE,
        FileGroup.WORKFLOWS,
    }
}


def get_workflows(base_sections: set[FileGroup]) -> set[Workflow]:
    return {workflow for workflow, items in workflow_mapping.items() if items.intersection(base_sections)}
