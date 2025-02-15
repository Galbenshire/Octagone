#include "pch.h"
#include "GameServer.hpp"
#include "Constants.hpp"
#include "Utility.hpp"
#include "NetworkProtocol.hpp"

GameServer::RemotePeer::RemotePeer()
  : ready(false)
  , timedOut(false)
{
  socket.setBlocking(false);
}

GameServer::GameServer(sf::Vector2f playfieldSize)
  : mThread(&GameServer::executionThread, this)
  , mListeningState(false)
  , mClientTimeoutTime(sf::seconds(3.f))
  , mMaxConnectedPlayers(MAX_PLAYERS)
  , mConnectedPlayers(0)
  , mPaddleCount(0)
  , mPeers(1)
  , mPaddleIdentifierCounter(1)
  , mWaitingThreadEnd(false)
  , mGameStarted(false)
  , mBallSpawner(mClock)
{
  mListenerSocket.setBlocking(false);
  mPeers[0].reset(new RemotePeer());
  mThread.launch();
}

GameServer::~GameServer() {
  mWaitingThreadEnd = true;
  mThread.wait();
}

void GameServer::notifyPlayerSpawn(sf::Int8 paddleIdentifier) {
  for (std::size_t i = 0; i < mConnectedPlayers; ++i) {
    if (mPeers[i]->ready) {
      sf::Packet packet;
      packet << static_cast<sf::Uint8>(ServerPacket::PlayerConnect);
      packet << paddleIdentifier << mPaddleInfo[paddleIdentifier].lerpPos << mPaddleInfo[paddleIdentifier].lives;
      mPeers[i]->socket.send(packet);
    }
  }
}

void GameServer::notifyPlayerRealtimeChange(sf::Int8 paddleIdentifier, sf::Int32 action, bool actionEnabled) {
  for (std::size_t i = 0; i < mConnectedPlayers; ++i) {
    if (mPeers[i]->ready) {
      sf::Packet packet;
      packet << static_cast<sf::Uint8>(ServerPacket::PlayerRealtimeChange);
      packet << paddleIdentifier;
      packet << action;
      packet << actionEnabled;

      mPeers[i]->socket.send(packet);
    }
  }
}

void GameServer::notifyPlayerEvent(sf::Int8 paddleIdentifier, sf::Int32 action) {
  for (std::size_t i = 0; i < mConnectedPlayers; ++i) {
    if (mPeers[i]->ready) {
      sf::Packet packet;
      packet << static_cast<sf::Uint8>(ServerPacket::PlayerEvent);
      packet << paddleIdentifier;
      packet << action;

      mPeers[i]->socket.send(packet);
    }
  }
}

void GameServer::setListening(bool enable) {
  // Check if it isn't already listening
  if (enable) {
    if (!mListeningState)
      mListeningState = (mListenerSocket.listen(ServerPort) == sf::TcpListener::Done);
  } else {
    mListenerSocket.close();
    mListeningState = false;
  }
}

void GameServer::executionThread() {
  setListening(true);

  sf::Time stepInterval = sf::seconds(1.f / 60.f);
  sf::Time stepTime = sf::Time::Zero;
  sf::Time tickInterval = sf::seconds(1.f / 20.f);
  sf::Time tickTime = sf::Time::Zero;
  sf::Clock stepClock, tickClock;

  while (!mWaitingThreadEnd) {
    handleIncomingPackets();
    handleIncomingConnections();

    stepTime += stepClock.getElapsedTime();
    stepClock.restart();

    tickTime += tickClock.getElapsedTime();
    tickClock.restart();

    // Fixed update step
    while (stepTime >= stepInterval) {
      stepTime -= stepInterval;
    }

    // Fixed tick step
    while (tickTime >= tickInterval) {
      tick();
      tickTime -= tickInterval;
    }

    // Sleep to prevent server from consuming 100% CPU
    sf::sleep(sf::milliseconds(100));
  }
}

void GameServer::tick() {
  updateClientState();

  if (!mGameStarted) { return; }

  //Remove paddles whose health have gone to 0 (both on the server's end and on each client's end
  for (auto it = mPaddleInfo.begin(); it != mPaddleInfo.end(); ) {
    if (it->second.lives == 0 && mPaddleInfo.size() > 1) {
      // Inform everyone of this player's death
      sendToAll(sf::Packet() << static_cast<sf::Uint8>(ServerPacket::RemoveDeadPlayer) << it->first);

      mBallSpawner.removePlayerID(it->first);
      it = mPaddleInfo.erase(it);
    } else {
      ++it;
    }
  }

  //Only one paddle remains, state that the game is over
  if (mPaddleInfo.size() <= 1) {
    endGame();
  }

  if (mBallSpawner.isTimeToSpawnBall()) {
    //Spawn a ball
    sf::Packet packet = mBallSpawner.createBallSpawnPacket();
    sendToAll(packet);

    mBallSpawner.reset();
  }
}

sf::Time GameServer::now() const {
  return mClock.getElapsedTime();
}

void GameServer::handleIncomingPackets() {
  bool detectedTimeout = false;

  for (PeerPtr& peer : mPeers) {
    if (peer->ready) {
      sf::Packet packet;
      while (peer->socket.receive(packet) == sf::Socket::Done) {
        // Interpret packet and react to it
        handleIncomingPacket(packet, *peer, detectedTimeout);

        // Packet was indeed received, update the ping timer
        peer->lastPacketTime = now();
        packet.clear();
      }

      if (now() >= peer->lastPacketTime + mClientTimeoutTime) {
        peer->timedOut = true;
        detectedTimeout = true;
      }
    }
  }

  if (detectedTimeout)
    handleDisconnections();
}

void GameServer::handleIncomingPacket(sf::Packet & packet, RemotePeer & receivingPeer, bool & detectedTimeout) {
  sf::Uint8 packetType;
  packet >> packetType;

  switch (packetType) {
    case static_cast<sf::Uint8>(ClientPacket::StartGame) :
    {
      sendToAll(sf::Packet() << static_cast<sf::Uint8>(ServerPacket::StartGame));
      mGameStarted = true;
      mBallSpawner.reset();
      setListening(false); //Tell the server to not accept any more players; the game has started
    } break;

    case static_cast<sf::Uint8>(ClientPacket::PlayerEvent):
    {
      sf::Int8 paddleIdentifier;
      sf::Int32 action;
      packet >> paddleIdentifier >> action;
      if (mPaddleInfo.count(paddleIdentifier) != 0) {
        notifyPlayerEvent(paddleIdentifier, action);
      }
    } break;

    case static_cast<sf::Uint8>(ClientPacket::PlayerRealtimeChange):
    {
      sf::Int8 paddleIdentifier;
      sf::Int32 action;
      bool actionEnabled;
      packet >> paddleIdentifier >> action >> actionEnabled;
      if (mPaddleInfo.count(paddleIdentifier) != 0) {
        mPaddleInfo[paddleIdentifier].realtimeActions[action] = actionEnabled;
        notifyPlayerRealtimeChange(paddleIdentifier, action, actionEnabled);
      }
    } break;

    case static_cast<sf::Uint8>(ClientPacket::PlayerRealtimeChangeGamepad):
    {
      sf::Int8 paddleIdentifier;
      sf::Int32 action;
      sf::Int32 oppositeAction;
      bool actionEnabled;
      packet >> paddleIdentifier >> action >> actionEnabled >> oppositeAction;
      if (mPaddleInfo.count(paddleIdentifier) != 0) {
        mPaddleInfo[paddleIdentifier].realtimeActions[action] = actionEnabled;
        notifyPlayerRealtimeChange(paddleIdentifier, action, actionEnabled);
        notifyPlayerRealtimeChange(paddleIdentifier, oppositeAction, false);
      }
    } break;

    case static_cast<sf::Uint8>(ClientPacket::PositionUpdate):
    {
      sf::Int8 paddleIdentifier;
      float paddleLerpPosition;
      sf::Uint8 lives;
      packet >> paddleIdentifier >> paddleLerpPosition >> lives;
      if (mPaddleInfo.count(paddleIdentifier) != 0) {
        mPaddleInfo[paddleIdentifier].lerpPos = paddleLerpPosition;
        mPaddleInfo[paddleIdentifier].lives = lives;
      }
    } break;

    case static_cast<sf::Uint8>(ClientPacket::GameEvent):
    {
      sf::Int32 action;
      float x, y;

      packet >> action;
      packet >> x;
      packet >> y;

      //All balls currently in view are removed
      if (action == GameActions::PlayerHurt) {
        sendToAll(sf::Packet() << static_cast<sf::Uint8>(ServerPacket::RemoveBalls));

        mBallSpawner.appendToSpawnTime(sf::seconds(1.f));
        mBallSpawner.rollBackBallCount(5);
      }
    } break;

    case static_cast<sf::Uint8>(ClientPacket::Quit):
    {
      receivingPeer.timedOut = true;
      detectedTimeout = true;
    } break;
  }
}

void GameServer::handleIncomingConnections() {
  if (!mListeningState || mConnectedPlayers >= mMaxConnectedPlayers)
    return;

  if (mListenerSocket.accept(mPeers[mConnectedPlayers]->socket) == sf::TcpListener::Done) {
    //Order the new client to spawn its paddle
    mPaddleInfo[mPaddleIdentifierCounter].lerpPos = 0.5f;
    mPaddleInfo[mPaddleIdentifierCounter].lives = 3;

    sf::Packet packet;
    packet << static_cast<sf::Uint8>(ServerPacket::SpawnSelf);
    packet << mPaddleIdentifierCounter;
    packet << mPaddleInfo[mPaddleIdentifierCounter].lerpPos;
    packet << mPaddleInfo[mPaddleIdentifierCounter].lives;
    
    mPeers[mConnectedPlayers]->paddleIdentifier = mPaddleIdentifierCounter;

    broadcastMessage("New player!");
    informWorldState(mPeers[mConnectedPlayers]->socket);
    notifyPlayerSpawn(mPaddleIdentifierCounter);
    mBallSpawner.addPlayerID(mPaddleIdentifierCounter);

    //Raise the counter to the next available paddle ID
    while (mPaddleInfo.find(++mPaddleIdentifierCounter) != mPaddleInfo.end() && mPaddleIdentifierCounter != 8) {}

    mPeers[mConnectedPlayers]->socket.send(packet);
    mPeers[mConnectedPlayers]->ready = true;
    mPeers[mConnectedPlayers]->lastPacketTime = now(); //Prevent initial timeouts
    mPaddleCount++;
    mConnectedPlayers++;

    if (mConnectedPlayers >= mMaxConnectedPlayers)
      setListening(false);
    else // Add a new waiting peer
      mPeers.push_back(PeerPtr(new RemotePeer()));
  }
}

void GameServer::handleDisconnections() {
  for (auto itr = mPeers.begin(); itr != mPeers.end(); ) {
    if ((*itr)->timedOut) {
      size_t prior_ConnectedPlayers(mConnectedPlayers);
      sf::Int8 oldID((*itr)->paddleIdentifier);

      // Inform everyone of the disconnection, erase
      sendToAll(sf::Packet() << static_cast<sf::Uint8>(ServerPacket::PlayerDisconnect) << (*itr)->paddleIdentifier);

      mPaddleInfo.erase((*itr)->paddleIdentifier);

      mConnectedPlayers--;
      mPaddleCount--;

      itr = mPeers.erase(itr);
      mBallSpawner.removePlayerID(mPaddleIdentifierCounter);

      //Reduce the paddle ID count if it's lower than the current value
      mPaddleIdentifierCounter = (oldID < mPaddleIdentifierCounter) ? oldID : mPaddleIdentifierCounter;

      // Go back to a listening state if needed
      if (prior_ConnectedPlayers >= mMaxConnectedPlayers && !mGameStarted) {
        mPeers.push_back(PeerPtr(new RemotePeer()));
        setListening(true);
      }

      broadcastMessage("A player\n has disconnected.");
    } else {
      ++itr;
    }
  }
}

// Tell the newly connected peer about how the world is currently
void GameServer::informWorldState(sf::TcpSocket & socket) {
  sf::Packet packet;
  packet << static_cast<sf::Uint8>(ServerPacket::InitialState);
  packet << static_cast<sf::Uint8>(mPaddleCount);

  for (std::size_t i = 0; i < mConnectedPlayers; ++i) {
    sf::Int8 id;
    if (mPeers[i]->ready) {
      id = mPeers[i]->paddleIdentifier;
      packet << id << mPaddleInfo[id].lerpPos << mPaddleInfo[id].lives;
    }
  }

  socket.send(packet);
}

void GameServer::broadcastMessage(const std::string & message, const bool& important) {
  for (std::size_t i = 0; i < mConnectedPlayers; ++i) {
    if (mPeers[i]->ready) {
      sf::Packet packet;
      packet << static_cast<sf::Uint8>(ServerPacket::BroadcastMessage);
      packet << message;
      packet << important;

      mPeers[i]->socket.send(packet);
    }
  }
}

void GameServer::sendToAll(sf::Packet & packet) {
  for (PeerPtr& peer : mPeers) {
    if (peer->ready)
      peer->socket.send(packet);
  }
}

void GameServer::updateClientState() {
  sf::Packet updateClientStatePacket;
  updateClientStatePacket << static_cast<sf::Uint8>(ServerPacket::UpdateClientState);
  updateClientStatePacket << static_cast<sf::Uint8>(mPaddleInfo.size());

  for (auto paddle : mPaddleInfo)
    updateClientStatePacket << paddle.first << paddle.second.lerpPos << paddle.second.lives;

  sendToAll(updateClientStatePacket);
}

void GameServer::endGame() {
  std::string winnerText("The winner is\nPlayer ");
  winnerText.append(std::to_string(mPaddleInfo.begin()->first));
  broadcastMessage(winnerText, true);

  sendToAll(sf::Packet() << static_cast<sf::Uint8>(ServerPacket::EndGame));

  mGameStarted = false;
}
