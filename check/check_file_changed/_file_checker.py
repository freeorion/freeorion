import re
from collections import Counter
from collections.abc import Collection
from pathlib import Path, PurePath
from typing import NamedTuple, NewType, Optional

try:
    # Require Python 3.11
    # It's recommended to run it with python 3.11+ on CI
    from tomllib import load  # type: ignore[reportMissingImports]
except ModuleNotFoundError:
    # Works in 3.9: pip install tomlkit
    from tomlkit import load  # type: ignore[reportMissingImports]

WorkflowName = NewType("WorkflowName", str)
GroupName = NewType("GroupName", str)


class FileGroup:
    name: GroupName
    examples: list[PurePath]
    patterns: list[re.Pattern]

    def __init__(self, name, examples, patterns):
        self.name = name
        self.examples = examples
        self.patterns = patterns
        self.matched = Counter({x: 0 for x in patterns})

        super().__init__()

    def accept(self, path: PurePath) -> bool:
        posix_path = path.as_posix()

        for pattern in self.patterns:
            if pattern.search(posix_path):
                self.matched[pattern] += 1
                return True
        return False


class FileChecker(NamedTuple):
    workflows: dict[WorkflowName, set[GroupName]]
    groups: list[FileGroup]

    def get_detected_group(self, path: str) -> Optional[GroupName]:
        for group in self.groups:
            if group.accept(PurePath(path)):
                return group.name
        else:
            return None

    def get_workflows(self, groups: Collection[GroupName]) -> set[WorkflowName]:
        return {k for k, v in self.workflows.items() if v.intersection(groups)}


def _extract_workflows(param) -> dict[WorkflowName, set[GroupName]]:
    return {WorkflowName(k): {GroupName(x) for x in v} for k, v in param.items()}


def _extract_groups(param) -> list[FileGroup]:
    groups = []
    for name, group_data in param.items():
        expected_keys = {"patterns", "examples"}
        assert expected_keys == set(group_data), f"Expect {expected_keys} got {set(group_data)} for [group.{name}]"
        examples = [PurePath(x) for x in group_data["examples"]]
        assert examples, f"Examples could not be empty in {name}"

        patterns = [re.compile(x, flags=re.IGNORECASE) for x in group_data["patterns"]]
        assert patterns, f"Patterns could not be empty in {name}"

        groups.append(FileGroup(GroupName(name), examples, patterns))
    return groups


def read(path: Path = Path(__file__).parent / "config.toml") -> FileChecker:
    with path.open("rb") as f:
        data = load(f)

    top_level_keys = {"workflow", "group"}
    assert top_level_keys == set(data)

    workflows = _extract_workflows(data["workflow"])
    groups = _extract_groups(data["group"])

    for group in groups:
        for example in group.examples:
            assert group.accept(
                example
            ), f"'{group.name}' example '{example.as_posix()}' does not match any {group.patterns}"

    all_groups = {group.name for group in groups}

    for workflow, workflow_groups in workflows.items():
        assert workflow_groups.issubset(
            all_groups
        ), f"'{workflow}' has unknown groups {', '.join(sorted(workflow_groups - all_groups))}"

    all_used_groups = {group for workflow_groups in workflows.values() for group in workflow_groups}
    assert all_groups.issubset(all_used_groups), f"Unused groups: {', '.join(sorted(all_groups - all_used_groups))}"

    return FileChecker(workflows, groups)
