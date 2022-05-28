import os
from glob import glob
from pathlib import Path


def get_dir() -> Path:
    if os.name == "nt":
        return Path(os.path.expanduser("~/Appdata/Roaming/Freeorion"))
    elif os.name == "posix":
        return Path(os.environ.get("XDG_DATA_HOME", os.environ.get("HOME", "") + "/.local/share")) / "freeorion"
    else:
        return Path(os.environ.get("HOME", "")) / ".freeorion"


def validate_data_dir(data_dir: Path) -> bool:
    return os.path.exists(data_dir / "freeorion.log")


def get_log_files(data_dir: Path, timestamp):
    log_pattern = (data_dir / "AI_*.log").as_posix()
    logfiles = glob(log_pattern)
    return [log for log in logfiles if abs(timestamp - os.path.getmtime(log)) < 300]


def return_file_list():
    data_dir = get_dir()
    if not validate_data_dir(data_dir):
        raise Exception("freeorion.log is missed, please run the game to fix it.")

    timestamp = os.path.getmtime(data_dir / "freeorion.log")

    return get_log_files(data_dir, timestamp)


return_file_list()
