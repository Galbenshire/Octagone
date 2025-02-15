#include "pch.h"
#include "PlayerBinding.hpp"
#include "Constants.hpp"

PlayerBinding::PlayerBinding()
  : mKeyMap()
  , mAxisMap()
{
  //Set the inputs for the player actions
  initializeActionsInFile();
  initializeDefaultActions();
}

void PlayerBinding::assignKey(Action action, sf::Keyboard::Key key) {
  // Remove all keys that already map to action
  for (auto itr = mKeyMap.begin(); itr != mKeyMap.end();) {
    if (itr->second == action)
      mKeyMap.erase(itr++);
    else
      ++itr;
  }

  // Insert new binding
  mKeyMap[key] = action;
}

sf::Keyboard::Key PlayerBinding::getAssignedKey(Action action) const {
  for (auto pair : mKeyMap) {
    if (pair.second == action)
      return pair.first;
  }

  return sf::Keyboard::Unknown;
}

bool PlayerBinding::checkAction(sf::Keyboard::Key key, Action & out) const {
  auto found = mKeyMap.find(key);
  if (found == mKeyMap.end()) {
    return false;
  } else {
    out = found->second;
    return true;
  }
}

bool PlayerBinding::checkAction(sf::Joystick::Axis axis, float position, Action& out, Action& outOpposite) const {

  auto found = mAxisMap.find(axis);
  if (found == mAxisMap.end()) {
	  return false;
  } else {
    out = (position > 0.f) ? found->second.positiveAction : found->second.negativeAction;
    outOpposite = (position > 0.f) ? found->second.negativeAction : found->second.positiveAction;
    return true;
  }
}

std::vector<PlayerBinding::Action> PlayerBinding::getRealtimeActions() const {
  // Return all realtime actions that are currently active.
  std::vector<Action> actions;

  for (auto pair : mKeyMap) {
    // If key is pressed and an action is a realtime action, store it
    if (sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
      actions.push_back(pair.second);
  }

  for (auto pair : mAxisMap) {
    if (sf::Joystick::getAxisPosition(0, pair.first) > AXIS_DEADZONE)
      actions.push_back(pair.second.positiveAction);
    else if (sf::Joystick::getAxisPosition(0, pair.first) < -AXIS_DEADZONE)
      actions.push_back(pair.second.negativeAction);
  }

  return actions;
}

void PlayerBinding::resetBindings() {
  mKeyMap.clear();
  initializeDefaultActions();
}

void PlayerBinding::initializeDefaultActions() {
  //Keyboard Input
  if (mKeyMap.size() != PlayerAction::Count) {
    mKeyMap.clear();

    mKeyMap[sf::Keyboard::A] = PlayerAction::MoveLeft;
    mKeyMap[sf::Keyboard::D] = PlayerAction::MoveRight;
    mKeyMap[sf::Keyboard::W] = PlayerAction::MoveUp;
    mKeyMap[sf::Keyboard::S] = PlayerAction::MoveDown;
  }

  //Gamepad Input
  mAxisMap[sf::Joystick::PovX] = AxisAction(PlayerAction::MoveRight, PlayerAction::MoveLeft);
  mAxisMap[sf::Joystick::PovY] = AxisAction(PlayerAction::MoveUp, PlayerAction::MoveDown);
}

void PlayerBinding::initializeActionsInFile() {
  //First, check if the file exists
  std::ifstream keyFile(BINDINGS_FILENAME);
  if (!keyFile) { return; } //File not found

  //Parse any valid bindings from the file using regex
  //Using this webpage as reference
  //https://stackoverflow.com/questions/29321249/regex-grouping-matches-with-c-11-regex-library
  //Accessed 27/02/19 11:53PM
  std::regex bindFormat("^([0-3]):(\\d+)$");
  std::smatch bindMatch;
  std::string line;
  while (std::getline(keyFile, line)) {
    if (std::regex_match(line, bindMatch, bindFormat)) {
      mKeyMap[static_cast<sf::Keyboard::Key>(std::stoi(bindMatch[2]))] = static_cast<PlayerAction::Type>(std::stoi(bindMatch[1]));
    }
  }
}

bool PlayerBinding::isRealtimeAction(PlayerAction::Type action) {
  switch (action) {
  case PlayerAction::MoveLeft:
  case PlayerAction::MoveRight:
  case PlayerAction::MoveDown:
  case PlayerAction::MoveUp:
    return true;

  default:
    return false;
  }
}
