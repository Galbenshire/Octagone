#include "pch.h"
#include "StateStack.hpp"

StateStack::StateStack(State::Context context)
  : mStack()
  , mPendingList()
  , mContext(context)
  , mFactories()
{
}

void StateStack::update(sf::Time dt) {
  // Iterate from top to bottom, stop as soon as update() returns false
  for (auto itr = mStack.rbegin(); itr != mStack.rend(); ++itr) {
    if (!(*itr)->update(dt))
      break;
  }

  applyPendingChanges();
}

void StateStack::draw() {
  // Draw all active states from bottom to top
  for (State::Ptr& state : mStack)
    state->draw();
}

void StateStack::handleEvent(const sf::Event & event) {
  // Iterate from top to bottom, stop as soon as handleEvent() returns false
  for (auto itr = mStack.rbegin(); itr != mStack.rend(); ++itr) {
    if (!(*itr)->handleEvent(event))
      break;
  }

  applyPendingChanges();
}

void StateStack::pushState(StateID stateID) {
  mPendingList.push_back(PendingChange(Action::Push, stateID));
}

void StateStack::popState() {
  mPendingList.push_back(PendingChange(Action::Pop));
}

void StateStack::clearStates() {
  mPendingList.push_back(PendingChange(Action::Clear));
}

bool StateStack::isEmpty() const {
  return mStack.empty();
}

State::Ptr StateStack::createState(StateID stateID) {
  auto found = mFactories.find(stateID);
  assert(found != mFactories.end());

  return found->second();
}

void StateStack::applyPendingChanges() {
  for (PendingChange change : mPendingList) {
    switch (change.action) {
    case Action::Push:
      mStack.push_back(createState(change.stateID));
      break;

    case Action::Pop:
      mStack.back()->onDestroy();
      mStack.pop_back();

      if (!mStack.empty())
        mStack.back()->onActivate();
      break;

    case Action::Clear:
      for (State::Ptr& state : mStack)
        state->onDestroy();

      mStack.clear();
      break;
    }
  }

  mPendingList.clear();
}

StateStack::PendingChange::PendingChange(Action action, StateID stateID)
  : action(action)
  , stateID(stateID)
{
}
