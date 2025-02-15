#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "PlayerBinding.hpp"
#include "StateStack.hpp"
#include "MusicPlayer.hpp"
#include "SoundPlayer.hpp"


class Application {
public:
  Application();
  void run();

private:
  void processInput();
  void update(sf::Time dt);
  void render();

  void updateStatistics(sf::Time dt);
  void registerStates();

private:
  static const sf::Time	TimePerFrame;
  static sf::String IPAddress;

  sf::RenderWindow mWindow;
  TextureHolder mTextures;
  FontHolder mFonts;
  MusicPlayer mMusic;
  SoundPlayer mSounds;
  PlayerBinding mKeyBinding;
  StateStack mStateStack;

  sf::Text mStatisticsText;
  sf::Time mStatisticsUpdateTime;
  std::size_t mStatisticsNumFrames;
};