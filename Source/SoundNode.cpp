#include "pch.h"
#include "SoundNode.hpp"
#include "SoundPlayer.hpp"

SoundNode::SoundNode(SoundPlayer & player)
  : SceneNode()
  , mSounds(player)
{
}

void SoundNode::playSound(SFXID sound, sf::Vector2f position) {
  mSounds.play(sound, position);
}

unsigned int SoundNode::getCategory() const {
  return Category::SoundEffect;
}
