from __future__ import print_function

import os
import sys
from ConfigParser import SafeConfigParser
from collections import OrderedDict as odict

AI_SUB_DIR = 'AI'
DEFAULT_SUB_DIR = os.path.join(AI_SUB_DIR, 'default')
CONFIG_DEFAULT_FILE = 'config.ini'

# CONFIG KEYS
TIMER_SECTION = 'Timers'
TIMERS_USE_TIMERS = 'timers_to_log'
TIMERS_TO_FILE = 'timers_dump'
TIMERS_DUMP_FOLDER = 'timers_dump_folder'
HANDLERS = 'handlers'


flat_options = odict()
sectioned_options = odict()


def _get_option_file(config_dir):
    # hack to allow lunch this code separately to dump default config
    ai_path = _get_default_file_path(config_dir)
    option_file = CONFIG_DEFAULT_FILE
    if ai_path and option_file:
        return os.path.join(ai_path, option_file)
    else:
        return None


def _get_default_file_path(config_dir):
    CONFIG_DEFAULT_DIR = os.path.join(config_dir, DEFAULT_SUB_DIR)

    try:
        if os.path.isdir(config_dir) and not os.path.isdir(CONFIG_DEFAULT_DIR):
            os.makedirs(CONFIG_DEFAULT_DIR)
    except OSError:
        sys.stderr.write("AI Config Error: could not create path %s\n" % CONFIG_DEFAULT_DIR)
        return config_dir
    return CONFIG_DEFAULT_DIR


def _get_preset_default_ai_options():
    """
    returns preset default options for AI; an ordered dict of dicts.
    each sub-dict corresponds to a config file section
    :return:
    """
    return odict([
        (TIMER_SECTION, odict([
            (TIMERS_USE_TIMERS, False),
            (TIMERS_TO_FILE, False),
            (TIMERS_DUMP_FOLDER, 'timers')
        ])
         ),
        ('main', odict([
            (HANDLERS, '')
        ])
         )  # module names in handler directory, joined by space
    ])


def _create_default_config_file(path):
    """
    Writes default config to file.
    """
    config = SafeConfigParser()
    presets = _get_preset_default_ai_options()
    for section, entries in presets.iteritems():
        config.add_section(section)
        for k, v in entries.iteritems():
            config.set(section, k, str(v))
    if path:
        try:
            with open(unicode(path, 'utf-8'), 'w') as configfile:
                config.write(configfile)
            print("default config is dumped to %s" % path)
        except IOError:
            sys.stderr.write("AI Config Error: could not write default config %s\n" % path)
    return config


def check_bool(option):
    return str(option).lower() in ["1", "on", "true", "yes"]


def get_option_dict():
    """
    Return options for AI
    :return: ordered dict of options
    """
    return flat_options


def get_sectioned_option_dict():
    """
    Return options for AI
    :return: ordered dict of ordered dicts of options
    """
    return sectioned_options


def parse_config(option_string, config_dir):

    if option_string is not None and not isinstance(option_string, str):
        # probably called by unit test
        print("Specified option string is not a string: ", option_string, file=sys.stderr)
        return

    # get defaults; check if don't already exist and can write
    default_file = _get_option_file(config_dir)
    if os.path.exists(default_file):
        config = SafeConfigParser()
        config.read([default_file])
    else:
        try:
            config = _create_default_config_file(default_file)
        except IOError:
            sys.stderr.write(
                "AI Config: default file is not present and not writable at location %s\n" % default_file)
            config = _create_default_config_file(os.path.join(config_dir, CONFIG_DEFAULT_FILE))

    if option_string:
        config_files = [option_string]
        configs_read = config.read(config_files)
        print("AI Config read config file(s): %s" % configs_read)
        if len(configs_read) != len(config_files):
            sys.stderr.write("AI Config Error; could NOT read config file(s): %s\n"
                             % list(set(config_files).difference(configs_read)))
    for section in config.sections():
        sectioned_options.setdefault(section, odict())
        for k, v in config.items(section):
            flat_options[k] = v
            sectioned_options[section][k] = v
