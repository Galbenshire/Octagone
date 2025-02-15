#pragma once
#include "State.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "GameServer.hpp"
#include "NetworkProtocol.hpp"

class MultiplayerGameState : public State {
public:
  MultiplayerGameState(StateStack& stack, Context context, bool isHost);

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event& event);
  virtual void onActivate();
  void onDestroy();

  void disableAllRealtimeActions();

private:
  void updateBroadcastMessage(sf::Time elapsedTime);
  void handlePacket(sf::Uint8 packetType, sf::Packet& packet);

private:
  typedef std::unique_ptr<Player> PlayerPtr;

private:
  World mWorld;
  sf::RenderWindow& mWindow;

  std::map<sf::Int8, PlayerPtr> mPlayers;
  sf::Int8 mLocalPlayerIdentifier;
  sf::TcpSocket mSocket;
  bool mConnected;
  std::unique_ptr<GameServer> mGameServer;
  sf::Clock mTickClock;

  std::vector<std::string> mBroadcasts;
  sf::Text mBroadcastText;
  sf::Time mBroadcastElapsedTime;

  sf::Text mConnectionText;
  sf::Clock mFailedConnectionClock;

  bool mIsSpectating; //Is the player merely watching the game?
  sf::Text mSpectatingText;

  sf::Text mWaitForPlayerText;

  bool mActiveState;
  bool mHasFocus;
  bool mHost;
  bool mGameStarted;
  sf::Time mClientTimeout;
  sf::Time mTimeSinceLastPacket;
};