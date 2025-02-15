#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

class MusicPlayer : private sf::NonCopyable {
public:
  MusicPlayer();

  void play(MusicID theme);
  void stop();

  void setPaused(bool paused);
  void setVolume(float volume);


private:
  sf::Music mMusic;
  std::unordered_map<MusicID, std::string> mFilenames;
  float mVolume;
};