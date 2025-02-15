#include "pch.h"
#include "Player.hpp"
#include "CommandQueue.hpp"
#include "Paddle.hpp"
#include "NetworkProtocol.hpp"

struct PaddleMover {

  PaddleMover(PlayerAction::Type type, int identifier)
    : type(type)
    , paddleID(identifier)
  {}

  void operator() (Paddle& paddle, sf::Time dt) const {
    if (paddle.getIdentifier() == paddleID)
      paddle.moveAlongSplinePos(type, dt);
  }

  PlayerAction::Type type;
  int paddleID;
};

Player::Player(sf::TcpSocket * socket, sf::Int8 identifier, const PlayerBinding * binding)
  : mKeyBinding(binding)
  , mIdentifier(identifier)
  , mSocket(socket)
{
  // Set initial action bindings
  initializeActions();

  // Assign all categories to player's aircraft
  for (auto& pair : mActionBinding)
    pair.second.category = Category::PlayerPaddle;
}

void Player::handleEvent(const sf::Event & event, const bool & hasFocus, CommandQueue & commands) {
  if (!mKeyBinding) { return;  }

  // Event
  if (event.type == sf::Event::KeyPressed) {
    Action action;
    if (mKeyBinding->checkAction(event.key.code, action) && !PlayerBinding::isRealtimeAction(action)) {
      // Network connected -> send event over network
      if (mSocket) {
        sf::Packet packet;
        packet << static_cast<sf::Uint8>(ClientPacket::PlayerEvent);
        packet << mIdentifier;
        packet << static_cast<sf::Int32>(action);
        mSocket->send(packet);
      }
      // Network disconnected -> local event
      else {
        commands.push(mActionBinding[action]);
      }
    }
  }

  // Realtime change (network connected)
  if ((event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased || event.type == sf::Event::JoystickMoved) && mSocket) {
    Action action;
    
    if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
      if (mKeyBinding->checkAction(event.key.code, action) && PlayerBinding::isRealtimeAction(action)) {
        // Send realtime change over network
        sf::Packet packet;
        packet << static_cast<sf::Uint8>(ClientPacket::PlayerRealtimeChange);
        packet << mIdentifier;
        packet << static_cast<sf::Int32>(action);
        packet << (event.type == sf::Event::KeyPressed);
        mSocket->send(packet);
      }
    } else if (event.type == sf::Event::JoystickMoved) {
      Action oppositeAction;
      if (mKeyBinding->checkAction(event.joystickMove.axis, event.joystickMove.position, action, oppositeAction) && PlayerBinding::isRealtimeAction(action)) {
        // Send realtime change over network
        sf::Packet packet;
        packet << static_cast<sf::Uint8>(ClientPacket::PlayerRealtimeChangeGamepad);
        packet << mIdentifier;
        packet << static_cast<sf::Int32>(action);
        packet << (std::abs(event.joystickMove.position) > AXIS_DEADZONE && hasFocus);
        packet << static_cast<sf::Int32>(oppositeAction);
        mSocket->send(packet);
      }
    }
  }
}

void Player::handleRealtimeInput(CommandQueue & commands) {
  // Check if this is a networked game and local player or just a single player game
  if ((mSocket && isLocal()) || !mSocket) {
    // Lookup all actions and push corresponding commands to queue
    std::vector<Action> activeActions = mKeyBinding->getRealtimeActions();
    for (Action action : activeActions)
      commands.push(mActionBinding[action]);
  }
}

void Player::handleRealtimeNetworkInput(CommandQueue & commands) {
  if (mSocket && !isLocal()) {
    // Traverse all realtime input proxies. Because this is a networked game, the input isn't handled directly
    for (auto pair : mActionProxies) {
      if (pair.second && PlayerBinding::isRealtimeAction(pair.first))
        commands.push(mActionBinding[pair.first]);
    }
  }
}

void Player::handleNetworkEvent(Action action, CommandQueue & commands) {
  commands.push(mActionBinding[action]);
}

void Player::handleNetworkRealtimeChange(Action action, bool actionEnabled) {
  mActionProxies[action] = actionEnabled;
}

void Player::disableAllRealtimeActions() {
  for (auto& action : mActionProxies) {
    sf::Packet packet;
    packet << static_cast<sf::Uint8>(ClientPacket::PlayerRealtimeChange);
    packet << mIdentifier;
    packet << static_cast<sf::Int32>(action.first);
    packet << false;
    mSocket->send(packet);
  }
}

bool Player::isLocal() const {
  // No key binding means this player is remote
  return mKeyBinding != nullptr;
}

void Player::initializeActions() {
  mActionBinding[PlayerAction::MoveLeft].action = derivedAction<Paddle>(PaddleMover(PlayerAction::MoveLeft, mIdentifier));
  mActionBinding[PlayerAction::MoveRight].action = derivedAction<Paddle>(PaddleMover(PlayerAction::MoveRight, mIdentifier));
  mActionBinding[PlayerAction::MoveUp].action = derivedAction<Paddle>(PaddleMover(PlayerAction::MoveUp, mIdentifier));
  mActionBinding[PlayerAction::MoveDown].action = derivedAction<Paddle>(PaddleMover(PlayerAction::MoveDown, mIdentifier));
}
