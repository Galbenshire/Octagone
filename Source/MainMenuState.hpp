#pragma once
#include "State.hpp"
#include "Container.hpp"
#include "ScrollingPattern.hpp"

class MainMenuState : public State {
public:
  MainMenuState(StateStack& stack, Context context);

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event& event);

private:
  ScrollingPattern mPattern;
  GUI::Container mGUIContainer;
};