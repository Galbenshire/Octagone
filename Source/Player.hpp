#pragma once

#include "Command.hpp"
#include "PlayerBinding.hpp"

class CommandQueue;

class Player : private sf::NonCopyable {
public:
  typedef PlayerAction::Type Action;

public:
  Player(sf::TcpSocket* socket, sf::Int8 identifier, const PlayerBinding* binding);

  void handleEvent(const sf::Event& event, const bool& hasFocus, CommandQueue& commands);
  void handleRealtimeInput(CommandQueue& commands);
  void handleRealtimeNetworkInput(CommandQueue& commands);

  // React to events or realtime state changes received over the network
  void handleNetworkEvent(Action action, CommandQueue& commands);
  void handleNetworkRealtimeChange(Action action, bool actionEnabled);

  void disableAllRealtimeActions();
  bool isLocal() const;

private:
  void initializeActions();

private:
  const PlayerBinding* mKeyBinding;
  std::map<Action, Command>	mActionBinding;
  std::map<Action, bool> mActionProxies;
  sf::Int8 mIdentifier;
  sf::TcpSocket* mSocket;
};