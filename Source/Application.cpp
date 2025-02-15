#include "pch.h"
#include "Application.hpp"
#include "Constants.hpp"
#include "TitleState.hpp"
#include "MainMenuState.hpp"
#include "MultiplayerGameState.hpp"
#include "SettingsState.hpp"
#include "CreditsState.hpp"
#include "PauseState.hpp"
#include "GameOverState.hpp"
#include "IPConfigState.hpp"

const sf::Time Application::TimePerFrame = sf::seconds(1.f / 60.f);
sf::String Application::IPAddress = DEFAULT_IP_ADDRESS;

Application::Application()
  : mWindow(sf::VideoMode(VIEW_WIDTH, VIEW_HEIGHT), "Octagone", sf::Style::Close)
  , mTextures()
  , mFonts()
  , mMusic()
  , mSounds()
  , mKeyBinding()
  , mStateStack(State::Context(mWindow, mTextures, mFonts, mMusic, mSounds, mKeyBinding, IPAddress))
  , mStatisticsText()
  , mStatisticsUpdateTime()
  , mStatisticsNumFrames(0)
{
  mWindow.setKeyRepeatEnabled(false);
  mWindow.setVerticalSyncEnabled(true);

  mFonts.load(FontID::Main, "Media/Fonts/8bitOperatorPlusSC-Bold.ttf");

  mTextures.load(TextureID::TitleScreen, "Media/Textures/TitleScreen.png");
  mTextures.load(TextureID::Buttons, "Media/Textures/Button.png");
  mTextures.load(TextureID::Entities, "Media/Textures/Entities.png");
  mTextures.load(TextureID::Particle, "Media/Textures/Particle.png");
  mTextures.load(TextureID::Pixel, "Media/Textures/Pixel.png");

  mStatisticsText.setFont(mFonts.get(FontID::Main));
  mStatisticsText.setPosition(5.f, 5.f);
  mStatisticsText.setCharacterSize(10u);

  registerStates();
  mStateStack.pushState(StateID::Title);

  mMusic.setVolume(25.f);
}

void Application::run() {
  sf::Clock clock;
  sf::Time timeSinceLastUpdate = sf::Time::Zero;

  while (mWindow.isOpen()) {
    sf::Time dt = clock.restart();
    timeSinceLastUpdate += dt;
    while (timeSinceLastUpdate > TimePerFrame) {
      timeSinceLastUpdate -= TimePerFrame;

      processInput();
      update(TimePerFrame);
      
      // Check inside this loop, because stack might be empty before update() call
      if (mStateStack.isEmpty())
        mWindow.close();
    }

    updateStatistics(dt);
    render();
  }
}

void Application::processInput() {
  sf::Event event;
  while (mWindow.pollEvent(event)) {
    mStateStack.handleEvent(event);

    if (event.type == sf::Event::Closed)
      mWindow.close();
  }
}

void Application::update(sf::Time dt) {
  mStateStack.update(dt);
}

void Application::render() {
  mWindow.clear();

  mStateStack.draw();

  mWindow.setView(mWindow.getDefaultView());
  mWindow.draw(mStatisticsText);

  mWindow.display();
}

void Application::updateStatistics(sf::Time dt) {
  mStatisticsUpdateTime += dt;
  mStatisticsNumFrames += 1;
  if (mStatisticsUpdateTime >= sf::seconds(1.0f)) {
    mStatisticsText.setString("FPS: " + std::to_string(mStatisticsNumFrames));

    mStatisticsUpdateTime -= sf::seconds(1.0f);
    mStatisticsNumFrames = 0;
  }
}

void Application::registerStates() {
  mStateStack.registerState<TitleState>(StateID::Title);
  mStateStack.registerState<MainMenuState>(StateID::MainMenu);
  mStateStack.registerState<MultiplayerGameState>(StateID::HostGame, true);
  mStateStack.registerState<MultiplayerGameState>(StateID::JoinGame, false);
  mStateStack.registerState<PauseState>(StateID::NetworkPause, true);
  mStateStack.registerState<SettingsState>(StateID::Settings);
  mStateStack.registerState<CreditsState>(StateID::Credits);
  mStateStack.registerState<GameOverState>(StateID::EndGame);
  mStateStack.registerState<IPConfigState>(StateID::IPConfig);
}
