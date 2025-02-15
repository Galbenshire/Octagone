#pragma once

#include "State.hpp"
#include "Container.hpp"

class PauseState : public State {
public:
  PauseState(StateStack& stack, Context context, bool letUpdatesThrough = false);
  ~PauseState();

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event& event);

private:
  sf::Text mPausedText;
  sf::Time mPausedTextInterval;
  GUI::Container mGUIContainer;
  bool mLetUpdatesThrough;
};