"""Handle diplomatic messages and response determination."""

import random
import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
from freeorion_tools import UserStringList, chat_on_error


def handle_pregame_chat(sender_player_id, message_txt):
    if fo.playerIsAI(sender_player_id):
        return
    possible_acknowledgments = UserStringList("AI_PREGAME_ACKNOWLEDGEMENTS__LIST")
    acknowledgement = random.choice(possible_acknowledgments)
    print "Acknowledging pregame chat with initial message (from %d choices): '%s'" % (
        len(possible_acknowledgments), acknowledgement)
    fo.sendChatMessage(sender_player_id, acknowledgement)


class DiplomaticCorp(object):
    def __init__(self):
        self.be_chatty = True

    @chat_on_error
    def handle_diplomatic_message(self, message):
        """Handle a diplomatic message update from the server,
        such as if another player declares war, accepts peace, or cancels a proposed peace treaty.
        :param message: message.recipient and message.sender are respective empire IDs
        :return:
        """
        print "Received diplomatic %s message from %s to %s." % (
            message.type, fo.getEmpire(message.sender),
            'me' if message.recipient == fo.empireID() else fo.getEmpire(message.recipient))
        # TODO: remove the following early return once proper support for third party diplomatic history is added
        if message.recipient != fo.empireID():
            return
        if message.type == fo.diplomaticMessageType.peaceProposal:
            foAI.foAIstate.log_peace_request(message.sender, message.recipient)
            proposal_sender_player = fo.empirePlayerID(message.sender)
            attitude = self.evaluate_diplomatic_attitude(message.sender)
            possible_acknowledgments = []
            if foAI.foAIstate.aggression <= fo.aggression.typical:
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
            print "Acknowledging proposal with initial message (from %d choices): '%s'" % (
                len(possible_acknowledgments), acknowledgement)
            fo.sendChatMessage(proposal_sender_player, acknowledgement)
            if attitude > 0:
                diplo_reply = fo.diplomaticMessage(message.recipient, message.sender,
                                                   fo.diplomaticMessageType.acceptProposal)
                print "Sending diplomatic message to empire %s of type %s" % (message.sender, diplo_reply.type)
                fo.sendDiplomaticMessage(diplo_reply)
            print "sending chat to player %d of empire %d, message body: '%s'" % (
                proposal_sender_player, message.sender, reply_text)
            fo.sendChatMessage(proposal_sender_player, reply_text)
        elif message.type == fo.diplomaticMessageType.warDeclaration:
            # note: apparently this is currently (normally?) sent not as a warDeclaration,
            # but as a simple diplomatic_status_update to war
            foAI.foAIstate.log_war_declaration(message.sender, message.recipient)

    @staticmethod
    def get_first_turn_greet_message():
        greet_lists = {
            fo.aggression.beginner:
                UserStringList("AI_FIRST_TURN_GREETING_LIST_BEGINNER"),
            fo.aggression.turtle:
                UserStringList("AI_FIRST_TURN_GREETING_LIST_TURTLE"),
            fo.aggression.cautious:
                UserStringList("AI_FIRST_TURN_GREETING_LIST_CAUTIOUS"),
            fo.aggression.typical:
                UserStringList("AI_FIRST_TURN_GREETING_LIST_TYPICAL"),
            fo.aggression.aggressive:
                UserStringList("AI_FIRST_TURN_GREETING_LIST_AGGRESSIVE"),
            fo.aggression.maniacal:
                UserStringList("AI_FIRST_TURN_GREETING_LIST_MANIACAL"),
        }
        greets = greet_lists.get(foAI.foAIstate.aggression, UserStringList("AI_FIRST_TURN_GREETING_LIST_BEGINNER"))
        # no such entry
        if len(greets) == 1 and greets[0] == 'ERROR: %s' % key:
            greets = UserStringList("AI_FIRST_TURN_GREETING_LIST_BEGINNER")
        return random.choice(greets)

    @chat_on_error
    def handle_diplomatic_status_update(self, status_update):
        """Handle an update about the diplomatic status between players, which may
        or may not include this player."""
        print "Received diplomatic status update to %s about empire %s and empire %s" % (
            status_update.status, status_update.empire1, status_update.empire2)
        if status_update.empire2 == fo.empireID() and status_update.status == fo.diplomaticStatus.war:
            foAI.foAIstate.log_war_declaration(status_update.empire1, status_update.empire2)

    @chat_on_error
    def evaluate_diplomatic_attitude(self, other_empire_id):
        """Evaluate this empire's diplomatic attitude regarding the other empire.
        :param other_empire_id: integer
        :return: a numeric rating, currently in the range [-10 : 10]
        """

        # TODO: consider proximity, competitive needs, relations with other empires, past history with this empire, etc.
        # in the meantime, somewhat random
        log_index = (other_empire_id, fo.empireID())
        num_peace_requests = len(foAI.foAIstate.diplomatic_logs.get('peace_requests', {}).get(log_index, []))
        num_war_declarations = len(foAI.foAIstate.diplomatic_logs.get('war_declarations', {}).get(log_index, []))
        # Too many requests for peace irritate the AI, as do any war declarations
        irritation = (foAI.foAIstate.aggression * (2.0 + num_peace_requests / 10.0 + 2.0 * num_war_declarations) + 0.5)
        attitude = 10 * random.random() - irritation
        return min(10, max(-10, attitude))

    def handle_midgame_chat(self, sender_player_id, message_txt):
        print "Midgame chat received from player %d, message: %s" % (sender_player_id, message_txt)
        if fo.playerIsAI(sender_player_id) or not self.be_chatty:
            return
        if "BE QUIET" in message_txt.upper():
            possible_acknowledgments = UserStringList("AI_BE_QUIET_ACKNOWLEDGEMENTS__LIST")
            acknowledgement = random.choice(possible_acknowledgments)
            print "Acknowledging 'Be Quiet' chat request with initial message (from %d choices): '%s'" % (
                len(possible_acknowledgments), acknowledgement)
            fo.sendChatMessage(sender_player_id, acknowledgement)
            self.be_chatty = False
            return
        if random.random() > 0.25:
            return
        possible_acknowledgments = UserStringList("AI_MIDGAME_ACKNOWLEDGEMENTS__LIST")
        acknowledgement = random.choice(possible_acknowledgments)
        print "Acknowledging midgame chat with initial message (from %d choices): '%s'" % (
            len(possible_acknowledgments), acknowledgement)
        fo.sendChatMessage(sender_player_id, acknowledgement)
        self.be_chatty = False


class BeginnerDiplomaticCorp(DiplomaticCorp):
    def evaluate_diplomatic_attitude(self, other_empire_id):
        return 9


class ManiacalDiplomaticCorp(DiplomaticCorp):
    def __init__(self):
        DiplomaticCorp.__init__(self)
        self.be_chatty = False

    def evaluate_diplomatic_attitude(self, other_empire_id):
        return -9
