#ifndef _Message_h_
#define _Message_h_

#ifndef	_XMLDoc_h_
#include "../GG/XML/XMLDoc.h"
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <string>

/** compresses \a str using zlib, and puts the result into \a zipped_str */
void ZipString(std::string str, std::string& zipped_str);

/** decompresses \a str using zlib, and puts the result into \a unzipped_str. The uncompressed size of 
   the string must be known beforehand, and passed in \a size.  Results are undefined when \a str does 
   not conatain a valid zipped byte sequence.*/
void UnzipString(std::string str, std::string& unzipped_str, int size);



class ClientNetworkCore;
class ServerNetworkCore;

/** FreeOrion network message class.  Messages are designed to be created, sent, received, read, then destroyed.
   They are not meant to be altered; there are no mutators methods. Upon creation, the text of the Message is
   compressed using zlib.  Subsequent reads incur the expense of decompressing the text.  The compressed text
   is stored in a string at the end of a shared_ptr, so copying Message objects is extremely inexpensive.*/
class Message
{
public:
   /** Represents the type of the message */
   enum MessageType {SERVER_STATUS,       ///< sent to the client when requested, and when the server first recieves a connection from a client
                     HOST_GAME,           ///< sent when a client wishes to establish a game at the server
                     JOIN_GAME,           ///< sent when a client wishes to join a game being established at the server
                     SAVE_GAME,           ///< sent to server (by the "host" client only) when a game is to be saved, or from the server to the clients when the game is being saved
                     LOAD_GAME,           ///< sent to server (by the "host" client only) when a game is to be loaded, or from the server to the clients when the game is being loaded
                     GAME_START,          ///< sent to each client before the first TURN_UPDATE of a new or newly loaded game
                     TURN_UPDATE,         ///< sent to a client when the server updates the client Universes and Empires, and sends the SitReps each turn; indicates to the receiver that a new turn has begun
                     TURN_ORDERS,         ///< sent to the server by a client that has orders to be processed at the end of a turn or when a game is saved
                     COMBAT_START,        ///< sent to clients when a combat is about to start
                     COMBAT_ROUND_UPDATE, ///< sent to clients when a combat round has been resolved
                     COMBAT_END,          ///< sent to clients when a combat is concluded
                     HUMAN_PLAYER_MSG,    ///< sent when one player sends a text message to another in multiplayer
                     PLAYER_ELIMINATED,   ///< sent to all clients when a player is eliminated from play
                     PLAYER_EXIT          ///< sent to the "host" client when another player leaves the game
                    };
               
   /** Represents the module which is the destination for the message */
   enum ModuleType {CORE,                    ///< this module is the ServerCore or ClientCore, as appropriate; all server-bound messages go here
                    CLIENT_UNIVERSE_MODULE,  ///< the ClientUniverse module
                    CLIENT_EMPIRE_MODULE,    ///< the ClientEmpire module
                    CLIENT_COMBAT_MODULE     ///< the client Combat module
                    // are other server modules necessary?
                   };

   /** \name Structors */ //@{
   /** standard ctor.  Senders that are not part of a game and so have no player number should send -1 as the \a 
      sender parameter.   \throw std::invalid_argument May throw std::invalid_argument if the parameters would form
      an invalid message */
   Message(MessageType msg_type, int sender, int receiver, ModuleType module, const std::string& text);
 
   /** convienience ctor that converts \a doc into a std::string automatically.  Senders that are not part of a game 
      and so have no player number should send -1 as the \a sender parameter.   \throw std::invalid_argument May
      throw std::invalid_argument if the parameters would form an invalid message */
   Message(MessageType msg_type, int sender, int receiver, ModuleType module, const GG::XMLDoc& doc);
   //@}

   /** \name Accessors */ //@{
   MessageType Type() const;      ///< returns type of message
   int         Sender() const;    ///< returns the ID of the player sending the message (-1 represents server or a client not yet in a game)
   int         Receiver() const;  ///< returns the ID of the player receiving the message (-1 represents server)
   ModuleType  Module() const;    ///< returns the module that is to get the message at the receiving end
   std::string GetText() const;   ///< returns the message text.  \note \note This function uncompresses the text in order to return it.
   //@}
 
private:
   /** private ctor to be used by the NetworkCore classes. Constructs a message from a string of bytes received over a 
      network connection. \throw std::invalid_argument May throw std::invalid_argument if the parameter would form
      an invalid message */
   Message(const std::string& raw_msg);
   
   std::string HeaderString() const; ///< for use by the NetworkCore classes to create a string of bytes of the non-text portion of a Message
   void ValidateMessage(); ///< checks that the data in the message are consistent
   void CompressMessage(std::string& compressed_msg) const;     ///< fills \a compressed_msg with the compressed text of the message text
   void DecompressMessage(std::string& uncompressed_msg) const; ///< fills \a decompressed_msg with the decompressed text of the message text

  MessageType                    m_message_type;
  int                            m_sending_player;
  int                            m_receiving_player;
  ModuleType                     m_receiving_module;
  boost::shared_ptr<std::string> m_message_text;
  bool                           m_compressed;
  int                            m_uncompressed_size;
 
  friend class NetworkCore;         ///< grant access for calls to HeaderString()
  friend class ClientNetworkCore;   ///< grant access for calls to private ctor and HeaderString()
  friend class ServerNetworkCore;   ///< grant access for calls to private ctor and HeaderString()
};

/** creates a HOST_GAME message*/
Message HostGameMessage(const GG::XMLDoc& game_parameters);

/** creates a JOIN_GAME message*/
Message JoinGameMessage(const std::string& player_name);

/** creates a JOIN_GAME acknowledgement message*/
Message JoinAckMessage(int player_id);

/** creates a HOST_GAME acknowledgement message*/
Message HostAckMessage(int player_id);

#endif // _Message_h_

