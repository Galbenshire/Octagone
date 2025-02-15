#include "pch.h"
#include "MultiplayerGameState.hpp"
#include "Utility.hpp"
#include "Constants.hpp"

namespace {
  sf::IpAddress getAddressFromFile() {
    { // Try to open existing file (RAII block)
      std::ifstream inputFile("ip.txt");
      std::string ipAddress;
      if (inputFile >> ipAddress)
        return ipAddress;
    }

    // If open/read failed, create new file
    std::ofstream outputFile;
    outputFile.open("ip.txt");
    std::string localAddress(DEFAULT_IP_ADDRESS);
    outputFile << localAddress << std::endl;
    outputFile.close();
    return localAddress;
  }
}

MultiplayerGameState::MultiplayerGameState(StateStack & stack, Context context, bool isHost)
  : State(stack, context)
  , mWorld(*context.window, *context.textures, *context.fonts, *context.sounds)
  , mWindow(*context.window)
  , mLocalPlayerIdentifier(-1)
  , mConnected(false)
  , mGameServer(nullptr)
  , mIsSpectating(false)
  , mActiveState(true)
  , mHasFocus(true)
  , mHost(isHost)
  , mGameStarted(false)
  , mClientTimeout(sf::seconds(2.f))
  , mTimeSinceLastPacket(sf::seconds(0.f))
{
  mBroadcastText.setFont(context.fonts->get(FontID::Main));
  mBroadcastText.setPosition(VIEW_WIDTH / 2, 32.f);

  mSpectatingText.setFont(context.fonts->get(FontID::Main));
  mSpectatingText.setString("--SPECTATING--");
  mSpectatingText.setCharacterSize(25);
  centerOrigin(mSpectatingText);
  mSpectatingText.setPosition(VIEW_WIDTH / 2, VIEW_HEIGHT * 0.75f);

  mWaitForPlayerText.setFont(context.fonts->get(FontID::Main));
  if (isHost)
    mWaitForPlayerText.setString("YOU NEED AT LEAST TWO PLAYERS\nTO PLAY THE GAME.\nPRESS -SPACE- WHEN READY.");
  else
    mWaitForPlayerText.setString("PLEASE WAIT UNTIL\nTHE HOST STARTS THE GAME");
  mWaitForPlayerText.setCharacterSize(20);
  centerOrigin(mWaitForPlayerText);
  mWaitForPlayerText.setPosition(mWindow.getSize().x / 2.f, mWindow.getSize().y / 2.f);

  // We reuse this text for "Attempt to connect" and "Failed to connect" messages
  mConnectionText.setFont(context.fonts->get(FontID::Main));
  mConnectionText.setString("Attempting to connect...");
  mConnectionText.setCharacterSize(20);
  mConnectionText.setFillColor(getGameColor(GameColors::COLOR_FOUR));
  centerOrigin(mConnectionText);
  mConnectionText.setPosition(mWindow.getSize().x / 2.f, mWindow.getSize().y / 2.f);

  // Render a "establishing connection" frame for user feedback
  mWindow.clear(getGameColor(GameColors::COLOR_ONE));
  mWindow.draw(mConnectionText);
  mWindow.display();
  mConnectionText.setString("Could not connect\nto the remote server!");
  mConnectionText.setFillColor(getGameColor(GameColors::COLOR_ONE));
  centerOrigin(mConnectionText);

  sf::IpAddress ip;
  if (isHost)
    mGameServer.reset(new GameServer(sf::Vector2f(mWindow.getSize())));
  ip = context.ip->toAnsiString();

  if (mSocket.connect(ip, ServerPort, sf::seconds(5.f)) == sf::TcpSocket::Done)
    mConnected = true;
  else
    mFailedConnectionClock.restart();

  mSocket.setBlocking(false);
}

void MultiplayerGameState::draw() {
  if (mConnected) {
    mWorld.draw();

    // Broadcast messages in default view
    mWindow.setView(mWindow.getDefaultView());

    if (!mBroadcasts.empty())
      mWindow.draw(mBroadcastText);

    if (mIsSpectating)
      mWindow.draw(mSpectatingText);

    if (!mGameStarted)
      mWindow.draw(mWaitForPlayerText);
  } else {
    mWindow.draw(mConnectionText);
  }
}

bool MultiplayerGameState::update(sf::Time dt) {
  // Connected to server: Handle all the network logic
  if (mConnected) {
    mWorld.update(dt);

    // Remove players whose aircrafts were destroyed
    bool foundLocalPaddle = false;
    for (auto itr = mPlayers.begin(); itr != mPlayers.end(); ) {
      // Check if there are no more local paddles for remote clients
      if (itr->first == mLocalPlayerIdentifier)
        foundLocalPaddle = true;

      if (!mWorld.getPaddle(itr->first))
        itr = mPlayers.erase(itr);
      else
        ++itr;
    }

    if (!foundLocalPaddle && mGameStarted) {
      //requestStackPush(States::GameOver);
    }

    // Only handle the realtime input if the window has focus and the game is unpaused
    if (mActiveState && mHasFocus && mGameStarted) {
      for (auto& pair : mPlayers)
        pair.second->handleRealtimeInput(mWorld.getCommandQueue());
    }

    // Always handle the network input
    CommandQueue& commands = mWorld.getCommandQueue();
    for (auto& pair : mPlayers)
      pair.second->handleRealtimeNetworkInput(commands);

    // Handle messages from server that may have arrived
    sf::Packet packet;
    if (mSocket.receive(packet) == sf::Socket::Done) {
      mTimeSinceLastPacket = sf::seconds(0.f);
      sf::Uint8 packetType;
      packet >> packetType;
      handlePacket(packetType, packet);
    } else {
      // Check for timeout with the server
      if (mTimeSinceLastPacket > mClientTimeout) {
        mConnected = false;

        mConnectionText.setString("Lost connection to server");
        centerOrigin(mConnectionText);

        mFailedConnectionClock.restart();
      }
    }

    updateBroadcastMessage(dt);

    // Events occurring in the game
    GameActions::Action gameAction;
    while (mWorld.pollGameAction(gameAction)) {
      sf::Packet packet;
      packet << static_cast<sf::Uint8>(ClientPacket::GameEvent);
      packet << static_cast<sf::Int32>(gameAction.type);
      packet << gameAction.position.x;
      packet << gameAction.position.y;

      mSocket.send(packet);
    }

    // Regular position updates
    if (mTickClock.getElapsedTime() > sf::seconds(1.f / 20.f) && mLocalPlayerIdentifier > 0) {
      sf::Packet positionUpdatePacket;
      positionUpdatePacket << static_cast<sf::Uint8>(ClientPacket::PositionUpdate);

      if (Paddle* paddle = mWorld.getPaddle(mLocalPlayerIdentifier)) {
        positionUpdatePacket << mLocalPlayerIdentifier << paddle->getSplinePos() << paddle->getLives();
      } else {
        //Dummy information. Even if the player has lost their paddle, they still need to send something or get kicked out
        positionUpdatePacket << mLocalPlayerIdentifier << 0.5f << static_cast<sf::Uint8>(0u);
      }

      mSocket.send(positionUpdatePacket);
      mTickClock.restart();
    }

    mTimeSinceLastPacket += dt;
  }

  // Failed to connect and waited for more than 5 seconds: Back to menu
  else if (mFailedConnectionClock.getElapsedTime() >= sf::seconds(5.f)) {
    requestStateClear();
    requestStackPush(StateID::MainMenu);
  }

  return true;
}

bool MultiplayerGameState::handleEvent(const sf::Event & event) {
  mWorld.resetEffect();

  if (mGameStarted) {
    // Forward event to all players
    for (auto& pair : mPlayers)
      pair.second->handleEvent(event, mHasFocus, mWorld.getCommandQueue());
  }

  if (event.type == sf::Event::KeyPressed) {
    // Escape pressed, trigger the pause screen
    if (event.key.code == sf::Keyboard::Escape && mGameStarted) {
      disableAllRealtimeActions();
      mWorld.setPauseEffect();
      requestStackPush(StateID::NetworkPause);
    } 
    //Is the game ready to be started? (The host must press 'Space' when waiting for players, must have two or more players)
    else if ((event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space)) { 
      if (!mGameStarted && mHost && mPlayers.size() >= 2) {
        //Send the packet to inform the server the game should start now
        sf::Packet startGamePacket;
        startGamePacket << static_cast<sf::Uint8>(ClientPacket::StartGame);
        mSocket.send(startGamePacket);
      }
    }
  } else if (event.type == sf::Event::GainedFocus) {
    mHasFocus = true;
  } else if (event.type == sf::Event::LostFocus) {
    mHasFocus = false;
  }

  return true;
}

void MultiplayerGameState::onActivate() {
  mActiveState = true;
}

void MultiplayerGameState::onDestroy() {
  if (!mHost && mConnected) {
    // Inform server this client is dying
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(ClientPacket::Quit);
    mSocket.send(packet);
  }
}

void MultiplayerGameState::disableAllRealtimeActions() {
  mActiveState = false;

  mPlayers[mLocalPlayerIdentifier]->disableAllRealtimeActions();
}

void MultiplayerGameState::updateBroadcastMessage(sf::Time elapsedTime) {
  if (mBroadcasts.empty())
    return;

  // Update broadcast timer
  mBroadcastElapsedTime += elapsedTime;
  if (mBroadcastElapsedTime > sf::seconds(2.5f)) {
    // If message has expired, remove it
    mBroadcasts.erase(mBroadcasts.begin());

    // Continue to display next broadcast message
    if (!mBroadcasts.empty()) {
      mBroadcastText.setString(mBroadcasts.front());
      centerOrigin(mBroadcastText);
      mBroadcastElapsedTime = sf::Time::Zero;
    }
  }
}

void MultiplayerGameState::handlePacket(sf::Uint8 packetType, sf::Packet & packet) {
  switch (packetType) {
    // Send message to all clients
    case static_cast<sf::Uint8>(ServerPacket::BroadcastMessage):
    {
      std::string message;
      bool important;
      packet >> message >> important;

      if (important) { mBroadcasts.clear(); }

      mBroadcasts.push_back(message);

      // Just added first message, display immediately
      if (mBroadcasts.size() == 1) {
        mBroadcastText.setString(mBroadcasts.front());
        centerOrigin(mBroadcastText);
        mBroadcastElapsedTime = sf::Time::Zero;
      }
    } break;

    // Sent by the server to order to spawn player paddle on connect
    case static_cast<sf::Uint8>(ServerPacket::SpawnSelf):
    {
      sf::Int8 paddleIdentifier;
      float lerpPos;
      sf::Uint8 lives;
      packet >> paddleIdentifier >> lerpPos >> lives;

      Paddle* paddle = mWorld.addPaddle(paddleIdentifier, true);
      paddle->setSplinePos(lerpPos);
      paddle->setLives(lives);

      mPlayers[paddleIdentifier].reset(new Player(&mSocket, paddleIdentifier, getContext().keys));
      mLocalPlayerIdentifier = paddleIdentifier;

      //mGameStarted = true;
    } break;

    // 
    case static_cast<sf::Uint8>(ServerPacket::InitialState):
    {
      sf::Uint8 paddleCount;
      packet >> paddleCount;

      for (sf::Uint8 i = 0u; i < paddleCount; ++i) {
        sf::Int8 paddleIdentifier;
        float paddleLerpPosition;
        sf::Uint8 lives;
        packet >> paddleIdentifier >> paddleLerpPosition >> lives;

        Paddle* paddle = mWorld.addPaddle(paddleIdentifier);
        paddle->setSplinePos(paddleLerpPosition);
        paddle->setLives(lives);

        mPlayers[paddleIdentifier].reset(new Player(&mSocket, paddleIdentifier, nullptr));
      }
    } break;

    case static_cast<sf::Uint8>(ServerPacket::StartGame):
    {
      mGameStarted = true;
    } break;

    case static_cast<sf::Uint8>(ServerPacket::SpawnBall):
    {
      float angle, speed, delay;
      sf::Uint8 ballType;
      packet >> angle >> speed >> delay >> ballType;

      mWorld.addBall(ballType, angle, speed, delay);
    } break;

    case static_cast<sf::Uint8>(ServerPacket::RemoveBalls) :
    {
      mWorld.removeBalls();
    } break;

    case static_cast<sf::Uint8>(ServerPacket::RemoveDeadPlayer) :
    {
      sf::Int8 paddleIdentifier;
      packet >> paddleIdentifier;

      if (mLocalPlayerIdentifier == paddleIdentifier) {
        mIsSpectating = true;
      }

      mWorld.removePaddle(paddleIdentifier);
      mPlayers.erase(paddleIdentifier);

    } break;

    // Player event occurs
    case static_cast<sf::Uint8>(ServerPacket::PlayerEvent):
    {
      sf::Int8 paddleIdentifier;
      sf::Int32 action;
      packet >> paddleIdentifier >> action;

      auto itr = mPlayers.find(paddleIdentifier);
      if (itr != mPlayers.end())
        itr->second->handleNetworkEvent(static_cast<Player::Action>(action), mWorld.getCommandQueue());
    } break;

    // Player's movement keyboard state changes
    case static_cast<sf::Uint8>(ServerPacket::PlayerRealtimeChange):
    {
      sf::Int8 paddleIdentifier;
      sf::Int32 action;
      bool actionEnabled;
      packet >> paddleIdentifier >> action >> actionEnabled;

      auto itr = mPlayers.find(paddleIdentifier);
      if (itr != mPlayers.end())
        itr->second->handleNetworkRealtimeChange(static_cast<Player::Action>(action), actionEnabled);
    } break;

    case static_cast<sf::Uint8>(ServerPacket::PlayerConnect) :
    {
      sf::Int8 paddleIdentifier;
      float paddleLerpPosition;
      sf::Uint8 lives;
      packet >> paddleIdentifier >> paddleLerpPosition >> lives;

      Paddle* paddle = mWorld.addPaddle(paddleIdentifier);
      paddle->setSplinePos(paddleLerpPosition);
      paddle->setLives(lives);

      mPlayers[paddleIdentifier].reset(new Player(&mSocket, paddleIdentifier, nullptr));
    } break;

	case static_cast<sf::Uint8>(ServerPacket::PlayerDisconnect) :
	{
		sf::Int8 paddleIdentifier;
		packet >> paddleIdentifier;

		mWorld.removePaddle(paddleIdentifier);
		mPlayers.erase(paddleIdentifier);
	} break;

    case static_cast<sf::Uint8>(ServerPacket::UpdateClientState):
    {
      sf::Uint8 paddleCount;
      packet >> paddleCount;

      for (sf::Uint8 i = 0u; i < paddleCount; ++i) {
        float paddleLerpPosition;
        sf::Int8 paddleIdentifier;
        sf::Uint8 lives;
        packet >> paddleIdentifier >> paddleLerpPosition >> lives;

        Paddle* paddle = mWorld.getPaddle(paddleIdentifier);
        bool isLocalPaddle = mLocalPlayerIdentifier == paddleIdentifier;
        if (paddle && !isLocalPaddle) {
          float interpolatedLerpPosition = paddle->getSplinePos() + (paddleLerpPosition - paddle->getSplinePos()) * 0.1f;
          paddle->setSplinePos(interpolatedLerpPosition);
          if (paddle->getLives() != lives) { paddle->setLives(lives); } //Only set lives if they have changed, reduces some processing
        }
      }
    } break;

    case static_cast<sf::Uint8>(ServerPacket::EndGame) :
    {
      mWorld.setGameEndEffect();
      requestStackPush(StateID::EndGame);
    } break;
  }
}
