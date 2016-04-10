import os
from ConfigParser import SafeConfigParser
from collections import OrderedDict as odict
import platform
import sys

try:
    import freeOrionAIInterface as fo  # pylint: disable=import-error
except ImportError:
    sys.stderr.write("Executing outside of FreeOrion.")


AI_SUB_DIR = 'AI'
DEFAULT_SUB_DIR = os.path.join(AI_SUB_DIR, 'default')
CONFIG_DEFAULT_DIR = os.path.join(fo.getUserDir(), DEFAULT_SUB_DIR)
CONFIG_DEFAULT_FILE = 'config.ini'

# CONFIG KEYS
TIMER_SECTION = 'Timers'
TIMERS_USE_TIMERS = 'timers_to_log'
TIMERS_TO_FILE = 'timers_dump'
TIMERS_DUMP_FOLDER = 'timers_dump_folder'
HANDLERS = 'handlers'

flat_options = odict()
sectioned_options = odict()


def check_bool(option):
    return str(option).lower() in ["1", "on", "true", "yes"]


def get_option_dict():
    """
    Return options for AI
    :return: ordered dict of options
    """

    if not flat_options:
        _parse_options()
    return flat_options


def get_sectioned_option_dict():
    """
    Return options for AI
    :return: ordered dict of ordered dicts of options
    """

    if not sectioned_options:
        _parse_options()
    return sectioned_options


def _parse_options():
    # get defaults; check if don't already exist and can write
    default_file = _get_option_file()
    if os.path.exists(default_file):
        config = SafeConfigParser()
        config.read([default_file])
    else:
        try:
            config = _create_default_config_file(default_file)
        except IOError:
            sys.stderr.write("AI Config: default file is not present and not writable at location %s\n" % default_file)
            config = _create_default_config_file(os.path.join(fo.getUserDir(), CONFIG_DEFAULT_FILE))

    option_file = _get_option_file()
    # if not os.path.exists(option_path):
    #    raise Exception('Error, option path "%s" does not exists.' % option_path)

    # read the defaults and then the specified config path
    if option_file:
        config_files = [option_file]
        configs_read = config.read(config_files)
        print "AI Config read config file(s): %s" % configs_read
        if len(configs_read) != len(config_files):
            sys.stderr.write("AI Config Error; could NOT read config file(s): %s"
                             % list(set(config_files).difference(configs_read)))
    for section in config.sections():
        sectioned_options.setdefault(section, odict())
        for k, v in config.items(section):
            flat_options[k] = v
            sectioned_options[section][k] = v


def _get_option_file():
    # hack to allow lunch this code separately to dump default config
    ai_path = _get_default_file_path()
    option_file = CONFIG_DEFAULT_FILE
    if ai_path and option_file:
        return os.path.join(ai_path, option_file)
    else:
        return None


def _get_default_file_path():
    try:
        if os.path.isdir(fo.getUserDir()) and not os.path.isdir(CONFIG_DEFAULT_DIR):
            os.makedirs(CONFIG_DEFAULT_DIR)
    except OSError:
        sys.stderr.write("AI Config Error: could not create path %s" % CONFIG_DEFAULT_DIR)
        return fo.getUserDir()

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
        with open(path, 'w') as configfile:
            config.write(configfile)
        print "default config is dumped to %s" % path
    return config

