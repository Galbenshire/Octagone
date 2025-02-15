#include "pch.h"
#include "GameOverState.hpp"
#include "Utility.hpp"
#include "Constants.hpp"
#include "ResourceHolder.hpp"

GameOverState::GameOverState(StateStack & stack, Context context)
  : State(stack, context)
  , mGameOverText()
  , mElapsedTime(sf::Time::Zero)
{
  sf::Font& font = context.fonts->get(FontID::Main);
  sf::Vector2f windowSize(context.window->getSize());

  mGameOverText.setFont(font);
  mGameOverText.setString("--FINISHED--");
  mGameOverText.setCharacterSize(50);
  mGameOverText.setFillColor(getGameColor(GameColors::COLOR_FOUR));
  centerOrigin(mGameOverText);
  mGameOverText.setPosition(0.5f * windowSize);
}

void GameOverState::draw() {
  sf::RenderWindow& window = *getContext().window;
  window.setView(window.getDefaultView());

  // Create dark, semitransparent background
  sf::RectangleShape backgroundShape;
  backgroundShape.setFillColor(getGameColor(GameColors::COLOR_ONE));
  backgroundShape.setSize(sf::Vector2f(VIEW_WIDTH, VIEW_HEIGHT / 3));
  backgroundShape.move(0.f, VIEW_HEIGHT / 3);

  window.draw(backgroundShape);
  window.draw(mGameOverText);
}

bool GameOverState::update(sf::Time dt) {
  // Show state for 3 seconds, after return to menu
  mElapsedTime += dt;
  if (mElapsedTime > sf::seconds(3)) {
    requestStateClear();
    requestStackPush(StateID::MainMenu);
  }
  return false;
}

bool GameOverState::handleEvent(const sf::Event & event) {
  return false;
}
