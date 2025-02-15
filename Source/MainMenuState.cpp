#include "pch.h"
#include "MainMenuState.hpp"
#include "Button.hpp"
#include "Label.hpp"
#include "Constants.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

//Data specific to this class
namespace {
  const float ScrollSpeed = 1.5f;
}

MainMenuState::MainMenuState(StateStack & stack, Context context)
  : State(stack, context)
  , mPattern(context.textures->get(TextureID::TitleScreen), sf::Vector2f(ScrollSpeed, 0.f))
  , mGUIContainer()
{
  mPattern.setColor(getGameColor(GameColors::COLOR_TWO), ScrollingPattern::Pattern);
  mPattern.setColor(getGameColor(GameColors::COLOR_ONE), ScrollingPattern::Background);

  auto hostButton = std::make_shared<GUI::Button>(context);
  hostButton->setPosition(16, 48);
  hostButton->setText("Host Game");
  hostButton->setCallback([this]() {
    requestStackPop();
    requestStackPush(StateID::HostGame);
  });

  auto joinButton = std::make_shared<GUI::Button>(context);
  joinButton->setPosition(16, 112);
  joinButton->setText("Join Game");
  joinButton->setCallback([this]() {
    requestStackPop();
    requestStackPush(StateID::JoinGame);
  });

  auto settingsButton = std::make_shared<GUI::Button>(context);
  settingsButton->setPosition(16, 176);
  settingsButton->setText("Settings");
  settingsButton->setCallback([this]() {
    requestStackPush(StateID::Settings);
  });

  auto ipConfigButton = std::make_shared<GUI::Button>(context);
  ipConfigButton->setPosition(16, 240);
  ipConfigButton->setText("IP Config");
  ipConfigButton->setCallback([this]() {
	  requestStackPush(StateID::IPConfig);
  });

  auto creditsButton = std::make_shared<GUI::Button>(context);
  creditsButton->setPosition(16, 304);
  creditsButton->setText("Credits");
  creditsButton->setCallback([this]() {
	  requestStackPush(StateID::Credits);
  });

  auto exitLabel = std::make_shared<GUI::Label>("PRESS ESC TO QUIT", *context.fonts);
  exitLabel->setPosition(VIEW_WIDTH / 2.f, VIEW_WIDTH - 100.f);

  mGUIContainer.pack(hostButton);
  mGUIContainer.pack(joinButton);
  mGUIContainer.pack(settingsButton);
  mGUIContainer.pack(ipConfigButton);
  mGUIContainer.pack(creditsButton);
  mGUIContainer.pack(exitLabel);
}

void MainMenuState::draw() {
  sf::RenderWindow& window = *getContext().window;
  window.setView(window.getDefaultView());

  window.draw(mPattern);
  window.draw(mGUIContainer);
}

bool MainMenuState::update(sf::Time dt) {
  mPattern.update();

  return true;
}

bool MainMenuState::handleEvent(const sf::Event & event) {
  if (event.type == sf::Event::KeyPressed) {
    if (event.key.code == sf::Keyboard::Escape) {
		requestStateClear();
		return false;
	}
  }

  mGUIContainer.handleEvent(event);
  return false;
}
