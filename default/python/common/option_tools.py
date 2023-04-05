import os
import sys
from collections import OrderedDict as odict
from configparser import ConfigParser
from pathlib import Path
from typing import Any, Optional

DEFAULT_SUB_DIR = Path("AI", "default")
CONFIG_DEFAULT_FILE = "config.ini"

# CONFIG KEYS
TIMER_SECTION = "Timers"
TIMERS_USE_TIMERS = "timers_to_log"
HANDLERS = "handlers"


flat_options = odict()


def _get_option_file(config_dir: Path) -> Optional[Path]:
    # hack to allow lunch this code separately to dump default config
    ai_path = _get_default_file_path(config_dir)
    option_file = CONFIG_DEFAULT_FILE
    if ai_path and option_file:
        return ai_path / option_file
    else:
        return None


def _get_default_file_path(config_dir: Path) -> Path:
    config_default_dir = config_dir / DEFAULT_SUB_DIR
    try:
        if config_dir.is_dir() and not config_default_dir.is_dir():
            config_default_dir.mkdir(parents=True)
    except OSError:
        sys.stderr.write(f"AI Config Error: could not create path {config_default_dir}\n")
        return config_dir
    return config_default_dir


def _get_preset_default_ai_options():
    """
    returns preset default options for AI; an ordered dict of dicts.
    each sub-dict corresponds to a config file section
    :return:
    """
    return odict(
        [
            (
                TIMER_SECTION,
                odict([(TIMERS_USE_TIMERS, False)]),
            ),
            ("main", odict([(HANDLERS, "")])),  # module names in handler directory, joined by space
        ]
    )


def _create_default_config_file(path: Path) -> ConfigParser:
    """
    Writes default config to file.
    """
    config = ConfigParser()
    presets = _get_preset_default_ai_options()
    for section, entries in presets.items():
        config.add_section(section)
        for k, v in entries.items():
            config.set(section, k, str(v))
    if path:
        try:
            with open(path, "w") as configfile:
                config.write(configfile)
            print(f"default config is dumped to {path}")
        except OSError:
            sys.stderr.write(f"AI Config Error: could not write default config {path}\n")
    return config


def check_bool(option):
    return str(option).lower() in ["1", "on", "true", "yes"]


def get_option_dict():
    """
    Return options for AI
    :return: ordered dict of options
    """
    return flat_options


def parse_config(option_string: Any, config_dir: str):
    config_dir = Path(config_dir)

    if option_string is not None and not isinstance(option_string, str):
        # probably called by unit test
        print("Specified option string is not a string: ", option_string, file=sys.stderr)
        return

    # get defaults; check if don't already exist and can write
    default_file = _get_option_file(config_dir)
    if os.path.exists(default_file):
        config = ConfigParser()
        config.read([default_file])
        print("AI Config read from default file(s): %s" % default_file)
    else:
        try:
            config = _create_default_config_file(default_file)
        except OSError:
            sys.stderr.write("AI Config: default file is not present and not writable at location %s\n" % default_file)
            config = _create_default_config_file(config_dir / CONFIG_DEFAULT_FILE)

    if option_string:
        config_files = [option_string]
        configs_read = config.read(config_files)
        print("AI Config read config file(s): %s" % configs_read)
        if len(configs_read) != len(config_files):
            sys.stderr.write(
                "AI Config Error; could NOT read config file(s): %s\n"
                % list(set(config_files).difference(configs_read))
            )
    for section in config.sections():
        for k, v in config.items(section):
            flat_options[k] = v
