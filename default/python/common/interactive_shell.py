import freeOrionAIInterface as fo
from freeorion_tools import chat_human
from code import InteractiveInterpreter
from cStringIO import StringIO
import sys


interpreter = InteractiveInterpreter({'fo': fo})
debug_mode = False


RED = '<rgba 255 0 0 255>%s</rgba>'
WHITE = '<rgba 255 255 255 255>%s</rgba>'
ENTERING_DEBUG_MESSAGE = 'Entering debug mode'


def handle_debug_chat(sender, message):
    global debug_mode
    human_id = [x for x in fo.allPlayerIDs() if fo.playerIsHost(x)][0]
    ais = [x for x in fo.allPlayerIDs() if not fo.playerIsHost(x)]
    is_debug_chat = False
    if message == ENTERING_DEBUG_MESSAGE:
        is_debug_chat = True
    if sender != human_id:
        return is_debug_chat  # don't chat with bots
    elif message == 'stop':
        is_debug_chat = True
        if debug_mode:
            chat_human("exiting debug mode")
        debug_mode = False
    elif debug_mode:
        print '>', message,
        is_debug_chat = True
        out, err = [x.strip('\n') for x in shell(message)]
        if out:
            chat_human(WHITE % out)
        if err:
            chat_human(RED % err)
    elif message.startswith('start'):
        is_debug_chat = True
        try:
            player_id = int(message[5:].strip())
        except ValueError as e:
            print e
            chat_human(str(e))
            return True
        if player_id == fo.playerID():
            debug_mode = True

            initial_code = [
                'import FreeOrionAI as foAI',
            ]

            # add some variables to scope: (name, help text, value)
            scopes_variable = (
                ('ai', 'aistate', 'foAI.foAIstate'),
                ('u', 'universe', 'fo.getUniverse()'),
                ('e', 'empire', 'fo.getEmpire()'),
            )
            for var, _, code in scopes_variable:
                initial_code.append('%s = %s' % (var, code))

            shell(';'.join(initial_code))

            variable_template = '<u><rgba 255 255 0 255>%s</rgba></u>%s %s'
            variables = (variable_template % (var, ' ' * (3 - len(var)), name) for var, name, _ in scopes_variable)
            chat_human(WHITE % "%s\n"
                               "Print <rgba 255 255 0 255>'stop'</rgba> to exit.\n"
                               "Local variables:\n"
                               "  %s" % (ENTERING_DEBUG_MESSAGE, '\n  '.join(variables)))

    elif message == 'help':
        is_debug_chat = True
        if ais[0] == fo.playerID():
            chat_human(WHITE % "Chat commands:")
            chat_human(WHITE % "  <u><rgba 0 255 255 255>start id</rgba></u>: start debug for selected empire")
            chat_human(WHITE % "  <u><rgba 0 255 255 255>stop</rgba></u>: stop debug")
            chat_human(WHITE % "Empire ids:")
            for player in fo.allPlayerIDs():
                if not fo.playerIsHost(player):
                    chat_human('  <rgba {0.colour.r} {0.colour.g} {0.colour.b} {0.colour.a}>id={0.empireID} empire_name={0.name}</rgba> player_name={1}'.format(fo.getEmpire(fo.playerEmpireID(player)), fo.playerName(player)))
    return is_debug_chat


def shell(msg):
    old_stdout = sys.stdout
    old_stderr = sys.stderr

    sys.stdout = StringIO()
    sys.stderr = StringIO()
    interpreter.runsource(msg)

    sys.stdout.seek(0)
    out = sys.stdout.read()
    sys.stderr.seek(0)
    err = sys.stderr.read()
    sys.stdout = old_stdout
    sys.stderr = old_stderr
    return out, err
