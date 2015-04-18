import freeOrionAIInterface as fo
from freeorion_tools import chat_human
from code import InteractiveInterpreter
from cStringIO import StringIO
import sys


interpreter = InteractiveInterpreter({'fo': fo})
debug_mode = False


RED = '<rgba 255 0 0 255>%s</rgba>'
WHITE = '<rgba 255 255 255 255>%s</rgba>'


def handle_debug_chat(sender, message):
    human_id = [x for x in fo.allPlayerIDs() if fo.playerIsHost(x)][0]
    ais = [x for x in fo.allPlayerIDs() if not fo.playerIsHost(x)]
    if sender != human_id:
        pass  # don't chat with bots
    elif message == 'stop':
        global debug_mode
        debug_mode = False
        if ais[0] == fo.playerID():
            chat_human("exiting debug mode")
    elif debug_mode:
        out, err = shell(message)
        if out:
            chat_human(WHITE % out)
        if err:
            chat_human(RED % err)
    elif message.startswith('start'):
        try:
            player_id = int(message[5:].strip())
        except ValueError as e:
            print e
            return
        if player_id == fo.playerID():
            global debug_mode
            debug_mode = True
            # add some variables to scope
            lines = ['import FreeOrionAI as foAI',
                      'ai = foAI.foAIstate',
                      'u = fo.getUniverse()',
                      'e = fo.getEmpire()'
                      ]
            shell(';'.join(lines))
            chat_human(WHITE % "Entering debug mode. print 'stop' to exit.")
            chat_human(WHITE % " Local vars: <u>u</u>(universe), <u>e</u>(empire), <u>ai</u>(aistate)")

    elif message == 'help':
        if ais[0] == fo.playerID():
            chat_human(WHITE % "Chat commands:")
            chat_human(WHITE % "  <u><rgba 0 255 255 255>start id</rgba></u>: start debug for selected empire")
            chat_human(WHITE % "  <u><rgba 0 255 255 255>stop</rgba></u>: stop debug")
            chat_human(WHITE % "Empire ids:")
            for player in fo.allPlayerIDs():
                if not fo.playerIsHost(player):
                    chat_human('  <rgba {0.colour.r} {0.colour.g} {0.colour.b} {0.colour.a}>id={0.empireID} empire_name={0.name}</rgba> player_name={1}'.format(fo.getEmpire(fo.playerEmpireID(player)), fo.playerName(player)))


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
