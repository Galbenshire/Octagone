#pragma once

#include "BallSpawner.hpp"

class GameServer {
public:
  explicit GameServer(sf::Vector2f playfieldSize);
  ~GameServer();

  void notifyPlayerSpawn(sf::Int8 paddleIdentifier);
  void notifyPlayerRealtimeChange(sf::Int8 paddleIdentifier, sf::Int32 action, bool actionEnabled);
  void notifyPlayerEvent(sf::Int8 paddleIdentifier, sf::Int32 action);

private:
  // A GameServerRemotePeer refers to one instance of the game, may it be local or from another computer
  struct RemotePeer {
    RemotePeer();

    sf::TcpSocket socket;
    sf::Time lastPacketTime;
    sf::Int8 paddleIdentifier;
    bool ready;
    bool timedOut;
  };

  // Structure to store information about current aircraft state
  struct PaddleInfo {
    float lerpPos;
    sf::Uint8 lives;
    std::map<sf::Int32, bool> realtimeActions;
  };

  // Unique pointer to remote peers
  typedef std::unique_ptr<RemotePeer> PeerPtr;

private:
  void setListening(bool enable);
  void executionThread();
  void tick();
  sf::Time now() const;

  void handleIncomingPackets();
  void handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout);

  void handleIncomingConnections();
  void handleDisconnections();

  void informWorldState(sf::TcpSocket& socket);
  void broadcastMessage(const std::string& message, const bool& important = false);
  void sendToAll(sf::Packet& packet);
  void updateClientState();

  void endGame();

private:
  sf::Thread mThread;
  sf::Clock mClock;
  sf::TcpListener mListenerSocket;
  bool mListeningState;
  sf::Time mClientTimeoutTime;

  std::size_t mMaxConnectedPlayers;
  std::size_t mConnectedPlayers;

  std::size_t mPaddleCount;
  std::map<sf::Int8, PaddleInfo> mPaddleInfo;

  std::vector<PeerPtr> mPeers;
  sf::Int8 mPaddleIdentifierCounter;
  bool mWaitingThreadEnd;

  bool mGameStarted;
  BallSpawner mBallSpawner;
};