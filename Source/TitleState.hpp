#pragma once

#include "State.hpp"
#include "ScrollingPattern.hpp"

class TitleState : public State {
public:
  TitleState(StateStack& stack, Context context);

  virtual void draw();
  virtual bool update(sf::Time dt);
  virtual bool handleEvent(const sf::Event& event);

private:
  enum TextID {
    TITLE,
    PRESS_START,
    COUNT
  };

private:
  ScrollingPattern mPattern; //The pattern you'll see on the screen
  std::vector<sf::Text> mTexts;

  bool mShowText;
  sf::Time mTextEffectTime;
};