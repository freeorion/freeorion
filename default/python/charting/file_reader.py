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


def is_close(time_a: float, time_b: float, seconds=60.0):
    return abs(time_a - time_b) < seconds


def _list_log_files(data_dir: Path):
    """
    List and filter log files.

    If file we modified close to "freeorion.log" we accept it, overwise filter it out.
    This helps in case when we have stale logs from previous games (where we had more players).
    """
    log_pattern = (data_dir / "AI_*.log").as_posix()
    logfiles = glob(log_pattern)

    timestamps = sorted([(os.path.getmtime(log), log) for log in logfiles], reverse=True)
    latest_time, _ = timestamps[0]
    return [file_ for time, file_ in timestamps if is_close(latest_time, time)]


def return_file_list():
    data_dir = _get_log_dir()
    if not _validate_data_dir(data_dir):
        raise Exception(f"freeorion.log is missed in {data_dir}, please run the game to fix it.")
    return _list_log_files(data_dir)
