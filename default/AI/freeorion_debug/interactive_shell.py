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
        is_debug_chat = True
        out, err = shell(message)
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
            # add some variables to scope
            lines = ['import FreeOrionAI as foAI',
                      'ai = foAI.foAIstate',
                      'u = fo.getUniverse()',
                      'e = fo.getEmpire()'
                      ]
            shell(';'.join(lines))
            # Notify all players this AI entering debug mode
            fo.sendChatMessage(-1, WHITE % ENTERING_DEBUG_MESSAGE)
            chat_human(WHITE % "Print 'stop' to exit.")
            chat_human(WHITE % " Local vars: <u>u</u>(universe), <u>e</u>(empire), <u>ai</u>(aistate)")

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
