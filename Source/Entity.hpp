#pragma once
#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"

class Entity : public SceneNode {
public:
  Entity(const sf::Texture& texture, const sf::IntRect& textureRect);

  void setVelocity(sf::Vector2f velocity);
  void setVelocity(float vx, float vy);
  void accelerate(sf::Vector2f velocity);
  void accelerate(float vx, float vy);
  sf::Vector2f getVelocity() const;

  void playLocalSound(CommandQueue& commands, SFXID effect);

  void destroy();
  virtual void remove();
  virtual bool isDestroyed() const;

protected:
  virtual void updateCurrent(sf::Time dt, CommandQueue& commands);

protected:
  sf::Sprite mSprite;

private:
  sf::Vector2f mVelocity;
  bool mDestroyed;
};