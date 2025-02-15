#pragma once

#include "State.hpp"
#include "StateIdentifiers.hpp"
#include "ResourceIdentifiers.hpp"

//Forward declaration
namespace sf {
  class Event;
  class RenderWindow;
}

class StateStack : private sf::NonCopyable {
public :
  enum class Action {
    Push,
    Pop,
    Clear
  };

public:
  explicit StateStack(State::Context context);

  template <typename T>
  void registerState(StateID stateID);
  template <typename T, typename Param1>
  void registerState(StateID stateID, Param1 arg1);

  void update(sf::Time dt);
  void draw();
  void handleEvent(const sf::Event& event);

  void pushState(StateID stateID);
  void popState();
  void clearStates();

  bool isEmpty() const;

private:
  State::Ptr createState(StateID stateID);
  void applyPendingChanges();

private:
  struct PendingChange {
    explicit PendingChange(Action action, StateID stateID = StateID::None);

    Action action;
    StateID stateID;
  };

private:
  std::vector<State::Ptr> mStack;
  std::vector<PendingChange> mPendingList;

  State::Context mContext;
  std::unordered_map<StateID, std::function<State::Ptr()>> mFactories;
};

template <typename T>
void StateStack::registerState(StateID stateID) {
  mFactories[stateID] = [this]() {
    return State::Ptr(new T(*this, mContext));
  };
}

template <typename T, typename Param1>
void StateStack::registerState(StateID stateID, Param1 arg1) {
  mFactories[stateID] = [this, arg1]() {
    return State::Ptr(new T(*this, mContext, arg1));
  };
}