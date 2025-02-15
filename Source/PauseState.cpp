#include "pch.h"
#include "PauseState.hpp"
#include "Button.hpp"
#include "Utility.hpp"
#include "Constants.hpp"
#include "ResourceHolder.hpp"

PauseState::PauseState(StateStack & stack, Context context, bool letUpdatesThrough)
  : State(stack, context)
  , mPausedText()
  , mPausedTextInterval(sf::seconds(0.5f))
  , mGUIContainer()
  , mLetUpdatesThrough(letUpdatesThrough)
{
  sf::Font& font = context.fonts->get(FontID::Main);
  sf::Vector2f windowSize(context.window->getSize());

  mPausedText.setFont(font);
  mPausedText.setString("-PAUSE-");
  mPausedText.setCharacterSize(30);
  mPausedText.setFillColor(getGameColor(GameColors::COLOR_ONE));
  centerOrigin(mPausedText);
  mPausedText.setPosition(0.2f * windowSize.x, 0.5f * windowSize.y);

  auto returnButton = std::make_shared<GUI::Button>(context);
  returnButton->setPosition(0.6f * windowSize.x, 0.5f * windowSize.y - 60.f);
  returnButton->setText("RETURN");
  returnButton->setCallback([this]() {
    requestStackPop();
  });

  auto backToMenuButton = std::make_shared<GUI::Button>(context);
  backToMenuButton->setPosition(0.6f * windowSize.x, 0.5f * windowSize.y + 12.f);
  backToMenuButton->setText("TO MENU");
  backToMenuButton->setCallback([this]() {
    requestStateClear();
    requestStackPush(StateID::MainMenu);
  });

  mGUIContainer.pack(returnButton);
  mGUIContainer.pack(backToMenuButton);
}

PauseState::~PauseState()
{}

void PauseState::draw() {
  sf::RenderWindow& window = *getContext().window;
  window.setView(window.getDefaultView());

  window.draw(mPausedText);
  window.draw(mGUIContainer);
}

bool PauseState::update(sf::Time dt) {
  mPausedTextInterval -= dt;
  if (mPausedTextInterval <= sf::Time::Zero) {
    if (mPausedText.getFillColor() == getGameColor(GameColors::COLOR_ONE))
      mPausedText.setFillColor(getGameColor(GameColors::COLOR_FOUR));
    else
      mPausedText.setFillColor(getGameColor(GameColors::COLOR_ONE));
    mPausedTextInterval = sf::seconds(0.5f);
  }

  return mLetUpdatesThrough;
}

bool PauseState::handleEvent(const sf::Event & event) {
  mGUIContainer.handleEvent(event);
  return false;
}
