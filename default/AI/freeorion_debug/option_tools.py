import os
from ConfigParser import SafeConfigParser
from collections import OrderedDict as odict
import platform

try:
    import freeOrionAIInterface as fo  # pylint: disable=import-error
except ImportError:
    print "Executing outside of FreeOrion."


DEFAULT_CONFIG_FILE = 'default_config.ini'

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
    default_file = _get_default_file_path()
    if os.path.exists(default_file):
        config = SafeConfigParser()
        config.read([default_file])
    else:
        if platform.system != "Linux":
            default_file = ""
        try:
            config = _create_default_config_file(default_file)
        except IOError:
            print "AI Config: default file is not present and not writable at location %s" % default_file
            config = _create_default_config_file("")

    option_path = _get_option_file_path()
    #if not os.path.exists(option_path):
    #    raise Exception('Error, option path "%s" does not exists.' % option_path)

    # read the defaults and then the specified config path
    if option_path:
        config_files = [option_path]
        configs_read = config.read(config_files)
        print "AI Config read config file(s): %s" % configs_read
        if len(configs_read) != len(config_files):
            print "AI Config Error; could NOT read config file(s): %s" % list(set(config_files).difference(configs_read))
    for section in config.sections():
        sectioned_options.setdefault(section, odict())
        for k, v in config.items(section):
            flat_options[k] = v
            sectioned_options[section][k] = v


def _get_AI_folder_path():
    # hack to allow lunch this code separately to dump default config
    try:
        return fo.getAIDir()
    except AttributeError as e:
        print "Cant get options file", e
        return None


def _get_option_file_name():
    # hack to allow lunch this code separately to dump default config
    try:
        return fo.getAIConfigStr()
    except AttributeError as e:
        print "Cant get options file", e
        return None


def _get_option_file_path():
    # hack to allow lunch this code separately to dump default config
    ai_path = _get_AI_folder_path()
    option_file = _get_option_file_name()
    if ai_path and option_file:
        return os.path.join(ai_path, option_file)
    else:
        return None


def _get_default_file_path():
    # TODO: determine more robust treatment in case ResourceDir is not writable by user
    return os.path.join(_get_AI_folder_path() or ".", DEFAULT_CONFIG_FILE)


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


if __name__ == '__main__':
    _create_default_config_file(_get_default_file_path())
