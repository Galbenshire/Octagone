#include "pch.h"
#include "NetworkNode.hpp"

NetworkNode::NetworkNode()
  : SceneNode()
  , mPendingActions()
{
}

void NetworkNode::notifyGameAction(GameActions::Type type, sf::Vector2f position) {
  mPendingActions.push(GameActions::Action(type, position));
}

bool NetworkNode::pollGameAction(GameActions::Action & out) {
  if (mPendingActions.empty()) {
    return false;
  } else {
    out = mPendingActions.front();
    mPendingActions.pop();
    return true;
  }
}

unsigned int NetworkNode::getCategory() const
{
  return Category::Network;
}
