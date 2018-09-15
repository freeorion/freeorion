"""Handle diplomatic messages and response determination."""
import random
from logging import debug

import freeOrionAIInterface as fo
from character.character_module import Aggression
from character.character_strings_module import possible_greetings
from freeorion_tools import UserStringList
from aistate_interface import get_aistate


def handle_pregame_chat(sender_player_id, message_txt):
    if fo.playerIsAI(sender_player_id):
        return
    possible_acknowledgments = UserStringList("AI_PREGAME_ACKNOWLEDGEMENTS__LIST")
    acknowledgement = random.choice(possible_acknowledgments)
    debug("Acknowledging pregame chat with initial message (from %d choices): '%s'" % (
        len(possible_acknowledgments), acknowledgement))
    fo.sendChatMessage(sender_player_id, acknowledgement)


class DiplomaticCorp(object):
    def __init__(self):
        self.be_chatty = True

    def handle_diplomatic_message(self, message):
        """Handle a diplomatic message update from the server,
        such as if another player declares war, accepts peace, or cancels a proposed peace treaty.
        :param message: message.recipient and message.sender are respective empire IDs
        :return:
        """
        debug("Received diplomatic %s message from %s to %s." % (
            message.type, fo.getEmpire(message.sender),
            'me' if message.recipient == fo.empireID() else fo.getEmpire(message.recipient)))
        # TODO: remove the following early return once proper support for third party diplomatic history is added
        if message.recipient != fo.empireID():
            return
        aistate = get_aistate()
        if message.type == fo.diplomaticMessageType.peaceProposal:
            aistate.log_peace_request(message.sender, message.recipient)
            proposal_sender_player = fo.empirePlayerID(message.sender)
            attitude = aistate.character.attitude_to_empire(message.sender, aistate.diplomatic_logs)
            possible_acknowledgments = []
            aggression = aistate.character.get_trait(Aggression)
            if aggression.key <= fo.aggression.typical:
                possible_acknowledgments = UserStringList("AI_PEACE_PROPOSAL_ACKNOWLEDGEMENTS_MILD_LIST")
                if attitude > 0:
                    possible_replies = UserStringList("AI_PEACE_PROPOSAL_RESPONSES_YES_MILD_LIST")
                else:
                    possible_replies = UserStringList("AI_PEACE_PROPOSAL_RESPONSES_NO_MILD_LIST")
            else:
                possible_acknowledgments = UserStringList("AI_PEACE_PROPOSAL_ACKNOWLEDGEMENTS_HARSH_LIST")
                if attitude > 0:
                    possible_replies = UserStringList("AI_PEACE_PROPOSAL_RESPONSES_YES_HARSH_LIST")
                else:
                    possible_replies = UserStringList("AI_PEACE_PROPOSAL_RESPONSES_NO_HARSH_LIST")
            acknowledgement = random.choice(possible_acknowledgments)
            reply_text = random.choice(possible_replies)
            debug("Acknowledging proposal with initial message (from %d choices): '%s'" % (
                len(possible_acknowledgments), acknowledgement))
            fo.sendChatMessage(proposal_sender_player, acknowledgement)
            if attitude > 0:
                diplo_reply = fo.diplomaticMessage(message.recipient, message.sender,
                                                   fo.diplomaticMessageType.acceptPeaceProposal)
                debug("Sending diplomatic message to empire %s of type %s" % (message.sender, diplo_reply.type))
                fo.sendDiplomaticMessage(diplo_reply)
            debug("sending chat to player %d of empire %d, message body: '%s'" % (
                proposal_sender_player, message.sender, reply_text))
            fo.sendChatMessage(proposal_sender_player, reply_text)
        elif message.type == fo.diplomaticMessageType.warDeclaration:
            # note: apparently this is currently (normally?) sent not as a warDeclaration,
            # but as a simple diplomatic_status_update to war
            aistate.log_war_declaration(message.sender, message.recipient)

    @staticmethod
    def get_first_turn_greet_message():
        greets = possible_greetings(get_aistate().character)
        # no such entry
        if len(greets) == 1 and greets[0] == '?':
            greets = UserStringList("AI_FIRST_TURN_GREETING_BEGINNER")
        return random.choice(greets)

    def handle_diplomatic_status_update(self, status_update):
        """Handle an update about the diplomatic status between players, which may
        or may not include this player."""
        debug("Received diplomatic status update to %s about empire %s and empire %s" % (
            status_update.status, status_update.empire1, status_update.empire2))
        if status_update.empire2 == fo.empireID() and status_update.status == fo.diplomaticStatus.war:
            get_aistate().log_war_declaration(status_update.empire1, status_update.empire2)

    def handle_midgame_chat(self, sender_player_id, message_txt):
        debug("Midgame chat received from player %d, message: %s" % (sender_player_id, message_txt))
        if fo.playerIsAI(sender_player_id) or not self.be_chatty:
            return
        if "BE QUIET" in message_txt.upper():
            possible_acknowledgments = UserStringList("AI_BE_QUIET_ACKNOWLEDGEMENTS__LIST")
            acknowledgement = random.choice(possible_acknowledgments)
            debug("Acknowledging 'Be Quiet' chat request with initial message (from %d choices): '%s'" % (
                len(possible_acknowledgments), acknowledgement))
            fo.sendChatMessage(sender_player_id, acknowledgement)
            self.be_chatty = False
            return
        if random.random() > 0.25:
            return
        possible_acknowledgments = UserStringList("AI_MIDGAME_ACKNOWLEDGEMENTS__LIST")
        acknowledgement = random.choice(possible_acknowledgments)
        debug("Acknowledging midgame chat with initial message (from %d choices): '%s'" % (
            len(possible_acknowledgments), acknowledgement))
        fo.sendChatMessage(sender_player_id, acknowledgement)
        self.be_chatty = False


class BeginnerDiplomaticCorp(DiplomaticCorp):
    pass


class ManiacalDiplomaticCorp(DiplomaticCorp):
    def __init__(self):
        DiplomaticCorp.__init__(self)
        self.be_chatty = False
