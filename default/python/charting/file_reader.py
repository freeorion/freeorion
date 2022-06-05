import os
import platform
from glob import glob
from pathlib import Path


def _get_log_dir() -> Path:
    """
    Get directory with logs.

    Note: this wont work with the new option to specify log files.
    Probably if not logs found we should check config.xml and persistent_config.xml
    """
    if os.name == "nt":
        return Path(os.path.expanduser("~/Appdata/Roaming/Freeorion"))
    elif platform.system() == "Darwin":
        return Path("~/Library/Application Support/FreeOrion").expanduser()
    elif os.name == "posix":
        return Path(os.environ.get("XDG_DATA_HOME", os.environ.get("HOME", "") + "/.local/share")) / "freeorion"
    else:
        return Path(os.environ.get("HOME", "")) / ".freeorion"


def _validate_data_dir(data_dir: Path) -> bool:
    return os.path.exists(data_dir / "freeorion.log")


def _list_log_files(data_dir: Path, timestamp):
    """
    List and filter log files.

    If file we modified close to "freeorion.log" we accept it, overwise filter it out.
    This helps in case when we have stale logs from previous games (where we had more players).
    """
    log_pattern = (data_dir / "AI_*.log").as_posix()
    logfiles = glob(log_pattern)
    return [log for log in logfiles if abs(timestamp - os.path.getmtime(log)) < 300]


def return_file_list():
    data_dir = _get_log_dir()
    if not _validate_data_dir(data_dir):
        raise Exception(f"freeorion.log is missed in {data_dir}, please run the game to fix it.")

    timestamp = os.path.getmtime(data_dir / "freeorion.log")

    return _list_log_files(data_dir, timestamp)
