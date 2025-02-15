#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

class SoundPlayer : private sf::NonCopyable {
public:
  SoundPlayer();

  void play(SFXID effect);
  void play(SFXID effect, sf::Vector2f position);

  void removeStoppedSounds();
  void setListenerPosition(sf::Vector2f position);
  sf::Vector2f getListenerPosition() const;

private:
  SoundBufferHolder mSoundBuffers;
  std::list<sf::Sound> mSounds;
};
