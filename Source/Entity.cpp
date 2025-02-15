#include "pch.h"
#include "Entity.hpp"
#include "CommandQueue.hpp"
#include "SoundNode.hpp"

Entity::Entity(const sf::Texture & texture, const sf::IntRect & textureRect)
  : mSprite(texture, textureRect)
  , mDestroyed(false)
{
}

void Entity::setVelocity(sf::Vector2f velocity) {
  mVelocity = velocity;
}

void Entity::setVelocity(float vx, float vy) {
  mVelocity.x = vx;
  mVelocity.y = vy;
}

void Entity::accelerate(sf::Vector2f velocity) {
  mVelocity += velocity;
}

void Entity::accelerate(float vx, float vy) {
  mVelocity.x += vx;
  mVelocity.y += vy;
}

sf::Vector2f Entity::getVelocity() const {
  return mVelocity;
}

void Entity::playLocalSound(CommandQueue & commands, SFXID effect) {
  sf::Vector2f worldPosition = getWorldPosition();

  Command command;
  command.category = Category::SoundEffect;
  command.action = derivedAction<SoundNode>([effect, worldPosition](SoundNode& node, sf::Time) {
    node.playSound(effect, worldPosition);
  });

  commands.push(command);
}

void Entity::destroy() {
  mDestroyed = true;
}

void Entity::remove() {
  destroy();
}

bool Entity::isDestroyed() const
{
  return mDestroyed;
}

void Entity::updateCurrent(sf::Time dt, CommandQueue & commands) {
  move(mVelocity * dt.asSeconds());
}
