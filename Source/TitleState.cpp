#include "pch.h"
#include "TitleState.hpp"
#include "Constants.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

TitleState::TitleState(StateStack & stack, Context context)
  : State(stack, context)
  , mPattern(context.textures->get(TextureID::TitleScreen))
  , mTexts(TextID::COUNT)
  , mShowText(true)
  , mTextEffectTime(sf::Time::Zero)
{
  mPattern.setColor(getGameColor(GameColors::COLOR_ONE), ScrollingPattern::Pattern);

  for (auto& text : mTexts) {
    text.setFont(context.fonts->get(FontID::Main));
    text.setFillColor(getGameColor(GameColors::COLOR_FOUR));
  }

  mTexts[TITLE].setCharacterSize(50u);
  mTexts[TITLE].setString("OCTAGONE");
  centerOrigin(mTexts[TITLE]);
  mTexts[TITLE].setPosition(sf::Vector2f(context.window->getSize().x  * 0.5f, context.window->getSize().y * 0.2f));

  mTexts[PRESS_START].setString("PRESS ANY KEY");
  centerOrigin(mTexts[PRESS_START]);
  mTexts[PRESS_START].setPosition(sf::Vector2f(context.window->getSize() / 2u));
}

void TitleState::draw() {
  sf::RenderWindow& window = *getContext().window;
  window.setView(window.getDefaultView());

  window.draw(mPattern);
  window.draw(mTexts[TITLE]);

  if (mShowText)
    window.draw(mTexts[PRESS_START]);
}

bool TitleState::update(sf::Time dt) {
  mPattern.update();

  mTextEffectTime += dt;

  if (mTextEffectTime >= sf::seconds(0.5f)) {
    mShowText = !mShowText;
    mTextEffectTime = sf::Time::Zero;
  }

  return true;
}

bool TitleState::handleEvent(const sf::Event & event) {
  // If any key is pressed, change the color of the background
  if (event.type == sf::Event::KeyPressed || event.type == sf::Event::JoystickButtonPressed) {
    mPattern.setVisible(false, ScrollingPattern::Pattern);
  }
  // Once that key is released, trigger the next screen
  else if (event.type == sf::Event::KeyReleased || event.type == sf::Event::JoystickButtonReleased) {
    requestStackPop();
    requestStackPush(StateID::MainMenu);
  }

  return true;
}
